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
#include <tt.h>

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
    constexpr int arg_vector3       = sim_stackitem_exvector3;
    constexpr int arg_map           = sim_stackitem_exmap;
    constexpr int arg_any           = sim_stackitem_exany;
    constexpr int arg_optional      = sim_stackitem_exoptional;
}

std::string callMethod(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    static std::map<std::string, std::function<std::string(int, CDetachedScript*, const CInterfaceStack*, CInterfaceStack*)>> funcTable;
    if (funcTable.size() == 0)
    {
        funcTable["getPosition"] = _method_getPosition;
        funcTable["setPosition"] = _method_setPosition;
        funcTable["getQuaternion"] = _method_getQuaternion;
        funcTable["setQuaternion"] = _method_setQuaternion;
        funcTable["getPose"] = _method_getPose;
        funcTable["setPose"] = _method_setPose;
        funcTable["setParent"] = _method_setParent;
        funcTable["handleMessagePump"] = _method_handleMessagePump;
        funcTable["handleSandboxScript"] = _method_handleSandboxScript;
        funcTable["handleAddOnScripts"] = _method_handleAddOnScripts;
        funcTable["handleCustomizationScripts"] = _method_handleCustomizationScripts;
        funcTable["handleSimulationScripts"] = _method_handleSimulationScripts;
        funcTable["loadModel"] = _method_loadModel;
        funcTable["loadModelFromBuffer"] = _method_loadModelFromBuffer;
        funcTable["loadModelInfo"] = _method_loadModelInfo;
        funcTable["loadModelInfoFromBuffer"] = _method_loadModelInfoFromBuffer;
        funcTable["saveModel"] = _method_saveModel;
        funcTable["saveModelToBuffer"] = _method_saveModelToBuffer;
        funcTable["loadScene"] = _method_loadScene;
        funcTable["loadSceneFromBuffer"] = _method_loadSceneFromBuffer;
        funcTable["save"] = _method_save;
        funcTable["saveToBuffer"] = _method_saveToBuffer;
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
        funcTable["removeItems"] = _method_removeItems;
        funcTable["callFunction"] = _method_callFunction;
        funcTable["executeString"] = _method_executeString;
//        funcTable["getApiInfo"] = _method_getApiInfo;
//        funcTable["getApiFunc"] = _method_getApiFunc;
        funcTable["getStackTraceback"] = _method_getStackTraceback;
        funcTable["init"] = _method_init;
        funcTable["scale"] = _method_scale;
        funcTable["scaleTree"] = _method_scaleTree;
        funcTable["simulation.start"] = _method_startSimulation;
        funcTable["simulation.pause"] = _method_pauseSimulation;
        funcTable["simulation.stop"] = _method_stopSimulation;
        funcTable["getName"] = _method_getName;
        funcTable["dynamicReset"] = _method_dynamicReset;
        funcTable["loadImage"] = _method_loadImage;
        funcTable["loadImageFromBuffer"] = _method_loadImageFromBuffer;
        funcTable["saveImage"] = _method_saveImage;
        funcTable["saveImageToBuffer"] = _method_saveImageToBuffer;
        funcTable["transformImage"] = _method_transformImage;
        funcTable["transformBuffer"] = _method_transformBuffer;
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
        funcTable["pack"] = _method_pack;
        funcTable["unpack"] = _method_unpack;
        funcTable["packArray"] = _method_packArray;
        funcTable["unpackArray"] = _method_unpackArray;
        funcTable["groupShapes"] = _method_groupShapes;
        funcTable["mergeShapes"] = _method_mergeShapes;
        funcTable["createCamera"] = _method_createCamera;
        funcTable["createShapeFromPath"] = _method_createShapeFromPath;
        funcTable["getClosestOnPath"] = _method_getClosestOnPath;
        funcTable["createLight"] = _method_createLight;
        funcTable["createGraph"] = _method_createGraph;
        funcTable["createCustomSceneObject"] = _method_createCustomSceneObject;
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
        funcTable["setVector3Property"] = _method_setVector3Property;
        funcTable["getMatrixProperty"] = _method_getMatrixProperty;
        funcTable["setMatrixProperty"] = _method_setMatrixProperty;
        funcTable["getMethodProperty"] = _method_getMethodProperty;
        funcTable["setMethodProperty"] = _method_setMethodProperty;
        funcTable["getTableProperty"] = _method_getTableProperty;
        funcTable["setTableProperty"] = _method_setTableProperty;
        funcTable["removeProperty"] = _method_removeProperty;
        funcTable["getPropertyName"] = _method_getPropertyName;
        funcTable["getPropertyInfo"] = _method_getPropertyInfo;
        funcTable["setPropertyInfo"] = _method_setPropertyInfo;
        funcTable["createCustomObjectClass"] = _method_createCustomObjectClass;
        funcTable["isValid"] = _method_isValid;
        funcTable["addCurve"] = _method_addCurve;
        funcTable["addSignal"] = _method_addSignal;
        funcTable["reset"] = _method_reset;
        funcTable["setSignalPoint"] = _method_setSignalPoint;
        funcTable["snapshotTrace"] = _method_snapshotTrace;
        funcTable["removeTrace"] = _method_removeTrace;
        funcTable["step"] = _method_step;
        funcTable["makeClass"] = _method_makeClass;
        funcTable["makeObject"] = _method_makeObject;
        funcTable["addFromObjects"] = _method_addFromObjects;
        funcTable["subtractFromObjects"] = _method_subtractFromObjects;
        funcTable["clear"] = _method_clear;
        funcTable["addVoxels"] = _method_addVoxels;
        funcTable["addPackedVoxels"] = _method_addPackedVoxels;
        funcTable["subtractVoxels"] = _method_subtractVoxels;
        funcTable["subtractPackedVoxels"] = _method_subtractPackedVoxels;
        funcTable["checkPoints"] = _method_checkPoints;
        funcTable["checkPackedPoints"] = _method_checkPackedPoints;
        funcTable["addPoints"] = _method_addPoints;
        funcTable["addPackedPoints"] = _method_addPackedPoints;
        funcTable["subtractPoints"] = _method_subtractPoints;
        funcTable["subtractPackedPoints"] = _method_subtractPackedPoints;
        funcTable["intersectPoints"] = _method_intersectPoints;
        funcTable["intersectPackedPoints"] = _method_intersectPackedPoints;
        funcTable["setTargetPosition"] = _method_setTargetPosition;
        funcTable["setTargetVelocity"] = _method_setTargetVelocity;
        funcTable["pushEvent"] = _method_pushEvent;
        funcTable["getContacts"] = _method_getContacts;
        funcTable["getGenesisEvents"] = _method_getGenesisEvents;
        funcTable["setEventFilters"] = _method_setEventFilters;
        funcTable["getPluginInfo"] = _method_getPluginInfo;
        funcTable["setPluginInfo"] = _method_setPluginInfo;
        funcTable["setModuleEntry"] = _method_setModuleEntry;
        funcTable["dynamics.step"] = _method_dynamicsStep;
        funcTable["broadcast"] = _method_broadcast;
        funcTable["texture.set"] = _method_textureSet;
        funcTable["texture.setData"] = _method_textureSetData;
        funcTable["texture.getData"] = _method_textureGetData;
        funcTable["getEnumInfo"] = _method_getEnumInfo;
    }

    std::string retVal("__notFound__");
    if (funcTable.find(method) != funcTable.end())
    {
        retVal = funcTable[method](targetObj, currentScript, inStack, outStack); // hard-coded method
        if (!retVal.empty())
            retVal = std::string("in method '") + method + "': " + retVal;
    }
    else
    {
        void* func;
        if (App::getMethodProperty_t(targetObj, method, func) == sim_propertyret_ok)
        { // method provided via property
            typedef char* (*MethodFunc)(int64_t, const char*, int64_t,  int64_t,  int64_t);
            MethodFunc methodFunc = reinterpret_cast<MethodFunc>(func);
            int64_t scriptHandle = -1;
            if (currentScript != nullptr)
                scriptHandle = currentScript->getObjectHandle();
            char* err = methodFunc(targetObj, method, inStack->getObjectHandle(), outStack->getObjectHandle(), scriptHandle);
            if (err == nullptr)
                retVal.clear();
            else
            {
                retVal = err;
                delete[] err;
            }
        }
    }
    return retVal;
}

bool checkInputArguments(const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs)
{ // inargs: a list of desired types. Following 3 types are special:
    // arg_table is followed by 2 auxiliary values: size (-1 for any) and type of content (arg_any for any)
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
 //                   printf("XXX %i, %i\n", t, desiredArgType);
                    if (desiredArgType == arg_any)
                        retVal = true;
                    else if (desiredArgType == arg_double)
                        retVal = (t == arg_integer);
                    else if (desiredArgType == arg_handle)
                        retVal = ((t == arg_integer) || (t == arg_null));
                    else if (desiredArgType == arg_map)
                        retVal = (t == arg_table);
                    else if (t == arg_matrix)
                    {
                        CInterfaceStackMatrix* m = (CInterfaceStackMatrix*)arg;
                        if ( (desiredArgType == arg_vector) && (m->getValue()->cols == 1) )
                            retVal = true;
                        else if ( (desiredArgType == arg_vector3) && (m->getValue()->cols == 1) && (m->getValue()->rows == 3) )
                            retVal = true;
                        else
                            retVal = false;
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
                                else if (desiredArgType == arg_matrix)
                                {
                                    retVal = tbl->isMatrixEquivalent(rows, cols);
                                    if (!retVal)
                                        retVal = tbl->isMatrixDataEquivalent(rows, cols);
                                }
                                else
                                {
                                    if (tbl->areAllValuesThis(arg_double, true))
                                    {
                                        if (desiredArgType == arg_quaternion)
                                            retVal = (tbl->getArraySize() == 4);
                                        else if (desiredArgType == arg_pose)
                                            retVal = (tbl->getArraySize() == 7);
                                        else if (desiredArgType == arg_color)
                                            retVal = ((tbl->getArraySize() == 3) || (tbl->getArraySize() == 4));
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
                            }
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
                            else if (desiredArgType == arg_handlearray)
                                msg += "a handle/object array";
                            else if (desiredArgType == arg_map)
                                msg += "a map";
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
                {
                    std::string msg("the method requires more arguments.");
                    errStr[0] = msg;
                }
                retVal = false;
                break;
            }
        }
    }
    return retVal;
}

template <typename F>
void withOptionalMap(const CInterfaceStack* inStack, int pos, std::string& errMsg, F&& worker)
{
    if (CInterfaceStackTable* map = fetchMap(inStack, pos))
    {
        std::string localErr;
        worker(map, localErr);
        if (!localErr.empty())
            errMsg = "bad argument #" + std::to_string(pos + 1) + ": " + localErr;
    }
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

int64_t fetchInt64(const CInterfaceStack* inStack, int index, int64_t defaultValue /*= -1*/)
{
    int64_t retVal = defaultValue;
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
    return int(fetchInt64(inStack, index, defaultValue));
}

int64_t fetchHandle(const CInterfaceStack* inStack, int index, int64_t defaultValue /*= -1*/)
{
    int64_t retVal = defaultValue;
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

void fetchInt64Array(const CInterfaceStack* inStack, int index, std::vector<int64_t>& outArr)
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
        else if (obj->getObjectType() == sim_stackitem_handlearray)
        {
            const CInterfaceStackHandleArray* arr = (CInterfaceStackHandleArray*)obj;
            size_t cnt;
            const int64_t* v = arr->getValue(&cnt);
            outArr.assign(v, v + cnt);
        }
    }
}

void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<int64_t>& outArr, std::initializer_list<int64_t> arr /*= {}*/)
{
    std::vector<int64_t> def;
    if (arr.size() != 0)
        for (int64_t x : arr) def.push_back(x);
    fetchHandleArray(inStack, index, outArr, def);
}

void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<int64_t>& outArr, std::vector<int64_t>& arr)
{
    outArr.clear();
    if (arr.size() != 0)
        for (int64_t x : arr) outArr.push_back(x);
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
            const int64_t* v = arr->getValue(&cnt);
            outArr.assign(v, v + cnt);
        }
    }
}

void fetchColor(const CInterfaceStack* inStack, int index, float outArr[4], std::initializer_list<float> arr /*= {}*/)
{
    float def[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    int i = 0;
    for (float x : arr)
    {
        if (i < 4)
            def[i++] = x;
    }
    fetchColor(inStack, index, outArr, def);
}

void fetchColor(const CInterfaceStack* inStack, int index, float outArr[4], const float defaultArr[4])
{
    if (defaultArr)
    {
        outArr[0] = defaultArr[0];
        outArr[1] = defaultArr[1];
        outArr[2] = defaultArr[2];
        outArr[3] = defaultArr[3];
    }
    else
    {
        outArr[0] = 0.0f;
        outArr[1] = 0.0f;
        outArr[2] = 0.0f;
        outArr[3] = 1.0f;
    }
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getFloatArray(outArr, std::min<int>(4, int(tbl->getArraySize())));
        }
        else if (obj->getObjectType() == sim_stackitem_color)
        {
            const CInterfaceStackColor* col = (CInterfaceStackColor*)obj;
            const float* f = col->getValue();
            for (size_t i = 0; i < 4; i++)
                outArr[i] = f[i];
        }
    }
}

CQuaternion fetchQuaternion(const CInterfaceStack* inStack, int index, std::initializer_list<double> wxyz /*= {}*/)
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

CQuaternion fetchQuaternion(const CInterfaceStack* inStack, int index, const double wxyz[4])
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
            return CQuaternion(q->getValue()[0]);
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getDoubleArray(d, 4);
            return CQuaternion(d, true);
        }
    }
    return CQuaternion(d, false);
}

CPose fetchPose(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyzqwqxqyqz /*= {}*/)
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

CPose fetchPose(const CInterfaceStack* inStack, int index, const double xyzqwqxqyqz[7])
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
            return CPose(p->getValue()[0]);
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            const CInterfaceStackTable* tbl = (CInterfaceStackTable*)obj;
            tbl->getDoubleArray(d, 7);
            return CPose(CQuaternion(d + 3, true), C3Vector(d));
        }
    }
    return CPose(CQuaternion(d + 3, false), C3Vector(d));
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

void fetchMatrixData(const CInterfaceStack* inStack, int index, std::vector<double>& data, bool rowByRow)
{
    data.clear();
    int argCnt = inStack->getStackSize();
    if (argCnt > index)
    {
        const CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_matrix)
        {
            if (rowByRow)
                data = ((CInterfaceStackMatrix*)obj)->getValue()->data;
            else
            {
                CMatrix m = ((CInterfaceStackMatrix*)obj)->getValue()[0];
                m.transpose();
                data = m.data;
            }
        }
        else if (obj->getObjectType() == sim_stackitem_table)
        {
            CInterfaceStackTable* table = (CInterfaceStackTable*)obj;
            table->getItemsAsConsecutiveDoubles(data);
        }
    }
}

void fetchMatrixData(const CInterfaceStack* inStack, int index, std::vector<float>& data, bool rowByRow)
{
    data.clear();
    std::vector<double> d;
    fetchMatrixData(inStack, index, d, rowByRow);
    data.resize(d.size());
    for (size_t i = 0; i < d.size(); i++)
        data[i] = (float)d[i];
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

CInterfaceStackTable* fetchMap(const CInterfaceStack* inStack, int index)
{
    CInterfaceStackTable* retVal = nullptr;
    if (inStack->getStackSize() > index)
    {
        CInterfaceStackObject* obj = inStack->getStackObjectFromIndex(index);
        if (obj->getObjectType() == sim_stackitem_table)
            retVal = (CInterfaceStackTable*)obj;
    }
    return retVal;
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

CSceneObject* getSceneObject(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CSceneObject* retVal = App::scene->sceneObjects->getObjectFromHandle(identifier);
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
    CCollection* retVal = App::scene->collections->getObjectFromHandle(identifier);
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
    CDrawingObject* retVal = App::scene->drawingCont->getObjectFromHandle(identifier);
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
    if (identifier > sim_object_sceneobjectend)
        retVal = App::scenes->getDetachedScriptFromHandle(identifier);
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

CMesh* getMesh(int identifier, std::string* errMsg /*= nullptr*/, size_t argPos /*= -1*/)
{
    CMesh* retVal = App::scene->sceneObjects->getMeshFromUid(identifier);
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
    if ((App::scene->sceneObjects->getObjectFromHandle(identifier) != nullptr) || (App::scene->collections->getObjectFromHandle(identifier) != nullptr))
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

std::string _method_getPosition(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
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
                CSceneObject* it2 = getSceneObject(relativeToObjectHandle, &errMsg, 0);
                if (it2 == nullptr)
                    return errMsg;
            }
            CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
            CPose tr;
            if (relObj == nullptr)
                tr = target->getCumulativeTransformation();
            else
            {
                if (relToJointBase)
                {
                    CPose relTr(relObj->getCumulativeTransformation());
                    tr = relTr.getInverse() * target->getCumulativeTransformation();
                }
                else
                {
                    if (target->getParent() == relObj)
                        tr = target->getLocalTransformation(); // in case of a series of get/set, not losing precision
                    else
                    {
                        CPose relTr(relObj->getFullCumulativeTransformation());
                        tr = relTr.getInverse() * target->getCumulativeTransformation();
                    }
                }
            }
            outStack->pushVector3OntoStack(tr.X);
        }
    }
    return errMsg;
}

std::string _method_setPosition(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_vector3, arg_map | arg_optional}))
    {
        C3Vector position = fetchVector3(inStack, 0);
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
        {
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
                    App::scene->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), position);
                else
                {
                    if (relToJointBase)
                    {
                        CPose absTr(target->getCumulativeTransformation());
                        CPose relTr(relObj->getCumulativeTransformation());
                        CPose x(relTr.getInverse() * absTr);
                        x.X = position;
                        absTr = relTr * x;
                        App::scene->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), absTr.X);
                    }
                    else
                    {
                        if (target->getParent() == relObj)
                        { // special here, in order to not lose precision in a series of get/set
                            CPose tr(target->getLocalTransformation());
                            tr.X = position;
                            target->setLocalTransformation(tr);
                        }
                        else
                        {
                            CPose absTr(target->getCumulativeTransformation());
                            CPose relTr(relObj->getFullCumulativeTransformation());
                            CPose x(relTr.getInverse() * absTr);
                            x.X = position;
                            absTr = relTr * x;
                            App::scene->sceneObjects->setObjectAbsolutePosition(target->getObjectHandle(), absTr.X);
                        }
                    }
                }
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
    }
    return errMsg;
}

std::string _method_getQuaternion(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
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
                if (getSceneObject(relativeToObjectHandle, &errMsg, 0) == nullptr)
                    return errMsg;
            }
            CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
            CPose tr;
            if (relObj == nullptr)
                tr = target->getCumulativeTransformation();
            else
            {
                if (relToJointBase)
                {
                    CPose relTr(relObj->getCumulativeTransformation());
                    tr = relTr.getInverse() * target->getCumulativeTransformation();
                }
                else
                {
                    if (target->getParent() == relObj)
                        tr = target->getLocalTransformation(); // in case of a series get/set, not to lose precision
                    else
                    {
                        CPose relTr(relObj->getFullCumulativeTransformation());
                        tr = relTr.getInverse() * target->getCumulativeTransformation();
                    }
                }
            }
            if (inverse)
                tr.Q.inverse();
            outStack->pushQuaternionOntoStack(tr.Q);
        }
    }
    return errMsg;
}

std::string _method_setQuaternion(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_quaternion, arg_map | arg_optional}))
    {
        CQuaternion quaternion = fetchQuaternion(inStack, 0);
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
        {
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
                    App::scene->sceneObjects->setObjectAbsoluteOrientation(target->getObjectHandle(), quaternion.getEulerAngles());
                }
                else
                {
                    if ((target->getParent() == relObj) && (!relToJointBase))
                    { // special here, in order to not lose precision in a series of get/set
                        CPose tr(target->getLocalTransformation());
                        tr.Q = quaternion;
                        tr.Q.normalize();
                        if (inverse)
                            tr.Q.inverse();
                        target->setLocalTransformation(tr);
                    }
                    else
                    {
                        CPose absTr(target->getCumulativeTransformation());
                        CPose relTr;
                        if (relToJointBase)
                            relTr = relObj->getCumulativeTransformation();
                        else
                            relTr = relObj->getFullCumulativeTransformation();
                        CPose x(relTr.getInverse() * absTr);
                        x.Q = quaternion;
                        x.Q.normalize();
                        if (inverse)
                            x.Q.inverse();
                        absTr = relTr * x;
                        App::scene->sceneObjects->setObjectAbsoluteOrientation(target->getObjectHandle(), absTr.Q.getEulerAngles());
                    }
                }
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
    }
    return errMsg;
}

std::string _method_getPose(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
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
                if (getSceneObject(relativeToObjectHandle, &errMsg, 0) == nullptr)
                    return errMsg;
            }
            CSceneObject* relObj = getSceneObject(relativeToObjectHandle);
            CPose tr;
            if (relObj == nullptr)
                tr = target->getCumulativeTransformation();
            else
            {
                CPose relTr;
                if (relToJointBase)
                    relTr = relObj->getCumulativeTransformation();
                else
                    relTr = relObj->getFullCumulativeTransformation();
                tr = relTr.getInverse() * target->getCumulativeTransformation();
            }
            if (inverse)
                tr.inverse();
            outStack->pushPoseOntoStack(tr);
        }
    }
    return errMsg;
}

std::string _method_setPose(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_pose, arg_map | arg_optional}))
    {
        CPose tr = fetchPose(inStack, 0);
        int64_t relativeToObjectHandle = sim_handle_world;
        bool relToJointBase = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt64FromKey("relativeToObject", relativeToObjectHandle, &err);
            map->fetchBoolFromKey("relativeToJointBase", relToJointBase, &err);
        });
        if (errMsg.empty())
        {
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
                    App::scene->sceneObjects->setObjectAbsolutePose(target->getObjectHandle(), tr, false);
                else
                {
                    CPose relTr;
                    if (relToJointBase)
                        relTr = objRel->getCumulativeTransformation();
                    else
                        relTr = objRel->getFullCumulativeTransformation();
                    App::scene->sceneObjects->setObjectAbsolutePose(target->getObjectHandle(), relTr * tr, false);
                }
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
    }
    return errMsg;
}

std::string _method_setParent(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_handle | arg_optional, arg_map | arg_optional}))
    {
        int parentObjectHandle = fetchHandle(inStack, 0, -1);
        std::string parentingMode = "keepAbsolutePose";
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("mode", parentingMode, &err);
        });
        if (errMsg.empty())
        {
            int pMode = -1;
            if (parentingMode == "keepAbsolutePose")
                pMode = sim_parentingmode_keepworldpose;
            if (parentingMode == "keepRelativePose")
                pMode = sim_parentingmode_keeplocalpose;
            if (parentingMode == "assembly")
                pMode = sim_parentingmode_assembly;
            if (pMode != -1)
            {
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
                if (pMode == sim_parentingmode_keepworldpose)
                    App::scene->sceneObjects->setObjectParent(target, parentIt, true);
                else
                {
                    if (pMode == sim_parentingmode_assembly)
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
                        App::scene->sceneObjects->setObjectParent(target, parentIt, false);
                }
            }
            else
                errMsg = "invalid mode.";
        }
    }
    return errMsg;
}

