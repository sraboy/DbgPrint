# Microsoft Developer Studio Generated NMAKE File, Based on DbgPrnHk.dsp
!IF "$(CFG)" == ""
CFG=DbgPrnHk - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DbgPrnHk - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DbgPrnHk - Win32 Release" && "$(CFG)" != "DbgPrnHk - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DbgPrnHk.mak" CFG="DbgPrnHk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DbgPrnHk - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DbgPrnHk - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

#makefile for nt4
BaseDir=$(BASEDIR)
DDKINC=/I $(BaseDir)\inc /I $(UNIATA_ROOT)

!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\DbgPrnHk.sys" ".\copy.msg"


CLEAN :
	-@erase "$(INTDIR)\DbgPrnHk.obj"
	-@erase "$(INTDIR)\pserial.obj"
	-@erase "$(INTDIR)\pfloppy.obj"
	-@erase "$(INTDIR)\pioata.obj"
	-@erase "$(INTDIR)\regtools.obj"
	-@erase "$(INTDIR)\binhook.obj"
	-@erase "$(INTDIR)\vsnprintf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DbgPrnHk.sys"
	-@erase ".\copy.msg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MT /W3 /GX /O2 $(DDKINC) /I"..\inc" /D "UNIATA_CORE" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DbgPrnHk_EXPORTS" /D "_X86_" /D _WIN32_WINNT=0x0400 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DbgPrnHk.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=$(BaseDir)\Lib\i386\Free\ntoskrnl.lib $(BaseDir)\Lib\i386\Free\int64.lib $(BaseDir)\Lib\i386\Checked\Hal.lib ..\sdk\lib\$(OUTDIR)\CrossNtK.lib /nologo /entry:"DriverEntry" /incremental:no /pdb:"$(OUTDIR)\DbgPrnHk.pdb" /machine:I386 /nodefaultlib /def:".\DbgPrnHk.def" /out:"$(OUTDIR)\DbgPrnHk.sys" /driver /subsystem:native /debug /pdb:"$(OUTDIR)\DbgPrnHk.pdb" /pdbtype:sept /opt:ref /opt:icf
DEF_FILE= \
	".\DbgPrnHk.def"
LINK32_OBJS= \
	"$(INTDIR)\regtools.obj" \
	"$(INTDIR)\pserial.obj" \
	"$(INTDIR)\pfloppy.obj" \
	"$(INTDIR)\binhook.obj" \
	"$(INTDIR)\vsnprintf.obj"\
	"$(INTDIR)\DbgPrnHk.obj"

"$(OUTDIR)\DbgPrnHk.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Release\DbgPrnHk.sys
InputPath=.\Release\DbgPrnHk.sys
SOURCE="$(InputPath)"

".\copy.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	copy $(TargetPath) C:\WINNT\System32\Drivers 
	copy $(TargetPath) ..\client
	copy $(TargetPath) ..\client\Release
	echo Driver copied > copy.msg 
<< 
	

!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\DbgPrnHk.sys" "$(OUTDIR)\DbgPrnHk.bsc" ".\copy.msg"


CLEAN :
	-@erase "$(INTDIR)\DbgPrnHk.obj"
	-@erase "$(INTDIR)\DbgPrnHk.sbr"
	-@erase "$(INTDIR)\regtools.obj"
	-@erase "$(INTDIR)\regtools.sbr"
	-@erase "$(INTDIR)\pserial.obj"
	-@erase "$(INTDIR)\pserial.sbr"
	-@erase "$(INTDIR)\pfloppy.obj"
	-@erase "$(INTDIR)\pfloppy.sbr"
	-@erase "$(INTDIR)\pioata.obj"
	-@erase "$(INTDIR)\pioata.sbr"
	-@erase "$(INTDIR)\binhook.obj"
	-@erase "$(INTDIR)\binhook.sbr"
	-@erase "$(INTDIR)\vsnprintf.obj"
	-@erase "$(INTDIR)\vsnprintf.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DbgPrnHk.bsc"
	-@erase "$(OUTDIR)\DbgPrnHk.map"
	-@erase "$(OUTDIR)\DbgPrnHk.pdb"
	-@erase "$(OUTDIR)\DbgPrnHk.sys"
	-@erase ".\copy.msg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MTd /W3 /GX /Z7 /Od $(DDKINC) /I"..\inc" /D "UNIATA_CORE" /D "_DEBUG" /D "DBG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DbgPrnHk_EXPORTS" /D "_X86_" /D _WIN32_WINNT=0x0400 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DbgPrnHk.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DbgPrnHk.sbr"

