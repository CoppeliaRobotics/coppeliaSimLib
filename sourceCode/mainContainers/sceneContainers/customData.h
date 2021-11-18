#pragma once

#include "ser.h"
#include "interfaceStackTable.h"

struct SCustomData
{
    std::string tag;
    std::string data;
};

class CCustomData
{
public:
    CCustomData();
    virtual ~CCustomData();

    bool setData(const char* tag,const char* data,size_t dataLen);
    std::string getData(const char* tag) const;
    std::string getAllTags(size_t* cnt) const;
    size_t getDataCount() const;
    void copyYourselfInto(CCustomData& theCopy) const;
    void serializeData(CSer &ar,const char* objectName);
    void appendEventData(CInterfaceStackTable* table) const;

protected:
    std::vector<SCustomData> _data;
};

