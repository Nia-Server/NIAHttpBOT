#include "DB_API.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "HttpV1.hpp"
#include "LocalDatabase.hpp"
#include "Logger.hpp"

namespace {

constexpr std::size_t kDefaultLimit = 100;
constexpr std::size_t kMaxLimit = 1000;

bool RequireCollection(const rapidjson::Document& request, httplib::Response& res, std::string& collection) {
	if (!HttpV1::RequireString(request, res, "collection", collection)) {
		return false;
	}
	if (collection.empty()) {
		HttpV1::RespondFail(res, 102, "请求体键的值格式错误: collection");
		return false;
	}
	return true;
}

bool RequireDocId(const rapidjson::Document& request, httplib::Response& res, std::string& docId) {
	if (!HttpV1::RequireString(request, res, "doc_id", docId)) {
		return false;
	}
	if (docId.empty()) {
		HttpV1::RespondFail(res, 102, "请求体键的值格式错误: doc_id");
		return false;
	}
	return true;
}

bool RequireDocument(const rapidjson::Document& request, httplib::Response& res, std::string& documentJson) {
	if (!request.HasMember("document")) {
		HttpV1::RespondFail(res, 101, "请求体缺少必要的键: document");
		return false;
	}
	if (!request["document"].IsObject() && !request["document"].IsArray()) {
		HttpV1::RespondFail(res, 102, "请求体键的值格式错误: document");
		return false;
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	request["document"].Accept(writer);
	documentJson = buffer.GetString();
	return true;
}

void HandleDbResult(httplib::Response& res, const LocalDatabase::Result& result) {
	if (result.ok) {
		HttpV1::RespondSuccess(res);
		return;
	}
	HttpV1::RespondFail(res, 300, result.message);
}

void RegisterCreateCollection(httplib::Server& svr) {
	svr.Post("/DBCreateCollection", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		if (!RequireCollection(request, res, collection)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().CreateCollection(collection));
	});
}

void RegisterDropCollection(httplib::Server& svr) {
	svr.Post("/DBDropCollection", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		if (!RequireCollection(request, res, collection)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().DropCollection(collection));
	});
}

void RegisterClearCollection(httplib::Server& svr) {
	svr.Post("/DBClearCollection", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		if (!RequireCollection(request, res, collection)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().ClearCollection(collection));
	});
}

void RegisterListCollections(httplib::Server& svr) {
	svr.Post("/DBListCollections", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		auto names = GetLocalDatabase().ListCollections();
		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		rapidjson::Value list(rapidjson::kArrayType);
		for (const auto& name : names) {
			list.PushBack(rapidjson::Value(name.c_str(), allocator), allocator);
		}
		data.AddMember("collections", list, allocator);
		data.AddMember("count", static_cast<int>(names.size()), allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterInsert(httplib::Server& svr) {
	svr.Post("/DBInsert", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		std::string docId;
		std::string documentJson;
		if (!RequireCollection(request, res, collection) || !RequireDocId(request, res, docId) || !RequireDocument(request, res, documentJson)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().Insert(collection, docId, documentJson));
	});
}

void RegisterUpdate(httplib::Server& svr) {
	svr.Post("/DBUpdate", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		std::string docId;
		std::string documentJson;
		if (!RequireCollection(request, res, collection) || !RequireDocId(request, res, docId) || !RequireDocument(request, res, documentJson)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().Update(collection, docId, documentJson));
	});
}

void RegisterUpsert(httplib::Server& svr) {
	svr.Post("/DBUpsert", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		std::string docId;
		std::string documentJson;
		if (!RequireCollection(request, res, collection) || !RequireDocId(request, res, docId) || !RequireDocument(request, res, documentJson)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().Upsert(collection, docId, documentJson));
	});
}

void RegisterDelete(httplib::Server& svr) {
	svr.Post("/DBDelete", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		std::string docId;
		if (!RequireCollection(request, res, collection) || !RequireDocId(request, res, docId)) {
			return;
		}

		HandleDbResult(res, GetLocalDatabase().Remove(collection, docId));
	});
}

