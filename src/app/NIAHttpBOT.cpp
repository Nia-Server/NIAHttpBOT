/*

Copyright (C) 2019-2026 Nia-Server

The developer is not responsible for you, and the developer is not obliged to write code for you, and is not liable for any consequences of your use.

In addition, you are required to comply with the terms of the AGPL-3.0 (https://github.com/Nia-Server/NIAHttpBOT/blob/main/LICENSE) open source licence for this project.

If you do not accept these terms, please delete this project immediately.

authors: NIANIANKNIA && jiansyuan

email: dev@mcnia.com

Project address: https://github.com/Nia-Server/NIAHttpBOT

If you have any problems with this project, please contact the authors.

*/


#include <ctime>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <syncstream>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <sstream>
#include <queue>
#include <vector>
#include <clocale>


#ifdef WIN32 //only enable TLS in windows
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <process.h>
#else
#include <unistd.h>
#endif

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>



#include "AppConfig.hpp"
#include "I18Nize.hpp"
#include "Logger.hpp"

#include "QQBot.h"
#include "QQBot_API.h"
#include "File_API.h"
#include "Game_API.h"
#include "BDS_API.h"
#include "HttpV1.hpp"


//定义版本号
#define VERSION "v1.1.0"


std::string LanguageFile = "";
std::string IPAddress = "127.0.0.1";
int ServerPort = 2333;

std::vector<BDSInstanceConfig> BdsInstances;
std::string DefaultBdsInstanceId = "default";
int AutoStartDelaySeconds = 5;

bool UseQQBot = false;
std::string QQIPAddress = "127.0.0.1";

int QQClientPort = 10023;
int QQServerPort = 10086;
std::string OwnerQQ = "123456789";
std::string QQGroup = "123456789";

namespace {

BDSInstanceConfig ToRuntimeInstanceConfig(const AppCfg::BdsInstanceConfig& item) {
	BDSInstanceConfig cfg;
	cfg.Id = item.Id;
	cfg.Name = item.Name;
	cfg.ExecutablePath = item.ExecutablePath;
	cfg.WorkingDirectory = item.WorkingDirectory;
	cfg.AutoStart = item.AutoStart;
	cfg.UseCmd = item.UseCmd;
	cfg.AutoBackup = item.AutoBackup;
	cfg.BackupHour = item.BackupHour;
	cfg.BackupMinute = item.BackupMinute;
	cfg.BackupSecond = item.BackupSecond;
	cfg.BackupFrom = item.BackupFrom;
	cfg.BackupTo = item.BackupTo;
	cfg.LogTag = item.LogTag;
	return cfg;
}

AppCfg::BdsInstanceConfig ToAppConfigInstance(const BDSInstanceConfig& item) {
	AppCfg::BdsInstanceConfig cfg;
	cfg.Id = item.Id;
	cfg.Name = item.Name;
	cfg.ExecutablePath = item.ExecutablePath;
	cfg.WorkingDirectory = item.WorkingDirectory;
	cfg.AutoStart = item.AutoStart;
	cfg.UseCmd = item.UseCmd;
	cfg.AutoBackup = item.AutoBackup;
	cfg.BackupHour = item.BackupHour;
	cfg.BackupMinute = item.BackupMinute;
	cfg.BackupSecond = item.BackupSecond;
	cfg.BackupFrom = item.BackupFrom;
	cfg.BackupTo = item.BackupTo;
	cfg.LogTag = item.LogTag;
	return cfg;
}

void ApplyLoadedConfig(const AppCfg::Config& appConfig) {
	LanguageFile = appConfig.LanguageFile;
	IPAddress = appConfig.IPAddress;
	ServerPort = appConfig.ServerPort;
	DefaultBdsInstanceId = appConfig.DefaultBdsInstanceId;
	AutoStartDelaySeconds = appConfig.AutoStartDelaySeconds;
	BdsInstances.clear();
	BdsInstances.reserve(appConfig.BdsInstances.size());
	for (const auto& item : appConfig.BdsInstances) {
		BdsInstances.push_back(ToRuntimeInstanceConfig(item));
	}
	UseQQBot = appConfig.UseQQBot;
	QQIPAddress = appConfig.QQIPAddress;
	QQClientPort = appConfig.QQClientPort;
	QQServerPort = appConfig.QQServerPort;
	OwnerQQ = appConfig.OwnerQQ;
	QQGroup = appConfig.QQGroup;
}

AppCfg::Config BuildCurrentConfigSnapshot() {
	AppCfg::Config currentCfg;
	currentCfg.LanguageFile = LanguageFile;
	currentCfg.IPAddress = IPAddress;
	currentCfg.ServerPort = ServerPort;
	currentCfg.DefaultBdsInstanceId = GetDefaultServerInstance();
	if (currentCfg.DefaultBdsInstanceId.empty()) {
		currentCfg.DefaultBdsInstanceId = DefaultBdsInstanceId;
	}
	currentCfg.AutoStartDelaySeconds = AutoStartDelaySeconds;
	currentCfg.BdsInstances.reserve(BdsInstances.size());
	for (const auto& item : BdsInstances) {
		currentCfg.BdsInstances.push_back(ToAppConfigInstance(item));
	}
	currentCfg.UseQQBot = UseQQBot;
	currentCfg.QQIPAddress = QQIPAddress;
	currentCfg.QQClientPort = QQClientPort;
	currentCfg.QQServerPort = QQServerPort;
	currentCfg.OwnerQQ = OwnerQQ;
	currentCfg.QQGroup = QQGroup;
	return currentCfg;
}

}

