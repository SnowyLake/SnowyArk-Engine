#include "FileSystem.h"
namespace Snowy::Ark
{
std::string currentPath = std::filesystem::current_path().generic_string();
extern std::string g_EngineRootPath = currentPath.substr(0, currentPath.rfind("/Engine/") + 1);

FileSystem::FileSystem()
{

}

std::vector<char> FileSystem::ReadSpirvShaderBinary(AStringIn path)
{
    std::ifstream file(path.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open file: {}", path));
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
}
