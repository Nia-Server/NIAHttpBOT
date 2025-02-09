#include "BDS_API.h"

extern int BackupHour;
extern int BackupMinute;
extern int BackupSecond;
extern std::string BackupFrom;
extern std::string BackupTo;


#ifdef WIN32
STARTUPINFO si;
PROCESS_INFORMATION pi;
SECURITY_ATTRIBUTES sa;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;
bool isCommand = false;
std::queue<std::string> g_McOutputQueue;
std::mutex g_McOutputMutex;
std::condition_variable g_McOutputCV;
bool g_McOutputReady = false;

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
        case CTRL_CLOSE_EVENT:
            // 检查 bedrock_server.exe 是否在运行
            if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
                INFO("检测到 bedrock_server.exe 正在运行，发送 stop 指令...");
                const char* command = "stop\n";
                DWORD written;
                if (!WriteFile(g_hChildStd_IN_Wr, command, strlen(command), &written, NULL)) {
                    WARN("向 bedrock_server.exe 发送 stop 命令失败!");
                } else {
                    INFO("已向 bedrock_server.exe 发送 stop 命令!");
                }

                // 等待子进程结束
                WaitForSingleObject(pi.hProcess, INFINITE);

                // 关闭进程和线程句柄
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                CloseHandle(g_hChildStd_IN_Wr);
                CloseHandle(g_hChildStd_IN_Rd);
                CloseHandle(g_hChildStd_OUT_Wr);
                CloseHandle(g_hChildStd_OUT_Rd);
                INFO("bedrock_server.exe 已成功关闭!");
            } else {
                INFO("bedrock_server.exe 未运行，直接关闭程序...");
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit(0);
            break;
        case CTRL_LOGOFF_EVENT:
            // 检查 bedrock_server.exe 是否在运行
            if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
                INFO("检测到 bedrock_server.exe 正在运行，发送 stop 指令...");
                const char* command = "stop\n";
                DWORD written;
                if (!WriteFile(g_hChildStd_IN_Wr, command, strlen(command), &written, NULL)) {
                    WARN("向 bedrock_server.exe 发送 stop 命令失败!");
                } else {
                    INFO("已向 bedrock_server.exe 发送 stop 命令!");
                }

                // 等待子进程结束
                WaitForSingleObject(pi.hProcess, INFINITE);

                // 关闭进程和线程句柄
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                CloseHandle(g_hChildStd_IN_Wr);
                CloseHandle(g_hChildStd_IN_Rd);
                CloseHandle(g_hChildStd_OUT_Wr);
                CloseHandle(g_hChildStd_OUT_Rd);
                INFO("bedrock_server.exe 已成功关闭!");
            } else {
                INFO("bedrock_server.exe 未运行，直接关闭程序...");
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit(0);
            break;
        case CTRL_SHUTDOWN_EVENT:
            // 检查 bedrock_server.exe 是否在运行
            if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
                INFO("检测到 bedrock_server.exe 正在运行，发送 stop 指令...");
                const char* command = "stop\n";
                DWORD written;
                if (!WriteFile(g_hChildStd_IN_Wr, command, strlen(command), &written, NULL)) {
                    WARN("向 bedrock_server.exe 发送 stop 命令失败!");
                } else {
                    INFO("已向 bedrock_server.exe 发送 stop 命令!");
                }

                // 等待子进程结束
                WaitForSingleObject(pi.hProcess, INFINITE);

                // 关闭进程和线程句柄
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                CloseHandle(g_hChildStd_IN_Wr);
                CloseHandle(g_hChildStd_IN_Rd);
				CloseHandle(g_hChildStd_OUT_Wr);
                CloseHandle(g_hChildStd_OUT_Rd);
                INFO("bedrock_server.exe 已成功关闭!");
            } else {
                INFO("bedrock_server.exe 未运行，直接关闭程序...");
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            exit(0);
            break;
        default:
            break;
    }
    return FALSE;
}
#endif


