// src/DirectoryWalker.cpp
#include "DirectoryWalker.h"
#include "Constants.h"
#include <filesystem>
#include <iostream>

DirectoryWalker::DirectoryWalker(
    const std::filesystem::path& root,
    std::shared_ptr<IFileFilter> filter,
    std::shared_ptr<IWriter> writer,
    bool enableLogging)
    : rootPath(root), filter(std::move(filter)), writer(std::move(writer)), loggingEnabled(enableLogging) {
    
    // Cache the canonical root path for performance
    try {
        rootCanonical = std::filesystem::canonical(rootPath);
    } catch (...) {
        rootCanonical = rootPath;  // Fallback to original path
    }
}

void DirectoryWalker::walk() {
    writer->writeHeader(rootPath);
    writeTree(rootPath);
    writer->writeSectionDivider("📂 Dumping file contents...");
    writeFiles(rootPath);
}

void DirectoryWalker::writeTree(const std::filesystem::path& path, const std::string& prefix) {
    // Security: Prevent infinite recursion from deep directory structures
    if (currentRecursionDepth > Constants::MAX_RECURSION_DEPTH) {
        logSkipped("Maximum recursion depth reached", path);
        return;  // Limit recursion depth
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) {
                logSkipped("Ignored by filter", entry.path());
                continue;
            }

            // Security: Validate path security (symlinks and traversal protection)
            if (!validatePathSecurity(entry.path())) {
                continue;
            }

            std::filesystem::path relPath;
            try {
                relPath = std::filesystem::relative(entry.path(), rootPath);
            } catch (...) {
                relPath = entry.path().filename();  // fallback
            }

            if (entry.is_directory()) {
                writer->writeTreeNode(relPath, "dir");
                currentRecursionDepth++;
                writeTree(entry.path(), prefix + "│   ");
                currentRecursionDepth--;
            } else {
                writer->writeTreeNode(relPath, "file");
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Security: Silently skip directories that can't be accessed
        logSkipped("Filesystem error: " + std::string(e.what()), path);
        return;
    }
}

void DirectoryWalker::writeFiles(const std::filesystem::path& path) {
    // Security: Prevent infinite recursion from deep directory structures
    if (currentRecursionDepth > Constants::MAX_RECURSION_DEPTH) {
        logSkipped("Maximum recursion depth reached", path);
        return;  // Limit recursion depth
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) {
                logSkipped("Ignored by filter", entry.path());
                continue;
            }

            // Security: Validate path security (symlinks and traversal protection)
            if (!validatePathSecurity(entry.path())) {
                continue;
            }

            if (entry.is_directory()) {
                currentRecursionDepth++;
                writeFiles(entry.path());
                currentRecursionDepth--;
            } else if (entry.is_regular_file()) {
                // Security: Check file size to prevent memory exhaustion
                try {
                    auto fileSize = std::filesystem::file_size(entry.path());
                    if (fileSize > Constants::MAX_FILE_SIZE_BYTES) {
                        writer->writeFileSkipped(entry.path(), rootPath, "[FILE TOO LARGE - SKIPPED]");
                        logSkipped("File too large (" + std::to_string(fileSize) + " bytes)", entry.path());
                        continue;
                    }
                } catch (...) {
                    // Skip files where size can't be determined
                    logSkipped("Cannot determine file size", entry.path());
                    continue;
                }

                if (filter->isBinary(entry.path())) {
                    writer->writeFileSkipped(entry.path(), rootPath, "[BINARY FILE SKIPPED]");
                    logSkipped("Binary file", entry.path());
                } else {
                    writer->writeFileContents(entry.path(), rootPath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Security: Silently skip directories that can't be accessed
        logSkipped("Filesystem error: " + std::string(e.what()), path);
        return;
    }
}

void DirectoryWalker::logSkipped(const std::string& reason, const std::filesystem::path& path) {
    if (loggingEnabled) {
        std::cerr << "⚠️ Skipped: " << reason << " - " << path << std::endl;
    }
}

bool DirectoryWalker::validatePathSecurity(const std::filesystem::path& entryPath) {
    // Security: First resolve canonical paths for more reliable validation
    std::filesystem::path canonical;
    try {
        canonical = std::filesystem::canonical(entryPath);
    } catch (...) {
        // If canonical path cannot be resolved, treat as potentially dangerous
        logSkipped("Cannot resolve canonical path", entryPath);
        return false;
    }

    // Security: Check if canonical path is within root directory bounds
    try {
        auto relativePath = std::filesystem::relative(canonical, rootCanonical);
        if (relativePath.string().substr(0, 2) == "..") {
            logSkipped("Path escapes root directory", entryPath);
            return false;
        }
    } catch (...) {
        logSkipped("Cannot make canonical path relative", entryPath);
        return false;
    }

    // Security: Additional check for symlinks that could lead outside root
    if (std::filesystem::is_symlink(entryPath)) {
        // We already validated the canonical path above, but log for visibility
        logSkipped("Symlink processed via canonical path validation", entryPath);
    }

    return true;
}