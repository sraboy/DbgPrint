#include "DbgPrnHk.h"

#include "..\sdk\CrossNt\CrossNt.h"

#include "binhook.h"

#define HOOK_MAX_x86_OPCODE_SZ     16

#define OPCODE_PREFX        0x80
#define OPCODE_32vs16       0x40
#define OPCODE_LONG_JMP     0x40
#define OPCODE_MOD1         0x20
#define OPCODE_chIP         0x10
#define OPCODE_BASESZ_MASK  0x03

#define OPCODE_INV      0xff
#define OPCODE_DEF(basesz, intarg, modoffs) \
    (((basesz-1) & OPCODE_BASESZ_MASK) | (intarg ? OPCODE_32vs16 : 0) | (modoffs ? OPCODE_MOD1 : 0))

#define OPCODE_Jxx(short_jmp) \
    (OPCODE_chIP | ((1-1) & OPCODE_BASESZ_MASK) | (short_jmp ? 0 : OPCODE_LONG_JMP))

#define OPCODE_JMP(basesz, intarg, modoffs) \
    (OPCODE_chIP | ((basesz-1) & OPCODE_BASESZ_MASK) | (intarg ? OPCODE_32vs16 : 0) | (modoffs ? OPCODE_MOD1 : 0))

static const
UCHAR BaseOpcodeFlags[256] =
{
    OPCODE_DEF(2, FALSE, 1),     /* 0x00: ADD r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x01: ADD r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x02: ADD r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x03: ADD r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x04: ADD i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x05: ADD i16                         */
    OPCODE_DEF(1, FALSE, 0),     /* 0x06: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x07: POP                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x08: OR r                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0x09: OR r                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0x0A: OR r                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0x0B: OR r                            */
    OPCODE_DEF(2, FALSE, 0),     /* 0x0C: OR i8                           */
    OPCODE_DEF(1, TRUE, 0),      /* 0x0D: OR i16                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0x0E: PUSH                            */
    OPCODE_PREFX,                /* 0x0F: Ext Op                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x10: ADC r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x11: ADC r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x12: ADC r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x13: ADC r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x14: ADC i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x15: ADC i32                         */
    OPCODE_DEF(1, FALSE, 0),     /* 0x16: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x17: POP                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x18: SBB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x19: SBB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x1A: SBB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x1B: SBB r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x1C: SBB i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x1D: SBB i32                         */
    OPCODE_DEF(1, FALSE, 0),     /* 0x1E: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x1F: POP                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x20: AND r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x21: AND r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x22: AND r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x23: AND r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x24: AND i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x25: AND i32                         */
    OPCODE_PREFX,                /* 0x26: ES prefix                       */
    OPCODE_DEF(1, FALSE, 0),     /* 0x27: DAA                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x28: SUB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x29: SUB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x2A: SUB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x2B: SUB r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x2C: SUB i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x2D: SUB i32                         */
    OPCODE_PREFX,                /* 0x2E: CS prefix                       */
    OPCODE_DEF(1, FALSE, 0),     /* 0x2F: DAS                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x30: XOR r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x31: XOR r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x32: XOR r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x33: XOR r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x34: XOR i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x35: XOR i32                         */
    OPCODE_PREFX,                /* 0x36: SS prefix                       */
    OPCODE_DEF(1, FALSE, 0),     /* 0x37: AAA                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x38: CMP r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x39: CMP r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x3A: CMP r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x3B: CMP r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x3C: CMP i8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0x3D: CMP i32                         */
    OPCODE_PREFX,                /* 0x3E: DS prefix                       */
    OPCODE_DEF(1, FALSE, 0),     /* 0x3F: AAS                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x40: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x41: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x42: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x43: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x44: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x45: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x46: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x47: INC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x48: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x49: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4A: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4B: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4C: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4D: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4E: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x4F: DEC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x50: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x51: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x52: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x53: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x54: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x55: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x56: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x57: PUSH                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x58: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x59: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5A: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5B: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5C: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5D: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5E: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x5F: POP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x60: PUSHAD                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0x61: POPAD                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x62: BOUND r                         */
    OPCODE_DEF(2, FALSE, 1),     /* 0x63: ARPL r                          */
    OPCODE_PREFX,                /* 0x64: FS prefix                       */
    OPCODE_PREFX,                /* 0x65: GS prefix                       */
    OPCODE_PREFX,                /* 0x66: im size prefix                  */
    OPCODE_PREFX,                /* 0x67: addr size prefix                */
    OPCODE_DEF(1, TRUE, 0),      /* 0x68: PUSH                            */
    OPCODE_DEF(2, TRUE, 1),      /* 0x69:                                 */
    OPCODE_DEF(2, FALSE, 0),     /* 0x6A: PUSH                            */
    OPCODE_DEF(3, FALSE, 1),     /* 0x6B: IMUL r  i8                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0x6C: INS                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x6D: INS                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x6E: OUTS/OUTSB                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0x6F: OUTS/OUTSW                      */
    OPCODE_Jxx(TRUE),            /* 0x70: JO                              */
    OPCODE_Jxx(TRUE),            /* 0x71: JNO                             */
    OPCODE_Jxx(TRUE),            /* 0x72: JB/JC/JNAE                      */
    OPCODE_Jxx(TRUE),            /* 0x73: JAE/JNB/JNC                     */
    OPCODE_Jxx(TRUE),            /* 0x74: JE/JZ                           */
    OPCODE_Jxx(TRUE),            /* 0x75: JNE/JNZ                         */
    OPCODE_Jxx(TRUE),            /* 0x76: JBE/JNA                         */
    OPCODE_Jxx(TRUE),            /* 0x77: JA/JNBE                         */
    OPCODE_Jxx(TRUE),            /* 0x78: JS                              */
    OPCODE_Jxx(TRUE),            /* 0x79: JNS                             */
    OPCODE_Jxx(TRUE),            /* 0x7A: JP/JPE                          */
    OPCODE_Jxx(TRUE),            /* 0x7B: JNP/JPO                         */
    OPCODE_Jxx(TRUE),            /* 0x7C: JL/JNGE                         */
    OPCODE_Jxx(TRUE),            /* 0x7D: JGE/JNL                         */
    OPCODE_Jxx(TRUE),            /* 0x7E: JLE/JNG                         */
    OPCODE_Jxx(TRUE),            /* 0x7F: JG/JNLE                         */
    OPCODE_DEF(3, FALSE, 1),     /* 0x80: ADC(2)ib,                       */
    OPCODE_DEF(2, TRUE, 1),      /* 0x81:                                 */
    OPCODE_DEF(2, FALSE, 0),     /* 0x82: MOV al                          */
    OPCODE_DEF(3, FALSE, 1),     /* 0x83: ADC(2)ib                        */
    OPCODE_DEF(2, FALSE, 1),     /* 0x84: TEST r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x85: TEST r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x86: XCHG r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x87: XCHG r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x88: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x89: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8A: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8B: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8C: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8D: LEA r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8E: MOV r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x8F: POP ?                           */
    OPCODE_DEF(1, FALSE, 0),     /* 0x90: NOP                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0x91: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x92: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x93: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x94: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x95: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x96: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x97: XCHG                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x98: CWDE                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x99: CDQ                             */
    OPCODE_INV,                  /* 0x9A: CALL cp                         */
    OPCODE_DEF(1, FALSE, 0),     /* 0x9B: WAIT/FWAIT                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0x9C: PUSHFD                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0x9D: POPFD                           */
    OPCODE_DEF(1, FALSE, 0),     /* 0x9E: SAHF                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0x9F: LAHF                            */
    OPCODE_DEF(1, TRUE, 0),      /* 0xA0: MOV                             */
    OPCODE_DEF(1, TRUE, 0),      /* 0xA1: MOV                             */
    OPCODE_DEF(1, TRUE, 0),      /* 0xA2: MOV                             */
    OPCODE_DEF(1, TRUE, 0),      /* 0xA3: MOV                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xA4: MOVS                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0xA5: MOVS/MOVSD                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xA6: CMPS/CMPSB                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xA7: CMPS/CMPSW                      */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA8: TEST                            */
    OPCODE_DEF(1, TRUE, 0),      /* 0xA9: TEST                            */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAA: STOS/STOSB                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAB: STOS/STOSW                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAC: LODS/LODSB                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAD: LODS/LODSW                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAE: SCAS/SCASB                      */
    OPCODE_DEF(1, FALSE, 0),     /* 0xAF: SCAS/SCASD                      */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB0: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB1: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB2: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB3: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB4: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB5: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB6: MOV r8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xB7: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xB8: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xB9: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBA: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBB: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBC: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBD: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBE: MOV r8                          */
    OPCODE_DEF(1, TRUE, 0),      /* 0xBF: MOV r8                          */
    OPCODE_DEF(3, FALSE, 1),     /* 0xC0: RCL i8                          */
    OPCODE_DEF(3, FALSE, 1),     /* 0xC1: RCL i8                          */
    OPCODE_INV,                  /* 0xC2: RET                             */
    OPCODE_INV,                  /* 0xC3: RET                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC4: LES                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC5: LDS                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC6: MOV                             */
    OPCODE_DEF(2, TRUE, 1),      /* 0xC7: MOV                             */
    OPCODE_DEF(4, FALSE, 0),     /* 0xC8: ENTER                           */
    OPCODE_DEF(1, FALSE, 0),     /* 0xC9: LEAVE                           */
    OPCODE_INV,                  /* 0xCA: RET                             */
    OPCODE_INV,                  /* 0xCB: RET                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xCC: INT 3                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCD: INT i8                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0xCE: INTO                            */
    OPCODE_INV,                  /* 0xCF: IRET                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD0: RCL/2                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD1: RCL/2                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD2: RCL/2                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD3: RCL/2                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xD4: AAM                             */
    OPCODE_DEF(2, FALSE, 0),     /* 0xD5: AAD                             */
    OPCODE_INV,                  /* 0xD6:                                 */
    OPCODE_DEF(1, FALSE, 0),     /* 0xD7: XLAT/XLATB                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD8: FADD                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD9: F2XM1                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDA: FLADD                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDB: FCLEX                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDC: FADD/0                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDD: FFREE                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDE: FADDP                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDF: FBLD                            */
    OPCODE_Jxx(TRUE),            /* 0xE0: LOOPNE                          */
    OPCODE_Jxx(TRUE),            /* 0xE1: LOOPE                           */
    OPCODE_Jxx(TRUE),            /* 0xE2: LOOP                            */
    OPCODE_Jxx(TRUE),            /* 0xE3: JCXZ/JECXZ                      */
    OPCODE_DEF(2, FALSE, 0),     /* 0xE4: IN i8                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xE5: IN i32                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xE6: OUT i8                          */
    OPCODE_DEF(2, FALSE, 0),     /* 0xE7: OUT i32                         */
    OPCODE_JMP(1, TRUE, 0),      /* 0xE8: CALL cd // need addr adjustment */
    OPCODE_JMP(1, TRUE, 0),      /* 0xE9: JMP cd  // ---    //    ----    */
    OPCODE_JMP(3, TRUE, 0),      /* 0xEA: JMP FAR                         */
    OPCODE_JMP(2, FALSE, 0),     /* 0xEB: JMP sh                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0xEC: IN i8                           */
    OPCODE_DEF(1, FALSE, 0),     /* 0xED: IN i32                          */
    OPCODE_DEF(1, FALSE, 0),     /* 0xEE: OUT                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xEF: OUT                             */
    OPCODE_PREFX,                /* 0xF0: LOCK prefix                     */
    OPCODE_INV,                  /* 0xF1:                                 */
    OPCODE_PREFX,                /* 0xF2: REPNE prefix                    */
    OPCODE_PREFX,                /* 0xF3: REPE prefix                     */
    OPCODE_DEF(1, FALSE, 0),     /* 0xF4: HLT                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xF5: CMC                             */
    OPCODE_PREFX,                /* 0xF6: TEST, DIV                       */
    OPCODE_PREFX,                /* 0xF7: TEST, DIV                       */
    OPCODE_DEF(1, FALSE, 0),     /* 0xF8: CLC                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xF9: STC                             */
    OPCODE_INV,                  /* 0xFA: CLI                             */
    OPCODE_INV,                  /* 0xFB: STI                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xFC: CLD                             */
    OPCODE_DEF(1, FALSE, 0),     /* 0xFD: STD                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xFE: DEC,INC                         */
    OPCODE_PREFX                 /* 0xFF: JMP prefix                      */

};

