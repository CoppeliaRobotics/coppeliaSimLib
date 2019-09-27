
#include "v_repConst.h"
#include "interfaceStack.h"
#include "interfaceStackNull.h"
#include "interfaceStackBool.h"
#include "interfaceStackNumber.h"
#include "interfaceStackString.h"
#include "interfaceStackTable.h"

CInterfaceStack::CInterfaceStack()
{
}

CInterfaceStack::~CInterfaceStack()
{
    clear();
}

void CInterfaceStack::setId(int id)
{
    _interfaceStackId=id;
}

int CInterfaceStack::getId()
{
    return(_interfaceStackId);
}

int CInterfaceStack::getStackSize() const
{
    return((int)_stackObjects.size());
}

void CInterfaceStack::printContent(int cIndex) const
{
    if (cIndex<0)
    {
        printf("STACK CONTENT:\n");
        printf("--------------\n");
        for (size_t i=0;i<_stackObjects.size();i++)
        {
            printf("Item %i:\n",(int)i);
            _stackObjects[i]->printContent(4);
        }
        printf("--------------\n");
    }
    else
    {
        if (cIndex<int(_stackObjects.size()))
        {
            printf("STACK CONTENT at index %i:\n",cIndex);
            printf("--------------\n");
            _stackObjects[cIndex]->printContent(0);
            printf("--------------\n");
        }
        else
            printf("STACK CONTENT: <invalid index>\n");
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

bool CInterfaceStack::getStackNumberValue(double& theValue) const
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

bool CInterfaceStack::getStackStringValue(std::string& theValue) const
{
    if (_stackObjects.size()!=0)
    {
        CInterfaceStackObject* it=_stackObjects[_stackObjects.size()-1];
        if (it->getObjectType()==STACK_OBJECT_STRING)
        {
            int l;
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

int CInterfaceStack::_countLuaStackTableEntries(luaWrap_lua_State* L,int index)
{
    int cnt=0;
    luaWrap_lua_pushvalue(L,index); // copy of the table to the top
    luaWrap_lua_pushnil(L); // nil on top
    while (luaWrap_lua_next(L,-2)) // pops a value, then pushes a key-value pair (if table is not empty)
    { // stack now contains at -1 the value, at -2 the key, at -3 the table
        luaWrap_lua_pop(L,1); // pop 1 value (the value)
        // stack now contains at -1 the key, at -2 the table
        cnt++;
    }
    luaWrap_lua_pop(L,1);
    // Stack is now restored to what it was at function entry
    return(cnt);
}

CInterfaceStackTable* CInterfaceStack::_generateTableMapFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables)
{ // there must be a table at the given index.
    CInterfaceStackTable* table=new CInterfaceStackTable();
    luaWrap_lua_pushvalue(L,index); // copy of the table to the top
    luaWrap_lua_pushnil(L); // nil on top
    while (luaWrap_lua_next(L,-2)) // pops a value, then pushes a key-value pair (if table is not empty)
    { // stack now contains at -1 the value, at -2 the key, at -3 the table
        // copy the key:
        luaWrap_lua_pushvalue(L,-2);
        // stack now contains at -1 the key, at -2 the value, at -3 the key, and at -4 the table
        if (luaWrap_lua_stype(L,-1)==STACK_OBJECT_NUMBER)
        { // the key is a number
            double key=luaWrap_lua_tonumber(L,-1);
            CInterfaceStackObject* obj=_generateObjectFromLuaStack(L,-2,visitedTables);
            table->appendMapObject(obj,key);
        }
        else if (luaWrap_lua_stype(L,-1)==STACK_OBJECT_BOOL)
        { // the key is a bool
            bool key=luaWrap_lua_toboolean(L,-1)!=0;
            CInterfaceStackObject* obj=_generateObjectFromLuaStack(L,-2,visitedTables);
            table->appendMapObject(obj,key);
        }
        else
        { // the key is probably a string (but could also be a function or table, etc., but doesn't make sense)
            const char* key=luaWrap_lua_tostring(L,-1);
            CInterfaceStackObject* obj=_generateObjectFromLuaStack(L,-2,visitedTables);
            table->appendMapObject(obj,key);
        }
        luaWrap_lua_pop(L,2); // pop 2 values (key+value)
        // stack now contains at -1 the key, at -2 the table
    }
    luaWrap_lua_pop(L,1);
    // Stack is now restored to what it was at function entry
    return(table);
}

CInterfaceStackTable* CInterfaceStack::_generateTableArrayFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables)
{ // there must be a table at the given index.
    CInterfaceStackTable* table=new CInterfaceStackTable();
    int arraySize=int(luaWrap_lua_objlen(L,index));
    for (int i=0;i<arraySize;i++)
    {
        // Push the element i+1 of the table to the top of Lua's stack:
        luaWrap_lua_rawgeti(L,index,i+1);
        CInterfaceStackObject* obj=_generateObjectFromLuaStack(L,-1,visitedTables);
        luaWrap_lua_pop(L,1); // we pop one element from the stack;
        table->appendArrayObject(obj);
    }
    return(table);
}

CInterfaceStackObject* CInterfaceStack::_generateObjectFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables)
{ // generates just one object at the given index
    int t=luaWrap_lua_stype(L,index);
    if (t==STACK_OBJECT_NULL)
        return(new CInterfaceStackNull());
    else if (t==STACK_OBJECT_BOOL)
        return(new CInterfaceStackBool(luaWrap_lua_toboolean(L,index)!=0));
    else if (t==STACK_OBJECT_NUMBER)
        return(new CInterfaceStackNumber(luaWrap_lua_tonumber(L,index)));
    else if (t==STACK_OBJECT_STRING)
    {
        size_t l;
        const char* c=luaWrap_lua_tolstring(L,index,&l);
        return(new CInterfaceStackString(c,(int)l));
    }
    else if (t==STACK_OBJECT_TABLE)
    { // this part is more tricky:
        // Following to avoid getting trapped in circular references:
        void* p=(void*)luaWrap_lua_topointer(L,index);
        std::map<void*,bool>::iterator it=visitedTables.find(p);
        CInterfaceStackTable* table=nullptr;
        if (it!=visitedTables.end())
        { // we have a circular reference!
            table=new CInterfaceStackTable();
            table->setCircularRef();
        }
        else
        {
            visitedTables[p]=true;
            int tableValueCnt=_countLuaStackTableEntries(L,index);
            int arraySize=int(luaWrap_lua_objlen(L,index));
            if (tableValueCnt==arraySize)
            { // we have an array (or keys that go from "1" to arraySize):
                table=_generateTableArrayFromLuaStack(L,index,visitedTables);
            }
            else
            { // we have a more complex table, a map, where the keys are specific:
                table=_generateTableMapFromLuaStack(L,index,visitedTables);
            }
            it=visitedTables.find(p);
            visitedTables.erase(it);
        }
        return(table);
    }
    else
    { // following types translate to strings (i.e. can't be handled outside of the Lua state)
        void* p=(void*)luaWrap_lua_topointer(L,index);
        char num[21];
        snprintf(num,20,"%p",p);
        std::string str;
        if (t==STACK_OBJECT_USERDAT)
            str="<USERDATA ";
        else if (t==STACK_OBJECT_FUNC)
            str="<FUNCTION ";
        else if (t==STACK_OBJECT_THREAD)
            str="<THREAD ";
        else if (t==STACK_OBJECT_LIGHTUSERDAT)
            str="<LIGHTUSERDATA ";
        else
            str="<UNKNOWNTYPE ";
        str+=num;
        str+=">";
        return(new CInterfaceStackString(str.c_str(),0));
    }
}

void CInterfaceStack::buildFromLuaStack(luaWrap_lua_State* L,int fromPos,int cnt)
{ // fromPos=1 and cnt=0 by default (i.e. the whole stack)
    clear();
    int numberOfArguments=luaWrap_lua_gettop(L);
    if (fromPos>1)
        numberOfArguments-=fromPos-1;
    if (cnt>0)
        numberOfArguments=SIM_MIN(numberOfArguments,cnt);
    for (int i=fromPos;i<fromPos+numberOfArguments;i++)
    {
        std::map<void*,bool> visitedTables;
        CInterfaceStackObject* obj=_generateObjectFromLuaStack(L,i,visitedTables);
        _stackObjects.push_back(obj);
    }
}

void CInterfaceStack::buildOntoLuaStack(luaWrap_lua_State* L,bool takeOnlyTop) const
{
    if (takeOnlyTop)
    {
        if (_stackObjects.size()>0)
            _pushOntoLuaStack(L,_stackObjects[_stackObjects.size()-1]);
    }
    else
    {
        for (size_t i=0;i<_stackObjects.size();i++)
        {
            CInterfaceStackObject* obj=_stackObjects[i];
            _pushOntoLuaStack(L,obj);
        }
    }
}

void CInterfaceStack::_pushOntoLuaStack(luaWrap_lua_State* L,CInterfaceStackObject* obj) const
{
    int t=obj->getObjectType();
    if (t==STACK_OBJECT_NULL)
        luaWrap_lua_pushnil(L);
    else if (t==STACK_OBJECT_BOOL)
        luaWrap_lua_pushboolean(L,((CInterfaceStackBool*)obj)->getValue());
    else if (t==STACK_OBJECT_NUMBER)
        luaWrap_lua_pushnumber(L,((CInterfaceStackNumber*)obj)->getValue());
    else if (t==STACK_OBJECT_STRING)
    {
        int l;
        const char* str=((CInterfaceStackString*)obj)->getValue(&l);
        luaWrap_lua_pushlstring(L,str,l);
    }
    else if (t==STACK_OBJECT_TABLE)
    {
        luaWrap_lua_newtable(L);
        CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
        if (table->isTableArray())
        { // array-type table
            for (int i=0;i<table->getArraySize();i++)
            {
                CInterfaceStackObject* tobj=table->getArrayItemAtIndex(i);
                _pushOntoLuaStack(L,tobj);
                luaWrap_lua_rawseti(L,-2,i+1);
            }
        }
        else
        { // map-type table
            for (int i=0;i<table->getMapEntryCount();i++)
            {
                std::string stringKey;
                double numberKey;
                bool boolKey;
                int keyType;
                bool isNumberKey;
                CInterfaceStackObject* tobj=table->getMapItemAtIndex(i,stringKey,numberKey,boolKey,keyType);
                if (keyType==STACK_OBJECT_STRING)
                    luaWrap_lua_pushstring(L,stringKey.c_str());
                if (keyType==STACK_OBJECT_NUMBER)
                    luaWrap_lua_pushnumber(L,numberKey);
                if (keyType==STACK_OBJECT_BOOL)
                    luaWrap_lua_pushboolean(L,boolKey);
                _pushOntoLuaStack(L,tobj);
                luaWrap_lua_settable(L,-3);
            }
        }
    }
}

CInterfaceStack* CInterfaceStack::copyYourself() const
{
    CInterfaceStack* retVal=new CInterfaceStack();
    for (size_t i=0;i<_stackObjects.size();i++)
        retVal->_stackObjects.push_back(_stackObjects[i]->copyYourself());
    return(retVal);
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

bool CInterfaceStack::getStackIntArray(int* array,int count) const
{
    if (_stackObjects.size()==0)
        return(false);
    CInterfaceStackObject* obj=_stackObjects[_stackObjects.size()-1];
    if (obj->getObjectType()!=STACK_OBJECT_TABLE)
        return(false);
    CInterfaceStackTable* table=(CInterfaceStackTable*)obj;
    if (!table->isTableArray())
        return(false);
    return(table->getIntArray(array,count));
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

bool CInterfaceStack::getStackMapFloatArray(const std::string& fieldName,float* array,int count) const
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

CInterfaceStackObject* CInterfaceStack::getStackMapObject(const std::string& fieldName) const
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

bool CInterfaceStack::getStackMapBoolValue(const std::string& fieldName,bool& val) const
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

bool CInterfaceStack::replaceStackMapBoolValue(const std::string& fieldName,bool val)
{
    CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_BOOL)
        {
            ((CInterfaceStackBool*)obj)->setValue(val);
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackMapFloatValue(const std::string& fieldName,float& val) const
{
    double v;
    bool retVal=getStackMapDoubleValue(fieldName,v);
    if (retVal)
        val=float(v);
    return(retVal);
}

bool CInterfaceStack::replaceStackMapFloatValue(const std::string& fieldName,float val)
{
    return(replaceStackMapDoubleValue(fieldName,double(val)));
}

bool CInterfaceStack::getStackMapIntValue(const std::string& fieldName,int& val) const
{
    double v;
    bool retVal=getStackMapDoubleValue(fieldName,v);
    if (retVal)
    {
        if (v<0.0)
            val=int(v-0.01);
        else
            val=int(v+0.01);
    }
    return(retVal);
}

bool CInterfaceStack::replaceStackMapIntValue(const std::string& fieldName,int val)
{
    return(replaceStackMapDoubleValue(fieldName,double(val)));
}

bool CInterfaceStack::getStackMapDoubleValue(const std::string& fieldName,double& val) const
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

bool CInterfaceStack::replaceStackMapDoubleValue(const std::string& fieldName,double val)
{
    CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_NUMBER)
        {
            ((CInterfaceStackNumber*)obj)->setValue(val);
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::getStackMapStringValue(const std::string& fieldName,std::string& val) const
{
    const CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_STRING)
        {
            int l;
            const char* vv=((CInterfaceStackString*)obj)->getValue(&l);
            val.assign(vv,vv+l);
            return(true);
        }
    }
    return(false);
}

bool CInterfaceStack::replaceStackMapStringValue(const std::string& fieldName,std::string val)
{
    CInterfaceStackObject* obj=getStackMapObject(fieldName);
    if (obj!=nullptr)
    {
        if (obj->getObjectType()==STACK_OBJECT_STRING)
        {
            ((CInterfaceStackString*)obj)->setValue(val.c_str(),int(val.size()));
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
            _stackObjects.push_back(new CInterfaceStackNumber((double)(i+1)));
            _stackObjects.push_back(tableObjects[i]);
        }
    }
    else
        _stackObjects.insert(_stackObjects.end(),tableObjects.begin(),tableObjects.end());
    return(true);
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

void CInterfaceStack::pushStringOntoStack(const char* str,int l)
{
    _stackObjects.push_back(new CInterfaceStackString(str,l));
}

void CInterfaceStack::pushIntArrayTableOntoStack(const int* arr,int l)
{
    CInterfaceStackTable* table=new CInterfaceStackTable();
    table->setIntArray(arr,l);
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
    if ((obj2->getObjectType()!=STACK_OBJECT_NUMBER)&&(obj2->getObjectType()!=STACK_OBJECT_STRING))
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
