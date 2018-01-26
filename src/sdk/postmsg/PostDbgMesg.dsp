# Microsoft Developer Studio Project File - Name="PostDbgMesg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PostDbgMesg - Win32 Kernel Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PostDbgMesg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PostDbgMesg.mak" CFG="PostDbgMesg - Win32 Kernel Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PostDbgMesg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PostDbgMesg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "PostDbgMesg - Win32 Native Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PostDbgMesg - Win32 Native Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "PostDbgMesg - Win32 Kernel Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PostDbgMesg - Win32 Kernel Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PostDbgMesg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x422 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
OutDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x422 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
OutDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Native Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PostDbgMesg___Win32_Native_Release"
# PROP BASE Intermediate_Dir "PostDbgMesg___Win32_Native_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Native_Release"
# PROP Intermediate_Dir "Native_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x422 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Native_Release\PostDbgMesgN.lib"
# Begin Special Build Tool
OutDir=.\Native_Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Native Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PostDbgMesg___Win32_Native_Debug"
# PROP BASE Intermediate_Dir "PostDbgMesg___Win32_Native_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Native_Debug"
# PROP Intermediate_Dir "Native_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x422 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Native_Debug\PostDbgMesgN.lib"
# Begin Special Build Tool
OutDir=.\Native_Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Debug
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Kernel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PostDbgMesg___Win32_Kernel_Release"
# PROP BASE Intermediate_Dir "PostDbgMesg___Win32_Kernel_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Kernel_Release"
# PROP Intermediate_Dir "Kernel_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(BASEDIR)\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /D "KERNEL_MODE" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x422 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Native_Release\PostDbgMesgN.lib"
# ADD LIB32 /nologo /out:"Kernel_Release\PostDbgMesgK.lib"
# Begin Special Build Tool
OutDir=.\Kernel_Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Kernel Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PostDbgMesg___Win32_Kernel_Debug"
# PROP BASE Intermediate_Dir "PostDbgMesg___Win32_Kernel_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Kernel_Debug"
# PROP Intermediate_Dir "Kernel_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /D "KERNEL_MODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x422 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Native_Debug\PostDbgMesgN.lib"
# ADD LIB32 /nologo /out:"Kernel_Debug\PostDbgMesgK.lib"
# Begin Special Build Tool
OutDir=.\Kernel_Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "PostDbgMesg - Win32 Release"
# Name "PostDbgMesg - Win32 Debug"
# Name "PostDbgMesg - Win32 Native Release"
# Name "PostDbgMesg - Win32 Native Debug"
# Name "PostDbgMesg - Win32 Kernel Release"
# Name "PostDbgMesg - Win32 Kernel Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PostDbgMesg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\inc\DbgPrnHk.h
# End Source File
# Begin Source File

SOURCE=.\PostDbgMesg.h
# End Source File
# Begin Source File

SOURCE=..\inc\tools.h
# End Source File
# Begin Source File

SOURCE=..\inc\version.h
# End Source File
# End Group
# End Target
# End Project
