/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
	(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreRotationAffector.h"
#include "OgreParticleSystem.h"
#include "OgreStringConverter.h"
#include "OgreParticle.h"


namespace Ogre {
    
    // init statics
	RotationAffector::CmdRotationSpeedRangeStart	RotationAffector::msRotationSpeedRangeStartCmd;
    RotationAffector::CmdRotationSpeedRangeEnd		RotationAffector::msRotationSpeedRangeEndCmd;
    RotationAffector::CmdRotationRangeStart			RotationAffector::msRotationRangeStartCmd;
    RotationAffector::CmdRotationRangeEnd			RotationAffector::msRotationRangeEndCmd;
    
    //-----------------------------------------------------------------------
	RotationAffector::RotationAffector() :
		mRotationSpeedRangeStart(0),
		mRotationSpeedRangeEnd(0),
		mRotationRangeStart(0),
		mRotationRangeEnd(0)
    {
		mType = "Rotator";

        // Init parameters
        if (createParamDictionary("RotationAffector"))
        {
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("rotation_speed_range_start", 
				"The start of a range of rotation speed to be assigned to emitted particles.", PT_REAL),
				&msRotationSpeedRangeStartCmd);

			dict->addParameter(ParameterDef("rotation_speed_range_end", 
				"The end of a range of rotation speed to be assigned to emitted particles.", PT_REAL),
				&msRotationSpeedRangeEndCmd);

			dict->addParameter(ParameterDef("rotation_range_start", 
				"The start of a range of rotation angles to be assigned to emitted particles.", PT_REAL),
				&msRotationRangeStartCmd);

			dict->addParameter(ParameterDef("rotation_range_end", 
				"The end of a range of rotation angles to be assigned to emitted particles.", PT_REAL),
				&msRotationRangeEndCmd);
        }
    }

    //-----------------------------------------------------------------------
	void RotationAffector::_initParticle(Particle* pParticle)
	{
		pParticle->setRotation(
            mRotationRangeStart + 
            (Math::UnitRandom() * 
                (mRotationRangeEnd - mRotationRangeStart)));
        pParticle->setRotationSpeed(
            mRotationSpeedRangeStart + 
            (Math::UnitRandom() * 
                (mRotationSpeedRangeEnd - mRotationSpeedRangeStart)));
        
	}
	//-----------------------------------------------------------------------
    void RotationAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;
        Real ds;

        // Rotation adjustments by time
        ds = timeElapsed;

		Real NewRotation;

        while (!pi.end())
        {
            p = pi.getNext();

			NewRotation = p->getRotation() + (ds * p->getRotationSpeed());
			p->setRotation( NewRotation );
        }

    }
    //-----------------------------------------------------------------------
    Real RotationAffector::getRotationSpeedRangeStart(void) const
    {
        return mRotationSpeedRangeStart;
    }
    //-----------------------------------------------------------------------
    Real RotationAffector::getRotationSpeedRangeEnd(void) const
    {
        return mRotationSpeedRangeEnd;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationSpeedRangeStart(Real val)
    {
        mRotationSpeedRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationSpeedRangeEnd(Real val )
    {
        mRotationSpeedRangeEnd = val;
    }
    //-----------------------------------------------------------------------
    Real RotationAffector::getRotationRangeStart(void) const
    {
        return mRotationRangeStart;
    }
    //-----------------------------------------------------------------------
    Real RotationAffector::getRotationRangeEnd(void) const
    {
        return mRotationRangeEnd;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationRangeStart(Real val)
    {
        mRotationRangeStart = val;
    }
    //-----------------------------------------------------------------------
    void RotationAffector::setRotationRangeEnd(Real val )
    {
        mRotationRangeEnd = val;
    }
	//-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationSpeedRangeEnd::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationSpeedRangeEnd() );
    }
    void RotationAffector::CmdRotationSpeedRangeEnd::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationSpeedRangeEnd(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationSpeedRangeStart::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationSpeedRangeStart() );
    }
    void RotationAffector::CmdRotationSpeedRangeStart::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationSpeedRangeStart(StringConverter::parseReal(val));
    }
    
	//-----------------------------------------------------------------------
    String RotationAffector::CmdRotationRangeEnd::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationRangeEnd() );
    }
    void RotationAffector::CmdRotationRangeEnd::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationRangeEnd(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String RotationAffector::CmdRotationRangeStart::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const RotationAffector*>(target)->getRotationRangeStart() );
    }
    void RotationAffector::CmdRotationRangeStart::doSet(void* target, const String& val)
    {
        static_cast<RotationAffector*>(target)->setRotationRangeStart(StringConverter::parseReal(val));
    }
}