static const
UCHAR Ext_0F_OpcodeFlags[256] =
{
    OPCODE_DEF(2, FALSE, 1),     /* 0x00: LLDT                                */
    OPCODE_DEF(2, FALSE, 1),     /* 0x01: INVLPG                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x02: LAR r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x03: LSL r                               */
    OPCODE_INV,                  /* 0x04:                                     */
    OPCODE_INV,                  /* 0x05:                                     */
    OPCODE_DEF(2, FALSE, 0),     /* 0x06: CLTS                                */
    OPCODE_INV,                  /* 0x07:                                     */
    OPCODE_DEF(2, FALSE, 0),     /* 0x08: INVD                                */
    OPCODE_DEF(2, FALSE, 0),     /* 0x09: WBINVD                              */
    OPCODE_INV,                  /* 0x0A:                                     */
    OPCODE_DEF(2, FALSE, 0),     /* 0x0B: UD2                                 */
    OPCODE_INV,                  /* 0x0C:                                     */
    OPCODE_INV,                  /* 0x0D:                                     */
    OPCODE_INV,                  /* 0x0E:                                     */
    OPCODE_INV,                  /* 0x0F:                                     */
    OPCODE_INV,                  /* 0x10:                                     */
    OPCODE_INV,                  /* 0x11:                                     */
    OPCODE_INV,                  /* 0x12:                                     */
    OPCODE_INV,                  /* 0x13:                                     */
    OPCODE_INV,                  /* 0x14:                                     */
    OPCODE_INV,                  /* 0x15:                                     */
    OPCODE_INV,                  /* 0x16:                                     */
    OPCODE_INV,                  /* 0x17:                                     */
    OPCODE_INV,                  /* 0x18:                                     */
    OPCODE_INV,                  /* 0x19:                                     */
    OPCODE_INV,                  /* 0x1A:                                     */
    OPCODE_INV,                  /* 0x1B:                                     */
    OPCODE_INV,                  /* 0x1C:                                     */
    OPCODE_INV,                  /* 0x1D:                                     */
    OPCODE_INV,                  /* 0x1E:                                     */
    OPCODE_INV,                  /* 0x1F:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x20: MOV r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x21: MOV r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x22: MOV r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x23: MOV r                               */
    OPCODE_INV,                  /* 0x24:                                     */
    OPCODE_INV,                  /* 0x25:                                     */
    OPCODE_INV,                  /* 0x26:                                     */
    OPCODE_INV,                  /* 0x27:                                     */
    OPCODE_INV,                  /* 0x28:                                     */
    OPCODE_INV,                  /* 0x29:                                     */
    OPCODE_INV,                  /* 0x2A:                                     */
    OPCODE_INV,                  /* 0x2B:                                     */
    OPCODE_INV,                  /* 0x2C:                                     */
    OPCODE_INV,                  /* 0x2D:                                     */
    OPCODE_INV,                  /* 0x2E:                                     */
    OPCODE_INV,                  /* 0x2F:                                     */
    OPCODE_DEF(2, FALSE, 0),     /* 0x30: WRMSR                               */
    OPCODE_DEF(2, FALSE, 0),     /* 0x31: RDTSC                               */
    OPCODE_DEF(2, FALSE, 0),     /* 0x32: RDMSR                               */
    OPCODE_DEF(2, FALSE, 0),     /* 0x33: RDPMC                               */
    OPCODE_DEF(2, FALSE, 0),     /* 0x34: SYSENTER                            */
    OPCODE_DEF(2, FALSE, 0),     /* 0x35: SYSEXIT                             */
    OPCODE_INV,                  /* 0x36:                                     */
    OPCODE_INV,                  /* 0x37:                                     */
    OPCODE_INV,                  /* 0x38:                                     */
    OPCODE_INV,                  /* 0x39:                                     */
    OPCODE_INV,                  /* 0x3A:                                     */
    OPCODE_INV,                  /* 0x3B:                                     */
    OPCODE_INV,                  /* 0x3C:                                     */
    OPCODE_INV,                  /* 0x3D:                                     */
    OPCODE_INV,                  /* 0x3E:                                     */
    OPCODE_INV,                  /* 0x3F:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x40: CMOVO                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x41: CMOVNO                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x42: CMOVB, CMOVNE                       */
    OPCODE_DEF(2, FALSE, 1),     /* 0x43: CMOVAE, CMOVNB                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x44: CMOVE, CMOVZ                        */
    OPCODE_DEF(2, FALSE, 1),     /* 0x45: CMOVNE, CMOVNZ                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x46: CMOVBE, CMOVNA                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x47: CMOVA, CMOVNBE                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x48: CMOVS                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x49: CMOVNS                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4A: CMOVP, CMOVPE                       */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4B: CMOVNP, CMOVPO                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4C: CMOVL, CMOVNGE                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4D: CMOVGE, CMOVNL                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4E: CMOVLE, CMOVNG                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x4F: CMOVG, CMOVNLE                      */
    OPCODE_INV,                  /* 0x50:                                     */
    OPCODE_INV,                  /* 0x51:                                     */
    OPCODE_INV,                  /* 0x52:                                     */
    OPCODE_INV,                  /* 0x53:                                     */
    OPCODE_INV,                  /* 0x54:                                     */
    OPCODE_INV,                  /* 0x55:                                     */
    OPCODE_INV,                  /* 0x56:                                     */
    OPCODE_INV,                  /* 0x57:                                     */
    OPCODE_INV,                  /* 0x58:                                     */
    OPCODE_INV,                  /* 0x59:                                     */
    OPCODE_INV,                  /* 0x5A:                                     */
    OPCODE_INV,                  /* 0x5B:                                     */
    OPCODE_INV,                  /* 0x5C:                                     */
    OPCODE_INV,                  /* 0x5D:                                     */
    OPCODE_INV,                  /* 0x5E:                                     */
    OPCODE_INV,                  /* 0x5F:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x60: PUNPCKLBW r                         */
    OPCODE_INV,                  /* 0x61:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x62: PUNPCKLWD r                         */
    OPCODE_DEF(2, FALSE, 1),     /* 0x63: PACKSSWB r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x64: PCMPGTB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x65: PCMPGTW r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x66: PCMPGTD r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x67: PACKUSWB r                          */
    OPCODE_DEF(2, FALSE, 1),     /* 0x68: PUNPCKHBW r                         */
    OPCODE_DEF(2, FALSE, 1),     /* 0x69: PUNPCKHWD r                         */
    OPCODE_DEF(2, FALSE, 1),     /* 0x6A: PUNPCKHDQ r                         */
    OPCODE_DEF(2, FALSE, 1),     /* 0x6B: PACKSSDW r                          */
    OPCODE_INV,                  /* 0x6C:                                     */
    OPCODE_INV,                  /* 0x6D:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x6E: MOVD r                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x6F: MOV r                               */
    OPCODE_INV,                  /* 0x70:                                     */
    OPCODE_DEF(3, FALSE, 1),     /* 0x71: PSLLW i8,PSRAW i8,PSRLW i8          */
    OPCODE_DEF(3, FALSE, 1),     /* 0x72: PSLLD i8,PSRAD i8,PSRLD i8          */
    OPCODE_DEF(3, FALSE, 1),     /* 0x73: PSLLQ i8,PSRLQ i8                   */
    OPCODE_DEF(2, FALSE, 1),     /* 0x74: PCMPEQB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x75: PCMPEQW r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0x76: PCMPEQD r                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0x77: EMMS                                */
    OPCODE_INV,                  /* 0x78:                                     */
    OPCODE_INV,                  /* 0x79:                                     */
    OPCODE_INV,                  /* 0x7A:                                     */
    OPCODE_INV,                  /* 0x7B:                                     */
    OPCODE_INV,                  /* 0x7C:                                     */
    OPCODE_INV,                  /* 0x7D:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0x7E: MOVD r                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x7F: MOV r                               */
    OPCODE_Jxx(FALSE),           /* 0x80: JO                                  */
    OPCODE_Jxx(FALSE),           /* 0x81: JNO                                 */
    OPCODE_Jxx(FALSE),           /* 0x82: JB,JC,JNAE                          */
    OPCODE_Jxx(FALSE),           /* 0x83: JAE,JNB,JNC                         */
    OPCODE_Jxx(FALSE),           /* 0x84: JE,JZ,JZ                            */
    OPCODE_Jxx(FALSE),           /* 0x85: JNE,JNZ                             */
    OPCODE_Jxx(FALSE),           /* 0x86: JBE,JNA                             */
    OPCODE_Jxx(FALSE),           /* 0x87: JA,JNBE                             */
    OPCODE_Jxx(FALSE),           /* 0x88: JS                                  */
    OPCODE_Jxx(FALSE),           /* 0x89: JNS                                 */
    OPCODE_Jxx(FALSE),           /* 0x8A: JP,JPE                              */
    OPCODE_Jxx(FALSE),           /* 0x8B: JNP,JPO                             */
    OPCODE_Jxx(FALSE),           /* 0x8C: JL,NGE                              */
    OPCODE_Jxx(FALSE),           /* 0x8D: JGE,JNL                             */
    OPCODE_Jxx(FALSE),           /* 0x8E: JLE,JNG                             */
    OPCODE_Jxx(FALSE),           /* 0x8F: JG,JNLE                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0x90: CMOVO                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x91: CMOVNO                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x92: CMOVB, CMOVC, CMOVNAE               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x93: CMOVAE, CMOVNB, CMOVNC              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x94: CMOVE, CMOVZ                        */
    OPCODE_DEF(2, FALSE, 1),     /* 0x95: CMOVNE, CMOVNZ                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x96: CMOVBE, CMOVNA                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x97: CMOVA, CMOVNBE                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x98: CMOVS                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0x99: CMOVNS                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9A: CMOVP, CMOVPE                       */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9B: CMOVNP, CMOVPO                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9C: CMOVL, CMOVNGE                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9D: CMOVGE, CMOVNL                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9E: CMOVLE, CMOVNG                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0x9F: CMOVG, CMOVNLE                      */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA0: PUSH                                */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA1: POP                                 */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA2: CPUID                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0xA3: BT                                  */
    OPCODE_DEF(3, FALSE, 1),     /* 0xA4: SHLD                                */
    OPCODE_DEF(2, FALSE, 1),     /* 0xA5: SHLD                                */
    OPCODE_INV,                  /* 0xA6:                                     */
    OPCODE_INV,                  /* 0xA7:                                     */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA8: PUSH                                */
    OPCODE_DEF(2, FALSE, 0),     /* 0xA9: POP                                 */
    OPCODE_DEF(2, FALSE, 0),     /* 0xAA: RSM                                 */
    OPCODE_DEF(2, FALSE, 1),     /* 0xAB: BTS                                 */
    OPCODE_DEF(3, FALSE, 1),     /* 0xAC: SHRD                                */
    OPCODE_DEF(2, FALSE, 1),     /* 0xAD: SHRD                                */
    OPCODE_DEF(2, FALSE, 1),     /* 0xAE: FXRSTOR,FXSAVE                      */
    OPCODE_DEF(2, FALSE, 1),     /* 0xAF: IMUL                                */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB0: CMPXCHG                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB1: CMPXCHG                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB2: LSS r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB3: BTR                                 */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB4: LFS r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB5: LGS r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB6: MOVZX r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xB7: MOVZX r                             */
    OPCODE_INV,                  /* 0xB8:                                     */
    OPCODE_INV,                  /* 0xB9:                                     */
    OPCODE_DEF(3, FALSE, 1),     /* 0xBA: BT, BTC, BTR, BTS                   */
    OPCODE_DEF(2, FALSE, 1),     /* 0xBB: BTC                                 */
    OPCODE_DEF(2, FALSE, 1),     /* 0xBC: BSF                                 */
    OPCODE_DEF(2, FALSE, 1),     /* 0xBD: BSR                                 */
    OPCODE_DEF(2, FALSE, 1),     /* 0xBE: MOVSX r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xBF: MOVSX r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC0: XADD r                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC1: XADD r                              */
    OPCODE_INV,                  /* 0xC2:                                     */
    OPCODE_INV,                  /* 0xC3:                                     */
    OPCODE_INV,                  /* 0xC4:                                     */
    OPCODE_INV,                  /* 0xC5:                                     */
    OPCODE_INV,                  /* 0xC6:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xC7: CMPXCHG8B                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xC8: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xC9: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCA: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCB: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCC: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCD: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCE: BSWAP r32                           */
    OPCODE_DEF(2, FALSE, 0),     /* 0xCF: BSWAP r32                           */
    OPCODE_INV,                  /* 0xD0:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD1: PSRLW r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD2: PSRLD r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD3: PSRLQ r                             */
    OPCODE_INV,                  /* 0xD4:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD5: PMULLW r                            */
    OPCODE_INV,                  /* 0xD6:                                     */
    OPCODE_INV,                  /* 0xD7:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD8: PSUBUSB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xD9: PSUBUSW r                           */
    OPCODE_INV,                  /* 0xDA:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDB: PAND r                              */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDC: PADDUSB r                           */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDD: PADDUSW r                           */
    OPCODE_INV,                  /* 0xDE:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xDF: PANDN r                             */
    OPCODE_INV,                  /* 0xE0:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xE1: PSRAW r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xE2: PSRAD r                             */
    OPCODE_INV,                  /* 0xE3:                                     */
    OPCODE_INV,                  /* 0xE4:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xE5: PMULHW r                            */
    OPCODE_INV,                  /* 0xE6:                                     */
    OPCODE_INV,                  /* 0xE7:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xE8: PSUBB r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xE9: PSUBW r                             */
    OPCODE_INV,                  /* 0xEA:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xEB: POR r                               */
    OPCODE_DEF(2, FALSE, 1),     /* 0xEC: PADDSB r                            */
    OPCODE_DEF(2, FALSE, 1),     /* 0xED: PADDSW r                            */
    OPCODE_INV,                  /* 0xEE:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xEF: PXOR r                              */
    OPCODE_INV,                  /* 0xF0:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF1: PSLLW r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF2: PSLLD r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF3: PSLLQ r                             */
    OPCODE_INV,                  /* 0xF4:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF5: PMADDWD r                           */
    OPCODE_INV,                  /* 0xF6:                                     */
    OPCODE_INV,                  /* 0xF7:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF8: PSUBB r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xF9: PSUBW r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xFA: PSUBD r                             */
    OPCODE_INV,                  /* 0xFB:                                     */
    OPCODE_DEF(2, FALSE, 1),     /* 0xFC: PADDB r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xFD: PADDW r                             */
    OPCODE_DEF(2, FALSE, 1),     /* 0xFE: PADDD r                             */
    OPCODE_INV                   /* 0xFF:                                     */
};


