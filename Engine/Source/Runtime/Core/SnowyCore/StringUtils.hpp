#pragma once
#include "Platform.hpp"

#include <Windows.h>

#include <string>

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

// String Type Define
using AnsiString = std::string;
using WideString = std::wstring;
using SString = std::conditional_t<SCharTypeIsWChar, WideString, AnsiString>;

/// <summary>
/// STL String Convertor.
/// </summary>
struct StringConvertor
{
    static std::string WideToAnsi(WideStringIn wstr)
    {
        std::string result;
        int wSize = static_cast<int>(wstr.size());
        //获取缓冲区大小，并申请空间，缓冲区大小按字节计算
        int aSize = WideCharToMultiByte(CP_ACP, 0, wstr.data(), wSize, NULL, 0, NULL, NULL);
        auto buffer = std::make_unique<AnsiChar[]>(static_cast<size_t>(aSize) + 1);
        //宽字节编码转换成多字节编码
        WideCharToMultiByte(CP_ACP, 0, wstr.data(), wSize, buffer.get(), aSize, NULL, NULL);
        buffer[aSize] = '\0';
        result.append(buffer.get());
        return result;
    }
    static std::wstring AnsiToWide(AnsiStringIn astr)
    {
        std::wstring result;
        int aSize = static_cast<int>(astr.size());
        //获取缓冲区大小，并申请空间，缓冲区大小按字符计算
        int wSize = MultiByteToWideChar(CP_ACP, 0, astr.data(), aSize, NULL, 0);
        auto buffer = std::make_unique<WideChar[]>(static_cast<size_t>(wSize) + 1);
        //多字节编码转换成宽字节编码
        MultiByteToWideChar(CP_ACP, 0, astr.data(), aSize, buffer.get(), wSize);
        buffer[wSize] = '\0';
        result.append(buffer.get());
        return result;
    }
    template<bool UseStdU8string = false>
    static auto WideToUtf8(WideStringIn wstr) -> std::conditional_t<UseStdU8string, std::u8string, AnsiString>
    {

    }
    template<bool UseStdU8string = false>
    static auto AnsiToUtf8(AnsiStringIn astr) -> std::conditional_t<UseStdU8string, std::u8string, AnsiString>
    {

    }
};
#define WIDE_TO_ANSI(x) StringConvertor::WideToAnsi(x)
#define ANSI_TO_WIDE(x) StringConvertor::AnsiToWide(x)
#define WIDE_TO_UTF8(x) StringConvertor::WideToUtf8(x)
#define ANSI_TO_UTF8(x) StringConvertor::AnsiToUtf8(x)
#if defined(SNOWY_CORE_CHAR_WIDE)
    #define WIDE_TO_SSTR(x) x
    #define ANSI_TO_SSTR(x) StringConvertor::AnsiToWide(x)
#else
    #define WIDE_TO_SSTR(x) StringConvertor::WideToAnsi(x)
    #define ANSI_TO_SSTR(x) x
#endif // defined(SNOWY_CORE_CHAR_WIDE)

/// <summary>
/// SnowyCore's custom string tools.
/// </summary>
class StringUtils
{

};

/// <summary>
/// SnowyCore's custom string class, which is std::string proxy class and add utf-8 support.
/// </summary>
template<typename Char>
class TStringWrapper
{
public:
    using WrappedType = std::conditional_t<std::is_same_v<Char, WideChar>, WideString, AnsiString>;

private:
    WrappedType m_Data;

public:
    TStringWrapper() = default;
    TStringWrapper(const WrappedType& data) : m_Data(data) {}
    TStringWrapper(WrappedType&& data) : m_Data(std::forward<WrappedType>(data)) {}
    TStringWrapper& operator=(const WrappedType& data) { m_Data = data; return *this; }
    TStringWrapper& operator=(WrappedType&& data) { m_Data = std::forward<WrappedType>(data); return *this; }

    bool operator==(const TStringWrapper& other) const { return m_Data == other.m_Data; }

    operator std::string() const { return m_Data; }
    operator std::string_view() const { return m_Data; }

    WrappedType& Get() { return m_Data; }
    WrappedType& operator*() { return m_Data; }
    WrappedType* operator->() { return &m_Data; }
public:
    // TODO: String Common Method
};
using StringWrapper = TStringWrapper<SChar>;


// Text Localized
// ---------------------------------

enum class ELocaleType
{
    English = 0,
    Chinese
};

/// <summary>
/// Localized string, english and chinese
/// </summary>
class LocaleText
{
public:
    static ELocaleType CurrentLocale;

private:
    std::unordered_map<ELocaleType, StringWrapper> m_LocaleText;
};
}