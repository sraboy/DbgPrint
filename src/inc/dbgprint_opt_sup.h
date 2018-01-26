
#include "arg_opt_unicode.h"

/**** declare groupped options structure ****/
#ifdef DBGPRINT_OPT_STRUCT

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)        struct {
  #define  DBGPRINT_OPT_SECTION_END(sect)          } sect;

  #define  DBGPRINT_OPT(type, name, defval)        type name;
  #define  DBGPRINT_OPT_DECL(type, name)           type name;
  #define  DBGPRINT_DRV_OPT(type, name, defval)    ULONG name; \
                                                   ULONG name##_upd;

  #undef DBGPRINT_OPT_STRUCT

#endif //DBGPRINT_OPT_STRUCT

/**** declare ungroupped options structure (aliases groupped version) ****/
#ifdef DBGPRINT_OPT_RAW_STRUCT

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #define  DBGPRINT_OPT(type, name, defval)        type name;
  #define  DBGPRINT_OPT_DECL(type, name)           type name;
  #define  DBGPRINT_DRV_OPT(type, name, defval)    ULONG name; \
                                                   ULONG name##_upd;

  #undef DBGPRINT_OPT_RAW_STRUCT

#endif //DBGPRINT_OPT_RAW_STRUCT

/**** init entire structure or driver option part ****/
#if defined(DBGPRINT_OPT_RAW_INIT) || defined(DBGPRINT_DRV_OPT_RAW_INIT)

  #ifdef   DBGPRINT_DRV_OPT_RAW_INIT_WITH_APPLY
    #define DBGPRINT_DRV_OPT_APPLY    TRUE
  #else
    #define DBGPRINT_DRV_OPT_APPLY    FALSE
  #endif

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #if defined(DBGPRINT_DRV_OPT_RAW_INIT)
    #define  DBGPRINT_OPT(type, name, defval)
  #else
    #define  DBGPRINT_OPT(type, name, defval)        pOpt->##name = defval;
  #endif
  #define  DBGPRINT_OPT_DECL(type, name)
  #define  DBGPRINT_DRV_OPT(type, name, defval)    pOpt->##name = defval; \
                                                   pOpt->##name##_upd = DBGPRINT_DRV_OPT_APPLY;

  #if defined(DBGPRINT_DRV_OPT_RAW_INIT)
    #undef DBGPRINT_DRV_OPT_RAW_INIT
  #endif
  #if defined(DBGPRINT_OPT_RAW_INIT)
    #undef DBGPRINT_OPT_RAW_INIT
  #endif

#endif //DBGPRINT_OPT_RAW_INIT

/**** recognize driver option name and fill corresponding structure member ****/
#ifdef DBGPRINT_DRV_OPT_RAW_RECOGNIZE

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #define  DBGPRINT_OPT(type, name, defval)
  #define  DBGPRINT_OPT_DECL(type, name)
  #define  DBGPRINT_DRV_OPT(type, name, defval)    if(!arg_strcmp(p_nam, arg_cTEXT(name))) { \
                                                       ULONG _tmp_v_; \
                                                       i+=d; \
                                                       if(i >= argc) {print_log("parameter " arg_pS " requires value\n", p_nam); return -2;}\
                                                       arg_sscanf(p_val, arg_TEXT("%d"), &_tmp_v_); \
                                                       pOpt->##name = _tmp_v_; \
                                                       if(saved_a) {eq_s[0] = saved_a;}\
                                                       pOpt->##name##_upd = TRUE; \
                                                   } else

  #undef DBGPRINT_DRV_OPT_RAW_RECOGNIZE

#endif //DBGPRINT_DRV_OPT_RAW_RECOGNIZE

#if 0
/**** recognize driver option name and fill corresponding structure member ****/
#ifdef DBGPRINT_DRV_OPT_RAW_RECOGNIZE_A

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #define  DBGPRINT_OPT(type, name, defval)
  #define  DBGPRINT_OPT_DECL(type, name)
  #define  DBGPRINT_DRV_OPT(type, name, defval)    if(!strcmp(p_nam, #name)) { \
                                                       ULONG _tmp_v_; \
                                                       i+=d; \
                                                       if(i >= argc) {print_log("parameter %s requires value\n", p_nam); return -2;}\
                                                       sscanf(p_val, "%d", &_tmp_v_); \
                                                       pOpt->##name = _tmp_v_; \
                                                       if(saved_a) {eq_s[0] = saved_a;}\
                                                       pOpt->##name##_upd = TRUE; \
                                                   } else

  #undef DBGPRINT_DRV_OPT_RAW_RECOGNIZE_A

#endif //DBGPRINT_DRV_OPT_RAW_RECOGNIZE_A
#endif//0

/**** write driver options to registry ****/
#ifdef DBGPRINT_DRV_OPT_RAW_WRITE_REG

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #define  DBGPRINT_OPT(type, name, defval)
  #define  DBGPRINT_OPT_DECL(type, name)
  #define  DBGPRINT_DRV_OPT(type, name, defval)    if(pOpt->##name##_upd) { \
                                                       k = sizeof(ULONG); \
                                                       if(RegSetValueExW(hKey, L#name, NULL, REG_DWORD, (PUCHAR)&(pOpt->##name), k) != ERROR_SUCCESS) { \
                                                           print_log("Error: Can't store driver parameters in registry\n"); \
                                                           exit(-4); \
                                                       } \
                                                   }

  #undef DBGPRINT_DRV_OPT_RAW_WRITE_REG

#endif //DBGPRINT_DRV_OPT_RAW_WRITE_REG

/**** read driver options from registry ****/
#ifdef DBGPRINT_DRV_OPT_RAW_READ_REG

  #define  DBGPRINT_OPT_SECTION_BEGIN(sect)
  #define  DBGPRINT_OPT_SECTION_END(sect)

  #define  DBGPRINT_OPT(type, name, defval)
  #define  DBGPRINT_OPT_DECL(type, name)
  #define  DBGPRINT_DRV_OPT(type, name, defval)    if(pOpt->##name##_upd) { \
                                                       k = sizeof(ULONG); \
                                                       if(RegQueryValueExW(hKey, L#name, NULL, &t, &(pOpt->##name), (PULONG)&k) != ERROR_SUCCESS) { \
                                                           pOpt->##name = defval; \
                                                       } \
                                                   }

  #undef DBGPRINT_DRV_OPT_RAW_READ_REG

#endif //DBGPRINT_DRV_OPT_RAW_READ_REG




/*******************************************/



/**** undefine all temporary macros declared above ****/
#ifdef DBGPRINT_OPT_MACRO_UNDEF

  #undef  DBGPRINT_OPT_SECTION_BEGIN
  #undef  DBGPRINT_OPT_SECTION_END

  #undef  DBGPRINT_OPT
  #undef  DBGPRINT_OPT_DECL
  #undef  DBGPRINT_DRV_OPT

  #ifdef DBGPRINT_DRV_OPT_APPLY
    #undef DBGPRINT_DRV_OPT_APPLY
  #endif //DBGPRINT_DRV_OPT_APPLY

#endif //DBGPRINT_OPT_MACRO_UNDEF

