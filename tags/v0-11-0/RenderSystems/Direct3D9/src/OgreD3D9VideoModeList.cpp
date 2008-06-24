#include "OgreD3D9VideoModeList.h"

#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre 
{
	D3D9VideoModeList::D3D9VideoModeList( D3D9Driver* pDriver )
	{
		if( NULL == pDriver )
			Except( Exception::ERR_INVALIDPARAMS, "pDriver parameter is NULL", "D3D9VideoModeList::D3D9VideoModeList" );

		mpDriver = pDriver;
		enumerate();
	}

	D3D9VideoModeList::~D3D9VideoModeList()
	{
		mpDriver = NULL;
		mModeList.clear();
	}

	BOOL D3D9VideoModeList::enumerate()
	{
		UINT iMode;
		LPDIRECT3D9 pD3D = mpDriver->getD3D();
		UINT adapter = mpDriver->getAdapterNumber();

		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_R5G6B5 ); iMode++ )
		{
			D3DDISPLAYMODE displayMode;
			pD3D->EnumAdapterModes( adapter, D3DFMT_R5G6B5, iMode, &displayMode );

			// Filter out low-resolutions
			if( displayMode.Width < 640 || displayMode.Height < 400 )
				continue;

			// Check to see if it is already in the list (to filter out refresh rates)
			BOOL found = FALSE;
			std::vector<D3D9VideoMode>::iterator it;
			for( it = mModeList.begin(); it != mModeList.end(); it++ )
			{
				D3DDISPLAYMODE oldDisp = it->getDisplayMode();
				if( oldDisp.Width == displayMode.Width &&
					oldDisp.Height == displayMode.Height &&
					oldDisp.Format == displayMode.Format )
				{
					// Check refresh rate and favour higher if poss
					if (oldDisp.RefreshRate < displayMode.RefreshRate)
						it->increaseRefreshRate(displayMode.RefreshRate);
					found = TRUE;
					break;
				}
			}

			if( !found )
				mModeList.push_back( D3D9VideoMode( displayMode ) );
		}

		for( iMode=0; iMode < pD3D->GetAdapterModeCount( adapter, D3DFMT_X8R8G8B8 ); iMode++ )
		{
			D3DDISPLAYMODE displayMode;
			pD3D->EnumAdapterModes( adapter, D3DFMT_X8R8G8B8, iMode, &displayMode );

			// Filter out low-resolutions
			if( displayMode.Width < 640 || displayMode.Height < 400 )
				continue;

			// Check to see if it is already in the list (to filter out refresh rates)
			BOOL found = FALSE;
			std::vector<D3D9VideoMode>::iterator it;
			for( it = mModeList.begin(); it != mModeList.end(); it++ )
			{
				D3DDISPLAYMODE oldDisp = it->getDisplayMode();
				if( oldDisp.Width == displayMode.Width &&
					oldDisp.Height == displayMode.Height &&
					oldDisp.Format == displayMode.Format )
				{
					// Check refresh rate and favour higher if poss
					if (oldDisp.RefreshRate < displayMode.RefreshRate)
						it->increaseRefreshRate(displayMode.RefreshRate);
					found = TRUE;
					break;
				}
			}

			if( !found )
				mModeList.push_back( D3D9VideoMode( displayMode ) );
		}

		return TRUE;
	}

	unsigned int D3D9VideoModeList::count()
	{
		return static_cast< unsigned int >( mModeList.size() );
	}

	D3D9VideoMode* D3D9VideoModeList::item( int index )
	{
		std::vector<D3D9VideoMode>::iterator p = mModeList.begin();

		return &p[index];
	}
}