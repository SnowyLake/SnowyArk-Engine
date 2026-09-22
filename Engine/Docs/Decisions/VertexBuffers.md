# 顶点缓冲

## 目录

- [上传由设备完成](#上传由设备完成)
- [顶点布局写在管线描述里](#顶点布局写在管线描述里)
- [缓冲绑定与绘制范围](#缓冲绑定与绘制范围)
- [每个缓冲单独分配内存](#每个缓冲单独分配内存)
- [矩形示例](#矩形示例)

## 上传由设备完成

`GraphicsDevice::CreateBuffer` 接收 `BufferUsage` 和初始字节. `BufferUsage` 目前有 `Vertex` 和 `Index`. Vulkan 后端把这些字节写入 host-visible 且 host-coherent 的暂存缓冲, 再拷到 device-local 缓冲. 拷贝提交到图形队列, 调用返回前等待这次提交的 fence. 图形队列族包含 transfer, 缓冲使用 exclusive sharing. 暂存缓冲留在 Vulkan 后端.

GAL 约定调用成功返回时初始数据已可用于绘制, 不暴露队列和 fence. Vulkan 后端用同步屏障把 transfer write 对后续 vertex attribute / index read 可见. 空数据返回空指针, 无效 usage 和后端故障抛出异常. 上传提交后若 fence 等待失败, 在局部资源析构前尽力等待设备空闲, 再传播异常进入应用关闭路径; 设备丢失时仍按既定策略清理, 不继续绘制.

`initialData` 只在这次调用期间借用. 返回的 `Buffer` 必须在创建它的 `GraphicsDevice` 销毁之前释放, 并且释放前 GPU 对这块缓冲的使用已经结束. `Application::Shutdown` 先 `WaitIdle`, 再由 `RenderPipeline::Shutdown` 释放缓冲和管线, 此时设备仍然存活.

这次等待发生在创建缓冲时, 也可能受图形队列中此前提交的工作影响. 当前矩形的两个缓冲在初始化时创建, 不在每帧路径上增加分配或等待. 当前接口用于静态数据初始化; 出现运行时批量上传或动态更新需求时, 再增加批量上传与完成状态管理.

## 顶点布局写在管线描述里

`GraphicsPipelineDesc` 的 `vertexBindings` 和 `vertexAttributes` 只在 `CreateGraphicsPipeline` 期间借用. 矩形使用一个 per-vertex binding, stride 是 20 字节. location 0 是 `R32G32Sfloat` 位置, location 1 是 `R32G32B32Sfloat` 颜色, 与 `Triangle.slang` 的 `[[vk::location]]` 一致.

创建管线时检查 binding / location 唯一性, 属性引用的 binding 是否存在, input rate 是否有效, 以及设备对数量, 编号, stride, offset 和顶点格式的支持. 不符合时抛出异常. Shader 输入与布局的对应关系仍由调用方保证, 当前没有 Shader 反射.

## 缓冲绑定与绘制范围

`Buffer` 保存后端无关的字节大小和用途, 通过 `GetSize` / `GetUsage` 查询. Vulkan 绑定入口检查所属设备, 用途, binding 上限和偏移范围. 顶点缓冲不能作为索引缓冲使用, 反向也不允许. 索引偏移必须按 `UInt16` / `UInt32` 的 2 / 4 字节对齐, 且至少剩余一个完整索引. 这些约束对应 [Vulkan 索引缓冲绑定要求](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdBindIndexBuffer.html).

`DrawIndexed` 检查当前录制是否已绑定索引缓冲, 并确保索引数量不超过绑定偏移之后的可用范围. 每次开始新帧录制时清空该状态. 这些检查不读取索引内容, 调用方仍须保证索引值和实例访问不会超出顶点缓冲. 无效参数抛出异常, 不录制对应的 Vulkan 命令.

## 每个缓冲单独分配内存

每个 `Buffer` 对应一次 `vkAllocateMemory`. `maxMemoryAllocationCount` 的下限是 4096. 缓冲数量上去之后再改为子分配. 当前只有一块顶点缓冲和一块索引缓冲.

## 矩形示例

`RenderPipeline` 保存 4 个交错顶点 (`float2` 位置, `float3` 颜色) 和 6 个 `uint16` 索引 `0, 1, 2, 2, 3, 0`. 录制时调用 `SetVertexBuffer`, `SetIndexBuffer` 和 `DrawIndexed`. 绑定之后, 缓冲要活到该帧 GPU 工作结束. 帧槽的 in-flight fence 在 `EndFrame` 提交时使用, 下一次占用同一帧槽的 `BeginFrame` 会等待它. 释放缓冲前应等待所有相关提交完成; 正常关闭通过设备 `WaitIdle` 完成等待, 故障清理规则见上文.

`RenderPipeline::Initialize` 在已持有资源时返回 false, 避免替换仍被 GPU 使用的对象. 重新初始化前, 调用方先完成 GPU 等待, 再调用 `Shutdown`. GPU 回归覆盖范围和运行命令见 [编译与测试](../Building.md#编译与测试).

视口高度为正, framebuffer 的 y 向下, NDC 的 y=-1 在窗口上方. 左上 `(-0.5, -0.5)` 是红, 右上 `(0.5, -0.5)` 是绿, 右下 `(0.5, 0.5)` 是蓝, 左下 `(-0.5, 0.5)` 是白.

不采用的方案:

- 把 `VkBuffer` 和内存类型写进 `RenderPipeline`
- 让 Pass 创建暂存缓冲并自己录制拷贝
- 单独的传输队列, 以及 concurrent sharing
- 现在引入 VulkanMemoryAllocator
