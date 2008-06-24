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

#include "OgreStableHeaders.h"
#include "OgreDistanceLodStrategy.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    DistanceLodStrategy::DistanceLodStrategy()
        : LodStrategy("Distance")
    { }
    //-----------------------------------------------------------------------
    Real DistanceLodStrategy::getValueImpl(const MovableObject *movableObject, const Ogre::Camera *camera) const
    {
        // Get squared depth taking into account bounding radius
        // (d - r) ^ 2 = d^2 - 2dr + r^2, but this requires a lot 
        // more computation (including a sqrt) so we approximate 
        // it with d^2 - r^2, which is good enough for determining 
        // lod.
        Real squaredDepth = movableObject->getParentNode()->getSquaredViewDepth(camera) - Math::Sqr(movableObject->getBoundingRadius());
        // Squared depth should never be below 0, so clamp
        squaredDepth = std::max(squaredDepth, Real(0));
        // Now adjust it by the camera bias and return the computed value
        return squaredDepth * camera->_getLodBiasInverse();
    }
    //---------------------------------------------------------------------
    Real DistanceLodStrategy::transformBias(Real factor) const
    {
        assert(factor > 0.0f && "Bias factor must be > 0!");
        return 1.0f / factor;
    }

} // namespace