# SnowyArk 构建与运行

## 目录

- [环境准备](#环境准备)
- [编译与测试](#编译与测试)
- [Rider 配置](#rider-配置)
- [构建配置与输出](#构建配置与输出)
- [CMake 维护](#cmake-维护)

## 环境准备

构建需要 CMake 3.24 或更高版本, Ninja 和支持 C++20 的编译器. Windows 开发使用 MSVC x64 工具链. 在命令行构建前, 打开已初始化编译器环境的 MSVC x64 开发者终端.

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

启动目标为 `SnowyArkEditor`, 当前只输出 `Hello SnowyArk Engine!` 后退出. 其他构建目标的状态见 [当前状态](../../README.md#当前状态).

`BUILD_TESTING` 控制是否生成 `SnowyArkTests`. test preset 在未发现测试时会报错. 当前测试只有空入口, 通过 CTest 只能说明测试程序能够运行.

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

共享配置写入 `Engine/CMakePresets.json`, 不硬编码本机编译器, SDK 或 IDE 安装路径. 编译器和 SDK 路径由 Rider 的本机工具链管理. 机器相关的 CMake 设置可以放在 `Engine/CMakeUserPresets.json`, 此文件和 `.idea/` 都由 Git 忽略.

项目的构建边界和依赖清单归属见 [构建与依赖配置](Architecture.md#构建与依赖配置), 其他生成内容的位置见 [生成目录](Architecture.md#生成目录).

## CMake 维护

- Runtime, Editor, ShaderCompiler 和 Tests 各自在目标目录内用 `GLOB_RECURSE CONFIGURE_DEPENDS` 收集 `.cpp` 和 `.h`. 新增普通源文件不需要逐项登记. 收集范围限定在各目标目录内, 不扫描整个 `Source/`, ThirdParty, 资产或构建目录.
- 生成的源码写入构建目录, 再通过 `target_sources()` 显式加入目标. 临时验证代码不要长期留在目标源码目录中.
- `Runtime/Platform/Windows/` 仅在 Windows 目标平台下参与构建. 新增平台或可选图形后端时补充选择条件. 新增独立程序时创建独立 target, 不把第二个 `main()` 放入已有目标.
- C++20 要求由 `SnowyArkProjectOptions` 声明, Runtime 通过 `PUBLIC` 依赖传递给使用方. Editor 和 Tests 从 Runtime 继承, 独立 ShaderCompiler 直接引用. `SnowyArkWarnings` 只由项目目标私有使用, 不应用到第三方目标. 依赖可见性遵循 [CMake target 规则](https://cmake.org/cmake/help/latest/guide/tutorial/In-Depth%20CMake%20Target%20Commands.html).
- 保留上文列出的两份构建配置. 优化参数, 调试信息和 `NDEBUG` 由 CMake 的标准 build type 管理, 不手工覆盖全局编译器 flags.

自动收集是本项目为减少清单维护作出的取舍. [CMake 官方仍推荐显式列出源文件](https://cmake.org/cmake/help/latest/command/file.html#glob), 因为目录扫描有构建开销, 且 `CONFIGURE_DEPENDS` 不保证兼容所有生成器. 更换生成器时需要重新验证源码自动发现; 若扫描成本影响构建速度, 再改用显式清单.

其他主题见 [文档目录](README.md).
