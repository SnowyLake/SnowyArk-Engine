#pragma once
#include<string>
#include<memory>

namespace Snowy
{
// Non-Template Type In/Out Argument Wrapper
template<typename T> struct InArg { using Type = const T&; };
template<>           struct InArg<std::string> { using Type = std::string_view; };
template<typename T> using  In = InArg<T>::Type;

template<typename T> struct OutArg { using Type = T*; };
template<typename T> using  Out = OutArg<T>::Type;

// Template Type In/Out Argument Wrapper
#define TIn(T) const T&
#define TOut(T) T*
}