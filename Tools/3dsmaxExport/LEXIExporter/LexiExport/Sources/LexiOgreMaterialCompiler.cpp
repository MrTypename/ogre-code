/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Mark Folkenberg,
Bo Krohn

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

#include "LexiStdAfx.h"
#include "LexiOgreMaterialCompiler.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include <fstream>
#include <iostream>
#include <direct.h>


COgreMaterialCompiler::COgreMaterialCompiler( CIntermediateMaterial* pIntermediateMaterial, Ogre::String sExtension, bool bExportColours )
:	m_bShadersSupported(false),
	m_bExportColours(bExportColours)
{
	REGISTER_MODULE("Ogre Material Compiler")

	m_pOgreMaterial.setNull();
	m_pIMaterial = pIntermediateMaterial;

	m_sExtensionOverride = sExtension;

	InitializeOgreComponents();

	Ogre::GpuProgramManager* pGPUMgr = Ogre::GpuProgramManager::getSingletonPtr();

	if(pGPUMgr->isSyntaxSupported("arbvp1") && pGPUMgr->isSyntaxSupported("arbfp1"))
		m_bShadersSupported = true;

	// TEST:
	//m_bShadersSupported = false;

	if(!m_bShadersSupported)
		LOGWARNING "Shaders not supported on current Hardware! Writing simple reference materials.");

	CreateOgreMaterial();

}

COgreMaterialCompiler::~COgreMaterialCompiler( void )
{
	UNREGISTER_MODULE	
}

void COgreMaterialCompiler::InitializeOgreComponents( void )
{
	Ogre::MaterialManager::getSingletonPtr()->unloadAll();
	Ogre::MaterialManager::getSingletonPtr()->removeAll();
}


void COgreMaterialCompiler::CreateOgreMaterial( void )
{
	assert(m_pIMaterial);

	Ogre::MaterialManager* pMatMgr = Ogre::MaterialManager::getSingletonPtr();
	m_pOgreMaterial = pMatMgr->getByName( m_pIMaterial->GetName().c_str() );

	if(m_pOgreMaterial.isNull())
		m_pOgreMaterial = (Ogre::MaterialPtr)pMatMgr->create( m_pIMaterial->GetName().c_str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	else 
		return;

	assert(!m_pOgreMaterial.isNull());

	Ogre::Technique* technique = m_pOgreMaterial->getTechnique(0);
	if(technique == NULL)
		technique = m_pOgreMaterial->createTechnique();

	technique->setName("Default");

	Ogre::Pass*	pass = technique->getPass(0);
	if(pass == NULL)
		pass = technique->createPass();

	pass->setName("Main");
	pass->setAmbient( m_pIMaterial->GetAmbientColor() );
	pass->setSpecular( m_pIMaterial->GetSpecularColor() );
	pass->setShininess( m_pIMaterial->GetGlossiness() );
	//pass->setSelfIllumination( );
	pass->setCullingMode( m_pIMaterial->Get2Sided() ? Ogre::CULL_NONE : Ogre::CULL_CLOCKWISE);
	pass->setShadingMode( m_pIMaterial->GetFaceted() ? Ogre::SO_FLAT : Ogre::SO_GOURAUD);
	pass->setPolygonMode( m_pIMaterial->GetWired() ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID);

	// create texture units
	ParseMaterialMaps(pass);
}

void COgreMaterialCompiler::ParseMaterialMaps( Ogre::Pass* pass )
{
	// Here we should pass the intermediate material on to the plugin which is registered with
	// a matching mask.. when we have the plugin system up and running :)

	// mask to see if material is of type diffuse or other

	short mask = m_pIMaterial->GetMask();

	LOGINFO "Parsing Material(%s) with Map Mask: %i", m_pIMaterial->GetName().c_str(), mask);

	if(mask == 0)		// no texture maps
		CreatePureBlinn(pass);
	else if(mask == 2)		// only diffuse
		CreateDiffuse(pass);
	else if(mask == 4)		// only specular color
		CreateSpecularColor(pass);
	else if(mask == 8)		// only specular level
		CreateSpecularLevel(pass);
	else if(mask == 10)
		CreateDiffuseAndSpecularLevel(pass);
	else if(mask == 32)		// only self illumination map
		CreateSelfIllumination(pass);
	else if(mask == 66)		// diffuse map + opacity
		CreateDiffuseAndOpacity(pass);
	else if(mask & 2)		// fallback on ordinary diffuse
		CreateDiffuse(pass);


//	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
//	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
//	while (it != lMats.end())
//	{
//		Ogre::String ident = it->first;
//		STextureMapInfo texInfo = it->second;
//
//		Ogre::TextureUnitState* pTexUnit;
//
//		Ogre::String baseFile;
//		Ogre::String basePath;
//		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);
//
//		pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
//		pTexUnit->setTextureNameAlias( texInfo.m_sMapType );
//
//		if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
//			pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
//		if(texInfo.m_fAngle)
//			pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
//		if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
//			pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);
//
//		pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);
//
////		if(texInfo.m_sMapType.compare( "self_illumination" ) == 0)
////			pTexUnit->setColourOperation(Ogre::LBO_ADD);
////		else if(texInfo.m_sMapType.compare( "reflection" ) == 0)
////			pTexUnit->setEnvironmentMap( true, Ogre::Env)
//		//else
//		//	pTexUnit->setColourOperationEx(Ogre::LBX_MODULATE_X2);	
//
//
//		pass->setAlphaRejectFunction( Ogre::CMPF_ALWAYS_PASS ); //Default
//
//		if(texInfo.m_bAlpha) 
//		{
//			pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
//			pass->setAlphaRejectValue(128);
//		}
//
//		it++;
//	}
}

