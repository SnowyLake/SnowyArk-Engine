# SnowyArk 构建与运行

## 目录

- [环境准备](#环境准备)
- [编译与测试](#编译与测试)
- [C++ 格式化](#c-格式化)
- [Rider 配置](#rider-配置)
- [构建配置与输出](#构建配置与输出)
- [CMake 维护](#cmake-维护)

## 环境准备

构建需要 CMake 3.24 或更高版本, Ninja 和支持 C++20 的编译器. Windows 开发使用 MSVC x64 工具链. 在命令行构建前, 打开已初始化编译器环境的 MSVC x64 开发者终端.

另外需要:

- LunarG Vulkan SDK 1.4 或更高版本, 默认装在 `C:\VulkanSDK\<version>`. 安装后确认系统环境变量 `VULKAN_SDK` 已设置, `%VULKAN_SDK%\Bin` 在 `PATH` 中, 并且 `slangc.exe` 可运行. CMake 用 `find_package(Vulkan 1.4)` 检查 SDK 头文件版本. 通过这项检查只说明编译用的 headers 至少是 1.4. SDK 不安 GPU 驱动, 运行时的 loader 和 GPU 仍可能低于 1.4. 运行编辑器要求 loader 和 GPU 都报告 Vulkan 1.4, 可在 `vulkaninfo` 里看各设备的 `apiVersion`. 装完后必须重启终端和 Rider, 否则 CMake 看不到这些变量.
- vcpkg, 本机设置 `VCPKG_ROOT`. 共享 Preset 通过 `$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake` 接入清单模式. 当前清单在 `Engine/vcpkg.json`, 依赖 `glfw3`. MSVC 开发者命令行可能会把 `VCPKG_ROOT` 改成 Visual Studio 自带的 vcpkg; 若要用自己的克隆, 在调用 `vcvars` 之后重新设置 `VCPKG_ROOT`.

`Engine/CMakeLists.txt` 是引擎的构建入口. 共享配置位于 `Engine/CMakePresets.json`, CMake 辅助文件位于 `Engine/CMake/`, 依赖清单位于 `Engine/vcpkg.json`.

## 编译与测试

从仓库根目录执行:

```sh
cd Engine
cmake --preset Develop
cmake --build Build/Develop
ctest --preset Develop

cmake --preset Release
cmake --build Build/Release
ctest --preset Release
```

启动目标为 `SnowyArkEditor`, 会打开窗口并绘制透视旋转的彩色矩形. 可执行文件旁边需要 `Shaders/Passes/Triangle.spv`, 由 CMake 调用 `slangc` 生成并在构建后复制. 其他构建目标的状态见 [当前状态](../../README.md#当前状态).

`BUILD_TESTING` 控制是否生成 `SnowyArkTests`. test preset 在未发现测试时会报错. 默认测试检查 `AcquiredFrameAction::Decide` 的 Success / Suboptimal / 连续 OutOfDate, `CleanupWait::TryWait` 在成功, `std::exception` 和非 `std::exception` 时不向外抛出, `Window::Create` 拒绝宽或高为 0 以及大于 `INT_MAX` 的初始尺寸 (这些路径不调用 `glfwInit`), 以及 `ShaderLibrary::Load` 在独占探测子目录中拒绝缺失 / 空 / 非 4 字节倍数文件, 并检查同路径再次 Load 会替换内容.

矩阵回归检查行主序旋转, 固定相机的位置变换, 宽高比变化, 近远平面到 0/1 深度的映射, 以及零 extent 和非有限时间的拒绝路径.

具备 Vulkan 1.4 GPU 和桌面窗口环境时, 可启用 GPU 回归 (在 `Engine/` 下执行, Release 同理):

```sh
cmake --preset Develop -DSNOWYARK_GPU_TESTS=ON
cmake --build Build/Develop
ctest --preset Develop
```

`SNOWYARK_GPU_TESTS` 默认关闭, 可用 `-DSNOWYARK_GPU_TESTS=OFF` 恢复. 开启后 CTest 增加 `SnowyArkGpuTests`, 用 `SnowyArkTests --gpu` 运行, 超时为 30 秒. 测试经 GAL 创建暂存上传的顶点与索引缓冲, 检查无效布局, 缓冲混绑, 偏移, 索引范围, 录制状态重置与重复初始化, 并交替提交 UInt16 矩形和 UInt32 三角形, 覆盖帧槽复用及同尺寸 swapchain 重建. 构建测试目标时同步更新其 Shader 副本. 负向用例会输出预期的引擎错误日志; 未捕获异常, 检查失败或 Vulkan validation 消息会使该 CTest 失败. Develop 启用 validation 和同步检查, Release 不启用.

Uniform 用例检查分配大小, 初始数据与写入边界, 布局重复编号和阶段, 集合中的空指针 / 错误用途 / 缺失或未知 binding / 无效范围, 创建管线身份不匹配, 缺失资源的 draw, 以及重新绑定管线后的状态重置. 每帧依据已完成的槽更新独立 UBO, 并通过真实 GPU 提交覆盖复用和重建.

这些测试未注入上传 fence 等待失败或设备丢失, 不覆盖 Application 故障清理, 真实窗口 resize / 最小化恢复, 不支持所需特性的 GPU 或低于 Vulkan 1.4 的 loader. GPU 测试检查提交行为和 validation, 不比较画面像素; 画面和窗口交互仍需启动编辑器另行验证.

复用旧 Ninja 构建树时, 若头文件修改后相关调用方没有重新编译, 先用 `ninja -C Build/Develop -t deps` 检查依赖记录. 旧对象若没有头文件依赖, 运行 `cmake --build Build/Develop --clean-first`, Release 同理, 再运行测试; 新编译的对象应包含相应头文件依赖. 仅重新链接不能修复旧对象的类型布局.

## C++ 格式化

`Engine/Source` 和 `Engine/Tests` 里的 `.cpp` / `.h` 按仓库根目录 `.clang-format` 处理. 配置基于 Microsoft 预设, 语言标准是 `c++20`, 缩进 4 个空格, 列宽 200, Allman 大括号. 第三方代码, vcpkg 安装目录和构建生成文件不在范围内.

当前检查使用 Visual Studio 自带的 clang-format 22.1.3. 它不在 `PATH` 里, 可执行文件是:

```
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe
```

Rider 也捆绑了 clang-format 22, 但版本字符串不是 22.1.3. 如果要和仓库检查使用同一份工具, 把 Rider 的 ClangFormat 指到上面这条路径.

从仓库根目录格式化自有源文件 (PowerShell):

```powershell
$cf = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"
$files = Get-ChildItem Engine\Source, Engine\Tests -Recurse -Include *.cpp, *.h
& $cf -i $files.FullName
```

只检查、不改文件:

```powershell
& $cf --dry-run --Werror $files.FullName
```

`.cpp` 的 include 顺序是: 自身头文件, C / C++ 标准库, 第三方和平台 SDK, 项目的 `Runtime` / `Editor` / `Tools`. 头文件在 `#pragma once` 之后按后三组排列. 组与组之间空一行, 组内按完整路径字母序. `IncludeBlocks: Regroup` 按类别拆组; 自身头文件由 `MainIncludeChar: AngleBracket` 识别并放在最前. 标准库类别写成 C++20 头文件和 C 兼容头的显式名单, 其余 `<>` 归第三方或平台. `#if` / `#define` 仍是边界, GLFW native 头必须紧跟 `GLFW_EXPOSE_NATIVE_WIN32`. `VulkanInclude.h` 里 `windows.h` 必须先于 `vulkan_raii.hpp`, 这两行用局部 `clang-format off` / `on` 固定顺序.

`Cpp11BracedListStyle` 是 `Block`. 声明行 `{` 前面有空格, 非空花括号内侧留空, 空 `{}` 保持紧凑.

短列表不加尾逗号, 保持一行. 这类列表包括颜色分量、`.offset` 的 `{ .x = 0, .y = 0 }`, 以及本来就写成一行、能放进 200 列的 designated init.

需要按字段换行的聚合, 必须在最后一项手写尾逗号. clang-format 22 的 `InsertTrailingCommas` 只支持 JavaScript, 不会给 C++ 自动插入. 没有尾逗号时, 能放进 200 列的列表会被收成一行.

`VulkanCommandBuffer::BeginRendering` 的排版:

```cpp
const vk::ClearColorValue clearValue { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
const vk::RenderingAttachmentInfo colorAttachment {
    .imageView = m_ImageView,
    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp = vk::AttachmentLoadOp::eClear,
    .storeOp = vk::AttachmentStoreOp::eStore,
    .clearValue = clearValue,
};
const vk::RenderingInfo renderingInfo {
    .renderArea = {
        .offset = { .x = 0, .y = 0 },
        .extent = m_Extent,
    },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachment,
};
```

多行列表字段缩进 4, 右 `}` 单独一行. 函数参数超出列宽时仍按 `(` 后对齐. 构造函数初始化列表使用 `PackConstructorInitializers: NextLineOnly`.

## Rider 配置

Rider 支持直接打开, 构建和调试 CMake 工程, 参见 [JetBrains 官方说明](https://blog.jetbrains.com/dotnet/2026/03/20/rider-2026-1-release-candidate/).

1. 打开 `Engine/CMakeLists.txt` 并作为项目加载, 或打开 `Engine/` 文件夹. 仓库根目录没有 CMake 入口.
2. 在 `Settings > Build, Execution, Deployment > Toolchains` 中选择 Visual Studio/MSVC 工具链, 使用 x64/amd64 架构, 并确认 CMake 与 Ninja 可用.
3. 在 `Settings > Build, Execution, Deployment > CMake` 中启用导入的 `Develop` 和 `Release` 配置. 若未自动导入, 通过 `Find Action` 执行 `Load CMake Presets`, 然后重新加载 CMake 项目.
4. 选择 `SnowyArkEditor` 运行目标. 使用 `Develop` 调试, 使用 `Release` 检查优化构建.

各目标会自动收集自身目录内的 `.cpp` 和 `.h`. 如果新增文件后 Rider 尚未显示其所属目标, 执行 `Reload CMake Project`.

## 构建配置与输出

| Preset | CMake build type | 用途 | 构建目录 |
| --- | --- | --- | --- |
| `Develop` | `Debug` | 日常开发和断点调试, 不优化, 保留调试信息 | `Engine/Build/Develop/` |
| `Release` | `Release` | 优化构建, 定义 `NDEBUG` | `Engine/Build/Release/` |

两套配置都使用 Ninja, 并生成 `compile_commands.json`. `Develop` 是项目配置名, 对应 CMake 的标准 `Debug` 类型.

Preset 只提供两项 configure preset 和对应的 test preset. 命令行构建直接指定构建目录, 不使用 build preset, 以免 Rider 同时导入 configure/build 后产生重复配置. 格式见 [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html).

共享配置写入 `Engine/CMakePresets.json`, 不硬编码本机编译器, SDK 或 IDE 安装路径. `VULKAN_SDK` 和 `VCPKG_ROOT` 从环境变量读取. 编译器路径由 Rider 的本机工具链管理. 机器相关的 CMake 设置可以放在 `Engine/CMakeUserPresets.json`, 此文件和 `.idea/` 都由 Git 忽略.

项目的构建边界和依赖清单归属见 [构建与依赖配置](Architecture.md#构建与依赖配置), 其他生成内容的位置见 [生成目录](Architecture.md#生成目录).

## CMake 维护

- Runtime, Editor, ShaderCompiler 和 Tests 各自在目标目录内用 `GLOB_RECURSE CONFIGURE_DEPENDS` 收集 `.cpp` 和 `.h`. 新增普通源文件不需要逐项登记. 收集范围限定在各目标目录内, 不扫描整个 `Source/`, ThirdParty, 资产或构建目录. Slang 源文件在 `Engine/Assets/Shaders/`, 由 `Engine/CMake/Shaders.cmake` 显式登记并调用 `slangc`.
- 生成的源码写入构建目录, 再通过 `target_sources()` 显式加入目标. 临时验证代码不要长期留在目标源码目录中.
- `Runtime/Platform/Windows/` 仅在 Windows 目标平台下参与构建. 新增平台或可选图形后端时补充选择条件. 新增独立程序时创建独立 target, 不把第二个 `main()` 放入已有目标.
- C++20 要求由 `SnowyArkProjectOptions` 声明, Runtime 通过 `PUBLIC` 依赖传递给使用方. Editor 和 Tests 从 Runtime 继承, 独立 ShaderCompiler 直接引用. `SnowyArkWarnings` 只由项目目标私有使用, 不应用到第三方目标. `glfw` 和 `Vulkan::Vulkan` 是 Runtime 的 `PRIVATE` 使用依赖; `SnowyArk` 是静态库, 链接时仍会把它们传给 Editor 和 Tests. 依赖可见性遵循 [CMake target 规则](https://cmake.org/cmake/help/latest/guide/tutorial/In-Depth%20CMake%20Target%20Commands.html).
- 保留上文列出的两份构建配置. 优化参数, 调试信息和 `NDEBUG` 由 CMake 的标准 build type 管理, 不手工覆盖全局编译器 flags.

自动收集是本项目为减少清单维护作出的取舍. [CMake 官方仍推荐显式列出源文件](https://cmake.org/cmake/help/latest/command/file.html#glob), 因为目录扫描有构建开销, 且 `CONFIGURE_DEPENDS` 不保证兼容所有生成器. 更换生成器时需要重新验证源码自动发现; 若扫描成本影响构建速度, 再改用显式清单.

其他主题见 [文档目录](README.md).
