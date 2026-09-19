#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace SnowyArk
{

struct ShaderBlob
{
    std::string name;
    std::vector<uint32_t> spirv;
};

}
