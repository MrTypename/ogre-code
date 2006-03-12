!define TARGET_COMPILER_DESCRIPTION "C::B + MingW + STLPort"
!define TARGET_COMPILER "CBMingW_STLP"
!define MINGW
!include ogresdk.nsh
!define STLPORT_PATH "..\..\..\stlport"

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*_stlp.cbp"
  SetOutPath "$INSTDIR\samples\src"
  SetOverwrite try
  File ".\Samples\src\*.cpp"
  SetOutPath "$INSTDIR\samples\include"
  SetOverwrite try
  File ".\Samples\include\*.h"

  ; Refapp
  SetOutPath "$INSTDIR\samples\refapp\scripts"
  SetOverwrite try
  File ".\samples\refapp\scripts\*_stlp.cbp"
  SetOutPath "$INSTDIR\samples\refapp\src"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\src\*.cpp"
  SetOutPath "$INSTDIR\samples\refapp\include"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\include\*.h"
  
  
  SetOutPath "$INSTDIR\samples"
  SetOverwrite try
  File ".\Samples\Samples.workspace"


SectionEnd

Section -STLPort
  SetOutPath "$INSTDIR\stlport\stlport"
  SetOverwrite try
  File /r "$(STLPORT_PATH)\stlport\*.*"
  
  SetOutPath "$INSTDIR\bin\debug"
  SetOverwrite try
  File "$(STLPORT_PATH)\bin\stlportstlg.5.0.dll"
  SetOutPath "$INSTDIR\bin\release"
  SetOverwrite try
  File "$(STLPORT_PATH)\bin\stlport.5.0.dll"
  
  SetOutPath "$INSTDIR\stlport\lib"
  SetOverwrite try
  File "$(STLPORT_PATH)\lib\libstlport.5.0.dll.a"
  File "$(STLPORT_PATH)\lib\libstlportstlg.5.0.dll.a"

SectionEnd