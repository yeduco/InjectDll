//
// Created by iWong on 7/4/2021.
//

#include "iostream"
#include "inject_dll/inject_dll.h"

int main(){
    std::cout << "Hello World" << std::endl;
    class InjectDll injectDll(L"I:\\interceptDll\\win32intercept\\x64\\Release\\win32intercept.dll");
//
//    class InjectDll injectDll(L"I:\\win32intercept\\cmake-build-debug\\libwin32intercept.dll");
    injectDll.OpenProcess(E_FETCH_CLASS_NAME, "Notepad", PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_WRITE |
                                                          PROCESS_VM_OPERATION, false);
    injectDll.Inject();
    return 0;
}