#ifndef BDS_API_H
#define BDS_API_H

#include <queue>
#include <regex>


#include "Logger.hpp"

extern std::string ServerLocate;

#ifdef WIN32
#include <windows.h>
#endif

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);

BOOL StartServer();
BOOL StopServer();

std::string runCommand(const std::string& command);

#endif