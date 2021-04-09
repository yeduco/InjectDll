//
// Created by iWong on 29/3/2021.
//

#ifndef BEHAVIORTREE_TOOLS_H
#define BEHAVIORTREE_TOOLS_H

#include "Windows.h"
#include "iostream"
#include "string"
#include "tlhelp32.h"
#include "vector"

#define VERSION_MAX 4
#define JUDGE_RETURN(CONDITION, RETURN) if (!(CONDITION)){return RETURN;}
typedef int (WINAPI *GetDpiForWindow)(HWND hwnd);

#define DPI_SCALE_FACTOR_100 96
#define DPI_SCALE_FACTOR_125 120
#define DPI_SCALE_FACTOR_150 144
#define DPI_SCALE_FACTOR_200 192

#define CSTRING const std::string


typedef struct EnumHwndArg{
    std::vector<HWND> *dwHwndVec{};
    std::string dwClassName;
}EnumHwndArg, *LPEnumHwndArg;


HWND GetWindowHwnd(const std::string& className);

void GetProcessPathByPid(DWORD& pid, char* path);

void GetProcessPathByHwnd(const HWND& hwnd, char* path);

void GetProcessVersion(char* path, char* version);

DWORD GetProcessIDByProcessName(const std::string& name);

DWORD GetProcessIDByClassName(const std::string& name);

void GetProcessTitleAndVersion(const HWND &hwnd, std::string& titleVersion);

void GetProcessHwndVecByClassName(const std::string& className, EnumHwndArg& eHwndArgs);

BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam);

double GetWindowDpiScaleFactor(int type);

void GetWindowWidthHeight(HWND hwnd, int& width, int& height);

bool Is64BitSystemOS();

bool Is64BitProcess(DWORD dwProcessID);

bool Is64BitProcess(HANDLE hProcess);

#endif //BEHAVIORTREE_TOOLS_H
