// src/DirectoryWalker.cpp
#include "DirectoryWalker.h"
#include <filesystem>

DirectoryWalker::DirectoryWalker(
    const std::filesystem::path& root,
    std::shared_ptr<IFileFilter> filter,
    std::shared_ptr<IWriter> writer)
    : rootPath(root), filter(std::move(filter)), writer(std::move(writer)) {}

void DirectoryWalker::walk() {
    writer->writeHeader(rootPath);
    writeTree(rootPath);
    writer->writeSectionDivider("📂 Dumping file contents...");
    writeFiles(rootPath);
}

void DirectoryWalker::writeTree(const std::filesystem::path& path, const std::string& prefix) {
    // Security: Prevent infinite recursion from deep directory structures
    static thread_local int recursionDepth = 0;
    if (recursionDepth > 100) {
        return;  // Limit recursion depth
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) continue;

            // Security: Check for symlinks that could lead outside root
            if (entry.is_symlink()) {
                try {
                    auto canonical = std::filesystem::canonical(entry.path());
                    auto rootCanonical = std::filesystem::canonical(rootPath);
                    auto relativePath = std::filesystem::relative(canonical, rootCanonical);
                    // Skip symlinks that point outside the root directory
                    if (relativePath.string().substr(0, 2) == "..") {
                        continue;
                    }
                } catch (...) {
                    // Skip symlinks that can't be resolved
                    continue;
                }
            }

            std::filesystem::path relPath;
            try {
                relPath = std::filesystem::relative(entry.path(), rootPath);
                // Security: Ensure relative path doesn't escape root
                if (relPath.string().substr(0, 2) == "..") {
                    continue;
                }
            } catch (...) {
                relPath = entry.path().filename();  // fallback
            }

            if (entry.is_directory()) {
                writer->writeTreeNode(relPath, "dir");
                recursionDepth++;
                writeTree(entry.path(), prefix + "│   ");
                recursionDepth--;
            } else {
                writer->writeTreeNode(relPath, "file");
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Security: Silently skip directories that can't be accessed
        return;
    }
}

void DirectoryWalker::writeFiles(const std::filesystem::path& path) {
    // Security: Prevent infinite recursion from deep directory structures
    static thread_local int recursionDepth = 0;
    if (recursionDepth > 100) {
        return;  // Limit recursion depth
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) continue;

            // Security: Check for symlinks that could lead outside root
            if (entry.is_symlink()) {
                try {
                    auto canonical = std::filesystem::canonical(entry.path());
                    auto rootCanonical = std::filesystem::canonical(rootPath);
                    auto relativePath = std::filesystem::relative(canonical, rootCanonical);
                    // Skip symlinks that point outside the root directory
                    if (relativePath.string().substr(0, 2) == "..") {
                        continue;
                    }
                } catch (...) {
                    // Skip symlinks that can't be resolved
                    continue;
                }
            }

            // Security: Additional path validation
            try {
                auto relPath = std::filesystem::relative(entry.path(), rootPath);
                if (relPath.string().substr(0, 2) == "..") {
                    continue;  // Skip paths that escape root
                }
            } catch (...) {
                continue;  // Skip paths that can't be made relative
            }

            if (entry.is_directory()) {
                recursionDepth++;
                writeFiles(entry.path());
                recursionDepth--;
            } else if (entry.is_regular_file()) {
                // Security: Check file size to prevent memory exhaustion
                try {
                    auto fileSize = std::filesystem::file_size(entry.path());
                    if (fileSize > 50 * 1024 * 1024) {  // 50MB limit
                        writer->writeFileSkipped(entry.path(), rootPath, "[FILE TOO LARGE - SKIPPED]");
                        continue;
                    }
                } catch (...) {
                    // Skip files where size can't be determined
                    continue;
                }

                if (filter->isBinary(entry.path())) {
                    writer->writeFileSkipped(entry.path(), rootPath, "[BINARY FILE SKIPPED]");
                } else {
                    writer->writeFileContents(entry.path(), rootPath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Security: Silently skip directories that can't be accessed
        return;
    }
}