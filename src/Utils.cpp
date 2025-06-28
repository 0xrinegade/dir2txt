// src/Utils.cpp
#include "Utils.h"
#include "Constants.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <mutex>

namespace Utils {

    std::string getRelativePath(const std::filesystem::path& path,
                                const std::filesystem::path& root) {
        try {
            return std::filesystem::relative(path, root).string();
        } catch (...) {
            return path.filename().string();  // fallback
        }
    }

    bool startsWithDot(const std::filesystem::path& path) {
        std::string name = path.filename().string();
        return !name.empty() && name[0] == '.';
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        
        // Thread-safe time formatting using a mutex
        static std::mutex time_mutex;
        std::lock_guard<std::mutex> lock(time_mutex);
        std::tm local_tm = *std::localtime(&now_time);
    
        std::ostringstream oss;
        oss << std::put_time(&local_tm, "%d%m%y%H%M%S");
        return oss.str();
    }

    std::string generateOutputFilename(const std::filesystem::path& rootPath) {
        std::filesystem::path cleaned = rootPath;
        while (cleaned.has_filename() == false && cleaned.has_parent_path()) {
            cleaned = cleaned.parent_path();
        }
        std::string dirName = cleaned.filename().string();
        
        // Security: Sanitize directory name for safe filename
        std::string sanitizedDirName;
        for (char c : dirName) {
            if (std::isalnum(c) || c == '_' || c == '-') {
                sanitizedDirName += c;
            } else {
                sanitizedDirName += '_';
            }
        }
        
        // Security: Ensure filename isn't empty and has reasonable length
        if (sanitizedDirName.empty()) {
            sanitizedDirName = "output";
        }
        if (sanitizedDirName.length() > Constants::MAX_FILENAME_LENGTH) {
            sanitizedDirName = sanitizedDirName.substr(0, Constants::MAX_FILENAME_LENGTH);
        }
        
        std::string timestamp = getCurrentTimestamp();
        std::string filename = sanitizedDirName + "_" + timestamp + "_dir2txt.txt";
        
        // Security: Ensure filename doesn't start with . or contain path separators
        if (filename[0] == '.' || filename.find('/') != std::string::npos || 
            filename.find('\\') != std::string::npos) {
            filename = "output_" + timestamp + "_dir2txt.txt";
        }
        
        return filename;
    }
}