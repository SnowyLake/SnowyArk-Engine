#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"

#define ENGINE_PATH(localPath) g_EngineRootPath + localPath

namespace Snowy::Ark
{
extern std::string g_EngineRootPath;

class FileSystem final : public TSingleton<FileSystem>
{
private:
    friend class TSingleton<FileSystem>;
    FileSystem();
    ~FileSystem() = default;

public:
    std::vector<char> ReadSpirvShaderBinary(AnsiStringIn path);
};
}
