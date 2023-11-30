#include <utils.h>
#include <simMath/mathDefines.h>
#include <stdarg.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <base64.h>
#include <vDateTime.h>
#include <regex>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <QByteArray>

void utils::lightBinaryEncode(char *data, int length)
{ // Very simple!
    for (int i = 0; i < length; i++)
        data[i] = (((data[i] & 0xf0) >> 4) | ((data[i] & 0x0f) << 4)) ^ (i & 0xff);
}

void utils::lightBinaryDecode(char *data, int length)
{ // very simple!
    for (int i = 0; i < length; i++)
    {
        char tmp = data[i] ^ (i & 0xff);
        data[i] = (((tmp & 0xf0) >> 4) | ((tmp & 0x0f) << 4));
    }
}

unsigned short utils::getCRC(char *data, int length)
{
    unsigned short crc = 0;
    int p = 0;
    for (int i = 0; i < length; i++)
    {
        crc = crc ^ (((unsigned short)data[p]) << 8);
        for (int j = 0; j < 8; j++)
        {
            if (crc & ((unsigned short)0x8000))
                crc = (crc << 1) ^ ((unsigned short)0x1021);
            else
                crc <<= 1;
        }
        p++;
    }
    return (crc);
}

unsigned short utils::getCRC(const std::string &data)
{
    if (data.length() == 0)
        return (0);
    return (getCRC((char *)(&data[0]), int(data.length())));
}

bool utils::extractCommaSeparatedWord(std::string &line, std::string &word)
{ // Returns true if a word could be extracted
    word = "";
    while ((line.length() != 0) && (line[0] != ','))
    {
        word.append(line.begin(), line.begin() + 1);
        line.erase(line.begin());
    }
    if ((line.length() != 0) && (line[0] == ','))
        line.erase(line.begin());
    return (word.length() != 0);
}

bool utils::extractSpaceSeparatedWord(std::string &line, std::string &word)
{ // Returns true if a word could be extracted
    word = "";
    while ((line.length() != 0) && (line[0] != ' '))
    {
        word.append(line.begin(), line.begin() + 1);
        line.erase(line.begin());
    }
    if ((line.length() != 0) && (line[0] == ' '))
        line.erase(line.begin());
    return (word.length() != 0);
}

bool utils::extractLine(std::string &multiline, std::string &line)
{ // Returns true if a line could be extracted
    size_t n = multiline.find('\n');
    if (n != std::string::npos)
    {
        line.assign(multiline.begin(), multiline.begin() + n);
        multiline.erase(multiline.begin(), multiline.begin() + n + 1);
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        return (true);
    }
    else
    {
        line = multiline;
        multiline.clear();
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        return (line.length() != 0);
    }
}

std::string utils::getLightEncodedString(const char *ss)
{ // ss can contain any char, also 0!
    std::string txt(ss);
    std::string s;
    for (int i = 0; i < int(txt.length()); i++)
    {
        int n[2];
        n[0] = (unsigned char)(txt[i]) + i;
        n[1] = n[0] / 16;
        n[0] -= n[1] * 16;
        for (int j = 0; j < 2; j++)
        {
            int t = n[1 - j];
            if (t > 9)
                s += 'A' + (t - 10);
            else
                s += '0' + t;
        }
    }
    return (s);
}

std::string utils::getLightDecodedString(const char *ss)
{ // return string can contain any char, also 0!
    std::string txt(ss);
    std::string s;
    for (int i = 0; i < int(txt.length()) / 2; i++)
    {
        int v = 0;
        for (int j = 0; j < 2; j++)
        {
            int w;
            char a = txt[2 * i + j];
            if (a >= 'A')
                w = 10 + (a - 'A');
            else
                w = a - '0';
            if (j == 0)
                v += w * 16;
            else
                v += w;
        }
        v -= i;
        s += char(v);
    }
    return (s);
}

bool utils::removeSpacesAtBeginningAndEnd(std::string &line)
{
    while ((line.length() != 0) && (line[0] == ' '))
        line.erase(line.begin());
    while ((line.length() != 0) && (line[line.length() - 1] == ' '))
        line.erase(line.begin() + line.length() - 1);
    return (line.length() != 0);
}

