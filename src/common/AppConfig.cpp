#include "AppConfig.hpp"

#include <fstream>
#include <unordered_set>

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

static bool ParseInstanceObject(const rapidjson::Value& instanceObj, BdsInstanceConfig& instance, std::string& error) {
    if (!instanceObj.IsObject()) {
        error = "字段 Instances[] 的元素必须是对象";
        return false;
    }

    if (!SetStringIfExists(&instanceObj, "Id", instance.Id, error)) return false;
    if (!SetStringIfExists(&instanceObj, "Name", instance.Name, error)) return false;
    if (!SetStringIfExists(&instanceObj, "ExecutablePath", instance.ExecutablePath, error)) return false;
    if (!SetStringIfExists(&instanceObj, "WorkingDirectory", instance.WorkingDirectory, error)) return false;
    if (!SetBoolIfExists(&instanceObj, "AutoStart", instance.AutoStart, error)) return false;
    if (!SetBoolIfExists(&instanceObj, "UseCmd", instance.UseCmd, error)) return false;
    if (!SetBoolIfExists(&instanceObj, "AutoBackup", instance.AutoBackup, error)) return false;
    if (!SetIntIfExists(&instanceObj, "BackupHour", instance.BackupHour, error)) return false;
    if (!SetIntIfExists(&instanceObj, "BackupMinute", instance.BackupMinute, error)) return false;
    if (!SetIntIfExists(&instanceObj, "BackupSecond", instance.BackupSecond, error)) return false;
    if (!SetStringIfExists(&instanceObj, "BackupFrom", instance.BackupFrom, error)) return false;
    if (!SetStringIfExists(&instanceObj, "BackupTo", instance.BackupTo, error)) return false;
    if (!SetStringIfExists(&instanceObj, "LogTag", instance.LogTag, error)) return false;

    if (instance.Id.empty()) {
        error = "字段 Instances[].Id 不能为空";
        return false;
    }
    if (instance.ExecutablePath.empty()) {
        error = "字段 Instances[].ExecutablePath 不能为空";
        return false;
    }
    return true;
}

bool LoadFromJsonFile(const std::string& path, Config& cfg, std::string& error) {
    rapidjson::Document doc;
    if (!ReadJsonFile(path, doc, error)) return false;

    Config loaded = DefaultConfig();

    const rapidjson::Value* base = nullptr;
    const rapidjson::Value* qqbot = nullptr;
    const rapidjson::Value* bds = nullptr;

    if (!GetObj(doc, "base", base, error)) return false;
    if (!GetObj(doc, "qqbot", qqbot, error)) return false;
    if (!GetObj(doc, "bds", bds, error)) return false;

    if (!SetStringIfExists(base, "LanguageFile", loaded.LanguageFile, error)) return false;
    if (!SetStringIfExists(base, "IPAddress", loaded.IPAddress, error)) return false;
    if (!SetIntIfExists(base, "ServerPort", loaded.ServerPort, error)) return false;
    if (!SetBoolIfExists(base, "EnableWebUI", loaded.EnableWebUI, error)) return false;
    if (!SetStringIfExists(base, "WebUIFile", loaded.WebUIFile, error)) return false;
    if (!SetStringIfExists(base, "WebUIWebsitePath", loaded.WebUIWebsitePath, error)) return false;

    if (!SetBoolIfExists(qqbot, "UseQQBot", loaded.UseQQBot, error)) return false;
    if (!SetStringIfExists(qqbot, "QQIPAddress", loaded.QQIPAddress, error)) return false;
    if (!SetIntIfExists(qqbot, "QQClientPort", loaded.QQClientPort, error)) return false;
    if (!SetIntIfExists(qqbot, "QQServerPort", loaded.QQServerPort, error)) return false;
    if (!SetStringIfExists(qqbot, "OwnerQQ", loaded.OwnerQQ, error)) return false;
    if (!SetStringIfExists(qqbot, "QQGroup", loaded.QQGroup, error)) return false;

    if (bds) {
        if (!SetStringIfExists(bds, "DefaultInstanceId", loaded.DefaultBdsInstanceId, error)) return false;
        if (!SetIntIfExists(bds, "AutoStartDelaySeconds", loaded.AutoStartDelaySeconds, error)) return false;
        if (bds->HasMember("Instances")) {
            const rapidjson::Value& arr = (*bds)["Instances"];
            if (!arr.IsArray()) {
                error = "字段 bds.Instances 必须是数组";
                return false;
            }
            loaded.BdsInstances.clear();
            std::unordered_set<std::string> seen;
            for (rapidjson::SizeType i = 0; i < arr.Size(); ++i) {
                BdsInstanceConfig item;
                if (!ParseInstanceObject(arr[i], item, error)) return false;
                if (!seen.insert(item.Id).second) {
                    error = "字段 bds.Instances 存在重复 Id: " + item.Id;
                    return false;
                }
                loaded.BdsInstances.push_back(item);
            }
        }
    }

    if (loaded.BdsInstances.empty()) {
        error = "bds.Instances 不能为空";
        return false;
    }

    if (loaded.DefaultBdsInstanceId.empty()) {
        loaded.DefaultBdsInstanceId = loaded.BdsInstances.front().Id;
    }

    if (loaded.AutoStartDelaySeconds < 0) {
        error = "字段 bds.AutoStartDelaySeconds 不能小于 0";
        return false;
    }

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

    rapidjson::Value qqbot(rapidjson::kObjectType);
    qqbot.AddMember("UseQQBot", cfg.UseQQBot, alloc);
    qqbot.AddMember("QQIPAddress", rapidjson::Value(cfg.QQIPAddress.c_str(), alloc), alloc);
    qqbot.AddMember("QQClientPort", cfg.QQClientPort, alloc);
    qqbot.AddMember("QQServerPort", cfg.QQServerPort, alloc);
    qqbot.AddMember("OwnerQQ", rapidjson::Value(cfg.OwnerQQ.c_str(), alloc), alloc);
    qqbot.AddMember("QQGroup", rapidjson::Value(cfg.QQGroup.c_str(), alloc), alloc);
    doc.AddMember("qqbot", qqbot, alloc);

    rapidjson::Value bds(rapidjson::kObjectType);
    bds.AddMember("DefaultInstanceId", rapidjson::Value(cfg.DefaultBdsInstanceId.c_str(), alloc), alloc);
    bds.AddMember("AutoStartDelaySeconds", cfg.AutoStartDelaySeconds, alloc);
    rapidjson::Value instances(rapidjson::kArrayType);
    for (const auto& item : cfg.BdsInstances) {
        rapidjson::Value instance(rapidjson::kObjectType);
        instance.AddMember("Id", rapidjson::Value(item.Id.c_str(), alloc), alloc);
        instance.AddMember("Name", rapidjson::Value(item.Name.c_str(), alloc), alloc);
        instance.AddMember("ExecutablePath", rapidjson::Value(item.ExecutablePath.c_str(), alloc), alloc);
        instance.AddMember("WorkingDirectory", rapidjson::Value(item.WorkingDirectory.c_str(), alloc), alloc);
        instance.AddMember("AutoStart", item.AutoStart, alloc);
        instance.AddMember("UseCmd", item.UseCmd, alloc);
        instance.AddMember("AutoBackup", item.AutoBackup, alloc);
        instance.AddMember("BackupHour", item.BackupHour, alloc);
        instance.AddMember("BackupMinute", item.BackupMinute, alloc);
        instance.AddMember("BackupSecond", item.BackupSecond, alloc);
        instance.AddMember("BackupFrom", rapidjson::Value(item.BackupFrom.c_str(), alloc), alloc);
        instance.AddMember("BackupTo", rapidjson::Value(item.BackupTo.c_str(), alloc), alloc);
        instance.AddMember("LogTag", rapidjson::Value(item.LogTag.c_str(), alloc), alloc);
        instances.PushBack(instance, alloc);
    }
    bds.AddMember("Instances", instances, alloc);
    doc.AddMember("bds", bds, alloc);

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
            else if (name == "UseQQBot") value = cfg.UseQQBot;
            else return false;
            out = value ? "1" : "0";
            return true;
        }
        case 'I': {
            int value;
            if (name == "ServerPort") value = cfg.ServerPort;
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
