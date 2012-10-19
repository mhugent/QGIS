!include "MUI2.nsh"

Name "NIWA Quantum MAP"
!define INSTALLATIONNAME "NIWA Quantum Map"
OutFile "niwa_quantum_map_setup.exe"

InstallDir $PROGRAMFILES\NIWA_Quantum_Map

!define MUI_ABORTWARNING
!define MUI_ICON ".\Installer-Files\nqmap.ico"
!define MUI_UNICON ".\Installer-Files\nqmap.ico"
!define MUI_HEADERIMAGE_BITMAP_NOSTETCH ".\Installer-Files\InstallHeaderImage.bmp"
!define MUI_HEADERIMAGE_UNBITMAP_NOSTRETCH ".\Installer-Files\UnInstallHeaderImage.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP ".\Installer-Files\nqmap_welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\Installer-Files\nqmap_unwelcome.bmp"

!insertmacro MUI_PAGE_WELCOME
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
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayName" "NIWA Quantum Map"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString" "$INSTDIR\uninstall.exe"

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoModify" 1

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoRepair" 1

  WriteRegStr HKEY_CURRENT_USER "Software\NIWA\NIWA Quantum Map\Plugins" "webdataplugin" "true"

  WriteRegStr HKEY_CURRENT_USER "Software\NIWA\NIWA Quantum Map\PythonPlugins" "survey_design" "true"

  WriteRegStr HKEY_CURRENT_USER "Software\NIWA\NIWA Quantum Map\Qgis" "showTips" "false"
  


;Desktop shortcut
  Delete "$DESKTOP\NIWA Quantum Map.lnk"
  CreateShortCut "$DESKTOP\NIWA Quantum Map.lnk" "$INSTDIR\bin\niwa_quantum_map.exe" "" "$INSTDIR\icons\nqmap.ico"
  Delete "$DESKTOP\Demo project.lnk"
  ;CreateShortCut "$DESKTOP\Demo project.lnk" "$INSTDIR\bin\niwa_quantum_map.exe" "$\"$INSTDIR\data\project1.qgs$\"" "$INSTDIR\icons\nqmap.ico"
  Delete "$DESKTOP\NIWA Quantum Map FAQ.lnk"
  CreateShortCut "$DESKTOP\NIWA Quantum Map FAQ.lnk" "$INSTDIR\resources\Quantum_Map_FAQ_v3.pdf"
  
SectionEnd

Section "Uninstall"
  Delete $INSTDIR\uninstall.exe
  RMDir /r "$INSTDIR"
  Delete "$DESKTOP\NIWA Quantum Map.lnk"
  Delete "$DESKTOP\Demo project.lnk"
  Delete "$DESKTOP\NIWA Quantum Map FAQ.lnk"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}"
SectionEnd

Function .onInit
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString"
  StrCmp $R0 "" done
  
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "${INSTALLATIONNAME} is already installed. $\n Click 'OK' to remove the \
  previous version or 'Cancel' to cancel this upgrade." \
  IDOK uninst
  Abort

 uninst:
  Exec $INSTDIR\uninstall.exe
 done:
FunctionEnd