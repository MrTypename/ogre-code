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
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreException.h"

namespace Ogre 
{
	D3D9TextureManager::D3D9TextureManager( LPDIRECT3DDEVICE9 pD3DDevice ) : TextureManager()
	{
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( Exception::ERR_INVALIDPARAMS, "Invalid Direct3DDevice passed", "D3D9TextureManager::D3D9TextureManager" );
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
	}

	D3D9TextureManager::~D3D9TextureManager()
	{
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

	}

    Resource* D3D9TextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        return new D3D9Texture(this, name, handle, group, isManual, loader, mpD3DDevice); 
    }


}