# Microsoft Developer Studio Generated NMAKE File, Based on CrossNt.dsp
!IF "$(CFG)" == ""
CFG=CrossNt - Win32 Kernel Debug
!MESSAGE No configuration specified. Defaulting to CrossNt - Win32 Kernel Debug.
!ENDIF 

!IF "$(CFG)" != "CrossNt - Win32 Kernel Release" && "$(CFG)" != "CrossNt - Win32 Kernel Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CrossNt - Win32 Kernel Release"

OUTDIR=.\Kernel_Release
INTDIR=.\Kernel_Release
# Begin Custom Macros
OutDir=.\Kernel_Release
# End Custom Macros

ALL : "$(OUTDIR)\CrossNtK.lib"


CLEAN :
	-@erase "$(INTDIR)\CrossNt.obj"
	-@erase "$(INTDIR)\ilock.obj"
	-@erase "$(INTDIR)\misc_i386.obj"
	-@erase "$(INTDIR)\rwlock.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\CrossNtK.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Gz /ML /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "KERNEL_MODE" /D "CROSS_NT_INTERNAL" /D "_X86_" /Fp"$(INTDIR)\CrossNt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CrossNt.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\CrossNtK.lib" 
LIB32_OBJS= \
	"$(INTDIR)\CrossNt.obj" \
	"$(INTDIR)\ilock.obj" \
	"$(INTDIR)\rwlock.obj" \
	"$(INTDIR)\misc_i386.obj"

"$(OUTDIR)\CrossNtK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Kernel_Release
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Kernel_Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\CrossNtK.lib"
   copy .\Kernel_Release\*.lib ..\lib\Release
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "CrossNt - Win32 Kernel Debug"

OUTDIR=.\Kernel_Debug
INTDIR=.\Kernel_Debug
# Begin Custom Macros
OutDir=.\Kernel_Debug
# End Custom Macros

ALL : "$(OUTDIR)\CrossNtK.lib"


CLEAN :
	-@erase "$(INTDIR)\CrossNt.obj"
	-@erase "$(INTDIR)\ilock.obj"
	-@erase "$(INTDIR)\misc_i386.obj"
	-@erase "$(INTDIR)\rwlock.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\CrossNtK.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /Gz /MLd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "KERNEL_MODE" /D "CROSS_NT_INTERNAL" /D "_X86_" /D "DBG" /Fp"$(INTDIR)\CrossNt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\CrossNt.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\CrossNtK.lib" 
LIB32_OBJS= \
	"$(INTDIR)\CrossNt.obj" \
	"$(INTDIR)\ilock.obj" \
	"$(INTDIR)\rwlock.obj" \
	"$(INTDIR)\misc_i386.obj"

"$(OUTDIR)\CrossNtK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Kernel_Debug
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Kernel_Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\CrossNtK.lib"
   copy .\Kernel_Debug\*.lib ..\lib\Debug
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
!IF EXISTS("CrossNt.dep")
!INCLUDE "CrossNt.dep"
!ELSE 
!MESSAGE Warning: cannot find "CrossNt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "CrossNt - Win32 Kernel Release" || "$(CFG)" == "CrossNt - Win32 Kernel Debug"
SOURCE=.\CrossNt.cpp

"$(INTDIR)\CrossNt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ilock.cpp

"$(INTDIR)\ilock.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\misc_i386.cpp

"$(INTDIR)\misc_i386.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\rwlock.cpp

"$(INTDIR)\rwlock.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

