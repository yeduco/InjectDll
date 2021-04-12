//
// Created by iWong on 7/4/2021.
//

#include "inject_dll.h"

#include <utility>

const char *__stdcall winstrerror(DWORD code) {
    char *str;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &str,
                  0,
                  NULL);
    static char err[0xff];
    strcpy_s(err, str), LocalFree(str);
    return err;
}


InjectDll::InjectDll() {
    this->m_hProcess = INVALID_HANDLE_VALUE;
    this->m_processBit = 0;
    this->m_sysOSBit = 0;
    this->m_injectType = E_TYPE_CREATE_REMOTE_THREAD;
}

InjectDll::InjectDll(std::wstring x86DllPath, std::wstring x64DllPath, int injectType) {
    this->m_x86DllPath = std::move(x86DllPath);
    this->m_x64DllPath = std::move(x64DllPath);
    this->m_hProcess = nullptr;
    this->m_injectType = injectType;
    this->m_sysOSBit = 0;
    this->m_processBit = 0;
}

InjectDll::~InjectDll() {
    this->m_hProcess = INVALID_HANDLE_VALUE;
}

bool InjectDll::Inject() {
    JUDGE_RETURN(nullptr != this->m_hProcess, false);
    const std::wstring dllPath = this->GetCurrentBitDllPath();
    SIZE_T size = dllPath.length() * sizeof(wchar_t);
    LPVOID baseAddress = VirtualAllocEx(this->m_hProcess, nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (baseAddress == nullptr) {
        std::cerr << winstrerror(GetLastError()) << std::endl;
        this->CloseProcess();
        return false;
    }
    size_t written = 0;
    if (!WriteProcessMemory(this->m_hProcess, baseAddress, dllPath.c_str(), size, reinterpret_cast<SIZE_T *>(&written))) {
        VirtualFreeEx(this->m_hProcess, baseAddress, size, MEM_RELEASE);
        this->CloseProcess();
        std::cerr << winstrerror(GetLastError()) << std::endl;
        return false;
    }
    HANDLE hRemoteThread = nullptr;
    DWORD threadId = 0;
    auto routine = this->GetLoadLibraryProc();
    switch (this->m_injectType) {
        case E_TYPE_CREATE_REMOTE_THREAD:
            hRemoteThread = CreateRemoteThread(this->m_hProcess, nullptr, 0, routine, baseAddress, 0, &threadId);
            break;
        case E_TYPE_NT_CREATE_THREAD_EX:
            auto NtCreateThreadEx = reinterpret_cast<::NtCreateThreadEx> (GetProcAddress(GetModuleHandle(TEXT("ntdll")),"NtCreateThreadEx"));
            NtCreateThreadEx(&hRemoteThread, 0x1FFFFF, nullptr, this->m_hProcess, (LPTHREAD_START_ROUTINE) routine, baseAddress, FALSE, 0, 0, 0, nullptr);
            break;
    }
    std::cerr << winstrerror(GetLastError()) << std::endl;
    WaitForSingleObject(hRemoteThread, INFINITE);
    VirtualFreeEx(this->m_hProcess, baseAddress, size, MEM_RELEASE);
    this->CloseProcess();
    return true;
}
bool InjectDll::OpenProcess(int type, CSTRING &name,
                            DWORD dwDesiredAccess = PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |PROCESS_VM_OPERATION,
                            WINBOOL bInheritHandle = false) {
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
        this->m_sysOSBit = E_BIT_TYPE_X64;
    } else {
        this->m_sysOSBit = E_BIT_TYPE_X86;
    }
    if (Bit64Process){
        this->m_processBit = E_BIT_TYPE_X64;
    }else{
        this->m_processBit = E_BIT_TYPE_X86;
    }
    return true;
}


void InjectDll::CloseProcess() {
    ::CloseHandle(this->m_hProcess);
}

void InjectDll::SetInjectType(int injectType) {
    this->m_injectType = injectType;
}

LPTHREAD_START_ROUTINE InjectDll::GetLoadLibraryProc() const {
    switch(this->m_processBit){
        case E_BIT_TYPE_X86:
            return reinterpret_cast<LPTHREAD_START_ROUTINE>(QueryX86ProcAddress(L"kernel32.dll", L"LoadLibraryW"));
        case E_BIT_TYPE_X64:
            return reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"LoadLibraryW"));
    }
    return nullptr;
}

void InjectDll::SetX86DllPath(std::wstring dllPath) {
    this->m_x86DllPath = std::move(dllPath);
}

void InjectDll::SetX64DllPath(std::wstring dllPath) {
    this->m_x64DllPath = std::move(dllPath);
}

const std::wstring &InjectDll::GetCurrentBitDllPath() const {
    if (this->m_processBit == E_BIT_TYPE_X86) {
        return this->m_x86DllPath;
    } else {
        return this->m_x64DllPath;
    }
}
