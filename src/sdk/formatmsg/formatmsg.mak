# Microsoft Developer Studio Generated NMAKE File, Based on formatmsg.dsp
!IF "$(CFG)" == ""
CFG=formatmsg - Win32 Debug
!MESSAGE No configuration specified. Defaulting to formatmsg - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "formatmsg - Win32 Release" && "$(CFG)" != "formatmsg - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "formatmsg.mak" CFG="formatmsg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "formatmsg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "formatmsg - Win32 Debug" (based on "Win32 (x86) Static Library")
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

!IF  "$(CFG)" == "formatmsg - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\formatmsg.lib"


CLEAN :
	-@erase "$(INTDIR)\fmt_output.obj"
	-@erase "$(INTDIR)\formatmsg.pch"
	-@erase "$(INTDIR)\PIDtoPN.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\formatmsg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\client" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\formatmsg.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\formatmsg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\formatmsg.lib" 
LIB32_OBJS= \
	"$(INTDIR)\fmt_output.obj" \
	"$(INTDIR)\PIDtoPN.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\formatmsg.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\formatmsg.lib"
   copy .\Release\*.lib ..\lib\Release
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "formatmsg - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\formatmsg.lib"


CLEAN :
	-@erase "$(INTDIR)\fmt_output.obj"
	-@erase "$(INTDIR)\formatmsg.pch"
	-@erase "$(INTDIR)\PIDtoPN.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\formatmsg.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\client" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\formatmsg.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\formatmsg.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\formatmsg.lib" 
LIB32_OBJS= \
	"$(INTDIR)\fmt_output.obj" \
	"$(INTDIR)\PIDtoPN.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\formatmsg.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\formatmsg.lib"
   copy .\Debug\*.lib ..\lib\Debug
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
!IF EXISTS("formatmsg.dep")
!INCLUDE "formatmsg.dep"
!ELSE 
!MESSAGE Warning: cannot find "formatmsg.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "formatmsg - Win32 Release" || "$(CFG)" == "formatmsg - Win32 Debug"
SOURCE=.\fmt_output.cpp

"$(INTDIR)\fmt_output.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\formatmsg.pch"


SOURCE=.\PIDtoPN.cpp

"$(INTDIR)\PIDtoPN.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\formatmsg.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "formatmsg - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /I "..\..\client" /I "..\..\inc" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\formatmsg.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\formatmsg.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "formatmsg - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\client" /I "..\..\inc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "USER_MODE" /Fp"$(INTDIR)\formatmsg.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\formatmsg.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

