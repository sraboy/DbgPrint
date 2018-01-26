#ifndef __NT_PS_TOOLS__H__
#define __NT_PS_TOOLS__H__

#ifdef __cplusplus
extern "C" {
#endif

PWCHAR
FindProcessNameW(
    ULONG pid
    );

#ifdef __cplusplus
};
#endif

#endif // __NT_PS_TOOLS__H__
