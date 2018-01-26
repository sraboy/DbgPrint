# Microsoft Developer Studio Generated NMAKE File, Based on DbgPrintLog.dsp
!IF "$(CFG)" == ""
CFG=DbgPrintLog - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DbgPrintLog - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DbgPrintLog - Win32 Release" && "$(CFG)" != "DbgPrintLog - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DbgPrintLog - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\DbgPrintLog.exe"


CLEAN :
	-@erase "$(INTDIR)\dbgprint_opt.obj"
	-@erase "$(INTDIR)\DbgPrintLog.obj"
	-@erase "$(INTDIR)\flusher.obj"
	-@erase "$(INTDIR)\Privilege.obj"
	-@erase "$(INTDIR)\SvcManLib.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DbgPrintLog.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\inc" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USER_MODE" /D "ARG_OPT_UNICODE" /Fp"$(INTDIR)\DbgPrintLog.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DbgPrintLog.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib PostDbgMesg.lib formatmsg.lib kdapis.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\DbgPrintLog.pdb" /machine:I386 /out:"$(OUTDIR)\DbgPrintLog.exe" /libpath:"..\sdk\Lib\Release" 
LINK32_OBJS= \
	"$(INTDIR)\dbgprint_opt.obj" \
	"$(INTDIR)\DbgPrintLog.obj" \
	"$(INTDIR)\flusher.obj" \
	"$(INTDIR)\Privilege.obj" \
	"$(INTDIR)\SvcManLib.obj"

"$(OUTDIR)\DbgPrintLog.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DbgPrintLog - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\DbgPrintLog.exe"


CLEAN :
	-@erase "$(INTDIR)\dbgprint_opt.obj"
	-@erase "$(INTDIR)\DbgPrintLog.obj"
	-@erase "$(INTDIR)\flusher.obj"
	-@erase "$(INTDIR)\Privilege.obj"
	-@erase "$(INTDIR)\SvcManLib.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DbgPrintLog.exe"
	-@erase "$(OUTDIR)\DbgPrintLog.ilk"
	-@erase "$(OUTDIR)\DbgPrintLog.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\inc" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USER_MODE" /D "ARG_OPT_UNICODE" /Fp"$(INTDIR)\DbgPrintLog.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DbgPrintLog.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib PostDbgMesg.lib formatmsg.lib kdapis.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\DbgPrintLog.pdb" /debug /machine:I386 /out:"$(OUTDIR)\DbgPrintLog.exe" /pdbtype:sept /libpath:"..\sdk\Lib\Debug" 
LINK32_OBJS= \
	"$(INTDIR)\dbgprint_opt.obj" \
	"$(INTDIR)\DbgPrintLog.obj" \
	"$(INTDIR)\flusher.obj" \
	"$(INTDIR)\Privilege.obj" \
	"$(INTDIR)\SvcManLib.obj"

"$(OUTDIR)\DbgPrintLog.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\DbgPrintLog.exe"
   copy Debug\*.exe .
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("DbgPrintLog.dep")
!INCLUDE "DbgPrintLog.dep"
!ELSE 
!MESSAGE Warning: cannot find "DbgPrintLog.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DbgPrintLog - Win32 Release" || "$(CFG)" == "DbgPrintLog - Win32 Debug"
SOURCE=.\dbgprint_opt.cpp

"$(INTDIR)\dbgprint_opt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DbgPrintLog.cpp

"$(INTDIR)\DbgPrintLog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flusher.cpp

"$(INTDIR)\flusher.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Privilege.cpp

"$(INTDIR)\Privilege.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SvcManLib.cpp

"$(INTDIR)\SvcManLib.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

