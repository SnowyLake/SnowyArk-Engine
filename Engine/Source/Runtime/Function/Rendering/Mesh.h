#pragma once
#include "Engine/Source/Runtime/Core/Base/Common.h"
#include "Engine/Source/Runtime/Function/Global/GlobalTypedef.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace Snowy::Ark
{
struct VertexPosition
{
    std::vector<glm::vec3> data;
    EVertexAttributeFormat format;
    uint32_t binding = 0;
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;
    Mesh(const Mesh&) = default;
    Mesh(Mesh&&) = default;
    Mesh& operator=(const Mesh&) = default;
    Mesh& operator=(Mesh&&) = default;

private:
    std::vector<uint32_t> m_Indices;

    VertexPosition position;

private:
    struct VertexAttributeFlags
    {
        unsigned int position  : 1;
        unsigned int normal    : 1;
        unsigned int tangent   : 1;
        unsigned int color     : 1;
        unsigned int texcoord0 : 1;
        unsigned int texcoord1 : 1;
        unsigned int texcoord2 : 1;
        unsigned int texcoord3 : 1;
        unsigned int texcoord4 : 1;
        unsigned int texcoord5 : 1;
        unsigned int texcoord6 : 1;
        unsigned int texcoord7 : 1;
    };
    VertexAttributeFlags m_Flags;
};
}
