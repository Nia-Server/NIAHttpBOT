
# NIAHttpBOT 新 API 调用说明

这份文档面向当前已经收口后的 API 面，重点说明新的文件接口 `/fs/stat`、`/fs/read`、`/fs/write`、`/fs/copy` 应该怎么调用，并补充当前仍然保留的 `/qq/sendmsg`。

如果你是从旧接口迁移过来，建议先看本文的“旧接口到新接口映射”一节，再看每条新接口的详细示例。

## 1. 基本信息

### 1.1 服务地址

- 主服务监听地址：`http://{IPAddress}:{ServerPort}`
- 典型本地开发地址：`http://127.0.0.1:2333`

### 1.2 当前主服务对外接口

| Method | Path | 说明 |
| --- | --- | --- |
| POST | /qq/sendmsg | 主动发送 QQ 消息 |
| POST | /fs/stat | 查询路径状态 |
| POST | /fs/read | 读取文本或 JSON 文件 |
| POST | /fs/write | 创建、覆盖、追加写入文件 |
| POST | /fs/copy | 复制文件或目录 |

### 1.3 这轮新接口的目标

旧的 13 条文件接口已经被收成下面 4 条：

- `/fs/stat`
- `/fs/read`
- `/fs/write`
- `/fs/copy`

也就是说，后续外部调用方不应该再调用这些旧路径：

- `/CheckFile`
- `/CheckDir`
- `/CreateNewFile`
- `/CreateNewJsonFile`
- `/GetFileData`
- `/GetJsonFileData`
- `/OverwriteFile`
- `/OverwriteJsonFile`
- `/WriteLineToFile`
- `/CopyFolder`
- `/CopyFolderOverwrite`
- `/CopyFile`
- `/CopyFileOverwrite`

这些旧路径现在都应当视为已下线，访问会返回 `404`。

## 2. 统一调用约定

### 2.1 `/fs/*` 和 `/qq/sendmsg` 使用 HttpV1 请求体

这几类接口都要求请求体是 JSON，并且至少包含：

```json
{
  "api_version": "1.0"
}
```

如果缺少 `api_version`，或者版本不是 `1.0`，接口会直接返回失败。

### 2.2 HttpV1 成功返回格式

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {}
}
```

`data` 的具体字段会因接口不同而不同。

### 2.3 HttpV1 失败返回格式

```json
{
  "httpbot_version": "1.0",
  "status": "fail",
  "data": {
    "message": "错误信息"
  },
  "failure_code": 101
}
```

### 2.4 常见错误码

| failure_code | 含义 | 常见场景 |
| --- | --- | --- |
| 100 | 请求体格式错误 | 不是合法 JSON，或者根节点不是对象 |
| 101 | 请求体缺少必要的键 | 缺少 `api_version`、`path`、`mode` 等 |
| 102 | 请求体键的值格式错误 | 字段类型不对，或枚举值非法 |
| 200 | API 版本不匹配 | `api_version` 不是 `1.0` |
| 300 | 服务内部错误或语义错误 | 工作目录无效、JSON 文件非法、文件系统异常 |
| 400 | 目标文件/路径不存在 | 读、覆盖写、复制时源路径不存在 |
| 401 | 目标已存在，当前动作不允许覆盖 | `create` 时目标已存在，或 copy 时未开启覆盖 |
| 500 | 未成功连接到 QQ 机器人 | `/qq/sendmsg` 在 QQBot 未初始化时 |
| 501 | 目标不存在或不可达 | 目标群聊或私聊对象不可达 |
| 502 | 消息发送失败 | QQ 机器人返回通用失败 |

### 2.5 路径解析规则

文件接口支持两种路径写法：

1. 绝对路径：直接使用，不依赖 `server_id`
2. 相对路径：会拼接到 `server_id` 对应实例的工作目录下

示例：

- 绝对路径：`D:/Project/Game/NiaServer-Core/config.json`
- 相对路径：`plugins/data/config.json`

如果你传的是相对路径，建议同时传 `server_id`。否则服务会尝试使用默认实例；如果默认实例不存在或工作目录无效，会返回失败。

### 2.6 请求头建议

建议所有 POST 请求都带：

```text
Content-Type: application/json
```

## 3. 新文件接口详细说明

## 3.1 POST /fs/stat

### 3.1.1 作用

查询一个路径当前是否存在、是不是普通文件、是不是目录，并返回归一化后的绝对路径。

这条接口可以替代旧的：

- `/CheckFile`
- `/CheckDir`

### 3.1.2 请求体

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| api_version | string | 是 | 固定为 `1.0` |
| path | string | 是 | 目标路径，支持绝对路径和相对路径 |
| server_id | string | 否 | 实例 ID；当 `path` 为相对路径时建议传 |

### 3.1.3 成功返回

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| data.exists | boolean | 路径是否存在 |
| data.is_file | boolean | 是否是普通文件 |
| data.is_directory | boolean | 是否是目录 |
| data.kind | string | `missing`、`file`、`directory`、`other` |
| data.absolute_path | string | 归一化后的绝对路径 |

### 3.1.4 请求示例

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/text.txt"
}
```

