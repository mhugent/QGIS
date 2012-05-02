Name "NQMAP installer"

OutFile "nqmap_setup.exe"

InstallDir $PROGRAMFILES\NQMAP

Page directory

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section ""

  SetOutPath "$INSTDIR"
  File /r .\nqmap\*.*
  WriteUninstaller $INSTDIR\uninstall.exe

SectionEnd

Section "Uninstall"
  Delete $INSTDIR\uninstall.exe
  RMDir /r "$INSTDIR"
SectionEnd