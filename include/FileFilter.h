// include/FileFilter.h
#pragma once

#include "IFileFilter.h"
#include "UniversalIgnoreParser.h"
#include <memory>
#include <set>
#include <string>

class FileFilter : public IFileFilter {
public:
    FileFilter(bool includeDotfiles,
               std::set<std::string> ignoredDirs,
               const std::filesystem::path& rootPath,
               size_t maxAsteriskCount = 5,
               size_t maxDotCount = 10);

    bool isBinary(const std::filesystem::path& filePath) const override;
    bool shouldIgnore(const std::filesystem::path& path) const override;

private:
    bool includeDotfiles;
    std::set<std::string> ignoredDirs;
    std::unique_ptr<UniversalIgnoreParser> ignoreParser;
    std::filesystem::path root;

    static const std::set<std::string> alwaysIgnored;
};