bool StartServer() {
    #ifdef WIN32
    INFO("正在启动BDS服务器...");
    if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
        INFO("检测到服务器正在运行，无需重新启动！");
        return true;
    } else {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0);
        SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);
        CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0);
        SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

        si.hStdError = g_hChildStd_OUT_Wr;
        si.hStdOutput = g_hChildStd_OUT_Wr;
        si.hStdInput = g_hChildStd_IN_Rd;
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (!CreateProcess(NULL, const_cast<char*>(ServerLocate.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            WARN("服务器启动失败：" + std::to_string(GetLastError()));
            return false;
        } else {
            INFO("服务器已启动成功...");
            std::thread outThread([](){
                char buffer[1024];
                DWORD bytesRead = 0;
                std::string a1, a2;
                a2.clear();
                int tot1 = 0, tot2 = 0;
                while (true) {
                    if (!ReadFile(g_hChildStd_OUT_Rd, buffer, sizeof(buffer), &bytesRead, NULL)) {
                        break;
                    }
                    std::string output(buffer, bytesRead);
                    a2+=output;
                    std::vector<int> vec;
                    for(int i=0;a2[i];i++){
                        if(a2[i]=='\n')  vec.push_back(i);
                    }
                    for (int i=0, last=0; i<vec.size(); i++){
                        a1 = a2.substr(last, vec[i]-last+1);
                        last = vec[i]+1;
                        if(a1[0]=='['&&a1[1]=='2'&&a1[a1.size()-1]=='\n')
                        SYNC(std::cout)<<([](const std::string& a) -> std::string {
                                std::string res;
                                if(a[25]=='I')
                                    res += (std::string)"\x1b[35m[" 
                                    + a[1]+a[2]+a[3]+a[4]+'/'+a[6]+a[7]+'/'+a[9]+a[10]+ ' ' 
                                    + a[12]+a[13]+a[14]+a[15]+a[16]+a[17]+a[18]+a[19]+a[20]+a[21]+a[22]+a[23] 
                                    + "] \x1b[0m" + "\x1b[32m[INFO]\x1b[0m "
                                    + a.substr(31);
                                else if(a[25]=='W')
                                    res += (std::string)"\x1b[35m[" 
                                    + a[1]+a[2]+a[3]+a[4]+'/'+a[6]+a[7]+'/'+a[9]+a[10]+ ' ' 
                                    + a[12]+a[13]+a[14]+a[15]+a[16]+a[17]+a[18]+a[19]+a[20]+a[21]+a[22]+a[23] 
                                    + "] \x1b[0m" + "\x1b[43;1m[WARN]\x1b[0m "
                                    + a.substr(31);
                                else if(a[25]=='E')
                                    res += (std::string)"\x1b[35m[" 
                                    + a[1]+a[2]+a[3]+a[4]+'/'+a[6]+a[7]+'/'+a[9]+a[10]+ ' ' 
                                    + a[12]+a[13]+a[14]+a[15]+a[16]+a[17]+a[18]+a[19]+a[20]+a[21]+a[22]+a[23] 
                                    + "] \x1b[0m" + "\x1b[41;1m[FAIL]\x1b[0m "
                                    + a.substr(32);
                                //if(a[32]=='S' && a[a.size()-1]==a[a.size()-2]) res.pop_back();
                                return res;
                            })(a1), tot1++;
                        else if(a1.size()>=5&&a1[a1.size()-1]=='\n')SYNC(std::cout)<<a1, tot1++;
                    }
                    a2 = a2.substr(vec[vec.size()-1], a2.size()-vec[vec.size()-1]);
                    tot2++;
                    // 这里将输出转发到 g_McOutputQueue
                    if (isCommand) {
                        std::lock_guard<std::mutex> lock(g_McOutputMutex);
                        g_McOutputQueue.push(output);
                        g_McOutputReady = true;
                        g_McOutputCV.notify_one();
                        isCommand = false;
                        continue;
                    }
                    std::cout.flush();
                }
            });
            outThread.detach();
            return false;
        }
    }
    #else
    INFO("暂时不支持Linux系统下的启动服务器功能");
    return false;
    #endif
    return false;

}

bool StopServer() {
    #ifdef WIN32
    	const char* command = "stop\n";
    	DWORD written;
    	if (!WriteFile(g_hChildStd_IN_Wr, command, strlen(command), &written, NULL)) {
    		WARN("向服务器发送stop命令失败,原因可能是未使用startserver启动服务器");
            return false;
    	} else {
    		INFO("已向服务器发送stop命令");
    	}

    	// 等待子进程结束
    	WaitForSingleObject(pi.hProcess, INFINITE);

    	CloseHandle(pi.hProcess);
    	CloseHandle(pi.hThread);
    	CloseHandle(g_hChildStd_IN_Wr);
    	CloseHandle(g_hChildStd_IN_Rd);
    	CloseHandle(g_hChildStd_OUT_Wr);
    	CloseHandle(g_hChildStd_OUT_Rd);
    	//检测bedrock_server.exe是否关闭
    	if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
    		WARN("服务器未成功关闭...");
            return false;
    	} else {
    		INFO("服务器已成功关闭...");
            return true;
    	}
    #else
    	INFO("暂时不支持Linux系统下的关闭服务器功能");
        return false;
    #endif
    return false;
}

