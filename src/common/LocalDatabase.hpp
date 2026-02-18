#ifndef LOCAL_DATABASE_HPP
#define LOCAL_DATABASE_HPP

#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace leveldb {
class DB;
}

class LocalDatabase {
public:
	struct Result {
		bool ok = false;
		std::string message;
	};

	LocalDatabase() = default;
	~LocalDatabase();
	LocalDatabase(const LocalDatabase&) = delete;
	LocalDatabase& operator=(const LocalDatabase&) = delete;

	Result Initialize(const std::filesystem::path& dbFilePath);
	Result CreateCollection(const std::string& collection);
	Result DropCollection(const std::string& collection);
	Result ClearCollection(const std::string& collection);
	Result Insert(const std::string& collection, const std::string& docId, const std::string& documentJson);
	Result Update(const std::string& collection, const std::string& docId, const std::string& documentJson);
	Result Upsert(const std::string& collection, const std::string& docId, const std::string& documentJson);
	Result Remove(const std::string& collection, const std::string& docId);

	bool HasCollection(const std::string& collection) const;
	bool HasDocument(const std::string& collection, const std::string& docId) const;
	bool Get(const std::string& collection, const std::string& docId, std::string& documentJson) const;
	std::vector<std::string> ListCollections() const;
	std::vector<std::pair<std::string, std::string>> ListDocuments(const std::string& collection, std::size_t offset, std::size_t limit, bool& collectionExists) const;
	std::size_t Count(const std::string& collection, bool& collectionExists) const;
	Result Compact();

private:
	std::string CollectionKey(const std::string& collection) const;
	std::string DocumentKey(const std::string& collection, const std::string& docId) const;
	std::string DocumentPrefix(const std::string& collection) const;
	std::string EncodeKeyPart(const std::string& raw) const;
	std::string DecodeKeyPart(const std::string& encoded) const;
	bool IsCollectionExists(const std::string& collection) const;
	bool ReadDocument(const std::string& collection, const std::string& docId, std::string& documentJson) const;
	Result EnsureInitialized() const;

	std::filesystem::path dbFilePath_;
	mutable std::shared_mutex mutex_;
	std::unique_ptr<leveldb::DB> db_;
	bool initialized_ = false;
};

LocalDatabase& GetLocalDatabase();

#endif
