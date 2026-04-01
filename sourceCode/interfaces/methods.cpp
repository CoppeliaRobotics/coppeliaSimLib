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
#include <interfaceStackHandleArray.h>
#include <interfaceStackColor.h>
#include <sceneObjectOperations.h>
#include <imgLoaderSaver.h>
#include <boost/algorithm/string.hpp>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif
#include <fileOperations.h>
#include <collisionRoutines.h>
#include <distanceRoutines.h>
#include <proxSensorRoutine.h>
#include <utils.h>

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
    constexpr int arg_handlearray   = sim_stackitem_handlearray;
    constexpr int arg_vector        = sim_stackitem_exvector;
    constexpr int arg_vector2       = sim_stackitem_exvector2;
    constexpr int arg_vector3       = sim_stackitem_exvector3;
    constexpr int arg_any           = sim_stackitem_exany;
    constexpr int arg_optional      = sim_stackitem_exoptional;
}

std::string callMethod(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    static std::map<std::string, std::function<std::string(int, const char*, CDetachedScript*, const CInterfaceStack*, CInterfaceStack*)>> funcTable;
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
        funcTable["_remove"] = _method__remove;
        funcTable["_removeObjects"] = _method__removeObjects;
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
        funcTable["removeItems"] = _method_removeItems;
        funcTable["callFunction"] = _method_callFunction;
        funcTable["executeString"] = _method_executeString;
        funcTable["getApiInfo"] = _method_getApiInfo;
        funcTable["getApiFunc"] = _method_getApiFunc;
        funcTable["getStackTraceback"] = _method_getStackTraceback;
        funcTable["init"] = _method_init;
        funcTable["scale"] = _method_scale;
        funcTable["scaleTree"] = _method_scaleTree;
        funcTable["startSimulation"] = _method_startSimulation;
        funcTable["pauseSimulation"] = _method_pauseSimulation;
        funcTable["stopSimulation"] = _method_stopSimulation;
        funcTable["getName"] = _method_getName;
        funcTable["dynamicReset"] = _method_dynamicReset;
        funcTable["loadImage"] = _method_loadImage;
        funcTable["loadImageFromBuffer"] = _method_loadImageFromBuffer;
        funcTable["saveImage"] = _method_saveImage;
        funcTable["saveImageToBuffer"] = _method_saveImageToBuffer;
        funcTable["transformImage"] = _method_transformImage;
        funcTable["getImage"] = _method_getImage;
        funcTable["setImage"] = _method_setImage;
        funcTable["getDepth"] = _method_getDepth;
        funcTable["relocateFrame"] = _method_relocateFrame;
        funcTable["alignBoundingBox"] = _method_alignBoundingBox;
        funcTable["logInfo"] = _method_logInfo;
        funcTable["logWarn"] = _method_logWarn;
        funcTable["logError"] = _method_logError;
        funcTable["quit"] = _method_quit;
        funcTable["systemLock"] = _method_systemLock;
        funcTable["setStepping"] = _method_setStepping;
        funcTable["getStepping"] = _method_getStepping;
        funcTable["getObject"] = _method_getObject;
        funcTable["announceChange"] = _method_announceChange;
        funcTable["getObjectFromUid"] = _method_getObjectFromUid;
        funcTable["getInertia"] = _method_getInertia;
        funcTable["setInertia"] = _method_setInertia;
        funcTable["computeInertia"] = _method_computeInertia;
        funcTable["addForce"] = _method_addForce;
        funcTable["addTorque"] = _method_addTorque;
        funcTable["ungroup"] = _method_ungroup;
        funcTable["divide"] = _method_divide;
        funcTable["packTable"] = _method_packTable;
        funcTable["unpackTable"] = _method_unpackTable;
        funcTable["pack"] = _method_pack;
        funcTable["unpack"] = _method_unpack;
        funcTable["packDoubleTable"] = _method_packDoubleTable;
        funcTable["packFloatTable"] = _method_packFloatTable;
        funcTable["packInt64Table"] = _method_packInt64Table;
        funcTable["packInt32Table"] = _method_packInt32Table;
        funcTable["packUInt32Table"] = _method_packUInt32Table;
        funcTable["packInt16Table"] = _method_packInt16Table;
        funcTable["packUInt16Table"] = _method_packUInt16Table;
        funcTable["packInt8Table"] = _method_packInt8Table;
        funcTable["packUInt8Table"] = _method_packUInt8Table;
        funcTable["unpackDoubleTable"] = _method_unpackDoubleTable;
        funcTable["unpackFloatTable"] = _method_unpackFloatTable;
        funcTable["unpackInt64Table"] = _method_unpackInt64Table;
        funcTable["unpackInt32Table"] = _method_unpackInt32Table;
        funcTable["unpackUInt32Table"] = _method_unpackUInt32Table;
        funcTable["unpackInt16Table"] = _method_unpackInt16Table;
        funcTable["unpackUInt16Table"] = _method_unpackUInt16Table;
        funcTable["unpackInt8Table"] = _method_unpackInt8Table;
        funcTable["unpackUInt8Table"] = _method_unpackUInt8Table;
        funcTable["groupShapes"] = _method_groupShapes;
        funcTable["mergeShapes"] = _method_mergeShapes;
        funcTable["_createCamera"] = _method__createCamera;
        funcTable["_createLight"] = _method__createLight;
        funcTable["_createGraph"] = _method__createGraph;
        funcTable["getBoolProperty"] = _method_getBoolProperty;
        funcTable["getBufferProperty"] = _method_getBufferProperty;
        funcTable["getColorProperty"] = _method_getColorProperty;
        funcTable["getFloatArrayProperty"] = _method_getFloatArrayProperty;
        funcTable["getFloatProperty"] = _method_getFloatProperty;
        funcTable["getStringArrayProperty"] = _method_getStringArrayProperty;
        funcTable["getHandleArrayProperty"] = _method_getHandleArrayProperty;
        funcTable["getHandleProperty"] = _method_getHandleProperty;
        funcTable["getIntArray2Property"] = _method_getIntArray2Property;
        funcTable["getIntArrayProperty"] = _method_getIntArrayProperty;
        funcTable["getIntProperty"] = _method_getIntProperty;
        funcTable["getLongProperty"] = _method_getLongProperty;
        funcTable["getPoseProperty"] = _method_getPoseProperty;
        funcTable["getQuaternionProperty"] = _method_getQuaternionProperty;
        funcTable["getStringProperty"] = _method_getStringProperty;
        funcTable["getVector2Property"] = _method_getVector2Property;
        funcTable["getVector3Property"] = _method_getVector3Property;
        funcTable["setBoolProperty"] = _method_setBoolProperty;
        funcTable["setBufferProperty"] = _method_setBufferProperty;
        funcTable["setColorProperty"] = _method_setColorProperty;
        funcTable["setFloatArrayProperty"] = _method_setFloatArrayProperty;
        funcTable["setFloatProperty"] = _method_setFloatProperty;
        funcTable["setStringArrayProperty"] = _method_setStringArrayProperty;
        funcTable["setHandleArrayProperty"] = _method_setHandleArrayProperty;
        funcTable["setHandleProperty"] = _method_setHandleProperty;
        funcTable["setIntArray2Property"] = _method_setIntArray2Property;
        funcTable["setIntArrayProperty"] = _method_setIntArrayProperty;
        funcTable["setIntProperty"] = _method_setIntProperty;
        funcTable["setLongProperty"] = _method_setLongProperty;
        funcTable["setPoseProperty"] = _method_setPoseProperty;
        funcTable["setQuaternionProperty"] = _method_setQuaternionProperty;
        funcTable["setStringProperty"] = _method_setStringProperty;
        funcTable["setVector2Property"] = _method_setVector2Property;
        funcTable["setVector3Property"] = _method_setVector3Property;
        funcTable["getMatrixProperty"] = _method_getMatrixProperty;
        funcTable["setMatrixProperty"] = _method_setMatrixProperty;
        funcTable["getMatrix3x3Property"] = _method_getMatrix3x3Property;
        funcTable["setMatrix3x3Property"] = _method_setMatrix3x3Property;
        funcTable["getMatrix4x4Property"] = _method_getMatrix4x4Property;
        funcTable["setMatrix4x4Property"] = _method_setMatrix4x4Property;
        funcTable["_getTableProperty"] = _method__getTableProperty;
        funcTable["_setTableProperty"] = _method__setTableProperty;
        funcTable["removeProperty"] = _method_removeProperty;
        funcTable["getPropertyName"] = _method_getPropertyName;
        funcTable["getPropertyInfo"] = _method_getPropertyInfo;
        funcTable["createCustomObject"] = _method_createCustomObject;
        funcTable["releaseCustomObject"] = _method_releaseCustomObject;
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
    // The type argument can be combined with arg_optional, in which case the arg can be missing or nil
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
            if (optional && (t == arg_null))
                retVal = true;
            else if (desiredArgType == arg_table)
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
                        else if ( (desiredArgType == arg_vector2) && (m->getValue()->cols == 1) && (m->getValue()->rows == 2) )
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
                                if (desiredArgType == arg_handlearray)
                                    retVal = tbl->areAllValuesThis(arg_handle, true);
                                else
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
                                        else if (desiredArgType == arg_vector2)
                                            retVal = (tbl->getArraySize() == 2);
                                        else if (desiredArgType == arg_vector3)
                                            retVal = (tbl->getArraySize() == 3);
                                        else
                                            retVal = false;
                                    }
                                    else
                                        retVal = false;
                                }
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
                            else if (desiredArgType == arg_vector2)
                                msg += "a vector2";
                            else if (desiredArgType == arg_handlearray)
                                msg += "a handle/object array";
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

