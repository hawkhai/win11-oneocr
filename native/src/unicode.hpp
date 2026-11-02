#pragma once
#include <string>
#include <vector>
namespace oneocr_native {
int utf16_length(const std::string& value);
std::u32string unicode_characters(const std::string& value);
std::vector<int> thai_word_boundaries(const std::string& value);
struct ReorderedWord {std::string text;size_t source;};
struct ReorderedText {std::string text;std::vector<ReorderedWord> words;};
ReorderedText visual_to_logical(const std::string& value);
}
