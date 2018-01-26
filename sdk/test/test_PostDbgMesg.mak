# Microsoft Developer Studio Generated NMAKE File, Based on test_PostDbgMesg.dsp
!IF "$(CFG)" == ""
CFG=test_PostDbgMesg - Win32 Debug
!MESSAGE No configuration specified. Defaulting to test_PostDbgMesg - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "test_PostDbgMesg - Win32 Release" && "$(CFG)" != "test_PostDbgMesg - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test_PostDbgMesg.mak" CFG="test_PostDbgMesg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test_PostDbgMesg - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test_PostDbgMesg - Win32 Debug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "test_PostDbgMesg - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\test_PostDbgMesg.exe"


CLEAN :
	-@erase "$(INTDIR)\test_PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\test_PostDbgMesg.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\test_PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\test_PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PostDbgMesg.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\test_PostDbgMesg.pdb" /machine:I386 /out:"$(OUTDIR)\test_PostDbgMesg.exe" /libpath:"..\Release" 
LINK32_OBJS= \
	"$(INTDIR)\test_PostDbgMesg.obj"

"$(OUTDIR)\test_PostDbgMesg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test_PostDbgMesg - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\test_PostDbgMesg.exe"


CLEAN :
	-@erase "$(INTDIR)\test_PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\test_PostDbgMesg.exe"
	-@erase "$(OUTDIR)\test_PostDbgMesg.ilk"
	-@erase "$(OUTDIR)\test_PostDbgMesg.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\test_PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\test_PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib PostDbgMesg.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\test_PostDbgMesg.pdb" /debug /machine:I386 /out:"$(OUTDIR)\test_PostDbgMesg.exe" /pdbtype:sept /libpath:"..\Debug" 
LINK32_OBJS= \
	"$(INTDIR)\test_PostDbgMesg.obj"

"$(OUTDIR)\test_PostDbgMesg.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
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
!IF EXISTS("test_PostDbgMesg.dep")
!INCLUDE "test_PostDbgMesg.dep"
!ELSE 
!MESSAGE Warning: cannot find "test_PostDbgMesg.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "test_PostDbgMesg - Win32 Release" || "$(CFG)" == "test_PostDbgMesg - Win32 Debug"
SOURCE=.\test_PostDbgMesg.cpp

"$(INTDIR)\test_PostDbgMesg.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

