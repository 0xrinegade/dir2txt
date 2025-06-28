// include/Constants.h
#pragma once

#include <cstddef>  // for size_t

namespace Constants {
    // Security limits for input validation
    constexpr size_t MAX_PATH_LENGTH = 1024;
    constexpr size_t MAX_IGNORE_LIST_LENGTH = 2048;
    constexpr size_t MAX_IGNORE_PATTERNS = 50;
    constexpr size_t MAX_PATTERN_LENGTH = 256;
    constexpr size_t MAX_FILENAME_LENGTH = 100;
    
    // File processing limits
    constexpr size_t MAX_LINES_PER_FILE = 50000;
    constexpr size_t MAX_LINE_LENGTH = 10000;
    constexpr size_t MAX_FILE_SIZE_MB = 50;
    constexpr size_t MAX_FILE_SIZE_BYTES = MAX_FILE_SIZE_MB * 1024 * 1024;
    
    // Directory traversal limits
    constexpr int MAX_RECURSION_DEPTH = 100;
    
    // Regex pattern complexity limits
    constexpr size_t MAX_ASTERISK_COUNT = 5;
    constexpr size_t MAX_DOT_COUNT = 10;
    
    // Cache size limits
    constexpr size_t MAX_PATH_CACHE_SIZE = 1024;
    // Note: No automatic eviction policy implemented. 
    // Cache growth is bounded by MAX_PATH_CACHE_SIZE and typical path patterns
    // In practice, cache rarely exceeds a few hundred entries during normal operation
}