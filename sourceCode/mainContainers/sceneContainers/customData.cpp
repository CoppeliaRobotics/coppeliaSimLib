#include "vrepMainHeader.h"
#include "customData.h"
#include "tt.h"

CCustomData::CCustomData()
{
}

CCustomData::~CCustomData()
{
    removeAllData();
}

void CCustomData::simulationAboutToStart()
{

}

void CCustomData::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CCustomData::simulationEnded()
{

}

void CCustomData::serializeData(CSer &ar,const char* objectName,int scriptHandle)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            for (unsigned int i=0;i<dat.size();i++)
            {
                ar.storeDataName("Dat");
                ar << head[i];
                ar << len[i];
                for (int j=0;j<len[i];j++)
                    ar << dat[i][j];
                ar.flush();
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            removeAllData();
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName=="Dat")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int e;
                        int l;
                        ar >> e;
                        ar >> l;
                        char* dd=new char[l];
                        for (int i=0;i<l;i++)
                            ar >> dd[i];
                        dat.push_back(dd);
                        len.push_back(l);
                        head.push_back(e);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

CCustomData* CCustomData::copyYourself()
{
    CCustomData* retVal=new CCustomData();
    for (unsigned int i=0;i<dat.size();i++)
    {
        char* d=new char[len[i]];
        retVal->dat.push_back(d);
        retVal->len.push_back(len[i]);
        retVal->head.push_back(head[i]);
        for (int j=0;j<len[i];j++)
            d[j]=dat[i][j];
    }
    return(retVal);
}

void CCustomData::setData(int header,const char* data,int datLen)
{
    removeData(header);
    if ((data==nullptr)||(datLen==0)) // Following 2 lines since 2010/03/04
        return;
    char* newData=new char[datLen];
    for (int i=0;i<datLen;i++)
        newData[i]=data[i];
    dat.push_back(newData);
    len.push_back(datLen);
    head.push_back(header);
}

int CCustomData::getDataLength(int header)
{
    if (header==-1)
    { // new since 19/09/2011
        // Here we want the length of the arry that contains all the header numbers
        return((int)head.size()*sizeof(int));
    }
    else
    {
        for (unsigned int i=0;i<dat.size();i++)
        {
            if (head[i]==header)
                return(len[i]);
        }
    }
    return(0);
}

void CCustomData::getData(int header,char* data)
{
    if (header==-1)
    { // new since 19/09/2011
        // Here we want the arry that contains all the header numbers
        for (unsigned int i=0;i<head.size();i++)
            ((int*)data)[i]=head[i];
    }
    else
    {
        for (unsigned int i=0;i<dat.size();i++)
        {
            if (head[i]==header)
            {
                for (int j=0;j<len[i];j++)
                    data[j]=dat[i][j];
                break;
            }
        }
    }
}

bool CCustomData::getHeader(int index,int& header)
{
    if (index>=int(head.size()))
        return(false);
    header=head[index];
    return(true);
}

void CCustomData::removeData(int header)
{
    for (unsigned int i=0;i<dat.size();i++)
    {
        if (head[i]==header)
        {
            delete[] dat[i];
            dat.erase(dat.begin()+i);
            len.erase(len.begin()+i);
            head.erase(head.begin()+i);
            break;
        }
    }
}

void CCustomData::removeAllData()
{
    for (unsigned int i=0;i<dat.size();i++)
        delete[] dat[i];
    dat.clear();
    len.clear();
    head.clear();
}
