#pragma once
#include <memory>
namespace Snowy
{
#define GetDecayedType(value) std::decay_t<decltype(value)>

// Pointer Wrapper
template<typename T> using RawHandle = T*;
template<typename T> using UniqueHandle = std::unique_ptr<T>;
template<typename T> using SharedHandle = std::shared_ptr<T>;
template<typename T> using WeakHandle = std::weak_ptr<T>;

template<typename T, typename... Args>
inline constexpr UniqueHandle<T> MakeUnique(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
template<typename T, typename... Args>
inline constexpr SharedHandle<T> MakeShared(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

// Argument Wrapper
template<typename T> using In = const T&;
template<typename T> using ArrayIn = std::span<T>;
using StringIn = std::string_view;

template<typename T> using Out = T* const;
}