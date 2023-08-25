#pragma once
// TODO: Log.h
#include <iostream>
#include <format>
#include <chrono>

#define LOG(x) std::cout << std::format("[{:%X}] {}\n", std::chrono::system_clock::now() + std::chrono::hours(8), x);

#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif