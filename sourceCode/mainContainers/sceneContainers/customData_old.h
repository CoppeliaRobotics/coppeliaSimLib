#pragma once

#include "ser.h"
#include "customData.h"

class CCustomData_old
{
public:
    CCustomData_old();
    virtual ~CCustomData_old();

    void setData(int header,const char* data,int datLen);
    int getDataLength(int header);
    void getData(int header,char* data) const;
    bool getHeader(int index,int& header) const;

    void removeData(int header);
    void removeAllData();
    CCustomData_old* copyYourself();

    void serializeData(CSer &ar,const char* objectName,int scriptHandle);
    void initNewFormat(CCustomData& newObj,bool objectData) const;

protected:
    static void _extractCustomDataFromBuffer(std::vector<char>& buffer,const char* dataName,std::string& extractedData);
    static void _extractCustomDataTagsFromBuffer(std::vector<char>& buffer,std::vector<std::string>& tags);

    std::vector<char*> dat;
    std::vector<int> len;
    std::vector<int> head;
};

