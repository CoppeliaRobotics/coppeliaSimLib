#include "simConst.h"
#include "interfaceStack.h"
#include "interfaceStackNull.h"
#include "interfaceStackBool.h"
#include "interfaceStackNumber.h"
#include "interfaceStackInteger.h"
#include "interfaceStackString.h"
#include "interfaceStackTable.h"
#include <algorithm>

CInterfaceStack::CInterfaceStack(int a,int b,const char* c)
{ // args just to avoid direct object creation
    _interfaceStackId=-1;
}

CInterfaceStack::~CInterfaceStack()
{
    clear();
}

void CInterfaceStack::setId(int id)
{
    _interfaceStackId=id;
}

int CInterfaceStack::getId() const
{
    if (_interfaceStackId==-1)
    {
        printf("CInterfaceStack::getId() returns -1! Crashing now...");
        abort();
    }
    return(_interfaceStackId);
}

int CInterfaceStack::getStackSize() const
{
    return((int)_stackObjects.size());
}

void CInterfaceStack::printContent(int cIndex,std::string& buffer) const
{
    if (cIndex<0)
    {
        buffer="STACK CONTENT:\n";
        buffer+="--------------\n";
        for (size_t i=0;i<_stackObjects.size();i++)
        {
            buffer+="Item ";
            buffer+=std::to_string(i);
            buffer+=":\n";
            _stackObjects[i]->printContent(4,buffer);
        }
        buffer+="--------------";
    }
    else
    {
        if (cIndex<int(_stackObjects.size()))
        {
            buffer="STACK CONTENT at index ";
            buffer+=std::to_string(cIndex);
            buffer+=":\n--------------\n";
            _stackObjects[cIndex]->printContent(0,buffer);
            buffer+="--------------";
        }
        else
            buffer="STACK CONTENT: <invalid index>";
    }
}

void CInterfaceStack::popStackValue(int cnt)
{
    if ( (cnt==0)||(cnt>(int)_stackObjects.size()) )
        cnt=(int)_stackObjects.size(); // remove all
    for (int i=0;i<cnt;i++)
    {
        delete _stackObjects[_stackObjects.size()-1];
        _stackObjects.pop_back();
    }
}

bool CInterfaceStack::moveStackItemToTop(int cIndex)
{
    if ( (cIndex<0)||(cIndex>=(int)_stackObjects.size()) )
        return(false);
    if (cIndex==(int)_stackObjects.size()-1)
        return(true); // no movement needed
    CInterfaceStackObject* it=_stackObjects[cIndex];
    _stackObjects.erase(_stackObjects.begin()+cIndex);
    _stackObjects.push_back(it);
    return(true);
}

CInterfaceStackObject* CInterfaceStack::getStackObjectFromIndex(size_t index) const
{
    if ( (_stackObjects.size()!=0)&&(index<_stackObjects.size()) )
        return(_stackObjects[index]);
    return(nullptr);
}

bool CInterfaceStack::getStackBoolValue(bool& theValue) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_BOOL)
        {
            theValue=((CInterfaceStackBool*)it)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackStrictNumberValue(double& theValue) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_NUMBER)
        {
            theValue=((CInterfaceStackNumber*)it)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackDoubleValue(double& theValue) const
{
    bool retVal=getStackStrictNumberValue(theValue);
    if (!retVal)
    {
        long long int v;
        retVal=getStackStrictIntegerValue(v);
        if (retVal)
            theValue=(double)v;
    }
    return(retVal);
}

bool CInterfaceStack::getStackFloatValue(float& theValue) const
{
    double v;
    bool retVal=getStackDoubleValue(v);
    if (retVal)
        theValue=(float)v;
    return(retVal);
}

bool CInterfaceStack::getStackStrictIntegerValue(long long int& theValue) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_INTEGER)
        {
            theValue=((CInterfaceStackInteger*)it)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackInt64Value(long long int& theValue) const
{
    bool retVal=getStackStrictIntegerValue(theValue);
    if (!retVal)
    {
        double v;
        retVal=getStackStrictNumberValue(v);
        if (retVal)
        {
            if (v>=0.0)
                theValue=(long long int)(v+0.1);
            else
                theValue=(long long int)(v-0.1);
        }
    }
    return(retVal);
}

bool CInterfaceStack::getStackInt32Value(int& theValue) const
{
    long long int v;
    bool retVal=getStackInt64Value(v);
    if (retVal)
        theValue=(int)v;
    return(retVal);
}

bool CInterfaceStack::getStackStringValue(std::string& theValue) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_STRING)
        {
            size_t l;
            const char* p=((CInterfaceStackString*)it)->getValue(&l);
            theValue.assign(p,p+l);
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::isStackValueNull() const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        return (it->getObjectType()==STACK_OBJECT_NULL);
    }
    return(false);
}