std::string _method_handleMessagePump(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
        App::simThread->handleExtCalls();
    return errMsg;
}

std::string _method_handleSandboxScript(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
            editMode = GuiApp::getEditModeType();
#endif
            if ((editMode == NO_EDIT_MODE) && (App::scenes->sandboxScript != nullptr))
                App::scenes->sandboxScript->systemCallScript(callType, nullptr, nullptr);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleAddOnScripts(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer}))
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
                calledCnt = App::scenes->addOnScriptContainer->callScripts(callType, nullptr, nullptr);
            outStack->pushInt32OntoStack(calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleSimulationScripts(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int calledCnt = 0;
            CInterfaceStack* stack = nullptr;
            if (inStack->getStackSize() > 1)
            {
                App::scenes->interfaceStackContainer->createStack();
                stack->copyFrom(inStack);
                CInterfaceStackObject* obj = stack->detachStackObjectFromIndex(0);
                delete obj;
            }
            calledCnt = App::scene->sceneObjects->callScripts_noMainScript(sim_scripttype_simulation, callType, stack, nullptr);
            if (stack != nullptr)
                App::scenes->interfaceStackContainer->destroyStack(stack);
            outStack->pushInt32OntoStack(calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_handleCustomizationScripts(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((currentScript != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer}))
    {
        if (currentScript->getScriptType() == sim_scripttype_main)
        {
            int callType = fetchInt(inStack, 0);
            int calledCnt = 0;
            CInterfaceStack* stack = nullptr;
            if (inStack->getStackSize() > 1)
            {
                App::scenes->interfaceStackContainer->createStack();
                stack->copyFrom(inStack);
                CInterfaceStackObject* obj = stack->detachStackObjectFromIndex(0);
                delete obj;
            }
            calledCnt = App::scene->sceneObjects->callScripts_noMainScript(sim_scripttype_customization, callType, stack, nullptr);
            if (stack != nullptr)
                App::scenes->interfaceStackContainer->destroyStack(stack);
            outStack->pushInt32OntoStack(calledCnt);
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_loadModel(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_scene)
    {
        if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
        {
            std::string path = fetchText(inStack, 0);
            C3Vector offset;
            int assemblyDummy = -1;
            offset.clear();
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                std::vector<double> arr;
                if (map->fetchMatrixDataFromKey("offset", arr, 3, 1, true, &err))
                    offset.setData(arr.data());
                if (map->fetchInt32FromKey("assemblyDummy", assemblyDummy, &err))
                {
                    CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(assemblyDummy);
                    if (obj == nullptr)
                        errMsg = "invalid assembly dummy";
                }
            });
            if (errMsg.empty())
            {
                std::string infoStr;
                std::vector<int> sel = App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0];
                if (CFileOperations::loadModel(path.c_str(), false, false, nullptr, false, false, &infoStr, &errMsg))
                {
                    int handle = App::scene->sceneObjects->getLastSelectionHandle();
                    outStack->pushHandleOntoStack(handle);
                    outStack->pushTextOntoStack(infoStr.c_str());
                    CSceneObject* it = App::scene->sceneObjects->getObjectFromHandle(handle);
                    if (assemblyDummy == -1)
                    {
                        CPose tr = it->getLocalTransformation();
                        tr.X += offset;
                        it->setLocalTransformation(tr);
                    }
                    else
                    {
                        if (!App::assemble(assemblyDummy, handle, false))
                            errMsg = "failed assembling model onto dummy";
                    }
                    App::scene->sceneObjects->setSelectedObjectHandles(sel.data(), sel.size());
#ifdef SIM_WITH_GUI
                    GuiApp::setRebuildHierarchyFlag();
#endif
                }
            }
        }
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_loadModelFromBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_scene)
    {
        if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
        {
            std::string data = fetchBuffer(inStack, 0);
            std::vector<char> buffer(data.data(), data.data() + data.size());
            C3Vector offset;
            int assemblyDummy = -1;
            offset.clear();
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                std::vector<double> arr;
                if (map->fetchMatrixDataFromKey("offset", arr, 3, 1, true, &err))
                    offset.setData(arr.data());
                if (map->fetchInt32FromKey("assemblyDummy", assemblyDummy, &err))
                {
                    CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(assemblyDummy);
                    if (obj == nullptr)
                        errMsg = "invalid assembly dummy";
                }
            });
            if (errMsg.empty())
            {
                std::string infoStr;
                std::vector<int> sel = App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0];
                if (CFileOperations::loadModel(nullptr, false, false, &buffer, false, false, &infoStr, &errMsg))
                {
                    int handle = App::scene->sceneObjects->getLastSelectionHandle();
                    outStack->pushHandleOntoStack(handle);
                    outStack->pushTextOntoStack(infoStr.c_str());
                    CSceneObject* it = App::scene->sceneObjects->getObjectFromHandle(handle);
                    if (assemblyDummy == -1)
                    {
                        CPose tr = it->getLocalTransformation();
                        tr.X += offset;
                        it->setLocalTransformation(tr);
                    }
                    else
                    {
                        if (!App::assemble(assemblyDummy, handle, false))
                            errMsg = "failed assembling model onto dummy";
                    }
                    App::scene->sceneObjects->setSelectedObjectHandles(sel.data(), sel.size());
#ifdef SIM_WITH_GUI
                    GuiApp::setRebuildHierarchyFlag();
#endif
                }
            }
        }
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_loadModelInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_app)
    {
        if (checkInputArguments(inStack, &errMsg, {arg_string}))
        {
            std::string path = fetchText(inStack, 0);
            std::string infoStr;
            std::vector<int> sel = App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0];
            if (VFile::doesFileExist(path.c_str()))
            {
                CSer serObj(path.c_str(), CSer::getFileTypeFromName(path.c_str()));
                int result = serObj.readOpenBinary(1, true, &infoStr, &errMsg);
                if (result == 1)
                {
                    CThumbnail* thumbO = App::scene->environment->modelThumbnail_notSerializedHere.copyYourself();
                    CPose modelTr;
                    modelTr.setIdentity();
                    C3Vector modelBoundingBoxSize;
                    modelBoundingBoxSize.clear();
                    double modelNonDefaultTranslationStepSize = 0.0;
                    App::scene->loadModel(serObj, true, false, &modelTr, &modelBoundingBoxSize, &modelNonDefaultTranslationStepSize);
                    CThumbnail* retThumbnail = App::scene->environment->modelThumbnail_notSerializedHere.copyYourself();
                    App::scene->environment->modelThumbnail_notSerializedHere.copyFrom(thumbO);
                    delete thumbO;
                    if (retThumbnail->getPointerToUncompressedImage() != nullptr)
                        result = 1;
                    serObj.readClose();

                    char* buff = new char[128 * 128 * 4];
                    bool opRes = retThumbnail->copyUncompressedImageToBuffer(buff);
                    delete retThumbnail;
                    if (opRes)
                    {
                        outStack->pushBufferOntoStack(buff, 128 * 128 * 4);
                        delete[] buff;
                        outStack->pushVector3OntoStack(modelBoundingBoxSize);
                        outStack->pushPoseOntoStack(modelTr);
                        outStack->pushTextOntoStack(infoStr.c_str());
                        return errMsg;
                    }
                    delete[] buff;
                    return errMsg;
                }
                else
                    errMsg = "File could not be read.";
            }
            else
                errMsg = SIM_ERROR_FILE_NOT_FOUND;

            App::scene->sceneObjects->setSelectedObjectHandles(sel.data(), sel.size());
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
        }
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_loadModelInfoFromBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_app)
    {
        if (checkInputArguments(inStack, &errMsg, {arg_string}))
        {
            std::string data = fetchBuffer(inStack, 0);
            std::vector<char> buffer(data.data(), data.data() + data.size());
            std::string infoStr;
            std::vector<int> sel = App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0];
            CSer serObj(buffer, CSer::filetype_csim_bin_model_buff);
            int result = serObj.readOpenBinary(1, true, &infoStr, &errMsg);
            if (result == 1)
            {
                CThumbnail* thumbO = App::scene->environment->modelThumbnail_notSerializedHere.copyYourself();
                CPose modelTr;
                modelTr.setIdentity();
                C3Vector modelBoundingBoxSize;
                modelBoundingBoxSize.clear();
                double modelNonDefaultTranslationStepSize = 0.0;
                App::scene->loadModel(serObj, true, false, &modelTr, &modelBoundingBoxSize, &modelNonDefaultTranslationStepSize);
                CThumbnail* retThumbnail = App::scene->environment->modelThumbnail_notSerializedHere.copyYourself();
                App::scene->environment->modelThumbnail_notSerializedHere.copyFrom(thumbO);
                delete thumbO;
                if (retThumbnail->getPointerToUncompressedImage() != nullptr)
                    result = 1;
                serObj.readClose();

                char* buff = new char[128 * 128 * 4];
                bool opRes = retThumbnail->copyUncompressedImageToBuffer(buff);
                delete retThumbnail;
                if (opRes)
                {
                    outStack->pushBufferOntoStack(buff, 128 * 128 * 4);
                    delete[] buff;
                    outStack->pushVector3OntoStack(modelBoundingBoxSize);
                    outStack->pushPoseOntoStack(modelTr);
                    outStack->pushTextOntoStack(infoStr.c_str());
                    return errMsg;
                }
                delete[] buff;
                return errMsg;
            }
            else
                errMsg = "File could not be read.";
            App::scene->sceneObjects->setSelectedObjectHandles(sel.data(), sel.size());
            setLastInfo(infoStr.c_str());
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
        }
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_saveModel(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
    {
        std::string filename = fetchText(inStack, 0);
        if (!App::scene->environment->getSceneLocked())
        {
            if (target->getModelBase())
            {
                std::vector<int> initSelection(App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0]);
                if (CFileOperations::saveModel(target->getObjectHandle(), filename.c_str(), false, nullptr, nullptr, &errMsg))
                    App::scene->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
            }
            else
                errMsg = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
        }
        else
            errMsg = SIM_ERROR_SCENE_LOCKED;
    }
    return errMsg;
}

std::string _method_saveModelToBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        if (!App::scene->environment->getSceneLocked())
        {
            if (target->getModelBase())
            {
                std::vector<int> initSelection(App::scene->sceneObjects->getSelectedObjectHandlesPtr()[0]);
                std::vector<char> buffer;
                std::string infoStr;
                if (CFileOperations::saveModel(target->getObjectHandle(), nullptr, false, &buffer, &infoStr, &errMsg))
                {
                    App::scene->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
                    setLastInfo(infoStr.c_str());
                    outStack->pushBufferOntoStack(buffer.data(), buffer.size());
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

std::string _method_loadScene(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_app)
    {
        if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation) && (currentScript->getScriptType() != sim_scripttype_customization)))
        {
            if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
            {
                std::string path = fetchText(inStack, 0);
                bool createNewScene = false;
                withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                {
                    map->fetchBoolFromKey("createNew", createNewScene, &err);
                });
                if (errMsg.empty())
                {
                    if (App::scene->simulation->isSimulationStopped())
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
                                App::scene->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                            }
                        }
                        else
                            CFileOperations::createNewScene(createNewScene);
                    }
                    else
                        errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
                }
            }
        }
        else
            errMsg = "cannot be called from an embedded script.";
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_loadSceneFromBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_app)
    {
        if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation) && (currentScript->getScriptType() != sim_scripttype_customization)))
        {
            if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
            {
                std::vector<char> buff;
                fetchBuffer(inStack, 0, buff);
                bool createNewScene = false;
                withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                {
                    map->fetchBoolFromKey("createNew", createNewScene, &err);
                });
                if (errMsg.empty())
                {
                    if (App::scene->simulation->isSimulationStopped())
                    {
                        if (createNewScene)
                            CFileOperations::createNewScene(true);
                        CFileOperations::loadScene(nullptr, false, &buff, nullptr, &errMsg);
                    }
                    else
                        errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
                }
            }
        }
        else
            errMsg = "cannot be called from an embedded script.";
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_save(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_scene)
    {
        if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation)))
        {
            if (checkInputArguments(inStack, &errMsg, {arg_string}))
            {
                std::string path = fetchText(inStack, 0);
                if (App::scene->simulation->isSimulationStopped())
                {
                    if (!App::scene->environment->getSceneLocked())
                    {
                        if (App::scene->environment->getRequestFinalSave())
                            App::scene->environment->setSceneLocked(); // silent locking!
                        if (CFileOperations::saveScene(path.c_str(), false, false, nullptr, nullptr, &errMsg))
                        {
#ifdef SIM_WITH_GUI
                            GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
                            App::scene->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
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
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_saveToBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (targetObj == sim_handle_scene)
    {
        if ((currentScript == nullptr) || ((currentScript->getScriptType() != sim_scripttype_simulation)))
        {
            if (checkInputArguments(inStack, &errMsg, {}))
            {
                if (App::scene->simulation->isSimulationStopped())
                {
                    if (!App::scene->environment->getSceneLocked())
                    {
                        std::vector<char> buffer;
                        std::string infoStr;
                        if (CFileOperations::saveScene(nullptr, false, false, &buffer, &infoStr, &errMsg))
                        {
#ifdef SIM_WITH_GUI
                            GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
                            setLastInfo(infoStr.c_str());
                            outStack->pushBufferOntoStack(buffer.data(), buffer.size());
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
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_removeModel(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        bool delayed = false;
        bool noError = false;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("delayed", delayed, &err);
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (target->getModelBase())
            {
                // Erase the objects:
                std::vector<int> sel;
                sel.push_back(targetObj);
                App::scene->sceneObjects->addModelObjects(sel);
                App::scene->sceneObjects->eraseObjects(&sel, true, delayed);
            }
            else
                errMsg = SIM_ERROR_OBJECT_NOT_MODEL_BASE;
        }
    }
    return errMsg;
}

std::string _method_remove(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        if (targetObj == sim_handle_scene)
        {
            if ((currentScript == nullptr) || (currentScript->getScriptType() == sim_scripttype_addon) || (currentScript->getScriptType() == sim_scripttype_sandbox))
            {
                if (App::scene->simulation->isSimulationStopped())
                    CFileOperations::closeScene();
                else
                    errMsg = SIM_ERROR_SIMULATION_NOT_STOPPED;
            }
            else
                errMsg = "cannot be called from the current script type.";
        }
        else
        {
            bool delayed = false;
            bool noError = false;
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchBoolFromKey("delayed", delayed, &err);
                map->fetchBoolFromKey("noError", noError, &err);
            });
            if (errMsg.size() == 0)
            {
                CSceneObject* sceneObj = getSceneObject(targetObj);
                CCollection* coll = getCollection(targetObj);
                CDetachedScript* script = getDetachedScript(targetObj);
                if (sceneObj != nullptr)
                {
                    std::vector<int> sel;
                    sel.push_back(targetObj);
                    App::scene->sceneObjects->eraseObjects(&sel, true, delayed);
                }
                else if (coll != nullptr)
                    App::scene->collections->removeCollection(targetObj);
                else if (script != nullptr)
                {
                    if (!App::scenes->addOnScriptContainer->removeAddOn(targetObj))
                        errMsg = SIM_ERROR_INVALID_SCRIPT_TYPE_OR_DOES_NOT_EXIST;
                }
                else
                {
                    CustomObject* customObj = App::scenes->customObjects->getObject(targetObj);
                    CustomObject* customClass = App::scenes->customObjects->getClass(targetObj);
                    if (customObj != nullptr)
                        App::scenes->customObjects->removeObject(targetObj);
                    else if (customClass != nullptr)
                        App::scenes->customObjects->removeClass(targetObj);
                    else
                    {
                        customObj = App::scene->customObjects->getObject(targetObj);
                        if (customObj != nullptr)
                            App::scene->customObjects->removeObject(targetObj);
                        else
                        {
                            CSceneObject* customSceneObjectClass = App::scenes->customSceneObjectClasses->getClass(targetObj);
                            if (customSceneObjectClass != nullptr)
                                App::scenes->customSceneObjectClasses->removeClass(targetObj);
                            else if (!noError)
                                errMsg = "object does not exist or cannot be removed.";
                        }
                    }
                }
            }
        }
    }
    return errMsg;
}

std::string _method_removeObjects(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_handlearray, arg_map | arg_optional}))
    {
        std::vector<int64_t> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        bool delayed = false;
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("delayed", delayed, &err);
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
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
                    App::scene->collections->removeCollection(objectHandle);
                else if (draw != nullptr)
                    App::scene->drawingCont->removeObject(objectHandle);
                else if (script != nullptr)
                {
                    if ((!App::scenes->addOnScriptContainer->removeAddOn(objectHandle)) && (!noError))
                        errMsg = SIM_ERROR_INVALID_SCRIPT_TYPE_OR_DOES_NOT_EXIST;
                }
                else
                {
                    if (!noError)
                        errMsg = SIM_ERROR_FOUND_INVALID_HANDLES;
                }
                if (errMsg != "")
                    break;
            }
            if ((errMsg == "") && (sceneObjectHandles.size() > 0))
            {
                if ((!App::scene->sceneObjects->eraseObjects(&sceneObjectHandles, true, delayed)) && (!noError))
                    errMsg = SIM_ERROR_FOUND_INVALID_HANDLES;
            }
        }
    }
    return errMsg;
}

std::string _method_duplicateObjects(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_handlearray, arg_map | arg_optional}))
    {
        std::vector<int64_t> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        bool models = false;
        bool noScripts = false;
        bool noCustomData = false;
        bool noObjectRefs = false;
        bool noTextures = false;
        bool noDna = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("models", models, &err);
            map->fetchBoolFromKey("noScripts", noScripts, &err);
            map->fetchBoolFromKey("noCustomData", noCustomData, &err);
            map->fetchBoolFromKey("noObjectRefs", noObjectRefs, &err);
            map->fetchBoolFromKey("noTextures", noTextures, &err);
            map->fetchBoolFromKey("noDna", noDna, &err);
        });
        if (errMsg.size() == 0)
        {
            if (objectHandles.size() > 0)
            {
                int options = 0;
                if (models)
                    options = options | 1;
                if (noScripts)
                    options = options | 2;
                if (noCustomData)
                    options = options | 4;
                if (noObjectRefs)
                    options = options | 8;
                if (noTextures)
                    options = options | 16;
                if (noDna)
                    options = options | 32;
                // memorize current selection:
                std::vector<int> initSel;
                for (size_t i = 0; i < App::scene->sceneObjects->getSelectionCount(); i++)
                    initSel.push_back(App::scene->sceneObjects->getObjectHandleFromSelectionIndex(i));
                for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(); i++)
                    App::scene->sceneObjects->getObjectFromIndex(i)->setCopyString("");
                // adjust the selection to copy:
                std::vector<int> selT;
                for (size_t i = 0; i < objectHandles.size(); i++)
                {
                    CSceneObject* it = App::scene->sceneObjects->getObjectFromHandle(int(objectHandles[i]));
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
                        CSceneObject* it = App::scene->sceneObjects->getObjectFromHandle(selT[i]);
                        bool ok = true;
                        if (it->getParent() != nullptr)
                        {
                            for (size_t j = 0; j < selT.size(); j++)
                            {
                                CSceneObject* it2 = App::scene->sceneObjects->getObjectFromHandle(selT[j]);
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
                    App::scene->sceneObjects->addModelObjects(sel);
                if ((options & 2) == 0)
                    App::scene->sceneObjects->addCompatibilityScripts(sel);
                App::scenes->copyBuffer->memorizeBuffer();
                App::scenes->copyBuffer->copyCurrentSelection(sel, App::scene->environment->getSceneLocked(), options >> 1);
                App::scene->sceneObjects->deselectObjects();
                App::scenes->copyBuffer->pasteBuffer(App::scene->environment->getSceneLocked(), 0);
                App::scenes->copyBuffer->restoreBuffer();
                App::scenes->copyBuffer->clearMemorizedBuffer();

                // Restore the initial selection:
                App::scene->sceneObjects->deselectObjects();
                for (size_t i = 0; i < initSel.size(); i++)
                    App::scene->sceneObjects->addObjectToSelection(initSel[i]);

                for (size_t i = 0; i < objectHandles.size(); i++)
                { // now return the handles of the copies. Each input handle has a corresponding output handle:
                    CSceneObject* original = App::scene->sceneObjects->getObjectFromHandle(int(objectHandles[i]));
                    objectHandles[i] = -1; // a handle in the output array can be -1 (e.g. with stripped-away scripts)
                    if (original != nullptr)
                    {
                        std::string str = original->getCopyString();
                        original->setCopyString("");
                        for (size_t j = 0; j < App::scene->sceneObjects->getObjectCount(); j++)
                        {
                            CSceneObject* potentialCopy = App::scene->sceneObjects->getObjectFromIndex(j);
                            if (potentialCopy->getCopyString().compare(str) == 0)
                            {
                                objectHandles[i] = potentialCopy->getObjectHandle();
                                break;
                            }
                        }
                    }
                }
            }
            outStack->pushHandleArrayOntoStack(objectHandles.data(), objectHandles.size());
        }
    }
    return errMsg;
}

std::string _method_addItem(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CCollection* target = getCollection(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_handle, arg_map | arg_optional}))
    {
        int objectHandle = fetchHandle(inStack, 0);
        int what = sim_handle_single;
        bool excludeObj = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32FromKey("mode", what, &err);
            map->fetchBoolFromKey("excludeObject", excludeObj, &err);
        });
        if (errMsg.size() == 0)
        {
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
    }
    return errMsg;
}

std::string _method_removeItem(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CCollection* target = getCollection(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_handle, arg_map | arg_optional}))
    {
        int objectHandle = fetchHandle(inStack, 0);
        int what = sim_handle_single;
        bool excludeObj = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32FromKey("mode", what, &err);
            map->fetchBoolFromKey("excludeObject", excludeObj, &err);
        });
        if (errMsg.size() == 0)
        {
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
    }
    return errMsg;
}

std::string _method_checkCollision(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (doesEntityExist(targetObj, &errMsg, -1) && checkInputArguments(inStack, &errMsg, {arg_handle | arg_optional}))
    {
        int otherEntity = fetchHandle(inStack, 0, sim_handle_all);
        if ((otherEntity == sim_handle_all) || doesEntityExist(otherEntity, &errMsg, 0))
        {
            int collidingIds[2] = {-1, -1};
            if (otherEntity == sim_handle_all)
                otherEntity = -1;
            outStack->pushBoolOntoStack( CCollisionRoutine::doEntitiesCollide(targetObj, otherEntity, nullptr, true, true, collidingIds));
            outStack->pushHandleArrayOntoStack(collidingIds, 2);
        }
    }
    return errMsg;
}

