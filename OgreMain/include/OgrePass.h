/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
#ifndef __Pass_H__
#define __Pass_H__

#include "OgrePrerequisites.h"
#include "OgreGpuProgram.h"
#include "OgreColourValue.h"
#include "OgreBlendMode.h"
#include "OgreCommon.h"

namespace Ogre {
    /** Class defining a single pass of a Technique (of a Material), ie
        a single rendering call. 
    @remarks
        Rendering can be repeated with many passes for more complex effects.
        Each pass is either a fixed-function pass (meaning it does not use
        a vertex or fragment program) or a programmable pass (meaning it does
        use a vertex and fragment program). Note that because of the nature of
        the custom inputs and outputs of the programmable pipeline, you must use
        both a vertex and fragment program, not just one or the other, if you
        decide to use a programmable pass.
    @par
        Programmable passes are complex to define, because they require custom
        programs and you have to set all constant inputs to the programs (like
        the position of lights, any base material colours you wish to use etc), but
        they do give you much total flexibility over the algorithms used to render your
        pass, and you can create some effects which are impossible with a fixed-function pass.
        On the other hand, you can define a fixed-function pass in very little time, and
        you can use a range of fixed-function effects like environment mapping very
        easily, plus your pass will be more likely to be compatible with older hardware.
        There are pros and cons to both, just remember that if you use a programmable
        pass to create some great effects, allow more time for definition and testing.
    */
    class _OgreExport Pass
    {
    protected:
        Technique* mParent;
        unsigned short mIndex; // pass index
        unsigned long mHash; // pass hash
        /// Is this a programmable pass?
        bool mIsProgrammable;
        //-------------------------------------------------------------------------
        // Colour properties, only applicable in fixed-function passes
        ColourValue mAmbient;
        ColourValue mDiffuse;
        ColourValue mSpecular;    
        ColourValue mEmissive;
        Real mShininess;
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // Blending factors
        SceneBlendFactor mSourceBlendFactor;    
        SceneBlendFactor mDestBlendFactor;
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------    
        // Depth buffer settings
        bool mDepthCheck;
        bool mDepthWrite;
        CompareFunction mDepthFunc;
        ushort mDepthBias;

        // Colour buffer settings
        bool mColourWrite;

        //-------------------------------------------------------------------------    

        //-------------------------------------------------------------------------
        // Culling mode
        CullingMode mCullMode;
        ManualCullingMode mManualCullMode;
        //-------------------------------------------------------------------------

        /// Lighting enabled?
        bool mLightingEnabled;

        /// Shading options
        ShadeOptions mShadeOptions;

        /// Texture filtering
        TextureFilterOptions mTextureFiltering;
        /// texture anisotropy level
        int mMaxAniso;
        /// is filtering default
        bool mIsDefFiltering;
        /// is anisotropy default
        bool mIsDefAniso;

        //-------------------------------------------------------------------------    
        // Fog
        bool mFogOverride;
        FogMode mFogMode;
        ColourValue mFogColour;
        Real mFogStart;
        Real mFogEnd;
        Real mFogDensity;
        //-------------------------------------------------------------------------    

        /// Storage of texture unit states 
        typedef std::vector<TextureUnitState*> TextureUnitStates;
        TextureUnitStates mTextureUnitStates;    

		// Vertex program details
		GpuProgramUsage *mVertexProgramUsage;
		// Fragment program details
		GpuProgramUsage *mFragmentProgramUsage;
    public:
        /// Default constructor
		Pass(Technique* parent, unsigned short index, bool programmable = false);
        /// Copy constructor
        Pass(Technique* parent, unsigned short index, const Pass& oth );
        /// Operator = overload
        Pass& operator=(const Pass& oth);
        ~Pass();

        /// Returns true if this pass is programmable ie supports vertex & fragment programs.
        bool isProgrammable(void) { return mIsProgrammable; }
        /// Sets whether this Pass is programmable or not
        void setProgrammable(bool programmable) {mIsProgrammable = programmable;}
        /// Gets the index of this Pass in the parent Technique
        unsigned short getIndex(void) const { return mIndex; }
        /** Sets the ambient colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much ambient light (directionless global light) is reflected. The default is full white, meaning
        objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light
        effects, or change the blend of colours to make the object have a base colour other than white.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setAmbient(Real red, Real green, Real blue);

        /** Sets the ambient colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much ambient light (directionless global light) is reflected. The default is full white, meaning
        objects are completely globally illuminated. Reduce this if you want to see diffuse or specular light
        effects, or change the blend of colours to make the object have a base colour other than white.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */

