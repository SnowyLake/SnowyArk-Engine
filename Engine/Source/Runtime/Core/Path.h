#pragma once

#include <filesystem>

namespace SnowyArk
{

class Path
{
public:
    /// Returns the directory that contains the running executable.
    static std::filesystem::path ExecutableDirectory();
};

}