std::string _method_checkDistance(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (doesEntityExist(targetObj, &errMsg, -1) && checkInputArguments(inStack, &errMsg, {arg_handle | arg_optional, arg_map | arg_optional}))
    {
        int otherEntity = fetchHandle(inStack, 0, sim_handle_all);
        double threshold = 0.0;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchDoubleFromKey("threshold", threshold, &err);
        });
        if (errMsg.size() == 0)
        {
            if ((otherEntity == sim_handle_all) || doesEntityExist(otherEntity, &errMsg, 0))
            {
                int distIds[2] = {-1, -1};
                if (otherEntity == sim_handle_all)
                    otherEntity = -1;
                int buffer[4];
                App::scene->cacheData->getCacheDataDist(targetObj, otherEntity, buffer);
                if (threshold <= 0.0)
                    threshold = DBL_MAX;
                double distanceData[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
                bool result = CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(targetObj, otherEntity, threshold, distanceData, buffer, buffer + 2, true, true);
                App::scene->cacheData->setCacheDataDist(targetObj, otherEntity, buffer);
                if (result)
                {
                    distIds[0] = buffer[0];
                    distIds[1] = buffer[2];
                }
                outStack->pushBoolOntoStack(result);
                outStack->pushDoubleOntoStack(distanceData[6]);
                outStack->pushVector3OntoStack(C3Vector(distanceData));
                outStack->pushVector3OntoStack(C3Vector(distanceData + 3));
                outStack->pushHandleArrayOntoStack(distIds, 2);
            }
        }
    }
    return errMsg;
}

std::string _method_checkSensor(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
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
            if (checkInputArguments(inStack, &errMsg, {arg_handle | arg_optional, arg_map | arg_optional}))
            {
                int entity = fetchHandle(inStack, 0, sim_handle_all);
                bool frontFaces;
                bool backFaces;
                bool exact;
                bool hasFrontFaces = false;
                bool hasBackFaces = false;
                bool hasExact = false;
                double maxNormal = 0.0;
                bool hasMaxNormal = false;
                withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                {
                    hasFrontFaces = map->fetchBoolFromKey("frontFaces", frontFaces, &err);
                    hasBackFaces = map->fetchBoolFromKey("backFaces", backFaces, &err);
                    hasExact = map->fetchBoolFromKey("exact", exact, &err);
                    hasMaxNormal = map->fetchDoubleFromKey("maxNormal", maxNormal, &err);
                });
                if (errMsg.size() == 0)
                {
                    if ((entity == sim_handle_all) || doesEntityExist(entity, &errMsg, 0))
                    {
                        if (entity == sim_handle_all)
                            entity = -1;
                        if (!hasFrontFaces)
                            frontFaces = proxSensor->getFrontFaceDetection();
                        if (!hasBackFaces)
                            backFaces = proxSensor->getBackFaceDetection();
                        if (!hasExact)
                            exact = proxSensor->getExactMode();
                        if (!hasMaxNormal)
                            maxNormal = proxSensor->getAllowedNormal();
                        int detectedObj;
                        C3Vector dPoint;
                        dPoint.clear();
                        double minThreshold = -1.0;
                        if (proxSensor->convexVolume->getSmallestDistanceAllowed() > 0.0)
                            minThreshold = proxSensor->convexVolume->getSmallestDistanceAllowed();
                        C3Vector normV;
                        normV.clear();
                        double dist = DBL_MAX;
                        bool detected = CProxSensorRoutine::detectEntity(targetObj, entity, exact, maxNormal > 0.0, maxNormal, dPoint, dist, frontFaces, backFaces, detectedObj, minThreshold, normV, true);
                        if (!detected)
                            dist = 0.0;
                        outStack->pushBoolOntoStack(detected);
                        outStack->pushDoubleOntoStack(dist);
                        outStack->pushVector3OntoStack(dPoint);
                        outStack->pushHandleOntoStack(detectedObj);
                        outStack->pushVector3OntoStack(normV);
                    }
                }
            }
        }
        if (visionSensor != nullptr)
        {
            if (checkInputArguments(inStack, &errMsg, {arg_handle | arg_optional}))
            {
                int entity = fetchHandle(inStack, 0, sim_handle_all);
                if ((entity == sim_handle_all) || doesEntityExist(entity, &errMsg, 0))
                {
                    bool detection;
                    std::vector<std::vector<double>> packets;
                    visionSensor->checkSensor(entity, true, &detection, &packets);
                    outStack->pushBoolOntoStack(detection);
                    if (packets.size() >= 1)
                        outStack->pushDoubleArrayOntoStack(packets[0].data(), packets[0].size());
                    else
                        outStack->pushDoubleArrayOntoStack(nullptr, 0);
                    if (packets.size() >= 2)
                        outStack->pushDoubleArrayOntoStack(packets[1].data(), packets[1].size());
                    else
                        outStack->pushDoubleArrayOntoStack(nullptr, 0);
                }
            }
        }
    }
    else
        errMsg = SIM_ERROR_METHOD_NOT_AVAILABLE_FOR_THAT_OBJECT;
    return errMsg;
}

std::string _method_resetSensor(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* visionSensor = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, nullptr, -1);
    CProxSensor* proxSensor = nullptr;
    if (visionSensor == nullptr)
        proxSensor = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((visionSensor != nullptr) || (proxSensor != nullptr))
    {
        if (checkInputArguments(inStack, &errMsg, {}))
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

std::string _method_handleSensor(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* visionSensor = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, nullptr, -1);
    CProxSensor* proxSensor = nullptr;
    if (visionSensor == nullptr)
        proxSensor = (CProxSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_proximitysensor, &errMsg, -1);
    if ((visionSensor != nullptr) || (proxSensor != nullptr))
    {
        if (checkInputArguments(inStack, &errMsg, {}))
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
                outStack->pushBoolOntoStack(detected);
                if (detected)
                {
                    smallest = proxSensor->getDetectedPoint();
                    smallestL = smallest.getLength();
                }
                outStack->pushDoubleOntoStack(smallestL);
                outStack->pushVector3OntoStack(smallest);
                outStack->pushHandleOntoStack(detectedObj);
                outStack->pushVector3OntoStack(detectedN);
            }
            if (visionSensor != nullptr)
            {
                bool detection = visionSensor->handleSensor();
                outStack->pushBoolOntoStack(detection);
                if (visionSensor->sensorAuxiliaryResult.size() >= 1)
                    outStack->pushDoubleArrayOntoStack(visionSensor->sensorAuxiliaryResult[0].data(), visionSensor->sensorAuxiliaryResult[0].size());
                else
                    outStack->pushDoubleArrayOntoStack(nullptr, 0);
                if (visionSensor->sensorAuxiliaryResult.size() >= 2)
                    outStack->pushDoubleArrayOntoStack(visionSensor->sensorAuxiliaryResult[1].data(), visionSensor->sensorAuxiliaryResult[1].size());
                else
                    outStack->pushDoubleArrayOntoStack(nullptr, 0);
            }
        }
    }
    else
        errMsg = SIM_ERROR_METHOD_NOT_AVAILABLE_FOR_THAT_OBJECT;
    return errMsg;
}

std::string _method_getObjects(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if ((targetObj == sim_handle_app) || (targetObj == sim_handle_scene))
    {
        if (checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
        {
            std::vector<std::string> types;
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchStringArrayFromKey("types", types, &err);
            });
            if (errMsg.empty())
            {
                std::vector<int> objects;
                if (types.size() == 0)
                    types = {"sceneObject", "drawingObject", "collection", "detachedScript", "mesh"};
                for (size_t j = 0; j < types.size(); j++)
                {
                    std::string t = types[j];
                    if (t == "sceneObject")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(); i++)
                            objects.push_back(App::scene->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "shape")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
                            objects.push_back(App::scene->sceneObjects->getShapeFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "joint")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_joint); i++)
                            objects.push_back(App::scene->sceneObjects->getJointFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "dummy")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_dummy); i++)
                            objects.push_back(App::scene->sceneObjects->getDummyFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "script")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_script); i++)
                            objects.push_back(App::scene->sceneObjects->getScriptFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "marker")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_marker); i++)
                            objects.push_back(App::scene->sceneObjects->getMarkerFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "mirror")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_mirror); i++)
                            objects.push_back(App::scene->sceneObjects->getMirrorFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "graph")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_graph); i++)
                            objects.push_back(App::scene->sceneObjects->getGraphFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "light")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_light); i++)
                            objects.push_back(App::scene->sceneObjects->getLightFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "camera")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_camera); i++)
                            objects.push_back(App::scene->sceneObjects->getCameraFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "proximitySensor")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_proximitysensor); i++)
                            objects.push_back(App::scene->sceneObjects->getProximitySensorFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "visionSensor")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_visionsensor); i++)
                            objects.push_back(App::scene->sceneObjects->getVisionSensorFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "path")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_path); i++)
                            objects.push_back(App::scene->sceneObjects->getPathFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "mill")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_mill); i++)
                            objects.push_back(App::scene->sceneObjects->getMillFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "forceSensor")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_forcesensor); i++)
                            objects.push_back(App::scene->sceneObjects->getForceSensorFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "ocTree")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_octree); i++)
                            objects.push_back(App::scene->sceneObjects->getOctreeFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "pointCloud")
                    {
                        for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_pointcloud); i++)
                            objects.push_back(App::scene->sceneObjects->getPointCloudFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "drawingObject")
                    {
                        for (size_t i = 0; i < App::scene->drawingCont->getObjectCount(); i++)
                            objects.push_back(App::scene->drawingCont->getObjectFromIndex(i)->getObjectHandle());
                    }
                    else if (t == "collection")
                    {
                        for (size_t i = 0; i < App::scene->collections->getObjectCount(); i++)
                            objects.push_back(int(App::scene->collections->getObjectFromIndex(i)->getObjectHandle()));
                    }
                    else if (t == "detachedScript")
                    {
                        if (App::scenes->sandboxScript != nullptr)
                            objects.push_back(App::scenes->sandboxScript->getSceneObjectOrDetachedScriptHandle());
                        std::vector<int> addOns = App::scenes->addOnScriptContainer->getAddOnHandles();
                        objects.insert(objects.end(), addOns.begin(), addOns.end());
                        objects.push_back(App::scene->sceneObjects->embeddedScriptContainer->getMainScript()->getSceneObjectOrDetachedScriptHandle());
                    }
                    else if (t == "mesh")
                    {
                        std::vector<CMesh*> meshes;
                        App::scene->sceneObjects->getAllMeshes(meshes);
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
                    outStack->pushHandleArrayOntoStack(objects.data(), objects.size());
            }
        }
    }

    return errMsg;
}

std::string _method_addItems(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<float> pts;
        fetchMatrixData(inStack, 0, pts, false);
        std::vector<float> ccols;
        std::vector<float> quats;
        std::vector<float> sizes;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchMatrixDataFromKey("colors", ccols, 4, int(pts.size() / 3), false, &err);
            map->fetchMatrixDataFromKey("quaternions", quats, 4, int(pts.size() / 3), false, &err);
            map->fetchMatrixDataFromKey("sizes", sizes, 3, int(pts.size() / 3), false, &err);
        });
        if (errMsg.empty())
        {
            std::vector<unsigned char> cols;
            cols.resize(ccols.size());
            for (size_t i = 0; i < ccols.size(); i++)
                cols[i] = (uint8_t)(ccols[i] * 255.1f);
            std::vector<int64_t> newIds;
            target->addItems(&pts, &quats, &cols, &sizes, true, &newIds);
            outStack->pushInt64ArrayOntoStack(newIds.data(), newIds.size());
        }
    }
    return errMsg;
}

std::string _method_clearItems(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        target->remItems(0);
    }
    return errMsg;
}

std::string _method_removeItems(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMarker* target = (CMarker*)getSpecificSceneObjectType(targetObj, sim_sceneobject_marker, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_table, -1, arg_integer}))
    {
        std::vector<int64_t> ids;
        fetchInt64Array(inStack, 0, ids);
        target->remItems(&ids);
    }
    return errMsg;
}

std::string _method_callFunction(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
    {
        std::string funcName = fetchText(inStack, 0);
        if (target->hasInterpreterState())
        {
            CInterfaceStack* inStack2 = App::scenes->interfaceStackContainer->createStackCopy(inStack);
            delete inStack2->detachStackObjectFromIndex(0);
            size_t p = funcName.find(':');
            if (p == std::string::npos)
            {
                int rr = target->callCustomScriptFunction(funcName.c_str(), inStack2, outStack);
                if (rr != 1)
                {
                    if (rr == -1)
                        errMsg = SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION;
                    else // rr==0
                        errMsg = SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                }
            }
            else
            {
                int h;
                if (tt::stringToInt(funcName.substr(0, p).c_str(), h))
                {
                    funcName.erase(0, p + 1);
                    funcName = "@" + funcName; // to indicate that we come from c
                    CInterfaceStack* inStackT = App::scenes->interfaceStackContainer->createStack();
                    inStackT->copyFrom(inStack);
                    inStackT->insertItem(0, new CInterfaceStackString(funcName.c_str()));
                    inStackT->insertItem(0, new CInterfaceStackHandle(h));
                    std::string errorMsg;
                    int rr = target->callCustomScriptFunction("@__2.sim.callMethod", inStackT, outStack, &errMsg); // @ here to indicate we do not want to call sysCall_ext, and __2 is the only global variable in sim-2!
                    App::scenes->interfaceStackContainer->destroyStack(inStackT);
                    if (rr <= 0)
                        errMsg = SIM_ERROR_FAILED_CALLING_METHOD;
                    else
                    {
                        CInterfaceStackString* res = (CInterfaceStackString*)outStack->detachStackObjectFromIndex(0); // first ret val is always an error string
                        size_t l;
                        std::string err(res->getValue(&l));
                        delete res;
                        if (err.size() > 0)
                            errMsg = err;
                    }
                }
                else
                    errMsg = SIM_ERROR_FAILED_CALLING_METHOD;
            }
        }
        else
            errMsg = SIM_ERROR_SCRIPT_NOT_INITIALIZED;
    }
    return errMsg;
}

std::string _method_executeString(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
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
                    if (target->getScriptState() == sim_scriptstate_initialized)
                    {
                        CInterfaceStack* tmpStack = App::scenes->interfaceStackContainer->createStack();
                        tmpStack->pushTextOntoStack(stringToExecute.c_str());
                        retVal = target->callCustomScriptFunction("_evalExecRet", tmpStack, outStack);
                        App::scenes->interfaceStackContainer->destroyStack(tmpStack);
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
/*
std::string _method_getApiInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
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

std::string _method_getApiFunc(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
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
*/
std::string _method_getStackTraceback(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        outStack->pushTextOntoStack(target->getAndClearLastStackTraceback().c_str());
    }
    return errMsg;
}

std::string _method_init(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        if (currentScript == target)
            App::asyncResetScript(targetObj); // delayed
        else
            target->initScript();
    }
    return errMsg;
}

std::string _method_scale(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_vector3}))
    {
        C3Vector s = fetchVector3(inStack, 0);
        if ((s(0) >= 0.0001) && (s(1) >= 0.0001) && (s(2) >= 0.0001))
            target->scaleObjectNonIsometrically(s(0), s(1), s(2));
        else
            errMsg = SIM_ERROR_INVALID_INPUT;
    }
    return errMsg;
}

std::string _method_scaleTree(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if (checkInputArguments(inStack, &errMsg, {arg_double, arg_map | arg_optional}))
    {
        double scalingFactor = fetchDouble(inStack, 0);
        bool rootPositionIsScaled = true;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("scaleRootPosition", rootPositionIsScaled, &err);
        });
        if (errMsg.empty())
        {
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
    }
    return errMsg;
}

std::string _method_startSimulation(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
    {
        if (!App::scene->simulation->isSimulationRunning())
            App::scene->simulation->startOrResumeSimulation();
    }
    return errMsg;
}

std::string _method_pauseSimulation(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
    {
        if (App::scene->simulation->isSimulationRunning())
            App::scene->simulation->pauseSimulation();
    }
    return errMsg;
}

std::string _method_stopSimulation(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
    {
        if (!App::scene->simulation->isSimulationStopped())
        {
            App::scene->simulation->incrementStopRequestCounter();
            App::scene->simulation->stopSimulation();
        }
    }
    return errMsg;
}

std::string _method_getName(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* targetTemplate = App::scenes->customSceneObjectClasses->getClass(targetObj);
    if (targetTemplate == nullptr)
    {
        CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
        if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string | arg_optional}))
        {
            std::string t = fetchText(inStack, 0, "name");
            std::string nm;
            if (t == "name")
                nm = target->getObjectAlias();
            else if (t == "nameOrder")
            {
                nm = target->getObjectAliasAndOrderIfRequired();
                if (nm[nm.size() - 1] != ']')
                    nm += "[0]";
            }
            else if (t == "shortPath")
                nm = target->getObjectAlias_shortPath();
            else if (t == "fullPath")
                nm = target->getObjectAlias_fullPath();
            else if (t == "nameHandle")
            {
                nm = target->getObjectAlias() + "__";
                nm += std::to_string(target->getObjectHandle());
                nm += "__";
            }
            else if (t == "printPath")
                nm = target->getObjectAlias_printPath();
            else if (t == "nameIndex")
            {
                nm = target->getObjectPathAndIndex(0);
                nm.erase(0, 1);
                if (nm[nm.size() - 1] != '}')
                    nm += "{0}";
            }
            else if (t == "pathIndex")
            {
                nm = target->getObjectPathAndIndex(999);
                if (nm[nm.size() - 1] != '}')
                    nm += "{0}";
                for (size_t i = nm.size(); i > 1; )
                {
                    --i;
                    if (nm[i] == '/' && nm[i - 1] != '}')
                        nm.insert(i, "{0}");
                }
            }
            if (!nm.empty())
                outStack->pushTextOntoStack(nm.c_str());
            else
                errMsg = "invalid format.";
        }
    }
    else
    {
        std::string s("class ");
        s += targetTemplate->getObjectTypeStr();
        outStack->pushTextOntoStack(s.c_str());
    }
    return errMsg;
}

std::string _method_dynamicReset(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        bool tree = false;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("tree", tree, &err);
        });
        if (errMsg.empty())
            target->setDynamicsResetFlag(true, tree);
    }
    return errMsg;
}

std::string _method_loadImage(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
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
            outStack->pushBufferOntoStack((char*)img, b * res[0] * res [1]);
            outStack->pushInt32ArrayOntoStack(res, 2);
            outStack->pushInt32OntoStack(b);
            delete[]((char*)img);
        }
        else
            errMsg = SIM_ERROR_FAILED_LOADING_IMAGE;
    }
    return errMsg;
}

std::string _method_loadImageFromBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
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
            outStack->pushBufferOntoStack((char*)img, b * res[0] * res [1]);
            outStack->pushInt32ArrayOntoStack(res, 2);
            outStack->pushInt32OntoStack(b);
            delete[]((char*)img);
        }
        else
            errMsg = SIM_ERROR_FAILED_LOADING_IMAGE;
    }
    return errMsg;
}

std::string _method_saveImage(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_string, arg_map | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> res;
        fetchIntArray(inStack, 1, res);
        std::string filename = fetchText(inStack, 2);
        int quality = -1;
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
        withOptionalMap(inStack, 3, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32FromKey("quality", quality, &err);
        });
        if (errMsg.empty())
        {
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
    }
    return errMsg;
}

std::string _method_saveImageToBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_map | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> res;
        fetchIntArray(inStack, 1, res);
        std::string ext = "png";
        int quality = -1;
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
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32FromKey("quality", quality, &err);
            map->fetchStringFromKey("format", ext, &err);
        });
        if (errMsg.empty())
        {
            ext = "." + ext;
            if ((img.size() == res[0] * res[1] * channels) && (res[0] > 0) && (res[1] > 0))
            {
                std::string retBuff;
                if (CImageLoaderSaver::save((unsigned char*)img.c_str(), res.data(), options, ext.c_str(), quality, &retBuff))
                    outStack->pushBufferOntoStack(retBuff.data(), retBuff.size());
                else
                    errMsg = SIM_ERROR_OPERATION_FAILED;
            }
            else
                errMsg = SIM_ERROR_INVALID_RESOLUTION;
        }
    }
    return errMsg;
}

std::string _method_transformImage(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, 2, arg_integer, arg_table, 2, arg_integer, arg_map | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> inRes;
        fetchIntArray(inStack, 1, inRes);
        std::vector<int> outRes;
        fetchIntArray(inStack, 2, outRes);
        std::string type = "rgb";
        std::string aspectRatio = "ignore";
        bool smooth = true;
        bool flipAxisX = false;
        bool flipAxisY = false;
        withOptionalMap(inStack, 3, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("type", type, &err);
            map->fetchStringFromKey("aspectRatio", aspectRatio, &err);
            map->fetchBoolFromKey("smooth", smooth, &err);
            map->fetchBoolFromKey("flipAxisX", flipAxisX, &err);
            map->fetchBoolFromKey("flipAxisY", flipAxisY, &err);
        });
        if (errMsg.empty())
        {
            int options = 0;
            if (!smooth)
                options |= 16;
            if (aspectRatio == "keep")
                options |= 4;
            else if (aspectRatio == "keepExpand")
                options |= 8;
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
                if ((type == "rgb") || (type == "grey"))
                    options -= 2;
                unsigned char* retVal = CImageLoaderSaver::getScaledImage((unsigned char*)img.data(), inRes.data(), outRes.data(), options);
                std::vector<unsigned char> imgOut;
                channels = 3;
                if (type == "rgb")
                    imgOut.assign(retVal, retVal + outRes[0] * outRes[1] * 3);
                else if (type == "rgba")
                {
                    imgOut.assign(retVal, retVal + outRes[0] * outRes[1] * 4);
                    channels = 4;
                }
                else if (type == "grey")
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
                    if (flipAxisX)
                        x2 = outRes[0] - 1 - x;
                    for (int y = 0; y < outRes[1]; y++)
                    {
                        int y2 = y;
                        if (flipAxisY)
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
                outStack->pushBufferOntoStack((char*)imgOut.data(), channels * outRes[0] * outRes[1]);
                outStack->pushInt32ArrayOntoStack(outRes.data(), 2);
                outStack->pushInt32OntoStack(channels);
            }
            else
                errMsg = SIM_ERROR_INVALID_RESOLUTION;
        }
    }
    return errMsg;
}

