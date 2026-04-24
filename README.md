# NIAHttpBOT

[![status](https://img.shields.io/github/actions/workflow/status/Nia-Server/NIAHttpBOT/pre-build.yml?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/actions/workflows/pre-build.yml)
[![GitHub Release Date](https://img.shields.io/github/release-date-pre/Nia-Server/NIAHttpBOT?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/releases)
[![Latest Release](https://img.shields.io/github/v/release/Nia-Server/NIAHttpBOT?include_prereleases&style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/releases/latest)
[![GitHub last commit](https://img.shields.io/github/last-commit/Nia-Server/NIAHttpBOT?style=for-the-badge)](https://github.com/Nia-Server/NIAHttpBOT/commits)

![NIAHttpBOT](https://socialify.git.ci/Nia-Server/NIAHttpBOT/image?description=1&descriptionEditable=%E5%9F%BA%E4%BA%8Ehttp%E4%B8%BABDS%E5%AE%9E%E7%8E%B0%E6%9B%B4%E5%A4%9A%E5%8F%AF%E8%83%BD&font=Source%20Code%20Pro&forks=1&issues=1&logo=https://docs.mcnia.com/logo.png&name=1&pattern=Floating%20Cogs&pulls=1&stargazers=1&theme=Auto)


## 简介

使用C++开发的BOT，基于http实现更多可能，本项目为[NiaServer-Core](https://github.com/Nia-Server/NiaServer-Core)子项目


## 为什么开发？

由于目前我的世界的Script-api无法实现诸如文件读写等功能,为此我们特此基于C++开发了`NIAHttpBOT`用来实现更多功能，从而赋予Script-api更多可能

***

## 功能特性

- 基于http可以实现对特定文件进行读写、创建以及删除等功能
- 基于http搭配**NiaServer-Core**以及**NapCatQQ**可以实现QQ机器人与服务器联动功能

***

## 项目优势

- 轻量级设计，内存占用低
- 即开即用，无需复杂环境配置
- 跨平台兼容
- 不受Minecraft BDS版本更新的影响

***

## 使用前注意事项

1. 本项目基于**http**进行通讯，故当前Minecraft版本应当注意启用**minecraft/server-net**模块（该模块只能运行在服务器上）

2. 您可以前往**NIAHttpBOT**项目地址的[release](https://github.com/Nia-Server/NIAHttpBOT/releases)下载最新release构建的**NIAHttpBOT.exe**来获取最新版的`NIAHttpBOT`

3. 如果您在使用期间遇到了问题/有建议，您可以前往**NIAHttpBOT**的[issues](https://github.com/Nia-Server/NIAHttpBOT/issues)进行反馈！

4. 由于**采用的是http通讯，而非https**，我们**非常不推荐**您将NIAHttpBOT与基岩版服务端分开放置于两台服务器上，这是非常不安全的操作！请务必**将NIAHttpBOT与基岩版服务端放置于同一台服务器之上**，并**注意防火墙设置**，不要开放使用过程中涉及的两个端口，以免对服务器安全造成威胁！

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

1. Windows平台下下载最新release构建的**NIAHttpBOT.exe**来获取最新版的`NIAHttpBOT`

2. 根据[NapCatQQ安装教程](https://napneko.icu/guide/start-install)安装相应的机器人框架

3. 安装后，打开机器人**WEBUI**界面，**点击侧边栏网络配置**，点击**添加配置**

4. 名称首先写上**服务器**，类型选择**http服务器**，启用，并将端口与NIAHttpBOT的配置文件中的**ClientPort**保持一致；

5. 然后再次点击**添加配置**，名称写上客户端，类型选择**http客户端**，启用，并将URL一栏与NIAHttpBOT中配置文件的`http://127.0.0.1:<ServerPort>/<Locate>`保持一致，如果是原始配置文件没有改动，则为`http://127.0.0.1:10086/qqEvent`，机器人至此配置完毕。

6. 按照[NapCatQQ安装教程](https://napneko.icu/guide/start-install)中指示启动机器人

7.  双击**NIAHttpBOT.exe**来启动，第一次启动时会生成配置文件，配置文件路径为`./NIAHttpBOT.json`，您可以根据自己的需求进行修改，具体修改教程见[配置文件](#配置文件)。

8.  最后启动MCBDS服务端即可！


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

4. Linux平台下载最新release构建的**NIAHttpBOT**来获取最新版的`NIAHttpBOT`

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

然后你就可以使用 `./NIAHttpBOT` 命令来运行你的程序了，第一次启动时会生成配置文件，配置文件路径为`./NIAHttpBOT.json`，您可以根据自己的需求进行修改，具体修改教程见[配置文件](#配置文件)。

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

```json
{
    "base": {
        "LanguageFile": "",
        "IPAddress": "127.0.0.1",
        "ServerPort": 2333
    },
    "bds": {
        "DefaultInstanceId": "main",
        "AutoStartDelaySeconds": 5,
        "Instances": [
            {
                "Id": "main",
                "Name": "Main",
                "ExecutablePath": "D:/NiaServer-Core/bedrock_server.exe",
                "WorkingDirectory": "D:/NiaServer-Core",
                "AutoStart": true,
                "UseCmd": false,
                "AutoBackup": false,
                "BackupHour": 4,
                "BackupMinute": 0,
                "BackupSecond": 0,
                "BackupFrom": "D:/NiaServer-Core/worlds/250117",
                "BackupTo": "./backup/default",
                "LogTag": "main"
            }
        ]
    },
    "qqbot": {
        "UseQQBot": false,
        "QQIPAddress": "127.0.0.1",
        "QQClientPort": 10023,
        "QQServerPort": 10086,
        "OwnerQQ": "123456789",
        "QQGroup": "123456789"
    }
}
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

## 第三方开源引用

#### [rapidjson](https://github.com/Tencent/rapidjson) - [MIT License](https://github.com/Tencent/rapidjson?tab=License-1-ov-file#readme)

#### [cpp-httplib](https://github.com/yhirose/cpp-httplib) - [MIT License](https://github.com/yhirose/cpp-httplib?tab=MIT-1-ov-file#readme)

#### [OBJ-Loader](https://github.com/Bly7/OBJ-Loader) - [MIT License](https://github.com/Bly7/OBJ-Loader?tab=MIT-1-ov-file)

## 许可证

本项目基于[`AGPL-3.0`](https://github.com/Nia-Server/NIAHttpBOT/blob/main/LICENSE)开源许可证条款
