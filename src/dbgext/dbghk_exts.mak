# Microsoft Developer Studio Generated NMAKE File, Based on dbghk_exts.dsp
!IF "$(CFG)" == ""
CFG=dbghk_exts - Win32 Release
!MESSAGE No configuration specified. Defaulting to dbghk_exts - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "dbghk_exts - Win32 Release" && "$(CFG)" != "dbghk_exts - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbghk_exts - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dbgprn.dll" ".\conv.msg"


CLEAN :
	-@erase "$(INTDIR)\dbghk_exts.obj"
	-@erase "$(INTDIR)\dbghk_exts.pch"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dbgprn.dll"
	-@erase "$(OUTDIR)\dbgprn.exp"
	-@erase "$(OUTDIR)\dbgprn.lib"
	-@erase "conv.msg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=df.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\inc" /D "NDEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Fp"$(INTDIR)\dbghk_exts.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbghk_exts.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=msvcrt.lib KERNEL32.LIB formatmsg.lib /nologo /base:"0x1000000" /dll /incremental:no /pdb:"$(OUTDIR)\dbgprn.pdb" /machine:I386 /nodefaultlib /def:".\dbghk_exts.def" /out:"$(OUTDIR)\dbgprn.dll" /implib:"$(OUTDIR)\dbgprn.lib" /libpath:"..\sdk\lib\Release" /optidata /noentry /subsystem:console,4.0 
DEF_FILE= \
	".\dbghk_exts.def"
LINK32_OBJS= \
	"$(INTDIR)\dbghk_exts.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tools.obj"

"$(OUTDIR)\dbgprn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Release\dbgprn.dll
InputPath=.\Release\dbgprn.dll
SOURCE="$(InputPath)"

".\conv.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	"C:\Program Files\SoftIceNT\KD2SYS.exe" /O $(TargetPath) 
	regedit /s dbghk_exts.reg 
	echo converted >conv.msg 
<< 
	

!ELSEIF  "$(CFG)" == "dbghk_exts - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dbgprn.dll" ".\conv.msg"


CLEAN :
	-@erase "$(INTDIR)\dbghk_exts.obj"
	-@erase "$(INTDIR)\dbghk_exts.pch"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dbgprn.dll"
	-@erase "$(OUTDIR)\dbgprn.exp"
	-@erase "$(OUTDIR)\dbgprn.ilk"
	-@erase "$(OUTDIR)\dbgprn.lib"
	-@erase "$(OUTDIR)\dbgprn.pdb"
	-@erase "conv.msg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90=df.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\inc" /D "_DEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Fp"$(INTDIR)\dbghk_exts.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbghk_exts.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=msvcrt.lib KERNEL32.LIB formatmsg.lib /nologo /base:"0x1000000" /dll /incremental:yes /pdb:"$(OUTDIR)\dbgprn.pdb" /debug /machine:I386 /nodefaultlib /def:".\dbghk_exts.def" /out:"$(OUTDIR)\dbgprn.dll" /implib:"$(OUTDIR)\dbgprn.lib" /pdbtype:sept /libpath:"..\sdk\lib\Debug" /optidata /noentry /subsystem:console,4.0 
DEF_FILE= \
	".\dbghk_exts.def"
LINK32_OBJS= \
	"$(INTDIR)\dbghk_exts.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\tools.obj"

"$(OUTDIR)\dbgprn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Debug\dbgprn.dll
InputPath=.\Debug\dbgprn.dll
SOURCE="$(InputPath)"

".\conv.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	"C:\Program Files\SoftIceNT\KD2SYS.exe" /O $(TargetPath) 
	regedit /s dbghk_exts.reg 
	echo converted >conv.msg 
<< 
	

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dbghk_exts.dep")
!INCLUDE "dbghk_exts.dep"
!ELSE 
!MESSAGE Warning: cannot find "dbghk_exts.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dbghk_exts - Win32 Release" || "$(CFG)" == "dbghk_exts - Win32 Debug"
SOURCE=.\dbghk_exts.cpp

"$(INTDIR)\dbghk_exts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbghk_exts.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "dbghk_exts - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /I "..\inc" /D "_DEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Fp"$(INTDIR)\dbghk_exts.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\dbghk_exts.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbghk_exts - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\inc" /D "_DEBUG" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_X86_" /D "_WINDLL" /D "USER_MODE" /Fp"$(INTDIR)\dbghk_exts.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\dbghk_exts.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\inc\tools.cpp

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbghk_exts.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

