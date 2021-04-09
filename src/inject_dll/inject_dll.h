//
// Created by iWong on 7/4/2021.
//

#ifndef INJECTDLL_INJECT_DLL_H
#define INJECTDLL_INJECT_DLL_H

#include <winternl.h>
#include <ntstatus.h>

#include <cstdlib>
#include <cstdio>
#include <Psapi.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <map>
#include "tools.h"

#include "process.h"


#pragma comment (lib, "Shlwapi.lib")
#pragma comment(lib, "psapi")

struct NTCREATETHREAD_BUFFER {
    ULONG m_size;
    ULONG m_unknown1;
    ULONG m_unknown2;
    PULONG m_unknown3;
    ULONG m_unknown4;
    ULONG m_unknown5;
    ULONG m_unknown6;
    PULONG m_unknown7;
    ULONG m_unknown8;
};

typedef NTSTATUS ( __stdcall *NTCREATETHREADEX)(
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
        LPVOID bytes_buffer);




enum FetchType {
    E_FETCH_CLASS_NAME = 1,
    E_FETCH_PROCESS_NAME = 2
};

typedef DWORD64 (WINAPI *NtCreateThreadEx64)(PHANDLE ThreadHandle, ACCESS_MASK DesiredAccess, LPVOID ObjectAttributes,
                                             HANDLE ProcessHandle, LPTHREAD_START_ROUTINE lpStartAddress,
                                             LPVOID lpParameter, BOOL CreateSuspended, DWORD64 dwStackSize, DWORD64 dw1,
                                             DWORD64 dw2, LPVOID Unknown);



class InjectDll {
public:
    InjectDll();

    explicit InjectDll(std::wstring dllPath);

    ~InjectDll();

    bool OpenProcess(int type, CSTRING &name, DWORD dwDesiredAccess, WINBOOL bInheritHandle);

    bool Inject();

public:

private:
    HANDLE m_hProcess;
    std::wstring m_dllPath;
};


#endif //INJECTDLL_INJECT_DLL_H
