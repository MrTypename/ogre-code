/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#include "OgreD3D9Mappings.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(ShadeOptions so)
	{
		switch( so )
		{
		case SO_FLAT:
			return D3DSHADE_FLAT;
		case SO_GOURAUD:
			return D3DSHADE_GOURAUD;
		case SO_PHONG:
			return D3DSHADE_PHONG;
		}
		return 0;
	}
	//---------------------------------------------------------------------
	D3DLIGHTTYPE D3D9Mappings::get(Ogre::Light::LightTypes lightType)
	{
		switch( lightType )
		{
		case Light::LT_POINT:
			return D3DLIGHT_POINT;
		case Light::LT_DIRECTIONAL:
			return D3DLIGHT_DIRECTIONAL;
		case Light::LT_SPOTLIGHT:
			return D3DLIGHT_SPOT;
		}
		return D3DLIGHT_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(TexCoordCalcMethod m, const D3DCAPS9& caps)
	{
		switch( m )
		{
		case TEXCALC_NONE:
			return D3DTSS_TCI_PASSTHRU;
		case TEXCALC_ENVIRONMENT_MAP_REFLECTION:
			return D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
		case TEXCALC_ENVIRONMENT_MAP_PLANAR:
			if (caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
			{
				// Use sphere map if available
				return D3DTSS_TCI_SPHEREMAP;
			}
			else
			{
				// If not, fall back on camera space reflection vector which isn't as good
                return D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
			}
		case TEXCALC_ENVIRONMENT_MAP_NORMAL:
			return D3DTSS_TCI_CAMERASPACENORMAL;
		case TEXCALC_ENVIRONMENT_MAP:
			if (caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
			{
				// Use sphere map if available
				return D3DTSS_TCI_SPHEREMAP;
			}
			else
			{
				// If not, fall back on camera space normal which isn't as good
				return D3DTSS_TCI_CAMERASPACENORMAL;
			}
		}
		return 0;
	}
	//---------------------------------------------------------------------
	D3DTEXTUREADDRESS D3D9Mappings::get(Material::TextureLayer::TextureAddressingMode tam)
	{
		switch( tam )
		{
		case Material::TextureLayer::TAM_WRAP:
			return D3DTADDRESS_WRAP;
		case Material::TextureLayer::TAM_MIRROR:
			return D3DTADDRESS_MIRROR;
		case Material::TextureLayer::TAM_CLAMP:
			return D3DTADDRESS_CLAMP;
		}
		return D3DTADDRESS_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	D3DTEXTURESTAGESTATETYPE D3D9Mappings::get(LayerBlendType lbt)
	{
		switch( lbt )
		{
		case LBT_COLOUR:
			return D3DTSS_COLOROP;
		case LBT_ALPHA:
			return D3DTSS_ALPHAOP;
		}
		return  D3DTSS_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(LayerBlendSource lbs)
	{
		switch( lbs )
		{
		case LBS_CURRENT:
			return D3DTA_CURRENT;
		case LBS_TEXTURE:
			return D3DTA_TEXTURE;
		case LBS_DIFFUSE:
			return D3DTA_DIFFUSE;
		case LBS_SPECULAR:
			return D3DTA_SPECULAR;
		case LBS_MANUAL:
			return D3DTA_TFACTOR;
		}
		return 0;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(LayerBlendOperationEx lbo, D3DCAPS9 devCaps)
	{
		switch( lbo )
		{
		case LBX_SOURCE1:
			return D3DTOP_SELECTARG1;
		case LBX_SOURCE2:
			return D3DTOP_SELECTARG2;
		case LBX_MODULATE:
			return D3DTOP_MODULATE;
		case LBX_MODULATE_X2:
			return D3DTOP_MODULATE2X;
		case LBX_MODULATE_X4:
			return D3DTOP_MODULATE4X;
		case LBX_ADD:
			return D3DTOP_ADD;
		case LBX_ADD_SIGNED:
			return D3DTOP_ADDSIGNED;
		case LBX_ADD_SMOOTH:
			return D3DTOP_ADDSMOOTH;
		case LBX_SUBTRACT:
			return D3DTOP_SUBTRACT;
		case LBX_BLEND_DIFFUSE_ALPHA:
			return D3DTOP_BLENDDIFFUSEALPHA;
		case LBX_BLEND_TEXTURE_ALPHA:
			return D3DTOP_BLENDTEXTUREALPHA;
		case LBX_BLEND_CURRENT_ALPHA:
			return D3DTOP_BLENDCURRENTALPHA;
		case LBX_BLEND_MANUAL:
			return D3DTOP_BLENDFACTORALPHA;
		case LBX_DOTPRODUCT:
			if (devCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
				return D3DTOP_DOTPRODUCT3;
			else
				return D3DTOP_MODULATE;
		}
		return 0;
	}
	//---------------------------------------------------------------------
	D3DBLEND D3D9Mappings::get(SceneBlendFactor sbf)
	{
		switch( sbf )
		{
		case SBF_ONE:
			return D3DBLEND_ONE;
		case SBF_ZERO:
			return D3DBLEND_ZERO;
		case SBF_DEST_COLOUR:
			return D3DBLEND_DESTCOLOR;
		case SBF_SOURCE_COLOUR:
			return D3DBLEND_SRCCOLOR;
		case SBF_ONE_MINUS_DEST_COLOUR:
			return D3DBLEND_INVDESTCOLOR;
		case SBF_ONE_MINUS_SOURCE_COLOUR:
			return D3DBLEND_INVSRCCOLOR;
		case SBF_DEST_ALPHA:
			return D3DBLEND_DESTALPHA;
		case SBF_SOURCE_ALPHA:
			return D3DBLEND_SRCALPHA;
		case SBF_ONE_MINUS_DEST_ALPHA:
			return D3DBLEND_INVDESTALPHA;
		case SBF_ONE_MINUS_SOURCE_ALPHA:
			return D3DBLEND_INVSRCALPHA;
		}
		return D3DBLEND_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(CompareFunction cf)
	{
		switch( cf )
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
		return 0;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(CullingMode cm, bool flip)
	{
		switch( cm )
		{
		case CULL_NONE:
			return D3DCULL_NONE;
		case CULL_CLOCKWISE:
			if( flip )
				return D3DCULL_CCW;
			else
				return D3DCULL_CW;
		case CULL_ANTICLOCKWISE:
			if( flip )
				return D3DCULL_CW;
			else
				return D3DCULL_CCW;
		}
		return 0;
	}
	//---------------------------------------------------------------------
	D3DFOGMODE D3D9Mappings::get(FogMode fm)
	{
		switch( fm )
		{
		case FOG_EXP:
			return D3DFOG_EXP;
		case FOG_EXP2:
			return D3DFOG_EXP2;
		case FOG_LINEAR:
			return D3DFOG_LINEAR;
		}
		return D3DFOG_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	D3DFILLMODE D3D9Mappings::get(SceneDetailLevel level)
	{
		switch(level)
		{
		case SDL_POINTS:
			return D3DFILL_POINT;
		case SDL_WIREFRAME:
			return D3DFILL_WIREFRAME;
		case SDL_SOLID:
			return D3DFILL_SOLID;
		}
		return D3DFILL_FORCE_DWORD;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(StencilOperation op)
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
		}
		return 0;
	}
	//---------------------------------------------------------------------
	DWORD D3D9Mappings::get(TextureFilterOptions tfo, D3DCAPS9 devCaps, eD3DTexType texType, eD3DFilterUsage usage)
	{
		DWORD capsType;
		DWORD anisoUsage;
		DWORD linearUsage;

		switch( usage )
		{
		case D3D_FUSAGE_MIN:
			anisoUsage = D3DPTFILTERCAPS_MINFANISOTROPIC;
			linearUsage = D3DPTFILTERCAPS_MINFLINEAR;
			break;
		case D3D_FUSAGE_MAG:
			anisoUsage = D3DPTFILTERCAPS_MAGFANISOTROPIC;
			linearUsage = D3DPTFILTERCAPS_MAGFLINEAR;
			break;
		case D3D_FUSAGE_MIP:
			linearUsage = D3DPTFILTERCAPS_MIPFLINEAR;
			break;
		}

		switch( texType )
		{
		case D3D_TEX_TYPE_NORMAL:
			capsType = devCaps.TextureFilterCaps;
			break;
		case D3D_TEX_TYPE_CUBE:
			capsType = devCaps.CubeTextureFilterCaps;
			break;
		case D3D_TEX_TYPE_VOLUME:
			capsType = devCaps.VolumeTextureFilterCaps;
			break;
		}

		switch( tfo )
		{
		// NOTE: Fall through if device doesn't support requested type
		case TFO_ANISOTROPIC:
			// mip filter anisotropic don't exist
			// fall to linear...
			if (usage != D3D_FUSAGE_MIP)
			{
				if( capsType & anisoUsage )
					return D3DTEXF_ANISOTROPIC;
			}
		case TFO_TRILINEAR:
		case TFO_BILINEAR:
			if( capsType & linearUsage )
				return D3DTEXF_LINEAR;
		case TFO_NONE:
            if (usage == D3D_FUSAGE_MIP)
            {
			    return D3DTEXF_POINT;
            }
            else
            {
			    return D3DTEXF_NONE;
            }
		}
		return 0;
	}
	//---------------------------------------------------------------------
	D3D9Mappings::eD3DTexType D3D9Mappings::get(TextureType ogreTexType)
	{
		switch( ogreTexType )
		{
		case TEX_TYPE_1D :
		case TEX_TYPE_2D :
			return D3D9Mappings::D3D_TEX_TYPE_NORMAL;
		case TEX_TYPE_CUBE_MAP :
			return D3D9Mappings::D3D_TEX_TYPE_CUBE;
		case TEX_TYPE_3D :
            return D3D9Mappings::D3D_TEX_TYPE_VOLUME;
		}
		return D3D9Mappings::D3D_TEX_TYPE_NONE;
	}
	//---------------------------------------------------------------------
    DWORD D3D9Mappings::get(HardwareBuffer::Usage usage)
    {
        DWORD ret = 0;
        if (usage & HardwareBuffer::HBU_DYNAMIC)
        {
            ret |= D3DUSAGE_DYNAMIC;
        }
        if (usage & HardwareBuffer::HBU_WRITE_ONLY)
        {
            ret |= D3DUSAGE_WRITEONLY;
        }
        return ret;
    }
	//---------------------------------------------------------------------
    DWORD D3D9Mappings::get(HardwareBuffer::LockOptions options)
    {
        DWORD ret = 0;
        if (options == HardwareBuffer::HBL_DISCARD)
        {
            ret |= D3DLOCK_DISCARD;
        }
        if (options == HardwareBuffer::HBL_READ_ONLY)
        {
            ret |= D3DLOCK_READONLY;
        }
        if (options == HardwareBuffer::HBL_NO_OVERWRITE)
        {
            ret |= D3DLOCK_NOOVERWRITE;
        }

        return ret;
    }
	//---------------------------------------------------------------------
    D3DFORMAT D3D9Mappings::get(HardwareIndexBuffer::IndexType itype)
    {
        if (itype == HardwareIndexBuffer::IT_32BIT)
        {
            return D3DFMT_INDEX32;
        }
        else
        {
            return D3DFMT_INDEX16;
        }
    }
	//---------------------------------------------------------------------
	D3DDECLTYPE D3D9Mappings::get(VertexElementType vType)
	{
		switch (vType)
		{
		case VET_COLOUR:
			return D3DDECLTYPE_D3DCOLOR;
			break;
		case VET_FLOAT1:
			return D3DDECLTYPE_FLOAT1;
			break;
		case VET_FLOAT2:
			return D3DDECLTYPE_FLOAT2;
			break;
		case VET_FLOAT3:
			return D3DDECLTYPE_FLOAT3;
			break;
		}
		// to keep compiler happy
		return D3DDECLTYPE_FLOAT3;
	}
	//---------------------------------------------------------------------
	D3DDECLUSAGE D3D9Mappings::get(VertexElementSemantic sem)
	{
		switch (sem)
		{
		case VES_BLEND_INDICES:
			return D3DDECLUSAGE_BLENDINDICES;
			break;
		case VES_BLEND_WEIGHTS:
			return D3DDECLUSAGE_BLENDWEIGHT;
			break;
		case VES_DIFFUSE:
			return D3DDECLUSAGE_COLOR; // NB index will differentiate
			break;
		case VES_SPECULAR:
			return D3DDECLUSAGE_COLOR; // NB index will differentiate
			break;
		case VES_NORMAL:
			return D3DDECLUSAGE_NORMAL;
			break;
		case VES_POSITION:
			return D3DDECLUSAGE_POSITION;
			break;
		case VES_TEXTURE_COORDINATES:
			return D3DDECLUSAGE_TEXCOORD;
			break;
		case VES_BINORMAL:
			return D3DDECLUSAGE_BINORMAL;
			break;
		case VES_TANGENT:
			return D3DDECLUSAGE_TANGENT;
			break;
		}
		// to keep compiler happy
		return D3DDECLUSAGE_POSITION;
	}


}