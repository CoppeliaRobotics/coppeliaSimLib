#pragma once

#include <string>
#include <vector>
#include <simTypes.h>

// FULLY STATIC CLASS
class tt  
{
public:

    static bool stringToFloat(const char* txt,double& f,bool allowNegativeValue,bool infGivesMinusOne);
    static std::string floatToEInfString(double f,bool canBeNegative);

    static std::string getEString(bool sign,double f,int precision);
    static std::string getFString(bool sign,double f,int precision);
    static std::string getDString(bool sign,double f,int precision);
    static std::string getAngleEString(bool sign,double angleInRad,int precision);
    static std::string getAngleFString(bool sign,double angleInRad,int precision);
    static std::string getIString(bool sign,int v);

    static bool stringToInt(const char* txt,int& a);
    static double floatToUserFloat(double f,double toUserConversion,bool minusValuesGiveInf);
    static double userFloatToFloat(double userFloat,double fromUserConversion,bool minusValuesGiveInf);

    static std::string decorateString(const char* prefix,const std::string mainText,const char* suffix);

    static void removeComments(std::string& line);
    static bool removeSpacesAtBeginningAndEnd(std::string& line);
    static bool removeSpacesAndEmptyLinesAtBeginningAndEnd(std::string& line);

    static std::string FNb(float number);
    static std::string FNb(double number);
    static std::string FNb(int number);
    static std::string FNb(int leadingZeros,double number,int decimals,bool sign=true);
    static std::string FNb(int leadingZeros,int number,bool sign=false);
    static int getDecimalPos(double number,int maxDec);

    static void lightEncodeBuffer(char* buff,int length);
    static void lightDecodeBuffer(char* buff,int length);

    static double getNormalizedAngle(double angle);
    static double getNormalizedAngle_range(double angle,double range);
    static double getAngleMinusAlpha(double angle,double alpha);
    static double getAngleMinusAlpha_range(double angle,double alpha,double range);

    static double getLimitedDouble(double minValue,double maxValue,double value);
    static void limitValue(double minValue,double maxValue,double value[2]);
    static void limitValue(int minValue,int maxValue,int* value);
    static bool getValidFloat(const char* text,double& value);
    static bool getValidInt(const char* text,int& value);

// Name handling and copy handling:
//************************************************************
    static std::string generateNewName_hashOrNoHash(const char* name,bool hash);
    // Following is for copied objects:
    static std::string generateNewName_hash(const char* name);
    static std::string generateNewName_hash(const char* name,int suffixOffset);

    // Following is for copied objects:
    static std::string generateNewName_noHash(const char* name);
    static std::string generateNewName_noHash(const char* name,int suffixOffset);

    // Following is for both:
    static bool isHashFree(const char* name);
    static int getNameSuffixNumber(const char* name,bool hash);
    static std::string getNameWithoutSuffixNumber(const char* name,bool hash);
    static bool isAliasValid(const char* alias);
    static std::string getValidAlias(const char* alias);
    static bool isObjectNameValid_old(const char* text,bool allowOneHashFollowedByNumbers);
    static bool removeIllegalCharacters(std::string& text,bool allowOneHashFollowedByNumbers);
    static bool removeAltNameIllegalCharacters(std::string& text);
    static std::string getObjectAltNameFromObjectName(const char* text);
//************************************************************

    static bool getValueOfKey(const char* key,const char* txt,std::string& value);
    static void removeKeyAndValue(const char* key,std::string& txt);
    static void insertKeyAndValue(const char* key,const char* value,std::string& txt);
    static int getAllKeyValuePairs(const char* txt,std::vector<std::string>& allKeys,std::vector<std::string>& allValues);
    static void appendKeyValuePair(std::string& txt,const char* key,const char* value);
    static int separateWords(const char* txt,char separationChar,std::vector<std::string>& words);


    static bool extractSpaceSeparatedWord(std::string& line,std::string& word);
    static bool extractSpaceSeparatedWord2(std::string& line,std::string& word,bool recognizeQuote,bool recognizeDoubleQuote,bool recognizeParenthesis,bool recognizeBrackets,bool recognizeSquareBrackets);
    static bool extractCommaSeparatedWord(std::string& line,std::string& word);

    static void addToFloatArray(std::vector<double>* ar,double x,double y,double z);
    static void addToIntArray(std::vector<int>* ar,int x,int y,int z);
    
    static void rgbToHsl(float rgb[3],float hsl[3]);
    static void hslToRgb(float hsl[3],float rgb[3]);

    static void orderAscending(std::vector<int>& toBeOrdered,std::vector<int>& index);
    static void orderAscending(std::vector<double>& toBeOrdered,std::vector<int>& index);
    static void orderStrings(std::vector<std::string>& toBeOrdered,std::vector<int>& index);

    static std::string getLowerUpperCaseString(std::string str,bool upper);

    static int getLimitedInt(int minValue,int maxValue,int value);
    static void limitValue(int minValue,int maxValue,int &value);
    static double getLimitedFloat(double minValue,double maxValue,double value);
    static void limitValue(double minValue,double maxValue,double &value);
    static void limitDoubleValue(double minValue,double maxValue,double &value);

private:
    static void _removeKeyAndValue(const char* key,std::string& txt);
    static void _removeKeysWithEmptyValues(std::string& txt);
    static void _insertKeyAndValue(const char* key,const char* value,std::string& txt);
    static float _hueToRgb(float m1,float m2,float h);
};