void COgreMaterialCompiler::CreateTextureUnits( Ogre::Pass* pass, STextureMapInfo texInfo  )
{
	Ogre::TextureUnitState* pTexUnit;

	Ogre::String baseFile;
	Ogre::String basePath;
	Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);

	if(m_sExtensionOverride != "")
	{
		Ogre::String sName; 
		Ogre::String sExtension = "."; 
		std::vector<Ogre::String> sSplitName = Ogre::StringUtil::split(baseFile,".");
		if(sSplitName.size()==2)
			baseFile = sSplitName[0];
		baseFile += m_sExtensionOverride;
	}

	pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
	pTexUnit->setTextureNameAlias( texInfo.m_sMapType );

	if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
		pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
	if(texInfo.m_fAngle)
		pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
	if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
		pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);

	pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);

	//	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
	//	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
	//	while (it != lMats.end())
	//	{
	//		Ogre::String ident = it->first;
	//		STextureMapInfo texInfo = it->second;
	//
	//		Ogre::TextureUnitState* pTexUnit;
	//
	//		Ogre::String baseFile;
	//		Ogre::String basePath;
	//		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);
	//
	//		pTexUnit = pass->createTextureUnitState( baseFile, texInfo.m_iCoordSet-1 );
	//		pTexUnit->setTextureNameAlias( texInfo.m_sMapType );
	//
	//		if(texInfo.m_fOffset[0] != 0 || texInfo.m_fOffset[1] != 0)
	//			pTexUnit->setTextureScroll(texInfo.m_fOffset[0], texInfo.m_fOffset[1]);
	//		if(texInfo.m_fAngle)
	//			pTexUnit->setTextureRotate(Ogre::Radian(-texInfo.m_fAngle));
	//		if(texInfo.m_fScale[0] != 1 || texInfo.m_fScale[1] != 1)
	//			pTexUnit->setTextureScale(1/texInfo.m_fScale[0], 1/texInfo.m_fScale[1]);
	//
	//		pTexUnit->setTextureAddressingMode(texInfo.m_AdressingMode);
	//
	////		if(texInfo.m_sMapType.compare( "self_illumination" ) == 0)
	////			pTexUnit->setColourOperation(Ogre::LBO_ADD);
	////		else if(texInfo.m_sMapType.compare( "reflection" ) == 0)
	////			pTexUnit->setEnvironmentMap( true, Ogre::Env)
	//		//else
	//		//	pTexUnit->setColourOperationEx(Ogre::LBX_MODULATE_X2);	
	//
	//
	//		pass->setAlphaRejectFunction( Ogre::CMPF_ALWAYS_PASS ); //Default
	//
	//		if(texInfo.m_bAlpha) 
	//		{
	//			pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
	//			pass->setAlphaRejectValue(128);
	//		}
	//
	//		it++;
	//	}
}

void COgreMaterialCompiler::SetBlendingByOpacity(Ogre::Pass* pass)
{
	if (m_pIMaterial->GetOpacity() < 1.0)
	{
		pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
		pass->setDepthWriteEnabled(false);
	}
}

void COgreMaterialCompiler::SetCommonFragmentProgramParameters(Ogre::Pass* pass, Ogre::GpuProgramParametersSharedPtr params )
{
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float specHack = m_pIMaterial->GetSpecularLevel() > 9.98 ? 9.98 : m_pIMaterial->GetSpecularLevel() ;
	// HACK: For some reason we cannot set the SpecularLevel if it is its max at 9.99, so we clamp it to 9.98
	float glossHack = m_pIMaterial->GetGlossiness() == 1 ? 0.9999 : m_pIMaterial->GetGlossiness() ;
	glossHack = glossHack > 0.01 ? glossHack * 100 : 0.0 ;
	// Ensure opacity isn't less than zero
	float fOpacity = m_pIMaterial->GetOpacity() < 0 ? 0.0 : m_pIMaterial->GetOpacity();

	params->setNamedConstant("ambientColor", m_pIMaterial->GetAmbientColor());
	params->setNamedConstant("diffuseColor", m_pIMaterial->GetDiffuseColor());
	params->setNamedConstant("specularColor", m_pIMaterial->GetSpecularColor());
	params->setNamedConstant("specularLevel", specHack);
	params->setNamedConstant("glossLevel", glossHack);
	params->setNamedConstant("opacity", fOpacity);
	
	SetBlendingByOpacity(pass);
}

