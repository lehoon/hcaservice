; 该脚本使用 HM VNISEdit 脚本编辑器向导产生

; 安装程序初始定义常量
!define PRODUCT_NAME "移动定位上位机系统"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "lehoon"
!define PRODUCT_WEB_SITE "http://www.lehoon.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\hcaservice.exe"
!define PRODUCT_INSTALL_PATH "hcaservice"



SetCompressor lzma

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "..\hcaservice\hcaservice.ico"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议页面
;!insertmacro MUI_PAGE_LICENSE "..\hcaservice\License.rtf"
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
!define MUI_FINISHPAGE_RUN "$INSTDIR\hcaservice.exe"
!insertmacro MUI_PAGE_FINISH

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; administrator rule
RequestExecutionLevel admin


;预定义安装目录
Function .onInit
	StrCpy $R1 "C:\Program Files\"
	StrCpy $INSTDIR "$R1${PRODUCT_INSTALL_PATH}"
FunctionEnd

; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "移动定位上位机系统安装程序.exe"
InstallDir "$PROGRAMFILES\hcaservice"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$SMPROGRAMS\移动定位上位机系统"
  CreateShortCut "$SMPROGRAMS\移动定位上位机系统\移动定位上位机系统.lnk" "$INSTDIR\hcaservice.exe"
  CreateShortCut "$DESKTOP\移动定位上位机系统.lnk" "$INSTDIR\hcaservice.exe"

  ReadRegStr $0 HKCU "Environment" "Path"
  WriteRegExpandStr HKCU "Environment" "Path" "$0;$INSTDIR\"
  ;SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment"
  ;File "hcaservice.ini"
  File "hcaservice.ini"
  File "hcaservice.exe"
  File "dbghelp.dll"
  File "sqlite3.dll"
  ;File "uninstall.exe"

  #uninstall.exe
  

  #安装服务
  DetailPrint "安装服务"
  SimpleSC::InstallService "hcaservice" "移动定位上位机系统" "16" "2" "C:\Program Files\hcaservice\hcaservice.exe" "" "" ""

  DetailPrint "启动服务"
  SimpleSC::StartService "hcaservice" "" 30
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\移动定位上位机系统\公司网址.lnk" "$INSTDIR\${PRODUCT_NAME}.url" "${MUI_ICON}"
SectionEnd

Section Uninstall
  SimpleSC::StopService "hcaservice" 0 5
  SimpleSC::RemoveService "hcaservice"
  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"
SectionEnd

Section -Post
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\hcaservice.exe"
  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "您确实要完全移除 $(^Name) ，及其所有的组件？" IDYES +2
  Abort
FunctionEnd

