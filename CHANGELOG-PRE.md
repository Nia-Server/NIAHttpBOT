# v1.0.0-pre-3 更新日志

[![NIAHttpBOT-VERSION](https://img.shields.io/badge/NIAHttpBOT-v1.0.0-orange?style=for-the-badge&logo=appveyor)](https://github.com/Nia-Server/NIAHttpBOT/) [![NapCatQQ-VERSION](https://img.shields.io/badge/NapCatQQ-v4.3.3-green?style=for-the-badge&logo=appveyor)](https://github.com/NapNeko/NapCatQQ/releases/tag/v4.3.3)

> 本项目为 [NiaServer-Core](https://github.com/Nia-Server/NiaServer-Core) 的子项目。

> **预发布版本提醒**：这是一个**预览版本**，可能存在一些bug，仅供测试，请勿在正式生产环境使用本版本！

> **未完成开发版本提醒**：该版本**部分功能仍然在开发状态中**，可能出现包括但不限于**服务器存档损坏**等问题，仅供开发者预览，请勿使用！

## 优化

1. 统一BDS服务器与NIAHttpBOT日志输出样式
2. 分离控制BDS服务器相关代码
3. `ctrl c`关闭NIAHttpBOT时，会自动关闭BDS服务器(当前版本仅windows支持)
4. BDS服务器关闭与开启逻辑
5. `reload`指令重启逻辑
6. 配置文件读取逻辑


## 新增

1. 配置项目`AutoStartServer`，用于控制NIAHttpBOT是否自动启动BDS服务器(当前版本仅windows支持)
2. QQBOT指令`/`，输入带有`/`前缀的指令，会作为BDS服务器指令执行(当前版本仅windows支持)
3. NIAHttpBOT指令`/`，输入带有`/`前缀的指令，会作为BDS服务器执行(当前版本仅windows支持)

## 修复

1. reload指令没有关闭bds服务器的问题
2. 指令执行后结果过长时输出不完整的问题


**配置说明：您可以前往[NIA服务器官方文档站](https://docs.mcnia.com/dev/Http-Bot.html)查看具体部署过程！**