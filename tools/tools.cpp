//
// Created by iWong on 29/3/2021.
//

#include "tools.h"
#include "psapi.h"
#include "winver.h"

struct NTCREATETHREAD_BUFFER  {
    ULONG     m_size;
    ULONG     m_unknown1;
    ULONG     m_unknown2;
    PULONG    m_unknown3;
    ULONG     m_unknown4;
    ULONG     m_unknown5;
    ULONG     m_unknown6;
    PULONG    m_unknown7;
    ULONG     m_unknown8;
};

typedef NTSTATUS ( __stdcall *NTCREATETHREADEX) (
        PHANDLE thread,
        ACCESS_MASK desired_access,
        LPVOID object_attributes,
        HANDLE process_handle,
        LPTHREAD_START_ROUTINE start_address,
        LPVOID lp,
        BOOL create_suspended,
        ULONG stack_zero_bits,
        ULONG size_of_stack_commit,
        ULONG size_of_stack_reserve,
        LPVOID bytes_buffer );


typedef NTSTATUS(WINAPI* LPFN_NTCREATETHREADEX)(
        OUT PHANDLE ThreadHandle,
        IN ACCESS_MASK DesiredAccess,
        IN LPVOID ObjectAttributes,
        IN HANDLE ProcessHandle,
        IN LPTHREAD_START_ROUTINE ThreadProcedure,
        IN LPVOID ParameterData,
        IN BOOL CreateSuspended,
        IN SIZE_T StackZeroBits,
        IN SIZE_T SizeOfStackCommit,
        IN SIZE_T SizeOfStackReserve,
        OUT LPVOID BytesBuffer);

HWND GetWindowHwnd(const std::string& className){
    HWND hwnd = FindWindow(className.c_str(), nullptr);
    if(nullptr == hwnd){
        MessageBox(nullptr, "process not run", "error", MB_OKCANCEL | MB_ICONHAND);
        return nullptr;
    }
    return hwnd;
}

void GetProcessPathByPid(DWORD& pid, char* path){
    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (nullptr == handle) {
        MessageBox(nullptr, "process not run", "error", MB_OKCANCEL | MB_ICONHAND);
        return;
    }
    GetModuleFileNameEx(handle, nullptr, path, MAX_PATH);
}

void GetProcessPathByHwnd(const HWND& hwnd, char* path){
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    GetProcessPathByPid(pid, path);
}

void GetProcessVersion(char* path, char* version){
    DWORD handle;
    DWORD versionInfoSize = GetFileVersionInfoSize(path, &handle);
    BYTE *versionInfo = new BYTE[versionInfoSize];
    if (FALSE == GetFileVersionInfo(path, 0, versionInfoSize, versionInfo)) {
        delete[] versionInfo;
        return;
    }
    VS_FIXEDFILEINFO* p_version = nullptr;
    unsigned int len;
    if(FALSE == VerQueryValue(versionInfo, TEXT("\\"), (void**)&p_version, &len)){
        return;
    }
    version[0] = HIWORD(p_version->dwFileVersionMS);
    version[1] = LOWORD(p_version->dwFileVersionMS);
    version[2] = HIWORD(p_version->dwFileVersionLS);
    version[3] = LOWORD(p_version->dwFileVersionLS);
    delete[] versionInfo;
}

DWORD GetProcessIDByClassName(const std::string& name){
    HWND hwnd = nullptr;
    DWORD dwProcessId = 0;
    hwnd = GetWindowHwnd(name);
    JUDGE_RETURN(nullptr != hwnd, 0);
    GetWindowThreadProcessId(hwnd, &dwProcessId);
    return dwProcessId;
}

DWORD GetProcessIDByProcessName(const std::string& name){
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE != hSnapshot) {
        return 0;
    }
    PROCESSENTRY32 pe = {sizeof(pe)};
    for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {
        if(strcmp(name.c_str(), pe.szExeFile) == 0){
            std::cout << "process name:" << pe.szExeFile << std::endl;
//            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    }
    CloseHandle(hSnapshot);
    return 0;
}


void GetProcessTitleAndVersion(const HWND& hwnd, std::string &titleVersion) {
    char version[VERSION_MAX] = {0};
    char path[MAX_PATH] = {0};
    char title[MAX_PATH] = {0};
    GetProcessPathByHwnd(hwnd, path);
    GetProcessVersion(path, version);
    GetWindowText(hwnd, title, MAX_PATH);
    titleVersion = title;
    titleVersion.append(" ");
    for(auto versionByte : version) {
        titleVersion.append(std::to_string(versionByte)).append(".");
    }
    titleVersion.pop_back();
}


void GetProcessHwndVecByClassName(const std::string& className, EnumHwndArg& eHwndArgs){
    eHwndArgs.dwClassName = className;
    eHwndArgs.dwHwndVec = new std::vector<HWND>;

    EnumWindows(lpEnumFunc, (LPARAM)&eHwndArgs);
}


BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam){
    auto *pArgs = (LPEnumHwndArg)lParam;
    char className[MAX_PATH];
    GetClassName(hwnd,className, MAX_PATH);
    if(strcmp(className, pArgs->dwClassName.c_str()) == 0){
        pArgs->dwHwndVec->push_back(hwnd);
    }
    return TRUE;
}

