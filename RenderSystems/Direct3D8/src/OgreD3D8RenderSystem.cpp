#include "OgreD3D8RenderSystem.h"

#include "OgreD3D8Prerequisites.h"

#include "OgreD3D8DriverList.h"
#include "OgreD3D8Driver.h"
#include "OgreD3D8VideoModeList.h"
#include "OgreD3D8VideoMode.h"
#include "OgreD3D8RenderWindow.h"
#include "OgreD3D8TextureManager.h"
#include "OgreD3D8Texture.h"

#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreString.h"
#include "OgreLight.h"
#include "OgreMath.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include <d3dx8.h>
#include <dxerr8.h>
#include "OgreMemoryMacros.h"
#include "dxutil.h"

namespace Ogre {

	D3D8RenderSystem::D3D8RenderSystem( HINSTANCE hInstance )
	{
		OgreGuard( "D3D8RenderSystem::D3D8RenderSystem" );

		LogManager::getSingleton().logMessage( getName() + " created." );

		mpD3D = NULL;
		mpD3DDevice = NULL;

		mDriverList = NULL;
		mActiveD3DDriver = NULL;

		mpRenderBuffer = NULL;
		mRenderBufferSize = 0;
		
		mhInstance = hInstance;

		mStreamsInUse = 0;
		ZeroMemory( &mpXYZBuffer, sizeof(HardwareVertexBuffer) );
		ZeroMemory( &mpNormalBuffer, sizeof(HardwareVertexBuffer) );
		ZeroMemory( &mpDiffuseBuffer, sizeof(HardwareVertexBuffer) );
		ZeroMemory( &mpSpecularBuffer, sizeof(HardwareVertexBuffer) );
        int i;
		for( i=0; i < OGRE_MAX_TEXTURE_LAYERS; i++ )
		{
			for( int j=0; j < 4; j++ )
				ZeroMemory( &mpTextures[i][j], sizeof(HardwareVertexBuffer) );
		}
		ZeroMemory( &mpIndicies, sizeof(HardwareIndexBuffer) );

		ZeroMemory( mCurrentDecl, sizeof(DWORD) * D3D_MAX_DECLSIZE );
		mhCurrentShader = 0;

		for(i=0; i < MAX_LIGHTS; i++ )
			mLights[i] = 0;

		// Create our Direct3D object
		if( NULL == (mpD3D = Direct3DCreate8( D3D_SDK_VERSION )) )
			Except( 999, "Failed to create Direct3D8 object", "D3D8RenderSystem::D3D8RenderSystem" );

		initConfigOptions();

		OgreUnguard();
	}

	D3D8RenderSystem::~D3D8RenderSystem()
	{
		OgreGuard( "D3D8RenderSystem::~D3D8RenderSystem" );

		// Make sure no streams are being used anymore
        int i;
		for( i=0; i < mStreamsInUse; i++ )
			mpD3DDevice->SetStreamSource( i, NULL, 0 );

		// Delete the shader
		if( mhCurrentShader )
			mpD3DDevice->DeleteVertexShader( mhCurrentShader );

		// Release the vertex buffers
		SAFE_RELEASE( mpXYZBuffer.buffer );
		SAFE_RELEASE( mpNormalBuffer.buffer );
		SAFE_RELEASE( mpDiffuseBuffer.buffer );
		SAFE_RELEASE( mpSpecularBuffer.buffer );
		for( i=0; i < OGRE_MAX_TEXTURE_LAYERS; i++ )
		{
			for( int j=0; j < 4; j++ )
				SAFE_RELEASE( mpTextures[i][j].buffer );
		}
		SAFE_RELEASE( mpIndicies.buffer );

		SAFE_DELETE( mDriverList );
		SAFE_DELETE( mTextureManager );
		SAFE_RELEASE( mpD3DDevice );
		SAFE_RELEASE( mpD3D );
		if( mpRenderBuffer )
			free( mpRenderBuffer );
		mpRenderBuffer = NULL;
		mRenderBufferSize = 0;

		LogManager::getSingleton().logMessage( getName() + " destroyed." );
		
		OgreUnguard();
	}

	const String& D3D8RenderSystem::getName() const
	{
		static String strName( "Direct3D8 Rendering Subsystem" );
		return strName;
	}

	void D3D8RenderSystem::initConfigOptions()
	{
		OgreGuard( "D3D8RenderSystem::initConfigOptions" );

		D3D8DriverList* driverList;
		D3D8Driver* driver;

		ConfigOption optDevice;
		ConfigOption optVideoMode;
		ConfigOption optFullScreen;
		ConfigOption optVSync;

		driverList = this->getDirect3DDrivers();

		optDevice.name = "Rendering Device";
		optDevice.currentValue = "";
		optDevice.possibleValues.clear();
		optDevice.immutable = false;

		optVideoMode.name = "Video Mode";
		optVideoMode.currentValue = "";
		optVideoMode.immutable = false;

		optFullScreen.name = "Full Screen";
		optFullScreen.possibleValues.push_back( "Yes" );
		optFullScreen.possibleValues.push_back( "No" );
		optFullScreen.currentValue = "Yes";
		optFullScreen.immutable = false;

		for( unsigned j=0; j < driverList->count(); j++ )
		{
			driver = driverList->item(j);
			optDevice.possibleValues.push_back( driver->DriverDescription() );

			// Make first one default
			if( j==0 )
				optDevice.currentValue = driver->DriverDescription();
		}

		optVSync.name = "VSync";
		optVSync.immutable = false;
		optVSync.possibleValues.push_back( "Yes" );
		optVSync.possibleValues.push_back( "No" );
		optVSync.currentValue = "Yes";

		mOptions[optDevice.name] = optDevice;
		mOptions[optVideoMode.name] = optVideoMode;
		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVSync.name] = optVSync;

		refreshD3DSettings();

