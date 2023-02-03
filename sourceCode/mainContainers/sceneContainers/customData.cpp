#include <customData.h>
#include <base64.h>
#include <tt.h>

CCustomData::CCustomData()
{
}

CCustomData::~CCustomData()
{
}

bool CCustomData::setData(const char* tag,const char* data,size_t dataLen)
{
    bool diff=false;
    if ( (tag==nullptr)||(std::strlen(tag)==0) )
    {
        diff=(_data.size()>0);
        _data.clear();
    }
    else
    {
        int f=-1;
        for (size_t i=0;i<_data.size();i++)
        {
            if (_data[i].tag.compare(tag)==0)
            {
                f=int(i);
                break;
            }
        }
        if (dataLen==0)
        { // clear
            if (f!=-1)
            {
                _data.erase(_data.begin()+f);
                diff=true;
            }
        }
        else
        { // change/add
            if (f==-1)
            {
                diff=true;
                SCustomData dat;
                _data.push_back(dat);
                f=int(_data.size()-1);
            }
            else
            {
                if (dataLen==_data[size_t(f)].data.size())
                {
                    for (size_t i=0;i<dataLen;i++)
                    {
                        if (_data[size_t(f)].data[i]!=data[i])
                        {
                            diff=true;
                            break;
                        }
                    }
                }
                else
                    diff=true;
            }
            _data[size_t(f)].tag=tag;
            _data[size_t(f)].data.assign(data,dataLen);
        }
    }
    return(diff);
}

std::string CCustomData::getData(const char* tag) const
{
    std::string retVal;
    for (size_t i=0;i<_data.size();i++)
    {
        if (_data[i].tag.compare(tag)==0)
        {
            retVal=_data[i].data;
            break;
        }
    }
    return(retVal);
}

size_t CCustomData::getDataCount() const
{
    return(_data.size());
}

std::string CCustomData::getAllTags(size_t* cnt) const
{
    if (cnt!=nullptr)
        cnt[0]=_data.size();
    std::string retVal;
    for (size_t i=0;i<_data.size();i++)
    {
        retVal.insert(retVal.end(),_data[i].tag.begin(),_data[i].tag.end());
        retVal+='\0';
    }
    return(retVal);
}

void CCustomData::copyYourselfInto(CCustomData& theCopy) const
{
    theCopy._data.clear();
    for (size_t i=0;i<_data.size();i++)
        theCopy._data.push_back(_data[i]);
}

void CCustomData::serializeData(CSer &ar,const char* objectName)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            for (size_t i=0;i<_data.size();i++)
            {
                ar.storeDataName("Cdt");
                ar << _data[i].tag;
                ar << int(_data[i].data.size());
                for (size_t j=0;j<_data[i].data.size();j++)
                    ar << _data[i].data[j];
                ar.flush();
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName=="Cdt")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        SCustomData dat;
                        ar >> dat.tag;
                        int l;
                        ar >> l;
                        dat.data.resize(size_t(l));
                        for (size_t i=0;i<size_t(l);i++)
                            ar >> dat.data[i];
                        _data.push_back(dat);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            size_t totSize=0;
            for (size_t i=0;i<_data.size();i++)
                totSize+=_data[i].data.size();
            if (ar.xmlSaveDataInline(totSize))
            {
                for (size_t i=0;i<_data.size();i++)
                {
                    ar.xmlPushNewNode("data");
                    ar.xmlAddNode_string("tag",_data[i].tag.c_str());
                    std::string str(base64_encode((unsigned char*)_data[i].data.c_str(),_data[i].data.size()));
                    ar.xmlAddNode_string("data_base64Coded",str.c_str());
                    ar.xmlPopNode();
                }
            }
            else
            {
                CSer* serObj=nullptr;
                if (objectName!=nullptr)
                    serObj=ar.xmlAddNode_binFile("file",(std::string("objectCustomData_")+objectName).c_str());
                else
                    serObj=ar.xmlAddNode_binFile("file","sceneCustomData");
                serObj[0] << int(_data.size());
                for (size_t i=0;i<_data.size();i++)
                {
                    serObj[0] << _data[i].tag;
                    serObj[0] << int(_data[i].data.size());
                    for (size_t j=0;j<_data[i].data.size();j++)
                        serObj[0] << _data[i].data[j];
                }
                serObj->flush();
                serObj->writeClose();
                delete serObj;
            }
        }
        else
        {
            CSer* serObj=ar.xmlGetNode_binFile("file",false);
            if (serObj==nullptr)
            {
                if (ar.xmlPushChildNode("data",false))
                {
                    while (true)
                    {
                        SCustomData dat;
                        ar.xmlGetNode_string("tag",dat.tag);
                        std::string data;
                        ar.xmlGetNode_string("data_base64Coded",data);
                        data=base64_decode(data);
                        dat.data=data;
                        _data.push_back(dat);
                        if (!ar.xmlPushSiblingNode("data",false))
                            break;
                    }
                    ar.xmlPopNode();
                }
            }
            else
            {
                int s;
                serObj[0] >> s;
                for (size_t i=0;i<s;i++)
                {
                    SCustomData dat;
                    serObj[0] >> dat.tag;
                    int l;
                    serObj[0] >> l;
                    ar >> l;
                    dat.data.resize(size_t(l));
                    for (size_t j=0;j<size_t(l);j++)
                        ar >> dat.data[j];
                    _data.push_back(dat);
                }
                serObj->readClose();
                delete serObj;
            }
        }
    }
}

void CCustomData::appendEventData(CInterfaceStackTable* table) const
{
    for (size_t i=0;i<_data.size();i++)
        table->appendMapObject_stringString(_data[i].tag.c_str(),_data[i].data.c_str(),_data[i].data.size());
}
