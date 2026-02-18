# NIAHttpBOT 本地数据库 API 文档（中文）

## 1. 介绍

NIAHttpBOT 内置了一个基于 **LevelDB** 的高效轻量本地数据库模块，特点如下：

- 使用成熟数据库引擎：基于 Google LevelDB（LSM Tree）。
- 本地持久化：数据直接落盘到 `./data/leveldb` 目录。
- 有序键值与高效读写：适合中小规模高频读写场景。
- 无需独立数据库服务进程：开箱即用。

> 所有接口都使用 `POST`，并使用统一的 HttpV1 请求格式。

---

## 2. 通用请求格式

```json
{
  "api_version": "1.0",
  "server_id": "default"
}
```

- `api_version`：必填，固定 `1.0`。
- `server_id`：可选。数据库接口不依赖实例，但为兼容通用请求规范可保留。

---

## 3. 通用返回格式

### 成功

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {}
}
```

### 失败

```json
{
  "httpbot_version": "1.0",
  "status": "fail",
  "data": {
    "message": "错误说明"
  },
  "failure_code": 300
}
```

---

## 4. 集合管理接口

### 4.1 创建集合
- 路径：`/DBCreateCollection`
- 字段：`collection` (string, 必填)

### 4.2 删除集合
- 路径：`/DBDropCollection`
- 字段：`collection` (string, 必填)

### 4.3 清空集合
- 路径：`/DBClearCollection`
- 字段：`collection` (string, 必填)

### 4.4 列出全部集合
- 路径：`/DBListCollections`
- 字段：无额外字段
- 返回：`collections`（数组）与 `count`（数量）

---

## 5. 文档 CRUD 接口

> 文档以 `doc_id` 作为主键，`document` 为 JSON 对象或 JSON 数组。

### 5.1 插入文档（必须不存在）
- 路径：`/DBInsert`
- 字段：
  - `collection` (string)
  - `doc_id` (string)
  - `document` (object/array)

### 5.2 更新文档（必须存在）
- 路径：`/DBUpdate`
- 字段同 `/DBInsert`

### 5.3 插入或更新
- 路径：`/DBUpsert`
- 字段同 `/DBInsert`

### 5.4 删除文档
- 路径：`/DBDelete`
- 字段：
  - `collection` (string)
  - `doc_id` (string)

### 5.5 读取单条文档
- 路径：`/DBGet`
- 字段：
  - `collection` (string)
  - `doc_id` (string)
- 返回：
  - `doc_id`
  - `document`

### 5.6 分页读取文档列表
- 路径：`/DBList`
- 字段：
  - `collection` (string)
  - `offset` (uint64, 可选，默认 0)
  - `limit` (uint64, 可选，默认 100，最大 1000)
- 返回：
  - `collection`
  - `offset`
  - `limit`
  - `total`
  - `items`（每项包含 `doc_id` 与 `document`）

### 5.7 统计集合文档数
- 路径：`/DBCount`
- 字段：`collection` (string)
- 返回：`count`

---

## 6. 维护接口

### 6.1 压缩数据库
- 路径：`/DBCompact`
- 说明：触发 LevelDB 全量压缩（compaction），优化磁盘结构与读取性能。

---

## 7. 请求示例

## 7.1 创建集合

```json
{
  "api_version": "1.0",
  "collection": "players"
}
```

## 7.2 插入文档

```json
{
  "api_version": "1.0",
  "collection": "players",
  "doc_id": "Steve",
  "document": {
    "level": 25,
    "coins": 1280,
    "online": true
  }
}
```

## 7.3 分页查询

```json
{
  "api_version": "1.0",
  "collection": "players",
  "offset": 0,
  "limit": 20
}
```

## 7.4 压缩数据库

```json
{
  "api_version": "1.0"
}
```

---

## 8. 错误码建议理解

- `100/101/102`：请求体格式问题（JSON 解析、缺少键、类型错误）。
- `200`：API 版本不匹配。
- `300`：服务内部错误或数据库状态错误（如集合已存在、LevelDB 写入失败等）。
- `400`：目标资源不存在（如集合/文档不存在）。

---

## 9. 数据文件说明

默认数据库目录路径：

- `./data/leveldb`

建议：

- 将 `data` 目录纳入备份计划。
- 在批量导入后调用一次 `/DBCompact` 以触发压缩优化。
