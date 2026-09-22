# 第一个三角形相关决策

## 目录

- [窗口使用 GLFW](#窗口使用-glfw)
- [GAL 使用抽象基类](#gal-使用抽象基类)
- [Shader 使用 Slang](#shader-使用-slang)
- [绘制使用 Dynamic Rendering](#绘制使用-dynamic-rendering)
- [Swapchain 只接受 R8G8B8A8Srgb](#swapchain-只接受-r8g8b8a8srgb)

## 窗口使用 GLFW

窗口和事件循环使用 GLFW, 与 [Vulkan Tutorial](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html) 对齐. 实现放在 `Runtime/Platform/Windows`, 公共头 `Window.h` 不包含 GLFW.

Surface 仍由 Vulkan 后端用 HWND 创建, 避免 `Platform` 依赖 Vulkan 头, 也避免 GAL 公共接口认识 GLFW. 实例扩展名由 `glfwGetRequiredInstanceExtensions` 提供.

可绘制时用 `glfwPollEvents`. framebuffer 为零时, 外层循环调用 `Window::WaitEvents` (`glfwWaitEvents`), 等关闭或尺寸恢复后再继续, 避免空转. `Application::Tick` 不在内部 sleep 或等待事件.

GLFW 通过 vcpkg 的 `glfw3` 包获取, 不 vendor 进 `ThirdParty/`.

## GAL 使用抽象基类

GAL 按小型 RHI 设计: `GraphicsDevice`, `SwapChain`, `CommandBuffer`, `PipelineState` 是抽象基类, `GraphicsDevice::Create(GraphicsBackend)` 返回后端实现. 目的是以后能接 OpenGL / D3D, 而不是现在实现这些后端.

不采用的方案:

- 把 Vulkan 对象写进 `EditorApplication` 或教程式的单文件 class
- CRTP 或编译期 `typedef` 成唯一后端
- UE 那种延迟命令列表和 RHI 线程
- 现在就建空的 `GAL/OpenGL` 或 `GAL/D3D12`

录制入口叫 `CommandBuffer`, 对应 Vulkan / Metal 的命名, 也方便以后映射 D3D12 的 command list. 索引矩形 Pass 放在 `RenderPipeline`, 只调用这些虚接口.

## Shader 使用 Slang

Shader 是 `Engine/Assets/Shaders/Passes/Triangle.slang`, 入口 `MainVertex` 和 `MainFragment`, 由 SDK 中的 `slangc` 编成一份 SPIR-V. 编译参数与教程相同: `-target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name`. 顶点着色器从 location 0 读取位置, 从 location 1 读取颜色. 数据放在顶点缓冲和索引缓冲里, 见 [顶点缓冲](VertexBuffers.md).

`SnowyArkShaderCompiler` 仍是空入口. 运行时 `ShaderLibrary` 只读可执行文件旁的 SPIR-V.

## 绘制使用 Dynamic Rendering

不创建 `VkRenderPass` 和 framebuffer. 图形管线带 `vk::PipelineRenderingCreateInfo`, 每帧 `beginRendering` / `endRendering`. 这与现行教程一致, 也更接近以后要做的多 pass.

Instance 请求 Vulkan 1.4. 选设备时先丢掉 `apiVersion` 低于 1.4 的 GPU, 再用 `PhysicalDeviceFeatures2` 链查询 `dynamicRendering` 和 `synchronization2`; 缺一项就看下一张卡. 逻辑设备启用这两项. 从 present 到 color attachment 的 layout transition 使用 `ColorAttachmentOutput` 作为 source stage, 与 acquire semaphore 的 wait stage 对齐.

## Swapchain 只接受 R8G8B8A8Srgb

本阶段不实现格式回退. 选设备和创建 swapchain 时都要求 `R8G8B8A8Srgb` 加 `SrgbNonlinear`; 表面不提供该组合时直接报错, 不会改用 `formats.front()` 或其他 GAL 未映射的格式.
