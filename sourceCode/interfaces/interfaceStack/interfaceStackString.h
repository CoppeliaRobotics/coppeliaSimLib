#pragma once

#include <interfaceStackObject.h>
#include <string>

class CInterfaceStackString : public CInterfaceStackObject
{
  public:
    CInterfaceStackString(const char* str);                                  // text string
    CInterfaceStackString(const char* str, size_t strLength, bool isBuffer); // binary string or buffer
    virtual ~CInterfaceStackString();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces, std::string& buffer) const;
    std::string getObjectData(std::string& auxInfos) const;
    void setCborCoded(bool coded);
    void setAuxData(unsigned char opt);
    void addCborObjectData(CCbor* cborObj) const;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects);
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const char* getValue(size_t* l) const;
    bool isBuffer() const;
    bool isText() const;

  protected:
    std::string _value;
    bool _isText;
    bool _isBuffer; // string vs buffer
    bool _cborCoded;
};
