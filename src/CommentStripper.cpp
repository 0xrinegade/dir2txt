// src/CommentStripper.cpp
#include "CommentStripper.h"
#include "Constants.h"
#include <fstream>
#include <regex>
#include <iostream>

std::vector<std::string> CommentStripper::strip(const std::filesystem::path& filePath) {
    std::ifstream file(filePath);
    std::vector<std::string> inputLines;
    if (!file) return inputLines;

    std::string line;
    size_t lineCount = 0;

    // Read all lines from file
    while (std::getline(file, line) && lineCount < Constants::MAX_LINES_PER_FILE) {
        inputLines.push_back(line);
        lineCount++;
    }
    
    // Process the lines using decoupled logic
    auto result = processLines(inputLines);
    
    // Security: Add truncation notice if file was too long
    if (lineCount >= Constants::MAX_LINES_PER_FILE) {
        result.push_back("[FILE TRUNCATED - TOO MANY LINES]");
    }

    return result;
}

std::vector<std::string> CommentStripper::processLines(const std::vector<std::string>& inputLines) {
    std::vector<std::string> result;
    bool inBlock = false;
    std::vector<std::string> tempBlock;
    std::vector<std::string> singleLineBuffer;

    for (const auto& line : inputLines) {
        std::string processedLine = truncateLineIfNeeded(line);
        std::string trimmed = trim(processedLine);

        if (inBlock) {
            if (endsCommentBlock(trimmed)) {
                tempBlock.push_back(processedLine);
                inBlock = false;
                flushBlockComments(tempBlock, result);
            } else {
                tempBlock.push_back(processedLine);
            }
            continue;
        }

        if (startsCommentBlock(trimmed)) {
            inBlock = true;
            tempBlock.clear();
            if (!endsCommentBlock(trimmed)) {
                tempBlock.push_back(processedLine);
                continue;
            } else {
                // one-liner /** ... */
                tempBlock.push_back(processedLine);
                inBlock = false;
                flushBlockComments(tempBlock, result);
                continue;
            }
        }

        if (isSingleLineComment(trimmed)) {
            singleLineBuffer.push_back(processedLine);
            continue;
        }

        // Flush single-line buffer if applicable
        flushSingleLineComments(singleLineBuffer, result);
        result.push_back(processedLine);
    }

    // Final flush if file ends with single-line comment(s)
    flushSingleLineComments(singleLineBuffer, result);

    return result;
}

bool CommentStripper::isSingleLineComment(const std::string& line) {
    std::string trimmed = trim(line);
    return (!trimmed.empty() && (trimmed.find("//") == 0 || trimmed.find("#") == 0));
}

bool CommentStripper::startsCommentBlock(const std::string& line) {
    return line.find("/*") != std::string::npos;
}

bool CommentStripper::endsCommentBlock(const std::string& line) {
    return line.find("*/") != std::string::npos;
}

std::string CommentStripper::trim(const std::string& str) {
    const size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    const size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void CommentStripper::flushBlockComments(std::vector<std::string>& tempBlock, std::vector<std::string>& result) {
    if (tempBlock.size() >= 2) {
        // skip multi-line block comments
    } else {
        result.insert(result.end(), tempBlock.begin(), tempBlock.end());
    }
    tempBlock.clear();
}

void CommentStripper::flushSingleLineComments(std::vector<std::string>& singleLineBuffer, std::vector<std::string>& result) {
    if (!singleLineBuffer.empty()) {
        if (singleLineBuffer.size() < 2) {
            result.insert(result.end(), singleLineBuffer.begin(), singleLineBuffer.end());
        }
        singleLineBuffer.clear();
    }
}

std::string CommentStripper::truncateLineIfNeeded(const std::string& line) {
    // Security: Limit line length to prevent memory issues while preserving UTF-8
    if (line.length() > Constants::MAX_LINE_LENGTH) {
        size_t truncPos = Constants::MAX_LINE_LENGTH;
        
        // Find a safe UTF-8 character boundary to truncate at
        while (truncPos > 0 && !isUtf8CharBoundary(line, truncPos)) {
            truncPos--;
        }
        
        // Fallback to original position if we can't find a boundary
        if (truncPos == 0) {
            truncPos = Constants::MAX_LINE_LENGTH;
        }
        
        return line.substr(0, truncPos) + " [LINE TRUNCATED]";
    }
    return line;
}

bool CommentStripper::isUtf8CharBoundary(const std::string& str, size_t pos) {
    if (pos >= str.length()) return true;
    
    unsigned char byte = static_cast<unsigned char>(str[pos]);
    // UTF-8 character boundary: either ASCII (0xxxxxxx) or start of multi-byte (11xxxxxx)
    return (byte & 0x80) == 0 || (byte & 0xC0) == 0xC0;
}
