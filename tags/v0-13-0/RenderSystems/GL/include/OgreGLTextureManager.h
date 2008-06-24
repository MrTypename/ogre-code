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

#ifndef __SDLTextureManager_H__
#define __SDLTextureManager_H__

#include "OgreGLPrerequisites.h"
#include "OgreTextureManager.h"
#include "OgreGLTexture.h"
#include "OgreGLSupport.h"

namespace Ogre {
    /** SDL/GL-specific implementation of a TextureManager */
    class GLTextureManager : public TextureManager
    {
    public:
        GLTextureManager(GLSupport& support);
        virtual ~GLTextureManager();

        /** Creates a SDLTexture resource. 
        */
        virtual Texture* create( const String& name, TextureType texType);
        /** Creates a render target surface.
        */
        virtual Texture * createAsRenderTarget( const String& name ) { return NULL; }

        virtual Texture * createManual( const String& name, TextureType texType,
            uint width, uint height, uint num_mips, PixelFormat format, 
            TextureUsage usage );

        /** Unloads & destroys textures. */
        void unloadAndDestroyAll();
    private:
        GLSupport& mGLSupport;
    };
}
#endif