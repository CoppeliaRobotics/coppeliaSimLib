#include <methods.h>
#include <simInternal.h>
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

std::string callMethod(int target, const char* methodName, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    static std::map<std::string, std::function<std::string(int, const char*, const CInterfaceStack*, CInterfaceStack*)>> funcTable;
    if (funcTable.size() == 0)
    {
        funcTable["getPosition"] = _method_getPosition;
    }

    std::string retVal("__notFound__");
    if (funcTable.find(methodName) != funcTable.end())
        retVal = funcTable[methodName](target, methodName, inStack, outStack);
    return retVal;
}

bool checkInputArguments(int target, const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs)
{ // inargs: a list of desired types. Following 2 types are special:
    // arg_table is followed by 2 auxiliary values: size (-1 for any, 0 for a map) and type of content (-1 for any)
    // arg_matrix is followed by 2 auxiliary values: rows (-1 for any) and cols (-1 for any)
    // The type argument can be combined with arg_optional
    bool retVal = true;
    size_t argP = 0;
    size_t argC = 0;
    while (argP < (inargs.size() - 1))
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

bool fetchBoolArg(const CInterfaceStack* inStack, int index, bool defaultValue /*= false*/)
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

long long int fetchLongArg(const CInterfaceStack* inStack, int index, long long int defaultValue /*= -1*/)
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

int fetchIntArg(const CInterfaceStack* inStack, int index, int defaultValue /*= -1*/)
{
    return int(fetchLongArg(inStack, index, defaultValue));
}

int fetchHandleArg(const CInterfaceStack* inStack, int index, int defaultValue /*= -1*/)
{
    int retVal = defaultValue;
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

double fetchDoubleArg(const CInterfaceStack* inStack, int index, double defaultValue /*= 0.0*/)
{
    double retVal = defaultValue;
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_double)
            retVal = ((CInterfaceStackNumber*)obj)->getValue();
        else if (obj->getObjectType() == sim_stackitem_integer)
            retVal = ((CInterfaceStackInteger*)obj)->getValue();
    }
    return retVal;
}

std::string fetchTextArg(const CInterfaceStack* inStack, int index, const char* txt /*= ""*/)
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

std::string fetchBufferArg(const CInterfaceStack* inStack, int index)
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

void fetchIntArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr /*= {}*/)
{
    std::vector<int> def;
    if (arr.size() != 0)
        for (int x : arr) def.push_back(x);
    fetchIntArrayArg(inStack, index, outArr, def);
}

void fetchIntArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr)
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
            int cnt = tbl->getArraySize();
            outArr.resize(cnt);
            tbl->getInt32Array(outArr.data(), cnt);
        }
    }
}

void fetchHandleArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr /*= {}*/)
{
    std::vector<int> def;
    if (arr.size() != 0)
        for (int x : arr) def.push_back(x);
    fetchHandleArrayArg(inStack, index, outArr, def);
}

void fetchHandleArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr)
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
            int cnt = tbl->getArraySize();
            outArr.resize(cnt);
            tbl->getInt32Array(outArr.data(), cnt);
        }
    }
}

void fetchColorArg(const CInterfaceStack* inStack, int index, float outArr[3], std::initializer_list<float> arr /*= {}*/)
{
    float def[3] = {0.0f, 0.0f, 0.0f};
    int i = 0;
    for (float x : arr)
    {
        if (i < 3)
            def[i++] = x;
    }
    fetchColorArg(inStack, index, outArr, def);
}

void fetchColorArg(const CInterfaceStack* inStack, int index, float outArr[3], const float defaultArr[3])
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

C4Vector fetchQuaternionArg(const CInterfaceStack* inStack, int index, std::initializer_list<double> wxyz /*= {}*/)
{
    double def[4] = {1.0, 0.0, 0.0, 0.0};
    int i = 0;
    for (double x : wxyz)
    {
        if (i < 4)
            def[i++] = x;
    }
    return fetchQuaternionArg(inStack, index, def);
}

C4Vector fetchQuaternionArg(const CInterfaceStack* inStack, int index, const double wxyz[4])
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

C7Vector fetchPoseArg(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyzqwqxqyqz /*= {}*/)
{
    double def[7] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    int i = 0;
    for (double x : xyzqwqxqyqz)
    {
        if (i < 7)
            def[i++] = x;
    }
    return fetchPoseArg(inStack, index, def);
}

