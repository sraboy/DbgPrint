if not exist ..\Dist mkdir ..\Dist
if not exist ..\Dist\src mkdir ..\Dist\src
if not exist ..\Dist\sdk mkdir ..\Dist\sdk
if not exist ..\Dist\sdk\lib mkdir ..\Dist\sdk\lib
if not exist ..\Dist\sdk\lib\Debug   mkdir ..\Dist\sdk\lib\Debug
if not exist ..\Dist\sdk\lib\Release mkdir ..\Dist\sdk\lib\Release
if not exist sdk\lib\Debug   mkdir sdk\lib\Debug
if not exist sdk\lib\Release mkdir sdk\lib\Release

cd sdk
cd postmsg
nmake %1 CFG="PostDbgMesg - Win32 Release"
nmake %1 CFG="PostDbgMesg - Win32 Debug"
nmake %1 CFG="PostDbgMesg - Win32 Native Release"
nmake %1 CFG="PostDbgMesg - Win32 Native Debug"
nmake %1 CFG="PostDbgMesg - Win32 Kernel Release"
nmake %1 CFG="PostDbgMesg - Win32 Kernel Debug"
cd ..
cd formatmsg
nmake %1 CFG="formatmsg - Win32 Release"
nmake %1 CFG="formatmsg - Win32 Debug"
cd ..
cd kdapis
nmake %1 CFG="kdapis - Win32 Release"
nmake %1 CFG="kdapis - Win32 Debug"
cd ..
cd CrossNt
nmake %1 CFG="CrossNt - Win32 Kernel Debug"
nmake %1 CFG="CrossNt - Win32 Kernel Release"
cd ..
cd ..
cd client
nmake %1 CFG="DbgPrintLog - Win32 Debug"
nmake %1 CFG="DbgPrintLog - Win32 Release"
cd ..
cd driver
nmake %1 CFG="DbgPrnHk - Win32 Debug"
nmake %1 CFG="DbgPrnHk - Win32 Release"
cd ..
cd EchoDbg
nmake %1 CFG="EchoDbg - Win32 Debug"
nmake %1 CFG="EchoDbg - Win32 Release"
cd ..
cd dbgext
nmake %1 CFG="dbghk_exts - Win32 Debug"
nmake %1 CFG="dbghk_exts - Win32 Release"
cd ..
copy driver\Release\DbgPrnHk.sys    ..\Dist\DbgPrnHk.sys
copy driver\Release\DbgPrnHk.pdb    ..\Dist\DbgPrnHk.pdb
copy client\Release\DbgPrintLog.exe ..\Dist\DbgPrintLog.exe
copy EchoDbg\Release\EchoDbg.exe    ..\Dist\EchoDbg.exe
copy dbgext\Release\dbgprn.dll      ..\Dist\dbgprn.dll

copy sdk\Lib\Release\PostDbgMesg.lib            ..\Dist\sdk\Lib\PostDbgMesg.lib
copy sdk\Lib\Debug\PostDbgMesg.lib              ..\Dist\sdk\Lib\PostDbgMesgD.lib
copy sdk\Lib\Release\PostDbgMesgN.lib           ..\Dist\sdk\Lib\PostDbgMesgN.lib
copy sdk\Lib\Debug\PostDbgMesgN.lib             ..\Dist\sdk\Lib\PostDbgMesgND.lib
copy sdk\Lib\Release\PostDbgMesgK.lib           ..\Dist\sdk\Lib\PostDbgMesgK.lib
copy sdk\Lib\Debug\PostDbgMesgK.lib             ..\Dist\sdk\Lib\PostDbgMesgKD.lib
copy sdk\postmsg\PostDbgMesg.h                  ..\Dist\sdk\PostDbgMesg.h
copy inc\DbgPrnHk.h                             ..\Dist\sdk\DbgPrnHk.h

copy sdk\Lib\Release\formatmsg.lib              ..\Dist\sdk\Lib\FormatMsg.lib
copy sdk\Lib\Debug\formatmsg.lib                ..\Dist\sdk\Lib\FormatMsgD.lib
copy sdk\formatmsg\fmt_output.h                 ..\Dist\sdk\fmt_output.h

copy sdk\Lib\Release\kdapis.lib                 ..\Dist\sdk\Lib\kdapis.lib
copy sdk\Lib\Debug\kdapis.lib                   ..\Dist\sdk\Lib\kdapisD.lib
copy sdk\kdapis\kdapis.h                        ..\Dist\sdk\kdapis.h
copy sdk\kdapis\windbgkd.h.h                    ..\Dist\sdk\windbgkd.h.h

call bc ..\Dist\src .

copy sdk\Lib\Release\CrossNtK.lib               ..\CrossNt\Lib\Release\CrossNtK.lib
copy sdk\Lib\Debug\CrossNtK.lib                 ..\CrossNt\Lib\Debug\CrossNtKD.lib
copy inc\ntddk_ex.h                             ..\CrossNt\inc\ntddk_ex.h
copy inc\tools.h                                ..\CrossNt\inc\tools.h
copy sdk\CrossNt\*.h                            ..\CrossNt\inc\

call bc ..\CrossNt\src .\sdk\CrossNt
