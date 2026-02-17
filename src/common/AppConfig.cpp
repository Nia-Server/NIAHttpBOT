#include "AppConfig.hpp"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace AppCfg {

Config DefaultConfig() {
    return Config{};
}

static std::string JsonTypeName(rapidjson::Type type) {
    switch (type) {
        case rapidjson::kNullType: return "null";
        case rapidjson::kFalseType:
        case rapidjson::kTrueType: return "bool";
        case rapidjson::kObjectType: return "object";
        case rapidjson::kArrayType: return "array";
        case rapidjson::kStringType: return "string";
        case rapidjson::kNumberType: return "number";
        default: return "unknown";
    }
}

static bool ReadJsonFile(const std::string& path, rapidjson::Document& doc, std::string& error) {
    std::ifstream in(path);
    if (!in.is_open()) {
        error = "无法打开配置文件: " + path;
        return false;
    }

    rapidjson::IStreamWrapper isw(in);
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        error = "JSON 解析失败，请检查语法";
        return false;
    }
    if (!doc.IsObject()) {
        error = "配置根节点必须是 JSON 对象";
        return false;
    }
    return true;
}

static bool GetObj(const rapidjson::Value& parent, const char* key, const rapidjson::Value*& out, std::string& error) {
    if (!parent.HasMember(key)) return true;
    const rapidjson::Value& value = parent[key];
    if (!value.IsObject()) {
        error = std::string("字段 ") + key + " 必须是对象，当前为 " + JsonTypeName(value.GetType());
        return false;
    }
    out = &value;
    return true;
}

static bool SetStringIfExists(const rapidjson::Value* obj, const char* key, std::string& target, std::string& error) {
    if (!obj || !obj->HasMember(key)) return true;
    const rapidjson::Value& value = (*obj)[key];
    if (!value.IsString()) {
        error = std::string("字段 ") + key + " 必须是字符串";
        return false;
    }
    target = value.GetString();
    return true;
}

static bool SetBoolIfExists(const rapidjson::Value* obj, const char* key, bool& target, std::string& error) {
    if (!obj || !obj->HasMember(key)) return true;
    const rapidjson::Value& value = (*obj)[key];
    if (!value.IsBool()) {
        error = std::string("字段 ") + key + " 必须是布尔值";
        return false;
    }
    target = value.GetBool();
    return true;
}

static bool SetIntIfExists(const rapidjson::Value* obj, const char* key, int& target, std::string& error) {
    if (!obj || !obj->HasMember(key)) return true;
    const rapidjson::Value& value = (*obj)[key];
    if (!value.IsInt()) {
        error = std::string("字段 ") + key + " 必须是整数";
        return false;
    }
    target = value.GetInt();
    return true;
}

bool LoadFromJsonFile(const std::string& path, Config& cfg, std::string& error) {
    rapidjson::Document doc;
    if (!ReadJsonFile(path, doc, error)) return false;

    Config loaded = DefaultConfig();

    const rapidjson::Value* base = nullptr;
    const rapidjson::Value* server = nullptr;
    const rapidjson::Value* backup = nullptr;
    const rapidjson::Value* features = nullptr;
    const rapidjson::Value* qqbot = nullptr;

    if (!GetObj(doc, "base", base, error)) return false;
    if (!GetObj(doc, "server", server, error)) return false;
    if (!GetObj(doc, "backup", backup, error)) return false;
    if (!GetObj(doc, "features", features, error)) return false;
    if (!GetObj(doc, "qqbot", qqbot, error)) return false;

    if (!SetStringIfExists(base, "LanguageFile", loaded.LanguageFile, error)) return false;
    if (!SetStringIfExists(base, "IPAddress", loaded.IPAddress, error)) return false;
    if (!SetIntIfExists(base, "ServerPort", loaded.ServerPort, error)) return false;
    if (!SetBoolIfExists(base, "EnableWebUI", loaded.EnableWebUI, error)) return false;
    if (!SetStringIfExists(base, "WebUIFile", loaded.WebUIFile, error)) return false;
    if (!SetStringIfExists(base, "WebUIWebsitePath", loaded.WebUIWebsitePath, error)) return false;

    if (!SetStringIfExists(server, "ServerLocate", loaded.ServerLocate, error)) return false;
    if (!SetBoolIfExists(server, "AutoStartServer", loaded.AutoStartServer, error)) return false;

    if (!SetBoolIfExists(backup, "AutoBackup", loaded.AutoBackup, error)) return false;
    if (!SetIntIfExists(backup, "BackupHour", loaded.BackupHour, error)) return false;
    if (!SetIntIfExists(backup, "BackupMinute", loaded.BackupMinute, error)) return false;
    if (!SetIntIfExists(backup, "BackupSecond", loaded.BackupSecond, error)) return false;
    if (!SetStringIfExists(backup, "BackupFrom", loaded.BackupFrom, error)) return false;
    if (!SetStringIfExists(backup, "BackupTo", loaded.BackupTo, error)) return false;

    if (!SetBoolIfExists(features, "UseCmd", loaded.UseCmd, error)) return false;

    if (!SetBoolIfExists(qqbot, "UseQQBot", loaded.UseQQBot, error)) return false;
    if (!SetStringIfExists(qqbot, "QQIPAddress", loaded.QQIPAddress, error)) return false;
    if (!SetIntIfExists(qqbot, "QQClientPort", loaded.QQClientPort, error)) return false;
    if (!SetIntIfExists(qqbot, "QQServerPort", loaded.QQServerPort, error)) return false;
    if (!SetStringIfExists(qqbot, "OwnerQQ", loaded.OwnerQQ, error)) return false;
    if (!SetStringIfExists(qqbot, "QQGroup", loaded.QQGroup, error)) return false;

    cfg = loaded;
    return true;
}

