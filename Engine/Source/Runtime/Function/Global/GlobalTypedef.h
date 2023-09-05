#pragma once
namespace Snowy::Ark
{
enum class ELogOutputTarget : uint8_t
{
    Console,
    Editor,

};
enum class ELogLevel : uint8_t
{
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};


enum class ERHIBackend : uint8_t
{
    None,
    OpenGL,
    Vulkan,
    DirectX11,
    DirectX12,
};
enum class ERHIQueueType : uint8_t
{
    Present = 0,
    Graphics,
    Compute,
    Transfer, // Blit?
};
}