### 3.1.5 返回示例

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {
    "exists": true,
    "is_file": true,
    "is_directory": false,
    "kind": "file",
    "absolute_path": "D:\\Project\\Game\\NiaServer-Core\\api-e2e-consolidated\\text.txt"
  }
}
```

### 3.1.6 curl 示例

```bash
curl -X POST "http://127.0.0.1:2333/fs/stat" \
  -H "Content-Type: application/json" \
  -d "{\"api_version\":\"1.0\",\"server_id\":\"default\",\"path\":\"api-e2e-consolidated/text.txt\"}"
```

### 3.1.7 PowerShell 示例

```powershell
$body = @{
  api_version = '1.0'
  server_id = 'default'
  path = 'api-e2e-consolidated/text.txt'
} | ConvertTo-Json

Invoke-RestMethod -Uri 'http://127.0.0.1:2333/fs/stat' -Method Post -ContentType 'application/json' -Body $body
```

## 3.2 POST /fs/read

### 3.2.1 作用

读取文件内容，支持两种模式：

- `text`：按文本字符串返回
- `json`：解析 JSON 后按对象/数组返回

这条接口可以替代旧的：

- `/GetFileData`
- `/GetJsonFileData`

### 3.2.2 请求体

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| api_version | string | 是 | 固定为 `1.0` |
| path | string | 是 | 目标文件路径 |
| format | string | 否 | `text` 或 `json`，默认 `text` |
| server_id | string | 否 | 实例 ID；相对路径建议传 |

### 3.2.3 成功返回

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| data.format | string | 实际读取格式，`text` 或 `json` |
| data.file_data | string/object/array | 文件内容 |

### 3.2.4 文本读取示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/text.txt"
}
```

返回体：

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {
    "format": "text",
    "file_data": "alpha|beta"
  }
}
```

### 3.2.5 JSON 读取示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/data.json",
  "format": "json"
}
```

返回体：

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {
    "format": "json",
    "file_data": {
      "alpha": 1,
      "beta": "two"
    }
  }
}
```

### 3.2.6 注意事项

- 当 `format` 省略时，默认按 `text` 读取。
- 当 `format=json` 时，目标文件必须是合法 JSON；否则会返回 `failure_code=300`，消息为“目标文件不是有效JSON”。
- 如果目标不存在，会返回 `failure_code=400`。

### 3.2.7 curl 示例

```bash
curl -X POST "http://127.0.0.1:2333/fs/read" \
  -H "Content-Type: application/json" \
  -d "{\"api_version\":\"1.0\",\"server_id\":\"default\",\"path\":\"api-e2e-consolidated/data.json\",\"format\":\"json\"}"
```

## 3.3 POST /fs/write

### 3.3.1 作用

统一处理文件写入动作，支持：

- `create`：创建新文件
- `overwrite`：覆盖已有文件
- `append`：向已有文本文件追加内容

这条接口可以替代旧的：

- `/CreateNewFile`
- `/CreateNewJsonFile`
- `/OverwriteFile`
- `/OverwriteJsonFile`
- `/WriteLineToFile`

### 3.3.2 请求体字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| api_version | string | 是 | 固定为 `1.0` |
| path | string | 是 | 目标文件路径 |
| mode | string | 是 | `create`、`overwrite`、`append` |
| format | string | 否 | `text` 或 `json`，默认 `text` |
| content | string/object/array | 是 | 当 `format=text` 时必须是字符串；当 `format=json` 时必须是对象或数组 |
| server_id | string | 否 | 实例 ID；相对路径建议传 |

### 3.3.3 规则说明

#### 文本写入

- `format` 不传时默认是 `text`
- `create`：目标文件必须不存在
- `overwrite`：目标文件必须已存在
- `append`：目标文件必须已存在

#### JSON 写入

- `format=json` 时，`content` 必须是对象或数组
- JSON 会按 PrettyWriter 格式化写入
- `format=json` 不支持 `append`

### 3.3.4 文本创建示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/text.txt",
  "mode": "create",
  "content": "alpha"
}
```

返回体：

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {}
}
```

### 3.3.5 文本追加示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/text.txt",
  "mode": "append",
  "content": "|beta"
}
```

注意：

- 这里是直接追加内容，不会自动补换行
- 如果你想追加换行，需要自己把 `\n` 写进 `content`

