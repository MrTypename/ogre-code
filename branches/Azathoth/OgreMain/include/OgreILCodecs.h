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

#ifndef _DDSCodec_H__
#define _DDSCodec_H__

#include <list>

namespace Ogre 
{
    class ILImageCodec;
    
    class ILCodecs {
    protected:
       static std::list<ILImageCodec*> codeclist;
    public:
    	// Register all codecs provided by this module
	   static void registerCodecs(void);
	   // Delete all codecs provided by this module
	   static void deleteCodecs(void);
    };

} // namespace Ogre


#endif // #ifndef _DDSCodec_H__

