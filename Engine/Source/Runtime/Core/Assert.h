#pragma once

#include <Runtime/Core/Log.h>

#define SNOWYARK_ASSERT(condition)                                                                                                                                                                     \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (!(condition))                                                                                                                                                                              \
        {                                                                                                                                                                                              \
            ::SnowyArk::Log::Fatal("Assertion failed: " #condition);                                                                                                                                   \
        }                                                                                                                                                                                              \
    } while (false)
