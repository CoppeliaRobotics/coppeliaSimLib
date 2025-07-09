#pragma once

#include <interfaceStackObject.h>
#include <string>

class CInterfaceStackString : public CInterfaceStackObject
{
  public:
    CInterfaceStackString(const char* str); // text string
    CInterfaceStackString(const char* str, size_t strLength, bool isBuffer); // binary string or buffer
    virtual ~CInterfaceStackString();

    CInterfaceStackObject* copyYourself() const override;
    void printContent(int spaces, std::string& buffer) const override;
    std::string getObjectData(std::string& auxInfos) const override;
    void setCborCoded(bool coded);
    void setAuxData(unsigned char opt);
    void addCborObjectData(CCbor* cborObj) const override;
    unsigned int createFromData(const char* data, unsigned char version, std::vector<CInterfaceStackObject*>& allCreatedObjects) override;
    static bool checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version);

    const char* getValue(size_t* l) const;
    void setValue(const char* str); // text string
    void setValue(const char* str, size_t strLength, bool isBuffer); // binary string or buffer
    bool isBuffer() const;
    bool isText() const;

  protected:
    std::string _value;
    bool _isText;
    bool _isBuffer; // string vs buffer
    bool _cborCoded;
};
