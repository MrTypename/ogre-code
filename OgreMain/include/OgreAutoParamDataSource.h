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
#ifndef __AutoParamDataSource_H_
#define __AutoParamDataSource_H_

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreMatrix4.h"
#include "OgreVector4.h"
#include "OgreLight.h"
#include "OgreColourValue.h"

namespace Ogre {


    /** This utility class is used to hold the information used to generate the matrices
    and other information required to automatically populate GpuProgramParameters.
    @remarks
        This class exercises a lazy-update scheme in order to avoid having to update all
        the information a GpuProgramParameters class could possibly want all the time. 
        It relies on the SceneManager to update it when the base data has changed, and
        will calculate concatenated matrices etc only when required, passing back precalculated
        matrices when they are requested more than once when the underlying information has
        not altered.
    */
    class _OgreExport AutoParamDataSource
    {
    protected:
        mutable Matrix4 mWorldMatrix[256];
        mutable size_t mWorldMatrixCount;
        mutable Matrix4 mWorldViewMatrix;
        mutable Matrix4 mViewProjMatrix;
        mutable Matrix4 mWorldViewProjMatrix;
        mutable Matrix4 mInverseWorldMatrix;
        mutable Matrix4 mInverseWorldViewMatrix;
        mutable Matrix4 mInverseViewMatrix;
        mutable Vector4 mCameraPositionObjectSpace;
        mutable Matrix4 mTextureViewProjMatrix;

        mutable bool mWorldMatrixDirty;
        mutable bool mWorldViewMatrixDirty;
        mutable bool mViewProjMatrixDirty;
        mutable bool mWorldViewProjMatrixDirty;
        mutable bool mInverseWorldMatrixDirty;
        mutable bool mInverseWorldViewMatrixDirty;
        mutable bool mInverseViewMatrixDirty;
        mutable bool mCameraPositionObjectSpaceDirty;
        mutable bool mTextureViewProjMatrixDirty;
		mutable ColourValue mAmbientLight;

        const Renderable* mCurrentRenderable;
        const Camera* mCurrentCamera;
        const LightList* mCurrentLightList;
        const Frustum* mCurrentTextureProjector;

        Light mBlankLight;
    public:
        AutoParamDataSource();
        ~AutoParamDataSource();
        /** Updates the current renderable */
        void setCurrentRenderable(const Renderable* rend);
        /** Updates the current camera */
        void setCurrentCamera(const Camera* cam);
        /** Sets the light list that should be used */
        void setCurrentLightList(const LightList* ll);
        /** Sets the current texture projector */
        void setTextureProjector(const Frustum* frust);

        const Matrix4& getWorldMatrix(void) const;
        const Matrix4* getWorldMatrixArray(void) const;
        size_t getWorldMatrixCount(void) const;
        const Matrix4& getViewMatrix(void) const;
        const Matrix4& getViewProjectionMatrix(void) const;
        const Matrix4& getProjectionMatrix(void) const;
        const Matrix4& getWorldViewProjMatrix(void) const;
        const Matrix4& getWorldViewMatrix(void) const;
        const Matrix4& getInverseWorldMatrix(void) const;
        const Matrix4& getInverseWorldViewMatrix(void) const;
        const Matrix4& getInverseViewMatrix(void) const;
        const Vector4& getCameraPositionObjectSpace(void) const;
        /** Get the light which is 'index'th closest to the current object */
        const Light& getLight(size_t index) const;
		void setAmbientLightColour(const ColourValue& ambient);
		const ColourValue& getAmbientLightColour(void) const;
        const Matrix4& getTextureViewProjMatrix(void) const;


    };
}

#endif
