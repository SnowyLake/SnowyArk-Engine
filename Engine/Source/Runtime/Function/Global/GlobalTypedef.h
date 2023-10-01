#pragma once
namespace Snowy::Ark
{

/// <summary>
/// Engine log type
/// </summary>
enum class ELogLevel : uint8_t
{
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    // ========
    Count,
};

/// <summary>
/// Engine log output target
/// </summary>
enum class ELogOutputTarget : uint8_t
{
    Console,
    Editor,
    // ========
    Count,
};

/// <summary>
/// RHI type
/// </summary>
enum class ERHIBackend : uint8_t
{
    None = 0,
    OpenGL,
    Vulkan,
    DirectX11,
    DirectX12,
    // ========
    Count,
};

/// <summary>
/// RHI queue type
/// </summary>
enum class ERHIQueue : uint8_t
{
    Present = 0,
    Graphics,
    Compute,
    Transfer, // Blit?
    // ========
    Count,
};

/// <summary>
/// RHI buffer type
/// </summary>
enum class ERHIBuffer : uint8_t
{
    Uniform = 0,
    Vertex,
    Index,
    //...
    // ========
    Count,
};


/// <summary>
/// Mesh vertex attribute type
/// </summary>
enum class EVertexAttribute : uint8_t
{
    Position = 0,
    Normal,
    Tangent,
    Color,
    Texcoord,
    Count
};

/// <summary>
/// Mesh vertex attribute format type
/// </summary>
enum class EVertexAttributeFormat : uint8_t
{

};
}