        void setAmbient(const ColourValue& ambient);

        /** Sets the diffuse colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much diffuse light (light from instances of the Light class in the scene) is reflected. The default
        is full white, meaning objects reflect the maximum white light they can from Light objects.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setDiffuse(Real red, Real green, Real blue);

        /** Sets the diffuse colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much diffuse light (light from instances of the Light class in the scene) is reflected. The default
        is full white, meaning objects reflect the maximum white light they can from Light objects.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setDiffuse(const ColourValue& diffuse);

        /** Sets the specular colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much specular light (highlights from instances of the Light class in the scene) is reflected.
        The default is to reflect no specular light.
        @note
        The size of the specular highlights is determined by the separate 'shininess' property.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setSpecular(Real red, Real green, Real blue);

        /** Sets the specular colour reflectance properties of this pass.
        @remarks
        The base colour of a pass is determined by how much red, green and blue light is reflects
        (provided texture layer #0 has a blend mode other than LBO_REPLACE). This property determines how
        much specular light (highlights from instances of the Light class in the scene) is reflected.
        The default is to reflect no specular light.
        @note
        The size of the specular highlights is determined by the separate 'shininess' property.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setSpecular(const ColourValue& specular);

        /** Sets the shininess of the pass, affecting the size of specular highlights.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setShininess(Real val);

        /** Sets the amount of self-illumination an object has.
        @remarks
        If an object is self-illuminating, it does not need external sources to light it, ambient or
        otherwise. It's like the object has it's own personal ambient light. This property is rarely useful since
        you can already specify per-pass ambient light, but is here for completeness.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setSelfIllumination(Real red, Real green, Real blue);

        /** Sets the amount of self-illumination an object has.
        @remarks
        If an object is self-illuminating, it does not need external sources to light it, ambient or
        otherwise. It's like the object has it's own personal ambient light. This property is rarely useful since
        you can already specify per-pass ambient light, but is here for completeness.
        @note
        This setting has no effect if dynamic lighting is disabled (see Pass::setLightingEnabled),
        or if this is a programmable pass.
        */
        void setSelfIllumination(const ColourValue& selfIllum);

        /** Gets the ambient colour reflectance of the pass.
        */
        const ColourValue& getAmbient(void) const;

        /** Gets the diffuse colour reflectance of the pass.
        */
        const ColourValue& getDiffuse(void) const;

        /** Gets the specular colour reflectance of the pass.
        */
        const ColourValue& getSpecular(void) const;

        /** Gets the self illumination colour of the pass.
        */
        const ColourValue& getSelfIllumination(void) const;

        /** Gets the 'shininess' property of the pass (affects specular highlights).
        */
        Real getShininess(void) const;

        /** Inserts a new TextureUnitState object into the Pass.
        @remarks
        This unit is is added on top of all previous units. 
        @param
        name The basic name of the texture e.g. brickwall.jpg, stonefloor.png
        @param
        texCoordSet The index of the texture coordinate set to use.
        @note 
        Applies to both fixed-function and programmable passes.
        */
        TextureUnitState* createTextureUnitState( const String& textureName, unsigned short texCoordSet = 0);
		/** Adds the passed in TextureUnitState, to the existing Pass. */
		void addTextureUnitState(TextureUnitState* state);
        /** Retrieves a pointer to a texture unit state so it may be modified.
        */
        TextureUnitState* getTextureUnitState(unsigned short index);

        typedef VectorIterator<TextureUnitStates> TextureUnitStateIterator;
        /** Get an iterator over the TextureUnitStates contained in this Pass. */
        TextureUnitStateIterator getTextureUnitStateIterator(void);

        /** Removes the indexed texture unit state from this pass.
        @remarks
            Note that removing a texture which is not the topmost will have a larger performance impact.
        */
        void removeTextureUnitState(unsigned short index);

        /** Removes all texture unit settings.
        */
        void removeAllTextureUnitStates(void);

        /** Returns the number of texture unit settings.
        */
        size_t getNumTextureUnitStates(void) const
        {
            return mTextureUnitStates.size();
        }

