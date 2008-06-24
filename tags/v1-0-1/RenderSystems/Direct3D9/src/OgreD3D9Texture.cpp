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
#include "OgreD3D9Texture.h"
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreNoMemoryMacros.h"
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	/****************************************************************************************/
    D3D9Texture::D3D9Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, IDirect3DDevice9 *pD3DDevice)
        :Texture(creator, name, handle, group, isManual, loader),
        mpDev(pD3DDevice), 
        mpD3D(NULL), 
        mpNormTex(NULL),
        mpCubeTex(NULL),
		mpVolumeTex(NULL),
        mpZBuff(NULL),
        mpTex(NULL),
        mAutoGenMipmaps(false),
		mDynamicTextures(false)
	{
        _initDevice();
	}
	/****************************************************************************************/
	D3D9Texture::~D3D9Texture()
	{
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
	}

	/****************************************************************************************/
	void D3D9Texture::copyToTexture(TexturePtr& target)
	{
        // check if this & target are the same format and type
		// blitting from or to cube textures is not supported yet
		if (target->getUsage() != this->getUsage() ||
			target->getTextureType() != this->getTextureType())
		{
			OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
					"Src. and dest. textures must be of same type and must have the same usage !!!", 
					"D3D9Texture::copyToTexture" );
		}

        HRESULT hr;
        D3D9Texture *other;
		// get the target
		other = reinterpret_cast< D3D9Texture * >( target.get() );
		// target rectangle (whole surface)
		RECT dstRC = {0, 0, other->getWidth(), other->getHeight()};

		// do it plain for normal texture
		if (this->getTextureType() == TEX_TYPE_2D)
		{
			// get our source surface
			IDirect3DSurface9 *pSrcSurface = 0;
			if( FAILED( hr = mpNormTex->GetSurfaceLevel(0, &pSrcSurface) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
			}

			// get our target surface
			IDirect3DSurface9 *pDstSurface = 0;
			IDirect3DTexture9 *pOthTex = other->getNormTexture();
			if( FAILED( hr = pOthTex->GetSurfaceLevel(0, &pDstSurface) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				SAFE_RELEASE(pSrcSurface);
			}

			// do the blit, it's called StretchRect in D3D9 :)
			if( FAILED( hr = mpDev->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
			}

			// release temp. surfaces
			SAFE_RELEASE(pSrcSurface);
			SAFE_RELEASE(pDstSurface);
		}
		else if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
		{
			// get the target cube texture
			IDirect3DCubeTexture9 *pOthTex = other->getCubeTexture();
			// blit to 6 cube faces
			for (size_t face = 0; face < 6; face++)
			{
				// get our source surface
				IDirect3DSurface9 *pSrcSurface = 0;
				if( FAILED( hr = mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pSrcSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				}

				// get our target surface
				IDirect3DSurface9 *pDstSurface = 0;
				if( FAILED( hr = pOthTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					SAFE_RELEASE(pSrcSurface);
				}

				// do the blit, it's called StretchRect in D3D9 :)
				if( FAILED( hr = mpDev->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					OGRE_EXCEPT( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					SAFE_RELEASE(pSrcSurface);
					SAFE_RELEASE(pDstSurface);
				}

				// release temp. surfaces
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
			}
		}
		else
		{
			OGRE_EXCEPT( Exception::UNIMPLEMENTED_FEATURE, 
					"Copy to texture is implemented only for 2D and cube textures !!!", 
					"D3D9Texture::copyToTexture" );
		}
	}
	/****************************************************************************************/
	void D3D9Texture::loadImage( const Image &img )
	{
		// Use OGRE its own codecs
		std::vector<const Image*> imagePtrs;
		imagePtrs.push_back(&img);
		_loadImages( imagePtrs );
	}
	/****************************************************************************************/
	void D3D9Texture::loadImpl()
	{
		// load based on tex.type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			this->_loadNormTex();
			break;
		case TEX_TYPE_3D:
            this->_loadVolumeTex();
            break;
		case TEX_TYPE_CUBE_MAP:
			this->_loadCubeTex();
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::loadImpl" );
		}

	}
	/****************************************************************************************/
	void D3D9Texture::unloadImpl()
	{
		_freeResources();
	}
	/****************************************************************************************/
	void D3D9Texture::_freeResources()
	{
		SAFE_RELEASE(mpTex);
		SAFE_RELEASE(mpNormTex);
		SAFE_RELEASE(mpCubeTex);
		SAFE_RELEASE(mpVolumeTex);
		SAFE_RELEASE(mpZBuff);
	}
	/****************************************************************************************/
	void D3D9Texture::_loadCubeTex()
	{
		assert(this->getTextureType() == TEX_TYPE_CUBE_MAP);

        // DDS load?
		if (StringUtil::endsWith(getName(), ".dds"))
        {
            // find & load resource data
			DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(mName, mGroup);
            MemoryDataStream stream( dstream );

            HRESULT hr = D3DXCreateCubeTextureFromFileInMemory(
                mpDev,
                stream.getPtr(),
                stream.size(),
                &mpCubeTex);

            if (FAILED(hr))
		    {
			    OGRE_EXCEPT( hr, "Can't create cube texture", "D3D9Texture::_loadCubeTex" );
			    this->_freeResources();
		    }

            hr = mpCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);

            if (FAILED(hr))
		    {
			    OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_loadCubeTex" );
			    this->_freeResources();
		    }

            D3DSURFACE_DESC texDesc;
            mpCubeTex->GetLevelDesc(0, &texDesc);
            // set src and dest attributes to the same, we can't know
            _setSrcAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
            _setFinalAttributes(texDesc.Width, texDesc.Height, 1,  _getPF(texDesc.Format));
			mIsLoaded = true;
        }
        else
        {
			// Load from 6 separate files
			// Use OGRE its own codecs
			String baseName, ext;
			std::vector<Image> images(6);
			std::vector<const Image*> imagePtrs;
			static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

			for(size_t i = 0; i < 6; i++)
			{
				size_t pos = mName.find_last_of(".");
				baseName = mName.substr(0, pos);
				ext = mName.substr(pos);
				String fullName = baseName + suffixes[i] + ext;

				images[i].load(fullName, mGroup);
				imagePtrs.push_back(&images[i]);
			}

            _loadImages( imagePtrs );
        }
	}
	/****************************************************************************************/
	void D3D9Texture::_loadVolumeTex()
	{
		assert(this->getTextureType() == TEX_TYPE_3D);
		// DDS load?
		if (StringUtil::endsWith(getName(), ".dds"))
		{
			// find & load resource data
			DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(mName, mGroup);
			MemoryDataStream stream(dstream);
	
			HRESULT hr = D3DXCreateVolumeTextureFromFileInMemory(
				mpDev,
				stream.getPtr(),
				stream.size(),
				&mpVolumeTex);
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Unable to load volume texture from " + this->getName(),
					"D3D9Texture::_loadVolumeTex");
			}
	
			hr = mpVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_loadVolumeTex" );
				this->_freeResources();
			}
	
			D3DVOLUME_DESC texDesc;
			hr = mpVolumeTex->GetLevelDesc(0, &texDesc);
	
			// set src and dest attributes to the same, we can't know
			_setSrcAttributes(texDesc.Width, texDesc.Height, texDesc.Depth, _getPF(texDesc.Format));
			_setFinalAttributes(texDesc.Width, texDesc.Height, texDesc.Depth, _getPF(texDesc.Format));
			mIsLoaded = true;
        }
		else
		{
			Image img;
			img.load(mName, mGroup);
			loadImage(img);
		}
    }
	/****************************************************************************************/
	void D3D9Texture::_loadNormTex()
	{
		assert(this->getTextureType() == TEX_TYPE_1D || this->getTextureType() == TEX_TYPE_2D);
		// DDS load?
		if (StringUtil::endsWith(getName(), ".dds"))
		{
			// Use D3DX
			// find & load resource data
			DataStreamPtr dstream = 
				ResourceGroupManager::getSingleton().openResource(mName, mGroup);
			MemoryDataStream stream(dstream);
	
			HRESULT hr = D3DXCreateTextureFromFileInMemory(
				mpDev,
				stream.getPtr(),
				stream.size(),
				&mpNormTex);
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
					"Unable to load texture from " + this->getName(),
					"D3D9Texture::_loadNormTex");
			}
	
			hr = mpNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
	
			if (FAILED(hr))
			{
				OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_loadNormTex" );
				this->_freeResources();
			}
	
			D3DSURFACE_DESC texDesc;
			mpNormTex->GetLevelDesc(0, &texDesc);
			// set src and dest attributes to the same, we can't know
			_setSrcAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
			_setFinalAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
			mIsLoaded = true;
        }
		else
		{
			Image img;
			img.load(mName, mGroup);
			loadImage(img);
		}
	}
	/****************************************************************************************/
    void D3D9Texture::createInternalResources(void)
	{
		// If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
		// through setWidth and setHeight, which set mWidth and mHeight. Take those values.
		if(mSrcWidth == 0 || mSrcHeight == 0) {
			mSrcWidth = mWidth;
			mSrcHeight = mHeight;
		}
		
		// Determine D3D pool to use
		// Use managed unless we're a render target or user has asked for 
		// a dynamic texture
		if ((mUsage & TU_RENDERTARGET) ||
			(mUsage & TU_DYNAMIC))
		{
			mD3DPool = D3DPOOL_DEFAULT;
		}
		else
		{
			mD3DPool = D3DPOOL_MANAGED;
		}
		// load based on tex.type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			this->_createNormTex();
			break;
		case TEX_TYPE_CUBE_MAP:
			this->_createCubeTex();
			break;
		case TEX_TYPE_3D:
			this->_createVolumeTex();
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::createInternalResources" );
			this->_freeResources();
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createNormTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// Check dynamic textures
		if (mUsage & TU_DYNAMIC)
		{
			if (_canUseDynamicTextures(usage, D3DRTYPE_TEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_DYNAMIC;
				mDynamicTextures = true;
			}
			else
			{
				mDynamicTextures = false;
			}

		}
		// check if mip maps are supported on hardware
		if ((mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP) && mNumMipmaps > 0)
		{
			// use auto.gen. if available, and if desired
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_TEXTURE, d3dPF);
			if ((mUsage & TU_AUTOMIPMAP) && mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// device don't support mip maps, or zero mipmaps requested
			mNumMipmaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateTexture(	
				mpDev,								// device
				mSrcWidth,							// width
				mSrcHeight,							// height
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
				mD3DPool,
				&mpNormTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			OGRE_EXCEPT( hr, "Error creating texture", "D3D9Texture::_createNormTex" );
		}
		
		// set the base texture we'll use in the render system
		hr = mpNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_createNormTex" );
			this->_freeResources();
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = mpNormTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get texture description", "D3D9Texture::_createNormTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, 1, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				OGRE_EXCEPT( hr, "Could not set best autogen filter type", "D3D9Texture::_createNormTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createCubeTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// check if mip map cube textures are supported
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP)
		{
			// use auto.gen. if available
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_CUBETEXTURE, d3dPF);
			if (mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateCubeTexture(	
				mpDev,								// device
				mSrcWidth,							// dimension
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
				mD3DPool,
				&mpCubeTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			OGRE_EXCEPT( hr, "Error creating texture", "D3D9Texture::_createCubeTex" );
		}

		// set the base texture we'll use in the render system
		hr = mpCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_createCubeTex" );
			this->_freeResources();
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = mpCubeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get texture description", "D3D9Texture::_createCubeTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, 1, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				OGRE_EXCEPT( hr, "Could not set best autogen filter type", "D3D9Texture::_createCubeTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createVolumeTex()
	{
		// we must have those defined here
		assert(mWidth > 0 && mHeight > 0 && mDepth>0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// check if mip map cube textures are supported
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP)
		{
			// use auto.gen. if available
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_VOLUMETEXTURE, d3dPF);
			if (mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateVolumeTexture(	
				mpDev,								// device
				mWidth,								// dimension
				mHeight,
				mDepth,
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
				mD3DPool,
				&mpVolumeTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			OGRE_EXCEPT( hr, "Error creating texture", "D3D9Texture::_createVolumeTex" );
		}

		// set the base texture we'll use in the render system
		hr = mpVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get base texture", "D3D9Texture::_createVolumeTex" );
			this->_freeResources();
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DVOLUME_DESC desc;
		hr = mpVolumeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Can't get texture description", "D3D9Texture::_createVolumeTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, desc.Depth, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				OGRE_EXCEPT( hr, "Could not set best autogen filter type", "D3D9Texture::_createCubeTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_initDevice(void)
	{ 
		assert(mpDev);
		HRESULT hr;

		// get device caps
		hr = mpDev->GetDeviceCaps(&mDevCaps);
		if (FAILED(hr))
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't get device description", "D3D9Texture::_setDevice" );

		// get D3D pointer
		hr = mpDev->GetDirect3D(&mpD3D);
		// decrement reference count
		mpD3D->Release();
		if (FAILED(hr))
			OGRE_EXCEPT( hr, "Failed to get D3D9 pointer", "D3D9Texture::_setDevice" );

		// get our device creation parameters
		hr = mpDev->GetCreationParameters(&mDevCreParams);
		if (FAILED(hr))
			OGRE_EXCEPT( hr, "Failed to get D3D9 device creation parameters", "D3D9Texture::_setDevice" );

		// get our back buffer pixel format
		IDirect3DSurface9 *pSrf;
		D3DSURFACE_DESC srfDesc;
		hr = mpDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSrf);
		// decrement reference count
		pSrf->Release();
		if (FAILED(hr))
			OGRE_EXCEPT( hr, "Failed to get D3D9 device pixel format", "D3D9Texture::_setDevice" );

		hr = pSrf->GetDesc(&srfDesc);
		if (FAILED(hr))
		{
			OGRE_EXCEPT( hr, "Failed to get D3D9 device pixel format", "D3D9Texture::_setDevice" );
		}

		mBBPixelFormat = srfDesc.Format;
	}
	/****************************************************************************************/
	void D3D9Texture::_setFinalAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set target texture attributes
		mHeight = height; 
		mWidth = width; 
        mDepth = depth;
		mFormat = format; 

		// Update size (the final size, not including temp space)
		// this is needed in Resource class
		unsigned short bytesPerPixel = mFinalBpp >> 3;
		if( !mHasAlpha && mFinalBpp == 32 )
			bytesPerPixel--;
		mSize = mWidth * mHeight * mDepth * bytesPerPixel 
            * (mTextureType == TEX_TYPE_CUBE_MAP)? 6 : 1;

		// say to the world what we are doing
		if (mWidth != mSrcWidth ||
			mHeight != mSrcHeight)
		{
			LogManager::getSingleton().logMessage("D3D9 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D9 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D9 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
		}
		
		// Create list of subsurfaces for getBuffer()
		_createSurfaceList();
	}
	/****************************************************************************************/
	void D3D9Texture::_setSrcAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set source image attributes
		mSrcWidth = width; 
		mSrcHeight = height; 
		mSrcBpp = PixelUtil::getNumElemBits(format); 
        mHasAlpha = PixelUtil::getFlags(format) & PFF_HASALPHA; 
		// say to the world what we are doing
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 1D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 1D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 2D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 2D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_3D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 3D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 3D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating Cube map RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading Cube Texture, base image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		default:
			OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::_setSrcAttributes" );
			this->_freeResources();
		}
	}
	/****************************************************************************************/
	D3DTEXTUREFILTERTYPE D3D9Texture::_getBestFilterMethod()
	{
		// those MUST be initialized !!!
		assert(mpDev);
		assert(mpD3D);
		assert(mpTex);
		
		DWORD filterCaps = 0;
		// Minification filter is used for mipmap generation
		// Pick the best one supported for this tex type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:		// Same as 2D
		case TEX_TYPE_2D:		filterCaps = mDevCaps.TextureFilterCaps;	break;
		case TEX_TYPE_3D:		filterCaps = mDevCaps.VolumeTextureFilterCaps;	break;
		case TEX_TYPE_CUBE_MAP:	filterCaps = mDevCaps.CubeTextureFilterCaps;	break;
		}
		if(filterCaps & D3DPTFILTERCAPS_MINFGAUSSIANQUAD)
			return D3DTEXF_GAUSSIANQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD)
			return D3DTEXF_PYRAMIDALQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
			return D3DTEXF_ANISOTROPIC;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFLINEAR)
			return D3DTEXF_LINEAR;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPOINT)
			return D3DTEXF_POINT;
		
		return D3DTEXF_POINT;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canUseDynamicTextures(DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat)
	{
		// those MUST be initialized !!!
		assert(mpDev);
		assert(mpD3D);

		// Check for dynamic texture support
		HRESULT hr;
		// check for auto gen. mip maps support
		hr = mpD3D->CheckDeviceFormat(
			mDevCreParams.AdapterOrdinal, 
			mDevCreParams.DeviceType, 
			mBBPixelFormat, 
			srcUsage | D3DUSAGE_DYNAMIC,
			srcType,
			srcFormat);
		if (hr == D3D_OK)
			return true;
		else
			return false;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canAutoGenMipmaps(DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat)
	{
		// those MUST be initialized !!!
		assert(mpDev);
		assert(mpD3D);

		if (mDevCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
		{
			HRESULT hr;
			// check for auto gen. mip maps support
			hr = mpD3D->CheckDeviceFormat(
					mDevCreParams.AdapterOrdinal, 
					mDevCreParams.DeviceType, 
					mBBPixelFormat, 
					srcUsage | D3DUSAGE_AUTOGENMIPMAP,
					srcType,
					srcFormat);
			// this HR could a SUCCES
			// but mip maps will not be generated
			if (hr == D3D_OK)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_chooseD3DFormat()
	{
		// Choose frame buffer pixel format in case PF_UNKNOWN was requested
		if(mFormat == PF_UNKNOWN)
			return mBBPixelFormat;
		// Choose closest supported D3D format as a D3D format
		return _getPF(_getClosestSupportedPF(mFormat));
	}
	/****************************************************************************************/
	void D3D9Texture::_createDepthStencil()
	{
		IDirect3DSurface9 *pSrf;
		D3DSURFACE_DESC srfDesc;
		HRESULT hr;

		/* Get the format of the depth stencil surface of our main render target. */
		hr = mpDev->GetDepthStencilSurface(&pSrf);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT( hr, "Error GetDepthStencilSurface : " + msg, "D3D9Texture::_createDepthStencil" );
			this->_freeResources();
		}
		// get it's description
		hr = pSrf->GetDesc(&srfDesc);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT( hr, "Error GetDesc : " + msg, "D3D9Texture::_createDepthStencil" );
			SAFE_RELEASE(pSrf);
			this->_freeResources();
		}
		// release the temp. surface
		SAFE_RELEASE(pSrf);
		/** Create a depth buffer for our render target, it must be of
		    the same format as other targets !!!
		. */
		hr = mpDev->CreateDepthStencilSurface( 
			mSrcWidth, 
			mSrcHeight, 
			srfDesc.Format, 
			srfDesc.MultiSampleType, 
			NULL, 
			FALSE, 
			&mpZBuff, 
			NULL);
		// cry if failed 
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			OGRE_EXCEPT( hr, "Error CreateDepthStencilSurface : " + msg, "D3D9Texture::_createDepthStencil" );
			this->_freeResources();
		}
	}
	
	/****************************************************************************************/
	// Macro to hide ugly cast
	#define GETLEVEL(face,mip) \
	 	static_cast<D3D9HardwarePixelBuffer*>(mSurfaceList[face*(mNumMipmaps+1)+mip].get())
	void D3D9Texture::_createSurfaceList(bool updateOldList)
	{
		IDirect3DSurface9 *surface;
		IDirect3DVolume9 *volume;
		D3D9HardwarePixelBuffer *buffer;
		size_t mip, face;
		assert(mpTex);
		// Make sure number of mips is right
		mNumMipmaps = mpTex->GetLevelCount() - 1;
		// Need to know static / dynamic
		HardwareBuffer::Usage bufusage;
		if ((mUsage & TU_DYNAMIC) && mDynamicTextures)
		{
			bufusage = HardwareBuffer::HBU_DYNAMIC;
		}
		else
		{
			bufusage = HardwareBuffer::HBU_STATIC;
		}

		if(!updateOldList)
		{
			// Create new list of surfaces
			mSurfaceList.clear();
			for(size_t face=0; face<getNumFaces(); ++face)
			{
				for(size_t mip=0; mip<=mNumMipmaps; ++mip)
				{
					buffer = new D3D9HardwarePixelBuffer(bufusage);
					mSurfaceList.push_back(
						HardwarePixelBufferSharedPtr(buffer)
					);
				}
			}
		}

		switch(getTextureType()) {
		case TEX_TYPE_2D:
		case TEX_TYPE_1D:
			assert(mpNormTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(mip=0; mip<=mNumMipmaps; ++mip)
			{
				if(mpNormTex->GetSurfaceLevel(mip, &surface) != D3D_OK)
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
		 				"D3D9Texture::_createSurfaceList");
				// decrement reference count, the GetSurfaceLevel call increments this
				// this is safe because the texture keeps a reference as well
				surface->Release();

				GETLEVEL(0, mip)->bind(mpDev, surface);
			}
			break;
		case TEX_TYPE_CUBE_MAP:
			assert(mpCubeTex);
			// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(face=0; face<6; ++face)
			{
				for(mip=0; mip<=mNumMipmaps; ++mip)
				{
					if(mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, mip, &surface) != D3D_OK)
						OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get cubemap surface failed",
		 				"D3D9Texture::getBuffer");
					// decrement reference count, the GetSurfaceLevel call increments this
					// this is safe because the texture keeps a reference as well
					surface->Release();
					
					GETLEVEL(face, mip)->bind(mpDev, surface);
				}
			}
			break;
		case TEX_TYPE_3D:
			assert(mpVolumeTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(mip=0; mip<=mNumMipmaps; ++mip)
			{
				if(mpVolumeTex->GetVolumeLevel(mip, &volume) != D3D_OK)
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
		 				"D3D9Texture::getBuffer");	
				// decrement reference count, the GetSurfaceLevel call increments this
				// this is safe because the texture keeps a reference as well
				volume->Release();
						
				GETLEVEL(0, mip)->bind(mpDev, volume);
			}
			break;
		};
		
		// Set autogeneration of mipmaps for each face of the texture, if it is enabled
		if(mNumMipmaps>0 && (mUsage & TU_AUTOMIPMAP)) 
		{
			for(face=0; face<getNumFaces(); ++face)
			{
				GETLEVEL(face, 0)->_setMipmapping(true, mAutoGenMipmaps, mpTex);
			}
		}
	}
	#undef GETLEVEL
	/****************************************************************************************/
	HardwarePixelBufferSharedPtr D3D9Texture::getBuffer(size_t face, size_t mipmap) 
	{
		if(face >= getNumFaces())
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "A three dimensional cube has six faces",
					"D3D9Texture::getBuffer");
		if(mipmap > mNumMipmaps)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"D3D9Texture::getBuffer");
		int idx = face*(mNumMipmaps+1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	
	/****************************************************************************************/
	PixelFormat D3D9Texture::_getPF(D3DFORMAT d3dPF)
	{
		switch(d3dPF)
		{
		case D3DFMT_A8:
			return PF_A8;
		case D3DFMT_L8:
			return PF_L8;
		case D3DFMT_L16:
			return PF_L16;
		case D3DFMT_A4L4:
			return PF_A4L4;
		case D3DFMT_A8L8:
			return PF_BYTE_LA;	// Assume little endian here
		case D3DFMT_R3G3B2:
			return PF_R3G3B2;
		case D3DFMT_A1R5G5B5:
			return PF_A1R5G5B5;
		case D3DFMT_A4R4G4B4:
			return PF_A4R4G4B4;
		case D3DFMT_R5G6B5:
			return PF_R5G6B5;
		case D3DFMT_R8G8B8:
			return PF_R8G8B8;
		case D3DFMT_X8R8G8B8:
			return PF_X8R8G8B8;
		case D3DFMT_A8R8G8B8:
			return PF_A8R8G8B8;
		case D3DFMT_X8B8G8R8:
			return PF_X8B8G8R8;
		case D3DFMT_A8B8G8R8:
			return PF_A8B8G8R8;
		case D3DFMT_A2R10G10B10:
			return PF_A2R10G10B10;
        case D3DFMT_A2B10G10R10:
           return PF_A2B10G10R10;
		case D3DFMT_R16F:
			return PF_FLOAT16_R;
		case D3DFMT_A16B16G16R16F:
			return PF_FLOAT16_RGBA;
		case D3DFMT_R32F:
			return PF_FLOAT32_R;
		case D3DFMT_A32B32G32R32F:
			return PF_FLOAT32_RGBA;
		case D3DFMT_A16B16G16R16:
			return PF_SHORT_RGBA;
		case D3DFMT_DXT1:
			return PF_DXT1;
		case D3DFMT_DXT2:
			return PF_DXT2;
		case D3DFMT_DXT3:
			return PF_DXT3;
		case D3DFMT_DXT4:
			return PF_DXT4;
		case D3DFMT_DXT5:
			return PF_DXT5;
		default:
			return PF_UNKNOWN;
		}
	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_getPF(PixelFormat ogrePF)
	{
		switch(ogrePF)
		{
		case PF_L8:
			return D3DFMT_L8;
		case PF_L16:
			return D3DFMT_L16;
		case PF_A8:
			return D3DFMT_A8;
		case PF_A4L4:
			return D3DFMT_A4L4;
		case PF_BYTE_LA:
			return D3DFMT_A8L8; // Assume little endian here
		case PF_R3G3B2:
			return D3DFMT_R3G3B2;
		case PF_A1R5G5B5:
			return D3DFMT_A1R5G5B5;
		case PF_R5G6B5:
			return D3DFMT_R5G6B5;
		case PF_A4R4G4B4:
			return D3DFMT_A4R4G4B4;
		case PF_R8G8B8:
			return D3DFMT_R8G8B8;
		case PF_A8R8G8B8:
			return D3DFMT_A8R8G8B8;
		case PF_A8B8G8R8:
			return D3DFMT_A8B8G8R8;
		case PF_X8R8G8B8:
			return D3DFMT_X8R8G8B8;
		case PF_X8B8G8R8:
			return D3DFMT_X8B8G8R8;
		case PF_A2B10G10R10:
            return D3DFMT_A2B10G10R10;
		case PF_A2R10G10B10:
			return D3DFMT_A2R10G10B10;
		case PF_FLOAT16_R:
			return D3DFMT_R16F;
		case PF_FLOAT16_RGBA:
			return D3DFMT_A16B16G16R16F;
		case PF_FLOAT32_R:
			return D3DFMT_R32F;
		case PF_FLOAT32_RGBA:
			return D3DFMT_A32B32G32R32F;
		case PF_SHORT_RGBA:
			return D3DFMT_A16B16G16R16;
		case PF_DXT1:
			return D3DFMT_DXT1;
		case PF_DXT2:
			return D3DFMT_DXT2;
		case PF_DXT3:
			return D3DFMT_DXT3;
		case PF_DXT4:
			return D3DFMT_DXT4;
		case PF_DXT5:
			return D3DFMT_DXT5;
		case PF_UNKNOWN:
		default:
			return D3DFMT_UNKNOWN;
		}
	}
	/****************************************************************************************/
	PixelFormat D3D9Texture::_getClosestSupportedPF(PixelFormat ogrePF)
	{
		if (_getPF(ogrePF) != D3DFMT_UNKNOWN)
		{
			return ogrePF;
		}
		switch(ogrePF)
		{
		case PF_B5G6R5:
			return PF_R5G6B5;
		case PF_B8G8R8:
			return PF_R8G8B8;
		case PF_B8G8R8A8:
			return PF_A8R8G8B8;
		case PF_FLOAT16_RGB:
			return PF_FLOAT16_RGBA;
		case PF_FLOAT32_RGB:
			return PF_FLOAT32_RGBA;
		case PF_UNKNOWN:
		default:
			return PF_A8R8G8B8;
		}
	}
	/****************************************************************************************/
	void D3D9Texture::releaseIfDefaultPool(void)
	{
		if(mD3DPool == D3DPOOL_DEFAULT)
		{
			unload();
		}
	}
	/****************************************************************************************/
	void D3D9Texture::recreateIfDefaultPool(LPDIRECT3DDEVICE9 pDev)
	{
		if(mD3DPool == D3DPOOL_DEFAULT)
		{
			// There are 2 possible scenarios here:
			// 1. This is a render target
			// 2. This is a dynamic texture, which probably won't have a loader,
			//    but if it does, we'll call it
			if ((mUsage & TU_RENDERTARGET) || !mLoader)
			{
				// render target, or dynamic texture with no loader
				// just recreate underlying surfaces
				createInternalResources();
			}
			else
			{
				// Dynamic texture with a loader, call it
                load();
			}
		}
		// re-query the surface list anyway
		_createSurfaceList(true);

	}


	/****************************************************************************************/
}