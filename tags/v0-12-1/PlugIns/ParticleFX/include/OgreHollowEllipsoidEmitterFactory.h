/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright ) 2002 Tels >http://bloodgate.com> based on BoxEmitter
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
#ifndef __HollowEllipsoidEmitterFactory_H__
#define __HollowEllipsoidEmitterFactory_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreHollowEllipsoidEmitter.h"


namespace Ogre {

    /** Factory class for particle emitter of type "HollowEllipsoid".
    @remarks
        Creates instances of HollowEllipsoidEmitter to be used in particle systems. 
    */
    class _OgreParticleFXExport HollowEllipsoidEmitterFactory : public ParticleEmitterFactory
    {
    protected:

    public:
        /** See ParticleEmitterFactory */
        String getName() 
        { 
            return "HollowEllipsoid"; 
        }

        /** See ParticleEmitterFactory */
        ParticleEmitter* createEmitter(void) 
        {
            ParticleEmitter* emit = new HollowEllipsoidEmitter();
            mEmitters.push_back(emit);
            return emit;
        }

    };

}

#endif

