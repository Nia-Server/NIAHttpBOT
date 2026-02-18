#include "File_API.h"

#include <fstream>

#include "BDS_API.h"
#include "HttpV1.hpp"

namespace {

bool ResolveTargetPath(const std::string& rawPath, const std::string& serverId, std::filesystem::path& targetPath, std::string& error) {
	targetPath = std::filesystem::path(rawPath);
	if (targetPath.is_absolute()) {
		return true;
	}

	const std::string workingDirectory = GetServerWorkingDirectory(serverId);
	if (workingDirectory.empty()) {
		error = "目标服务器实例不存在或工作目录无效";
		return false;
	}

	targetPath = std::filesystem::path(workingDirectory) / targetPath;
	return true;
}

rapidjson::Value JsonDataFromDocument(const rapidjson::Document& source, rapidjson::Document::AllocatorType& allocator) {
	rapidjson::Value result;
	result.CopyFrom(source, allocator);
	return result;
}

rapidjson::Document ParseJsonFile(const std::filesystem::path& filePath, bool& ok) {
	rapidjson::Document doc;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		ok = false;
		return doc;
	}
	ok = true;
	rapidjson::IStreamWrapper isw(file);
	doc.ParseStream(isw);
	return doc;
}

void RegisterCheckFile(httplib::Server& svr) {
	svr.Post("/CheckFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		if (!HttpV1::RequireString(request, res, "file_name", fileName)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data = HttpV1::MakeBoolData(allocator, "result", std::filesystem::exists(targetPath) && std::filesystem::is_regular_file(targetPath));
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterCheckDir(httplib::Server& svr) {
	svr.Post("/CheckDir", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string dirName;
		if (!HttpV1::RequireString(request, res, "dir_name", dirName)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(dirName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data = HttpV1::MakeBoolData(allocator, "result", std::filesystem::exists(targetPath) && std::filesystem::is_directory(targetPath));
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterCreateNewFile(httplib::Server& svr) {
	svr.Post("/CreateNewFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		std::string content;
		if (!HttpV1::RequireString(request, res, "file_name", fileName) || !HttpV1::RequireString(request, res, "content", content)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (std::filesystem::exists(targetPath)) {
			HttpV1::RespondFail(res, 401, "目标文件已经存在无法创建");
			return;
		}

		std::ofstream file(targetPath);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << content;

		HttpV1::RespondSuccess(res);
	});
}

void RegisterCreateNewJsonFile(httplib::Server& svr) {
	svr.Post("/CreateNewJsonFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		if (!HttpV1::RequireString(request, res, "file_name", fileName) || !HttpV1::RequireObject(request, res, "content")) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (std::filesystem::exists(targetPath)) {
			HttpV1::RespondFail(res, 401, "目标文件已经存在无法创建");
			return;
		}

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		writer.SetIndent(' ', 4);
		request["content"].Accept(writer);

		std::ofstream file(targetPath);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << buffer.GetString();

		HttpV1::RespondSuccess(res);
	});
}

void RegisterGetFileData(httplib::Server& svr) {
	svr.Post("/GetFileData", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		if (!HttpV1::RequireString(request, res, "file_name", fileName)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		std::ifstream file(targetPath);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		std::string line;
		std::string fileData;
		while (std::getline(file, line)) {
			fileData += line;
			fileData += "\n";
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data = HttpV1::MakeStringData(allocator, "file_data", fileData);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterGetJsonFileData(httplib::Server& svr) {
	svr.Post("/GetJsonFileData", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		if (!HttpV1::RequireString(request, res, "file_name", fileName)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		bool readOk = false;
		rapidjson::Document jsonData = ParseJsonFile(targetPath, readOk);
		if (!readOk) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}
		if (jsonData.HasParseError()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("file_data", JsonDataFromDocument(jsonData, allocator), allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterOverwriteFile(httplib::Server& svr) {
	svr.Post("/OverwriteFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		std::string content;
		if (!HttpV1::RequireString(request, res, "file_name", fileName) || !HttpV1::RequireString(request, res, "content", content)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (!std::filesystem::exists(targetPath)) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		std::ofstream file(targetPath, std::ios::trunc);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << content;

		HttpV1::RespondSuccess(res);
	});
}

void RegisterOverwriteJsonFile(httplib::Server& svr) {
	svr.Post("/OverwriteJsonFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		if (!HttpV1::RequireString(request, res, "file_name", fileName) || !HttpV1::RequireObject(request, res, "content")) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (!std::filesystem::exists(targetPath)) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		writer.SetIndent(' ', 4);
		request["content"].Accept(writer);

		std::ofstream file(targetPath, std::ios::trunc);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << buffer.GetString();

		HttpV1::RespondSuccess(res);
	});
}

void RegisterWriteLineToFile(httplib::Server& svr) {
	svr.Post("/WriteLineToFile", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fileName;
		std::string content;
		if (!HttpV1::RequireString(request, res, "file_name", fileName) || !HttpV1::RequireString(request, res, "content", content)) {
			return;
		}

		std::filesystem::path targetPath;
		std::string error;
		if (!ResolveTargetPath(fileName, HttpV1::GetServerId(request), targetPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (!std::filesystem::exists(targetPath)) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		std::ofstream file(targetPath, std::ios::app);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << content;

		HttpV1::RespondSuccess(res);
	});
}

void RegisterCopyFolder(httplib::Server& svr, bool overwrite) {
	const char* route = overwrite ? "/CopyFolderOverwrite" : "/CopyFolder";
	svr.Post(route, [overwrite](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fromDir;
		std::string toDir;
		if (!HttpV1::RequireString(request, res, "from_dir", fromDir) || !HttpV1::RequireString(request, res, "to_dir", toDir)) {
			return;
		}

		const std::string serverId = HttpV1::GetServerId(request);
		std::filesystem::path fromPath;
		std::filesystem::path toPath;
		std::string error;
		if (!ResolveTargetPath(fromDir, serverId, fromPath, error) || !ResolveTargetPath(toDir, serverId, toPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (!std::filesystem::exists(fromPath)) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		try {
			auto options = std::filesystem::copy_options::recursive;
			if (overwrite) {
				options = options | std::filesystem::copy_options::overwrite_existing;
			}
			std::filesystem::copy(fromPath, toPath, options);
			HttpV1::RespondSuccess(res);
		} catch (const std::filesystem::filesystem_error&) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
		}
	});
}

void RegisterCopyFile(httplib::Server& svr, bool overwrite) {
	const char* route = overwrite ? "/CopyFileOverwrite" : "/CopyFile";
	svr.Post(route, [overwrite](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string fromFile;
		std::string toFile;
		if (!HttpV1::RequireString(request, res, "from_file", fromFile) || !HttpV1::RequireString(request, res, "to_file", toFile)) {
			return;
		}

		const std::string serverId = HttpV1::GetServerId(request);
		std::filesystem::path fromPath;
		std::filesystem::path toPath;
		std::string error;
		if (!ResolveTargetPath(fromFile, serverId, fromPath, error) || !ResolveTargetPath(toFile, serverId, toPath, error)) {
			HttpV1::RespondFail(res, 300, error);
			return;
		}

		if (!std::filesystem::exists(fromPath)) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		try {
			auto options = overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none;
			std::filesystem::copy_file(fromPath, toPath, options);
			HttpV1::RespondSuccess(res);
		} catch (const std::filesystem::filesystem_error&) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
		}
	});
}

}

void init_file_API(httplib::Server &svr) {
	RegisterCheckFile(svr);
	RegisterCheckDir(svr);
	RegisterCreateNewFile(svr);
	RegisterCreateNewJsonFile(svr);
	RegisterGetFileData(svr);
	RegisterGetJsonFileData(svr);
	RegisterOverwriteFile(svr);
	RegisterOverwriteJsonFile(svr);
	RegisterWriteLineToFile(svr);
	RegisterCopyFolder(svr, false);
	RegisterCopyFolder(svr, true);
	RegisterCopyFile(svr, false);
	RegisterCopyFile(svr, true);
}