        /** Sets the kind of blending this pass has with the existing contents of the scene.
        @remarks
        Wheras the texture blending operations seen in the TextureUnitState class are concerned with
        blending between texture layers, this blending is about combining the output of the Pass
        as a whole with the existing contents of the rendering target. This blending therefore allows
        object transparency and other special effects. If all passes in a technique have a scene
        blend, then the whole technique is considered to be transparent.
        @par
        This method allows you to select one of a number of predefined blending types. If you require more
        control than this, use the alternative version of this method which allows you to specify source and
        destination blend factors.
        @note
        This method is applicable for both the fixed-function and programmable pipelines.
        @param
        sbt One of the predefined SceneBlendType blending types
        */
        void setSceneBlending( const SceneBlendType sbt );

        /** Allows very fine control of blending this Pass with the existing contents of the scene.
        @remarks
        Wheras the texture blending operations seen in the TextureUnitState class are concerned with
        blending between texture layers, this blending is about combining the output of the material
        as a whole with the existing contents of the rendering target. This blending therefore allows
        object transparency and other special effects.
        @par
        This version of the method allows complete control over the blending operation, by specifying the
        source and destination blending factors. The result of the blending operation is:
        <span align="center">
        final = (texture * sourceFactor) + (pixel * destFactor)
        </span>
        @par
        Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
        enumerated type.
        @param
        sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
        @param
        destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
        @note
        This method is applicable for both the fixed-function and programmable pipelines.
        */
        void setSceneBlending( const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor);

        /** Retrieves the source blending factor for the material (as set using Materiall::setSceneBlending).
        */
        SceneBlendFactor getSourceBlendFactor() const;

        /** Retrieves the destination blending factor for the material (as set using Materiall::setSceneBlending).
        */
        SceneBlendFactor getDestBlendFactor() const;

		/** Returns true if this pass has some element of transparency. */
		bool isTransparent(void) const;

		/** Sets whether or not this pass renders with depth-buffer checking on or not.
        @remarks
        If depth-buffer checking is on, whenever a pixel is about to be written to the frame buffer
        the depth buffer is checked to see if the pixel is in front of all other pixels written at that
        point. If not, the pixel is not written.
        @par
        If depth checking is off, pixels are written no matter what has been rendered before.
        Also see setDepthFunction for more advanced depth check configuration.
        @see
        setDepthFunction
        */
        void setDepthCheckEnabled(bool enabled);

        /** Returns whether or not this pass renders with depth-buffer checking on or not.
        @see
        setDepthCheckEnabled
        */
        bool getDepthCheckEnabled(void) const;

        /** Sets whether or not this pass renders with depth-buffer writing on or not.
        @remarks
        If depth-buffer writing is on, whenever a pixel is written to the frame buffer
        the depth buffer is updated with the depth value of that new pixel, thus affecting future
        rendering operations if future pixels are behind this one.
        @par
        If depth writing is off, pixels are written without updating the depth buffer Depth writing should
        normally be on but can be turned off when rendering static backgrounds or when rendering a collection
        of transparent objects at the end of a scene so that they overlap each other correctly.
        */
        void setDepthWriteEnabled(bool enabled);

        /** Returns whether or not this pass renders with depth-buffer writing on or not.
        @see
        setDepthWriteEnabled
        */
        bool getDepthWriteEnabled(void) const;

        /** Sets the function used to compare depth values when depth checking is on.
        @remarks
        If depth checking is enabled (see setDepthCheckEnabled) a comparison occurs between the depth
        value of the pixel to be written and the current contents of the buffer. This comparison is
        normally CMPF_LESS_EQUAL, i.e. the pixel is written if it is closer (or at the same distance)
        than the current contents. If you wish you can change this comparison using this method.
        */
        void setDepthFunction( CompareFunction func );
        /** Returns the function used to compare depth values when depth checking is on.
        @see
        setDepthFunction
        */
        CompareFunction getDepthFunction(void) const;

		/** Sets whether or not colour buffer writing is enabled for this Pass.
		@remarks
			For some effects, you might wish to turn off the colour write operation
			when rendering geometry; this means that only the depth buffer will be
			updated (provided you have depth buffer writing enabled, which you 
			probably will do, although you may wish to only update the stencil
			buffer for example - stencil buffer state is managed at the RenderSystem
			level only, not the Material since you are likely to want to manage it 
			at a higher level).
		*/
		void setColourWriteEnabled(bool enabled);
		/** Determines if colour buffer writing is enabled for this pass. */
		bool getColourWriteEnabled(void);

