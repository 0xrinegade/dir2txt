// include/DirectoryWalker.h
#pragma once

#include "IDirectoryWalker.h"
#include "IFileFilter.h"
#include "IWriter.h"
#include <filesystem>
#include <memory>
#include <string>

class DirectoryWalker : public IDirectoryWalker {
public:
    DirectoryWalker(const std::filesystem::path& root,
                    std::shared_ptr<IFileFilter> filter,
                    std::shared_ptr<IWriter> writer,
                    bool enableLogging = false);

    void walk() override;

private:
    void writeTree(const std::filesystem::path& path, const std::string& prefix = "");
    void writeFiles(const std::filesystem::path& path);
    void logSkipped(const std::string& reason, const std::filesystem::path& path);
    bool validatePathSecurity(const std::filesystem::path& entryPath);

    std::filesystem::path rootPath;
    std::filesystem::path rootCanonical;  // Cached canonical root path
    std::shared_ptr<IFileFilter> filter;
    std::shared_ptr<IWriter> writer;
    bool loggingEnabled;
};