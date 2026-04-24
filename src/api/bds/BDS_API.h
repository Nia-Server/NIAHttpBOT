#ifndef BDS_API_H
#define BDS_API_H

#include <queue>
#include <regex>
#include <thread>
#include <condition_variable>
#include <vector>

struct BDSInstanceState {
	std::string Id;
	std::string Name;
	std::string ExecutablePath;
	std::string WorkingDirectory;
	bool AutoStart = false;
	bool UseCmd = false;
	bool Running = false;
	std::string LogTag;
};

#include "Logger.hpp"

struct BDSInstanceConfig {
	std::string Id;
	std::string Name;
	std::string ExecutablePath;
	std::string WorkingDirectory;
	bool AutoStart = false;
	bool UseCmd = false;
	bool AutoBackup = false;
	int BackupHour = 4;
	int BackupMinute = 0;
	int BackupSecond = 0;
	std::string BackupFrom;
	std::string BackupTo;
	std::string LogTag;
};

#ifdef WIN32
#include <windows.h>

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
#endif

bool ConfigureBdsInstances(const std::vector<BDSInstanceConfig>& instances, const std::string& defaultInstanceId, std::string& error);
std::vector<std::string> ListServerInstances();
std::vector<BDSInstanceState> ListServerInstanceStates();
bool SetDefaultServerInstance(const std::string& instanceId);
std::string GetDefaultServerInstance();
std::string GetServerWorkingDirectory(const std::string& instanceId = "");
bool IsCmdEnabledForInstance(const std::string& instanceId = "");
bool IsServerRunning(const std::string& instanceId = "");
std::vector<std::string> GetServerOutputHistory(const std::string& instanceId = "", std::size_t limit = 200);

bool StartServer(const std::string& instanceId = "");
bool StopServer(const std::string& instanceId = "");
void StopAllServers();
void StopAllServersForExit();
void BackupServer();
std::string runCommand(const std::string& command, const std::string& instanceId = "");
bool AddPlayerToWhitelist(const std::string& player_name, const std::string& instanceId = "");
bool RemovePlayerFromWhitelist(const std::string& player_name, const std::string& instanceId = "");

#endif