bool hasNonNullArg(const CInterfaceStack* inStack, int index)
{
    bool retVal = false;
    if (inStack->getStackSize() > index)
        retVal = (inStack->getStackObjectFromIndex(index)->getObjectType() != sim_stackitem_null);
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

void fetchLongArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr)
{
    outArr.clear();
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

void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr)
{
    outArr.clear();
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
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        {
            const CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t cnt;
            const long long int* v = arr->getValue(&cnt);
            outArr.assign(v, v + cnt);
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
            if (matr->cols == 1)
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

void pushHandle(CInterfaceStack* outStack, long long int v)
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

void pushVector2(CInterfaceStack* outStack, const double v[2])
{
    outStack->pushMatrixOntoStack(v, 2, 1);
}

void pushMatrix(CInterfaceStack* outStack, const CMatrix& v)
{
    outStack->pushMatrixOntoStack(v.data.data(), v.rows, v.cols);
}

void pushIntArray(CInterfaceStack* outStack, const int* v, size_t length)
{
    outStack->pushInt32ArrayOntoStack(v, length);
}

void pushLongArray(CInterfaceStack* outStack, const long long int* v, size_t length)
{
    outStack->pushInt64ArrayOntoStack(v, length);
}

void pushHandleArray(CInterfaceStack* outStack, const long long int* v, size_t length)
{
    outStack->pushHandleArrayOntoStack(v, length);
}

void pushShortHandleArray(CInterfaceStack* outStack, const int* v, size_t length)
{
    outStack->pushShortHandleArrayOntoStack(v, length);
}

void pushFloatArray(CInterfaceStack* outStack, const float* v, size_t length)
{
    outStack->pushFloatArrayOntoStack(v, length);
}

void pushDoubleArray(CInterfaceStack* outStack, const double* v, size_t length)
{
    outStack->pushDoubleArrayOntoStack(v, length);
}

void pushTextArray(CInterfaceStack* outStack, const std::string* v, size_t length)
{
    outStack->pushTextArrayOntoStack(v, length);
}

void pushObject(CInterfaceStack* outStack, CInterfaceStackObject* obj)
{
    outStack->pushObjectOntoStack(obj);
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

CDetachedScript* getDetachedScript(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CDetachedScript* retVal = nullptr;
    if (identifier > SIM_IDEND_SCENEOBJECT)
        retVal = App::worldContainer->getDetachedScriptFromHandle(identifier);
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


std::string _method_test(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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
        printf("string array arg: size %zu, data: %s, %s\n", strArrArg.size(), strArrArg[0].c_str(), strArrArg[1].c_str());
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
        printf("Vector arg: size %zu, data: %f, %f\n", vectArg.size(), vectArg[0], vectArg[1]);
    }

//    outStack->copyFrom(inStack);
//    std::string str;
//    outStack->printContent(-1, str);
//    printf(str.c_str());
    return errMsg;
}

std::string _method_getPosition(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setPosition(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_getQuaternion(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setQuaternion(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_getPose(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setPose(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_setParent(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_handleSandboxScript(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_handleAddOnScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_handleSimulationScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_handleCustomizationScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadModelFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadModelThumbnail(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadModelThumbnailFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_saveModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_saveModelToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadScene(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_loadSceneFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_saveScene(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_saveSceneToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_removeModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method__remove(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_bool | arg_optional}))
    {
        bool delayed = fetchBool(inStack, 0, false);
        CSceneObject* sceneObj = getSceneObject(targetObj);
        CCollection* coll = getCollection(targetObj);
        CDrawingObject* draw = getDrawingObject(targetObj);
        CDetachedScript* script = getDetachedScript(targetObj);
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

std::string _method__removeObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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
            CDetachedScript* script = getDetachedScript(objectHandle);
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

std::string _method_duplicateObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_addItem(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_removeItem(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_checkCollision(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_checkDistance(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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

std::string _method_checkSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* visionSensor = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, nullptr, -1);
    CProxSensor* proxSensor = nullptr;
    if (visionSensor == nullptr)
        proxSensor = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((visionSensor != nullptr) || (proxSensor != nullptr))
    {
        if (proxSensor != nullptr)
        {
            if (checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional, arg_integer | arg_optional, arg_double | arg_optional}))
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
                        if (proxSensor->getFrontFaceDetection())
                            options = options | 1;
                        if (proxSensor->getBackFaceDetection())
                            options = options | 2;
                        if (!proxSensor->getExactMode())
                            options = options | 4;
                    }
                    if (maxNormal == 0.0)
                        maxNormal = proxSensor->getAllowedNormal();
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
                    if (proxSensor->convexVolume->getSmallestDistanceAllowed() > 0.0)
                        minThreshold = proxSensor->convexVolume->getSmallestDistanceAllowed();
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
        }
        if (visionSensor != nullptr)
        {
            if (checkInputArguments(method, inStack, &errMsg, {arg_handle | arg_optional}))
            {
                int entity = fetchHandle(inStack, 0, sim_handle_all);
                if ((entity == sim_handle_all) || doesEntityExist(entity, &errMsg, 0))
                {
                    bool detection;
                    std::vector<std::vector<double>> packets;
                    visionSensor->checkSensor(entity, true, &detection, &packets);
                    pushBool(outStack, detection);
                    if (packets.size() >= 1)
                        pushDoubleArray(outStack, packets[0].data(), packets[0].size());
                    else
                        pushDoubleArray(outStack, nullptr, 0);
                    if (packets.size() >= 2)
                        pushDoubleArray(outStack, packets[1].data(), packets[1].size());
                    else
                        pushDoubleArray(outStack, nullptr, 0);
                }
            }
        }
    }
    else
        errMsg = SIM_ERROR_METHOD_NOT_AVAILABLE_FOR_THAT_OBJECT;
    return errMsg;
}

std::string _method_resetSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* visionSensor = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, nullptr, -1);
    CProxSensor* proxSensor = nullptr;
    if (visionSensor == nullptr)
        proxSensor = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((visionSensor != nullptr) || (proxSensor != nullptr))
    {
        if (checkInputArguments(method, inStack, &errMsg, {}))
        {
            if (visionSensor != nullptr)
                visionSensor->resetSensor();
            if (proxSensor != nullptr)
                proxSensor->resetSensor(false);
        }
    }
    else
        errMsg = SIM_ERROR_METHOD_NOT_AVAILABLE_FOR_THAT_OBJECT;
    return errMsg;
}

std::string _method_handleSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* visionSensor = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, nullptr, -1);
    CProxSensor* proxSensor = nullptr;
    if (visionSensor == nullptr)
        proxSensor = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((visionSensor != nullptr) || (proxSensor != nullptr))
    {
        if (checkInputArguments(method, inStack, &errMsg, {}))
        {
            if (proxSensor != nullptr)
            {
                C3Vector smallest;
                smallest.clear();
                double smallestL = 0.0;
                int detectedObj = -1;
                C3Vector detectedN;
                detectedN.clear();
                bool detected = proxSensor->handleSensor(false, detectedObj, detectedN);
                pushBool(outStack, detected);
                if (detected)
                {
                    smallest = proxSensor->getDetectedPoint();
                    smallestL = smallest.getLength();
                }
                pushDouble(outStack, smallestL);
                pushVector3(outStack, smallest);
                pushHandle(outStack, detectedObj);
                pushVector3(outStack, detectedN);
            }
            if (visionSensor != nullptr)
            {
                bool detection = visionSensor->handleSensor();
                pushBool(outStack, detection);
                if (visionSensor->sensorAuxiliaryResult.size() >= 1)
                    pushDoubleArray(outStack, visionSensor->sensorAuxiliaryResult[0].data(), visionSensor->sensorAuxiliaryResult[0].size());
                else
                    pushDoubleArray(outStack, nullptr, 0);
                if (visionSensor->sensorAuxiliaryResult.size() >= 2)
                    pushDoubleArray(outStack, visionSensor->sensorAuxiliaryResult[1].data(), visionSensor->sensorAuxiliaryResult[1].size());
                else
                    pushDoubleArray(outStack, nullptr, 0);
            }
        }
    }
    else
        errMsg = SIM_ERROR_METHOD_NOT_AVAILABLE_FOR_THAT_OBJECT;
    return errMsg;
}

std::string _method_getObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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
                        objects.push_back(App::currentWorld->drawingCont->getObjectFromIndex(i)->getObjectHandle());
                }
                else if (t == "collection")
                {
                    for (size_t i = 0; i < App::currentWorld->collections->getObjectCount(); i++)
                        objects.push_back(int(App::currentWorld->collections->getObjectFromIndex(i)->getObjectHandle()));
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
                        objects.push_back(int(meshes[i]->getObjectHandle()));
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

std::string _method_addItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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
        std::vector<long long int> newIds;
        target->addItems(&pts, &quats, &cols, &sizes, true, &newIds);
        pushLongArray(outStack, newIds.data(), newIds.size());
    }
    return errMsg;
}

std::string _method_clearItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        target->remItems(0);
    }
    return errMsg;
}

std::string _method_removeItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_integer}))
    {
        std::vector<long long int> ids;
        fetchLongArray(inStack, 0, ids);
        target->remItems(&ids);
    }
    return errMsg;
}

std::string _method_callFunction(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string funcName = fetchText(inStack, 0);
        if (target->hasInterpreterState())
        {
            CInterfaceStack* inStack2 = App::worldContainer->interfaceStackContainer->createStackCopy(inStack);
            delete inStack2->detachStackObjectFromIndex(0);
            int rr = target->callCustomScriptFunction(funcName.c_str(), inStack2, outStack);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack2);
            if (rr != 1)
            {
                if (rr == -1)
                    errMsg = SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION;
                else // rr==0
                    errMsg = SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
            }
        }
        else
            errMsg = SIM_ERROR_SCRIPT_NOT_INITIALIZED;
    }
    return errMsg;
}

