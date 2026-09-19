#include <Runtime/Shader/ShaderLibrary.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <utility>

#include <Runtime/Core/Log.h>
#include <Runtime/Core/Path.h>

namespace SnowyArk
{

bool ShaderLibrary::Load(const char* const relativePath)
{
    const std::filesystem::path path = Path::ExecutableDirectory() / "Shaders" / relativePath;
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        Log::Error("Failed to open shader '{}'.", path.string());
        return false;
    }

    const std::streampos endPos = file.tellg();
    if (!file || endPos < std::streampos { 0 })
    {
        Log::Error("Failed to query shader '{}' size.", path.string());
        return false;
    }

    const auto byteCountSigned = static_cast<std::streamoff>(endPos);
    if (byteCountSigned <= 0)
    {
        Log::Error("Shader '{}' has an invalid SPIR-V size ({} bytes).", path.string(), byteCountSigned);
        return false;
    }

    const auto byteCount = static_cast<std::uintmax_t>(byteCountSigned);
    if ((byteCount % sizeof(uint32_t)) != 0 || byteCount > std::numeric_limits<std::size_t>::max() || byteCount > static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()))
    {
        Log::Error("Shader '{}' has an invalid SPIR-V size ({} bytes).", path.string(), byteCount);
        return false;
    }

    const auto fileSize = static_cast<std::size_t>(byteCount);
    std::vector<uint32_t> spirv(fileSize / sizeof(uint32_t));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(spirv.data()), static_cast<std::streamsize>(fileSize));
    if (!file)
    {
        Log::Error("Failed to read shader '{}'.", path.string());
        return false;
    }

    m_Shaders[relativePath] = std::move(spirv);
    Log::Info("Loaded shader '{}' ({} bytes).", relativePath, fileSize);
    return true;
}

const std::vector<uint32_t>& ShaderLibrary::GetSpirv(const char* const relativePath) const
{
    const auto it = m_Shaders.find(relativePath);
    if (it == m_Shaders.end())
    {
        Log::Fatal("Shader '{}' has not been loaded.", relativePath);
    }
    return it->second;
}

}