int CInterfaceStack::getStackTableInfo(int infoType) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_TABLE)
        {
            CInterfaceStackTable* table=(CInterfaceStackTable*)it;
            return(table->getTableInfo(infoType));
        }
        return(sim_stack_table_not_table);
    }
    return(-1); // error
}

void CInterfaceStack::clear()
{
    for (size_t i=0;i<_stackObjects.size();i++)
        delete _stackObjects[i];
    _stackObjects.clear();
}

CInterfaceStack* CInterfaceStack::copyYourself() const
{
    CInterfaceStack* retVal=new CInterfaceStack(1,1,"");
    for (size_t i=0;i<_stackObjects.size();i++)
        retVal->_stackObjects.push_back(_stackObjects[i]->copyYourself());
    return(retVal);
}

void CInterfaceStack::copyFrom(const CInterfaceStack* source)
{
    clear();
    for (size_t i=0;i<source->_stackObjects.size();i++)
        _stackObjects.push_back(source->_stackObjects[i]->copyYourself());
}

bool CInterfaceStack::getStackUCharArray(unsigned char* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getUCharArray(array,count));
}

bool CInterfaceStack::getStackInt32Array(int* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getInt32Array(array,count));
}

bool CInterfaceStack::getStackInt64Array(long long int* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getInt64Array(array,count));
}

bool CInterfaceStack::getStackFloatArray(float* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getFloatArray(array,count));
}

bool CInterfaceStack::getStackDoubleArray(double* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getDoubleArray(array,count));
}

bool CInterfaceStack::getStackMapFloatArray(const char* fieldName,float* array,int count) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_TABLE)
        {
            CInterfaceStackTable* tble=(CInterfaceStackTable*)obj;
            if ( tble->isTableArray()&&(tble->getArraySize()>0) )
                return(tble->getFloatArray(array,count));
        }
    }
    return(false);
}

CInterfaceStackObject* CInterfaceStack::getStackMapObject(const char* fieldName) const
{
    if (_stackObjects.size()==0)
        return(nullptr);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(nullptr);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (table->isTableArray())
        return(nullptr);
    return(table->getMapObject(fieldName));
}

bool CInterfaceStack::getStackMapBoolValue(const char* fieldName,bool& val) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_BOOL)
        {
            val=((CInterfaceStackBool*)obj)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackMapFloatValue(const char* fieldName,float& val) const
{
    double v;
    bool retVal=getStackMapDoubleValue(fieldName,v);
    if (retVal)
        val=float(v);
    return(retVal);
}

bool CInterfaceStack::getStackMapLongIntValue(const char* fieldName,long long int& val) const
{
    bool retVal=getStackMapStrictIntegerValue(fieldName,val);
    if (!retVal)
    {
        double v;
        retVal=getStackMapStrictNumberValue(fieldName,v);
        if (retVal)
        {
            if (v>=0.0)
                val=(long long int)(v+0.1);
            else
                val=(long long int)(v-0.1);
        }
    }
    return(retVal);
}

bool CInterfaceStack::getStackMapIntValue(const char* fieldName,int& val) const
{
    long long int v;
    bool retVal=getStackMapLongIntValue(fieldName,v);
    if (retVal)
        val=(int)v;
    return(retVal);
}

bool CInterfaceStack::getStackMapStrictNumberValue(const char* fieldName,double& val) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_NUMBER)
        {
            val=((CInterfaceStackNumber*)obj)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackMapStrictIntegerValue(const char* fieldName,long long int& val) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_INTEGER)
        {
            val=((CInterfaceStackInteger*)obj)->getValue();
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackMapDoubleValue(const char* fieldName,double& val) const
{
    bool retVal=getStackMapStrictNumberValue(fieldName,val);
    if (!retVal)
    {
        long long int v;
        retVal=getStackMapStrictIntegerValue(fieldName,v);
        if (retVal)
            val=(double)v;
    }
    return(retVal);
}

bool CInterfaceStack::getStackMapStringValue(const char* fieldName,std::string& val) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_STRING)
        {
            size_t l;
            const char* vv=((CInterfaceStackString*)obj)->getValue(&l);
            val.assign(vv,vv+l);
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::unfoldStackTable()
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    _stackObjects.pop_back();
    bool isArray=table->isTableArray();
    std::vector<CInterfaceStackObject*> tableObjects;
    table->getAllObjectsAndClearTable(tableObjects);
    delete table;
    if (isArray)
    {
        for (size_t i=0;i<tableObjects.size();i++)
        {
            _stackObjects.push_back(new CInterfaceStackInteger(i+1));
            _stackObjects.push_back(tableObjects[i]);
        }
    }
    else
        _stackObjects.insert(_stackObjects.end(),tableObjects.begin(),tableObjects.end());
    return(true);
}

