#include "simConst.h"
#include "interfaceStackNull.h"
#include "interfaceStackBool.h"
#include "interfaceStackNumber.h"
#include "interfaceStackInteger.h"
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

bool CInterfaceStackTable::isEmpty() const
{
    return(_tableObjects.size()==0);
}

bool CInterfaceStackTable::isTableArray() const
{
    return(_isTableArray);
}

size_t CInterfaceStackTable::getArraySize() const
{
    if (!_isTableArray)
        return(0);
    return(_tableObjects.size());
}

size_t CInterfaceStackTable::getMapEntryCount() const
{
    if (_isTableArray)
        return(0);
    return(_tableObjects.size()/2);
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
        else if (t==STACK_OBJECT_INTEGER)
            array[i]=(unsigned char)((CInterfaceStackInteger*)_tableObjects[i])->getValue();
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

bool CInterfaceStackTable::getInt32Array(int* array,int count) const
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
        else if (t==STACK_OBJECT_INTEGER)
            array[i]=(int)((CInterfaceStackInteger*)_tableObjects[i])->getValue();
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

bool CInterfaceStackTable::getInt64Array(long long int* array,int count) const
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
            array[i]=(long long int)((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else if (t==STACK_OBJECT_INTEGER)
            array[i]=(long long int)((CInterfaceStackInteger*)_tableObjects[i])->getValue();
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

bool CInterfaceStackTable::getFloatArray(floatFloat* array,int count) const
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
            array[i]=(floatFloat)((CInterfaceStackNumber*)_tableObjects[i])->getValue();
        else if (t==STACK_OBJECT_INTEGER)
            array[i]=(floatFloat)((CInterfaceStackInteger*)_tableObjects[i])->getValue();
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
        else if (t==STACK_OBJECT_INTEGER)
            array[i]=double(((CInterfaceStackInteger*)_tableObjects[i])->getValue());
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

bool CInterfaceStackTable::containsKey(const char* fieldName) const
{
    if (_isTableArray)
        return(false);
    for (size_t i=0;i<_tableObjects.size()/2;i++)
    {
        CInterfaceStackObject* key=_tableObjects[2*i+0];
        if (key->getObjectType()==STACK_OBJECT_STRING)
        {
            std::string theKey(((CInterfaceStackString*)key)->getValue(0));
            if (theKey.compare(fieldName)==0)
                return(true);
        }
    }
    return(false);
}

CInterfaceStackObject* CInterfaceStackTable::getMapObject(const char* fieldName) const
{
    if (_isTableArray)
        return(nullptr);
    size_t p=std::string(fieldName).find('.',0);
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
        std::string tableName1(fieldName,fieldName+p);
        std::string tableName2(fieldName+p+1);
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
                        return(theTable->getMapObject(tableName2.c_str()));
                }
            }
        }
    }
    return(nullptr);
}

bool CInterfaceStackTable::removeFromKey(const char* keyToRemove)
{
    if (_isTableArray)
        return(false);
    for (size_t i=0;i<_tableObjects.size()/2;i++)
    {
        CInterfaceStackObject* key=_tableObjects[2*i+0];
        if (key->getObjectType()==STACK_OBJECT_STRING)
        {
            std::string theKey(((CInterfaceStackString*)key)->getValue(0));
            if (theKey.compare(keyToRemove)==0)
            {
                delete key;
                delete _tableObjects[2*i+1];
                _tableObjects.erase(_tableObjects.begin()+2*i,_tableObjects.begin()+2*i+2);
                return(true);
            }
        }
    }
    return(false);
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
        if ( (key->getObjectType()==STACK_OBJECT_INTEGER)&&(keyToRemove->getObjectType()==STACK_OBJECT_INTEGER) )
        {
            long long int theKey1(((CInterfaceStackInteger*)key)->getValue());
            long long int theKey2(((CInterfaceStackInteger*)keyToRemove)->getValue());
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
    long long int index;
    if (_isTableArray)
        _tableObjects.push_back(obj);
    else
    {
        index=(long long int)((_tableObjects.size()/2)+1);
        appendArrayOrMapObject(new CInterfaceStackInteger(index),obj);
    }
}

void CInterfaceStackTable::insertArrayObject(CInterfaceStackObject* obj,size_t pos)
{
    if (_isTableArray)
        _tableObjects.insert(_tableObjects.begin()+pos,obj);
}

void CInterfaceStackTable::appendMapObject(const char* key,size_t keyL,CInterfaceStackObject* obj)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,keyL),obj);
}

