#include <simLib/simConst.h>
#include <interfaceStack.h>
#include <interfaceStackNull.h>
#include <interfaceStackBool.h>
#include <interfaceStackNumber.h>
#include <interfaceStackInteger.h>
#include <interfaceStackString.h>
#include <interfaceStackTable.h>
#include <interfaceStackMatrix.h>
#include <interfaceStackQuaternion.h>
#include <interfaceStackPose.h>
#include <interfaceStackHandle.h>
#include <interfaceStackColor.h>
#include <interfaceStackHandleArray.h>
#include <cbor.h>
#include <tt.h>
#include <utils.h>
#include <algorithm>

CInterfaceStack::CInterfaceStack(int a, int b, const char* c)
{ // args just to avoid direct object creation
    _objectHandle = -1;
    _objectTypeStr = "stack";
    _originalObjectTypeStr = _objectTypeStr;
    setMetaInfo("superClass: object");
}

CInterfaceStack::~CInterfaceStack()
{
    clear();
}

void CInterfaceStack::setId(int id)
{
    _objectHandle = id;
}

int CInterfaceStack::getStackSize() const
{
    return ((int)_stackObjects.size());
}

bool CInterfaceStack::insertItem(int pos, CInterfaceStackObject* item)
{
    bool retVal = false;
    if ((pos >= 0) && (pos <= int(_stackObjects.size())))
    {
        retVal = true;
        _stackObjects.insert(_stackObjects.begin() + pos, item);
    }
    return retVal;
}

void CInterfaceStack::printContent(int cIndex, std::string& buffer) const
{
    if (cIndex < 0)
    {
//        buffer = "STACK CONTENT:\n";
//        buffer += "--------------\n";
        for (size_t i = 0; i < _stackObjects.size(); i++)
        {
            buffer += "Item ";
            buffer += std::to_string(i);
            buffer += ":\n";
            _stackObjects[i]->printContent(4, buffer);
        }
//        buffer += "--------------";
    }
    else
    {
        if (cIndex < int(_stackObjects.size()))
        {
//            buffer = "STACK CONTENT at index ";
//            buffer += std::to_string(cIndex);
//            buffer += ":\n--------------\n";
            _stackObjects[cIndex]->printContent(0, buffer);
//            buffer += "--------------";
        }
//        else
//            buffer = "STACK CONTENT: <invalid index>";
    }
    if (buffer.size() > 0)
        buffer.pop_back(); // pop the carriage return
}

void CInterfaceStack::popStackValue(int cnt)
{
    if ((cnt == 0) || (cnt > (int)_stackObjects.size()))
        cnt = (int)_stackObjects.size(); // remove all
    for (int i = 0; i < cnt; i++)
    {
        delete _stackObjects[_stackObjects.size() - 1];
        _stackObjects.pop_back();
    }
}

bool CInterfaceStack::moveStackItemToTop(int cIndex)
{
    if ((cIndex < 0) || (cIndex >= (int)_stackObjects.size()))
        return (false);
    if (cIndex == (int)_stackObjects.size() - 1)
        return (true); // no movement needed
    CInterfaceStackObject* it = _stackObjects[cIndex];
    _stackObjects.erase(_stackObjects.begin() + cIndex);
    _stackObjects.push_back(it);
    return (true);
}

int CInterfaceStack::getStackItemType(int cIndex)
{
    if ((cIndex < 0) || (cIndex >= (int)_stackObjects.size()))
        return (-1);
    CInterfaceStackObject* it = _stackObjects[cIndex];
    return (it->getObjectType());
}

int CInterfaceStack::getStackStringType(int cIndex)
{
    int retVal = -1;
    if ((cIndex >= 0) && (cIndex < (int)_stackObjects.size()))
    {
        CInterfaceStackObject* it = _stackObjects[cIndex];
        if (it->getObjectType() == sim_stackitem_string)
        {
            CInterfaceStackString* str = (CInterfaceStackString*)it;
            retVal = sim_string_binary;
            if (str->isText())
                retVal = sim_string_text;
            if (str->isBuffer())
                retVal = sim_string_buffer;
        }
    }
    return retVal;
}

CInterfaceStackObject* CInterfaceStack::getStackObjectFromIndex(size_t index) const
{ // zero-based index
    if ((_stackObjects.size() != 0) && (index < _stackObjects.size()))
        return (_stackObjects[index]);
    return nullptr;
}

bool CInterfaceStack::replaceStackObjectFromIndex(size_t index, CInterfaceStackObject* obj)
{
    bool retVal = false;
    if ((_stackObjects.size() != 0) && (index < _stackObjects.size()))
    {
        delete _stackObjects[index];
        _stackObjects[index] = obj;
        retVal = true;
    }
    return retVal;
}

CInterfaceStackObject* CInterfaceStack::detachStackObjectFromIndex(size_t index)
{ // zero-based index
    CInterfaceStackObject* retVal = nullptr;
    if ((_stackObjects.size() != 0) && (index < _stackObjects.size()))
    {
        retVal = _stackObjects[index];
        _stackObjects.erase(_stackObjects.begin() + index);
    }
    return retVal;
}

bool CInterfaceStack::getStackBoolValue(bool& theValue) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_bool)
        {
            theValue = ((CInterfaceStackBool*)it)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackStrictDoubleValue(double& theValue) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_double)
        {
            theValue = ((CInterfaceStackNumber*)it)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackDoubleValue(double& theValue) const
{
    bool retVal = getStackStrictDoubleValue(theValue);
    if (!retVal)
    {
        long long int v;
        retVal = getStackStrictInt64Value(v);
        if (retVal)
            theValue = (double)v;
    }
    return (retVal);
}

bool CInterfaceStack::getStackFloatValue(float& theValue) const
{
    double v;
    bool retVal = getStackDoubleValue(v);
    if (retVal)
        theValue = (float)v;
    return (retVal);
}

bool CInterfaceStack::getStackStrictInt64Value(long long int& theValue) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_integer)
        {
            theValue = ((CInterfaceStackInteger*)it)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackInt64Value(long long int& theValue) const
{
    bool retVal = getStackStrictInt64Value(theValue);
    if (!retVal)
    {
        double v;
        retVal = getStackStrictDoubleValue(v);
        if (retVal)
        {
            if (v >= 0.0)
                theValue = (long long int)(v + 0.1);
            else
                theValue = (long long int)(v - 0.1);
        }
    }
    return (retVal);
}

bool CInterfaceStack::getStackHandleValue(long long int& theValue) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_handle)
        {
            theValue = ((CInterfaceStackHandle*)it)->getValue();
            return true;
        }
    }
    return false;
}

bool CInterfaceStack::getStackInt32Value(int& theValue) const
{
    long long int v;
    bool retVal = getStackInt64Value(v);
    if (retVal)
        theValue = (int)v;
    return (retVal);
}

bool CInterfaceStack::getStackStringValue(std::string& theValue) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_string)
        {
            size_t l;
            const char* p = ((CInterfaceStackString*)it)->getValue(&l);
            theValue.assign(p, p + l);
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::isStackValueNull() const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        return (it->getObjectType() == sim_stackitem_null);
    }
    return (false);
}

