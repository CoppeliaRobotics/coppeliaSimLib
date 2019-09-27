
#pragma once

#include "mainCont.h"
#include "ser.h"

class CCustomData : public CMainCont 
{
public:
    CCustomData();
    virtual ~CCustomData();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void setData(int header,const char* data,int datLen);
    int getDataLength(int header);
    void getData(int header,char* data);
    bool getHeader(int index,int& header);

    void removeData(int header);
    void removeAllData();
    CCustomData* copyYourself();

    void serializeData(CSer &ar,const char* objectName,int scriptHandle);

protected:
    std::vector<char*> dat;
    std::vector<int> len;
    std::vector<int> head;
};