std::string _method_executeString(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string stringToExecute = fetchText(inStack, 0);
        if (target->hasInterpreterState())
        {
            int lang = sim_lang_undefined;
            if (boost::algorithm::ends_with(stringToExecute.c_str(), "@lua"))
            {
                lang = sim_lang_lua;
                stringToExecute.resize(stringToExecute.size() - 4);
            }
            else if (boost::algorithm::ends_with(stringToExecute.c_str(), "@python"))
            {
                lang = sim_lang_python;
                stringToExecute.resize(stringToExecute.size() - 7);
            }

            int retVal = -1; // error
            if ((target->getLang() == "lua") || (lang == sim_lang_lua))
                retVal = target->executeScriptString(stringToExecute.c_str(), outStack);
            else
            {
                if (target->getLang() == "python")
                {
                    if (target->getScriptState() == CDetachedScript::scriptState_initialized)
                    {
                        CInterfaceStack* tmpStack = App::worldContainer->interfaceStackContainer->createStack();
                        tmpStack->pushTextOntoStack(stringToExecute.c_str());
                        retVal = target->callCustomScriptFunction("_evalExecRet", tmpStack, outStack);
                        App::worldContainer->interfaceStackContainer->destroyStack(tmpStack);
                        if (retVal == 1)
                        {
                            retVal = 0;
                            CInterfaceStackObject* obj = outStack->getStackObjectFromIndex(0);
                            if (obj->getObjectType() == sim_stackitem_string)
                            {
                                CInterfaceStackString* str = (CInterfaceStackString*)obj;
                                std::string tmp(str->getValue(nullptr));
                                if (tmp == "_*empty*_")
                                    outStack->clear();
                            }
                        }
                    }
                    else
                        errMsg = SIM_ERROR_SCRIPT_NOT_INITIALIZED;
                }
                else
                    errMsg = "script has unsupported language.";
            }
            if (retVal != 0)
                errMsg = SIM_ERROR_OPERATION_FAILED;
        }
        else
            errMsg = SIM_ERROR_SCRIPT_NOT_INITIALIZED;
    }
    return errMsg;
}

std::string _method_getApiInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string apiWord = fetchText(inStack, 0);
        if (apiWord.size() > 0)
        {
            std::string tip(CDetachedScript::getFunctionCalltip(apiWord.c_str(), target));
            outStack->pushTextOntoStack(tip.c_str());
        }
        else
            errMsg = SIM_ERROR_INVALID_ARGUMENTS;
    }
    return errMsg;
}

std::string _method_getApiFunc(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string apiW = fetchText(inStack, 0);
        bool funcs = true;
        bool vars = true;
        if (apiW.size() > 0)
        {
            if ((apiW[0] == '+') || (apiW[0] == '-'))
            {
                vars = (apiW[0] != '+');
                funcs = (apiW[0] != '-');
                apiW.erase(0, 1);
            }
        }
        std::set<std::string> t;
        if (funcs)
            CDetachedScript::getMatchingFunctions(apiW.c_str(), t, target);
        if (vars)
            CDetachedScript::getMatchingConstants(apiW.c_str(), t, target);
        std::vector<std::string> theWords;
        for (const auto& str : t)
            theWords.push_back(str);
        outStack->pushTextArrayOntoStack(theWords.data(), theWords.size());
    }
    return errMsg;
}

std::string _method_getStackTraceback(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        outStack->pushTextOntoStack(target->getAndClearLastStackTraceback().c_str());
    }
    return errMsg;
}

std::string _method_init(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        if (currentScript == target)
            App::asyncResetScript(targetObj); // delayed
        else
            target->initScript();
    }
    return errMsg;
}

std::string _method_scale(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_vector3}))
    {
        C3Vector s = fetchVector3(inStack, 0);
        if ((s(0) >= 0.0001) && (s(1) >= 0.0001) && (s(2) >= 0.0001))
            target->scaleObjectNonIsometrically(s(0), s(1), s(2));
        else
            errMsg = SIM_ERROR_INVALID_INPUT;
    }
    return errMsg;
}

std::string _method_scaleTree(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if (checkInputArguments(method, inStack, &errMsg, {arg_double, arg_bool | arg_optional}))
    {
        double scalingFactor = fetchDouble(inStack, 0);
        bool rootPositionIsScaled = fetchBool(inStack, 1, true);
        std::vector<int> sel;
        target->getAllHandlesRecursive(&sel);
        if (scalingFactor >= 0.0001)
        {
            CSceneObjectOperations::scaleObjects(sel, scalingFactor, true, !rootPositionIsScaled);
#ifdef SIM_WITH_GUI
            GuiApp::setFullDialogRefreshFlag();
#endif
        }
        else
            errMsg = SIM_ERROR_INVALID_INPUT;
    }
    return errMsg;
}

std::string _method_startSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {}))
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
            App::currentWorld->simulation->startOrResumeSimulation();
    }
    return errMsg;
}

std::string _method_pauseSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {}))
    {
        if (App::currentWorld->simulation->isSimulationRunning())
            App::currentWorld->simulation->pauseSimulation();
    }
    return errMsg;
}

std::string _method_stopSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {}))
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            App::currentWorld->simulation->incrementStopRequestCounter();
            App::currentWorld->simulation->stopSimulation();
        }
    }
    return errMsg;
}

std::string _method_getName(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_integer | arg_optional}))
    {
        int t = fetchInt(inStack, 0, -1);
        std::string nm;
        if (t == -1)
            nm = target->getObjectAlias(); // just the alias, e.g. "alias"
        if (t == 0)
            nm = target->getObjectAliasAndOrderIfRequired(); // the alias with order, e.g. "alias[0]"
        if (t == 1)
            nm = target->getObjectAlias_shortPath(); // the alias with unique path, short, e.g. "/obj/alias[0]"
        if (t == 2)
            nm = target->getObjectAlias_fullPath(); // the alias with full path, e.g. "/obj/obj2/alias[0]"
        if (t == 3)
        { // just the alias, if unique, e.g. "alias", otherwise the alias with handle, e.g. "alias__42__"
            if (App::currentWorld->sceneObjects->getObjectFromPath(
                    nullptr, (std::string("/") + target->getObjectAlias()).c_str(), 1) == nullptr)
                nm = target->getObjectAlias();
            else
                t = 4;
        }
        if (t == 4)
        { // the alias with object handle, e.g. "alias__42__"
            nm = target->getObjectAlias() + "__";
            nm += std::to_string(target->getObjectHandle());
            nm += "__";
        }
        if (t == 5)
            nm = target->getObjectAlias_printPath(); // the print version, not guaranteed to be unique, e.g.
                                                 // "/obj/.../alias[0]"
        if (t == 6)
            nm = target->getObjectPathAndIndex(0); // the path with index, e.g. "/alias{3}"
        if (t == 7)
            nm = target->getObjectPathAndIndex(1); // the path with index, e.g. "/parentModel{1}/alias{3}"
        if (t == 8)
            nm = target->getObjectPathAndIndex(2); // the path with index, e.g. "/greatParent{0}/parentModel{1}/alias{3}"
        if (t == 9)
            nm = target->getObjectPathAndIndex(999); // the path with index, e.g. "/.../.../greatParent{0}/parentModel{1}/alias{3}"
        pushText(outStack, nm.c_str());
    }
    return errMsg;
}

std::string _method_dynamicReset(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_bool | arg_optional}))
    {
        bool wholeTree = fetchBool(inStack, 0, false);
        target->setDynamicsResetFlag(true, wholeTree);
    }
    return errMsg;
}

std::string _method_loadImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string filename = fetchText(inStack, 0);
        int res[2];
        bool hasAlpha;
        unsigned char* img = CImageLoaderSaver::load(res, -1, filename.c_str(), nullptr, &hasAlpha);
        if (img != nullptr)
        {
            int b = 3;
            if (hasAlpha)
                b = 4;
            pushBuffer(outStack, (char*)img, b * res[0] * res [1]);
            pushIntArray(outStack, res, 2);
            pushInt(outStack, b);
            delete[]((char*)img);
        }
        else
            errMsg = SIM_ERROR_FAILED_LOADING_IMAGE;
    }
    return errMsg;
}

std::string _method_loadImageFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        int res[2];
        bool hasAlpha;
        int reserved[1] = {(int)buff.size()};
        unsigned char* img = CImageLoaderSaver::load(res, -1, (char*)buff.c_str(), reserved, &hasAlpha);
        if (img != nullptr)
        {
            int b = 3;
            if (hasAlpha)
                b = 4;
            pushBuffer(outStack, (char*)img, b * res[0] * res [1]);
            pushIntArray(outStack, res, 2);
            pushInt(outStack, b);
            delete[]((char*)img);
        }
        else
            errMsg = SIM_ERROR_FAILED_LOADING_IMAGE;
    }
    return errMsg;
}

std::string _method_saveImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_string, arg_integer | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> res;
        fetchIntArray(inStack, 1, res);
        std::string filename = fetchText(inStack, 2);
        int quality = fetchInt(inStack, 3, -1);
        int options = 0;
        int channels = 3;
        if (img.size() == res[0] * res[1])
        {
            channels = 1;
            options = 2;
        }
        if (img.size() == (res[0] * res[1]) * 4)
        {
            channels = 4;
            options = 1;
        }
        if ((img.size() == res[0] * res[1] * channels) && (res[0] > 0) && (res[1] > 0))
        {
            if (filename.size() > 0)
            {
                if (!CImageLoaderSaver::save((unsigned char*)img.c_str(), res.data(), options, filename.c_str(), quality, nullptr))
                    errMsg = SIM_ERROR_OPERATION_FAILED;
            }
            else
                errMsg = SIM_ERROR_INVALID_FILENAME;
        }
        else
            errMsg = SIM_ERROR_INVALID_RESOLUTION;
    }
    return errMsg;
}

