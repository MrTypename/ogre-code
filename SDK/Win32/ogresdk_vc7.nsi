!define TARGET_COMPILER_DESCRIPTION "Visual C++.Net 2002"
!define TARGET_COMPILER "VC70"
!include ogresdk.nsh

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*.vcproj"
  SetOutPath "$INSTDIR\samples\src"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.cpp"
  SetOutPath "$INSTDIR\samples\include"
  SetOverwrite try
  File /r /x CVS /x CEGUIRenderer "..\..\Samples\*.h"

  ; Refapp
  SetOutPath "$INSTDIR\samples\refapp\scripts"
  SetOverwrite try
  File ".\samples\refapp\scripts\*.vcproj"
  SetOutPath "$INSTDIR\samples\refapp\src"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\src\*.cpp"
  SetOutPath "$INSTDIR\samples\refapp\include"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\include\*.h"
  
  
  SetOutPath "$INSTDIR\samples"
  SetOverwrite try
  File ".\Samples\Samples.sln"
  File ".\Samples\Samples.suo"


SectionEnd