std::string _method_transformBuffer(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_string, arg_string, arg_map | arg_optional}))
    {
        std::string inBuff = fetchBuffer(inStack, 0);
        std::string inFormat = fetchBuffer(inStack, 1);
        std::string outFormat = fetchBuffer(inStack, 2);
        double scale = 1.0;
        double offset = 0.0;
        bool hasScale = false;
        bool hasOffset = false;
        bool clamp = false;
        withOptionalMap(inStack, 3, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            hasScale = map->fetchDoubleFromKey("scale", scale, &err);
            hasOffset = map->fetchDoubleFromKey("offset", offset, &err);
            map->fetchBoolFromKey("clamp", clamp, &err);
        });
        if (errMsg.empty())
        {
            bool noScalingNorOffset = !(hasScale | hasOffset);
            size_t dataLength = inBuff.size();
            bool invalidData = true;
            if (inFormat == "float")
            {
                const float* data = (const float*)inBuff.data();
                dataLength -= (dataLength % sizeof(float));
                dataLength /= sizeof(float);
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "uint8")
                    {
                        uint8_t* dat = new uint8_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint8_t)(data[i]);
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint8_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                    if (outFormat == "rgb")
                    {
                        uint8_t* dat = new uint8_t[dataLength * 3];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)data[i];
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(data[i] * scale + offset);
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                    if (outFormat == "int8")
                    {
                        char* dat = new char[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (char)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (char)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                    if (outFormat == "uint16")
                    {
                        uint16_t* dat = new uint16_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint16_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint16_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint16_t));
                        delete[] dat;
                    }
                    if (outFormat == "int16")
                    {
                        int16_t* dat = new int16_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int16_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int16_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint16_t));
                        delete[] dat;
                    }
                    if (outFormat == "uint32")
                    {
                        uint32_t* dat = new uint32_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint32_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint32_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint32_t));
                        delete[] dat;
                    }
                    if (outFormat == "int32")
                    {
                        int32_t* dat = new int32_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                                   : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                                   : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int32_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int32_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(int32_t));
                        delete[] dat;
                    }
                    if (outFormat == "uint64")
                    {
                        uint64_t* dat = new uint64_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 18446744073709551615.499) ? (18446744073709551615ULL) : ((uint64_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 18446744073709551615.499) ? (18446744073709551615ULL) : ((uint64_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint64_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint64_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint64_t));
                        delete[] dat;
                    }
                    if (outFormat == "int64")
                    {
                        int64_t* dat = new int64_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i];
                                    dat[i] = (v < -9223372036854775808.499) ? (-9223372036854775807 - 1)
                                                                   : ((v > 9223372036854775807.499) ? (9223372036854775807) : ((int64_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    float v = data[i] * scale + offset;
                                    dat[i] = (v < -9223372036854775808.499) ? (-9223372036854775807 - 1)
                                                                   : ((v > 9223372036854775807.499) ? (9223372036854775807) : ((int64_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int64_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int64_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(int64_t));
                        delete[] dat;
                    }
                    if (outFormat == "float")
                    {
                        float* dat = new float[dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = data[i] * scale + offset;
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(float));
                        delete[] dat;
                    }
                    if (outFormat == "double")
                    {
                        double* dat = new double[dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (double)data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (double)(data[i] * scale + offset);
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(double));
                        delete[] dat;
                    }
                }
            }
            if (inFormat == "double")
            {
                const double* data = (const double*)inBuff.data();
                dataLength -= (dataLength % sizeof(double));
                dataLength /= sizeof(double);
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "uint8")
                    {
                        uint8_t* dat = new uint8_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint8_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint8_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                    if (outFormat == "rgb")
                    {
                        uint8_t* dat = new uint8_t[dataLength * 3];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)data[i];
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(data[i] * scale + offset);
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                    if (outFormat == "int8")
                    {
                        char* dat = new char[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < -128.499) ? (-128) : ((v > 127.499) ? (127) : ((char)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (char)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (char)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                    if (outFormat == "uint16")
                    {
                        uint16_t* dat = new uint16_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 65535.499) ? (65535) : ((uint16_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint16_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint16_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint16_t));
                        delete[] dat;
                    }
                    if (outFormat == "int16")
                    {
                        int16_t* dat = new int16_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < -32768.499) ? (-32768) : ((v > 32767.499) ? (32767) : ((int16_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int16_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int16_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint16_t));
                        delete[] dat;
                    }
                    if (outFormat == "uint32")
                    {
                        uint32_t* dat = new uint32_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 4294967295.499) ? (4294967295) : ((uint32_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint32_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint32_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint32_t));
                        delete[] dat;
                    }
                    if (outFormat == "int32")
                    {
                        int32_t* dat = new int32_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                                   : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < -2147483648.499) ? (-2147483647)
                                                                   : ((v > 2147483647.499) ? (2147483647) : ((int32_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int32_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int32_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint32_t));
                        delete[] dat;
                    }
                    if (outFormat == "uint64")
                    {
                        uint64_t* dat = new uint64_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < 0.0) ? (0) : ((v > 18446744073709551615.499) ? (18446744073709551615ULL) : ((uint64_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 18446744073709551615.499) ? (18446744073709551615ULL) : ((uint64_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint64_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint64_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(uint64_t));
                        delete[] dat;
                    }
                    if (outFormat == "int64")
                    {
                        int64_t* dat = new int64_t[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < -9223372036854775808.499) ? (-9223372036854775807 - 1)
                                                                   : ((v > 9223372036854775807.499) ? (9223372036854775807) : ((int64_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < -9223372036854775808.499) ? (-9223372036854775807 - 1)
                                                                   : ((v > 9223372036854775807.499) ? (9223372036854775807) : ((int64_t)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int64_t)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (int64_t)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(int64_t));
                        delete[] dat;
                    }
                    if (outFormat == "float")
                    {
                        float* dat = new float[dataLength];
                        if (clamp)
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i];
                                    dat[i] = (v < -FLT_MAX) ? (-FLT_MAX) : ((v > FLT_MAX) ? (FLT_MAX) : ((float)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = data[i] * scale + offset;
                                    dat[i] = (v < -FLT_MAX) ? (-FLT_MAX) : ((v > FLT_MAX) ? (FLT_MAX) : ((float)v));
                                }
                            }
                        }
                        else
                        {
                            if (noScalingNorOffset)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (float)data[i];
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (float)(data[i] * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(float));
                        delete[] dat;
                    }
                    if (outFormat == "double")
                    {
                        double* dat = new double[dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = data[i];
                        }
                        else
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = data[i] * scale + offset;
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * sizeof(double));
                        delete[] dat;
                    }
                }
            }
            if ((inFormat == "rgb") || (inFormat == "bgr"))
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                dataLength -= (dataLength % 3);
                dataLength /= 3;
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (((inFormat == "rgb") && (outFormat == "bgr")) ||
                        ((inFormat == "bgr") && (outFormat == "rgb")))
                    {
                        uint8_t* dat = new uint8_t[3 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = data[3 * i + 2];
                                dat[3 * i + 1] = data[3 * i + 1];
                                dat[3 * i + 2] = data[3 * i + 0];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[3 * i + 2]) * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[3 * i + 1]) * scale + offset;
                                    dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[3 * i + 0]) * scale + offset;
                                    dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(double(data[3 * i + 2]) * scale + offset);
                                    dat[3 * i + 1] = (uint8_t)(double(data[3 * i + 1]) * scale + offset);
                                    dat[3 * i + 2] = (uint8_t)(double(data[3 * i + 0]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                    if (inFormat == outFormat)
                    {
                        uint8_t* dat = new uint8_t[3 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = data[3 * i + 0];
                                dat[3 * i + 1] = data[3 * i + 1];
                                dat[3 * i + 2] = data[3 * i + 2];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[3 * i + 0]) * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[3 * i + 1]) * scale + offset;
                                    dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[3 * i + 2]) * scale + offset;
                                    dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(double(data[3 * i + 0]) * scale + offset);
                                    dat[3 * i + 1] = (uint8_t)(double(data[3 * i + 1]) * scale + offset);
                                    dat[3 * i + 2] = (uint8_t)(double(data[3 * i + 2]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                }
            }
            if (inFormat == "rgba")
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                dataLength -= (dataLength % 4);
                dataLength /= 4;
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "rgb")
                    {
                        uint8_t* dat = new uint8_t[3 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = data[4 * i + 0];
                                dat[3 * i + 1] = data[4 * i + 1];
                                dat[3 * i + 2] = data[4 * i + 2];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[4 * i + 0]) * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 1]) * scale + offset;
                                    dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 2]) * scale + offset;
                                    dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(double(data[4 * i + 0]) * scale + offset);
                                    dat[3 * i + 1] = (uint8_t)(double(data[4 * i + 1]) * scale + offset);
                                    dat[3 * i + 2] = (uint8_t)(double(data[4 * i + 2]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                    if (outFormat == inFormat)
                    {
                        uint8_t* dat = new uint8_t[4 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[4 * i + 0] = data[4 * i + 0];
                                dat[4 * i + 1] = data[4 * i + 1];
                                dat[4 * i + 2] = data[4 * i + 2];
                                dat[4 * i + 3] = data[4 * i + 3];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[4 * i + 0]) * scale + offset;
                                    dat[4 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 1]) * scale + offset;
                                    dat[4 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 2]) * scale + offset;
                                    dat[4 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 3]) * scale + offset;
                                    dat[4 * i + 3] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[4 * i + 0] = (uint8_t)(double(data[4 * i + 0]) * scale + offset);
                                    dat[4 * i + 1] = (uint8_t)(double(data[4 * i + 1]) * scale + offset);
                                    dat[4 * i + 2] = (uint8_t)(double(data[4 * i + 2]) * scale + offset);
                                    dat[4 * i + 3] = (uint8_t)(double(data[4 * i + 3]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 4);
                        delete[] dat;
                    }
                }
            }
            if (inFormat == "argb")
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                dataLength -= (dataLength % 4);
                dataLength /= 4;
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "rgb")
                    {
                        uint8_t* dat = new uint8_t[3 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = data[4 * i + 1];
                                dat[3 * i + 1] = data[4 * i + 2];
                                dat[3 * i + 2] = data[4 * i + 3];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[4 * i + 1]) * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 2]) * scale + offset;
                                    dat[3 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 3]) * scale + offset;
                                    dat[3 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(double(data[4 * i + 1]) * scale + offset);
                                    dat[3 * i + 1] = (uint8_t)(double(data[4 * i + 2]) * scale + offset);
                                    dat[3 * i + 2] = (uint8_t)(double(data[4 * i + 3]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                    if (outFormat == inFormat)
                    {
                        uint8_t* dat = new uint8_t[4 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[4 * i + 0] = data[4 * i + 0];
                                dat[4 * i + 1] = data[4 * i + 1];
                                dat[4 * i + 2] = data[4 * i + 2];
                                dat[4 * i + 3] = data[4 * i + 3];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[4 * i + 0]) * scale + offset;
                                    dat[4 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 1]) * scale + offset;
                                    dat[4 * i + 1] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 2]) * scale + offset;
                                    dat[4 * i + 2] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    v = double(data[4 * i + 3]) * scale + offset;
                                    dat[4 * i + 3] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[4 * i + 0] = (uint8_t)(double(data[4 * i + 0]) * scale + offset);
                                    dat[4 * i + 1] = (uint8_t)(double(data[4 * i + 1]) * scale + offset);
                                    dat[4 * i + 2] = (uint8_t)(double(data[4 * i + 2]) * scale + offset);
                                    dat[4 * i + 3] = (uint8_t)(double(data[4 * i + 3]) * scale + offset);
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 4);
                        delete[] dat;
                    }
                }
            }
            if (inFormat == "uint8")
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "base64")
                    {
                        std::string inDat(data, data + dataLength);
                        std::string outDat(utils::encode64(inDat));
                        outStack->pushBufferOntoStack((const char*)outDat.data(), outDat.length());
                    }
                    if (outFormat == "uint8")
                    {
                        uint8_t* dat = new uint8_t[dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = data[i];
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[i]) * scale + offset;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                    dat[i] = (uint8_t)(double(data[i]) * scale + offset);
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                    if (outFormat == "rgb")
                    {
                        uint8_t* dat = new uint8_t[3 * dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                            {
                                dat[3 * i + 0] = data[i];
                                dat[3 * i + 1] = data[i];
                                dat[3 * i + 2] = data[i];
                            }
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = double(data[i]) * scale + offset;
                                    dat[3 * i + 0] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    dat[3 * i + 0] = (uint8_t)(double(data[i]) * scale + offset);
                                    dat[3 * i + 1] = dat[3 * i + 0];
                                    dat[3 * i + 2] = dat[3 * i + 0];
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength * 3);
                        delete[] dat;
                    }
                }
            }
            if ((inFormat == "rgb") || (inFormat == "bgr"))
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                dataLength -= (dataLength % 3);
                dataLength /= 3;
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "uint8")
                    {
                        uint8_t* dat = new uint8_t[dataLength];
                        if (noScalingNorOffset)
                        {
                            for (size_t i = 0; i < dataLength; i++)
                                dat[i] = (int(data[3 * i + 0]) + int(data[3 * i + 1]) + int(data[3 * i + 2])) / 3;
                        }
                        else
                        {
                            if (clamp)
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = offset + (double(data[3 * i + 0]) + double(data[3 * i + 1]) +
                                                      double(data[3 * i + 2])) *
                                                         scale / 3.0;
                                    dat[i] = (v < 0.0) ? (0) : ((v > 255.499) ? (255) : ((uint8_t)v));
                                }
                            }
                            else
                            {
                                for (size_t i = 0; i < dataLength; i++)
                                {
                                    double v = offset + (double(data[3 * i + 0]) + double(data[3 * i + 1]) +
                                                      double(data[3 * i + 2])) *
                                                         scale / 3.0;
                                    dat[i] = (uint8_t)v;
                                }
                            }
                        }
                        outStack->pushBufferOntoStack((const char*)dat, dataLength);
                        delete[] dat;
                    }
                }
            }
            if (inFormat == "base64")
            {
                const uint8_t* data = (const uint8_t*)inBuff.data();
                if (dataLength != 0)
                {
                    invalidData = false;
                    if (outFormat == "uint8")
                    {
                        std::string inDat(data, data + dataLength);
                        std::string outDat(utils::decode64(inDat));
                        outStack->pushBufferOntoStack(outDat.c_str(), outDat.length());
                    }
                }
            }
            if (outStack->getStackSize() == 0)
            {
                if (invalidData)
                    errMsg = SIM_ERROR_INVALID_DATA;
                else
                    errMsg = "unsupported format.";
            }
        }
    }
    return errMsg;
}

std::string _method_getImage(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CCamera* cTarget = (CCamera*)getSpecificSceneObjectType(targetObj, sim_sceneobject_camera);
    CVisionSensor* vTarget = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor);
    if ((vTarget != nullptr) || (cTarget != nullptr))
    {
        if ((vTarget != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
        {
            std::vector<int> pos = {0, 0};
            std::vector<int> size = {0, 0};
            std::string type("rgb");
            double rgbaCutOff = 0.999;
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchInt32ArrayFromKey("position", pos.data(), 2, &err);
                map->fetchInt32ArrayFromKey("size", size.data(), 2, &err);
                map->fetchStringFromKey("type", type, &err);
                map->fetchDoubleFromKey("rgbaCutOff", rgbaCutOff, &err);
            });
            if (errMsg.empty())
            {
                int options = 0;
                int res[2];
                vTarget->getResolution(res);
                if (type == "rgba")
                    options = 2;
                else if (type == "grey")
                    options = 1;
                if (size[0] == 0)
                    size[0] = res[0];
                if (size[1] == 0)
                    size[1] = res[1];
                unsigned char* img = vTarget->readPortionOfCharImage(pos[0], pos[1], size[0], size[1], rgbaCutOff, options);
                if (img != nullptr)
                {
                    int s = 3;
                    if (type == "grey")
                        s = 1; // greyscale
                    if (type == "rgba")
                        s = 4; // + alpha channel
                    outStack->pushBufferOntoStack((char*)img, s * size[0] * size[1]);
                    delete[]((char*)img);
                    outStack->pushInt32ArrayOntoStack(res, 2);
                }
                else
                    errMsg = SIM_ERROR_INVALID_ARGUMENTS;
            }
        }
        if (errMsg.empty() && (cTarget != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
        {
            int resolution[2] = {1920, 1080};
            double clippingPlanes[2] = {0.05, 30.0};
            double viewAngle = 60.0 * degToRad;
            double viewSize = 2.0;
            bool hasViewAngle = false;
            bool hasViewSize = false;
            std::string rendMode = "openGl";
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchInt32ArrayFromKey("resolution", resolution, 2, &err);
                map->fetchDoubleArrayFromKey("clippingPlanes", clippingPlanes, 2, &err);
                map->fetchStringFromKey("renderMode", rendMode, &err);
                hasViewAngle = map->fetchDoubleFromKey("viewAngle", viewAngle, &err);
                hasViewSize = map->fetchDoubleFromKey("viewSize", viewSize, &err);
            });
            if (errMsg.empty())
            {
                auto rmValue = magic_enum::enum_cast<renderMode>(rendMode.c_str());
                if (rmValue.has_value())
                {
                    int rm = static_cast<int>(*rmValue);
                    CVisionSensor* sensor = new CVisionSensor();
                    App::scene->sceneObjects->addObjectToScene(sensor, false, false);
                    sensor->setPerspective(hasViewAngle || (!hasViewSize));
                    sensor->setViewAngle(viewAngle);
                    sensor->setOrthoViewSize(viewSize);
                    sensor->setResolution(resolution);
                    sensor->setClippingPlanes(clippingPlanes[0], clippingPlanes[1]);
                    sensor->setRenderMode(rm);
                    sensor->setLocalTransformation(cTarget->getCumulativeTransformation());
                    sensor->handleSensor();
                    unsigned char* img = sensor->readPortionOfCharImage(0, 0, resolution[0], resolution[1], 1.0, 0);
                    if (img != nullptr)
                    {
                        float* buff = sensor->readPortionOfImage(0, 0, resolution[0], resolution[1], 2);
                        if (buff != nullptr)
                        {
                            double np, fp;
                            sensor->getClippingPlanes(np, fp);
                            float n = (float)np;
                            float f = (float)fp;
                            float fmn = f - n;
                            for (int i = 0; i < resolution[0] * resolution[1]; i++)
                                buff[i] = n + fmn * buff[i];
                            outStack->pushBufferOntoStack((char*)img, 3 * resolution[0] * resolution[1]);
                            outStack->pushBufferOntoStack((char*)buff, resolution[0] * resolution[1] * sizeof(float));
                            outStack->pushInt32ArrayOntoStack(resolution, 2);
                            delete[]((char*)buff);
                        }
                        delete[]((char*)img);
                    }
                    else
                        errMsg = SIM_ERROR_INVALID_ARGUMENTS;
                    App::scene->sceneObjects->eraseObject(sensor, false, false);
                }
                else
                    errMsg = "invalid render mode.";
            }
        }
    }
    else
        errMsg = "invalid object.";
    return errMsg;
}

std::string _method_setImage(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* target = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::vector<char> img;
        fetchBuffer(inStack, 0, img);
        std::vector<int> pos = {0, 0};
        std::vector<int> size = {0, 0};
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32ArrayFromKey("position", pos.data(), 2, &err);
            map->fetchInt32ArrayFromKey("size", size.data(), 2, &err);
        });
        if (errMsg.empty())
        {
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
    }
    return errMsg;
}

std::string _method_getDepth(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CVisionSensor* target = (CVisionSensor*)getSpecificSceneObjectType(targetObj, sim_sceneobject_visionsensor, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        std::vector<int> pos = {0, 0};
        std::vector<int> size = {0, 0};
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32ArrayFromKey("position", pos.data(), 2, &err);
            map->fetchInt32ArrayFromKey("size", size.data(), 2, &err);
        });
        if (errMsg.empty())
        {
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
                outStack->pushMatrixOntoStack(buff, size[1], size[0]);
                delete[]((char*)buff);
                outStack->pushInt32ArrayOntoStack(res, 2);
            }
            else
                errMsg = SIM_ERROR_INVALID_ARGUMENTS;
        }
    }
    return errMsg;
}

std::string _method_relocateFrame(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* target = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_pose | arg_optional}))
    {
        CPose tr = fetchPose(inStack, 0);
        if ((!target->getMesh()->isPure()) || (target->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (hasNonNullArg(inStack, 0))
            {
                if ((tr.Q(0) == 0.0) && (tr.Q(1) == 0.0) && (tr.Q(2) == 0.0) && (tr.Q(3) == 0.0))
                    target->relocateFrame("mesh");
                else
                {
                    tr.Q.normalize();
                    CPose x(tr.getInverse() * target->getCumulativeTransformation());
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

std::string _method_alignBoundingBox(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* target = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_quaternion | arg_optional}))
    {
        CQuaternion q = fetchQuaternion(inStack, 0);
        if ((!target->getMesh()->isPure()) || (target->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (hasNonNullArg(inStack, 0))
            {
                CPose tr;
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

std::string _method_logInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string | arg_optional, arg_map | arg_optional}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                bool val;
                if (map->fetchBoolFromKey("undecorated", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_undecorated;
                }
                if (map->fetchBoolFromKey("onlyTerminal", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_onlyterminal;
                }
                if (map->fetchBoolFromKey("once", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_once;
                }
            });
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

std::string _method_logWarn(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string | arg_optional, arg_map | arg_optional}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                bool val;
                if (map->fetchBoolFromKey("undecorated", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_undecorated;
                }
                if (map->fetchBoolFromKey("onlyTerminal", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_onlyterminal;
                }
                if (map->fetchBoolFromKey("once", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_once;
                }
            });
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

std::string _method_logError(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string | arg_optional, arg_map | arg_optional}))
    {
        std::string msg = fetchText(inStack, 0);
        if (hasNonNullArg(inStack, 0))
        {
            int verb = 0;
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                bool val;
                if (map->fetchBoolFromKey("undecorated", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_undecorated;
                }
                if (map->fetchBoolFromKey("onlyTerminal", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_onlyterminal;
                }
                if (map->fetchBoolFromKey("once", val, &err))
                {
                    if (val)
                        verb |= sim_verbosity_once;
                }
            });
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

std::string _method_quit(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
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

std::string _method_systemLock(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_bool}))
    {
        std::string key = fetchText(inStack, 0);
        bool acquire = fetchBool(inStack, 1);
        App::systemSemaphore(key.c_str(), acquire);
    }
    return errMsg;
}

std::string _method_setStepping(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_bool}))
    {
        bool enable = fetchBool(inStack, 0);
        if (enable)
            target->changeAutoYieldingForbidLevel(1, false);
        else
            target->changeAutoYieldingForbidLevel(-1, false);
    }
    return errMsg;
}

std::string _method_getStepping(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        outStack->pushBoolOntoStack(target->getAutoYieldingForbidLevel() > 0);
    }
    return errMsg;
}

std::string _method_getObject(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string origPath = fetchText(inStack, 0);
        std::string path(origPath);
        if ((path.size() == 0) || ((path[0] != '.') && (path[0] != '/')))
            path = "./" + path;
        int index = -1;
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32FromKey("index", index, &err);
            map->fetchBoolFromKey("noError", noError, &err);
        });
        CSceneObject* it = nullptr;
        CSceneObject* prox = nullptr;
        if (targetObj >= 0)
        {
            prox = App::scene->sceneObjects->getObjectFromHandle(targetObj);
            if ((prox == nullptr) && (currentScript->getSceneObjectOrDetachedScriptHandle() <= sim_object_sceneobjectend))
                prox = App::scene->sceneObjects->getScriptFromHandle(currentScript->getSceneObjectOrDetachedScriptHandle());
//                prox = App::scene->getDetachedScriptFromHandle(targetObj);
        }
        it = App::scene->sceneObjects->getObjectFromPath(prox, path.c_str(), index);

        if (it != nullptr)
            outStack->pushHandleOntoStack(it->getObjectHandle());
        else
        {
            // Check if we maybe have an object with such a persistent UID:
            if (index == -1)
            {
                it = App::scene->sceneObjects->getObjectFromPersistentUid(origPath.c_str());
                if (it != nullptr)
                {
                    if (prox != nullptr)
                    { // we search from a proxy
                        if (!it->hasAncestor(prox))
                            it = nullptr;
                    }
                    if (it != nullptr)
                        outStack->pushHandleOntoStack(it->getObjectHandle());
                }
            }
        }
        if (it == nullptr)
        {
            if (noError)
                outStack->pushHandleOntoStack(-1);
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

std::string _method_announceChange(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string | arg_optional}))
    {
        std::string changeName = fetchText(inStack, 0);
        App::scene->undoBufferContainer->announceChange();
    }
    return errMsg;
}

