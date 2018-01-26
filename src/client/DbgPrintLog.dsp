# Microsoft Developer Studio Project File - Name="DbgPrintLog" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DbgPrintLog - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DbgPrintLog.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DbgPrintLog.mak" CFG="DbgPrintLog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DbgPrintLog - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DbgPrintLog - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DbgPrintLog - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\inc" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USER_MODE" /D "ARG_OPT_UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x422 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib PostDbgMesg.lib formatmsg.lib kdapis.lib /nologo /subsystem:console /machine:I386 /libpath:"..\sdk\Lib\Release"

!ELSEIF  "$(CFG)" == "DbgPrintLog - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\inc" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USER_MODE" /D "ARG_OPT_UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x422 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib PostDbgMesg.lib formatmsg.lib kdapis.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\sdk\Lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.exe .
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DbgPrintLog - Win32 Release"
# Name "DbgPrintLog - Win32 Debug"
# Begin Source File

SOURCE=.\dbgprint_opt.cpp
# End Source File
# Begin Source File

SOURCE=.\dbgprint_opt.h
# End Source File
# Begin Source File

SOURCE=.\DbgPrintLog.cpp
# End Source File
# Begin Source File

SOURCE=.\DbgPrintLog.h
# End Source File
# Begin Source File

SOURCE=.\flusher.cpp
# End Source File
# Begin Source File

SOURCE=.\flusher.h
# End Source File
# Begin Source File

SOURCE=.\fmt_output_opt.h
# End Source File
# Begin Source File

SOURCE=.\Privilege.cpp
# End Source File
# Begin Source File

SOURCE=.\Privilege.h
# End Source File
# Begin Source File

SOURCE=.\SvcManLib.cpp
# End Source File
# Begin Source File

SOURCE=.\SvcManLib.h
# End Source File
# End Target
# End Project
