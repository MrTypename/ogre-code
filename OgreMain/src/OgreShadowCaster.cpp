/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2004 The OGRE Team
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
#include "OgreLight.h"

namespace Ogre {
    // ------------------------------------------------------------------------
    void ShadowCaster::updateEdgeListLightFacing(EdgeData* edgeData, 
        const Vector4& lightPos)
    {
        edgeData->updateTriangleLightFacing(lightPos);
    }
    // ------------------------------------------------------------------------
    void ShadowCaster::generateShadowVolume(EdgeData* edgeData, 
        HardwareIndexBufferSharedPtr indexBuffer, const Light* light,
        ShadowRenderableList& shadowRenderables, unsigned long flags)
    {
        // Edge groups should be 1:1 with shadow renderables
        assert(edgeData->edgeGroups.size() == shadowRenderables.size());

        EdgeData::EdgeGroupList::iterator egi, egiend;
        ShadowRenderableList::iterator si;

        Light::LightTypes lightType = light->getType();

        // Lock index buffer for writing
        unsigned short* pIdx = static_cast<unsigned short*>(
            indexBuffer->lock(HardwareBuffer::HBL_DISCARD));
        size_t indexStart = 0;

        // Iterate over the groups and form renderables for each based on their
        // lightFacing
        si = shadowRenderables.begin();
        egiend = edgeData->edgeGroups.end();
        for (egi = edgeData->edgeGroups.begin(); egi != egiend; ++egi, ++si)
        {
            EdgeData::EdgeGroup& eg = *egi;
            // Initialise the index bounds for this shadow renderable
            RenderOperation* shadOp = (*si)->getRenderOperationForUpdate();
            shadOp->indexData->indexCount = 0;
            shadOp->indexData->indexStart = indexStart;
            // original number of verts (without extruded copy)
            size_t originalVertexCount = eg.vertexData->vertexCount;

            EdgeData::EdgeList::iterator i, iend;
            iend = eg.edges.end();
            for (i = eg.edges.begin(); i != iend; ++i)
            {
                EdgeData::Edge& edge = *i;

                EdgeData::Triangle &t1 = edgeData->triangles[edge.triIndex[0]];
                EdgeData::Triangle &t2 = edgeData->triangles[edge.triIndex[1]];
                if (t1.lightFacing && (edge.degenerate || !t2.lightFacing))
                {
                    /* Silhouette edge, first tri facing the light
                    Also covers degenerate tris where only tri 1 is valid
                    Remember verts run anticlockwise along the edge from 
                    tri 0 so to point shadow volume tris outward, light cap 
                    indexes have to be backwards

                    We emit 2 tris if light is a point light, 1 if light 
                    is directional, because directional lights cause all
                    points to converge to a single point at infinity.

                    First side tri = near1, near0, far0
                    Second tri = far0, far1, near1

                    'far' indexes are 'near' index + originalVertexCount
                    because 'far' verts are in the second half of the 
                    buffer
                    */
                    *pIdx++ = edge.vertIndex[1];
                    *pIdx++ = edge.vertIndex[0];
                    *pIdx++ = edge.vertIndex[0] + originalVertexCount;
                    shadOp->indexData->indexCount += 3;

                    if (lightType != Light::LT_DIRECTIONAL)
                    {
                        // additional tri to make quad
                        *pIdx++ = edge.vertIndex[0] + originalVertexCount;
                        *pIdx++ = edge.vertIndex[1] + originalVertexCount;
                        *pIdx++ = edge.vertIndex[1];
                        shadOp->indexData->indexCount += 3;
                    }
                }
                else if (!t1.lightFacing && (edge.degenerate || t2.lightFacing))
                {
                    // Silhouette edge, second tri facing the light
                    // Note edge indexes inverse of when t1 is light facing 
                    *pIdx++ = edge.vertIndex[0];
                    *pIdx++ = edge.vertIndex[1];
                    *pIdx++ = edge.vertIndex[1] + originalVertexCount;
                    shadOp->indexData->indexCount += 3;

                    if (lightType != Light::LT_DIRECTIONAL)
                    {
                        // additional tri to make quad
                        *pIdx++ = edge.vertIndex[1] + originalVertexCount;
                        *pIdx++ = edge.vertIndex[0] + originalVertexCount;
                        *pIdx++ = edge.vertIndex[0];
                        shadOp->indexData->indexCount += 3;
                    }
                }

            }
            // update next indexStart (al renderables sharing the buffer)
            indexStart += shadOp->indexData->indexCount;

        }

        // Unlock index buffer
        indexBuffer->unlock();

    }
    // ------------------------------------------------------------------------
    void ShadowCaster::extrudeVertices(
        HardwareVertexBufferSharedPtr vertexBuffer, 
        size_t originalVertexCount, const Vector4& light)
    {
        #define EXTRUSION_DISTANCE 10000000;
        // Extrude the first area of the buffer into the second area
        // Lock the entire buffer for writing, even though we'll only be
        // updating the latter because you can't have 2 locks on the same
        // buffer
        Real* pSrc = static_cast<Real*>(
            vertexBuffer->lock(HardwareBuffer::HBL_NORMAL));

        Real* pDest = pSrc + originalVertexCount * 3;
        // Assume directional light, extrusion is along light direction
        Vector3 extrusionDir(light.x, light.y, light.z);
        extrusionDir.normalise();
        extrusionDir *= EXTRUSION_DISTANCE;
        for (size_t vert = 0; vert < originalVertexCount; ++vert)
        {
            if (light.w != 0.0f)
            {
                // Point light, adjust extrusionDir
                extrusionDir.x = pSrc[0] - light.x;
                extrusionDir.y = pSrc[1] - light.y;
                extrusionDir.z = pSrc[2] - light.z;
                extrusionDir.normalise();
                extrusionDir *= EXTRUSION_DISTANCE;
            }
            *pDest++ = *pSrc++ + extrusionDir.x;
            *pDest++ = *pSrc++ + extrusionDir.y;
            *pDest++ = *pSrc++ + extrusionDir.z;

        }
        vertexBuffer->unlock();

    }
}