int CInterfaceStack::getStackTableInfo(int infoType) const
{
    int retVal = -1; // error
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)it;
            retVal = table->getTableInfo(infoType);
        }
        else if (it->getObjectType() == sim_stackitem_string)
        { // if we have a string (text, binary or buffer), it can identify as a table too
            CInterfaceStackString* str = (CInterfaceStackString*)it;
            if (infoType == 0)
            { // array size
                size_t s;
                str->getValue(&s);
                retVal = int(s);
            }
            else
                retVal = 0;
        }
        else if (it->getObjectType() == sim_stackitem_handlearray)
        { // can also identify as a table
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)it;
            if (infoType == 0)
            { // array size
                size_t s;
                arr->getValue(&s);
                retVal = int(s);
            }
            else if (infoType == 2)
                retVal = 1;
            else if (infoType == 9)
                retVal = 1;
            else
                retVal = 0;
        }
        else if (it->getObjectType() == sim_stackitem_quaternion)
        { // can also identify as a table
            if (infoType == 0)
                retVal = 4;
            else if (infoType == 2)
                retVal = 1;
            else
                retVal = 0;
        }
        else if (it->getObjectType() == sim_stackitem_pose)
        { // can also identify as a table
            if (infoType == 0)
                retVal = 7;
            else if (infoType == 2)
                retVal = 1;
            else
                retVal = 0;
        }
        else if (it->getObjectType() == sim_stackitem_color)
        { // can also identify as a table
            if (infoType == 0)
                retVal = 3;
            else if (infoType == 2)
                retVal = 1;
            else
                retVal = 0;
        }
        else if (it->getObjectType() == sim_stackitem_matrix)
        { // can also identify as a table when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)it;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                if (infoType == 0)
                    retVal = M->cols * M->rows;
                else if (infoType == 2)
                    retVal = 1;
                else
                    retVal = 0;
            }
            else
                retVal = sim_stack_table_not_table;
        }
        else
            retVal = sim_stack_table_not_table;
    }
    return retVal;
}

void CInterfaceStack::clear()
{
    for (size_t i = 0; i < _stackObjects.size(); i++)
        delete _stackObjects[i];
    _stackObjects.clear();
}

CInterfaceStack* CInterfaceStack::copyYourself() const
{
    CInterfaceStack* retVal = new CInterfaceStack(1, 1, "");
    for (size_t i = 0; i < _stackObjects.size(); i++)
        retVal->_stackObjects.push_back(_stackObjects[i]->copyYourself());
    return (retVal);
}

void CInterfaceStack::copyFrom(const CInterfaceStack* source)
{
    clear();
    for (size_t i = 0; i < source->_stackObjects.size(); i++)
        _stackObjects.push_back(source->_stackObjects[i]->copyYourself());
}

