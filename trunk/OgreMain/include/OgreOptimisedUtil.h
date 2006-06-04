/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
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
#ifndef __OptimisedUtil_H__
#define __OptimisedUtil_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Utility class for provides optimised functions.
    @note
        This class are supposed used by internal engine only.
    */
    class _OgreExport OptimisedUtil
    {
    private:
        /// Privated copy constructor, to prevent misuse
        OptimisedUtil(const OptimisedUtil& rhs); /* do nothing, should not use */
        /// Privated operator=, to prevent misuse
        OptimisedUtil& operator=(const OptimisedUtil& rhs); /* do not use */

    protected:
        /// Store a pointer to the implementation
        static OptimisedUtil* msImplementation;

        /// Detect best implementation based on run-time environment
        static OptimisedUtil* _detectImplementation(void);

    public:
        // Default constructor
        OptimisedUtil(void) {}
        // Destructor
        virtual ~OptimisedUtil() {}

        /** Gets the implementation of this class.
        @note
            Don't cache the pointer returned by this function, it'll change due
            run-time environment detection to pick up the best implementation.
        */
        static OptimisedUtil* getImplementation(void) { return msImplementation; }

        /** Performs software vertex skinning.
        @param srcPosPtr Pointer to source position buffer.
        @param destPosPtr Pointer to destination position buffer.
        @param srcNormPtr Pointer to source normal buffer, if NULL,
            means blend position only.
        @param destNormPtr Pointer to destination normal buffer, it's
            ignored if srcNormPtr is NULL.
        @param blendWeightPtr Pointer to blend weight buffer.
        @param blendIndexPtr Pointer to blend index buffer.
        @param blendMatrices An array of pointer of blend matrix, the matrix
            must be aligned to SIMD alignment, but not necessary for the array
            itself.
        @param srcPosStride The stride of source position in bytes.
        @param destPosStride The stride of destination position in bytes.
        @param srcNormStride The stride of source normal in bytes,
            it's ignored if srcNormPtr is NULL.
        @param destNormStride The stride of destination normal in bytes,
            it's ignored if srcNormPtr is NULL.
        @param blendWeightStride The stride of blend weight buffer in bytes.
        @param blendIndexStride The stride of blend index buffer in bytes.
        @param numWeightsPerVertex Number of blend weights per-vertex, as well
            as for blend indices.
        @param numVertices Number of vertices to blend.
        */
        virtual void softwareVertexSkinning(
            const float *srcPosPtr, float *destPosPtr,
            const float *srcNormPtr, float *destNormPtr,
            const float *blendWeightPtr, const unsigned char* blendIndexPtr,
            const Matrix4* const* blendMatrices,
            size_t srcPosStride, size_t destPosStride,
            size_t srcNormStride, size_t destNormStride,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numVertices) = 0;

        /** Concatenate an affine matrix to an array of affine matrices.
        @note
            An affine matrix is a 4x4 matrix with row 3 equal to (0, 0, 0, 1),
            e.g. no projective coefficients.
        @param baseMatrix The matrix used as first operand.
        @param srcMatrices An array of matrix used as second operand.
        @param dstMatrices An array of matrix to store matrix concatenate results.
        @param numMatrices Number of matrices in the array.
        */
        virtual void concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices) = 0;
    };

    /** Returns raw offseted of the given pointer.
    @note
        The offset are in bytes, no matter what type of the pointer.
    */
    template <class T>
    static FORCEINLINE T* rawOffsetPointer(T* ptr, ptrdiff_t offset)
    {
        return (T*)((char*)(ptr) + offset);
    }

    /** Advance the pointer with raw offset.
    @note
        The offset are in bytes, no matter what type of the pointer.
    */
    template <class T>
    static FORCEINLINE void advanceRawPointer(T*& ptr, ptrdiff_t offset)
    {
        ptr = rawOffsetPointer(ptr, offset);
    }

}

#endif  // __OptimisedUtil_H__
