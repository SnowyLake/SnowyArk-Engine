#pragma once
#include<string>

namespace Snowy
{
// TODO: My String
//class String;

/// <summary>
/// Fixed String, can use for template param.
/// </summary>
/// <typeparam name="N"></typeparam>
template<size_t N>
struct FixedString
{
    char buffer[N + 1]{};
    constexpr FixedString(const char* s) 
    {
        for (size_t i = 0; i != N; ++i)
        {
            buffer[i] = s[i];
        }
    }
    constexpr operator const char*() const { return buffer; }

    // not mandatory anymore
    auto operator<=>(const FixedString&) const = default;
};
template<size_t N> FixedString(const char (&)[N]) -> FixedString<N - 1>;
}