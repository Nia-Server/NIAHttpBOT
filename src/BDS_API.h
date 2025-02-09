#ifndef BDS_API_H
#define BDS_API_H

#include <queue>
#include <regex>


#include "Logger.hpp"

extern std::string ServerLocate;

#ifdef WIN32
#include <windows.h>

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
#endif

bool StartServer();
bool StopServer();
void BackupServer();
std::string runCommand(const std::string& command);
bool AddPlayerToWhitelist(const std::string& player_name);
bool RemovePlayerFromWhitelist(const std::string& player_name);

#endif