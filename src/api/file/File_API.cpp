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

bool GetOptionalString(const rapidjson::Document& doc, httplib::Response& res, const char* key, std::string& value) {
	if (!doc.HasMember(key)) {
		return true;
	}
	if (!doc[key].IsString()) {
		HttpV1::RespondFail(res, 102, std::string("请求体键的值格式错误: ") + key);
		return false;
	}
	value = doc[key].GetString();
	return true;
}

bool GetOptionalBool(const rapidjson::Document& doc, httplib::Response& res, const char* key, bool& value) {
	if (!doc.HasMember(key)) {
		return true;
	}
	if (!doc[key].IsBool()) {
		HttpV1::RespondFail(res, 102, std::string("请求体键的值格式错误: ") + key);
		return false;
	}
	value = doc[key].GetBool();
	return true;
}

bool RequireResolvedPath(const rapidjson::Document& request, httplib::Response& res, const char* key, std::filesystem::path& targetPath) {
	std::string rawPath;
	if (!HttpV1::RequireString(request, res, key, rawPath)) {
		return false;
	}

	std::string error;
	if (!ResolveTargetPath(rawPath, HttpV1::GetServerId(request), targetPath, error)) {
		HttpV1::RespondFail(res, 300, error);
		return false;
	}
	return true;
}

