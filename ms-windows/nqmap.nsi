!include "MUI2.nsh"

Name "NQMAP installer"
!define INSTALLATIONNAME "NQMAP"
OutFile "nqmap_setup.exe"

InstallDir $PROGRAMFILES\NQMAP

!insertmacro MUI_PAGE_LICENSE ".\Installer-Files\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section ""

  SetOutPath "$INSTDIR"
  File /r .\nqmap\*.*

;Uninstaller
  WriteUninstaller $INSTDIR\uninstall.exe
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayName" "NQMAP"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString" "$INSTDIR\uninstall.exe"

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoModify" 1

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoRepair" 1

;Desktop shortcut
  Delete "$DESKTOP\NQMAP.lnk"
  CreateShortCut "$DESKTOP\NQMAP.lnk" "$INSTDIR\bin\qgis.exe" "" "$INSTDIR\icons\QGIS.ico"

  
SectionEnd

Section "Uninstall"
  Delete $INSTDIR\uninstall.exe
  RMDir /r "$INSTDIR"
  Delete "$DESKTOP\NQMAP.lnk"

SectionEnd