std::string _method_saveImageToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_string | arg_optional, arg_integer | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> res;
        fetchIntArray(inStack, 1, res);
        std::string ext = fetchText(inStack, 2, "png");
        ext = "." + ext;
        int quality = fetchInt(inStack, 3, -1);
        int options = 0;
        int channels = 3;
        if (img.size() == res[0] * res[1])
        {
            channels = 1;
            options = 2;
        }
        if (img.size() == (res[0] * res[1]) * 4)
        {
            channels = 4;
            options = 1;
        }
        if ((img.size() == res[0] * res[1] * channels) && (res[0] > 0) && (res[1] > 0))
        {
            std::string retBuff;
            if (CImageLoaderSaver::save((unsigned char*)img.c_str(), res.data(), options, ext.c_str(), quality, &retBuff))
                pushBuffer(outStack, retBuff.data(), retBuff.size());
            else
                errMsg = SIM_ERROR_OPERATION_FAILED;
        }
        else
            errMsg = SIM_ERROR_INVALID_RESOLUTION;
    }
    return errMsg;
}

std::string _method_transformImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_table, 2, arg_integer, arg_integer | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> inRes;
        fetchIntArray(inStack, 1, inRes);
        std::vector<int> outRes;
        fetchIntArray(inStack, 2, outRes);
        int options = fetchInt(inStack, 3, 0);
        int optionsS = options & 0x0003;
        options |= 1;
        options -= 1;
        int channels = 3;
        if (img.size() == inRes[0] * inRes[1])
            channels = 1;
        if (img.size() == (inRes[0] * inRes[1]) * 4)
        {
            channels = 4;
            options |= 1;
        }
        if ((img.size() == inRes[0] * inRes[1] * channels) && (inRes[0] > 0) && (inRes[1] > 0))
        {
            if (channels == 1)
            {
                std::string img2;
                img2.reserve(img.size() * 3);
                for (size_t i = 0; i < img.size(); i++)
                {
                    img2[3 * i + 0] = img[i];
                    img2[3 * i + 1] = img[i];
                    img2[3 * i + 2] = img[i];
                }
                img.swap(img2);
            }
            options |= 2;
            if ((optionsS == 0) || (optionsS == 2))
                options -= 2;
            unsigned char* retVal = CImageLoaderSaver::getScaledImage((unsigned char*)img.data(), inRes.data(), outRes.data(), options);
            std::vector<unsigned char> imgOut;
            channels = 3;
            if (optionsS == 0)
                imgOut.assign(retVal, retVal + outRes[0] * outRes[1] * 3);
            else if (optionsS == 1)
            {
                imgOut.assign(retVal, retVal + outRes[0] * outRes[1] * 4);
                channels = 4;
            }
            else if (optionsS == 2)
            {
                channels = 1;
                imgOut.resize(outRes[0] * outRes[1]);
                for (size_t i = 0; i < outRes[0] * outRes[1]; i++)
                {
                    int g = int(retVal[3 * i + 0]) + int(retVal[3 * i + 1]) + int(retVal[3 * i + 2]);
                    if (g > 255)
                        g = 255;
                    imgOut[i] = (unsigned char)g;
                }
            }
            delete[] retVal;
            std::vector<unsigned char> imgOut2(imgOut);
            for (int x = 0; x < outRes[0]; x++)
            {
                int x2 = x;
                if (options & 32)
                    x2 = outRes[0] - 1 - x;
                for (int y = 0; y < outRes[1]; y++)
                {
                    int y2 = y;
                    if (options & 64)
                        y2 = outRes[1] - 1 - y;
                    if (channels >= 1)
                        imgOut[channels * (x + y * outRes[0]) + 0] = imgOut2[channels * (x2 + y2 * outRes[0]) + 0];
                    if (channels >= 3)
                    {
                        imgOut[channels * (x + y * outRes[0]) + 1] = imgOut2[channels * (x2 + y2 * outRes[0]) + 1];
                        imgOut[channels * (x + y * outRes[0]) + 2] = imgOut2[channels * (x2 + y2 * outRes[0]) + 2];
                        if (channels >= 4)
                            imgOut[channels * (x + y * outRes[0]) + 3] = imgOut2[channels * (x2 + y2 * outRes[0]) + 3];
                    }
                }
            }
            pushBuffer(outStack, (char*)imgOut.data(), channels * outRes[0] * outRes[1]);
            pushIntArray(outStack, outRes.data(), 2);
            pushInt(outStack, channels);
        }
        else
            errMsg = SIM_ERROR_INVALID_RESOLUTION;
    }
    return errMsg;
}

std::string _method_getImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* target = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_table | arg_optional, 2, arg_integer, arg_table | arg_optional, 2, arg_integer, arg_integer | arg_optional, arg_double | arg_optional}))
    {
        std::vector<int> pos;
        fetchIntArray(inStack, 0, pos, {0, 0});
        std::vector<int> size;
        fetchIntArray(inStack, 1, size, {0, 0});
        int options = fetchInt(inStack, 2, 0); // rgb
        double rgbaCutOff = fetchDouble(inStack, 3, 0.999);
        int res[2];
        target->getResolution(res);
        options &= 0x003;
        int opts = options;
        if (options == 1)
            options = 2;
        else if (options == 2)
            options = 1;
        if (size[0] == 0)
            size[0] = res[0];
        if (size[1] == 0)
            size[1] = res[1];
        unsigned char* img = target->readPortionOfCharImage(pos[0], pos[1], size[0], size[1], rgbaCutOff, options);
        if (img != nullptr)
        {
            int s = 3;
            if (opts == 2)
                s = 1; // greyscale
            if (opts == 1)
                s = 4; // + alpha channel
            pushBuffer(outStack, (char*)img, s * size[0] * size[1]);
            delete[]((char*)img);
            pushIntArray(outStack, res, 2);
        }
        else
            errMsg = SIM_ERROR_INVALID_ARGUMENTS;
    }
    return errMsg;
}

std::string _method_setImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* target = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table | arg_optional, 2, arg_integer, arg_table | arg_optional, 2, arg_integer}))
    {
        std::vector<char> img;
        fetchBuffer(inStack, 0, img);
        std::vector<int> pos;
        fetchIntArray(inStack, 1, pos, {0, 0});
        std::vector<int> size;
        fetchIntArray(inStack, 2, size, {0, 0});
        int res[2];
        target->getResolution(res);
        if (size[0] == 0)
            size[0] = res[0];
        if (size[1] == 0)
            size[1] = res[1];
        int s = 0;
        int opt = 0;
        if (size[0] * size[1] == img.size())
        {
            s = 1;
            opt = 1;
        }
        else if (size[0] * size[1] * 3 == img.size())
            s = 3;
        else if (size[0] * size[1] * 4 == img.size())
        {
            s = 4;
            opt = 2;
        }
        if (s > 0)
        {
            if (!target->writePortionOfCharImage((unsigned char*)img.data(), pos[0], pos[1], size[0], size[1], opt))
                errMsg = SIM_ERROR_INVALID_ARGUMENTS;
        }
        else
            errMsg = SIM_ERROR_INCORRECT_BUFFER_SIZE;
    }
    return errMsg;
}

std::string _method_getDepth(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* target = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_table | arg_optional, 2, arg_integer, arg_table | arg_optional, 2, arg_integer}))
    {
        std::vector<int> pos;
        fetchIntArray(inStack, 0, pos, {0, 0});
        std::vector<int> size;
        fetchIntArray(inStack, 1, size, {0, 0});
        int res[2];
        target->getResolution(res);
        if ((size[0] == 0) && (size[1] == 0))
        {
            size[0] = res[0];
            size[1] = res[1];
        }
        float* buff = target->readPortionOfImage(pos[0], pos[1], size[0], size[1], 2);
        if (buff != nullptr)
        {
            double np, fp;
            target->getClippingPlanes(np, fp);
            float n = (float)np;
            float f = (float)fp;
            float fmn = f - n;
            for (int i = 0; i < size[0] * size[1]; i++)
                buff[i] = n + fmn * buff[i];
            pushFloatArray(outStack, buff, size[0] * size[1]);
            delete[]((char*)buff);
            pushIntArray(outStack, res, 2);
        }
        else
            errMsg = SIM_ERROR_INVALID_ARGUMENTS;
    }
    return errMsg;
}

std::string _method_relocateFrame(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* target = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_pose | arg_optional}))
    {
        C7Vector tr = fetchPose(inStack, 0);
        if ((!target->getMesh()->isPure()) || (target->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (hasNonNullArg(inStack, 0))
            {
                if ((tr.Q(0) == 0.0) && (tr.Q(1) == 0.0) && (tr.Q(2) == 0.0) && (tr.Q(3) == 0.0))
                    target->relocateFrame("mesh");
                else
                {
                    tr.Q.normalize();
                    C7Vector x(tr.getInverse() * target->getCumulativeTransformation());
                    target->setLocalTransformation(target->getFullParentCumulativeTransformation().getInverse() * x);
                    target->relocateFrame("world");
                    target->setLocalTransformation(target->getLocalTransformation() * tr);
                }
            }
            else
                target->relocateFrame("mesh");
        }
    }
    return errMsg;
}

std::string _method_alignBoundingBox(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* target = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_quaternion | arg_optional}))
    {
        C4Vector q = fetchQuaternion(inStack, 0);
        if ((!target->getMesh()->isPure()) || (target->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (hasNonNullArg(inStack, 0))
            {
                C7Vector tr;
                tr.X = C3Vector::zeroVector;
                tr.Q = q;
                if ((tr.Q(0) == 0.0) && (tr.Q(1) == 0.0) && (tr.Q(2) == 0.0) && (tr.Q(3) == 0.0))
                    target->alignBB("mesh");
                else
                {
                    tr.Q.normalize();
                    target->alignBB("custom", &tr);
                }
            }
            else
                target->alignBB("mesh");
        }
    }
    return errMsg;
}

std::string _method_logInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string | arg_optional, arg_table | arg_optional, 0, arg_any}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            if (hasNonNullArg(inStack, 1))
            {
                CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
                CInterfaceStackObject* obj = map->getMapObject("undecorated");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_undecorated;
                }
                obj = map->getMapObject("onlyterminal");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_onlyterminal;
                }
                obj = map->getMapObject("once");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_once;
                }
            }
            if (currentScript != nullptr)
            {
                verb += sim_verbosity_scriptinfos;
                App::logScriptMsg(currentScript, verb, msg.c_str());
            }
            else
            {
                verb += sim_verbosity_loadinfos;
                App::logMsg(verb, msg.c_str());
            }
        }