std::string utils::getFormattedString(const char *a, const char *b, const char *c, const char *d, const char *e,
                                      const char *f, const char *g, const char *h)
{
    std::string retVal("");
    if (a == nullptr)
        return (retVal);
    retVal += a;
    if (b == nullptr)
        return (retVal);
    retVal += b;
    if (c == nullptr)
        return (retVal);
    retVal += c;
    if (d == nullptr)
        return (retVal);
    retVal += d;
    if (e == nullptr)
        return (retVal);
    retVal += e;
    if (f == nullptr)
        return (retVal);
    retVal += f;
    if (g == nullptr)
        return (retVal);
    retVal += g;
    if (h == nullptr)
        return (retVal);
    retVal += h;
    return (retVal);
}

std::string utils::getLowerCaseString(const char *str)
{
    std::string retValue(str);
    for (int i = 0; i < int(retValue.length()); i++)
    {
        if ((retValue[i] >= 65) && (retValue[i] <= 90))
            retValue[i] += 32;
    }
    return (retValue);
}

void utils::scaleLightDown_(float *rgb)
{ // this is for backward compatibility (29/8/2013). Default lights are now less intense, and more similar to other
  // applications
    rgb[0] *= 0.675f;
    rgb[1] *= 0.675f;
    rgb[2] *= 0.675f;
}

void utils::scaleColorUp_(float *rgb)
{ // this is for backward compatibility (29/8/2013). Default lights are now less intense, and more similar to other
  // applications
    float f = 1.5f;
    float m = std::max<float>(std::max<float>(rgb[0], rgb[1]), rgb[2]);
    float d = 1.0f / m;
    if (d < f)
        f = d;
    rgb[0] *= f;
    rgb[1] *= f;
    rgb[2] *= f;
}

std::string utils::decode64(const std::string &data)
{
    QByteArray arr(data.c_str(), data.size());
    return (QByteArray::fromBase64(arr, QByteArray::Base64Encoding).toStdString());
    // return(base64_decode(data));
}

std::string utils::encode64(const std::string &data)
{
    QByteArray arr(data.c_str(), data.size());
    return (arr.toBase64().toStdString());
    // return(base64_encode((const unsigned char*)data.c_str(),(unsigned int)data.size()));
}

std::string utils::generateUniqueString()
{
    std::string s("1234567890123456");
    char a[17];
    for (size_t i = 0; i < 16; i++)
        a[i] = (unsigned char)(((static_cast<double>(rand()) / static_cast<double>(RAND_MAX))) * 255.1);
    char b[17];
    sprintf(b, "%i", (int)VDateTime::getTimeInMs());
    char c[17];
    sprintf(c, "%i", int(VDateTime::getSecondsSince1970()));
    for (size_t i = 0; i < 16; i++)
        s[i] += a[i] + b[i] + c[i];
    return (s);
}

std::string utils::generateUniqueAlphaNumericString()
{
    static bool seeded = false;
    if (!seeded)
    {
        srand((unsigned int)VDateTime::getTimeInMs() + (unsigned int)VDateTime::getSecondsSince1970());
        seeded = true;
    }
    std::string str;
    char num[4];
    for (size_t i = 0; i < 8; i++)
    {
        unsigned char nb = (unsigned char)(((static_cast<double>(rand()) / static_cast<double>(RAND_MAX))) * 255.1);
        snprintf(num, 3, "%x", nb);
        if (strlen(num) == 1)
            str += std::string("0") + num;
        else
            str += num;
    }
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return (str);
}

bool utils::isAlphaNumeric(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) { return std::isalnum(ch); });
}

