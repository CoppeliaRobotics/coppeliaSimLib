#pragma once

#include "ser.h"

class CCustomData 
{
public:
    CCustomData();
    virtual ~CCustomData();

    void setData(int header,const char* data,int datLen);
    int getDataLength(int header);
    void getData(int header,char* data) const;
    bool getHeader(int index,int& header) const;

    void removeData(int header);
    void removeAllData();
    CCustomData* copyYourself();

    void serializeData(CSer &ar,const char* objectName,int scriptHandle);

protected:
    std::vector<char*> dat;
    std::vector<int> len;
    std::vector<int> head;
};

