# Uniform 缓冲

## 目录

- [GAL 接口](#gal-接口)
- [帧槽与生命周期](#帧槽与生命周期)
- [矩阵与画面](#矩阵与画面)
- [当前范围](#当前范围)

## GAL 接口

本阶段对应 Vulkan 教程的 [Descriptor layout and buffer](https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html) 和 [Descriptor pool and sets](https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html). 示例更新模型, 观察和投影矩阵, GAL 负责通用的缓冲和 Shader 资源绑定.

`BufferUsage::Uniform` 沿用 `GraphicsDevice::CreateBuffer`. `BufferDesc::size` 指定非零字节数, 当前不超过设备的 `maxUniformBufferRange`; `initialData` 可省略, 未提供的字节清零. `Buffer::Write` 接收字节 span 和偏移, 检查用途与范围. Vulkan 使用 host-visible, host-coherent 内存并持续映射, 不创建暂存缓冲. CPU 写入在提交前完成, coherent 内存不需要手动 flush. Vertex 和 Index 仍走原有同步上传路径, 不允许 `Write`.

`GraphicsPipelineDesc::uniformBindings` 声明 set 0 中各 binding 的编号和可见阶段, 支持 Vertex, Fragment 或两者. 后端检查重复编号和设备限制, 并将声明复制到管线. 空布局可用于不读取 Uniform 的 Shader. 布局与 Shader 的 binding, 类型, 阶段和实际读取范围必须一致, 当前没有自动反射来校验这一对应关系.

`GraphicsDevice::CreateResourceSet` 接收管线和 `UniformBufferBinding` 列表. 每项绑定一段 Uniform 缓冲, 必须完整匹配布局, 不允许遗漏或重复. 缓冲必须来自同一设备, size 非零且范围有效, offset 满足设备的 Uniform 偏移对齐要求. 创建后绑定关系不可修改, 缓冲内容仍可在同步条件满足时更新.

`CommandBuffer::SetPipeline` 后调用 `SetResourceSet`, 再绘制. 当前只接受由同一个管线实例创建的集合, 即使另一管线的布局相同也不共享. 切换或重复绑定管线会清空集合绑定状态, 新帧也清空状态. `Draw` 与 `DrawIndexed` 在缺少声明的资源绑定时抛出异常. descriptor set layout, pool 和 set 均由 Vulkan 后端管理, 不出现在公开 GAL 类型中.

## 帧槽与生命周期

`GraphicsDevice::GetFrameCount` 返回固定的在途帧槽数. `BeginFrame` 成功后, `CommandBuffer::GetFrameIndex` 返回已等待前次提交完成的槽号, 它不是 swapchain image index. 槽号只在本次有效录制期间使用, 不能跨 `EndFrame`, 下一次 `BeginFrame`, swapchain 重建或关闭保存为写入许可.

`RenderPipeline` 初始化时为每个槽创建一个 Uniform 缓冲和一个资源集合. 每次成功取图后, 只更新当前槽的矩阵并绑定其集合. 当前 pass 每帧调用一次 `Render`; 如果同帧多次绘制需要不同常量, 必须使用不同缓冲区域或不同缓冲, 不能在录制后改写先前 draw 将读取的字节. 通用 `Buffer::Write` 不自动等待 GPU, 调用方负责同步; 当前槽的 fence 也不能保护在其他槽中共享使用的同一缓冲.

常规帧只计算矩阵, 拷贝 192 字节并绑定已有集合, 不增加动态分配, descriptor 分配或 GPU 等待. swapchain 重建不重建 UBO 和描述符; 恢复取图后依据新 extent 计算投影. 零尺寸和取图失败沿用原来的跳帧流程.

资源集合借用管线和缓冲, 三者都要存活到 GPU 使用结束. 关闭时先等待 GPU, 然后释放集合, Uniform 缓冲和管线, 最后释放设备. 后端集合先释放 descriptor set 再释放 pool; 管线先释放 pipeline 再释放 pipeline layout 和 descriptor set layout. 初始化中的局部 RAII 对象也遵循此顺序. 设备等待失败仍沿用既有的故障清理路径.

## 矩阵与画面

`RenderTransforms` 属于 `RenderPipeline`, 不属于 GAL. 它保存三个行主序 4x4 矩阵, 总大小为 192 字节, 字节偏移为 0, 64, 128. Slang 显式写 `row_major`, 以 `mul(projection, mul(view, mul(model, position)))` 变换顶点. 当前 Slang 编译器在 SPIR-V 中用转置的矩阵表示, 因此输出是 `ColMajor` 装饰配合 `OpVectorTimesMatrix`; 不能只看装饰名称判断 CPU 布局. 矩阵 stride 为 16 字节.

模型绕 Z 轴每秒转 90 度, 时间来自 `steady_clock`. 固定相机位于 `(2, 2, 2)`, 看向原点, Z 轴向上. 透视使用 45 度垂直视角, 近远平面为 0.1 和 10, 深度范围为 0 到 1, 宽高比取当前 swapchain extent. 这些小型矩阵构造只用于本 pass, 没有引入通用数学库或相机系统.

视口取 `y = height`, `height = -height` 翻转 Y, 投影矩阵保持右手形式. 矩形使用 `FrontFace::CounterClockwise`, 与负高度视口对应; GAL 的 `GraphicsPipelineDesc::frontFace` 默认仍为 Clockwise, 旧的正高度视口调用方不受影响. 顶点和索引布局见 [顶点缓冲](VertexBuffers.md).

## 当前范围

当前只有 set 0 和非数组 Uniform binding, 每个不可变资源集合独占一个 descriptor pool. 这足够覆盖当前少量帧资源, 材质数量使 pool 创建成本明显时再共享池. 多 set, 动态偏移, 描述符数组, 纹理和采样器, Shader 反射以及跨管线布局共享均未实现.

CPU 测试检查旋转, 相机位置, 投影宽高比和深度范围. GPU 回归检查缓冲写入, 集合匹配, 绑定状态重置, 帧槽复用和 swapchain 重建. 命令及验证边界见 [编译与测试](../Building.md#编译与测试).
