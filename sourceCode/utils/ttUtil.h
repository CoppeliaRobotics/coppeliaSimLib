
#pragma once

#include "vrepMainHeader.h"

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
    static std::string intToString(int intVal);
    static std::string dwordToString(unsigned int dwordVal);
    static std::string dataToString(char* data,int startPos,int length);
    static void stringToData(const std::string& str,char* data,int startPos);
    static std::string getAdjustedString(const std::string& str,int length);
    static std::string getLightEncodedString(const std::string& ss);
    static std::string getLightDecodedString(const std::string& ss);
    static bool removeSpacesAtBeginningAndEnd(std::string& line);
    static std::string getFormattedString(const char* a=nullptr,const char* b=nullptr,const char* c=nullptr,const char* d=nullptr,const char* e=nullptr,const char* f=nullptr,const char* g=nullptr,const char* h=nullptr);
    static std::string formatString(const std::string fmt, ...);
    static std::string getLowerCaseString(const char* str);
    static void scaleLightDown_(float& r,float& g,float& b);
    static void scaleColorUp_(float& r,float& g,float& b);
    static void scaleLightDown_(float* rgb);
    static void scaleColorUp_(float* rgb);
    static std::string decode64(const std::string &data);
    static std::string encode64(const std::string &data);
    static std::string generateUniqueString();
    static std::string generateUniqueReadableString();

    static int replaceWordInLine(std::string& line,const std::string& oldText,const std::string& newText);
};