		OgreUnguard();
	}

	D3D8DriverList* D3D8RenderSystem::getDirect3DDrivers()
	{
		if( !mDriverList )
			mDriverList = new D3D8DriverList( mpD3D );

		return mDriverList;
	}

	void D3D8RenderSystem::refreshD3DSettings()
	{
		OgreGuard( "D3D8RenderSystem::refreshD3DSettings" );

		ConfigOption* optVideoMode;
		D3D8Driver* driver;
		D3D8VideoMode* videoMode;

		ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );

		if( opt != mOptions.end() )
		{
			for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
			{
				driver = getDirect3DDrivers()->item(j);
				if( driver->DriverDescription() == opt->second.currentValue )
					break;
			}

			opt = mOptions.find( "Video Mode" );
			optVideoMode = &opt->second;

			// get vide modes for this device
			optVideoMode->possibleValues.clear();

			for( unsigned k=0; k < driver->getVideoModeList()->count(); k++ )
			{
				videoMode = driver->getVideoModeList()->item( k );
				optVideoMode->possibleValues.push_back( videoMode->getDescription() );
			}
		}

		OgreUnguard();
	}

	ConfigOptionMap& D3D8RenderSystem::getConfigOptions()
	{
		// return a COPY of the current config options
		return mOptions;
	}

	void D3D8RenderSystem::setConfigOption( const String &name, const String &value )
	{
		OgreGuard( "D3D8RenderSystem::setConfigOption" );

		char msg[128];
		sprintf( msg, "RenderSystem Option: %s = %s", name.c_str(), value.c_str() );
		LogManager::getSingleton().logMessage( msg );

		// Find option
		ConfigOptionMap::iterator it = mOptions.find( name );

		// Update
		if( it != mOptions.end() )
			it->second.currentValue = value;
		else
		{
			sprintf( msg, "Option named '%s' does not exist.", name.c_str() );
			Except( Exception::ERR_INVALIDPARAMS, msg, "D3D8RenderSystem::setConfigOption" );
		}

		// Refresh other options if D3DDriver changed
		if( name == "Rendering Device" )
			refreshD3DSettings();

		if( name == "Full Screen" )
		{
			if( value == "No" )
			{
				// Video mode is not applicable
				it = mOptions.find( "Video Mode" );
				it->second.currentValue = "N/A";
				it->second.immutable = true;
			}
			else
			{
				// Video mode is applicable
				it = mOptions.find( "Video Mode" );
				// default to 640 x 480 @ 16
				it->second.currentValue = "640 x 480 @ 16-bit colour";
				it->second.immutable = false;
			}
		}

		OgreUnguard();
	}

	String D3D8RenderSystem::validateConfigOptions()
	{
		// Check video mode specified in full screen mode
		ConfigOptionMap::iterator it = mOptions.find( "Full Screen" );
		if( it->second.currentValue == "Yes" )
		{
			// Check video moe
			it = mOptions.find( "Video Mode" );
			if( it->second.currentValue == "" )
			{
				return "A video mode must be selected for running in full-screen mode.";
			}
		}

		return "";
	}

	RenderWindow* D3D8RenderSystem::initialise( bool autoCreateWindow )
	{
		RenderWindow* autoWindow = NULL;

		// call superclass method
		RenderSystem::initialise( autoCreateWindow );

		LogManager::getSingleton().logMessage( "****************************************" );
		LogManager::getSingleton().logMessage( "*** Direct3D8 Subsystem Initialising ***" );
		LogManager::getSingleton().logMessage( "****************************************" );

		// Init using current settings
		mActiveD3DDriver = NULL;
		ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
		for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
		{
			if( getDirect3DDrivers()->item(j)->DriverDescription() == opt->second.currentValue )
			{
				mActiveD3DDriver = getDirect3DDrivers()->item(j);
				break;
			}
		}

		if( !mActiveD3DDriver )
			Except( Exception::ERR_INVALIDPARAMS, "Problems finding requested Direct3D driver!", "D3D8RenderSystem::initialise" );

		if( autoCreateWindow )
		{
			bool fullScreen;
			opt = mOptions.find( "Full Screen" );
			if( opt == mOptions.end() )
				Exception( 999, "Can't find full screen option!", "D3D8RenderSystem::initialise" );
			fullScreen = opt->second.currentValue == "Yes";

			D3D8VideoMode* videoMode = NULL;
			int width, height, colourDepth;
			String temp;
			if( fullScreen )
			{
				opt = mOptions.find( "Video Mode" );
				if( opt == mOptions.end() )
					Exception( 999, "Can't find Video Mode option!", "D3D8RenderSystem::initialise" );

				for( unsigned j=0; j < mActiveD3DDriver->getVideoModeList()->count(); j++ )
				{
					temp = mActiveD3DDriver->getVideoModeList()->item(j)->getDescription();
					if( temp == opt->second.currentValue )
					{
						videoMode = mActiveD3DDriver->getVideoModeList()->item(j);
						break;
					}
				}

				if( !videoMode )
					Except( 999, "Can't find requested video mode.", "D3D8RenderSystem::initialise" );

				width = videoMode->getWidth();
				height = videoMode->getHeight();
				colourDepth = videoMode->getColourDepth();
			}
			else
			{
				width = 640;
				height = 480;
				colourDepth = 0;
			}

			autoWindow = this->createRenderWindow( "OGRE Render Window", width, height, colourDepth, fullScreen );
		}

		LogManager::getSingleton().logMessage( "******************************************" );
		LogManager::getSingleton().logMessage( "*** Direct3D8 Subsystem Initialised OK ***" );
		LogManager::getSingleton().logMessage( "******************************************" );

		return autoWindow;
	}

	void D3D8RenderSystem::reinitialise()
	{
		LogManager::getSingleton().logMessage( "D3D8RenderSystem::reinitialise" );
		this->shutdown();
		this->initialise( true );
	}

	void D3D8RenderSystem::shutdown()
	{
		RenderSystem::shutdown();

		// Cleanup
		SAFE_DELETE( mDriverList );

		mActiveD3DDriver = NULL;

        LogManager::getSingleton().logMessage("*-*-* Direct3D 8 Subsystem shutting down cleanly.");
	}

	void D3D8RenderSystem::startRendering()
	{
		OgreGuard( "D3D8RenderSystem::startRendering" );

        BOOL bGotMsg;
        bool isActive;
        bool allWindowsClosed;
        MSG  msg;
        static clock_t lastStartTime;
        static clock_t lastEndTime;

        // Init times to avoid large first-frame time
        lastStartTime = lastEndTime = clock();

        // Call superclass
        RenderSystem::startRendering();

        // Render this window
        PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

        while( /*WM_QUIT != msg.message*/ true  ) // no need to check for WM_QUIT anymore
        {
            // Check all windows to see if any are active / closed
            isActive = false;
            allWindowsClosed = true; // assume all closed unless we find otherwise
            for(RenderWindowMap::iterator i = mRenderWindows.begin(); i != mRenderWindows.end(); )
            {
                if (i->second->isActive())
                    isActive = true;

                if (i->second->isClosed())
                {
                    // Window has been closed, destroy
                    destroyRenderWindow(i->second);

                    RenderWindowMap::iterator j = i; ++j;
                    mRenderWindows.erase(i);
                    i = j;
                }
                else
                {
                    allWindowsClosed = false;
                    ++i;
                }

            }

            // Break out if all windows closed
            if (allWindowsClosed)
            {
                return;
            }


            // Use PeekMessage() if any windows are active, so we can use idle time to
            // render the scene. Else, use GetMessage() to avoid eating CPU time.
            if( isActive )
            {
                bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
            }
            else
            {
                bGotMsg = GetMessage( &msg, NULL, 0U, 0U );
            }
            if( bGotMsg )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            else
            {
                FrameEvent evt;

                // Do frame start event, only if time has advanced
                // Protects us against over-updating when FPS very high
                clock_t fTime = clock(); // Get current time
                if (fTime != lastStartTime || fTime != lastEndTime)
                {
                    evt.timeSinceLastFrame = (float)(fTime - lastStartTime) / CLOCKS_PER_SEC;
                    evt.timeSinceLastEvent = (float)(fTime - lastEndTime) / CLOCKS_PER_SEC;
                    // Stop rendering if frame callback says so
                    if(!fireFrameStarted(evt))
                        return;
                }
                lastStartTime = fTime;

                // Render a frame during idle time (no messages are waiting)
                for(i = mRenderWindows.begin(); i!=mRenderWindows.end(); ++i)
                {
                    if (i->second->isActive())
                    {
                        i->second->update();
                    }
                }

                // Do frame ended event
                fTime = clock(); // Get current time
                if (lastEndTime != fTime || fTime != lastStartTime)
                {
                    evt.timeSinceLastFrame = (float)(fTime - lastEndTime) / CLOCKS_PER_SEC;
                    evt.timeSinceLastEvent = (float)(fTime - lastStartTime) / CLOCKS_PER_SEC;
                    // Stop rendering if frame callback says so
                    if(!fireFrameEnded(evt))
                        return;
                }
                lastEndTime = fTime;

            }
        }

        OgreUnguard();
	}

	void D3D8RenderSystem::setAmbientLight( float r, float g, float b )
	{
		HRESULT hr = mpD3DDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( r, g, b, 1.0f ) );
		if( FAILED( hr ) )
			Except( hr, "Failed to set render stat D3DRS_AMBIENT", "D3D8RenderSystem::setAmbientLight" );
	}

	void D3D8RenderSystem::setShadingType( ShadeOptions so )
	{
		D3DSHADEMODE shadeMode;
		switch( so )
		{
		case SO_FLAT:
			shadeMode = D3DSHADE_FLAT;
			break;
		case SO_GOURAUD:
			shadeMode = D3DSHADE_GOURAUD;
			break;
		case SO_PHONG:
			shadeMode = D3DSHADE_PHONG;
			break;
		}
		HRESULT hr = mpD3DDevice->SetRenderState( D3DRS_SHADEMODE, shadeMode );
		if( FAILED( hr ) )
			Except( hr, "Failed to set render stat D3DRS_SHADEMODE", "D3D8RenderSystem::setShadingType" );
	}

	void D3D8RenderSystem::setTextureFiltering( TextureFilterOptions fo )
	{
		int units = _getNumTextureUnits();
		for( int i=0; i < units; i++ )
		{
			switch( fo )
			{
			// NOTE: Fall through if device doesn't support requested type
			case TFO_TRILINEAR:
				if( mCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR )
				{
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
					LogManager::getSingleton().logMessage( "Texture filtering set to: TRILINEAR" );
					break;
				}

			case TFO_BILINEAR:
				if( mCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR )
				{
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
					mpD3DDevice->SetTextureStageState( i, D3DTSS_MIPFILTER, D3DTEXF_POINT );
					LogManager::getSingleton().logMessage( "Texture filtering set to: BILINEAR" );
					break;
				}

			case TFO_NONE:
				mpD3DDevice->SetTextureStageState( i, D3DTSS_MAGFILTER, D3DTEXF_POINT );
				mpD3DDevice->SetTextureStageState( i, D3DTSS_MINFILTER, D3DTEXF_POINT );
				mpD3DDevice->SetTextureStageState( i, D3DTSS_MIPFILTER, D3DTEXF_NONE );
				LogManager::getSingleton().logMessage( "Texture filtering disabled" );
				break;
			}
		}
	}

	void D3D8RenderSystem::setLightingEnabled( bool enabled )
	{
		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_LIGHTING, enabled ) ) )
			Except( 999, "Failed to set render state D3DRS_LIGHTING", "D3D8RenderSystem::setLightingEnabled" );
	}

	RenderWindow* D3D8RenderSystem::createRenderWindow( const String &name, int width, int height, int colourDepth,
		bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle )
	{
		OgreGuard( "D3D8RenderSystem::createRenderWindow" );

		String msg;

		// Make sure we don't already have a render target of the 
		// sam name as the one supplied
		if( mRenderWindows.find( name ) != mRenderWindows.end() )
		{
			msg = "A render target of the same name '" + name + "' already "
				"exists.  You cannot create a new window with this name.";
			Except( 999, msg, "D3D8RenderSystem::createRenderWindow" );
		}

		RenderWindow* win = new D3D8RenderWindow();
		win->create( name, width, height, colourDepth, fullScreen, 
			left, top, depthBuffer, &mhInstance, mActiveD3DDriver, parentWindowHandle );

		// Add window to render target list
		mRenderWindows[name] = win;

		// If this is the parent window, get the D3D device and create the texture manager
		if( NULL == parentWindowHandle )
		{
			win->getCustomAttribute( "D3DDEVICE", &mpD3DDevice );
			mpD3DDevice->AddRef();
			// get caps
			mpD3D->GetDeviceCaps( mActiveD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, &mCaps );

			// Create the texture manager for use by others
			mTextureManager = new D3D8TextureManager( mpD3DDevice );
		}

		OgreUnguardRet( win );
	}

	void D3D8RenderSystem::destroyRenderWindow( RenderWindow* pWin )
	{
		// Find it to remove from list
		RenderWindowMap::iterator i = mRenderWindows.begin();

		while( i->second != pWin && i != mRenderWindows.end() )
		{
			if( i->second == pWin )
			{
				mRenderWindows.erase(i);
				delete pWin;
				break;
			}
		}
	}

	String D3D8RenderSystem::getErrorDescription( long errorNumber )
	{
		String errMsg = DXGetErrorDescription8( errorNumber );
		return errMsg;
	}

	void D3D8RenderSystem::_addLight( Light* lt )
	{
		// Find first free slot
		int i;
		for( i=0; i < MAX_LIGHTS; i++ )
		{
			if( !mLights[i] )
			{
				mLights[i] = lt;
				break;
			}
		}

		// No space in array?
		if( i == MAX_LIGHTS )
			Except( 999, "No free light slots - cannot add light.", "D3D8RenderSystem::_addLight" );
		
		setD3D8Light( i, lt );
	}

	void D3D8RenderSystem::_removeLight( Light* lt )
	{
		for( int i=0; i < MAX_LIGHTS; i++ )
		{
			if( mLights[i] == lt )
			{
				mpD3DDevice->LightEnable( i, FALSE );
				mLights[0] = NULL;
				break;
			}
		}
	}

	void D3D8RenderSystem::_modifyLight( Light* lt )
	{
		int i;
		for( i=0; i < MAX_LIGHTS; i++ )
		{
			if( mLights[i] == lt )
				break;
		}

		if( i == MAX_LIGHTS )
			Except( Exception::ERR_INVALIDPARAMS, "Cannot locate light to modify.", "D3D8RenderSystem::_modifyLight" );

		setD3D8Light( i, lt );
	}

	void D3D8RenderSystem::_removeAllLights()
	{
		for( int i=0; i < MAX_LIGHTS; i++ )
		{
			if( mLights[i] )
			{
				mpD3DDevice->LightEnable( i, FALSE );
				mLights[i] = NULL;
			}
		}
	}

	void D3D8RenderSystem::setD3D8Light( int index, Light* lt )
	{
		HRESULT hr;

		D3DLIGHT8 d3dLight;
		ZeroMemory( &d3dLight, sizeof(d3dLight) );

		switch( lt->getType() )
		{
		case Light::LT_POINT:
			d3dLight.Type = D3DLIGHT_POINT;
			break;

		case Light::LT_DIRECTIONAL:
			d3dLight.Type = D3DLIGHT_DIRECTIONAL;
			break;

		case Light::LT_SPOTLIGHT:
			d3dLight.Type = D3DLIGHT_SPOT;
			d3dLight.Falloff = lt->getSpotlightFalloff();
			d3dLight.Theta = Math::getSingleton().DegreesToRadians( lt->getSpotlightInnerAngle() );
			d3dLight.Phi = Math::getSingleton().DegreesToRadians( lt->getSpotlightOuterAngle() );
			break;
		}

		ColourValue col;
		col = lt->getDiffuseColour();
		d3dLight.Diffuse = D3DXCOLOR( col.r, col.g, col.b, col.a );

		col = lt->getSpecularColour();
		d3dLight.Specular = D3DXCOLOR( col.r, col.g, col.b, col.a );

		Vector3 vec;
		if( lt->getType() != Light::LT_DIRECTIONAL )
		{
			vec = lt->getDerivedPosition();
			d3dLight.Position = D3DXVECTOR3( vec.x, vec.y, vec.z );
		}
		if( lt->getType() != Light::LT_POINT )
		{
			vec = lt->getDerivedDirection();
			d3dLight.Direction = D3DXVECTOR3( vec.x, vec.y, vec.z );
		}

		d3dLight.Range = lt->getAttenuationRange();
		d3dLight.Attenuation0 = lt->getAttenuationConstant();
		d3dLight.Attenuation1 = lt->getAttenuationLinear();
		d3dLight.Attenuation2 = lt->getAttenuationQuadric();

		if( FAILED( hr = mpD3DDevice->SetLight( index, &d3dLight ) ) )
			Except( hr, "Unable to set light details", "D3D8RenderSystem::setD3D8Light" );

		if( FAILED( hr = mpD3DDevice->LightEnable( index, TRUE ) ) )
			Except( hr, "Unable to enable light", "D3D8RenderSystem::setD3D8Light" );

		lt->_clearModified();
	}

	void D3D8RenderSystem::_pushRenderState()
	{
		Except( Exception::UNIMPLEMENTED_FEATURE, "Sorry, this feature is not yet available.",
			"D3D8RenderSystem::_pushRenderState" );
	}

	void D3D8RenderSystem::_popRenderState()
	{
		Except( Exception::UNIMPLEMENTED_FEATURE, "Sorry, this feature is not yet available.",
			"D3D8RenderSystem::_popRenderState" );
	}

	D3DXMATRIX D3D8RenderSystem::makeD3DXMatrix( const Matrix4& mat )
	{
		// Transpose matrix
		// D3D8 uses row vectors i.e. V*M
		// Ogre, OpenGL and everything else uses column vectors i.e. M*V
		D3DXMATRIX d3dMat;
		d3dMat.m[0][0] = mat[0][0];
		d3dMat.m[0][1] = mat[1][0];
		d3dMat.m[0][2] = mat[2][0];
		d3dMat.m[0][3] = mat[3][0];

		d3dMat.m[1][0] = mat[0][1];
		d3dMat.m[1][1] = mat[1][1];
		d3dMat.m[1][2] = mat[2][1];
		d3dMat.m[1][3] = mat[3][1];

		d3dMat.m[2][0] = mat[0][2];
		d3dMat.m[2][1] = mat[1][2];
		d3dMat.m[2][2] = mat[2][2];
		d3dMat.m[2][3] = mat[3][2];

		d3dMat.m[3][0] = mat[0][3];
		d3dMat.m[3][1] = mat[1][3];
		d3dMat.m[3][2] = mat[2][3];
		d3dMat.m[3][3] = mat[3][3];

		return d3dMat;
	}

	Matrix4 D3D8RenderSystem::convertD3DXMatrix( const D3DXMATRIX& mat )
	{
		Matrix4 ogreMat;
		ogreMat[0][0] = mat.m[0][0];
		ogreMat[1][0] = mat.m[0][1];
		ogreMat[2][0] = mat.m[0][2];
		ogreMat[3][0] = mat.m[0][3];

		ogreMat[0][1] = mat.m[1][0];
		ogreMat[1][1] = mat.m[1][1];
		ogreMat[2][1] = mat.m[1][2];
		ogreMat[3][1] = mat.m[1][3];

		ogreMat[0][2] = mat.m[2][0];
		ogreMat[1][2] = mat.m[2][1];
		ogreMat[2][2] = mat.m[2][2];
		ogreMat[3][2] = mat.m[2][3];

		ogreMat[0][3] = mat.m[3][0];
		ogreMat[1][3] = mat.m[3][1];
		ogreMat[2][3] = mat.m[3][2];
		ogreMat[3][3] = mat.m[3][3];

		return ogreMat;
	}

	void D3D8RenderSystem::_setViewMatrix( const Matrix4 &m )
	{
		D3DXMATRIX d3dmat = makeD3DXMatrix( m );
        d3dmat.m[0][2] = -d3dmat.m[0][2];
        d3dmat.m[1][2] = -d3dmat.m[1][2];
        d3dmat.m[2][2] = -d3dmat.m[2][2];
        d3dmat.m[3][2] = -d3dmat.m[3][2];

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_VIEW, &d3dmat ) ) )
			Except( hr, "Cannot set D3D8 view matrix", "D3D8RenderSystem::_setViewMatrix" );
	}

	void D3D8RenderSystem::_setProjectionMatrix( const Matrix4 &m )
	{
		D3DXMATRIX d3dMat = makeD3DXMatrix( m );

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_PROJECTION, &d3dMat ) ) )
			Except( hr, "Cannot set D3D8 view matrix", "D3D8RenderSystem::_setProjectionMatrix" );
	}

	void D3D8RenderSystem::_setWorldMatrix( const Matrix4 &m )
	{
		D3DXMATRIX d3dMat = makeD3DXMatrix( m );

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->SetTransform( D3DTS_WORLD, &d3dMat ) ) )
			Except( hr, "Cannot set D3D8 view matrix", "D3D8RenderSystem::_setWorldMatrix" );
	}

	void D3D8RenderSystem::_setSurfaceParams( ColourValue &ambient, ColourValue &diffuse,
		ColourValue &specular, ColourValue &emissive, Real shininess )
	{
		// Remember last call
		static ColourValue lastAmbient = ColourValue::Black;
		static ColourValue lastDiffuse = ColourValue::Black;
		static ColourValue lastSpecular = ColourValue::Black;
		static ColourValue lastEmissive = ColourValue::Black;
		static Real lastShininess = 0.0;

		// Only update if changed
		if( ambient != lastAmbient || diffuse != lastDiffuse ||
			specular != lastSpecular || emissive != lastEmissive ||
			shininess != lastShininess )
		{
			D3DMATERIAL8 material;

			material.Diffuse = D3DXCOLOR( diffuse.r, diffuse.g, diffuse.b, diffuse.a );
			material.Ambient = D3DXCOLOR( ambient.r, ambient.g, ambient.b, ambient.a );
			material.Specular = D3DXCOLOR( specular.r, specular.g, specular.b, specular.a );
			material.Emissive = D3DXCOLOR( emissive.r, emissive.g, emissive.b, emissive.a );
			material.Power = shininess;

			HRESULT hr = mpD3DDevice->SetMaterial( &material );
			if( FAILED( hr ) )
				Except( hr, "Error setting D3D material", "D3D8RenderSystem::_setSurfaceParams" );

			// Remember the details
			lastAmbient = ambient;
			lastDiffuse = diffuse;
			lastSpecular = specular;
			lastEmissive = emissive;
			lastShininess = shininess;
		}
	}

	unsigned short D3D8RenderSystem::_getNumTextureUnits()
	{
		return mCaps.MaxSimultaneousTextures;
	}

	void D3D8RenderSystem::_setTexture( int stage, bool enabled, const String &texname )
	{
		HRESULT hr;

		D3D8Texture* dt = (D3D8Texture*)TextureManager::getSingleton().getByName( texname );
		if( enabled && dt )
		{
			hr = mpD3DDevice->SetTexture( stage, dt->getD3DTexture() );
		}
		else
		{
			hr = mpD3DDevice->SetTexture( stage, NULL );
		}

		if( FAILED( hr ) )
			Except( hr, "Unable to set texture in D3D", "D3D8RenderSystem::_setTexture" );
	}

	void D3D8RenderSystem::_setTextureCoordSet( int stage, int index )
	{
		HRESULT hr;
		hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, index );
		if( FAILED( hr ) )
			Except( hr, "Unable to set texture stage state D3DTSS_TEXCOORDINDEX", "D3D8RenderSystem::_setTextureCoordSet" );
	}

	void D3D8RenderSystem::_setTextureCoordCalculation( int stage, TexCoordCalcMethod m )
	{
		HRESULT hr = S_OK;
		D3DXMATRIX matTrans;
		switch( m )
		{
		case TEXCALC_NONE:
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
			D3DXMatrixIdentity( &matTrans );
			hr = mpD3DDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &matTrans );
			break;

		case TEXCALC_ENVIRONMENT_MAP:
			// Sets the flags required for an environment map effect
			hr = mpD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			
			D3DXMatrixIdentity( &matTrans );
			matTrans(0,0) = 0.5f;
			matTrans(3,0) = 0.5f;
			matTrans(1,1) = -0.5f;
			matTrans(3,1) = 0.5f;
			hr = mpD3DDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &matTrans );
			break;

		case TEXCALC_ENVIRONMENT_MAP_PLANAR:
			// Sets the flags required for an environment map effect
			hr = mpD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE );
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
			
			D3DXMatrixIdentity( &matTrans );
			matTrans(0,0) = 0.5f;
			matTrans(3,0) = 0.5f;
			matTrans(1,1) = -0.5f;
			matTrans(3,1) = 0.5f;
			hr = mpD3DDevice->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &matTrans );
			break;
		}

		if( FAILED( hr ) )
			Except( hr, "Error setting texture coord calculation", "D3D8RenderSystem::_setTextureCoordCalculation" );
	}

	void D3D8RenderSystem::_setTextureBlendMode( int stage, const LayerBlendModeEx& bm )
	{
		HRESULT hr = S_OK;
		D3DTEXTURESTAGESTATETYPE tss;
		DWORD value;

		if( bm.blendType == LBT_COLOUR )
			tss = D3DTSS_COLOROP;
		else if( bm.blendType == LBT_ALPHA )
			tss = D3DTSS_ALPHAOP;

		switch( bm.operation )
		{
		case LBX_SOURCE1:
			value = D3DTOP_SELECTARG1;
			break;

		case LBX_SOURCE2:
			value = D3DTOP_SELECTARG2;
			break;

		case LBX_MODULATE:
			value = D3DTOP_MODULATE;
			break;

		case LBX_MODULATE_X2:
			value = D3DTOP_MODULATE2X;
			break;

		case LBX_MODULATE_X4:
			value = D3DTOP_MODULATE4X;
			break;

		case LBX_ADD:
			value = D3DTOP_ADD;
			break;

		case LBX_ADD_SIGNED:
			value = D3DTOP_ADDSIGNED;
			break;

		case LBX_ADD_SMOOTH:
			value = D3DTOP_ADDSMOOTH;
			break;

		case LBX_SUBTRACT:
			value = D3DTOP_SUBTRACT;
			break;

		case LBX_BLEND_DIFFUSE_ALPHA:
			value = D3DTOP_BLENDDIFFUSEALPHA;
			break;

		case LBX_BLEND_TEXTURE_ALPHA:
			value = D3DTOP_BLENDTEXTUREALPHA;
			break;

		case LBX_BLEND_CURRENT_ALPHA:
			value = D3DTOP_BLENDCURRENTALPHA;
			break;

		case LBX_BLEND_MANUAL:
			value = D3DTOP_BLENDFACTORALPHA;
			// set facto in render state
			hr = mpD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, D3DXCOLOR(0.0, 0.0, 0.0,  bm.factor) );
			break;
		}

		// Make call to set operation
		hr = mpD3DDevice->SetTextureStageState( stage, tss, value );

		// Now set up sources
		D3DCOLOR manualD3D;
		if( bm.blendType == LBT_COLOUR )
		{
			tss = D3DTSS_COLORARG1;
			manualD3D = D3DXCOLOR( bm.colourArg1.r, bm.colourArg1.g, bm.colourArg1.b, 1.0 );
		}
		else if( bm.blendType == LBT_ALPHA )
		{
			tss = D3DTSS_ALPHAARG1;
			manualD3D = D3DXCOLOR( 0.0, 0.0, 0.0, bm.alphaArg1 );
		}
		LayerBlendSource bs = bm.source1;
		for( int i=0; i < 2; i++ )
		{
			switch( bs )
			{
			case LBS_CURRENT:
				value = D3DTA_CURRENT;
				break;

			case LBS_TEXTURE:
				value = D3DTA_TEXTURE;
				break;

			case LBS_DIFFUSE:
				value = D3DTA_DIFFUSE;
				break;

			case LBS_SPECULAR:
				value = D3DTA_SPECULAR;
				break;

			case LBS_MANUAL:
				value = D3DTA_TFACTOR;
				// Set factor in render state
				hr = mpD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, manualD3D );
				break;
			}

			// set source
			hr = mpD3DDevice->SetTextureStageState( stage, tss, value );

			// Source2
			bs = bm.source2;
			if( bm.blendType == LBT_COLOUR )
			{
				tss = D3DTSS_COLORARG2;
				manualD3D = D3DXCOLOR( bm.colourArg2.r, bm.colourArg2.g, bm.colourArg2.b, 1.0 );
			}
			else if( bm.blendType == LBT_ALPHA )
			{
				tss = D3DTSS_ALPHAARG2;
				manualD3D = D3DXCOLOR( 0.0, 0.0, 0.0, bm.alphaArg2 );
			}
		}

		if( FAILED( hr ) )
			Except( hr , "Failed to set texture blend mode", "D3D8RenderSystem::_setTextureBlendMode" );
	}

	void D3D8RenderSystem::_setTextureAddressingMode( int stage, Material::TextureLayer::TextureAddressingMode tam )
	{
		HRESULT hr;
		D3DTEXTUREADDRESS d3dType;

		switch( tam )
		{
		case Material::TextureLayer::TAM_WRAP:
			d3dType = D3DTADDRESS_WRAP;
			break;

		case Material::TextureLayer::TAM_MIRROR:
			d3dType = D3DTADDRESS_MIRROR;
			break;

		case Material::TextureLayer::TAM_CLAMP:
			d3dType = D3DTADDRESS_CLAMP;
			break;
		}

		if( FAILED( hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_ADDRESSU, d3dType ) ) )
			Except( hr, "Failed to set texture addressing mode", "D3D8RenderSystem::_setTextureAddressingMode" );
		if( FAILED( hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_ADDRESSV, d3dType ) ) )
			Except( hr, "Failed to set texture addressing mode", "D3D8RenderSystem::_setTextureAddressingMode" );

	}

	void D3D8RenderSystem::_setTextureMatrix( int stage, const Matrix4& xForm )
	{
		HRESULT hr;
		D3DTRANSFORMSTATETYPE d3dType;
		D3DXMATRIX d3dMat, d3dMatCur;

		if( xForm == Matrix4::IDENTITY )
		{
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		}
		else
		{
			// Set 2D input
			// TODO: deal with 3D coordinates when cubic environment mapping supported
			hr = mpD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );

			d3dType = (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage );
			d3dMat = makeD3DXMatrix( xForm );

			hr = mpD3DDevice->SetTransform( d3dType, &d3dMat );

			if( FAILED( hr ) )
				Except( hr, "Unable to set texture transform", "D3D8RenderSystem::_setTextureMatrix" );
		}
	}

	void D3D8RenderSystem::_setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor )
	{
		HRESULT hr;
		D3DBLEND d3dSrcBlend, d3dDestBlend;

		D3DBLEND* pBlend = &d3dSrcBlend;
		SceneBlendFactor ogreBlend = sourceFactor;

		for( int i=0; i<2; i++ )
		{
			switch( ogreBlend )
			{
			case SBF_ONE:
				*pBlend = D3DBLEND_ONE;
				break;

			case SBF_ZERO:
				*pBlend = D3DBLEND_ZERO;
				break;

			case SBF_DEST_COLOUR:
				*pBlend = D3DBLEND_DESTCOLOR;
				break;

			case SBF_SOURCE_COLOUR:
				*pBlend = D3DBLEND_SRCCOLOR;
				break;

			case SBF_ONE_MINUS_DEST_COLOUR:
				*pBlend = D3DBLEND_INVDESTCOLOR;
				break;

			case SBF_ONE_MINUS_SOURCE_COLOUR:
				*pBlend = D3DBLEND_INVSRCCOLOR;
				break;

			case SBF_DEST_ALPHA:
				*pBlend = D3DBLEND_DESTALPHA;
				break;

			case SBF_SOURCE_ALPHA:
				*pBlend = D3DBLEND_SRCALPHA;
				break;

			case SBF_ONE_MINUS_DEST_ALPHA:
				*pBlend = D3DBLEND_INVDESTALPHA;
				break;

			case SBF_ONE_MINUS_SOURCE_ALPHA:
				*pBlend = D3DBLEND_INVSRCALPHA;
				break;
			}

			ogreBlend = destFactor;
			pBlend = &d3dDestBlend;
		}

		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_SRCBLEND, d3dSrcBlend ) ) )
			Except( hr, "Failed to set render state D3DRS_SRCBLEND", "D3D8RenderSystem::_setSceneBlending" );
		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_DESTBLEND, d3dDestBlend ) ) )
			Except( hr, "Failed to set render state D3DRS_DESTBLEND", "D3D8RenderSystem::_setSceneBlending" );
	}

	void D3D8RenderSystem::_setAlphaRejectSettings( CompareFunction func, unsigned char value )
	{
		HRESULT hr;


		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, 
            convertCompareFunction(func)) ) )
			Except( hr, "Failed to set render state D3DRS_ALPHAFUNC", "D3D8RenderSystem::_setAlphaRejectSettings" );
		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_ALPHAREF, value ) ) )
			Except( hr, "Failed to set render state D3DRS_ALPHAREF", "D3D8RenderSystem::_setAlphaRejectSettings" );
	}

	void D3D8RenderSystem::_setViewport( Viewport *vp )
	{
		if( vp != mActiveViewport || vp->_isUpdated() )
		{
			mActiveViewport = vp;

			// ok, it's different, time to set render target (maybe) and viewport params
			D3DVIEWPORT8 d3dvp;
			HRESULT hr;

			// Set render target
			// TODO: maybe only set when required?
			// TODO: deal with rendering to textures
			//RenderTarget* target;
			//target = vp->getTarget();
			// Get DD back buffer
			//LPDIRECTDRAWSURFACE7 pBack;
			//target->getCustomAttribute( "DDBACKBUFFER", &pBack );
			//hr = mpD3DDevice->SetRenderTarget( pBack, 0 );

			// set viewport dimensions
			d3dvp.X = vp->getActualLeft();
			d3dvp.Y = vp->getActualTop();
			d3dvp.Width = vp->getActualWidth();
			d3dvp.Height = vp->getActualHeight();

			// Z-values from 0.0 to 1.0 (TODO: standardise with OpenGL)
			d3dvp.MinZ = 0.0f;
			d3dvp.MaxZ = 1.0f;

			if( FAILED( hr = mpD3DDevice->SetViewport( &d3dvp ) ) )
				Except( hr, "Error setting D3D viewport", "D3D8RenderSystem::_setViewport" );

			vp->_clearUpdatedFlag();

		}
	}

	void D3D8RenderSystem::_beginFrame()
	{
		OgreGuard( "D3D8RenderSystem::_beginFrame" );

		HRESULT hr;

		if( !mActiveViewport )
			Except( 999, "Cannot begin frame - no viewport selected.", "D3D8RenderSystem::_beginFrame" );

		// Clear the viewport if required
		if( mActiveViewport->getClearEveryFrame() )
		{
			if( FAILED( hr = mpD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
				mActiveViewport->getBackgroundColour().getAsLongARGB(), 1.0f, 0 ) ) )
				Except( hr, "Error clearing viewport.", "D3D8RenderSystem::_beginFrame" );
		}

		if( FAILED( hr = mpD3DDevice->BeginScene() ) )
			Except( hr, "Error beginning frame.", "D3D8RenderSystem::_beginFrame" );


		static bool firstTime = true;
		if( firstTime )
		{
			// First-time 
			// setup some defaults

            // Allow alpha blending
            hr = mpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            if (FAILED(hr))
                Except(hr, "Error enabling alpha blending option.",
                    "D3D8RenderSystem::_beginFrame");
            // Allow stencilling
            hr = mpD3DDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
            if (FAILED(hr))
                Except(hr, "Error enabling stencilling.",
                    "D3D8RenderSystem::_beginFrame");
            firstTime = false;
		}

		OgreUnguard();
	}

	inline bool D3D8RenderSystem::compareDecls( DWORD* pDecl1, DWORD* pDecl2, int size )
	{
		for( int i=0; i < size; i++ )
		{
			if( pDecl1[i] != pDecl2[i] )
				return false;
		}
		return true;
	}

	void D3D8RenderSystem::_render( RenderOperation &op )
	{
		OgreGuard( "D3D8RenderSystem::_render" );

		// Okay, lets explain what is going to happen in here.  Currently there are a pile of HardwareBuffers
		// for each of the rendering types (XYZ, Normal, Diffuse, Specular, TextureCoordinates).  Each of
		// these HardwareBuffers currently contains a count as to the number of vertices that it can represent.
		// So if our little RenderOperation has more vertices then we need to resize each of these buffers,
		// if it is going to be used in this render call.  At the same time, we then set the stream source to
		// the vertex buffer contained in the hardware buffer.
		//
		// Finally we have to put the data into these vertex buffers.  If we are using non strided data, then
		// it is as simple as copying each of the RenderOperations buffers data across to the HardwareBuffers
		// vertex buffer for that type e.g. op.pVertices <-> mpXYZBuffer.buffer.  If however it is strided, 
		// we need to step through all of the data and copy those pieces to the relevate hardware buffers.
		//
		// Then all that is left to do is render the lot...sounds simple eh?

		// Update: okay, this just got a little uglier.  Inorder to use multiple streams we need to create
		// our own vertex shader based on a given declaration (which may change all the time).  This 
		// delcaration is just an array of DWORDs, so hopefully it can just be built up along the way (HOPE).
		// One thing to speed this up a bit is to keep a hold of the old declartion is see if it is the same.

		HRESULT hr;
		int i;
		DWORD shaderDecl[D3D_MAX_DECLSIZE];	
		int current = 0;		// the current index into the shader decl
		ZeroMemory( shaderDecl, sizeof(DWORD) * D3D_MAX_DECLSIZE );

		// We may need to erase any streams that are not to be used anymore
		UINT previousStreams = mStreamsInUse;
		mStreamsInUse = 0;
		// To make things easier we will just hold a local copy of the texture buffers here
		LPDIRECT3DVERTEXBUFFER8 pTextureBuffers[OGRE_MAX_TEXTURE_LAYERS];
		int pTextureBufferSizes[OGRE_MAX_TEXTURE_LAYERS];

		// Call super class
		RenderSystem::_render( op );


		
		if( op.vertexOptions == 0 )
		{
			// Must include at least vertex normal, colour or tex coords
			Except( 999, "You must specify at least vertex normals, "
				"vertex colours ot texture co-ordinates to render.", "D3D8RenderSystem::_render" );
		}
		
		// By default we will have XYZ positional data (3 floats)
		if( mpXYZBuffer.count < op.numVertices )
		{
			// Resize the vertex buffer
			SAFE_RELEASE( mpXYZBuffer.buffer );
			if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(D3DXVECTOR3)*op.numVertices, 
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpXYZBuffer.buffer ) ) )
			{
				Except( hr, "Failed to create XYZ vertex buffer", "D3D8RenderSystem::_render" );
			}
			mpXYZBuffer.count = op.numVertices;
		}
		if( mStreamsInUse != 0 )
		{
			OutputDebugStr( "Why?" );
		}
		shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
		shaderDecl[current++] = D3DVSD_REG( D3DVSDE_POSITION, D3DVSDT_FLOAT3 );
		if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpXYZBuffer.buffer, sizeof(D3DXVECTOR3) ) ) )
			Except( hr, "Failed to set stream source for XYZ buffer", "D3D8RenderSystem::_render" );

		// Normal
		if( op.vertexOptions & RenderOperation::VO_NORMALS )
		{
			if( mpNormalBuffer.count < op.numVertices )
			{
				SAFE_RELEASE( mpNormalBuffer.buffer );
				if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(D3DXVECTOR3)*op.numVertices, 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpNormalBuffer.buffer ) ) )
				{
					Except( hr, "Failed to create normals vertex buffer", "D3D8RenderSystem::_render" );
				}
				mpNormalBuffer.count = op.numVertices;
			}
			shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
			shaderDecl[current++] = D3DVSD_REG( D3DVSDE_NORMAL, D3DVSDT_FLOAT3 );
			if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpNormalBuffer.buffer, sizeof(D3DXVECTOR3) ) ) )
				Except( hr, "Failed to set stream source for normal buffer", "D3D8RenderSystem::_render" );
		}

		// Vertex colours
		if( op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS )
		{
			if( mpDiffuseBuffer.count < op.numVertices )
			{
				SAFE_RELEASE( mpDiffuseBuffer.buffer );
				if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(D3DCOLOR)*op.numVertices, 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpDiffuseBuffer.buffer ) ) )
				{
					Except( hr, "Failed to create diffuse vertex buffer", "D3D8RenderSystem::_render" );
				}
				mpDiffuseBuffer.count = op.numVertices;
			}
			shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
			shaderDecl[current++] = D3DVSD_REG( D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR );
			if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpDiffuseBuffer.buffer, sizeof(D3DCOLOR) ) ) )
				Except( hr, "Failed to set stream source for diffuse buffer", "D3D8RenderSystem::_render" );
		}

		if( op.vertexOptions & RenderOperation::VO_SPECULAR_COLOURS )
		{
			if( mpSpecularBuffer.count < op.numVertices )
			{
				SAFE_RELEASE( mpSpecularBuffer.buffer );
				if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(D3DCOLOR)*op.numVertices, 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpSpecularBuffer.buffer ) ) )
				{
					Except( hr, "Failed to create specular vertex buffer", "D3D8RenderSystem::_render" );
				}
				mpSpecularBuffer.count = op.numVertices;
			}
			shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
			shaderDecl[current++] = D3DVSD_REG( D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR );
			if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpSpecularBuffer.buffer, sizeof(D3DCOLOR) ) ) )
				Except( hr, "Failed to set stream source for specular buffer", "D3D8RenderSystem::_render" );
		}

		// Do texture co-ords
		if( op.vertexOptions & RenderOperation::VO_TEXTURE_COORDS )
		{
			for( i=0; i < op.numTextureCoordSets; i++ )
			{
				switch( op.numTextureDimensions[i] )
				{
				case 1:	// One dimensional texture
					if( mpTextures[i][0].count < op.numVertices )
					{
						SAFE_RELEASE( mpTextures[i][0].buffer );
						if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(float)*op.numVertices, 
							D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpTextures[i][0].buffer ) ) )
						{
							Except( hr, "Failed to create texture vertex buffer", "D3D8RenderSystem::_render" );
						}
						mpTextures[i][0].count = op.numVertices;
					}
					shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
					shaderDecl[current++] = D3DVSD_REG( D3DVSDE_TEXCOORD0 + i, D3DVSDT_FLOAT1 );
					if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpTextures[i][0].buffer, sizeof(float) ) ) )
						Except( hr, "Failed to set stream source for specular buffer", "D3D8RenderSystem::_render" );
					pTextureBuffers[i] = mpTextures[i][0].buffer;
					pTextureBufferSizes[i] = sizeof(float);
					break;

				case 2: // Two dimensional texture
					if( mpTextures[i][1].count < op.numVertices )
					{
						SAFE_RELEASE( mpTextures[i][1].buffer );
						if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(float)*2*op.numVertices, 
							D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpTextures[i][1].buffer ) ) )
						{
							Except( hr, "Failed to create texture vertex buffer", "D3D8RenderSystem::_render" );
						}
						mpTextures[i][1].count = op.numVertices;
					}
					shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
					shaderDecl[current++] = D3DVSD_REG( D3DVSDE_TEXCOORD0 + i, D3DVSDT_FLOAT2 );
					if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpTextures[i][1].buffer, sizeof(float)*2 ) ) )
						Except( hr, "Failed to set stream source for specular buffer", "D3D8RenderSystem::_render" );
					pTextureBuffers[i] = mpTextures[i][1].buffer;
					pTextureBufferSizes[i] = sizeof(float)*2;
					break;

				case 3: // Three dimensional texture
					if( mpTextures[i][2].count < op.numVertices )
					{
						SAFE_RELEASE( mpTextures[i][2].buffer );
						if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(float)*3*op.numVertices, 
							D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpTextures[i][2].buffer ) ) )
						{
							Except( hr, "Failed to create texture vertex buffer", "D3D8RenderSystem::_render" );
						}
						mpTextures[i][2].count = op.numVertices;
					}
					shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
					shaderDecl[current++] = D3DVSD_REG( D3DVSDE_TEXCOORD0 + i, D3DVSDT_FLOAT3 );
					if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpTextures[i][2].buffer, sizeof(float)*3 ) ) )
						Except( hr, "Failed to set stream source for specular buffer", "D3D8RenderSystem::_render" );
					pTextureBuffers[i] = mpTextures[i][2].buffer;
					pTextureBufferSizes[i] = sizeof(float)*3;
					break;

				case 4:
					if( mpTextures[i][3].count < op.numVertices )
					{
						SAFE_RELEASE( mpTextures[i][3].buffer );
						if( FAILED( hr = mpD3DDevice->CreateVertexBuffer( sizeof(float)*4*op.numVertices, 
							D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mpTextures[i][3].buffer ) ) )
						{
							Except( hr, "Failed to create texture vertex buffer", "D3D8RenderSystem::_render" );
						}
						mpTextures[i][3].count = op.numVertices;
					}
					shaderDecl[current++] = D3DVSD_STREAM( mStreamsInUse );
					shaderDecl[current++] = D3DVSD_REG( D3DVSDE_TEXCOORD0 + i, D3DVSDT_FLOAT4 );
					if( FAILED( hr = mpD3DDevice->SetStreamSource( mStreamsInUse++, mpTextures[i][4].buffer, sizeof(float)*4 ) ) )
						Except( hr, "Failed to set stream source for specular buffer", "D3D8RenderSystem::_render" );
					pTextureBuffers[i] = mpTextures[i][4].buffer;
					pTextureBufferSizes[i] = sizeof(float)*4;
					break;

				}
			} // for( i...
		}
		shaderDecl[current++] = D3DVSD_END();

		// Clear any previous steam sources
		for( UINT x=mStreamsInUse; x < previousStreams; x++ )
			mpD3DDevice->SetStreamSource( x, NULL, 0 );

		// Determine rendering operation
		D3DPRIMITIVETYPE primType;
		DWORD primCount = 0;
		switch( op.operationType )
		{
        case RenderOperation::OT_POINT_LIST:
            primType = D3DPT_POINTLIST;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices);
            break;

		case RenderOperation::OT_LINE_LIST:
            primType = D3DPT_LINELIST;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices) / 2;
	        break;

		case RenderOperation::OT_LINE_STRIP:
            primType = D3DPT_LINESTRIP;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices) - 1;
            break;

		case RenderOperation::OT_TRIANGLE_LIST:
            primType = D3DPT_TRIANGLELIST;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices) / 3;
            break;

		case RenderOperation::OT_TRIANGLE_STRIP:
            primType = D3DPT_TRIANGLESTRIP;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices) - 2;
            break;

		case RenderOperation::OT_TRIANGLE_FAN:
            primType = D3DPT_TRIANGLEFAN;
			primCount = (op.useIndexes ? op.numIndexes : op.numVertices) - 2;
            break;
		}

		// Okay time to get the information into the buffers.  For the non-strided data it is a simple 
		// matter of copying the data into the relvant buffers, of course using the dynamic update 
		// method.  For strided data, we will need to step through the data and copy the data one piece
		// at a time into the vertex buffers
		if( op.vertexStride )
		{
			// Copy the strided data across to the seperate vertex buffers.
			
			// Ok, we will need some pointers to the data that will be locked, so here they are.
			BYTE *pXYZ, *pNormal, *pDiffuse, *pSpecular, *pTexture[OGRE_MAX_TEXTURE_LAYERS];
			// To make stepping thourgh the data a bit easier (I think) hold a list of pointers to the data
			BYTE *pSrcXYZ, *pSrcNormal, *pSrcDiffuse, *pSrcSpecular, *pSrcTexture[OGRE_MAX_TEXTURE_LAYERS];

			// Lock all the buffers that we will need
			if( FAILED( hr = mpXYZBuffer.buffer->Lock( 0, 0, &pXYZ, D3DLOCK_DISCARD ) ) )
				Except( hr, "Failed to lock XYZ buffer", "D3D8RenderSystem::_render" );
			pSrcXYZ = (BYTE*)op.pVertices;
			if( op.vertexOptions & RenderOperation::VO_NORMALS )
			{
				if( FAILED( hr = mpNormalBuffer.buffer->Lock( 0, 0, &pNormal, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock normal buffer", "D3D8RenderSystem::_render" );
				pSrcNormal = (BYTE*)op.pNormals;
			}
			if( op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS )
			{
				if( FAILED( hr = mpDiffuseBuffer.buffer->Lock( 0, 0, &pDiffuse, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock diffuse buffer", "D3D8RenderSystem::_render" );
				pSrcDiffuse = (BYTE*)op.pDiffuseColour;
			}
			if( op.vertexOptions & RenderOperation::VO_SPECULAR_COLOURS )
			{
				if( FAILED( hr = mpSpecularBuffer.buffer->Lock( 0, 0, &pSpecular, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock specular buffer", "D3D8RenderSystem::_render" );
				pSrcSpecular = (BYTE*)op.pSpecularColour;
			}
			for( i=0; i < op.numTextureCoordSets; i++ )
			{
				if( FAILED( hr = pTextureBuffers[i]->Lock( 0, 0, &(pTexture[i]), D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock texture buffer", "D3D8RenderSystem::_render" );
				pSrcTexture[i] = (BYTE*)op.pTexCoords[i];
			}

			// Copy the data across
			// This actually shouldn't be that difficult....
			for( int n=0; n < op.numVertices; n++ )
			{
				memcpy( pXYZ, pSrcXYZ, sizeof(D3DXVECTOR3) );
				pXYZ += sizeof(D3DXVECTOR3);
				pSrcXYZ += op.vertexStride + (sizeof(float)*3);
				if( op.vertexOptions & RenderOperation::VO_NORMALS )
				{
					memcpy( pNormal, pSrcNormal, sizeof(D3DXVECTOR3) );
					pNormal += sizeof(D3DXVECTOR3);
					pSrcNormal += op.normalStride + (sizeof(float)*3);
				}
				if( op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS )
				{
					memcpy( pDiffuse, pSrcDiffuse, sizeof(D3DCOLOR) );
					pDiffuse += sizeof(D3DCOLOR);
					pSrcDiffuse += op.diffuseStride + sizeof(long);
				}
				if( op.vertexOptions & RenderOperation::VO_SPECULAR_COLOURS )
				{
					memcpy( pDiffuse, pSrcSpecular, sizeof(D3DCOLOR) );
					pDiffuse += sizeof(D3DCOLOR);
					pSrcSpecular += op.specularStride + sizeof(long);
				}
				for( i=0; i < op.numTextureCoordSets; i++ )
				{
					memcpy( pTexture[i], pSrcTexture[i], pTextureBufferSizes[i] );
					pTexture[i] += pTextureBufferSizes[i];
					pSrcTexture[i] += op.texCoordStride[i] + pTextureBufferSizes[i];
				}
			}

			// Unlock all the buffers that were locked
			mpXYZBuffer.buffer->Unlock();
			if( op.vertexOptions & RenderOperation::VO_NORMALS )
				mpNormalBuffer.buffer->Unlock();
			if( op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS )
				mpDiffuseBuffer.buffer->Unlock();
			if( op.vertexOptions & RenderOperation::VO_SPECULAR_COLOURS )
				mpSpecularBuffer.buffer->Unlock();
			for( i=0; i < op.numTextureCoordSets; i++ )
				pTextureBuffers[i]->Unlock();
		}
		else
		{
			// Copy the data to the relevant vertex buffers
			// These data pointers are used so that it is easier to see items in debug mode
			D3DXVECTOR3* pVec3;
			D3DCOLOR* pCol;
			float* pFloat;
			
			if( FAILED( hr = mpXYZBuffer.buffer->Lock( 0, 0, (BYTE**)&pVec3, D3DLOCK_DISCARD ) ) )
				Except( hr, "Failed to lock XYZ buffer", "D3D8RenderSystem::_render" );
			memcpy( pVec3, op.pVertices, sizeof(D3DXVECTOR3)*op.numVertices );
			mpXYZBuffer.buffer->Unlock();

			if( op.vertexOptions & RenderOperation::VO_NORMALS )
			{
				if( FAILED( hr = mpNormalBuffer.buffer->Lock( 0, 0, (BYTE**)&pVec3, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock normal buffer", "D3D8RenderSystem::_render" );
				memcpy( pVec3, op.pNormals, sizeof(D3DXVECTOR3)*op.numVertices );
				mpNormalBuffer.buffer->Unlock();
			}

			if( op.vertexOptions & RenderOperation::VO_DIFFUSE_COLOURS )
			{
				if( FAILED( hr = mpDiffuseBuffer.buffer->Lock( 0, 0, (BYTE**)&pCol, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock diffuse buffer", "D3D8RenderSystem::_render" );
				memcpy( pCol, op.pDiffuseColour, sizeof(D3DCOLOR)*op.numVertices );
				mpDiffuseBuffer.buffer->Unlock();
			}

			if( op.vertexOptions & RenderOperation::VO_SPECULAR_COLOURS )
			{
				if( FAILED( hr = mpSpecularBuffer.buffer->Lock( 0, 0, (BYTE**)&pCol, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock specular buffer", "D3D8RenderSystem::_render" );
				memcpy( pCol, op.pSpecularColour, sizeof(D3DCOLOR)*op.numVertices );
				mpSpecularBuffer.buffer->Unlock();
			}

			for( i=0; i < op.numTextureCoordSets; i++ )
			{
				if( FAILED( hr = pTextureBuffers[i]->Lock( 0, 0, (BYTE**)&pFloat, D3DLOCK_DISCARD ) ) )
					Except( hr, "Failed to lock texture buffer", "D3D8RenderSystem::_render" );
				memcpy( pFloat, op.pTexCoords[i], pTextureBufferSizes[i]*op.numVertices );
				pTextureBuffers[i]->Unlock();
			}
		}

		// Create our vertex shader based on the declaration, specifying NULL for the function signifies
		// that we want to use the fixed funtion pipeline
		if( !compareDecls( mCurrentDecl, shaderDecl, D3D_MAX_DECLSIZE ) )
		{
			if( mhCurrentShader )
				mpD3DDevice->DeleteVertexShader( mhCurrentShader );
			memcpy( mCurrentDecl, shaderDecl, sizeof(DWORD)*D3D_MAX_DECLSIZE );
			if( FAILED( hr = mpD3DDevice->CreateVertexShader( mCurrentDecl, NULL, &mhCurrentShader, 0 ) ) )
				Except( hr, "Failed to create vertex shader", "D3D8RenderSystem::_render" );
		}

		//mpD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		
		hr = mpD3DDevice->SetVertexShader( mhCurrentShader );
		if( op.useIndexes )
		{
			if( mpIndicies.count < op.numIndexes )
			{
				mpD3DDevice->SetIndices( NULL, 0 );
				SAFE_RELEASE( mpIndicies.buffer );
				if( FAILED( hr = mpD3DDevice->CreateIndexBuffer( op.numIndexes*sizeof(unsigned short), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
					D3DPOOL_DEFAULT, &mpIndicies.buffer ) ) )
					Except( hr, "Failed to create index buffer", "D3D8RenderSystem::_render" );
				mpIndicies.count = op.numIndexes;
			}
			
			BYTE* pByte;
			if( FAILED( hr = mpIndicies.buffer->Lock( 0, 0, (BYTE**)&pByte, D3DLOCK_DISCARD ) ) )
				Except( hr, "Failed to lock index buffer", "D3D8RenderSystem::_render" );
			memcpy( pByte, op.pIndexes, op.numIndexes * sizeof(unsigned short) );
			mpIndicies.buffer->Unlock();
			mpD3DDevice->SetIndices( mpIndicies.buffer, 0 );
			hr = mpD3DDevice->DrawIndexedPrimitive( primType, 0, op.numVertices, 0, primCount );
		}
		else
		{
			hr = mpD3DDevice->DrawPrimitive( primType, 0, primCount );
		}

		if( FAILED( hr ) )
		{
			char msg[255];
			sprintf( msg, "Error occured in draw primitive (%d) '%s'", hr, DXGetErrorDescription8(hr) );
			Except( hr, msg, "D3D8RenderSystem::_render" );
		}

		OgreUnguard();
	}

	void D3D8RenderSystem::_endFrame()
	{
		OgreGuard( "D3D8RenderSystem::_endFrame" );

		HRESULT hr;
		if( FAILED( hr = mpD3DDevice->EndScene() ) )
			Except( hr, "Error ending frame", "D3D8RenderSystem::_endFrame" );

		OgreUnguard();
	}

	void D3D8RenderSystem::_setCullingMode( CullingMode mode )
	{
		HRESULT hr;
		DWORD d3dMode;

		switch( mode )
		{
		case CULL_NONE:
			d3dMode = D3DCULL_NONE;
			break;

		case CULL_CLOCKWISE:
			d3dMode = D3DCULL_CW;
			break;

		case CULL_ANTICLOCKWISE:
			d3dMode = D3DCULL_CCW;
			break;
		}

		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_CULLMODE, d3dMode ) ) )
			Except( hr, "Unable to set D3D culling mode", "D3D8RenderSystem::_setCullingMode" );
	}

	void D3D8RenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
	{
		_setDepthBufferCheckEnabled( depthTest );
		_setDepthBufferWriteEnabled( depthWrite );
		_setDepthBufferFunction( depthFunction );
	}

	void D3D8RenderSystem::_setDepthBufferCheckEnabled( bool enabled )
	{
		HRESULT hr;

		if( enabled )
		{
			// Use w-buffer if abialable
			if( mCaps.RasterCaps & D3DPRASTERCAPS_WBUFFER )
				hr = mpD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
			else
				hr = mpD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

		}
		else
		{
			hr = mpD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
		}

		if( FAILED( hr ) )
			Except( hr, "Error setting depth buffer test state", "D3D8RenderSystem::_setDepthBufferCheckEnabled" );
	}

	void D3D8RenderSystem::_setDepthBufferWriteEnabled( bool enabled )
	{
		HRESULT hr;

		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, enabled ) ) )
			Except( hr, "Error setting depth buffer write state", "D3D8RenderSystem::_setDepthBufferWriteEnabled" );
	}

	void D3D8RenderSystem::_setDepthBufferFunction( CompareFunction func )
	{
		HRESULT hr;


		if( FAILED( hr = mpD3DDevice->SetRenderState( D3DRS_ZFUNC, convertCompareFunction(func) ) ) )
			Except( hr, "Error setting depth buffer test function", "D3D8RenderSystem::_setDepthBufferFunction" );
	}

	void D3D8RenderSystem::_setFog( FogMode mode, ColourValue colour, Real densitiy, Real start, Real end )
	{
		HRESULT hr;

		if( mode == FOG_NONE )
		{
			// just disable
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
		}
		else
		{
			// Allow fog
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );

			// Set pixel fog mode
			D3DFOGMODE d3dFogMode;
			switch( mode )
			{
			case FOG_EXP:
				d3dFogMode = D3DFOG_EXP;
				break;

			case FOG_EXP2:
				d3dFogMode = D3DFOG_EXP2;
				break;

			case FOG_LINEAR:
				d3dFogMode = D3DFOG_LINEAR;
				break;
			}

			hr = mpD3DDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_NONE );
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGTABLEMODE, d3dFogMode );

			hr = mpD3DDevice->SetRenderState( D3DRS_FOGCOLOR, colour.getAsLongARGB() );
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGSTART, *((LPDWORD)(&start)) );
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGEND, *((LPDWORD)(&end)) );
			hr = mpD3DDevice->SetRenderState( D3DRS_FOGDENSITY, *((LPDWORD)(&densitiy)) );
		}

		if( FAILED( hr ) )
			Except( hr, "Error setting render state", "D3D8RenderSystem::_setFog" );
	}

	void D3D8RenderSystem::convertColourValue( const ColourValue& colour, unsigned long* pDest )
	{
		*pDest = colour.getAsLongARGB();
	}
    //---------------------------------------------------------------------
    void D3D8RenderSystem::_makeProjectionMatrix(Real fovy, Real aspect, Real nearPlane, Real farPlane, Matrix4& dest)
    {
        Real theta = Math::DegreesToRadians(fovy * 0.5);
        Real w = 1 / Math::Tan(theta * aspect);
        Real h = 1 / Math::Tan(theta);
        Real Q = farPlane / ( farPlane - nearPlane );

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = Q;
        dest[3][2] = 1.0f;
        dest[2][3] = -Q * nearPlane;

    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::_setRasterisationMode(SceneDetailLevel level)
    {
        D3DFILLMODE d3dmode;

        switch(level)
        {
        case SDL_POINTS:
            d3dmode = D3DFILL_POINT;
            break;
        case SDL_WIREFRAME:
            d3dmode = D3DFILL_WIREFRAME;
            break;
        case SDL_SOLID:
            d3dmode = D3DFILL_SOLID;
            break;

        }
        
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_FILLMODE, d3dmode);
        if (FAILED(hr))
        {
            Except(hr, "Error setting rasterisation mode.", 
                "D3D8RenderSystem::setRasterisationMode");
        }


    }
    //---------------------------------------------------------------------
    bool D3D8RenderSystem::hasHardwareStencil(void)
    {

        LPDIRECT3DSURFACE8 pSurf;
        D3DSURFACE_DESC surfDesc;
        mpD3DDevice->GetDepthStencilSurface(&pSurf);
        pSurf->GetDesc(&surfDesc);
        if (surfDesc.Format == D3DFMT_D24X8)
        {
            return true;
        }
        else
        {
            return false;
        }

    }
    //---------------------------------------------------------------------
    ushort D3D8RenderSystem::getStencilBufferBitDepth(void)
    {
        // Actually, it's always 8-bit
        return 8;
    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferFunction(CompareFunction func)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILFUNC, 
            convertCompareFunction(func));
        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer test function.",
                "D3D8RenderSystem::_setStencilBufferFunction");

    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferReferenceValue(ulong refValue)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILREF, refValue);
        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer reference value.",
                "D3D8RenderSystem::setStencilBufferReferenceValue");
    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferMask(ulong mask)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILMASK, mask);
        if (FAILED(hr))
            Except(hr, "Error setting stencil buffer mask.",
                "D3D8RenderSystem::setStencilBufferMask");
    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferFailOperation(StencilOperation op)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILFAIL, 
            convertStencilOp(op));
        if (FAILED(hr))
            Except(hr, "Error setting stencil fail operation.",
                "D3D8RenderSystem::setStencilBufferFailOperation");
    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferDepthFailOperation(StencilOperation op)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILZFAIL, 
            convertStencilOp(op));
        if (FAILED(hr))
            Except(hr, "Error setting stencil depth fail operation.",
                "D3D8RenderSystem::setStencilBufferDepthFailOperation");
    }
    //---------------------------------------------------------------------
    void D3D8RenderSystem::setStencilBufferPassOperation(StencilOperation op)
    {
        HRESULT hr = mpD3DDevice->SetRenderState(D3DRS_STENCILPASS, 
            convertStencilOp(op));
        if (FAILED(hr))
            Except(hr, "Error setting stencil pass operation.",
                "D3D8RenderSystem::setStencilBufferPassOperation");
    }
    //---------------------------------------------------------------------
    D3DCMPFUNC D3D8RenderSystem::convertCompareFunction(CompareFunction func)
    {
        switch(func)
        {
        case CMPF_ALWAYS_FAIL:
            return D3DCMP_NEVER;
        case CMPF_ALWAYS_PASS:
            return D3DCMP_ALWAYS;
        case CMPF_LESS:
            return D3DCMP_LESS;
        case CMPF_LESS_EQUAL:
            return D3DCMP_LESSEQUAL;
        case CMPF_EQUAL:
            return D3DCMP_EQUAL;
        case CMPF_NOT_EQUAL:
            return D3DCMP_NOTEQUAL;
        case CMPF_GREATER_EQUAL:
            return D3DCMP_GREATEREQUAL;
        case CMPF_GREATER:
            return D3DCMP_GREATER;
        };
        // to shut the compiler up
        return D3DCMP_ALWAYS;
    }
    //---------------------------------------------------------------------
    D3DSTENCILOP D3D8RenderSystem::convertStencilOp(StencilOperation op)
    {
        switch(op)
        {
        case SOP_KEEP:
            return D3DSTENCILOP_KEEP;
        case SOP_ZERO:
            return D3DSTENCILOP_ZERO;
        case SOP_REPLACE:
            return D3DSTENCILOP_REPLACE;
        case SOP_INCREMENT:
            return D3DSTENCILOP_INCRSAT;
        case SOP_DECREMENT:
            return D3DSTENCILOP_DECRSAT;
        case SOP_INVERT:
            return D3DSTENCILOP_INVERT;
        };
        // To shut the compiler up
        return D3DSTENCILOP_KEEP;

    }

}