ULONG
HookGetInstrSz(
    IN PUCHAR  pOp, 
    //IN ULONG   prev_sz // must be 0 on top-level call
    //IN BOOLEAN x32vs16 // must be FALSE==32bit on top-level call 
   OUT PULONG  pOpFlags
    )
{
    ULONG opsz = 0;
    UCHAR opc;
    UCHAR OpcodeFlags;
    BOOLEAN Mod1 = FALSE;

    //ULONG x32vs16_sz;

    // read opcode
    opc = pOp[0];
    // read corresponding flags
    OpcodeFlags = BaseOpcodeFlags[opc];

    if(OpcodeFlags == OPCODE_INV) {
        // this opcode cannot be decoded or thunk'ed
        // stop our walk
        return -1;
    }

    if(pOpFlags) {
        (*pOpFlags) = OpcodeFlags;
    }

    if(OpcodeFlags & OPCODE_chIP) {
        switch(OpcodeFlags) {
        case OPCODE_chIP:
            return 2;
        case (OPCODE_chIP | OPCODE_LONG_JMP):
            return 5;
        default:
            return -1;
        }
    }

    if(OpcodeFlags == OPCODE_PREFX) {

        UCHAR pref;
        
        opsz++;
        pOp++;
        pref = opc;
        opc = pOp[0];
        OpcodeFlags = BaseOpcodeFlags[opc];
        // decode prefixes
        switch (pref) {
        case 0x0F:

            OpcodeFlags = Ext_0F_OpcodeFlags[opc];
            opsz += (OpcodeFlags & OPCODE_BASESZ_MASK) + 1 + ((OpcodeFlags & OPCODE_32vs16) ? 4 : 0);
            break;

        case 0x66:
        case 0x67:

            opsz += (OpcodeFlags & OPCODE_BASESZ_MASK) + 1 + ((OpcodeFlags & OPCODE_32vs16) ? 2 : 0);
            break;

        case 0xF6:
        case 0xF7:
            // reg (bits 3-5) of ModR/M == 0
            opsz += 1 + ((opc & 0x38) ? 0 : 2*(pref-0xF6+1));
            Mod1 = TRUE;
            break;

        case 0xFF:

            opsz = 2;
            Mod1 = TRUE;
            break;

        // CD/DS/ES/SS ...
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
        // ... and other prefixes
        default:
            opsz += HookGetInstrSz(pOp, pOpFlags);
            if(opsz > HOOK_MAX_x86_OPCODE_SZ)
                return -1;
            return opsz;
        }
    } else {
        opsz += (OpcodeFlags & OPCODE_BASESZ_MASK) + 1 + ((OpcodeFlags & OPCODE_32vs16) ? 4 : 0);
    }

    Mod1 |= (OpcodeFlags & OPCODE_MOD1) ? TRUE : FALSE;

    if(Mod1) {
        UCHAR mod_rm = pOp[1];

        switch(mod_rm & 0xc0) {
        case 0xC0:
            // do nothing
            break;
        case 0x80: {
            opsz = +3;
        }
        case 0x40: {
            opsz ++;
            if((mod_rm & 0x07) == 0x04) {
                opsz ++;
            }
            break;
        }
        case 0x00: {
            if((mod_rm & 0x07) == 0x05) {
                opsz += 4;
            } else
            if((mod_rm & 0x07) == 0x04) {
                opsz ++;
                if((pOp[2] & 0x07) == 0x05) {
                    
                    switch (mod_rm & 0xC0) {
                    case 0x40:
                        opsz++;
                        break;

                    case 0x00:
                    case 0x80:
                        opsz += 4;
                        break;
                    }
                }
            }
            break;
        }
        }

    }

    return opsz;
} // end HookGetInstrSz()