bool IsSupportedValue(const std::string& value, std::initializer_list<const char*> supported) {
	for (const char* item : supported) {
		if (value == item) {
			return true;
		}
	}
	return false;
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

std::string ReadTextFile(const std::filesystem::path& filePath, bool& ok) {
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		ok = false;
		return {};
	}
	ok = true;
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string DetectPathKind(const std::filesystem::path& targetPath) {
	std::error_code ec;
	if (!std::filesystem::exists(targetPath, ec)) {
		return "missing";
	}
	if (std::filesystem::is_regular_file(targetPath, ec)) {
		return "file";
	}
	if (std::filesystem::is_directory(targetPath, ec)) {
		return "directory";
	}
	return "other";
}

void RegisterFsStat(httplib::Server& svr) {
	svr.Post("/fs/stat", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::filesystem::path targetPath;
		if (!RequireResolvedPath(request, res, "path", targetPath)) {
			return;
		}

		std::error_code ec;
		const bool exists = std::filesystem::exists(targetPath, ec);
		if (ec) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		const bool isFile = exists && std::filesystem::is_regular_file(targetPath, ec);
		if (ec) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		const bool isDirectory = exists && std::filesystem::is_directory(targetPath, ec);
		if (ec) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		const std::string kind = DetectPathKind(targetPath);
		const std::string absolutePath = targetPath.lexically_normal().string();

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("exists", exists, allocator);
		data.AddMember("is_file", isFile, allocator);
		data.AddMember("is_directory", isDirectory, allocator);
		data.AddMember("kind", rapidjson::Value(kind.c_str(), allocator), allocator);
		data.AddMember("absolute_path", rapidjson::Value(absolutePath.c_str(), allocator), allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterFsRead(httplib::Server& svr) {
	svr.Post("/fs/read", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::filesystem::path targetPath;
		if (!RequireResolvedPath(request, res, "path", targetPath)) {
			return;
		}

		std::string format = "text";
		if (!GetOptionalString(request, res, "format", format)) {
			return;
		}
		if (!IsSupportedValue(format, {"text", "json"})) {
			HttpV1::RespondFail(res, 102, "请求体键的值格式错误: format");
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("format", rapidjson::Value(format.c_str(), allocator), allocator);

		if (format == "json") {
			bool readOk = false;
			rapidjson::Document jsonData = ParseJsonFile(targetPath, readOk);
			if (!readOk) {
				HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
				return;
			}
			if (jsonData.HasParseError()) {
				HttpV1::RespondFail(res, 300, "目标文件不是有效JSON");
				return;
			}
			data.AddMember("file_data", JsonDataFromDocument(jsonData, allocator), allocator);
			HttpV1::RespondSuccess(res, &data);
			return;
		}

		bool readOk = false;
		std::string fileData = ReadTextFile(targetPath, readOk);
		if (!readOk) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		data.AddMember("file_data", rapidjson::Value(fileData.c_str(), allocator), allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterFsWrite(httplib::Server& svr) {
	svr.Post("/fs/write", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::filesystem::path targetPath;
		if (!RequireResolvedPath(request, res, "path", targetPath)) {
			return;
		}

		std::string mode;
		if (!HttpV1::RequireString(request, res, "mode", mode)) {
			return;
		}
		if (!IsSupportedValue(mode, {"create", "overwrite", "append"})) {
			HttpV1::RespondFail(res, 102, "请求体键的值格式错误: mode");
			return;
		}

		std::string format = "text";
		if (!GetOptionalString(request, res, "format", format)) {
			return;
		}
		if (!IsSupportedValue(format, {"text", "json"})) {
			HttpV1::RespondFail(res, 102, "请求体键的值格式错误: format");
			return;
		}
		if (format == "json" && mode == "append") {
			HttpV1::RespondFail(res, 102, "请求体键的值格式错误: mode");
			return;
		}

		const bool exists = std::filesystem::exists(targetPath);
		if (mode == "create" && exists) {
			HttpV1::RespondFail(res, 401, "目标文件已经存在无法创建");
			return;
		}
		if (mode != "create" && !exists) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		if (format == "json") {
			if (!HttpV1::RequireObject(request, res, "content")) {
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
			return;
		}

		std::string content;
		if (!HttpV1::RequireString(request, res, "content", content)) {
			return;
		}

		std::ios::openmode openMode = std::ios::out;
		if (mode == "append") {
			openMode |= std::ios::app;
		} else {
			openMode |= std::ios::trunc;
		}

		std::ofstream file(targetPath, openMode);
		if (!file.is_open()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}
		file << content;
		HttpV1::RespondSuccess(res);
	});
}

void RegisterFsCopy(httplib::Server& svr) {
	svr.Post("/fs/copy", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::filesystem::path fromPath;
		std::filesystem::path toPath;
		if (!RequireResolvedPath(request, res, "from_path", fromPath) || !RequireResolvedPath(request, res, "to_path", toPath)) {
			return;
		}

		bool overwrite = false;
		if (!GetOptionalBool(request, res, "overwrite", overwrite)) {
			return;
		}

		std::error_code ec;
		if (!std::filesystem::exists(fromPath, ec) || ec) {
			HttpV1::RespondFail(res, 400, "目标操作文件/路径不存在");
			return;
		}

		if (std::filesystem::is_directory(fromPath, ec)) {
			if (ec) {
				HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
				return;
			}
			try {
				auto options = std::filesystem::copy_options::recursive;
				if (overwrite) {
					options |= std::filesystem::copy_options::overwrite_existing;
				}
				std::filesystem::copy(fromPath, toPath, options);
				HttpV1::RespondSuccess(res);
			} catch (const std::filesystem::filesystem_error&) {
				HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			}
			return;
		}

		if (ec) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		if (!std::filesystem::is_regular_file(fromPath, ec) || ec) {
			HttpV1::RespondFail(res, 400, "仅支持复制文件或目录");
			return;
		}

		if (!overwrite && std::filesystem::exists(toPath)) {
			HttpV1::RespondFail(res, 401, "目标文件已经存在无法复制");
			return;
		}

		const auto options = overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::none;
		if (!std::filesystem::copy_file(fromPath, toPath, options, ec) || ec) {
			if (!overwrite && std::filesystem::exists(toPath)) {
				HttpV1::RespondFail(res, 401, "目标文件已经存在无法复制");
				return;
			}
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		HttpV1::RespondSuccess(res);
	});
}

}

void init_file_API(httplib::Server &svr) {
	RegisterFsStat(svr);
	RegisterFsRead(svr);
	RegisterFsWrite(svr);
	RegisterFsCopy(svr);
}
