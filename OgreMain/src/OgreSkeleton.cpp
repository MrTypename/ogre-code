/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreAnimationState.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeletonSerializer.h"
#include "OgreStringConverter.h"
// Just for logging
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"


namespace Ogre {

    //---------------------------------------------------------------------
	Skeleton::Skeleton()
		: Resource(),
		mNextAutoHandle(0), mBlendState(ANIMBLEND_AVERAGE)
	{
	}
	//---------------------------------------------------------------------
    Skeleton::Skeleton(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader) 
        : Resource(creator, name, handle, group, isManual, loader), 
        mNextAutoHandle(0), mBlendState(ANIMBLEND_AVERAGE)
        // set animation blending to weighted, not cumulative
    {
        if (createParamDictionary("Skeleton"))
        {
            // no custom params
        }
    }
    //---------------------------------------------------------------------
    Skeleton::~Skeleton()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
    }
    //---------------------------------------------------------------------
    void Skeleton::loadImpl(void)
    {
        SkeletonSerializer serializer;
		StringUtil::StrStreamType msg;
		msg << "Skeleton: Loading " << mName;
        LogManager::getSingleton().logMessage(msg.str());

        DataStreamPtr stream = 
            ResourceGroupManager::getSingleton().openResource(mName, mGroup);

        serializer.importSkeleton(stream, this);

    }
    //---------------------------------------------------------------------
    void Skeleton::unloadImpl(void)
    {
        // destroy bones
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            delete *i;
        }
        mBoneList.clear();
        mBoneListByName.clear();


        // Destroy animations
        AnimationList::iterator ai;
        for (ai = mAnimationsList.begin(); ai != mAnimationsList.end(); ++ai)
        {
            delete ai->second;
        }
        mAnimationsList.clear();

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(void)
    {
        // use autohandle
        return createBone(mNextAutoHandle++);
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(const String& name)
    {
        return createBone(name, mNextAutoHandle++);
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(unsigned short handle)
    {
        if (handle >= OGRE_MAX_NUM_BONES)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Exceeded the maximum number of bones per skeleton.",
                "Skeleton::createBone");
        }
        // Check handle not used
        if (handle < mBoneList.size() && mBoneList[handle] != NULL)
        {
            OGRE_EXCEPT(
                Exception::ERR_DUPLICATE_ITEM,
                "A bone with the handle " + StringConverter::toString(handle) + " already exists",
                "Skeleton::createBone" );
        }
        Bone* ret = new Bone(handle, this);
        assert(mBoneListByName.find(ret->getName()) == mBoneListByName.end());
        if (mBoneList.size() <= handle)
        {
            mBoneList.resize(handle+1);
        }
        mBoneList[handle] = ret;
        mBoneListByName[ret->getName()] = ret;
        return ret;

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(const String& name, unsigned short handle)
    {
        if (handle >= OGRE_MAX_NUM_BONES)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Exceeded the maximum number of bones per skeleton.",
                "Skeleton::createBone");
        }
        // Check handle not used
        if (handle < mBoneList.size() && mBoneList[handle] != NULL)
        {
            OGRE_EXCEPT(
                Exception::ERR_DUPLICATE_ITEM,
                "A bone with the handle " + StringConverter::toString(handle) + " already exists",
                "Skeleton::createBone" );
        }
        // Check name not used
        if (mBoneListByName.find(name) != mBoneListByName.end())
        {
            OGRE_EXCEPT(
                Exception::ERR_DUPLICATE_ITEM,
                "A bone with the name " + name + " already exists",
                "Skeleton::createBone" );
        }
        Bone* ret = new Bone(name, handle, this);
        if (mBoneList.size() <= handle)
        {
            mBoneList.resize(handle+1);
        }
        mBoneList[handle] = ret;
        mBoneListByName[name] = ret;
        return ret;
    }



    //---------------------------------------------------------------------
    Bone* Skeleton::getRootBone(void) const
    {
        if (mRootBones.empty())
        {
            deriveRootBone();
        }

        return mRootBones[0];
    }
    //---------------------------------------------------------------------
    void Skeleton::setAnimationState(const AnimationStateSet& animSet)
    {
        /* 
        Algorithm:
          1. Check if animation state is any different from last, if not do nothing
          2. Reset all bone positions
          3. Iterate per AnimationState, if enabled get Animation and call Animation::apply
        */

        if (mLastAnimationState.size() == animSet.size())
        {
            // Same size, may be able to skip update
            bool different = false;
            AnimationStateSet::iterator i;
            AnimationStateSet::const_iterator j;
            i = mLastAnimationState.begin();
            j = animSet.begin();
            for (; i != mLastAnimationState.end(); ++i, ++j)
            {
                if (i->second != j->second)
                {
                    different = true;
                    break;
                }
            }
            // Check any differences?
            if (!different)
            {
                // No, no need to update
                return;
            }
        }

        // Ok, we've established the animation state is different

        // Reset bones
        reset();

        // Per animation state
        AnimationStateSet::const_iterator istate;
        for (istate = animSet.begin(); istate != animSet.end(); ++istate)
        {
            // Apply if enabled
            const AnimationState& animState = istate->second;
            if (animState.getEnabled())
            {
                Animation* anim = getAnimation(animState.getAnimationName());
				anim->apply(this, animState.getTimePosition(), animState.getWeight(), mBlendState == ANIMBLEND_CUMULATIVE);
            }
        }

        mLastAnimationState = animSet;


    }
    //---------------------------------------------------------------------
    void Skeleton::setBindingPose(void)
    {
        // Update the derived transforms
        _updateTransforms();


        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {            
            (*i)->setBindingPose();
        }
    }
    //---------------------------------------------------------------------
    void Skeleton::reset(bool resetManualBones)
    {
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            if(!(*i)->isManuallyControlled() || resetManualBones)
                (*i)->reset();
        }
    }
    //---------------------------------------------------------------------
    Animation* Skeleton::createAnimation(const String& name, Real length)
    {
        // Check name not used
        if (mAnimationsList.find(name) != mAnimationsList.end())
        {
            OGRE_EXCEPT(
                Exception::ERR_DUPLICATE_ITEM,
                "An animation with the name " + name + " already exists",
                "Skeleton::createAnimation");
        }

        Animation* ret = new Animation(name, length);

        // Add to list
        mAnimationsList[name] = ret;

        // Also add to state
        mLastAnimationState[name] = AnimationState(name, 0, length);

        return ret;

    }
    //---------------------------------------------------------------------
    Animation* Skeleton::getAnimation(const String& name) const
    {
        AnimationList::const_iterator i = mAnimationsList.find(name);

        if (i == mAnimationsList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name, 
            "Skeleton::getAnimation");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    void Skeleton::removeAnimation(const String& name)
    {
        AnimationList::iterator i = mAnimationsList.find(name);

        if (i == mAnimationsList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name, 
            "Skeleton::getAnimation");
        }

        delete i->second;

        mAnimationsList.erase(i);

    }
    //---------------------------------------------------------------------
    const AnimationStateSet& Skeleton::getAnimationState(void) const
    {
        return mLastAnimationState;
    }
    //-----------------------------------------------------------------------
    void Skeleton::_initAnimationState(AnimationStateSet* animSet)
    {
        animSet->clear();
           
        AnimationList::iterator i;
        for (i = mAnimationsList.begin(); i != mAnimationsList.end(); ++i)
        {
            Animation* anim = i->second;
            // Create animation at time index 0, default params mean this has weight 1 and is disabled
            String animName = anim->getName();
            (*animSet)[animName] = AnimationState(animName, 0.0, anim->getLength());
        }
    }
    //-----------------------------------------------------------------------
    unsigned short Skeleton::getNumBones(void) const
    {
        return (unsigned short)mBoneList.size();
    }
    //-----------------------------------------------------------------------
    void Skeleton::_getBoneMatrices(Matrix4* pMatrices)
    {
        // Update derived transforms
        _updateTransforms();

        /* 
            Calculating the bone matrices
            -----------------------------
            Now that we have the derived orientations & positions in the Bone nodes, we have
            to compute the Matrix4 to apply to the vertices of a mesh.
            Because any modification of a vertex has to be relative to the bone, we must first
            reverse transform by the Bone's original derived position/orientation, then transform
            by the new derived position / orientation.
        */

        BoneList::iterator i, boneend;
        boneend = mBoneList.end();
        
       
        for(i = mBoneList.begin();i != boneend; ++i)
        {
            Bone* pBone = *i;
            *pMatrices = pBone->_getFullTransform() *  pBone->_getBindingPoseInverseTransform();
            pMatrices++;
        }

    }
    //---------------------------------------------------------------------
    unsigned short Skeleton::getNumAnimations(void) const
    {
        return (unsigned short)mAnimationsList.size();
    }
    //---------------------------------------------------------------------
    Animation* Skeleton::getAnimation(unsigned short index) const
    {
		// If you hit this assert, then the index is out of bounds.
        assert( index < mAnimationsList.size() );

        AnimationList::const_iterator i = mAnimationsList.begin();

        while (index--)
            ++i;

        return i->second;
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getBone(unsigned short handle) const
    {
        assert(handle < mBoneList.size() && "Index out of bounds");
        return mBoneList[handle];
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getBone(const String& name) const
    {
        BoneListByName::const_iterator i = mBoneListByName.find(name);

        if (i == mBoneListByName.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Bone named '" + name + "' not found.", 
                "Skeleton::getBone");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Skeleton::deriveRootBone(void) const
    {
        // Start at the first bone and work up
        if (mBoneList.empty())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot derive root bone as this "
                "skeleton has no bones!", "Skeleton::deriveRootBone");
        }

        mRootBones.empty();

        Bone* currentBone;
        BoneList::const_iterator i;
        BoneList::const_iterator iend = mBoneList.end();
        for (i = mBoneList.begin(); i != iend; ++i)
        {
            currentBone = *i;
            if (currentBone->getParent() == 0)
            {
                // This is a root
                mRootBones.push_back(currentBone);
            }
        }
    }
    //---------------------------------------------------------------------
    void Skeleton::_dumpContents(const String& filename)
    {
        std::ofstream of;

        Quaternion q;
        Radian angle;
        Vector3 axis;
        of.open(filename.c_str());

        of << "-= Debug output of skeleton " << mName << " =-" << std::endl << std::endl;
        of << "== Bones ==" << std::endl;
        of << "Number of bones: " << (unsigned int)mBoneList.size() << std::endl;
        
        BoneList::iterator bi;
        for (bi = mBoneList.begin(); bi != mBoneList.end(); ++bi)
        {
            Bone* bone = *bi;

            of << "-- Bone " << bone->getHandle() << " --" << std::endl;
            of << "Position: " << bone->getPosition();
            q = bone->getOrientation();
            of << "Rotation: " << q;
            q.ToAngleAxis(angle, axis);
            of << " = " << angle.valueRadians() << " radians around axis " << axis << std::endl << std::endl;
        }

        of << "== Animations ==" << std::endl;
        of << "Number of animations: " << (unsigned int)mAnimationsList.size() << std::endl;

        AnimationList::iterator ai;
        for (ai = mAnimationsList.begin(); ai != mAnimationsList.end(); ++ai)
        {
            Animation* anim = ai->second;

            of << "-- Animation '" << anim->getName() << "' (length " << anim->getLength() << ") --" << std::endl;
            of << "Number of tracks: " << anim->getNumTracks() << std::endl;

            int ti;
            for (ti = 0; ti < anim->getNumTracks(); ++ti)
            {
                AnimationTrack* track = anim->getTrack(ti);
                of << "  -- AnimationTrack " << ti << " --" << std::endl;
                of << "  Affects bone: " << ((Bone*)track->getAssociatedNode())->getHandle() << std::endl;
                of << "  Number of keyframes: " << track->getNumKeyFrames() << std::endl;

                int ki;
                
                for (ki = 0; ki < track->getNumKeyFrames(); ++ki)
                {
                    KeyFrame* key = track->getKeyFrame(ki);
                    of << "    -- KeyFrame " << ki << " --" << std::endl;
                    of << "    Time index: " << key->getTime(); 
                    of << "    Translation: " << key->getTranslate() << std::endl;
                    q = key->getRotation();
                    of << "    Rotation: " << q;
                    q.ToAngleAxis(angle, axis);
                    of << " = " << angle.valueRadians() << " radians around axis " << axis << std::endl;
                }

            }



        }

    }
    //---------------------------------------------------------------------
	SkeletonAnimationBlendMode Skeleton::getBlendMode() 
    {
		return mBlendState;
	}
    //---------------------------------------------------------------------
	void Skeleton::setBlendMode(SkeletonAnimationBlendMode state) 
    {
		mBlendState = state;
	}
    //---------------------------------------------------------------------
    Skeleton::BoneIterator Skeleton::getRootBoneIterator(void)
    {
        if (mRootBones.empty())
        {
            deriveRootBone();
        }
        return BoneIterator(mRootBones.begin(), mRootBones.end());
    }
    //---------------------------------------------------------------------
    Skeleton::BoneIterator Skeleton::getBoneIterator(void)
    {
        return BoneIterator(mBoneList.begin(), mBoneList.end());
    }
    //---------------------------------------------------------------------
    void Skeleton::_updateTransforms(void)
    {
        BoneList::iterator i, iend;
        iend = mRootBones.end();
        for (i = mRootBones.begin(); i != iend; ++i)
        {
            (*i)->_update(true, false);
        }
    }
}

