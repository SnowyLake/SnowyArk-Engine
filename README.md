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

项目已能在编辑器窗口中绘制透视旋转的彩色矩形: 顶点和索引缓冲保存几何, 每个在途帧槽通过独立 Uniform 缓冲更新模型, 观察和投影矩阵. 支持窗口尺寸变化和最小化恢复. 窗口由 GLFW 创建, 渲染走 Vulkan 1.4 Dynamic Rendering, Shader 用 Slang 编成 SPIR-V. GAL 是虚函数形式的小型 RHI, 目前只有 Vulkan 后端, 支持 Uniform 资源布局和集合绑定. 还没有纹理采样, 编辑器 UI 或离线 ShaderCompiler.

取图失败时跳过本帧, 最小化时等待窗口事件. 当前要求 surface 支持 `R8G8B8A8Srgb` 与 `SrgbNonlinear`. 窗口, GAL 和 Dynamic Rendering 的约定见 [第一个三角形相关决策](Engine/Docs/Decisions/FirstTriangle.md). 几何上传见 [顶点缓冲](Engine/Docs/Decisions/VertexBuffers.md), 帧槽更新和资源绑定见 [Uniform 缓冲](Engine/Docs/Decisions/UniformBuffers.md).

| 构建目标 | 当前内容 |
| --- | --- |
| `SnowyArk` | 引擎运行时静态库, 含窗口, GAL, Uniform 资源绑定与旋转矩形 pass |
| `SnowyArkEditor` | 打开窗口并绘制透视旋转的彩色矩形 |
| `SnowyArkShaderCompiler` | 离线 Shader 编译工具入口, 尚未实现编译功能 |
| `SnowyArkTests` | 帧决策, 清理等待, 窗口尺寸, Shader 加载和矩阵回归; 可选 GPU 回归检查资源绑定与 16/32 位索引绘制, 见 [编译与测试](Engine/Docs/Building.md#编译与测试) |

## 开始使用

构建需要 CMake 3.24 或更高版本, Ninja, 支持 C++20 的编译器, LunarG Vulkan SDK 和 vcpkg. Windows 开发使用 MSVC x64 工具链.

使用 Rider 时, 打开 `Engine/CMakeLists.txt` 或 `Engine/` 文件夹. 日常开发选择 `Develop`, 对应 CMake 的 `Debug`; 优化构建选择 `Release`. 启动目标为 `SnowyArkEditor`.

完整的环境配置, 命令行构建和 Rider 操作步骤见 [构建与运行](Engine/Docs/Building.md).

## 文档

从 [文档库](Engine/Docs/README.md) 查看各主题的阅读入口.

- [构建与运行](Engine/Docs/Building.md): 编译配置, 测试命令和 Rider 设置.
- [工程结构](Engine/Docs/Architecture.md): 目录用途, 模块职责和依赖关系.
- [第一个三角形相关决策](Engine/Docs/Decisions/FirstTriangle.md): GLFW, GAL 虚接口, Slang 和 Dynamic Rendering.
- [顶点缓冲](Engine/Docs/Decisions/VertexBuffers.md): 设备内上传, 顶点布局, 索引矩形.
- [Uniform 缓冲](Engine/Docs/Decisions/UniformBuffers.md): 资源布局, 描述符绑定, 帧槽同步与 MVP.
- [AI 开发约定](AGENTS.md): AI 修改本项目时应遵循的代码, 构建和验证要求.

## 许可证

本项目使用 [MIT License](LICENSE).
