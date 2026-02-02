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
#include <guiApp.h>
#include <fileOperations.h>
#include <collisionRoutines.h>
#include <distanceRoutines.h>
#include <proxSensorRoutine.h>

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
    constexpr int arg_vector        = sim_stackitem_exvector;
    constexpr int arg_vector3       = sim_stackitem_exvector3;
    constexpr int arg_any           = sim_stackitem_exany;
    constexpr int arg_optional      = sim_stackitem_exoptional;
}

std::string callMethod(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    static std::map<std::string, std::function<std::string(int, const char*, CScriptObject*, const CInterfaceStack*, CInterfaceStack*)>> funcTable;
    if (funcTable.size() == 0)
    {
        funcTable["test"] = _method_test;
        funcTable["getPosition"] = _method_getPosition;
        funcTable["setPosition"] = _method_setPosition;
        funcTable["getQuaternion"] = _method_getQuaternion;
        funcTable["setQuaternion"] = _method_setQuaternion;
        funcTable["getPose"] = _method_getPose;
        funcTable["setPose"] = _method_setPose;
        funcTable["setParent"] = _method_setParent;
        funcTable["handleSandboxScript"] = _method_handleSandboxScript;
        funcTable["handleAddOnScripts"] = _method_handleAddOnScripts;
        funcTable["handleCustomizationScripts"] = _method_handleCustomizationScripts;
        funcTable["handleSimulationScripts"] = _method_handleSimulationScripts;
        funcTable["loadModel"] = _method_loadModel;
        funcTable["loadModelFromBuffer"] = _method_loadModelFromBuffer;
        funcTable["loadModelThumbnail"] = _method_loadModelThumbnail;
        funcTable["loadModelThumbnailFromBuffer"] = _method_loadModelThumbnailFromBuffer;
        funcTable["saveModel"] = _method_saveModel;
        funcTable["saveModelToBuffer"] = _method_saveModelToBuffer;
        funcTable["loadScene"] = _method_loadScene;
        funcTable["loadSceneFromBuffer"] = _method_loadSceneFromBuffer;
        funcTable["saveScene"] = _method_saveScene;
        funcTable["saveSceneToBuffer"] = _method_saveSceneToBuffer;
        funcTable["removeModel"] = _method_removeModel;
        funcTable["remove"] = _method_remove;
        funcTable["removeObjects"] = _method_removeObjects;
        funcTable["duplicateObjects"] = _method_duplicateObjects;
        funcTable["addItem"] = _method_addItem;
        funcTable["removeItem"] = _method_removeItem;
        funcTable["checkCollision"] = _method_checkCollision;
        funcTable["checkDistance"] = _method_checkDistance;
        funcTable["handleSensor"] = _method_handleSensor;
        funcTable["resetSensor"] = _method_resetSensor;
        funcTable["checkSensor"] = _method_checkSensor;
        funcTable["getObjects"] = _method_getObjects;
        funcTable["addItems"] = _method_addItems;
        funcTable["clearItems"] = _method_clearItems;
    }

    std::string retVal("__notFound__");
    if (funcTable.find(method) != funcTable.end())
        retVal = funcTable[method](targetObj, method, currentScript, inStack, outStack);
    return retVal;
}

bool checkInputArguments(const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs)
{ // inargs: a list of desired types. Following 3 types are special:
    // arg_table is followed by 2 auxiliary values: size (-1 for any, 0 for a map) and type of content (arg_any for any)
    // arg_matrix is followed by 2 auxiliary values: rows (-1 for any) and cols (-1 for any)
    // arg_any stands for any (is ignored)
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
                                if ((tableContent != arg_any) && (!tbl->areAllValuesThis(tableContent, true)))
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
                    if (desiredArgType == arg_any)
                        retVal = true;
                    else if (desiredArgType == arg_double)
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

void fetchBuffer(const CInterfaceStack* inStack, int index, std::vector<char>& buff)
{
    buff.clear();
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_string)
        {
            size_t l;
            const char* s = ((CInterfaceStackString*)obj)->getValue(&l);
            buff.assign(s, s + l);
        }
    }
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
            int cnt = int(tbl->getArraySize());
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

void fetchArrayAsConsecutiveNumbers(const CInterfaceStack* inStack, int index, std::vector<float>& outArr)
{ // quaternions and poses are fetched as qx,qy,qz,qw and x,y,z,qx,qy,qz,qw respectively
    outArr.clear();
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getItemsAsConsecutiveFloats(outArr);
        }
    }
}

