/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
#ifndef __D3D9HARDWAREINDEXBUFFER_H__
#define __D3D9HARDWAREINDEXBUFFER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre { 


    class D3D9HardwareIndexBuffer : public HardwareIndexBuffer
    {
    protected:
        LPDIRECT3DINDEXBUFFER9 mlpD3DBuffer;
        /** See HardwareBuffer. */
        void* lockImpl(size_t offset, size_t length, LockOptions options);
        /** See HardwareBuffer. */
		void unlockImpl(void);
    public:
		D3D9HardwareIndexBuffer(IndexType idxType, size_t numIndexes, 
			HardwareBuffer::Usage usage, LPDIRECT3DDEVICE9 pDev, bool useSystemMem, bool useShadowBuffer);
        ~D3D9HardwareIndexBuffer();
        /** See HardwareBuffer. */
        void readData(size_t offset, size_t length, void* pDest);
        /** See HardwareBuffer. */
        void writeData(size_t offset, size_t length, const void* pSource,
				bool discardWholeBuffer = false);

        /// Get the D3D-specific index buffer
        LPDIRECT3DINDEXBUFFER9 getD3DIndexBuffer(void) { return mlpD3DBuffer; }



    };

}



#endif

