# v1.0.0-pre-2 更新日志

[![NIAHttpBOT-VERSION](https://img.shields.io/badge/NIAHttpBOT-v1.0.0-orange?style=for-the-badge&logo=appveyor)](https://github.com/Nia-Server/NIAHttpBOT/) [![NapCatQQ-VERSION](https://img.shields.io/badge/NapCatQQ-v4.3.3-green?style=for-the-badge&logo=appveyor)](https://github.com/NapNeko/NapCatQQ/releases/tag/v4.3.3)

> 本项目为 [NiaServer-Core](https://github.com/Nia-Server/NiaServer-Core) 的子项目。

> **预发布版本提醒**：这是一个**预览版本**，可能存在一些bug，仅供测试，请勿在正式生产环境使用本版本！

> **未完成开发版本提醒**：该版本**部分功能仍然在开发状态中**，可能出现包括但不限于**服务器存档损坏**等问题，仅供开发者预览，请勿使用！

## 新增

1. **startserver** 指令，用于启动BDS服务器
2. **mc** 指令，用于执行Minecraft指令(当前版本仅windows支持)
3. **stopserver** 指令，用于停止BDS服务器(当前版本仅windows支持)
4. QQBOT新增**开服、关服、cmd**指令，用于控制BDS服务器(当前版本仅windows支持)

## 修复

1. 调用QQBOT_API时，部分情况传回400错误而导致NIAHttpBOT崩溃的问题

**配置说明：您可以前往[NIA服务器官方文档站](https://docs.mcnia.com/dev/Http-Bot.html)查看具体部署过程！**