# SnowyArk 工程结构

## 目录

- [目标](#目标)
- [顶层目录](#顶层目录)
- [构建与依赖配置](#构建与依赖配置)
- [引擎模块](#引擎模块)
- [Runtime 与 Editor 职责](#runtime-与-editor-职责)
- [依赖规则](#依赖规则)
- [生成目录](#生成目录)
- [后续扩展](#后续扩展)

## 目标

SnowyArk 用于实时渲染的学习和实验. 当前编辑器可以打开窗口并提交硬编码 triangle pass. 本文说明代码应放在哪里, 以及各模块已经承担和后续实现时应承担的职责.

项目使用 CMake 和 C++20. 编译选项见 [CMake 维护](Building.md#cmake-维护), 命名和注释要求见 [代码约定](../../AGENTS.md#代码约定).

## 顶层目录

```text
SnowyArk-Engine/
├─ Engine/
│  ├─ Assets/
│  │  └─ Shaders/
│  │     ├─ Include/
│  │     └─ Passes/
│  ├─ Source/
│  │  ├─ Runtime/
│  │  │  ├─ Application/
│  │  │  ├─ Core/
│  │  │  ├─ Platform/
│  │  │  ├─ GAL/
│  │  │  │  └─ Vulkan/
│  │  │  ├─ Shader/
│  │  │  ├─ RenderPipeline/
│  │  │  └─ CMakeLists.txt
│  │  ├─ Editor/
│  │  │  ├─ Main.cpp
│  │  │  ├─ EditorApplication.h
│  │  │  ├─ EditorApplication.cpp
│  │  │  └─ CMakeLists.txt
│  │  ├─ Tools/
│  │  │  └─ ShaderCompiler/
│  │  └─ ThirdParty/
│  ├─ Tests/
│  ├─ CMake/
│  ├─ Docs/
│  │  ├─ README.md
│  │  ├─ Building.md
│  │  ├─ Architecture.md
│  │  └─ Decisions/
│  │     └─ FirstTriangle.md
│  ├─ Scripts/
│  ├─ CMakeLists.txt
│  ├─ CMakePresets.json
│  └─ vcpkg.json
├─ Samples/
│  └─ .gitkeep
├─ README.md
├─ AGENTS.md
├─ LICENSE
├─ .gitattributes
└─ .gitignore
```

`Engine/` 包含 Runtime, Editor, Tools 以及引擎所需的资产, 测试和开发配置. `Samples/` 用于独立示例工程, 当前只有 `.gitkeep`, 尚无示例代码或构建目标.

`Engine/Assets/` 保存源资产, Shader 源文件目录为 `Engine/Assets/Shaders/`. 编辑器专用资产有实际需求后再增加目录. `Engine/Tests/` 保存测试, `Engine/Scripts/` 保存开发脚本, `Engine/Docs/` 保存构建说明, 架构和设计决策, 入口为 [文档目录](README.md).

## 构建与依赖配置

`Engine/CMakeLists.txt` 是引擎的独立构建入口, 负责项目声明, C++20 和 CTest 设置, 并组织四个主要 target:

| Target | 源码位置 | 职责 |
| --- | --- | --- |
| `SnowyArk` | `Engine/Source/Runtime/` | 运行时静态库 |
| `SnowyArkEditor` | `Engine/Source/Editor/` | 编辑器入口 |
| `SnowyArkShaderCompiler` | `Engine/Source/Tools/ShaderCompiler/` | 独立离线工具 |
| `SnowyArkTests` | `Engine/Tests/` | 测试入口, 由 `BUILD_TESTING` 控制是否生成 |

`Engine/CMake/` 保存编译选项, 警告和依赖配置辅助文件. 编译命令和 Rider 设置见 [构建与运行](Building.md), 源文件收集, 平台选择和编译选项的传递规则见 [CMake 维护](Building.md#cmake-维护).

`Engine/vcpkg.json` 保存包依赖清单, 当前包含 `glfw3`. `Engine/Source/ThirdParty/` 保存随源码维护的第三方代码, 包管理器的安装产物不放在这里. Vulkan headers 和 `slangc` 来自本机 LunarG SDK, 不进仓库.

Engine 和每个 `Samples/<Name>/` 分别维护 CMake 入口, Preset 和 `Build/` 目录, 依赖清单按实际需要添加. 各项目使用自己的 CMake 缓存, 仓库根目录不提供统一构建入口或 Preset.

根目录的 `README.md` 介绍项目并提供文档入口, `AGENTS.md` 指导 AI 开发. `LICENSE` 和 `.gitignore` 分别管理整个仓库的许可证和忽略规则. `.gitattributes` 自动识别文本文件, 将其换行统一为 LF, 二进制文件不做换行转换.

## 引擎模块

运行时模块均位于 `Engine/Source/Runtime/`. 下表是各目录的职责约定.

| 目录 | 职责 |
| --- | --- |
| `Core/` | 不依赖其他引擎模块的基础设施, 目前有日志, 可执行文件路径, 以及关闭时的 GPU 等待辅助 |
| `Application/` | 运行时子系统的初始化, 每帧推进 (Tick), 关闭与资源释放. Tick 不等待窗口事件; BeginFrame 返回空指针时跳过 Render / EndFrame |
| `Platform/` | 通用窗口接口, 含非阻塞 `PumpEvents` 和可阻塞的 `WaitEvents`. Windows 实现当前使用 GLFW, 不使用原生 Win32 窗口 |
| `GAL/` | Graphics Abstract Layer. 公共头是抽象基类 (`GraphicsDevice`, `SwapChain`, `CommandBuffer`, `PipelineState`), 由 `GraphicsDevice::Create` 按 `GraphicsBackend` 创建 |
| `GAL/Vulkan/` | GAL 的 Vulkan 后端, 唯一允许出现 `vk::*` 的目录 |
| `Shader/` | 运行时读取 SPIR-V. `ShaderLibrary::Load` 在文件缺失, 大小为 0, 不是 4 字节倍数, 或超出 `size_t` / `streamsize` 可表示范围时返回 false. 源文件是 Slang, 由 CMake 调用 `slangc` 编译 |
| `RenderPipeline/` | 上层帧级渲染管线. 当前是硬编码 triangle pass, 只通过 GAL 虚接口录制 |

这些目录共同构建为 `SnowyArk` 静态库. 新模块先放入对应目录, 有独立构建需求时再拆分 target.

## Runtime 与 Editor 职责

`EditorApplication` 已创建窗口并驱动主循环, `Runtime/Application` 已初始化图形设备并每帧提交 triangle pass. 后续仍按以下职责分工:

- `Engine/Source/Editor/Main.cpp` 作为可执行入口, 创建并启动 `EditorApplication`.
- `EditorApplication` 掌握外层主循环, 调用 Runtime Application 的初始化, Tick 和关闭操作, 并组织编辑器 UI, 编辑命令及未来的编辑/播放状态. framebuffer 为零时, 主循环调用 `Window::WaitEvents` 等待事件, 不空转; 关闭或恢复窗口后 WaitEvents 返回. `Application::Tick` 不等待窗口事件, 零尺寸时直接返回且不消耗 resize 标记; 绘制仍可能等待 GPU fence 或取图.
- `Runtime/Application` 只管理运行时子系统的生命周期和每帧推进, 不包含编辑器面板, 编辑命令或编辑器状态.
- 通用窗口创建和平台事件能力由 `Runtime/Platform` 提供, 面板布局和编辑器视图选择由 Editor 决定.
- 暂停游戏逻辑时, 编辑器界面和渲染视图仍应能够更新, 因此它们的更新不能始终绑定到游戏逻辑的 Tick.

Editor 首版已经有窗口和硬编码 triangle pass. GUI 框架, 播放模式, `IApplication` 和继承体系不在当前阶段引入.

GAL 用虚函数做后端分发, 等价于小型 RHI. `RenderPipeline` 和 `Application` 只依赖抽象基类. 现在 `GraphicsDevice::Create` 只实现 `GraphicsBackend::Vulkan`. 以后加 D3D 或 OpenGL 时新增 `GAL/<Api>/` 目录和工厂分支, 不改 Pass 录制代码. 不使用 CRTP, 也不把 `GraphicsDevice` typedef 成某个后端. 延迟命令编码和 RHI 线程不在当前范围.

录制对象命名为 `CommandBuffer`. fence, command pool, image view 留在 Vulkan 后端内部.

`GraphicsDevice::BeginFrame` 返回非拥有的 `CommandBuffer*`, 有效期到本次 `EndFrame`, 下一次 `BeginFrame`, swapchain 重建或 `Shutdown`. 取图失败, 窗口为零, 或 swapchain 仍待重建时返回空指针; 上层只在非空时调用 `Render` / `EndFrame`. acquire 区分 Success, Suboptimal, OutOfDate. Suboptimal 完成本帧并消费 acquire semaphore, 把重建留到本帧呈现之后. OutOfDate 和零尺寸跳过本帧, 不 reset fence, 也不使用无效 image index. 重建请求在零尺寸期间保留, 窗口恢复后再创建. 新的 image 和 image view 创建成功后再写入成员, 避免只清掉 view 的半成品. 创建会传入 `oldSwapchain`, 失败时旧 swapchain 也可能已退役 (retired), 所以抛出后不能再当可绘制 swapchain 用, 只留给随后的清理.

关闭时, `Application::Shutdown` 和 `VulkanDevice::Shutdown` 为 `noexcept`. `waitIdle` 失败 (例如 DeviceLost) 时, `CleanupWait::TryWait` 向 stderr 写一条尽力而为的诊断, 然后仍按依赖顺序释放已创建对象, 析构不向外抛异常. `Application::Initialize` 若已持有 window, device 或 swapchain, 返回 false 且不改现有对象. 工厂返回空指针或其他失败会先 Shutdown; 构造过程中抛出时由析构走同一条路径.

当前 swapchain 只接受 `R8G8B8A8Srgb` 与 `SrgbNonlinear`. 选设备要求 Vulkan 1.4, 以及逻辑设备会启用的 `shaderDrawParameters`, `dynamicRendering` 和 `synchronization2`; 选设备和创建 swapchain 时都会检查表面格式. 不满足则给出明确错误, 不会回退到其他格式.

## 依赖规则

- `Core` 不依赖其他 SnowyArk 模块.
- `RenderPipeline` 使用 `GAL` 的通用接口与 `Shader` 模块, 不直接依赖 `GAL/Vulkan` 后端.
- Vulkan 类型只允许出现在 `Engine/Source/Runtime/GAL/Vulkan/`. `GraphicsDevice.cpp` 是工厂, 可以 include 后端头文件以调用 `VulkanDevice::Create`, 但 `vk::*` 仍不应写进该文件.
- 平台专用类型只允许出现在对应的平台实现目录.
- `ShaderCompiler` 可以包含 `ShaderPackage.h`, 但不链接完整的 `SnowyArk` 运行时库.
- `Editor` 链接由 Runtime 构建的 `SnowyArk` 引擎库, `Runtime` 不依赖 `Editor`, `Tools` 或 `Samples`.
- include 搜索根目录为 `Engine/Source/`, 运行时头文件使用 `<Runtime/...>` 路径, 编辑器和工具分别使用 `<Editor/...>` 和 `<Tools/...>`, 不使用父目录相对路径.
- 引擎公开类型和自由函数位于命名空间 `SnowyArk`. 不按 Runtime / Editor 再拆嵌套命名空间. `main` 留在全局命名空间.

## 生成目录

- `Engine/Build/` 保存引擎的 CMake 构建树和二进制产物. 未来示例的构建产物分别位于 `Samples/<Name>/Build/`, 不在仓库根目录共享构建树.
- `Engine/Cache/` 计划保存引擎 Shader 和资产的可再生成派生数据.
- `Engine/Saved/` 计划保存引擎与编辑器日志, GPU capture 和用户配置.
- `Engine/Dist/` 保存未来的引擎可分发产物, 示例的分发产物归各自工程管理.

`.gitignore` 排除 Engine 和各示例工程的构建目录, Cache, Saved 和 Dist, 同时过滤 CMake 生成文件, 本机 Preset, 编译中间文件, 依赖与工具缓存, 临时文件, 日志, IDE 配置和会话记录. 项目输出目录的规则限定在 `Engine/` 和 `Samples/<Name>/` 下, 不影响源码中的同名目录.

上述生成内容均不应提交到版本库. `.obj` 也用于模型资产, `.lib`, `.dll` 等文件可能属于随源码维护的第三方依赖, `Makefile` 也可能是第三方源码的一部分, 因此不按这些文件名全局过滤; 它们在构建目录中的生成副本由目录规则排除. 新增其他输出位置时, 同步补充限定到对应路径的忽略规则.

## 后续扩展

后续计划随实际里程碑逐步加入 `Math`, `Material`, `RenderScene`, `RenderGraph`, `Asset`, `ECS`, `Scene` 和 `AssetCompiler`. 实现对应功能时再创建模块.

需要脱离编辑器运行游戏时, 再增加薄的 Player 入口来驱动 Runtime Application. 当前没有 `Engine/Source/Player`.

其他文档见 [文档目录](README.md).
