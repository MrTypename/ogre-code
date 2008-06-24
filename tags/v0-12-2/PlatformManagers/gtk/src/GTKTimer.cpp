/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourgeforge.net/

Copyright � 2000-2003 Steven J. Streeting
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

#include "GTKTimer.h"
#include <sys/time.h>

using namespace Ogre;

void GTKTimer::reset()
{
    Timer::reset();
	gettimeofday(&start, NULL);
}

unsigned long GTKTimer::getMilliseconds()
{
    struct timeval now;
	gettimeofday(&now, NULL);
    return (now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
}