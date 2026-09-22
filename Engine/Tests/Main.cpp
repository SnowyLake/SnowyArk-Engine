#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <Runtime/Core/CleanupWait.h>
#include <Runtime/Core/Log.h>
#include <Runtime/Core/Path.h>
#include <Runtime/GAL/Buffer.h>
#include <Runtime/GAL/CommandBuffer.h>
#include <Runtime/GAL/FrameBegin.h>
#include <Runtime/GAL/GraphicsDevice.h>
#include <Runtime/GAL/PipelineState.h>
#include <Runtime/GAL/SwapChain.h>
#include <Runtime/Platform/Window.h>
#include <Runtime/RenderPipeline/RenderPipeline.h>
#include <Runtime/Shader/ShaderLibrary.h>

namespace SnowyArk
{
struct ShaderProbeDir
{
    std::filesystem::path directory;

    /// Deletes only this exclusively created probe directory under `Shaders/`.
    ~ShaderProbeDir()
    {
        if (directory.empty())
        {
            return;
        }
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
};

class TestRunner
{
public:
    /// Records a failed check without aborting the remaining tests.
    static void Require(bool condition, const char* name);

    /// Returns how many Require checks have failed.
    static int FailureCount();

    /// Writes `byteCount` zero bytes to `path`. Returns false when the file cannot be written.
    static bool WriteShaderFile(const std::filesystem::path& path, std::size_t byteCount);

    /// Creates a new directory under executable `Shaders/` that did not already exist.
    static bool CreateUniqueProbeDir(ShaderProbeDir& probe);

    /// Runs opt-in Vulkan checks through GAL, including uploads, invalid input, frame-slot reuse, and swapchain recreation.
    static void RunGpuChecks();

    /// Checks that a rejected operation reports a runtime error instead of recording invalid GPU work.
    template <typename Operation> static void RequireThrows(Operation operation, const char* name)
    {
        try
        {
            operation();
            Require(false, name);
        }
        catch (const std::runtime_error&)
        {
        }
    }

private:
    static int s_Failures;
    static int s_ProbeSerial;
};

int TestRunner::s_Failures = 0;
int TestRunner::s_ProbeSerial = 0;

void TestRunner::Require(const bool condition, const char* const name)
{
    if (!condition)
    {
        SnowyArk::Log::Error("FAILED: {}", name);
        ++s_Failures;
    }
}

int TestRunner::FailureCount()
{
    return s_Failures;
}

bool TestRunner::WriteShaderFile(const std::filesystem::path& path, const std::size_t byteCount)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        return false;
    }
    if (byteCount == 0)
    {
        return static_cast<bool>(file);
    }

    const std::string bytes(byteCount, '\0');
    file.write(bytes.data(), static_cast<std::streamsize>(byteCount));
    return static_cast<bool>(file);
}

bool TestRunner::CreateUniqueProbeDir(ShaderProbeDir& probe)
{
    const std::filesystem::path shadersRoot = Path::ExecutableDirectory() / "Shaders";
    std::error_code error;
    std::filesystem::create_directories(shadersRoot, error);
    if (error)
    {
        return false;
    }

    for (int attempt = 0; attempt < 64; ++attempt)
    {
        const std::string name = "probe-" + std::to_string(++s_ProbeSerial) + "-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        std::filesystem::path dir = shadersRoot / name;
        if (std::filesystem::create_directory(dir, error) && !error)
        {
            probe.directory = std::move(dir);
            return true;
        }
    }
    return false;
}

/// Thrown to cover cleanup-wait handling of exceptions that are not std::exception.
struct NonStdThrow
{
};

struct CleanupOwner
{
    bool* released = nullptr;

    /// Continues local cleanup after a failed wait; does not release Application GPU objects.
    ~CleanupOwner()
    {
        SnowyArk::CleanupWait::TryWait([] { throw std::runtime_error("DeviceLost"); });
        if (released != nullptr)
        {
            *released = true;
        }
    }
};

struct GpuCleanupWait
{
    GraphicsDevice& device;

    /// Drains submitted work before test resources unwind, including on a failed check.
    ~GpuCleanupWait()
    {
        CleanupWait::TryWait([this] { device.WaitIdle(); });
    }
};

