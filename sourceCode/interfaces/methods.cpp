#include <methods.h>
#include <functional>
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

namespace {
    constexpr int arg_null          = sim_stackitem_null;
    constexpr int arg_double        = sim_stackitem_double;
    constexpr int arg_bool          = sim_stackitem_bool;
    constexpr int arg_string        = sim_stackitem_string;
    constexpr int arg_table         = sim_stackitem_table;
    constexpr int arg_integer       = sim_stackitem_integer;
    constexpr int arg_matrix        = sim_stackitem_matrix;
    constexpr int arg_quaternion    = sim_stackitem_quaternion;
    constexpr int arg_pose          = sim_stackitem_pose;
    constexpr int arg_handle        = sim_stackitem_handle;
    constexpr int arg_color         = sim_stackitem_color;
    constexpr int arg_vector        = 1000;
    constexpr int arg_vector3       = 1001;
    constexpr int arg_optional      = 1024;
}

std::string callMethod(int targetObj, const char* methodName, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    static std::map<std::string, std::function<std::string(int, const char*, const CInterfaceStack*, CInterfaceStack*)>> funcTable;
    if (funcTable.size() == 0)
    {
        funcTable["test"] = _method_test;
        funcTable["getPosition"] = _method_getPosition;
        funcTable["setPosition"] = _method_setPosition;
        funcTable["getQuaternion"] = _method_getQuaternion;
        funcTable["setQuaternion"] = _method_setQuaternion;
        funcTable["getPose"] = _method_getPose;
        funcTable["setPose"] = _method_setPose;
    }

    std::string retVal("__notFound__");
    if (funcTable.find(methodName) != funcTable.end())
        retVal = funcTable[methodName](targetObj, methodName, inStack, outStack);
    return retVal;
}