### 3.3.6 JSON 创建示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/data.json",
  "mode": "create",
  "format": "json",
  "content": {
    "alpha": 1,
    "beta": "two"
  }
}
```

### 3.3.7 JSON 覆盖示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "path": "api-e2e-consolidated/data.json",
  "mode": "overwrite",
  "format": "json",
  "content": {
    "gamma": true
  }
}
```

### 3.3.8 常见失败场景

| 场景 | 返回 |
| --- | --- |
| `mode=create` 但目标已存在 | `failure_code=401`，消息“目标文件已经存在无法创建” |
| `mode=overwrite` 或 `append` 但目标不存在 | `failure_code=400`，消息“目标操作文件/路径不存在” |
| `format=json` 且 `mode=append` | `failure_code=102` |
| `format=json` 但 `content` 不是对象或数组 | `failure_code=102` |

### 3.3.9 curl 示例

```bash
curl -X POST "http://127.0.0.1:2333/fs/write" \
  -H "Content-Type: application/json" \
  -d "{\"api_version\":\"1.0\",\"server_id\":\"default\",\"path\":\"api-e2e-consolidated/text.txt\",\"mode\":\"append\",\"content\":\"|beta\"}"
```

## 3.4 POST /fs/copy

### 3.4.1 作用

统一处理文件和目录复制。

这条接口可以替代旧的：

- `/CopyFile`
- `/CopyFileOverwrite`
- `/CopyFolder`
- `/CopyFolderOverwrite`

### 3.4.2 请求体字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| api_version | string | 是 | 固定为 `1.0` |
| from_path | string | 是 | 源路径 |
| to_path | string | 是 | 目标路径 |
| overwrite | boolean | 否 | 是否允许覆盖，默认 `false` |
| server_id | string | 否 | 实例 ID；相对路径建议传 |

### 3.4.3 行为说明

- 如果源路径是目录：使用递归复制
- 如果源路径是文件：使用文件复制
- `overwrite=false` 且目标已存在时，会返回 `failure_code=401`
- 如果源路径不存在，会返回 `failure_code=400`

### 3.4.4 文件复制示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "from_path": "api-e2e-consolidated/text.txt",
  "to_path": "api-e2e-consolidated/text-copy.txt",
  "overwrite": false
}
```

### 3.4.5 目录复制示例

请求体：

```json
{
  "api_version": "1.0",
  "server_id": "default",
  "from_path": "api-e2e-consolidated/dir-src",
  "to_path": "api-e2e-consolidated/dir-copy",
  "overwrite": true
}
```

### 3.4.6 成功返回示例

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {}
}
```

### 3.4.7 PowerShell 示例

```powershell
$body = @{
  api_version = '1.0'
  server_id = 'default'
  from_path = 'api-e2e-consolidated/dir-src'
  to_path = 'api-e2e-consolidated/dir-copy'
  overwrite = $true
} | ConvertTo-Json

Invoke-RestMethod -Uri 'http://127.0.0.1:2333/fs/copy' -Method Post -ContentType 'application/json' -Body $body
```


## 4. 当前仍保留的其他接口

## 4.1 POST /qq/sendmsg

### 4.1.1 说明

向指定 QQ 目标发送消息。这条接口使用 HttpV1 协议，当前已经收成 QQ 能力命名空间下的 `/qq/sendmsg`。

这次路由调整的重点不是只改路径名，而是把请求体抽成了更可扩展的结构：

- `target_type`：目标类型
- `target_id`：目标 ID
- `message`：消息内容
- `auto_escape`：可选发送参数

当前实现支持的 `target_type`：

- `group`
- `private`

其中 `target_type` 省略时默认按 `group` 处理。

### 4.1.2 请求体

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| api_version | string | 是 | 固定为 `1.0` |
| target_type | string | 否 | `group` 或 `private`，默认 `group` |
| target_id | string | 是 | 群号或用户 QQ 号 |
| message | string | 是 | 要发送的消息内容 |
| auto_escape | boolean | 否 | 是否启用 `auto_escape`，默认 `false` |

### 4.1.3 群消息请求示例

```json
{
  "api_version": "1.0",
  "target_type": "group",
  "target_id": "123456789",
  "message": "你好，这是一条测试消息",
  "auto_escape": false
}
```

### 4.1.4 私聊消息请求示例

```json
{
  "api_version": "1.0",
  "target_type": "private",
  "target_id": "123456789",
  "message": "你好，这是一条私聊测试消息"
}
```

### 4.1.5 成功返回示例

```json
{
  "httpbot_version": "1.0",
  "status": "success",
  "data": {
    "message_id": 123456,
    "target_type": "group",
    "target_id": "123456789"
  }
}
```

### 4.1.6 QQBot 未启用时的返回示例

