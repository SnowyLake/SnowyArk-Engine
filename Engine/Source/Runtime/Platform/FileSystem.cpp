#include "FileSystem.h"
namespace Snowy::Ark
{
FileSystem::FileSystem()
{
    auto path = std::filesystem::current_path().generic_string();
    EngineRootPath = path.substr(0, path.rfind("/Engine/") + 1);
}

std::string FileSystem::FullPath(const std::string& filePath)
{
    return EngineRootPath + filePath;
}

std::vector<char> FileSystem::ReadSpirvShaderBinary(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open file: {}", filePath));
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
}
