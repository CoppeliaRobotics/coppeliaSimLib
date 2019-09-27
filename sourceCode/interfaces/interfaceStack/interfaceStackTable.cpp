
#include "v_repConst.h"
#include "interfaceStackNull.h"
#include "interfaceStackBool.h"
#include "interfaceStackNumber.h"
#include "interfaceStackString.h"
#include "interfaceStackTable.h"
#include <algorithm> // std::sort, etc.

CInterfaceStackTable::CInterfaceStackTable()
{
    _objectType=STACK_OBJECT_TABLE;
    _isTableArray=true;
    _isCircularRef=false;
}

CInterfaceStackTable::~CInterfaceStackTable()
{
    for (size_t i=0;i<_tableObjects.size();i++)
        delete _tableObjects[i];
}

bool CInterfaceStackTable::isTableArray() const
{
    return(_isTableArray);
}

int CInterfaceStackTable::getArraySize() const
{
    if (!_isTableArray)
        return(0);
    return((int)_tableObjects.size());
}

int CInterfaceStackTable::getMapEntryCount() const
{
    if (_isTableArray)
        return(0);
    return((int)_tableObjects.size()/2);
}

bool CInterfaceStackTable::isCircularRef() const
{
    return(_isCircularRef);
}

void CInterfaceStackTable::setCircularRef()
{
    _isCircularRef=true;
}