void fetchArrayAsConsecutiveNumbers(const CInterfaceStack* inStack, int index, std::vector<double>& outArr)
{ // quaternions and poses are fetched as qx,qy,qz,qw and x,y,z,qx,qy,qz,qw respectively
    outArr.clear();
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getItemsAsConsecutiveDoubles(outArr);
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

void pushShortHandleArray(CInterfaceStack* outStack, const int* v, size_t length)
{
    outStack->pushShortHandleArrayOntoStack(v, length);
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

CSceneObject* getSpecificSceneObjectType(int identifier, int type, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CSceneObject* retVal = getSceneObject(identifier, errMsg, argPos);
    if (retVal != nullptr)
    {
        if (retVal->getObjectType() != type)
        {
            retVal = nullptr;
            if (errMsg != nullptr)
            {
                if (argPos == -1)
                    errMsg[0] = "target object is not the correct type."; // can happen when calling from C
                else
                {
                    std::string msg("bad argument #");
                    msg += std::to_string(argPos + 1);
                    msg += " (object is not the correct type).";
                    errMsg->assign(msg.c_str());
                }
            }
        }
    }
    return retVal;
}

CCollection* getCollection(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CCollection* retVal = App::currentWorld->collections->getObjectFromHandle(identifier);
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

CDrawingObject* getDrawingObject(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CDrawingObject* retVal = App::currentWorld->drawingCont->getObjectFromHandle(identifier);
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

CScriptObject* getDetachedScript(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CScriptObject* retVal = nullptr;
    if (identifier > SIM_IDEND_SCENEOBJECT)
        retVal = App::worldContainer->getScriptObjectFromHandle(identifier);
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

bool doesEntityExist(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    bool retVal = false;
    if ((App::currentWorld->sceneObjects->getObjectFromHandle(identifier) != nullptr) || (App::currentWorld->collections->getObjectFromHandle(identifier) != nullptr))
        retVal = true;
    else if (errMsg != nullptr)
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

std::string getInvalidArgString(size_t argPos)
{
    std::string retVal = "invalid argument #";
    retVal += std::to_string(argPos + 1);
    retVal += ".";
    return retVal;
}


std::string _method_test(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_getPosition(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setPosition(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_getQuaternion(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setQuaternion(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_getPose(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setPose(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setParent(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_integer | arg_optional}))
    {
        int parentObjectHandle = fetchHandle(inStack, 0, -1);
        int  parentingMode = fetchInt(inStack, 1, sim_parentingmode_keepworldpose);
        if (parentObjectHandle != -1)
        {
            if (getSceneObject(parentObjectHandle, &errMsg, 0) == nullptr)
                return errMsg;
        }
        CSceneObject* parentIt = getSceneObject(parentObjectHandle);
        CSceneObject* pp = parentIt;
        while (pp != nullptr)
        {
            if (pp == target)
            {
                errMsg = SIM_ERROR_OBJECT_IS_ANCESTOR_OF_DESIRED_PARENT;
                return errMsg;
            }
            pp = pp->getParent();
        }
        if (parentingMode == sim_parentingmode_keepworldpose)
            App::currentWorld->sceneObjects->setObjectParent(target, parentIt, true);
        else
        {
            if (parentingMode == sim_parentingmode_assembly)
            {
                if (parentIt != nullptr)
                {
                    if (!App::assemble(parentIt->getObjectHandle(), target->getObjectHandle(), false))
                    {
                        errMsg = SIM_ERROR_INVALID_ASSEMBLY;
                        return errMsg;
                    }
                }
                else
                {
                    errMsg = SIM_ERROR_INVALID_ASSEMBLY;
                    return errMsg;
                }
            }
            else
                App::currentWorld->sceneObjects->setObjectParent(target, parentIt, false);
        }
    }
    return errMsg;
}

std::string _method_handleSandboxScript(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if ((editMode == NO_EDIT_MODE) && (App::worldContainer->sandboxScript != nullptr))
                App::worldContainer->sandboxScript->systemCallScript(callType, nullptr, nullptr);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleAddOnScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int editMode = NO_EDIT_MODE;
            int calledCnt = 0;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if (editMode == NO_EDIT_MODE)
                calledCnt = App::worldContainer->addOnScriptContainer->callScripts(callType, nullptr, nullptr);
            pushInt(outStack, calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleSimulationScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int calledCnt = 0;
            CInterfaceStack* stack = nullptr;
            if (inStack->getStackSize() > 1)
            {
                App::worldContainer->interfaceStackContainer->createStack();
                stack->copyFrom(inStack);
                CInterfaceStackObject* obj = stack->detachStackObjectFromIndex(0);
                delete obj;
            }
            calledCnt = App::currentWorld->sceneObjects->callScripts_noMainScript(sim_scripttype_simulation, callType, stack, nullptr);
            if (stack != nullptr)
                App::worldContainer->interfaceStackContainer->destroyStack(stack);
            pushInt(outStack, calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleCustomizationScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int calledCnt = 0;
            CInterfaceStack* stack = nullptr;
            if (inStack->getStackSize() > 1)
            {
                App::worldContainer->interfaceStackContainer->createStack();
                stack->copyFrom(inStack);
                CInterfaceStackObject* obj = stack->detachStackObjectFromIndex(0);
                delete obj;
            }
            calledCnt = App::currentWorld->sceneObjects->callScripts_noMainScript(sim_scripttype_customization, callType, stack, nullptr);
            if (stack != nullptr)
                App::worldContainer->interfaceStackContainer->destroyStack(stack);
            pushInt(outStack, calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_loadModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string path = fetchText(inStack, 0);
        std::string infoStr;
        if (CFileOperations::loadModel(path.c_str(), false, false, nullptr, false, false, &infoStr, &errMsg))
        {
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
            pushHandle(outStack, App::currentWorld->sceneObjects->getLastSelectionHandle());
        }
        setLastInfo(infoStr.c_str());
    }
    return errMsg;
}

std::string _method_loadModelFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string data = fetchBuffer(inStack, 0);
        std::vector<char> buffer(data.data(), data.data() + data.size());
        std::string infoStr;
        if (CFileOperations::loadModel(nullptr, false, false, &buffer, false, false, &infoStr, &errMsg))
        {
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
            pushHandle(outStack, App::currentWorld->sceneObjects->getLastSelectionHandle());
        }
        setLastInfo(infoStr.c_str());
    }
    return errMsg;
}

std::string _method_loadModelThumbnail(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string path = fetchText(inStack, 0);
        std::string infoStr;
        if (CFileOperations::loadModel(path.c_str(), false, false, nullptr, true, false, &infoStr, &errMsg))
        {
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
            char* buff = new char[128 * 128 * 4];
            bool opRes = App::currentWorld->environment->modelThumbnail_notSerializedHere.copyUncompressedImageToBuffer(buff);
            if (opRes)
            {
                pushBuffer(outStack, buff, 128 * 128 * 4);
                delete[] buff;
                return errMsg;
            }
            delete[] buff;
            return errMsg;
        }
        setLastInfo(infoStr.c_str());
    }
    return errMsg;
}

std::string _method_loadModelThumbnailFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string data = fetchBuffer(inStack, 0);
        std::vector<char> buffer(data.data(), data.data() + data.size());
        std::string infoStr;
        if (CFileOperations::loadModel(nullptr, false, false, &buffer, true, false, &infoStr, &errMsg))
        {
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
            char* buff = new char[128 * 128 * 4];
            bool opRes = App::currentWorld->environment->modelThumbnail_notSerializedHere.copyUncompressedImageToBuffer(buff);
            if (opRes)
            {
                pushBuffer(outStack, buff, 128 * 128 * 4);
                delete[] buff;
                return errMsg;
            }
            delete[] buff;
            return errMsg;
        }
        setLastInfo(infoStr.c_str());
    }
    return errMsg;
}

std::string _method_saveModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string filename = fetchText(inStack, 0);
        if (!App::currentWorld->environment->getSceneLocked())
        {
            if (target->getModelBase())
            {
                std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
                if (CFileOperations::saveModel(target->getObjectHandle(), filename.c_str(), false, nullptr, nullptr, &errMsg))
                    App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
            }
            else
                errMsg = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
        }
        else
            errMsg = SIM_ERROR_SCENE_LOCKED;
    }
    return errMsg;
}

std::string _method_saveModelToBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        if (!App::currentWorld->environment->getSceneLocked())
        {
            if (target->getModelBase())
            {
                std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
                std::vector<char> buffer;
                std::string infoStr;
                if (CFileOperations::saveModel(target->getObjectHandle(), nullptr, false, &buffer, &infoStr, &errMsg))
                {
                    App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
                    setLastInfo(infoStr.c_str());
                    pushBuffer(outStack, buffer.data(), buffer.size());
                }
            }
            else
                errMsg = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
        }
        else
            errMsg = SIM_ERROR_SCENE_LOCKED;
    }
    return errMsg;
}

std::string _method_loadScene(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation) && (currentScript->getScriptType() != sim_scripttype_customization)))
    {
        if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_bool | arg_optional}))
        {
            std::string path = fetchText(inStack, 0);
            bool createNewScene = fetchBool(inStack, 1, false);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (path.size() > 0)
                {
                    if (createNewScene)
                        CFileOperations::createNewScene(true);
                    if (CFileOperations::loadScene(path.c_str(), false, nullptr, nullptr, &errMsg))
                    {
#ifdef SIM_WITH_GUI
                        if (GuiApp::mainWindow != nullptr)
                            GuiApp::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the correct dimensions
#endif
                        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    }
                }
                else
                    CFileOperations::createNewScene(createNewScene);
            }
            else
                errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
        }
    }
    else
        errMsg = "cannot be called from an embedded script.";
    return errMsg;
}

