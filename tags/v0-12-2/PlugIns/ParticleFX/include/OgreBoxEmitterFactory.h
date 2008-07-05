/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2002 The OGRE Team
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
#ifndef __BoxEmitterFactory_H__
#define __BoxEmitterFactory_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreBoxEmitter.h"


namespace Ogre {

    /** Factory class for particle emitter of type "Box".
    @remarks
        Creates instances of BoxEmitter to be used in particle systems. 
    */
    class _OgreParticleFXExport BoxEmitterFactory : public ParticleEmitterFactory
    {
    protected:

    public:
        /** See ParticleEmitterFactory */
        String getName() 
        { 
            return "Box"; 
        }

        /** See ParticleEmitterFactory */
        ParticleEmitter* createEmitter(void) 
        {
            ParticleEmitter* emit = new BoxEmitter();
            mEmitters.push_back(emit);
            return emit;
        }

    };

}

#endif