bool checkInputArguments(const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs)
{ // inargs: a list of desired types. Following 2 types are special:
    // arg_table is followed by 2 auxiliary values: size (-1 for any, 0 for a map) and type of content (-1 for any)
    // arg_matrix is followed by 2 auxiliary values: rows (-1 for any) and cols (-1 for any)
    // The type argument can be combined with arg_optional
    bool retVal = true;
    size_t argP = 0;
    size_t argC = 0;
    while (argP < (inargs.size()))
    {
        int rows, cols, tableSize, tableContent;
        bool optional = false;
        int desiredArgType = inargs[argP++];
        if (desiredArgType & arg_optional)
        {
            optional = true;
            desiredArgType -= arg_optional;
        }
        if (desiredArgType == arg_matrix)
        {
            rows = inargs[argP++];
            cols = inargs[argP++];
        }
        else if (desiredArgType == arg_table)
        {
            tableSize = inargs[argP++];
            tableContent = inargs[argP++];
        }
        if (inStack->getStackSize() > argC)
        {
            const CInterfaceStackObject* arg = inStack->getStackObjectFromIndex(argC++);
            int t = arg->getObjectType();
            if (desiredArgType == arg_table)
            {
                retVal = (t == arg_table);
                if (retVal)
                {
                    CInterfaceStackTable* tbl = (CInterfaceStackTable*)arg;
                    if (tableSize != 0)
                    { // we do not expect a map
                        if (tbl->isTableArray())
                        {
                            if ((tableSize == -1) || (tbl->getArraySize() == tableSize))
                            {
                                if ((tableContent != -1) && (!tbl->areAllValuesThis(tableContent, true)))
                                {
                                    retVal = false;
                                    if (errStr != nullptr)
                                    {
                                        std::string msg("bad argument #");
                                        msg += std::to_string(argC);
                                        msg += " (invalid table content).";
                                        errStr->assign(msg.c_str());
                                    }
                                }
                            }
                            else
                            {
                                retVal = false;
                                if (errStr != nullptr)
                                {
                                    std::string msg("bad argument #");
                                    msg += std::to_string(argC);
                                    msg += " (expected an array-like table of size ";
                                    msg += std::to_string(tableSize);
                                    msg += ").";
                                    errStr->assign(msg.c_str());
                                }
                            }
                        }
                        else
                        {
                            retVal = false;
                            if (errStr != nullptr)
                            {
                                std::string msg("bad argument #");
                                msg += std::to_string(argC);
                                msg += " (expected an array-like table).";
                                errStr->assign(msg.c_str());
                            }
                        }
                    }
                }
                else
                {
                    retVal = false;
                    if (errStr != nullptr)
                    {
                        std::string msg("bad argument #");
                        msg += std::to_string(argC);
                        msg += " (expected a table).";
                        errStr->assign(msg.c_str());
                    }
                }
            }
            else
            {
                if (t != desiredArgType)
                { // we have a different type than expected. Check if we tolerate that other type
                    if (desiredArgType == arg_double)
                        retVal = (t == arg_integer);
                    else if (desiredArgType == arg_handle)
                        retVal = (t == arg_integer);
                    else if (t == arg_matrix)
                    {
                        CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)arg;
                        if ( (desiredArgType == arg_vector) && (m->getValue()->cols == 1) )
                            retVal = true;
                        else if ( (desiredArgType == arg_vector3) && (m->getValue()->cols == 1) && (m->getValue()->rows == 3) )
                            retVal = true;
                    }
                    else
                    {
                        if (t == arg_table)
                        {
                            CInterfaceStackTable* tbl = (CInterfaceStackTable*)arg;
                            if (tbl->isTableArray())
                            {
                                if (tbl->areAllValuesThis(arg_double, true))
                                {
                                    if (desiredArgType == arg_quaternion)
                                        retVal = (tbl->getArraySize() == 4);
                                    else if (desiredArgType == arg_pose)
                                        retVal = (tbl->getArraySize() == 7);
                                    else if (desiredArgType == arg_color)
                                        retVal = (tbl->getArraySize() == 3);
                                    else if (desiredArgType == arg_vector)
                                        retVal = (tbl->getArraySize() >= 1);
                                    else if (desiredArgType == arg_vector3)
                                        retVal = (tbl->getArraySize() == 3);
                                    else
                                        retVal = false;
                                }
                                else
                                    retVal = false;
                            }
                            else
                                retVal = false;
                        }
                        else
                            retVal = false;
                    }
                    if (!retVal)
                    {
                        if (errStr != nullptr)
                        {
                            std::string msg("bad argument #");
                            msg += std::to_string(argC);
                            msg += " (expected ";
                            if (desiredArgType == arg_null)
                                msg += "null";
                            else if (desiredArgType == arg_double)
                                msg += "a number";
                            else if (desiredArgType == arg_bool)
                                msg += "a boolean";
                            else if (desiredArgType == arg_string)
                                msg += "a string/buffer";
                            else if (desiredArgType == arg_integer)
                                msg += "an integer";
                            else if (desiredArgType == arg_matrix)
                                msg += "a matrix";
                            else if (desiredArgType == arg_handle)
                                msg += "a handle/object";
                            else if (desiredArgType == arg_quaternion)
                                msg += "a quaternion";
                            else if (desiredArgType == arg_pose)
                                msg += "a pose";
                            else if (desiredArgType == arg_color)
                                msg += "a color";
                            else if (desiredArgType == arg_vector)
                                msg += "a vector";
                            else if (desiredArgType == arg_vector3)
                                msg += "a vector3";
                            else
                                msg += "an unknown type";
                            msg += ").";
                            errStr->assign(msg.c_str());
                        }
                    }
                }
                else
                {
                    if (desiredArgType == arg_matrix)
                    {
                        CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)arg;
                        const CMatrix* mat = m->getValue();
                        if ((mat->rows != rows) && (rows != -1))
                            retVal = false;
                        if ((mat->cols != cols) && (cols != -1))
                            retVal = false;
                        if (!retVal)
                        {
                            if (errStr != nullptr)
                            {
                                std::string msg("bad argument #");
                                msg += std::to_string(argC);
                                msg += " (expected a ";
                                if (rows > 0)
                                {
                                    if (cols > 0)
                                    {
                                        msg += std::to_string(rows);
                                        msg += "x";
                                        msg += std::to_string(cols);
                                    }
                                    else
                                    {
                                        msg += std::to_string(rows);
                                        msg += "xN";
                                    }
                                }
                                else
                                {
                                    msg += "Nx";
                                    msg += std::to_string(cols);
                                }
                                msg += " matrix).";
                                errStr->assign(msg.c_str());
                            }
                        }
                    }
                }
            }
            if (!retVal)
                break;
        }
        else
        {
            if (!optional)
            {
                if (errStr != nullptr)
                    errStr->assign(SIM_ERROR_FUNCTION_REQUIRES_MORE_ARGUMENTS);
                retVal = false;
                break;
            }
        }
    }
    return retVal;
}