std::string _method_getObjectFromUid(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_integer, arg_map | arg_optional}))
    {
        int64_t uid = fetchInt64(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            CInterfaceStackObject* obj = map->getMapObject("noError");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_bool))
                noError = ((CInterfaceStackBool*)obj)->getValue();
        });
        CSceneObject* it = App::scene->sceneObjects->getObjectFromUid(uid);
        if (it != nullptr)
            outStack->pushHandleOntoStack(it->getObjectHandle());
        else
        {
            if (noError)
                outStack->pushHandleOntoStack(-1);
            else
                errMsg = SIM_ERROR_OBJECT_INEXISTANT;
        }
    }
    return errMsg;
}

std::string _method_getInertia(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        C3X3Matrix m(shape->getMesh()->getInertia());
        m *= shape->getMesh()->getMass();
        outStack->pushMatrixOntoStack(m);
        outStack->pushVector3OntoStack(shape->getMesh()->getCOM());
    }
    return errMsg;
}

std::string _method_setInertia(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, 3, arg_pose}))
    {
        CMatrix _m = fetchMatrix(inStack, 0);
        C3X3Matrix m;
        m.setData(_m.data.data());
        m.axis[0](1) = m.axis[1](0);
        m.axis[0](2) = m.axis[2](0);
        m.axis[1](2) = m.axis[2](1);
        m /= shape->getMesh()->getMass(); // in CoppeliaSim we work with the "massless inertia"
        CPose tr = fetchPose(inStack, 1);

        shape->getMesh()->setCOM(tr.X);
        m = CMeshWrapper::getInertiaInNewFrame(tr.Q, m, CQuaternion::identityRotation);
        shape->getMesh()->setInertia(m);
        shape->setDynamicsResetFlag(true, false);
    }
    return errMsg;
}

std::string _method_computeInertia(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {arg_double}))
    {
        double density = fetchDouble(inStack, 0);
        shape->computeMassAndInertia(density);
    }
    return errMsg;
}

std::string _method_addForce(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {arg_vector3, arg_map | arg_optional}))
    {
        C3Vector force = fetchVector3(inStack, 0);
        C3Vector pos;
        pos.clear();
        bool reset = false;
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("reset", reset, &err);
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleArrayFromKey("offset", pos.data, 3, &err);
        });
        C3Vector t(pos ^ force);
        // force & t are relative to the shape's frame now
        if (relative)
        {
            CQuaternion q(shape->getCumulativeTransformation().Q);
            force = q * force;
            t = q * t;
        }
        if (reset)
            shape->clearAdditionalForce();
        shape->addAdditionalForceAndTorque(force, t);
    }
    return errMsg;
}

std::string _method_addTorque(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {arg_vector3, arg_map | arg_optional}))
    {
        C3Vector torque = fetchVector3(inStack, 0);
        bool reset = false;
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("reset", reset, &err);
            map->fetchBoolFromKey("relative", relative, &err);
        });
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

std::string _method_ungroup(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        std::vector<int> sel;
        sel.push_back(shape->getObjectHandle());
        if (!shape->getMesh()->isMesh())
            CSceneObjectOperations::ungroupSelection(&sel, true);
        if (sel.size() <= 1)
            sel.clear();
        outStack->pushHandleArrayOntoStack(sel.data(), sel.size());
    }
    return errMsg;
}

std::string _method_divide(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* shape = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if ((shape != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        std::vector<int> sel;
        sel.push_back(shape->getObjectHandle());
        if (shape->getMesh()->isMesh())
        {
            CSceneObjectOperations::divideSelection(&sel);
            if (sel.size() <= 1)
                sel.clear();
            outStack->pushHandleArrayOntoStack(sel.data(), sel.size());
        }
        else
            errMsg = SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE;
    }
    return errMsg;
}

std::string _method_groupShapes(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_handlearray}))
    {
        std::vector<int64_t> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        std::vector<int> shapeHandles;
        for (size_t i = 0; i < objectHandles.size(); i++)
        {
            CShape* it = App::scene->sceneObjects->getShapeFromHandle(objectHandles[i]);
            if (it != nullptr)
                shapeHandles.push_back(objectHandles[i]);
        }
        if ((shapeHandles.size() > 1) && (shapeHandles.size() == objectHandles.size()))
        {
            int h = CSceneObjectOperations::groupSelection(&shapeHandles);
            outStack->pushHandleOntoStack(h);
        }
        else
            errMsg = "invalid objects, or not enough shapes.";
    }
    return errMsg;
}

std::string _method_mergeShapes(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_handlearray}))
    {
        std::vector<int64_t> objectHandles;
        fetchHandleArray(inStack, 0, objectHandles);
        std::vector<int> shapeHandles;
        for (size_t i = 0; i < objectHandles.size(); i++)
        {
            CShape* it = App::scene->sceneObjects->getShapeFromHandle(objectHandles[i]);
            if (it != nullptr)
                shapeHandles.push_back(objectHandles[i]);
        }
        if ((shapeHandles.size() > 1) && (shapeHandles.size() == objectHandles.size()))
        {
            int h = CSceneObjectOperations::mergeSelection(&shapeHandles);
            outStack->pushHandleOntoStack(h);
        }
        else
            errMsg = "invalid objects, or not enough shapes.";
    }
    return errMsg;
}
/*
std::string _method_packTable(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // use pack instead
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_any}))
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
            outStack->pushBufferOntoStack(s.c_str(), s.size());
        }
        else
            outStack->pushBufferOntoStack("", 0);
    }
    return errMsg;
}

std::string _method_unpackTable(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{ // use unpack instead
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
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
                    outStack->pushObjectOntoStack(table);
                }
                else
                    errMsg = SIM_ERROR_INVALID_DATA;
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
        else
            outStack->pushInt32ArrayOntoStack(nullptr, 0); // empty buffer results in an empty table
    }
    return errMsg;
}
*/
std::string _method_pack(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_any}))
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
        outStack->pushBufferOntoStack(s.c_str(), s.size());
    }
    return errMsg;
}

std::string _method_unpack(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
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
                    outStack->pushObjectOntoStack(table->getArrayItemAtIndex(0));
                }
                else
                    errMsg = SIM_ERROR_INVALID_DATA;
            }
            else
                errMsg = SIM_ERROR_INVALID_DATA;
        }
        else
            outStack->pushInt32ArrayOntoStack(nullptr, 0); // empty buffer results in an empty table
    }
    return errMsg;
}

std::string _method_packArray(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_table, -1, arg_any, arg_map | arg_optional}))
    {
        std::string theType = "double";
        int startIndex = 0;
        int count = 0;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("type", theType, &err);
            map->fetchInt32FromKey("start", startIndex, &err);
            map->fetchInt32FromKey("count", count, &err);
        });
        if (errMsg.empty())
        {
            if (theType == "double")
            {
                std::vector<double> arr;
                fetchDoubleArray(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "float")
            {
                std::vector<double> arr;
                fetchDoubleArray(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "int64")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "int32")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "uint32")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "int16")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "uint16")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "int8")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else if (theType == "uint8")
            {
                std::vector<int64_t> arr;
                fetchInt64Array(inStack, 0, arr);
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
                outStack->pushBufferOntoStack(data.data(), data.size());
            }
            else
                errMsg = "unsupported type.";
        }
    }
    return errMsg;
}

std::string _method_unpackArray(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string dat = fetchBuffer(inStack, 0);
        std::string theType = "double";
        int startIndex = 0;
        int count = 0;
        int additionalCharOffset = 0;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("type", theType, &err);
            map->fetchInt32FromKey("start", startIndex, &err);
            map->fetchInt32FromKey("count", count, &err);
            map->fetchInt32FromKey("byteOffset", additionalCharOffset, &err);
        });
        if (errMsg.empty())
        {
            if (theType == "double")
            {
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
                outStack->pushDoubleArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "float")
            {
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
                outStack->pushFloatArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "int64")
            {
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
                outStack->pushInt64ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "int32")
            {
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
                outStack->pushInt32ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "uint32")
            {
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
                outStack->pushInt64ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "int16")
            {
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
                outStack->pushInt32ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "uint16")
            {
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
                outStack->pushInt32ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "int8")
            {
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
                outStack->pushInt32ArrayOntoStack(outData.data(), outData.size());
            }
            else if (theType == "uint8")
            {
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
                outStack->pushInt32ArrayOntoStack(outData.data(), outData.size());
            }
            else
                errMsg = "unsupported type.";
        }
    }
    return errMsg;
}

std::string _method_createCamera(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_map}))
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
            App::scene->sceneObjects->addObjectToScene(it, false, true);
            outStack->pushHandleOntoStack(it->getObjectHandle());
        }
    }
    return errMsg;
}

std::string _method_createLight(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_map}))
    {
        CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
        std::string t = "omnidirectional";
        map->fetchStringFromKey("light.type", t, &errMsg);
        if (errMsg.size() == 0)
        {
            auto value = magic_enum::enum_cast<lightType>(t);
            if (value.has_value())
            {
                int lightType = static_cast<int>(*value);
                CLight* it = new CLight(lightType);
                App::scene->sceneObjects->addObjectToScene(it, false, true);
                outStack->pushHandleOntoStack(it->getObjectHandle());
            }
            else
                errMsg = "invalid light type.";
        }
    }
    return errMsg;
}

std::string _method_createGraph(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_map}))
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
            App::scene->sceneObjects->addObjectToScene(it, false, true);
            CScript* script = new CScript(sim_scripttype_customization, "graph = require('models.graph_customization-2')", 0, "lua");
            script->setScriptExecPriority_raw(sim_scriptexecorder_last);
            App::scene->sceneObjects->addObjectToScene(script, false, true);
            App::scene->sceneObjects->setObjectParent(script, it, true);
            it->setObjectProperty(it->getObjectProperty() | sim_objectproperty_collapsed);
            it->setModelBase(true);
            outStack->pushHandleOntoStack(it->getObjectHandle());
        }
    }
    return errMsg;
}

std::string _method_createCustomSceneObject(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_optional | arg_map}))
    {
        CCustomSceneObject* it = new CCustomSceneObject();
        App::scene->sceneObjects->addObjectToScene(it, false, true);
        outStack->pushHandleOntoStack(it->getObjectHandle());
    }
    return errMsg;
}

std::string _method_getBoolProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int pValue;
            if (CALL_C_API(simGetBoolProperty, targetObj, pName.c_str(), &pValue) > 0)
                outStack->pushBoolOntoStack(pValue != 0);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getBufferProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            char* pValue;
            int pLength;
            if (CALL_C_API(simGetBufferProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                outStack->pushBufferOntoStack(pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getColorProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            float pValue[4];
            if (CALL_C_API(simGetColor4Property, targetObj, pName.c_str(), pValue) > 0)
                outStack->pushColorOntoStack(pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getFloatArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double* pValue;
            int pLength;
            if (CALL_C_API(simGetFloatArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                outStack->pushDoubleArrayOntoStack(pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getFloatProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double pValue;
            if (CALL_C_API(simGetFloatProperty, targetObj, pName.c_str(), &pValue) > 0)
                outStack->pushDoubleOntoStack(pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getStringArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
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
                outStack->pushTextArrayOntoStack(vv.data(), vv.size());
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getHandleArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int64_t* pValue = nullptr;
            int pLength;
            if (CALL_C_API(simGetHandleArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                outStack->pushHandleArrayOntoStack(pValue, pLength);
//                outStack->pushInt64ArrayOntoStack(pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getHandleProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int64_t pValue;
            if (CALL_C_API(simGetHandleProperty, targetObj, pName.c_str(), &pValue) > 0)
                outStack->pushHandleOntoStack(pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntArray2Property(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int pValue[2];
            if (CALL_C_API(simGetIntArray2Property, targetObj, pName.c_str(), pValue) > 0)
                outStack->pushInt32ArrayOntoStack(pValue, 2);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int* pValue;
            int pLength;
            if (CALL_C_API(simGetIntArrayProperty, targetObj, pName.c_str(), &pValue, &pLength) > 0)
            {
                outStack->pushInt32ArrayOntoStack(pValue, pLength);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getIntProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int pValue;
            if (CALL_C_API(simGetIntProperty, targetObj, pName.c_str(), &pValue) > 0)
                outStack->pushInt32OntoStack(pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getLongProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int64_t pValue;
            if (CALL_C_API(simGetLongProperty, targetObj, pName.c_str(), &pValue) > 0)
                outStack->pushInt64OntoStack(pValue);
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getPoseProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double pValue[7];
            if (CALL_C_API(simGetPoseProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                CPose p;
                p.setData(pValue, true);
                outStack->pushPoseOntoStack(p);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getQuaternionProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double pValue[4];
            if (CALL_C_API(simGetQuaternionProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                CQuaternion q;
                q.setData(pValue, true);
                outStack->pushQuaternionOntoStack(q);
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getStringProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            char* pValue = nullptr;
            int res = CALL_C_API(simGetStringProperty, targetObj, pName.c_str(), &pValue);
            if (res > 0)
            {
                outStack->pushTextOntoStack(pValue);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getVector3Property(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double pValue[3];
            if (CALL_C_API(simGetVector3Property, targetObj, pName.c_str(), pValue) > 0)
                outStack->pushVector3OntoStack(C3Vector(pValue));
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setBoolProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_bool, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool pValue = fetchBool(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetBoolProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setBufferProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchBuffer(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetBufferProperty, targetObj, pName.c_str(), pValue.c_str(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setColorProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_color, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        float pValue[4];
        fetchColor(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetColor4Property, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setFloatArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, -1, arg_double, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<double> pValue;
        fetchDoubleArray(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetFloatArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setFloatProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_double, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        double pValue = fetchDouble(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetFloatProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setStringArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, -1, arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<std::string> pValue;
        fetchTextArray(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
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
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setHandleArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_handlearray, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<int64_t> pValue;
        fetchHandleArray(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetHandleArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setHandleProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_handle, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        int64_t pValue = fetchHandle(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetHandleProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setIntArray2Property(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, -1, arg_integer, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<int> pValue;
        fetchIntArray(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntArray2Property, targetObj, pName.c_str(), pValue.data()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setIntArrayProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_table, -1, arg_integer, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::vector<int> pValue;
        fetchIntArray(inStack, 1, pValue);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntArrayProperty, targetObj, pName.c_str(), pValue.data(), pValue.size()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setIntProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_integer, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        int pValue = fetchInt(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetIntProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setLongProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_integer, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        int64_t pValue = fetchInt64(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetLongProperty, targetObj, pName.c_str(), pValue) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setPoseProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_pose, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        CPose pState = fetchPose(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double p[7];
            pState.getData(p, true);
            if (CALL_C_API(simSetPoseProperty, targetObj, pName.c_str(), p) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setQuaternionProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_quaternion, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        CQuaternion pState = fetchQuaternion(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double q[4];
            pState.getData(q, true);
            if (CALL_C_API(simSetQuaternionProperty, targetObj, pName.c_str(), q) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setStringProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchText(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetStringProperty, targetObj, pName.c_str(), pValue.c_str()) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_setVector3Property(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_vector3, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        C3Vector pValue = fetchVector3(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetVector3Property, targetObj, pName.c_str(), pValue.data) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_getMatrixProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            double* pValue;
            int r, c;
            if (CALL_C_API(simGetMatrixProperty, targetObj, pName.c_str(), &pValue, &r, &c) > 0)
            {
                CMatrix m(r, c);
                m.data.assign(pValue, pValue + r * c);
                outStack->pushMatrixOntoStack(m);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setMatrixProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_matrix, -1, -1, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        CMatrix pValue = fetchMatrix(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetMatrixProperty, targetObj, pName.c_str(), pValue.data.data(), pValue.rows, pValue.cols) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_getMethodProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            std::string byteCode;
            int res = App::getMethodProperty_t(targetObj, pName.c_str(), byteCode);
            if (res == sim_propertyret_ok)
                outStack->pushBufferOntoStack(byteCode.data(), byteCode.size());
            else
            {
                if (!noError)
                {
                    if (res == sim_propertyret_unknowntarget)
                        errMsg = SIM_ERROR_TARGET_DOES_NOT_EXIST;
                    else
                    {
                        std::string err("'");
                        err += pName + "' ";
                        int info;
                        std::string infoTxt;
                        int p = App::getPropertyInfo_t(targetObj, pName.c_str(), info, infoTxt);
                        if (p < sim_propertytype_start)
                            errMsg = err + SIM_ERROR_UNKNOWN_PROPERTY;
                        else if ((p & 0xff) == sim_propertytype_method)
                            errMsg = err + SIM_ERROR_PROPERTY_CANNOT_BE_READ;
                        else
                           errMsg = err + SIM_ERROR_PROPERTY_TYPE_MISMATCH;
                    }
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setMethodProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchBuffer(inStack, 1); // can be nil, in which case we register a dummy function
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int res = App::setMethodProperty_t(targetObj, pName.c_str(), pValue);
            if ((res != sim_propertyret_ok) && (!noError))
            {
                if (res == sim_propertyret_unknowntarget)
                    errMsg = SIM_ERROR_TARGET_DOES_NOT_EXIST;
                else
                {
                    std::string err("'");
                    err += pName + "' ";
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo_t(targetObj, pName.c_str(), info, infoTxt);
                    if (p < sim_propertytype_start)
                        errMsg = err + SIM_ERROR_UNKNOWN_PROPERTY;
                    else if ((p & 0xff) == sim_propertytype_method)
                        errMsg = err + SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN;
                    else
                       errMsg = err + SIM_ERROR_PROPERTY_TYPE_MISMATCH;
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getTableProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            int pValueL;
            char* pValue = nullptr;
            int res = CALL_C_API(simGetTableProperty, targetObj, pName.c_str(), &pValue, &pValueL);
            if (res > 0)
            {
                outStack->pushBufferOntoStack(pValue, pValueL);
                delete[] pValue;
            }
            else
            {
                errMsg = CApiErrors::getAndClearLastError();
                if (noError)
                {
                    outStack->pushNullOntoStack();
                    errMsg.clear();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setTableProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        std::string pValue = fetchBuffer(inStack, 1);
        bool noError = false;
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simSetTableProperty, targetObj, pName.c_str(), pValue.c_str(), int(pValue.size())) > 0)
            {
                if ((currentScript != nullptr) && utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
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

std::string _method_removeProperty(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.size() == 0)
        {
            if (CALL_C_API(simRemoveProperty, targetObj, pName.c_str()) == sim_propertyret_ok)
            {
                if (utils::startsWith(pName.c_str(), SIGNALPREFIXDOT))
                {
                    std::string nn(pName);
                    if (targetObj == sim_handle_app)
                        nn = "app." + nn;
                    else if (targetObj != sim_handle_scene)
                        nn = "obj." + nn;
                    currentScript->signalRemoved(nn.c_str());
                }
            }
            else
                errMsg = CApiErrors::getAndClearLastError();
        }
    }
    return errMsg;
}

std::string _method_getPropertyName(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_integer, arg_optional | arg_map}))
    {
        int index = fetchInt(inStack, 0);
        SPropertyOptions opt;
        std::string propertyPrefix;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("prefix", propertyPrefix, &err);
            map->fetchInt32FromKey("excludeFlags", opt.excludeFlags, &err);
        });
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
                outStack->pushTextOntoStack(w2.c_str());
                outStack->pushTextOntoStack(w1.c_str());
            }
            else
            {
                if (App::isTargetValid_t(targetObj))
                {
                    outStack->pushNullOntoStack();
                    outStack->pushNullOntoStack();
                }
                else
                    errMsg = SIM_ERROR_TARGET_DOES_NOT_EXIST;
            }
        }
    }
    return errMsg;
}

std::string _method_getPropertyInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string pName = fetchText(inStack, 0);
        bool noError = false;
        SPropertyOptions opt;
        opt.bitCoded = 1;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("noError", noError, &err);
        });
        if (errMsg.empty())
        {
            SPropertyInfo infos;
            int res = CALL_C_API(simGetPropertyInfo, targetObj, pName.c_str(), &infos, &opt);
            if (res == sim_propertyret_ok)
            {
                outStack->pushInt32OntoStack(infos.type);
                outStack->pushInt32OntoStack(infos.flags);
                if (infos.infoTxt == nullptr)
                    outStack->pushTextOntoStack("");
                else
                {
                    outStack->pushTextOntoStack(infos.infoTxt);
                    delete[] infos.infoTxt;
                }
            }
            else
            {
                if (!noError)
                    errMsg = CApiErrors::getAndClearLastError();
                else
                {
                    outStack->pushNullOntoStack();
                    outStack->pushNullOntoStack();
                    outStack->pushNullOntoStack();
                }
            }
        }
    }
    return errMsg;
}

std::string _method_setPropertyInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_integer, arg_string}))
    {
        std::string pName = fetchText(inStack, 0);
        SPropertyInfo infos;
        infos.flags = fetchInt(inStack, 1);
        std::string infoTxt = fetchText(inStack, 2);
        infos.infoTxt = (char*)infoTxt.c_str();
        int res = CALL_C_API(simSetPropertyInfo, targetObj, pName.c_str(), &infos);
        if (res != sim_propertyret_ok)
            errMsg = CApiErrors::getAndClearLastError();
    }
    return errMsg;
}

std::string _method_createCustomObjectClass(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_optional | arg_map}))
    {
        std::string typeStr = fetchText(inStack, 0);
        bool hasSuperClassInfo = false;
        std::vector<std::string> superClass = {};
        std::vector<std::string> nameSpaces = {};
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            CInterfaceStackObject* obj = map->getMapObject("metaInfo");
            if (obj->getObjectType() == sim_stackitem_table)
            {
                CInterfaceStackTable* mInfo = (CInterfaceStackTable*)obj;
                hasSuperClassInfo = mInfo->fetchStringArrayFromKey("superClass", superClass, &err);
                mInfo->fetchStringArrayFromKey("namespaces", nameSpaces, &err);
            }
            else
                errMsg = "invalid 'metaInfo' field.";
        });
        if (errMsg.size() == 0)
        {
            if (!hasSuperClassInfo)
                superClass.push_back("object");
            int64_t retVal = App::scenes->customObjects->makeClass(typeStr.c_str(), superClass, nameSpaces);
            if (retVal >= 0)
                outStack->pushHandleOntoStack(retVal);
            else
                errMsg = "class already defined, or invalid metaInfo.";
        }
    }
    return errMsg;
}

std::string _method_isValid(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    outStack->pushBoolOntoStack(App::isTargetValid_t(targetObj));
    return errMsg;
}

std::string _method_addCurve(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_table, 2, arg_integer, arg_optional | arg_map}))
    {
        std::vector<int> streamIds;
        fetchIntArray(inStack, 0, streamIds);
        float color[3] = {0.0f, 1.0f, 0.0f};
        std::string name("curve");
        std::string unitStr("");
        double defaultVals[2] = {0.0, 0.0};
        int width = 2;
        bool hideLabel = false;
        bool drawLine = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchFloatArrayFromKey("color", color, 3, &err);
            map->fetchStringFromKey("name", name, &err);
            map->fetchStringFromKey("unit", unitStr, &err);
            map->fetchDoubleArrayFromKey("default", defaultVals, 2, &err);
            map->fetchInt32FromKey("width", width, &err);
            map->fetchBoolFromKey("hideLabel", hideLabel, &err);
            map->fetchBoolFromKey("drawLine", drawLine, &err);
        });
        if (errMsg.size() == 0)
        {
            if (name.size() == 0)
                name = "_";
            tt::removeIllegalCharacters(name, false);
            int h = -1;
            if (currentScript != nullptr)
                h = currentScript->getSceneObjectOrDetachedScriptHandle();
            int options = 0;
            if (hideLabel)
                options += 2;
            if (!drawLine)
                options += 4;
            CGraphCurve* curve = new CGraphCurve(2, streamIds.data(), defaultVals, name.c_str(), unitStr.c_str(), options, color, width, h);
            int retVal = target->addOrUpdateCurve(curve);
            if (retVal == -1)
            {
                delete curve;
                errMsg = SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE;
            }
            else
                outStack->pushInt32OntoStack(retVal);
        }
    }
    return errMsg;
}

std::string _method_addSignal(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_optional | arg_map}))
    {
        float color[3] = {1.0f, 0.0f, 0.0f};
        std::string name("signal");
        std::string unitStr("");
        bool hideSignal = false;
        bool hideLabel = false;
        bool drawLine = false;
        double cyclicRange = 0.0;
        std::string trType("raw");
        double scale = 1.0;
        double offset = 0.0;
        int smoothing = 1;
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchFloatArrayFromKey("color", color, 3, &err);
            map->fetchStringFromKey("name", name, &err);
            map->fetchStringFromKey("unit", unitStr, &err);
            map->fetchBoolFromKey("hideSignal", hideSignal, &err);
            map->fetchBoolFromKey("hideLabel", hideLabel, &err);
            map->fetchBoolFromKey("drawLine", drawLine, &err);
            map->fetchDoubleFromKey("cyclicRange", cyclicRange, &err);
            CInterfaceStackObject* obj = map->getMapObject("transformation");
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_table))
            {
                CInterfaceStackTable* transf = (CInterfaceStackTable*)obj;
                transf->fetchStringFromKey("type", trType, &err);
                transf->fetchDoubleFromKey("scale", scale, &err);
                transf->fetchDoubleFromKey("offset", offset, &err);
                transf->fetchInt32FromKey("smoothing", smoothing, &err);
            }
        });
        if (errMsg.size() == 0)
        {
            if (name.size() == 0)
                name = "_";
            tt::removeIllegalCharacters(name, false);
            int h = -1;
            if (currentScript != nullptr)
                h = currentScript->getSceneObjectOrDetachedScriptHandle();
            int options = 0;
            if (hideSignal)
                options += 1;
            if (hideLabel)
                options += 2;
            if (!drawLine)
                options += 4;
            CGraphDataStream* str = new CGraphDataStream(name.c_str(), unitStr.c_str(), options, color, cyclicRange, h);
            int retVal = target->addOrUpdateDataStream(str);
            if (retVal == -1)
            {
                delete str;
                errMsg = SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE;
            }
            else
            {
                int t = sim_stream_transf_raw;
                if (trType == "derivative")
                    t = sim_stream_transf_derivative;
                else if (trType == "integral")
                    t = sim_stream_transf_integral;
                else if (trType == "cumulative")
                    t = sim_stream_transf_cumulative;
                target->setDataStreamTransformation(retVal, t, scale, offset, smoothing);
                outStack->pushInt32OntoStack(retVal);
            }
        }
    }
    return errMsg;
}

std::string _method_reset(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {}))
    {
        target->resetGraph();
    }
    return errMsg;
}

std::string _method_setSignalPoint(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer, arg_double}))
    {
        int signal = fetchInt(inStack, 0);
        double v = fetchDouble(inStack, 1);
        if (!target->setNextValueToInsert(signal, v))
            errMsg = SIM_ERROR_INVALID_SIGNAL_ID;
    }
    return errMsg;
}

std::string _method_removeTrace(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer}))
    {
        int trace = fetchInt(inStack, 0);
        if (trace == -1)
        {
            target->removeAllStreamsAndCurves();
            target->removeAllStreamsAndCurves_old();
        }
        else
        {
            if (!target->removeGraphCurve(trace))
            {
                if (!target->removeGraphDataStream(trace))
                    errMsg = SIM_ERROR_INVALID_TRACE_ID;
            }
        }
    }
    return errMsg;
}

std::string _method_snapshotTrace(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer, arg_optional | arg_map}))
    {
        int trace = fetchInt(inStack, 0);
        float color[3] = {1.0f, 1.0f, 1.0f};
        std::string name("snapshot");
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchFloatArrayFromKey("color", color, 3, &err);
            map->fetchStringFromKey("name", name, &err);
        });
        if (errMsg.size() == 0)
        {
            if (name.size() == 0)
                name = "_";
            tt::removeIllegalCharacters(name, false);
            int retVal = target->duplicateCurveToStatic(trace, name.c_str(), color);
            if (retVal != -1)
                outStack->pushInt32OntoStack(retVal);
            else
                errMsg = SIM_ERROR_INVALID_TRACE_ID;
        }
    }
    return errMsg;
}

