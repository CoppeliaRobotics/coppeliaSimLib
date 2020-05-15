#pragma once

#include "interfaceStackObject.h"
#include <string>

class CInterfaceStackString : public CInterfaceStackObject
{
public:
    CInterfaceStackString(const char* str,int l);
    virtual ~CInterfaceStackString();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    const char* getValue(int* l) const;
    void setValue(const char* str,int l);

protected:
    std::string _value;
};