#ifdef SIM_WITH_GUI
        else
            GuiApp::clearStatusbar();
#endif
    }
    return errMsg;
}

std::string _method_logWarn(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string | arg_optional, arg_table | arg_optional, 0, arg_any}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            if (hasNonNullArg(inStack, 1))
            {
                CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
                CInterfaceStackObject* obj = map->getMapObject("undecorated");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_undecorated;
                }
                obj = map->getMapObject("onlyterminal");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_onlyterminal;
                }
                obj = map->getMapObject("once");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_once;
                }
            }
            if (currentScript != nullptr)
            {
                verb += sim_verbosity_scriptwarnings;
                App::logScriptMsg(currentScript, verb, msg.c_str());
            }
            else
            {
                verb += sim_verbosity_warnings;
                App::logMsg(verb, msg.c_str());
            }
        }
#ifdef SIM_WITH_GUI
        else
            GuiApp::clearStatusbar();
#endif
    }
    return errMsg;
}

std::string _method_logError(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string | arg_optional, arg_table | arg_optional, 0, arg_any}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            if (hasNonNullArg(inStack, 1))
            {
                CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
                CInterfaceStackObject* obj = map->getMapObject("undecorated");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_undecorated;
                }
                obj = map->getMapObject("onlyterminal");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_onlyterminal;
                }
                obj = map->getMapObject("once");
                if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                {
                    if (((CInterfaceStackBool*)obj)->getValue())
                        verb |= sim_verbosity_once;
                }
            }
            if (currentScript != nullptr)
            {
                verb += sim_verbosity_scripterrors;
                App::logScriptMsg(currentScript, verb, msg.c_str());
            }
            else
            {
                verb += sim_verbosity_errors;
                App::logMsg(verb, msg.c_str());
            }
        }
#ifdef SIM_WITH_GUI
        else
            GuiApp::clearStatusbar();
#endif
    }
    return errMsg;
}

std::string _method_quit(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {}))
    {
#ifdef SIM_WITH_GUI
        SSimulationThreadCommand cmd;
        cmd.cmdId = EXIT_REQUEST_CMD;
        App::appendSimulationThreadCommand(cmd);
#else
        App::postExitRequest();
#endif
    }
    return errMsg;
}

std::string _method_systemLock(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_bool}))
    {
        std::string key = fetchText(inStack, 0);
        bool acquire = fetchBool(inStack, 1);
        App::systemSemaphore(key.c_str(), acquire);
    }
    return errMsg;
}

std::string _method_setStepping(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_bool}))
    {
        bool enable = fetchBool(inStack, 0);
        if (enable)
            target->changeAutoYieldingForbidLevel(1, false);
        else
            target->changeAutoYieldingForbidLevel(-1, false);
    }
    return errMsg;
}

std::string _method_getStepping(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        pushBool(outStack, target->getAutoYieldingForbidLevel() > 0);
    }
    return errMsg;
}

std::string _method_getObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table | arg_optional, 0, arg_any}))
    {
        std::string origPath = fetchText(inStack, 0);
        std::string path(origPath);
        if ((path.size() == 0) || ((path[0] != '.') && (path[0] != '/')))
            path = "./" + path;
        int index = -1;
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            CInterfaceStackObject* obj = map->getMapObject("index");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_integer))
                index = (int)((CInterfaceStackInteger*)obj)->getValue();
            obj = map->getMapObject("noError");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
            {
                if (((CInterfaceStackBool*)obj)->getValue())
                    noError = true;
            }
        }
        CSceneObject* it = nullptr;
        CSceneObject* prox = nullptr;
        if (targetObj >= 0)
        {
            prox = App::currentWorld->sceneObjects->getObjectFromHandle(targetObj);
            if ((prox == nullptr) && (currentScript->getScriptHandle() <= SIM_IDEND_SCENEOBJECT))
                prox = App::currentWorld->sceneObjects->getScriptFromHandle(currentScript->getScriptHandle());
//                prox = App::currentWorld->getDetachedScriptFromHandle(targetObj);
        }
        it = App::currentWorld->sceneObjects->getObjectFromPath(prox, path.c_str(), index);

        if (it != nullptr)
            pushHandle(outStack, it->getObjectHandle());
        else
        {
            // Check if we maybe have an object with such a persistent UID:
            if (index == -1)
            {
                it = App::currentWorld->sceneObjects->getObjectFromPersistentUid(origPath.c_str());
                if (it != nullptr)
                {
                    if (prox != nullptr)
                    { // we search from a proxy
                        if (!it->hasAncestor(prox))
                            it = nullptr;
                    }
                    if (it != nullptr)
                        pushHandle(outStack, it->getObjectHandle());
                }
            }
        }
        if (it == nullptr)
        {
            if (noError)
                pushHandle(outStack, -1);
            else
            {
                errMsg = "object was not found, or name/path ('";
                errMsg += origPath;
                errMsg += "') is ill formatted.";
            }
        }
    }
    return errMsg;
}

std::string _method_announceChange(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string | arg_optional}))
    {
        std::string changeName = fetchText(inStack, 0);
        App::currentWorld->undoBufferContainer->announceChange();
    }
    return errMsg;
}

std::string _method_getObjectFromUid(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_integer, arg_table | arg_optional, 0, arg_any}))
    {
        long long int uid = fetchLong(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            CInterfaceStackObject* obj = map->getMapObject("noError");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                noError = ((CInterfaceStackBool*)obj)->getValue();
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromUid(uid);
        if (it != nullptr)
            pushHandle(outStack, it->getObjectHandle());
        else
        {
            if (noError)
                pushHandle(outStack, -1);
            else
                errMsg = SIM_ERROR_OBJECT_INEXISTANT;
        }
    }
    return errMsg;
}

std::string _method_getInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        C3X3Matrix m(shape->getMesh()->getInertia());
        m *= shape->getMesh()->getMass();
        pushMatrix(outStack, m);
        pushVector3(outStack, shape->getMesh()->getCOM());
    }
    return errMsg;
}

std::string _method_setInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_matrix, 3, 3, arg_pose}))
    {
        CMatrix _m = fetchMatrix(inStack, 0);
        C3X3Matrix m;
        m.setData(_m.data.data());
        m.axis[0](1) = m.axis[1](0);
        m.axis[0](2) = m.axis[2](0);
        m.axis[1](2) = m.axis[2](1);
        m /= shape->getMesh()->getMass(); // in CoppeliaSim we work with the "massless inertia"
        C7Vector tr = fetchPose(inStack, 1);

        shape->getMesh()->setCOM(tr.X);
        m = CMeshWrapper::getInertiaInNewFrame(tr.Q, m, C4Vector::identityRotation);
        shape->getMesh()->setInertia(m);
        shape->setDynamicsResetFlag(true, false);
    }
    return errMsg;
}

std::string _method_computeInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_double}))
    {
        double density = fetchDouble(inStack, 0);
        shape->computeMassAndInertia(density);
    }
    return errMsg;
}

std::string _method_addForce(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_vector3, arg_vector3 | arg_optional, arg_table | arg_optional, 0, arg_any}))
    {
        C3Vector force = fetchVector3(inStack, 0);
        C3Vector pos;
        pos.clear();
        if (hasNonNullArg(inStack, 1))
            pos = fetchVector3(inStack, 1);
        bool reset = false;
        bool relative = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            CInterfaceStackObject* obj = map->getMapObject("reset");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                reset = ((CInterfaceStackBool*)obj)->getValue();
            obj = map->getMapObject("relative");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                relative = ((CInterfaceStackBool*)obj)->getValue();
        }
        C3Vector t(pos ^ force);
        // force & t are relative to the shape's frame now
        if (relative)
        {
            C4Vector q(shape->getCumulativeTransformation().Q);
            force = q * force;
            t = q * t;
        }
        if (reset)
            shape->clearAdditionalForce();
        shape->addAdditionalForceAndTorque(force, t);
    }
    return errMsg;
}

std::string _method_addTorque(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {arg_vector3, arg_table | arg_optional, 0, arg_any}))
    {
        C3Vector torque = fetchVector3(inStack, 0);
        bool reset = false;
        bool relative = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            CInterfaceStackObject* obj = map->getMapObject("reset");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                reset = ((CInterfaceStackBool*)obj)->getValue();
            obj = map->getMapObject("relative");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                relative = ((CInterfaceStackBool*)obj)->getValue();
        }
        C3Vector force;
        force.clear();
        if (relative)
            torque = shape->getCumulativeTransformation().Q * torque;
        if (reset)
            shape->clearAdditionalTorque();
        shape->addAdditionalForceAndTorque(force, torque);
    }
    return errMsg;
}

std::string _method_ungroup(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        std::vector<int> sel;
        sel.push_back(shape->getObjectHandle());
        if (!shape->getMesh()->isMesh())
            CSceneObjectOperations::ungroupSelection(&sel, true);
        if (sel.size() <= 1)
            sel.clear();
        pushShortHandleArray(outStack, sel.data(), sel.size());
    }
    return errMsg;
}

std::string _method_divide(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(method, inStack, &errMsg, {}))
    {
        std::vector<int> sel;
        sel.push_back(shape->getObjectHandle());
        if (shape->getMesh()->isMesh())
        {
            CSceneObjectOperations::divideSelection(&sel);
            if (sel.size() <= 1)
                sel.clear();
            pushShortHandleArray(outStack, sel.data(), sel.size());
        }
        else
            errMsg = SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE;
    }
    return errMsg;
}

