#pragma once

#include <iostream>

#if ASSERTIONS_ENABLED

#define VRD_INTERNAL_DEBUG_BREAK() __debugbreak()
#define VRD_ASSERT(expr) \
    if (expr) { } \
    else \
    { \
        std::cout << "ASSERT FAILURE (" << #expr << ") in file: " \
                  << __FILE__ << " at line: " << __LINE__ \
                  << std::endl; \
        VRD_INTERNAL_DEBUG_BREAK(); \
    }
#else
#define VRD_ASSERT(expr) ((void)0)
#endif


// This is static assert implementation for pre C++11.
// Left for reference and study purposes.
#define _ASSERT_GLUE(a, b) a ## b
#define ASSERT_GLUE(a, b) _ASSERT_GLUE(a, b)
#define VRD_STATIC_ASSERT(expr) \
    enum \
    { \
        _ASSERT_GLUE(g_assert_fail_, __LINE__) \
            = 1 / (int)(!!(expr))    \
    }