bool fetchBool(const CInterfaceStack* inStack, int index, bool defaultValue /*= false*/)
{
    bool retVal = defaultValue;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_bool)
            retVal = ((CInterfaceStackBool*)obj)->getValue();
    }
    return retVal;
}

long long int fetchLong(const CInterfaceStack* inStack, int index, long long int defaultValue /*= -1*/)
{
    long long int retVal = defaultValue;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_integer)
            retVal = ((CInterfaceStackInteger*)obj)->getValue();
        else if (obj->getObjectType() == sim_stackitem_handle)
            retVal = ((CInterfaceStackHandle*)obj)->getValue();
    }
    return retVal;
}

int fetchInt(const CInterfaceStack* inStack, int index, int defaultValue /*= -1*/)
{
    return int(fetchLong(inStack, index, defaultValue));
}

long long int fetchHandle(const CInterfaceStack* inStack, int index, long long int defaultValue /*= -1*/)
{
    long long int retVal = defaultValue;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_integer)
            retVal = ((CInterfaceStackInteger*)obj)->getValue();
        else if (obj->getObjectType() == sim_stackitem_handle)
            retVal = ((CInterfaceStackHandle*)obj)->getValue();
    }
    return retVal;
}

double fetchDouble(const CInterfaceStack* inStack, int index, double defaultValue /*= 0.0*/)
{
    double retVal = defaultValue;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_double)
            retVal = ((CInterfaceStackNumber*)obj)->getValue();
        else if (obj->getObjectType() == sim_stackitem_integer)
            retVal = double(((CInterfaceStackInteger*)obj)->getValue());
    }
    return retVal;
}

std::string fetchText(const CInterfaceStack* inStack, int index, const char* txt /*= ""*/)
{
    std::string retVal = txt;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_string)
        {
            size_t l;
            retVal = ((CInterfaceStackString*)obj)->getValue(&l);
        }
    }
    return retVal;
}

std::string fetchBuffer(const CInterfaceStack* inStack, int index)
{
    std::string retVal;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_string)
        {
            size_t l;
            const char* s = ((CInterfaceStackString*)obj)->getValue(&l);
            retVal.assign(s, s + l);
        }
    }
    return retVal;
}

void fetchIntArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr /*= {}*/)
{
    std::vector<int> def;
    if (arr.size() != 0)
        for (int x : arr) def.push_back(x);
    fetchIntArray(inStack, index, outArr, def);
}

void fetchIntArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr)
{
    outArr.clear();
    if (arr.size() != 0)
        for (int x : arr) outArr.push_back(x);
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            int cnt = int(tbl->getArraySize());
            outArr.resize(cnt);
            tbl->getInt32Array(outArr.data(), cnt);
        }
    }
}

void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::initializer_list<long long int> arr /*= {}*/)
{
    std::vector<long long int> def;
    if (arr.size() != 0)
        for (long long int x : arr) def.push_back(x);
    fetchHandleArray(inStack, index, outArr, def);
}

void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::vector<long long int>& arr)
{
    outArr.clear();
    if (arr.size() != 0)
        for (long long int x : arr) outArr.push_back(x);
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            int cnt = int(tbl->getArraySize());
            outArr.resize(cnt);
            tbl->getInt64Array(outArr.data(), cnt);
        }
    }
}

void fetchColor(const CInterfaceStack* inStack, int index, float outArr[3], std::initializer_list<float> arr /*= {}*/)
{
    float def[3] = {0.0f, 0.0f, 0.0f};
    int i = 0;
    for (float x : arr)
    {
        if (i < 3)
            def[i++] = x;
    }
    fetchColor(inStack, index, outArr, def);
}

