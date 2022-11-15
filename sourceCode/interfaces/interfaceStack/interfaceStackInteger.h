#pragma once

#include "interfaceStackObject.h"

class CInterfaceStackInteger : public CInterfaceStackObject
{
public:
    CInterfaceStackInteger(long long int theValue);
    virtual ~CInterfaceStackInteger();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    void addCborObjectData(CCbor* cborObj) const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    long long int getValue() const;
    void setValue(long long int v);

protected:
    long long int _value;
};
