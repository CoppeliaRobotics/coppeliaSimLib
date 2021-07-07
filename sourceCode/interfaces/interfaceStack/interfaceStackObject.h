#pragma once

#include <string>

enum {  STACK_OBJECT_NULL=0,
        STACK_OBJECT_NUMBER,
        STACK_OBJECT_BOOL,
        STACK_OBJECT_STRING,
        STACK_OBJECT_TABLE,
        STACK_OBJECT_FUNC,
        STACK_OBJECT_USERDAT,
        STACK_OBJECT_THREAD,
        STACK_OBJECT_LIGHTUSERDAT,
        STACK_OBJECT_INTEGER
};

class CInterfaceStackObject
{
public:
    CInterfaceStackObject();
    virtual ~CInterfaceStackObject();

    virtual CInterfaceStackObject* copyYourself() const;
    virtual void printContent(int spaces,std::string& buffer) const;
    virtual std::string getObjectData() const;
    virtual unsigned int createFromData(const char* data);
    static CInterfaceStackObject* createFromDataStatic(const char* data,unsigned int& retOffset);

    int getObjectType() const;

protected:
    int _objectType;
};
