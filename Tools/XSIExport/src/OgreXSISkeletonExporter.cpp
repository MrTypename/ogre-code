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
#include "OgreXSISkeletonExporter.h"
#include "OgreResourceGroupManager.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreSkeletonSerializer.h"
#include "OgreQuaternion.h"
#include <xsi_model.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_math.h>
#include <xsi_rotation.h>
#include <xsi_animationsourceitem.h>
#include <xsi_source.h>
#include <xsi_fcurve.h>
#include <xsi_fcurvekey.h>
#include <xsi_time.h>
#include <xsi_chaineffector.h>
#include <xsi_chainroot.h>
#include <xsi_chainbone.h>

using namespace XSI;

namespace Ogre
{
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::XsiSkeletonExporter()
	{
		mXsiSceneRoot = X3DObject(mXsiApp.GetActiveSceneRoot());
		mXSITrackTypeNames["posx"] = XTT_POS_X;
		mXSITrackTypeNames["posy"] = XTT_POS_Y;
		mXSITrackTypeNames["posz"] = XTT_POS_Z;
		mXSITrackTypeNames["rotx"] = XTT_ROT_X;
		mXSITrackTypeNames["roty"] = XTT_ROT_Y;
		mXSITrackTypeNames["rotz"] = XTT_ROT_Z;
		mXSITrackTypeNames["sclx"] = XTT_SCL_X;
		mXSITrackTypeNames["scly"] = XTT_SCL_Y;
		mXSITrackTypeNames["sclz"] = XTT_SCL_Z;
	}
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::~XsiSkeletonExporter()
	{
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::exportSkeleton(const String& skeletonFileName, 
		DeformerMap& deformers, float framesPerSecond, AnimationList& animList)
	{
		LogOgreAndXSI(L"** Begin OGRE Skeleton Export **");

		SkeletonPtr skeleton = SkeletonManager::getSingleton().create("export",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// construct the hierarchy
		buildBoneHierarchy(skeleton.get(), deformers, animList);

		// create animations 
		createAnimations(skeleton.get(), deformers, framesPerSecond, animList);


		SkeletonSerializer ser;
		ser.exportSkeleton(skeleton.get(), skeletonFileName);

		LogOgreAndXSI(L"** OGRE Skeleton Export Complete **");


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildBoneHierarchy(Skeleton* pSkeleton, 
		DeformerMap& deformers, AnimationList& animList)
	{
		/// Copy all entries from map into a list so iterators won't get invalidated
		std::list<DeformerEntry*> deformerList;
		LogOgreAndXSI(L"-- Bones with vertex assignments:");
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			deformerList.push_back(deformer);
			LogOgreAndXSI(deformer->obj.GetName());
		}

		/* XSI allows you to use any object at all as a bone, not just chain elements.
		   A typical choice is a hierarchy of nulls, for example. In order to 
		   build a skeleton hierarchy which represents the actual one, we need
		   to find the relationships between all the deformers that we found.
		   
		   Well do this by navigating up the scene tree from each bone, looking for
		   a match in the existing bone list or creating a new one where we need it
		   to reach the root. We add bones even if they're not assigned vertices
		   because the animation may depend on them. If the
		   traversal hits the scene root this bone is clearly a root bone 
		   (there may be more than one). 
	   */
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			if (deformer->parentName.empty())
			{
				linkBoneWithParent(deformer, deformers, deformerList);
			}
		}

		// Now eliminate all bones without any animated kine parameters
		// Need to do this after we've determined all relationships
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			validateAsBone(pSkeleton, deformer, deformers, deformerList, animList);
		}

		// Now link
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;

			// link to parent
			if (!deformer->parentName.empty())
			{
				DeformerMap::iterator p = deformers.find(deformer->parentName);
				assert (p != deformers.end() && "Parent not found");
				assert (deformer->pBone && "Child bone not created");
				assert(p->second->pBone && "Parent bone not created");
				DeformerEntry* parent = p->second;
				parent->pBone->addChild(deformer->pBone);

			}
		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::linkBoneWithParent(DeformerEntry* child, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList)
	{
		X3DObject parent(child->obj.GetParent());
		String childName = XSItoOgre(child->obj.GetName());

		if (child->obj == mXsiSceneRoot /* safety check for start node */)
			return;

		// Check for parenting by a chain end effector
		// These are sneaky little buggers - we actually want to attach the
		// child to the end of the final bone in the chain
		if (parent.IsA(XSI::siChainEffectorID))
		{
			ChainEffector effector(parent);
			CRefArray chainBones = effector.GetRoot().GetBones();
			// get the last
			parent = chainBones[chainBones.GetCount()-1];
			
		}
		// is the parent the scene root?
		if (parent == mXsiSceneRoot)
		{
			// we hit the root node
		}
		else
		{

			String parentName = XSItoOgre(parent.GetName());
			// Otherwise, check to see if the parent is in the deformer list
			DeformerMap::iterator i = deformers.find(parentName);
			DeformerEntry* parentDeformer = 0;
			if (i == deformers.end())
			{
				// not found, create entry for parent 
				parentDeformer = new DeformerEntry(deformers.size(), parent);
				deformers[parentName] = parentDeformer;
				deformerList.push_back(parentDeformer);
				LogOgreAndXSI(CString(L"Added ") + parent.GetName() + 
					CString(L" as a parent of ") + child->obj.GetName() );
			}
			else
			{
				parentDeformer = i->second;
			}

			// Link child entry with parent (not bone yet)
			// link child to parent by name
			child->parentName = parentName;
			parentDeformer->childNames.push_back(childName);


		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::validateAsBone(Skeleton* pSkeleton, 
		DeformerEntry* deformer, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList, 
		AnimationList& animList)
	{
		/* The purpose of this method is to find out whether a node in the 
		   bone hierarchy is animated, and if not, to eliminate it and propagate
		   it's static transform contribution to it's children.
		   We do this because it's quite easy in XSI to build deep bone chains
		   with intermediate points that are only used for manipulation. We
		   don't want to include all of those.
	   */

		// TODO


		// if we weren't static, create bone
		if (!deformer->pBone)
		{
			String name = XSItoOgre(deformer->obj.GetName());
			deformer->pBone = pSkeleton->createBone(name, deformer->boneID);
			MATH::CTransformation trans; 

			if (deformer->parentName.empty())
			{
				// set transform on bone to global transform since no parents
				trans = deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// set transform on bone to local transform (since child)
				trans = deformer->obj.GetKinematics().GetLocal().GetTransform();
			}
			deformer->pBone->setPosition(XSItoOgre(trans.GetTranslation()));
			deformer->pBone->setOrientation(XSItoOgre(trans.GetRotation().GetQuaternion()));
			deformer->pBone->setScale(XSItoOgre(trans.GetScaling()));
		}



	}
	//-----------------------------------------------------------------------------
	/*
	void XsiSkeletonExporter::findActionSources(DeformerMap& deformers)
	{
		// We're interested in the ActionSourceItem instances which have a
		// target string which begins with a deformer name. XSI has ActionSources,
		// which have ActionSourceItems, which have target strings of the form
		// '[objectname].kine.local.[pos|rot|scl][x|y|z]' - so each ASI points
		// at a 'track' which is one element of the transform. 
		// We can ignore ASIs with a target which begins with anything other than 
		// the name of a valid deformer, but once we find one we'll hook up the 
		// ASI into the DeformerEntry so that, by the time we're finished, we should
		// have the full complement of transform tracks. 
		// Note also that XSI stores action sources against the scene model, and
		// potentially child models. It can also sometimes store them directly 
		// against the object itself, although this seems more unusual. So we look
		// in all places.
		// 

		// Look in model and all children
		Model root = mXsiApp.GetActiveSceneRoot();
		findActionSources(root, deformers);

		// Find all children (recursively)
		CRefArray children = root.FindChildren(L"", siModelType, CStringArray());
		for (int c = 0; c < children.GetCount(); ++c)
		{
			Model child(children[c]);
			findActionSources(child, deformers);
		}

		// look against deformers themselves; look for all parameter sources
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			CRefArray paramList = di->second->obj.GetParameters();
			for (int p = 0; p < paramList.GetCount(); ++p)
			{
				Parameter param(paramList[p]);
				Source src(param.GetSource());
				if (src.IsValid() && src.IsA(siActionSourceID))
				{
					ActionSource actSource(src);
					processActionSource(actSource, deformers); 
					
				}
			}
		}

		
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::findActionSources(const XSI::Model& obj, 
		DeformerMap& deformers)
	{			if (

		CRefArray sources = obj.GetSources();
		for (int s = 0; s < sources.GetCount(); ++s)
		{
			XSI::Source src(sources[s]);
			if (src.IsA(siActionSourceID))
			{
				ActionSource actSource(src);
				processActionSource(actSource, deformers);

			}
			
		}
		
	}
	*/
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::processActionSource(const XSI::ActionSource& actSource,
		DeformerMap& deformers)
	{
		// Clear existing deformer links
		for(DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			for (int tt = XTT_POS_X; tt < XTT_COUNT; ++tt)
			{
				di->second->xsiTrack[tt].ResetObject();
			}
		}
		// Get all the items
		CRefArray items = actSource.GetItems();
		for (int i = 0; i < items.GetCount(); ++i)
		{
			XSI::AnimationSourceItem item = items[i];

			// Check the target
			String target = XSItoOgre(item.GetTarget());
			size_t firstDotPos = target.find_first_of(".");
			size_t lastDotPos = target.find_last_of(".");
			if (firstDotPos != String::npos && lastDotPos != String::npos)
			{
				String targetName = target.substr(0, firstDotPos);
				String paramName = target.substr(lastDotPos+1, 
					target.size() - lastDotPos - 1);
				// locate deformer
				DeformerMap::iterator di = deformers.find(targetName);
				if (di != deformers.end())
				{
					DeformerEntry* deformer = di->second;
					// determine parameter
					std::map<String, int>::iterator pi = mXSITrackTypeNames.find(paramName);
					if (pi != mXSITrackTypeNames.end())
					{
						deformer->xsiTrack[pi->second] = item;
					}
				}
			}

		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimations(Skeleton* pSkel, 
		DeformerMap& deformers, float fps, AnimationList& animList)
	{
		for (AnimationList::iterator ai = animList.begin(); ai != animList.end(); ++ai)
		{
			AnimationEntry& animEntry = *ai;
			// tease out all the animation source items
			processActionSource(animEntry.source, deformers);

			// Get the keyframe numbers from all XSI tracks
			// XSI tracks might be sparse
			buildKeyframeList(deformers, animEntry);

			float animLength = (float)(animEntry.endFrame - animEntry.startFrame) / fps;
			Animation* anim = pSkel->createAnimation(animEntry.animationName, animLength);

			createAnimationTracks(anim, animEntry, deformers, fps);
			
		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildKeyframeList(DeformerMap& deformers, 
		AnimationEntry& animEntry)
	{
		bool first = true;
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;
			for (int tt = XTT_POS_X; tt < XTT_COUNT; ++tt)
			{
				AnimationSourceItem item = deformer->xsiTrack[tt];
				// skip invalid or non-FCurve items
				if (!item.IsValid() || !item.GetSource().IsA(XSI::siFCurveID))
					continue;
				
				FCurve fcurve = item.GetSource();
				CRefArray keys = fcurve.GetKeys();
				for (int k = 0; k < keys.GetCount(); ++k)
				{
					long currFrame = fcurve.GetKeyTime(k).GetTime();
					if (first)
					{
						animEntry.startFrame = currFrame;
						animEntry.endFrame = currFrame;
						first = false;
					}
					else 
					{
						if (currFrame < animEntry.startFrame)
						{
							animEntry.startFrame = currFrame;
						}
						if (currFrame > animEntry.endFrame)
						{
							animEntry.endFrame = currFrame;
						}
					}
					
					animEntry.frames.insert(currFrame);
				}
			}
		}
	


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimationTracks(Animation* pAnim, 
		AnimationEntry& animEntry, DeformerMap& deformers, float fps)
	{
		/* We have to iterate over the list of deformers, and create a track
		 * for each one. Since XSI stores keys for all 9 components separately,
		 * we need to bake OGRE keyframes (which include position, rotation and 
		 * translation) by interpolation. We can merge the list of frames from all
		 * action source item fcurves, then use the Eval method to get XSI to sample
		 * the curve properly for us.
		 * We will also use this to ensure there is a keyframe at the start and end of the
		 * animation.
		 */
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;

			// create track
			AnimationTrack* track = pAnim->createTrack(deformer->boneID, deformer->pBone);

			// Iterate over the frames and pull out the values we need
			// bake keyframe for all
			for (std::set<long>::iterator fi = animEntry.frames.begin(); 
				fi != animEntry.frames.end(); ++fi)
			{
				Vector3 pos, rot, scl;
				pos.x = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_X], *fi);
				pos.y = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_Y], *fi);
				pos.z = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_Z], *fi);
				rot.x = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_X], *fi);
				rot.y = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_Y], *fi);
				rot.z = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_Z], *fi);
				scl.x = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_X], *fi);
				scl.y = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_Y], *fi);
				scl.z = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_Z], *fi);

				// HACK - XSI seems to be reporting 0 all the time for scale??
				// TODO
				scl = Vector3::UNIT_SCALE;

				// Build combined rotation (assume rotation ordering)
				Quaternion rotQX(Degree(rot.x), Vector3::UNIT_X);
				Quaternion rotQY(Degree(rot.y), Vector3::UNIT_Y);
				Quaternion rotQZ(Degree(rot.z), Vector3::UNIT_Z);
				Quaternion combinedRot = rotQX * rotQY * rotQZ;

				// make relative to bindpos
				// Only seems necessary for non-root bones?
				if (deformer->pBone->getParent() != 0)
				{
					Vector3 invScale = deformer->pBone->getScale();
					invScale.x = 1.0f / invScale.x;
					invScale.y = 1.0f / invScale.y;
					invScale.z = 1.0f / invScale.z;
					Quaternion invRot = deformer->pBone->getOrientation().Inverse();

					// Inverse SRT on position
					pos = pos - deformer->pBone->getPosition();
					pos = invRot * pos;
					pos = pos * invScale;

					// Inverse rotation
					combinedRot = invRot * combinedRot;

					// Inverse scale
					scl = invScale * scl;
				}

				// create keyframe
				KeyFrame* kf = track->createKeyFrame((float)(*fi - 1) / fps);
				kf->setTranslate(pos);
				kf->setRotation(combinedRot);
				kf->setScale(scl);


			}
		}


	}
	//-----------------------------------------------------------------------------
	float XsiSkeletonExporter::deriveKeyFrameValue(
		XSI::AnimationSourceItem item, long frame)
	{
		FCurve curve(item.GetSource());
		// let fcurve evaluate
		return static_cast<float>(curve.Eval(CTime(frame)));
	}
}