void fetchColor(const CInterfaceStack* inStack, int index, float outArr[3], const float defaultArr[3])
{
    if (defaultArr)
    {
        outArr[0] = defaultArr[0];
        outArr[1] = defaultArr[1];
        outArr[2] = defaultArr[2];
    }
    else
    {
        outArr[0] = 0.0f;
        outArr[1] = 0.0f;
        outArr[2] = 0.0f;
    }
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getFloatArray(outArr, 3);
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        {
            const CInterfaceStackColor* col = (CInterfaceStackColor*)obj;
            const float* f = col->getValue();
            for (size_t i = 0; i < 3; i++)
                outArr[i] = f[i];
        }
    }
}

C4Vector fetchQuaternion(const CInterfaceStack* inStack, int index, std::initializer_list<double> wxyz /*= {}*/)
{
    double def[4] = {1.0, 0.0, 0.0, 0.0};
    int i = 0;
    for (double x : wxyz)
    {
        if (i < 4)
            def[i++] = x;
    }
    return fetchQuaternion(inStack, index, def);
}

C4Vector fetchQuaternion(const CInterfaceStack* inStack, int index, const double wxyz[4])
{
    double d[4] = {1.0, 0.0, 0.0, 0.0};
    if (wxyz)
    {
        for (size_t i = 0; i < 4; i++)
            d[i] = wxyz[i];
    }
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_quaternion)
        {
            const CInterfaceStackQuaternion* q = (CInterfaceStackQuaternion*)obj;
            return C4Vector(q->getValue()[0]);
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getDoubleArray(d, 4);
            return C4Vector(d, true);
        }
    }
    return C4Vector(d, false);
}

C7Vector fetchPose(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyzqwqxqyqz /*= {}*/)
{
    double def[7] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    int i = 0;
    for (double x : xyzqwqxqyqz)
    {
        if (i < 7)
            def[i++] = x;
    }
    return fetchPose(inStack, index, def);
}

C7Vector fetchPose(const CInterfaceStack* inStack, int index, const double xyzqwqxqyqz[7])
{
    double d[7] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    if (xyzqwqxqyqz)
    {
        for (size_t i = 0; i < 7; i++)
            d[i] = xyzqwqxqyqz[i];
    }
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_pose)
        {
            const CInterfaceStackPose* p = (CInterfaceStackPose*)obj;
            return C7Vector(p->getValue()[0]);
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getDoubleArray(d, 7);
            return C7Vector(C4Vector(d + 3, true), C3Vector(d));
        }
    }
    return C7Vector(C4Vector(d + 3, false), C3Vector(d));
}

C3Vector fetchVector3(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyz /*= {}*/)
{
    double def[3] = {0.0, 0.0, 0.0};
    int i = 0;
    for (double x : xyz)
    {
        if (i < 3)
            def[i++] = x;
    }
    return fetchVector3(inStack, index, def);
}

C3Vector fetchVector3(const CInterfaceStack* inStack, int index, const double xyz[3])
{
    double d[3] = {0.0, 0.0, 0.0};
    if (xyz)
    {
        for (size_t i = 0; i < 3; i++)
            d[i] = xyz[i];
    }
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_matrix)
        {
            const CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* matr = m->getValue();
            if ( (matr->rows == 3) && (matr->cols == 1) )
                return C3Vector(matr->data.data());
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getDoubleArray(d, 3);
            return C3Vector(d);
        }
    }
    return C3Vector(d);
}

void fetchVector(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr /*= {}*/)
{
    fetchDoubleArray(inStack, index, outArr, arr);
}

CMatrix fetchMatrix(const CInterfaceStack* inStack, int index)
{
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_matrix)
            return CMatrix(((CInterfaceStackMatrix*)obj)->getValue()[0]);
    }
    return CMatrix();
}

void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr /*= {}*/)
{
    std::vector<double> def;
    if (arr.size() != 0)
        for (double x : arr) def.push_back(x);
    fetchDoubleArray(inStack, index, outArr, def);
}