bool utils::checkAssemblyTagValidity(const char *parentSideTag, const char *childSideTag)
{
    bool retVal = false;
    if (childSideTag == nullptr)
    {
        std::string p(parentSideTag);
        if (p.size() > 0)
            retVal = ((p[0] == '*') || (p[0] == '+') || (p[0] == '-') || (p[0] == ':') || (p[0] == '#'));
    }
    else if (parentSideTag == nullptr)
    {
        std::string c(childSideTag);
        if (c.size() > 0)
            retVal = ((c[0] == '*') || (c[0] == '+') || (c[0] == '-') || (c[0] == '.') || (c[0] == '#'));
    }
    else
    {
        std::string p(parentSideTag);
        std::string c(childSideTag);
        if ((p.size() > 0) && (c.size() > 0))
        {
            bool fine = false;
            if (p[0] == '*')
                fine = ((c[0] == '*') || (c[0] == '+') || (c[0] == '-'));
            else if (p[0] == ':')
                fine = (c[0] == '.');
            else if (p[0] == '+')
                fine = ((c[0] == '-') || (c[0] == '*'));
            else if (p[0] == '-')
                fine = ((c[0] == '+') || (c[0] == '*'));
            else if (p[0] == '#')
                fine = (c[0] == '#');
            if (fine)
            {
                p.erase(0, 1);
                c.erase(0, 1);
                retVal = (p == c);
            }
        }
    }
    return retVal;
}

void utils::replaceSubstring(std::string &str, const char *subStr, const char *replacementSubStr)
{
    size_t index = 0;
    size_t str1L = strlen(subStr);
    size_t str2L = strlen(replacementSubStr);
    while (true)
    {
        index = str.find(subStr, index);
        if (index == std::string::npos)
            break;
        str.replace(index, str1L, replacementSubStr);
        index += str2L;
    }
}

void utils::regexReplace(std::string &str, const char *regexStr, const char *regexReplacementSubStr)
{
    str = std::regex_replace(str, std::regex(regexStr), regexReplacementSubStr);
}

void utils::removeComments(std::string &line)
{
    replaceSubstring(line, "://", "doubleptslashslash");
    size_t p = line.find("//");
    if (p != std::string::npos)
        line.erase(p);
    replaceSubstring(line, "doubleptslashslash", "://");
}

int utils::lineCountAtOffset(const char *str, int offset)
{
    int retVal = 0;
    std::string all(str);
    std::string line;
    while (extractLine(all, line))
    {
        retVal++;
        offset -= int(line.size());
        if (offset <= 0)
            break;
        offset--; // line feed
    }
    return (retVal);
}

bool utils::doStringMatch_wildcard(const char *wildcardStr, const char *otherStr)
{
    if ((wildcardStr[0] == '\0') && (otherStr[0] == '\0'))
        return true;
    if ((wildcardStr[0] == '*') && ((wildcardStr + 1)[0] != '\0') && (otherStr[0] == '\0'))
        return false;
    if ((wildcardStr[0] == '?') || (wildcardStr[0] == otherStr[0]))
        return doStringMatch_wildcard(wildcardStr + 1, otherStr + 1);
    if (wildcardStr[0] == '*')
        return (doStringMatch_wildcard(wildcardStr + 1, otherStr) || doStringMatch_wildcard(wildcardStr, otherStr + 1));
    return false;
}

std::string utils::getDoubleString(bool sign, double num, int minDecimals, int maxDecimals,
                                   double minForExpNotation /*=0.0*/, double maxForExpNotation /*=0.0*/)
{
    std::string retVal;
    if (((minForExpNotation != 0.0) && (num != 0.0) && (fabs(num) <= minForExpNotation)) ||
        ((maxForExpNotation != 0.0) && (fabs(num) >= maxForExpNotation)))
    {
        std::stringstream s;
        s.precision(3);
        s.setf(std::ios::scientific, std::ios::floatfield);
        s << num;
        retVal = s.str();
        if (sign && (num >= 0.0))
            retVal = "+" + retVal;
        // Following very unelegant but temporary (to avoid an exponent of 3 width):
        int l = retVal.size();
        if (((retVal[l - 4] == '-') || (retVal[l - 4] == '+')) && (retVal[l - 3] == '0'))
            retVal.erase(retVal.begin() + l - 3);
    }
    else
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(maxDecimals) << num;
        retVal = stream.str();
        int total_decimals = retVal.length() - (retVal.find(".") + 1);
        while (total_decimals < maxDecimals)
        {
            retVal += '0';
            total_decimals++;
        }
        for (int i = 0; i < maxDecimals - minDecimals; i++)
        {
            if (retVal[retVal.length() - 1] == '0')
                retVal.erase(retVal.length() - 1, 1);
        }
        if (retVal[retVal.length() - 1] == '.')
            retVal.erase(retVal.length() - 1, 1);
        if ((retVal[0] == '-') || (retVal[0] == '+'))
            retVal.erase(0, 1);
        bool zero = true;
        for (size_t i = 0; i < retVal.length(); i++)
        {
            if ((retVal[i] != '.') && (retVal[i] != '0'))
            {
                zero = false;
                break;
            }
        }
        if (!zero)
        {
            if (num <= 0.0)
                retVal = "-" + retVal;
            else if (sign)
                retVal = "+" + retVal;
        }
    }
    return retVal;
}