C7Vector fetchPoseArg(const CInterfaceStack* inStack, int index, const double xyzqwqxqyqz[7])
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

C3Vector fetchVector3Arg(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyz /*= {}*/)
{
    double def[3] = {0.0, 0.0, 0.0};
    int i = 0;
    for (double x : xyz)
    {
        if (i < 3)
            def[i++] = x;
    }
    return fetchVector3Arg(inStack, index, def);
}

C3Vector fetchVector3Arg(const CInterfaceStack* inStack, int index, const double xyz[3])
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

void fetchVectorArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr /*= {}*/)
{
    fetchDoubleArrayArg(inStack, index, outArr, arr);
}

CMatrix fetchMatrixArg(const CInterfaceStack* inStack, int index)
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

void fetchDoubleArrayArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr /*= {}*/)
{
    std::vector<double> def;
    if (arr.size() != 0)
        for (double x : arr) def.push_back(x);
    fetchDoubleArrayArg(inStack, index, outArr, def);
}

void fetchDoubleArrayArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::vector<double>& arr)
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

void fetchTextArrayArg(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr /*= {}*/)
{
    std::vector<std::string> def;
    if (arr.size() != 0)
        for (std::string x : arr) def.push_back(x);
    fetchTextArrayArg(inStack, index, outArr, def);
}

void fetchTextArrayArg(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr)
{
    outArr.clear();
    if (arr.size() != 0)
        for (std::string x : arr) outArr.push_back(x);
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

std::string _method_getPosition(int target, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    printf("in getPosition!!\n");
    std::string errMsg;
    if (checkInputArguments(target, method, inStack, &errMsg, {sim_stackitem_null,sim_stackitem_double,sim_stackitem_bool,sim_stackitem_string,sim_stackitem_integer,sim_stackitem_quaternion,sim_stackitem_pose,sim_stackitem_handle,sim_stackitem_color,sim_stackitem_table,2,sim_stackitem_string,sim_stackitem_matrix | arg_optional,2,3}))
    {
        double doubleArg = fetchDoubleArg(inStack, 1);
        printf("Double arg: %f\n", doubleArg);
        bool boolArg = fetchBoolArg(inStack, 2);
        printf("Bool arg: %u\n", boolArg);
        std::string stringArg = fetchTextArg(inStack, 3);
        printf("text arg: %s\n", stringArg.c_str());
        int intArg = fetchIntArg(inStack, 4);
        printf("Int arg: %i\n", intArg);
        C4Vector q = fetchQuaternionArg(inStack, 5);
        printf("Quaternion arg: %f, %f, %f, %f\n", q.data[1], q.data[2], q.data[3], q.data[0]);
        C7Vector p = fetchPoseArg(inStack, 6);
        printf("Pose arg: %f, %f, %f,%f, %f, %f, %f\n", p.X(0), p.X(1), p.X(2), p.Q(1), p.Q(2), p.Q(3), p.Q(0));
        int handleArg = fetchHandleArg(inStack, 7);
        printf("Handle arg: %i\n", handleArg);
        float col[3];
        fetchColorArg(inStack, 8, col);
        printf("Color arg: %f, %f, %f\n", col[0], col[1], col[2]);
        std::vector<std::string> strArrArg;
        fetchTextArrayArg(inStack, 9, strArrArg);
        printf("string array arg: size %i, data: %s, %s\n", strArrArg.size(), strArrArg[0].c_str(), strArrArg[1].c_str());
        CMatrix matrixArg = fetchMatrixArg(inStack, 10);
        printf("Matrix arg: ");
        for (size_t i = 0; i <matrixArg.data.size(); i++)
            printf("%f, ", matrixArg.data[i]);
        printf("\n");
        C3Vector v = fetchVector3Arg(inStack, 11);
        printf("Vector3 arg: %f, %f, %f\n", v.data[0], v.data[1], v.data[2]);
        std::vector<double> vectArg;
        fetchVectorArg(inStack, 12, vectArg);
        printf("Vector arg: size %i, data: %f, %f\n", vectArg.size(), vectArg[0], vectArg[1]);
    }


                outStack->copyFrom(inStack);
    std::string str;
    outStack->printContent(-1, str);
    printf(str.c_str());
    return errMsg;
}
