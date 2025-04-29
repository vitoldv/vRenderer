#pragma once

#include <stdio.h>

// Helper function for runtime concatenation
static inline const char* concat(const char* a, const char* b) {
    static char buffer[256]; // Ensure buffer is large enough
    snprintf(buffer, sizeof(buffer), "%s%s", a, b);
    return buffer;
}   