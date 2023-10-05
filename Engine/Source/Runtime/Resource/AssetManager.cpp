#include "AssetManager.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include<stb/stb_image.h>
//#define TINYOBJLOADER_IMPLEMENTATION
//#include<tinyobjloader/tiny_obj_loader.h>

namespace Snowy::Ark
{
void AssetManager::Init()
{
    auto currentPath = std::filesystem::current_path().generic_string();
    EngineRootPath = currentPath.substr(0, currentPath.rfind("/Engine/") + 1);
}

void AssetManager::Destory()
{}

std::vector<char> AssetManager::LoadSpirvShaderBinary(std::filesystem::path path)
{
    std::ifstream file(path.c_str(), std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open file: {}", path.string()));
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
UniqueHandle<TextureData> AssetManager::LoadTexture(std::filesystem::path path)
{
    auto data = MakeUnique<TextureData>();
    data->pixels = stbi_load(path.string().c_str(), &data->width, &data->height, &data->channel, STBI_rgb_alpha);
    if (!data->pixels)
    {
        SA_LOG_ERROR("Failed to load texture: {}", PATH_TO_SSTR(path));
        return nullptr;
    }
    return data;
}
}