std::string _method_groupShapes(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_handle}))
    {
        std::vector<long long int> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        std::vector<int> shapeHandles;
        for (size_t i = 0; i < objectHandles.size(); i++)
        {
            CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objectHandles[i]);
            if (it != nullptr)
                shapeHandles.push_back(objectHandles[i]);
        }
        if ((shapeHandles.size() > 1) && (shapeHandles.size() == objectHandles.size()))
        {
            int h = CSceneObjectOperations::groupSelection(&shapeHandles);
            pushHandle(outStack, h);
        }
        else
            errMsg = "invalid objects, or not enough shapes.";
    }
    return errMsg;
}

std::string _method_mergeShapes(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_handle}))
    {
        std::vector<long long int> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        std::vector<int> shapeHandles;
        for (size_t i = 0; i < objectHandles.size(); i++)
        {
            CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objectHandles[i]);
            if (it != nullptr)
                shapeHandles.push_back(objectHandles[i]);
        }
        if ((shapeHandles.size() > 1) && (shapeHandles.size() == objectHandles.size()))
        {
            int h = CSceneObjectOperations::mergeSelection(&shapeHandles);
            pushHandle(outStack, h);
        }
        else
            errMsg = "invalid objects, or not enough shapes.";
    }
    return errMsg;
}

std::string _method_packTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // use pack instead
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_any}))
    {
        if (inStack->getStackObjectFromIndex(0)->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
            // Following is the version of the pack format. 0 was when all numbers would be packed as double
            // (Lua5.1) 1-4 are reserved in order to detect other non-CoppeliaSim formats, check sim.lua
            // for details.
            // Make sure not to use any byte value that could be a first byte in a cbor string!
            unsigned char version = 5;
            std::string auxInfos;
            std::string s = (char)version + table->getObjectData(auxInfos);
            // Following are auxiliary string infos (text/binary string/buffer) we append to the end, in order
            // to keep backward compatible. The aux infos can be any byte value, except for 255. One aux. value
            // per string object:
            s += auxInfos + (char)255;
            pushBuffer(outStack, s.c_str(), s.size());
        }
        else
            pushBuffer(outStack, "", 0);
    }
    return errMsg;
}

std::string _method_unpackTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // use unpack instead
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string data = fetchBuffer(inStack, 0);
        if (data.size() > 0)
        {
            if ((data[0] == 0) || (data[0] == 5))
            {
                unsigned char version = data[0]; // the version of the pack format
                unsigned int w = 0;
                if (CInterfaceStackTable::checkCreateFromData(data.data() + 1, w, data.size() - 1, version))
                {
                    std::vector<CInterfaceStackObject*> allCreatedObjects;
                    CInterfaceStackTable* table = new CInterfaceStackTable();
                    int mainDataSize = 1 + table->createFromData(data.data() + 1, version, allCreatedObjects);
                    if (mainDataSize < int(data.size()))
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
                    pushObject(outStack, table);
                }
                else
                    errMsg = SIM_ERROR_INVALID_DATA;
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
        else
            pushIntArray(outStack, nullptr, 0); // empty buffer results in an empty table
    }
    return errMsg;
}

std::string _method_pack(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_any}))
    {
        CInterfaceStackTable* table = new CInterfaceStackTable();
        table->appendArrayObject(inStack->getStackObjectFromIndex(0)->copyYourself());
        // Following is the version of the pack format. 0 was when all numbers would be packed as double
        // (Lua5.1) 1-4 are reserved in order to detect other non-CoppeliaSim formats, check sim.lua
        // for details.
        unsigned char version = 5;
        std::string auxInfos;
        std::string s = (char)version + table->getObjectData(auxInfos);
        delete table;
        // Following are auxiliary string infos (text/binary string/buffer) we append to the end, in order
        // to keep backward compatible. The aux infos can be any byte value, except for 255. One aux. value
        // per string object:
        s += auxInfos + (char)255;
        pushBuffer(outStack, s.c_str(), s.size());
    }
    return errMsg;
}

std::string _method_unpack(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string}))
    {
        std::string data = fetchBuffer(inStack, 0);
        if (data.size() > 0)
        {
            if ((data[0] == 0) || (data[0] == 5))
            {
                unsigned char version = data[0]; // the version of the pack format
                unsigned int w = 0;
                if (CInterfaceStackTable::checkCreateFromData(data.data() + 1, w, data.size() - 1, version))
                {
                    std::vector<CInterfaceStackObject*> allCreatedObjects;
                    CInterfaceStackTable* table = new CInterfaceStackTable();
                    int mainDataSize = 1 + table->createFromData(data.data() + 1, version, allCreatedObjects);
                    if (mainDataSize < int(data.size()))
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
                    pushObject(outStack, table->getArrayItemAtIndex(0));
                }
                else
                    errMsg = SIM_ERROR_INVALID_DATA;
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
        else
            pushIntArray(outStack, nullptr, 0); // empty buffer results in an empty table
    }
    return errMsg;
}

std::string _method_packDoubleTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<double> arr;
        fetchDoubleArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(double) * count);
            for (int i = 0; i < count; i++)
            {
                double v = arr[startIndex + i];
                data[sizeof(double) * i + 0] = ((char*)&v)[0];
                data[sizeof(double) * i + 1] = ((char*)&v)[1];
                data[sizeof(double) * i + 2] = ((char*)&v)[2];
                data[sizeof(double) * i + 3] = ((char*)&v)[3];
                data[sizeof(double) * i + 4] = ((char*)&v)[4];
                data[sizeof(double) * i + 5] = ((char*)&v)[5];
                data[sizeof(double) * i + 6] = ((char*)&v)[6];
                data[sizeof(double) * i + 7] = ((char*)&v)[7];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packFloatTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<double> arr;
        fetchDoubleArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(float) * count);
            for (int i = 0; i < count; i++)
            {
                float v;
                double x = arr[startIndex + i];
                if (x > double(std::numeric_limits<float>::max()))
                    v = std::numeric_limits<float>::max();
                else if (x < -double(std::numeric_limits<float>::max()))
                    v = -std::numeric_limits<float>::max();
                else
                    v = float(x);
                data[sizeof(float) * i + 0] = ((char*)&v)[0];
                data[sizeof(float) * i + 1] = ((char*)&v)[1];
                data[sizeof(float) * i + 2] = ((char*)&v)[2];
                data[sizeof(float) * i + 3] = ((char*)&v)[3];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packInt64Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(int64_t) * count);
            for (int i = 0; i < count; i++)
            {
                int64_t v = arr[startIndex + i];
                data[sizeof(int64_t) * i + 0] = ((char*)&v)[0];
                data[sizeof(int64_t) * i + 1] = ((char*)&v)[1];
                data[sizeof(int64_t) * i + 2] = ((char*)&v)[2];
                data[sizeof(int64_t) * i + 3] = ((char*)&v)[3];
                data[sizeof(int64_t) * i + 4] = ((char*)&v)[4];
                data[sizeof(int64_t) * i + 5] = ((char*)&v)[5];
                data[sizeof(int64_t) * i + 6] = ((char*)&v)[6];
                data[sizeof(int64_t) * i + 7] = ((char*)&v)[7];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(int32_t) * count);
            for (int i = 0; i < count; i++)
            {
                int32_t v;
                int64_t x = arr[startIndex + i];
                if (x > std::numeric_limits<int32_t>::max())
                    v = std::numeric_limits<int32_t>::max();
                else if (x < std::numeric_limits<int32_t>::min())
                    v = std::numeric_limits<int32_t>::min();
                else
                    v = int32_t(x);
                data[sizeof(int32_t) * i + 0] = ((char*)&v)[0];
                data[sizeof(int32_t) * i + 1] = ((char*)&v)[1];
                data[sizeof(int32_t) * i + 2] = ((char*)&v)[2];
                data[sizeof(int32_t) * i + 3] = ((char*)&v)[3];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packUInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(uint32_t) * count);
            for (int i = 0; i < count; i++)
            {
                uint32_t v;
                int64_t x = arr[startIndex + i];
                if (x > int64_t(std::numeric_limits<uint32_t>::max()))
                    v = std::numeric_limits<uint32_t>::max();
                else if (x < 0)
                    v = 0;
                else
                    v = uint32_t(x);
                data[sizeof(uint32_t) * i + 0] = ((char*)&v)[0];
                data[sizeof(uint32_t) * i + 1] = ((char*)&v)[1];
                data[sizeof(uint32_t) * i + 2] = ((char*)&v)[2];
                data[sizeof(uint32_t) * i + 3] = ((char*)&v)[3];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(int16_t) * count);
            for (int i = 0; i < count; i++)
            {
                int16_t v;
                int64_t x = arr[startIndex + i];
                if (x > int64_t(std::numeric_limits<int16_t>::max()))
                    v = std::numeric_limits<int16_t>::max();
                else if (x < std::numeric_limits<int16_t>::min())
                    v = std::numeric_limits<int16_t>::min();
                else
                    v = int16_t(x);
                data[sizeof(int16_t) * i + 0] = ((char*)&v)[0];
                data[sizeof(int16_t) * i + 1] = ((char*)&v)[1];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packUInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(uint16_t) * count);
            for (int i = 0; i < count; i++)
            {
                uint16_t v;
                int64_t x = arr[startIndex + i];
                if (x > int64_t(std::numeric_limits<uint16_t>::max()))
                    v = std::numeric_limits<uint16_t>::max();
                else if (x < 0)
                    v = 0;
                else
                    v = uint16_t(x);
                data[sizeof(uint16_t) * i + 0] = ((char*)&v)[0];
                data[sizeof(uint16_t) * i + 1] = ((char*)&v)[1];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(int8_t) * count);
            for (int i = 0; i < count; i++)
            {
                int8_t v;
                int64_t x = arr[startIndex + i];
                if (x > int64_t(std::numeric_limits<int8_t>::max()))
                    v = std::numeric_limits<int8_t>::max();
                else if (x < std::numeric_limits<int8_t>::min())
                    v = std::numeric_limits<int8_t>::min();
                else
                    v = int8_t(x);
                data[i] = ((char*)&v)[0];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_packUInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, -1, arg_any, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::vector<int64_t> arr;
        fetchLongArray(inStack, 0, arr);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int tableSize = int(arr.size());

        if (count < 0)
            count = 0;
        if ((startIndex < 0) || (startIndex >= tableSize))
            count = 0;
        else
        {
            if (count == 0)
                count = tableSize - startIndex;
            if (count > tableSize - startIndex)
                count = tableSize - startIndex;
        }
        std::string data;
        if (count > 0)
        {
            data.resize(sizeof(uint8_t) * count);
            for (int i = 0; i < count; i++)
            {
                uint8_t v;
                int64_t x = arr[startIndex + i];
                if (x > int64_t(std::numeric_limits<uint8_t>::max()))
                    v = std::numeric_limits<uint8_t>::max();
                else if (x < 0)
                    v = 0;
                else
                    v = uint8_t(x);
                data[i] = ((char*)&v)[0];
            }
        }
        pushBuffer(outStack, data.data(), data.size());
    }
    return errMsg;
}