typedef struct _HOOK_CONTEXT {

    UCHAR saved_op[HOOK_MAX_x86_OPCODE_SZ*4];
    PVOID patchPtr;
    ULONG patch_sz;
    ULONG counter;
    ULONG signature;

} HOOK_CONTEXT, *PHOOK_CONTEXT;

PVOID
HookAtPtr(
    PVOID _ptr,
    PVOID hook_entry
    )
{
    ULONG sz=0;
    ULONG n;
    PHOOK_CONTEXT ctx;
    ULONG OpFlags;

    KdPrint(("HookAtPtr: target %#x, hook %#x\n", _ptr, hook_entry));

    if(!_ptr)
        return NULL;

    _ptr = CrNtSkipImportStub(_ptr);

    if(!_ptr)
        return NULL;

    ctx = (PHOOK_CONTEXT)ExAllocatePool(NonPagedPool, sizeof(HOOK_CONTEXT));
    if(!ctx)
        return NULL;

    memset(ctx->saved_op, 0x90, sizeof(ctx->saved_op));
    ctx->patchPtr = _ptr;

    while(sz < 5 /* size of JMP xxx */) {
        n = HookGetInstrSz((PUCHAR)_ptr + sz, &OpFlags);
        if(n == -1) {
            ExFreePool(ctx);
            return NULL;
        }
        if(OpFlags & OPCODE_chIP) {
            ExFreePool(ctx);
            return NULL;
        } else {
            memcpy(ctx->saved_op+sz, _ptr, n);
        }
        sz += n;
    }

    ctx->saved_op[sz] = 0xE9; // JMP
    ctx->patch_sz = sz;
    ((PULONG)&(ctx->saved_op[sz+1]))[0] = (ULONG)_ptr + sz - (ULONG)&(ctx->saved_op[sz+5]);
    ctx->counter = 0;
    ctx->signature = 'kooH';

    _asm {

        // save flags and disable interrupts for current CPU
        pushfd
        cli

        // Clear WriteProtection bit in CR0:
        push ebx
        push ecx

        mov  ebx, cr0
        push ebx
        and  ebx, ~0x10000  // reset WriteProtect bit
        mov  cr0, ebx

        mov  eax, dword ptr [hook_entry]
        mov  ebx, dword ptr [_ptr]
        sub  eax, ebx
        sub  eax, 5
        mov  byte ptr [ebx],0xE9
        mov  dword ptr [ebx+1],eax

        // Restore WriteProtection bit in CR0:
        pop ebx
        mov cr0, ebx

        pop ecx
        pop ebx

        // restore flags (e.g. interrupt flag)
        popfd
    }

    return ctx;

} // end HookAtPtr()