double GetWindowDpiScaleFactor(int type) {
    switch (type) {
        case DPI_SCALE_FACTOR_100:
            return 1;
        case DPI_SCALE_FACTOR_125:
            return 1.25;
        case DPI_SCALE_FACTOR_150:
            return 1.5;
        case DPI_SCALE_FACTOR_200:
            return 2;
        default:
            return 1;
    }
}

void GetWindowWidthHeight(HWND hwnd, int& width, int& height){
    auto fGetDpiForWindow = (GetDpiForWindow) GetProcAddress(GetModuleHandle(TEXT("User32")), "GetDpiForWindow");
    JUDGE_RETURN(fGetDpiForWindow != nullptr, ;);
    HWND desktopHwnd = GetDesktopWindow();
    int dpi = (*fGetDpiForWindow)(desktopHwnd);
    double dpiScaleFactor = GetWindowDpiScaleFactor(dpi);

    RECT rc;
    GetWindowRect(hwnd, &rc);
    width = int((rc.right - rc.left) * dpiScaleFactor);
    height = int((rc.bottom - rc.top) * dpiScaleFactor);
}

bool Is64BitSystemOS(){
    typedef VOID(WINAPI *LPFNGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
    auto fGetNativeSystemInfo = reinterpret_cast<LPFNGetNativeSystemInfo>(GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetNativeSystemInfo"));
    JUDGE_RETURN(fGetNativeSystemInfo != nullptr, -1);
    SYSTEM_INFO stInfo = {0};
    fGetNativeSystemInfo(&stInfo);
    return stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64;
}

bool Is64BitProcess(DWORD dwProcessID) {
    JUDGE_RETURN(Is64BitSystemOS() == true, false);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
    JUDGE_RETURN(nullptr != hProcess, false);
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    auto fIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    JUDGE_RETURN(nullptr != fIsWow64Process, false);
    BOOL bIsWow64 = false;
    fIsWow64Process(hProcess, &bIsWow64);
    CloseHandle(hProcess);
    return bIsWow64;
}

bool Is64BitProcess(HANDLE hProcess){
    JUDGE_RETURN(Is64BitSystemOS() == true, false);
    JUDGE_RETURN(nullptr != hProcess, false);
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    auto fIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    JUDGE_RETURN(nullptr != fIsWow64Process, false);
    BOOL bIsWow64 = false;
    fIsWow64Process(hProcess, &bIsWow64);
    if (bIsWow64)
    {
        return false;
    }
    else
    {
        return true;
    }
}



//    DWORD dwSize = strlen(dllPath.c_str()) + 1;
//    DWORD dwRealSize;
//    LPVOID remoteBuf = VirtualAllocEx(hProcess, nullptr, dwSize, MEM_COMMIT, PAGE_READWRITE);
//    if(WriteProcessMemory(hProcess, remoteBuf, (void*)dllPath.c_str(), dwSize, reinterpret_cast<SIZE_T *>(&dwRealSize))){
//        DWORD dwThreadId = 0;
//        HANDLE hRemoteThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE) funAddr, remoteBuf, 0, &dwThreadId);
//        DWORD errId = GetLastError();
//
//        std::cout << "Thread:" << hRemoteThread <<" " << dwThreadId << " error:" << errId << std::endl;
//        if(nullptr == hRemoteThread){
//            VirtualFreeEx(hProcess, remoteBuf, dwSize, MEM_RELEASE);
//            CloseHandle(hProcess);
//            return false;
//        }
//        WaitForSingleObject(hRemoteThread, INFINITE);
//        CloseHandle(hRemoteThread);
//        VirtualFreeEx(hProcess, remoteBuf, dwSize, MEM_RELEASE);
//        CloseHandle(hProcess);
//        return true;
//    } else {
//        VirtualFreeEx(hProcess, remoteBuf, dwSize, MEM_RELEASE);
//        CloseHandle(hProcess);
//        return false;
//    };