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
#ifndef __D3D9DRIVER_H__
#define __D3D9DRIVER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreString.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	static unsigned int driverCount = 0;

	class D3D9VideoModeList;
	class D3D9VideoMode;

	class D3D9Driver
	{
	private:
		LPDIRECT3D9 mpD3D;
		unsigned int mAdapterNumber;
		D3DADAPTER_IDENTIFIER9 mAdapterIdentifier;
		D3DDISPLAYMODE mDesktopDisplayMode;
		D3D9VideoModeList* mpVideoModeList;
		unsigned int tempNo;

	public:
		// Constructors
		D3D9Driver();						// Default
		D3D9Driver( const D3D9Driver &ob );	// Copy
		D3D9Driver( LPDIRECT3D9 pD3D, unsigned int adapterNumber, D3DADAPTER_IDENTIFIER9 adapterIdentifer, D3DDISPLAYMODE desktopDisplayMode );
		~D3D9Driver();

		// Information accessors
		String DriverName() const;
		String DriverDescription() const;

		LPDIRECT3D9 getD3D() { return mpD3D; }
		unsigned int getAdapterNumber() const { return mAdapterNumber; }
		D3DADAPTER_IDENTIFIER9 getAdapterIdentifier() const { return mAdapterIdentifier; }
		D3DDISPLAYMODE getDesktopMode() const { return mDesktopDisplayMode; }

		D3D9VideoModeList* getVideoModeList();
	};
}
#endif