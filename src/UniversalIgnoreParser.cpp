// src/UniversalIgnoreParser.cpp
#include "UniversalIgnoreParser.h"
#include "Constants.h"
#include <fstream>
#include <iostream>

void UniversalIgnoreParser::loadFromDirectory(const std::filesystem::path& root) {
    for (const auto& ignoreFile : knownIgnoreFiles) {
        loadFromFile(root / ignoreFile);
    }
}

void UniversalIgnoreParser::loadFromFile(const std::filesystem::path& file) {
    if (!std::filesystem::exists(file)) return;

    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        try {
            patterns.push_back(convertToRegex(line));
        } catch (...) {
            std::cerr << "⚠️ Failed to parse ignore pattern: " << line << std::endl;
        }
    }
}

void UniversalIgnoreParser::addManualIgnore(const std::string& pattern) {
    try {
        patterns.push_back(convertToRegex(pattern));
    } catch (...) {
        std::cerr << "⚠️ Failed to compile manual ignore: " << pattern << std::endl;
    }
}

bool UniversalIgnoreParser::shouldIgnore(const std::filesystem::path& relPath) const {
    std::string pathStr = relPath.generic_string();
    
    // Security: Limit path string length to prevent ReDoS
    if (pathStr.length() > Constants::MAX_PATH_LENGTH) {
        return false;  // Very long paths are likely not in ignore patterns
    }

    auto it = cache.find(pathStr);
    if (it != cache.end()) {
        return it->second;
    }

    for (const auto& regex : patterns) {
        try {
            // Security: Use std::regex_match which is generally safer than std::regex_search
            if (std::regex_match(pathStr, regex)) {
                cache[pathStr] = true;
                return true;
            }
        } catch (const std::regex_error&) {
            // Security: Skip malformed regex patterns that somehow got through
            continue;
        }
    }

    cache[pathStr] = false;
    return false;
}

std::regex UniversalIgnoreParser::convertToRegex(const std::string& pattern) const {
    // Security: Validate pattern to prevent ReDoS attacks
    if (pattern.empty() || pattern.length() > Constants::MAX_PATTERN_LENGTH) {
        throw std::invalid_argument("Invalid pattern length");
    }
    
    // Security: Check for potentially dangerous regex patterns
    size_t asteriskCount = 0;
    size_t dotCount = 0;
    for (char c : pattern) {
        if (c == '*') asteriskCount++;
        if (c == '.') dotCount++;
        // Prevent control characters that could cause issues
        if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
            throw std::invalid_argument("Invalid character in pattern");
        }
    }
    
    // Security: Limit number of wildcards to prevent catastrophic backtracking
    if (asteriskCount > maxAsteriskCount || dotCount > maxDotCount) {
        throw std::invalid_argument("Pattern too complex");
    }

    std::string regexStr = "^";

    if (pattern.back() == '/') {
        std::string dirName = pattern.substr(0, pattern.length() - 1);
        std::string escapedDirName = escapeRegexSpecialChars(dirName);
        regexStr += "(" + escapedDirName + ")(/.*)?$";
    } else if (pattern.find('/') == std::string::npos && pattern.find('*') == std::string::npos) {
        std::string escapedPattern = escapeRegexSpecialChars(pattern);
        regexStr = ".*/" + escapedPattern + "$|^" + escapedPattern + "$";
    } else {
        for (char c : pattern) {
            switch (c) {
                case '*': regexStr += "[^/]*"; break;  // Security: Make * non-greedy for paths
                case '.': regexStr += "\\."; break;
                case '/': regexStr += "/"; break;
                // Security: Escape other regex metacharacters
                case '^': case '$': case '|': case '(': case ')':
                case '[': case ']': case '{': case '}': case '+': case '?':
                    regexStr += "\\";
                    regexStr += c;
                    break;
                default:  regexStr += c; break;
            }
        }
        regexStr += "$";
    }

    try {
        return std::regex(regexStr, std::regex_constants::optimize);
    } catch (const std::regex_error& e) {
        throw std::invalid_argument("Failed to compile regex pattern: " + pattern);
    }
}

std::string UniversalIgnoreParser::escapeRegexSpecialChars(const std::string& input) const {
    std::string escaped;
    escaped.reserve(input.length() * 2);  // Reserve space to avoid reallocations
    
    const std::string specialChars = ".^$|()[]{}+?";
    
    for (char c : input) {
        if (specialChars.find(c) != std::string::npos) {
            escaped += "\\";
        }
        escaped += c;
    }
    
    return escaped;
}

void UniversalIgnoreParser::setComplexityLimits(size_t maxAsterisk, size_t maxDots) {
    maxAsteriskCount = maxAsterisk;
    maxDotCount = maxDots;
}