        /** Sets the culling mode for this pass  based on the 'vertex winding'.
        @remarks
        A typical way for the rendering engine to cull triangles is based on the 'vertex winding' of
        triangles. Vertex winding refers to the direction in which the vertices are passed or indexed
        to in the rendering operation as viewed from the camera, and will wither be clockwise or
        anticlockwise (that's 'counterclockwise' for you Americans out there ;) The default is
        CULL_CLOCKWISE i.e. that only triangles whose vertices are passed/indexed in anticlockwise order
        are rendered - this is a common approach and is used in 3D studio models for example. You can
        alter this culling mode if you wish but it is not advised unless you know what you are doing.
        @par
        You may wish to use the CULL_NONE option for mesh data that you cull yourself where the vertex
        winding is uncertain.
        */
        void setCullingMode( CullingMode mode );

        /** Returns the culling mode for geometry rendered with this pass. See setCullingMode for more information.
        */
        CullingMode getCullingMode(void) const;

        /** Sets the manual culling mode, performed by CPU rather than hardware.
        @pemarks
        In some situations you want to use manual culling of triangles rather than sending the
        triangles to the hardware and letting it cull them. This setting only takes effect on SceneManager's
        that use it (since it is best used on large groups of planar world geometry rather than on movable
        geometry since this would be expensive), but if used can cull geometry before it is sent to the
        hardware.
        @note
        The default for this setting is MANUAL_CULL_BACK.
        @param
        mode The mode to use - see enum ManualCullingMode for details

        */
        void setManualCullingMode( ManualCullingMode mode );

        /** Retrieves the manual culling mode for this pass
        @see
        setManualCullingMode
        */
        ManualCullingMode getManualCullingMode(void) const;

        /** Sets whether or not dynamic lighting is enabled.
        @param
        enabled
        If true, dynamic lighting is performed on geometry with normals supplied, geometry without
        normals will not be displayed.
        @par
        If false, no lighting is applied and all geometry will be full brightness.
        */
        void setLightingEnabled(bool enabled);

        /** Returns whether or not dynamic lighting is enabled.
        */
        bool getLightingEnabled(void) const;

        /** Sets the type of light shading required
        @note
        The default shading method is Gouraud shading.
        */
        void setShadingMode( ShadeOptions mode );

        /** Returns the type of light shading to be used.
        */
        ShadeOptions getShadingMode(void) const;


        /** Sets the fogging mode applied to this pass.
        @remarks
        Fogging is an effect that is applied as polys are rendered. Sometimes, you want
        fog to be applied to an entire scene. Other times, you want it to be applied to a few
        polygons only. This pass-level specification of fog parameters lets you easily manage
        both.
        @par
        The SceneManager class also has a setFog method which applies scene-level fog. This method
        lets you change the fog behaviour for this pass compared to the standard scene-level fog.
        @param
        overrideScene If true, you authorise this pass to override the scene's fog params with it's own settings.
        If you specify false, so other parameters are necessary, and this is the default behaviour for passs.
        @param
        mode Only applicable if overrideScene is true. You can disable fog which is turned on for the
        rest of the scene by specifying FOG_NONE. Otherwise, set a pass-specific fog mode as
        defined in the enum FogMode.
        @param
        colour The colour of the fog. Either set this to the same as your viewport background colour,
        or to blend in with a skydome or skybox.
        @param
        expDensity The density of the fog in FOG_EXP or FOG_EXP2 mode, as a value between 0 and 1. 
        The default is 0.001.
        @param
        linearStart Distance in world units at which linear fog starts to encroach. 
        Only applicable if mode is FOG_LINEAR.
        @param
        linearEnd Distance in world units at which linear fog becomes completely opaque.
        Only applicable if mode is FOG_LINEAR.
        */
        void setFog(
            bool overrideScene,
            FogMode mode = FOG_NONE,
            const ColourValue& colour = ColourValue::White,
            Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0 );

        /** Returns true if this pass is to override the scene fog settings.
        */
        bool getFogOverride(void) const;

        /** Returns the fog mode for this pass.
        @note
        Only valid if getFogOverride is true.
        */
        FogMode getFogMode(void) const;

        /** Returns the fog colour for the scene.
        */
        const ColourValue& getFogColour(void) const;