std::string _method_loadSceneFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation) && (currentScript->getScriptType() != sim_scripttype_customization)))
    {
        if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_bool | arg_optional}))
        {
            std::vector<char> buff;
            fetchBuffer(inStack, 0, buff);
            bool createNewScene = fetchBool(inStack, 1, false);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (createNewScene)
                    CFileOperations::createNewScene(true);
                CFileOperations::loadScene(nullptr, false, &buff, nullptr, &errMsg);
            }
            else
                errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
        }
    }
    else
        errMsg = "cannot be called from an embedded script.";
    return errMsg;
}

std::string _method_saveScene(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation)))
    {
        if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
        {
            std::string path = fetchText(inStack, 0);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    if (App::currentWorld->environment->getRequestFinalSave())
                        App::currentWorld->environment->setSceneLocked(); // silent locking!
                    if (CFileOperations::saveScene(path.c_str(), false, false, nullptr, nullptr, &errMsg))
                    {
#ifdef SIM_WITH_GUI
                        GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
                        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    }
                }
                else
                    errMsg = SIM_ERROR_SCENE_LOCKED;
            }
            else
                errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
        }
    }
    else
        errMsg = "cannot be called from a simulation script.";
    return errMsg;
}

std::string _method_saveSceneToBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation)))
    {
        if (checkInputArguments(method, inStack, &errMsg, {}))
        {
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    std::vector<char> buffer;
                    std::string infoStr;
                    if (CFileOperations::saveScene(nullptr, false, false, &buffer, &infoStr, &errMsg))
                    {
#ifdef SIM_WITH_GUI
                        GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
                        setLastInfo(infoStr.c_str());
                        pushBuffer(outStack, buffer.data(), buffer.size());
                    }
                    else
                        setLastInfo(infoStr.c_str());
                }
                else
                    errMsg = SIM_ERROR_SCENE_LOCKED;
            }
            else
                errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
        }
    }
    else
        errMsg = "cannot be called from a simulation script.";
    return errMsg;
}