void TestRunner::RunGpuChecks()
{
    Window window;
    if (!window.Create({ .width = 320, .height = 240, .title = "SnowyArk GPU regression" }))
    {
        Require(false, "GPU test window creation");
        return;
    }
    auto device = GraphicsDevice::Create(GraphicsBackend::Vulkan);
    if (!device->Initialize(window.GetNativeHandle(), window.GetInstanceExtensions()))
    {
        Require(false, "GPU test device initialization");
        return;
    }
    auto swapChain = device->CreateSwapChain(window);
    ShaderLibrary shaders;
    if (!shaders.Load("Passes/Triangle.spv"))
    {
        Require(false, "GPU test shader load");
        return;
    }
    RenderPipeline renderPipeline;
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    std::unique_ptr<PipelineState> pipeline;
    GpuCleanupWait cleanup { *device };
    const bool initialized = renderPipeline.Initialize(*device, shaders, *swapChain);
    Require(initialized, "Rectangle initializes");
    if (!initialized)
    {
        return;
    }
    Require(!renderPipeline.Initialize(*device, shaders, *swapChain), "Repeated initialization preserves live resources");

    constexpr float vertices[] { -0.5f, -0.5f, 1, 0, 0, 0.5f, -0.5f, 0, 1, 0, 0, 0.5f, 0, 0, 1 };
    constexpr uint32_t indices[] { 0, 1, 2 };
    const auto vertexBytes = std::as_bytes(std::span { vertices });
    vertexBuffer = device->CreateBuffer({ .usage = BufferUsage::Vertex, .initialData = vertexBytes });
    indexBuffer = device->CreateBuffer({ .usage = BufferUsage::Index, .initialData = std::as_bytes(std::span { indices }) });
    Require(vertexBuffer->GetSize() == sizeof(vertices) && vertexBuffer->GetUsage() == BufferUsage::Vertex, "Buffer preserves size and usage");
    Require(device->CreateBuffer({}) == nullptr, "Empty upload is rejected");
    RequireThrows([&] { device->CreateBuffer({ .usage = static_cast<BufferUsage>(-1), .initialData = vertexBytes }); }, "Invalid usage is rejected");

    VertexBinding bindings[] { { .binding = 0, .stride = 20 }, { .binding = 0, .stride = 20 } };
    VertexAttribute attributes[] { { .location = 0, .binding = 0, .format = Format::R32G32Sfloat }, { .location = 1, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = 8 } };
    GraphicsPipelineDesc desc { .shaderSpirv = shaders.GetSpirv("Passes/Triangle.spv"), .colorFormat = swapChain->GetColorFormat(), .vertexBindings = bindings, .vertexAttributes = attributes };
    RequireThrows([&] { device->CreateGraphicsPipeline(desc); }, "Duplicate vertex bindings are rejected");
    desc.vertexBindings = std::span { bindings }.first(1);
    attributes[1].location = 0;
    RequireThrows([&] { device->CreateGraphicsPipeline(desc); }, "Duplicate attribute locations are rejected");
    attributes[1].location = 1;
    attributes[1].binding = 1;
    RequireThrows([&] { device->CreateGraphicsPipeline(desc); }, "Missing attribute binding is rejected");
    attributes[1].binding = 0;
    bindings[0].stride = UINT32_MAX;
    RequireThrows([&] { device->CreateGraphicsPipeline(desc); }, "Unsupported stride is rejected");
    bindings[0].stride = 20;
    pipeline = device->CreateGraphicsPipeline(desc);

    uint32_t rendered = 0;
    for (uint32_t attempt = 0; attempt < 32 && rendered < 6; ++attempt)
    {
        window.PumpEvents();
        auto* commandBuffer = device->BeginFrame(*swapChain);
        if (commandBuffer == nullptr)
        {
            continue;
        }
        RequireThrows([&] { commandBuffer->DrawIndexed(3, 1); }, "Index binding does not leak across recordings");
        RequireThrows([&] { commandBuffer->SetVertexBuffer(*indexBuffer); }, "Index buffer cannot bind as vertex input");
        RequireThrows([&] { commandBuffer->SetVertexBuffer(*vertexBuffer, UINT32_MAX); }, "Unsupported vertex binding is rejected");
        RequireThrows([&] { commandBuffer->SetVertexBuffer(*vertexBuffer, 0, vertexBuffer->GetSize()); }, "End-of-buffer vertex offset is rejected");
        RequireThrows([&] { commandBuffer->SetIndexBuffer(*vertexBuffer, IndexType::UInt16); }, "Vertex buffer cannot bind as index input");
        RequireThrows([&] { commandBuffer->SetIndexBuffer(*indexBuffer, IndexType::UInt16, 1); }, "Misaligned UInt16 offset is rejected");
        RequireThrows([&] { commandBuffer->SetIndexBuffer(*indexBuffer, IndexType::UInt32, 2); }, "Misaligned UInt32 offset is rejected");
        RequireThrows([&] { commandBuffer->SetIndexBuffer(*indexBuffer, static_cast<IndexType>(-1)); }, "Invalid index type is rejected");
        commandBuffer->SetIndexBuffer(*indexBuffer, IndexType::UInt32, sizeof(uint32_t));
        RequireThrows([&] { commandBuffer->DrawIndexed(3, 1); }, "Index range honors binding offset");

        if (rendered % 2 == 0)
        {
            renderPipeline.Render(*commandBuffer, *swapChain);
        }
        else
        {
            const auto extent = swapChain->GetExtent();
            commandBuffer->TransitionToColorTarget();
            commandBuffer->BeginRendering({ .r = 0.08f, .g = 0.08f, .b = 0.10f, .a = 1 });
            commandBuffer->SetPipeline(*pipeline);
            commandBuffer->SetViewport({ .width = static_cast<float>(extent.width), .height = static_cast<float>(extent.height) });
            commandBuffer->SetScissor({ .width = extent.width, .height = extent.height });
            commandBuffer->SetVertexBuffer(*vertexBuffer);
            commandBuffer->SetIndexBuffer(*indexBuffer, IndexType::UInt32);
            commandBuffer->DrawIndexed(3, 1);
            commandBuffer->EndRendering();
            commandBuffer->TransitionToPresent();
        }
        device->EndFrame(*swapChain);
        ++rendered;
        if (rendered == 3)
        {
            swapChain->Resize(window.GetWidth(), window.GetHeight());
        }
    }
    Require(rendered == 6, "UInt16 and UInt32 draws survive frame-slot reuse and swapchain recreation");
    device->WaitIdle();
    renderPipeline.Shutdown();
    Require(renderPipeline.Initialize(*device, shaders, *swapChain), "Render pipeline reinitializes after shutdown");
}
}