bool CInterfaceStack::getStackUCharArray(unsigned char* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getUCharArray(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // strings can also be seen as arrays
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            size_t l = 0;
            const unsigned char* strb = (unsigned char*)str->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = strb[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        { // handle arrays can also be seen as arrays
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t l = 0;
            const long long int* v = arr->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (unsigned char)v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_quaternion)
        { // can also be seen as an array
            CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            const C4Vector* Q = q->getValue();
            double dat[4];
            Q->getData(dat, true);
            size_t ml = std::min<size_t>(4, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (unsigned char)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_pose)
        { // can also be seen as an array
            CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            const C7Vector* P = p->getValue();
            double dat[7];
            P->getData(dat, true);
            size_t ml = std::min<size_t>(7, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (unsigned char)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        { // can also be seen as an array
            CInterfaceStackColor* c = (CInterfaceStackColor*)obj;
            const float* C = c->getValue();
            size_t ml = std::min<size_t>(3, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (unsigned char)C[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        { // can also be seen as an array, when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                size_t ml = std::min<size_t>(M->cols * M->rows, size_t(count));
                for (size_t i = 0; i < ml; i++)
                    array[i] = (unsigned char)M->data[i];
                for (size_t i = ml; i < size_t(count); i++)
                    array[i] = 0;
                retVal = true;
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackInt32Array(int* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getInt32Array(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // strings can also be seen as arrays
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            size_t l = 0;
            const unsigned char* strb = (unsigned char*)str->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (int)strb[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        { // handle arrays can also be seen as arrays
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t l = 0;
            const long long int* v = arr->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (int)v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_quaternion)
        { // can also be seen as an array
            CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            const C4Vector* Q = q->getValue();
            double dat[4];
            Q->getData(dat, true);
            size_t ml = std::min<size_t>(4, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (int)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_pose)
        { // can also be seen as an array
            CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            const C7Vector* P = p->getValue();
            double dat[7];
            P->getData(dat, true);
            size_t ml = std::min<size_t>(7, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (int)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        { // can also be seen as an array
            CInterfaceStackColor* c = (CInterfaceStackColor*)obj;
            const float* C = c->getValue();
            size_t ml = std::min<size_t>(3, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (int)C[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        { // can also be seen as an array, when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                size_t ml = std::min<size_t>(M->cols * M->rows, size_t(count));
                for (size_t i = 0; i < ml; i++)
                    array[i] = (int)M->data[i];
                for (size_t i = ml; i < size_t(count); i++)
                    array[i] = 0;
                retVal = true;
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackInt64Array(long long int* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getInt64Array(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // strings can also be seen as arrays
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            size_t l = 0;
            const unsigned char* strb = (unsigned char*)str->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (long long int)strb[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        { // handle arrays can also be seen as arrays
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t l = 0;
            const long long int* v = arr->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_quaternion)
        { // can also be seen as an array
            CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            const C4Vector* Q = q->getValue();
            double dat[4];
            Q->getData(dat, true);
            size_t ml = std::min<size_t>(4, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_pose)
        { // can also be seen as an array
            CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            const C7Vector* P = p->getValue();
            double dat[7];
            P->getData(dat, true);
            size_t ml = std::min<size_t>(7, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        { // can also be seen as an array
            CInterfaceStackColor* c = (CInterfaceStackColor*)obj;
            const float* C = c->getValue();
            size_t ml = std::min<size_t>(3, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = C[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        { // can also be seen as an array, when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                size_t ml = std::min<size_t>(M->cols * M->rows, size_t(count));
                for (size_t i = 0; i < ml; i++)
                    array[i] = M->data[i];
                for (size_t i = ml; i < size_t(count); i++)
                    array[i] = 0;
                retVal = true;
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackHandleArray(long long int* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getHandleArray(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        {
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t cnt;
            const long long int* v = arr->getValue(&cnt);
            size_t ml = std::min<size_t>(cnt, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = -1;
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackTextArray(std::vector<std::string>& array) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
            {
                retVal = true;
                table->getTextArray(array);
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackFloatArray(float* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getFloatArray(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // strings can also be seen as arrays
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            size_t l = 0;
            const unsigned char* strb = (unsigned char*)str->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (float)strb[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0f;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        { // handle arrays can also be seen as arrays
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t l = 0;
            const long long int* v = arr->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (float)v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0f;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_quaternion)
        { // can also be seen as an array
            CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            const C4Vector* Q = q->getValue();
            double dat[4];
            Q->getData(dat, true);
            size_t ml = std::min<size_t>(4, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (float)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0f;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_pose)
        { // can also be seen as an array
            CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            const C7Vector* P = p->getValue();
            double dat[7];
            P->getData(dat, true);
            size_t ml = std::min<size_t>(7, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (float)dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0f;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        { // can also be seen as an array
            CInterfaceStackColor* c = (CInterfaceStackColor*)obj;
            const float* C = c->getValue();
            size_t ml = std::min<size_t>(3, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (float)C[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0f;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        { // can also be seen as an array, when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                size_t ml = std::min<size_t>(M->cols * M->rows, size_t(count));
                for (size_t i = 0; i < ml; i++)
                    array[i] = (float)M->data[i];
                for (size_t i = ml; i < size_t(count); i++)
                    array[i] = 0.0f;
                retVal = true;
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackDoubleArray(double* array, int count) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (table->isTableArray())
                retVal = table->getDoubleArray(array, count);
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // strings can also be seen as arrays
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            size_t l = 0;
            const unsigned char* strb = (unsigned char*)str->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (double)strb[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        { // handle arrays can also be seen as arrays
            CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t l = 0;
            const long long int* v = arr->getValue(&l);
            size_t ml = std::min<size_t>(l, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (double)v[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_quaternion)
        { // can also be seen as an array
            CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            const C4Vector* Q = q->getValue();
            double dat[4];
            Q->getData(dat, true);
            size_t ml = std::min<size_t>(4, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_pose)
        { // can also be seen as an array
            CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            const C7Vector* P = p->getValue();
            double dat[7];
            P->getData(dat, true);
            size_t ml = std::min<size_t>(7, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = dat[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        { // can also be seen as an array
            CInterfaceStackColor* c = (CInterfaceStackColor*)obj;
            const float* C = c->getValue();
            size_t ml = std::min<size_t>(3, size_t(count));
            for (size_t i = 0; i < ml; i++)
                array[i] = (double)C[i];
            for (size_t i = ml; i < size_t(count); i++)
                array[i] = 0.0;
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        { // can also be seen as an array, when one dimension is 1
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* M = m->getValue();
            if ((M->cols == 1) || (M->rows == 1))
            {
                size_t ml = std::min<size_t>(M->cols * M->rows, size_t(count));
                for (size_t i = 0; i < ml; i++)
                    array[i] = M->data[i];
                for (size_t i = ml; i < size_t(count); i++)
                    array[i] = 0.0;
                retVal = true;
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackColor(float array[3]) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_color)
        {
            const float* f = ((CInterfaceStackColor*)obj)->getValue();
            array[0] = f[0];
            array[1] = f[1];
            array[2] = f[2];
            retVal = true;
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackMatrix(CMatrix& m) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_matrix)
        {
            m = ((CInterfaceStackMatrix*)obj)->getValue()[0];
            retVal = true;
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackQuaternion(C4Vector& q) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_quaternion)
        {
            q = ((CInterfaceStackQuaternion*)obj)->getValue()[0];
            retVal = true;
        }
    }
    return retVal;
}

bool CInterfaceStack::getStackPose(C7Vector& p) const
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_pose)
        {
            p = ((CInterfaceStackPose*)obj)->getValue()[0];
            retVal = true;
        }
    }
    return retVal;
}

const CMatrix* CInterfaceStack::getStackMatrix() const
{
    const CMatrix* retVal = nullptr;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_matrix)
            retVal = ((CInterfaceStackMatrix*)obj)->getValue();
    }
    return retVal;
}

const C4Vector* CInterfaceStack::getStackQuaternion() const
{
    const C4Vector* retVal = nullptr;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_quaternion)
            retVal = ((CInterfaceStackQuaternion*)obj)->getValue();
    }
    return retVal;
}

const C7Vector* CInterfaceStack::getStackPose() const
{
    const C7Vector* retVal = nullptr;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_pose)
            retVal = ((CInterfaceStackPose*)obj)->getValue();
    }
    return retVal;
}

bool CInterfaceStack::getStackMapFloatArray(const char* fieldName, float* array, int count) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* tble = (CInterfaceStackTable*)obj;
            if (tble->isTableArray() && (tble->getArraySize() > 0))
                return (tble->getFloatArray(array, count));
        }
    }
    return (false);
}

bool CInterfaceStack::getStackMapDoubleArray(const char* fieldName, double* array, int count) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* tble = (CInterfaceStackTable*)obj;
            if (tble->isTableArray() && (tble->getArraySize() > 0))
                return (tble->getDoubleArray(array, count));
        }
    }
    return (false);
}

bool CInterfaceStack::getStackMapColor(const char* fieldName, float array[3]) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_color)
        {
            const float* c = ((CInterfaceStackColor*)obj)->getValue();
            {
                array[0] = c[0];
                array[1] = c[1];
                array[2] = c[2];
                return true;
            }
        }
    }
    return false;
}

const CMatrix* CInterfaceStack::getStackMapMatrix(const char* fieldName) const
{
    const CMatrix* retVal = nullptr;
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_matrix)
        {
            CInterfaceStackMatrix* matr = (CInterfaceStackMatrix*)obj;
            retVal = matr->getValue();
        }
    }
    return retVal;
}

const C4Vector* CInterfaceStack::getStackMapQuaternion(const char* fieldName) const
{
    const C4Vector* retVal = nullptr;
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_quaternion)
        {
            CInterfaceStackQuaternion* quat = (CInterfaceStackQuaternion*)obj;
            retVal = quat->getValue();
        }
    }
    return retVal;
}

const C7Vector* CInterfaceStack::getStackMapPose(const char* fieldName) const
{
    const C7Vector* retVal = nullptr;
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_pose)
        {
            CInterfaceStackPose* pose = (CInterfaceStackPose*)obj;
            retVal = pose->getValue();
        }
    }
    return retVal;
}

CInterfaceStackObject* CInterfaceStack::getStackMapObject(const char* fieldName) const
{
    if (_stackObjects.size() == 0)
        return (nullptr);
    CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
    if (obj->getObjectType() != sim_stackitem_table)
        return (nullptr);
    CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
    if (table->isTableArray())
        return (nullptr);
    return (table->getMapObject(fieldName));
}

CInterfaceStackObject* CInterfaceStack::getStackIntMapObject(long long int key) const
{
    if (_stackObjects.size() == 0)
        return (nullptr);
    CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
    if (obj->getObjectType() != sim_stackitem_table)
        return (nullptr);
    CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
    if (table->isTableArray())
        return (nullptr);
    return (table->getIntMapObject(key));
}

void CInterfaceStack::getStackMapKeys(std::vector<std::string>* stringKeys, std::vector<long long int>* intKeys) const
{
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            if (!table->isTableArray())
                table->getMapKeys(stringKeys, intKeys);
        }
    }
}

bool CInterfaceStack::getStackMapBoolValue(const char* fieldName, bool& val) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_bool)
        {
            val = ((CInterfaceStackBool*)obj)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackMapFloatValue(const char* fieldName, float& val) const
{
    double v;
    bool retVal = getStackMapDoubleValue(fieldName, v);
    if (retVal)
        val = float(v);
    return (retVal);
}

bool CInterfaceStack::getStackMapInt64Value(const char* fieldName, long long int& val) const
{
    bool retVal = getStackMapStrictInt64Value(fieldName, val);
    if (!retVal)
    {
        double v;
        retVal = getStackMapStrictDoubleValue(fieldName, v);
        if (retVal)
        {
            if (v >= 0.0)
                val = (long long int)(v + 0.1);
            else
                val = (long long int)(v - 0.1);
        }
    }
    return (retVal);
}

bool CInterfaceStack::getStackMapHandleValue(const char* fieldName, long long int& val) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_handle)
        {
            val = ((CInterfaceStackHandle*)obj)->getValue();
            return true;
        }
    }
    return false;

}

bool CInterfaceStack::getStackMapInt32Value(const char* fieldName, int& val) const
{
    long long int v;
    bool retVal = getStackMapInt64Value(fieldName, v);
    if (retVal)
        val = (int)v;
    return (retVal);
}

bool CInterfaceStack::getStackMapStrictDoubleValue(const char* fieldName, double& val) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_double)
        {
            val = ((CInterfaceStackNumber*)obj)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackMapStrictInt64Value(const char* fieldName, long long int& val) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_integer)
        {
            val = ((CInterfaceStackInteger*)obj)->getValue();
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::getStackMapDoubleValue(const char* fieldName, double& val) const
{
    bool retVal = getStackMapStrictDoubleValue(fieldName, val);
    if (!retVal)
    {
        long long int v;
        retVal = getStackMapStrictInt64Value(fieldName, v);
        if (retVal)
            val = (double)v;
    }
    return (retVal);
}

bool CInterfaceStack::getStackMapStringValue(const char* fieldName, std::string& val) const
{
    const CInterfaceStackObject* obj = getStackMapObject(fieldName);
    if (obj != nullptr)
    {
        if (obj->getObjectType() == sim_stackitem_string)
        {
            size_t l;
            const char* vv = ((CInterfaceStackString*)obj)->getValue(&l);
            val.assign(vv, vv + l);
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStack::unfoldStackTable()
{
    bool retVal = false;
    if (_stackObjects.size() > 0)
    {
        CInterfaceStackObject* obj = _stackObjects[_stackObjects.size() - 1];
        if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            _stackObjects.pop_back();
            bool isArray = table->isTableArray();
            std::vector<CInterfaceStackObject*> tableObjects;
            table->getAllObjectsAndClearTable(tableObjects);
            delete table;
            if (isArray)
            {
                for (size_t i = 0; i < tableObjects.size(); i++)
                {
                    _stackObjects.push_back(new CInterfaceStackInteger(i + 1));
                    _stackObjects.push_back(tableObjects[i]);
                }
            }
            else
                _stackObjects.insert(_stackObjects.end(), tableObjects.begin(), tableObjects.end());
            retVal = true;
        }
        else if (obj->getObjectType() == sim_stackitem_string)
        { // we can handle a string like a table of unsigned bytes
            CInterfaceStackString* str = (CInterfaceStackString*)obj;
            _stackObjects.pop_back();
            size_t l;
            const unsigned char* strv = (unsigned char*)str->getValue(&l);
            for (size_t i = 0; i < l; i++)
            {
                _stackObjects.push_back(new CInterfaceStackInteger(i + 1));
                _stackObjects.push_back(new CInterfaceStackInteger(strv[i]));
            }
            delete str;
            retVal = true;
        }
    }
    return retVal;
}

void CInterfaceStack::pushObjectOntoStack(CInterfaceStackObject* obj, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), obj);
    else
        _stackObjects.push_back(obj);
}

void CInterfaceStack::pushNullOntoStack(bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackNull());
    else
        _stackObjects.push_back(new CInterfaceStackNull());
}

void CInterfaceStack::pushBoolOntoStack(bool v, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackBool(v));
    else
        _stackObjects.push_back(new CInterfaceStackBool(v));
}

void CInterfaceStack::pushFloatOntoStack(float v, bool toFront /*=false*/)
{
    pushDoubleOntoStack((double)v, toFront);
}

void CInterfaceStack::pushDoubleOntoStack(double v, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackNumber(v));
    else
        _stackObjects.push_back(new CInterfaceStackNumber(v));
}

void CInterfaceStack::pushInt32OntoStack(int v, bool toFront /*=false*/)
{
    pushInt64OntoStack(v, toFront);
}

void CInterfaceStack::pushInt64OntoStack(long long int v, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackInteger(v));
    else
        _stackObjects.push_back(new CInterfaceStackInteger(v));
}

void CInterfaceStack::pushHandleOntoStack(long long int v, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackHandle(v));
    else
        _stackObjects.push_back(new CInterfaceStackHandle(v));
}

void CInterfaceStack::pushBufferOntoStack(const char* buff, size_t buffLength, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackString(buff, buffLength, true));
    else
        _stackObjects.push_back(new CInterfaceStackString(buff, buffLength, true));
}

void CInterfaceStack::pushBinaryStringOntoStack(const char* buff, size_t buffLength, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackString(buff, buffLength, false));
    else
        _stackObjects.push_back(new CInterfaceStackString(buff, buffLength, false));
}

void CInterfaceStack::pushTextOntoStack(const char* str, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackString(str));
    else
        _stackObjects.push_back(new CInterfaceStackString(str));
}

void CInterfaceStack::pushInt32ArrayOntoStack(const int* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setInt32Array(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushInt64ArrayOntoStack(const long long int* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setInt64Array(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushHandleArrayOntoStack(const long long int* arr, size_t l, bool toFront /*=false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackHandleArray(arr, l));
    else
        _stackObjects.push_back(new CInterfaceStackHandleArray(arr, l));
}

void CInterfaceStack::pushShortHandleArrayOntoStack(const int* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackHandleArray* o = new CInterfaceStackHandleArray(nullptr, 0);
    o->setValue(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), o);
    else
        _stackObjects.push_back(o);
}

void CInterfaceStack::pushUCharArrayOntoStack(const unsigned char* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setUCharArray(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushFloatArrayOntoStack(const float* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setFloatArray(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushDoubleArrayOntoStack(const double* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setDoubleArray(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushTextArrayOntoStack(const std::string* arr, size_t l, bool toFront /*=false*/)
{
    CInterfaceStackTable* table = new CInterfaceStackTable();
    table->setTextArray(arr, l);
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), table);
    else
        _stackObjects.push_back(table);
}

void CInterfaceStack::pushMatrixOntoStack(const CMatrix& matrix, bool toFront /*= false*/)
{
    pushMatrixOntoStack(matrix.data.data(), matrix.rows, matrix.cols, toFront);
}

void CInterfaceStack::pushQuaternionOntoStack(const C4Vector& quaternion, bool toFront /*= false*/)
{
    pushQuaternionOntoStack(quaternion.data, toFront);
}

void CInterfaceStack::pushPoseOntoStack(const C7Vector& pose, bool toFront /*= false*/)
{
    double p[7];
    pose.getData(p);
    pushPoseOntoStack(p, toFront);
}

void CInterfaceStack::pushMatrixOntoStack(const double* matrix, size_t rows, size_t cols, bool toFront /*= false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackMatrix(matrix, rows, cols));
    else
        _stackObjects.push_back(new CInterfaceStackMatrix(matrix, rows, cols));
}

void CInterfaceStack::pushQuaternionOntoStack(const double* q, bool toFront /*= false*/, bool xyzwLayout /*= false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackQuaternion(q, xyzwLayout));
    else
        _stackObjects.push_back(new CInterfaceStackQuaternion(q, xyzwLayout));
}

void CInterfaceStack::pushPoseOntoStack(const double* p, bool toFront /*= false*/, bool xyzqxqyqzqwLayout /*= false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackPose(p, xyzqxqyqzqwLayout));
    else
        _stackObjects.push_back(new CInterfaceStackPose(p, xyzqxqyqzqwLayout));
}

void CInterfaceStack::pushColorOntoStack(const float c[3], bool toFront /*= false*/)
{
    if (toFront)
        _stackObjects.insert(_stackObjects.begin(), new CInterfaceStackColor(c));
    else
        _stackObjects.push_back(new CInterfaceStackColor(c));
}

void CInterfaceStack::insertKeyNullIntoStackTable(const char* key)
{
    pushTextOntoStack(key);
    pushNullOntoStack();
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyBoolIntoStackTable(const char* key, bool value)
{
    pushTextOntoStack(key);
    pushBoolOntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyFloatIntoStackTable(const char* key, float value)
{
    pushTextOntoStack(key);
    pushFloatOntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyDoubleIntoStackTable(const char* key, double value)
{
    pushTextOntoStack(key);
    pushDoubleOntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyInt32IntoStackTable(const char* key, int value)
{
    pushTextOntoStack(key);
    pushInt32OntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyInt64IntoStackTable(const char* key, long long int value)
{
    pushTextOntoStack(key);
    pushInt64OntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyHandleIntoStackTable(const char* key, long long int value)
{
    pushTextOntoStack(key);
    pushHandleOntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyTextIntoStackTable(const char* key, const char* value)
{
    pushTextOntoStack(key);
    pushTextOntoStack(value);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyBinaryStringIntoStackTable(const char* key, const char* value, size_t l)
{
    pushTextOntoStack(key);
    pushBinaryStringOntoStack(value, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyBufferIntoStackTable(const char* key, const char* value, size_t l)
{
    pushTextOntoStack(key);
    pushBufferOntoStack(value, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyInt32ArrayIntoStackTable(const char* key, const int* arr, size_t l)
{
    pushTextOntoStack(key);
    pushInt32ArrayOntoStack(arr, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyInt64ArrayIntoStackTable(const char* key, const long long int* arr, size_t l)
{
    pushTextOntoStack(key);
    pushInt64ArrayOntoStack(arr, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyHandleArrayIntoStackTable(const char* key, const long long int* arr, size_t l)
{
    pushTextOntoStack(key);
    pushHandleArrayOntoStack(arr, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyFloatArrayIntoStackTable(const char* key, const float* arr, size_t l)
{
    pushTextOntoStack(key);
    pushFloatArrayOntoStack(arr, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyDoubleArrayIntoStackTable(const char* key, const double* arr, size_t l)
{
    pushTextOntoStack(key);
    pushDoubleArrayOntoStack(arr, l);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyMatrixIntoStackTable(const char* key, const double* matrix, size_t rows, size_t cols)
{
    pushTextOntoStack(key);
    pushMatrixOntoStack(matrix, rows, cols);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyQuaternionIntoStackTable(const char* key, const double* q, bool xyzwLayout /*= false*/)
{
    pushTextOntoStack(key);
    pushQuaternionOntoStack(q, false, xyzwLayout);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyPoseIntoStackTable(const char* key, const double* p, bool xyzqxqyqzqwLayout /*= false*/)
{
    pushTextOntoStack(key);
    pushPoseOntoStack(p, false, xyzqxqyqzqwLayout);
    insertDataIntoStackTable();
}

void CInterfaceStack::insertKeyColorIntoStackTable(const char* key, const float c[3])
{
    pushTextOntoStack(key);
    pushColorOntoStack(c);
    insertDataIntoStackTable();
}

void CInterfaceStack::pushTableOntoStack()
{
    _stackObjects.push_back(new CInterfaceStackTable());
}

bool CInterfaceStack::insertDataIntoStackTable()
{
    if (_stackObjects.size() < 3)
        return (false);
    CInterfaceStackObject* obj1 = _stackObjects[_stackObjects.size() - 3];
    CInterfaceStackObject* obj2 = _stackObjects[_stackObjects.size() - 2];
    CInterfaceStackObject* obj3 = _stackObjects[_stackObjects.size() - 1];
    if (obj1->getObjectType() != sim_stackitem_table)
        return (false);
    if ((obj2->getObjectType() != sim_stackitem_double) && (obj2->getObjectType() != sim_stackitem_integer) &&
        (obj2->getObjectType() != sim_stackitem_string) && (obj2->getObjectType() != sim_stackitem_bool))
        return (false);
    CInterfaceStackTable* table = (CInterfaceStackTable*)obj1;
    table->appendArrayOrMapObject(obj2, obj3);
    _stackObjects.pop_back();
    _stackObjects.pop_back();
    return (true);
}

bool CInterfaceStack::pushTableFromBuffer(const char* data, unsigned int l)
{ // since 20.03.2024: empty buffer results in an empty table
    if (data != nullptr)
    {
        if (l > 0)
        {
            unsigned char version = data[0]; // the version of the pack format
            unsigned int w = 0;
            if (CInterfaceStackTable::checkCreateFromData(data + 1, w, l - 1, version))
            {
                std::vector<CInterfaceStackObject*> allCreatedObjects;
                CInterfaceStackTable* table = new CInterfaceStackTable();
                int mainDataSize = 1 + table->createFromData(data + 1, version, allCreatedObjects);
                // printf("Main dataSize: %i, total size: %i\n", mainDataSize, l);
                // handle aux. data, for now only for strings:
                if (mainDataSize < int(l))
                {
                    size_t strCnt = 0;
                    for (size_t i = 0; i < allCreatedObjects.size(); i++)
                    {
                        if (allCreatedObjects[i]->getObjectType() == sim_stackitem_string)
                        {
                            ((CInterfaceStackString*)allCreatedObjects[i])->setAuxData((unsigned char)data[mainDataSize + strCnt]);
                            strCnt++;
                        }
                    }
                }
                _stackObjects.push_back(table);
                return (true);
            }
        }
        else
        {
            _stackObjects.push_back(new CInterfaceStackTable());
            return (true);
        }
    }
    return (false);
}

std::string CInterfaceStack::getBufferFromTable() const
{
    std::string retVal; // empty string=error
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        if (it->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)it;
            // Following is the version of the pack format. 0 was when all numbers would be packed as double
            // (Lua5.1) 1-4 are reserved in order to detect other non-CoppeliaSim formats, check sim.lua
            // for details.
            // Make sure not to use any byte value that could be a first byte in a cbor string!
            unsigned char version = 5;
            std::string auxInfos;
            retVal = (char)version + table->getObjectData(auxInfos);
            // Following are auxiliary string infos (text/binary string/buffer) we append to the end, in order
            // to keep backward compatible. The aux infos can be any byte value, except for 255. One aux. value
            // per string object:
            retVal += auxInfos + (char)255;
        }
    }
    return (retVal);
}

std::string CInterfaceStack::getCborEncodedBuffer(int options) const
{
    std::string retVal; // empty string=error
    if (_stackObjects.size() != 0)
    {
        CInterfaceStackObject* it = _stackObjects[_stackObjects.size() - 1];
        CCbor cborObj(nullptr, options);
        it->addCborObjectData(&cborObj);
        retVal = cborObj.getBuff();
    }
    return (retVal);
}

int CInterfaceStack::setBoolProperty(const char*ppName, bool pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackBool(pState));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_bool(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_bool(key.c_str(), pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getBoolProperty(const char*ppName, bool& pState) const
{
    int retVal = Obj::getBoolProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string key;
        int arrIndex;
        int stackIndex;
        int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
        if (retVal == 0)
        {
            retVal = sim_propertyret_unknownproperty;
            if (key.size() == 0)
            {
                CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_bool) )
                {
                    pState = ((CInterfaceStackBool*)it)->getValue();
                    retVal = sim_propertyret_ok;
                }
            }
            else
            { // we want to read a specific array/map item
                CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
                if (obj->getObjectType() == sim_stackitem_table)
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                    CInterfaceStackObject* it = nullptr;
                    if (arrIndex >= 0)
                        it = tbl->getArrayItemAtIndex(arrIndex);
                    else
                        it = tbl->getMapObject(key.c_str());
                    if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_bool) )
                    {
                        pState = ((CInterfaceStackBool*)it)->getValue();
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setIntProperty(const char*ppName, int pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackInteger(pState));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_int32(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_int32(key.c_str(), pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getIntProperty(const char*ppName, int& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_integer) )
            {
                pState = int(((CInterfaceStackInteger*)it)->getValue());
                retVal = sim_propertyret_ok;
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_integer) )
                {
                    pState = int(((CInterfaceStackInteger*)it)->getValue());
                    retVal = sim_propertyret_ok;
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setLongProperty(const char*ppName, long long int pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackInteger(pState));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_int64(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_int64(key.c_str(), pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getLongProperty(const char*ppName, long long int& pState) const
{
    int retVal = Obj::getLongProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string key;
        int arrIndex;
        int stackIndex;
        retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
        if (retVal == 0)
        {
            retVal = sim_propertyret_unknownproperty;
            if (key.size() == 0)
            {
                CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_integer) )
                {
                    pState = ((CInterfaceStackInteger*)it)->getValue();
                    retVal = sim_propertyret_ok;
                }
            }
            else
            { // we want to read a specific array/map item
                CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
                if (obj->getObjectType() == sim_stackitem_table)
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                    CInterfaceStackObject* it = nullptr;
                    if (arrIndex >= 0)
                        it = tbl->getArrayItemAtIndex(arrIndex);
                    else
                        it = tbl->getMapObject(key.c_str());
                    if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_integer) )
                    {
                        pState = ((CInterfaceStackInteger*)it)->getValue();
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setHandleProperty(const char*ppName, long long int pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackHandle(pState));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_handle(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_handle(key.c_str(), pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getHandleProperty(const char*ppName, long long int& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_handle) )
            {
                pState = ((CInterfaceStackHandle*)it)->getValue();
                retVal = sim_propertyret_ok;
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_handle) )
                {
                    pState = ((CInterfaceStackHandle*)it)->getValue();
                    retVal = sim_propertyret_ok;
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setFloatProperty(const char*ppName, double pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackNumber(pState));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_double(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_double(key.c_str(), pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getFloatProperty(const char*ppName, double& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_double) )
            {
                pState = ((CInterfaceStackNumber*)it)->getValue();
                retVal = sim_propertyret_ok;
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_double) )
                {
                    pState = ((CInterfaceStackNumber*)it)->getValue();
                    retVal = sim_propertyret_ok;
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setStringProperty(const char*ppName, const std::string& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackString(pState.c_str()));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_text(pState.c_str());
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_text(key.c_str(), pState.c_str());
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getStringProperty(const char* ppName, std::string& pState) const
{
    int retVal = Obj::getStringProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (strcmp(propStack_content.name, ppName) == 0)
        {
            printContent(-1, pState);
            retVal = sim_propertyret_ok;
        }
    }
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string key;
        int arrIndex;
        int stackIndex;
        retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
        if (retVal == 0)
        {
            retVal = sim_propertyret_unknownproperty;
            if (key.size() == 0)
            {
                CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_string) && ((CInterfaceStackString*)it)->isText() )
                {
                    pState = ((CInterfaceStackString*)it)->getValue(nullptr);
                    retVal = sim_propertyret_ok;
                }
            }
            else
            { // we want to read a specific array/map item
                CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
                if (obj->getObjectType() == sim_stackitem_table)
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                    CInterfaceStackObject* it = nullptr;
                    if (arrIndex >= 0)
                        it = tbl->getArrayItemAtIndex(arrIndex);
                    else
                        it = tbl->getMapObject(key.c_str());
                    if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_string) && ((CInterfaceStackString*)it)->isText() )
                    {
                        pState = ((CInterfaceStackString*)it)->getValue(nullptr);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setBufferProperty(const char*ppName, const std::string& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            replaceStackObjectFromIndex(stackIndex, new CInterfaceStackString(pState.data(), pState.size(), true));
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        tbl->appendArrayObject_buffer(pState.data(), pState.size());
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        tbl->appendMapObject_buffer(key.c_str(), pState.data(), pState.size());
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getBufferProperty(const char*ppName, std::string& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_string) && (!((CInterfaceStackString*)it)->isText()) )
            {
                size_t l;
                const char* val = ((CInterfaceStackString*)it)->getValue(&l);
                pState.assign(val, val + l);
                retVal = sim_propertyret_ok;
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_string) && (!((CInterfaceStackString*)it)->isText()) )
                {
                    size_t l;
                    const char* val = ((CInterfaceStackString*)it)->getValue(&l);
                    pState.assign(val, val + l);
                    retVal = sim_propertyret_ok;
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setIntArray2Property(const char*ppName, const int* pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackTable* tbl = new CInterfaceStackTable();
            tbl->setInt32Array(pState, 2);
            replaceStackObjectFromIndex(stackIndex, tbl);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setInt32Array(pState, 2);
                        tbl->appendArrayObject(tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setInt32Array(pState, 2);
                        tbl->appendMapObject_object(key.c_str(), tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getIntArray2Property(const char*ppName, int* pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                size_t arrSize = tbl->getArraySize();
                if (tbl->areAllValuesThis(sim_stackitem_integer, true) && (arrSize == 2))
                {
                    tbl->getInt32Array(pState, int(arrSize));
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                    size_t arrSize = tbl->getArraySize();
                    if (tbl->areAllValuesThis(sim_stackitem_integer, true) && (arrSize == 2))
                    {
                        tbl->getInt32Array(pState, int(arrSize));
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setVector3Property(const char*ppName, const C3Vector& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data, 3, 1);
            replaceStackObjectFromIndex(stackIndex, m);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data, 3, 1);
                        tbl->appendArrayObject(m);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data, 3, 1);
                        tbl->appendMapObject_object(key.c_str(), m);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getVector3Property(const char*ppName, C3Vector& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_matrix)
                {
                    CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)it;
                    const CMatrix* w = m->getValue();
                    if ((w->rows == 3) && (w->cols == 1))
                    {
                        pState.setData(w->data.data());
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_matrix)
                    {
                        CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)it;
                        const CMatrix* w = m->getValue();
                        if ((w->rows == 3) && (w->cols == 1))
                        {
                            pState.setData(w->data.data());
                            retVal = sim_propertyret_ok;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setMatrixProperty(const char*ppName, const CMatrix& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data.data(), pState.rows, pState.cols);
            replaceStackObjectFromIndex(stackIndex, m);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data.data(), pState.rows, pState.cols);
                        tbl->appendArrayObject(m);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackMatrix* m = new CInterfaceStackMatrix(pState.data.data(), pState.rows, pState.cols);
                        tbl->appendMapObject_object(key.c_str(), m);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getMatrixProperty(const char*ppName, CMatrix& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_matrix)
                {
                    CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)it;
                    const CMatrix* w = m->getValue();
                    pState.set(w[0]);
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_matrix)
                    {
                        CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)it;
                        const CMatrix* w = m->getValue();
                        pState.set(w[0]);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setQuaternionProperty(const char*ppName, const C4Vector& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackQuaternion* q = new CInterfaceStackQuaternion(pState.data, false);
            replaceStackObjectFromIndex(stackIndex, q);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackQuaternion* q = new CInterfaceStackQuaternion(pState.data, false);
                        tbl->appendArrayObject(q);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackQuaternion* q = new CInterfaceStackQuaternion(pState.data, false);
                        tbl->appendMapObject_object(key.c_str(), q);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getQuaternionProperty(const char*ppName, C4Vector& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_quaternion)
                {
                    CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)it;
                    pState = q->getValue()[0];
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_quaternion)
                    {
                        CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)it;
                        pState = q->getValue()[0];
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setPoseProperty(const char*ppName, const C7Vector& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        double dat[7];
        pState.getData(dat);
        if (key.size() == 0)
        {
            CInterfaceStackPose* q = new CInterfaceStackPose(dat, false);
            replaceStackObjectFromIndex(stackIndex, q);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackPose* q = new CInterfaceStackPose(dat, false);
                        tbl->appendArrayObject(q);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackPose* q = new CInterfaceStackPose(dat, false);
                        tbl->appendMapObject_object(key.c_str(), q);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getPoseProperty(const char*ppName, C7Vector& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_pose)
                {
                    CInterfaceStackPose* q = (CInterfaceStackPose*)it;
                    pState = q->getValue()[0];
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_pose)
                    {
                        CInterfaceStackPose* q = (CInterfaceStackPose*)it;
                        pState = q->getValue()[0];
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setColorProperty(const char*ppName, const float* pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackColor* c = new CInterfaceStackColor(pState);
            replaceStackObjectFromIndex(stackIndex, c);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackColor* c = new CInterfaceStackColor(pState);
                        tbl->appendArrayObject(c);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackColor* c = new CInterfaceStackColor(pState);
                        tbl->appendMapObject_object(key.c_str(), c);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getColorProperty(const char*ppName, float* pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_color)
                {
                    CInterfaceStackColor* c = (CInterfaceStackColor*)it;
                    pState[0] = c->getValue()[0];
                    pState[1] = c->getValue()[1];
                    pState[2] = c->getValue()[2];
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_color)
                    {
                        CInterfaceStackColor* c = (CInterfaceStackColor*)it;
                        pState[0] = c->getValue()[0];
                        pState[1] = c->getValue()[1];
                        pState[2] = c->getValue()[2];
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setFloatArrayProperty(const char*ppName, const std::vector<double>& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackTable* tbl = new CInterfaceStackTable();
            tbl->setDoubleArray(pState.data(), pState.size());
            replaceStackObjectFromIndex(stackIndex, tbl);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setDoubleArray(pState.data(), pState.size());
                        tbl->appendArrayObject(tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setDoubleArray(pState.data(), pState.size());
                        tbl->appendMapObject_object(key.c_str(), tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getFloatArrayProperty(const char*ppName, std::vector<double>& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                size_t arrSize = tbl->getArraySize();
                if (tbl->areAllValuesThis(sim_stackitem_double, true))
                {
                    pState.resize(arrSize);
                    tbl->getDoubleArray(pState.data(), int(arrSize));
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                    size_t arrSize = tbl->getArraySize();
                    if (tbl->areAllValuesThis(sim_stackitem_double, true))
                    {
                        pState.resize(arrSize);
                        tbl->getDoubleArray(pState.data(), int(arrSize));
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setIntArrayProperty(const char*ppName, const std::vector<int>& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackTable* tbl = new CInterfaceStackTable();
            tbl->setInt32Array(pState.data(), pState.size());
            replaceStackObjectFromIndex(stackIndex, tbl);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setInt32Array(pState.data(), pState.size());
                        tbl->appendArrayObject(tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setInt32Array(pState.data(), pState.size());
                        tbl->appendMapObject_object(key.c_str(), tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getIntArrayProperty(const char*ppName, std::vector<int>& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                size_t arrSize = tbl->getArraySize();
                if (tbl->areAllValuesThis(sim_stackitem_integer, true))
                {
                    pState.resize(arrSize);
                    tbl->getInt32Array(pState.data(), int(arrSize));
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                    size_t arrSize = tbl->getArraySize();
                    if (tbl->areAllValuesThis(sim_stackitem_integer, true))
                    {
                        pState.resize(arrSize);
                        tbl->getInt32Array(pState.data(), int(arrSize));
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setHandleArrayProperty(const char*ppName, const std::vector<long long int>& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackHandleArray* arr = new CInterfaceStackHandleArray(pState.data(), pState.size());
            replaceStackObjectFromIndex(stackIndex, arr);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackHandleArray* arr = new CInterfaceStackHandleArray(pState.data(), pState.size());
                        tbl->appendArrayObject(arr);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackHandleArray* arr = new CInterfaceStackHandleArray(pState.data(), pState.size());
                        tbl->appendMapObject_object(key.c_str(), arr);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getHandleArrayProperty(const char*ppName, std::vector<long long int>& pState) const
{
    std::string key;
    int arrIndex;
    int stackIndex;
    int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
            if (it != nullptr)
            {
                if (it->getObjectType() == sim_stackitem_handlearray)
                {
                    CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)it;
                    size_t cnt;
                    const long long int* a = arr->getValue(&cnt);
                    pState.assign(a, a + cnt);
                    retVal = sim_propertyret_ok;
                }
            }
        }
        else
        { // we want to read a specific array/map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                CInterfaceStackObject* it = nullptr;
                if (arrIndex >= 0)
                    it = tbl->getArrayItemAtIndex(arrIndex);
                else
                    it = tbl->getMapObject(key.c_str());
                if (it != nullptr)
                {
                    if (it->getObjectType() == sim_stackitem_handlearray)
                    {
                        CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)it;
                        size_t cnt;
                        const long long int* a = arr->getValue(&cnt);
                        pState.assign(a, a + cnt);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::setStringArrayProperty(const char*ppName, const std::vector<std::string>& pState)
{
    std::string key;
    int stackIndex;
    int retVal = _getStackLocation_write(ppName, stackIndex, key);
    if (retVal == 0)
    {
        retVal = sim_propertyret_unknownproperty;
        if (key.size() == 0)
        {
            CInterfaceStackTable* tbl = new CInterfaceStackTable();
            tbl->setTextArray(pState.data(), pState.size());
            replaceStackObjectFromIndex(stackIndex, tbl);
            retVal = sim_propertyret_ok;
        }
        else
        { // we want to append an array or map item
            CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                if (key == "@arrayAppend@")
                {
                    if (tbl->isTableArray())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setTextArray(pState.data(), pState.size());
                        tbl->appendArrayObject(tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
                else
                {
                    if (tbl->isTableMap())
                    {
                        CInterfaceStackTable* tbl2 = new CInterfaceStackTable();
                        tbl2->setTextArray(pState.data(), pState.size());
                        tbl->appendMapObject_object(key.c_str(), tbl2);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getStringArrayProperty(const char*ppName, std::vector<std::string>& pState) const
{
    int retVal = Obj::getStringArrayProperty(ppName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string key;
        int arrIndex;
        int stackIndex;
        int retVal = _getStackLocation_read(ppName, stackIndex, key, arrIndex);
        if (retVal == 0)
        {
            retVal = sim_propertyret_unknownproperty;
            if (key.size() == 0)
            {
                CInterfaceStackObject* it = getStackObjectFromIndex(stackIndex);
                if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                    if (tbl->areAllValuesThis(sim_stackitem_string, true))
                    {
                        tbl->getTextArray(pState);
                        retVal = sim_propertyret_ok;
                    }
                }
            }
            else
            { // we want to read a specific array/map item
                CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
                if (obj->getObjectType() == sim_stackitem_table)
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
                    CInterfaceStackObject* it = nullptr;
                    if (arrIndex >= 0)
                        it = tbl->getArrayItemAtIndex(arrIndex);
                    else
                        it = tbl->getMapObject(key.c_str());
                    if ( (it != nullptr) && (it->getObjectType() == sim_stackitem_table) )
                    {
                        CInterfaceStackTable* tbl = (CInterfaceStackTable*)it;
                        if (tbl->areAllValuesThis(sim_stackitem_string, true))
                        {
                            tbl->getTextArray(pState);
                            retVal = sim_propertyret_ok;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::removeProperty(const char*ppName)
{
    int retVal = sim_propertyret_unknownproperty;
    int stackIndex;
    if (tt::stringToInt(ppName, stackIndex))
    {
        if (stackIndex < 0)
            stackIndex += getStackSize();
        if (stackIndex < getStackSize())
        {
            CInterfaceStackObject* obj = detachStackObjectFromIndex(stackIndex);
            if (obj != nullptr)
            {
                delete obj;
                retVal = sim_propertyret_ok;
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        appartenance = _objectTypeStr;
        for (size_t i = 0; i < allProps_stack.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_stack[i].name, pName.c_str()))
            {
                if ((allProps_stack[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_stack[i].name;
                        retVal = sim_propertyret_ok;
                        break;
                    }
                }
            }
        }
    }
    if (retVal == sim_propertyret_unknownproperty)
    {
        int flags = (sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude);
        for (size_t i = 0; i < getStackSize(); i++)
        {
            if (pName.size() == 0)
            {
                if ((flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = std::to_string(i);
                        retVal = sim_propertyret_ok;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::getPropertyInfo(const char*ppName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(ppName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        for (size_t i = 0; i < allProps_stack.size(); i++)
        {
            if (strcmp(allProps_stack[i].name, ppName) == 0)
            {
                retVal = allProps_stack[i].type;
                info = allProps_stack[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_stack[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_stack[i].shortInfoTxt.c_str()).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
    }
    if (retVal == sim_propertyret_unknownproperty)
    {
        int stackIndex;
        if (tt::stringToInt(ppName, stackIndex))
        {
            if (stackIndex < 0)
                stackIndex += getStackSize();
            if (stackIndex < getStackSize())
            {
                CInterfaceStackObject* obj = getStackObjectFromIndex(stackIndex);
                if (obj != nullptr)
                {
                    info = sim_propertyinfo_removable | sim_propertyinfo_modelhashexclude;
                    retVal = _getPropertyTypeForStackItem(obj, infoTxt);
                }
            }
        }
    }
    return retVal;
}

int CInterfaceStack::_getPropertyTypeForStackItem(const CInterfaceStackObject* obj, std::string& str, bool firstCall /*= true*/)
{
    int retVal = -1;
    str.clear();
    if (obj != nullptr)
    {
        int t = obj->getObjectType();
        if (t == sim_stackitem_null)
        {
            retVal = sim_propertytype_null;
            str = "null";
        }
        else if (t == sim_stackitem_bool)
        {
            retVal = sim_propertytype_bool;
            str = "bool";
        }
        else if (t == sim_stackitem_integer)
        {
            retVal = sim_propertytype_long;
            str = "long";
        }
        else if (t == sim_stackitem_handle)
        {
            retVal = sim_propertytype_handle;
            str = "handle";
        }
        else if (t == sim_stackitem_double)
        {
            retVal = sim_propertytype_float;
            str = "float";
        }
        else if (t == sim_stackitem_string)
        {
            CInterfaceStackString* stritem = (CInterfaceStackString*)obj;
            if (stritem->isText())
            {
                retVal = sim_propertytype_string;
                str = "string";
            }
            else
            {
                retVal = sim_propertytype_buffer;
                str = "buffer";
            }
        }
        else if (t == sim_stackitem_quaternion)
        {
            retVal = sim_propertytype_quaternion;
            str = "quaternion";
        }
        else if (t == sim_stackitem_pose)
        {
            retVal = sim_propertytype_pose;
            str = "pose";
        }
        else if (t == sim_stackitem_color)
        {
            retVal = sim_propertytype_color;
            str = "color";
        }
        else if (t == sim_stackitem_handlearray)
        {
            retVal = sim_propertytype_handlearray;
            str = "handlearray";
        }
        else if (t == sim_stackitem_matrix)
        {
            CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* mat = m->getValue();
            retVal = sim_propertytype_matrix;
            str = "matrix" + std::to_string(mat->rows) + "x" + std::to_string(mat->cols);
        }
        else if (t == sim_stackitem_table)
        {
            CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            if (tbl->isTableArray())
            {
                if (tbl->areAllValuesThis(sim_stackitem_integer, false))
                {
                    retVal = sim_propertytype_intarray;
                    str = "intarray" + std::to_string(tbl->getArraySize());
                }
                else if (tbl->areAllValuesThis(sim_stackitem_double, true))
                {
                    retVal = sim_propertytype_floatarray;
                    str = "floatarray" + std::to_string(tbl->getArraySize());
                }
                else
                { // we have a random array
                    retVal = sim_propertytype_array;
                    if (firstCall)
                    {
                        for (size_t i = 0; i < tbl->getArraySize(); i++)
                        {
                            if (i != 0)
                                str += ";";
                            std::string str2;
                            _getPropertyTypeForStackItem(tbl->getArrayItemAtIndex(i), str2, false);
                            str += std::to_string(i) + ":" + str2;
                        }
                    }
                    else
                        str = "array" + std::to_string(tbl->getArraySize());
                }
            }
            else
            { // we have a map
                retVal = sim_propertytype_map;
                if (firstCall)
                {
                    size_t i = 0;
                    while (true)
                    {
                        std::string key;
                        double numberKey;
                        long long int integerKey;
                        bool boolKey;
                        int keyType;
                        CInterfaceStackObject* it = tbl->getMapItemAtIndex(i, key, numberKey, integerKey, boolKey, keyType);
                        if (it == nullptr)
                            break;
                        if (keyType != sim_stackitem_string)
                            key = "*";
                        if (i != 0)
                            str += ";";
                        std::string str2;
                        _getPropertyTypeForStackItem(it, str2, false);
                        str += key + ":" + str2;
                        i++;
                    }
                }
                else
                    str = "map" + std::to_string(tbl->getMapEntryCount());
            }
        }
    }
    return retVal;
}

bool CInterfaceStack::_getStackLocation_write(const char* ppName, int& ind, std::string& key)
{
    int retVal = -2;
    std::string ll(ppName);
    size_t p = ll.find(".");
    if (p != std::string::npos)
    {
        key.assign(ll.begin() + p + 1, ll.end());
        if (key.size() == 0)
            key = "@arrayAppend@";
        ll.resize(p);
    }
    retVal = -1;
    if ((ll.size() == 0) || tt::stringToInt(ll.c_str(), ind))
    {
        if (ll.size() == 0)
        { // append item to stack
            if (key.size() != 0)
                pushTableOntoStack(); // in order to later be able to append an array or map item
            else
                pushNullOntoStack(); // doesn't matter what we push, will be overwritten later
            ind = getStackSize() - 1;
            retVal = 0;
        }
        else
        {
            if (ind < 0)
                ind += getStackSize();
            if (ind < getStackSize())
                retVal = 0;
        }
    }
    return retVal;
}

bool CInterfaceStack::_getStackLocation_read(const char* ppName, int& ind, std::string& key, int& arrIndex) const
{
    int retVal = -2; // error, target does not exist
    std::string ll(ppName);
    size_t p = ll.find(".");
    if (p != std::string::npos)
    {
        key.assign(ll.begin() + p + 1, ll.end());
        if (key.size() == 0)
            ll.clear(); // index/key not specified: error
        else
        {
            ll.resize(p);
            if (!tt::stringToInt(key.c_str(), arrIndex))
                arrIndex = -1; // key is a map key
        }
    }
    retVal = -1; // error
    if (tt::stringToInt(ll.c_str(), ind))
    {
        if (ind < 0)
            ind += getStackSize();
        if (ind < getStackSize())
            retVal = 0;
    }
    return retVal;
}

