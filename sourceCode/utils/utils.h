#pragma once

#include <vector>
#include <string>
#include <simLib/simTypes.h>

class utils
{ // FULLY STATIC!!
  public:
    static void lightBinaryEncode(char* data, int length);
    static void lightBinaryDecode(char* data, int length);
    static unsigned short getCRC(char* data, int length);
    static unsigned short getCRC(const std::string& data);
    static std::string extractWord(std::string& str, const char* separationWord);
    static bool extractCommaSeparatedWord(std::string& line, std::string& word);
    static bool extractSpaceSeparatedWord(std::string& line, std::string& word);
    static bool extractLine(std::string& multiline, std::string& line);
    static std::string getLightEncodedString(const std::string& ss);
    static std::string getLightDecodedString(const std::string& ss);
    static bool removeSpacesAtBeginningAndEnd(std::string& line);
    static std::string getWithoutPrefix(const char* input, const char* prefix);
    static std::string getFormattedString(const char* a = nullptr, const char* b = nullptr, const char* c = nullptr,
                                          const char* d = nullptr, const char* e = nullptr, const char* f = nullptr,
                                          const char* g = nullptr, const char* h = nullptr);
    static std::string getLowerCaseString(const char* str);
    static void scaleLightDown_(float* rgb);
    static void scaleColorUp_(float* rgb);
    static std::string decode64(const std::string& data);
    static std::string encode64(const std::string& data);
    static std::string toHex(const char* buffer, size_t length = 0);
    static std::string fromHex(const char* hexString);
    static std::string xorHexStrings(const char* a, const char* b, bool repeatShortStringToSameSize);
    static std::string generateUniqueString();
    static std::string generateUniqueAlphaNumericString();
    static bool isAlphaNumeric(const std::string& str);
    static bool checkAssemblyTagValidity(const char* parentSideTag, const char* childSideTag);
    static bool replaceSubstring(std::string& str, const char* subStr, const char* replacementSubStr);
    static bool replaceSubstringStart(std::string& str, const char* subStr, const char* replacementSubStr);
    static bool replaceSubstringEnd(std::string& str, const char* subStr, const char* replacementSubStr);
    static bool startsWith(const char* str, const char* prefix);
    static void regexReplace(std::string& str, const char* regexStr, const char* regexReplacementSubStr);
    static void removeComments(std::string& line);
    static int lineCountAtOffset(const char* str, int offset);
    static bool doStringMatch_wildcard(const char* wildcardStr, const char* otherStr);
    static std::string getPosString(bool sign, double num);
    static std::string getSizeString(bool sign, double num);
    static std::string getAngleString(bool sign, double num);
    static std::string getTimeString(bool seconds, double num);
    static std::string getVolumeString(double num);
    static std::string getDensityString(double num);
    static std::string getMultString(bool sign, double num);
    static std::string getForceTorqueString(bool sign, double num);
    static std::string getMassString(double num);
    static std::string getTensorString(bool massless, double num);
    static std::string getGravityString(bool signe, double num);
    static std::string getLinVelString(bool sign, double num);
    static std::string getAngVelString(bool sign, double num);
    static std::string getLinAccelString(bool sign, double num);
    static std::string getAngAccelString(bool sign, double num);
    static std::string getLinJerkString(bool sign, double num);
    static std::string getAngJerkString(bool sign, double num);
    static std::string get0To1String(bool sign, double num);

    static std::string getDoubleString(bool sign, double num, int minDecimals, int maxDecimals,
                                       double minForExpNotation = 0.0, double maxForExpNotation = 0.0);
    static std::string getDoubleEString(bool sign, double num);
    static std::string getIntString(bool sign, int num, int minDigits = 1);

    static double getDoubleFromString(const char* str, double minMaxValue = 0.0);
};