void COgreMaterialCompiler::SetCommonVertexProgramParameters(Ogre::Pass* pass, Ogre::GpuProgramParametersSharedPtr params )
{
	// If we doesn't export vertex colours, we need to tell the shader since some GFX cards defaults to black!
	float fUseVertCol = m_bExportColours ? 1.0 : 0.0; 
	params->setNamedConstant("useVertCol", Ogre::Real(fUseVertCol));
}



void COgreMaterialCompiler::CreatePureBlinn( Ogre::Pass* pass )
{
	if(!m_bShadersSupported)
		return;
	
	pass->setVertexProgram("BlinnVP");
	pass->setFragmentProgram( "Blinn_Pure_FP" );

	Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
	SetCommonVertexProgramParameters(pass, vertParams);

	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	SetCommonFragmentProgramParameters(pass, params);
	pass->setFragmentProgramParameters(params);
}

void COgreMaterialCompiler::CreateDiffuse( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("diffuse");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("BlinnVP");
		pass->setFragmentProgram( "Blinn_DiffuseMap_FP" );

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if ( !texInfo.isNull()) params->setNamedConstant("amount", texInfo.m_fAmount);

		pass->setFragmentProgramParameters(params);
	}

	CreateTextureUnits(pass, texInfo);
}

void COgreMaterialCompiler::CreateSpecularColor( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("specular_color");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("BlinnVP");
		pass->setFragmentProgram( "Blinn_SpecularColor_FP" );

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if (!texInfo.isNull()) params->setNamedConstant("amount", texInfo.m_fAmount);

		pass->setFragmentProgramParameters(params);
	}
	CreateTextureUnits(pass, texInfo);
}

void COgreMaterialCompiler::CreateSpecularLevel( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("specular_level");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("BlinnVP");
		pass->setFragmentProgram( "Blinn_SpecularLevel_FP" );

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if ( !texInfo.isNull()) params->setNamedConstant("amount", texInfo.m_fAmount);

		pass->setFragmentProgramParameters(params);
	}

	CreateTextureUnits(pass, texInfo);
}

void COgreMaterialCompiler::CreateSelfIllumination( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("self_illumination");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("Blinn_4UV_VP");
		pass->setFragmentProgram( "Blinn_SelfIllumination_FP" );

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if ( !texInfo.isNull())
		{
			params->setNamedConstant("amount", texInfo.m_fAmount);
			params->setNamedConstant("uvIndex", (float)texInfo.m_iCoordSet-1);
		}
		pass->setFragmentProgramParameters(params);
	}

	CreateTextureUnits(pass, texInfo);
}

void COgreMaterialCompiler::CreateDiffuseAndOpacity( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("diffuse");
	STextureMapInfo texInfo2 = m_pIMaterial->GetTextureMapInfoFromName("opacity");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("BlinnVP");
		pass->setFragmentProgram( "Blinn_DiffuseAndOpacityMap_FP" );

		pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
		pass->setDepthWriteEnabled(false);

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if (!texInfo.isNull()) params->setNamedConstant("amount", texInfo.m_fAmount);
		if (!texInfo2.isNull()) params->setNamedConstant("opacity", texInfo2.m_fAmount);
		
		pass->setFragmentProgramParameters(params);
	}
	
	CreateTextureUnits(pass, texInfo);
	CreateTextureUnits(pass, texInfo2);
}

void COgreMaterialCompiler::CreateDiffuseAndSpecularLevel( Ogre::Pass* pass )
{
	STextureMapInfo texInfo = m_pIMaterial->GetTextureMapInfoFromName("diffuse");
	STextureMapInfo texInfo2 = m_pIMaterial->GetTextureMapInfoFromName("specular_level");

	if(m_bShadersSupported)
	{
		pass->setVertexProgram("BlinnVP");
		pass->setFragmentProgram( "Blinn_DiffuseAndSpecularMap_FP" );

		Ogre::GpuProgramParametersSharedPtr vertParams = pass->getVertexProgramParameters();
		SetCommonVertexProgramParameters(pass, vertParams);

		Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		SetCommonFragmentProgramParameters(pass, params);

		if (!texInfo.isNull()) params->setNamedConstant("amount", texInfo.m_fAmount);
		if (!texInfo2.isNull()) params->setNamedConstant("amount", texInfo2.m_fAmount);
		pass->setFragmentProgramParameters(params);
	}

	CreateTextureUnits(pass, texInfo);
	CreateTextureUnits(pass, texInfo2);
}

