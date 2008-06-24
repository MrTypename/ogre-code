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
#include "OgreAnimationTrack.h"
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"
#include "OgreNode.h"
#include "OgreLogManager.h"

// Debug
#include "OgreRenderWindow.h"
#include "OgreRoot.h"
Ogre::RenderWindow* mMainWindow = 0;
// End Debug

namespace Ogre {

    //---------------------------------------------------------------------
    AnimationTrack::AnimationTrack(Animation* parent) : mParent(parent)
    {
        mTargetNode = 0;
        mMaxKeyFrameTime = -1;
        mSplineBuildNeeded = false;
		mUseShortestRotationPath = true ;
    }
    //---------------------------------------------------------------------
    AnimationTrack::AnimationTrack(Animation* parent, Node* targetNode) 
        : mParent(parent), mTargetNode(targetNode)
    {
        mMaxKeyFrameTime = -1;
        mSplineBuildNeeded = false;
		mUseShortestRotationPath = true ;
    }
    //---------------------------------------------------------------------
    AnimationTrack::~AnimationTrack()
    {
        removeAllKeyFrames();
    }
    //---------------------------------------------------------------------
    unsigned short AnimationTrack::getNumKeyFrames(void) const
    {
        return (unsigned short)mKeyFrames.size();
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::getKeyFrame(unsigned short index) const
    {
		// If you hit this assert, then the keyframe index is out of bounds
        assert( index < (ushort)mKeyFrames.size() );

        return mKeyFrames[index];
    }
    //---------------------------------------------------------------------
    Real AnimationTrack::getKeyFramesAtTime(Real timePos, KeyFrame** keyFrame1, KeyFrame** keyFrame2,
            unsigned short* firstKeyIndex) const
    {
        short firstIndex = -1;
        Real totalAnimationLength = mParent->getLength();

        // Wrap time 
        while (timePos > totalAnimationLength)
        {
            timePos -= totalAnimationLength;
        }

        KeyFrameList::const_iterator i = mKeyFrames.begin();
        // Find last keyframe before or on current time
        while (i != mKeyFrames.end() && (*i)->getTime() <= timePos)
        {
            *keyFrame1 = *i++;
            ++firstIndex;
        }

        // Trap case where there is no key before this time (problem with animation config)
        // In this case use the first key anyway and pretend it's time index 0
        if (firstIndex == -1)
        {
            *keyFrame1 = *i;
            ++firstIndex;
        }

        // Fill index of the first key
        if (firstKeyIndex != NULL)
        {
            *firstKeyIndex = firstIndex;
        }

        // Parametric time
        // t1 = time of previous keyframe
        // t2 = time of next keyframe 
        Real t1, t2;
        // Find first keyframe after the time
        // If no next keyframe, wrap back to first
        if (i == mKeyFrames.end())
        {
            *keyFrame2 = mKeyFrames[0];
            t2 = totalAnimationLength;
        }
        else
        {
            *keyFrame2 = *i;
            t2 = (*keyFrame2)->getTime();
        }

        t1 = (*keyFrame1)->getTime();

        if (t1 == t2)
        {
            // Same KeyFrame (only one)
            return 0.0;
        }
        else
        {
            return (timePos - t1) / (t2 - t1);
        }
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::createKeyFrame(Real timePos)
    {
        KeyFrame* kf = new KeyFrame(this, timePos);

        // Insert at correct location
        if (timePos > mMaxKeyFrameTime || (timePos == 0 && mKeyFrames.empty()))
        {
            // Quick insert at end
            mKeyFrames.push_back(kf);
            mMaxKeyFrameTime = timePos;
        }
        else
        {
            // Search 
            KeyFrameList::iterator i = mKeyFrames.begin();
            while ((*i)->getTime() < timePos && i != mKeyFrames.end())
            {
                ++i;
            }
            mKeyFrames.insert(i, kf);
        }

        _keyFrameDataChanged();

        return kf;

    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeKeyFrame(unsigned short index)
    {
		// If you hit this assert, then the keyframe index is out of bounds
        assert( index < (ushort)mKeyFrames.size() );

        KeyFrameList::iterator i = mKeyFrames.begin();

        i += index;

        delete *i;

        mKeyFrames.erase(i);

        _keyFrameDataChanged();


    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeAllKeyFrames(void)
    {
        KeyFrameList::iterator i = mKeyFrames.begin();

        for (; i != mKeyFrames.end(); ++i)
        {
            delete *i;
        }

        _keyFrameDataChanged();

        mKeyFrames.clear();

    }
    //---------------------------------------------------------------------
    KeyFrame AnimationTrack::getInterpolatedKeyFrame(Real timeIndex) const
    {
        // Return value (note unattached)
        KeyFrame kret(0, timeIndex);
        
        // Keyframe pointers
        KeyFrame *k1, *k2;
        unsigned short firstKeyIndex;

        Real t = this->getKeyFramesAtTime(timeIndex, &k1, &k2, &firstKeyIndex);

        if (t == 0.0)
        {
            // Just use k1
            kret.setRotation(k1->getRotation());
            kret.setTranslate(k1->getTranslate());
            kret.setScale(k1->getScale());
        }
        else
        {
            // Interpolate by t
            Animation::InterpolationMode im = mParent->getInterpolationMode();
            Animation::RotationInterpolationMode rim = 
                mParent->getRotationInterpolationMode();
            Vector3 base;
            switch(im)
            {
            case Animation::IM_LINEAR:
                // Interpolate linearly
                // Rotation
                // Interpolate to nearest rotation if mUseShortestRotationPath set
                if (rim == Animation::RIM_LINEAR)
                {
                    kret.setRotation( Quaternion::nlerp(t, k1->getRotation(), 
                        k2->getRotation(), mUseShortestRotationPath) );
                }
                else //if (rim == Animation::RIM_SPHERICAL)
                {
                    kret.setRotation( Quaternion::Slerp(t, k1->getRotation(), 
					    k2->getRotation(), mUseShortestRotationPath) );
                }

                // Translation
                base = k1->getTranslate();
                kret.setTranslate( base + ((k2->getTranslate() - base) * t) );

                // Scale
                base = k1->getScale();
                kret.setScale( base + ((k2->getScale() - base) * t) );
                break;

            case Animation::IM_SPLINE:
                // Spline interpolation

                // Build splines if required
                if (mSplineBuildNeeded)
                {
                    buildInterpolationSplines();
                }

                // Rotation, take mUseShortestRotationPath into account
                kret.setRotation( mRotationSpline.interpolate(firstKeyIndex, t, 
					mUseShortestRotationPath) );

                // Translation
                kret.setTranslate( mPositionSpline.interpolate(firstKeyIndex, t) );

                // Scale
                kret.setScale( mScaleSpline.interpolate(firstKeyIndex, t) );

                break;
            }

        }
        
        return kret;
        
    }
    //---------------------------------------------------------------------
    void AnimationTrack::apply(Real timePos, Real weight, bool accumulate, 
		Real scale)
    {
        applyToNode(mTargetNode, timePos, weight, accumulate, scale);
        
    }
    //---------------------------------------------------------------------
    Node* AnimationTrack::getAssociatedNode(void) const
    {
        return mTargetNode;
    }
    //---------------------------------------------------------------------
    void AnimationTrack::setAssociatedNode(Node* node)
    {
        mTargetNode = node;
    }
    //---------------------------------------------------------------------
    void AnimationTrack::applyToNode(Node* node, Real timePos, Real weight, 
		bool accumulate, Real scl)
    {
        KeyFrame kf = this->getInterpolatedKeyFrame(timePos);
		if (accumulate) 
        {
            // add to existing. Weights are not relative, but treated as absolute multipliers for the animation
            Vector3 translate = kf.getTranslate() * weight * scl;
			node->translate(translate);

			// interpolate between no-rotation and full rotation, to point 'weight', so 0 = no rotate, 1 = full
            Quaternion rotate;
            Animation::RotationInterpolationMode rim =
                mParent->getRotationInterpolationMode();
            if (rim == Animation::RIM_LINEAR)
            {
                rotate = Quaternion::nlerp(weight, Quaternion::IDENTITY, kf.getRotation());
            }
            else //if (rim == Animation::RIM_SPHERICAL)
            {
                rotate = Quaternion::Slerp(weight, Quaternion::IDENTITY, kf.getRotation());
            }
			node->rotate(rotate);

			Vector3 scale = kf.getScale();
			// Not sure how to modify scale for cumulative anims... leave it alone
			//scale = ((Vector3::UNIT_SCALE - kf.getScale()) * weight) + Vector3::UNIT_SCALE;
			if (scl != 1.0f && scale != Vector3::UNIT_SCALE)
			{
				scale = Vector3::UNIT_SCALE + (scale - Vector3::UNIT_SCALE) * scl;
			}
			node->scale(scale);
		} 
        else 
        {
			// apply using weighted transform method
			Vector3 scale = kf.getScale();
			if (scl != 1.0f && scale != Vector3::UNIT_SCALE)
			{
				scale = Vector3::UNIT_SCALE + (scale - Vector3::UNIT_SCALE) * scl;
			}
			node->_weightedTransform(weight, kf.getTranslate() * scl, kf.getRotation(),
				scale);
		}

        /*
        // DEBUG
        if (!mMainWindow)
        {
            mMainWindow = Root::getSingleton().getRenderWindow("OGRE Render Window");
        }
        String msg = "Time: ";
        msg << timePos;
        mMainWindow->setDebugText(msg);
        */

        //node->rotate(kf.getRotation() * weight);
        //node->translate(kf.getTranslate() * weight);




    }
    //---------------------------------------------------------------------
    void AnimationTrack::buildInterpolationSplines(void) const
    {
        // Don't calc automatically, do it on request at the end
        mPositionSpline.setAutoCalculate(false);
        mRotationSpline.setAutoCalculate(false);
        mScaleSpline.setAutoCalculate(false);

        mPositionSpline.clear();
        mRotationSpline.clear();
        mScaleSpline.clear();

        KeyFrameList::const_iterator i, iend;
        iend = mKeyFrames.end(); // precall to avoid overhead
        for (i = mKeyFrames.begin(); i != iend; ++i)
        {
            mPositionSpline.addPoint((*i)->getTranslate());
            mRotationSpline.addPoint((*i)->getRotation());
            mScaleSpline.addPoint((*i)->getScale());
        }

        mPositionSpline.recalcTangents();
        mRotationSpline.recalcTangents();
        mScaleSpline.recalcTangents();


        mSplineBuildNeeded = false;
    }
	
    //---------------------------------------------------------------------
	void AnimationTrack::setUseShortestRotationPath(bool useShortestPath)
	{
		mUseShortestRotationPath = useShortestPath ;
	}
	
    //---------------------------------------------------------------------
	bool AnimationTrack::getUseShortestRotationPath() const
	{
		return mUseShortestRotationPath ;
	}
    //---------------------------------------------------------------------
    void AnimationTrack::_keyFrameDataChanged(void) const
    {
        mSplineBuildNeeded = true;
    }
    //---------------------------------------------------------------------
	bool AnimationTrack::hasNonZeroKeyFrames(void) const
	{
        KeyFrameList::const_iterator i = mKeyFrames.begin();
        for (; i != mKeyFrames.end(); ++i)
        {
			// look for keyframes which have any component which is non-zero
			// Since exporters can be a little inaccurate sometimes we use a
			// tolerance value rather than looking for nothing
			KeyFrame* kf = *i;
			Vector3 trans = kf->getTranslate();
			Vector3 scale = kf->getScale();
			Vector3 axis;
			Radian angle;
			kf->getRotation().ToAngleAxis(angle, axis);
			Real tolerance = 1e-3f;
			if (!trans.positionEquals(Vector3::ZERO, tolerance) ||
				!scale.positionEquals(Vector3::UNIT_SCALE, tolerance) ||
				!Math::RealEqual(angle.valueRadians(), 0.0f, tolerance))
			{
				return true;
			}
			
		}

		return false;
	}
    //---------------------------------------------------------------------
	void AnimationTrack::optimise(void)
	{
		// Eliminate duplicate keyframes from 2nd to penultimate keyframe 
		// NB only eliminate middle keys from sequences of 5+ identical keyframes
		// since we need to preserve the boundary keys in place, and we need
		// 2 at each end to preserve tangents for spline interpolation
		Vector3 lasttrans;
		Vector3 lastscale;
		Quaternion lastorientation;
        KeyFrameList::iterator i = mKeyFrames.begin();
		Radian quatTolerance(1e-3f);
		std::list<unsigned short> removeList;
		unsigned short k = 0;
		ushort dupKfCount = 0;
        for (; i != mKeyFrames.end(); ++i, ++k)
        {
			KeyFrame* kf = *i;
			Vector3 newtrans = kf->getTranslate();
			Vector3 newscale = kf->getScale();
			Quaternion neworientation = kf->getRotation();
			// Ignore first keyframe; now include the last keyframe as we eliminate
			// only k-2 in a group of 5 to ensure we only eliminate middle keys
			if (i != mKeyFrames.begin() &&
				newtrans.positionEquals(lasttrans) &&
				newscale.positionEquals(lastscale) && 
				neworientation.equals(lastorientation, quatTolerance))
			{
				++dupKfCount;

				// 4 indicates this is the 5th duplicate keyframe
				if (dupKfCount == 4)
				{
					// remove the 'middle' keyframe
					removeList.push_back(k-2);	
					--dupKfCount;
				}
			}
			else
			{
				// reset
				dupKfCount = 0;
				lasttrans = newtrans;
				lastscale = newscale;
				lastorientation = neworientation;
			}
		}

		// Now remove keyframes, in reverse order to avoid index revocation
		std::list<unsigned short>::reverse_iterator r = removeList.rbegin();
		for (; r!= removeList.rend(); ++r)
		{
			removeKeyFrame(*r);
		}
			
			
	}
	
}
