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
#ifndef _D3DTexture_H__
#define _D3DTexture_H__

#include "OgreD3D7Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreRenderTargetListener.h"

namespace Ogre {

    /** Direct3D7-specific texture resource implementation. 
    */
    class D3DTexture : public Texture
    {
    public:
        // Constructor, called from D3DTextureManager
        D3DTexture( const String& name, TextureType texType, IDirect3DDevice7 * lpDirect3dDevice, TextureUsage usage );
		/** Constructor that can be used to manually create a texture and set its parameters. */
		D3DTexture( 
			const String& name, 
			TextureType texType, 
			IDirect3DDevice7 * lpDirect3dDevice, 
			uint width, 
			uint height, 
			uint num_mips,
			PixelFormat format,
			TextureUsage usage );
        virtual ~D3DTexture();

        virtual void load(void);
        virtual void loadImage( const Image &img );
		virtual void loadImage3D( const Image imgs[]);
        virtual void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );        
        virtual void blitImage( const Image& src, 
            const Image::Rect imgRect, const Image::Rect texRect );
		virtual void blitImage3D(const Image src[],
            const Image::Rect imgRect, const Image::Rect texRect );
        virtual void unload(void);
		virtual void copyToTexture( Texture * target );

        /// D3D-specific member that returns the underlying surface.
        LPDIRECTDRAWSURFACE7 getDDSurface(void);

    protected:
        IDirect3DDevice7 * mD3DDevice;       ///< A pointer to the Direct3D device.
        IDirectDrawSurface7 * mSurface;      ///< Surface of the (first) device-specific texture.
		/// cube texture individual face names
		String mCubeFaceNames[6];

		
		void createSurface(void);
		void createSurface2D(void);
		void createSurface3D(void);

		/// internal method, return a D3D pixel format for texture creation
		void _chooseD3DFormat(DDPIXELFORMAT &ddpf);

		/// internal method, construct full cube texture face names from a given string
		void _constructCubeFaceNames(const String name);

		/// internal method, the cube map face name for the spec. face index
		String _getCubeFaceName(unsigned char face)
		{ assert(face < 6); return mCubeFaceNames[face]; }
		/// internal method, return the BPP for the specified format
		static unsigned short _getPFBpp(PixelFormat ogrePF)
		{ return Image::getNumElemBits(ogrePF); }
    };

    class D3D7RenderTexture : public RenderTexture
    {
    public:
        D3D7RenderTexture( const String & name, uint width, uint height )
			: RenderTexture( name, width, height )
        {
            mPrivateTex = TextureManager::getSingleton().createManual( mName + 
                "_PRIVATE##", TEX_TYPE_2D, mWidth, mHeight, 0, PF_R8G8B8, TU_RENDERTARGET );
        }

		virtual ~D3D7RenderTexture()
		{
			mPrivateTex->unload();
			delete mPrivateTex;
		}

		bool requiresTextureFlipping() const { return false; }

        virtual void getCustomAttribute( const String& name, void* pData )
        {
            if( name == "DDBACKBUFFER" )
            {
                LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

                *pSurf = ((D3DTexture*)mPrivateTex)->getDDSurface();
                return;
            }
            else if( name == "DDFRONTBUFFER" )
            {
                LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

                *pSurf = ((D3DTexture*)mPrivateTex)->getDDSurface();
                return;
            }
            else if( name == "HWND" )
            {
                HWND *pHwnd = (HWND*)pData;

                *pHwnd = NULL;
                return;
            }
            else if( name == "isTexture" )
            {
                bool *b = reinterpret_cast< bool * >( pData );
                *b = true;

                return;
            }
        }
        virtual void writeContentsToFile( const String & filename ) {}
        virtual void outputText(int x, int y, const String& text) {}

    protected:
        /// The texture to which rendering takes place.
        Texture * mPrivateTex;

    protected:
        virtual void _copyToTexture()
        {
            // Copy the newly-rendered data to the public texture surface.
            mPrivateTex->copyToTexture( mTexture );
        }
    };
}

#endif
