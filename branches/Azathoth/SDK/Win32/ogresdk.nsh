; Script generated by the HM NIS Edit Script Wizard.

!include WriteEnvStr.nsh

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "OGRE SDK"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "The OGRE Team"
!define PRODUCT_WEB_SITE "http://www.ogre3d.org"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"

SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "OGRE SDK"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION} for ${TARGET_COMPILER_DESCRIPTION}"
OutFile "OgreSDKSetup${PRODUCT_VERSION}_${TARGET_COMPILER}.exe"
InstallDir "c:\OgreSDK"
ShowInstDetails show
ShowUnInstDetails show

Section -Headers

  ; Required header files
  ; Core
  SetOutPath "$INSTDIR\include"
  SetOverwrite try
  File "..\..\OgreMain\include\*.h"
  File "..\..\Samples\Common\include\*.h"
  File "..\..\Samples\Common\CEGUIRenderer\include\*.h"
  ; Dependencies - only ODE and CEGui
  SetOutPath "$INSTDIR\include\CEGUI"
  SetOverwrite try
  File /r /x CVS "..\..\Dependencies\include\CEGUI\*.*"
  SetOutPath "$INSTDIR\include\ode"
  SetOverwrite try
  File /r /x CVS "..\..\Dependencies\include\ode\*.*"

  ; Optional headers (for linking direct to plugins)
  SetOutPath "$INSTDIR\include\opt"
  SetOverwrite try
  File /r /x CVS "..\..\Plugins\OctreeSceneManager\include\*.h"
  File /r /x CVS "..\..\Plugins\BspSceneManager\include\*.h"

SectionEnd

Section -Libs
  ; Library files
  SetOutPath "$INSTDIR\lib"
  SetOverwrite try
  File "..\..\OgreMain\lib\Debug\OgreMain_d.lib"
  File "..\..\OgreMain\lib\Debug\OgreMain_d.pdb"
  File "..\..\Dependencies\lib\Debug\CEGUIBase_d.lib"
  ; ode.lib is only one available, no separate release version
  File "..\..\Dependencies\lib\Debug\ode.lib"
  File "..\..\Samples\Common\CEGUIRenderer\lib\OgreGUIRenderer_d.lib"
  File "..\..\Samples\Common\CEGUIRenderer\bin\Debug\OgreGUIRenderer.pdb"

  File "..\..\OgreMain\lib\Release\OgreMain.lib"
  File "..\..\Dependencies\lib\Release\CEGUIBase.lib"
  File "..\..\Samples\Common\CEGUIRenderer\lib\OgreGUIRenderer.lib"

  ; Optional library files (for linking direct to plugins)
  SetOutPath "$INSTDIR\lib\opt\debug"
  SetOverwrite try
  File "..\..\Plugins\OctreeSceneManager\bin\debug\Plugin_OctreeSceneManager.lib"
  File "..\..\Plugins\OctreeSceneManager\bin\debug\Plugin_OctreeSceneManager.pdb"
  File "..\..\Plugins\BspSceneManager\bin\debug\Plugin_BspSceneManager.lib"
  File "..\..\Plugins\BspSceneManager\bin\debug\Plugin_BspSceneManager.pdb"
  SetOutPath "$INSTDIR\lib\opt\release"
  SetOverwrite try
  File "..\..\Plugins\OctreeSceneManager\bin\release\Plugin_OctreeSceneManager.lib"
  File "..\..\Plugins\BspSceneManager\bin\release\Plugin_BspSceneManager.lib"

SectionEnd

