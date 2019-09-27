
#pragma once

#include "interfaceStackObject.h"

class CInterfaceStackNull : public CInterfaceStackObject
{
public:
    CInterfaceStackNull();
    virtual ~CInterfaceStackNull();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

protected:
};