void CInterfaceStackTable::appendMapObject(double key,CInterfaceStackObject* obj)
{
    appendArrayOrMapObject(new CInterfaceStackNumber(key),obj);
}

void CInterfaceStackTable::appendMapObject(long long int key,CInterfaceStackObject* obj)
{
    appendArrayOrMapObject(new CInterfaceStackInteger(key),obj);
}

void CInterfaceStackTable::appendMapObject(bool key,CInterfaceStackObject* obj)
{
    appendArrayOrMapObject(new CInterfaceStackBool(key),obj);
}

void CInterfaceStackTable::appendArrayOrMapObject(CInterfaceStackObject* key,CInterfaceStackObject* obj)
{   // here we basically treat this table as an array, until the key is:
    // 1) not a number, 2) not consecutive, 3) does not start at 1.
    // In that case, we then convert that table from array to map representation
    bool valueInserted=false;
    if (_isTableArray)
    {
        if ( (key->getObjectType()==STACK_OBJECT_NUMBER)||(key->getObjectType()==STACK_OBJECT_INTEGER) )
        {
            long long int ind;
            if (key->getObjectType()==STACK_OBJECT_NUMBER)
            {
                double v=((CInterfaceStackNumber*)key)->getValue();
                ind=(long long int)v;
                if ( ((double)ind)!=v )
                    ind=-1; // this can't be an array, not a round index
            }
            else
                ind=((CInterfaceStackInteger*)key)->getValue();
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
                _tableObjects.insert(_tableObjects.begin()+2*i,new CInterfaceStackInteger(i+1));
        }
        _isTableArray=false;
        removeFromKey(key); // first remove a possibly existing object with the same key
        _tableObjects.push_back(key);
        _tableObjects.push_back(obj);
    }
}

void CInterfaceStackTable::appendArrayObject_null()
{
    appendArrayObject(new CInterfaceStackNull());
}

void CInterfaceStackTable::appendArrayObject_bool(bool value)
{
    appendArrayObject(new CInterfaceStackBool(value));
}

void CInterfaceStackTable::appendArrayObject_float(floatFloat value)
{
    appendArrayObject(new CInterfaceStackNumber((double)value));
}

void CInterfaceStackTable::appendArrayObject_double(double value)
{
    appendArrayObject(new CInterfaceStackNumber(value));
}

void CInterfaceStackTable::appendArrayObject_int32(int value)
{
    appendArrayObject(new CInterfaceStackInteger(value));
}

void CInterfaceStackTable::appendArrayObject_int64(long long int value)
{
    appendArrayObject(new CInterfaceStackInteger(value));
}

void CInterfaceStackTable::appendArrayObject_string(const char* value,size_t l,bool cborCoded/*=false*/)
{
    CInterfaceStackString* str=new CInterfaceStackString(value,l);
    str->setCborCoded(cborCoded);
    appendArrayObject(str);
}

void CInterfaceStackTable::appendArrayObject_int32Array(const int* arr,size_t l)
{
    CInterfaceStackTable* newObj=new CInterfaceStackTable();
    newObj->setInt32Array(arr,l);
    appendArrayObject(newObj);
}

void CInterfaceStackTable::appendArrayObject_int64Array(const long long int* arr,size_t l)
{
    CInterfaceStackTable* newObj=new CInterfaceStackTable();
    newObj->setInt64Array(arr,l);
    appendArrayObject(newObj);
}

void CInterfaceStackTable::appendArrayObject_floatArray(const floatFloat* arr,size_t l)
{
    CInterfaceStackTable* newObj=new CInterfaceStackTable();
    newObj->setFloatArray(arr,l);
    appendArrayObject(newObj);
}

