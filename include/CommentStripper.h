// include/CommentStripper.h
#pragma once

#include <string>
#include <vector>
#include <filesystem>

class CommentStripper {
public:
    // Strips comments from the input file and returns lines with comments removed
    static std::vector<std::string> strip(const std::filesystem::path& filePath);
    
    // Public utility function for UTF-8 safe line truncation
    static std::string truncateLineIfNeeded(const std::string& line);

private:
    static bool isSingleLineComment(const std::string& line);
    static bool startsCommentBlock(const std::string& line);
    static bool endsCommentBlock(const std::string& line);
    static std::string trim(const std::string& str);
    
    // Helper functions to reduce duplication in block comment handling
    static void flushBlockComments(std::vector<std::string>& tempBlock, std::vector<std::string>& result);
    static void flushSingleLineComments(std::vector<std::string>& singleLineBuffer, std::vector<std::string>& result);
    static bool isUtf8CharBoundary(const std::string& str, size_t pos);
};
