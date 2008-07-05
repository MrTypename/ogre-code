/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#ifndef __REFAPP_PLANE_H__
#define __REFAPP_PLANE_H__

#include "OgreRefAppPrerequisites.h"
#include "OgreRefAppApplicationObject.h"

namespace OgreRefApp {

    class _OgreRefAppExport FinitePlane : public ApplicationObject
    {
    protected:
        Real mWidth;
        Real mHeight;
        void setUp(const String& name);
    public:
        FinitePlane(const String& name, Real width, Real height);
        ~FinitePlane();
    };

}
#endif