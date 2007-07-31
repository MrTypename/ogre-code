/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __MATERIALSCRIPTCOMPILER2_H_
#define __MATERIALSCRIPTCOMPILER2_H_

#include "OgreScriptCompiler.h"
#include "OgreTextureUnitState.h"

namespace Ogre{

	// Forward declaration for the compiler
	class MaterialScriptCompiler2;

	/// This is the listener for the new compiler, which allows for custom behavior and information callbacks during compilation.
	class _OgreExport MaterialScriptCompilerListener : public ScriptCompilerListener
	{
	public:
		MaterialScriptCompilerListener();
		/// Override this to do custom processing of the script nodes
		virtual bool processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, MaterialScriptCompiler2*);
		/// Overriding this method allows for custom material allocation
		virtual Material *getMaterial(const String &name, const String &group);
		/// Overriding this method allows for custom asm GPU program allocation
		virtual GpuProgram *getGpuProgram(const String &name, const String &group, GpuProgramType type, const String &syntax);
		/// Overriding this method allows for custom high-level GPU program allocation
		virtual HighLevelGpuProgram *getHighLevelGpuProgram(const String &name, const String &group, GpuProgramType type, const String &language);
		/// This is called just before texture aliases found in a script are applied to a material
		virtual void preApplyTextureAliases(Ogre::AliasTextureNamePairList &aliases);
		/// This accepts the name of a texture to be referenced, and is expected to ensure it loads and return the real name for it
		virtual String getTexture(const String &name);
	};
	
	/** This is the new compiler for material scripts. It uses the parser to parse the material
		and this class processes the token stream that is produced and compiles it into Ogre materials.
	*/
	class _OgreExport MaterialScriptCompiler2 : public ScriptCompiler
	{
	public:
		MaterialScriptCompiler2();

		/// Sets the listener to override behavior
		void setListener(MaterialScriptCompilerListener *listener);
	protected:
		/// This begins the compilation of the particle system from the final transformed AST
		bool compileImpl(ScriptNodeListPtr nodes);
		/// Delegates to the listener if it can, otherwise returns false. If it returns true, then some input was consumed.
		bool processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		/// This is the override for loading imports
		ScriptNodeListPtr loadImportPath(const String &name);
	private: // Private handlers to compile pieces of the material script
		void compileMaterial(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileLodDistances(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileReceiveShadows(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileTransparencyCastsShadows(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileSetTextureAlias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);

		void compileTechnique(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileScheme(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique);
		void compileLodIndex(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique);

		void compilePass(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique);
		void compileAmbient(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileDiffuse(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileSpecular(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileEmissive(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileSceneBlend(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileSeparateSceneBlend(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileDepthCheck(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileDepthWrite(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileDepthFunc(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileDepthBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileIterationDepthBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileAlphaRejection(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileLightScissor(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileLightClipPlanes(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileIlluminationStage(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileCullHardware(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileCullSoftware(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileNormaliseNormals(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileLighting(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileShading(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePolygonMode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePolygonModeOverrideable(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileFogOverride(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileColourWrite(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileMaxLights(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileStartLight(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileIteration(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePointSize(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePointSprites(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePointSizeAttenuation(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePointSizeMin(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compilePointSizeMax(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);

		void compileTextureUnit(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass);
		void compileTextureAlias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileAnimTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileCubicTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileTexCoordSet(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileTexAddressMode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileTexBorderColour(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileFiltering(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileMaxAnisotropy(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileMipmapBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileColourOp(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileColourOpEx(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileColourOpMultipassFallback(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileAlphaOpEx(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileEnvMap(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileScroll(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileScrollAnim(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileRotate(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileRotateAnim(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileScale(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileWaveXForm(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileTransform(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileBindingType(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		void compileContentType(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState);
		
		bool parseColour(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c);
		bool parseBlendFactor(const String &str, SceneBlendFactor &factor);
		bool parseCompareFunction(const String &str, CompareFunction &func);
		bool parseBlendOp(const String &str, LayerBlendOperationEx &op);
		bool parseBlendSource(const String &str, LayerBlendSource &source);
		bool parseXFormType(const String &str, TextureUnitState::TextureTransformType &type);
		bool parseWaveType(const String &str, WaveformType &wave);
	private:
		// The listener
		MaterialScriptCompilerListener *mListener;
		// This is the material being compiled
		Material *mMaterial;
		// Stores the texture aliases applied to a compiling material
		Ogre::AliasTextureNamePairList mTextureAliases;
		// This is the GpuProgram that is being set up
		GpuProgram *mProgram;
		// This is the GpuProgramParameters being built for the program
		GpuProgramParametersSharedPtr mProgramParameters;
	};

}

#endif
