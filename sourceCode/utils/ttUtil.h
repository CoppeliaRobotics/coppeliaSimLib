#pragma once

#include <vector>
#include <string>
#include "simTypes.h"

class CTTUtil  
{ // FULLY STATIC!!
public:
    static void lightBinaryEncode(char* data,int length);
    static void lightBinaryDecode(char* data,int length);
    static unsigned short getCRC(char* data,int length);
    static unsigned short getCRC(const std::string& data);
    static bool extractCommaSeparatedWord(std::string& line,std::string& word);
    static bool extractSpaceSeparatedWord(std::string& line,std::string& word);
    static bool extractLine(std::string& multiline,std::string& line);
    static std::string getLightEncodedString(const char* ss);
    static std::string getLightDecodedString(const char* ss);
    static bool removeSpacesAtBeginningAndEnd(std::string& line);
    static std::string getFormattedString(const char* a=nullptr,const char* b=nullptr,const char* c=nullptr,const char* d=nullptr,const char* e=nullptr,const char* f=nullptr,const char* g=nullptr,const char* h=nullptr);
    static std::string getLowerCaseString(const char* str);
    static void scaleLightDown_(float* rgb);
    static void scaleColorUp_(float* rgb);
    static std::string decode64(const std::string &data);
    static std::string encode64(const std::string &data);
    static std::string generateUniqueString();
    static std::string generateUniqueReadableString();
    static void replaceSubstring(std::string& str,const char* subStr,const char* replacementSubStr);
    static void regexReplace(std::string& str,const char* regexStr,const char* regexReplacementSubStr);
    static void removeComments(std::string& line);
    static int lineCountAtOffset(const char* str,int offset);
    static bool doStringMatch_wildcard(const char* wildcardStr,const char* otherStr);

};
