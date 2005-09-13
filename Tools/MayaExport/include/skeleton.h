#ifndef _SKELETON_H
#define _SKELETON_H

#include "mayaExportLayer.h"
#include "paramList.h"

namespace OgreMayaExporter
{
	/***** structure to hold joint info *****/
	typedef struct jointTag
	{
		MString name;
		int id;
		MMatrix localMatrix;
		MMatrix worldMatrix;
		int parentIndex;
		double posx,posy,posz;
		double angle;
		double axisx,axisy,axisz;
	} joint;

	/***** structure to hold keyframes *****/
	typedef struct keyframeTag
	{
		double time;							//time of keyframe
		double tx,ty,tz;						//translation
		double angle,axis_x,axis_y,axis_z;		//rotation
		double sx,sy,sz;						//scale
	} keyframe;

	/***** structure to hold an animation track *****/
	typedef struct trackTag
	{
		MString bone;
		std::vector<keyframe> keyframes;
	} track;

	/***** structure to hold an animation *****/
	typedef struct animationTag
	{
		MString name;
		double length;
		std::vector<track> tracks;
	} animation;

	/*********** Class Skeleton **********************/
	class Skeleton
	{
	public:
		//constructor
		Skeleton();
		//destructor
		~Skeleton();
		//clear skeleton data
		void clear();
		//load skeleton data
		MStatus load(MFnSkinCluster* pSkinCluster,ParamList& params);
		//load skeletal animations
		MStatus loadAnims(MDagPath& jointDag,int jointId,ParamList& params);
		//load a clip
		MStatus loadClip(MDagPath& jointDag,int jointId,MString clipName,double start,
			double stop,double rate,ParamList& params);
		//get joints
		std::vector<joint>& getJoints();
		//get animations
		std::vector<animation>& getAnimations();
		//write skeleton data to Ogre XML
		MStatus writeXML(ParamList &params);

	protected:
		MStatus loadJoint(MDagPath& jointDag, joint* parent, ParamList& params);

		MFnSkinCluster* m_pSkinCluster;
		std::vector<joint> m_joints;
		std::vector<animation> m_animations;
	};

	std::vector<double> mergesorted(const std::vector<double>& v1, const std::vector<double>& v2);

}	//end namespace

#endif
