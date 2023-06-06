#pragma once
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <algorithm>
#include <format>
#include <chrono>
#include <span>
#include <set>

#include "Engine/Source/Runtime/Core/Base/Macro.h"
#include "Engine/Source/Runtime/Core/Base/Define.h"

namespace Snowy::Ark
{
template<typename T>
struct InArg
{
    using Type = const T&;
};
template<>
struct InArg<std::string>
{
    using Type = const std::string_view;
};

template<typename T>
struct OutArg
{
    using Type = T*;
};

template<typename T> using In = InArg<T>::Type;
template<typename T> using Out = OutArg<T>::Type;
}
namespace Ark = Snowy::Ark;