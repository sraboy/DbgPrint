# Microsoft Developer Studio Generated NMAKE File, Based on PostDbgMesg.dsp
!IF "$(CFG)" == ""
CFG=PostDbgMesg - Win32 Kernel Debug
!MESSAGE No configuration specified. Defaulting to PostDbgMesg - Win32 Kernel Debug.
!ENDIF 

!IF "$(CFG)" != "PostDbgMesg - Win32 Release" && "$(CFG)" != "PostDbgMesg - Win32 Debug" && "$(CFG)" != "PostDbgMesg - Win32 Native Release" && "$(CFG)" != "PostDbgMesg - Win32 Native Debug" && "$(CFG)" != "PostDbgMesg - Win32 Kernel Release" && "$(CFG)" != "PostDbgMesg - Win32 Kernel Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PostDbgMesg - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesg.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PostDbgMesg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesg.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesg.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Release
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesg.lib"
   copy .\Release\*.lib ..\lib\Release
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesg.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PostDbgMesg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesg.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesg.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Debug
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesg.lib"
   copy .\Debug\*.lib ..\lib\Debug
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Native Release"

OUTDIR=.\Native_Release
INTDIR=.\Native_Release
# Begin Custom Macros
OutDir=.\Native_Release
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesgN.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PostDbgMesgN.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesgN.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesgN.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Native_Release
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Native_Release
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesgN.lib"
   copy .\Native_Release\*.lib ..\lib\Release
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Native Debug"

OUTDIR=.\Native_Debug
INTDIR=.\Native_Debug
# Begin Custom Macros
OutDir=.\Native_Debug
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesgN.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PostDbgMesgN.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesgN.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesgN.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

OutDir=.\Native_Debug
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Native_Debug
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesgN.lib"
   copy .\Native_Debug\*.lib ..\lib\Debug
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Kernel Release"

OUTDIR=.\Kernel_Release
INTDIR=.\Kernel_Release
# Begin Custom Macros
OutDir=.\Kernel_Release
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesgK.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PostDbgMesgK.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "$(BASEDIR)\inc" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /D "KERNEL_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesgK.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesgK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesgK.lib"
   copy .\Kernel_Release\*.lib ..\lib\Release
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "PostDbgMesg - Win32 Kernel Debug"

OUTDIR=.\Kernel_Debug
INTDIR=.\Kernel_Debug
# Begin Custom Macros
OutDir=.\Kernel_Debug
# End Custom Macros

ALL : "$(OUTDIR)\PostDbgMesgK.lib"


CLEAN :
	-@erase "$(INTDIR)\PostDbgMesg.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PostDbgMesgK.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(BASEDIR)\inc" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /D "NATIVE_MODE" /D "KERNEL_MODE" /Fp"$(INTDIR)\PostDbgMesg.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PostDbgMesg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\PostDbgMesgK.lib" 
LIB32_OBJS= \
	"$(INTDIR)\PostDbgMesg.obj"

"$(OUTDIR)\PostDbgMesgK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\PostDbgMesgK.lib"
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
!IF EXISTS("PostDbgMesg.dep")
!INCLUDE "PostDbgMesg.dep"
!ELSE 
!MESSAGE Warning: cannot find "PostDbgMesg.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PostDbgMesg - Win32 Release" || "$(CFG)" == "PostDbgMesg - Win32 Debug" || "$(CFG)" == "PostDbgMesg - Win32 Native Release" || "$(CFG)" == "PostDbgMesg - Win32 Native Debug" || "$(CFG)" == "PostDbgMesg - Win32 Kernel Release" || "$(CFG)" == "PostDbgMesg - Win32 Kernel Debug"
SOURCE=.\PostDbgMesg.cpp

"$(INTDIR)\PostDbgMesg.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