void COgreMaterialCompiler::CopyTextureMaps( Ogre::String outPath, Ogre::String extension )
{
	// copy texture files to target path
	// optionally convert to .dds

	Ogre::String sLastLocation="";
	bool bGroupAdded = false;

	const std::map< Ogre::String, STextureMapInfo >& lMats = m_pIMaterial->GetTextureMaps();
	std::map< Ogre::String, STextureMapInfo >::const_iterator it = lMats.begin();
	while (it != lMats.end())
	{
		Ogre::String ident = it->first;
		STextureMapInfo texInfo = it->second;

		Ogre::String baseFile;
		Ogre::String basePath;
		Ogre::StringUtil::splitFilename( texInfo.m_sFilename, baseFile, basePath);

		Ogre::String sName; 
		Ogre::String sExtension = "."; 
		std::vector<Ogre::String> sSplitName = Ogre::StringUtil::split(baseFile,".");
		if(sSplitName.size()==2)
		{
			sName = sSplitName[0];
			sExtension += sSplitName[1];
		}

		if( (sExtension == extension) || (extension==""))
		{
			// no need for any conversion, so we just do a simple copy
			if( doFileCopy(texInfo.m_sFilename, outPath+baseFile) != 0 )
				LOGWARNING "Couldn�t copy texture map: %s", texInfo.m_sFilename.c_str());
		}
		else
		{
			try
			{
				if(sLastLocation != basePath)
				{
					if(bGroupAdded) Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup("ImageLocation");
					Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(basePath, "FileSystem", "ImageLocation");
					Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup("ImageLocation");
					sLastLocation = basePath;
					bGroupAdded = true;
				}
				Ogre::Image inImage;
				inImage.load(texInfo.m_sFilename,"ImageLocation");
				inImage.save(outPath+sName+extension);
			} catch (Ogre::Exception e) 
			{
				LOGERROR "%s. Image not copied.", e.getDescription().c_str()); 
			}
		}
		it++;
	}
	if(bGroupAdded) Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup("ImageLocation");
}

void COgreMaterialCompiler::CopyShaderSources( Ogre::String outPath )
{
	// copy texture files to target path
	// optionally convert to .dds

	// ensure we are in the 3dMax dir
	char szAppPath[MAX_PATH] = "";
	::GetModuleFileName(NULL,szAppPath,sizeof(szAppPath) - 1);

	Ogre::String cwd(szAppPath);
	Ogre::String fileName, filePath;
	Ogre::StringUtil::splitFilename(cwd, fileName, filePath);

	_chdir(filePath.c_str());
	
	doFileCopy("LexiExporter\\shaders\\Blinn.cg", outPath+Ogre::String("Blinn.cg") );
	doFileCopy("LexiExporter\\shaders\\Shaders.program", outPath+Ogre::String("Shaders.program") );
}

bool COgreMaterialCompiler::WriteOgreMaterial( const Ogre::String& sFilename )
{
	Ogre::MaterialSerializer* pMatWriter = new Ogre::MaterialSerializer();
	try
	{
		pMatWriter->exportMaterial( m_pOgreMaterial, sFilename );
	}
	catch (Ogre::Exception& e)
	{
		MessageBox( NULL, e.getFullDescription().c_str(), "ERROR", MB_ICONERROR);
		return false;
	}
	delete pMatWriter;
	return true;
}

Ogre::MaterialPtr COgreMaterialCompiler::GetOgreMaterial( void )
{
	return m_pOgreMaterial;
}


#define BUFF_SIZE 2048
int COgreMaterialCompiler::doFileCopy(Ogre::String inFile, Ogre::String outFile)
{
	char buff[BUFF_SIZE];
	int readBytes = 1;

	ifstream inFileStream(inFile.c_str(), ios::in|ios::binary);
	if(!inFileStream)
	{
		return -1;
	}

	// Test to see if file already exists
	//ifstream tmpStream(outFile.c_str());
	//if(tmpStream)
	//{
	//	return -1;
	//}
	//tmpStream.close();

	ofstream outFileStream(outFile.c_str(), ios::out|ios::binary);
	if(!outFileStream)
	{
		return -1;
	}

	while(readBytes != 0)
	{
		inFileStream.read((char*)buff, BUFF_SIZE);
		readBytes = inFileStream.gcount();
		outFileStream.write((char*)buff, readBytes);
	}
	return 0;
}