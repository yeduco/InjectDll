//
// Created by iWong on 7/4/2021.
//

#ifndef INJECTDLL_INJECT_DLL_H
#define INJECTDLL_INJECT_DLL_H
#include "../include/include.h"
#include "../../include/tools.h"
#include "../include/wow64ext.h"

enum FetchType {
    E_FETCH_CLASS_NAME = 1,
    E_FETCH_PROCESS_NAME = 2
};

enum InjectType {
    E_TYPE_CREATE_REMOTE_THREAD = 1,
    E_TYPE_NT_CREATE_THREAD_EX = 2
};

enum BitType {
    E_BIT_TYPE_X86 = 1,
    E_BIT_TYPE_X64 = 2
};

typedef DWORD64 (WINAPI *NtCreateThreadEx)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, LPVOID ObjectAttributes,
                                             HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress,
                                             LPVOID lpParameter, BOOL CreateSuspended, DWORD64 dwStackSize, DWORD64 dw1,
                                             DWORD64 dw2, LPVOID Unknown);


class InjectDll {
public:
    InjectDll();

    explicit InjectDll(std::wstring x86DllPath, std::wstring x64DllPath, int injectType = E_TYPE_CREATE_REMOTE_THREAD);

    ~InjectDll();

    bool OpenProcess(int type, CSTRING &name, DWORD dwDesiredAccess, WINBOOL bInheritHandle);
    void CloseProcess();
    void SetInjectType(int injectType = E_TYPE_CREATE_REMOTE_THREAD);
    void SetX86DllPath(std::wstring dllPath);
    void SetX64DllPath(std::wstring dllPath);
    const std::wstring& GetCurrentBitDllPath() const;
    bool Inject();

public:

private:
    LPTHREAD_START_ROUTINE GetLoadLibraryProc() const;

    HANDLE m_hProcess;
    std::wstring m_x86DllPath;
    std::wstring m_x64DllPath;
    int m_processBit;
    int m_injectType;
    int m_sysOSBit;
};


#endif //INJECTDLL_INJECT_DLL_H