```json
{
  "httpbot_version": "1.0",
  "status": "fail",
  "data": {
    "message": "没有成功链接到qq机器人"
  },
  "failure_code": 500
}
```

### 4.1.7 常见失败场景

| 场景 | 返回 |
| --- | --- |
| `target_type` 不是 `group` 或 `private` | `failure_code=102` |
| `target_id` 缺失或不是字符串 | `failure_code=101/102` |
| 目标不存在或不可达 | `failure_code=501` |
| 机器人链路超时或发送失败 | `failure_code=502` |

### 4.1.8 curl 示例

```bash
curl -X POST "http://127.0.0.1:2333/qq/sendmsg" \
  -H "Content-Type: application/json" \
  -d "{\"api_version\":\"1.0\",\"target_type\":\"group\",\"target_id\":\"123456789\",\"message\":\"你好，这是一条测试消息\"}"
```

### 4.1.9 PowerShell 示例

```powershell
$body = @{
  api_version = '1.0'
  target_type = 'group'
  target_id = '123456789'
  message = '你好，这是一条测试消息'
  auto_escape = $false
} | ConvertTo-Json

Invoke-RestMethod -Uri 'http://127.0.0.1:2333/qq/sendmsg' -Method Post -ContentType 'application/json' -Body $body
```

## 4.2 POST /

### 4.2.1 说明

这条路由是 NapCat/QQ 事件回调入口，不是给通用外部调用方手工发请求的业务接口。

它只有在满足下面条件时才会注册：

- `UseQQBot=true`
- QQBot 成功初始化

如果当前运行配置里 `UseQQBot=false`，那么 `POST /` 会返回 `404`，这是预期行为。

## 5. 旧接口到新接口映射

| 旧接口 | 新接口 | 迁移说明 |
| --- | --- | --- |
| /GetConfig | 无 | 已删除，当前不再提供公开配置读取接口 |
| /SendQQGroupMessage | /qq/sendmsg | 旧的 `group_id` 改为 `target_id`，建议显式传 `target_type=group` |
| /CheckFile | /fs/stat | 看 `exists` + `is_file` |
| /CheckDir | /fs/stat | 看 `exists` + `is_directory` |
| /GetFileData | /fs/read | `format` 省略或传 `text` |
| /GetJsonFileData | /fs/read | `format=json` |
| /CreateNewFile | /fs/write | `mode=create`，默认 `format=text` |
| /CreateNewJsonFile | /fs/write | `mode=create` + `format=json` |
| /OverwriteFile | /fs/write | `mode=overwrite`，默认 `format=text` |
| /OverwriteJsonFile | /fs/write | `mode=overwrite` + `format=json` |
| /WriteLineToFile | /fs/write | `mode=append` |
| /CopyFile | /fs/copy | `from_path` + `to_path` + `overwrite=false` |
| /CopyFileOverwrite | /fs/copy | `from_path` + `to_path` + `overwrite=true` |
| /CopyFolder | /fs/copy | 源路径传目录，`overwrite=false` |
| /CopyFolderOverwrite | /fs/copy | 源路径传目录，`overwrite=true` |

## 6. 推荐调用习惯

### 6.1 新接入方建议只使用这 4 条文件接口

如果你是新调用方，不要再依赖旧的历史路径，直接按下面思路接：

1. 先用 `/fs/stat` 判断路径状态
2. 用 `/fs/read` 读取文本或 JSON
3. 用 `/fs/write` 处理创建、覆盖、追加
4. 用 `/fs/copy` 处理文件和目录复制

### 6.2 相对路径尽量带 `server_id`

虽然当前实现支持不传 `server_id` 时走默认实例，但为了减少隐式依赖，建议所有相对路径请求都显式传 `server_id`。

### 6.3 对失败码做明确处理

最常见的几类失败是：

- 101：漏字段
- 102：字段类型或枚举值写错
- 400：目标不存在
- 401：目标已存在，当前动作不允许覆盖
- 500/501/502：QQ 发送链路问题

调用方最好按这些错误码分支处理，而不是只看 HTTP 状态码。当前这些接口即使业务失败，HTTP 状态通常仍然是 `200`，真正的成功与否要看 JSON 里的 `status` 和 `failure_code`。

## 7. 已验证示例

这几条接口已经做过端到端验证：

- `/qq/sendmsg`
- `/fs/stat`
- `/fs/read`
- `/fs/write`
- `/fs/copy`

并且已经确认下面这些旧接口返回 `404`：

- `/GetConfig`
- `/RunCmd`
- `/ServerStarted`
- `/PlayerJoin`
- `/PlayerLeave`
- `/PlayerChat`
- `/exchange_data`
- `/SendQQGroupMessage`
- 所有旧文件接口

如果你后续还要给外部脚本仓库做迁移，这份文档可以直接当作迁移说明使用。