std::string _method_removeModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_bool | arg_optional}))
    {
        bool delayed = fetchBool(inStack, 0, false);
        if (target->getModelBase())
        {
            // Erase the objects:
            std::vector<int> sel;
            sel.push_back(targetObj);
            App::currentWorld->sceneObjects->addModelObjects(sel);
            App::currentWorld->sceneObjects->eraseObjects(&sel, true, delayed);
        }
        else
            errMsg = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
    }
    return errMsg;
}

std::string _method_remove(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_bool | arg_optional}))
    {
        bool delayed = fetchBool(inStack, 0, false);
        CSceneObject* sceneObj = getSceneObject(targetObj);
        CCollection* coll = getCollection(targetObj);
        CDrawingObject* draw = getDrawingObject(targetObj);
        CScriptObject* script = getDetachedScript(targetObj);
        if (sceneObj != nullptr)
        {
            std::vector<int> sel;
            sel.push_back(targetObj);
            App::currentWorld->sceneObjects->eraseObjects(&sel, true, delayed);
        }
        else if (coll != nullptr)
            App::currentWorld->collections->removeCollection(targetObj);
        else if (draw != nullptr)
            App::currentWorld->drawingCont->removeObject(targetObj);
        else if (script != nullptr)
        {
            if (!App::worldContainer->addOnScriptContainer->removeAddOn(targetObj))
                errMsg = SIM_ERROR_INVALID_SCRIPT_TYPE_OR_DOES_NOT_EXIST;
        }
        else
            errMsg = "object does not exist or cannot be removed.";
    }
    return errMsg;
}