void RegisterGet(httplib::Server& svr) {
	svr.Post("/DBGet", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		std::string docId;
		if (!RequireCollection(request, res, collection) || !RequireDocId(request, res, docId)) {
			return;
		}

		std::string documentJson;
		if (!GetLocalDatabase().Get(collection, docId, documentJson)) {
			HttpV1::RespondFail(res, 400, "目标操作文档不存在");
			return;
		}

		rapidjson::Document doc;
		doc.Parse(documentJson.c_str());
		if (doc.HasParseError()) {
			HttpV1::RespondFail(res, 300, "NIAHttpBOT自身错误");
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("doc_id", rapidjson::Value(docId.c_str(), allocator), allocator);
		rapidjson::Value documentValue;
		documentValue.CopyFrom(doc, allocator);
		data.AddMember("document", documentValue, allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterList(httplib::Server& svr) {
	svr.Post("/DBList", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		if (!RequireCollection(request, res, collection)) {
			return;
		}

		std::size_t offset = 0;
		std::size_t limit = kDefaultLimit;
		if (request.HasMember("offset")) {
			if (!request["offset"].IsUint64()) {
				HttpV1::RespondFail(res, 102, "请求体键的值格式错误: offset");
				return;
			}
			offset = static_cast<std::size_t>(request["offset"].GetUint64());
		}
		if (request.HasMember("limit")) {
			if (!request["limit"].IsUint64()) {
				HttpV1::RespondFail(res, 102, "请求体键的值格式错误: limit");
				return;
			}
			limit = static_cast<std::size_t>(request["limit"].GetUint64());
			if (limit == 0) {
				limit = kDefaultLimit;
			}
			limit = std::min(limit, kMaxLimit);
		}

		bool collectionExists = false;
		auto docs = GetLocalDatabase().ListDocuments(collection, offset, limit, collectionExists);
		if (!collectionExists) {
			HttpV1::RespondFail(res, 400, "目标操作集合不存在");
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		rapidjson::Value list(rapidjson::kArrayType);

		for (const auto& [docId, documentJson] : docs) {
			rapidjson::Document doc;
			doc.Parse(documentJson.c_str());
			if (doc.HasParseError()) {
				continue;
			}
			rapidjson::Value item(rapidjson::kObjectType);
			item.AddMember("doc_id", rapidjson::Value(docId.c_str(), allocator), allocator);
			rapidjson::Value documentValue;
			documentValue.CopyFrom(doc, allocator);
			item.AddMember("document", documentValue, allocator);
			list.PushBack(item, allocator);
		}

		bool countExists = false;
		const auto total = GetLocalDatabase().Count(collection, countExists);

		data.AddMember("collection", rapidjson::Value(collection.c_str(), allocator), allocator);
		data.AddMember("offset", static_cast<uint64_t>(offset), allocator);
		data.AddMember("limit", static_cast<uint64_t>(limit), allocator);
		data.AddMember("total", static_cast<uint64_t>(total), allocator);
		data.AddMember("items", list, allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterCount(httplib::Server& svr) {
	svr.Post("/DBCount", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}

		std::string collection;
		if (!RequireCollection(request, res, collection)) {
			return;
		}

		bool collectionExists = false;
		const auto count = GetLocalDatabase().Count(collection, collectionExists);
		if (!collectionExists) {
			HttpV1::RespondFail(res, 400, "目标操作集合不存在");
			return;
		}

		rapidjson::Document dataDoc;
		auto& allocator = dataDoc.GetAllocator();
		rapidjson::Value data(rapidjson::kObjectType);
		data.AddMember("collection", rapidjson::Value(collection.c_str(), allocator), allocator);
		data.AddMember("count", static_cast<uint64_t>(count), allocator);
		HttpV1::RespondSuccess(res, &data);
	});
}

void RegisterCompact(httplib::Server& svr) {
	svr.Post("/DBCompact", [](const httplib::Request& req, httplib::Response& res) {
		rapidjson::Document request;
		if (!HttpV1::ParseRequestV1(req, res, request)) {
			return;
		}
		HandleDbResult(res, GetLocalDatabase().Compact());
	});
}

} // namespace

void init_db_API(httplib::Server& svr) {
	RegisterCreateCollection(svr);
	RegisterDropCollection(svr);
	RegisterClearCollection(svr);
	RegisterListCollections(svr);
	RegisterInsert(svr);
	RegisterUpdate(svr);
	RegisterUpsert(svr);
	RegisterDelete(svr);
	RegisterGet(svr);
	RegisterList(svr);
	RegisterCount(svr);
	RegisterCompact(svr);
	INFO("数据库 API 已加载");
}