void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::vector<double>& arr)
{
    outArr.clear();
    if (arr.size() != 0)
        for (double x : arr) outArr.push_back(x);
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            int cnt = tbl->getArraySize();
            outArr.resize(cnt);
            tbl->getDoubleArray(outArr.data(), cnt);
        }
        else if (obj->getObjectType() == sim_stackitem_matrix)
        {
            const CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)obj;
            const CMatrix* matr = m->getValue();
            if ( (matr->rows == 1) || (matr->cols == 1) )
                outArr.assign(matr->data.begin(), matr->data.end());
        }
    }
}

void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr /*= {}*/)
{
    std::vector<std::string> def;
    if (arr.size() != 0)
    {
        for (const auto& str : arr)
            def.push_back(str);
    }
    fetchTextArray(inStack, index, outArr, def);
}

void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr)
{
    outArr.clear();
    for (size_t i = 0; i < arr.size(); i++)
        outArr.push_back(arr[i]);
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getTextArray(outArr);
        }
    }
}

void pushNull(CInterfaceStack* outStack)
{
    outStack->pushNullOntoStack();
}

void pushBool(CInterfaceStack* outStack, bool v)
{
    outStack->pushBoolOntoStack(v);
}

void pushLong(CInterfaceStack* outStack, long long int v)
{
    outStack->pushInt64OntoStack(v);
}

void pushInt(CInterfaceStack* outStack, int v)
{
    outStack->pushInt32OntoStack(v);
}

void pushHandle(CInterfaceStack* outStack, int v)
{
    outStack->pushHandleOntoStack(v);
}

void pushDouble(CInterfaceStack* outStack, double v)
{
    outStack->pushDoubleOntoStack(v);
}

void pushText(CInterfaceStack* outStack, const char* v)
{
    outStack->pushTextOntoStack(v);
}

void pushBuffer(CInterfaceStack* outStack, const char* buff, size_t length)
{
    outStack->pushBufferOntoStack(buff, length);
}

void pushColor(CInterfaceStack* outStack, float v[3])
{
    outStack->pushColorOntoStack(v);
}

void pushQuaternion(CInterfaceStack* outStack, const C4Vector& v)
{
    outStack->pushQuaternionOntoStack(v.data);
}

void pushPose(CInterfaceStack* outStack, const C7Vector& v)
{
    double d[7];
    v.getData(d);
    outStack->pushPoseOntoStack(d);
}

void pushVector3(CInterfaceStack* outStack, const C3Vector& v)
{
    outStack->pushMatrixOntoStack(v.data, 3, 1);
}

void pushVector(CInterfaceStack* outStack, const double* v, size_t length)
{
    outStack->pushMatrixOntoStack(v, length, 1);
}

void pushMatrix(CInterfaceStack* outStack, const CMatrix& v)
{
    outStack->pushMatrixOntoStack(v.data.data(), v.rows, v.cols);
}

void pushIntArray(CInterfaceStack* outStack, const int* v, size_t length)
{
    outStack->pushInt32ArrayOntoStack(v, length);
}

void pushHandleArray(CInterfaceStack* outStack, const long long int* v, size_t length)
{
    outStack->pushHandleArrayOntoStack(v, length);
}

void pushDoubleArray(CInterfaceStack* outStack, const double* v, size_t length)
{
    outStack->pushDoubleArrayOntoStack(v, length);
}

void pushTextArray(CInterfaceStack* outStack, const std::string* v, size_t length)
{
    outStack->pushTextArrayOntoStack(v, length);
}

CSceneObject* getSceneObject(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CSceneObject* retVal = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if ( (retVal == nullptr) && (errMsg != nullptr) )
    {
        if (argPos == -1)
            errMsg[0] = "target object does not exist."; // can happen when calling from C
        else
        {
            std::string msg("bad argument #");
            msg += std::to_string(argPos + 1);
            msg += " (object does not exist).";
            errMsg->assign(msg.c_str());
        }
    }
    return retVal;
}

