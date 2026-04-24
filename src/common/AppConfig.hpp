#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <vector>

namespace AppCfg {

struct BdsInstanceConfig {
    std::string Id = "default";
    std::string Name = "Default";
    std::string ExecutablePath = "D:/NiaServer-Core/bedrock_server.exe";
    std::string WorkingDirectory = "D:/NiaServer-Core";
    bool AutoStart = false;
    bool UseCmd = false;

    bool AutoBackup = false;
    int BackupHour = 4;
    int BackupMinute = 0;
    int BackupSecond = 0;
    std::string BackupFrom = "D:/NiaServer-Core/worlds/250117";
    std::string BackupTo = "./backup/default";

    std::string LogTag = "default";
};

struct Config {
    std::string LanguageFile = "";
    std::string IPAddress = "127.0.0.1";
    int ServerPort = 2333;

    std::string DefaultBdsInstanceId = "default";
    int AutoStartDelaySeconds = 5;
    std::vector<BdsInstanceConfig> BdsInstances { BdsInstanceConfig{} };

    bool UseQQBot = false;
    std::string QQIPAddress = "127.0.0.1";
    int QQClientPort = 10023;
    int QQServerPort = 10086;
    std::string OwnerQQ = "123456789";
    std::string QQGroup = "123456789";
};

Config DefaultConfig();
bool LoadFromJsonFile(const std::string& path, Config& cfg, std::string& error);
bool SaveToJsonFile(const std::string& path, const Config& cfg, std::string& error);
bool GetValueByType(const Config& cfg, const std::string& name, char type, std::string& out);

} // namespace AppCfg

#endif
