/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreWin32Timer.h"

namespace Ogre {
    //-------------------------------------------------------------------------
    void Win32Timer::reset()
    {
        Timer::reset();
        QueryPerformanceFrequency(&mFrequency);
        QueryPerformanceCounter(&mStartTime);
        mStartTick = GetTickCount();
    }
    //-------------------------------------------------------------------------
    unsigned long Win32Timer::getMilliseconds()
    {
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);
        
        // scale by 1000 for milliseconds
        unsigned long newTicks = (unsigned long)
            (1000*(curTime.QuadPart-mStartTime.QuadPart)/mFrequency.QuadPart);

        // detect and compensate for performance counter leaps
        // (surprisingly common, see Microsoft KB: Q274323)
        unsigned long check = GetTickCount() - mStartTick;
        signed long msecOff = (signed long)(newTicks - check);
        if (msecOff < -100 || msecOff > 100)
        {
            mStartTime.QuadPart += msecOff * mFrequency.QuadPart / 1000;
            newTicks = check;
        }

        return newTicks;
	}
    //-------------------------------------------------------------------------
	unsigned long Win32Timer::getMicroseconds()
	{
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);
        
        // scale by 1000000 for microseconds;
        // also get milliseconds to check against GetTickCount
        unsigned long newMicro = (unsigned long)
            (1000000*(curTime.QuadPart-mStartTime.QuadPart)/mFrequency.QuadPart);
        unsigned long newTicks = newMicro / 1000;
        
        // detect and compensate for performance counter leaps
        // (surprisingly common, see Microsoft KB: Q274323)
        unsigned long check = GetTickCount() - mStartTick;
        signed long msecOff = (signed long)(newTicks - check);
        if (msecOff < -100 || msecOff > 100)
        {
            mStartTime.QuadPart += msecOff * mFrequency.QuadPart / 1000;
            newMicro -= msecOff * 1000;
        }

        return newMicro;
	}

}