std::string _method_test(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {sim_stackitem_null,sim_stackitem_double,sim_stackitem_bool,sim_stackitem_string,sim_stackitem_integer,sim_stackitem_quaternion,sim_stackitem_pose,sim_stackitem_handle,sim_stackitem_color,sim_stackitem_table,2,sim_stackitem_string,sim_stackitem_matrix | arg_optional,2,3}))
    {
        pushNull(outStack);
        double doubleArg = fetchDouble(inStack, 1);
        pushDouble(outStack, doubleArg);
        printf("Double arg: %f\n", doubleArg);
        bool boolArg = fetchBool(inStack, 2);
        pushBool(outStack, boolArg);
        printf("Bool arg: %u\n", boolArg);
        std::string stringArg = fetchText(inStack, 3);
        pushText(outStack, stringArg.c_str());
        printf("text arg: %s\n", stringArg.c_str());
        int intArg = fetchInt(inStack, 4);
        pushInt(outStack, intArg);
        printf("Int arg: %i\n", intArg);
        C4Vector q = fetchQuaternion(inStack, 5);
        pushQuaternion(outStack, q);
        printf("Quaternion arg: %f, %f, %f, %f\n", q.data[1], q.data[2], q.data[3], q.data[0]);
        C7Vector p = fetchPose(inStack, 6);
        pushPose(outStack, p);
        printf("Pose arg: %f, %f, %f,%f, %f, %f, %f\n", p.X(0), p.X(1), p.X(2), p.Q(1), p.Q(2), p.Q(3), p.Q(0));
        int handleArg = fetchHandle(inStack, 7);
        pushHandle(outStack, handleArg);
        printf("Handle arg: %i\n", handleArg);
        float col[3];
        fetchColor(inStack, 8, col);
        pushColor(outStack, col);
        printf("Color arg: %f, %f, %f\n", col[0], col[1], col[2]);
        std::vector<std::string> strArrArg;
        fetchTextArray(inStack, 9, strArrArg);
        pushTextArray(outStack, strArrArg.data(), strArrArg.size());
        printf("string array arg: size %i, data: %s, %s\n", strArrArg.size(), strArrArg[0].c_str(), strArrArg[1].c_str());
        CMatrix matrixArg = fetchMatrix(inStack, 10);
        pushMatrix(outStack, matrixArg);
        printf("Matrix arg: ");
        for (size_t i = 0; i <matrixArg.data.size(); i++)
            printf("%f, ", matrixArg.data[i]);
        printf("\n");
        C3Vector v = fetchVector3(inStack, 11);
        pushVector3(outStack, v);
        printf("Vector3 arg: %f, %f, %f\n", v.data[0], v.data[1], v.data[2]);
        std::vector<double> vectArg;
        fetchVector(inStack, 12, vectArg);
        pushVector(outStack, vectArg.data(), vectArg.size());
        printf("Vector arg: size %i, data: %f, %f\n", vectArg.size(), vectArg[0], vectArg[1]);
    }

//    outStack->copyFrom(inStack);
//    std::string str;
//    outStack->printContent(-1, str);
//    printf(str.c_str());
    return errMsg;
}

std::string _method_getPosition(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        int relativeToObjectHandle = fetchHandle(inStack, 0, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 1, false);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = target->getParent();
            if (parent != nullptr)
                relativeToObjectHandle = parent->getObjectHandle();
        }
        if (relativeToObjectHandle != sim_handle_world)
        {
            CSceneObject* it2 = getSceneObject(relativeToObjectHandle, &errMsg, 0);
            if (it2 == nullptr)
                return errMsg;
        }
        CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = target->getCumulativeTransformation();
        else
        {
            if (relToJointBase)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr = relTr.getInverse() * target->getCumulativeTransformation();
            }
            else
            {
                if (target->getParent() == relObj)
                    tr = target->getLocalTransformation(); // in case of a series of get/set, not losing precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr = relTr.getInverse() * target->getCumulativeTransformation();
                }
            }
        }
        pushVector3(outStack, tr.X);
    }
    return errMsg;
}

