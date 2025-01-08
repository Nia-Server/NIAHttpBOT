# NIAHttpBOT


[![status](https://img.shields.io/github/actions/workflow/status/Nia-Server/NIAHttpBOT/pre-build.yml?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/actions/workflows/pre-build.yml)
[![GitHub Release Date](https://img.shields.io/github/release-date-pre/Nia-Server/NIAHttpBOT?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/releases)
[![Latest Release](https://img.shields.io/github/v/release/Nia-Server/NIAHttpBOT?include_prereleases&style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/releases/latest)
[![GitHub last commit](https://img.shields.io/github/last-commit/Nia-Server/NIAHttpBOT?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/commits)

![NiaServer-Core](https://socialify.git.ci/Nia-Server/NIAHttpBOT/image?description=1&descriptionEditable=NIA%E6%9C%8D%E5%8A%A1%E5%99%A8HttpBOT&font=Source%20Code%20Pro&forks=1&issues=1&logo=https://docs.mcnia.com/logo.png&name=1&pattern=Floating%20Cogs&pulls=1&stargazers=1&theme=Auto)


## 简介

使用C++开发的BOT，基于http实现更多可能，本项目为[NiaServer-Core](https://github.com/Nia-Server/NiaServer-Core)子项目


## 为什么开发？

由于目前我的世界的Script-api无法实现诸如文件读写等功能,为此我们特此基于C++开发了`NIA-Http-Bot`用来实现更多功能，从而赋予Script-api更多可能

***

## 功能特性

- 基于http可以实现对特定文件进行读写、创建以及删除等功能
- 基于http搭配**NiaServer-Core**以及**NapCatQQ**可以实现QQ机器人与服务器联动功能

***

## 使用前注意事项

1.本项目基于**http**进行通讯，故当前Minecraft版本应当注意启用**minecraft/server-net**模块（该模块只能运行在服务器上）

2.您可以前往**NiaServer-Core**项目地址的[release](https://github.com/Nia-Server/NIAHttpBOT/releases)下载最新release构建的**NIAHttpBOT.exe**来获取最新版的`NIA-Http-Bot`

3.如果您在使用期间遇到了问题/有建议，您可以前往**NiaServer-Core**的[issues](https://github.com/Nia-Server/NiaServer-Core/issues)进行反馈！

4.由于**采用的是http通讯，而非https**，我们**非常不推荐**您将NIAHttpBOT与基岩版服务端分开放置于两台服务器上，这是非常不安全的操作！请务必**将NiaHttpBOT与基岩版服务端放置于同一台服务器之上**，并**注意防火墙设置**，不要开放使用过程中涉及的两个端口，以免对服务器安全造成威胁！

***

## 使用教程

### Windows平台

> [!tip]
> 不使用QQ机器人的用户可以跳过第5-9步

1. 前往我的世界官网[下载BDS](https://www.minecraft.net/en-us/download/server/bedrock)，并将下好的服务端解压

2. 安装好对应的行为包

3. 修改服务器端文件，来启用net模块：将`config/default/permissions.json`内容改为

```json
{
    "allowed_modules": [
        "@minecraft/server-gametest",
        "@minecraft/server",
        "@minecraft/server-ui",
        "@minecraft/server-admin",
        "@minecraft/server-editor",
        "@minecraft/server-net"
    ]
}

```

4. Windows平台下下载最新release构建的**NIAHttpBOT.exe**来获取最新版的`NIAHttp-Bot`

5. 根据[NapCatQQ安装教程](https://napneko.icu/guide/start-install)安装相应的机器人框架

6. 安装后，打开机器人**WEBUI**界面，**点击侧边栏网络配置**，点击**添加配置**

7. 名称首先写上**服务器**，类型选择**http服务器**，启用，并将端口与NIAHttpBOT的配置文件中的**ClientPort**保持一致；

8. 然后再次点击**添加配置**，名称写上客户端，类型选择**http客户端**，启用，并将URL一栏与NIAHttpBOT中配置文件的`http://127.0.0.1:<ServerPort>/<Locate>`保持一致，如果是原始配置文件没有改动，则为`http://127.0.0.1:10086/qqEvent`，机器人至此配置完毕。

9. 按照[NapCatQQ安装教程](https://napneko.icu/guide/start-install)中指示启动机器人

10. 双击**NIAHttpBOT.exe**来启动，第一次启动时会生成配置文件，配置文件路径为`./NIAHttpBOT.cfg`，您可以根据自己的需求进行修改，具体修改教程见[配置文件](#配置文件)，修改后请输入`reload`来重新加载配置文件。

11. 最后启动MCBDS服务端即可！


### Linux平台

> [!tip]
> 不使用QQ机器人的用户可以跳过第5-9步

1. 前往我的世界官网[下载BDS](https://www.minecraft.net/en-us/download/server/bedrock)，并将下好的服务端解压

2. 安装好对应的行为包

3. 修改服务器端文件，来启用net模块：将`config/default/permissions.json`内容改为

```json
{
    "allowed_modules": [
        "@minecraft/server-gametest",
        "@minecraft/server",
        "@minecraft/server-ui",
        "@minecraft/server-admin",
        "@minecraft/server-editor",
        "@minecraft/server-net"
    ]
}

```

4. Linux平台下载最新release构建的**NIAHttpBOT**来获取最新版的`NIAHttp-Bot`

5. 根据[NapCatQQ安装教程](https://napneko.icu/guide/start-install)安装相应的机器人框架

6. 安装后，打开机器人WEBUI界面，**点击侧边栏网络配置**，点击**添加配置**

7. 名称首先写上**服务器**，类型选择**http服务器**，启用，并将端口与NIAHttpBOT的配置文件中的**ClientPort**保持一致；

8. 然后再次点击**添加配置**，名称写上客户端，类型选择**http客户端**，启用，并将URL一栏与NIAHttpBOT中配置文件的`http://127.0.0.1:<ServerPort>/<Locate>`保持一致，如果是原始配置文件没有改动，则为`http://127.0.0.1:10086/qqEvent`，机器人至此配置完毕。

9. 按照[NapCatQQ安装教程](https://napneko.icu/guide/start-install)中指示启动机器人

10. 在终端输入`./NIAHttpBOT`来启动NIAhttpBOT

> [!tip]
> 在Linux如果出项**权限不够**的提示，这个错误是因为你试图运行的文件没有执行权限。你可以使用 `chmod` 命令来给文件添加执行权限。以下是具体的步骤（11-13）：

11. 打开终端

12. 使用 `cd` 命令导航到文件所在的目录

13. 运行 `chmod +x NIAHttpBOT` 命令给文件添加执行权限

然后你就可以使用 `./NIAHttpBOT` 命令来运行你的程序了，第一次启动时会生成配置文件，配置文件路径为`./NIAHttpBOT.cfg`，您可以根据自己的需求进行修改，具体修改教程见[配置文件](#配置文件)，修改后请输入`reload`来重新加载配置文件。

14. 最后启动MCBDS服务端即可！


## 开发注意事项


1. 我们是基于NapCatQQ开发的QQ机器人

2. 由于**minecraft/server-net**模块在本地存档中无法启用，所以我们应当在本地搭建一个如上述教程服务器环境用于开发

3. 在本地开发时,我们应当解除loopback，否则我们无法在游戏中通过`127.0.0.1`进入游戏，具体操作步骤如下：

**以管理员身份打开PowerShell**，然后根据MC版本输入以下命令：

Minecraft Bedrock Edition（正式版）
```PowerShell
CheckNetIsolation.exe LoopbackExempt -a -p=S-1-15-2-1958404141-86561845-1752920682-3514627264-368642714-62675701-733520436
```

Minecraft Bedrock Edition Preview（预览版）
```PowerShell
CheckNetIsolation.exe LoopbackExempt -a -p=S-1-15-2-424268864-5579737-879501358-346833251-474568803-887069379-4040235476
```


***

## 配置文件

```cfg
# 语言文件路径,默认为空-CN
LanguageFile = ""

# ip地址，一般为不用改
IPAddress = "127.0.0.1"

# 服务器端口，需与行为包端口以及QQ机器人作为客户端上报事件地址保持一致
ServerPort = 10086

# 功能配置:

#是否启用DOS指令功能
UseCmd = false

# QQ机器人配置:

# 是否启用QQ机器人,默认为true
UseQQBot = true

# 客户端端口，需要与QQ机器人作为服务器设置的监听Http端口一致
ClientPort = 10023

# 不知道啥作用的不用改
Locate = "/qqEvent"

# 主人QQ配置
OwnerQQ = "123456789"

# 监听QQ群
QQGroup = "123456789"

```

***
## QQ-BOT 相关指令

### 一般指令

#### 参数说明

`<XboxID>` 玩家的XboxID

注：当前还不能录入带空格的XboxID,需要管理员手动修改`player_data.json`文件

#### 详细指令

```
#帮助: 显示帮助菜单

#赞我: 给自己点10个赞

#绑定 <XboxID>: 绑定XboxID

例：#绑定 NIANIANKNIA

#查：查询自己账号的相关信息

#查 @要查询的人 : 查询别人账号的相关信息
```

### 管理指令

#### 参数说明

`<时间>` 应当以min(分钟)、h(小时)、d(天)为结尾，前面只能为阿拉伯数字

例：1min、10h、100d等

#### 详细指令

```

#禁言 @要禁言的人 <时间>: 禁言指定群成员

例：#禁言 @NIANIANKNIA 1h

#解禁 @要解禁的人: 解禁指定群成员

#改绑 @要改绑的人 <XboxID>: 改绑XboxID

#封禁 @要封禁的人 <时间>: 封禁指定群成员游戏账号

例：#封禁 @NIANIANKNIA 1d

#解封 @要解封的人: 解封指定群成员账号

#改权限 @要改权限的人 <权限>: 改变指定群成员的权限
```


## HTTP API一览表

> [!warning]
> 以下所有api均基于最新构建的版本（见下方RELEASE）

[![Latest Release](https://img.shields.io/github/v/release/Nia-Server/NIAHttpBOT?include_prereleases&style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/releases/latest)

### [POST] `/RunCmd`

执行DOS命令

> [!warning]
> 由于API涉及服务器安全性问题，本功能默认关闭，请在确定做好准备的条件下修改配置文件后启用本功能！


可以实现的功能：

- [创建文件夹](https://learn.microsoft.com/zh-cn/windows-server/administration/windows-commands/mkdir)
- [删除文件](https://learn.microsoft.com/zh-cn/windows-server/administration/windows-commands/del)
- [向NIAHttpBOT显示一行输出](https://learn.microsoft.com/zh-cn/windows-server/administration/windows-commands/echo)

如需了解更多DOS指令，请前往[微软官方文档站](https://learn.microsoft.com/zh-cn/windows-server/administration/windows-commands/windows-commands)查看

使用示例
```js
const port = 3000
const reqRunCmd = new HttpRequest(`http://127.0.0.1:${port}/RunCmd`);
    reqRunCmd.body = "del 123.txt"
    reqRunCmd.method = HttpRequestMethod.Post;
    reqRunCmd.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqRunCmd).then((response) => {
    if (response.status == 200) {
        console.log("Dos command executed successfully!")
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```


### [POST] `/CheckFile`

检查一个文件是否存在，目标文件存在则返回`true`,状态码为`200`，不存在则返回`false`，状态码为`400`

使用示例

```js
const port = 3000
const reqCheckFile = new HttpRequest(`http://127.0.0.1:${port}/CheckFile`);
    reqCheckFile.body = "FileName.json"
    reqCheckFile.method = HttpRequestMethod.Post;
    reqCheckFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqCheckFile).then((response) => {
    if (response.status == 200) {
        console.log("Target file exists.")
    } else if (response.status == 400) {
        console.error("The target file does not exist")
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/CheckDir`

检查目标文件夹是否存在，目标文件夹存在则返回`true`，状态码为`200`，不存在则返回`false`，状态码为`400`

使用示例

```js
const port = 3000
const reqCheckDir = new HttpRequest(`http://127.0.0.1:${port}/CheckDir`);
    reqCheckDir.body = "./A"
    reqCheckDir.method = HttpRequestMethod.Post;
    reqCheckDir.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqCheckDir).then((response) => {
    if (response.status == 200) {
        console.log("Target folder exists.")
    } else if (response.status == 400) {
        console.error("The target folder does not exist")
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/CreateNewFile`

创建一个文件，创建成功返回`success`，状态码为`200`，创建失败则返回`失败原因`，状态码为`400`


使用示例

```js
const port = 3000
const reqCreateNewFile = new HttpRequest(`http://127.0.0.1:${port}/CreateNewFile`);
    reqCreateNewFile.body = JSON.stringify({"fileName":"test.txt","content":"这是第一行\n这是第二行"})
    reqCreateNewFile.method = HttpRequestMethod.Post;
    reqCreateNewFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqCreateNewFile).then((response) => {
    if (response.status == 200) {
        console.log("File created successfully!")
    } else if (response.status == 400) {
        console.error(response.body)
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```



### [POST] `/CreateNewJsonFile`

创建一个JSON文件，创建成功返回`success`，状态码为`200`，创建失败则返回`失败原因`，状态码为`400`

使用示例

```js
const port = 3000
const reqCreateNewJsonFile = new HttpRequest(`http://127.0.0.1:${port}/CreateNewJsonFile`);
    reqCreateNewJsonFile.body = JSON.stringify({"fileName":"market111.json","content":{"a":10}})
    reqCreateNewJsonFile.method = HttpRequestMethod.Post;
    reqCreateNewJsonFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqCreateNewJsonFile).then((response) => {
    if (response.status == 200) {
        console.log("File created successfully!")
    } else if (response.status == 400) {
        console.error(response.body)
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/GetFileData`

获取文件数据，获取成功则返回文件数据（类型为字符串），状态码为`200`，获取失败则返回`fail`，状态码为`400`

使用示例

```js
const port = 3000
const reqGetFileData = new HttpRequest(`http://127.0.0.1:${port}/GetFileData`);
    reqGetFileData.body = "text.txt"
    reqGetFileData.method = HttpRequestMethod.Post;
    reqGetFileData.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqGetFileData).then((response) => {
    if (response.status == 200) {
        console.log("Get file data successfully! File data:" + response.body)
    } else if (response.status == 400) {
        console.error("The target file does not exist")
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```


### [POST] `/GetJsonFileData`

::: warning 警告
json文件应当没有任何语法错误/注释，否则将无法正确读取json数据！，详细请查看[json文件读取注意事项](#json文件读取注意事项)
:::

获取JSON文件数据，获取成功则返回json格式的数据，状态码为`200`，获取失败则返回`fail`，状态码为`400`

使用示例
```js
const port = 3000
const reqGetJsonFileData = new HttpRequest(`http://127.0.0.1:${port}/GetJsonFileData`);
    reqGetJsonFileData.body = "market.json"
    reqGetJsonFileData.method = HttpRequestMethod.Post;
    reqGetJsonFileData.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqGetJsonFileData).then((response) => {
    if (response.status == 200) {
        console.log("Get file data successfully! File data:" + response.body)
    } else if (response.status == 400) {
        console.error("The target file does not exist")
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/OverwriteFile`

覆盖文件内容，覆盖成功则返回`success`，状态码为`200`，覆盖失败则返回`失败原因`，状态码为`400`

使用示例

```js
const port = 3000
const reqOverwriteFile = new HttpRequest(`http://127.0.0.1:${port}/OverwriteFile`);
    reqOverwriteFile.body = JSON.stringify({"fileName":"FileName.txt","content": "这是第一行\n这是第二行"})
    reqOverwriteFile.method = HttpRequestMethod.Post;
    reqOverwriteFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqOverwriteFile).then((response) => {
    if (response.status == 200) {
        console.log("Overwrite file data successfully!")
    } else if (response.status == 400) {
        console.error(response.body)
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/OverwriteJsonFile`

覆盖JSON文件内容，覆盖成功则返回`success`，状态码为`200`，覆盖失败则返回`失败原因`，状态码为`200`

使用示例

```js
const port = 3000
const reqOverwriteJsonFile = new HttpRequest(`http://127.0.0.1:${port}/OverwriteJsonFile`);
    reqOverwriteJsonFile.body = JSON.stringify({"fileName":"FileName.json","content":{"a":"呵呵呵呵"}})
    reqOverwriteJsonFile.method = HttpRequestMethod.Post;
    reqOverwriteJsonFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqOverwriteJsonFile).then((response) => {
    if (response.status == 200) {
        console.log("Overwrite file data successfully!")
    } else if (response.status == 400) {
        console.error(response.body)
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```



### [POST] `/WriteLineToFile`

向目标文件最后写入如一行内容，成功则返回`success`，状态码为`200`，失败则返回`失败原因`，状态码为`400`

**注意增加换行符（\n），否则不会换行！**

使用示例

```js
const port = 3000
const reqWriteLineToFile = new HttpRequest(`http://127.0.0.1:${port}/WriteLineToFile`);
    reqWriteLineToFile.body = JSON.stringify({"fileName":"123.txt","content": "这是一行测试内容" + "\n"})
    reqWriteLineToFile.method = HttpRequestMethod.Post;
    reqWriteLineToFile.headers = [
        new HttpHeader("Content-Type", "text/plain"),
    ];
http.request(reqWriteLineToFile).then((response) => {
    if (response.status == 200) {
        console.log("Overwrite file data successfully!")
    } else if (response.status == 400) {
        console.error(response.body)
    } else {
        console.error("Dependent server connection failed! Check whether the dependent server started successfully.")
    }
})
```

### [POST] `/CopyFolder`

将特定文件夹复制到指定位置

## 附加说明

### JSON文件读取注意事项

**json文件应当没有任何语法错误/注释，否则将无法正确读取json数据！**

以下是错误示例：

多了一个`,`产生了语法错误！
```json
{
    "key":value,
}
```

不能进行注释，否则会导致无法正确读取！
```json
{
    //这是一行注释，这会导致无法正常读取！
    "key":value
}
```

错误的使用了中文的标点符号导致语法错误！
```json
{
    //这是一行注释，这会导致无法正常读取！
    "key1":value，
    "key2"：value
}
```

## 使用示例

**建议使用的方法**

创建`./API/filesystem.js`文件，内容如下

(目前写了一些常用的功能，更多功能将在后续更新)

[点击下载示例文件](https://github.com/Nia-Server/NiaServer-Core/blob/dev/development_behavior_packs/NIA_V4.0_BP/scripts/API/filesystem.js)


```js
import {http,HttpRequestMethod,HttpRequest,HttpHeader} from '@minecraft/server-net';

const port = 10086
const server_url = "http://127.0.0.1"

export class ExternalFS {

    /**
     * @function 执行DOS命令
     * @param {String} cmd
     * @return {String | Number} 获取成功返回success，服务器连接失败返回-1
     */
    RunCmd(cmd) {
        const reqRunCmd = new HttpRequest(`${server_url}:${port}/RunCmd`)
        .setBody(cmd)
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqRunCmd);
            if (response.status == 200) {
                resolve(response.body);
            } else {
                resolve(-1);
            }
        })
    }

    /**
     * @function 获取文件内容
     * @param {String} filename
     * @return {String | Number} 获取成功返回文件数据，文件不存在返回0，服务器连接失败返回-1
     */
    GetFileData(filename) {
        const reqGetFileData = new HttpRequest(`${server_url}:${port}/GetFileData`)
        .setBody(filename)
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqGetFileData);
            if (response.status == 200) {
                resolve(JSON.parse(response.body));
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        })
    }

    /**
     * @function 获取json文件内容
     * @param {String} filename
     * @return {Object | Number} 获取成功返回json数据，文件不存在返回0，服务器连接失败返回-1
     */
    GetJSONFileData(filename) {
        const reqGetJsonFileData = new HttpRequest(`${server_url}:${port}/GetJsonFileData`)
        .setBody(filename)
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqGetJsonFileData);
            if (response.status == 200) {
                resolve(JSON.parse(response.body));
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        })
    }

    /**
     * @function 创建新文件
     * @param {String} filename
     * @param {String} filecontent
     * @return {String | Number} 创建成功返回success，创建失败返回0，服务器连接失败返回-1
     */
    CreateNewFile(filename,filecontent) {
        const reqCreateNewFile = new HttpRequest(`${server_url}:${port}/CreateNewFile`)
        .setBody(JSON.stringify({"fileName":filename,"content":filecontent}))
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain")
        return new Promise(async (resolve) => {
            const response = await http.request(reqCreateNewFile);
            if (response.status == 200) {
                resolve(response.body);
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        });
    }

    /**
     * @function 创建json文件
     * @param {String} filename
     * @param {Object} filecontent
     * @return {String | Number} 创建成功返回success，创建失败返回0，服务器连接失败返回-1
     */
    CreateNewJsonFile(filename,filecontent) {
        const reqCreateNewJsonFile = new HttpRequest(`${server_url}:${port}/CreateNewJsonFile`)
        .setBody(JSON.stringify({"fileName":filename,"content":filecontent}))
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain")
        return new Promise(async (resolve) => {
            const response = await http.request(reqCreateNewJsonFile);
            if (response.status == 200) {
                resolve(response.body);
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        });
    }

    /**
     * @function 覆写文件
     * @param {String} filename
     * @param {String} filecontent
     * @return {String | Number} 覆写成功返回success，覆写失败返回0，服务器连接失败返回-1
     */
    OverwriteFile(filename,filecontent) {
        const reqOverwriteFile = new HttpRequest(`${server_url}:${port}/OverwriteFile`)
        .setBody(JSON.stringify({"fileName":filename,"content":filecontent}))
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqOverwriteFile);
            if (response.status == 200) {
                resolve(response.body);
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        })
    }

    /**
     * @function 覆写json文件
     * @param {String} filename
     * @param {Object} filecontent
     * @return {String | Number} 覆写成功返回success，覆写失败返回0，服务器连接失败返回-1
     */
    OverwriteJsonFile(filename,filecontent) {
        const reqOverwriteJsonFile = new HttpRequest(`${server_url}:${port}/OverwriteJsonFile`)
        .setBody(JSON.stringify({"fileName":filename,"content":filecontent}))
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqOverwriteJsonFile);
            if (response.status == 200) {
                resolve(response.body);
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        })
    }

    /**
     * @function 向特定文件写入一行内容
     * @param {String} filename
     * @param {String} filecontent
     * @return {String | Number} 写入成功返回success，覆写失败返回0，服务器连接失败返回-1
     */
    WriteLineToFile(filename,filecontent) {
        const reqWriteLineToFile = new HttpRequest(`${server_url}:${port}/WriteLineToFile`)
        .setBody(JSON.stringify({"fileName":filename,"content":filecontent}))
        .setMethod(HttpRequestMethod.Post)
        .addHeader("Content-Type", "text/plain");
        return new Promise(async (resolve) => {
            const response = await http.request(reqWriteLineToFile);
            if (response.status == 200) {
                resolve(response.body);
            } else if (response.status == 400) {
                resolve(0);
            } else {
                resolve(-1);
            }
        })
    }
}




```

然后我们可以直接调用即可，不用反复写

调用示例(截取自玩家交易市场部分代码)

```js
import { world } from '@minecraft/server';
import { ExternalFS } from './API/filesystem';

//违禁物品，等后期接入配置文件
const fs = new ExternalFS();
const port = 10086
var MarketData = [-1]


//服务器启动监听&&获得玩家市场数据
world.afterEvents.worldInitialize.subscribe(() => {
    fs.getJSONFileData("market.json").then((result) => {
        //文件不存在
        if (result === 0) {
            fs.CreateNewJsonFile("market.json",[]).then((result) => {
                if (result === "success") {
                    MarketData = [];
                    console.log("玩家市场文件不存在，已成功创建！");
                } else if (result === -1) {
                    console.error("依赖服务器连接失败！请检查依赖服务器是否成功启动，以及端口是否设置正确！");
                }
            });
        } else if (result === -1) {
            console.error("依赖服务器连接失败！请检查依赖服务器是否成功启动，以及端口是否设置正确！");
        } else {
            //文件存在且服务器连接成功
            MarketData = result;
            console.log("玩家市场数据获取成功！")
        }
    })
})
```

**实际应用**

[玩家交易市场](https://github.com/Nia-Server/NiaServer-Core/blob/dev/development_behavior_packs/NIA_V4.0_BP/scripts/market.js)

## 许可证

本项目基于[`AGPL-3.0`](https://github.com/Nia-Server/NIAHttpBOT/blob/main/LICENSE)开源许可证条款



