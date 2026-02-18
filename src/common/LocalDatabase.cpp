#include "LocalDatabase.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>

#include <leveldb/db.h>
#include <leveldb/iterator.h>
#include <leveldb/write_batch.h>

namespace {

constexpr const char* kCollectionPrefix = "c/";
constexpr const char* kDocumentPrefix = "d/";

bool StartsWith(const std::string& value, const std::string& prefix) {
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

} // namespace

LocalDatabase& GetLocalDatabase() {
	static LocalDatabase instance;
	return instance;
}

LocalDatabase::~LocalDatabase() {
	std::unique_lock lock(mutex_);
	db_.reset();
	initialized_ = false;
}

LocalDatabase::Result LocalDatabase::Initialize(const std::filesystem::path& dbFilePath) {
	std::unique_lock lock(mutex_);

	dbFilePath_ = dbFilePath;
	if (!dbFilePath_.parent_path().empty()) {
		std::error_code ec;
		std::filesystem::create_directories(dbFilePath_.parent_path(), ec);
		if (ec) {
			return {false, "创建数据库目录失败"};
		}
	}

	leveldb::Options options;
	options.create_if_missing = true;
	options.error_if_exists = false;
	leveldb::DB* rawDb = nullptr;
	const leveldb::Status status = leveldb::DB::Open(options, dbFilePath_.string(), &rawDb);
	if (!status.ok()) {
		return {false, "打开 LevelDB 失败: " + status.ToString()};
	}

	db_.reset(rawDb);
	initialized_ = true;
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::CreateCollection(const std::string& collection) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (collection.empty()) {
		return {false, "collection 不能为空"};
	}
	if (IsCollectionExists(collection)) {
		return {false, "集合已存在"};
	}

	const leveldb::Status status = db_->Put(leveldb::WriteOptions{}, CollectionKey(collection), "1");
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::DropCollection(const std::string& collection) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}

	leveldb::WriteBatch batch;
	batch.Delete(CollectionKey(collection));

	const std::string prefix = DocumentPrefix(collection);
	std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions{}));
	for (it->Seek(prefix); it->Valid(); it->Next()) {
		const std::string key = it->key().ToString();
		if (!StartsWith(key, prefix)) {
			break;
		}
		batch.Delete(key);
	}
	if (!it->status().ok()) {
		return {false, "遍历 LevelDB 失败: " + it->status().ToString()};
	}

	const leveldb::Status status = db_->Write(leveldb::WriteOptions{}, &batch);
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::ClearCollection(const std::string& collection) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}

	leveldb::WriteBatch batch;
	const std::string prefix = DocumentPrefix(collection);
	std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions{}));
	for (it->Seek(prefix); it->Valid(); it->Next()) {
		const std::string key = it->key().ToString();
		if (!StartsWith(key, prefix)) {
			break;
		}
		batch.Delete(key);
	}
	if (!it->status().ok()) {
		return {false, "遍历 LevelDB 失败: " + it->status().ToString()};
	}

	const leveldb::Status status = db_->Write(leveldb::WriteOptions{}, &batch);
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::Insert(const std::string& collection, const std::string& docId, const std::string& documentJson) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}
	if (docId.empty()) {
		return {false, "doc_id 不能为空"};
	}
	std::string existing;
	if (ReadDocument(collection, docId, existing)) {
		return {false, "文档已存在"};
	}

	const leveldb::Status status = db_->Put(leveldb::WriteOptions{}, DocumentKey(collection, docId), documentJson);
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::Update(const std::string& collection, const std::string& docId, const std::string& documentJson) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}
	std::string existing;
	if (!ReadDocument(collection, docId, existing)) {
		return {false, "文档不存在"};
	}

	const leveldb::Status status = db_->Put(leveldb::WriteOptions{}, DocumentKey(collection, docId), documentJson);
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::Upsert(const std::string& collection, const std::string& docId, const std::string& documentJson) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}
	if (docId.empty()) {
		return {false, "doc_id 不能为空"};
	}

	const leveldb::Status status = db_->Put(leveldb::WriteOptions{}, DocumentKey(collection, docId), documentJson);
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

LocalDatabase::Result LocalDatabase::Remove(const std::string& collection, const std::string& docId) {
	std::unique_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	if (!IsCollectionExists(collection)) {
		return {false, "集合不存在"};
	}
	std::string existing;
	if (!ReadDocument(collection, docId, existing)) {
		return {false, "文档不存在"};
	}

	const leveldb::Status status = db_->Delete(leveldb::WriteOptions{}, DocumentKey(collection, docId));
	if (!status.ok()) {
		return {false, "写入 LevelDB 失败: " + status.ToString()};
	}
	return {true, ""};
}

bool LocalDatabase::HasCollection(const std::string& collection) const {
	std::shared_lock lock(mutex_);
	if (!initialized_) {
		return false;
	}
	return IsCollectionExists(collection);
}

bool LocalDatabase::HasDocument(const std::string& collection, const std::string& docId) const {
	std::shared_lock lock(mutex_);
	if (!initialized_) {
		return false;
	}
	std::string document;
	return ReadDocument(collection, docId, document);
}