std::string _method_step(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CGraph* target = (CGraph*)getSpecificSceneObjectType(targetObj, sim_sceneobject_graph, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_double}))
    {
        double t = fetchDouble(inStack, 0);
        target->addNextPoint(t);
    }
    return errMsg;
}

std::string _method_makeClass(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string}))
    {
        std::string className = fetchText(inStack, 0);
        if (!className.empty())
        {
            int retVal = App::scenes->customSceneObjectClasses->makeClass(target, className.c_str());
            if (retVal >= 0)
                outStack->pushHandleOntoStack(retVal);
            else
                errMsg = "class exists already.";
        }
        else
            errMsg = "invalid class name.";
    }
    return errMsg;
}

std::string _method_makeObject(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* customSceneObjectClass = App::scenes->customSceneObjectClasses->getClass(targetObj);
    CustomObject* customObjectClass = App::scenes->customObjects->getClass(targetObj);
    if (customSceneObjectClass != nullptr)
    {
        if (checkInputArguments(inStack, &errMsg, {}))
        {
            int retVal = App::scenes->customSceneObjectClasses->makeObject(targetObj);
            if (retVal >= 0)
                outStack->pushHandleOntoStack(retVal);
            else
                errMsg = "class does not exist.";
        }
    }
    else if (customObjectClass != nullptr)
    {
        if (checkInputArguments(inStack, &errMsg, {arg_optional | arg_map}))
        {
            bool appScope = true;
            bool scriptPersistent = false;
            bool isVolatile = true;
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchBoolFromKey("appScope", appScope, &err);
                map->fetchBoolFromKey("scriptPersistent", scriptPersistent, &err);
                map->fetchBoolFromKey("volatile", isVolatile, &err);
            });
            if (errMsg.size() == 0)
            {
                int h = -1;
                if ((currentScript != nullptr) && (!scriptPersistent) && isVolatile)
                    h = currentScript->getObjectHandle();
                int64_t retVal = -1;
                if (appScope)
                    retVal = App::scenes->customObjects->makeObject(customObjectClass, isVolatile, h);
                else
                    retVal = App::scene->customObjects->makeObject(customObjectClass, isVolatile, h);
                outStack->pushInt64OntoStack(retVal);
            }
        }
    }
    else
        errMsg = SIM_ERROR_INVALID_TARGET;
    return errMsg;
}

std::string _method_addFromObjects(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if (target != nullptr)
    {
        COcTree* ocTree = nullptr;
        CPointCloud* ptCloud = nullptr;
        if (target->getObjectType() == sim_sceneobject_octree)
            ocTree = (COcTree*)target;
        if (target->getObjectType() == sim_sceneobject_pointcloud)
            ptCloud = (CPointCloud*)target;
        if ((ocTree != nullptr) || (ptCloud != nullptr))
        {
            if (checkInputArguments(inStack, &errMsg, {arg_handlearray, arg_map | arg_optional}))
            {
                std::vector<int> objects;
                fetchHandleArray(inStack, 0, objects);
                if (ocTree != nullptr)
                {
                    float color[3];
                    bool hasColor = false;
                    int tag = 0;
                    withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                    {
                        hasColor = map->fetchFloatArrayFromKey("color", color, 3, &err);
                        map->fetchInt32FromKey("tag", tag, &err);
                    });
                    if (errMsg.size() == 0)
                    {
                        float savedCols[3];
                        ocTree->getColor()->getColor(savedCols, sim_materialcomponent_diffuse);
                        float* cptr = ocTree->getColor()->getColorsPtr();
                        if (hasColor)
                        {
                            cptr[0] = color[0];
                            cptr[1] = color[1];
                            cptr[2] = color[2];
                        }
                        for (size_t i = 0; i < objects.size(); i++)
                        {
                            CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(objects[i]);
                            if (obj != nullptr)
                                ocTree->insertObject(obj, tag);
                        }
                        cptr[0] = savedCols[0];
                        cptr[1] = savedCols[1];
                        cptr[2] = savedCols[2];
                    }
                }
                if (ptCloud != nullptr)
                {
                    float color[3];
                    bool hasColor = false;
                    double gridSize = 0.02;
                    double duplicateTolerance;
                    bool hasDuplicateTolerance = false;
                    withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                    {
                        hasColor = map->fetchFloatArrayFromKey("color", color, 3, &err);
                        map->fetchDoubleFromKey("gridSize", gridSize, &err);
                        hasDuplicateTolerance = map->fetchDoubleFromKey("duplicateTolerance", duplicateTolerance, &err);
                    });
                    if (errMsg.size() == 0)
                    {
                        double savedGridSize = ptCloud->getBuildResolution();
                        ptCloud->setBuildResolution(gridSize);
                        float savedCols[3];
                        ptCloud->getColor()->getColor(savedCols, sim_materialcomponent_diffuse);
                        if (hasColor)
                        {
                            ptCloud->getColor()->getColorsPtr()[0] = color[0];
                            ptCloud->getColor()->getColorsPtr()[1] = color[1];
                            ptCloud->getColor()->getColorsPtr()[2] = color[2];
                        }
                        double insertionToleranceSaved = ptCloud->getInsertionDistanceTolerance();
                        if (hasDuplicateTolerance)
                            ptCloud->setInsertionDistanceTolerance(duplicateTolerance);

                        for (size_t i = 0; i < objects.size(); i++)
                        {
                            CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(objects[i]);
                            if (obj != nullptr)
                                ptCloud->insertObject(obj);
                        }

                        ptCloud->setInsertionDistanceTolerance(insertionToleranceSaved);
                        ptCloud->setBuildResolution(savedGridSize);
                        ptCloud->getColor()->getColorsPtr()[0] = savedCols[0];
                        ptCloud->getColor()->getColorsPtr()[1] = savedCols[1];
                        ptCloud->getColor()->getColorsPtr()[2] = savedCols[2];
                    }
                }
            }
        }
        else
            errMsg = "method not available for that target.";
    }
    return errMsg;
}

std::string _method_subtractFromObjects(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if (target != nullptr)
    {
        COcTree* ocTree = nullptr;
        CPointCloud* ptCloud = nullptr;
        if (target->getObjectType() == sim_sceneobject_octree)
            ocTree = (COcTree*)target;
        if (target->getObjectType() == sim_sceneobject_pointcloud)
            ptCloud = (CPointCloud*)target;
        if ((ocTree != nullptr) || (ptCloud != nullptr))
        {
            if (checkInputArguments(inStack, &errMsg, {arg_handlearray, arg_map | arg_optional}))
            {
                std::vector<int> objects;
                fetchHandleArray(inStack, 0, objects);
                if (ocTree != nullptr)
                {
                    for (size_t i = 0; i < objects.size(); i++)
                    {
                        CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(objects[i]);
                        if (obj != nullptr)
                            ocTree->subtractObject(obj);
                    }
                }
                if (ptCloud != nullptr)
                {
                    double tolerance = 0.02;
                    withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
                    {
                        map->fetchDoubleFromKey("tolerance", tolerance, &err);
                    });
                    if (errMsg.size() == 0)
                    {
                        for (size_t i = 0; i < objects.size(); i++)
                        {
                            CSceneObject* obj = App::scene->sceneObjects->getObjectFromHandle(objects[i]);
                            if (obj != nullptr)
                                ptCloud->subtractObject(obj, tolerance);
                        }
                    }
                }
            }
        }
        else
            errMsg = "method not available for that target.";
    }
    return errMsg;
}

std::string _method_clear(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CSceneObject* target = getSceneObject(targetObj, &errMsg, -1);
    if (target != nullptr)
    {
        COcTree* ocTree = nullptr;
        CPointCloud* ptCloud = nullptr;
        if (target->getObjectType() == sim_sceneobject_octree)
            ocTree = (COcTree*)target;
        if (target->getObjectType() == sim_sceneobject_pointcloud)
            ptCloud = (CPointCloud*)target;
        if ((ocTree != nullptr) || (ptCloud != nullptr))
        {
            if (checkInputArguments(inStack, &errMsg, {}))
            {
                if (ocTree != nullptr)
                    ocTree->clear();
                if (ptCloud != nullptr)
                    ptCloud->clear();
            }
        }
        else
            errMsg = "method not available for that target.";
    }
    return errMsg;
}

std::string _method_addVoxels(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        std::vector<float> colors;
        std::vector<unsigned int> tags;
        tags.resize(pts.size() / 3);
        bool hasColors = false;
        bool hasTags = false;
        float color[4];
        int tag = 0;
        bool hasColor = false;
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            hasColors = map->fetchMatrixDataFromKey("colors", colors, 4, pts.size() / 3, false, &err);
            hasColor = map->fetchFloatArrayFromKey("color", color, 4, &err);
            hasTags = map->fetchUInt32ArrayFromKey("tags", tags.data(), tags.size(), &err);
            map->fetchInt32FromKey("tag", tag, &err);
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            std::vector<uint8_t> _cols;
            _cols.resize(4 * (pts.size() / 3));
            if (hasColors)
            {
                for (size_t i = 0; i < 4 * (pts.size() / 3); i++)
                    _cols[i] = (uint8_t)(colors[i] * 255.1f);
            }
            else
            {
                if (!hasColor)
                {
                    target->getColor()->getColor(color, sim_materialcomponent_diffuse);
                    color[3] = 255;
                    if (target->getColor()->getTranslucid())
                        color[3] = target->getColor()->getOpacity() * 255.1f;
                }
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    _cols[4 * i + 0] = (uint8_t)(color[0] * 255.1f);
                    _cols[4 * i + 1] = (uint8_t)(color[1] * 255.1f);
                    _cols[4 * i + 2] = (uint8_t)(color[2] * 255.1f);
                    _cols[4 * i + 3] = (uint8_t)(color[3] * 255.1f);
                }
            }
            if (!hasTags)
            {
                tags.resize(0);
                tags.resize(pts.size() / 3, tag);
            }
            target->insertPoints(pts.data(), int(pts.size()) / 3, relative, _cols.data(), true, tags.data(), 0);
        }
    }
    return errMsg;
}

std::string _method_addPackedVoxels(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        std::vector<float> colors;
        std::vector<unsigned int> tags;
        tags.resize(pts.size() / 3);
        bool hasColors = false;
        bool hasTags = false;
        float color[4];
        int tag = 0;
        bool hasColor = false;
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            buff.clear();
            hasColors = map->fetchStringFromKey("colors", buff, &err);
            if (hasColors)
            {
                l = buff.size() / sizeof(float);
                if ( (l / 4) != (pts.size() / 3))
                    errMsg = "invalid 'colors' field.";
                else
                {
                    colors.resize(l);
                    for (size_t i = 0; i < l; i++)
                        colors[i] = ((float*)buff.data())[i];
                }
            }
            buff.clear();
            hasTags = map->fetchStringFromKey("tags", buff, &err);
            if (hasTags)
            {
                l = buff.size() / sizeof(unsigned int);
                if ( l != pts.size() / 3)
                    errMsg = "invalid 'tags' field.";
                else
                {
                    tags.resize(l);
                    for (size_t i = 0; i < l; i++)
                        tags[i] = ((unsigned int*)buff.data())[i];
                }
            }
            hasColor = map->fetchFloatArrayFromKey("color", color, 4, &err);
            map->fetchInt32FromKey("tag", tag, &err);
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            std::vector<uint8_t> _cols;
            _cols.resize(4 * (pts.size() / 3));
            if (hasColors)
            {
                for (size_t i = 0; i < 4 * (pts.size() / 3); i++)
                    _cols[i] = (uint8_t)(colors[i] * 255.1f);
            }
            else
            {
                if (!hasColor)
                {
                    target->getColor()->getColor(color, sim_materialcomponent_diffuse);
                    color[3] = 255;
                    if (target->getColor()->getTranslucid())
                        color[3] = target->getColor()->getOpacity() * 255.1f;
                }
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    _cols[4 * i + 0] = (uint8_t)(color[0] * 255.1f);
                    _cols[4 * i + 1] = (uint8_t)(color[1] * 255.1f);
                    _cols[4 * i + 2] = (uint8_t)(color[2] * 255.1f);
                    _cols[4 * i + 3] = (uint8_t)(color[3] * 255.1f);
                }
            }
            if (!hasTags)
            {
                tags.resize(0);
                tags.resize(pts.size() / 3, tag);
            }
            target->insertPoints(pts.data(), int(pts.size()) / 3, relative, _cols.data(), true, tags.data(), 0);
        }
    }
    return errMsg;
}

std::string _method_subtractVoxels(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            target->subtractPoints(pts.data(), int(pts.size()) / 3, relative);
        }
    }
    return errMsg;
}

std::string _method_subtractPackedVoxels(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            target->subtractPoints(pts.data(), int(pts.size()) / 3, relative);
        }
    }
    return errMsg;
}

std::string _method_checkPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            unsigned int tag = 0;
            uint64_t location = 0;
            unsigned int locLow = location & 0xffffffff;
            unsigned int locHigh = (location >> 32) & 0xffffffff;

            if (relative)
            {
                CPose tr(target->getFullCumulativeTransformation());
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    C3Vector v(pts.data() + 3 * i);
                    v *= tr;
                    pts[3 * i + 0] = v(0);
                    pts[3 * i + 1] = v(1);
                    pts[3 * i + 2] = v(2);
                }
            }

            bool coll = false;
            if (pts.size() == 3)
                coll = App::scenes->pluginContainer->geomPlugin_getOctreePointCollision(target->getOctreeInfo(), target->getFullCumulativeTransformation(), C3Vector(pts.data()), &tag, &location);
            else
                coll = App::scenes->pluginContainer->geomPlugin_getOctreePointsCollision(target->getOctreeInfo(), target->getFullCumulativeTransformation(), pts.data(), pts.size() / 3);

            outStack->pushBoolOntoStack(coll);
            outStack->pushInt64OntoStack(tag);
            outStack->pushInt64OntoStack(locLow);
            outStack->pushInt64OntoStack(locHigh);
        }
    }
    return errMsg;
}

std::string _method_checkPackedPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    COcTree* target = (COcTree*)getSpecificSceneObjectType(targetObj, sim_sceneobject_octree, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        bool relative = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
        });
        if (errMsg.size() == 0)
        {
            unsigned int tag = 0;
            uint64_t location = 0;
            unsigned int locLow = location & 0xffffffff;
            unsigned int locHigh = (location >> 32) & 0xffffffff;

            if (relative)
            {
                CPose tr(target->getFullCumulativeTransformation());
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    C3Vector v(pts.data() + 3 * i);
                    v *= tr;
                    pts[3 * i + 0] = v(0);
                    pts[3 * i + 1] = v(1);
                    pts[3 * i + 2] = v(2);
                }
            }

            bool coll = false;
            if (pts.size() == 3)
                coll = App::scenes->pluginContainer->geomPlugin_getOctreePointCollision(target->getOctreeInfo(), target->getFullCumulativeTransformation(), C3Vector(pts.data()), &tag, &location);
            else
                coll = App::scenes->pluginContainer->geomPlugin_getOctreePointsCollision(target->getOctreeInfo(), target->getFullCumulativeTransformation(), pts.data(), pts.size() / 3);

            outStack->pushBoolOntoStack(coll);
            outStack->pushInt64OntoStack(tag);
            outStack->pushInt64OntoStack(locLow);
            outStack->pushInt64OntoStack(locHigh);
        }
    }
    return errMsg;
}

std::string _method_addPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        std::vector<float> colors;
        bool hasColors = false;
        float color[4];
        bool hasColor = false;
        bool relative = false;
        double tolerance = target->getInsertionDistanceTolerance();
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            hasColors = map->fetchMatrixDataFromKey("colors", colors, 4, pts.size() / 3, false, &err);
            hasColor = map->fetchFloatArrayFromKey("color", color, 4, &err);
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
        {
            std::vector<uint8_t> _cols;
            _cols.resize(4 * (pts.size() / 3));
            if (hasColors)
            {
                for (size_t i = 0; i < 4 * (pts.size() / 3); i++)
                    _cols[i] = (uint8_t)(colors[i] * 255.1f);
            }
            else
            {
                if (!hasColor)
                {
                    target->getColor()->getColor(color, sim_materialcomponent_diffuse);
                    color[3] = 255;
                    if (target->getColor()->getTranslucid())
                        color[3] = target->getColor()->getOpacity() * 255.1f;
                }
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    _cols[4 * i + 0] = (uint8_t)(color[0] * 255.1f);
                    _cols[4 * i + 1] = (uint8_t)(color[1] * 255.1f);
                    _cols[4 * i + 2] = (uint8_t)(color[2] * 255.1f);
                    _cols[4 * i + 3] = (uint8_t)(color[3] * 255.1f);
                }
            }
            double insertionToleranceSaved = target->getInsertionDistanceTolerance();
            target->setInsertionDistanceTolerance(tolerance);
            target->insertPoints(pts.data(), int(pts.size()) / 3, relative, _cols.data(), true);
            target->setInsertionDistanceTolerance(insertionToleranceSaved);
        }
    }
    return errMsg;
}

