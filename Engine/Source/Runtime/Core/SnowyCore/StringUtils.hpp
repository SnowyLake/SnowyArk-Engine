#pragma once
#include<string>

namespace Snowy
{
/// <summary>
/// Fixed String, can use for template param.
/// </summary>
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
    constexpr operator const char* () const { return buffer; }

    // not mandatory anymore
    auto operator<=>(const FixedString&) const = default;
};
template<size_t N> FixedString(const char(&)[N]) -> FixedString<N - 1>;


// Char code macros
#if !defined(SNOWY_CORE_CHARCODE_DEFAULT)
#define SNOWY_CORE_CHARCODE_UTF8
#endif // !defined(SNOWY_CORE_CHARCODE_DEFAULT)

// Utf-8 Support
#if defined(SNOWY_CORE_CHARCODE_UTF8)
#if _MSVC_LANG >= 202002L	/*CXX20*/
#define SCHAR(t) reinterpret_cast<const char*>(u8##t)
#else	
#define SCHAR(t) u8##t
#endif // _MSVC_LANG >= 202002L
#else
#define SCHAR(t) t
#endif // defined(SNOWY_CORE_CHARCODE_UTF8)

/// <summary>
/// SnowyCore's custom string class, which is std::string proxy class and add utf-8 support.
/// </summary>
class StringProxy
{
private:
    using DataType = std::string;
    DataType m_Data;

public:
    StringProxy() = default;
    StringProxy(const DataType& data) : m_Data(data) {}
    StringProxy(DataType&& data) : m_Data(std::forward<DataType>(data)) {}
    StringProxy& operator=(const DataType& data) { m_Data = data; return *this; }
    StringProxy& operator=(DataType&& data) { m_Data = std::forward<DataType>(data); return *this; }

    bool operator==(const StringProxy& other) const { return m_Data == other.m_Data; }

    operator std::string() const { return m_Data; }
    operator std::string_view() const { return m_Data; }

public:
    DataType& Proxy() { return m_Data; }

};

/// <summary>
/// SnowyCore's custom string tools class.
/// </summary>
class StringUtils
{

};
}