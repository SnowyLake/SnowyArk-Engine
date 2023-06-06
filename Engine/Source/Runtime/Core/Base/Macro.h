#pragma once

#define LOG(x) std::cout << std::format("[{:%X}] {}\n", std::chrono::system_clock::now() + std::chrono::hours(8), x);

#define CHECK_VK_RESULT(result, log) \
    if(result != vk::Result::eSuccess) \
    { \
        throw std::runtime_error(log); \
    } \
    else \

#define CHECK_VK_RESULT_TARGET(result, log, target) \
    if(result != target) \
    { \
        throw std::runtime_error(log); \
    } \
    else \