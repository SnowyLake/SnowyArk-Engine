#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
namespace Snowy::Ark
{
class FileSystem final : public Singleton<FileSystem>
{
private:
    friend class Singleton<FileSystem>;
    FileSystem();
    ~FileSystem() = default;

public:
    inline static std::string EngineRootPath;

public:
    std::string FullPath(const std::string& filePath);
    std::vector<char> ReadSpirvShaderBinary(const std::string& filePath);
};
}