bool LocalDatabase::Get(const std::string& collection, const std::string& docId, std::string& documentJson) const {
	std::shared_lock lock(mutex_);
	if (!initialized_) {
		return false;
	}
	return ReadDocument(collection, docId, documentJson);
}

std::vector<std::string> LocalDatabase::ListCollections() const {
	std::shared_lock lock(mutex_);
	std::vector<std::string> names;
	if (!initialized_) {
		return names;
	}

	const std::string prefix = kCollectionPrefix;
	std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions{}));
	for (it->Seek(prefix); it->Valid(); it->Next()) {
		const std::string key = it->key().ToString();
		if (!StartsWith(key, prefix)) {
			break;
		}
		names.push_back(DecodeKeyPart(key.substr(prefix.size())));
	}

	std::sort(names.begin(), names.end());
	return names;
}

std::vector<std::pair<std::string, std::string>> LocalDatabase::ListDocuments(const std::string& collection, std::size_t offset, std::size_t limit, bool& collectionExists) const {
	std::shared_lock lock(mutex_);
	std::vector<std::pair<std::string, std::string>> result;
	collectionExists = false;
	if (!initialized_) {
		return result;
	}
	if (!IsCollectionExists(collection)) {
		return result;
	}
	collectionExists = true;

	if (limit == 0) {
		return result;
	}

	const std::string prefix = DocumentPrefix(collection);
	std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions{}));
	std::size_t index = 0;
	for (it->Seek(prefix); it->Valid(); it->Next()) {
		const std::string key = it->key().ToString();
		if (!StartsWith(key, prefix)) {
			break;
		}
		if (index++ < offset) {
			continue;
		}
		const std::string encodedDocId = key.substr(prefix.size());
		result.emplace_back(DecodeKeyPart(encodedDocId), it->value().ToString());
		if (result.size() >= limit) {
			break;
		}
	}

	return result;
}

std::size_t LocalDatabase::Count(const std::string& collection, bool& collectionExists) const {
	std::shared_lock lock(mutex_);
	collectionExists = false;
	if (!initialized_) {
		return 0;
	}
	if (!IsCollectionExists(collection)) {
		return 0;
	}
	collectionExists = true;

	std::size_t count = 0;
	const std::string prefix = DocumentPrefix(collection);
	std::unique_ptr<leveldb::Iterator> it(db_->NewIterator(leveldb::ReadOptions{}));
	for (it->Seek(prefix); it->Valid(); it->Next()) {
		const std::string key = it->key().ToString();
		if (!StartsWith(key, prefix)) {
			break;
		}
		++count;
	}
	return count;
}

LocalDatabase::Result LocalDatabase::Compact() {
	std::shared_lock lock(mutex_);
	auto initState = EnsureInitialized();
	if (!initState.ok) {
		return initState;
	}
	db_->CompactRange(nullptr, nullptr);
	return {true, ""};
}

std::string LocalDatabase::CollectionKey(const std::string& collection) const {
	return std::string(kCollectionPrefix) + EncodeKeyPart(collection);
}

std::string LocalDatabase::DocumentKey(const std::string& collection, const std::string& docId) const {
	return DocumentPrefix(collection) + EncodeKeyPart(docId);
}

std::string LocalDatabase::DocumentPrefix(const std::string& collection) const {
	return std::string(kDocumentPrefix) + EncodeKeyPart(collection) + "/";
}

std::string LocalDatabase::EncodeKeyPart(const std::string& raw) const {
	std::string encoded;
	encoded.reserve(raw.size() * 3);
	constexpr char hex[] = "0123456789ABCDEF";
	for (unsigned char ch : raw) {
		if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.') {
			encoded.push_back(static_cast<char>(ch));
		} else {
			encoded.push_back('%');
			encoded.push_back(hex[(ch >> 4) & 0x0F]);
			encoded.push_back(hex[ch & 0x0F]);
		}
	}
	return encoded;
}

std::string LocalDatabase::DecodeKeyPart(const std::string& encoded) const {
	std::string decoded;
	decoded.reserve(encoded.size());
	for (std::size_t i = 0; i < encoded.size(); ++i) {
		if (encoded[i] != '%' || i + 2 >= encoded.size()) {
			decoded.push_back(encoded[i]);
			continue;
		}
		unsigned int value = 0;
		const auto begin = encoded.data() + static_cast<std::ptrdiff_t>(i + 1);
		const auto end = begin + 2;
		const auto result = std::from_chars(begin, end, value, 16);
		if (result.ec != std::errc{}) {
			decoded.push_back(encoded[i]);
			continue;
		}
		decoded.push_back(static_cast<char>(value));
		i += 2;
	}
	return decoded;
}

bool LocalDatabase::IsCollectionExists(const std::string& collection) const {
	std::string value;
	const leveldb::Status status = db_->Get(leveldb::ReadOptions{}, CollectionKey(collection), &value);
	return status.ok();
}

bool LocalDatabase::ReadDocument(const std::string& collection, const std::string& docId, std::string& documentJson) const {
	const leveldb::Status status = db_->Get(leveldb::ReadOptions{}, DocumentKey(collection, docId), &documentJson);
	return status.ok();
}

LocalDatabase::Result LocalDatabase::EnsureInitialized() const {
	if (!initialized_ || db_ == nullptr) {
		return {false, "数据库尚未初始化"};
	}
	return {true, ""};
}