std::string utils::getDoubleEString(bool sign, double num)
{
    return getDoubleString(sign, num, 1, 1, 1.0, 1.0);
}

std::string utils::getPosString(bool sign, double num)
{
    return getDoubleString(sign, num, 3, 5);
}

std::string utils::getTimeString(bool seconds, double num)
{
    std::string retVal;
    if (seconds)
        retVal = getDoubleString(false, num, 3, 6, 0.00001);
    else
    { // hours:minutes:seconds:ms
        retVal = getIntString(false, int(num / 3600.0), 2);
        retVal += ":";
        retVal += getIntString(false, int(fmod(num / 60.0, 60.0)), 2) + ":";
        retVal += getIntString(false, int(fmod(num, 60.0)), 2) + ".";
        retVal += getIntString(false, int(fmod(num * 100.0, 100.0)), 2);
    }
    return retVal;
}

std::string utils::getGravityString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 4, 0.001);
}

std::string utils::get0To1String(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 3);
}

std::string utils::getSizeString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 5, 0.0001, 9999.0);
}

std::string utils::getAngleString(bool sign, double num)
{
    return getDoubleString(sign, num * radToDeg, 2, 3);
}

std::string utils::getVolumeString(double num)
{
    return getDoubleString(false, num, 5, 6);
}

std::string utils::getDensityString(double num)
{
    return getDoubleString(false, num, 0, 2);
}

std::string utils::getMultString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 5, 0.0001, 9999.0);
}

std::string utils::getForceTorqueString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 5, 0.0001, 9999.0);
}

std::string utils::getMassString(double num)
{
    return getDoubleString(false, num, 2, 6);
}

std::string utils::getTensorString(bool massless, double num)
{
    return getDoubleString(true, num, 3, 5, 0.0001, 99.0);
}

std::string utils::getLinVelString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 6, 0.00001, 9999.0);
}

std::string utils::getAngVelString(bool sign, double num)
{
    return getDoubleString(sign, num * radToDeg, 2, 5, 0.0001, 9999.0);
}

std::string utils::getLinAccelString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 5, 0.0001, 9999.0);
}

std::string utils::getAngAccelString(bool sign, double num)
{
    return getDoubleString(sign, num * radToDeg, 2, 4, 0.001, 9999.0);
}

std::string utils::getLinJerkString(bool sign, double num)
{
    return getDoubleString(sign, num, 2, 5, 0.0001, 9999.0);
}

std::string utils::getAngJerkString(bool sign, double num)
{
    return getDoubleString(sign, num * radToDeg, 2, 4, 0.001, 9999.0);
}

double utils::getDoubleFromString(const char *str, double minMaxValue /*=0.0*/)
{
    double result;
    std::stringstream ss(str);
    ss >> result;
    if (ss.fail() || ss.bad())
    {
        if (std::string(str).front() == '-')
            result = -std::numeric_limits<double>::max();
        else
            result = std::numeric_limits<double>::max();
    }
    if (minMaxValue != 0.0)
    {
        if (result > minMaxValue)
            result = minMaxValue;
        if (result < -minMaxValue)
            result = -minMaxValue;
    }
    return result;
}

std::string utils::getIntString(bool sign, int num, int minDigits /*=1*/)
{
    std::string retVal(std::to_string(abs(num)));
    if (minDigits > 1)
    {
        while (retVal.size() < minDigits)
            retVal = "0" + retVal;
    }
    if (num < 0)
        retVal = "-" + retVal;
    else
    {
        if (sign && (num != 0))
            retVal = "+" + retVal;
    }
    return retVal;
}
