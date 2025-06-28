// src/Config.cpp
#include "Config.h"
#include "Constants.h"
#include "cxxopts.hpp"
#include <iostream>

Config::Config(int argc, char* argv[]) {
    try {
        cxxopts::Options options(argv[0], "A blazing-fast CLI tool to export a directory's structure and contents into a neatly formatted `.txt` or `.json` file.");

        options.positional_help("<directory>")
               .show_positional_help();

        options.add_options()
            ("i,ignore", "Comma-separated list of files/folders to ignore", cxxopts::value<std::string>())
            ("include-dotfiles", "Include hidden dotfiles and dotfolders. It will still ignore .git, .cache, etc.")
            ("s,strip-comments", "Strip comments from source files")
            ("j,json", "Output results in JSON format")
            ("v,verbose", "Enable verbose logging of skipped files and directories")
            ("max-asterisk", "Maximum number of asterisks in ignore patterns (default: 5)", cxxopts::value<size_t>())
            ("max-dots", "Maximum number of dots in ignore patterns (default: 10)", cxxopts::value<size_t>())
            ("h,help", "Print usage")
            ("directory", "Root directory", cxxopts::value<std::string>());

        options.parse_positional({"directory"});

        auto result = options.parse(argc, argv);

        if (result.count("help") || !result.count("directory")) {
            std::cout << options.help() << std::endl;
            std::exit(0);
        }

        std::string dirPath = result["directory"].as<std::string>();
        
        // Security: Validate directory path
        if (dirPath.empty() || dirPath.length() > Constants::MAX_PATH_LENGTH) {
            std::cerr << "❌ Invalid directory path length" << std::endl;
            std::exit(1);
        }
        
        // Security: Check for suspicious path patterns
        if (dirPath.find("..") != std::string::npos) {
            std::cerr << "❌ Path traversal not allowed" << std::endl;
            std::exit(1);
        }

        rootPath = std::filesystem::path(dirPath);

        // Security: Canonicalize path to prevent traversal attacks
        try {
            rootPath = std::filesystem::canonical(rootPath);
        } catch (const std::filesystem::filesystem_error&) {
            std::cerr << "❌ Cannot resolve directory path: " << dirPath << std::endl;
            std::exit(1);
        }

        if (!std::filesystem::exists(rootPath) || !std::filesystem::is_directory(rootPath)) {
            std::cerr << "❌ Invalid directory: " << rootPath << std::endl;
            std::exit(1);
        }

        if (result.count("ignore")) {
            std::string list = result["ignore"].as<std::string>();
            
            // Security: Validate ignore list length
            if (list.length() > Constants::MAX_IGNORE_LIST_LENGTH) {
                std::cerr << "❌ Ignore list too long" << std::endl;
                std::exit(1);
            }
            
            size_t pos;
            size_t patternCount = 0;
            while ((pos = list.find(',')) != std::string::npos) {
                std::string pattern = list.substr(0, pos);
                // Security: Limit number and length of ignore patterns
                if (++patternCount > Constants::MAX_IGNORE_PATTERNS || pattern.length() > Constants::MAX_PATTERN_LENGTH) {
                    std::cerr << "❌ Too many or too long ignore patterns" << std::endl;
                    std::exit(1);
                }
                ignoredDirs.insert(pattern);
                list.erase(0, pos + 1);
            }
            if (!list.empty()) {
                if (++patternCount > Constants::MAX_IGNORE_PATTERNS || list.length() > Constants::MAX_PATTERN_LENGTH) {
                    std::cerr << "❌ Too many or too long ignore patterns" << std::endl;
                    std::exit(1);
                }
                ignoredDirs.insert(list);
            }
        }

        includeDotfiles = result.count("include-dotfiles") > 0;
        stripComments = result.count("strip-comments") > 0;
        asJson = result.count("json") > 0;
        enableLogging = result.count("verbose") > 0;
        
        // Configure regex pattern complexity limits
        if (result.count("max-asterisk")) {
            maxAsteriskCount = result["max-asterisk"].as<size_t>();
        }
        if (result.count("max-dots")) {
            maxDotCount = result["max-dots"].as<size_t>();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Argument parsing error: " << e.what() << std::endl;
        std::exit(1);
    }
}

const std::filesystem::path& Config::getRootPath() const {
    return rootPath;
}

const std::set<std::string>& Config::getIgnoredDirs() const {
    return ignoredDirs;
}

bool Config::shouldIncludeDotfiles() const {
    return includeDotfiles;
}

bool Config::shouldStripComments() const {
    return stripComments;
}

bool Config::outputAsJson() const {
    return asJson;
}

bool Config::shouldEnableLogging() const {
    return enableLogging;
}

size_t Config::getMaxAsteriskCount() const {
    return maxAsteriskCount;
}

size_t Config::getMaxDotCount() const {
    return maxDotCount;
}