/// Runs the SnowyArk test executable.
int main(const int argc, char* argv[])
{
    using namespace SnowyArk;

    if (argc == 2 && std::string_view(argv[1]) == "--gpu")
    {
        try
        {
            TestRunner::RunGpuChecks();
        }
        catch (const std::exception& exception)
        {
            Log::Error(exception.what());
            TestRunner::Require(false, "GPU regression completed without unexpected exceptions");
        }
        return TestRunner::FailureCount() == 0 ? 0 : 1;
    }

    const AcquiredFrameAction success = AcquiredFrameAction::Decide(SwapChainAcquireStatus::Success, 2);
    TestRunner::Require(success.recordAndPresent && !success.requestRecreation && success.imageIndex == 2, "Success acquire records the image");

    const AcquiredFrameAction suboptimal = AcquiredFrameAction::Decide(SwapChainAcquireStatus::Suboptimal, 1);
    TestRunner::Require(suboptimal.recordAndPresent && suboptimal.requestRecreation && suboptimal.imageIndex == 1, "Suboptimal acquire completes the frame and defers recreation");

    const AcquiredFrameAction firstOutOfDate = AcquiredFrameAction::Decide(SwapChainAcquireStatus::OutOfDate, 3);
    const AcquiredFrameAction secondOutOfDate = AcquiredFrameAction::Decide(SwapChainAcquireStatus::OutOfDate, 3);
    TestRunner::Require(!firstOutOfDate.recordAndPresent && firstOutOfDate.requestRecreation && firstOutOfDate.imageIndex == 0, "OutOfDate skips recording without using a placeholder image index");
    TestRunner::Require(!secondOutOfDate.recordAndPresent && secondOutOfDate.requestRecreation && secondOutOfDate.imageIndex == 0,
                        "Consecutive OutOfDate results keep skipping instead of retrying acquire");

    bool waitRan = false;
    TestRunner::Require(CleanupWait::TryWait([&] { waitRan = true; }), "Successful cleanup wait returns true");
    TestRunner::Require(waitRan, "Successful cleanup wait invokes the wait");

    bool stdWaitPropagated = false;
    bool continuedAfterStdWait = false;
    try
    {
        TestRunner::Require(!CleanupWait::TryWait([] { throw std::runtime_error("DeviceLost"); }), "std::exception cleanup wait returns false");
        continuedAfterStdWait = true;
    }
    catch (...)
    {
        stdWaitPropagated = true;
    }
    TestRunner::Require(!stdWaitPropagated && continuedAfterStdWait, "Caller continues after a std::exception cleanup wait");

    bool nonStdWaitPropagated = false;
    bool continuedAfterNonStdWait = false;
    try
    {
        TestRunner::Require(!CleanupWait::TryWait([] { throw NonStdThrow {}; }), "Non-std cleanup wait returns false");
        continuedAfterNonStdWait = true;
    }
    catch (...)
    {
        nonStdWaitPropagated = true;
    }
    TestRunner::Require(!nonStdWaitPropagated && continuedAfterNonStdWait, "Caller continues after a non-std cleanup wait");

    bool destructorReleased = false;
    bool destructorPropagated = false;
    try
    {
        CleanupOwner owner;
        owner.released = &destructorReleased;
    }
    catch (...)
    {
        destructorPropagated = true;
    }
    TestRunner::Require(destructorReleased && !destructorPropagated, "Destructor continues after a failed cleanup wait");

    Window zeroWidth;
    WindowDesc zeroWidthDesc;
    zeroWidthDesc.width = 0;
    zeroWidthDesc.height = 720;
    TestRunner::Require(!zeroWidth.Create(zeroWidthDesc), "Zero width is rejected without glfwInit");

    Window zeroHeight;
    WindowDesc zeroHeightDesc;
    zeroHeightDesc.width = 1280;
    zeroHeightDesc.height = 0;
    TestRunner::Require(!zeroHeight.Create(zeroHeightDesc), "Zero height is rejected without glfwInit");

    Window tooWide;
    WindowDesc tooWideDesc;
    tooWideDesc.width = static_cast<uint32_t>((std::numeric_limits<int>::max)()) + 1u;
    tooWideDesc.height = 720;
    TestRunner::Require(!tooWide.Create(tooWideDesc), "Width greater than INT_MAX is rejected without glfwInit");

    ShaderProbeDir probe;
    TestRunner::Require(TestRunner::CreateUniqueProbeDir(probe), "Exclusive shader probe directory can be created");
    if (!probe.directory.empty())
    {
        const std::string prefix = probe.directory.filename().string();
        ShaderLibrary shaders;
        TestRunner::Require(!shaders.Load((prefix + "/missing.spv").c_str()), "Missing SPIR-V returns false");
        TestRunner::Require(TestRunner::WriteShaderFile(probe.directory / "empty.spv", 0), "Empty shader probe can be written");
        TestRunner::Require(!shaders.Load((prefix + "/empty.spv").c_str()), "Empty SPIR-V returns false");
        TestRunner::Require(TestRunner::WriteShaderFile(probe.directory / "odd.spv", 3), "Odd-sized shader probe can be written");
        TestRunner::Require(!shaders.Load((prefix + "/odd.spv").c_str()), "Odd-sized SPIR-V returns false");

        const std::string wordRel = prefix + "/word.spv";
        TestRunner::Require(TestRunner::WriteShaderFile(probe.directory / "word.spv", 4), "4-byte shader probe can be written");
        TestRunner::Require(shaders.Load(wordRel.c_str()), "4-byte SPIR-V size is accepted");
        const std::vector<uint32_t>& spirv = shaders.GetSpirv(wordRel.c_str());
        TestRunner::Require(spirv.size() == 1, "4-byte SPIR-V has one word");
        TestRunner::Require(TestRunner::WriteShaderFile(probe.directory / "word.spv", 8), "8-byte shader probe can be written");
        TestRunner::Require(shaders.Load(wordRel.c_str()), "Same-key reload is accepted");
        TestRunner::Require(spirv.size() == 2, "Same-key Load replaces vector contents");
    }

    if (TestRunner::FailureCount() > 0)
    {
        Log::Error("{} test(s) failed.", TestRunner::FailureCount());
        return 1;
    }

    Log::Info("SnowyArk frame, cleanup, window, device-feature, and shader load regression checks passed.");
    return 0;
}
