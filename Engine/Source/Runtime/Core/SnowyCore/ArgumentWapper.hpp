#pragma once
#include<string>

namespace Snowy
{
template<typename T> struct InArg { using Type = const T&; };
template<>           struct InArg<std::string> { using Type = std::string_view; };
template<typename T> using  In = InArg<T>::Type;

template<typename T> struct OutArg { using Type = T*; };
template<typename T> using  Out = OutArg<T>::Type;

template<typename T> struct HandleArg { using Type = T*; };
template<typename T> using  Handle = HandleArg<T>::Type;
}