std::string _method_unpackDoubleTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(double) * (remainingLength / sizeof(double));
        int packetCount = int(dataLength / sizeof(double));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<double> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    double v;
                    ((char*)&v)[0] = data[sizeof(double) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(double) * (i + startIndex) + 1];
                    ((char*)&v)[2] = data[sizeof(double) * (i + startIndex) + 2];
                    ((char*)&v)[3] = data[sizeof(double) * (i + startIndex) + 3];
                    ((char*)&v)[4] = data[sizeof(double) * (i + startIndex) + 4];
                    ((char*)&v)[5] = data[sizeof(double) * (i + startIndex) + 5];
                    ((char*)&v)[6] = data[sizeof(double) * (i + startIndex) + 6];
                    ((char*)&v)[7] = data[sizeof(double) * (i + startIndex) + 7];
                    outData.push_back(v);
                }
            }
        }
        pushDoubleArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackFloatTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(float) * (remainingLength / sizeof(float));
        int packetCount = int(dataLength / sizeof(float));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<float> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    float v;
                    ((char*)&v)[0] = data[sizeof(float) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(float) * (i + startIndex) + 1];
                    ((char*)&v)[2] = data[sizeof(float) * (i + startIndex) + 2];
                    ((char*)&v)[3] = data[sizeof(float) * (i + startIndex) + 3];
                    outData.push_back(v);
                }
            }
        }
        pushFloatArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackInt64Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(int64_t) * (remainingLength / sizeof(int64_t));
        int packetCount = int(dataLength / sizeof(int64_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int64_t> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    int64_t v;
                    ((char*)&v)[0] = data[sizeof(int64_t) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(int64_t) * (i + startIndex) + 1];
                    ((char*)&v)[2] = data[sizeof(int64_t) * (i + startIndex) + 2];
                    ((char*)&v)[3] = data[sizeof(int64_t) * (i + startIndex) + 3];
                    ((char*)&v)[4] = data[sizeof(int64_t) * (i + startIndex) + 4];
                    ((char*)&v)[5] = data[sizeof(int64_t) * (i + startIndex) + 5];
                    ((char*)&v)[6] = data[sizeof(int64_t) * (i + startIndex) + 6];
                    ((char*)&v)[7] = data[sizeof(int64_t) * (i + startIndex) + 7];
                    outData.push_back(v);
                }
            }
        }
        pushLongArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(int32_t) * (remainingLength / sizeof(int32_t));
        int packetCount = int(dataLength / sizeof(int32_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    int32_t v;
                    ((char*)&v)[0] = data[sizeof(int32_t) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(int32_t) * (i + startIndex) + 1];
                    ((char*)&v)[2] = data[sizeof(int32_t) * (i + startIndex) + 2];
                    ((char*)&v)[3] = data[sizeof(int32_t) * (i + startIndex) + 3];
                    outData.push_back(v);
                }
            }
        }
        pushIntArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackUInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(uint32_t) * (remainingLength / sizeof(uint32_t));
        int packetCount = int(dataLength / sizeof(uint32_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int64_t> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    uint32_t v;
                    ((char*)&v)[0] = data[sizeof(uint32_t) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(uint32_t) * (i + startIndex) + 1];
                    ((char*)&v)[2] = data[sizeof(uint32_t) * (i + startIndex) + 2];
                    ((char*)&v)[3] = data[sizeof(uint32_t) * (i + startIndex) + 3];
                    outData.push_back(v);
                }
            }
        }
        pushLongArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(int16_t) * (remainingLength / sizeof(int16_t));
        int packetCount = int(dataLength / sizeof(int16_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    int16_t v;
                    ((char*)&v)[0] = data[sizeof(int16_t) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(int16_t) * (i + startIndex) + 1];
                    outData.push_back(v);
                }
            }
        }
        pushIntArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackUInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(uint16_t) * (remainingLength / sizeof(uint16_t));
        int packetCount = int(dataLength / sizeof(uint16_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    uint16_t v;
                    ((char*)&v)[0] = data[sizeof(uint16_t) * (i + startIndex) + 0];
                    ((char*)&v)[1] = data[sizeof(uint16_t) * (i + startIndex) + 1];
                    outData.push_back(v);
                }
            }
        }
        pushIntArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(int8_t) * (remainingLength / sizeof(int8_t));
        int packetCount = int(dataLength / sizeof(int8_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    int8_t v;
                    ((char*)&v)[0] = data[i + startIndex];
                    outData.push_back(v);
                }
            }
        }
        pushIntArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method_unpackUInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer | arg_optional, arg_integer | arg_optional, arg_integer | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        int startIndex = fetchInt(inStack, 1, 0);
        int count = fetchInt(inStack, 2, 0);
        int additionalCharOffset = fetchInt(inStack, 3, 0);

        if (additionalCharOffset < 0)
            additionalCharOffset = 0;
        if (additionalCharOffset > int(dat.size()))
            additionalCharOffset = int(dat.size());

        size_t offset = size_t(additionalCharOffset);
        size_t remainingLength = dat.size() - offset;
        const char* data = dat.data() + offset;
        size_t dataLength = sizeof(uint8_t) * (remainingLength / sizeof(uint8_t));
        int packetCount = int(dataLength / sizeof(uint8_t));

        if (count < 0)
            count = 0;
        if (count == 0)
            count = int(1999999999);

        std::vector<int> outData;
        if (dataLength != 0)
        {
            if ((startIndex >= 0) && (startIndex < packetCount))
            {
                if (count > packetCount - startIndex)
                    count = packetCount - startIndex;
                outData.reserve(count);
                for (int i = 0; i < count; i++)
                {
                    uint8_t v;
                    ((char*)&v)[0] = data[i + startIndex];
                    outData.push_back(v);
                }
            }
        }
        pushIntArray(outStack, outData.data(), outData.size());
    }
    return errMsg;
}

std::string _method__createCamera(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, 0, arg_any}))
    {
        CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
        double clipp[2] = {0.05, 30.0};
        map->fetchDoubleArrayFromKey("clippingPlanes", clipp, 2, &errMsg);
        double viewAngle = 60.0 * degToRad;
        map->fetchDoubleFromKey("viewAngle", viewAngle, &errMsg);
        double viewSize = 2.0;
        bool perspective = !map->fetchDoubleFromKey("viewSize", viewSize, &errMsg);
        if (errMsg.size() == 0)
        {
            CCamera* it = new CCamera();
            it->setViewAngle(viewAngle);
            it->setOrthoViewSize(viewSize);
            it->setPerspective(perspective);
            it->setClippingPlanes(clipp[0], clipp[1]);
            App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
            pushHandle(outStack, it->getObjectHandle());
        }
    }
    return errMsg;
}

std::string _method__createLight(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, 0, arg_any}))
    {
        CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
        int lightType = sim_light_omnidirectional;
        map->fetchInt32FromKey("lightType", lightType, &errMsg);
        if (errMsg.size() == 0)
        {
            CLight* it = new CLight(lightType);
            App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
            pushHandle(outStack, it->getObjectHandle());
        }
    }
    return errMsg;
}

std::string _method__createGraph(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_table, 0, arg_any}))
    {
        float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
        float foregroundColor[3] = {0.8f, 0.8f, 0.8f};
        CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
        map->fetchFloatArrayFromKey("backgroundColor", backgroundColor, 3, &errMsg);
        map->fetchFloatArrayFromKey("foregroundColor", foregroundColor, 3, &errMsg);
        if (errMsg.size() == 0)
        {
            CGraph* it = new CGraph();
            for (size_t i = 0; i < 3; i++)
            {
                it->backgroundColor[i] = backgroundColor[i];
                it->foregroundColor[i] = foregroundColor[i];
            }
            App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
            CScript* script = new CScript(sim_scripttype_customization, "graph = require('models.graph_customization-2')", 0, "lua");
            script->setScriptExecPriority_raw(sim_scriptexecorder_last);
            App::currentWorld->sceneObjects->addObjectToScene(script, false, true);
            App::currentWorld->sceneObjects->setObjectParent(script, it, true);
            it->setObjectProperty(it->getObjectProperty() | sim_objectproperty_collapsed);
            it->setModelBase(true);
            pushHandle(outStack, it->getObjectHandle());
        }
    }
    return errMsg;
}

