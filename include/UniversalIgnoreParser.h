// include/UniversalIgnoreParser.h
#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>

class UniversalIgnoreParser {
public:
    void loadFromDirectory(const std::filesystem::path& root);
    void addManualIgnore(const std::string& pattern);
    bool shouldIgnore(const std::filesystem::path& relPath) const;
    void setComplexityLimits(size_t maxAsterisk, size_t maxDots);

private:
    std::vector<std::regex> patterns;
    mutable std::unordered_map<std::string, bool> cache;  // ✅ Cache for performance
    size_t maxAsteriskCount = 5;  // Default values
    size_t maxDotCount = 10;

    std::regex convertToRegex(const std::string& pattern) const;
    void loadFromFile(const std::filesystem::path& file);
    std::string escapeRegexSpecialChars(const std::string& input) const;
    std::string escapeRegexSpecialChars(const std::string& input, size_t length) const;

    const std::vector<std::string> knownIgnoreFiles = {
        ".gitignore", ".dockerignore", ".npmignore", ".dir2txtignore"
    };
};