std::string _method_removeObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_handle, arg_bool | arg_optional}))
    {
        std::vector<long long int> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        bool delayed = fetchBool(inStack, 1, false);
        std::vector<int> sceneObjectHandles;
        for (size_t i = 0; i < objectHandles.size(); i ++)
        {
            int objectHandle = int(objectHandles[i]);
            CSceneObject* sceneObj = getSceneObject(objectHandle);
            CCollection* coll = getCollection(objectHandle);
            CDrawingObject* draw = getDrawingObject(objectHandle);
            CScriptObject* script = getDetachedScript(objectHandle);
            if (sceneObj != nullptr)
                sceneObjectHandles.push_back(objectHandle);
            else if (coll != nullptr)
                App::currentWorld->collections->removeCollection(objectHandle);
            else if (draw != nullptr)
                App::currentWorld->drawingCont->removeObject(objectHandle);
            else if (script != nullptr)
            {
                if (!App::worldContainer->addOnScriptContainer->removeAddOn(objectHandle))
                    errMsg = SIM_ERROR_INVALID_SCRIPT_TYPE_OR_DOES_NOT_EXIST;
            }
            else
                errMsg = SIM_ERROR_FOUND_INVALID_HANDLES;
            if (errMsg != "")
                break;
        }
        if ((errMsg == "") && (sceneObjectHandles.size() > 0))
        {
            if (!App::currentWorld->sceneObjects->eraseObjects(&sceneObjectHandles, true, delayed))
                errMsg = SIM_ERROR_FOUND_INVALID_HANDLES;
        }
    }
    return errMsg;
}

std::string _method_duplicateObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_handle, arg_integer | arg_optional}))
    {
        std::vector<long long int> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        int options = fetchInt(inStack, 1, 0);
        if (objectHandles.size() > 0)
        {
            // memorize current selection:
            std::vector<int> initSel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                App::currentWorld->sceneObjects->getObjectFromIndex(i)->setCopyString("");
            // adjust the selection to copy:
            std::vector<int> selT;
            for (size_t i = 0; i < objectHandles.size(); i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(int(objectHandles[i]));
                if (it != nullptr)
                {
                    if (((options & 1) == 0) || it->getModelBase())
                        selT.push_back(int(objectHandles[i]));
                    // Here we can't use custom data, dna, etc. since it might be stripped away during the copy, dep. on the options
                    it->setCopyString(std::to_string(objectHandles[i]).c_str());
                }
            }
            // if we just wanna handle models, make sure no model has a parent that will also be copied:
            std::vector<int> sel;
            if (options & 1)
            {
                for (size_t i = 0; i < selT.size(); i++)
                {
                    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(selT[i]);
                    bool ok = true;
                    if (it->getParent() != nullptr)
                    {
                        for (size_t j = 0; j < selT.size(); j++)
                        {
                            CSceneObject* it2 = App::currentWorld->sceneObjects->getObjectFromHandle(selT[j]);
                            if (it != it2)
                            {
                                if (it->hasAncestor(it2))
                                {
                                    ok = false;
                                    break;
                                }
                            }
                        }
                    }
                    if (ok)
                        sel.push_back(selT[i]);
                }
            }
            else
                sel.assign(selT.begin(), selT.end());

            if (options & 1)
                App::currentWorld->sceneObjects->addModelObjects(sel);
            if ((options & 2) == 0)
                App::currentWorld->sceneObjects->addCompatibilityScripts(sel);
            App::worldContainer->copyBuffer->memorizeBuffer();
            App::worldContainer->copyBuffer->copyCurrentSelection(sel, App::currentWorld->environment->getSceneLocked(), options >> 1);
            App::currentWorld->sceneObjects->deselectObjects();
            App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(), 0);
            App::worldContainer->copyBuffer->restoreBuffer();
            App::worldContainer->copyBuffer->clearMemorizedBuffer();

            // Restore the initial selection:
            App::currentWorld->sceneObjects->deselectObjects();
            for (size_t i = 0; i < initSel.size(); i++)
                App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

            for (size_t i = 0; i < objectHandles.size(); i++)
            { // now return the handles of the copies. Each input handle has a corresponding output handle:
                CSceneObject* original = App::currentWorld->sceneObjects->getObjectFromHandle(int(objectHandles[i]));
                objectHandles[i] = -1; // a handle in the output array can be -1 (e.g. with stripped-away scripts)
                if (original != nullptr)
                {
                    std::string str = original->getCopyString();
                    original->setCopyString("");
                    for (size_t j = 0; j < App::currentWorld->sceneObjects->getObjectCount(); j++)
                    {
                        CSceneObject* potentialCopy = App::currentWorld->sceneObjects->getObjectFromIndex(j);
                        if (potentialCopy->getCopyString().compare(str) == 0)
                        {
                            objectHandles[i] = potentialCopy->getObjectHandle();
                            break;
                        }
                    }
                }
            }
        }
        pushHandleArray(outStack, objectHandles.data(), objectHandles.size());
    }
    return errMsg;
}

