// src/UniversalIgnoreParser.cpp
#include "UniversalIgnoreParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

void UniversalIgnoreParser::loadFromDirectory(const std::filesystem::path& root) {
    for (const auto& ignoreFile : knownIgnoreFiles) {
        loadFromFile(root / ignoreFile);
    }
}

void UniversalIgnoreParser::loadFromFile(const std::filesystem::path& file) {
    if (!std::filesystem::exists(file)) return;

    // Security: Check if the file is within reasonable size limits
    try {
        auto fileSize = std::filesystem::file_size(file);
        if (fileSize > 1024 * 1024) { // 1MB limit
            std::cerr << "⚠️ Warning: Ignore file too large, skipping: " << file << std::endl;
            return;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "⚠️ Warning: Cannot check ignore file size: " << file << " - " << e.what() << std::endl;
        return;
    }

    std::ifstream in(file);
    if (!in) {
        std::cerr << "⚠️ Warning: Cannot open ignore file: " << file << std::endl;
        return;
    }

    std::string line;
    size_t lineCount = 0;
    while (std::getline(in, line) && lineCount < 10000) { // Limit number of patterns
        lineCount++;
        if (line.empty() || line[0] == '#') continue;
        
        // Security: Validate line length
        if (line.length() > 1000) {
            std::cerr << "⚠️ Warning: Ignore pattern line too long, skipping line " << lineCount << " in " << file << std::endl;
            continue;
        }
        
        try {
            patterns.push_back(convertToRegex(line));
        } catch (...) {
            std::cerr << "⚠️ Failed to parse ignore pattern: " << line << std::endl;
        }
    }
    
    if (lineCount >= 10000) {
        std::cerr << "⚠️ Warning: Too many ignore patterns in file, truncated: " << file << std::endl;
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

    auto it = cache.find(pathStr);
    if (it != cache.end()) {
        return it->second;
    }

    for (const auto& regex : patterns) {
        if (std::regex_match(pathStr, regex)) {
            cache[pathStr] = true;
            return true;
        }
    }

    cache[pathStr] = false;
    return false;
}

std::regex UniversalIgnoreParser::convertToRegex(const std::string& pattern) const {
    // Security: Validate pattern length to prevent ReDoS
    if (pattern.length() > 1000) {
        std::cerr << "⚠️ Warning: Ignore pattern too long, skipping: " << pattern.substr(0, 50) << "..." << std::endl;
        return std::regex("^$"); // Match nothing
    }
    
    // Security: Check for potentially dangerous regex patterns
    size_t starCount = std::count(pattern.begin(), pattern.end(), '*');
    size_t plusCount = std::count(pattern.begin(), pattern.end(), '+');
    if (starCount > 10 || plusCount > 5) {
        std::cerr << "⚠️ Warning: Potentially dangerous ignore pattern, skipping: " << pattern << std::endl;
        return std::regex("^$"); // Match nothing
    }

    std::string regexStr = "^";

    if (pattern.back() == '/') {
        std::string dirName = pattern.substr(0, pattern.length() - 1);
        // Escape special regex characters in directory name
        std::string escapedDirName;
        for (char c : dirName) {
            if (c == '.' || c == '^' || c == '$' || c == '(' || c == ')' || 
                c == '[' || c == ']' || c == '{' || c == '}' || c == '|' || 
                c == '\\' || c == '?' || c == '+') {
                escapedDirName += "\\";
            }
            escapedDirName += c;
        }
        regexStr += "(" + escapedDirName + ")(/.*)?$";
    } else if (pattern.find('/') == std::string::npos && pattern.find('*') == std::string::npos) {
        // Escape special regex characters
        std::string escapedPattern;
        for (char c : pattern) {
            if (c == '.' || c == '^' || c == '$' || c == '(' || c == ')' || 
                c == '[' || c == ']' || c == '{' || c == '}' || c == '|' || 
                c == '\\' || c == '?' || c == '+') {
                escapedPattern += "\\";
            }
            escapedPattern += c;
        }
        regexStr = ".*/" + escapedPattern + "$|^" + escapedPattern + "$";
    } else {
        for (char c : pattern) {
            switch (c) {
                case '*': regexStr += ".*"; break;
                case '.': regexStr += "\\."; break;
                case '/': regexStr += "/"; break;
                case '^': case '$': case '(': case ')': case '[': case ']':
                case '{': case '}': case '|': case '\\': case '?': case '+':
                    regexStr += "\\";
                    regexStr += c;
                    break;
                default:  regexStr += c; break;
            }
        }
        regexStr += "$";
    }

    try {
        return std::regex(regexStr);
    } catch (const std::regex_error& e) {
        std::cerr << "⚠️ Warning: Invalid regex pattern, skipping: " << pattern << " - " << e.what() << std::endl;
        return std::regex("^$"); // Match nothing
    }
}