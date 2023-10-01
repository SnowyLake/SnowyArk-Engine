#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Core/Log/Logger.h"
namespace Snowy::Ark
{
struct TextureData
{
    TextureData() = default;
    ~TextureData()
    {
        free(pixels);
    }
    unsigned char* pixels;
    int width;
    int height;
    int channel;
};

struct TextureParams
{

};

class RHITexture
{
public:
    RHITexture() = default;
    virtual ~RHITexture() = default;
    RHITexture(const RHITexture&) = default;
    RHITexture(RHITexture&&) = default;
    RHITexture& operator=(const RHITexture&) = default;
    RHITexture& operator=(RHITexture&&) = default;
};
}
