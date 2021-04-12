//
// Created by iWong on 7/4/2021.
//

#include "iostream"
#include "inject_dll/inject_dll.h"

int main(){
    std::cout << "Hello World" << std::endl;
    class InjectDll injectDll(L"I:\\InjectDll\\dll\\x86\\x86win32intercept.dll", L"I:\\InjectDll\\dll\\x64\\x64win32intercept.dll)");
//    class InjectDll injectDll(L"I:\\interceptDll\\win32intercept\\Debug\\win32intercept.dll");
//    class InjectDll injectDll(L"I:\\win32intercept\\cmake-build-debug\\libwin32intercept.dll");
    injectDll.OpenProcess(E_FETCH_CLASS_NAME, "ytWindow", PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |
                                                          PROCESS_VM_OPERATION, false);
    injectDll.Inject();
    return 0;
}