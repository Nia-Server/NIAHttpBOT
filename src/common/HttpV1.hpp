#ifndef HTTP_V1_HPP
#define HTTP_V1_HPP

#include <string>

#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace HttpV1 {

inline constexpr const char* kApiVersion = "1.0";
inline constexpr const char* kHttpBotVersion = "1.0";

inline void SetJsonContent(httplib::Response& res, const std::string& body) {
	res.status = 200;
	res.set_content(body, "application/json; charset=utf-8");
}

inline std::string BuildFailBody(int failureCode, const std::string& message) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	writer.StartObject();
	writer.Key("httpbot_version");
	writer.String(kHttpBotVersion);
	writer.Key("status");
	writer.String("fail");
	writer.Key("data");
	writer.StartObject();
	writer.Key("message");
	writer.String(message.c_str());
	writer.EndObject();
	writer.Key("failure_code");
	writer.Int(failureCode);
	writer.EndObject();
	return buffer.GetString();
}

inline void RespondFail(httplib::Response& res, int failureCode, const std::string& message) {
	SetJsonContent(res, BuildFailBody(failureCode, message));
}

inline std::string BuildSuccessBody(const rapidjson::Value* data = nullptr) {
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("httpbot_version", rapidjson::Value(kHttpBotVersion, allocator), allocator);
	doc.AddMember("status", rapidjson::Value("success", allocator), allocator);

	rapidjson::Value dataValue;
	if (data != nullptr) {
		dataValue.CopyFrom(*data, allocator);
	} else {
		dataValue.SetObject();
	}
	doc.AddMember("data", dataValue, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	return buffer.GetString();
}

inline void RespondSuccess(httplib::Response& res, const rapidjson::Value* data = nullptr) {
	SetJsonContent(res, BuildSuccessBody(data));
}

inline bool ParseRequestV1(const httplib::Request& req, httplib::Response& res, rapidjson::Document& doc) {
	doc.Parse(req.body.c_str());
	if (doc.HasParseError() || !doc.IsObject()) {
		RespondFail(res, 100, "请求体格式错误");
		return false;
	}

	if (!doc.HasMember("api_version")) {
		RespondFail(res, 101, "请求体缺少必要的键: api_version");
		return false;
	}
	if (!doc["api_version"].IsString()) {
		RespondFail(res, 102, "请求体键的值格式错误: api_version");
		return false;
	}
	if (std::string(doc["api_version"].GetString()) != kApiVersion) {
		RespondFail(res, 200, "API版本不匹配");
		return false;
	}

	if (doc.HasMember("server_id") && !doc["server_id"].IsString()) {
		RespondFail(res, 102, "请求体键的值格式错误: server_id");
		return false;
	}

	return true;
}

inline bool RequireString(const rapidjson::Document& doc, httplib::Response& res, const char* key, std::string& value) {
	if (!doc.HasMember(key)) {
		RespondFail(res, 101, std::string("请求体缺少必要的键: ") + key);
		return false;
	}
	if (!doc[key].IsString()) {
		RespondFail(res, 102, std::string("请求体键的值格式错误: ") + key);
		return false;
	}
	value = doc[key].GetString();
	return true;
}

inline bool RequireObject(const rapidjson::Document& doc, httplib::Response& res, const char* key) {
	if (!doc.HasMember(key)) {
		RespondFail(res, 101, std::string("请求体缺少必要的键: ") + key);
		return false;
	}
	if (!doc[key].IsObject() && !doc[key].IsArray()) {
		RespondFail(res, 102, std::string("请求体键的值格式错误: ") + key);
		return false;
	}
	return true;
}

inline std::string GetServerId(const rapidjson::Document& doc) {
	if (!doc.HasMember("server_id") || !doc["server_id"].IsString()) {
		return "";
	}
	return doc["server_id"].GetString();
}

inline rapidjson::Value MakeBoolData(rapidjson::Document::AllocatorType& allocator, const char* key, bool value) {
	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember(rapidjson::Value(key, allocator), rapidjson::Value(value), allocator);
	return data;
}

inline rapidjson::Value MakeStringData(rapidjson::Document::AllocatorType& allocator, const char* key, const std::string& value) {
	rapidjson::Value data(rapidjson::kObjectType);
	data.AddMember(rapidjson::Value(key, allocator), rapidjson::Value(value.c_str(), allocator), allocator);
	return data;
}

}

#endif