bool SaveToJsonFile(const std::string& path, const Config& cfg, std::string& error) {
    std::ofstream out(path);
    if (!out.is_open()) {
        error = "无法写入配置文件: " + path;
        return false;
    }

    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    rapidjson::Value base(rapidjson::kObjectType);
    base.AddMember("LanguageFile", rapidjson::Value(cfg.LanguageFile.c_str(), alloc), alloc);
    base.AddMember("IPAddress", rapidjson::Value(cfg.IPAddress.c_str(), alloc), alloc);
    base.AddMember("ServerPort", cfg.ServerPort, alloc);
    base.AddMember("EnableWebUI", cfg.EnableWebUI, alloc);
    base.AddMember("WebUIFile", rapidjson::Value(cfg.WebUIFile.c_str(), alloc), alloc);
    base.AddMember("WebUIWebsitePath", rapidjson::Value(cfg.WebUIWebsitePath.c_str(), alloc), alloc);
    doc.AddMember("base", base, alloc);

    rapidjson::Value server(rapidjson::kObjectType);
    server.AddMember("ServerLocate", rapidjson::Value(cfg.ServerLocate.c_str(), alloc), alloc);
    server.AddMember("AutoStartServer", cfg.AutoStartServer, alloc);
    doc.AddMember("server", server, alloc);

    rapidjson::Value backup(rapidjson::kObjectType);
    backup.AddMember("AutoBackup", cfg.AutoBackup, alloc);
    backup.AddMember("BackupHour", cfg.BackupHour, alloc);
    backup.AddMember("BackupMinute", cfg.BackupMinute, alloc);
    backup.AddMember("BackupSecond", cfg.BackupSecond, alloc);
    backup.AddMember("BackupFrom", rapidjson::Value(cfg.BackupFrom.c_str(), alloc), alloc);
    backup.AddMember("BackupTo", rapidjson::Value(cfg.BackupTo.c_str(), alloc), alloc);
    doc.AddMember("backup", backup, alloc);

    rapidjson::Value features(rapidjson::kObjectType);
    features.AddMember("UseCmd", cfg.UseCmd, alloc);
    doc.AddMember("features", features, alloc);

    rapidjson::Value qqbot(rapidjson::kObjectType);
    qqbot.AddMember("UseQQBot", cfg.UseQQBot, alloc);
    qqbot.AddMember("QQIPAddress", rapidjson::Value(cfg.QQIPAddress.c_str(), alloc), alloc);
    qqbot.AddMember("QQClientPort", cfg.QQClientPort, alloc);
    qqbot.AddMember("QQServerPort", cfg.QQServerPort, alloc);
    qqbot.AddMember("OwnerQQ", rapidjson::Value(cfg.OwnerQQ.c_str(), alloc), alloc);
    qqbot.AddMember("QQGroup", rapidjson::Value(cfg.QQGroup.c_str(), alloc), alloc);
    doc.AddMember("qqbot", qqbot, alloc);

    rapidjson::OStreamWrapper osw(out);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    writer.SetIndent(' ', 2);
    doc.Accept(writer);

    return true;
}

bool GetValueByType(const Config& cfg, const std::string& name, char type, std::string& out) {
    switch (type) {
        case 'B': {
            bool value;
            if (name == "EnableWebUI") value = cfg.EnableWebUI;
            else if (name == "AutoStartServer") value = cfg.AutoStartServer;
            else if (name == "AutoBackup") value = cfg.AutoBackup;
            else if (name == "UseCmd") value = cfg.UseCmd;
            else if (name == "UseQQBot") value = cfg.UseQQBot;
            else return false;
            out = value ? "1" : "0";
            return true;
        }
        case 'I': {
            int value;
            if (name == "ServerPort") value = cfg.ServerPort;
            else if (name == "BackupHour") value = cfg.BackupHour;
            else if (name == "BackupMinute") value = cfg.BackupMinute;
            else if (name == "BackupSecond") value = cfg.BackupSecond;
            else if (name == "QQClientPort") value = cfg.QQClientPort;
            else if (name == "QQServerPort") value = cfg.QQServerPort;
            else return false;
            out = std::to_string(value);
            return true;
        }
        case 'S': {
            if (name == "LanguageFile") out = cfg.LanguageFile;
            else if (name == "IPAddress") out = cfg.IPAddress;
            else if (name == "WebUIFile") out = cfg.WebUIFile;
            else if (name == "WebUIWebsitePath") out = cfg.WebUIWebsitePath;
            else if (name == "ServerLocate") out = cfg.ServerLocate;
            else if (name == "BackupFrom") out = cfg.BackupFrom;
            else if (name == "BackupTo") out = cfg.BackupTo;
            else if (name == "QQIPAddress") out = cfg.QQIPAddress;
            else if (name == "OwnerQQ") out = cfg.OwnerQQ;
            else if (name == "QQGroup") out = cfg.QQGroup;
            else return false;
            return true;
        }
        case 'C':
        default:
            return false;
    }
}

} // namespace AppCfg
