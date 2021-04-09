//
// Created by iWong on 7/4/2021.
//

#include "inject_dll.h"
#include <cerrno>
#include <utility>

const char * __stdcall winstrerror( DWORD code ) {
    char *str;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   code,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPTSTR) &str,
                   0,
                   NULL);
    static char err[0xff];
    strcpy_s( err, str ), LocalFree( str );
    return err;
}

DWORD WINAPI ThreadProcEnd()
{
//    MyOutputDebugStringA("ThreadProcEnd");
    return 0;
}


InjectDll::InjectDll() {
    this->m_hProcess = INVALID_HANDLE_VALUE;
}

InjectDll::InjectDll(std::wstring dllPath) {
    this->m_dllPath = std::move(dllPath);
    this->m_hProcess = nullptr;
}

InjectDll::~InjectDll() {
    this->m_hProcess = INVALID_HANDLE_VALUE;
}

bool InjectDll::Inject() {
    JUDGE_RETURN(nullptr != this->m_hProcess, false);
    SIZE_T size = this->m_dllPath.length() * sizeof(wchar_t);
    LPVOID baseAddress = VirtualAllocEx(this->m_hProcess, nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if( baseAddress == nullptr ) {
        std::cerr << winstrerror(GetLastError()) << std::endl;
        return false;
    }
    size_t written = 0;
    if (!WriteProcessMemory(this->m_hProcess, baseAddress, this->m_dllPath.c_str(), size,
                            reinterpret_cast<SIZE_T *>(&written))) {
        std::cerr << winstrerror(GetLastError()) << std::endl;
        return false;
    }
//    THREAD_ALL_ACCESS
    auto routine = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress( GetModuleHandle(TEXT("kernel32")), "LoadLibraryW"));
    std::cout << "routine addr " << std::hex << &routine << std::endl;
    auto NtCreateThreadEx = reinterpret_cast<NtCreateThreadEx64> (GetProcAddress( GetModuleHandle(TEXT("ntdll")), "NtCreateThreadEx"));
    HANDLE remoteThread;
//    CreateRemoteThread64
    DWORD threadId = 0;
    HANDLE hThread = CreateRemoteThread(this->m_hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)routine, baseAddress, 0, &threadId);
//    remoteThread = CreateRemoteThread(this->m_hProcess, nullptr, 0, routine, baseAddress, 0, nullptr);
//    bool ret = NtCreateThreadEx(&remoteThread, 0x1FFFFF, nullptr, this->m_hProcess, (LPTHREAD_START_ROUTINE)routine, baseAddress, FALSE, 0, 0, 0, nullptr);
//    if( !ret ) {
        std::cout << hThread << " " << threadId << std::endl;
        std::cout << GetLastError() << std::endl;
        std::cerr << winstrerror(GetLastError()) << std::endl;
//    }
    DWORD exitCode1;
    WaitForSingleObject(remoteThread, INFINITE);
    GetExitCodeThread(remoteThread, &exitCode1);
    std::cout << exitCode1 << std::endl;
    return true;
}

bool InjectDll::OpenProcess(int type, CSTRING &name, DWORD dwDesiredAccess, WINBOOL bInheritHandle) {
    DWORD dwProcessId = 0;
    switch (type) {
        case E_FETCH_CLASS_NAME:
            dwProcessId = GetProcessIDByClassName(name);
            break;
        case E_FETCH_PROCESS_NAME:
            dwProcessId = GetProcessIDByProcessName(name);
            break;
        default:
            break;
    }
    JUDGE_RETURN(0 != dwProcessId, false);
    this->m_hProcess = ::OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    JUDGE_RETURN(INVALID_HANDLE_VALUE != this->m_hProcess, false);
    bool Is64BitOS = Is64BitSystemOS();
    bool Bit64Process = Is64BitProcess(this->m_hProcess);
    if (Is64BitOS) {
        std::cout << "is 64bit os" << std::endl;
    } else {
        std::cout << "is 32bit os" << std::endl;
    }
    if (Bit64Process){
        std::cout << "is 64bit process" << std::endl;
    }else{
        std::cout << "is 32bit process" << std::endl;
    }

    return true;
}
