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
#ifndef __JPEGCodec_H__
#define __JPEGCodec_H__

#include "OgrePrerequisites.h"

#include "OgreILImageCodec.h"

namespace Ogre {

    /** ImageCodec specialized in JPEG images.
    */
    class _OgreExport JPEGCodec : public ILImageCodec
    {
    public:
        void code( const DataChunk& input, DataChunk* output, ... ) const;
        virtual Codec::CodecData* decode(const DataChunk& input, 
                                         DataChunk* output, ... ) const;

        String getType() const { return "jpeg"; }

        unsigned int getILType(void) const;
    };

    class _OgreExport JPGCodec : public JPEGCodec
    {
    public:
        String getType() const { return "jpg"; }
    };

} //namespace

#endif