std::string _method_addPackedPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        std::vector<float> colors;
        bool hasColors = false;
        float color[4];
        bool hasColor = false;
        bool relative = false;
        double tolerance = target->getInsertionDistanceTolerance();
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            buff.clear();
            hasColors = map->fetchStringFromKey("colors", buff, &err);
            if (hasColors)
            {
                l = buff.size() / sizeof(float);
                if ( (l / 4) != (pts.size() / 3))
                    errMsg = "invalid 'colors' field.";
                else
                {
                    colors.resize(l);
                    for (size_t i = 0; i < l; i++)
                        colors[i] = ((float*)buff.data())[i];
                }
            }
            hasColor = map->fetchFloatArrayFromKey("color", color, 4, &err);
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
        {
            std::vector<unsigned char> _cols;
            _cols.resize(4 * (pts.size() / 3));
            if (hasColors)
            {
                for (size_t i = 0; i < 4 * (pts.size() / 3); i++)
                    _cols[i] = (uint8_t)(colors[i] * 255.1f);
            }
            else
            {
                if (!hasColor)
                {
                    target->getColor()->getColor(color, sim_materialcomponent_diffuse);
                    color[3] = 255;
                    if (target->getColor()->getTranslucid())
                        color[3] = target->getColor()->getOpacity() * 255.1f;
                }
                for (size_t i = 0; i < pts.size() / 3; i++)
                {
                    _cols[4 * i + 0] = (uint8_t)(color[0] * 255.1f);
                    _cols[4 * i + 1] = (uint8_t)(color[1] * 255.1f);
                    _cols[4 * i + 2] = (uint8_t)(color[2] * 255.1f);
                    _cols[4 * i + 3] = (uint8_t)(color[3] * 255.1f);
                }
            }
            double insertionToleranceSaved = target->getInsertionDistanceTolerance();
            target->setInsertionDistanceTolerance(tolerance);
            target->insertPoints(pts.data(), int(pts.size()) / 3, relative, _cols.data(), true);
            target->setInsertionDistanceTolerance(insertionToleranceSaved);
        }
    }
    return errMsg;
}

std::string _method_intersectPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        bool relative = false;
        double tolerance = 0.01;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
            target->intersectPoints(pts.data(), int(pts.size()) / 3, relative, tolerance);
    }
    return errMsg;
}

std::string _method_intersectPackedPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        bool relative = false;
        double tolerance = 0.01;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
            target->intersectPoints(pts.data(), int(pts.size()) / 3, relative, tolerance);
    }
    return errMsg;
}

std::string _method_subtractPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_matrix, 3, -1, arg_map | arg_optional}))
    {
        std::vector<double> pts;
        fetchMatrixData(inStack, 0, pts, false);
        bool relative = false;
        double tolerance = 0.01;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
        {
            target->removePoints(pts.data(), int(pts.size()) / 3, relative, tolerance);
        }
    }
    return errMsg;
}

std::string _method_subtractPackedPoints(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CPointCloud* target = (CPointCloud*)getSpecificSceneObjectType(targetObj, sim_sceneobject_pointcloud, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string buff = fetchBuffer(inStack, 0);
        size_t l = buff.size() / sizeof(float);
        std::vector<double> pts;
        pts.resize(l);
        for (size_t i = 0; i < l; i++)
            pts[i] = (double)((float*)buff.data())[i];
        bool relative = false;
        double tolerance = 0.01;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchBoolFromKey("relative", relative, &err);
            map->fetchDoubleFromKey("tolerance", tolerance, &err);
        });
        if (errMsg.size() == 0)
        {
            target->removePoints(pts.data(), int(pts.size()) / 3, relative, tolerance);
        }
    }
    return errMsg;
}

std::string _method_setTargetPosition(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CJoint* target = (CJoint*)getSpecificSceneObjectType(targetObj, sim_sceneobject_joint, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_double, arg_map | arg_optional}))
    {
        double targetPos = fetchDouble(inStack, 0);
        double origMaxVelAccelJerk[3];
        double maxVelAccelJerk[3];
        target->getMaxVelAccelJerk(maxVelAccelJerk);
        target->getMaxVelAccelJerk(origMaxVelAccelJerk);
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchDoubleFromKey("maxVelocity", maxVelAccelJerk[0], &err);
            map->fetchDoubleFromKey("maxAcceleration", maxVelAccelJerk[1], &err);
            map->fetchDoubleFromKey("maxJerk", maxVelAccelJerk[2], &err);
        });
        if (errMsg.size() == 0)
        {
            if (target->getJointType() != sim_joint_spherical)
            {
                if ((target->getJointMode() == sim_jointmode_kinematic) || (target->getJointMode() == sim_jointmode_dynamic))
                {
                    if ((target->getJointMode() != sim_jointmode_kinematic) && (!target->getDynSmoothMotionProfile()))
                    {
                        maxVelAccelJerk[1] = origMaxVelAccelJerk[1];
                        maxVelAccelJerk[2] = origMaxVelAccelJerk[2];
                    }
                    target->setMaxVelAccelJerk(maxVelAccelJerk);
                }
                if (target->getJointMode() == sim_jointmode_dynamic)
                {
                    target->setTargetPosition(targetPos);
                    target->setKinematicMotionType(0, true); // reset
                }
                else
                {
                    if (target->getJointMode() == sim_jointmode_kinematic)
                    {
                        target->setTargetPosition(targetPos);
                        target->setKinematicMotionType(1, false); // pos
                    }
                    else
                        target->setKinematicMotionType(0, true); // reset
                }
            }
            else
                errMsg = SIM_ERROR_JOINT_SPHERICAL;
        }
    }
    return errMsg;
}

std::string _method_setTargetVelocity(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CJoint* target = (CJoint*)getSpecificSceneObjectType(targetObj, sim_sceneobject_joint, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_double, arg_map | arg_optional}))
    {
        double targetVel = fetchDouble(inStack, 0);
        double origMaxVelAccelJerk[3];
        double maxVelAccelJerk[3];
        target->getMaxVelAccelJerk(maxVelAccelJerk);
        target->getMaxVelAccelJerk(origMaxVelAccelJerk);
        double initVel;
        bool hasInitVel = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            hasInitVel = map->fetchDoubleFromKey("initVelocity", initVel, &err);
            map->fetchDoubleFromKey("maxAcceleration", maxVelAccelJerk[1], &err);
            map->fetchDoubleFromKey("maxJerk", maxVelAccelJerk[2], &err);
        });
        if (errMsg.size() == 0)
        {
            if (target->getJointType() != sim_joint_spherical)
            {
                if ((target->getJointMode() == sim_jointmode_kinematic) || (target->getJointMode() == sim_jointmode_dynamic))
                {
                    if ((target->getJointMode() != sim_jointmode_kinematic) && (!target->getDynSmoothMotionProfile()))
                    {
                        maxVelAccelJerk[0] = origMaxVelAccelJerk[0];
                        maxVelAccelJerk[1] = origMaxVelAccelJerk[1];
                        maxVelAccelJerk[2] = origMaxVelAccelJerk[2];
                    }
                    target->setMaxVelAccelJerk(maxVelAccelJerk);
                }
                if (hasInitVel)
                    target->setKinematicMotionType(2, true, initVel);


                if (target->getJointMode() == sim_jointmode_dynamic)
                {
                    target->setTargetVelocity(targetVel);
                    target->setKinematicMotionType(0, true); // reset
                }
                else
                {
                    if (target->getJointMode() == sim_jointmode_kinematic)
                    {
                        target->setTargetVelocity(targetVel);
                        target->setKinematicMotionType(2, false); // vel
                    }
                    else
                        target->setKinematicMotionType(0, true); // reset
                }
            }
            else
                errMsg = SIM_ERROR_JOINT_SPHERICAL;
        }
    }
    return errMsg;
}

std::string _method_pushEvent(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_map, arg_map | arg_optional}))
    {
        std::string eventName("userEvent");
        int64_t eventHandle = -1;
        int64_t eventUid = -1;
        bool mergeable = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("name", eventName, &err);
            map->fetchInt64FromKey("handle", eventHandle, &err);
            map->fetchInt64FromKey("uid", eventUid, &err);
            map->fetchBoolFromKey("mergeable", mergeable, &err);
        });
        if (errMsg.size() == 0)
        {
            if (App::scenes->getEventsEnabled())
            {
                CCbor* ev = App::scenes->createNakedEvent(eventName.c_str(), eventHandle, eventUid, mergeable);
                ev->appendText("data");
                std::string buff = inStack->getCborEncodedBuffer(0, 0);
                ev->appendRaw((unsigned char*)buff.data(), buff.size());
                App::scenes->pushEvent();
            }
        }
    }
    return errMsg;
}

std::string _method_getContacts(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CShape* target = nullptr;
    if (targetObj != sim_handle_scene)
        target = (CShape*)getSpecificSceneObjectType(targetObj, sim_sceneobject_shape, &errMsg, -1);
    if (errMsg.empty())
    {
        if (checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
        {
            int dynPass = App::scenes->pluginContainer->dyn_getDynamicStepDivider() - 1;
            withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchInt32FromKey("dynPass", dynPass, &err);
            });
            if (errMsg.size() == 0)
            {
                int obj = sim_handle_all;
                if (target != nullptr)
                    obj = target->getObjectHandle();
                double contactInfo[9];
                int objectHandles[2];
                struct SCont {
                    std::vector<int> body2;
                    std::vector<double> contactPoints;
                    std::vector<double> contactForces;
                    std::vector<double> contactNormals;
                };
                std::map<int, SCont> colls;
                int index = sim_handleflag_extended;
                while (App::scene->dynamicsContainer->getContactForce(dynPass, obj, index, objectHandles, contactInfo))
                {
                    auto it = colls.find(objectHandles[0]);
                    if (it == colls.end())
                    {
                        colls[objectHandles[0]] = {{}, {}, {}, {}};
                        it = colls.find(objectHandles[0]);
                    }
                    it->second.body2.push_back(objectHandles[1]);
                    it->second.contactPoints.push_back(contactInfo[0]);
                    it->second.contactPoints.push_back(contactInfo[1]);
                    it->second.contactPoints.push_back(contactInfo[2]);
                    it->second.contactForces.push_back(contactInfo[3]);
                    it->second.contactForces.push_back(contactInfo[4]);
                    it->second.contactForces.push_back(contactInfo[5]);
                    it->second.contactNormals.push_back(contactInfo[6]);
                    it->second.contactNormals.push_back(contactInfo[7]);
                    it->second.contactNormals.push_back(contactInfo[8]);
                    index++;
                }

                CInterfaceStackTable* contactObjects = new CInterfaceStackTable();
                std::vector<double> contactPoints;
                std::vector<double> contactForces;
                std::vector<double> contactNormals;
                for (const auto& pair : colls)
                {
                    contactPoints.insert(contactPoints.end(), pair.second.contactPoints.begin(), pair.second.contactPoints.end());
                    contactForces.insert(contactForces.end(), pair.second.contactForces.begin(), pair.second.contactForces.end());
                    contactNormals.insert(contactNormals.end(), pair.second.contactNormals.begin(), pair.second.contactNormals.end());
                    for (size_t i = 0; i < pair.second.body2.size(); i++)
                    {
                        int hs[2] = {pair.first, pair.second.body2[i]};
                        contactObjects->appendArrayObject_handleArray(hs, 2);
                    }
                }
                outStack->pushObjectOntoStack(contactObjects);
                if (contactPoints.size() > 0)
                {
                    CMatrix m(3, contactPoints.size()/3);
                    m.data = contactPoints;
                    m.switchStorage();
                    outStack->pushMatrixOntoStack(m);
                    m.data = contactForces;
                    m.switchStorage();
                    outStack->pushMatrixOntoStack(m);
                    m.data = contactNormals;
                    m.switchStorage();
                    outStack->pushMatrixOntoStack(m);
                }
                else
                {
                    outStack->pushMatrixOntoStack((float*)nullptr, 3, 0);
                    outStack->pushMatrixOntoStack((float*)nullptr, 3, 0);
                    outStack->pushMatrixOntoStack((float*)nullptr, 3, 0);
                }
            }
        }
    }
    return errMsg;
}

std::string _method_getGenesisEvents(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {}))
    {
        std::vector<unsigned char> genesisEvents;
        App::scenes->getGenesisEvents(&genesisEvents, nullptr);
        outStack->pushBufferOntoStack((char*)genesisEvents.data(), genesisEvents.size());
    }
    return errMsg;
}

std::string _method_setEventFilters(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map}))
    {
        CInterfaceStackTable* map = (CInterfaceStackTable*)inStack->getStackObjectFromIndex(0);
        std::vector<int64_t> intKeys;
        map->getMapKeys(nullptr, &intKeys);
        std::vector<std::string> textKeys;
        map->getMapKeys(&textKeys, nullptr);
        std::map<int64_t, std::set<std::string>> targetFilters;
        std::map<std::string, std::set<std::string>> typeFilters;
        for (size_t i = 0; i < intKeys.size(); i++)
        {
            bool first = true;
            CInterfaceStackObject* obj = map->getIntMapObject(intKeys[i]);
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_table))
            {
                CInterfaceStackTable* arr = ((CInterfaceStackTable*)obj);
                if (arr->isTableArray())
                {
                    std::vector<std::string> fields;
                    arr->getTextArray(fields);
                    if (fields.size() != 0)
                    {
                        for (size_t j = 0; j < fields.size(); j++)
                        {
                            if (fields[j].size() > 0)
                            {
                                if (first)
                                {
                                    targetFilters[intKeys[i]] = std::set<std::string>();
                                    first = false;
                                }
                                targetFilters[intKeys[i]].insert(fields[j]);
                            }
                        }
                    }
                    else
                        targetFilters[intKeys[i]] = std::set<std::string>(); // empty table --> we want all events for that target object
                }
            }
        }
        for (size_t i = 0; i < textKeys.size(); i++)
        {
            bool first = true;
            CInterfaceStackObject* obj = map->getMapObject(textKeys[i].c_str());
            if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_table))
            {
                CInterfaceStackTable* arr = ((CInterfaceStackTable*)obj);
                if (arr->isTableArray())
                {
                    std::vector<std::string> fields;
                    arr->getTextArray(fields);
                    if (fields.size() != 0)
                    {
                        for (size_t j = 0; j < fields.size(); j++)
                        {
                            if (fields[j].size() > 0)
                            {
                                if (first)
                                {
                                    typeFilters[textKeys[i]] = std::set<std::string>();
                                    first = false;
                                }
                                typeFilters[textKeys[i]].insert(fields[j]);
                            }
                        }
                    }
                    else
                        typeFilters[textKeys[i]] = std::set<std::string>(); // empty table --> we want all events for that target object
                }
            }
        }
        target->setEventFilters(targetFilters, typeFilters);
    }
    return errMsg;
}

std::string _method_getPluginInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
    {
        std::string pluginName = fetchText(inStack, 0);
        CPlugin* plug = App::scenes->pluginContainer->getPluginFromName(pluginName.c_str());
        if (plug == nullptr)
            plug = App::scenes->pluginContainer->getPluginFromName_old(pluginName.c_str(), true);
        if (plug != nullptr)
        {
            CInterfaceStackTable* map = new CInterfaceStackTable();
            map->appendMapObject_text("versionStr", plug->getExtendedVersionString().c_str());
            map->appendMapObject_text("buildDate", plug->getBuildDateString().c_str());
            map->appendMapObject_int32("version", plug->getExtendedVersionInt());
            map->appendMapObject_int32("consoleVerbosity", App::getConsoleVerbosity(pluginName.c_str()));
            map->appendMapObject_int32("statusbarVerbosity", App::getStatusbarVerbosity(pluginName.c_str()));
            outStack->pushObjectOntoStack(map);
        }
        else
            errMsg = SIM_ERROR_INVALID_PLUGIN_NAME;
    }
    return errMsg;
}

std::string _method_setPluginInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string, arg_map}))
    {
        CPlugin* plug = nullptr;
        std::string pluginName = fetchText(inStack, 0);
        if (pluginName.empty())
            plug = App::scenes->pluginContainer->getCurrentPlugin();
        else
        {
            plug = App::scenes->pluginContainer->getPluginFromName(pluginName.c_str());
            if (plug == nullptr)
                plug = App::scenes->pluginContainer->getPluginFromName_old(pluginName.c_str(), true);
        }
        if (plug != nullptr)
        {
            std::string versionStr;
            std::string buildDate;
            int version;
            int consoleVerbosity;
            int statusbarVerbosity;
            bool hasVersion = false;
            bool hasConsoleVerbosity = false;
            bool hasStatusbarVerbosity = false;
            withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchStringFromKey("versionStr", versionStr, &err);
                map->fetchStringFromKey("buildDate", buildDate, &err);
                hasVersion = map->fetchInt32FromKey("version", version, &err);
                hasConsoleVerbosity = map->fetchInt32FromKey("consoleVerbosity", consoleVerbosity, &err);
                hasStatusbarVerbosity = map->fetchInt32FromKey("statusbarVerbosity", statusbarVerbosity, &err);
            });
            if (errMsg.empty())
            {
                if (hasVersion)
                    plug->setExtendedVersionInt(version);
                if (!versionStr.empty())
                    plug->setExtendedVersionString(versionStr.c_str());
                if (!buildDate.empty())
                    plug->setBuildDateString(buildDate.c_str());
                if (hasConsoleVerbosity)
                    App::setConsoleVerbosity(consoleVerbosity, pluginName.c_str());
                if (hasStatusbarVerbosity)
                    App::setStatusbarVerbosity(statusbarVerbosity, pluginName.c_str());
            }
        }
        else
            errMsg = SIM_ERROR_INVALID_PLUGIN_NAME;
    }
    return errMsg;
}

std::string _method_setModuleEntry(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_integer, arg_map}))
    {
        int handle = fetchInt(inStack, 0);
        std::string label;
        bool enabled = true;
        bool checkable = true;
        bool checked = false;
        bool enabledSet = false;
        bool checkableSet = false;
        bool checkedSet = false;
        bool remove = false;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchStringFromKey("label", label, &err);
            enabledSet = map->fetchBoolFromKey("enabled", enabled, &err);
            checkedSet = map->fetchBoolFromKey("checked", checked, &err);
            checkableSet = map->fetchBoolFromKey("checkable", checkable, &err);
             map->fetchBoolFromKey("remove", remove, &err);
        });
        if (errMsg.empty())
        {
            bool first = false;
            if (handle == -1)
            {
                first = true;
                handle = App::scenes->moduleMenuItemContainer->addMenuItem(label.c_str(), targetObj);
            }
            CModuleMenuItem* item = App::scenes->moduleMenuItemContainer->getItemFromHandle(handle);
            if (item != nullptr)
            {
                if (remove)
                    App::scenes->moduleMenuItemContainer->removeMenuItem(handle);
                else
                {
                    if ((!first) && (label != ""))
                        item->setLabel(label.c_str());
                    int s = item->getState();
                    if (enabledSet)
                    {
                        s = s & 6;
                        if (enabled)
                            s = s | 1;
                    }
                    if (checkedSet)
                    {
                        s = s & 5;
                        if (checked)
                            s = s | 2;
                    }
                    if (checkableSet)
                    {
                        s = s & 3;
                        if (checkable)
                            s = s | 4;
                    }
                    item->setState(s);
                }
                outStack->pushInt32OntoStack(handle);
            }
            else
                errMsg = SIM_ERROR_INVALID_HANDLE;
        }
    }
    return errMsg;
}

std::string _method_dynamicsStep(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_double | arg_optional}))
    {
        if ((currentScript != nullptr) && (currentScript->getScriptType() == sim_scripttype_main))
        {
            if (App::scene->dynamicsContainer->getDynamicsEnabled())
            {
                double stepSize = fetchDouble(inStack, 0, 0.0);
                if (stepSize == 0.0)
                    stepSize = App::scene->simulation->getTimeStep();
                App::scene->dynamicsContainer->handleDynamics(stepSize);
                CApiErrors::getAndClearLastError();
                if (!App::scene->dynamicsContainer->isWorldThere())
                    App::scene->dynamicsContainer->markForWarningDisplay_physicsEngineNotSupported();
            }
        }
        else
            errMsg = SIM_ERROR_CAN_ONLY_BE_CALLED_FROM_MAIN_SCRIPT;
    }
    return errMsg;
}

std::string _method_broadcast(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CDetachedScript* target = getDetachedScript(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map}))
    {
        CInterfaceStack* stack = App::scenes->interfaceStackContainer->createStackCopy(inStack);
        if (stack->getStackSize() > 1)
            stack->popStackValue(stack->getStackSize() - 1);
        stack->pushInt32OntoStack(targetObj, false);
        App::scenes->broadcastMsg(stack, targetObj, 0);
        App::scenes->interfaceStackContainer->destroyStack(stack);
    }
    return errMsg;
}