bool CInterfaceStackTable::getUCharArray(unsigned char* array,int count) const
{
    if (!_isTableArray)
        return(false);
    bool retVal=true;
    size_t c=(size_t)count;
    if (c>_tableObjects.size())
        c=_tableObjects.size();
    for (size_t i=0;i<c;i++)
    {
        int t=_tableObjects[i]->getObjectType();
        if (t==STACK_OBJECT_NUMBER)
            array[i]=(unsigned char)((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else
        {
            array[i]=0;
            retVal=false;
        }
    }
    for (size_t i=c;i<(size_t)count;i++)
        array[i]=0; // fill with zeros
    return(retVal);
}

bool CInterfaceStackTable::getIntArray(int* array,int count) const
{
    if (!_isTableArray)
        return(false);
    bool retVal=true;
    size_t c=(size_t)count;
    if (c>_tableObjects.size())
        c=_tableObjects.size();
    for (size_t i=0;i<c;i++)
    {
        int t=_tableObjects[i]->getObjectType();
        if (t==STACK_OBJECT_NUMBER)
            array[i]=(int)((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else
        {
            array[i]=0;
            retVal=false;
        }
    }
    for (size_t i=c;i<(size_t)count;i++)
        array[i]=0; // fill with zeros
    return(retVal);
}

bool CInterfaceStackTable::getFloatArray(float* array,int count) const
{
    if (!_isTableArray)
        return(false);
    bool retVal=true;
    size_t c=(size_t)count;
    if (c>_tableObjects.size())
        c=_tableObjects.size();
    for (size_t i=0;i<c;i++)
    {
        int t=_tableObjects[i]->getObjectType();
        if (t==STACK_OBJECT_NUMBER)
            array[i]=(float)((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else
        {
            array[i]=0.0;
            retVal=false;
        }
    }
    for (size_t i=c;i<(size_t)count;i++)
        array[i]=0.0; // fill with zeros
    return(retVal);
}

bool CInterfaceStackTable::getDoubleArray(double* array,int count) const
{
    if (!_isTableArray)
        return(false);
    bool retVal=true;
    size_t c=(size_t)count;
    if (c>_tableObjects.size())
        c=_tableObjects.size();
    for (size_t i=0;i<c;i++)
    {
        int t=_tableObjects[i]->getObjectType();
        if (t==STACK_OBJECT_NUMBER)
            array[i]=((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else
        {
            array[i]=0.0;
            retVal=false;
        }
    }
    for (size_t i=c;i<(size_t)count;i++)
        array[i]=0.0; // fill with zeros
    return(retVal);
}

CInterfaceStackObject* CInterfaceStackTable::getMapObject(const std::string& fieldName) const
{
    if (_isTableArray)
        return(nullptr);
    size_t p=fieldName.find('.',0);
    if (p==std::string::npos)
    {
        for (size_t i=0;i<_tableObjects.size()/2;i++)
        {
            CInterfaceStackObject* key=_tableObjects[2*i+0];
            CInterfaceStackObject* obj=_tableObjects[2*i+1];
            if (key->getObjectType()==STACK_OBJECT_STRING)
            {
                std::string theKey(((CInterfaceStackString*)key)->getValue(0));
                if (theKey.compare(fieldName)==0)
                    return(obj);
            }
        }
    }
    else
    {
        std::string tableName1(fieldName.begin(),fieldName.begin()+p);
        std::string tableName2(fieldName.begin()+p+1,fieldName.end());
        for (size_t i=0;i<_tableObjects.size()/2;i++)
        {
            CInterfaceStackObject* key=_tableObjects[2*i+0];
            CInterfaceStackObject* otherMap=_tableObjects[2*i+1];
            if ( (key->getObjectType()==STACK_OBJECT_STRING)&&(otherMap->getObjectType()==STACK_OBJECT_TABLE) )
            {
                std::string theKey(((CInterfaceStackString*)key)->getValue(0));
                CInterfaceStackTable* theTable=(CInterfaceStackTable*)otherMap;
                if (!theTable->isTableArray())
                {
                    if (theKey.compare(tableName1)==0)
                        return(theTable->getMapObject(tableName2));
                }
            }
        }
    }
    return(nullptr);
}

bool CInterfaceStackTable::removeFromKey(const CInterfaceStackObject* keyToRemove)
{
    if (_isTableArray)
        return(false);
    for (size_t i=0;i<_tableObjects.size()/2;i++)
    {
        CInterfaceStackObject* key=_tableObjects[2*i+0];
        CInterfaceStackObject* obj=_tableObjects[2*i+1];
        if ( (key->getObjectType()==STACK_OBJECT_STRING)&&(keyToRemove->getObjectType()==STACK_OBJECT_STRING) )
        {
            std::string theKey1(((CInterfaceStackString*)key)->getValue(0));
            std::string theKey2(((CInterfaceStackString*)keyToRemove)->getValue(0));
            if (theKey1.compare(theKey2)==0)
            {
                delete key;
                delete obj;
                _tableObjects.erase(_tableObjects.begin()+2*i,_tableObjects.begin()+2*i+2);
                return(true);
            }
        }
        if ( (key->getObjectType()==STACK_OBJECT_NUMBER)&&(keyToRemove->getObjectType()==STACK_OBJECT_NUMBER) )
        {
            double theKey1(((CInterfaceStackNumber*)key)->getValue());
            double theKey2(((CInterfaceStackNumber*)keyToRemove)->getValue());
            if (theKey1==theKey2)
            {
                delete key;
                delete obj;
                _tableObjects.erase(_tableObjects.begin()+2*i,_tableObjects.begin()+2*i+2);
                return(true);
            }
        }
        if ( (key->getObjectType()==STACK_OBJECT_BOOL)&&(keyToRemove->getObjectType()==STACK_OBJECT_BOOL) )
        {
            double theKey1(((CInterfaceStackBool*)key)->getValue());
            double theKey2(((CInterfaceStackBool*)keyToRemove)->getValue());
            if (theKey1==theKey2)
            {
                delete key;
                delete obj;
                _tableObjects.erase(_tableObjects.begin()+2*i,_tableObjects.begin()+2*i+2);
                return(true);
            }
        }
    }
    return(false);
}

void CInterfaceStackTable::appendArrayObject(CInterfaceStackObject* obj)
{
    _tableObjects.push_back(obj);
}

void CInterfaceStackTable::appendMapObject(CInterfaceStackObject* obj,const char* key)
{
    _isTableArray=false;
    _tableObjects.push_back(new CInterfaceStackString(key,0));
    _tableObjects.push_back(obj);
}

void CInterfaceStackTable::appendMapObject(CInterfaceStackObject* obj,double key)
{
    _isTableArray=false;
    _tableObjects.push_back(new CInterfaceStackNumber(key));
    _tableObjects.push_back(obj);
}

void CInterfaceStackTable::appendMapObject(CInterfaceStackObject* obj,bool key)
{
    _isTableArray=false;
    _tableObjects.push_back(new CInterfaceStackBool(key));
    _tableObjects.push_back(obj);
}

void CInterfaceStackTable::appendArrayOrMapObject(CInterfaceStackObject* obj,CInterfaceStackObject* key)
{   // here we basically treat this table as an array, until the key is:
    // 1) not a number, 2) not consecutive, 3) does not start at 1.
    // In that case, we then convert that table from array to map representation
    bool valueInserted=false;
    if (_isTableArray)
    {
        if (key->getObjectType()==STACK_OBJECT_NUMBER)
        {
            int ind=(int)((CInterfaceStackNumber*)key)->getValue();
            if (int(_tableObjects.size())+1==ind)
            {
                _tableObjects.push_back(obj);
                delete key;
                valueInserted=true;
            }
        }
    }
    if (!valueInserted)
    {
        if (_isTableArray)
        { // we need to convert the table to a map representation:
            size_t l=_tableObjects.size();
            for (size_t i=0;i<l;i++)
                _tableObjects.insert(_tableObjects.begin()+2*i,new CInterfaceStackNumber((double)(i+1)));
        }
        _isTableArray=false;
        removeFromKey(key); // first remove a possibly existing object with the same key
        _tableObjects.push_back(key);
        _tableObjects.push_back(obj);
    }
}

CInterfaceStackObject* CInterfaceStackTable::getArrayItemAtIndex(int ind) const
{
    if ( (!_isTableArray)||(ind>=(int)_tableObjects.size()) )
        return(nullptr);
    return(_tableObjects[ind]);
}

CInterfaceStackObject* CInterfaceStackTable::getMapItemAtIndex(int ind,std::string& stringKey,double& numberKey,bool& boolKey,int& keyType) const
{
    if ( (_isTableArray)||(ind>=(int)_tableObjects.size()/2) )
        return(nullptr);
    keyType=_tableObjects[2*ind+0]->getObjectType();
    if (keyType==STACK_OBJECT_BOOL)
    {
        CInterfaceStackBool* keyObj=(CInterfaceStackBool*)_tableObjects[2*ind+0];
        boolKey=keyObj->getValue();
    }
    if (keyType==STACK_OBJECT_NUMBER)
    {
        CInterfaceStackNumber* keyObj=(CInterfaceStackNumber*)_tableObjects[2*ind+0];
        numberKey=keyObj->getValue();
    }
    if (keyType==STACK_OBJECT_STRING)
    {
        CInterfaceStackString* keyObj=(CInterfaceStackString*)_tableObjects[2*ind+0];
        stringKey=keyObj->getValue(nullptr);
    }
    return(_tableObjects[2*ind+1]);
}

CInterfaceStackObject* CInterfaceStackTable::copyYourself() const
{
    CInterfaceStackTable* retVal=new CInterfaceStackTable();
    for (size_t i=0;i<_tableObjects.size();i++)
        retVal->_tableObjects.push_back(_tableObjects[i]->copyYourself());
    retVal->_isTableArray=_isTableArray;
    retVal->_isCircularRef=_isCircularRef;
    return(retVal);
}

void CInterfaceStackTable::getAllObjectsAndClearTable(std::vector<CInterfaceStackObject*>& allObjs)
{
    allObjs.clear();
    allObjs.assign(_tableObjects.begin(),_tableObjects.end());
    _tableObjects.clear();
    _isTableArray=true;
}

void CInterfaceStackTable::setUCharArray(const unsigned char* array,int l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (int i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber((double)array[i]));
}

void CInterfaceStackTable::setIntArray(const int* array,int l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (int i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber((double)array[i]));
}

void CInterfaceStackTable::setFloatArray(const float* array,int l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (int i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber((float)array[i]));
}

void CInterfaceStackTable::setDoubleArray(const double* array,int l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (int i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber(array[i]));
}

int CInterfaceStackTable::getTableInfo(int infoType) const
{
    if (infoType==0)
    { // array size or table type (array/map)
        if (_isCircularRef)
            return(sim_stack_table_circular_ref);
        if (_isTableArray)
            return(getArraySize());
        return(sim_stack_table_map);
    }
    int retVal=0;
    if ( (infoType==1)&&_areAllValueThis(STACK_OBJECT_NULL) )
        retVal=1;
    if ( (infoType==2)&&_areAllValueThis(STACK_OBJECT_NUMBER) )
        retVal=1;
    if ( (infoType==3)&&_areAllValueThis(STACK_OBJECT_BOOL) )
        retVal=1;
    if ( (infoType==4)&&_areAllValueThis(STACK_OBJECT_STRING) )
        retVal=1;
    if ( (infoType==5)&&_areAllValueThis(STACK_OBJECT_TABLE) )
        retVal=1;
    return(retVal);
}

bool CInterfaceStackTable::_areAllValueThis(int what) const
{
    if (_tableObjects.size()==0)
        return(true);
    if (_isTableArray)
    {
        for (size_t i=0;i<_tableObjects.size();i++)
        {
            if (_tableObjects[i]->getObjectType()!=what)
                return(false);
        }
    }
    else
    {
        for (size_t i=0;i<_tableObjects.size()/2;i++)
        {
            if (_tableObjects[2*i+1]->getObjectType()!=what)
                return(false);
        }
    }
    return(true);
}

void CInterfaceStackTable::printContent(int spaces) const
{
    for (int i=0;i<spaces;i++)
        printf(" ");
    if (_isCircularRef)
        printf("TABLE: <circular reference>\n");
    else
    {
        if (_tableObjects.size()==0)
            printf("TABLE: <empty>\n");
        else
        {
            if (_isTableArray)
            {
                printf("ARRAY TABLE (%i items, keys are omitted):\n",(int)_tableObjects.size()*2);
                for (size_t i=0;i<_tableObjects.size();i++)
                    _tableObjects[i]->printContent(spaces+4);
            }
            else
            {
                printf("MAP TABLE (%i items, key and value):\n",(int)_tableObjects.size());
                for (size_t i=0;i<_tableObjects.size();i++)
                    _tableObjects[i]->printContent(spaces+4);
            }
        }
    }
}

std::string CInterfaceStackTable::getObjectData() const
{
    std::string retVal;

    if (_isCircularRef)
        retVal=char(2);
    else
    {
        if (_isTableArray)
            retVal=char(1);
        else
            retVal=char(0);
    }
    unsigned int l=(unsigned int)_tableObjects.size();
    char* tmp=(char*)(&l);
    for (size_t i=0;i<sizeof(l);i++)
        retVal.push_back(tmp[i]);
    if (_isTableArray)
    {
        for (size_t i=0;i<_tableObjects.size();i++)
        {
            retVal.push_back((char)_tableObjects[i]->getObjectType());
            retVal+=_tableObjects[i]->getObjectData();
        }
    }
    else
    { // we need to store the map data according to a specific key order,
      // otherwise we can't compare packed tables (which is very convenient in Lua):
        int boolFalse=-1;
        int boolTrue=-1;
        std::vector<std::pair<double,int>> numberKeys;
        std::vector<std::pair<std::string,int>> stringKeys;
        for (int i=0;i<int(_tableObjects.size()/2);i++)
        {
            CInterfaceStackObject* key=_tableObjects[2*i+0];
            if (key->getObjectType()==STACK_OBJECT_BOOL)
            {
                if ( ((CInterfaceStackBool*)key)->getValue()==false )
                    boolFalse=i;
                else
                    boolTrue=i;
            }
            else if (key->getObjectType()==STACK_OBJECT_NUMBER)
                numberKeys.push_back(std::make_pair(((CInterfaceStackNumber*)key)->getValue(),i));
            else if (key->getObjectType()==STACK_OBJECT_STRING)
                stringKeys.push_back(std::make_pair(((CInterfaceStackString*)key)->getValue(nullptr),i));
            else
            { // should normally not happen. We push unordered
                retVal.push_back((char)key->getObjectType());
                retVal+=key->getObjectData();
                CInterfaceStackObject* obj=_tableObjects[2*i+1];
                retVal.push_back((char)obj->getObjectType());
                retVal+=obj->getObjectData();
            }
        }
        if (boolFalse>=0)
        {
            retVal.push_back((char)_tableObjects[2*boolFalse+0]->getObjectType());
            retVal+=_tableObjects[2*boolFalse+0]->getObjectData();
            retVal.push_back((char)_tableObjects[2*boolFalse+1]->getObjectType());
            retVal+=_tableObjects[2*boolFalse+1]->getObjectData();
        }
        if (boolTrue>=0)
        {
            retVal.push_back((char)_tableObjects[2*boolTrue+0]->getObjectType());
            retVal+=_tableObjects[2*boolTrue+0]->getObjectData();
            retVal.push_back((char)_tableObjects[2*boolTrue+1]->getObjectType());
            retVal+=_tableObjects[2*boolTrue+1]->getObjectData();
        }
        std::sort(numberKeys.begin(),numberKeys.end());
        std::sort(stringKeys.begin(),stringKeys.end());
        for (size_t i=0;i<numberKeys.size();i++)
        {
            int ind=numberKeys[i].second;
            retVal.push_back((char)_tableObjects[2*ind+0]->getObjectType());
            retVal+=_tableObjects[2*ind+0]->getObjectData();
            retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
            retVal+=_tableObjects[2*ind+1]->getObjectData();
        }
        for (size_t i=0;i<stringKeys.size();i++)
        {
            int ind=stringKeys[i].second;
            retVal.push_back((char)_tableObjects[2*ind+0]->getObjectType());
            retVal+=_tableObjects[2*ind+0]->getObjectData();
            retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
            retVal+=_tableObjects[2*ind+1]->getObjectData();
        }
    }
    return(retVal);
}

unsigned int CInterfaceStackTable::createFromData(const char* data)
{
    unsigned int retVal=0;
    _isTableArray=((data[retVal]&1)!=0);
    _isCircularRef=((data[retVal]&2)!=0);
    retVal++;
    unsigned int l;
    char* tmp=(char*)(&l);
    for (size_t i=0;i<sizeof(l);i++)
        tmp[i]=data[retVal+i];
    retVal+=sizeof(l);
    for (size_t i=0;i<l;i++)
    {
        unsigned int r=0;
        CInterfaceStackObject* obj=CInterfaceStackObject::createFromDataStatic(data+retVal,r);
        _tableObjects.push_back(obj);
        retVal+=r;
    }
    return(retVal);
}

bool CInterfaceStackTable::checkCreateFromData(const char* data,unsigned int& w,unsigned int l)
{
    unsigned int m;
    if (l<sizeof(m)+1)
        return(false);
    char* tmp=(char*)(&m);
    for (size_t i=0;i<sizeof(m);i++)
        tmp[i]=data[1+i];
    w=1+sizeof(m);
    for (size_t i=0;i<m;i++)
    {
        if (l<w+1)
            return(false);
        unsigned char t=data[w++];
        bool res=false;
        unsigned int v=0;
        if (t==STACK_OBJECT_NULL)
            res=CInterfaceStackNull::checkCreateFromData(data+w,v,l-w);
        if (t==STACK_OBJECT_NUMBER)
            res=CInterfaceStackNumber::checkCreateFromData(data+w,v,l-w);
        if (t==STACK_OBJECT_BOOL)
            res=CInterfaceStackBool::checkCreateFromData(data+w,v,l-w);
        if (t==STACK_OBJECT_STRING)
            res=CInterfaceStackString::checkCreateFromData(data+w,v,l-w);
        if (t==STACK_OBJECT_TABLE)
            res=CInterfaceStackTable::checkCreateFromData(data+w,v,l-w);
        if (!res)
            return(false);
        w+=v;
    }
    return(true);
}
