#pragma once

#include "luaWrapper.h"

class CInterfaceStackObject
{
public:
    CInterfaceStackObject();
    virtual ~CInterfaceStackObject();

    virtual CInterfaceStackObject* copyYourself() const;
    virtual void printContent(int spaces) const;
    virtual std::string getObjectData() const;
    virtual unsigned int createFromData(const char* data);
    static CInterfaceStackObject* createFromDataStatic(const char* data,unsigned int& retOffset);

    int getObjectType() const;

protected:
    int _objectType;
};