Section -Binaries

  ; Binaries - debug
  SetOutPath "$INSTDIR\bin\debug"
  SetOverwrite ifnewer
  File "..\..\Samples\Common\bin\Debug\ilut.dll"
  File "..\..\Samples\Common\bin\Debug\ilu.dll"
  File "..\..\Samples\Common\bin\Debug\devil.dll"
  File "..\..\Samples\Common\bin\Debug\cg.dll"
  File "..\..\Samples\Common\bin\Debug\zlib1.dll"
  File "..\..\Samples\Common\bin\Debug\xerces-c_2_5_0D.dll"

  File "..\..\Samples\Common\bin\Debug\OgreMain_d.dll"
  File "..\..\Samples\Common\bin\Debug\OgrePlatform_d.dll"
  File "..\..\Samples\Common\bin\Debug\CEGUIBase_d.dll"
  File "..\..\Samples\Common\bin\Debug\CEGUITaharezLook_d.dll"
  File "..\..\Samples\Common\bin\Debug\CEGUIWindowsLook_d.dll"
  File "..\..\Samples\Common\bin\Debug\Plugin_BSPSceneManager.dll"
  File "..\..\Samples\Common\bin\Debug\Plugin_CgProgramManager.dll"
  File "..\..\Samples\Common\bin\Debug\Plugin_OctreeSceneManager.dll"
  File "..\..\Samples\Common\bin\Debug\Plugin_ParticleFX.dll"
  File "..\..\Samples\Common\bin\Debug\RenderSystem_Direct3D7.dll"
  File "..\..\Samples\Common\bin\Debug\RenderSystem_Direct3D9.dll"
  File "..\..\Samples\Common\bin\Debug\RenderSystem_GL.dll"
  File "..\..\Samples\Common\bin\Debug\OgreGUIRenderer_d.dll"

  File "..\..\Samples\Common\bin\Debug\resources.cfg"
  File "..\..\Samples\Common\bin\Debug\plugins.cfg"
  File "..\..\Samples\Common\bin\Debug\terrain.cfg"
  File "..\..\Samples\Common\bin\Debug\media.cfg"
  ; Binaries - release
  SetOutPath "$INSTDIR\bin\release"
  SetOverwrite ifnewer
  File "..\..\Samples\Common\bin\Release\ilut.dll"
  File "..\..\Samples\Common\bin\Release\ilu.dll"
  File "..\..\Samples\Common\bin\Release\devil.dll"
  File "..\..\Samples\Common\bin\Release\cg.dll"
  File "..\..\Samples\Common\bin\Release\zlib1.dll"
  File "..\..\Samples\Common\bin\Release\xerces-c_2_5_0.dll"

  File "..\..\Samples\Common\bin\Release\OgreMain.dll"
  File "..\..\Samples\Common\bin\Release\OgrePlatform.dll"
  File "..\..\Samples\Common\bin\Release\CEGUIBase.dll"
  File "..\..\Samples\Common\bin\Release\CEGUITaharezLook.dll"
  File "..\..\Samples\Common\bin\Release\CEGUIWindowsLook.dll"
  File "..\..\Samples\Common\bin\Release\Plugin_BSPSceneManager.dll"
  File "..\..\Samples\Common\bin\Release\Plugin_CgProgramManager.dll"
  File "..\..\Samples\Common\bin\Release\Plugin_OctreeSceneManager.dll"
  File "..\..\Samples\Common\bin\Release\Plugin_ParticleFX.dll"
  File "..\..\Samples\Common\bin\Release\RenderSystem_Direct3D7.dll"
  File "..\..\Samples\Common\bin\Release\RenderSystem_Direct3D9.dll"
  File "..\..\Samples\Common\bin\Release\RenderSystem_GL.dll"
  File "..\..\Samples\Common\bin\Release\OgreGUIRenderer.dll"

  File "..\..\Samples\Common\bin\Release\resources.cfg"
  File "..\..\Samples\Common\bin\Release\plugins.cfg"
  File "..\..\Samples\Common\bin\Release\terrain.cfg"
  File "..\..\Samples\Common\bin\Release\media.cfg"

SectionEnd

Section -Media
  SetOutPath "$INSTDIR\media"
  SetOverwrite ifnewer

  File /r /x CVS "..\..\Samples\Media\*.*"

SectionEnd

Section -Docs
  ; Documentation
  SetOutPath "$INSTDIR\docs\manual\images"
  SetOverwrite try
  File "..\..\Docs\manual\images\*.*"
  SetOutPath "$INSTDIR\docs\manual"
  File "..\..\Docs\manual\*.*"

  SetOutPath "$INSTDIR\docs\api"
  SetOverwrite try
  File "..\..\Docs\api\html\OgreAPIReference.*"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\OGRE Manual.lnk" "$INSTDIR\docs\manual\index.html"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\OGRE API Reference.lnk" "$INSTDIR\docs\api\OgreAPIReference.chm"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


Section -AdditionalIcons
  SetOutPath $INSTDIR
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  WriteIniStr "$INSTDIR\OgreWebSite.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\OGRE Website.lnk" "$INSTDIR\OgreWebSite.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\uninst.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  
  ; Register OGRE_HOME
  Push "OGRE_HOME"
  Push $INSTDIR
  Call WriteEnvStr

SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
  RMDir /r "$INSTDIR"

  Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\Website.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\OGRE API Reference.lnk"
  Delete "$SMPROGRAMS\$ICONS_GROUP\OGRE Manual.lnk"

  RMDir "$SMPROGRAMS\$ICONS_GROUP"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd
