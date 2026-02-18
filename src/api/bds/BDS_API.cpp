#include "BDS_API.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <unordered_map>

#ifdef WIN32

namespace {

struct InstanceRuntime {
    BDSInstanceConfig cfg;

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    SECURITY_ATTRIBUTES sa{};

    HANDLE stdinWrite = NULL;
    HANDLE stdinRead = NULL;
    HANDLE stdoutRead = NULL;
    HANDLE stdoutWrite = NULL;

    std::mutex outputMutex;
    std::condition_variable outputCv;
    std::queue<std::string> outputQueue;
    bool outputReady = false;

    std::atomic<bool> running{false};
};

std::unordered_map<std::string, std::unique_ptr<InstanceRuntime>> g_instances;
std::mutex g_instancesMutex;
std::string g_defaultInstanceId;

std::string ResolveInstanceId(const std::string& input) {
    if (!input.empty()) {
        return input;
    }
    return g_defaultInstanceId;
}

InstanceRuntime* GetRuntimeUnsafe(const std::string& id) {
    auto it = g_instances.find(id);
    if (it == g_instances.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool IsProcessAlive(const InstanceRuntime& runtime) {
    if (!runtime.running.load()) {
        return false;
    }
    if (runtime.pi.hProcess == NULL) {
        return false;
    }
    DWORD waitRes = WaitForSingleObject(runtime.pi.hProcess, 0);
    return waitRes == WAIT_TIMEOUT;
}

void CleanupHandles(InstanceRuntime& runtime) {
    if (runtime.pi.hProcess != NULL) {
        CloseHandle(runtime.pi.hProcess);
        runtime.pi.hProcess = NULL;
    }
    if (runtime.pi.hThread != NULL) {
        CloseHandle(runtime.pi.hThread);
        runtime.pi.hThread = NULL;
    }
    if (runtime.stdinWrite != NULL) {
        CloseHandle(runtime.stdinWrite);
        runtime.stdinWrite = NULL;
    }
    if (runtime.stdinRead != NULL) {
        CloseHandle(runtime.stdinRead);
        runtime.stdinRead = NULL;
    }
    if (runtime.stdoutWrite != NULL) {
        CloseHandle(runtime.stdoutWrite);
        runtime.stdoutWrite = NULL;
    }
    if (runtime.stdoutRead != NULL) {
        CloseHandle(runtime.stdoutRead);
        runtime.stdoutRead = NULL;
    }
}

std::string TrimLine(std::string value) {
    while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
        value.pop_back();
    }
    return value;
}

std::string TrimRightSpace(std::string value) {
    auto isSpace = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };

    while (!value.empty() && isSpace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string NormalizeBdsLine(const std::string& input) {
    static const std::regex kNoLogPrefix(R"(^NO LOG FILE!\s*-\s*)");
    static const std::regex kNestedTimestamp(R"(^\[\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}(?::\d{3})?\s+[A-Z]+\]\s*)");
    static const std::regex kScriptingTag(R"(\[Scripting\]\s*)");

    std::string line = TrimRightSpace(input);
    if (line.empty()) {
        return line;
    }

    line = std::regex_replace(line, kNoLogPrefix, "");
    line = std::regex_replace(line, kNestedTimestamp, "");
    line = std::regex_replace(line, kScriptingTag, "");
    return TrimRightSpace(line);
}

std::string BuildBdsLogPrefix(const InstanceRuntime& runtime) {
    const std::string& tag = runtime.cfg.LogTag.empty() ? runtime.cfg.Id : runtime.cfg.LogTag;
    return "[BDS/" + tag + "] ";
}

void ReaderThread(InstanceRuntime* runtime) {
    char buffer[1024];
    DWORD bytesRead = 0;
    std::string remain;

    while (ReadFile(runtime->stdoutRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        remain.append(buffer, bytesRead);

        std::size_t pos = 0;
        while ((pos = remain.find('\n')) != std::string::npos) {
            std::string line = TrimLine(remain.substr(0, pos + 1));
            remain.erase(0, pos + 1);
            line = NormalizeBdsLine(line);

            if (!line.empty()) {
                INFO(BuildBdsLogPrefix(*runtime) + line);
                std::lock_guard<std::mutex> lock(runtime->outputMutex);
                runtime->outputQueue.push(line);
                runtime->outputReady = true;
                runtime->outputCv.notify_all();
            }
        }
    }

    if (!remain.empty()) {
        std::string line = TrimLine(remain);
        line = NormalizeBdsLine(line);
        if (!line.empty()) {
            INFO(BuildBdsLogPrefix(*runtime) + line);
            std::lock_guard<std::mutex> lock(runtime->outputMutex);
            runtime->outputQueue.push(line);
            runtime->outputReady = true;
            runtime->outputCv.notify_all();
        }
    }

    runtime->running.store(false);
}

bool StartServerInternal(InstanceRuntime& runtime) {
    if (IsProcessAlive(runtime)) {
        INFO("实例 " + runtime.cfg.Id + " 已在运行");
        return true;
    }

    ZeroMemory(&runtime.si, sizeof(runtime.si));
    runtime.si.cb = sizeof(runtime.si);
    ZeroMemory(&runtime.pi, sizeof(runtime.pi));

    runtime.sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    runtime.sa.bInheritHandle = TRUE;
    runtime.sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&runtime.stdinRead, &runtime.stdinWrite, &runtime.sa, 0)) {
        WARN("实例 " + runtime.cfg.Id + " 创建输入管道失败");
        return false;
    }
    if (!SetHandleInformation(runtime.stdinWrite, HANDLE_FLAG_INHERIT, 0)) {
        WARN("实例 " + runtime.cfg.Id + " 设置输入管道句柄失败");
        CleanupHandles(runtime);
        return false;
    }

    if (!CreatePipe(&runtime.stdoutRead, &runtime.stdoutWrite, &runtime.sa, 0)) {
        WARN("实例 " + runtime.cfg.Id + " 创建输出管道失败");
        CleanupHandles(runtime);
        return false;
    }
    if (!SetHandleInformation(runtime.stdoutRead, HANDLE_FLAG_INHERIT, 0)) {
        WARN("实例 " + runtime.cfg.Id + " 设置输出管道句柄失败");
        CleanupHandles(runtime);
        return false;
    }

    runtime.si.hStdError = runtime.stdoutWrite;
    runtime.si.hStdOutput = runtime.stdoutWrite;
    runtime.si.hStdInput = runtime.stdinRead;
    runtime.si.dwFlags |= STARTF_USESTDHANDLES;

    std::string commandLine = "\"" + runtime.cfg.ExecutablePath + "\"";
    std::vector<char> cmdline(commandLine.begin(), commandLine.end());
    cmdline.push_back('\0');

    const char* workingDir = runtime.cfg.WorkingDirectory.empty() ? nullptr : runtime.cfg.WorkingDirectory.c_str();

    if (!CreateProcessA(
        nullptr,
        cmdline.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        workingDir,
        &runtime.si,
        &runtime.pi)) {
        WARN("实例 " + runtime.cfg.Id + " 启动失败，错误码: " + std::to_string(GetLastError()));
        CleanupHandles(runtime);
        return false;
    }

    if (runtime.stdinRead != NULL) {
        CloseHandle(runtime.stdinRead);
        runtime.stdinRead = NULL;
    }
    if (runtime.stdoutWrite != NULL) {
        CloseHandle(runtime.stdoutWrite);
        runtime.stdoutWrite = NULL;
    }

    runtime.running.store(true);
    std::thread(ReaderThread, &runtime).detach();
    INFO("实例 " + runtime.cfg.Id + " 启动成功");
    return true;
}

bool StopServerInternal(InstanceRuntime& runtime) {
    if (!IsProcessAlive(runtime)) {
        INFO("实例 " + runtime.cfg.Id + " 未运行");
        CleanupHandles(runtime);
        runtime.running.store(false);
        return true;
    }

    const std::string stopCommand = "stop\n";
    DWORD written = 0;
    if (runtime.stdinWrite == NULL || !WriteFile(runtime.stdinWrite, stopCommand.c_str(), static_cast<DWORD>(stopCommand.size()), &written, NULL)) {
        WARN("实例 " + runtime.cfg.Id + " 发送 stop 失败，错误码: " + std::to_string(GetLastError()));
    }

    DWORD waitRes = WaitForSingleObject(runtime.pi.hProcess, 30000);
    if (waitRes == WAIT_TIMEOUT) {
        WARN("实例 " + runtime.cfg.Id + " 在 30 秒内未退出，尝试强制结束");
        TerminateProcess(runtime.pi.hProcess, 1);
        WaitForSingleObject(runtime.pi.hProcess, 5000);
    }

    CleanupHandles(runtime);
    runtime.running.store(false);
    INFO("实例 " + runtime.cfg.Id + " 已关闭");
    return true;
}

} // namespace

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            StopAllServers();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit(0);
        default:
            break;
    }
    return FALSE;
}

bool ConfigureBdsInstances(const std::vector<BDSInstanceConfig>& instances, const std::string& defaultInstanceId, std::string& error) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);

    if (instances.empty()) {
        error = "实例列表不能为空";
        return false;
    }

    for (auto& [_, runtime] : g_instances) {
        StopServerInternal(*runtime);
    }
    g_instances.clear();

    for (const auto& cfg : instances) {
        if (cfg.Id.empty()) {
            error = "实例 Id 不能为空";
            return false;
        }
        if (cfg.ExecutablePath.empty()) {
            error = "实例 ExecutablePath 不能为空: " + cfg.Id;
            return false;
        }
        if (g_instances.count(cfg.Id) > 0) {
            error = "实例 Id 重复: " + cfg.Id;
            return false;
        }

        auto runtime = std::make_unique<InstanceRuntime>();
        runtime->cfg = cfg;
        if (runtime->cfg.Name.empty()) {
            runtime->cfg.Name = cfg.Id;
        }
        if (runtime->cfg.LogTag.empty()) {
            runtime->cfg.LogTag = cfg.Id;
        }
        if (runtime->cfg.WorkingDirectory.empty()) {
            std::filesystem::path executablePath(cfg.ExecutablePath);
            runtime->cfg.WorkingDirectory = executablePath.parent_path().string();
        }
        g_instances[cfg.Id] = std::move(runtime);
    }

    if (defaultInstanceId.empty() || g_instances.count(defaultInstanceId) == 0) {
        g_defaultInstanceId = instances.front().Id;
    } else {
        g_defaultInstanceId = defaultInstanceId;
    }

    return true;
}

std::vector<std::string> ListServerInstances() {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    std::vector<std::string> ids;
    ids.reserve(g_instances.size());
    for (const auto& [id, _] : g_instances) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

bool SetDefaultServerInstance(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    if (g_instances.count(instanceId) == 0) {
        return false;
    }
    g_defaultInstanceId = instanceId;
    return true;
}

std::string GetDefaultServerInstance() {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    return g_defaultInstanceId;
}

std::string GetServerWorkingDirectory(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    const std::string id = ResolveInstanceId(instanceId);
    InstanceRuntime* runtime = GetRuntimeUnsafe(id);
    if (runtime == nullptr) {
        return "";
    }
    return runtime->cfg.WorkingDirectory;
}

bool IsCmdEnabledForInstance(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    const std::string id = ResolveInstanceId(instanceId);
    InstanceRuntime* runtime = GetRuntimeUnsafe(id);
    if (runtime == nullptr) {
        return false;
    }
    return runtime->cfg.UseCmd;
}

bool StartServer(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    const std::string id = ResolveInstanceId(instanceId);
    InstanceRuntime* runtime = GetRuntimeUnsafe(id);
    if (runtime == nullptr) {
        WARN("未找到实例: " + id);
        return false;
    }
    return StartServerInternal(*runtime);
}

bool StopServer(const std::string& instanceId) {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    const std::string id = ResolveInstanceId(instanceId);
    InstanceRuntime* runtime = GetRuntimeUnsafe(id);
    if (runtime == nullptr) {
        WARN("未找到实例: " + id);
        return false;
    }
    return StopServerInternal(*runtime);
}

void StopAllServers() {
    std::lock_guard<std::mutex> lock(g_instancesMutex);
    for (auto& [_, runtime] : g_instances) {
        StopServerInternal(*runtime);
    }
}

void BackupServer() {
    WARN("多实例模式下，BackupServer 入口尚未启用，请使用实例级备份任务");
}

std::string runCommand(const std::string& inputCommand, const std::string& instanceId) {
    if (inputCommand.empty()) {
        WARN("命令不能为空");
        return "命令不能为空";
    }

    std::unique_lock<std::mutex> managerLock(g_instancesMutex);
    const std::string id = ResolveInstanceId(instanceId);
    InstanceRuntime* runtime = GetRuntimeUnsafe(id);
    if (runtime == nullptr) {
        return "未找到实例: " + id;
    }

    if (!IsProcessAlive(*runtime)) {
        return "实例未运行: " + id;
    }

    std::string command = inputCommand;
    if (command == "stop") {
        managerLock.unlock();
        return StopServer(id) ? "实例已关闭" : "实例关闭失败";
    }
    if (command == "start") {
        managerLock.unlock();
        return StartServer(id) ? "实例已启动" : "实例启动失败";
    }

    command += "\n";
    DWORD written = 0;
    if (runtime->stdinWrite == NULL || !WriteFile(runtime->stdinWrite, command.c_str(), static_cast<DWORD>(command.size()), &written, NULL)) {
        WARN("向实例 " + id + " 发送命令失败，错误码: " + std::to_string(GetLastError()));
        return "发送命令失败";
    }

    INFO("[BDS:" + id + "] 已发送命令: " + inputCommand);

    std::unique_lock<std::mutex> outputLock(runtime->outputMutex);
    runtime->outputReady = false;
    if (!runtime->outputCv.wait_for(outputLock, std::chrono::seconds(3), [&runtime] { return runtime->outputReady; })) {
        return "实例在 3 秒内无输出";
    }

    std::string merged;
    while (!runtime->outputQueue.empty()) {
        if (!merged.empty()) {
            merged += "\n";
        }
        merged += runtime->outputQueue.front();
        runtime->outputQueue.pop();
    }
    runtime->outputReady = false;

    if (merged.empty()) {
        return "命令已发送";
    }
    return merged;
}

bool AddPlayerToWhitelist(const std::string& player_name, const std::string& instanceId) {
    const std::string command = "whitelist add \"" + player_name + "\"";
    const std::string result = runCommand(command, instanceId);
    return result.find("Player added to allowlist") != std::string::npos
        || result.find("Player already in allowlist") != std::string::npos;
}

bool RemovePlayerFromWhitelist(const std::string& player_name, const std::string& instanceId) {
    const std::string command = "whitelist remove \"" + player_name + "\"";
    const std::string result = runCommand(command, instanceId);
    return result.find("Player removed from allowlist") != std::string::npos;
}

#else

bool ConfigureBdsInstances(const std::vector<BDSInstanceConfig>&, const std::string&, std::string&) { return false; }
std::vector<std::string> ListServerInstances() { return {}; }
bool SetDefaultServerInstance(const std::string&) { return false; }
std::string GetDefaultServerInstance() { return ""; }
std::string GetServerWorkingDirectory(const std::string&) { return ""; }
bool IsCmdEnabledForInstance(const std::string&) { return false; }
bool StartServer(const std::string&) { return false; }
bool StopServer(const std::string&) { return false; }
void StopAllServers() {}
void BackupServer() {}
std::string runCommand(const std::string&, const std::string&) { return "暂不支持"; }
bool AddPlayerToWhitelist(const std::string&, const std::string&) { return false; }
bool RemovePlayerFromWhitelist(const std::string&, const std::string&) { return false; }

#endif