std::string _method_textureSet(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMesh* target = getMesh(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string | arg_optional, arg_table | arg_optional, 2, arg_integer, arg_map | arg_optional}))
    {
        std::string img = fetchBuffer(inStack, 0);
        std::vector<int> resolution;
        fetchIntArray(inStack, 1, resolution);
        double uvScaling[2] = {1.0, 1.0};
        bool repeatU = false;
        bool repeatV = false;
        bool interpolate = false;
        bool decal = false;
        bool flipH = false;
        bool flipV = false;
        std::vector<float> texCoords;
        CPose pose;
        pose.setIdentity();
        withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchDoubleArrayFromKey("scalingUV", uvScaling, 2, &err);
            map->fetchBoolFromKey("repeatU", repeatU, &err);
            map->fetchBoolFromKey("repeatV", repeatV, &err);
            map->fetchBoolFromKey("interpolate", interpolate, &err);
            map->fetchBoolFromKey("decal", decal, &err);
            map->fetchBoolFromKey("flipH", flipH, &err);
            map->fetchBoolFromKey("flipV", flipV, &err);
            texCoords.resize(target->getIndices()->size() * 2);
            if (map->fetchFloatArrayFromKey("coordinates", texCoords.data(), target->getIndices()->size() * 2, &err))
            {
                flipH = false;
                flipV = false;
            }
            else
                texCoords.clear();
            double poseDat[7];
            if (map->fetchDoubleArrayFromKey("pose", poseDat, 7, &err))
                pose.setData(poseDat, true);
        });
        if (errMsg.empty())
        {
            int64_t shapeHandle;
            CPose dummyPose;
            target->getHandleProperty_mesh(prop(PropMesh::shape).name, shapeHandle, dummyPose);
            CTextureProperty* tp = target->getTextureProperty();
            if (tp != nullptr)
            {
                delete tp;
                target->setTextureProperty(nullptr);
            }
            if (!img.empty())
            {
                if ((resolution.size() == 2) && (resolution[0] > 0) && (resolution[1] > 0) && (resolution[0] <= 8192) && (resolution[1] <= 8192))
                {
                    if ((img.size() == resolution[0] * resolution[1] * 3) || (img.size() == resolution[0] * resolution[1] * 4))
                    {
                        int n = int(img.size() / (resolution[0] * resolution[1]));
                        CTextureObject* textureObj = new CTextureObject(resolution[0], resolution[1]);
                        textureObj->setImage(n == 4, flipH, !flipV, (unsigned char*)img.data());
                        textureObj->addDependentObject(shapeHandle, target->getObjectHandle());
                        int texID = App::scene->textureContainer->addObject(textureObj, false); // might erase the textureObj and return a similar object already present!!
                        CTextureProperty* tp = new CTextureProperty(texID);
                        target->setTextureProperty(tp);
                        tp->setInterpolateColors(interpolate);
                        if (decal)
                            tp->setApplyMode(1);
                        else
                            tp->setApplyMode(0);
                        if (texCoords.size() > 0)
                            tp->setFixedCoordinates(&texCoords);
                        else
                        {
                            tp->setRepeatU(repeatU);
                            tp->setRepeatV(repeatV);
                            tp->setTextureScaling(uvScaling[0], uvScaling[1]);
                            tp->setTextureRelativeConfig(pose);
                        }
                    }
                    else
                        errMsg = SIM_ERROR_INVALID_RESOLUTION;
                }
                else
                    errMsg = SIM_ERROR_INVALID_RESOLUTION;
            }

            // send texture update event:
            std::vector<CMesh*> all;
            std::vector<CPose> allTr;
            CShape* shape = App::scene->sceneObjects->getShapeFromHandle(shapeHandle);
            shape->getMesh()->getAllMeshComponentsCumulative(CPose::identityTransformation, all, &allTr);
            for (size_t i = 0; i < all.size(); i++)
            {
                if (all[i] == target)
                    target->pushObjectCreationOrChangeEvent(shapeHandle, shape->getObjectUid(), allTr[i], 2);
            }
        }
    }
    return errMsg;
}

std::string _method_textureSetData(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMesh* target = getMesh(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_string, arg_map | arg_optional}))
    {
        std::string data = fetchBuffer(inStack, 0);
        int position[2] = {0, 0};
        int size[2] = {0, 0};
        double interpolation = 0.0;
        bool rectangular = true;
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32ArrayFromKey("position", position, 2, &err);
            map->fetchInt32ArrayFromKey("size", size, 2, &err);
            map->fetchDoubleFromKey("interpolation", interpolation, &err);
            map->fetchBoolFromKey("rectangular", rectangular, &err);
        });
        if (errMsg.empty())
        {
            CTextureProperty* tp = target->getTextureProperty();
            if (tp != nullptr)
            {
                CTextureObject* to = tp->getTextureObject();
                if (to != nullptr)
                {
                    int resX, resY;
                    to->getTextureSize(resX, resY);
                    if ((size[0] >= 0) && (size[1] >= 0) && (data.size() >= 3) && (position[0] >= 0) && (position[1] >= 0) && (position[0] + size[0] <= resX) && (position[1] + size[1] <= resY))
                    {
                        if (size[0] == 0)
                        {
                            position[0] = 0;
                            size[0] = resX;
                        }
                        if (size[1] == 0)
                        {
                            position[1] = 0;
                            size[1] = resY;
                        }
                        if (int(data.size()) < size[0] * size[1] * 3)
                        {
                            std::string d(data);
                            data.resize(size[0] * size[1] * 3);
                            for (size_t i = 0; i < size[0] * size[1]; i++)
                            {
                                data[3 * i + 0] = d[0];
                                data[3 * i + 1] = d[1];
                                data[3 * i + 2] = d[2];
                            }
                        }
                        to->writePortionOfTexture((unsigned char*)data.data(), position[0], position[1], size[0], size[1], !rectangular, interpolation);

                        // send texture update event:
                        int64_t shapeHandle;
                        CPose dummyPose;
                        target->getHandleProperty_mesh(prop(PropMesh::shape).name, shapeHandle, dummyPose);
                        std::vector<CMesh*> all;
                        std::vector<CPose> allTr;
                        CShape* shape = App::scene->sceneObjects->getShapeFromHandle(shapeHandle);
                        shape->getMesh()->getAllMeshComponentsCumulative(CPose::identityTransformation, all, &allTr);
                        for (size_t i = 0; i < all.size(); i++)
                        {
                            if (all[i] == target)
                                target->pushObjectCreationOrChangeEvent(shapeHandle, shape->getObjectUid(), allTr[i], 2);
                        }
                    }
                    else
                        errMsg = SIM_ERROR_INVALID_ARGUMENTS;
                }
                else
                    errMsg = SIM_ERROR_TEXTURE_INEXISTANT;
            }
            else
                errMsg = SIM_ERROR_TEXTURE_INEXISTANT;
        }
    }
    return errMsg;
}

std::string _method_textureGetData(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    CMesh* target = getMesh(targetObj, &errMsg, -1);
    if ((target != nullptr) && checkInputArguments(inStack, &errMsg, {arg_map | arg_optional}))
    {
        int position[2] = {0, 0};
        int size[2] = {0, 0};
        withOptionalMap(inStack, 0, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            map->fetchInt32ArrayFromKey("position", position, 2, &err);
            map->fetchInt32ArrayFromKey("size", size, 2, &err);
        });
        if (errMsg.empty())
        {
            CTextureProperty* tp = target->getTextureProperty();
            if (tp != nullptr)
            {
                CTextureObject* to = tp->getTextureObject();
                if (to != nullptr)
                {
                    int resX, resY;
                    to->getTextureSize(resX, resY);
                    if ((size[0] >= 0) && (size[1] >= 0) && (position[0] >= 0) && (position[1] >= 0) && (position[0] + size[0] <= resX) && (position[1] + size[1] <= resY))
                    {
                        if (size[0] == 0)
                        {
                            position[0] = 0;
                            size[0] = resX;
                        }
                        if (size[1] == 0)
                        {
                            position[1] = 0;
                            size[1] = resY;
                        }
                        uint8_t* retVal = to->readPortionOfTexture(position[0], position[1], size[0], size[1]);
                        outStack->pushBufferOntoStack((const char*)retVal, size[0] * size[1] * 3);
                    }
                    else
                        errMsg = SIM_ERROR_INVALID_ARGUMENTS;
                }
                else
                    errMsg = SIM_ERROR_TEXTURE_INEXISTANT;
            }
            else
                errMsg = SIM_ERROR_TEXTURE_INEXISTANT;
        }
    }
    return errMsg;
}

std::string _method_getEnumInfo(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_string}))
    {
        std::string enumT = fetchText(inStack, 0);

        using MapFn = std::map<int, std::string>(*)();
        std::map<std::string, MapFn> builder_map;

        #define X(type) builder_map.emplace(#type, []() { return buildEnumToStringMap<type>(); });
        SIM_ENUM_TYPES
        #undef X

        std::map<int, std::string> enum_to_string;
        auto it = builder_map.find(enumT);
        if (it != builder_map.end())
        {
            enum_to_string = it->second();
            CInterfaceStackTable* tbl_strToEnum = new CInterfaceStackTable();
            for (const auto& [value, name] : enum_to_string)
                tbl_strToEnum->appendMapObject_int32(name.c_str(), value);
            outStack->pushObjectOntoStack(tbl_strToEnum);
        }
        else
            errMsg = "unsupported type.";
    }
    return errMsg;
}

std::string _method_createShapeFromPath(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_matrix, 7, -1, arg_map | arg_optional}))
    {
        std::vector<double> pppath;
        fetchMatrixData(inStack, 0, pppath, false);
        std::vector<double> section = {-0.01, -0.01, 0.01, -0.01, 0.01, 0.01, -0.01, 0.01, -0.01, -0.01};
        C3Vector zvect = C3Vector::unitZVector;
        int axis = 0;
        bool closedPath = false;
        if (pppath.size() >= 21)
        {
            closedPath = true;
            for (size_t i = 0; i < 7; i++)
            {
                if (pppath[i] != pppath[pppath.size() - 7 + i])
                {
                    closedPath = false;
                    break;
                }
            }
        }
        withOptionalMap(inStack, 1, errMsg, [&](CInterfaceStackTable* map, std::string& err)
        {
            std::vector<double> s;
            if (map->fetchArrayAsConsecutiveDoublesFromKey("section", s, &err))
            {
                int ss = s.size();
                s.clear();
                if (map->fetchMatrixDataFromKey("section", s, 2, ss / 2, false, &err))
                    section = s;
            }
            std::vector<double> arr;
            if (map->fetchMatrixDataFromKey("upVector", arr, 3, 1, true, &err))
                zvect.setData(arr.data());
            map->fetchInt32FromKey("axis", axis, &err);
        });
        if (errMsg.size() == 0)
        {
            int pathSize = pppath.size();
            if (pathSize >= 14)
            {
                int sectionSize = section.size();
                // First make sure the points are not coincident:
                std::vector<double> ppath;
                C3Vector prevV;
                prevV.clear();
                CQuaternion prevQ;
                prevQ.clear();
                for (int i = 0; i < pathSize / 7; i++)
                {
                    C3Vector v(pppath.data() + 7 * i);
                    CQuaternion q(pppath.data() + 7 * i + 3, true);
                    q.normalize();
                    double d = (prevV - v).getLength();
                    if ((d >= 0.0005) || (i == 0))
                    {
                        prevV = v;
                        prevQ = q;
                        for (size_t j = 0; j < 3; j++)
                            ppath.push_back(v(j));
                        for (size_t j = 0; j < 4; j++)
                            ppath.push_back(q(j));
                    }
                }
                pathSize = int(ppath.size());
                if (pathSize >= 7 * 2)
                {
                    size_t confCnt = size_t(pathSize) / 7;
                    size_t elementCount = confCnt;
                    size_t secVertCnt = size_t(sectionSize) / 2;
                    std::vector<double> path;
                    for (size_t i = 0; i < confCnt; i++)
                    {
                        C3Vector p0, p1, p2;
                        if (i != 0)
                            p0 = C3Vector(&ppath[0] + 7 * (i - 1));
                        else
                        {
                            if (closedPath)
                                p0 = C3Vector(&ppath[0] + pathSize - 7);
                        }
                        p1 = C3Vector(&ppath[0] + 7 * i);
                        CQuaternion q(&ppath[0] + 7 * i + 3, false); // Quaternion notation was changed above!
                        if (axis != 0)
                            zvect = q.getAxis(axis - 1);
                        if (i != (confCnt - 1))
                            p2 = C3Vector(&ppath[0] + 7 * (i + 1));
                        else
                        {
                            if (closedPath)
                                p2 = C3Vector(&ppath[0] + 7 * 1);
                        }
                        C3Vector vy;
                        if (closedPath || ((i != 0) && (i != (confCnt - 1))))
                            vy = (p1 - p0) + (p2 - p1);
                        else
                        {
                            if (i == 0)
                                vy = (p2 - p1);
                            else
                                vy = (p1 - p0);
                        }
                        vy.normalize();
                        C3Vector vx = vy ^ zvect;
                        vx.normalize();
                        C4X4Matrix m;
                        m.X = p1;
                        m.M.axis[0] = vx;
                        m.M.axis[1] = vy;
                        m.M.axis[2] = vx ^ vy;
                        CPose p(m.getTransformation());
                        for (size_t j = 0; j < 7; j++)
                            path.push_back(p(j));
                    }

                    bool sectionClosed = ((section[0] == section[sectionSize - 2]) && (section[1] == section[sectionSize - 1]));
                    if (sectionClosed)
                        secVertCnt--;

                    std::vector<double> vertices;
                    std::vector<int> indices;
                    CPose tr0;
                    tr0.setData(&path[0]);
                    for (size_t i = 0; i <= secVertCnt - 1; i++)
                    {
                        C3Vector v(section[i * 2 + 0], 0.0, section[i * 2 + 1]);
                        v = tr0 * v;
                        vertices.push_back(v(0));
                        vertices.push_back(v(1));
                        vertices.push_back(v(2));
                    }

                    int previousVerticesOffset = 0;
                    for (size_t ec = 1; ec < elementCount; ec++)
                    {
                        CPose tr;
                        tr.setData(&path[ec * 7]);
                        int forwOff = int(secVertCnt);
                        for (int i = 0; i <= int(secVertCnt) - 1; i++)
                        {
                            C3Vector v(section[i * 2 + 0], 0.0, section[i * 2 + 1]);
                            if (closedPath && (ec == (elementCount - 1)))
                                forwOff = -previousVerticesOffset;
                            else
                            {
                                v = tr * v;
                                vertices.push_back(v(0));
                                vertices.push_back(v(1));
                                vertices.push_back(v(2));
                            }
                            if (i != int(secVertCnt - 1))
                            {
                                indices.push_back(previousVerticesOffset + 0 + i);
                                indices.push_back(previousVerticesOffset + forwOff + i);
                                indices.push_back(previousVerticesOffset + 1 + i);
                                indices.push_back(previousVerticesOffset + 1 + i);
                                indices.push_back(previousVerticesOffset + forwOff + i);
                                indices.push_back(previousVerticesOffset + forwOff + i + 1);
                            }
                            else
                            {
                                if (sectionClosed)
                                {
                                    indices.push_back(previousVerticesOffset + 0 + i);
                                    indices.push_back(previousVerticesOffset + forwOff + i);
                                    indices.push_back(previousVerticesOffset + 0);
                                    indices.push_back(previousVerticesOffset + 0);
                                    indices.push_back(previousVerticesOffset + forwOff + i);
                                    indices.push_back(previousVerticesOffset + forwOff + 0);
                                }
                            }
                        }
                        previousVerticesOffset += int(secVertCnt);
                    }
                    int h = simCreateShape_internal(0, 0.0, &vertices[0], int(vertices.size()), &indices[0], int(indices.size()), nullptr, nullptr, nullptr, nullptr);
                    outStack->pushHandleOntoStack(h);
                }
            }
            else
                errMsg = "not enough path points.";
        }
    }
    return errMsg;
}

std::string _method_getClosestOnPath(int targetObj, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack)
{
    std::string errMsg;
    if (checkInputArguments(inStack, &errMsg, {arg_matrix, -1, -1, arg_vector, arg_map | arg_optional}))
    {
        CMatrix matr = fetchMatrix(inStack, 0);
        matr.transpose();
        std::vector<double> points = matr.data;
        int dim = int(matr.cols);
        std::vector<double> point;
        fetchDoubleArray(inStack, 1, point);
        if (int(point.size()) == dim)
        {
            std::vector<double> metric;
            metric.resize(dim, 1.0);
            std::vector<int> types; // 0 = linear, 1 = revolute, 2 = quaternions (4 values)
            types.resize(dim, 0); // linear is default
            withOptionalMap(inStack, 2, errMsg, [&](CInterfaceStackTable* map, std::string& err)
            {
                map->fetchDoubleArrayFromKey("metric", metric.data(), dim, &err);
                map->fetchInt32ArrayFromKey("types", types.data(), dim, &err);
            });
            if (errMsg.size() == 0)
            {
                // Validate the types vector: values in {0,1,2}, quaternions in blocks of 4:
                bool typesOk = true;
                bool allLinear = true;
                {
                    int j = 0;
                    while (j < dim)
                    {
                        if (types[j] == 0)
                            j++;
                        else if (types[j] == 1)
                        {
                            allLinear = false;
                            j++;
                        }
                        else if (types[j] == 2)
                        {
                            allLinear = false;
                            if ((j + 3 < dim) && (types[j + 1] == 2) && (types[j + 2] == 2) && (types[j + 3] == 2))
                                j += 4;
                            else
                            {
                                typesOk = false;
                                break;
                            }
                        }
                        else
                        {
                            typesOk = false;
                            break;
                        }
                    }
                }
                if (typesOk)
                {
                    const size_t n = points.size() / dim;
                    if (n >= 2)
                    {
                        std::vector<double> foundPoint;
                        const size_t segCount = n - 1;

                        // Wrap an angle to (-pi, pi]:
                        auto wrapAngle = [](double a) -> double
                        {
                            a = fmod(a, 2.0 * piValue);
                            if (a > piValue)
                                a -= 2.0 * piValue;
                            else if (a <= -piValue)
                                a += 2.0 * piValue;
                            return a;
                        };

                        // Angular distance between two quaternions stored as (qx,qy,qz,qw):
                        auto quatAngle = [&](const double* qA, const double* qB) -> double
                        {
                            simReal a[4], b[4];
                            for (int j = 0; j < 4; j++)
                            {
                                a[j] = (simReal)qA[j];
                                b[j] = (simReal)qB[j];
                            }
                            CQuaternion Qa(a, true), Qb(b, true);
                            Qa.normalize();
                            Qb.normalize();
                            return (double)Qa.getAngleBetweenQuaternions(Qb);
                        };

                        // Interpolate a configuration on segment [A,B] at t in [0,1]:
                        auto interp = [&](const double* A, const double* B, double t, std::vector<double>& out)
                        {
                            out.resize(dim);
                            int j = 0;
                            while (j < dim)
                            {
                                if (types[j] == 0)
                                {   // linear: plain lerp
                                    out[j] = A[j] + t * (B[j] - A[j]);
                                    j++;
                                }
                                else if (types[j] == 1)
                                {   // revolute: lerp along the shortest arc, wrapped result
                                    double d = wrapAngle(B[j] - A[j]);
                                    out[j] = wrapAngle(A[j] + t * d);
                                    j++;
                                }
                                else
                                {   // quaternion block (4 values): slerp
                                    simReal a[4], b[4], r[4];
                                    for (int k = 0; k < 4; k++)
                                    {
                                        a[k] = (simReal)A[j + k];
                                        b[k] = (simReal)B[j + k];
                                    }
                                    CQuaternion qa(a, true), qb(b, true);
                                    qa.normalize(); // be safe about input
                                    qb.normalize();
                                    CQuaternion q;
                                    q.buildInterpolation(qa, qb, (simReal)t);
                                    q.getData(r, true);
                                    for (int k = 0; k < 4; k++)
                                        out[j + k] = (double)r[k];
                                    j += 4;
                                }
                            }
                        };

                        // Weighted squared distance between a configuration and 'point'.
                        // For quaternion blocks, metric[j] (first value of the block) weights
                        // the angular distance; the 3 following metric values are ignored.
                        auto dist2 = [&](const std::vector<double>& q) -> double
                        {
                            double d = 0.0;
                            int j = 0;
                            while (j < dim)
                            {
                                if (types[j] == 0)
                                {
                                    double dd = q[j] - point[j];
                                    d += metric[j] * dd * dd;
                                    j++;
                                }
                                else if (types[j] == 1)
                                {
                                    double dd = wrapAngle(q[j] - point[j]);
                                    d += metric[j] * dd * dd;
                                    j++;
                                }
                                else
                                {
                                    double ang = quatAngle(&q[j], &point[j]);
                                    d += metric[j] * ang * ang;
                                    j += 4;
                                }
                            }
                            return d;
                        };

                        // Golden-section search on [lo,hi] for the given segment:
                        auto goldenSection = [&](const double* A, const double* B,
                                                 double lo, double hi, std::vector<double>& cand) -> double
                        {
                            const double invPhi = (sqrt(5.0) - 1.0) / 2.0;
                            double t1 = hi - invPhi * (hi - lo);
                            double t2 = lo + invPhi * (hi - lo);
                            interp(A, B, t1, cand); double c1 = dist2(cand);
                            interp(A, B, t2, cand); double c2 = dist2(cand);
                            while (hi - lo > 1e-8)
                            {
                                if (c1 < c2)
                                {
                                    hi = t2; t2 = t1; c2 = c1;
                                    t1 = hi - invPhi * (hi - lo);
                                    interp(A, B, t1, cand); c1 = dist2(cand);
                                }
                                else
                                {
                                    lo = t1; t1 = t2; c1 = c2;
                                    t2 = lo + invPhi * (hi - lo);
                                    interp(A, B, t2, cand); c2 = dist2(cand);
                                }
                            }
                            return (lo + hi) / 2.0;
                        };

                        double globalBestDist = std::numeric_limits<double>::max();
                        int globalBestDistSegmentIndex = 0;
                        std::vector<double> cand;

                        for (size_t i = 0; i < segCount; i++)
                        {
                            const double* A = &points[dim * i];
                            const double* B = &points[dim * (i + 1)];

                            double tOpt;
                            if (allLinear)
                            {   // closed-form weighted projection onto the segment
                                double num = 0.0, den = 0.0;
                                for (int j = 0; j < dim; j++)
                                {
                                    double ab = B[j] - A[j];
                                    num += metric[j] * (point[j] - A[j]) * ab;
                                    den += metric[j] * ab * ab;
                                }
                                tOpt = (den > 0.0) ? (num / den) : 0.0;
                                if (tOpt < 0.0) tOpt = 0.0;
                                if (tOpt > 1.0) tOpt = 1.0;
                            }
                            else
                            {   // revolute/quaternion terms make the cost non-quadratic
                                // (and possibly multimodal, e.g. when a wrapped angular
                                // difference crosses +/-pi). Coarse scan to bracket the
                                // global minimum, then golden-section refinement:
                                const int coarseSamples = 20;
                                double bestT = 0.0, bestC = std::numeric_limits<double>::max();
                                for (int k = 0; k <= coarseSamples; k++)
                                {
                                    double t = double(k) / double(coarseSamples);
                                    interp(A, B, t, cand);
                                    double c = dist2(cand);
                                    if (c < bestC) { bestC = c; bestT = t; }
                                }
                                double lo = std::max(0.0, bestT - 1.0 / coarseSamples);
                                double hi = std::min(1.0, bestT + 1.0 / coarseSamples);
                                tOpt = goldenSection(A, B, lo, hi, cand);
                            }

                            // Keep this segment's best only if it beats the global best:
                            interp(A, B, tOpt, cand);
                            double c = dist2(cand);
                            if (c < globalBestDist)
                            {
                                globalBestDist = c;
                                globalBestDistSegmentIndex = int(i);
                                foundPoint = cand;
                            }
                        }
                        outStack->pushVectorOntoStack(foundPoint.data(), foundPoint.size());
                        outStack->pushInt32OntoStack(globalBestDistSegmentIndex);
                    }
                    else
                        errMsg = "not enough path points.";
                }
                else
                    errMsg = "invalid types (values must be 0, 1 or 2, and quaternions require 4 consecutive values of type 2).";
            }
        }
        else
            errMsg = "mismatch between path and point dimensions.";
    }
    return errMsg;
}
