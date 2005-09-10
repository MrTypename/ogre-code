/**
*******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************
*/
#ifndef H_WJ_MaterialGenerator
#define H_WJ_MaterialGenerator

#include "OgreMaterial.h"
#include "OgreStringVector.h"

/** Caching, on-the-fly material generator. This is a class that automatically 
	generates and stores different permutations of a material, and its shaders. 
	It can be used if you have a material that has lots of slightly different 
	variations, like whether to use a specular light, skinning, normal mapping 
	and other options. Writing all these out is a tedioius job. Of course it is 
	possible to always use the material with all features, but that might result 
	in large, slow shader programs. This class provides an efficient solution
	to that.
 */
class MaterialGenerator
{
public:
	/** Bitfield used to signify a material permutations */
	typedef Ogre::uint32 Perm;
	~MaterialGenerator();

	const Ogre::MaterialPtr &getMaterial(Perm permutation);

	/** Implementation class that takes care of actual generation or lookup
		of the various constituent parts (template material, fragment shader
		and vertex shader). These methods are only called once for every permutation,
		after which the result is stored and re-used.
	*/
	class Impl
	{
	public:
		virtual ~Impl();
		virtual Ogre::GpuProgramPtr generateVertexShader(Perm permutation) = 0;
		virtual Ogre::GpuProgramPtr generateFragmentShader(Perm permutation) = 0;
		virtual Ogre::MaterialPtr generateTemplateMaterial(Perm permutation) = 0;
	};
protected:
	/** The constructor is protected as this base class should never be constructed
		as-is. It is meant to be subclassed so that values can be assigned to
		the various fields controlling material generator, and most importantly, the
		mImpl field.
	*/
	MaterialGenerator();

	const Ogre::GpuProgramPtr &getVertexShader(Perm permutation);
	const Ogre::GpuProgramPtr &getFragmentShader(Perm permutation);
	const Ogre::MaterialPtr &getTemplateMaterial(Perm permutation);
	
	/// Base name of materials generated by this
	Ogre::String materialBaseName;
	/// Name of every bit
	Ogre::StringVector bitNames;
	/// Mask of permutation bits that influence vertex shader choice
	Perm vsMask;
	/// Mask of permutation bits that influence fragment shader choice
	Perm fsMask;
	/// Mask of permutation bits that influence template material choice
	Perm matMask;
	/// Generator
	Impl *mImpl;

	typedef std::map<Perm, Ogre::GpuProgramPtr> ProgramMap;
	typedef std::map<Perm, Ogre::MaterialPtr> MaterialMap;

	ProgramMap mVs, mFs;
	MaterialMap mTemplateMat, mMaterials;
};

#endif
