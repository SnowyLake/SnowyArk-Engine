#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Rendering/Interface/RHITexture.h"
#include <filesystem>

namespace Snowy::Ark
{
class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = default;
    AssetManager(AssetManager&&) = default;
    AssetManager& operator=(const AssetManager&) = default;
    AssetManager& operator=(AssetManager&&) = default;

    void Init();
    void Destory();

public:
    std::vector<char> LoadSpirvShaderBinary(std::filesystem::path path);
    UniqueHandle<TextureData> LoadTexture(std::filesystem::path path);
    void LoadModel(std::filesystem::path path);


public:
    static inline std::string EngineRootPath;
};

#define SA_ENGINE_PATH(localPath) AssetManager::EngineRootPath + localPath
}
