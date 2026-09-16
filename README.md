# SnowyArk Engine

## 目录

- [项目介绍](#项目介绍)
- [当前状态](#当前状态)
- [开始使用](#开始使用)
- [文档](#文档)
- [许可证](#许可证)

## 项目介绍

SnowyArk 是一个用于学习和实验实时渲染的 C++20 游戏引擎项目.

引擎代码, 工具, 资产和测试放在 `Engine/`. `Samples/` 用于示例工程, 目前为空. 引擎与各个示例分别构建.

## 当前状态

项目处于搭建初期, 已有目录结构和 CMake 构建配置. 窗口, 主循环和渲染功能尚未实现, Vulkan 后端也只有代码骨架.

| 构建目标 | 当前内容 |
| --- | --- |
| `SnowyArk` | 引擎运行时静态库, 各模块保留代码骨架 |
| `SnowyArkEditor` | 编辑器入口, 输出 `Hello SnowyArk Engine!` 后退出 |
| `SnowyArkShaderCompiler` | 离线 Shader 编译工具入口, 尚未实现编译功能 |
| `SnowyArkTests` | 测试入口, 尚未添加引擎功能测试 |

## 开始使用

构建需要 CMake 3.24 或更高版本, Ninja 和支持 C++20 的编译器. Windows 开发使用 MSVC x64 工具链.

使用 Rider 时, 打开 `Engine/CMakeLists.txt` 或 `Engine/` 文件夹. 日常开发选择 `Develop`, 对应 CMake 的 `Debug`; 优化构建选择 `Release`. 启动目标为 `SnowyArkEditor`.

完整的环境配置, 命令行构建和 Rider 操作步骤见 [构建与运行](Engine/Docs/Building.md).

## 文档

从 [文档库](Engine/Docs/README.md) 查看各主题的阅读入口.

- [构建与运行](Engine/Docs/Building.md): 编译配置, 测试命令和 Rider 设置.
- [工程结构](Engine/Docs/Architecture.md): 目录用途, 模块职责和依赖关系.
- [AI 开发约定](AGENTS.md): AI 修改本项目时应遵循的代码, 构建和验证要求.

## 许可证

本项目使用 [MIT License](LICENSE).
