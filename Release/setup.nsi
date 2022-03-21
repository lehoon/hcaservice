; �ýű�ʹ�� HM VNISEdit �ű��༭���򵼲���

; ��װ�����ʼ���峣��
!define PRODUCT_NAME "�ƶ���λ��λ��ϵͳ"
!define PRODUCT_VERSION "1.0"
!define PRODUCT_PUBLISHER "lehoon"
!define PRODUCT_WEB_SITE "http://www.lehoon.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\hcaservice.exe"
!define PRODUCT_INSTALL_PATH "hcaservice"



SetCompressor lzma

; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "..\hcaservice\hcaservice.ico"

; ��ӭҳ��
!insertmacro MUI_PAGE_WELCOME
; ���Э��ҳ��
;!insertmacro MUI_PAGE_LICENSE "..\hcaservice\License.rtf"
; ��װ����ҳ��
!insertmacro MUI_PAGE_INSTFILES
; ��װ���ҳ��
!define MUI_FINISHPAGE_RUN "$INSTDIR\hcaservice.exe"
!insertmacro MUI_PAGE_FINISH

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

; ��װԤ�ͷ��ļ�
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; administrator rule
RequestExecutionLevel admin


;Ԥ���尲װĿ¼
Function .onInit
	StrCpy $R1 "C:\Program Files\"
	StrCpy $INSTDIR "$R1${PRODUCT_INSTALL_PATH}"
FunctionEnd

; ------ MUI �ִ����涨����� ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "�ƶ���λ��λ��ϵͳ��װ����.exe"
InstallDir "$PROGRAMFILES\hcaservice"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$SMPROGRAMS\�ƶ���λ��λ��ϵͳ"
  CreateShortCut "$SMPROGRAMS\�ƶ���λ��λ��ϵͳ\�ƶ���λ��λ��ϵͳ.lnk" "$INSTDIR\hcaservice.exe"
  CreateShortCut "$DESKTOP\�ƶ���λ��λ��ϵͳ.lnk" "$INSTDIR\hcaservice.exe"

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
  

  #��װ����
  DetailPrint "��װ����"
  SimpleSC::InstallService "hcaservice" "�ƶ���λ��λ��ϵͳ" "16" "2" "C:\Program Files\hcaservice\hcaservice.exe" "" "" ""

  DetailPrint "��������"
  SimpleSC::StartService "hcaservice" "" 30
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\�ƶ���λ��λ��ϵͳ\��˾��ַ.lnk" "$INSTDIR\${PRODUCT_NAME}.url" "${MUI_ICON}"
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
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "��ȷʵҪ��ȫ�Ƴ� $(^Name) ���������е������" IDYES +2
  Abort
FunctionEnd