void CInterfaceStack::pushObjectOntoStack(CInterfaceStackObject* obj)
{
    _stackObjects.push_back(obj);
}

void CInterfaceStack::pushNullOntoStack()
{
    _stackObjects.push_back(new CInterfaceStackNull());
}

void CInterfaceStack::pushBoolOntoStack(bool v)
{
    _stackObjects.push_back(new CInterfaceStackBool(v));
}

void CInterfaceStack::pushNumberOntoStack(double v)
{
    _stackObjects.push_back(new CInterfaceStackNumber(v));
}

void CInterfaceStack::pushInt32OntoStack(int v)
{
    _stackObjects.push_back(new CInterfaceStackInteger(v));
}

void CInterfaceStack::pushInt64OntoStack(long long int v)
{
    _stackObjects.push_back(new CInterfaceStackInteger(v));
}

void CInterfaceStack::pushStringOntoStack(const char* str,size_t l)
{
    _stackObjects.push_back(new CInterfaceStackString(str,l));
}

void CInterfaceStack::pushInt32ArrayTableOntoStack(const int* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setInt32Array(arr,l);
    _stackObjects.push_back(table);
}

void CInterfaceStack::pushInt64ArrayTableOntoStack(const long long int* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setInt64Array(arr,l);
    _stackObjects.push_back(table);
}

void CInterfaceStack::pushUCharArrayTableOntoStack(const unsigned char* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setUCharArray(arr,l);
    _stackObjects.push_back(table);
}

void CInterfaceStack::pushFloatArrayTableOntoStack(const float* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setFloatArray(arr,l);
    _stackObjects.push_back(table);
}

void CInterfaceStack::pushDoubleArrayTableOntoStack(const double* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setDoubleArray(arr,l);
    _stackObjects.push_back(table);
}

void CInterfaceStack::pushTableOntoStack()
{
    _stackObjects.push_back(new CInterfaceStackTable());
}

bool CInterfaceStack::insertDataIntoStackTable()
{
    if (_stackObjects.size()<3)
        return(false);
    CInterfaceStackObject* obj1=_stackObjects[_stackObjects.size()-3];
    CInterfaceStackObject* obj2=_stackObjects[_stackObjects.size()-2];
    CInterfaceStackObject* obj3=_stackObjects[_stackObjects.size()-1];
    if (obj1->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    if ((obj2->getObjectType()!=STACK_OBJECT_NUMBER)&&(obj2->getObjectType()!=STACK_OBJECT_INTEGER)&&(obj2->getObjectType()!=STACK_OBJECT_STRING))
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj1;
    table->appendArrayOrMapObject(obj3,obj2);
    _stackObjects.pop_back();
    _stackObjects.pop_back();
    return(true);
}

bool CInterfaceStack::pushTableFromBuffer(const char* data,unsigned int l)
{
    if ( (data!=nullptr)&&(l>0) )
    {
        //unsigned char version=data[0]; // the version of the pack format
        unsigned int w=0;
        if (CInterfaceStackTable::checkCreateFromData(data+1,w,l-1))
        {
            CInterfaceStackTable* table=new CInterfaceStackTable();
            table->createFromData(data+1);
            _stackObjects.push_back(table);
            return(true);
        }
    }
    return(false);
}

std::string CInterfaceStack::getBufferFromTable() const
{
    std::string retVal;  // empty string=error
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_TABLE)
        {
            CInterfaceStackTable* table=(CInterfaceStackTable*)it;
            retVal='m'+table->getObjectData();
            retVal[0]=0; // this is the version of the pack format
        }
    }
    return(retVal);
}
