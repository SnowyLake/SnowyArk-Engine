#pragma once

#define LOG(x) std::cout << std::format("[{:%X}] {}\n", std::chrono::system_clock::now() + std::chrono::hours(8), x);