#ifdef WIN32


#define popen _popen
#define pclose _pclose
#define WEXITSTATUS




void sslThread(){
	//与GitHub的API通信来检查更新
	httplib::SSLClient cli("api.github.com");
	auto res = cli.Get("/repos/Nia-Server/NIAHttpBOT/releases");
	if (res && res->status == 200) {
		rapidjson::Document doc;
		doc.Parse(res->body.c_str());
		if (doc.HasParseError()) {
			WARN("解析JSON数据失败！");
			return;
		}
		if (doc.Size() == 0) {
			WARN("未找到任何版本信息！");
			return;
		}
		std::string latest_version = doc[0]["tag_name"].GetString();
		std::string changelog = doc[0]["body"].GetString();
		std::string download_url = doc[0]["html_url"].GetString();
		std::string current_version = VERSION;

		// 分割版本号和预发布版本号
		std::string latest_main_version = latest_version;
		std::string current_main_version = current_version;
		std::string latest_pre_version, current_pre_version;
		if (latest_version.find("-") != std::string::npos) {
			latest_main_version = latest_version.substr(0, latest_version.find("-"));
			latest_pre_version = latest_version.substr(latest_version.find("-") + 1);
		}
		if (current_version.find("-") != std::string::npos) {
			current_main_version = current_version.substr(0, current_version.find("-"));
			current_pre_version = current_version.substr(current_version.find("-") + 1);
		}

		bool is_latest_greater = false;
		if (latest_main_version != current_main_version) {
			// 如果主版本号不同，直接比较主版本号
			is_latest_greater = latest_main_version > current_main_version;
		} else if (!latest_pre_version.empty() && !current_pre_version.empty()) {
			// 如果主版本号相同，并且都有预发布版本号，比较预发布版本号
			is_latest_greater = latest_pre_version > current_pre_version;
		} else if (!latest_pre_version.empty() && current_pre_version.empty()) {
			// 如果主版本号相同，最新版本有预发布版本号，当前版本没有预发布版本号，那么当前版本是最新的
			is_latest_greater = false;
		} else if (latest_pre_version.empty() && !current_pre_version.empty()) {
			// 如果主版本号相同，最新版本没有预发布版本号，当前版本有预发布版本号，那么最新版本是最新的
			is_latest_greater = true;
		}

		if (is_latest_greater) {
			//如果是预发布版本，输出警告
			if (latest_version.find("-pre-") != std::string::npos) {
				WARN("检查更新成功，当前github的release最新版本为：" + latest_version + "，当前版本为：" + VERSION + "，但请注意，这是一个预发布版本。可能存在未知的问题，请根据实际情况更新");
			} else {
				WARN("检查更新成功，当前github的release最新版本为：" + latest_version + "，当前版本为：" + VERSION + "，请及时更新");
			}
			//向控制台输出最新版本release更新日志
			INFO(changelog);
			INFO("下载地址：" + download_url);
		} else {
			INFO("检查更新成功，当前版本已是最新版本");
		}
	} else {
		//向控制台输出错误状态码
		WARN("检查更新失败，错误状态码：" + res->status);
		WARN("检查更新失败");
	}

}