std::string _method_addItem(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CCollection* target = getCollection(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle, arg_integer | arg_optional, arg_bool | arg_optional}))
    {
        int objectHandle = fetchHandle(inStack, 0);
        int what = fetchInt(inStack, 1, sim_handle_single);
        bool excludeObj = fetchBool(inStack, 2, false);

        if (what != sim_handle_all)
        {
            if (getSceneObject(objectHandle, &errMsg, 0) == nullptr)
                return errMsg;
        }
        CCollectionElement* el = nullptr;
        if (what == sim_handle_all)
            el = new CCollectionElement(-1, sim_collectionelement_all, true);
        if (what == sim_handle_single)
            el = new CCollectionElement(objectHandle, sim_collectionelement_loose, true);
        if (what == sim_handle_tree)
        {
            int what = sim_collectionelement_frombaseincluded;
            if (excludeObj)
                what = sim_collectionelement_frombaseexcluded;
            el = new CCollectionElement(objectHandle, what, true);
        }
        if (what == sim_handle_chain)
        {
            int what = sim_collectionelement_fromtipincluded;
            if (excludeObj)
                what = sim_collectionelement_fromtipexcluded;
            el = new CCollectionElement(objectHandle, what, true);
        }
        if (el == nullptr)
            return getInvalidArgString(1);
        target->addCollectionElement(el);
    }
    return errMsg;
}

std::string _method_removeItem(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CCollection* target = getCollection(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle, arg_integer | arg_optional, arg_bool | arg_optional}))
    {
        int objectHandle = fetchHandle(inStack, 0);
        int what = fetchInt(inStack, 1, sim_handle_single);
        bool excludeObj = fetchBool(inStack, 2, false);

        if (what != sim_handle_all)
        {
            if (getSceneObject(objectHandle, &errMsg, 0) == nullptr)
                return errMsg;
        }
        CCollectionElement* el = nullptr;
        if (what == sim_handle_all)
            el = new CCollectionElement(-1, sim_collectionelement_all, false);
        if (what == sim_handle_single)
            el = new CCollectionElement(objectHandle, sim_collectionelement_loose, false);
        if (what == sim_handle_tree)
        {
            int what = sim_collectionelement_frombaseincluded;
            if (excludeObj)
                what = sim_collectionelement_frombaseexcluded;
            el = new CCollectionElement(objectHandle, what, false);
        }
        if (what == sim_handle_chain)
        {
            int what = sim_collectionelement_fromtipincluded;
            if (excludeObj)
                what = sim_collectionelement_fromtipexcluded;
            el = new CCollectionElement(objectHandle, what, false);
        }
        if (el == nullptr)
            return getInvalidArgString(1);
        target->addCollectionElement(el);
    }
    return errMsg;
}

std::string _method_checkCollision(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (doesEntityExist(targetObj, &errMsg, -1) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional}))
    {
        int otherEntity = fetchHandle(inStack, 0, sim_handle_all);
        if ((otherEntity == sim_handle_all) || doesEntityExist(otherEntity, &errMsg, 0))
        {
            int collidingIds[2] = {-1, -1};
            if (otherEntity == sim_handle_all)
                otherEntity = -1;
            pushBool(outStack, CCollisionRoutine::doEntitiesCollide(targetObj, otherEntity, nullptr, true, true, collidingIds));
            pushShortHandleArray(outStack, collidingIds, 2);
        }
    }
    return errMsg;
}