std::string _method_setPosition(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_vector3, arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        C3Vector position = fetchVector3(inStack, 0);
        int relativeToObjectHandle = fetchHandle(inStack, 1, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 2, false);
        if (isFloatArrayOk(position.data, 3))
        {
            if (relativeToObjectHandle == sim_handle_parent)
            {
                relativeToObjectHandle = sim_handle_world;
                CSceneObject* parent = target->getParent();
                if (parent != nullptr)
                    relativeToObjectHandle = parent->getObjectHandle();
            }
            if (relativeToObjectHandle != sim_handle_world)
            {
                CSceneObject* it2 = getSceneObject(relativeToObjectHandle, &errMsg, 1);
                if (it2 == nullptr)
                    return errMsg;
            }
            if (target->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
                target->setDynamicsResetFlag(true, true);
            CSceneObject* relObj =getSceneObject(relativeToObjectHandle);
            if (relObj == nullptr)
                App::currentWorld->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), position);
            else
            {
                if (relToJointBase)
                {
                    C7Vector absTr(target->getCumulativeTransformation());
                    C7Vector relTr(relObj->getCumulativeTransformation());
                    C7Vector x(relTr.getInverse() * absTr);
                    x.X = position;
                    absTr = relTr * x;
                    App::currentWorld->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), absTr.X);
                }
                else
                {
                    if (target->getParent() == relObj)
                    { // special here, in order to not lose precision in a series of get/set
                        C7Vector tr(target->getLocalTransformation());
                        tr.X = position;
                        target->setLocalTransformation(tr);
                    }
                    else
                    {
                        C7Vector absTr(target->getCumulativeTransformation());
                        C7Vector relTr(relObj->getFullCumulativeTransformation());
                        C7Vector x(relTr.getInverse() * absTr);
                        x.X = position;
                        absTr = relTr * x;
                        App::currentWorld->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), absTr.X);
                    }
                }
            }
        }
        else
            errMsg = SIM_ERROR_INVALID_DATA;
    }
    return errMsg;
}

std::string _method_getQuaternion(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        int relativeToObjectHandle = fetchHandle(inStack, 0, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 1, false);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = target->getParent();
            if (parent != nullptr)
                relativeToObjectHandle = parent->getObjectHandle();
        }
        bool inverse = false;
        if (relativeToObjectHandle == sim_handle_inverse)
        {
            inverse = true;
            relativeToObjectHandle = sim_handle_world;
        }
        if (relativeToObjectHandle != sim_handle_world)
        {
            if (getSceneObject(relativeToObjectHandle, &errMsg, 0) == nullptr)
                return errMsg;
        }
        CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = target->getCumulativeTransformation();
        else
        {
            if (relToJointBase)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr = relTr.getInverse() * target->getCumulativeTransformation();
            }
            else
            {
                if (target->getParent() == relObj)
                    tr = target->getLocalTransformation(); // in case of a series get/set, not to lose precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr = relTr.getInverse() * target->getCumulativeTransformation();
                }
            }
        }
        if (inverse)
            tr.Q.inverse();
        pushQuaternion(outStack, tr.Q);
    }
    return errMsg;
}

