
#pragma once

#include "interfaceStackObject.h"

class CInterfaceStackNumber : public CInterfaceStackObject
{
public:
    CInterfaceStackNumber(double theValue);
    virtual ~CInterfaceStackNumber();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    double getValue() const;
    void setValue(double theValue);

protected:
    double _value;
};