std::string _method_checkDistance(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (doesEntityExist(targetObj, &errMsg, -1) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_double | arg_optional}))
    {
        int otherEntity = fetchHandle(inStack, 0, sim_handle_all);
        double threshold = fetchDouble(inStack, 1, 0.0);
        if ((otherEntity == sim_handle_all) || doesEntityExist(otherEntity, &errMsg, 0))
        {
            int distIds[2] = {-1, -1};
            if (otherEntity == sim_handle_all)
                otherEntity = -1;
            int buffer[4];
            App::currentWorld->cacheData->getCacheDataDist(targetObj, otherEntity, buffer);
            if (threshold <= 0.0)
                threshold = DBL_MAX;
            double distanceData[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            bool result = CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(targetObj, otherEntity, threshold, distanceData, buffer, buffer + 2, true, true);
            App::currentWorld->cacheData->setCacheDataDist(targetObj, otherEntity, buffer);
            if (result)
            {
                distIds[0] = buffer[0];
                distIds[1] = buffer[2];
            }
            pushBool(outStack, result);
            pushDouble(outStack, distanceData[6]);
            pushVector3(outStack, C3Vector(distanceData));
            pushVector3(outStack, C3Vector(distanceData + 3));
            pushShortHandleArray(outStack, distIds, 2);
        }
    }
    return errMsg;
}

std::string _method_checkSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CProxSensor* target = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_integer | arg_optional, arg_double | arg_optional}))
    {
        int entity = fetchHandle(inStack, 0, sim_handle_all);
        int options = fetchInt(inStack, 1, -1);
        double maxNormal = fetchDouble(inStack, 2, 0.0);
        if ((entity == sim_handle_all) || doesEntityExist(entity, &errMsg, 0))
        {
            if (entity == sim_handle_all)
                entity = -1;
            if (options == -1)
            {
                options = 0;
                if (target->getFrontFaceDetection())
                    options = options | 1;
                if (target->getBackFaceDetection())
                    options = options | 2;
                if (!target->getExactMode())
                    options = options | 4;
            }
            if (maxNormal == 0.0)
                maxNormal = target->getAllowedNormal();
            if (maxNormal != 0.0)
                options = options | 8;
            bool frontFace = (options & 1);
            bool backFace = (options & 2);
            if (!(frontFace || backFace))
                frontFace = true;
            bool fastDetection = (options & 4);
            int detectedObj;
            C3Vector dPoint;
            dPoint.clear();
            double minThreshold = -1.0;
            if (target->convexVolume->getSmallestDistanceAllowed() > 0.0)
                minThreshold = target->convexVolume->getSmallestDistanceAllowed();
            C3Vector normV;
            normV.clear();
            double dist = DBL_MAX;
            bool detected = CProxSensorRoutine::detectEntity(targetObj, entity, !fastDetection, maxNormal > 0.0, maxNormal, dPoint, dist, frontFace, backFace, detectedObj, minThreshold, normV, true);
            if (!detected)
                dist = 0.0;
            pushBool(outStack, detected);
            pushDouble(outStack, dist);
            pushVector3(outStack, dPoint);
            pushHandle(outStack, detectedObj);
            pushVector3(outStack, normV);
        }
    }
    return errMsg;
}

std::string _method_resetSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CProxSensor* target = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        target->resetSensor(false);
    }
    return errMsg;
}

std::string _method_handleSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CProxSensor* target = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        C3Vector smallest;
        smallest.clear();
        double smallestL = 0.0;
        int detectedObj = -1;
        C3Vector detectedN;
        detectedN.clear();
        bool detected = target->handleSensor(false, detectedObj, detectedN);
        pushBool(outStack, detected);
        if (detected)
        {
            smallest = target->getDetectedPoint();
            smallestL = smallest.getLength();
        }
        pushDouble(outStack, smallestL);
        pushVector3(outStack, smallest);
        pushHandle(outStack, detectedObj);
        pushVector3(outStack, detectedN);
    }
    return errMsg;
}

