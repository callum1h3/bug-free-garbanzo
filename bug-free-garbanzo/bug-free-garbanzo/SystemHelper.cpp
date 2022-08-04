#include "SystemHelper.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <Windows.h>
#include <comdef.h>

std::string SystemHelper::PATH_EXE;
std::string SystemHelper::PATH_DIRECTORY;

void SystemHelper::Initialize()
{
    WCHAR path[512];
    GetModuleFileNameW(NULL, path, 512);
    _bstr_t b(path);
    PATH_EXE = b;

    std::string EXE_NAME = "bug-free-garbanzo.exe";
    std::string DIRECTORY_PATH = std::string(PATH_EXE);
    DIRECTORY_PATH.erase(DIRECTORY_PATH.length() - EXE_NAME.length());

    PATH_DIRECTORY = DIRECTORY_PATH.c_str();
}