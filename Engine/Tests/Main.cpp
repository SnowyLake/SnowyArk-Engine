#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <Runtime/Core/CleanupWait.h>
#include <Runtime/Core/Log.h>
#include <Runtime/Core/Path.h>
#include <Runtime/GAL/FrameBegin.h>
#include <Runtime/Platform/Window.h>
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
}

/// Runs the SnowyArk test executable.
int main()
{
    using namespace SnowyArk;

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