std::string _method_setQuaternion(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_quaternion, arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        C4Vector quaternion = fetchQuaternion(inStack, 0);
        int relativeToObjectHandle = fetchHandle(inStack, 1, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 2, false);
        if (isFloatArrayOk(quaternion.data, 4))
        {
            if (relativeToObjectHandle == sim_handle_parent)
            {
                relativeToObjectHandle = sim_handle_world;
                CSceneObject* parent = target->getParent();
                if (parent != nullptr)
                    relativeToObjectHandle = parent->getObjectHandle();
            }
            bool inverse = false;
            if (relativeToObjectHandle == sim_handle_inverse)
            {
                inverse = true;
                relativeToObjectHandle = sim_handle_world;
            }
            if (relativeToObjectHandle != sim_handle_world)
            {
                if (getSceneObject(relativeToObjectHandle, &errMsg, 1) == nullptr)
                    return errMsg;
            }
            if (target->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
                target->setDynamicsResetFlag(true, true);
            CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
            if (relObj == nullptr)
            {
                quaternion.normalize();
                if (inverse)
                    quaternion.inverse();
                App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(target->getObjectHandle(), quaternion.getEulerAngles());
            }
            else
            {
                if ((target->getParent() == relObj) && (!relToJointBase))
                { // special here, in order to not lose precision in a series of get/set
                    C7Vector tr(target->getLocalTransformation());
                    tr.Q = quaternion;
                    tr.Q.normalize();
                    if (inverse)
                        tr.Q.inverse();
                    target->setLocalTransformation(tr);
                }
                else
                {
                    C7Vector absTr(target->getCumulativeTransformation());
                    C7Vector relTr;
                    if (relToJointBase)
                        relTr = relObj->getCumulativeTransformation();
                    else
                        relTr = relObj->getFullCumulativeTransformation();
                    C7Vector x(relTr.getInverse() * absTr);
                    x.Q = quaternion;
                    x.Q.normalize();
                    if (inverse)
                        x.Q.inverse();
                    absTr = relTr * x;
                    App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(target->getObjectHandle(), absTr.Q.getEulerAngles());
                }
            }
        }
        else
            errMsg = SIM_ERROR_INVALID_DATA;
    }
    return errMsg;
}

std::string _method_getPose(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        int relativeToObjectHandle = fetchHandle(inStack, 0, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 1, false);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = target->getParent();
            if (parent != nullptr)
                relativeToObjectHandle = parent->getObjectHandle();
        }
        bool inverse = false;
        if (relativeToObjectHandle == sim_handle_inverse)
        {
            inverse = true;
            relativeToObjectHandle = sim_handle_world;
        }
        if (relativeToObjectHandle != sim_handle_world)
        {
            if (getSceneObject(relativeToObjectHandle, &errMsg, 0) == nullptr)
                return errMsg;
        }
        CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = target->getCumulativeTransformation();
        else
        {
            C7Vector relTr;
            if (relToJointBase)
                relTr = relObj->getCumulativeTransformation();
            else
                relTr = relObj->getFullCumulativeTransformation();
            tr = relTr.getInverse() * target->getCumulativeTransformation();
        }
        if (inverse)
            tr.inverse();
        pushPose(outStack, tr);
    }
    return errMsg;
}

std::string _method_setPose(int targetObj, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_pose, arg_handle | arg_optional, arg_bool | arg_optional}))
    {
        C7Vector tr = fetchPose(inStack, 0);
        int relativeToObjectHandle = fetchHandle(inStack, 1, sim_handle_world);
        bool relToJointBase = fetchBool(inStack, 2, false);

        if (isFloatArrayOk(tr.X.data, 3) && isFloatArrayOk(tr.Q.data, 4))
        {
            if (relativeToObjectHandle == sim_handle_parent)
            {
                relativeToObjectHandle = sim_handle_world;
                CSceneObject* parent = target->getParent();
                if (parent != nullptr)
                    relativeToObjectHandle = parent->getObjectHandle();
            }
            bool inverse = false;
            if (relativeToObjectHandle == sim_handle_inverse)
            {
                inverse = true;
                relativeToObjectHandle = sim_handle_world;
            }
            if (relativeToObjectHandle != sim_handle_world)
            {
                if (getSceneObject(relativeToObjectHandle, &errMsg, 1) == nullptr)
                    return errMsg;
            }
            if (target->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
                target->setDynamicsResetFlag(true, true);
            tr.Q.normalize();
            if (inverse)
                tr.inverse();
            CSceneObject* objRel = getSceneObject(relativeToObjectHandle);
            if (objRel == nullptr)
                App::currentWorld->sceneObjects->setObjectAbsolutePose(target->getObjectHandle(), tr, false);
            else
            {
                C7Vector relTr;
                if (relToJointBase)
                    relTr = objRel->getCumulativeTransformation();
                else
                    relTr = objRel->getFullCumulativeTransformation();
                App::currentWorld->sceneObjects->setObjectAbsolutePose(target->getObjectHandle(), relTr * tr, false);
            }
        }
        else
            errMsg = SIM_ERROR_INVALID_DATA;
    }
    return errMsg;
}

