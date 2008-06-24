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
#ifndef __XSIHELPER_H__
#define __XSIHELPER_H__

#include <xsi_string.h>
#include <xsi_vertexcolor.h>
#include <stdlib.h>
#include "OgrePrerequisites.h"

/// Useful function to convert an XSI CString to an Ogre String
inline Ogre::String XSItoOgre(const XSI::CString& xsistr)
{
    // XSI CString is wide character

    if (xsistr.IsEmpty())
    {
        return Ogre::StringUtil::BLANK;
    }

    // first find out the size required
    size_t c = ::wcstombs(0, xsistr.GetWideString(), 2048);
    // temp character string (add one for terminator)
    char* tmp = new char[c+1];
    // do the real conversion
    ::wcstombs(tmp, xsistr.GetWideString(), c);
    Ogre::String ret(tmp);
    delete [] tmp;

    return ret;
}
/// Useful function to convert an Ogre String to an XSI CString
inline XSI::CString OgretoXSI(const Ogre::String& str)
{
    // XSI CString is wide character

    if (str.empty())
    {
        return XSI::CString();
    }

    // first find out the size required
    size_t c = ::mbstowcs(0, str.c_str(), 2048);
    // temp character string (add one for terminator)
    wchar_t* tmp = new wchar_t[c+1];
    // do the real conversion
    ::mbstowcs(tmp, str.c_str(), c);
    XSI::CString ret(tmp);
    delete [] tmp;

    return ret;
}

inline Ogre::Vector3 XSItoOgre(const XSI::MATH::CVector3& xsiVec)
{
    return Ogre::Vector3(xsiVec.GetX(), xsiVec.GetY(), xsiVec.GetZ());
}

inline Ogre::RGBA XSItoOgre(const XSI::CVertexColor& xsiColour)
{
    Ogre::uint32 ret = 0;
    ret += xsiColour.a << 24;
    ret += xsiColour.r << 16;
    ret += xsiColour.g << 8;
    ret += xsiColour.b;

    return ret;

}

#endif
