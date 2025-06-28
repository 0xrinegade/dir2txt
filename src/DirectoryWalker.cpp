// src/DirectoryWalker.cpp
#include "DirectoryWalker.h"
#include <filesystem>
#include <iostream>

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
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) continue;
            
            // Security: Check if entry is a symlink that goes outside rootPath
            if (entry.is_symlink()) {
                try {
                    auto resolved = std::filesystem::canonical(entry.path());
                    auto rootCanonical = std::filesystem::canonical(rootPath);
                    
                    // Check if the resolved path is within the root directory
                    auto rel = std::filesystem::relative(resolved, rootCanonical);
                    if (rel.empty() || rel.string().starts_with("..")) {
                        // Symlink points outside root directory - skip it
                        continue;
                    }
                } catch (...) {
                    // If we can't resolve the symlink, skip it for security
                    continue;
                }
            }

            std::filesystem::path relPath;
            try {
                relPath = std::filesystem::relative(entry.path(), rootPath);
            } catch (...) {
                relPath = entry.path().filename();  // fallback
            }

            if (entry.is_directory()) {
                writer->writeTreeNode(relPath, "dir");
                writeTree(entry.path(), prefix + "│   ");
            } else {
                writer->writeTreeNode(relPath, "file");
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle permission denied and other filesystem errors gracefully
        std::cerr << "⚠️ Warning: Cannot access directory " << path << ": " << e.what() << std::endl;
    }
}

void DirectoryWalker::writeFiles(const std::filesystem::path& path) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (filter->shouldIgnore(entry.path())) continue;
            
            // Security: Check if entry is a symlink that goes outside rootPath
            if (entry.is_symlink()) {
                try {
                    auto resolved = std::filesystem::canonical(entry.path());
                    auto rootCanonical = std::filesystem::canonical(rootPath);
                    
                    // Check if the resolved path is within the root directory
                    auto rel = std::filesystem::relative(resolved, rootCanonical);
                    if (rel.empty() || rel.string().starts_with("..")) {
                        // Symlink points outside root directory - skip it
                        writer->writeFileSkipped(entry.path(), rootPath, "[SYMLINK OUTSIDE ROOT - SKIPPED FOR SECURITY]");
                        continue;
                    }
                } catch (...) {
                    // If we can't resolve the symlink, skip it for security
                    writer->writeFileSkipped(entry.path(), rootPath, "[UNRESOLVABLE SYMLINK - SKIPPED FOR SECURITY]");
                    continue;
                }
            }

            if (entry.is_directory()) {
                writeFiles(entry.path());
            } else if (entry.is_regular_file()) {
                if (filter->isBinary(entry.path())) {
                    writer->writeFileSkipped(entry.path(), rootPath, "[BINARY FILE SKIPPED]");
                } else {
                    writer->writeFileContents(entry.path(), rootPath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle permission denied and other filesystem errors gracefully
        std::cerr << "⚠️ Warning: Cannot access directory " << path << ": " << e.what() << std::endl;
    }
}