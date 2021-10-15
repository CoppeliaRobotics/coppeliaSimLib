#pragma once

#include "interfaceStackObject.h"
#include <string>

class CInterfaceStackString : public CInterfaceStackObject
{
public:
    CInterfaceStackString(const char* str,size_t l);
    virtual ~CInterfaceStackString();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    void addCborObjectData(CCbor* cborObj) const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    const char* getValue(size_t* l) const;

protected:
    std::string _value;
};
