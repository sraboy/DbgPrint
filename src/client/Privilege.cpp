#include <windows.h>

BOOL
Privilege(
    LPTSTR pszPrivilege,
    BOOL bEnable
    )
{
    HANDLE           hToken;
    TOKEN_PRIVILEGES tp;

    // obtain the token, first check the thread and then the process
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE, &hToken)) {
        if (GetLastError() == ERROR_NO_TOKEN) {
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }

    // get the luid for the privilege
    if (!LookupPrivilegeValue(NULL, pszPrivilege, &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;

    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // enable or disable the privilege
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0)) {
        CloseHandle(hToken);
        return FALSE;
    }

    if (!CloseHandle(hToken))
        return FALSE;

    return TRUE;
}
