// include/Config.h
#pragma once

#include "Constants.h"
#include <filesystem>   // for std::filesystem::path
#include <set>          // for std::set
#include <string>       // for std::string

class Config {
    public:
        Config(int argc, char* argv[]);
        
        const std::filesystem::path& getRootPath() const;
        const std::set<std::string>& getIgnoredDirs() const;
        bool shouldIncludeDotfiles() const;
        bool shouldStripComments() const;
        bool outputAsJson() const;
        bool shouldEnableLogging() const;
        size_t getMaxAsteriskCount() const;
        size_t getMaxDotCount() const;
    
    private:
        std::filesystem::path rootPath;
        std::set<std::string> ignoredDirs;
        bool includeDotfiles = false;
        bool stripComments = false;
        bool asJson = false;
        bool enableLogging = false;
        size_t maxAsteriskCount = Constants::MAX_ASTERISK_COUNT;
        size_t maxDotCount = Constants::MAX_DOT_COUNT;
    };