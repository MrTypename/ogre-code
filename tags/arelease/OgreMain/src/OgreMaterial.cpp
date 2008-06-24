/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreMaterial.h"

#include "OgreSceneManagerEnumerator.h"

namespace Ogre {

    Material* Material::mDefaultSettings = 0;

    //-----------------------------------------------------------------------
    Material::Material()
    {
        static unsigned short num = 1;
        char name[14];


        mDeferLoad = false;
        sprintf(name, "Undefined%d", num++);
        mName = name;

        mHandle = -1;

        // Default to white ambient & diffuse, no specular / emissive
        mAmbient = mDiffuse = ColourValue::White;
        mSpecular = mEmissive = ColourValue::Black;
        mShininess = 0;



        // No textures
        mNumTextureLayers = 0;

        // No fog
        mFogOverride = false;

        // Default blending (overwrite)
        mSourceBlendFactor = SBF_ONE;
        mDestBlendFactor = SBF_ZERO;

        mDepthCheck = true;
        mDepthWrite = true;
        mDepthFunc = CMPF_LESS_EQUAL;
        mCullMode = CULL_CLOCKWISE;
        mManualCullMode = MANUAL_CULL_BACK;
        mLightingEnabled = true;
        mShadeOptions = SO_GOURAUD;
        mTextureFiltering = TFO_BILINEAR;


    }
    //-----------------------------------------------------------------------
    Material::Material( const String& name, bool deferLoad)
    {
        applyDefaults();
        mDeferLoad = deferLoad;

        // Assign name
        mName = name;
        mHandle = -1;


    }
    //-----------------------------------------------------------------------
    Material& Material::operator=(const Material& rhs)
    {
        mName = rhs.mName;
        mHandle = rhs.mHandle;
        mAmbient = rhs.mAmbient;
        mDiffuse = rhs.mDiffuse;
        mEmissive = rhs.mEmissive;
        mShininess = rhs.mShininess;
        mSpecular = rhs.mSpecular;
        mNumTextureLayers = rhs.mNumTextureLayers;
        mSourceBlendFactor = rhs.mSourceBlendFactor;
        mDestBlendFactor = rhs.mDestBlendFactor;

        // Copy texture layers
        for (int i = 0; i < mNumTextureLayers; ++i)
        {
            mTextureLayers[i] = rhs.mTextureLayers[i];
        }

        mDepthCheck = rhs.mDepthCheck;
        mDepthWrite = rhs.mDepthWrite;
        mDepthFunc = rhs.mDepthFunc;
        mCullMode = rhs.mCullMode;
        mLightingEnabled = rhs.mLightingEnabled;
        mShadeOptions = rhs.mShadeOptions;
        mTextureFiltering = rhs.mTextureFiltering;
        mFogOverride = rhs.mFogOverride;
        mFogMode = rhs.mFogMode;
        mFogColour = rhs.mFogColour;
        mFogDensity = rhs.mFogDensity;
        mFogStart = rhs.mFogStart;
        mFogEnd = rhs.mFogEnd;
        mDeferLoad = rhs.mDeferLoad;



        return *this;

    }
    //-----------------------------------------------------------------------
    void Material::setAmbient(Real red, Real green, Real blue)
    {
        mAmbient.r = red;
        mAmbient.g = green;
        mAmbient.b = blue;

    }
    //-----------------------------------------------------------------------
    void Material::setAmbient(const ColourValue& ambient)
    {
        mAmbient = ambient;
    }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(Real red, Real green, Real blue)
    {
        mDiffuse.r = red;
        mDiffuse.g = green;
        mDiffuse.b = blue;
    }
    //-----------------------------------------------------------------------
    void Material::setDiffuse(const ColourValue& diffuse)
    {
        mDiffuse = diffuse;
    }
    //-----------------------------------------------------------------------
    void Material::setSpecular(Real red, Real green, Real blue)
    {
        mSpecular.r = red;
        mSpecular.g = green;
        mSpecular.b = blue;
    }
    //-----------------------------------------------------------------------
    void Material::setSpecular(const ColourValue& specular)
    {
        mSpecular = specular;
    }
    //-----------------------------------------------------------------------
    void Material::setShininess(Real val)
    {
        mShininess = val;
    }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(Real red, Real green, Real blue)
    {
        mEmissive.r = red;
        mEmissive.g = green;
        mEmissive.b = blue;

    }
    //-----------------------------------------------------------------------
    void Material::setSelfIllumination(const ColourValue& selfIllum)
    {
        mEmissive = selfIllum;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Material::getAmbient(void) const
    {
        return mAmbient;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Material::getDiffuse(void) const
    {
        return mDiffuse;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Material::getSpecular(void) const
    {
        return mSpecular;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Material::getSelfIllumination(void) const
    {
        return mEmissive;
    }
    //-----------------------------------------------------------------------
    Real Material::getShininess(void) const
    {
        return mShininess;
    }


    //-----------------------------------------------------------------------
    // Single definition of method to retrieve next handle, to avoid duplication of static member value
    void Material::assignNextHandle(void)
    {
        static int nextHandle = 1;

        mHandle = nextHandle++;
    }
    //-----------------------------------------------------------------------
    const String& Material::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    int Material::getHandle(void) const
    {
        return mHandle;
    }
    //-----------------------------------------------------------------------
    Material::TextureLayer* Material::addTextureLayer(const String& textureName, int texCoordSet)
    {
        mTextureLayers[mNumTextureLayers].setDeferredLoad(mDeferLoad);
        mTextureLayers[mNumTextureLayers].setTextureName(textureName);
        mTextureLayers[mNumTextureLayers].setTextureCoordSet(texCoordSet);
        return &mTextureLayers[mNumTextureLayers++];
    }
    //-----------------------------------------------------------------------
    Material::TextureLayer* Material::getTextureLayer(int index)
    {
        return &mTextureLayers[index];
    }
    //-----------------------------------------------------------------------
    void Material::removeTextureLayer()
    {
        mNumTextureLayers--;
    }
    //-----------------------------------------------------------------------
    void Material::removeAllTextureLayers(void)
    {
        mNumTextureLayers = 0;
    }
    //-----------------------------------------------------------------------
    void Material::setSceneBlending(SceneBlendType sbt)
    {
        // Turn predefined type into blending factors
        switch (sbt)
        {
        case SBT_TRANSPARENT_ALPHA:
            setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
            break;
        case SBT_TRANSPARENT_COLOUR:
            setSceneBlending(SBF_SOURCE_COLOUR, SBF_ONE_MINUS_SOURCE_COLOUR);
            break;
        case SBT_ADD:
            setSceneBlending(SBF_ONE, SBF_ONE);
            break;
        // TODO: more
        }

    }
    //-----------------------------------------------------------------------
    void Material::setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        mSourceBlendFactor = sourceFactor;
        mDestBlendFactor = destFactor;

        if ((sourceFactor == SBF_ONE && destFactor == SBF_ZERO) ||
            (sourceFactor == SBF_ZERO && destFactor == SBF_ONE))
        {
            // Solid

        }
        else
        {
            // Transparent
            setDepthWriteEnabled(false);
        }

    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Material::getSourceBlendFactor(void) const
    {
        return mSourceBlendFactor;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Material::getDestBlendFactor(void) const
    {
        return mDestBlendFactor;
    }
    //-----------------------------------------------------------------------
    bool Material::isTransparent(void) const
    {
        if (mSourceBlendFactor != SBF_ONE || mDestBlendFactor != SBF_ZERO)
            return true;
        else
            return false;
    }
    //-----------------------------------------------------------------------
    void Material::setDepthCheckEnabled(bool enabled)
    {
        mDepthCheck = enabled;
    }
    //-----------------------------------------------------------------------
    bool Material::getDepthCheckEnabled(void) const
    {
        return mDepthCheck;
    }
    //-----------------------------------------------------------------------
    void Material::setDepthWriteEnabled(bool enabled)
    {
        mDepthWrite = enabled;
    }
    //-----------------------------------------------------------------------
    bool Material::getDepthWriteEnabled(void) const
    {
        return mDepthWrite;
    }
    //-----------------------------------------------------------------------
    void Material::setDepthFunction( CompareFunction func)
    {
        mDepthFunc = func;
    }
    //-----------------------------------------------------------------------
    CompareFunction Material::getDepthFunction(void) const
    {
        return mDepthFunc;
    }
    //-----------------------------------------------------------------------
    void Material::setCullingMode( CullingMode mode)
    {
        mCullMode = mode;
    }
    //-----------------------------------------------------------------------
    CullingMode Material::getCullingMode(void) const
    {
        return mCullMode;
    }
    //-----------------------------------------------------------------------
    void Material::setLightingEnabled(bool enabled)
    {
        mLightingEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    bool Material::getLightingEnabled(void) const
    {
        return mLightingEnabled;
    }
    //-----------------------------------------------------------------------
    void Material::setShadingMode(ShadeOptions mode)
    {
        mShadeOptions = mode;
    }
    //-----------------------------------------------------------------------
    ShadeOptions Material::getShadingMode(void) const
    {
        return mShadeOptions;
    }
    //-----------------------------------------------------------------------
    void Material::setTextureFiltering(TextureFilterOptions mode)
    {
        mTextureFiltering = mode;
    }
    //-----------------------------------------------------------------------
    TextureFilterOptions Material::getTextureFiltering(void) const
    {
        return mTextureFiltering;
    }
    //-----------------------------------------------------------------------
    void Material::applyDefaults(void)
    {
        *this = *Material::mDefaultSettings;
    }
    //-----------------------------------------------------------------------
    void Material::setManualCullingMode(ManualCullingMode mode)
    {
        mManualCullMode = mode;
    }
    //-----------------------------------------------------------------------
    ManualCullingMode Material::getManualCullingMode(void) const
    {
        return mManualCullMode;
    }
    //-----------------------------------------------------------------------
    void Material::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
        mFogOverride = overrideScene;
        if (overrideScene)
        {
            mFogMode = mode;
            mFogColour = colour;
            mFogStart = start;
            mFogEnd = end;
            mFogDensity = density;
        }
    }
    //-----------------------------------------------------------------------
    bool Material::getFogOverride(void) const
    {
        return mFogOverride;
    }
    //-----------------------------------------------------------------------
    FogMode Material::getFogMode(void) const
    {
        return mFogMode;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Material::getFogColour(void) const
    {
        return mFogColour;
    }
    //-----------------------------------------------------------------------
    Real Material::getFogStart(void) const
    {
        return mFogStart;
    }
    //-----------------------------------------------------------------------
    Real Material::getFogEnd(void) const
    {
        return mFogEnd;
    }
    //-----------------------------------------------------------------------
    Real Material::getFogDensity(void) const
    {
        return mFogDensity;
    }
    //-----------------------------------------------------------------------
    void Material::load(void)
    {
        if (!mIsLoaded)
        {
            if (mDeferLoad)
            {
                // Load all textures & controllers
                for (int i = 0; i < mNumTextureLayers; ++i)
                {
                    mTextureLayers[i]._load();
                }
                mDeferLoad = false;
            }


            mIsLoaded = true;
        }


    }
    //-----------------------------------------------------------------------
    void Material::unload(void)
    {
    }
    //-----------------------------------------------------------------------
    bool Material::_compareSurfaceParams(const Material& cmp) const
    {
        if (mAmbient != cmp.mAmbient || mDiffuse != cmp.mDiffuse ||
         mSpecular != cmp.mSpecular || mEmissive != cmp.mEmissive ||
         mShininess != cmp.mShininess)
        {
            return false;
        }
        else
        {
            return true;
        }
    }




}