void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void sslThread(){
}
#endif

signed int main(signed int argc, char** argv) {

	const std::string configPath = "./NIAHttpBOT.json";
	AppCfg::Config appConfig = AppCfg::DefaultConfig();
	#ifdef WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	std::setlocale(LC_ALL, ".UTF-8");
    EnableVirtualTerminalProcessing();
    #endif

	std::cout << "\033]0;NIAHttpBOT " << VERSION <<"\007";

	std::atexit([]() {
		StopAllServersForExit();
	});


	//检测是否有其他进程正在运行&&终端关闭检测
	#ifdef WIN32
		SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		HANDLE hMutex = CreateMutex(NULL, FALSE, "NIAHttpBOT");
		if (hMutex == NULL) {
			WARN("CreateMutex failed!");
			std::this_thread::sleep_for(std::chrono::seconds(3));
			return 1;
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			WARN("检测到已有进程正在运行，3s后即将关闭当前进程。");
			std::this_thread::sleep_for(std::chrono::seconds(3));
			CloseHandle(hMutex);
			return 1;
		}
	#else
		int fd = open("lockfile", O_RDWR | O_CREAT, 0666);
		if (fd < 0) {
			WARN("open lockfile failed");
			std::this_thread::sleep_for(std::chrono::seconds(3));
			return 1;
		}
		if (lockf(fd, F_TLOCK, 0) < 0) {
			WARN("检测到已有进程正在运行，3s后即将关闭当前进程。");
			std::this_thread::sleep_for(std::chrono::seconds(3));
			close(fd);
			return 1;
		}
	#endif

	std::ios::sync_with_stdio(false), std::cin.tie(0), std::cout.tie(0);

	std::cout<<"\x1b[36m"<<R"(
    __/\\\\\_____/\\\___/\\\\\\\\\\\______/\\\\\\\\\____
     _\/\\\\\\___\/\\\__\/////\\\///_____/\\\\\\\\\\\\\__
      _\/\\\/\\\__\/\\\______\/\\\_______/\\\/////////\\\_
       _\/\\\//\\\_\/\\\______\/\\\______\/\\\_______\/\\\_
        _\/\\\\//\\\\/\\\______\/\\\______\/\\\\\\\\\\\\\\\_
         _\/\\\_\//\\\/\\\______\/\\\______\/\\\/////////\\\_
          _\/\\\__\//\\\\\\______\/\\\______\/\\\_______\/\\\_
           _\/\\\___\//\\\\\___/\\\\\\\\\\\__\/\\\_______\/\\\_
            _\///_____\/////___\///////////___\///________\///__
    )";

	std::cout<<"\x1b[0;32m"<<
	R"(
	   _  _    _____   _____     ___             ___     ___    _____
    o O O | || |  |_   _| |_   _|   | _ \    o O O  | _ )   / _ \  |_   _|
   o      | __ |    | |     | |     |  _/   o       | _ \  | (_) |   | |
  TS__[O] |_||_|   _|_|_   _|_|_   _|_|_   TS__[O]  |___/   \___/   _|_|_
 {======|_|"""""|_|"""""|_|"""""|_| """ | {======|_|"""""|_|"""""|_|"""""|
./o--000'"`-0-0-'"`-0-0-'"`-0-0-'"`-0-0-'./o--000'"`-0-0-'"`-0-0-'"`-0-0-'
	)" <<"\x1b[0m"<< std::endl;


	INFO("当前版本：" + std::string(VERSION) + " 构建时间: " + std::string(__DATE__) + " " + std::string(__TIME__) + " (UTC +8)");
	//解析版本号，如果版本号后面有-pre-则输出警告这是一个预览版本
	if (std::string(VERSION).find("-pre-") != std::string::npos) {
		WARN("这是一个预发布版本，仅供开发者预览，不要在正式生产环境中使用");
	}

	std::string configError;
	if (!std::filesystem::exists(configPath)) {
		if (!AppCfg::SaveToJsonFile(configPath, appConfig, configError)) {
			FAIL("初始化 JSON 配置文件失败: " + configError);
			return 1;
		}
		WARN("未找到配置文件，已自动初始化配置文件 NIAHttpBOT.json");
	}

	if (!AppCfg::LoadFromJsonFile(configPath, appConfig, configError)) {
		FAIL("读取 JSON 配置文件失败: " + configError);
		return 1;
	}

	ApplyLoadedConfig(appConfig);

	if (BdsInstances.empty()) {
		FAIL("BDS 实例列表为空，请检查 NIAHttpBOT.json 中 bds.Instances 配置");
		return 1;
	}

	if (DefaultBdsInstanceId.empty()) {
		DefaultBdsInstanceId = BdsInstances.front().Id;
	}

	if (!ConfigureBdsInstances(BdsInstances, DefaultBdsInstanceId, configError)) {
		FAIL("初始化 BDS 实例失败: " + configError);
		return 1;
	}
	DefaultBdsInstanceId = GetDefaultServerInstance();

	INFO("已成功读取配置文件");
	if (LanguageFile.empty()) INFO("已使用默认语言");
	else if (!i18n.loadFromFile(LanguageFile)) WARN("语言文件加载失败");
	else XINFO("语言配置已加载成功");

	INFO(XX("sapi事件监听服务器已在 http://") + IPAddress + ":" + std::to_string(ServerPort) + XX(" 上成功启动"));
	if (UseQQBot) {
		INFO(XX("qq-bot事件监听服务器已在 http://") + QQIPAddress + ":" + std::to_string(QQServerPort) + XX(" 上成功启动"));
		INFO(XX("qq-bot客户端已在 http://") + QQIPAddress + ":" + std::to_string(QQClientPort) + XX(" 上成功启动"));
	}
	XINFO("项目地址：https://github.com/Nia-Server/NIAHttpBOT/");
	XINFO("项目作者：@NIANIANKNIA @jiansyuan");
	XINFO("在使用中遇到问题请前往项目下的 issue 反馈，如果觉得本项目不错不妨点个 star");
	for (const auto& item : BdsInstances) {
		if (item.UseCmd) {
			XWARN("检测到实例 " + item.Id + " 已启用执行DOS命令功能，请注意服务器安全");
		}
	}


	#ifdef WIN32
	// std::thread ssl_thread(sslThread);
	// ssl_thread.detach();
	sslThread();


	#endif


	//初始化服务器
	httplib::Server svr;
	httplib::Server qqsvr;

    svr.Post("/GetConfig", [](const httplib::Request& req, httplib::Response& res){
		rapidjson::Document req_json;
		req_json.Parse(req.body.c_str()), res.status = 400;
		if(req_json.HasParseError()||!req_json.HasMember("Name")||!req_json.HasMember("Type")
			||!req_json["Name"].IsString()||!req_json["Type"].IsString()||req_json["Type"].GetStringLength()!=1) [[unlikely]]
			return res.set_content("json data error", "text/plain");
		const AppCfg::Config currentCfg = BuildCurrentConfigSnapshot();

		std::string value;
		const std::string name = req_json["Name"].GetString();
		const char type = req_json["Type"].GetString()[0];
		if (!AppCfg::GetValueByType(currentCfg, name, type, value)) {
			res.status = 400;
			return res.set_content("config key/type error", "text/plain");
		}

		res.status = 200;
		res.set_content(value, "text/plain");

	});

	//执行cmd命令
	svr.Post("/RunCmd",  [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string cmd;
		if (!HttpV1::RequireString(request, res, "cmd", cmd)) {
			return;
		}

		const std::string serverId = HttpV1::GetServerId(request);

		//首先判断配置文件是否启用
		if (!IsCmdEnabledForInstance(serverId)) [[unlikely]] {
			XWARN("执行DOS命令的功能暂未启用，请在启用后使用");
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		WARN(XX("收到一条执行DOS命令的请求：") + cmd);
		auto [cmdres, excd] = ([&cmd]() -> std::pair<std::string, int> {
			int exitCode = 0;
			std::array<char, 64> buffer {};
			std::string result;
			FILE *pipe = popen(cmd.c_str(), "r");
			if (pipe == nullptr) [[unlikely]] return {"popen() error!!", -114514};
			std::size_t bytesRead;
			while ((bytesRead = std::fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0)
				result += std::string(buffer.data(), bytesRead);
			exitCode = WEXITSTATUS(pclose(pipe));
			return {result, exitCode};
		})();
		if(!cmdres.empty() && cmdres.back() == '\n') [[likely]] cmdres.pop_back();
		INFO(XX("命令执行输出: ") + cmdres);
		if (excd!=0) [[unlikely]] WARN(XXX("命令执行失败, 返回值: ")+std::to_string(excd));
		else XINFO("命令执行成功！返回值: 0");

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("output", rapidjson::Value(cmdres.c_str(), allocator), allocator);
		data.AddMember("exit_code", excd, allocator);
		HttpV1::RespondSuccess(res, &data);
	});

	//qq机器人主函数
	main_qqbot(qqsvr);
	init_qq_API(svr);

	//初始化游戏API
	init_game_API(svr);

	//初始化文件API
	init_file_API(svr);

	//按实例自动启动服务器
	int autoStartPending = 0;
	for (const auto& item : BdsInstances) {
		if (item.AutoStart) {
			autoStartPending++;
		}
	}

	for (const auto& item : BdsInstances) {
		if (item.AutoStart) {
			StartServer(item.Id);
			autoStartPending--;
			if (autoStartPending > 0 && AutoStartDelaySeconds > 0) {
				std::this_thread::sleep_for(std::chrono::seconds(AutoStartDelaySeconds));
			}
		}
	}

	//监听终端命令输入
	using CommandHandler = std::function<void(const std::vector<std::string>&)>;
	auto printCommandResult = [](const std::string& instanceId, const std::string& result) {
		std::istringstream resultStream(result);
		std::string line;
		bool printed = false;
		while (std::getline(resultStream, line)) {
			INFO("[" + instanceId + "] " + line);
			printed = true;
		}
		if (!printed) {
			INFO("[" + instanceId + "]");
		}
	};

    std::unordered_map<std::string, CommandHandler> commandMap;

    commandMap["help"] = [](const std::vector<std::string>&) {
        std::cout << "可用指令列表：" << std::endl;
        std::cout << "  restart - 重启程序" << std::endl;
        std::cout << "  stop - 关闭程序" << std::endl;
		std::cout << "  listserver - 列出已配置实例" << std::endl;
		std::cout << "  use <id> - 切换默认实例" << std::endl;
		std::cout << "  startserver <id> - 启动指定实例" << std::endl;
		std::cout << "  mc <id> <command> - 向指定实例发送命令并显示回传" << std::endl;
		std::cout << "  /<id> <command> - 直接向指定实例发送命令并显示回传" << std::endl;
		std::cout << "  stopserver <id> - 关闭指定实例" << std::endl;
    };

	std::string programName = argv[0];
	commandMap["restart"] = [programName](const std::vector<std::string>&) {
        INFO("1s后重启程序..." );
        std::this_thread::sleep_for(std::chrono::seconds(1));
        #ifdef _WIN32
			StopAllServersForExit();
			STARTUPINFOA si2;
			PROCESS_INFORMATION pi2;
			ZeroMemory(&si2, sizeof(si2));
			si2.cb = sizeof(si2);
			ZeroMemory(&pi2, sizeof(pi2));
			if(!CreateProcessA(programName.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si2, &pi2)) {
				WARN("重新启动进程失败：" + std::to_string(GetLastError()));
			} else {
				INFO("已成功重新启动进程!");
			}
        #else
			if (fork() == 0) {
				execl(programName.c_str(), programName.c_str(), (char*)NULL);
			}
        #endif
        exit(0);
    };

	commandMap["listserver"] = [](const std::vector<std::string>&) {
		auto ids = ListServerInstances();
		INFO("当前默认实例: " + GetDefaultServerInstance());
		for (const auto& id : ids) {
			INFO("- " + id);
		}
	};

	commandMap["use"] = [](const std::vector<std::string>& args) {
		if (args.size() < 2) {
			WARN("use 指令需要参数: <id>");
			return;
		}
		if (!SetDefaultServerInstance(args[1])) {
			WARN("切换默认实例失败，未找到实例: " + args[1]);
			return;
		}
		DefaultBdsInstanceId = GetDefaultServerInstance();
		INFO("默认实例已切换到: " + args[1]);
	};

	commandMap["startserver"] = [](const std::vector<std::string>& args) {
		if (args.size() < 2) {
			WARN("startserver 指令需要参数: <id>");
			return;
		}
		StartServer(args[1]);
	};

	commandMap["mc"] = [&printCommandResult](const std::vector<std::string>& args) {
		if (args.size() < 3) {
			WARN("mc 指令需要参数: <id> <command>");
			return;
		}
		std::string instanceId = args[1];
		std::string command = args[2];
		for (int i = 3; i < args.size(); i++) {
			command += " " + args[i];
		}
		std::string result = runCommand(command, instanceId);
		printCommandResult(instanceId, result);
	};

	commandMap["whitelist"] = [](const std::vector<std::string>& args) {
		if (args.size() < 4) {
			WARN("whitelist 指令格式: whitelist <id> <add|remove> <player_name>");
			return;
		}
		std::string instanceId = args[1];
		if (args[2] == "add") {
			std::string player_name = args[3];
			if (AddPlayerToWhitelist(player_name, instanceId)) {
				INFO("已成功添加玩家 " + player_name + " 到白名单");
			} else {
				WARN("添加玩家 " + player_name + " 到白名单失败");
			}
		} else if (args[2] == "remove") {
			std::string player_name = args[3];
			if (RemovePlayerFromWhitelist(player_name, instanceId)) {
				INFO("已成功从白名单中移除玩家 " + player_name);
			} else {
				WARN("从白名单中移除玩家 " + player_name + " 失败");
			}
		} else {
			WARN("未知子命令: " + args[2] + "，可选 add/remove");
		}
	};


	//关闭程序,向bedrock_server.exe中发送stop
	commandMap["stopserver"] = [](const std::vector<std::string>& args) {
		if (args.size() < 2) {
			WARN("stopserver 指令需要参数: <id>");
			return;
		}
		StopServer(args[1]);
	};

    commandMap["stop"] = [](const std::vector<std::string>&) {
        INFO("1s后将关闭程序...");
		StopAllServersForExit();
        #ifdef _WIN32
			std::this_thread::sleep_for(std::chrono::seconds(1));
			exit(0);
		#else
		std::this_thread::sleep_for(std::chrono::seconds(1));
        exit(0);
		#endif
    };

	// 启动输入监听线程
	std::thread inputThread([&commandMap, &printCommandResult]() {
		std::string line;
		while (true) {
			if (!std::getline(std::cin, line)) {
				// Avoid high CPU usage if stdin is closed (e.g. running as service)
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (std::cin.eof()) continue; // Keep running if intention is to stay alive
				std::cin.clear(); // Clear error state to retry
				continue; 
			}
            std::istringstream iss(line);
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) {
                tokens.push_back(token);
            }
            if (tokens.empty()) {
                continue;
            }
			if (tokens[0][0] == '/') {
				if (tokens.size() < 2) {
					WARN("斜杠命令格式: /<实例ID> <command>");
					continue;
				}
				std::string instanceId = tokens[0].substr(1);
				std::string command = tokens[1];
				for (int i = 2; i < tokens.size(); i++) {
					command += " " + tokens[i];
				}
				std::string result = runCommand(command, instanceId);
				printCommandResult(instanceId, result);
				continue;
			}
            auto it = commandMap.find(tokens[0]);
            if (it != commandMap.end()) {
                it->second(tokens); // 调用对应的处理函数
            } else {
                WARN("未知指令: " + tokens[0] + "，输入 help 查看帮助");
            }
        }
    });
	std::thread([&svr]() {
		svr.listen(IPAddress, ServerPort);
	}).detach();

	std::thread([&qqsvr]() {
		qqsvr.listen(QQIPAddress, QQServerPort);
	}).detach();

    // 等待输入线程完成
    inputThread.join();

	return 0;
}