        /** Returns the fog start distance for this pass.
        @note
        Only valid if getFogOverride is true.
        */
        Real getFogStart(void) const;

        /** Returns the fog end distance for this pass.
        @note
        Only valid if getFogOverride is true.
        */
        Real getFogEnd(void) const;

        /** Returns the fog density for this pass.
        @note
        Only valid if getFogOverride is true.
        */
        Real getFogDensity(void) const;

        /** Sets the depth bias to be used for this material.
        @remarks
        When polygons are coplanar, you can get problems with 'depth fighting' where
        the pixels from the two polys compete for the same screen pixel. This is particularly
        a problem for decals (polys attached to another surface to represent details such as
        bulletholes etc.).
        @par
        A way to combat this problem is to use a depth bias to adjust the depth buffer value
        used for the decal such that it is slightly higher than the true value, ensuring that
        the decal appears on top.
        @param bias The bias value, should be between 0 and 16.
        */
        void setDepthBias(ushort bias);

        /** Retrieves the depth bias value as set by setDepthValue. */
        ushort getDepthBias(void) const;

        /// Gets the parent Technique
        Technique* getParent(void) { return mParent; }

		/** Sets the details of the vertex program to use.
		@remarks
			Only applicable to programmable passes, this sets the details of
			the vertex program to use in this pass. The program will not be
			loaded until the parent Material is loaded.
		@param name The name of the program - this must have been 
			created using GpuProgramManager by the time that this Pass 
			is loaded.
		*/
		void setVertexProgram(const String& name);
		/** Sets the vertex program parameters.
		@remarks
			Only applicable to programmable passes, and this particular call is
			designed for low-level programs; use the named parameter methods
			for setting high-level program parameters.
		*/
		void setVertexProgramParameters(GpuProgramParametersSharedPtr params);
		/** Gets the name of the vertex program used by this pass. */
		const String& getVertexProgramName(void);
		/** Gets the vertex program parameters used by this pass. */
		GpuProgramParametersSharedPtr getVertexProgramParameters(void);
		/** Gets the vertex program used by this pass, only available after _load(). */
		GpuProgram* getVertexProgram(void);
		/** Sets the details of the fragment program to use.
		@remarks
			Only applicable to programmable passes, this sets the details of
			the fragment program to use in this pass. The program will not be
			loaded until the parent Material is loaded.
		@param name The name of the program - this must have been 
			created using GpuProgramManager by the time that this Pass 
			is loaded.
		*/
		void setFragmentProgram(const String& name);
		/** Sets the vertex program parameters.
		@remarks
			Only applicable to programmable passes.
		*/
		void setFragmentProgramParameters(GpuProgramParametersSharedPtr params);
		/** Gets the name of the fragment program used by this pass. */
		const String& getFragmentProgramName(void);
		/** Gets the vertex program parameters used by this pass. */
		GpuProgramParametersSharedPtr getFragmentProgramParameters(void);
		/** Gets the vertex program used by this pass, only available after _load(). */
		GpuProgram* getFragmentProgram(void);

		/** Splits this Pass to one which can be handled in the number of
			texture units specified.
		@remarks
			Only works on non-programmable passes, programmable passes cannot be
			split, it's up to the author to ensure that there is a fallback Technique
			for less capable cards.
		@param numUnits The target number of texture units
		@returns A new Pass which contains the remaining units, and a scene_blend
				setting appropriate to approximate the multitexture. This Pass will be 
				attached to the parent Technique of this Pass.
		*/
		Pass* _split(unsigned short numUnits);

		/** Internal method for loading this pass. */
		void _load(void);
		/** Internal method for unloading this pass. */
		void _unload(void);
        // Is this loaded?
        bool isLoaded(void);

        /** Gets the 'hash' of this pass, ie a precomputed number to use for sorting
        @remarks
            This hash is used to sort passes, and for this reason the pass is hashed
            using firstly its index (so that all passes are rendered in order), then
            by the textures which it's TextureUnitState instances are using.
        */
        unsigned long getHash(void);

        /// Internal method for recalculating the hash
        void _recalculateHash(void);
        /** Tells the pass that it needs recompilation. */
        void _notifyNeedsRecompile(void);

        /** Update any automatic parameters on this pass */
        void _updateAutoParams(Renderable* rend, Camera* cam);

        // --------------------------------------------------------------------
    };


}

#endif
