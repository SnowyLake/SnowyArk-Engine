#pragma once
#include "Platform.hpp"
#include <memory>

#define DecayedTypeOf(v) std::decay_t<decltype(v)>
#define NameOf(v) #v

namespace Snowy
{
// Pointer Wrapper
template<typename T> using RawHandle = T*;
template<typename T> using UniqueHandle = std::unique_ptr<T>;
template<typename T> using SharedHandle = std::shared_ptr<T>;
template<typename T> using WeakHandle = std::weak_ptr<T>;
template<typename T> class ObserverHandle
{
public:
    using ElementType = std::remove_reference_t<T>;

private:
    ElementType* m_Ptr = nullptr;

public:
    ObserverHandle() noexcept : m_Ptr(nullptr) {}
    ObserverHandle(ElementType* ptr) noexcept : m_Ptr(ptr) {}
    ObserverHandle(const ObserverHandle&) = default;
    ObserverHandle(ObserverHandle&&) = default;
    template<class U> ObserverHandle(ObserverHandle<U> other) noexcept
        requires std::convertible_to<typename ObserverHandle<U>::ElementType*, ElementType*>
    : m_Ptr(other.Get())
    {}
    ~ObserverHandle() = default;    // ObserverHandle has no ownership of the resource, so dtor do nothing.

    ElementType* operator->() const noexcept { return m_Ptr; }
    ElementType& operator* () const { return *m_Ptr; }
    bool operator==(const ObserverHandle& other) const { return m_Ptr == other.m_Ptr; }

    operator bool() const noexcept { return m_Ptr != nullptr; }
    operator ElementType* () const noexcept { return m_Ptr; }

    ElementType* Get() const noexcept { return m_Ptr; }
    ElementType* Release() noexcept { auto ptr = m_Ptr; m_Ptr = nullptr; return ptr; }
    void Reset(ElementType* ptr = nullptr) noexcept { m_Ptr = ptr; }
    void Swap(ObserverHandle& other) noexcept { auto ptr = m_Ptr; m_Ptr = other.m_Ptr; other.m_Ptr = ptr; }
};

template<typename T, typename... Args>
inline constexpr UniqueHandle<T> MakeUnique(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
template<typename T, typename... Args>
inline constexpr SharedHandle<T> MakeShared(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
template<typename T, typename... Args>
inline constexpr ObserverHandle<T> MakeObserver(Args&&... args) { return ObserverHandle<T>(std::forward<Args>(args)...); }


// Argument Wrapper
template<typename T> using In = const T&;
template<typename T> using Ref = T&;
template<typename T> using Out = T* const;
template<typename T> using ArrayIn = std::span<T>;

using AnsiStringIn = std::string_view;
using WideStringIn = std::wstring_view;
using SStringIn = std::conditional_t<SCharTypeIsWChar, WideStringIn, AnsiStringIn>;
}