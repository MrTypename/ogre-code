/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
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
#include "OgreD3D7TextureManager.h"
#include "OgreD3D7Texture.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    //TextureManager* D3DTextureManager::ms_Singleton=0;

    // IMPORTANT: auto-inlining must be disabled for this class
    // otherwise problems with the dll boundary occur on the constructor in release mode

    D3DTextureManager::D3DTextureManager(LPDIRECT3DDEVICE7 lpD3D)
        : TextureManager()
    {
        mlpD3DDevice = lpD3D;
    }
    //-----------------------------------------------------------------------
    D3DTextureManager::~D3DTextureManager()
    {
        this->unloadAndDestroyAll();
    }
    //-----------------------------------------------------------------------
    Resource* D3DTextureManager::create( const String& name)
    {
        D3DTexture* t = new D3DTexture(name, mlpD3DDevice);
        t->enable32Bit(mIs32Bit);
        return t;
    }
    //-----------------------------------------------------------------------
    void D3DTextureManager::unloadAndDestroyAll()
    {
        // Reimplement this to ensure deletion happens in same dll as creation

        // Unload & delete resources in turn
        for (ResourceMap::iterator i = mResources.begin(); i != mResources.end(); ++i)
        {
            i->second->unload();
            delete i->second;
        }

        // Empty the list
        mResources.clear();
    }
}