NTSTATUS
UnHookAtPtr(
    PVOID _ctx,
    ULONG timeout // seconds
    )
{
    PHOOK_CONTEXT ctx = (PHOOK_CONTEXT)_ctx;
    PVOID _ptr;
    LARGE_INTEGER Delay;

    if(!ctx)
        return STATUS_INVALID_PARAMETER;
    if(ctx->signature != 'kooH')
        return STATUS_INVALID_PARAMETER;

    _ptr = ctx->patchPtr;

    _asm {

        // save flags and disable interrupts for current CPU
        pushfd
        cli

        // Clear WriteProtection bit in CR0:
        push ebx
        push ecx

        mov  ebx, cr0
        push ebx
        and  ebx, ~0x10000  // reset WriteProtect bit
        mov  cr0, ebx

        mov  ecx, dword ptr [ctx]
        mov  ebx, dword ptr [_ptr]

        mov  al, byte ptr [ecx]
        mov  byte ptr [ebx],al
        mov  eax, dword ptr [ecx+1]
        mov  dword ptr [ebx+1],eax

        // Restore WriteProtection bit in CR0:
        pop ebx
        mov cr0, ebx

        pop ecx
        pop ebx

        // restore flags (e.g. interrupt flag)
        popfd
    }

    Delay.QuadPart = -1*1000*1000; // 0.1 sec

    timeout *= 10;
    while(ctx->counter && timeout) {
        KdPrint(("UnHookAtPtr: wait for Unpatch...\n"));
        KeDelayExecutionThread(KernelMode, FALSE, &Delay);
    }
    return STATUS_SUCCESS;

} // end UnHookAtPtr()

NTSTATUS
ReleaseHookAtPtr(
    PVOID _ctx
    )
{
    PHOOK_CONTEXT ctx = (PHOOK_CONTEXT)_ctx;

    if(!ctx)
        return STATUS_INVALID_PARAMETER;
    if(ctx->signature != 'kooH')
        return STATUS_INVALID_PARAMETER;
                    	
    if(!ctx->counter) {
        ctx->signature = 0;
        ExFreePool(ctx);
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
} // end ReleaseHookAtPtr()

PUCHAR
PtrRelativeToAbs(
    PVOID f_ptr
    )
{
    return ( (PUCHAR)((ULONG)(f_ptr)+4+(*(PULONG)(f_ptr))) );
} // PtrRelativeToAbs()

