/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Lasse Tassing

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
#include "LexiMaxExport.h"
#include "LexiOgreMeshCompiler.h"
#include "LexiOgreMaterialCompiler.h"
#include "LexiIntermediateBuilder.h"
#include "LexiIntermediateMesh.h"

CExportObjectRoot::CExportObjectRoot(CDDObject *pConfig) : CExportObject(pConfig)
{
//	m_sName="<root>";
	m_pDDConfig->SetString("Name", "<root>");	// Hardwire name
	m_pMetaCtrl=NULL;
	m_pDDMetaDesc=NULL;
}

// Get window for editing ExportObject properties
GDI::Window* CExportObjectRoot::GetEditWindow(GDI::Window *pParent)
{
	if(m_pMetaCtrl==NULL)
	{
		HWND hMeta = ::CreateWindow("MetaControl", NULL, WS_TABSTOP | WS_CHILD | WS_CLIPCHILDREN, 0, 0, 100, 100, pParent->m_hWnd, NULL, CExporter::m_hInstance, NULL);

		m_pMetaCtrl = new GDI::MetaControl;
		m_pMetaCtrl->Attach(hMeta);

		m_pDDMetaDesc=BuildMetaDesc();
		m_pMetaCtrl->CreateFromMetaData(m_pDDMetaDesc, m_pDDConfig);	
	}
	return m_pMetaCtrl;
}

// Close edit window
void CExportObjectRoot::CloseEditWindow()
{
	if(m_pMetaCtrl!=NULL)
	{
		// Destroy meta windows
		HWND hMeta=m_pMetaCtrl->m_hWnd;		
		m_pMetaCtrl->Detach();
		::DestroyWindow(hMeta);
		delete m_pMetaCtrl;
		m_pMetaCtrl=NULL;

		if(m_pDDMetaDesc) 
		{
			m_pDDMetaDesc->Release();
			m_pDDMetaDesc=NULL;
		}
	}
}

// Called when object is first created [by user].
// This allows for wizard-style editing of required data.
// If this function returns false, the object is not created
bool CExportObjectRoot::OnCreate(CExporterPropertiesDlg *pPropDialog)
{
	return false; // Cannot be created
}

// Check if ExportObject supports a given ExportObject instance as parent
bool CExportObjectRoot::SupportsParentType(const CExportObject *pParent) const
{	
	return false; // Root cannot act as child
}

//
CDDObject* CExportObjectRoot::BuildMetaDesc( void )
{
	CDDObject* pDDMetaDesc = new CDDObject();

	fastvector<const CDDObject*> lSettings;
	CDDObject* pDDMetaElement;

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", ResolveMacros("$(username)@$(hostname).RootFolder").c_str());
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Root");
	pDDMetaElement->SetString("Help", ResolveMacros("Location of root folder (unique for $(username)@$(hostname))").c_str());
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultMeshFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Meshes");
	pDDMetaElement->SetString("Help", "Location of default mesh folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultSkeletonFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Skeletons");
	pDDMetaElement->SetString("Help", "Location of default skeleton folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultMaterialFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Materials");
	pDDMetaElement->SetString("Help", "Location of default material folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultTextureFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Textures");
	pDDMetaElement->SetString("Help", "Location of default texture folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultShaderFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Shaders");
	pDDMetaElement->SetString("Help", "Location of default shader folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultSceneFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Scenes");
	pDDMetaElement->SetString("Help", "Location of default scene folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "DefaultAnimationFolder");
	pDDMetaElement->SetString("Type", "string");
	pDDMetaElement->SetString("Group", "Default Folders");
	pDDMetaElement->SetString("Caption", "Animations");
	pDDMetaElement->SetString("Help", "Location of default animation folder");
	pDDMetaElement->SetString("Default", "");
	pDDMetaElement->SetString("HelperButton", "BrowseForFolder");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID","referenceShadersID");
	pDDMetaElement->SetString("Type","bool");
	pDDMetaElement->SetString("Group","Default Mesh Settings");
	pDDMetaElement->SetString("Caption","Reference Shaders in Materials");
	pDDMetaElement->SetString("Help","Include references to shaders in material files.");
	pDDMetaElement->SetBool("Default", true);
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "ForceSeperateAnimID");
	pDDMetaElement->SetString("Type", "bool");
	pDDMetaElement->SetString("Group", "Export Settings");
	pDDMetaElement->SetString("Caption", "Force Seperate Skeletons");
	pDDMetaElement->SetString("Help", "Globally force all animations to be written in seperate files");
	pDDMetaElement->SetBool("Default", false);
		// Disable for now since reindexing bone weights are broken!
	pDDMetaElement->SetString("Condition", "$ForceSeperateAnimID=true");
	lSettings.push_back(pDDMetaElement);

	pDDMetaElement = new CDDObject();
	pDDMetaElement->SetString("ID", "OptimizeMeshID");
	pDDMetaElement->SetString("Type", "bool");
	pDDMetaElement->SetString("Group", "Export Settings");
	pDDMetaElement->SetString("Caption", "Optimize Mesh Output");
	pDDMetaElement->SetString("Help", "Optimize all mesh files");	
	pDDMetaElement->SetBool("Default", false);
		// Disable for now since reindexing bone weights are broken!
	pDDMetaElement->SetString("Condition", "$OptimizeMeshID=true");
	lSettings.push_back(pDDMetaElement);	

	pDDMetaDesc->SetDDList("MetaList", lSettings, false);

	return pDDMetaDesc;
}

