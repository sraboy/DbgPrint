#include "SvcManLib.h"

UINT
NtServiceIsRunning(
    LPCTSTR ServiceName
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    SERVICE_STATUS  ssStatus;
    UINT            return_value;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    QueryServiceStatus(schService, &ssStatus);
    if(ssStatus.dwCurrentState == SERVICE_RUNNING) return_value = 1;
                                              else return_value = 0;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceStart(
    LPCTSTR ServiceName
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = StartService(schService, 0, NULL) ? 1 : -1;
    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceStop(
    LPCTSTR ServiceName,
    ULONG   TimeoutSeconds
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
    SERVICE_STATUS  SrvStatus;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = ControlService(schService, SERVICE_CONTROL_STOP, &SrvStatus) ? 1 : -1;

    if(return_value) {
        while(QueryServiceStatus(schService, &SrvStatus)) {
            if ( SrvStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
                if(!TimeoutSeconds)
                    break;
                if(TimeoutSeconds != -1)
                    TimeoutSeconds--;
                Sleep( 1000 );
            } else {
                break;
            }
        }
        if(SrvStatus.dwCurrentState == SERVICE_STOPPED) {
            return_value = 1;
        } else {
            return_value = -1;
        }
    } else {
        return_value = -2;
    }

    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceInstall(
    LPCTSTR ServiceName,
    PCHAR   PathToExecutable,
    BOOLEAN KernelDriver,
    ULONG   StartType,
    PCHAR   Dependencies
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value = 0;
    CHAR path_name1[MAX_PATH];
    CHAR path_name[MAX_PATH];
    CHAR sys_path_name[MAX_PATH];
    CHAR drv_path_name[MAX_PATH];
    CHAR cur_dir[MAX_PATH];
    CHAR mod_dir[MAX_PATH];
    PCHAR mod_fname;
    PCHAR path_ptr[4];
    ULONG i;

    if(!GetSystemDirectory(sys_path_name, MAX_PATH-40)) {
        return -1;
    }
    if(!GetCurrentDirectory(MAX_PATH-40, cur_dir)) {
        return -1;
    }
    if(!GetModuleFileName(NULL, mod_dir, MAX_PATH-40)) {
        return -1;
    }
    mod_fname = strrchr(mod_dir, '\\');
    if(!mod_fname) {
        return -1;
    }
    mod_fname[0] = 0;

    path_ptr[0] = PathToExecutable;
    path_ptr[1] = cur_dir;
    path_ptr[2] = mod_dir;

    for(i=0; i<3; i++) {
        if(KernelDriver) {
            _snprintf(path_name,     MAX_PATH-1, "%s\\drivers\\%s.sys", sys_path_name, ServiceName);
            _snprintf(path_name1,    MAX_PATH-1, "%s\\%s.sys", path_ptr[i], ServiceName);
            _snprintf(drv_path_name, MAX_PATH-1, "System32\\drivers\\%s.sys", ServiceName);
        } else {
            _snprintf(path_name,     MAX_PATH-1, "%s\\%s.exe", sys_path_name, ServiceName);
            _snprintf(path_name1,    MAX_PATH-1, "%s\\%s.exe", path_ptr[i], ServiceName);
            _snprintf(drv_path_name, MAX_PATH-1, "System32\\%s.exe", ServiceName);
        }

        SetFileAttributes(path_name, FILE_ATTRIBUTE_NORMAL);
        return_value = CopyFile(path_name1, path_name, 0);
        if(return_value)
            break;
    }
    if(!return_value) {
        return_value = GetLastError();
        printf("Can't copy %s to %s\nerror code %#x\n", path_name1, path_name, return_value);
        return -1;
    }

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = CreateService(schSCManager, ServiceName, ServiceName, SERVICE_ALL_ACCESS,
                                KernelDriver ? SERVICE_KERNEL_DRIVER : SERVICE_WIN32_OWN_PROCESS,
                                StartType, SERVICE_ERROR_IGNORE,
                                drv_path_name, NULL, NULL, Dependencies, NULL, NULL);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    } else {
        return_value = 1;
    }
//    return_value = StartService(schService, 0, NULL) ? 1 : -1;
    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceRemove(
    LPCTSTR ServiceName
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
//    SERVICE_STATUS  SrvStatus;

/*
    return_value = NtServiceStop(ServiceName);
    if(return_value != 1) {
        return return_value;
    }
*/
    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = DeleteService(schService) ? 1 : -1;
    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceSetStartMode(
    LPCTSTR ServiceName,
    ULONG StartMode
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    return_value = ChangeServiceConfig(schService, 
        SERVICE_NO_CHANGE,
        StartMode,
        SERVICE_NO_CHANGE,
        NULL, NULL, NULL /*tag*/,
        NULL, NULL, NULL /*pwd*/,
        NULL);
    return_value = 0;
    RC = GetLastError();
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

UINT
NtServiceGetStartMode(
    LPCTSTR ServiceName,
    ULONG*  StartMode
    )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
    UCHAR           buffer[2048];
    QUERY_SERVICE_CONFIG* svc_config;

    schSCManager = OpenSCManager(
                                NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager) return -1;
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        RC = GetLastError();
        CloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) return -2;
                                           else return -1;
    }
    svc_config = (QUERY_SERVICE_CONFIG*)&buffer;
    return_value = QueryServiceConfig(schService, 
        svc_config,
        sizeof(buffer),
        &RC);
    if(return_value) {
        return_value = 0;
        (*StartMode) = svc_config->dwStartType;
    } else {
        RC = GetLastError();
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return return_value;
}

