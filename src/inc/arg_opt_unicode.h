
#ifdef ARG_OPT_UNICODE
  #define arg_strcmp    wcscmp
  #define arg_sscanf    swscanf
  #define arg_stricmp   wcsicmp
  #define arg_cTEXT(x)  L#x
  #define arg_TEXT(x)   L##x
  #define arg_pS        "%S"
#else
  #define arg_strcmp    strcmp
  #define arg_sscanf    sscanf
  #define arg_stricmp   _stricmp
  #define arg_cTEXT(x)  #x
  #define arg_TEXT(x)   x
  #define arg_pS        "%s"
#endif