"$(OUTDIR)\DbgPrnHk.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(BaseDir)\Lib\i386\Checked\ntoskrnl.lib $(BaseDir)\Lib\i386\Checked\int64.lib $(BaseDir)\Lib\i386\Checked\Hal.lib ..\sdk\lib\$(OUTDIR)\CrossNtK.lib /nologo /entry:"DriverEntry" /incremental:no /pdb:"$(OUTDIR)\DbgPrnHk.pdb" /map:"$(INTDIR)\DbgPrnHk.map" /debug /machine:I386 /nodefaultlib /def:".\DbgPrnHk.def" /out:"$(OUTDIR)\DbgPrnHk.sys" /pdbtype:sept /driver /subsystem:native,4.00 
DEF_FILE= \
	".\DbgPrnHk.def"
LINK32_OBJS= \
	"$(INTDIR)\regtools.obj" \
	"$(INTDIR)\pserial.obj" \
	"$(INTDIR)\pfloppy.obj" \
	"$(INTDIR)\binhook.obj" \
	"$(INTDIR)\vsnprintf.obj" \
	"$(INTDIR)\DbgPrnHk.obj"

"$(OUTDIR)\DbgPrnHk.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

TargetPath=.\Debug\DbgPrnHk.sys
InputPath=.\Debug\DbgPrnHk.sys
SOURCE="$(InputPath)"

".\copy.msg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	copy $(TargetPath) C:\WINNT\System32\Drivers 
	copy $(TargetPath) ..\client
	copy $(TargetPath) ..\client\Debug
	echo Driver copied > copy.msg 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("DbgPrnHk.dep")
!INCLUDE "DbgPrnHk.dep"
!ELSE 
!MESSAGE Warning: cannot find "DbgPrnHk.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DbgPrnHk - Win32 Release" || "$(CFG)" == "DbgPrnHk - Win32 Debug"

SOURCE=.\DbgPrnHk.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\DbgPrnHk.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\DbgPrnHk.obj"	"$(INTDIR)\DbgPrnHk.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

SOURCE=.\regtools.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\regtools.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\regtools.obj"	"$(INTDIR)\regtools.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

SOURCE=.\pserial.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\pserial.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\pserial.obj"	"$(INTDIR)\pserial.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

SOURCE=.\pfloppy.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\pfloppy.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\pfloppy.obj"	"$(INTDIR)\pfloppy.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

SOURCE=.\binhook.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\binhook.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\binhook.obj"	"$(INTDIR)\binhook.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

SOURCE=.\vsnprintf.cpp
!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
"$(INTDIR)\vsnprintf.obj" : $(SOURCE) "$(INTDIR)"
!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
"$(INTDIR)\vsnprintf.obj"	"$(INTDIR)\vsnprintf.sbr" : $(SOURCE) "$(INTDIR)"
!ENDIF 

#SOURCE=.\pioata.cpp
#!IF  "$(CFG)" == "DbgPrnHk - Win32 Release"
#"$(INTDIR)\pioata.obj" : $(SOURCE) "$(INTDIR)"
#!ELSEIF  "$(CFG)" == "DbgPrnHk - Win32 Debug"
#"$(INTDIR)\pioata.obj"	"$(INTDIR)\pioata.sbr" : $(SOURCE) "$(INTDIR)"
#!ENDIF 


!ENDIF 

