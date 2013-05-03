;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

  !define ALL_USERS
  
;--------------------------------
;General
  
  !define COMPANY_DIR_NAME	"Whirling Chair"
  !define APP_NAME "Loom Previewer"
  !define VERSION "0.0.2"
  !define APP_EXECUTABLE "LoomPreview.exe"
  
  ;Name and file
  Name "Loom Preview (${VERSION})"
  OutFile "LoomPreviewInstaller.${VERSION}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${COMPANY_DIR_NAME}\${APP_NAME}"
  
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "LICENSE.GPL3"
  ;!insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section

  SetOutPath $INSTDIR
  
  File "release\${APP_EXECUTABLE}"
  
  CreateDirectory $INSTDIR\ui
  SetOutPath $INSTDIR\ui
  ;File ui\*.png
  File ui\*.qrc

  SetOutPath $INSTDIR

  File "$%VS80COMNTOOLS%\..\..\SDK\v2.0\BootStrapper\Packages\vcredist_x86\vcredist_x86.exe"
  DetailPrint "Installing Microsoft Visual Studio redistributable DLLs."
  ExecWait '"$INSTDIR\vcredist_x86.exe"'
  Delete $INSTDIR\vcredist_x86.exe

  File $%QT_DIR%\bin\QtCore4.dll
  File $%QT_DIR%\bin\QtGui4.dll

  CreateDirectory $INSTDIR\imageformats
  SetOutPath $INSTDIR\imageformats
  File $%QT_DIR%\plugins\imageformats\qtiff4.dll

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\${COMPANY_DIR_NAME}"
  CreateDirectory "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}"

  CreateShortCut "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}\LoomPreview.lnk" "$INSTDIR\${APP_EXECUTABLE}"
  CreateShortCut "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete $INSTDIR\*.exe
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\imageformats\*.dll
  Delete $INSTDIR\*.txt
  
  Delete $INSTDIR\UI\*.*
  RMDir /r $INSTDIR\UI

  RMDir /r "$INSTDIR"
  RMDir "$PROGRAMFILES\${COMPANY_DIR_NAME}"

  Delete "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}\LoomPreview.lnk"
  Delete "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}\Uninstall.lnk"

  RMDir "$SMPROGRAMS\${COMPANY_DIR_NAME}\${APP_NAME}"
  RMDir "$SMPROGRAMS\${COMPANY_DIR_NAME}"

SectionEnd
