
#pragma once

#include "interfaceStackObject.h"

class CInterfaceStackBool : public CInterfaceStackObject
{
public:
    CInterfaceStackBool(bool theValue);
    virtual ~CInterfaceStackBool();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    bool getValue() const;
    void setValue(bool theValue);

protected:
    bool _value;
};
