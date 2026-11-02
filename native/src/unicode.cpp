#include "unicode.hpp"
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <array>
#include <algorithm>
#include <memory>
#include <stdexcept>

namespace oneocr_native {
static icu::UnicodeString from_utf8(const std::string& value) {return icu::UnicodeString::fromUTF8(value);}
static std::string to_utf8(const icu::UnicodeString& value) {std::string result;value.toUTF8String(result);return result;}
static void check(UErrorCode error) {if(U_FAILURE(error))throw std::runtime_error(std::string("ICU: ")+u_errorName(error));}
int utf16_length(const std::string& value) {return from_utf8(value).length();}
std::u32string unicode_characters(const std::string& value) {
    auto input=from_utf8(value);std::u32string result;
    for(int i=0;i<input.length();){auto character=input.char32At(i);result.push_back(char32_t(character));i+=U16_LENGTH(character);}
    return result;
}
std::vector<int> thai_word_boundaries(const std::string& value) {
    auto input=from_utf8(value);UErrorCode error=U_ZERO_ERROR;
    std::unique_ptr<UBreakIterator,decltype(&ubrk_close)> iterator(ubrk_open(UBRK_WORD,"th",input.getBuffer(),input.length(),&error),ubrk_close);check(error);
    std::vector<int> result;
    for(int boundary=ubrk_first(iterator.get());boundary!=UBRK_DONE;boundary=ubrk_next(iterator.get())) {
        if(!boundary || input.charAt(boundary)==' ' || input.charAt(boundary-1)==' ')continue;
        if(boundary==input.length() || ublock_getCode(input.char32At(boundary))==UBLOCK_THAI || ublock_getCode(input.char32At(boundary-1))==UBLOCK_THAI)result.push_back(boundary);
    }
    return result;
}
ReorderedText visual_to_logical(const std::string& value) {
    auto input=from_utf8(value);ReorderedText result;
    if(input.isEmpty())return result;
    // Native 1801ae050 scans Unicode blocks (not first-strong bidi classes).
    constexpr std::array<int,8> rtl_blocks{12,128,210,81,85,192,211,11};
    UBiDiLevel paragraph=UBIDI_DEFAULT_LTR;
    for(int i=0;i<input.length();i++) {
        int code=input.char32At(i),block=ublock_getCode(code);
        if(code==0x20aa || std::find(rtl_blocks.begin(),rtl_blocks.end(),block)!=rtl_blocks.end()){paragraph=UBIDI_DEFAULT_RTL;break;}
    }
    std::unique_ptr<UBiDi,decltype(&ubidi_close)> bidi(ubidi_open(),ubidi_close);
    if(!bidi)throw std::bad_alloc();
    ubidi_setReorderingMode(bidi.get(),UBIDI_REORDER_INVERSE_LIKE_DIRECT);
    ubidi_setReorderingOptions(bidi.get(),0);
    UErrorCode error=U_ZERO_ERROR;ubidi_setPara(bidi.get(),input.getBuffer(),input.length(),paragraph,nullptr,&error);check(error);
    std::vector<UChar> output(input.length()+1);int size=ubidi_writeReordered(bidi.get(),output.data(),int(output.size()),UBIDI_DO_MIRRORING,&error);check(error);
    std::vector<int32_t> map(input.length());ubidi_getVisualMap(bidi.get(),map.data(),&error);check(error);
    icu::UnicodeString logical(output.data(),size);result.text=to_utf8(logical);
    std::vector<int> source_word(input.length(),-1);int word=0;
    for(int i=0;i<input.length();i++){if(input.charAt(i)==' ')word++;else source_word[i]=word;}
    int previous=-1,begin=0;
    auto emit=[&](int end){if(previous>=0)result.words.push_back({to_utf8(logical.tempSubStringBetween(begin,end)),size_t(previous)});};
    for(int i=0;i<size;i++) {
        int source=map.at(i),current=source>=0?source_word.at(source):-1;
        if(current!=previous){emit(i);previous=current;begin=i;}
    }
    emit(size);return result;
}
}
