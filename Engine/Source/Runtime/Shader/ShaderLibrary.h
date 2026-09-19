#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace SnowyArk
{

class ShaderLibrary
{
public:
    /// Loads a SPIR-V file from `Shaders/` next to the executable.
    /// Returns false when the file cannot be opened, sized, or read, or when the size is 0, not a multiple of 4, or too large for `size_t` / `streamsize`.
    [[nodiscard]] bool Load(const char* relativePath);

    /// Returns the loaded SPIR-V. Lives until library destruction; same-key Load replaces contents and invalidates prior data/span/element pointers. Fatals if not loaded.
    const std::vector<uint32_t>& GetSpirv(const char* relativePath) const;

private:
    std::unordered_map<std::string, std::vector<uint32_t>> m_Shaders;
};

}