void BackupServer() {
    std::thread([]() {
        while (true) {
            std::time_t current_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm current_tm = *std::localtime(&current_time_t);

            current_tm.tm_hour = BackupHour;
            current_tm.tm_min = BackupMinute%60;
            current_tm.tm_sec = BackupSecond%60;

            std::time_t target_time_t = std::mktime(&current_tm);

            if (target_time_t <= current_time_t) {
                current_tm.tm_mday += 1;
                target_time_t = std::mktime(&current_tm);
            }

            double seconds_to_wait = difftime(target_time_t, current_time_t);

            std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(seconds_to_wait)));

			std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			StopServer();

			std::this_thread::sleep_for(std::chrono::seconds(5));

			std::string From = BackupFrom;
			std::string To = BackupTo;

			// 获取当前时间作为文件名
			std::tm *time_info = std::localtime(&now);
			char buffer[80];
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", time_info);
			std::string backup_filename = std::string(buffer) + ".zip"; // 使用当前时间作为文件名

			// 创建目标路径
			std::string backup_file_path = To + "/" + backup_filename;

			// 使用系统命令压缩指定目录
			std::string command = "zip -r \"" + backup_file_path + "\" \"" + From + "\"";
			std::cout << "Running command: " << command << std::endl;

			// 执行压缩命令
			int result = std::system(command.c_str());
			if (result == 0) {
				std::cout << "Backup successful: " << backup_file_path << std::endl;
                return true;
			} else {
                std::cout << "Backup failed!" << std::endl;
                return false;
			}
        }
    }).detach();
}

std::string runCommand(const std::string& input_command) {
    #ifdef WIN32
    if (input_command.empty()) {
        WARN("命令不能为空");
        return "命令不能为空";
    }
    std::string command = input_command;
    if (command == "stop") {
        StopServer();
        //检查bedrock_server.exe是否关闭
        if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
            return "服务器未成功关闭...";
        } else {
            return "服务器已成功关闭...";
        }
    }
    if (command == "start") {
        StartServer();
        //检查bedrock_server.exe是否启动
        if (std::system("tasklist | findstr bedrock_server.exe") == 0) {
            return "服务器已成功启动...";
        } else {
            return "服务器启动失败...";
        }
    }
    command += "\n";
    DWORD written;
    if (!WriteFile(g_hChildStd_IN_Wr, command.c_str(), command.size(), &written, NULL)) {
        WARN("向服务器发送命令失败,原因可能是未使用startserver启动服务器");
        return "向服务器发送命令失败,原因可能是未使用startserver启动服务器";
    } else {
        //删去命令中的\n并赋值给std_command
        std::string std_command = command;
        std_command.erase(std::remove(std_command.begin(), std_command.end(), '\n'), std_command.end());
        INFO("已向服务器发送命令: " + std_command);
        isCommand = true;
        std::unique_lock<std::mutex> lock(g_McOutputMutex);
        if (g_McOutputCV.wait_for(lock, std::chrono::seconds(3), [] { return g_McOutputReady; })) {
            while (!g_McOutputQueue.empty()) {
                std::string line = g_McOutputQueue.front();
                g_McOutputQueue.pop();
                line = std::regex_replace(line, std::regex(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}:\d{3} (INFO|ERROR|WARN)\] )"), "");
                line.erase(0, line.find_first_not_of(" "));
                g_McOutputReady = false;
                g_McOutputQueue = std::queue<std::string>();
                return line;
            }
            g_McOutputReady = false;
        } else {
            WARN("服务器3s内无任何返回，命令返回捕捉超时");
            return "服务器3s内无任何返回，命令返回捕捉超时";
        }
    }
    #else
    WARN("暂时不支持Linux系统下的执行mc指令功能");
    return "暂时不支持Linux系统下的执行mc指令功能";
    #endif
    return "";
}

bool AddPlayerToWhitelist(const std::string& player_name) {
    std::string command = "whitelist add " + player_name;
    std::string result = runCommand(command);
    if (result.find("Player added to allowlist") != std::string::npos || result.find("Player already in allowlist") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

bool RemovePlayerFromWhitelist(const std::string& player_name) {
    std::string command = "whitelist remove " + player_name;
    std::string result = runCommand(command);
    if (result.find("Player removed from allowlist") != std::string::npos) {
        return true;
    } else {
        return false;
    }
}
