# v1.0.1 更新日志

[![NIAHttpBOT-VERSION](https://img.shields.io/badge/NIAHttpBOT-v1.0.0-blue?style=for-the-badge&logo=appveyor)](https://github.com/Nia-Server/NIAHttpBOT/) [![NapCatQQ-VERSION](https://img.shields.io/badge/NapCatQQ-v4.3.3-green?style=for-the-badge&logo=appveyor)](https://github.com/NapNeko/NapCatQQ/releases/tag/v4.3.3)

> 本项目为 [NiaServer-Core](https://github.com/Nia-Server/NiaServer-Core) 的子项目。



## 优化

配置文件项目内容

变更前内容如下

## 新增

1. 基于[NapCatQQ](https://napneko.icu/)使用[onebot-11](https://github.com/botuniverse/onebot-11/)为NIAHttpBOT新增QQ机器人功能
- 基础群管功能（禁言、屏蔽词等）
- 玩家白名单自助绑定
- 玩家封禁&&解封
- 群聊执行BDS服务器开服&&关服
- 群聊执行BDS服务器内指令
- 群聊&&服务器内玩家消息互通（仅在搭配NiaServer-Core v1.5.0及以上版本时可用）
- BDS服务器玩家白名单增添与删除
2. 指令系统
- **reload** 指令，用于重载NIAHttpBOT
- **stop** 指令，用于关闭NIAHttpBOT
- **help** 指令，获取NIAHttpBOT指令帮助信息
- **startserver** 指令，用于启动BDS服务器
- **mc** && **/** 指令，用于执行BDS服务器中指令(当前版本仅windows支持)
- **stopserver** 指令，用于停止BDS服务器(当前版本仅windows支持)
3. https相关功能支持（当前版本仅windows支持）
4. 根据github最新release版本提供检查更新功能
5. 读取&&解析OBJ文件功能
6. 在启动NIAHttpBOT时，可选自动启动BDS服务器(当前版本仅windows支持)
7. 在使用NIAHttpBOT启动BDS服务器输出消息可内嵌在NIAHttpBOT输出日志中
8. 自动备份BDS服务器存档功能支持
9. 基于以上功能，配置文件新增`ServerLocate`、`AutoStartServer`、`UseQQBot`、`Locate`、`OwnerQQ`、`QQGroup`配置项

## 修复

启动时log信息ip信息显示错误


**配置说明：您可以前往[NIA服务器官方文档站](https://docs.mcnia.com/dev/Http-Bot.html)查看具体部署过程！**