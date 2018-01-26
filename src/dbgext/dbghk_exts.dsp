# Microsoft Developer Studio Project File - Name="dbghk_exts" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dbghk_exts - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dbghk_exts.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbghk_exts.mak" CFG="dbghk_exts - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbghk_exts - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbghk_exts - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbghk_exts - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "dbghk_exts_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\inc" /D "NDEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 msvcrt.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msvcrt.lib KERNEL32.LIB formatmsg.lib /nologo /base:"0x1000000" /dll /machine:I386 /nodefaultlib /out:"Release/dbgprn.dll" /libpath:"..\sdk\lib\Release" /optidata /noentry /subsystem:console,4.0
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=.\Release\dbgprn.dll
InputPath=.\Release\dbgprn.dll
SOURCE="$(InputPath)"

"conv.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"C:\Program Files\SoftIceNT\KD2SYS.exe" /O $(TargetPath) 
	regedit /s dbghk_exts.reg 
	echo converted >conv.msg 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "dbghk_exts - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "dbghk_exts_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\inc" /D "_DEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 msvcrt.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrt.lib KERNEL32.LIB formatmsg.lib /nologo /base:"0x1000000" /dll /debug /machine:I386 /nodefaultlib /out:"Debug/dbgprn.dll" /pdbtype:sept /libpath:"..\sdk\lib\Debug" /optidata /noentry /subsystem:console,4.0
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=.\Debug\dbgprn.dll
InputPath=.\Debug\dbgprn.dll
SOURCE="$(InputPath)"

"conv.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"C:\Program Files\SoftIceNT\KD2SYS.exe" /O $(TargetPath) 
	regedit /s dbghk_exts.reg 
	echo converted >conv.msg 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "dbghk_exts - Win32 Release"
# Name "dbghk_exts - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dbghk_exts.cpp
# End Source File
# Begin Source File

SOURCE=.\dbghk_exts.def
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\inc\tools.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\client\dbgprint_opt.h
# End Source File
# Begin Source File

SOURCE=..\inc\DbgPrnHk.h
# End Source File
# Begin Source File

SOURCE=..\client\fmt_output_opt.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\inc\tools.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
