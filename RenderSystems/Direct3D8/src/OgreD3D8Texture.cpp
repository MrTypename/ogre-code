#include "OgreD3D8Texture.h"
#include "OgreException.h"
#include "OgreBitwise.h"
#include "OgreImage.h"
#include "OgreLogManager.h"

#include "png.h"
#include "dxutil.h"
#include "OgreRoot.h"

#include "OgreNoMemoryMacros.h"
#include <d3dx8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

	D3D8Texture::D3D8Texture( String name, LPDIRECT3DDEVICE8 pD3DDevice )
	{
		mName = name;
		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( 999, "Invalid Direct3D Device passed in", "D3D8Texture::D3D8Texture" );
		mpD3DDevice->AddRef();
		enable32Bit( false );
		mpTexture = NULL;
		mpTempTexture = NULL;
	}

	D3D8Texture::~D3D8Texture()
	{
		if( mIsLoaded )
			unload();
		SAFE_RELEASE( mpD3DDevice );
	}

	void D3D8Texture::load()
	{
		Image img;
		img.load( mName );

		loadImage( img );

		// Just for fun, save this texture to a file, so we can see that it worked
		//HRESULT hr;
		//LPDIRECT3DSURFACE8 pSurface;
		//char filename[128];
		//for( unsigned i=0 ; i <= mNumMipMaps; i++ )
		//{
		//	hr = mpTexture->GetSurfaceLevel( i, &pSurface );
		//	if( SUCCEEDED( hr ) )
		//	{
		//		sprintf( filename, "%s%d.bmp", mName.c_str(), i );
		//		D3DXSaveSurfaceToFile( filename, D3DXIFF_BMP, pSurface, NULL, NULL );
		//	}
		//}

	}

	void D3D8Texture::loadImage( const Image& img )
	{
		if( mIsLoaded )
			unload();

		LogManager::getSingleton().logMessage( LML_TRIVIAL, "D3D8Texture: Loading %s iwth %d mipmaps from Image.",
			mName.c_str(), mNumMipMaps );

		Image::PixelFormat pf = img.getFormat();
		mSrcBpp = Image::PF2BPP( pf );

		if( pf & Image::FMT_ALPHA )
			mHasAlpha = true;
		else
			mHasAlpha = false;

		mSrcWidth = img.getWidth();
		mSrcHeight = img.getHeight();

		uchar *pTempData = new uchar[ img.getSize() ];
		memcpy( pTempData, img.getData(), img.getSize() );

		createTexture();
		applyGamma( pTempData, img.getSize(), mSrcBpp );
		copyMemoryToTexture( pTempData );

		SAFE_DELETE_ARRAY( pTempData );

		// Now that the image has been loaded, check the dimensions that were created
		D3DSURFACE_DESC desc;
		mpTexture->GetLevelDesc( 0, &desc );
		if( desc.Width != mSrcWidth || desc.Height != mSrcHeight )
		{
			char msg[255];
			sprintf( msg, "Surface dimensions for requested texture '%s' have been altered by the renderer.", mName.c_str() );
			LogManager::getSingleton().logMessage( msg );

			sprintf( msg, "  Requested: %dx%d  Actual: %dx%d", mSrcWidth, mSrcHeight, desc.Width, desc.Height );
			LogManager::getSingleton().logMessage(msg);

			LogManager::getSingleton().logMessage( "  Likely cause is that requested dimensions are not a power of 2, "
				"or device requires square textures." );

		}
		mWidth = desc.Width;
		mHeight = desc.Height;
	}

	void D3D8Texture::unload()
	{
		SAFE_RELEASE( mpTexture );
		SAFE_RELEASE( mpTempTexture );
	}

	void D3D8Texture::blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY )
	{
		LogManager::getSingleton().logMessage( "D3D8Texture::blitToTexture called but not implemented" );
	}

	void D3D8Texture::createTexture()
	{
		D3DFORMAT format = D3DFMT_A8R8G8B8;
		if( mFinalBpp > 16 && mHasAlpha )
			format = D3DFMT_A8R8G8B8;
		else if( mFinalBpp > 16 && !mHasAlpha )
			format = D3DFMT_R8G8B8;
		else if( mFinalBpp == 16 && mHasAlpha )
			format = D3DFMT_A4R4G4B4;
		else if( mFinalBpp == 16 && !mHasAlpha )
			format = D3DFMT_R5G6B5;

		HRESULT hr;
		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		if( FAILED( hr = D3DXCreateTexture( mpD3DDevice, mSrcWidth, mSrcHeight, mNumMipMaps, 0, format, 
			D3DPOOL_SYSTEMMEM, &mpTempTexture ) ) )
		{
			Except( hr, "Error creating temp Direct3D texture", "D3DXTexture::createTexture" );
		}

		if( FAILED( hr = D3DXCreateTexture( mpD3DDevice, mSrcWidth, mSrcHeight, mNumMipMaps, 0, format, 
			D3DPOOL_DEFAULT, &mpTexture ) ) )
		{
			Except( hr, "Error creating Direct3D texture", "D3D8Texture::createTexture" );
		}

		// Check the actual dimensions vs requested
		D3DSURFACE_DESC desc;
		if( FAILED( hr = mpTexture->GetLevelDesc( 0, &desc ) ) )
		{
			Except( hr, "Failed to get level 0 surface description", "D3D8Texture::createTexture" );
		}
		if( desc.Width != mSrcWidth || desc.Height != mSrcHeight )
		{
			char msg[255];
            sprintf(msg, "Surface dimensions for requested texture %s have been altered by "
                "the renderer.", mName.c_str());
            LogManager::getSingleton().logMessage(msg);

            sprintf(msg,"   Requested: %dx%d Actual: %dx%d",
				mSrcWidth, mSrcHeight, desc.Width, desc.Height);
            LogManager::getSingleton().logMessage(msg);

            LogManager::getSingleton().logMessage("   Likely cause is that requested dimensions are not a power of 2, "
                "or device requires square textures.");
		}
		mWidth = desc.Width;
		mHeight = desc.Height;
	}

	void D3D8Texture::getColourMasks( D3DFORMAT format, DWORD* pdwRed, DWORD* pdwGreen, DWORD* pdwBlue, DWORD* pdwAlpha, DWORD* pdwRGBBitCount )
	{
		switch( format )
		{
		case D3DFMT_R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 24;
			break;
		case D3DFMT_A8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0xFF000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_X8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_R5G6B5:
			*pdwRed = 0x0000F800; *pdwGreen = 0x000007E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_X1R5G5B5:
			*pdwRed = 0x00007C00; *pdwGreen = 0x000003E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A1R5G5B5:
			*pdwRed = 0x00007C00; *pdwGreen = 0x000003E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00008000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A4R4G4B4:
			*pdwRed = 0x00000F00; *pdwGreen = 0x000000F0; *pdwBlue = 0x000000F0; *pdwAlpha = 0x0000F000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A8:
			*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x000000FF;
			*pdwRGBBitCount = 8;
			break;
		case D3DFMT_R3G3B2:
			*pdwRed = 0x000000E0; *pdwGreen = 0x0000001C; *pdwBlue = 0x00000003; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 8;
			break;
		case D3DFMT_A8R3G3B2:
			*pdwRed = 0x000000E0; *pdwGreen = 0x0000001C; *pdwBlue = 0x00000003; *pdwAlpha = 0x0000FF00;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_X4R4G4B4:
			*pdwRed = 0x00000F00; *pdwGreen = 0x000000F0; *pdwBlue = 0x0000000F; *pdwAlpha = 0x0000F000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A2B10G10R10:
			*pdwRed = 0x3FF00000; *pdwGreen = 0x000FFC00; *pdwBlue = 0x000003FF; *pdwAlpha = 0xC0000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_G16R16:
			*pdwRed = 0xFFFF0000; *pdwGreen = 0x0000FFFF; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 32;
			break;
		// Not implemented
		//case D3DFMT_A8P8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_P8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_L8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_A8L8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 16;
		//	break;
		//case D3DFMT_A4L4 :
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;

		default:
			*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 0;
			break;
		}
	}

	void D3D8Texture::copyMemoryToTexture( unsigned char* pBuffer )
	{
		HRESULT hr;
		// Create a tempoary texture at the same format as final texture
		D3DSURFACE_DESC desc;
		if( FAILED( hr = mpTempTexture->GetLevelDesc( 0, &desc ) ) )
			Except( hr, "Failed to get texture level 0 descripition", "D3D8Texture::copyMemoryToTexture" );

		// Create a temporary surface for the texture
		LPDIRECT3DSURFACE8 pTempSurface;
		//if( FAILED( hr = mpTexture->GetSurfaceLevel( 0, &pTempSurface ) ) )
		//	Except( hr, "Failed to get level 0 surface of texture", "D3D8Texture::copyMemoryToTexture" );
		if( FAILED( hr = mpD3DDevice->CreateImageSurface( mSrcWidth, mSrcHeight, desc.Format, &pTempSurface ) ) )
			Except( hr, "Failed to create tempoarary surface", "D3D8Texture::copyMemoryToTexture" );

		// Copy the image from the buffer to the temporary surface.
		// We have to do our own colour conversion here since we don't 
		// have a DC to do it for us

		// NOTE - only non-palettised surfaces supported for now
		BYTE *pSurf8;
		BYTE *pBuf8;
		DWORD data32;
		DWORD out32;
		DWORD temp32;
		DWORD srcPattern;
		unsigned iRow, iCol;

		// NOTE - dimensions of surface may differ from buffer
		// dimensions (e.g. power of 2 or square adjustments)

		// Lock surface
		pTempSurface->GetDesc( &desc );
		DWORD aMask, rMask, gMask, bMask, rgbBitCount;
		getColourMasks( desc.Format, &rMask, &gMask, &bMask, &aMask, &rgbBitCount );

		D3DLOCKED_RECT rect;
		if( FAILED( hr = pTempSurface->LockRect( &rect, NULL, D3DLOCK_NOSYSLOCK ) ) )
		{
			String msg = DXGetErrorDescription8(hr);
			Except( hr, "Unable to lock temp texture surface", "D3D8Texture::copyMemoryToTexture" );
		}
		else
			pBuf8 = (BYTE*)pBuffer;

		for( iRow = 0; iRow < mSrcHeight; iRow++ )
		{
			// NOTE: Direct3D used texture coordinates where (0,0) is the TOP LEFT corner of texture
			// Everybody else (OpenGL, 3D Studio, etc) uses (0,0) as BOTTOM LEFT corner
			// So whilst we load, flip the texture in the Y-axis to compensate
			pSurf8 = (BYTE*)rect.pBits + ((mSrcHeight-iRow-1) * rect.Pitch);

			for( iCol = 0; iCol < mSrcWidth; iCol++ )
			{
				// Read RGBA values from buffer
				data32 = 0;
				if( mSrcBpp >= 24 )
				{
					// Data in buffer is in RGB(A) format
					// Read into a 32-bit structure
					// Uses bytes for 24-bit compatibility
					// NOTE: buffer is big-endian
					data32 |= *pBuf8++ << 24;
					data32 |= *pBuf8++ << 16;
					data32 |= *pBuf8++ << 8;
				}
				else if( mSrcBpp == 8 ) // Greyscale, not palettised (palettised NOT supported)
				{
					// Duplicate same greyscale value across R,G,B
					data32 |= *pBuf8 << 24;
					data32 |= *pBuf8 << 16;
					data32 |= *pBuf8++ << 8;
				}

				if( mHasAlpha )
					data32 |= *pBuf8++;
				else
					data32 |= 0xFF;	// Set opaque

				// Write RGBA values to surface
				// Data in surface can be in varying formats

				// Use bit concersion function
				// NOTE: we use a 32-bit value to manipulate
				// Will be reduced to size later
				out32 = 0;

				// Red
				srcPattern = 0xFF000000;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &rMask, 32 );
				out32 |= temp32;

				// Green
				srcPattern = 0x00FF0000;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &gMask, 32 );
				out32 |= temp32;

				// Blue
				srcPattern = 0x0000FF00;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &bMask, 32 );
				out32 |= temp32;

				// Alpha
				if( aMask > 0 )
				{
					srcPattern = 0x000000FF;
					Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &aMask, 32 );
					out32 |= temp32;
				}

				// Assign results to surface pixel
				// Write up to 4 bytes
				// Surfaces are little-endian (low byte first)
				if( rgbBitCount >= 8 )
					*pSurf8++ = (BYTE)out32;
				if( rgbBitCount >= 16 )
					*pSurf8++ = (BYTE)(out32 >> 8);
				if( rgbBitCount >= 24 )
					*pSurf8++ = (BYTE)(out32 >> 16);
				if( rgbBitCount >= 32 )
					*pSurf8++ = (BYTE)(out32 >> 24);
			} // for( iCol...
		} // for( iRow...

		pTempSurface->UnlockRect();

		// Now we need to copy the surface to the texture level 0 surface
		LPDIRECT3DSURFACE8 pDestSurface;
		if( FAILED( hr = mpTempTexture->GetSurfaceLevel( 0, &pDestSurface ) ) )
			Except( hr, "Failed to get level 0 surface for texture", "D3D8Texture::copyMemoryToTexture" );
		if( FAILED( hr = mpD3DDevice->CopyRects( pTempSurface, NULL, 0, pDestSurface, NULL ) ) )
			Except( hr, "Failed to copy temp surface to texture", "D3D8Texture::copyMemoryToTexture" );
		if( FAILED( hr = mpD3DDevice->UpdateTexture( mpTempTexture, mpTexture ) ) )
			Except( hr, "Failed to update texture", "D3D8Texture::copyMemoryToTexture" );

		// Finally we will use D3DX to create the mip map levels
		if( FAILED( hr = D3DXFilterTexture( mpTexture, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
			Except( hr, "Failed to filter texture (generate mip maps)", "D3D8Texture::copyMemoryToTexture" );

		SAFE_RELEASE( pTempSurface );
		SAFE_RELEASE( pDestSurface );
		SAFE_RELEASE( mpTempTexture );
	}
}