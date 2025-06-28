// src/main.cpp
#include <iostream>
#include "Config.h"
#include "FileFilter.h"
#include "FileWriterText.h"
#include "FileWriterJson.h"
#include "DirectoryWalker.h"
#include "Utils.h"

#include <memory>

int main(int argc, char* argv[]) {
    std::cout << R"(
       _ _     ____  _        _     _                 _           _     _                                 
    __| (_)_ _|___ \| |___  _| |_  | |__  _   _   ___| |__  _   _| |__ | |__   __ _ _ __ ___   ___  _   _ 
   / _  | |  __|__) | __\ \/ / __| |  _ \| | | | / __|  _ \| | | |  _ \|  _ \ / _  |  _   _ \ / _ \| | | |
  | (_| | | |  / __/| |_ >  <| |_  | |_) | |_| | \__ \ | | | |_| | |_) | | | | (_| | | | | | | (_) | |_| |
   \____|_|_| |_____|\__/_/\_\\__| |____/ \__/ / |___/_| |_|\____|____/|_| |_|\__,_|_| |_| |_|\___/ \__/ /
                                          |___/                                                     |___/ 
       )" << "\n\n";

    Config config(argc, argv);

    auto filter = std::make_shared<FileFilter>(
        config.shouldIncludeDotfiles(),
        config.getIgnoredDirs(),
        config.getRootPath()
    );

    std::string outputFilename = Utils::generateOutputFilename(config.getRootPath());
    
    // Security: Ensure output filename is safe and doesn't contain path traversal
    std::filesystem::path outputPath = std::filesystem::current_path() / outputFilename;
    try {
        outputPath = std::filesystem::canonical(outputPath.parent_path()) / outputPath.filename();
    } catch (...) {
        std::cerr << "❌ Cannot resolve output path for security validation" << std::endl;
        return 1;
    }
    
    std::shared_ptr<IWriter> writer;

    if (config.outputAsJson()) {
        outputFilename = outputPath.stem().string() + ".json";
        outputPath = outputPath.parent_path() / outputFilename;
        writer = std::make_shared<FileWriterJson>(outputPath.string(), config.shouldStripComments());
    } else {
        writer = std::make_shared<FileWriterText>(outputPath.string(), config.shouldStripComments());
    }

    DirectoryWalker walker(
        config.getRootPath(),
        filter,
        writer
    );

    walker.walk();

    std::cout << "✅ Done! Output written to: " << outputPath << "\n";
    return 0;
}