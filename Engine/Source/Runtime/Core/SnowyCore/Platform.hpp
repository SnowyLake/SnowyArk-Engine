#pragma once
#include <type_traits>
namespace Snowy
{
// An ANSI char. 8-bit fixed-width representation of 7-bit chars.
using AnsiChar = char;

// A wide char. In-memory only. ?-bit fixed-width representation of the platform's natural wide char set. Could be different sizes on different platforms.
using WideChar = wchar_t;

// UTF Code char types.
using Utf8Char = char8_t;
using Utf16Char = char16_t;
using Utf32Char = char32_t;

// Char code macros
#if !defined(SNOWY_CORE_CHAR_ANSI) || defined(UNICODE) // Win32 Api
    #define SNOWY_CORE_CHAR_WIDE
#endif // !defined(SNOWY_CORE_CHAR_ANSI)

// Char type for SnowyCore.
#if defined(SNOWY_CORE_CHAR_WIDE)
using SChar = WideChar;
using SCharUtfCode = std::conditional_t<sizeof(SChar) == sizeof(Utf16Char), Utf16Char, Utf32Char>;
#else
using SChar = AnsiChar;
using SCharUtfCode = Utf8Char;
#endif  // defined(SNOWY_CORE_CHAR_WIDE)

constexpr inline bool SCharTypeIsWChar = std::is_same_v<SChar, WideChar>;
constexpr inline bool SCharSizeIsUtf16 = std::is_same_v<SCharUtfCode, Utf16Char>;

// ANSI Support
#define ANSI_TEXT(x) x

// WIDE Support
#define WIDE_TEXT(x) L ## x

// Utf-8 Support
#if _MSVC_LANG >= 202002L	/*CXX20*/
#define UTF8_TEXT(x) reinterpret_cast<const char*>(u8 ## x)
#else	
#define UTF8_TEXT(x) u8 ## x
#endif  // _MSVC_LANG >= 202002L

// Utf-16 Support
#define UTF16_TEXT(x) u ## x

// Utf-32 Support
#define UTF32_TEXT(x) U ## x

#if defined(SNOWY_CORE_CHAR_WIDE)
    #define STEXT(x) WIDE_TEXT(x)
#else
    #define STEXT(x) ANSI_TEXT(x)
#endif  // defined(SNOWY_CORE_CHAR_WIDE)
}