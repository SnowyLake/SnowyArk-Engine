#pragma once
#include <cassert>
#include <chrono>
#include <thread>

#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif

#define ArkSleep(_ms) std::this_thread::sleep_for(std::chrono::milliseconds(_ms));