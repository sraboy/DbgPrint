# Microsoft Developer Studio Project File - Name="CrossNt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CrossNt - Win32 Kernel Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CrossNt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CrossNt.mak" CFG="CrossNt - Win32 Kernel Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CrossNt - Win32 Kernel Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CrossNt - Win32 Kernel Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CrossNt - Win32 Kernel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CrossNt___Win32_Kernel_Release"
# PROP BASE Intermediate_Dir "CrossNt___Win32_Kernel_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Kernel_Release"
# PROP Intermediate_Dir "Kernel_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "$(BASEDIR)\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /c
# ADD CPP /nologo /Gz /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "KERNEL_MODE" /D "CROSS_NT_INTERNAL" /D "_X86_" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x422 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Native_Release\CrossNtN.lib"
# ADD LIB32 /nologo /out:"Kernel_Release\CrossNtK.lib"
# Begin Special Build Tool
OutDir=.\Kernel_Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CrossNt - Win32 Kernel Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "CrossNt___Win32_Kernel_Debug"
# PROP BASE Intermediate_Dir "CrossNt___Win32_Kernel_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Kernel_Debug"
# PROP Intermediate_Dir "Kernel_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "KERNEL_MODE" /D "CROSS_NT_INTERNAL" /D "_X86_" /D "DBG" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x422 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Native_Debug\CrossNtN.lib"
# ADD LIB32 /nologo /out:"Kernel_Debug\CrossNtK.lib"
# Begin Special Build Tool
OutDir=.\Kernel_Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(OUTDIR)\*.lib ..\lib\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "CrossNt - Win32 Kernel Release"
# Name "CrossNt - Win32 Kernel Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CrossNt.cpp
# End Source File
# Begin Source File

SOURCE=.\ilock.cpp
# End Source File
# Begin Source File

SOURCE=.\misc_i386.cpp
# End Source File
# Begin Source File

SOURCE=.\rwlock.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CrNtDecl.h
# End Source File
# Begin Source File

SOURCE=.\CrNtStubs.h
# End Source File
# Begin Source File

SOURCE=.\CrossNt.h
# End Source File
# Begin Source File

SOURCE=.\ilock.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\ntddk_ex.h
# End Source File
# Begin Source File

SOURCE=..\..\inc\tools.h
# End Source File
# End Group
# End Target
# End Project