void CInterfaceStackTable::appendArrayObject_doubleArray(const double* arr,size_t l)
{
    CInterfaceStackTable* newObj=new CInterfaceStackTable();
    newObj->setDoubleArray(arr,l);
    appendArrayObject(newObj);
}

void CInterfaceStackTable::appendMapObject_stringNull(const char* key)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackNull());
}

void CInterfaceStackTable::appendMapObject_stringBool(const char* key,bool value)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackBool(value));
}

void CInterfaceStackTable::appendMapObject_stringFloat(const char* key,floatFloat value)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackNumber(double(value)));
}

void CInterfaceStackTable::appendMapObject_stringDouble(const char* key,double value)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackNumber(value));
}

void CInterfaceStackTable::appendMapObject_stringInt32(const char* key,int value)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackInteger((long long int)(value)));
}

void CInterfaceStackTable::appendMapObject_stringInt64(const char* key,long long int value)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),new CInterfaceStackInteger(value));
}

void CInterfaceStackTable::appendMapObject_stringString(const char* key,const char* value,size_t l,bool cborCoded/*=false*/)
{
    CInterfaceStackString* str=new CInterfaceStackString(value,l);
    str->setCborCoded(cborCoded);
    appendArrayOrMapObject(new CInterfaceStackString(key,0),str);
}

void CInterfaceStackTable::appendMapObject_stringInt32Array(const char* key,const int* arr,size_t l)
{
    CInterfaceStackTable* obj=new CInterfaceStackTable();
    obj->setInt32Array(arr,l);
    appendArrayOrMapObject(new CInterfaceStackString(key,0),obj);
}

void CInterfaceStackTable::appendMapObject_stringInt64Array(const char* key,const long long int* arr,size_t l)
{
    CInterfaceStackTable* obj=new CInterfaceStackTable();
    obj->setInt64Array(arr,l);
    appendArrayOrMapObject(new CInterfaceStackString(key,0),obj);
}

void CInterfaceStackTable::appendMapObject_stringFloatArray(const char* key,const floatFloat* arr,size_t l)
{
    CInterfaceStackTable* obj=new CInterfaceStackTable();
    obj->setFloatArray(arr,l);
    appendArrayOrMapObject(new CInterfaceStackString(key,0),obj);
}

void CInterfaceStackTable::appendMapObject_stringDoubleArray(const char* key,const double* arr,size_t l)
{
    CInterfaceStackTable* obj=new CInterfaceStackTable();
    obj->setDoubleArray(arr,l);
    appendArrayOrMapObject(new CInterfaceStackString(key,0),obj);
}

void CInterfaceStackTable::appendMapObject_stringObject(const char* key,CInterfaceStackObject* obj)
{
    appendArrayOrMapObject(new CInterfaceStackString(key,0),obj);
}

void CInterfaceStackTable::removeArrayItemAtIndex(size_t ind)
{
    delete _tableObjects[ind];
    _tableObjects.erase(_tableObjects.begin()+ind);
}

CInterfaceStackObject* CInterfaceStackTable::getArrayItemAtIndex(size_t ind) const
{
    if ( (!_isTableArray)||(ind>=_tableObjects.size()) )
        return(nullptr);
    return(_tableObjects[ind]);
}

CInterfaceStackObject* CInterfaceStackTable::getMapItemAtIndex(size_t ind,std::string& stringKey,double& numberKey,long long int& integerKey,bool& boolKey,int& keyType) const
{
    if ( (_isTableArray)||(ind>=_tableObjects.size()/2) )
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
    if (keyType==STACK_OBJECT_INTEGER)
    {
        CInterfaceStackInteger* keyObj=(CInterfaceStackInteger*)_tableObjects[2*ind+0];
        integerKey=keyObj->getValue();
    }
    if (keyType==STACK_OBJECT_STRING)
    {
        CInterfaceStackString* keyObj=(CInterfaceStackString*)_tableObjects[2*ind+0];
        size_t l;
        const char* c=keyObj->getValue(&l);
        if (l>0)
            stringKey.assign(c,c+l);
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

void CInterfaceStackTable::setUCharArray(const unsigned char* array,size_t l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (size_t i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackInteger(array[i]));
}

void CInterfaceStackTable::setInt32Array(const int* array,size_t l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (size_t i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackInteger(array[i]));
}

void CInterfaceStackTable::setInt64Array(const long long int* array,size_t l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (size_t i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackInteger(array[i]));
}

void CInterfaceStackTable::setFloatArray(const floatFloat* array,size_t l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (size_t i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber((double)array[i]));
}

void CInterfaceStackTable::setDoubleArray(const double* array,size_t l)
{
    _tableObjects.clear();
    _isTableArray=true;
    for (size_t i=0;i<l;i++)
        _tableObjects.push_back(new CInterfaceStackNumber(array[i]));
}

int CInterfaceStackTable::getTableInfo(int infoType) const
{
    if (infoType==0)
    { // array size or table type (array/map)
        if (_isCircularRef)
            return(sim_stack_table_circular_ref);
        if (_isTableArray)
            return(int(getArraySize()));
        return(sim_stack_table_map);
    }
    int retVal=0;
    if ( (infoType==1)&&_areAllValueThis(STACK_OBJECT_NULL,false) )
        retVal=1;
    if ( (infoType==2)&&_areAllValueThis(STACK_OBJECT_NUMBER,true) )
        retVal=1;
    if ( (infoType==3)&&_areAllValueThis(STACK_OBJECT_BOOL,false) )
        retVal=1;
    if ( (infoType==4)&&_areAllValueThis(STACK_OBJECT_STRING,false) )
        retVal=1;
    if ( (infoType==5)&&_areAllValueThis(STACK_OBJECT_TABLE,false) )
        retVal=1;
    return(retVal);
}

bool CInterfaceStackTable::_areAllValueThis(int what,bool integerAndDoubleTolerant) const
{
    if (_tableObjects.size()==0)
        return(true);
    if (_isTableArray)
    {
        for (size_t i=0;i<_tableObjects.size();i++)
        {
            if ( integerAndDoubleTolerant&&((what==STACK_OBJECT_NUMBER)||(what==STACK_OBJECT_INTEGER)) )
            {
                if ( (_tableObjects[i]->getObjectType()!=STACK_OBJECT_NUMBER)&&(_tableObjects[i]->getObjectType()!=STACK_OBJECT_INTEGER) )
                    return(false);
            }
            else
            {
                if (_tableObjects[i]->getObjectType()!=what)
                    return(false);
            }
        }
    }
    else
    {
        for (size_t i=0;i<_tableObjects.size()/2;i++)
        {
            if ( integerAndDoubleTolerant&&((what==STACK_OBJECT_NUMBER)||(what==STACK_OBJECT_INTEGER)) )
            {
                if ( (_tableObjects[2*i+1]->getObjectType()!=STACK_OBJECT_NUMBER)&&(_tableObjects[2*i+1]->getObjectType()!=STACK_OBJECT_INTEGER) )
                    return(false);
            }
            else
            {
                if (_tableObjects[2*i+1]->getObjectType()!=what)
                    return(false);
            }
        }
    }
    return(true);
}

void CInterfaceStackTable::printContent(int spaces,std::string& buffer) const
{
    for (int i=0;i<spaces;i++)
        buffer+=" ";
    if (_isCircularRef)
        buffer+="TABLE: <circular reference>\n";
    else
    {
        if (_tableObjects.size()==0)
            buffer+="TABLE: <empty>\n";
        else
        {
            if (_isTableArray)
            {
                buffer+="ARRAY TABLE (";
                buffer+=std::to_string((int)_tableObjects.size()*2);
                buffer+=" items, keys are omitted):\n";
                for (size_t i=0;i<_tableObjects.size();i++)
                    _tableObjects[i]->printContent(spaces+4,buffer);
            }
            else
            {
                buffer+="MAP TABLE (";
                buffer+=std::to_string((int)_tableObjects.size());
                buffer+=" items, key and value):\n";
                for (size_t i=0;i<_tableObjects.size();i++)
                    _tableObjects[i]->printContent(spaces+4,buffer);
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
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[i]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[i]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[i]->getObjectType());
#endif
            retVal+=_tableObjects[i]->getObjectData();
        }
    }
    else
    { // we need to store the map data according to a specific key order,
      // otherwise we can't compare packed tables (which is very convenient in Lua):
        int boolFalse=-1;
        int boolTrue=-1;
        std::vector<std::pair<double,int>> numberKeys;
        std::vector<std::pair<long long int,int>> integerKeys;
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
            else if (key->getObjectType()==STACK_OBJECT_INTEGER)
#ifdef LUA_STACK_COMPATIBILITY_MODE
                numberKeys.push_back(std::make_pair((double)((CInterfaceStackInteger*)key)->getValue(),i));
#else
                integerKeys.push_back(std::make_pair(((CInterfaceStackInteger*)key)->getValue(),i));
#endif
            else if (key->getObjectType()==STACK_OBJECT_STRING)
                stringKeys.push_back(std::make_pair(((CInterfaceStackString*)key)->getValue(nullptr),i));
            else
            { // should normally not happen. We push unordered
                retVal.push_back((char)key->getObjectType());
                retVal+=key->getObjectData();
                CInterfaceStackObject* obj=_tableObjects[2*i+1];
#ifdef LUA_STACK_COMPATIBILITY_MODE
                if (obj->getObjectType()==STACK_OBJECT_INTEGER)
                    retVal.push_back(STACK_OBJECT_NUMBER);
                else
                    retVal.push_back((char)obj->getObjectType());
#else
                retVal.push_back((char)obj->getObjectType());
#endif
                retVal+=obj->getObjectData();
            }
        }




        if (boolFalse>=0)
        { // the key is 'false'
            retVal.push_back((char)STACK_OBJECT_BOOL);
            retVal+=_tableObjects[2*boolFalse+0]->getObjectData();
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[2*boolFalse+1]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[2*boolFalse+1]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[2*boolFalse+1]->getObjectType());
#endif
            retVal+=_tableObjects[2*boolFalse+1]->getObjectData();
        }
        if (boolTrue>=0)
        { // the key is 'true'
            retVal.push_back((char)STACK_OBJECT_BOOL);
            retVal+=_tableObjects[2*boolTrue+0]->getObjectData();
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[2*boolTrue+1]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[2*boolTrue+1]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[2*boolTrue+1]->getObjectType());
#endif
            retVal+=_tableObjects[2*boolTrue+1]->getObjectData();
        }
        std::sort(numberKeys.begin(),numberKeys.end());
        std::sort(integerKeys.begin(),integerKeys.end());
        std::sort(stringKeys.begin(),stringKeys.end());
        for (size_t i=0;i<numberKeys.size();i++)
        {
            int ind=numberKeys[i].second;
            retVal.push_back((char)STACK_OBJECT_NUMBER);
            retVal+=_tableObjects[2*ind+0]->getObjectData();
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[2*ind+1]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#endif
            retVal+=_tableObjects[2*ind+1]->getObjectData();
        }
        for (size_t i=0;i<integerKeys.size();i++)
        {
            int ind=integerKeys[i].second;
            retVal.push_back((char)STACK_OBJECT_INTEGER);
            retVal+=_tableObjects[2*ind+0]->getObjectData();
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[2*ind+1]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#endif
            retVal+=_tableObjects[2*ind+1]->getObjectData();
        }
        for (size_t i=0;i<stringKeys.size();i++)
        {
            int ind=stringKeys[i].second;
            retVal.push_back((char)STACK_OBJECT_STRING);
            retVal+=_tableObjects[2*ind+0]->getObjectData();
#ifdef LUA_STACK_COMPATIBILITY_MODE
            if (_tableObjects[2*ind+1]->getObjectType()==STACK_OBJECT_INTEGER)
                retVal.push_back((char)STACK_OBJECT_NUMBER);
            else
                retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#else
            retVal.push_back((char)_tableObjects[2*ind+1]->getObjectType());
#endif
            retVal+=_tableObjects[2*ind+1]->getObjectData();
        }
    }
    return(retVal);
}

void CInterfaceStackTable::addCborObjectData(CCbor* cborObj) const
{
    if (_isCircularRef)
        cborObj->appendMap(0);
    else
    {
        if (_isTableArray)
        {
            if (_tableObjects.size()==0)
                cborObj->appendMap(0);
            else
                cborObj->appendArray(_tableObjects.size());
        }
        else
            cborObj->appendMap(_tableObjects.size()/2);
    }
    if (_isTableArray)
    {
        for (size_t i=0;i<_tableObjects.size();i++)
            _tableObjects[i]->addCborObjectData(cborObj);
    }
    else
    { // we need to store the map data according to a specific key order,
      // otherwise we can't compare packed tables (which is very convenient in Lua):
        int boolFalse=-1;
        int boolTrue=-1;
        std::vector<std::pair<double,int>> numberKeys;
        std::vector<std::pair<long long int,int>> integerKeys;
        std::vector<std::pair<std::string,int>> stringKeys;
        for (int i=0;i<int(_tableObjects.size()/2);i++)
        {
            CInterfaceStackObject* key=_tableObjects[2*i+0];
            /*
            if (key->getObjectType()==STACK_OBJECT_BOOL)
            {
                if ( ((CInterfaceStackBool*)key)->getValue()==false )
                    boolFalse=i;
                else
                    boolTrue=i;
            }
            else if (key->getObjectType()==STACK_OBJECT_NUMBER)
                numberKeys.push_back(std::make_pair(((CInterfaceStackNumber*)key)->getValue(),i));
            else if (key->getObjectType()==STACK_OBJECT_INTEGER)
                integerKeys.push_back(std::make_pair(((CInterfaceStackInteger*)key)->getValue(),i));
            else if (key->getObjectType()==STACK_OBJECT_STRING)
                stringKeys.push_back(std::make_pair(((CInterfaceStackString*)key)->getValue(nullptr),i));
            else
            { // should normally not happen. We push unordered
                key->addCborObjectData(cborObj);
                _tableObjects[2*i+1]->addCborObjectData(cborObj);
            }
            */

            // Ignore all key-val pairs that are not strings:
            if (key->getObjectType()==STACK_OBJECT_STRING)
                stringKeys.push_back(std::make_pair(((CInterfaceStackString*)key)->getValue(nullptr),i));
        }

        /*
        if (boolFalse>=0)
        { // the key is 'false'
            _tableObjects[2*boolFalse+0]->addCborObjectData(cborObj);
            _tableObjects[2*boolFalse+1]->addCborObjectData(cborObj);
        }
        if (boolTrue>=0)
        { // the key is 'true'
            _tableObjects[2*boolTrue+0]->addCborObjectData(cborObj);
            _tableObjects[2*boolTrue+1]->addCborObjectData(cborObj);
        }
        std::sort(numberKeys.begin(),numberKeys.end());
        for (size_t i=0;i<numberKeys.size();i++)
        {
            int ind=numberKeys[i].second;
            _tableObjects[2*ind+0]->addCborObjectData(cborObj);
            _tableObjects[2*ind+1]->addCborObjectData(cborObj);
        }
        std::sort(integerKeys.begin(),integerKeys.end());
        for (size_t i=0;i<integerKeys.size();i++)
        {
            int ind=integerKeys[i].second;
            _tableObjects[2*ind+0]->addCborObjectData(cborObj);
            _tableObjects[2*ind+1]->addCborObjectData(cborObj);
        }
        */
        std::sort(stringKeys.begin(),stringKeys.end());
        for (size_t i=0;i<stringKeys.size();i++)
        {
            int ind=stringKeys[i].second;
            _tableObjects[2*ind+0]->addCborObjectData(cborObj);
            _tableObjects[2*ind+1]->addCborObjectData(cborObj);
        }
    }
    cborObj->appendBreakIfApplicable();
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
        if (t==STACK_OBJECT_INTEGER)
            res=CInterfaceStackInteger::checkCreateFromData(data+w,v,l-w);
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