std::string _method_getBoolProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int pValue;
            if (CALL_C_API(simGetBoolProperty, targetObj, pName.c_str(), &pValue) > 0)
                pushBool(outStack, pValue != 0);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getBufferProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            char* pValue;
            int pLength;
            if (CALL_C_API(simGetBufferProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                pushBuffer(outStack, pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getColorProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            float pValue[3];
            if (CALL_C_API(simGetColorProperty, targetObj, pName.c_str(), pValue) > 0)
                pushColor(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getFloatArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double* pValue;
            int pLength;
            if (CALL_C_API(simGetFloatArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                pushDoubleArray(outStack, pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getFloatProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue;
            if (CALL_C_API(simGetFloatProperty, targetObj, pName.c_str(), &pValue) > 0)
                pushDouble(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getStringArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int cnt;
            char* pValue = nullptr;
            int res = CALL_C_API(simGetStringArrayProperty, targetObj, pName.c_str(), &pValue, &cnt);
            if (res > 0)
            {
                std::vector<std::string> vv;
                const char* ptr = pValue;
                for (int i = 0; i < cnt; i++)
                {
                    size_t len = strlen(ptr);
                    vv.push_back(ptr);
                    ptr += len + 1;
                }
                delete[] pValue;
                pushTextArray(outStack, vv.data(), vv.size());
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getHandleArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            long long int* pValue = nullptr;
            int pLength;
            if (CALL_C_API(simGetHandleArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                pushHandleArray(outStack, pValue, pLength);
//                pushLongArray(outStack, pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getHandleProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            long long int pValue;
            if (CALL_C_API(simGetHandleProperty, targetObj, pName.c_str(), &pValue) > 0)
                pushHandle(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntArray2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int pValue[2];
            if (CALL_C_API(simGetIntArray2Property, targetObj, pName.c_str(), pValue) > 0)
                pushIntArray(outStack, pValue, 2);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int* pValue;
            int pLength;
            if (CALL_C_API(simGetIntArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                pushIntArray(outStack, pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int pValue;
            if (CALL_C_API(simGetIntProperty, targetObj, pName.c_str(), &pValue) > 0)
                pushInt(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getLongProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            long long int pValue;
            if (CALL_C_API(simGetLongProperty, targetObj, pName.c_str(), &pValue) > 0)
                pushLong(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getPoseProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[7];
            if (CALL_C_API(simGetPoseProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                C7Vector p;
                p.setData(pValue, true);
                pushPose(outStack, p);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getQuaternionProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[4];
            if (CALL_C_API(simGetQuaternionProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                C4Vector q;
                q.setData(pValue, true);
                pushQuaternion(outStack, q);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getStringProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            char* pValue = nullptr;
            int res = CALL_C_API(simGetStringProperty, targetObj, pName.c_str(), &pValue);
            if (res > 0)
            {
                pushText(outStack, pValue);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getVector2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[2];
            if (CALL_C_API(simGetVector2Property, targetObj, pName.c_str(), pValue) > 0)
                pushVector2(outStack, pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getVector3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[3];
            if (CALL_C_API(simGetVector3Property, targetObj, pName.c_str(), pValue) > 0)
                pushVector3(outStack, C3Vector(pValue));
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setBoolProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_bool, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool pValue = fetchBool(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetBoolProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setBufferProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchBuffer(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetBufferProperty, targetObj, pName.c_str(), pValue.c_str(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setColorProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_color, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        float pValue[3];
        fetchColor(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetColorProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setFloatArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, -1, arg_double, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<double> pValue;
        fetchDoubleArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetFloatArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setFloatProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_double, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        double pValue = fetchDouble(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetFloatProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setStringArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, -1, arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<std::string> pValue;
        fetchTextArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            std::vector<char> v;
            for (size_t i = 0; i < pValue.size(); i++)
            {
                for (size_t j = 0; j < pValue[i].size(); j++)
                    v.push_back(pValue[i][j]);
                v.push_back('\0');
            }
            if (CALL_C_API(simSetStringArrayProperty, targetObj, pName.c_str(), v.data(), int(pValue.size())) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setHandleArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_handlearray, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<long long int> pValue;
        fetchHandleArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetHandleArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setHandleProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_handle, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        long long int pValue = fetchHandle(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetHandleProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setIntArray2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, -1, arg_integer, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<int> pValue;
        fetchIntArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntArray2Property, targetObj, pName.c_str(), pValue.data()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setIntArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_table, -1, arg_integer, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<int> pValue;
        fetchIntArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setIntProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        int pValue = fetchInt(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setLongProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_integer, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        long long int pValue = fetchLong(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetLongProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setPoseProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_pose, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        C7Vector pState = fetchPose(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double p[7];
            pState.getData(p, true);
            if (CALL_C_API(simSetPoseProperty, targetObj, pName.c_str(), p) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setQuaternionProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_quaternion, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        C4Vector pState = fetchQuaternion(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double q[4];
            pState.getData(q, true);
            if (CALL_C_API(simSetQuaternionProperty, targetObj, pName.c_str(), q) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setStringProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchText(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetStringProperty, targetObj, pName.c_str(), pValue.c_str()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setVector2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_vector2, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<double> pValue;
        fetchDoubleArray(inStack, 1, pValue);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetVector2Property, targetObj, pName.c_str(), pValue.data()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_setVector3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_vector3, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        C3Vector pValue = fetchVector3(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetVector3Property, targetObj, pName.c_str(), pValue.data) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_getMatrixProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double* pValue;
            int r, c;
            if (CALL_C_API(simGetMatrixProperty, targetObj, pName.c_str(), &pValue, &r, &c) > 0)
            {
                CMatrix m(r, c);
                m.data.assign(pValue, pValue + r * c);
                pushMatrix(outStack, m);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setMatrixProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_matrix, -1, -1, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        CMatrix pValue = fetchMatrix(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetMatrixProperty, targetObj, pName.c_str(), pValue.data.data(), pValue.rows, pValue.cols) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_getMatrix3x3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[3 * 3];
            if (CALL_C_API(simGetMatrix3x3Property, targetObj, pName.c_str(), pValue) > 0)
            {
                CMatrix m(3, 3);
                m.data.assign(pValue, pValue + 3 * 3);
                pushMatrix(outStack, m);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setMatrix3x3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_matrix, 3, 3, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        CMatrix pValue = fetchMatrix(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetMatrix3x3Property, targetObj, pName.c_str(), pValue.data.data()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_getMatrix4x4Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            double pValue[4 * 4];
            if (CALL_C_API(simGetMatrix4x4Property, targetObj, pName.c_str(), pValue) > 0)
            {
                CMatrix m(4, 4);
                m.data.assign(pValue, pValue + 4 * 4);
                pushMatrix(outStack, m);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setMatrix4x4Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_matrix, 4, 4, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        CMatrix pValue = fetchMatrix(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetMatrix4x4Property, targetObj, pName.c_str(), pValue.data.data()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method__getTableProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            int pValueL;
            char* pValue = nullptr;
            int res = CALL_C_API(simGetTableProperty, targetObj, pName.c_str(), &pValue, &pValueL);
            if (res > 0)
            {
                pushBuffer(outStack, pValue, pValueL);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    pushNull(outStack);
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method__setTableProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchBuffer(inStack, 1);
        bool noError = false;
        if (hasNonNullArg(inStack, 2))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(2);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetTableProperty, targetObj, pName.c_str(), pValue.c_str(), int(pValue.size())) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIX))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalSet(nn.c_str(), targetObj);
                }
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                    errMsg.clear();
            }
        }
    }
    return errMsg;
}

std::string _method_removeProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("noError", noError, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            CALL_C_API(simRemoveProperty, targetObj, pName.c_str());
            if (utils::startsWith(pName.c_str(), SIGNALPREFIX))
            {
                std::string nn(pName);
                if (targetObj == sim_handle_app)
                    nn = "app." + nn;
                else if (targetObj != sim_handle_scene)
                    nn = "obj." + nn;
                currentScript->signalRemoved(nn.c_str());
            }
        }
    }
    return errMsg;
}

std::string _method_getPropertyName(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_integer, arg_optional | arg_table, 0, arg_any}))
    {
        int index = fetchInt(inStack, 0);
        SPropertyOptions opt;
        std::string propertyPrefix;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchStringFromKey("prefix", propertyPrefix, &errMsg);
            map->fetchInt32FromKey("excludeFlags", opt.excludeFlags, &errMsg);
        }
        if (errMsg.size() == 0)
        {
            if (propertyPrefix.size() > 0)
                opt.prefix = propertyPrefix.c_str();
            char* pValue = CALL_C_API(simGetPropertyName, targetObj, index, &opt);
            if (pValue != nullptr)
            {
                std::string w1(pValue);
                delete[] pValue;
                std::string w2;
                utils::extractCommaSeparatedWord(w1, w2);
                pushText(outStack, w2.c_str());
                pushText(outStack, w1.c_str());
            }
            else
            {
                pushNull(outStack);
                pushNull(outStack);
            }
        }
    }
    return errMsg;
}

std::string _method_getPropertyInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_optional | arg_table, 0, arg_any}))
    {
        std::string pName = fetchText(inStack, 0);
        SPropertyOptions opt;
        if (hasNonNullArg(inStack, 1))
        {
            CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(1);
            map->fetchBoolFromKey("shortInfoTxt", opt.shortInfoTxt, &errMsg);
            map->fetchInt32FromKey("bitCoded", opt.bitCoded, &errMsg);
        }
        SPropertyInfo infos;
        int res = CALL_C_API(simGetPropertyInfo, targetObj, pName.c_str(), &infos, &opt);
        if (res > 0)
        {
            pushInt(outStack, infos.type);
            pushInt(outStack, infos.flags);
            if (infos.infoTxt == nullptr)
                pushText(outStack, "");
            else
            {
                pushText(outStack, infos.infoTxt);
                delete[] infos.infoTxt;
            }
        }
    }
    return errMsg;
}

std::string _method_createCustomObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_string, arg_string}))
    {
        std::string typeStr = fetchText(inStack, 0);
        std::string metaInfoStr = fetchText(inStack, 1);
        pushLong(outStack, App::createCustomObject(typeStr.c_str(), metaInfoStr.c_str()));
    }
    return errMsg;
}

std::string _method_releaseCustomObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(method, inStack, &errMsg, {arg_integer}))
        App::releaseCustomObject(fetchHandle(inStack, 0));
    return errMsg;
}