std::string _method_getObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((targetObj == sim_handle_app) || (targetObj == sim_handle_scene))
    {
        if (checkInputArguments(method, inStack, &errMsg, {arg_table | arg_optional, -1, arg_string}))
        {
            std::vector<std::string> types;
            fetchTextArray(inStack, 0, types);
            std::vector<int> objects;
            if (types.size() == 0)
                types = {"sceneObject", "drawingObject", "collection", "detachedScript", "mesh"};
            for (size_t j = 0; j < types.size(); j++)
            {
                std::string t = types[j];
                if (t == "sceneObject")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
                }
                else if (t == "shape")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getShapeFromIndex(i)->getObjectHandle());
                }
                else if (t == "joint")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_joint); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getJointFromIndex(i)->getObjectHandle());
                }
                else if (t == "dummy")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_dummy); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getDummyFromIndex(i)->getObjectHandle());
                }
                else if (t == "script")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_script); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getScriptFromIndex(i)->getObjectHandle());
                }
                else if (t == "marker")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_marker); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getMarkerFromIndex(i)->getObjectHandle());
                }
                else if (t == "mirror")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_mirror); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getMirrorFromIndex(i)->getObjectHandle());
                }
                else if (t == "graph")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_graph); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getGraphFromIndex(i)->getObjectHandle());
                }
                else if (t == "light")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_light); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getLightFromIndex(i)->getObjectHandle());
                }
                else if (t == "camera")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_camera); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getCameraFromIndex(i)->getObjectHandle());
                }
                else if (t == "proximitySensor")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_proximitysensor); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getProximitySensorFromIndex(i)->getObjectHandle());
                }
                else if (t == "visionSensor")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_visionsensor); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getVisionSensorFromIndex(i)->getObjectHandle());
                }
                else if (t == "path")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_path); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getPathFromIndex(i)->getObjectHandle());
                }
                else if (t == "mill")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_mill); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getMillFromIndex(i)->getObjectHandle());
                }
                else if (t == "forceSensor")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_forcesensor); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getForceSensorFromIndex(i)->getObjectHandle());
                }
                else if (t == "ocTree")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_octree); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getOctreeFromIndex(i)->getObjectHandle());
                }
                else if (t == "pointCloud")
                {
                    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_pointcloud); i++)
                        objects.push_back(App::currentWorld->sceneObjects->getPointCloudFromIndex(i)->getObjectHandle());
                }
                else if (t == "drawingObject")
                {
                    for (size_t i = 0; i < App::currentWorld->drawingCont->getObjectCount(); i++)
                        objects.push_back(App::currentWorld->drawingCont->getObjectFromIndex(i)->getObjectId());
                }
                else if (t == "collection")
                {
                    for (size_t i = 0; i < App::currentWorld->collections->getObjectCount(); i++)
                        objects.push_back(App::currentWorld->collections->getObjectFromIndex(i)->getCollectionHandle());
                }
                else if (t == "detachedScript")
                {
                    if (App::worldContainer->sandboxScript != nullptr)
                        objects.push_back(App::worldContainer->sandboxScript->getScriptHandle());
                    std::vector<int> addOns = App::worldContainer->addOnScriptContainer->getAddOnHandles();
                    objects.insert(objects.end(), addOns.begin(), addOns.end());
                    objects.push_back(App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript()->getScriptHandle());
                }
                else if (t == "mesh")
                {
                    std::vector<CMesh*> meshes;
                    App::currentWorld->sceneObjects->getAllMeshes(meshes);
                    for (size_t i = 0; i < meshes.size(); i++)
                        objects.push_back(int(meshes[i]->getUniqueID()));
                }
                else
                {
                    errMsg = "invalid object type(s).";
                    break;
                }
            }
            if (errMsg == "")
                pushShortHandleArray(outStack, objects.data(), objects.size());
        }
    }

    return errMsg;
}

std::string _method_addItems(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_vector3, arg_table | arg_optional, -1, arg_color, arg_table | arg_optional, -1, arg_quaternion, arg_table | arg_optional, -1, arg_vector3}))
    {
        std::vector<float> pts;
        fetchArrayAsConsecutiveNumbers(inStack, 0, pts);
        std::vector<float> ccols;
        fetchArrayAsConsecutiveNumbers(inStack, 1, ccols);
        std::vector<unsigned char> cols;
        for (size_t i = 0; i < ccols.size() / 3; i++)
        {
            cols.push_back((unsigned char)(ccols[3 * i + 0] * 255.1f));
            cols.push_back((unsigned char)(ccols[3 * i + 1] * 255.1f));
            cols.push_back((unsigned char)(ccols[3 * i + 2] * 255.1f));
            cols.push_back(255);
        }
        std::vector<float> quats;
        fetchArrayAsConsecutiveNumbers(inStack, 2, quats);
        std::vector<float> sizes;
        fetchArrayAsConsecutiveNumbers(inStack, 3, sizes);
        target->addItems(&pts, &quats, &cols, &sizes);
    }
    return errMsg;
}

std::string _method_clearItems(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        target->remItems(0);
    }
    return errMsg;
}
