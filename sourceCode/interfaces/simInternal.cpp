#include <simInternal.h>
#include <simulation.h>
#include <collisionRoutines.h>
#include <distanceRoutines.h>
#include <proxSensorRoutine.h>
#include <meshRoutines.h>
#include <tt.h>
#include <fileOperations.h>
#include <persistentDataContainer.h>
#include <graphingRoutines_old.h>
#include <sceneObjectOperations.h>
#include <threadPool_old.h>
#include <addOperations.h>
#include <app.h>
#include <mesh.h>
#include <vDateTime.h>
#include <utils.h>
#include <vVarious.h>
#include <imgLoaderSaver.h>
#include <apiErrors.h>
#include <memorizedConf_old.h>
#include <algorithm>
#include <iostream>
#include <tinyxml2.h>
#include <simFlavor.h>
#include <regex>
#include <interfaceStackString.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

int _currentScriptNameIndex = -1;
int _currentScriptHandle = -1;

bool fullModelCopyFromApi = true;
bool waitingForTrigger = false;
bool doNotRunMainScriptFromRosInterface = false;

std::string lastInfoString;

void setLastInfo(const char* infoStr)
{
    lastInfoString = infoStr;
}

int simGetExitRequest_internal()
{
    return (App::getExitRequest());
}

int simLoop_internal(void (*callback)(), int options)
{
    App::loop(callback, (options & 1) == 0);
    return (1);
}

int simInitialize_internal(const char* appDir, int options)
{
    App::init(appDir, options);
    return (1);
}

int simDeinitialize_internal()
{ // If already called, then means we closed from the UI and dont need to post another request
    if (!App::getExitRequest())
        App::postExitRequest();
    App::cleanup();
    return (1);
}

int simPostExitRequest_internal()
{ // If already called, then means we closed from the UI and dont need to post another request
    if (!App::getExitRequest())
        App::postExitRequest();
    return (1);
}

void simRegCallback_internal(int index, void* callback)
{
    while (index >= int(App::callbacks.size()))
        App::callbacks.push_back(nullptr);
    App::callbacks[index] = callback;
}

void simRunGui_internal(int options)
{
#ifdef SIM_WITH_GUI
    GuiApp app;
    app.runGui(options);
#endif
}

int simTest_internal(int mode, void* ptr1, void* ptr2, void* ptr3)
{
    if (mode == 0)
        VThread::launchQtThread((SIMPLE_VTHREAD_START_ADDRESS)ptr1);
    return (0);
}

std::string getIndexAdjustedObjectName(const char* nm)
{
    std::string retVal;
    if (strlen(nm) != 0)
    {
        retVal = nm;
        if (retVal.find('#') == std::string::npos)
        { // e.g. "myObject42"
            if (_currentScriptNameIndex != -1)
            { // for backward compatibility
                retVal += "#";
                retVal += boost::lexical_cast<std::string>(_currentScriptNameIndex);
            }
        }
        if ((retVal.length() != 0) && (retVal[retVal.length() - 1] == '#')) // e.g. "myObject#"
            retVal.erase(retVal.end() - 1);
    }
    return (retVal);
}

void setCurrentScriptInfo_cSide(int scriptHandle, int scriptNameIndex)
{
    _currentScriptHandle = scriptHandle;
    _currentScriptNameIndex = scriptNameIndex;
}

int getCurrentScriptNameIndex_cSide()
{
    return (_currentScriptNameIndex);
}

bool ifEditModeActiveGenerateErrorAndReturnTrue(const char* functionName)
{
#ifdef SIM_WITH_GUI
    if (GuiApp::getEditModeType() != NO_EDIT_MODE)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERRROR_EDIT_MODE_ACTIVE);
        return (true);
    }
#endif
    return (false);
}

bool canBoolIntOrFloatParameterBeSetOrGet(const char* functionName, int when)
{ // 'when' coding: bit1: sim launched(2), bit2: mainWindow not present(4),
    // bit3: mainWIndow present(8),bit4: sim not running(16), bit5: sim running(32)
    int st = 2;
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow == nullptr)
        st |= 4;
    else
        st |= 8;
#else
    st |= 4;
#endif
    if (App::currentWorld->simulation->isSimulationStopped())
        st |= 16;
    else
        st |= 32;
    if (((st & 3) & when) == 0)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_CANNOT_SET_GET_PARAM_LAUNCH);
        return (false);
    }
    if (((st & 12) & when) == 0)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_CANNOT_SET_GET_PARAM_WINDOW);
        return (false);
    }
    if (((st & 48) & when) == 0)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_CANNOT_SET_GET_PARAM_SIM);
        return (false);
    }
    return (true);
}

bool doesObjectExist(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else
        return (true);
}

bool doesObjectOrScriptExist(const char* functionName, int identifier)
{
    bool retVal;
    if (identifier > SIM_IDEND_SCENEOBJECT)
        retVal = (App::worldContainer->getScriptObjectFromHandle(identifier) != nullptr);
    else
        retVal = (App::currentWorld->sceneObjects->getObjectFromHandle(identifier) != nullptr);
    if (!retVal)
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
    return retVal;
}

bool doesEntityExist(const char* functionName, int identifier)
{
    if (identifier > SIM_IDEND_SCENEOBJECT)
    {
        if (App::currentWorld->collections->getObjectFromHandle(identifier) == nullptr)
        {
            CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_ENTITY_INEXISTANT);
            return (false);
        }
        return (true);
    }
    else
    {
        if (App::currentWorld->sceneObjects->getObjectFromHandle(identifier) == nullptr)
        {
            CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_ENTITY_INEXISTANT);
            return (false);
        }
        return (true);
    }
}

bool doesCollectionExist(const char* functionName, int identifier)
{
    if (App::currentWorld->collections->getObjectFromHandle(identifier) == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_COLLECTION_INEXISTANT);
        return (false);
    }
    return (true);
}

bool doesCollisionObjectExist(const char* functionName, int identifier)
{
    if (App::currentWorld->collisions_old->getObjectFromHandle(identifier) == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_COLLISION_INEXISTANT);
        return (false);
    }
    else
        return (true);
}

bool doesDistanceObjectExist(const char* functionName, int identifier)
{
    if (App::currentWorld->distances_old->getObjectFromHandle(identifier) == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_DISTANCE_INEXISTANT);
        return (false);
    }
    else
        return (true);
}

bool isJoint(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_joint)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_JOINT);
        return (false);
    }
    return (true);
}
bool isShape(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_shape)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_SHAPE);
        return (false);
    }
    return (true);
}
bool isSensor(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_proximitysensor)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_PROX_SENSOR);
        return (false);
    }
    return (true);
}
bool isMill(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_mill)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_MILL);
        return (false);
    }
    return (true);
}
bool isForceSensor(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_forcesensor)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_FORCE_SENSOR);
        return (false);
    }
    return (true);
}
bool isVisionSensor(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_visionsensor)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_VISION_SENSOR);
        return (false);
    }
    return (true);
}
bool isCamera(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_camera)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_CAMERA);
        return (false);
    }
    return (true);
}
bool isGraph(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_graph)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_GRAPH);
        return (false);
    }
    return (true);
}
bool isPath(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_path)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_PATH);
        return (false);
    }
    return (true);
}
bool isLight(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_light)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_LIGHT);
        return (false);
    }
    return (true);
}
bool isDummy(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_dummy)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_DUMMY);
        return (false);
    }
    return (true);
}
bool isOctree(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_octree)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_OCTREE);
        return (false);
    }
    return (true);
}
bool isPointCloud(const char* functionName, int identifier)
{
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_INEXISTANT);
        return (false);
    }
    else if (it->getObjectType() != sim_sceneobject_pointcloud)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_OBJECT_NOT_POINTCLOUD);
        return (false);
    }
    return (true);
}
bool doesUIExist(const char* functionName, int elementHandle)
{
    CButtonBlock* it = App::currentWorld->buttonBlockContainer_old->getBlockWithID(elementHandle);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_UI_INEXISTANT);
        return (false);
    }
    return (true);
}

bool doesUIButtonExist(const char* functionName, int elementHandle, int buttonHandle)
{
    CButtonBlock* it = App::currentWorld->buttonBlockContainer_old->getBlockWithID(elementHandle);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_UI_INEXISTANT);
        return (false);
    }
    else if (it->getButtonWithID(buttonHandle) == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_UI_BUTTON_INEXISTANT);
        return (false);
    }
    return (true);
}

bool doesIKGroupExist(const char* functionName, int identifier)
{
    CIkGroup_old* it = App::currentWorld->ikGroups_old->getObjectFromHandle(identifier);
    if (it == nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_IK_GROUP_INEXISTANT);
        return (false);
    }
    else
        return (true);
}

char* _extractCustomDataFromBuffer(std::vector<char>& buffer, const char* dataName, int* extractedBufSize)
{
    int off = 0;
    while (off + 8 < int(buffer.size()))
    {
        int sizeIncr = ((int*)((&buffer[0]) + off))[0];
        int nameLength = ((int*)((&buffer[0]) + off))[1];
        std::string datName(((&buffer[0]) + off) + 4 + 4);
        if (datName.compare(dataName) == 0)
        { // ok, we have the correct data here
            int datLength = sizeIncr - 4 - 4 - nameLength;
            char* returnBuffer = new char[datLength];
            extractedBufSize[0] = datLength;
            for (int i = 0; i < datLength; i++)
                returnBuffer[i] = buffer[off + 4 + 4 + nameLength + i];
            buffer.erase(buffer.begin() + off, buffer.begin() + off + sizeIncr);
            return (returnBuffer);
        }
        // continue searching
        off += sizeIncr;
    }
    extractedBufSize[0] = 0;
    return (nullptr);
}

void _extractCustomDataTagsFromBuffer(std::vector<char>& buffer, std::vector<std::string>& tags)
{
    int off = 0;
    while (off + 8 < int(buffer.size()))
    {
        int sizeIncr = ((int*)((&buffer[0]) + off))[0];
        // int nameLength=((int*)((&buffer[0])+off))[1];
        std::string datName(((&buffer[0]) + off) + 4 + 4);
        tags.push_back(datName);
        off += sizeIncr;
    }
}

void _appendCustomDataToBuffer(std::vector<char>& buffer, const char* dataName, const char* data, int dataSize)
{
    if ((data != nullptr) && (dataSize > 0))
    {
        int nameLength = int(strlen(dataName) + 1); // with zero char
        int sizeIncr = 4 + 4 + nameLength + dataSize;
        buffer.push_back(((char*)&sizeIncr)[0]);
        buffer.push_back(((char*)&sizeIncr)[1]);
        buffer.push_back(((char*)&sizeIncr)[2]);
        buffer.push_back(((char*)&sizeIncr)[3]);
        buffer.push_back(((char*)&nameLength)[0]);
        buffer.push_back(((char*)&nameLength)[1]);
        buffer.push_back(((char*)&nameLength)[2]);
        buffer.push_back(((char*)&nameLength)[3]);
        for (int i = 0; i < nameLength; i++)
            buffer.push_back(dataName[i]);
        for (int i = 0; i < dataSize; i++)
            buffer.push_back(data[i]);
    }
}

void* simGetMainWindow_internal(int type)
{ // 0=window handle , otherwise Qt pointer
    C_API_START;

    //    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
        {
            if (type == 0)
            {
#ifdef WIN_SIM
                return ((void*)GuiApp::mainWindow->winId());
#endif
            }
            if (type == 1)
            {
                return (GuiApp::mainWindow);
            }
        }
#endif
        return (nullptr);
    }
    //    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simRefreshDialogs_internal(int refreshDegree)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (refreshDegree == 0)
            GuiApp::setLightDialogRefreshFlag();
        if (refreshDegree == 2)
            GuiApp::setFullDialogRefreshFlag();
        GuiApp::setDialogRefreshDontPublishFlag();
#endif
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

char* simGetLastInfo_internal()
{
    // C_API_START; not here!

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (lastInfoString.size() == 0)
            return (nullptr);
        char* retVal = new char[lastInfoString.length() + 1];
        for (unsigned int i = 0; i < lastInfoString.length(); i++)
            retVal[i] = lastInfoString[i];
        retVal[lastInfoString.length()] = 0;
        lastInfoString.clear();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

char* simGetLastError_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string lastErr(CApiErrors::getAndClearLastWarningOrError());
        if (lastErr.size() == 0)
            return (nullptr);
        char* retVal = new char[lastErr.length() + 1];
        for (unsigned int i = 0; i < lastErr.length(); i++)
            retVal[i] = lastErr[i];
        retVal[lastErr.length()] = 0;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simSetLastError_internal(const char* setToNullptr, const char* errorMessage)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string func;
        if (setToNullptr != nullptr)
            func = setToNullptr;
        if (func.compare(0, 8, "warning@") == 0)
            CApiErrors::setThreadBasedFirstCapiWarning_old(errorMessage);
        else
            CApiErrors::setThreadBasedFirstCapiError_old(errorMessage);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

bool isPropertyNameValid(const char* functionName, const char* pName)
{
    while (*pName)
    {
        char c = *pName++;
        if (!(std::isalnum(c) || c == '_' || c == '.'))
        {
            CApiErrors::setLastWarningOrError(functionName, SIM_ERROR_INVALID_PROPERTY_NAME);
            return false;
        }
    }
    return true;
}

int simSetBoolProperty_internal(long long int target, const char* ppName, int pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_bool))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_bool))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)&pState, sizeof(int));
            else
            {
                int res = App::setBoolProperty(target, pName.c_str(), pState != 0);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_bool)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetBoolProperty_internal(long long int target, const char* ppName, int* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_bool))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_bool))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == sizeof(int))
                    {
                        pState[0] = ((int*)data)[0];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                bool ppState;
                int res = App::getBoolProperty(target, pName.c_str(), ppState);
                if (res == 1)
                {
                    pState[0] = int(ppState);
                    retVal = 1;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_bool)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetIntProperty_internal(long long int target, const char* ppName, int pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_int))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_int))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)&pState, sizeof(pState));
            else
            {
                int res = App::setIntProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_int)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetIntProperty_internal(long long int target, const char* ppName, int* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_int))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_int))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == sizeof(int))
                    {
                        pState[0] = ((int*)data)[0];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getIntProperty(target, pName.c_str(), pState[0]);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_int)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetLongProperty_internal(long long int target, const char* ppName, long long int pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_long))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_long))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)&pState, sizeof(pState));
            else
            {
                int res = App::setLongProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_long)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetLongProperty_internal(long long int target, const char* ppName, long long int* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_long))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_long))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == sizeof(long long int))
                    {
                        pState[0] = ((long long int*)data)[0];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getLongProperty(target, pName.c_str(), pState[0]);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_long)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetFloatProperty_internal(long long int target, const char* ppName, double pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_float))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_float))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)&pState, sizeof(double));
            else
            {
                int res = App::setFloatProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_float)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetFloatProperty_internal(long long int target, const char* ppName, double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_float))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_float))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == sizeof(double))
                    {
                        pState[0] = ((double*)data)[0];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getFloatProperty(target, pName.c_str(), pState[0]);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_float)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetStringProperty_internal(long long int target, const char* ppName, const char* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_string))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_string))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), pState, int(strlen(pState)));
            else
            {
                int res = App::setStringProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_string)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

char* simGetStringProperty_internal(long long int target, const char* ppName)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal = nullptr;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_string))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_string))))
            {
                int l;
                char* dat = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (dat != nullptr)
                {
                    retVal = new char[l + 1];
                    for (size_t i = 0; i < l; i++)
                        retVal[i] = dat[i];
                    retVal[l] = 0;
                    delete[] dat;
                }
            }
            else
            {
                std::string s;
                int res = App::getStringProperty(target, pName.c_str(), s);
                if (res == 1)
                {
                    retVal = new char[s.size() + 1];
                    for (size_t i = 0; i < s.size(); i++)
                        retVal[i] = s[i];
                    retVal[s.size()] = 0;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_string)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simSetTableProperty_internal(long long int target, const char* ppName, const char* buffer, int bufferL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_table))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_table))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), buffer, bufferL);
            else
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PROPERTY_NAME);
                /*
                int res = App::setTableProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_table)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
                */
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

char* simGetTableProperty_internal(long long int target, const char* ppName, int* bufferL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal = nullptr;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_table))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_table))))
                retVal = simGetBufferProperty_internal(target, pName.c_str(), bufferL);
            else
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PROPERTY_NAME);
                /*
                std::string s;
                int res = App::getStringProperty(target, pName.c_str(), s);
                if (res == 1)
                {
                    retVal = new char[s.size() + 1];
                    for (size_t i = 0; i < s.size(); i++)
                        retVal[i] = s[i];
                    retVal[s.size()] = 0;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_string)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
                */
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simSetBufferProperty_internal(long long int target, const char* ppName, const char* buffer, int bufferL)
{ // this is also called from all other property type setters
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if ((std::string(ppName).find("&.") != std::string::npos) || isPropertyNameValid(__func__, ppName))
        {
            int res = App::setBufferProperty(target, ppName, buffer, bufferL);
            if (res == 1)
                retVal = 1;
            else if (res == -2)
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
            else
            {
                std::string pN(ppName);
                for (size_t i = 0; i < propertyTypes.size(); i++)
                    utils::replaceSubstring(pN, propertyTypes[i].second.c_str(), "");
                int info;
                std::string infoTxt;
                int p = App::getPropertyInfo(target, pN.c_str(), info, infoTxt, false);
                if (p < 0)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                else if ((p & 0xff) == sim_propertytype_buffer)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                else
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

char* simGetBufferProperty_internal(long long int target, const char* ppName, int* bufferL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal = nullptr;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string b;
            int res = App::getBufferProperty(target, ppName, b);
            if (res == 1)
            {
                retVal = new char[b.size()];
                for (size_t i = 0; i < b.size(); i++)
                    retVal[i] = b[i];
                bufferL[0] = int(b.size());
            }
            else if (res == -2)
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
            else
            {
                std::string pN(ppName);
                for (size_t i = 0; i < propertyTypes.size(); i++)
                    utils::replaceSubstring(pN, propertyTypes[i].second.c_str(), "");
                int info;
                std::string infoTxt;
                int p = App::getPropertyInfo(target, pN.c_str(), info, infoTxt, false);
                if (p < 0)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                else if ((p & 0xff) == sim_propertytype_buffer)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                else
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simSetIntArray2Property_internal(long long int target, const char* ppName, const int* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_intarray2))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_intarray2))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 2 * sizeof(int));
            else
            {
                int res = App::setIntArray2Property(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_intarray2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetIntArray2Property_internal(long long int target, const char* ppName, int* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_intarray2))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_intarray2))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 2 * sizeof(int))
                    {
                        for (size_t i = 0; i < 2; i++)
                            pState[i] = ((int*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getIntArray2Property(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_intarray2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetVector2Property_internal(long long int target, const char* ppName, const double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_vector2))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_vector2))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 2 * sizeof(double));
            else
            {
                int res = App::setVector2Property(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_vector2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetVector2Property_internal(long long int target, const char* ppName, double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_vector2))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_vector2))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 2 * sizeof(double))
                    {
                        for (size_t i = 0; i < 2; i++)
                            pState[i] = ((double*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getVector2Property(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_vector2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetVector3Property_internal(long long int target, const char* ppName, const double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_vector3))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_vector3))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 3 * sizeof(double));
            else
            {
                C3Vector v(pState);
                int res = App::setVector3Property(target, pName.c_str(), v);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_vector3)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetVector3Property_internal(long long int target, const char* ppName, double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_vector3))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_vector3))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 3 * sizeof(double))
                    {
                        for (size_t i = 0; i < 3; i++)
                            pState[i] = ((double*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                C3Vector v;
                int res = App::getVector3Property(target, pName.c_str(), v);
                if (res == 1)
                {
                    v.getData(pState);
                    retVal = 1;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_vector3)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetQuaternionProperty_internal(long long int target, const char* ppName, const double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_quaternion))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_quaternion))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 4 * sizeof(double));
            else
            {
                C4Vector q(pState, true);
                int res = App::setQuaternionProperty(target, pName.c_str(), q);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_quaternion)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetQuaternionProperty_internal(long long int target, const char* ppName, double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_quaternion))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_quaternion))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 4 * sizeof(double))
                    {
                        for (size_t i = 0; i < 4; i++)
                            pState[i] = ((double*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                C4Vector q;
                int res = App::getQuaternionProperty(target, pName.c_str(), q);
                if (res == 1)
                {
                    q.getData(pState, true);
                    retVal = 1;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_quaternion)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetPoseProperty_internal(long long int target, const char* ppName, const double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_pose))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_pose))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 7 * sizeof(double));
            else
            {
                C7Vector p;
                p.setData(pState, true);
                int res = App::setPoseProperty(target, pName.c_str(), p);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_pose)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetPoseProperty_internal(long long int target, const char* ppName, double* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_pose))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_pose))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 7 * sizeof(double))
                    {
                        for (size_t i = 0; i < 7; i++)
                            pState[i] = ((double*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                C7Vector p;
                int res = App::getPoseProperty(target, pName.c_str(), p);
                if (res == 1)
                {
                    p.getData(pState, true);
                    retVal = 1;
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_pose)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetColorProperty_internal(long long int target, const char* ppName, const float* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_color))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_color))))
                retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)pState, 3 * sizeof(float));
            else
            {
                int res = App::setColorProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_color)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int simGetColorProperty_internal(long long int target, const char* ppName, float* pState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_color))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_color))))
            {
                int l;
                const char* data = simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (data != nullptr)
                {
                    if (l == 3 * sizeof(float))
                    {
                        for (size_t i = 0; i < 3; i++)
                            pState[i] = ((float*)data)[i];
                        retVal = 1;
                    }
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_IS_CORRUPT);
                    delete[] data;
                }
            }
            else
            {
                int res = App::getColorProperty(target, pName.c_str(), pState);
                if (res == 1)
                    retVal = 1;
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if (p == sim_propertytype_color)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simSetFloatArrayProperty_internal(long long int target, const char* ppName, const double* v, int vL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            if (vL >= 0)
            {
                std::string pName(ppName);
                if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_floatarray))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_floatarray))))
                    retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)v, vL * sizeof(double));
                else
                {
                    int res = App::setFloatArrayProperty(target, pName.c_str(), v, vL);
                    if (res == 1)
                        retVal = 1;
                    else if (res == -2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                    else
                    {
                        int info;
                        std::string infoTxt;
                        int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                        if (p < 0)
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                        else if ((p & 0xff) == sim_propertytype_floatarray)
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                        else
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                    }
                }
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_INVALID_SIZE);
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

double* simGetFloatArrayProperty_internal(long long int target, const char* ppName, int* vL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        double* retVal = nullptr;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_floatarray))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_floatarray))))
            {
                int l;
                retVal = (double*)simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (retVal != nullptr)
                    vL[0] = l / sizeof(double);
            }
            else
            {
                std::vector<double> v;
                int res = App::getFloatArrayProperty(target, pName.c_str(), v);
                if (res == 1)
                {
                    retVal = new double[v.size()];
                    for (size_t i = 0; i < v.size(); i++)
                        retVal[i] = v[i];
                    vL[0] = int(v.size());
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_floatarray)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simSetIntArrayProperty_internal(long long int target, const char* ppName, const int* v, int vL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1;
        if (isPropertyNameValid(__func__, ppName)) // only when writing data, we still want to read legacy data
        {
            if (vL >= 0)
            {
                std::string pName(ppName);
                if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_intarray))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_intarray))))
                    retVal = simSetBufferProperty_internal(target, pName.c_str(), (char*)v, vL * sizeof(int));
                else
                {
                    int res = App::setIntArrayProperty(target, pName.c_str(), v, vL);
                    if (res == 1)
                        retVal = 1;
                    else if (res == -2)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                    else
                    {
                        int info;
                        std::string infoTxt;
                        int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                        if (p < 0)
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                        else if ((p & 0xff) == sim_propertytype_intarray)
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_WRITTEN);
                        else
                            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                    }
                }
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_INVALID_SIZE);
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return -1;
}

int* simGetIntArrayProperty_internal(long long int target, const char* ppName, int* vL)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int* retVal = nullptr;
        // should always pass when reading, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            std::string pName(ppName);
            if ((utils::replaceSubstringStart(pName, CUSTOMDATAPREFIX, STRCONCAT(CUSTOMDATAPREFIX, proptypetag_intarray))) || (utils::replaceSubstringStart(pName, SIGNALPREFIX, STRCONCAT(SIGNALPREFIX, proptypetag_intarray))))
            {
                int l;
                retVal = (int*)simGetBufferProperty_internal(target, pName.c_str(), &l);
                if (retVal != nullptr)
                    vL[0] = l / sizeof(int);
            }
            else
            {
                std::vector<int> v;
                int res = App::getIntArrayProperty(target, pName.c_str(), v);
                if (res == 1)
                {
                    retVal = new int[v.size()];
                    for (size_t i = 0; i < v.size(); i++)
                        retVal[i] = v[i];
                    vL[0] = int(v.size());
                }
                else if (res == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, pName.c_str(), info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else if ((p & 0xff) == sim_propertytype_intarray)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_READ);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_TYPE_MISMATCH);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simRemoveProperty_internal(long long int target, const char* ppName)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            retVal = App::removeProperty(target, ppName);
            if (retVal != 1)
            {
                if (retVal == -2)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
                else
                {
                    int info;
                    std::string infoTxt;
                    int p = App::getPropertyInfo(target, ppName, info, infoTxt, false);
                    if (p < 0)
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_UNKNOWN_PROPERTY);
                    else
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PROPERTY_CANNOT_BE_REMOVED);
                }
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

char* simGetPropertyName_internal(long long int target, int index, SPropertyOptions* options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal = nullptr;
        std::string pName;
        std::string appartenance;
        bool staticParsing = false;
        if (options != nullptr)
        {
            if ((options->structSize >= 8) && (options->objectType != -1))
            {
                target = options->objectType;
                staticParsing = true;
            }
            if ((options->structSize >= 24) && (options->prefix != nullptr))
                pName = options->prefix;
        }
        int res = App::getPropertyName(target, index, pName, appartenance, staticParsing);
        if (res == -2)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
        else if ((res == 1) && (pName.size() > 0))
        {
            pName += ",";
            pName += appartenance;
            retVal = new char[pName.size() + 1];
            for (size_t i = 0; i < pName.size(); i++)
                retVal[i] = pName[i];
            retVal[pName.size()] = 0;
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return nullptr;
}

int simGetPropertyInfo_internal(long long int target, const char* ppName, SPropertyInfo* infos, SPropertyOptions* options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        // should always pass, (for legacy data names) if (isPropertyNameValid(__func__, ppName))
        {
            bool staticParsing = false;
            bool shortInfoTxt = false;
            std::string infoTxt;
            if (options != nullptr)
            {
                if ((options->structSize >= 12) && options->shortInfoTxt)
                    infoTxt = "s";
                if ((options->structSize >= 8) && (options->objectType != -1))
                {
                    target = options->objectType;
                    staticParsing = true;
                }
            }
            int res = App::getPropertyInfo(target, ppName, infos->flags, infoTxt, staticParsing);
            if (res == -2)
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TARGET_DOES_NOT_EXIST);
            else if (res >= 0)
            {
                infos->type = res;
                infos->infoTxt = new char[infoTxt.size() + 1];
                std::memcpy(infos->infoTxt, infoTxt.c_str(), infoTxt.size() + 1);
                retVal = 1;
            }
            else
            {
                infos->type = -1; // property does not exist;
                retVal = 0;
            }
        }
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return -1;
}

int simGetObject_internal(const char* objectPath, int index, int proxy, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        CSceneObject* it = nullptr;
        CSceneObject* prox = nullptr;
        if (proxy != -1)
        {
            prox = App::currentWorld->sceneObjects->getObjectFromHandle(proxy);
            if (prox == nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PROXY_OBJECT);
                return (-1);
            }
        }
        const CSceneObject* emittingObj = nullptr;
        if (prox == nullptr)
        {
            if (_currentScriptHandle <= SIM_IDEND_SCENEOBJECT)
            {
                CScript* it = App::currentWorld->sceneObjects->getScriptFromHandle(_currentScriptHandle);
                if (it != nullptr)
                {
                    if (it->scriptObject->getParentIsProxy())
                        emittingObj = it->getParent();
                    else
                        emittingObj = it;
                }
            }
            else
            { // legacy child+customization scripts (and main script)
                int objHandle = App::currentWorld->sceneObjects->embeddedScriptContainer->getObjectHandleFromScriptHandle(_currentScriptHandle);
                emittingObj = App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
            }
        }
        else
            emittingObj = prox;
        it = App::currentWorld->sceneObjects->getObjectFromPath(emittingObj, objectPath, index);

        if (it != nullptr)
            retVal = it->getObjectHandle();
        else
        {
            if ((options & 1) == 0)
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

long long int simGetObjectUid_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        return (it->getObjectUid());
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectFromUid_internal(long long int uid, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromUid(uid);
        if (it != nullptr)
            return (it->getObjectHandle());
        if ((options & 1) == 0)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_INEXISTANT);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetScriptHandleEx_internal(int scriptType, int objectHandle, const char* scriptName)
{ // with new scripts, objectHandle should be -1
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it = nullptr;
        if (scriptType == sim_scripttype_main)
            it = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
        if (scriptType == sim_scripttype_sandbox)
            it = App::worldContainer->sandboxScript;
        if ((scriptType == sim_scripttype_simulation) || (scriptType == sim_scripttype_customization))
        { // deprecated with new scripts
            if ((objectHandle < 0) && (scriptName != nullptr))
                objectHandle = simGetObjectHandleEx_internal(scriptName, -1, -1, 0); // deprecated usage
            it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(scriptType, objectHandle);
            if (it == nullptr)
            {
                CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (obj != nullptr)
                {
                    std::vector<CScriptObject*> scripts;
                    obj->getAttachedScripts(scripts, scriptType, false);
                    if (scripts.size() > 0)
                        it = scripts[0];
                }
            }
        }
        if ((scriptType == sim_scripttype_addon) && (scriptName != nullptr))
            it = App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName);
        if (it == nullptr)
        { // new scripts:
            it = App::currentWorld->sceneObjects->getScriptObjectFromHandle(objectHandle);
            if (it != nullptr)
            {
                if (it->getScriptType() != scriptType)
                    it = nullptr;
            }
            else
            {
                CSceneObject* o = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (o != nullptr)
                {
                    for (size_t i = 0; i < o->getChildCount(); i++)
                    {
                        CSceneObject* c = o->getChildFromIndex(i);
                        if (c->getObjectType() == sim_sceneobject_script)
                        {
                            CScript* s = (CScript*)c;
                            if (s->scriptObject->getScriptType() == scriptType)
                            {
                                it = s->scriptObject;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if ((it != nullptr) && (!it->getFlaggedForDestruction()))
            return (it->getScriptHandle());
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRemoveObjects_internal(const int* objectHandles, int count)
{ // neg. count: delayed removal
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool delayed = (count < 0);
        count = abs(count);

        // Erase the objects:
        std::vector<int> sel;
        for (int i = 0; i < count; i++)
            sel.push_back(objectHandles[size_t(i)]);
        if (App::currentWorld->sceneObjects->eraseObjects(&sel, true, delayed))
            return (1);

        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_FOUND_INVALID_HANDLES);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simRemoveModel_internal(int objectHandle)
{ // -objectHandle-1: delayed removal
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool delayed = (objectHandle < 0);
        if (delayed)
            objectHandle = -objectHandle - 1;

        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_INEXISTANT);
            return (-1);
        }
        if (!it->getModelBase())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_MODEL_BASE);
            return (-1);
        }

        /*        // memorize current selection:
        std::vector<int> initSel;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
*/
        // Erase the objects:
        std::vector<int> sel;
        sel.push_back(objectHandle);
        App::currentWorld->sceneObjects->addModelObjects(sel);
        App::currentWorld->sceneObjects->eraseObjects(&sel, true, delayed);

        /*        // Restore the initial selection:
        App::currentWorld->sceneObjects->deselectObjects();
        for (size_t i = 0; i < initSel.size(); i++)
            App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);
*/
        return ((int)sel.size());
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

char* simGetObjectAlias_internal(int objectHandle, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (nullptr);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string nm;
        if (options == -1)
            nm = it->getObjectAlias(); // just the alias, e.g. "alias"
        if (options == 0)
            nm = it->getObjectAliasAndOrderIfRequired(); // the alias with order, e.g. "alias[0]"
        if (options == 1)
            nm = it->getObjectAlias_shortPath(); // the alias with unique path, short, e.g. "/obj/alias[0]"
        if (options == 2)
            nm = it->getObjectAlias_fullPath(); // the alias with full path, e.g. "/obj/obj2/alias[0]"
        if (options == 3)
        { // just the alias, if unique, e.g. "alias", otherwise the alias with handle, e.g. "alias__42__"
            if (App::currentWorld->sceneObjects->getObjectFromPath(
                    nullptr, (std::string("/") + it->getObjectAlias()).c_str(), 1) == nullptr)
                nm = it->getObjectAlias();
            else
                options = 4;
        }
        if (options == 4)
        { // the alias with object handle, e.g. "alias__42__"
            nm = it->getObjectAlias() + "__";
            nm += std::to_string(it->getObjectHandle());
            nm += "__";
        }
        if (options == 5)
            nm = it->getObjectAlias_printPath(); // the print version, not guaranteed to be unique, e.g.
                                                 // "/obj/.../alias[0]"
        if (options == 6)
            nm = it->getObjectPathAndIndex(0); // the path with index, e.g. "/alias{3}"
        if (options == 7)
            nm = it->getObjectPathAndIndex(1); // the path with index, e.g. "/parentModel{1}/alias{3}"
        if (options == 8)
            nm = it->getObjectPathAndIndex(2); // the path with index, e.g. "/greatParent{0}/parentModel{1}/alias{3}"
        if (options == 9)
            nm = it->getObjectPathAndIndex(
                999); // the path with index, e.g. "/.../.../greatParent{0}/parentModel{1}/alias{3}"
        char* retVal = new char[nm.length() + 1];
        for (size_t i = 0; i < nm.length(); i++)
            retVal[i] = nm[i];
        retVal[nm.length()] = 0;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simSetObjectAlias_internal(int objectHandle, const char* objectAlias, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        App::currentWorld->sceneObjects->setObjectAlias(it, objectAlias, true);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetObjectMatrix_internal(int objectHandle, int relativeToObjectHandle, double* matrix)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = it->getCumulativeTransformation();
        else
        {
            C7Vector relTr;
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                relTr = relObj->getCumulativeTransformation();
            else
                relTr = relObj->getFullCumulativeTransformation();
            tr = relTr.getInverse() * it->getCumulativeTransformation();
        }
        if (inverse)
            tr.inverse();
        tr.getMatrix().getData(matrix);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectMatrix_internal(int objectHandle, int relativeToObjectHandle, const double* matrix)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(matrix, 12))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        if (it->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true, true);
        C4X4Matrix m;
        m.setData(matrix);
        m.M.normalize();
        if (inverse)
            m.inverse();
        CSceneObject* objRel = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (objRel == nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(), m.getTransformation(), false);
        else
        {
            C7Vector relTr;
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                relTr = objRel->getCumulativeTransformation();
            else
                relTr = objRel->getFullCumulativeTransformation();
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(), relTr * m.getTransformation(),
                                                                   false);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectPose_internal(int objectHandle, int relativeToObjectHandle, double* pose)
{
    C_API_START;
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces (default): x y z w

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = it->getCumulativeTransformation();
        else
        {
            C7Vector relTr;
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                relTr = relObj->getCumulativeTransformation();
            else
                relTr = relObj->getFullCumulativeTransformation();
            tr = relTr.getInverse() * it->getCumulativeTransformation();
        }
        if (inverse)
            tr.inverse();
        tr.getData(pose, (handleFlags & sim_handleflag_wxyzquat) == 0);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectPose_internal(int objectHandle, int relativeToObjectHandle, const double* pose)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(pose, 7))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        if (it->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true, true);
        C7Vector tr;
        tr.setData(pose, (handleFlags & sim_handleflag_wxyzquat) == 0);
        tr.Q.normalize();
        if (inverse)
            tr.inverse();
        CSceneObject* objRel = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (objRel == nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(), tr, false);
        else
        {
            C7Vector relTr;
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                relTr = objRel->getCumulativeTransformation();
            else
                relTr = objRel->getFullCumulativeTransformation();
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(), relTr * tr, false);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectPosition_internal(int objectHandle, int relativeToObjectHandle, double* position)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
            if (parent != nullptr)
                relativeToObjectHandle = parent->getObjectHandle();
        }
        if (relativeToObjectHandle != sim_handle_world)
        {
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = it->getCumulativeTransformation();
        else
        {
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr = relTr.getInverse() * it->getCumulativeTransformation();
            }
            else
            {
                if (it->getParent() == relObj)
                    tr = it->getLocalTransformation(); // in case of a series of get/set, not losing precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr = relTr.getInverse() * it->getCumulativeTransformation();
                }
            }
        }
        tr.X.getData(position);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectPosition_internal(int objectHandle, int relativeToObjectHandle, const double* position)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(position, 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
            if (parent != nullptr)
                relativeToObjectHandle = parent->getObjectHandle();
        }
        if (relativeToObjectHandle != sim_handle_world)
        {
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        if (it->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true, true);
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj == nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(), C3Vector(position));
        else
        {
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
            {
                C7Vector absTr(it->getCumulativeTransformation());
                C7Vector relTr(relObj->getCumulativeTransformation());
                C7Vector x(relTr.getInverse() * absTr);
                x.X.setData(position);
                absTr = relTr * x;
                App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(), absTr.X);
            }
            else
            {
                if (it->getParent() == relObj)
                { // special here, in order to not lose precision in a series of get/set
                    C7Vector tr(it->getLocalTransformation());
                    tr.X = position;
                    it->setLocalTransformation(tr);
                }
                else
                {
                    C7Vector absTr(it->getCumulativeTransformation());
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    C7Vector x(relTr.getInverse() * absTr);
                    x.X.setData(position);
                    absTr = relTr * x;
                    App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(), absTr.X);
                }
            }
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectOrientation_internal(int objectHandle, int relativeToObjectHandle, double* eulerAngles)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = it->getCumulativeTransformation();
        else
        {
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr = relTr.getInverse() * it->getCumulativeTransformation();
            }
            else
            {
                C7Vector relTr(relObj->getFullCumulativeTransformation());
                tr = relTr.getInverse() * it->getCumulativeTransformation();
            }
        }
        if (inverse)
            tr.Q.inverse();
        C3Vector(tr.Q.getEulerAngles()).getData(eulerAngles);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectOrientation_internal(int objectHandle, int relativeToObjectHandle, const double* eulerAngles)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(eulerAngles, 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        if (it->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true, true);
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj == nullptr)
        {
            C3Vector eul(eulerAngles);
            if (inverse)
            {
                C4Vector q;
                q.setEulerAngles(eulerAngles);
                q.inverse();
                eul = q.getEulerAngles();
            }
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(), eul);
        }
        else
        {
            C7Vector absTr(it->getCumulativeTransformation());
            C7Vector relTr;
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                relTr = relObj->getCumulativeTransformation();
            else
                relTr = relObj->getFullCumulativeTransformation();
            C7Vector x(relTr.getInverse() * absTr);
            x.Q.setEulerAngles(eulerAngles);
            if (inverse)
                x.Q.inverse();
            absTr = relTr * x;
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),
                                                                          absTr.Q.getEulerAngles());
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointPosition_internal(int objectHandle, double* position)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        position[0] = it->getPosition();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointPosition_internal(int objectHandle, double position)
{ // this should not interrupt a possible kin. joint motion started with sim.setJointTargetPosition or sim.setJointTargetVelocity! (sometimes a joint position is simply set the same (e.g. simIK dependency handling, etc.))
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        // info: do not try to trigger a sysCall_jointCallback call for that function, it really doesn't make sense

        int retVal = 1;

        // on 20.06.2024, from this:
        // it->setPosition(position);
        // it->setKinematicMotionType(0, true);
        // to that:
        if ((it->getKinematicMotionType() & 3) == 0)
            it->setPosition(position);
        else
            retVal = 0;

        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointTargetPosition_internal(int objectHandle, double targetPosition)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        if (it->getJointMode() == sim_jointmode_dynamic)
        {
            it->setTargetPosition(targetPosition);
            it->setKinematicMotionType(0, true); // reset
        }
        else
        {
            if (it->getJointMode() == sim_jointmode_kinematic)
            {
                it->setTargetPosition(targetPosition);
                it->setKinematicMotionType(1, false); // pos
            }
            else
                it->setKinematicMotionType(0, true); // reset
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointTargetPosition_internal(int objectHandle, double* targetPosition)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        targetPosition[0] = it->getTargetPosition();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointTargetVelocity_internal(int objectHandle, double targetVelocity)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        if (it->getJointMode() == sim_jointmode_dynamic)
        {
            it->setTargetVelocity(targetVelocity);
            it->setKinematicMotionType(0, true); // reset
        }
        else
        {
            if (it->getJointMode() == sim_jointmode_kinematic)
            {
                it->setTargetVelocity(targetVelocity);
                it->setKinematicMotionType(2, false); // vel
            }
            else
                it->setKinematicMotionType(0, true); // reset
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointTargetVelocity_internal(int objectHandle, double* targetVelocity)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        targetVelocity[0] = it->getTargetVelocity();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectChildPose_internal(int objectHandle, double* pose)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        C7Vector tr;
        tr.setIdentity();
        if (obj->getObjectType() == sim_sceneobject_joint)
            tr = ((CJoint*)obj)->getIntrinsicTransformation(true);
        if (obj->getObjectType() == sim_sceneobject_forcesensor)
            tr = ((CForceSensor*)obj)->getIntrinsicTransformation(true);
        tr.getData(pose, true);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectChildPose_internal(int objectHandle, const double* pose)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(pose, 7))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (obj->getObjectType() == sim_sceneobject_joint)
        {
            CJoint* it = (CJoint*)obj;
            if (it->getJointType() == sim_joint_spherical)
            {
                C7Vector tr;
                tr.setData(pose, true);
                tr.Q.normalize();
                it->setSphericalTransformation(C4Vector(tr.Q));
                it->setIntrinsicTransformationError(C7Vector::identityTransformation);
            }
        }
        if (obj->getObjectType() == sim_sceneobject_forcesensor)
        {
            CForceSensor* it = (CForceSensor*)obj;
            it->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointInterval_internal(int objectHandle, bool* cyclic, double* interval)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        cyclic[0] = 0;
        if (it->getIsCyclic())
            cyclic[0] = 1;
        it->getInterval(interval[0], interval[1]);
        interval[1] = interval[1] - interval[0];
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointInterval_internal(int objectHandle, bool cyclic, const double* interval)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(interval, 2))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        // Some models need to modify that
        //        if (
        //        App::currentWorld->simulation->isSimulationStopped()||((it->getJointMode()!=sim_jointmode_dynamic)&&(!it->getHybridFunctionality_old()))
        //        )
        {
            double previousPos = it->getPosition();
            it->setIsCyclic(cyclic != 0);
            it->setInterval(interval[0], interval[0] + interval[1]);
            it->setPosition(previousPos);
            return (1);
        }
        //        return(-1);
        //        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectParent_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
        {
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal = -1;
        if (it->getParent() != nullptr)
            retVal = it->getParent()->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectChild_internal(int objectHandle, int index)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        if (objectHandle == sim_handle_scene)
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getOrphanFromIndex(index);
            if (it != nullptr)
                retVal = it->getObjectHandle();
        }
        else
        {
            if (doesObjectExist(__func__, objectHandle))
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (int(it->getChildCount()) > index)
                    retVal = it->getChildFromIndex(index)->getObjectHandle();
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectHierarchyOrder_internal(int objectHandle, int* totalSiblings)
{ // totalSiblings can be nullptr
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        if (totalSiblings != nullptr)
            totalSiblings[0] = 0;
        if (doesObjectExist(__func__, objectHandle))
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            retVal = App::currentWorld->sceneObjects->getObjectSequence(it, totalSiblings);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectHierarchyOrder_internal(int objectHandle, int order)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        if (doesObjectExist(__func__, objectHandle))
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (App::currentWorld->sceneObjects->setObjectSequence(it, order))
                retVal = 1;
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OPERATION_FAILED);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectParent_internal(int objectHandle, int parentObjectHandle, bool keepInPlace)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags = 0;
        if (objectHandle >= 0)
        {
            handleFlags = objectHandle & 0xff00000;
            objectHandle = objectHandle & 0xfffff;
        }
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (parentObjectHandle != -1)
        {
            if (!doesObjectExist(__func__, parentObjectHandle))
                return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CSceneObject* parentIt = App::currentWorld->sceneObjects->getObjectFromHandle(parentObjectHandle);
        CSceneObject* pp = parentIt;
        while (pp != nullptr)
        {
            if (pp == it)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_IS_ANCESTOR_OF_DESIRED_PARENT);
                return (-1);
            }
            pp = pp->getParent();
        }
        if (keepInPlace)
            App::currentWorld->sceneObjects->setObjectParent(it, parentIt, true);
        else
        {
            if (handleFlags & sim_handleflag_assembly)
            { // only assembling
                if (parentIt != nullptr)
                {
                    if (!App::assemble(parentIt->getObjectHandle(), it->getObjectHandle(), false))
                    {
                        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ASSEMBLY);
                        return (-1);
                    }
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ASSEMBLY);
                    return (-1);
                }
            }
            else
                App::currentWorld->sceneObjects->setObjectParent(it, parentIt, false);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetObjectType_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal = it->getObjectType();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointType_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
        {
            return (-1);
        }
        if (!isJoint(__func__, objectHandle))
        {
            return (-1);
        }
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        int retVal = it->getJointType();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simBuildIdentityMatrix_internal(double* matrix)
{
    C_API_START;

    C4X4Matrix m;
    m.setIdentity();
    m.getData(matrix);
    return (1);
}

int simBuildMatrix_internal(const double* position, const double* eulerAngles, double* matrix)
{
    C_API_START;
    if ((!isFloatArrayOk(position, 3)) || (!isFloatArrayOk(eulerAngles, 3)))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C4X4Matrix m;
    m.M.setEulerAngles(C3Vector(eulerAngles));
    m.X.setData(position);
    m.getData(matrix);
    return (1);
}

int simBuildPose_internal(const double* position, const double* eulerAngles, double* pose)
{
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces: x y z w
    C_API_START;
    if ((!isFloatArrayOk(position, 3)) || (!isFloatArrayOk(eulerAngles, 3)))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C7Vector tr(C4Vector(eulerAngles[0], eulerAngles[1], eulerAngles[2]), C3Vector(position));
    tr.getData(pose, true);
    return (1);
}

int simGetEulerAnglesFromMatrix_internal(const double* matrix, double* eulerAngles)
{
    C_API_START;
    if (!isFloatArrayOk(matrix, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C4X4Matrix m;
    m.setData(matrix);
    m.M.normalize();
    m.M.getEulerAngles().getData(eulerAngles);
    return (1);
}

int simInvertMatrix_internal(double* matrix)
{
    C_API_START;
    if (!isFloatArrayOk(matrix, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C4X4Matrix m;
    m.setData(matrix);
    m.M.normalize();
    m.inverse();
    m.getData(matrix);
    return (1);
}

int simInvertPose_internal(double* pose)
{
    C_API_START;
    if (!isFloatArrayOk(pose, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C7Vector p;
    p.setData(pose, true);
    p.inverse();
    p.getData(pose, true);
    return (1);
}

int simMultiplyMatrices_internal(const double* matrixIn1, const double* matrixIn2, double* matrixOut)
{
    C_API_START;
    if (!isFloatArrayOk(matrixIn1, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(matrixIn2, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C4X4Matrix mIn1;
    mIn1.setData(matrixIn1);
    mIn1.M.normalize();
    C4X4Matrix mIn2;
    mIn2.setData(matrixIn2);
    mIn2.M.normalize();
    (mIn1 * mIn2).getData(matrixOut);
    return (1);
}

int simMultiplyPoses_internal(const double* poseIn1, const double* poseIn2, double* poseOut)
{
    C_API_START;
    if (!isFloatArrayOk(poseIn1, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(poseIn2, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C7Vector pIn1;
    pIn1.setData(poseIn1, true);
    pIn1.Q.normalize();
    C7Vector pIn2;
    pIn2.setData(poseIn2, true);
    pIn2.Q.normalize();
    (pIn1 * pIn2).getData(poseOut, true);
    return (1);
}

int simPoseToMatrix_internal(const double* poseIn, double* matrixOut)
{
    C_API_START;
    if (!isFloatArrayOk(poseIn, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C7Vector pIn;
    pIn.setData(poseIn, true);
    pIn.Q.normalize();
    pIn.getMatrix().getData(matrixOut);
    return (1);
}

int simMatrixToPose_internal(const double* matrixIn, double* poseOut)
{
    C_API_START;
    if (!isFloatArrayOk(matrixIn, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C4X4Matrix mIn;
    mIn.setData(matrixIn);
    mIn.M.normalize();
    mIn.getTransformation().getData(poseOut, true);
    return (1);
}

int simInterpolateMatrices_internal(const double* matrixIn1, const double* matrixIn2, double interpolFactor,
                                    double* matrixOut)
{
    C_API_START;
    if (!isFloatArrayOk(matrixIn1, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(matrixIn2, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C4X4Matrix mIn1;
    mIn1.setData(matrixIn1);
    mIn1.M.normalize();
    C4X4Matrix mIn2;
    mIn2.setData(matrixIn2);
    mIn2.M.normalize();
    C7Vector tr;
    tr.buildInterpolation(mIn1.getTransformation(), mIn2.getTransformation(), interpolFactor);
    (tr.getMatrix()).getData(matrixOut);
    return (1);
}

int simInterpolatePoses_internal(const double* poseIn1, const double* poseIn2, double interpolFactor, double* poseOut)
{
    C_API_START;
    if (!isFloatArrayOk(poseIn1, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(poseIn2, 7))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C7Vector pIn1;
    pIn1.setData(poseIn1, true);
    pIn1.Q.normalize();
    C7Vector pIn2;
    pIn2.setData(poseIn2, true);
    pIn2.Q.normalize();
    C7Vector tr;
    tr.buildInterpolation(pIn1, pIn2, interpolFactor);
    tr.getData(poseOut, true);
    return (1);
}

int simTransformVector_internal(const double* matrix, double* vect)
{
    C_API_START;
    if (!isFloatArrayOk(matrix, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    C4X4Matrix m;
    m.setData(matrix);
    m.M.normalize();
    C3Vector v(vect);
    (m * v).getData(vect);
    return (1);
}

int simReservedCommand_internal(int v, int w)
{
    C_API_START;

    if (v == 0)
        VThread::sleep(w);
    if (v == 2)
    {
        App::logMsg(sim_verbosity_errors, "functionality deprecated! (simReservedCommand, v=2)");
        int retVal = App::worldContainer->getModificationFlags(true);
        return (retVal);
    }
    return (CSimFlavor::getIntVal_2int(0, v, w));
}

double simGetSimulationTime_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    return (App::currentWorld->simulation->getSimulationTime());
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1.0);
}

int simGetSimulationState_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = App::currentWorld->simulation->getSimulationState();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

double simGetSystemTime_internal()
{
    return (VDateTime::getTime());
}

int simLoadScene_internal(const char* filename)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SIMULATION_NOT_STOPPED);
            return (-1);
        }

        std::string nm(filename);
        size_t keepCurrentPos = nm.find("@keepCurrent");
        bool keepCurrent = (keepCurrentPos != std::string::npos);
        if (keepCurrent)
            nm.erase(nm.begin() + keepCurrentPos, nm.end());

        if (nm.size() > 0)
        {
            if (keepCurrent)
                CFileOperations::createNewScene(true);
            std::string errorStr;
            if (CFileOperations::loadScene(nm.c_str(), false, nullptr, &lastInfoString, &errorStr))
            {
#ifdef SIM_WITH_GUI
                if (GuiApp::mainWindow != nullptr)
                    GuiApp::mainWindow->refreshDimensions(); // this is important so that the new pages and views are
                                                             // set to the correct dimensions
#endif
                App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__, errorStr.c_str());
                return (-1);
            }
        }
        else
            CFileOperations::createNewScene(keepCurrent);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCloseScene_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SIMULATION_NOT_STOPPED);
            return (-1);
        }
        CFileOperations::closeScene();
        return (App::worldContainer->getCurrentWorldIndex());
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simLoadModel_internal(const char* filename)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string nm(filename);
        size_t atCopyPos = nm.find("@copy"); // deprecated option
        bool forceAsCopy = (atCopyPos != std::string::npos);
        if (forceAsCopy)
            nm.erase(nm.begin() + atCopyPos, nm.end());

        std::string errorStr;
        if (CFileOperations::loadModel(nm.c_str(), false, true, nullptr, false, forceAsCopy, &lastInfoString,
                                       &errorStr))
        {
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag();
#endif
            return (App::currentWorld->sceneObjects->getLastSelectionHandle());
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__, errorStr.c_str());
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

char* simGetSimulatorMessage_internal(int* messageID, int* auxiliaryData, int* returnedDataSize)
{
    C_API_START;

    char* retVal =
        App::worldContainer->simulatorMessageQueue->extractOneCommand(messageID[0], auxiliaryData, returnedDataSize[0]);
    return (retVal);
}

int simSaveScene_internal(const char* filename)
{
    C_API_START;

    if (App::currentWorld->environment->getSceneLocked())
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCENE_LOCKED);
        return (-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::currentWorld->environment->getRequestFinalSave())
            App::currentWorld->environment->setSceneLocked(); // silent locking!

        std::string errorStr;
        if (CFileOperations::saveScene(filename, false, false, nullptr, &lastInfoString, &errorStr))
        {
#ifdef SIM_WITH_GUI
            GuiApp::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
#endif
            // 21.07.2023
            // CFileOperations::_addToRecentlyOpenedScenes(App::currentWorld->mainSettings_old->getScenePathAndName());
            App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
            return (1);
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__, errorStr.c_str());
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simSaveModel_internal(int baseOfModelHandle, const char* filename)
{
    C_API_START;

    if (App::currentWorld->environment->getSceneLocked())
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCENE_LOCKED);
        return (-1);
    }
    if (!doesObjectExist(__func__, baseOfModelHandle))
        return (-1);
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(baseOfModelHandle);
    if (!it->getModelBase())
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_MODEL_BASE);
        return (-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
        std::string errorStr;
        if (CFileOperations::saveModel(baseOfModelHandle, filename, false, nullptr, &lastInfoString, &errorStr))
        {
            App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
            return (1);
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__, errorStr.c_str());
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simDoesFileExist_internal(const char* filename)
{
    C_API_START;

    if (!VFile::doesFileExist(filename))
        return (0);
    return (1);
}

int* simGetObjectSel_internal(int* cnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        const std::vector<int>* handles = App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr();
        int* retVal = new int[handles->size()];
        for (size_t i = 0; i < handles->size(); i++)
            retVal[i] = handles->at(i);
        cnt[0] = int(handles->size());
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simSetObjectSel_internal(const int* handles, int cnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::currentWorld->sceneObjects->deselectObjects();
        if ((handles != nullptr) && (cnt > 0))
        {
            for (int i = 0; i < cnt; i++)
                App::currentWorld->sceneObjects->addObjectToSelection(handles[i]);
        }
        return (int(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()->size()));
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simHandleProximitySensor_internal(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                      double* normalVector)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (sensorHandle >= 0)
        { // handle just one sensor (this is explicit handling)
            CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
            if (it == nullptr)
            {
                return (-1);
            }
            else
            {
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return (-1);
                }

                int retVal = 0;
                C3Vector smallest;
                double smallestL = DBL_MAX;
                int detectedObj;
                C3Vector detectedSurf;
                bool detected = it->handleSensor(false, detectedObj, detectedSurf);
                if (detected)
                {
                    smallest = it->getDetectedPoint();
                    smallestL = smallest.getLength();
                    retVal = 1;
                    if (detectedPoint != nullptr)
                    {
                        detectedPoint[0] = smallest(0);
                        detectedPoint[1] = smallest(1);
                        detectedPoint[2] = smallest(2);
                        detectedPoint[3] = smallestL;
                    }
                    if (detectedObjectHandle != nullptr)
                        detectedObjectHandle[0] = detectedObj;
                    if (normalVector != nullptr)
                    {
                        normalVector[0] = detectedSurf(0);
                        normalVector[1] = detectedSurf(1);
                        normalVector[2] = detectedSurf(2);
                    }
                }
                return (retVal);
            }
        }
        else
        { // handle several sensors at once (with sim_handle_all or sim_handle_all_except_explicit
            int retVal = 0;
            C3Vector allSmallest;
            int detectedObjectID = -1;
            C3Vector detectedSurfaceNormal;
            double allSmallestL = DBL_MAX;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_proximitysensor); i++)
            {
                int detectedObj;
                C3Vector detectedSurf;
                CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromIndex(i);
                bool detected = false;
                detected = it->handleSensor(sensorHandle == sim_handle_all_except_explicit, detectedObj, detectedSurf);
                if (detected)
                {
                    C3Vector smallest(it->getDetectedPoint());
                    double smallestL = smallest.getLength();

                    if (smallestL < allSmallestL)
                    {
                        allSmallest = smallest;
                        allSmallestL = smallestL;
                        detectedObjectID = detectedObj;
                        detectedSurfaceNormal = detectedSurf;
                        retVal = 1;
                    }
                }
            }
            if (retVal == 1)
            {
                if (detectedPoint != nullptr)
                {
                    detectedPoint[0] = allSmallest(0);
                    detectedPoint[1] = allSmallest(1);
                    detectedPoint[2] = allSmallest(2);
                    detectedPoint[3] = allSmallestL;
                }
                if (detectedObjectHandle != nullptr)
                    detectedObjectHandle[0] = detectedObjectID;
                if (normalVector != nullptr)
                {
                    normalVector[0] = detectedSurfaceNormal(0);
                    normalVector[1] = detectedSurfaceNormal(1);
                    normalVector[2] = detectedSurfaceNormal(2);
                }
            }
            return (retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simReadProximitySensor_internal(int sensorHandle, double* detectedPoint, int* detectedObjectHandle,
                                    double* normalVector)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isSensor(__func__, sensorHandle))
            return (-1);
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        C3Vector dPt;
        int dObjHandle;
        C3Vector nVect;
        int retVal = it->readSensor(dPt, dObjHandle, nVect);
        if (retVal == 1)
        {
            if (detectedPoint != nullptr)
            {
                dPt.getData(detectedPoint);
                detectedPoint[3] = dPt.getLength();
            }
            if (detectedObjectHandle != nullptr)
                detectedObjectHandle[0] = dObjHandle;
            if (normalVector != nullptr)
                nVect.getData(normalVector);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simHandleDynamics_internal(double deltaTime)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->dynamicsContainer->handleDynamics(deltaTime);
        CApiErrors::clearThreadBasedFirstCapiErrorAndWarning_old();
        if ((!App::currentWorld->dynamicsContainer->isWorldThere()) && App::currentWorld->dynamicsContainer->getDynamicsEnabled())
        {
            App::currentWorld->dynamicsContainer->markForWarningDisplay_physicsEngineNotSupported();
            return (0);
        }
        return (App::worldContainer->pluginContainer->dyn_getDynamicStepDivider());
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simResetScript_internal(int scriptHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle == sim_handle_all)
        {
            App::currentWorld->sceneObjects->embeddedScriptContainer->killAllSimulationLuaStates();
            return (1);
        }
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
        if (it == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCRIPT_INEXISTANT);
            return (-1);
        }
        it->resetScript();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simResetProximitySensor_internal(int sensorHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((sensorHandle != sim_handle_all) && (sensorHandle != sim_handle_all_except_explicit))
        {
            if (!isSensor(__func__, sensorHandle))
            {
                return (-1);
            }
        }
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_proximitysensor); i++)
        {
            CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromIndex(i);
            if (sensorHandle >= 0)
            { // Explicit handling
                it = (CProxSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(sensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return (-1);
                }
                it->resetSensor(false);
                break;
            }
            else
                it->resetSensor(sensorHandle == sim_handle_all_except_explicit);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckProximitySensor_internal(int sensorHandle, int entityHandle, double* detectedPoint)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isSensor(__func__, sensorHandle))
            return (-1);
        if ((entityHandle != sim_handle_all) && (!doesEntityExist(__func__, entityHandle)))
            return (-1);
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        int options = 0;
        if (it->getFrontFaceDetection())
            options = options | 1;
        if (it->getBackFaceDetection())
            options = options | 2;
        if (!it->getExactMode())
            options = options | 4;
        if (it->getAllowedNormal() > 0.0)
            options = options | 8;
        int retVal = simCheckProximitySensorEx_internal(sensorHandle, entityHandle, options, DBL_MAX,
                                                        it->getAllowedNormal(), detectedPoint, nullptr, nullptr);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckProximitySensorEx_internal(int sensorHandle, int entityHandle, int detectionMode, double detectionThreshold,
                                       double maxAngle, double* detectedPoint, int* detectedObjectHandle,
                                       double* normalVector)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isSensor(__func__, sensorHandle))
            return (-1);
        if ((entityHandle != sim_handle_all) && (!doesEntityExist(__func__, entityHandle)))
            return (-1);
        if (entityHandle == sim_handle_all)
            entityHandle = -1;
        if (!App::currentWorld->mainSettings_old->proximitySensorsEnabled)
            return (0);

        bool frontFace = SIM_IS_BIT_SET(detectionMode, 0);
        bool backFace = SIM_IS_BIT_SET(detectionMode, 1);
        bool fastDetection = SIM_IS_BIT_SET(detectionMode, 2);
        bool limitedAngle = SIM_IS_BIT_SET(detectionMode, 3);
        if (!limitedAngle)
            maxAngle = 0.0;
        if (!(frontFace || backFace))
            frontFace = true;
        if (detectionThreshold < 0.0)
            detectionThreshold = 0.0;
        tt::limitValue(0.0, piValD2, maxAngle);
        int detectedObj;
        C3Vector dPoint;
        double minThreshold = -1.0;
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        if ((it != nullptr) && (it->convexVolume->getSmallestDistanceAllowed() > 0.0))
            minThreshold = it->convexVolume->getSmallestDistanceAllowed();
        C3Vector normV;
        bool returnValue;
        returnValue = CProxSensorRoutine::detectEntity(sensorHandle, entityHandle, !fastDetection, maxAngle > 0.0,
                                                       maxAngle, dPoint, detectionThreshold, frontFace, backFace,
                                                       detectedObj, minThreshold, normV, true);

        if (returnValue)
        {
            if (detectedPoint != nullptr)
            {
                detectedPoint[0] = dPoint(0);
                detectedPoint[1] = dPoint(1);
                detectedPoint[2] = dPoint(2);
                detectedPoint[3] = detectionThreshold;
            }
            if (detectedObjectHandle != nullptr)
                detectedObjectHandle[0] = detectedObj;
            if (normalVector != nullptr)
            {
                normalVector[0] = normV(0);
                normalVector[1] = normV(1);
                normalVector[2] = normV(2);
            }
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckProximitySensorEx2_internal(int sensorHandle, double* vertexPointer, int itemType, int itemCount,
                                        int detectionMode, double detectionThreshold, double maxAngle,
                                        double* detectedPoint, double* normalVector)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isSensor(__func__, sensorHandle))
        {
            return (-1);
        }

        if (!App::currentWorld->mainSettings_old->proximitySensorsEnabled)
        {
            return (0);
        }
        bool frontFace = SIM_IS_BIT_SET(detectionMode, 0);
        bool backFace = SIM_IS_BIT_SET(detectionMode, 1);
        bool fastDetection = SIM_IS_BIT_SET(detectionMode, 2);
        bool limitedAngle = SIM_IS_BIT_SET(detectionMode, 3);
        if (!(frontFace || backFace))
            frontFace = true;
        if (detectionThreshold < 0.0)
            detectionThreshold = 0.0;
        tt::limitValue(0.0, piValD2, maxAngle);
        C3Vector dPoint;
        double minThreshold = -1.0;
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        if ((it != nullptr) && (it->convexVolume->getSmallestDistanceAllowed() > 0.0))
            minThreshold = it->convexVolume->getSmallestDistanceAllowed();
        C3Vector normV;
        bool returnValue;
        returnValue = CProxSensorRoutine::detectPrimitive(sensorHandle, vertexPointer, itemType, itemCount,
                                                          !fastDetection, limitedAngle, maxAngle, dPoint,
                                                          detectionThreshold, frontFace, backFace, minThreshold, normV);
        if (returnValue)
        {
            if (detectedPoint != nullptr)
            {
                detectedPoint[0] = dPoint(0);
                detectedPoint[1] = dPoint(1);
                detectedPoint[2] = dPoint(2);
                detectedPoint[3] = detectionThreshold;
            }
            if (normalVector != nullptr)
            {
                normalVector[0] = normV(0);
                normalVector[1] = normV(1);
                normalVector[2] = normV(2);
            }
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRegisterScriptCallbackFunction_internal(const char* func, const char* reserved_setToNull,
                                               void (*callBack)(struct SScriptCallBack* cb))
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        CPlugin* plug = App::worldContainer->pluginContainer->getCurrentPlugin();
        /*
        if (plug!=nullptr)
        {
            printf("PluginName: %s\n",plug->getName().c_str());
            if (plug->isLegacyPlugin())
                printf("    legacy plugin\n");
            else
                printf("    new plugin\n");
        }
        */

        if ((plug != nullptr) && (!plug->isLegacyPlugin()))
        { // new plugins. e.g. 'createGroup', and not 'simIK.createGroup'
            if (callBack != nullptr)
            {
                if (plug->getPluginCallbackContainer()->addCallback(func, callBack))
                    retVal = 1;
                else
                    retVal = 0;
                if ((strcmp(func, "vhacd") == 0) && (retVal == 0))
                    printf("failed adding vhacd callback\n");
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CALLBACK_IS_NULL);
        }
        else
        { // old plugins
            if (strcmp(func, "vhacd") == 0)
                printf("failed registering vhacd\n");
            // App::worldContainer->pluginContainer->printPluginStack();
            std::string funcName;
            std::string pluginName;

            std::string funcNameAtPluginNm(func);
            size_t p = funcNameAtPluginNm.find('@');
            if (p != std::string::npos)
            {
                pluginName.assign(funcNameAtPluginNm.begin() + p + 1, funcNameAtPluginNm.end());
                funcName.assign(funcNameAtPluginNm.begin(), funcNameAtPluginNm.begin() + p);
            }
            if (pluginName.size() < 1)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_MISSING_PLUGIN_NAME);
                return (-1);
            }

            retVal = 1;
            if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomFunction(func))
                retVal = 0; // that function already existed. We remove it and replace it!
            CScriptCustomFunction* newFunction = new CScriptCustomFunction(func, reserved_setToNull, callBack, true);
            if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomFunction(newFunction))
            {
                delete newFunction;
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
                retVal = -1;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRegisterScriptVariable_internal(const char* var, const char* val, int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        CPlugin* plug = App::worldContainer->pluginContainer->getCurrentPlugin();
        if ((plug != nullptr) && (!plug->isLegacyPlugin()))
        { // new plugins. e.g. 'method_jacobian_transpose', not 'simIK.method_jacobian_transpose'
            if (plug->getPluginVariableContainer()->addVariable(var, val, stackHandle))
                retVal = 1;
            else
                retVal = 0;
        }
        else
        { // old plugins
            retVal = 1;
            if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomVariable(var))
                retVal = 0; // that variable already existed. We remove it and replace it!
            if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomVariable(var, val, stackHandle))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CUSTOM_LUA_VAR_COULD_NOT_BE_REGISTERED);
                retVal = -1;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRegisterScriptFuncHook_internal(int scriptHandle, const char* funcToHook, const char* userFunction,
                                       bool executeBefore, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
        if (it != nullptr)
            retVal = it->registerFunctionHook(funcToHook, userFunction, executeBefore);
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCRIPT_INEXISTANT);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

void* simCreateBuffer_internal(int size)
{
    C_API_START;

    void* retVal = (void*)new char[size];
    return (retVal);
}

int simReleaseBuffer_internal(const void* buffer)
{
    C_API_START;

    delete[](char*) buffer;
    return (1);
}

int simCheckCollision_internal(int entity1Handle, int entity2Handle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((!doesEntityExist(__func__, entity1Handle)) ||
            ((entity2Handle != sim_handle_all) && (!doesEntityExist(__func__, entity2Handle))))
            return (-1);
        if (entity2Handle == sim_handle_all)
            entity2Handle = -1;

        if (!App::currentWorld->mainSettings_old->collisionDetectionEnabled)
            return (0);
        bool returnValue =
            CCollisionRoutine::doEntitiesCollide(entity1Handle, entity2Handle, nullptr, true, true, nullptr);
        return (returnValue);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckCollisionEx_internal(int entity1Handle, int entity2Handle, double** intersectionSegments)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((!doesEntityExist(__func__, entity1Handle)) ||
            ((entity2Handle != sim_handle_all) && (!doesEntityExist(__func__, entity2Handle))))
        {
            return (-1);
        }
        if (entity2Handle == sim_handle_all)
            entity2Handle = -1;

        if (!App::currentWorld->mainSettings_old->collisionDetectionEnabled)
        {
            return (0);
        }

        std::vector<double> intersect;
        CCollisionRoutine::doEntitiesCollide(entity1Handle, entity2Handle, &intersect, true, true, nullptr);
        if ((intersectionSegments != nullptr) && (intersect.size() != 0))
        {
            intersectionSegments[0] = new double[intersect.size()];
            for (int i = 0; i < int(intersect.size()); i++)
                (*intersectionSegments)[i] = intersect[i];
        }
        return ((int)intersect.size() / 6);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckDistance_internal(int entity1Handle, int entity2Handle, double threshold, double* distanceData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((!doesEntityExist(__func__, entity1Handle)) ||
            ((entity2Handle != sim_handle_all) && (!doesEntityExist(__func__, entity2Handle))))
        {
            return (-1);
        }
        if (entity2Handle == sim_handle_all)
            entity2Handle = -1;

        if (!App::currentWorld->mainSettings_old->distanceCalculationEnabled)
            return (0);

        int buffer[4];
        App::currentWorld->cacheData->getCacheDataDist(entity1Handle, entity2Handle, buffer);
        if (threshold <= 0.0)
            threshold = DBL_MAX;
        bool result = CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(
            entity1Handle, entity2Handle, threshold, distanceData, buffer, buffer + 2, true, true);
        App::currentWorld->cacheData->setCacheDataDist(entity1Handle, entity2Handle, buffer);
        if (result)
            return (1);
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetSimulationTimeStep_internal(double timeStep)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SIMULATION_NOT_STOPPED);
            return (-1);
        }
        App::currentWorld->simulation->setTimeStep(timeStep);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

double simGetSimulationTimeStep_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    return (App::currentWorld->simulation->getTimeStep());
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1.0);
}

int simGetRealTimeSimulation_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->simulation->getIsRealTimeSimulation())
        {
            return (1);
        }
        else
        {
            return (0);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simStartSimulation_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            App::currentWorld->simulation->startOrResumeSimulation();
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simStopSimulation_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            App::currentWorld->simulation->incrementStopRequestCounter();
            App::currentWorld->simulation->stopSimulation();
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simPauseSimulation_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->simulation->isSimulationRunning())
        {
            App::currentWorld->simulation->pauseSimulation();
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simHandleGraph_internal(int graphHandle, double simulationTime)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((graphHandle != sim_handle_all) && (graphHandle != sim_handle_all_except_explicit))
        {
            if (!isGraph(__func__, graphHandle))
                return (-1);
        }
        if (graphHandle < 0)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_graph); i++)
            {
                CGraph* it = App::currentWorld->sceneObjects->getGraphFromIndex(i);
                if ((!it->getExplicitHandling()) || (graphHandle == sim_handle_all))
                    it->addNextPoint(simulationTime);
            }
        }
        else
        { // explicit handling
            CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return (-1);
            }
            it->addNextPoint(simulationTime);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simResetGraph_internal(int graphHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((graphHandle != sim_handle_all) && (graphHandle != sim_handle_all_except_explicit))
        {
            if (!isGraph(__func__, graphHandle))
                return (-1);
        }
        if (graphHandle < 0)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_graph); i++)
            {
                CGraph* it = App::currentWorld->sceneObjects->getGraphFromIndex(i);
                if ((!it->getExplicitHandling()) || (graphHandle == sim_handle_all))
                    it->resetGraph();
            }
        }
        else
        { // explicit handling
            CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
            //            if (!it->getExplicitHandling())
            //            {
            //                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
            //                return(-1);
            //            }
            it->resetGraph();
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAddGraphStream_internal(int graphHandle, const char* streamName, const char* unitStr, int options,
                               const float* color, double cyclicRange)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        if (strlen(streamName) == 0)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_EMPTY_STRING_NOT_ALLOWED);
            return (-1);
        }
        std::string nm(streamName);
        tt::removeIllegalCharacters(nm, false);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphDataStream* str =
            new CGraphDataStream(nm.c_str(), unitStr, options, color, cyclicRange, _currentScriptHandle);
        int retVal = it->addOrUpdateDataStream(str);
        if (retVal == -1)
        {
            delete str;
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simDestroyGraphCurve_internal(int graphHandle, int curveId)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (curveId == -1)
        {
            it->removeAllStreamsAndCurves();
            it->removeAllStreamsAndCurves_old();
            return (1);
        }
        else
        {
            if (it->removeGraphCurve(curveId))
                return (1);
            if (it->removeGraphDataStream(curveId))
                return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_CURVE_ID);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetGraphStreamTransformation_internal(int graphHandle, int streamId, int trType, double mult, double off,
                                             int movingAvgPeriod)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (it->setDataStreamTransformation(streamId, trType, mult, off, movingAvgPeriod))
            return (1);
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_CURVE_ID);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simDuplicateGraphCurveToStatic_internal(int graphHandle, int curveId, const char* curveName)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        std::string nm(curveName);
        tt::removeIllegalCharacters(nm, false);
        int retVal = it->duplicateCurveToStatic(curveId, nm.c_str());
        if (retVal == -1)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_CURVE_ID);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAddGraphCurve_internal(int graphHandle, const char* curveName, int dim, const int* streamIds,
                              const double* defaultValues, const char* unitStr, int options, const float* color,
                              int curveWidth)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        if (strlen(curveName) == 0)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_EMPTY_STRING_NOT_ALLOWED);
            return (-1);
        }
        std::string nm(curveName);
        tt::removeIllegalCharacters(nm, false);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphCurve* curve = new CGraphCurve(dim, streamIds, defaultValues, nm.c_str(), unitStr, options, color,
                                             curveWidth, _currentScriptHandle);
        int retVal = it->addOrUpdateCurve(curve);
        if (retVal == -1)
        {
            delete curve;
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetGraphStreamValue_internal(int graphHandle, int streamId, double value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__, graphHandle))
            return (-1);
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (it->setNextValueToInsert(streamId, value))
            return (1);
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_CURVE_ID);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

char* simGetPluginName_internal(int index, unsigned char* setToNull)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug = App::worldContainer->pluginContainer->getPluginFromIndex(index);
        if (plug == nullptr)
            return (nullptr);
        char* name = new char[plug->getName().length() + 1];
        for (size_t i = 0; i < plug->getName().length(); i++)
            name[i] = plug->getName()[i];
        name[plug->getName().length()] = 0;
        if (setToNull != nullptr)
            setToNull[0] = (unsigned char)plug->getPluginVersion();
        return (name);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simAddLog_internal(const char* pluginName, int verbosityLevel, const char* logMsg)
{ // keep this as simple as possible (no trace, no thread checking). For now
    int retVal = 0;
    if (logMsg == nullptr)
    {
#ifdef SIM_WITH_GUI
        GuiApp::clearStatusbar();
#endif
        retVal = 1;
    }
    else
    {
        if (strcmp(logMsg, "starting a remote API server on port 19997") == 0) // suppress that msg from legacy remote API
            retVal = 1;
        else if (App::logPluginMsg(pluginName, verbosityLevel, logMsg))
            retVal = 1;
    }
    return (retVal);
}

int simSetNavigationMode_internal(int navigationMode)
{
#ifdef SIM_WITH_GUI
    GuiApp::setMouseMode(navigationMode);
#endif
    return (1);
}

int simGetNavigationMode_internal()
{
    int retVal = -1;
#ifdef SIM_WITH_GUI
    retVal = GuiApp::getMouseMode();
#endif
    return (retVal);
}

int simSetPage_internal(int index)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        App::currentWorld->pageContainer->setActivePage(index);
#endif
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetPage_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        int retVal = App::currentWorld->pageContainer->getActivePageIndex();
        return (retVal);
#else
        return (0);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCopyPasteObjects_internal(int* objectHandles, int objectCount, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return (-1);
        int retVal = 0;
        if (objectCount > 0)
        {
            // memorize current selection:
            std::vector<int> initSel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                App::currentWorld->sceneObjects->getObjectFromIndex(i)->setCopyString("");

            // adjust the selection to copy:
            std::vector<int> selT;
            for (int i = 0; i < objectCount; i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
                if (it != nullptr)
                {
                    if (((options & 1) == 0) || it->getModelBase())
                        selT.push_back(objectHandles[i]);
                    // Here we can't use custom data, dna, etc. since it might be stripped away during the copy, dep. on
                    // the options
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

            for (int i = 0; i < objectCount; i++)
            { // now return the handles of the copies. Each input handle has a corresponding output handle:
                CSceneObject* original = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
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
                            retVal++;
                            break;
                        }
                    }
                }
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simScaleObjects_internal(const int* objectHandles, int objectCount, double scalingFactor, bool scalePositionsToo)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return (-1);
        std::vector<int> sel;
        sel.assign(objectHandles, objectHandles + objectCount);

        int retVal = -1;
        if (scalingFactor >= 0.0001)
        {
            CSceneObjectOperations::scaleObjects(sel, scalingFactor, scalePositionsToo != 0);
#ifdef SIM_WITH_GUI
            GuiApp::setFullDialogRefreshFlag();
#endif
            retVal = 1;
        }
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_INPUT);
        return retVal;
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simAddDrawingObject_internal(int objectType, double size, double duplicateTolerance, int parentObjectHandle,
                                 int maxItemCount, const float* color, const float* setToNULL, const float* setToNULL2,
                                 const float* setToNULL3)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (parentObjectHandle != -1)
        {
            if (!doesObjectExist(__func__, parentObjectHandle))
                return (-1);
        }
        int creatorHandle = -1;
        if ((objectType & sim_drawing_persistent) == 0)
            creatorHandle = _currentScriptHandle;
        CDrawingObject* it =
            new CDrawingObject(objectType, size, duplicateTolerance, parentObjectHandle, maxItemCount, creatorHandle);
        if (color != nullptr)
            it->color.setColor(color, sim_colorcomponent_ambient_diffuse);
        if (setToNULL2 != nullptr)
            it->color.setColor(setToNULL2, sim_colorcomponent_specular);
        if (setToNULL3 != nullptr)
        {
            it->color.setColor(setToNULL3, sim_colorcomponent_emission);
            if ((objectType & sim_drawing_auxchannelcolor1) != 0)
                it->color.setColor(setToNULL3 + 3, sim_colorcomponent_auxiliary);
        }
        int retVal = App::currentWorld->drawingCont->addObject(it);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simRemoveDrawingObject_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handle = objectHandle;
        int handleFlags = 0;
        if (objectHandle >= 0)
        {
            handleFlags = objectHandle & 0x0ff00000;
            handle = objectHandle & 0x000fffff;
        }

        if (handle == sim_handle_all)
            App::currentWorld->drawingCont->eraseAllObjects();
        else
        {
            CDrawingObject* it = App::currentWorld->drawingCont->getObject(handle);
            if (it != nullptr)
                App::currentWorld->drawingCont->removeObject(handle);
            else
            {
                if (handleFlags != sim_handleflag_silenterror)
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_INEXISTANT);
                return (-1);
            }
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simAddDrawingObjectItem_internal(int objectHandle, const double* itemData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CDrawingObject* it = App::currentWorld->drawingCont->getObject(objectHandle);
        if (it == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_INEXISTANT);
            return (-1);
        }
        if (it->addItem(itemData))
            return (1);
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

double simGetObjectSizeFactor_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1.0);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

        double retVal = it->getSizeFactor();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAnnounceSceneContentChange_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->undoBufferContainer->announceChange())
            return (1);
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simReadForceSensor_internal(int objectHandle, double* forceVector, double* torqueVector)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = objectHandle;
        int handleFlags = 0;
        if (objectHandle >= 0)
        {
            handleFlags = objectHandle & 0x0ff00000;
            handle = objectHandle & 0x000fffff;
        }

        if (!doesObjectExist(__func__, handle))
            return (-1);
        if (!isForceSensor(__func__, handle))
            return (-1);
        if (App::currentWorld->simulation->isSimulationStopped())
        {
            //            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return (0);
        }
        CForceSensor* it = App::currentWorld->sceneObjects->getForceSensorFromHandle(handle);
        int retVal = 0;

        C3Vector f, t;
        if (it->getDynamicForces(f, (handleFlags & sim_handleflag_rawvalue) != 0))
        {
            it->getDynamicTorques(t, (handleFlags & sim_handleflag_rawvalue) != 0);
            if (forceVector != nullptr)
                f.getData(forceVector);
            if (torqueVector != nullptr)
                t.getData(torqueVector);
            retVal |= 1;
        }

        CSceneObject* child = it->getChildFromIndex(0);
        if (child == nullptr)
            retVal |= 2;

        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetVelocity_internal(int shapeHandle, double* linearVelocity, double* angularVelocity)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, shapeHandle))
            return (-1);
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        C3Vector lv(it->getDynamicLinearVelocity());
        C3Vector av(it->getDynamicAngularVelocity());
        if (linearVelocity != nullptr)
            lv.getData(linearVelocity);
        if (angularVelocity != nullptr)
            av.getData(angularVelocity);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectVelocity_internal(int objectHandle, double* linearVelocity, double* angularVelocity)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = objectHandle;
        int handleFlags = 0;
        if (objectHandle >= 0)
        {
            handleFlags = objectHandle & 0x0ff00000;
            handle = objectHandle & 0x000fffff;
        }
        if (!doesObjectExist(__func__, handle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        C3Vector lv(it->getMeasuredLinearVelocity());
        C3Vector av;
        if ((handleFlags & sim_handleflag_axis) != 0)
            av = it->getMeasuredAngularVelocityAxis();
        else
            av = it->getMeasuredAngularVelocity3();
        if (linearVelocity != nullptr)
            lv.getData(linearVelocity);
        if (angularVelocity != nullptr)
            av.getData(angularVelocity);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointVelocity_internal(int jointHandle, double* velocity)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__, jointHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        velocity[0] = it->getMeasuredJointVelocity();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAddForceAndTorque_internal(int shapeHandle, const double* force, const double* torque)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = shapeHandle;
        int handleFlags = 0;
        if (shapeHandle >= 0)
        {
            handleFlags = shapeHandle & 0x0ff00000;
            handle = shapeHandle & 0x000fffff;
        }
        if (!doesObjectExist(__func__, handle))
            return (-1);
        if (!isShape(__func__, handle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(handle);
        C3Vector f;
        C3Vector t;
        f.clear();
        t.clear();
        if (force != nullptr)
        {
            if (!isFloatArrayOk(force, 3))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            f.setData(force);
        }
        if (torque != nullptr)
        {
            if (!isFloatArrayOk(torque, 3))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            t.setData(torque);
        }

        if ((handleFlags & sim_handleflag_resetforce) != 0)
            it->clearAdditionalForce();
        if ((handleFlags & sim_handleflag_resettorque) != 0)
            it->clearAdditionalTorque();

        it->addAdditionalForceAndTorque(f, t);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAddForce_internal(int shapeHandle, const double* position, const double* force)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = shapeHandle;
        int handleFlags = 0;
        if (shapeHandle >= 0)
        {
            handleFlags = shapeHandle & 0x0ff00000;
            handle = shapeHandle & 0x000fffff;
        }
        if (!doesObjectExist(__func__, handle))
            return (-1);
        if (!isShape(__func__, handle))
            return (-1);
        if (!isFloatArrayOk(position, 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        if (!isFloatArrayOk(force, 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(handle);
        C3Vector r(position);
        C3Vector f(force);
        C3Vector t(r ^ f);
        // f & t are relative to the shape's frame now. We have to make them absolute:
        C4Vector q(it->getCumulativeTransformation().Q);
        f = q * f;
        t = q * t;
        if ((handleFlags & sim_handleflag_resetforcetorque) != 0)
        {
            it->clearAdditionalForce();
            it->clearAdditionalTorque();
        }
        it->addAdditionalForceAndTorque(f, t);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetExplicitHandling_internal(int objectHandle, int explicitFlags)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((objectHandle >= SIM_IDSTART_SCENEOBJECT) && (objectHandle <= SIM_IDEND_SCENEOBJECT))
        { // scene objects
            if (!doesObjectExist(__func__, objectHandle))
            {
                return (-1);
            }
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it->getObjectType() == sim_sceneobject_joint)
            { // Joints
                ((CJoint*)it)->setExplicitHandling_DEPRECATED(explicitFlags & 1);
                return (1);
            }
            if (it->getObjectType() == sim_sceneobject_graph)
            { // Graphs
                ((CGraph*)it)->setExplicitHandling(explicitFlags & 1);
                return (1);
            }
            if (it->getObjectType() == sim_sceneobject_mill)
            { // Mills
                ((CMill*)it)->setExplicitHandling(explicitFlags & 1);
                return (1);
            }
            if (it->getObjectType() == sim_sceneobject_path)
            { // Paths
                ((CPath_old*)it)->setExplicitHandling(explicitFlags & 1);
                return (1);
            }
            if (it->getObjectType() == sim_sceneobject_visionsensor)
            { // vision sensors
                ((CVisionSensor*)it)->setExplicitHandling(explicitFlags & 1);
                return (1);
            }
            if (it->getObjectType() == sim_sceneobject_proximitysensor)
            { // Proximity sensors
                ((CProxSensor*)it)->setExplicitHandling(explicitFlags & 1);
                return (1);
            }
        }
        // Following for backward compatibility (03.11.2020)
        // -------------------------------------------------------
        if ((objectHandle >= SIM_IDSTART_COLLISION_old) && (objectHandle < SIM_IDEND_COLLISION_old))
        { // collision objects
            if (!doesCollisionObjectExist(__func__, objectHandle))
            {
                return (-1);
            }
            CCollisionObject_old* it = App::currentWorld->collisions_old->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags & 1);
            return (1);
        }
        if ((objectHandle >= SIM_IDSTART_DISTANCE_old) && (objectHandle < SIM_IDEND_DISTANCE_old))
        { // distance objects
            if (!doesDistanceObjectExist(__func__, objectHandle))
            {
                return (-1);
            }
            CDistanceObject_old* it = App::currentWorld->distances_old->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags & 1);
            return (1);
        }
        if ((objectHandle >= SIM_IDSTART_IKGROUP_old) && (objectHandle < SIM_IDEND_IKGROUP_old))
        { // IK objects
            if (!doesIKGroupExist(__func__, objectHandle))
            {
                return (-1);
            }
            CIkGroup_old* it = App::currentWorld->ikGroups_old->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags & 1);
            return (1);
        }
        // -------------------------------------------------------
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENT);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetExplicitHandling_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((objectHandle >= SIM_IDSTART_SCENEOBJECT) && (objectHandle <= SIM_IDEND_SCENEOBJECT))
        { // scene objects
            if (!doesObjectExist(__func__, objectHandle))
                return (-1);
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it->getObjectType() == sim_sceneobject_joint)
            { // Joints
                bool exp = ((CJoint*)it)->getExplicitHandling_DEPRECATED();
                return (exp);
            }
            if (it->getObjectType() == sim_sceneobject_graph)
            { // Graphs
                bool exp = ((CGraph*)it)->getExplicitHandling();
                return (exp);
            }
            if (it->getObjectType() == sim_sceneobject_mill)
            { // Mills
                bool exp = ((CMill*)it)->getExplicitHandling();
                return (exp);
            }
            if (it->getObjectType() == sim_sceneobject_path)
            { // Paths
                bool exp = ((CPath_old*)it)->getExplicitHandling();
                return (exp);
            }
            if (it->getObjectType() == sim_sceneobject_visionsensor)
            { // vision sensors
                bool exp = ((CVisionSensor*)it)->getExplicitHandling();
                return (exp);
            }
            if (it->getObjectType() == sim_sceneobject_proximitysensor)
            { // Proximity sensors
                bool exp = ((CProxSensor*)it)->getExplicitHandling();
                return (exp);
            }
        }
        // Following for backward compatibility (03.11.2020)
        // -------------------------------------------------------
        if ((objectHandle >= SIM_IDSTART_COLLISION_old) && (objectHandle < SIM_IDEND_COLLISION_old))
        { // collision objects
            if (!doesCollisionObjectExist(__func__, objectHandle))
            {
                return (-1);
            }
            CCollisionObject_old* it = App::currentWorld->collisions_old->getObjectFromHandle(objectHandle);
            bool exp = it->getExplicitHandling();
            return (exp);
        }
        if ((objectHandle >= SIM_IDSTART_DISTANCE_old) && (objectHandle < SIM_IDEND_DISTANCE_old))
        { // distance objects
            if (!doesDistanceObjectExist(__func__, objectHandle))
            {
                return (-1);
            }
            CDistanceObject_old* it = App::currentWorld->distances_old->getObjectFromHandle(objectHandle);
            bool exp = it->getExplicitHandling();
            return (exp);
        }
        if ((objectHandle >= SIM_IDSTART_IKGROUP_old) && (objectHandle < SIM_IDEND_IKGROUP_old))
        { // IK objects
            if (!doesIKGroupExist(__func__, objectHandle))
            {
                return (-1);
            }
            CIkGroup_old* it = App::currentWorld->ikGroups_old->getObjectFromHandle(objectHandle);
            bool exp = it->getExplicitHandling();
            return (exp);
        }
        // -------------------------------------------------------
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENT);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetLinkDummy_internal(int dummyHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isDummy(__func__, dummyHandle))
        {
            return (-1);
        }
        CDummy* it = App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);
        int retVal = it->getLinkedDummyHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetLinkDummy_internal(int dummyHandle, int linkedDummyHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isDummy(__func__, dummyHandle))
            return (-1);
        if (linkedDummyHandle != -1)
        {
            if (!isDummy(__func__, linkedDummyHandle))
                return (-1);
        }
        CDummy* it = App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);
        it->setLinkedDummyHandle(linkedDummyHandle, true);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectColor_internal(int objectHandle, int index, int colorComponent, const float* rgbData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal = 0;
        if (it->getObjectType() == sim_sceneobject_shape)
        {
            CShape* shape = (CShape*)it;
            std::vector<CMesh*> all;
            shape->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all);
            if ((index >= 0) && (index < int(all.size())) && (colorComponent <= sim_colorcomponent_auxiliary))
            {
                CMesh* geom = all[index];
#if SIM_EVENT_PROTOCOL_VERSION == 2
                if (colorComponent == sim_colorcomponent_transparency)
                {
                    geom->color.setTranslucid(rgbData[0] != 0.0);
                    geom->color.setOpacity(rgbData[0]);
                }
                else
                    geom->color.setColor(rgbData, colorComponent);
                geom->color.pushShapeColorChangeEvent(objectHandle, index);
#else
                if (colorComponent == sim_colorcomponent_transparency)
                {
                    float ccol = 1.0f - rgbData[0];
                    geom->setColor(&ccol, colorComponent);
                }
                else
                    geom->setColor(rgbData, colorComponent);
#endif
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_dummy)
        {
            CDummy* dummy = (CDummy*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                dummy->getDummyColor()->setColor(rgbData, colorComponent);
#if SIM_EVENT_PROTOCOL_VERSION == 2
                float cols[9];
                dummy->getDummyColor()->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle, cols);
#endif
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_camera)
        {
            CCamera* camera = (CCamera*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                camera->getColor(false)->setColor(rgbData, colorComponent);
#if SIM_EVENT_PROTOCOL_VERSION == 2
                float cols[9];
                camera->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle, cols);
#endif
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_joint)
        {
            CJoint* joint = (CJoint*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                joint->getColor(false)->setColor(rgbData, colorComponent);
#if SIM_EVENT_PROTOCOL_VERSION == 2
                float cols[9];
                joint->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle, cols);
#endif
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_light)
        {
            CLight* light = (CLight*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                light->getColor(index == 1)->setColor(rgbData, colorComponent);
                retVal = 1;
            }
#if SIM_EVENT_PROTOCOL_VERSION == 2
            float cols[2 * 9];
            light->getColor(false)->getNewColors(cols);
            light->getColor(true)->getNewColors(cols + 9);
            CColorObject::pushColorChangeEvent(objectHandle, cols, cols + 9);
#endif
        }
        if (it->getObjectType() == sim_sceneobject_proximitysensor)
        {
            CProxSensor* sensor = (CProxSensor*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor(index)->setColor(rgbData, colorComponent);
                retVal = 1;
            }
#if SIM_EVENT_PROTOCOL_VERSION == 2
            float cols[2 * 9];
            sensor->getColor(0)->getNewColors(cols);
            sensor->getColor(1)->getNewColors(cols + 9);
            CColorObject::pushColorChangeEvent(objectHandle, cols, cols + 9);
#endif
        }
        if (it->getObjectType() == sim_sceneobject_visionsensor)
        {
            CVisionSensor* sensor = (CVisionSensor*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor()->setColor(rgbData, colorComponent);
#if SIM_EVENT_PROTOCOL_VERSION == 2
                float cols[9];
                sensor->getColor()->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle, cols);
#endif
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_forcesensor)
        {
            CForceSensor* sensor = (CForceSensor*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor(false)->setColor(rgbData, colorComponent);
#if SIM_EVENT_PROTOCOL_VERSION == 2
                float cols[9];
                sensor->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle, cols);
#endif
                retVal = 1;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectColor_internal(int objectHandle, int index, int colorComponent, float* rgbData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal = 0;
        if (it->getObjectType() == sim_sceneobject_shape)
        {
            CShape* shape = (CShape*)it;
            std::vector<CMesh*> all;
            shape->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all);
            if ((index >= 0) && (index < int(all.size())) && (colorComponent <= sim_colorcomponent_auxiliary))
            {
                CMesh* geom = all[index];
                geom->color.getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_dummy)
        {
            CDummy* dummy = (CDummy*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                dummy->getDummyColor()->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_camera)
        {
            CCamera* camera = (CCamera*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                camera->getColor(index == 1)->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_joint)
        {
            CJoint* joint = (CJoint*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                if (index == 0)
                    joint->getColor(false)->getColor(rgbData, colorComponent);
                if (index == 1)
                    joint->getColor(true)->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_light)
        {
            CLight* light = (CLight*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                light->getColor(index == 1)->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_proximitysensor)
        {
            CProxSensor* sensor = (CProxSensor*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor(index)->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_visionsensor)
        {
            CVisionSensor* sensor = (CVisionSensor*)it;
            if ((index == 0) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor()->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        if (it->getObjectType() == sim_sceneobject_forcesensor)
        {
            CForceSensor* sensor = (CForceSensor*)it;
            if ((index >= 0) && (index <= 1) && (colorComponent <= sim_colorcomponent_emission))
            {
                sensor->getColor(index == 1)->getColor(rgbData, colorComponent);
                retVal = 1;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetShapeColor_internal(int shapeHandle, const char* colorName, int colorComponent, const float* rgbData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((shapeHandle != sim_handle_all) && (!isShape(__func__, shapeHandle)))
            return (-1);
        if (shapeHandle == sim_handle_all)
        { // deprecated functionality
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
            {
                CShape* it = App::currentWorld->sceneObjects->getShapeFromIndex(i);
                it->setColor(colorName, colorComponent, rgbData);
            }
        }
        else
        {
            CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            it->setColor(colorName, colorComponent, rgbData);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetShapeColor_internal(int shapeHandle, const char* colorName, int colorComponent, float* rgbData)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        int retVal = 0;
        if (it->getColor(colorName, colorComponent, rgbData))
            retVal = 1;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simResetDynamicObject_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = 0;
        if (objectHandle >= 0)
        {
            handleFlags = objectHandle & 0xff00000;
            objectHandle = objectHandle & 0xfffff;
        }

        if ((objectHandle != sim_handle_all) && (!doesObjectExist(__func__, objectHandle)))
            return (-1);
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if ((it->getObjectHandle() == objectHandle) || (objectHandle == sim_handle_all))
            {
                if (it->getObjectHandle() == objectHandle)
                    it->setDynamicsResetFlag(true, handleFlags & sim_handleflag_model);
                else
                    it->setDynamicsResetFlag(true, false);
            }
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointMode_internal(int jointHandle, int jointMode, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__, jointHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        it->setJointMode(jointMode);
        it->setHybridFunctionality_old(options & 1);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointMode_internal(int jointHandle, int* options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__, jointHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        int retVal = it->getJointMode();
        options[0] = 0;
        if (it->getHybridFunctionality_old())
            options[0] |= 1;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSerialOpen_internal(const char* portString, int baudRate, void* reserved1, void* reserved2)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = -1;
#ifdef SIM_WITH_GUI
        handle = App::worldContainer->serialPortContainer->serialPortOpen(false, portString, baudRate);
#endif
        return (handle);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSerialClose_internal(int portHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
#ifdef SIM_WITH_GUI
        if (App::worldContainer->serialPortContainer->serialPortClose(portHandle))
            retVal = 1;
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PORT_HANDLE);
#endif
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSerialSend_internal(int portHandle, const char* data, int dataLength)
{
    C_API_START;

    int retVal = -1;
#ifdef SIM_WITH_GUI
    std::string dat(data, data + dataLength);
    retVal = App::worldContainer->serialPortContainer->serialPortSend(portHandle, dat);
    if (retVal == -1)
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return (retVal);
}

int simSerialRead_internal(int portHandle, char* buffer, int dataLengthToRead)
{
    C_API_START;

    int retVal = -1;
#ifdef SIM_WITH_GUI
    std::string data;
    data.resize(dataLengthToRead);
    retVal = App::worldContainer->serialPortContainer->serialPortReceive(portHandle, data, dataLengthToRead);
    if (retVal > 0)
    {
        for (int i = 0; i < retVal; i++)
            buffer[i] = data[i];
    }
    if (retVal == -1)
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return (retVal);
}

int simSerialCheck_internal(int portHandle)
{
    C_API_START;

    int retVal = -1;
#ifdef SIM_WITH_GUI
    retVal = App::worldContainer->serialPortContainer->serialPortCheck(portHandle);
    if (retVal == -1)
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return (retVal);
}

int simGetContactInfo_internal(int dynamicPass, int objectHandle, int index, int* objectHandles, double* contactInfo)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = 0;
        if (App::currentWorld->dynamicsContainer->getContactForce(dynamicPass, objectHandle, index, objectHandles,
                                                                  contactInfo) != 0)
            retVal = 1;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAuxiliaryConsoleOpen_internal(const char* title, int maxLines, int mode, const int* position, const int* size,
                                     const float* textColor, const float* backgroundColor)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = 1; // in headless mode, we just return a random handle
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
        {
            int tCol[3] = {0, 0, 0};
            int bCol[3] = {255, 255, 255};
            for (size_t i = 0; i < 3; i++)
            {
                if (textColor != nullptr)
                    tCol[i] = int(textColor[i] * 255.1);
                if (backgroundColor != nullptr)
                    bCol[i] = int(backgroundColor[i] * 255.1);
            }
            retVal = GuiApp::mainWindow->codeEditorContainer->openConsole(title, maxLines, mode, position, size, tCol,
                                                                          bCol, _currentScriptHandle);
        }
#endif
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simAuxiliaryConsoleClose_internal(int consoleHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        if ((GuiApp::mainWindow != nullptr) &&
            (GuiApp::mainWindow->codeEditorContainer->close(consoleHandle, nullptr, nullptr, nullptr)))
            return (1);
#endif
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simAuxiliaryConsoleShow_internal(int consoleHandle, bool showState)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        int handleFlags = consoleHandle & 0x0ff00000;
        int handle = consoleHandle & 0x000fffff;
        if ((handleFlags & sim_handleflag_extended) != 0)
        { // we just wanna now if the console is still open
            if ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->codeEditorContainer->isHandleValid(handle)))
                return (1);
        }
        else
        { // normal operation
            if (GuiApp::mainWindow != nullptr)
                return (GuiApp::mainWindow->codeEditorContainer->showOrHide(handle, showState != 0));
        }
#endif
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAuxiliaryConsolePrint_internal(int consoleHandle, const char* text)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = 1; // in headless mode we fake success
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
        {
            retVal = 0;
            if (text == nullptr)
            {
                if (GuiApp::mainWindow->codeEditorContainer->setText(consoleHandle, ""))
                    retVal = 1;
            }
            else
            {
                if (GuiApp::mainWindow->codeEditorContainer->appendText(consoleHandle, text))
                    retVal = 1;
            }
        }
#endif
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simImportShape_internal(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance,
                            double scalingFactor)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::worldContainer->pluginContainer->isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return (-1);
        }
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_FILE_NOT_FOUND);
            return (-1);
        }
        int op = 32;
        if ((options & 1) != 0)
            op |= 16;
        if ((options & 8) != 0)
            op |= 2;
        if ((options & 16) == 0)
            op |= 1;
        if ((options & 32) != 0)
            op |= 64;
        if ((options & 128) != 0)
            op |= 128;
        int h = -1;
        int cnt = 0;
        int* shapes =
            App::worldContainer->pluginContainer->assimp_importShapes(pathAndFilename, 512, scalingFactor, 1, op, &cnt);
        if (cnt > 0)
        {
            h = shapes[0];
            delete[] shapes;
        }
        App::currentWorld->sceneObjects->deselectObjects();
        return (h);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simImportMesh_internal(int fileformat, const char* pathAndFilename, int options, double identicalVerticeTolerance,
                           double scalingFactor, double*** vertices, int** verticesSizes, int*** indices,
                           int** indicesSizes, double*** reserved, char*** names)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_FILE_NOT_FOUND);
            return (-1);
        }
        if (!App::worldContainer->pluginContainer->isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return (-1);
        }
        int op = 0;
        if ((options & 1) != 0)
            op |= 16;
        if ((options & 128) != 0)
            op |= 128;
        int retVal = App::worldContainer->pluginContainer->assimp_importMeshes(
            pathAndFilename, scalingFactor, 1, op, vertices, verticesSizes, indices, indicesSizes);
        if (names != nullptr)
            names[0] = new char*[retVal];
        for (int i = 0; i < retVal; i++)
        {
            if (names != nullptr)
            {
                names[0][i] = new char[1];
                names[0][i][0] = 0;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simExportMesh_internal(int fileformat, const char* pathAndFilename, int options, double scalingFactor,
                           int elementCount, const double** vertices, const int* verticesSizes, const int** indices,
                           const int* indicesSizes, double** reserved, const char** names)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCENE_LOCKED);
            return (-1);
        }
        if (!App::worldContainer->pluginContainer->isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return (-1);
        }
        std::string format;
        if (fileformat == 0)
            format = "obj";
        if (fileformat == 3)
            format = "stl";
        if (fileformat == 4)
            format = "stlb";
        if (fileformat == 5)
            format = "collada";
        if (fileformat == 6)
            format = "ply";
        if (fileformat == 7)
            format = "plyb";
        if (format.size() == 0)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_FILE_FORMAT);
            return (-1);
        }
        bool invalidValues = false;
        if (elementCount < 1)
            invalidValues = true;
        if (!invalidValues)
        {
            for (int i = 0; i < elementCount; i++)
            {
                if ((verticesSizes[i] < 9) || ((verticesSizes[i] / 3) * 3 != verticesSizes[i]))
                    invalidValues = true;

                if (!isFloatArrayOk(vertices[i], verticesSizes[i]))
                    invalidValues = true;
                if ((indicesSizes[i] < 3) || ((indicesSizes[i] / 3) * 3 != indicesSizes[i]))
                    invalidValues = true;
            }
        }
        if (invalidValues)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        int op = 0;
        App::worldContainer->pluginContainer->assimp_exportMeshes(elementCount, vertices, verticesSizes, indices,
                                                                  indicesSizes, pathAndFilename, format.c_str(),
                                                                  scalingFactor, 1, op);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreateShape_internal(int options, double shadingAngle, const double* vertices, int verticesSize,
                            const int* indices, int indicesSize, const double* normals, const float* textureCoords,
                            const unsigned char* texture, const int* textureRes)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isFloatArrayOk(vertices, verticesSize))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        std::vector<double> vert(vertices, vertices + verticesSize);
        std::vector<int> ind(indices, indices + indicesSize);
        std::vector<double> _norm;
        std::vector<double>* norm = nullptr;
        if (normals != nullptr)
        {
            _norm.assign(normals, normals + indicesSize * 3);
            norm = &_norm;
        }
        std::vector<float> _textCoords;
        std::vector<float>* textCoords = nullptr;
        const unsigned char* img = nullptr;
        const int* res = nullptr;
        if ((textureCoords != nullptr) && (texture != nullptr) && (textureRes != nullptr))
        {
            _textCoords.assign(textureCoords, textureCoords + indicesSize * 2);
            textCoords = &_textCoords;
            img = texture;
            res = textureRes;
        }
        else
        { // Simplify meshes only at import, and only if there are no textures (for now):
            CMeshRoutines::removeDuplicateVerticesAndTriangles(vert, &ind, norm, nullptr,
                                                               App::userSettings->identicalVertexTolerance);
        }

        CShape* shape = new CShape(C7Vector::identityTransformation, vert, ind, norm, textCoords, options);
        shape->alignBB("mesh");
        shape->getSingleMesh()->setShadingAngle(shadingAngle);
        shape->getSingleMesh()->setEdgeThresholdAngle(shadingAngle);
        int h = App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
        if (img != nullptr)
        {
            CTextureObject* textureObj = new CTextureObject(textureRes[0], textureRes[1]);
            textureObj->setImage(options & 16, options & 32, (options & 64) == 0, texture);
            textureObj->setObjectName("importedTexture");
            textureObj->addDependentObject(h, shape->getSingleMesh()->getUniqueID());
            int h = App::currentWorld->textureContainer->addObject(
                textureObj, false); // might erase the textureObj and return a similar object already present!!
            shape->getSingleMesh()->getTextureProperty()->setTextureObjectID(h);
        }
        return (h);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateMeshShape_internal(int options, double shadingAngle, const double* vertices, int verticesSize,
                                const int* indices, int indicesSize, double* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isFloatArrayOk(vertices, verticesSize))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        if ((indicesSize >= 3) && ((indicesSize / 3) * 3 == indicesSize))
        {
            bool badIndices = false;
            for (int i = 0; i < indicesSize; i++)
            {
                if ((indices[i] < 0) || (indices[i] >= verticesSize / 3))
                {
                    badIndices = true;
                    break;
                }
            }
            if (!badIndices)
            {
                if ((verticesSize >= 9) && ((verticesSize / 3) * 3 == verticesSize))
                {
                    std::vector<double> vert(vertices, vertices + verticesSize);
                    std::vector<int> ind(indices, indices + indicesSize);
                    // Simplify meshes only at import:
                    CMeshRoutines::removeDuplicateVerticesAndTriangles(vert, &ind, nullptr, nullptr,
                                                                       App::userSettings->identicalVertexTolerance);
                    CShape* shape = new CShape(C7Vector::identityTransformation, vert, ind, nullptr, nullptr, 0);
                    shape->getSingleMesh()->setShadingAngle(shadingAngle);
                    shape->getSingleMesh()->setEdgeThresholdAngle(shadingAngle);
                    shape->setCulling((options & 1) != 0);
                    shape->setVisibleEdges((options & 2) != 0);
                    App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
                    return (shape->getObjectHandle());
                }
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_BAD_VERTICES);
                return (-1);
            }
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_BAD_INDICES);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetShapeMesh_internal(int shapeHandle, double** vertices, int* verticesSize, int** indices, int* indicesSize,
                             double** normals)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<double> wvert;
        std::vector<int> wind;
        std::vector<double> wnorm;
        it->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation, wvert, &wind, &wnorm);
        vertices[0] = new double[wvert.size()];
        verticesSize[0] = int(wvert.size());
        indices[0] = new int[wind.size()];
        indicesSize[0] = int(wind.size());
        if (normals != nullptr)
            normals[0] = new double[wnorm.size()];
        for (size_t i = 0; i < wvert.size(); i++)
            vertices[0][i] = wvert[i];
        for (size_t i = 0; i < wind.size(); i++)
            indices[0][i] = wind[i];
        if (normals != nullptr)
        {
            for (size_t i = 0; i < wnorm.size(); i++)
                normals[0][i] = wnorm[i];
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreatePrimitiveShape_internal(int primitiveType, const double* sizes, int options)
{ // options: bit: 0=culling, 1=sharp edges, 2=open ends with cylinders, 3=force simple shape (i.e. not pure)
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isFloatArrayOk(sizes, 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        C3Vector s(tt::getLimitedFloat(0.00001, 100000.0, sizes[0]), tt::getLimitedFloat(0.00001, 100000.0, sizes[1]),
                   tt::getLimitedFloat(0.00001, 100000.0, sizes[2]));
        int pureType = 1; // pure
        if ((options & 8) != 0)
            pureType = 0; // non-pure
        CShape* shape =
            CAddOperations::addPrimitiveShape(primitiveType, s, options, nullptr, 0, 32, 0, false, pureType);
        int retVal = -1;
        if (shape != nullptr)
        {
            shape->setLocalTransformation(C7Vector::identityTransformation);
            retVal = shape->getObjectHandle();
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateDummy_internal(double size, const float* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CDummy* it = new CDummy();
        it->setDummySize(size);
        if (reserved != nullptr)
        {
            it->getDummyColor()->setColor(reserved + 0, sim_colorcomponent_ambient_diffuse);
            it->getDummyColor()->setColor(reserved + 6, sim_colorcomponent_specular);
            it->getDummyColor()->setColor(reserved + 9, sim_colorcomponent_emission);
        }
        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateScript_internal(int scriptType, const char* scriptText, int options, const char* lang)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScript* it = new CScript(scriptType, scriptText, options, lang);
        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateProximitySensor_internal(int sensorType, int subType, int options, const int* intParams,
                                      const double* floatParams, const double* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CProxSensor* it = new CProxSensor(sensorType);
        it->setSensableType(subType);
        it->setExplicitHandling((options & 1) != 0);
        it->setShowVolume((options & 4) == 0);
        it->setFrontFaceDetection((options & 8) == 0);
        it->setBackFaceDetection((options & 16) == 0);
        it->setExactMode((options & 32) == 0);
        it->setAllowedNormal(floatParams[10]);
        if ((options & 64) == 0) // was angle check
            it->setAllowedNormal(0.0);
        it->setRandomizedDetection((sensorType == sim_proximitysensor_ray) && (options & 512) != 0);

        if ((sensorType == sim_proximitysensor_cylinder) || (sensorType == sim_proximitysensor_disc) ||
            (sensorType == sim_proximitysensor_cone))
            it->convexVolume->setFaceNumber(intParams[0]);
        if (sensorType == sim_proximitysensor_disc)
            it->convexVolume->setFaceNumberFar(intParams[1]);
        if (sensorType == sim_proximitysensor_cone)
        {
            it->convexVolume->setSubdivisions(intParams[2]);
            it->convexVolume->setSubdivisionsFar(intParams[3]);
        }

        if (it->getRandomizedDetection())
        {
            it->setRandomizedDetectionSampleCount(intParams[4]);
            it->setRandomizedDetectionCountForDetection(intParams[5]);
        }

        it->convexVolume->setOffset(floatParams[0]);
        it->convexVolume->setRange(floatParams[1]);
        if (sensorType == sim_proximitysensor_pyramid)
            it->convexVolume->setXSize(floatParams[2]);
        if ((sensorType == sim_proximitysensor_pyramid) || (sensorType == sim_proximitysensor_disc))
            it->convexVolume->setYSize(floatParams[3]);
        if (sensorType == sim_proximitysensor_pyramid)
        {
            it->convexVolume->setXSizeFar(floatParams[4]);
            it->convexVolume->setYSizeFar(floatParams[5]);
        }
        if ((sensorType == sim_proximitysensor_cone) || (sensorType == sim_proximitysensor_disc))
            it->convexVolume->setInsideAngleThing(floatParams[6]);

        if (((sensorType == sim_proximitysensor_ray) && it->getRandomizedDetection()) ||
            (sensorType == sim_proximitysensor_cylinder) || (sensorType == sim_proximitysensor_cone) ||
            (sensorType == sim_proximitysensor_disc))
            it->convexVolume->setRadius(floatParams[7]);
        if (sensorType == sim_proximitysensor_cylinder)
            it->convexVolume->setRadiusFar(floatParams[8]);
        if (((sensorType == sim_proximitysensor_ray) && it->getRandomizedDetection()) ||
            (sensorType == sim_proximitysensor_cone) || (sensorType == sim_proximitysensor_disc))
            it->convexVolume->setAngle(floatParams[9]);

        double ddist = floatParams[11];
        if ((options & 256) == 0)
            ddist = 0.0;
        it->convexVolume->setSmallestDistanceAllowed(ddist);

        it->setProxSensorSize(floatParams[12]);

        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateForceSensor_internal(int options, const int* intParams, const double* floatParams, const double* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CForceSensor* it = new CForceSensor();

        it->setEnableForceThreshold((options & 1) != 0);
        it->setEnableTorqueThreshold((options & 2) != 0);

        it->setFilterType(intParams[0]);
        it->setFilterSampleSize(intParams[1]);
        it->setConsecutiveViolationsToTrigger(intParams[2]);

        it->setForceSensorSize(floatParams[0]);
        it->setForceThreshold(floatParams[1]);
        it->setTorqueThreshold(floatParams[2]);

        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateVisionSensor_internal(int options, const int* intParams, const double* floatParams, const double* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CVisionSensor* it = new CVisionSensor();

        it->setExplicitHandling((options & 1) != 0);
        it->setPerspective((options & 2) != 0);
        it->setShowVolume((options & 4) == 0);
        it->setUseExternalImage((options & 16) != 0);
        it->setUseLocalLights((options & 32) != 0);
        it->setShowFogIfAvailable((options & 64) == 0);
        it->setUseEnvironmentBackgroundColor((options & 128) == 0);
        it->setResolution(intParams);

        it->setClippingPlanes(floatParams[0], floatParams[1]);
        if (it->getPerspective())
            it->setViewAngle(floatParams[2]);
        else
            it->setOrthoViewSize(floatParams[2]);
        it->setVisionSensorSize(floatParams[3]);
        float w[3] = {(float)floatParams[6], (float)floatParams[6], (float)floatParams[6]};
        it->setDefaultBufferValues(w);

        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateJoint_internal(int jointType, int jointMode, int options, const double* sizes, const double* reservedA,
                            const double* reservedB)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CJoint* it = new CJoint(jointType);
        it->setJointMode(jointMode);
        it->setHybridFunctionality_old(options & 1);
        if (sizes != nullptr)
        {
            if (!isFloatArrayOk(sizes, 2))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            it->setSize(sizes[0], sizes[1]);
        }
        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simFloatingViewAdd_internal(double posX, double posY, double sizeX, double sizeY, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CSPage* page =
            App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
        if (page == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PAGE_INEXISTANT);
            return (-1);
        }
        CSView* theFloatingView = new CSView(-1);
        posX = 1.0 - posX;
        posY = 1.0 - posY;
        if (posX < 0.01)
            posX = 0.01;
        if (posX > 0.99)
            posX = 0.99;
        if (posY < 0.01)
            posY = 0.01;
        if (posY > 0.99)
            posY = 0.99;
        sizeX = std::min<double>(sizeX, 2.0 * std::min<double>(posX, 1.0 - posX));
        sizeY = std::min<double>(sizeY, 2.0 * std::min<double>(posY, 1.0 - posY));
        double sizes[2] = {sizeX, sizeY};
        double positions[2] = {posX - sizeX * 0.5, posY - sizeY * 0.5};
        page->addFloatingView(theFloatingView, sizes, positions);
        theFloatingView->setCanSwapViewWithMainView(false);
        if (options & 1)
            theFloatingView->setCanSwapViewWithMainView(true);
        if (options & 2)
            theFloatingView->setCanBeClosed(false);
        if (options & 4)
            theFloatingView->setCanBeShifted(false);
        if (options & 8)
            theFloatingView->setCanBeResized(false);
        int retVal = theFloatingView->getUniqueID();
        return (retVal);
#else
        return (-1);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simFloatingViewRemove_internal(int floatingViewHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        for (int i = 0; i < 8; i++)
        {
            CSPage* page = App::currentWorld->pageContainer->getPage(i);
            if (page != nullptr)
            {
                int viewIndex = page->getViewIndexFromViewUniqueID(floatingViewHandle);
                if (viewIndex != -1)
                {
                    if (size_t(viewIndex) >= page->getRegularViewCount())
                    {
                        page->removeFloatingView(size_t(viewIndex));
                        return (1);
                    }
                    break; // We can't remove the view because it is not floating (anymore?)
                }
            }
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCameraFitToView_internal(int viewHandleOrIndex, int objectCount, const int* objectHandles, int options,
                                double scaling)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = 0;
        if (viewHandleOrIndex >= 0)
        {
            handleFlags = viewHandleOrIndex & 0xff00000;
            viewHandleOrIndex = viewHandleOrIndex & 0xfffff;
        }

        CSView* view = nullptr;
        CCamera* camera = nullptr;
        if ((handleFlags & sim_handleflag_camera) == 0)
        { // normal operation: we provide a view
            if (viewHandleOrIndex >= 10000)
            {
                for (int i = 0; i < 8; i++)
                {
                    CSPage* page = App::currentWorld->pageContainer->getPage(i);
                    int index = page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                    if (index != -1)
                    {
                        view = page->getView(size_t(index));
                        break;
                    }
                }
            }
            else
            {
#ifdef SIM_WITH_GUI
                CSPage* page =
                    App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
#else
                CSPage* page = App::currentWorld->pageContainer->getPage(0);
#endif
                if (page == nullptr)
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PAGE_INEXISTANT);
                    return (-1);
                }
                else
                    view = page->getView(size_t(viewHandleOrIndex));
            }
            if (view == nullptr)
            { // silent error
                return (0);
            }
            camera = App::currentWorld->sceneObjects->getCameraFromHandle(view->getLinkedObjectID());
            if (camera == nullptr)
            { // silent error
                return (0);
            }
        }
        else
        { // special operation: we provide a camera
            if (!isCamera(__func__, viewHandleOrIndex))
                return (-1);
            options |= 2; // 1x1 proportions
            camera = App::currentWorld->sceneObjects->getCameraFromHandle(viewHandleOrIndex);
        }

        std::vector<int> objectsToFrame;
        if ((objectCount != 0) && (objectHandles != nullptr))
        {
            for (int i = 0; i < objectCount; i++)
                objectsToFrame.push_back(objectHandles[i]);
        }
        double xByY = 1.0;
        if ((options & 2) == 0)
        {
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
            {
                int vs[2];
                view->getViewSize(vs);
                xByY = double(vs[0]) / double(vs[1]);
            }
            else
#endif
                xByY = 455.0 / 256.0; // in headless mode
        }
        bool perspective = true;
        if (view != nullptr)
            perspective = view->getPerspectiveDisplay();
        camera->frameSceneOrSelectedObjects(xByY, perspective, &objectsToFrame, false, (options & 1) == 0, scaling,
                                            view);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simAdjustView_internal(int viewHandleOrIndex, int associatedViewableObjectHandle, int options,
                           const char* viewLabel)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSView* view = nullptr;
        if (viewHandleOrIndex >= 10000)
        {
            for (int i = 0; i < 8; i++)
            {
                CSPage* page = App::currentWorld->pageContainer->getPage(i);
                int index = page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                if (index != -1)
                {
                    view = page->getView(size_t(index));
                    break;
                }
            }
        }
        else
        {
#ifdef SIM_WITH_GUI
            CSPage* page =
                App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
#else
            CSPage* page = App::currentWorld->pageContainer->getPage(0);
#endif
            if (page == nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_PAGE_INEXISTANT);
                return (-1);
            }
            else
                view = page->getView(size_t(viewHandleOrIndex));
        }
        if (view == nullptr)
        {
            if (options & 0x100)
                return (1);
            return (0);
        }
        if (options & 0x100)
            return (2);
        if (options & 0x200)
        { // just return the object associated with the view:
            return (view->getLinkedObjectID());
        }
        if (associatedViewableObjectHandle != -1)
        {
            if (!doesObjectExist(__func__, associatedViewableObjectHandle))
            {
                return (-1);
            }
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(associatedViewableObjectHandle);
            int objType = it->getObjectType();
            if ((objType != sim_sceneobject_camera) && (objType != sim_sceneobject_graph) &&
                (objType != sim_sceneobject_visionsensor))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_IS_NOT_VIEWABLE);
                return (-1);
            }
        }
        view->setLinkedObjectID(associatedViewableObjectHandle, true);
        view->setRenderingMode(options & 0x000f);
        view->setPerspectiveDisplay((options & 0x0010) == 0);
        view->setTimeGraph((options & 0x0020) == 0);
        view->setXYGraphIsOneOneProportional((options & 0x0400) != 0);
        view->setRemoveFloatingViewAtSimulationEnd((options & 0x0040) != 0);
        view->setDoNotSaveFloatingView((options & 0x0080) != 0);

        if (viewLabel != nullptr)
            view->setAlternativeViewName(viewLabel);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreateHeightfieldShape_internal(int options, double shadingAngle, int xPointCount, int yPointCount, double xSize,
                                       const double* heights)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ((xPointCount < 2) || (xPointCount > 2048) || (yPointCount < 2) || (yPointCount > 2048) || (xSize < 0.00001))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
            return (-1);
        }
        std::vector<std::vector<double>*> allData;
        for (int i = 0; i < yPointCount; i++)
        {
            std::vector<double>* vect = new std::vector<double>;
            for (int j = 0; j < xPointCount; j++)
                vect->push_back(heights[i * xPointCount + j]);
            allData.push_back(vect);
        }
        int retVal =
            CFileOperations::createHeightfield(xPointCount, xSize / (xPointCount - 1), allData, shadingAngle, options);
        for (int i = 0; i < int(allData.size()); i++)
            delete allData[i];
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetRotationAxis_internal(const double* matrixStart, const double* matrixGoal, double* axis, double* angle)
{
    C_API_START;

    if (!isFloatArrayOk(matrixStart, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(matrixGoal, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C4X4Matrix mStart;
    mStart.setData(matrixStart);
    mStart.M.normalize();
    C4X4Matrix mGoal;
    mGoal.setData(matrixGoal);
    mGoal.M.normalize();

    // Following few lines taken from the quaternion interpolation part:
    C4Vector AA(mStart.M.getQuaternion());
    C4Vector BB(mGoal.M.getQuaternion());
    if (AA(0) * BB(0) + AA(1) * BB(1) + AA(2) * BB(2) + AA(3) * BB(3) < 0.0)
        AA = AA * -1.0;
    C4Vector r((AA.getInverse() * BB).getAngleAndAxis());

    C3Vector v(r(1), r(2), r(3));
    v = AA * v;

    axis[0] = v(0);
    axis[1] = v(1);
    axis[2] = v(2);
    double l = sqrt(v(0) * v(0) + v(1) * v(1) + v(2) * v(2));
    if (l != 0.0)
    {
        axis[0] /= l;
        axis[1] /= l;
        axis[2] /= l;
    }
    angle[0] = r(0);

    return (1);
}

int simRotateAroundAxis_internal(const double* matrixIn, const double* axis, const double* axisPos, double angle, double* matrixOut)
{
    C_API_START;

    if (!isFloatArrayOk(matrixIn, 12))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(axis, 3))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    if (!isFloatArrayOk(axisPos, 3))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    C4X4Matrix mIn;
    mIn.setData(matrixIn);
    mIn.M.normalize();
    C7Vector m(mIn);
    C3Vector ax(axis);
    C3Vector pos(axisPos);

    //    angle = std::fmod(angle, piValT2);
    //    if (angle < 0.0)
    //        angle += piValT2;

    double alpha = -atan2(ax(1), ax(0));
    double beta = atan2(-sqrt(ax(0) * ax(0) + ax(1) * ax(1)), ax(2));
    m.X -= pos;
    C7Vector r;
    r.X.clear();
    r.Q.setEulerAngles(0.0, 0.0, alpha);
    m = r * m;
    r.Q.setEulerAngles(0.0, beta, 0.0);
    m = r * m;
    r.Q.setEulerAngles(0.0, 0.0, angle);
    m = r * m;
    r.Q.setEulerAngles(0.0, -beta, 0.0);
    m = r * m;
    r.Q.setEulerAngles(0.0, 0.0, -alpha);
    m = r * m;
    m.X += pos;
    m.getMatrix().getData(matrixOut);

    return (1);
}

int simGetJointForce_internal(int jointHandle, double* forceOrTorque)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle = jointHandle;
        int handleFlags = 0;
        if (jointHandle >= 0)
        {
            handleFlags = jointHandle & 0x0ff00000;
            handle = jointHandle & 0x000fffff;
        }
        if (!doesObjectExist(__func__, handle))
            return (-1);
        if (!isJoint(__func__, handle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(handle);
        if (it->getJointType() == sim_joint_spherical)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_JOINT_SPHERICAL);
            return (-1);
        }
        double f;
        forceOrTorque[0] = 0.0;
        if (it->getDynamicForceOrTorque(f, (handleFlags & sim_handleflag_rawvalue) != 0))
        {
            forceOrTorque[0] = f;
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetJointTargetForce_internal(int jointHandle, double* forceOrTorque)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, jointHandle))
            return (-1);
        if (!isJoint(__func__, jointHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        forceOrTorque[0] = it->getTargetForce(true);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetJointTargetForce_internal(int objectHandle, double forceOrTorque, bool signedValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isJoint(__func__, objectHandle))
            return (-1);
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        it->setTargetForce(forceOrTorque, signedValue);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simIsHandle_internal(int generalObjectHandle, int generalObjectType)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (((generalObjectType == -1) || (generalObjectType == sim_objecttype_sceneobject)) &&
            (App::currentWorld->sceneObjects->getObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_objecttype_collection)) &&
            (App::currentWorld->collections->getObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_script_type)) &&
            (App::worldContainer->getScriptObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_objecttype_texture)) &&
            (App::currentWorld->textureContainer->getObject(generalObjectHandle) != nullptr))
            return (1);

        // Old:
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_ui_type)) &&
            (App::currentWorld->buttonBlockContainer_old->getBlockWithID(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_collision_type)) &&
            (App::currentWorld->collisions_old->getObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_distance_type)) &&
            (App::currentWorld->distances_old->getObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_ik_type)) &&
            (App::currentWorld->ikGroups_old->getObjectFromHandle(generalObjectHandle) != nullptr))
            return (1);
        if (((generalObjectType == -1) || (generalObjectType == sim_appobj_pathplanning_type)) &&
            (App::currentWorld->pathPlanning_old->getObject(generalObjectHandle) != nullptr))
            return (1);
        return (0); // handle is not valid!
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simHandleVisionSensor_internal(int visionSensorHandle, double** auxValues, int** auxValuesCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((visionSensorHandle != sim_handle_all) && (visionSensorHandle != sim_handle_all_except_explicit))
        {
            if (!isVisionSensor(__func__, visionSensorHandle))
                return (-1);
        }
        if (auxValues != nullptr)
            auxValues[0] = nullptr;
        if (auxValuesCount != nullptr)
            auxValuesCount[0] = nullptr;
        int retVal = 0;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_visionsensor); i++)
        {
            CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromIndex(i);
            if (visionSensorHandle >= 0)
            { // explicit handling
                it = (CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return (-1);
                }
                retVal = it->handleSensor();
                if ((auxValues != nullptr) && (auxValuesCount != nullptr))
                {
                    auxValuesCount[0] = new int[1 + int(it->sensorAuxiliaryResult.size())];
                    auxValuesCount[0][0] = int(it->sensorAuxiliaryResult.size());
                    int fvs = 0;
                    for (size_t j = 0; j < it->sensorAuxiliaryResult.size(); j++)
                        fvs += (int)it->sensorAuxiliaryResult[j].size();
                    auxValues[0] = new double[fvs];
                    fvs = 0;
                    for (size_t j = 0; j < it->sensorAuxiliaryResult.size(); j++)
                    {
                        auxValuesCount[0][j + 1] = int(it->sensorAuxiliaryResult[j].size());
                        for (size_t k = 0; k < it->sensorAuxiliaryResult[j].size(); k++)
                            auxValues[0][fvs++] = it->sensorAuxiliaryResult[j][k];
                    }
                }
            }
            else
            {
                if ((!it->getExplicitHandling()) || (visionSensorHandle == sim_handle_all))
                    retVal += it->handleSensor();
            }
            if (visionSensorHandle >= 0)
                break;
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simReadVisionSensor_internal(int visionSensorHandle, double** auxValues, int** auxValuesCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isVisionSensor(__func__, visionSensorHandle))
            return (-1);
        if (auxValues != nullptr)
            auxValues[0] = nullptr;
        if (auxValuesCount != nullptr)
            auxValuesCount[0] = nullptr;
        CVisionSensor* it = (CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
        int retVal = -1;
        if (it->sensorResult.sensorResultIsValid)
        {
            if (it->sensorResult.sensorWasTriggered)
                retVal = 1;
            else
                retVal = 0;
            if ((auxValues != nullptr) && (auxValuesCount != nullptr))
            {
                auxValuesCount[0] = new int[1 + int(it->sensorAuxiliaryResult.size())];
                auxValuesCount[0][0] = int(it->sensorAuxiliaryResult.size());
                int fvs = 0;
                for (int j = 0; j < int(it->sensorAuxiliaryResult.size()); j++)
                    fvs += (int)it->sensorAuxiliaryResult[j].size();
                auxValues[0] = new double[fvs];
                fvs = 0;
                for (int j = 0; j < int(it->sensorAuxiliaryResult.size()); j++)
                {
                    auxValuesCount[0][j + 1] = int(it->sensorAuxiliaryResult[j].size());
                    for (int k = 0; k < int(it->sensorAuxiliaryResult[j].size()); k++)
                        auxValues[0][fvs++] = it->sensorAuxiliaryResult[j][k];
                }
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simResetVisionSensor_internal(int visionSensorHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ((visionSensorHandle != sim_handle_all) && (visionSensorHandle != sim_handle_all_except_explicit))
        {
            if (!isVisionSensor(__func__, visionSensorHandle))
            {
                return (-1);
            }
        }
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_visionsensor); i++)
        {
            CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromIndex(i);
            if (visionSensorHandle >= 0)
            { // Explicit handling
                it = (CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return (-1);
                }
                it->resetSensor();
                break;
            }
            else
            {
                if ((!it->getExplicitHandling()) || (visionSensorHandle == sim_handle_all))
                    it->resetSensor();
            }
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCheckVisionSensor_internal(int sensorHandle, int entityHandle, double** auxValues, int** auxValuesCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isVisionSensor(__func__, sensorHandle))
            return (-1);
        if ((entityHandle != sim_handle_all) && (!doesEntityExist(__func__, entityHandle)))
            return (-1);
        if (entityHandle == sim_handle_all)
            entityHandle = -1;

        if (!App::currentWorld->mainSettings_old->visionSensorsEnabled)
            return (0);

        if (auxValues != nullptr)
            auxValues[0] = nullptr;
        if (auxValuesCount != nullptr)
            auxValuesCount[0] = nullptr;
        int retVal = 0;

        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        if (it->checkSensor(entityHandle, true))
            retVal = 1;

        if ((auxValues != nullptr) && (auxValuesCount != nullptr))
        {
            auxValuesCount[0] = new int[1 + int(it->sensorAuxiliaryResult.size())];
            auxValuesCount[0][0] = int(it->sensorAuxiliaryResult.size());
            int fvs = 0;
            for (int j = 0; j < int(it->sensorAuxiliaryResult.size()); j++)
                fvs += (int)it->sensorAuxiliaryResult[j].size();
            auxValues[0] = new double[fvs];
            fvs = 0;
            for (int j = 0; j < int(it->sensorAuxiliaryResult.size()); j++)
            {
                auxValuesCount[0][j + 1] = int(it->sensorAuxiliaryResult[j].size());
                for (int k = 0; k < int(it->sensorAuxiliaryResult[j].size()); k++)
                    auxValues[0][fvs++] = it->sensorAuxiliaryResult[j][k];
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

float* simCheckVisionSensorEx_internal(int sensorHandle, int entityHandle, bool returnImage)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (nullptr);
        if (!isVisionSensor(__func__, sensorHandle))
            return (nullptr);
        if ((entityHandle != sim_handle_all) && (!doesEntityExist(__func__, entityHandle)))
            return (nullptr);
        if (entityHandle == sim_handle_all)
            entityHandle = -1;

        if (!App::currentWorld->mainSettings_old->visionSensorsEnabled)
            return (nullptr);

        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        float* retBuffer = it->checkSensorEx(entityHandle, returnImage != 0, false, false, true);
        return (retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

unsigned char* simGetVisionSensorImg_internal(int sensorHandle, int options, double rgbaCutOff, const int* pos,
                                              const int* size, int* resolution)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (nullptr);
        if (!isVisionSensor(__func__, sensorHandle))
            return (nullptr);
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolution != nullptr)
        {
            resolution[0] = res[0];
            resolution[1] = res[1];
        }
        int posX = 0;
        int posY = 0;
        if (pos != nullptr)
        {
            posX = pos[0];
            posY = pos[1];
        }
        int sizeX = res[0];
        int sizeY = res[1];
        if ((size != nullptr) && (size[0] > 0))
        {
            sizeX = size[0];
            sizeY = size[1];
        }
        unsigned char* img = it->readPortionOfCharImage(posX, posY, sizeX, sizeY, rgbaCutOff, options);
        if (img == nullptr)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
        return (img);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simSetVisionSensorImg_internal(int sensorHandle, const unsigned char* img, int options, const int* pos,
                                   const int* size)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isVisionSensor(__func__, sensorHandle))
            return (-1);
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        int posX = 0;
        int posY = 0;
        if (pos != nullptr)
        {
            posX = pos[0];
            posY = pos[1];
        }
        int sizeX = res[0];
        int sizeY = res[1];
        if ((size != nullptr) && (size[0] > 0))
        {
            sizeX = size[0];
            sizeY = size[1];
        }
        if (!it->writePortionOfCharImage(img, posX, posY, sizeX, sizeY, options))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
            return (-1);
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

float* simGetVisionSensorDepth_internal(int sensorHandle, int options, const int* pos, const int* size, int* resolution)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (nullptr);
        if (!isVisionSensor(__func__, sensorHandle))
            return (nullptr);
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolution != nullptr)
        {
            resolution[0] = res[0];
            resolution[1] = res[1];
        }
        int posX = 0;
        int posY = 0;
        if (pos != nullptr)
        {
            posX = pos[0];
            posY = pos[1];
        }
        int sizeX = res[0];
        int sizeY = res[1];
        if ((size != nullptr) && (size[0] > 0))
        {
            sizeX = size[0];
            sizeY = size[1];
        }
        float* retBuff = it->readPortionOfImage(posX, posY, sizeX, sizeY, 2);
        if (((options & 1) != 0) && retBuff)
        {
            double np, fp;
            it->getClippingPlanes(np, fp);
            float n = (float)np;
            float f = (float)fp;
            float fmn = f - n;
            for (int i = 0; i < sizeX * sizeY; i++)
                retBuff[i] = n + fmn * retBuff[i];
        }
        if (retBuff == nullptr)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);

        return (retBuff);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int _simSetVisionSensorDepth_internal(int sensorHandle, int options, const float* depth)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isVisionSensor(__func__, sensorHandle))
            return (-1);
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        it->writeImage(depth, 2);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRuckigPos_internal(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel,
                          const double* currentAccel, const double* maxVel, const double* maxAccel,
                          const double* maxJerk, const bool* selection, const double* targetPos,
                          const double* targetVel, double* reserved1, int* reserved2)
{ // input floats are check on the plugin side
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = App::worldContainer->pluginContainer->ruckigPlugin_pos(
            _currentScriptHandle, dofs, baseCycleTime, flags, currentPos, currentVel, currentAccel, maxVel, maxAccel,
            maxJerk, selection, targetPos, targetVel);
        if (retVal == -2)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRuckigVel_internal(int dofs, double baseCycleTime, int flags, const double* currentPos, const double* currentVel,
                          const double* currentAccel, const double* maxAccel, const double* maxJerk,
                          const bool* selection, const double* targetVel, double* reserved1, int* reserved2)
{ // input floats are check on the plugin side
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = App::worldContainer->pluginContainer->ruckigPlugin_vel(_currentScriptHandle, dofs, baseCycleTime,
                                                                            flags, currentPos, currentVel, currentAccel,
                                                                            maxAccel, maxJerk, selection, targetVel);
        if (retVal == -2)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRuckigStep_internal(int objHandle, double cycleTime, double* newPos, double* newVel, double* newAccel,
                           double* syncTime, double* reserved1, int* reserved2)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = App::worldContainer->pluginContainer->ruckigPlugin_step(objHandle, cycleTime, newPos, newVel,
                                                                             newAccel, syncTime);
        if (retVal == -3)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_RUCKIG_CYCLETIME_ERROR);
        if (retVal == -2)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        if (retVal == -1)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_RUCKIG_OBJECT_INEXISTANT);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simRuckigRemove_internal(int objHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = App::worldContainer->pluginContainer->ruckigPlugin_remove(objHandle);
        if (retVal == -2)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        if (retVal == -1)
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_RUCKIG_OBJECT_INEXISTANT);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjectQuaternion_internal(int objectHandle, int relativeToObjectHandle, double* quaternion)
{
    C_API_START;
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces (default): x y z w

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;

        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj == nullptr)
            tr = it->getCumulativeTransformation();
        else
        {
            if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr = relTr.getInverse() * it->getCumulativeTransformation();
            }
            else
            {
                if (it->getParent() == relObj)
                    tr = it->getLocalTransformation(); // in case of a series get/set, not to lose precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr = relTr.getInverse() * it->getCumulativeTransformation();
                }
            }
        }
        if (inverse)
            tr.Q.inverse();
        tr.Q.getData(quaternion, (handleFlags & sim_handleflag_wxyzquat) == 0);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetObjectQuaternion_internal(int objectHandle, int relativeToObjectHandle, const double* quaternion)
{
    C_API_START;
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces (default): x y z w

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;

        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        if (!isFloatArrayOk(quaternion, 4))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle == sim_handle_parent)
        {
            relativeToObjectHandle = sim_handle_world;
            CSceneObject* parent = it->getParent();
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
            if (!doesObjectExist(__func__, relativeToObjectHandle))
                return (-1);
        }
        if (it->getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true, true);
        CSceneObject* relObj = App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj == nullptr)
        {
            C4Vector q;
            q.setData(quaternion, (handleFlags & sim_handleflag_wxyzquat) == 0);
            q.normalize();
            if (inverse)
                q.inverse();
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(), q.getEulerAngles());
        }
        else
        {
            if ((it->getParent() == relObj) && ((handleFlags & sim_handleflag_reljointbaseframe) == 0))
            { // special here, in order to not lose precision in a series of get/set
                C7Vector tr(it->getLocalTransformation());
                tr.Q.setData(quaternion, (handleFlags & sim_handleflag_wxyzquat) == 0);
                tr.Q.normalize();
                if (inverse)
                    tr.Q.inverse();
                it->setLocalTransformation(tr);
            }
            else
            {
                C7Vector absTr(it->getCumulativeTransformation());
                C7Vector relTr;
                if ((handleFlags & sim_handleflag_reljointbaseframe) != 0)
                    relTr = relObj->getCumulativeTransformation();
                else
                    relTr = relObj->getFullCumulativeTransformation();
                C7Vector x(relTr.getInverse() * absTr);
                x.Q.setData(quaternion, (handleFlags & sim_handleflag_wxyzquat) == 0);
                x.Q.normalize();
                if (inverse)
                    x.Q.inverse();
                absTr = relTr * x;
                App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),
                                                                              absTr.Q.getEulerAngles());
            }
        }
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetShapeMass_internal(int shapeHandle, double* mass)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        mass[0] = it->getMesh()->getMass();
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetShapeMass_internal(int shapeHandle, double mass)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        if (mass < 0.0000001)
            mass = 0.0000001;
        it->getMesh()->setMass(mass);
        it->setDynamicsResetFlag(true, false);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetShapeInertia_internal(int shapeHandle, double* inertiaMatrix, double* transformationMatrix)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        C4X4Matrix tr;
        tr.setIdentity();
        tr.X = it->getMesh()->getCOM();
        tr.getData(transformationMatrix);
        C3X3Matrix m(it->getMesh()->getInertia());
        m *= it->getMesh()->getMass();
        m.getData(inertiaMatrix);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetShapeInertia_internal(int shapeHandle, const double* inertiaMatrix, const double* transformationMatrix)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        if (!isFloatArrayOk(inertiaMatrix, 9))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        if (!isFloatArrayOk(transformationMatrix, 12))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        C4X4Matrix tr;
        tr.setData(transformationMatrix);
        tr.M.normalize();

        C3X3Matrix m;
        m.setData(inertiaMatrix);
        m.axis[0](1) = m.axis[1](0);
        m.axis[0](2) = m.axis[2](0);
        m.axis[1](2) = m.axis[2](1);
        m /= it->getMesh()->getMass(); // in CoppeliaSim we work with the "massless inertia"

        it->getMesh()->setCOM(tr.X);
        m = CMeshWrapper::getInertiaInNewFrame(tr.M.getQuaternion(), m, C4Vector::identityRotation);
        it->getMesh()->setInertia(m);
        it->setDynamicsResetFlag(true, false);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simIsDynamicallyEnabled_internal(int objectHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal = 0;
        if (it->getObjectType() == sim_sceneobject_shape)
        {
            CShape* shape = (CShape*)it;
            if (it->getDynamicSimulationIconCode() == sim_dynamicsimicon_objectisdynamicallysimulated)
                retVal = 1;
        }
        if (it->getObjectType() == sim_sceneobject_joint)
        {
            CJoint* joint = (CJoint*)it;
            if ((joint->getJointMode() == sim_jointmode_dynamic) &&
                (it->getDynamicSimulationIconCode() == sim_dynamicsimicon_objectisdynamicallysimulated))
                retVal = 1; // we do not consider a joint dyn. enabled when in deprecated hybrid mode
        }
        if (it->getObjectType() == sim_sceneobject_forcesensor)
        {
            CForceSensor* shape = (CForceSensor*)it;
            if (it->getDynamicSimulationIconCode() == sim_dynamicsimicon_objectisdynamicallysimulated)
                retVal = 1;
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGenerateShapeFromPath_internal(const double* pppath, int pathSize, const double* section, int sectionSize,
                                      int options, const double* upVector, double reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isFloatArrayOk(pppath, pathSize))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        if (!isFloatArrayOk(section, sectionSize))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        // First make sure the points are not coincident:
        std::vector<double> ppath;
        C3Vector prevV;
        prevV.clear();
        C4Vector prevQ;
        prevQ.clear();
        for (int i = 0; i < pathSize / 7; i++)
        {
            C3Vector v(pppath + 7 * i);
            C4Vector q(pppath + 7 * i + 3, true);
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
            C3Vector zvect;
            if (upVector != nullptr)
                zvect.setData(upVector);
            else
                zvect = C3Vector::unitZVector;
            bool closedPath = (options & 4) != 0;
            int axis = options & 3;

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
                C4Vector q(&ppath[0] + 7 * i + 3, false); // Quaternion notation was changed above!
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
                C7Vector p(m.getTransformation());
                for (size_t j = 0; j < 7; j++)
                    path.push_back(p(j));
            }

            bool sectionClosed = ((section[0] == section[sectionSize - 2]) && (section[1] == section[sectionSize - 1]));
            if (sectionClosed)
                secVertCnt--;

            std::vector<double> vertices;
            std::vector<int> indices;
            C7Vector tr0;
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
                C7Vector tr;
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
            int h = simCreateShape_internal(0, 0.0, &vertices[0], int(vertices.size()), &indices[0],
                                            int(indices.size()), nullptr, nullptr, nullptr, nullptr);
            return (h);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PATH);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

double simGetClosestPosOnPath_internal(const double* path, int pathSize, const double* pathLengths, const double* absPt)
{
    C_API_START;
    double retVal = 0.0;

    if (pathSize >= 6)
    {
        double d = DBL_MAX;
        C3Vector pppt(absPt);
        for (int i = 0; i < (pathSize / 3) - 1; i++)
        {
            C3Vector v0(path + i * 3);
            C3Vector v1(path + (i + 1) * 3);
            C3Vector vd(v1 - v0);
            C3Vector theSearchedPt;
            if (CMeshRoutines::getMinDistBetweenSegmentAndPoint_IfSmaller(v0, vd, pppt, d, theSearchedPt))
            {
                double vdL = vd.getLength();
                if (vdL == 0.0)
                    retVal = pathLengths[i]; // // Coinciding points
                else
                {
                    double l = (theSearchedPt - v0).getLength();
                    double c = l / vdL;
                    retVal = pathLengths[i] * (1.0 - c) + pathLengths[i + 1] * c;
                }
            }
        }
    }
    return (retVal);
}

int simInitScript_internal(int scriptHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
        if (it != nullptr)
        {
            it->resetScript();
            it->initScript();
            return (1);
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCRIPT_INEXISTANT);
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simModuleEntry_internal(int handle, const char* label, int state)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool first = false;
        if (handle == -1)
        {
            first = true;
            handle = App::worldContainer->moduleMenuItemContainer->addMenuItem(label, _currentScriptHandle);
        }
        CModuleMenuItem* item = App::worldContainer->moduleMenuItemContainer->getItemFromHandle(handle);
        if (item != nullptr)
        {
            if (state == -2)
                App::worldContainer->moduleMenuItemContainer->removeMenuItem(handle);
            else
            {
                if ((!first) && (label != nullptr))
                    item->setLabel(label);
                if (state != -1)
                    item->setState(state);
            }
            return (handle);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCheckExecAuthorization_internal(const char* what, const char* args, int scriptHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = 0;
        if (App::userSettings->execUnsafe)
            retVal = 1;
        else
        {
            bool auth = false;
            int h = scriptHandle;
            if (h < 0)
                h = CScriptObject::getInExternalCall();
            CScriptObject* it = nullptr;
            std::string x, y;
            if (h >= 0)
            {
                it = App::worldContainer->getScriptObjectFromHandle(h);
                if (it != nullptr)
                {
                    x = x + args + " ";
                    if (x.find("pythonLauncher.py") != std::string::npos)
                    { // followin pattern occur constantly with python scripts within CoppeliaSim
                        x = std::regex_replace(x, std::regex(R"(tcp://127\.0\.0\.1:(\d+))"), "localhost");
                        x = std::regex_replace(x, std::regex(R"(tcp://localhost:(\d+))"), "localhost");
                        x = std::regex_replace(x, std::regex("([^ ]*)pythonLauncher.py "), "pythonLauncher.py ");
                        x = std::regex_replace(x, std::regex("^([^ ]*)python([^ ]*) "), "py ");
                        x = std::regex_replace(x, std::regex("^([^ ]*)py.exe "), "py ");
                    }
                    x = std::regex_replace(x, std::regex(" "), "_");
                    y = x + std::to_string(it->getSimpleHash());
                    std::hash<std::string> hasher;
                    y = std::to_string(hasher(y)) + "EXECUNSAFE";

                    std::string val;
                    CPersistentDataContainer cont;
                    if (cont.readData(y.c_str(), val))
                        auth = true;
                }
            }
#ifdef SIM_WITH_GUI
            if ((!auth) && (GuiApp::mainWindow != nullptr))
            {
                if (GuiApp::uiThread->checkExecuteUnsafeOk(what, args, x.c_str()))
                {
                    auth = true;
                    if (it != nullptr)
                    {
                        CPersistentDataContainer cont;
                        cont.writeData(y.c_str(), "OK", true, false);
                    }
                }
            }
#endif
            if (auth)
                retVal = 1;
            else
            {
                std::string tmp("function was hindered to execute for your safety. You can enable its execution and "
                                "every other unsafe function with 'execUnsafe=true' in ");
                tmp += App::folders->getUserSettingsPath() + "/usrset.txt";
                CApiErrors::setLastWarningOrError(__func__, tmp.c_str());
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGroupShapes_internal(const int* shapeHandles, int shapeCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> shapes;
        bool merging = (shapeCount < 0);
        if (shapeCount < 0)
            shapeCount = -shapeCount;
        for (int i = 0; i < shapeCount; i++)
        {
            CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandles[i]);
            if (it != nullptr)
                shapes.push_back(it->getObjectHandle());
        }
        if (shapes.size() < 2)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_NOT_ENOUGH_SHAPES);
            return (-1);
        }
        std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
        int retVal;
        if (merging)
            retVal = CSceneObjectOperations::mergeSelection(&shapes);
        else
            retVal = CSceneObjectOperations::groupSelection(&shapes);
        App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection.data(), initSelection.size());
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int* simUngroupShape_internal(int shapeHandle, int* shapeCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool dividing = false;
        if (shapeHandle <= -2)
        {
            shapeHandle = -shapeHandle - 2;
            dividing = true;
        }

        if (!isShape(__func__, shapeHandle))
        {
            shapeCount[0] = 0;
            return (nullptr);
        }
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        if (it->getMesh()->isMesh())
        {
            if (dividing)
            {
                std::vector<int> sel;
                sel.push_back(shapeHandle);
                CSceneObjectOperations::divideSelection(&sel);
                int* retVal = new int[sel.size()];
                for (int i = 0; i < int(sel.size()); i++)
                    retVal[i] = sel[i];
                shapeCount[0] = int(sel.size());
                return (retVal);
            }
            else
            {
                shapeCount[0] = 1;
                int* retVal = new int[1];
                retVal[0] = shapeHandle;
                return (retVal);
            }
        }
        else
        {
            if (dividing)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE);
                shapeCount[0] = 0;
                return (nullptr);
            }
            else
            {
                std::vector<int> sel;
                sel.push_back(shapeHandle);
                CSceneObjectOperations::ungroupSelection(&sel, true);
                int* retVal = new int[sel.size()];
                for (int i = 0; i < int(sel.size()); i++)
                    retVal[i] = sel[i];
                shapeCount[0] = int(sel.size());
                return (retVal);
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (nullptr);
}

int simConvexDecompose_internal(int shapeHandle, int options, const int* intParams, const double* floatParams)
{ // one shape at a time!
    C_API_START;

    int retVal = CSceneObjectOperations::convexDecompose(shapeHandle, options, intParams, floatParams);
    return (retVal);
}

void simQuitSimulator_internal(bool ignoredArgument)
{
    C_API_START;
#ifdef SIM_WITH_GUI
    SSimulationThreadCommand cmd;
    cmd.cmdId = EXIT_REQUEST_CMD;
    App::appendSimulationThreadCommand(cmd);
#else
    App::postExitRequest();
#endif
}

int simSetShapeMaterial_internal(int shapeHandle, int materialIdOrShapeHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal = -1; // means error
        if (isShape(__func__, shapeHandle))
        {
            // For backward compatibility (28/10/2016)
            // We now do not share materials anymore: each shape has its own material, so
            // the material of a shape is identified by the shape handle itself
            if (materialIdOrShapeHandle != -1)
            {
                if (materialIdOrShapeHandle >= sim_dynmat_default)
                { // set a predefined material
                    CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                    shape->getDynMaterial()->generateDefaultMaterial(materialIdOrShapeHandle);
                }
                else
                { // set the same material as another shape
                    CShape* matShape = App::currentWorld->sceneObjects->getShapeFromHandle(materialIdOrShapeHandle);
                    if (matShape != nullptr)
                    {
                        CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                        shape->setDynMaterial(matShape->getDynMaterial()->copyYourself());
                        retVal = 1;
                    }
                }
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetTextureId_internal(const char* textureName, int* resolution)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1; // means error
        CTextureObject* to = App::currentWorld->textureContainer->getObject(textureName);
        if (to != nullptr)
        {
            retVal = to->getObjectID();
            if (resolution != nullptr)
                to->getTextureSize(resolution[0], resolution[1]);
        }
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TEXTURE_INEXISTANT);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

unsigned char* simReadTexture_internal(int textureId, int options, int posX, int posY, int sizeX, int sizeY)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to = App::currentWorld->textureContainer->getObject(textureId);
        if (to != nullptr)
        {
            int resX, resY;
            to->getTextureSize(resX, resY);
            if ((posX >= 0) && (posY >= 0) && (sizeX >= 0) && (sizeY >= 0) && (posX + sizeX <= resX) &&
                (posY + sizeY <= resY))
            {
                if (sizeX == 0)
                {
                    posX = 0;
                    sizeX = resX;
                }
                if (sizeY == 0)
                {
                    posY = 0;
                    sizeY = resY;
                }
                unsigned char* retVal = to->readPortionOfTexture(posX, posY, sizeX, sizeY);
                return (retVal);
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simWriteTexture_internal(int textureId, int options, const char* data, int posX, int posY, int sizeX, int sizeY,
                             double interpol)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to = App::currentWorld->textureContainer->getObject(textureId);
        if (to != nullptr)
        {
            int resX, resY;
            to->getTextureSize(resX, resY);
            if ((sizeX >= 0) && (sizeY >= 0))
            {
                if (sizeX == 0)
                {
                    posX = 0;
                    sizeX = resX;
                }
                if (sizeY == 0)
                {
                    posY = 0;
                    sizeY = resY;
                }
                int retVal = -1;
                if (to->writePortionOfTexture((unsigned char*)data, posX, posY, sizeX, sizeY, (options & 4) != 0,
                                              interpol))
                    retVal = 1;
                return (retVal);
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreateTexture_internal(const char* fileName, int options, const double* planeSizes, const double* scalingUV,
                              const double* xy_g, int fixedResolution, int* textureId, int* resolution,
                              const void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (strlen(fileName) != 0)
        { // loading a texture
            if (VFile::doesFileExist(fileName))
            {
                int resX, resY, n;
                unsigned char* data = CImageLoaderSaver::load(fileName, &resX, &resY, &n, 0, fixedResolution);
                bool rgba = (n == 4);
                if (n < 3)
                {
                    delete[] data;
                    data = nullptr;
                }
                if (data != nullptr)
                {
                    C3Vector s(0.1, 0.1, 0.00001);
                    if (planeSizes != nullptr)
                        s = C3Vector(tt::getLimitedFloat(0.00001, 100000.0, planeSizes[0]),
                                     tt::getLimitedFloat(0.00001, 100000.0, planeSizes[1]), 0.00001);
                    CShape* shape = CAddOperations::addPrimitiveShape(sim_primitiveshape_plane, s);

                    C7Vector identity;
                    identity.setIdentity();
                    shape->setLocalTransformation(identity);
                    shape->setCulling(false);
                    shape->setVisibleEdges(false);
                    shape->setRespondable(false);
                    shape->setStatic(true);
                    shape->getMesh()->setMass(1.0);

                    if (resolution != nullptr)
                    {
                        resolution[0] = resX;
                        resolution[1] = resY;
                    }

                    CTextureObject* textureObj = new CTextureObject(resX, resY);
                    textureObj->setImage(rgba, false, false, data); // keep false,false
                    textureObj->setObjectName(App::folders->getNameFromFull(fileName).c_str());
                    delete[] data;
                    textureObj->addDependentObject(shape->getObjectHandle(), shape->getSingleMesh()->getUniqueID());
                    int texID = App::currentWorld->textureContainer->addObject(
                        textureObj, false); // might erase the textureObj and return a similar object already present!!
                    CTextureProperty* tp = new CTextureProperty(texID);
                    shape->getSingleMesh()->setTextureProperty(tp);
                    tp->setInterpolateColors((options & 1) == 0);
                    if ((options & 2) != 0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    tp->setRepeatU((options & 4) != 0);
                    tp->setRepeatV((options & 8) != 0);
                    if (scalingUV != nullptr)
                        tp->setTextureScaling(scalingUV[0], scalingUV[1]);
                    else
                        tp->setTextureScaling(s(0), s(1));
                    if (xy_g != nullptr)
                    {
                        C7Vector tr;
                        tr.setIdentity();
                        tr.X(0) = xy_g[0];
                        tr.X(1) = xy_g[1];
                        tr.Q = C4Vector(0.0, 0.0, xy_g[2]);
                        tp->setTextureRelativeConfig(tr);
                    }
                    if (textureId != nullptr)
                        textureId[0] = texID;
                    return (shape->getObjectHandle());
                }
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_FILE_NOT_FOUND);
        }
        else
        { // just creating a texture (not loading it)
            if (resolution != nullptr)
            {
                C3Vector s(0.1, 0.1, 0.00001);
                if (planeSizes != nullptr)
                    s = C3Vector(tt::getLimitedFloat(0.00001, 100000.0, planeSizes[0]),
                                 tt::getLimitedFloat(0.00001, 100000.0, planeSizes[1]), 0.00001);
                CShape* shape = CAddOperations::addPrimitiveShape(sim_primitiveshape_plane, s);
                C7Vector identity;
                identity.setIdentity();
                shape->setLocalTransformation(identity);
                shape->setCulling(false);
                shape->setVisibleEdges(false);
                shape->setRespondable(false);
                shape->setStatic(true);
                shape->getMesh()->setMass(1.0);

                CTextureObject* textureObj = new CTextureObject(resolution[0], resolution[1]);
                textureObj->setRandomContent();
                textureObj->setObjectName(App::folders->getNameFromFull(fileName).c_str());
                textureObj->addDependentObject(shape->getObjectHandle(), shape->getSingleMesh()->getUniqueID());
                int texID = App::currentWorld->textureContainer->addObject(
                    textureObj, false); // might erase the textureObj and return a similar object already present!!
                CTextureProperty* tp = new CTextureProperty(texID);
                shape->getSingleMesh()->setTextureProperty(tp);
                tp->setInterpolateColors((options & 1) == 0);
                if ((options & 2) != 0)
                    tp->setApplyMode(1);
                else
                    tp->setApplyMode(0);
                tp->setRepeatU((options & 4) != 0);
                tp->setRepeatV((options & 8) != 0);
                if (scalingUV != nullptr)
                    tp->setTextureScaling(scalingUV[0], scalingUV[1]);
                else
                    tp->setTextureScaling(s(0), s(1));
                if (xy_g != nullptr)
                {
                    C7Vector tr;
                    tr.setIdentity();
                    tr.X(0) = xy_g[0];
                    tr.X(1) = xy_g[1];
                    tr.Q = C4Vector(0.0, 0.0, xy_g[2]);
                    tp->setTextureRelativeConfig(tr);
                }
                if (textureId != nullptr)
                    textureId[0] = texID;
                return (shape->getObjectHandle());
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
        }
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetShapeGeomInfo_internal(int shapeHandle, int* intData, double* floatData, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1; // means error
        if (isShape(__func__, shapeHandle))
        {
            CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            retVal = 0;
            if (shape->getMesh()->isMesh())
            {
                CMesh* geom = shape->getSingleMesh();
                if (intData != nullptr)
                {
                    intData[0] = geom->getPurePrimitiveType();
                }
                if (floatData != nullptr)
                {
                    C3Vector s;
                    geom->getPurePrimitiveSizes(s);
                    floatData[0] = s(0);
                    floatData[1] = s(1);
                    floatData[2] = s(2);
                    floatData[3] = geom->getPurePrimitiveInsideScaling_OLD();
                }
                if (geom->isPure())
                    retVal |= 2;
                if (geom->isConvex())
                    retVal |= 4;
            }
            else
            { // we have a compound...
                retVal |= 1;
                if (shape->getMesh()->isPure())
                    retVal |= 2;
                if (shape->getMesh()->isConvex())
                    retVal |= 4;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetObjects_internal(int index, int objectType)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int cnter = 0;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
        {
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if (objectType != sim_handle_all)
            {
                if (it->getObjectType() != objectType)
                    it = nullptr;
            }
            if (it != nullptr)
            {
                if (cnter == index)
                {
                    int retVal = it->getObjectHandle();
                    return (retVal);
                }
                cnter++;
            }
        }
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int* simGetObjectsInTree_internal(int treeBaseHandle, int objectType, int options, int* objectCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int* retVal = nullptr;
        if ((treeBaseHandle == sim_handle_scene) || doesObjectExist(__func__, treeBaseHandle))
        {
            std::vector<CSceneObject*> toExplore;
            if (treeBaseHandle != sim_handle_scene)
            {
                CSceneObject* baseObj = App::currentWorld->sceneObjects->getObjectFromHandle(treeBaseHandle);
                toExplore.push_back(baseObj);
            }
            else
            {
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getOrphanCount(); i++)
                {
                    CSceneObject* obj = App::currentWorld->sceneObjects->getOrphanFromIndex(i);
                    toExplore.push_back(obj);
                }
            }
            std::vector<int> outHandles;
            bool firstChildrenDone = false;
            while (toExplore.size() != 0)
            {
                CSceneObject* obj = toExplore[0];
                toExplore.erase(toExplore.begin(), toExplore.begin() + 1);
                if ((treeBaseHandle != sim_handle_scene) || ((options & 2) == 0))
                {
                    if ((!firstChildrenDone) || ((options & 2) == 0))
                    {
                        for (size_t i = 0; i < obj->getChildCount(); i++)
                            toExplore.push_back(obj->getChildFromIndex(i));
                    }
                    firstChildrenDone = true;
                }
                if ((obj->getObjectType() == objectType) || (objectType == sim_handle_all))
                {
                    if (((options & 1) == 0) || (obj->getObjectHandle() != treeBaseHandle))
                    {
                        if (((options & 2) == 0) || (treeBaseHandle != sim_handle_scene))
                            outHandles.push_back(obj->getObjectHandle());
                        else
                        { // only first children! Just handle the scene part here
                            if (obj->getParent() == nullptr)
                                outHandles.push_back(obj->getObjectHandle());
                        }
                    }
                }
            }
            retVal = new int[outHandles.size()];
            for (int i = 0; i < int(outHandles.size()); i++)
                retVal[i] = outHandles[i];
            objectCount[0] = int(outHandles.size());
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simScaleObject_internal(int objectHandle, double xScale, double yScale, double zScale, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (doesObjectExist(__func__, objectHandle))
        {
            CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

            if ((xScale >= 0.0001) && (yScale >= 0.0001) && (zScale >= 0.0001) && obj->scaleObjectNonIsometrically(xScale, yScale, zScale))
                return (1);
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_INPUT);
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetShapeTextureId_internal(int shapeHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (isShape(__func__, shapeHandle))
        {
            CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMesh()->isMesh())
            {
                CTextureProperty* tp = shape->getSingleMesh()->getTextureProperty();
                if (tp != nullptr)
                {
                    CTextureObject* to = App::currentWorld->textureContainer->getObject(tp->getTextureObjectID());
                    return (to->getObjectID());
                }
                return (-1);
            }
            return (-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetShapeTexture_internal(int shapeHandle, int textureId, int mappingMode, int options, const double* uvScaling,
                                const double* position, const double* orientation)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__, shapeHandle))
        {
            CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            CTextureObject* to = nullptr;
            if (textureId != -1)
                to = App::currentWorld->textureContainer->getObject(textureId);
            std::vector<CMesh*> meshItems;
            shape->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, meshItems);
            for (size_t i = 0; i < meshItems.size(); i++)
            {
                CMesh* mesh = meshItems[i];
                CTextureProperty* tp = mesh->getTextureProperty();
                if (tp != nullptr)
                { // first remove any existing texture:
                    //         App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                    delete tp;
                    mesh->setTextureProperty(nullptr);
                }
                if (to != nullptr)
                {
                    to->addDependentObject(shape->getObjectHandle(), mesh->getUniqueID());
                    tp = new CTextureProperty(textureId);
                    mesh->setTextureProperty(tp);
                    tp->setTextureMapMode(mappingMode);
                    mesh->setTextureInterpolate((options & 1) == 0);
                    if ((options & 2) != 0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    mesh->setTextureRepeatU((options & 4) != 0);
                    mesh->setTextureRepeatV((options & 8) != 0);
                    tp->setTextureScaling(uvScaling[0], uvScaling[1]);
                    C7Vector tr;
                    tr.setIdentity();
                    if (position != nullptr)
                        tr.X.setData(position);
                    if (orientation != nullptr)
                        tr.Q = C4Vector(orientation[0], orientation[1], orientation[2]);
                    tp->setTextureRelativeConfig(tr);
                }
            }
            return 1;
        }
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreateCollectionEx_internal(int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CCollection* it = new CCollection(_currentScriptHandle);
        it->setCollectionName("___col___", false); // is actually not used anymore
        App::currentWorld->collections->addCollection(it, false);
        it->setOverridesObjectMainProperties((options & 1) != 0);
        return (it->getCollectionHandle());
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simAddItemToCollection_internal(int collectionHandle, int what, int objectHandle, int options)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__, collectionHandle))
            return (-1);
        CCollection* it = App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (what != sim_handle_all)
        {
            if (!doesObjectExist(__func__, objectHandle))
                return (-1);
        }
        CCollectionElement* el = nullptr;
        if (what == sim_handle_all)
            el = new CCollectionElement(-1, sim_collectionelement_all, true);
        if (what == sim_handle_single)
            el = new CCollectionElement(objectHandle, sim_collectionelement_loose, (options & 1) == 0);
        if (what == sim_handle_tree)
        {
            int what = sim_collectionelement_frombaseincluded;
            if ((options & 2) != 0)
                what = sim_collectionelement_frombaseexcluded;
            el = new CCollectionElement(objectHandle, what, (options & 1) == 0);
        }
        if (what == sim_handle_chain)
        {
            int what = sim_collectionelement_fromtipincluded;
            if ((options & 2) != 0)
                what = sim_collectionelement_fromtipexcluded;
            el = new CCollectionElement(objectHandle, what, (options & 1) == 0);
        }
        if (el == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENT);
            return (-1);
        }
        it->addCollectionElement(el);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simDestroyCollection_internal(int collectionHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CCollection* it = App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (it == nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COLLECTION_INEXISTANT);
            return (-1);
        }
        App::currentWorld->collections->removeCollection(collectionHandle);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int* simGetCollectionObjects_internal(int collectionHandle, int* objectCount)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesCollectionExist(__func__, collectionHandle))
        {
            CCollection* coll = App::currentWorld->collections->getObjectFromHandle(collectionHandle);
            objectCount[0] = int(coll->getSceneObjectCountInCollection());
            int* retVal = new int[objectCount[0]];
            for (int i = 0; i < objectCount[0]; i++)
                retVal[i] = coll->getSceneObjectHandleFromIndex(i);
            return (retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simAlignShapeBB_internal(int shapeHandle, const double* pose)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* theShape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        if ((!theShape->getMesh()->isPure()) || (theShape->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (pose == nullptr)
                theShape->alignBB("mesh");
            else
            {
                if (!isFloatArrayOk(pose, 7))
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                    return (-1);
                }
                C7Vector tr;
                tr.setData(pose, true);
                if ((tr.Q(0) == 0.0) && (tr.Q(1) == 0.0) && (tr.Q(2) == 0.0) && (tr.Q(3) == 0.0))
                    theShape->alignBB("mesh");
                else
                {
                    tr.Q.normalize();
                    theShape->alignBB("custom", &tr);
                }
            }
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simRelocateShapeFrame_internal(int shapeHandle, const double* pose)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isShape(__func__, shapeHandle))
            return (-1);
        CShape* theShape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        if ((!theShape->getMesh()->isPure()) || (theShape->isCompound()))
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (pose == nullptr)
                theShape->relocateFrame("mesh");
            else
            {
                if (!isFloatArrayOk(pose, 7))
                {
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                    return (-1);
                }
                C7Vector tr;
                tr.setData(pose, true);
                if ((tr.Q(0) == 0.0) && (tr.Q(1) == 0.0) && (tr.Q(2) == 0.0) && (tr.Q(3) == 0.0))
                    theShape->relocateFrame("mesh");
                else
                {
                    tr.Q.normalize();
                    C7Vector x(tr.getInverse() * theShape->getCumulativeTransformation());
                    theShape->setLocalTransformation(theShape->getFullParentCumulativeTransformation().getInverse() *
                                                     x);
                    theShape->relocateFrame("world");
                    theShape->setLocalTransformation(theShape->getLocalTransformation() * tr);
                }
            }
            return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simSaveImage_internal(const unsigned char* image, const int* resolution, int options, const char* filename,
                          int quality, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = -1;
        if (CImageLoaderSaver::save(image, resolution, options, filename, quality, nullptr))
            retVal = 1;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

unsigned char* simLoadImage_internal(int* resolution, int options, const char* filename, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal = CImageLoaderSaver::load(resolution, options, filename, reserved);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

unsigned char* simGetScaledImage_internal(const unsigned char* imageIn, const int* resolutionIn, int* resolutionOut,
                                          int options, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal = CImageLoaderSaver::getScaledImage(imageIn, resolutionIn, resolutionOut, options);
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simTransformImage_internal(unsigned char* image, const int* resolution, int options, const double* floatParams,
                               const int* intParams, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (CImageLoaderSaver::transformImage(image, resolution[0], resolution[1], options))
            return (1);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetQHull_internal(const double* inVertices, int inVerticesL, double** verticesOut, int* verticesOutL,
                         int** indicesOut, int* indicesOutL, int reserved1, const double* reserved2)
{
    C_API_START;

    if (!isFloatArrayOk(inVertices, inVerticesL))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = 0;
        std::vector<double> vIn(inVertices, inVertices + inVerticesL);
        std::vector<double> vOut;
        std::vector<int> iOut;
        bool res = CMeshRoutines::getConvexHull(vIn, vOut, iOut);
        if (res)
        {
            verticesOut[0] = new double[vOut.size()];
            verticesOutL[0] = (int)vOut.size();
            for (size_t i = 0; i < vOut.size(); i++)
                verticesOut[0][i] = vOut[i];
            if (indicesOut != nullptr)
            {
                indicesOut[0] = new int[iOut.size()];
                indicesOutL[0] = (int)iOut.size();
                for (size_t i = 0; i < iOut.size(); i++)
                    indicesOut[0][i] = iOut[i];
            }
            retVal = 1;
        }
        else
        {
            verticesOut[0] = nullptr;
            verticesOutL[0] = 0;
            if (indicesOut != nullptr)
            {
                indicesOut[0] = nullptr;
                indicesOutL[0] = 0;
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetDecimatedMesh_internal(const double* inVertices, int inVerticesL, const int* inIndices, int inIndicesL,
                                 double** verticesOut, int* verticesOutL, int** indicesOut, int* indicesOutL,
                                 double decimationPercent, int reserved1, const double* reserved2)
{
    C_API_START;

    if (!isFloatArrayOk(inVertices, inVerticesL))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal = 0;
        std::vector<double> vOut;
        std::vector<int> iOut;
        std::vector<double> vIn(inVertices, inVertices + inVerticesL);
        std::vector<int> iIn(inIndices, inIndices + inIndicesL);
        bool res = CMeshRoutines::getDecimatedMesh(vIn, iIn, decimationPercent, vOut, iOut,
                                                   App::userSettings->identicalVertexTolerance);
        if (res)
        {
            verticesOut[0] = new double[vOut.size()];
            verticesOutL[0] = (int)vOut.size();
            for (size_t i = 0; i < vOut.size(); i++)
                verticesOut[0][i] = vOut[i];
            indicesOut[0] = new int[iOut.size()];
            indicesOutL[0] = (int)iOut.size();
            for (size_t i = 0; i < iOut.size(); i++)
                indicesOut[0][i] = iOut[i];
            retVal = 1;
        }
        else
        {
            verticesOut[0] = nullptr;
            verticesOutL[0] = 0;
            indicesOut[0] = nullptr;
            indicesOutL[0] = 0;
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCallScriptFunctionEx_internal(int scriptHandle, const char* functionName, int stackId)
{
    C_API_START;
    CScriptObject* script = nullptr;
    std::string funcName;

    // printf("ScriptHandle, funcName: %i, %s\n", scriptHandle, functionName);

    int handleFlags = 0;
    if (scriptHandle >= 0)
    {
        handleFlags = scriptHandle & 0x0ff00000;
        scriptHandle = scriptHandle & 0x000fffff;
    }
    else
    {
        if (scriptHandle == -1)
            scriptHandle = simGetScriptHandleEx_internal(sim_scripttype_sandbox, -1, nullptr);
        if (scriptHandle == -2)
            scriptHandle = simGetScriptHandleEx_internal(sim_scripttype_main, -1, nullptr);
    }

    std::string funcNameAtScriptName(functionName);
    int lang = sim_lang_undefined;
    if (boost::algorithm::ends_with(funcNameAtScriptName.c_str(), "@lua"))
    {
        lang = sim_lang_lua;
        funcNameAtScriptName.resize(funcNameAtScriptName.size() - 4);
    }
    else if (boost::algorithm::ends_with(funcNameAtScriptName.c_str(), "@python"))
    {
        lang = sim_lang_python;
        funcNameAtScriptName.resize(funcNameAtScriptName.size() - 7);
    }

    size_t p = funcNameAtScriptName.rfind('@'); // back compat.
    if (p != std::string::npos)
        funcName.assign(funcNameAtScriptName.begin(), funcNameAtScriptName.begin() + p);
    else
        funcName = funcNameAtScriptName;
    script = App::worldContainer->getScriptObjectFromHandle(scriptHandle);

    std::string tmp("External call to simCallScriptFunction failed ('");
    tmp += functionName;
    tmp += "'): ";
    if (script != nullptr)
    {
        int retVal = -1; // error
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackId);
        if (stack != nullptr)
        {
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // very special handling here!
                if (VThread::areThreadIdsSame(script->getThreadedScriptThreadId_old(), VThread::getCurrentThreadId()))
                    retVal = script->callCustomScriptFunction(funcName.c_str(), stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType = 1;
                    d[0] = &callType;
                    d[1] = script;
                    d[2] = (void*)funcName.c_str();
                    d[3] = stack;
                    retVal = CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(), d);
                }
            }
            else
            {
                if (VThread::isSimThread())
                {
                    if (script->getLang() == "lua")
                    {
                        if (lang == sim_lang_python)
                            funcName +=
                                "@python"; // explicit python when Lua script --> generates an error further down
                    }
                    if (script->getLang() == "python")
                    {
                        if (lang == sim_lang_lua)
                            funcName += "@lua"; // explicit lua when Python script
                    }

                    retVal = script->callCustomScriptFunction(funcName.c_str(), stack);
                    if (stack->getStackSize() > 0)
                    { // when the script is a Python script, we must check for other errors, since the call is handled
                        // via sysCall_ext:
                        CInterfaceStackObject* obj = stack->getStackObjectFromIndex(0);
                        if (obj->getObjectType() == sim_stackitem_string)
                        {
                            CInterfaceStackString* str = (CInterfaceStackString*)obj;
                            std::string tmp(str->getValue(nullptr));
                            if (tmp == "_*funcNotFound*_")
                            {
                                retVal = 0;
                                stack->clear();
                            }
                            if (tmp == "_*runtimeError*_")
                            {
                                retVal = -1;
                                stack->clear();
                            }
                        }
                    }
                }
            }
            if (retVal == -1)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION);
                tmp += SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION;
                if ((handleFlags & sim_handleflag_silenterror) == 0)
                    App::logMsg(sim_verbosity_errors, tmp.c_str()); // log error here (special, for easier debugging)
            }
            if (retVal == 0)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
                tmp += SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                if ((handleFlags & sim_handleflag_silenterror) == 0)
                    App::logMsg(sim_verbosity_errors, tmp.c_str()); // log error here (special, for easier debugging)
                retVal = -1;                                        // to stay backward compatible
            }
            if (retVal == 1)
                retVal = 0; // to stay backward compatible
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
            tmp += SIM_ERROR_INVALID_HANDLE;
            if ((handleFlags & sim_handleflag_silenterror) == 0)
                App::logMsg(sim_verbosity_errors, tmp.c_str()); // log error here (special, for easier debugging)
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCRIPT_INEXISTANT);
    tmp += SIM_ERROR_SCRIPT_INEXISTANT;
    if ((handleFlags & sim_handleflag_silenterror) == 0)
        App::logMsg(sim_verbosity_errors, tmp.c_str()); // log error here (special, for easier debugging)

    return (-1);
}

char* simGetExtensionString_internal(int objectHandle, int index, const char* key)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal = nullptr;
        std::string extensionString;
        if (objectHandle == -1)
            extensionString = App::currentWorld->environment->getExtensionString();
        else
        {
            if (doesObjectExist(__func__, objectHandle))
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if ((it->getObjectType() == sim_sceneobject_shape) && (index >= 0))
                {
                    CMesh* geom =
                        ((CShape*)it)->getMesh()->getMeshComponentAtIndex(C7Vector::identityTransformation, index);
                    if (geom != nullptr)
                        extensionString = geom->color.getExtensionString();
                }
                else
                    extensionString = it->getExtensionString();
            }
        }
        if ((key != nullptr) && (strlen(key) != 0) && (extensionString.size() > 0))
        {
            std::string value;
            if (tt::getValueOfKey(key, extensionString.c_str(), value))
                extensionString = value;
            else
                extensionString.clear();
        }
        if (extensionString.size() > 0)
        {
            retVal = new char[extensionString.size() + 1];
            for (size_t i = 0; i < extensionString.size(); i++)
                retVal[i] = extensionString[i];
            retVal[extensionString.size()] = 0;
        }
        return (retVal);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simComputeMassAndInertia_internal(int shapeHandle, double density)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__, shapeHandle))
        {
            CShape* shape = (CShape*)App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->computeMassAndInertia(density))
                return (1);
            return (0);
        }
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreateStack_internal()
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
        return (stack->getId());
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simReleaseStack_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::worldContainer->interfaceStackContainer->destroyStack(stackHandle))
            return (1);
        return (0);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCopyStack_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
            return (App::worldContainer->interfaceStackContainer->createStackCopy(stack)->getId());
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushNullOntoStack_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushNullOntoStack();
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushBoolOntoStack_internal(int stackHandle, bool value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushBoolOntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushInt32OntoStack_internal(int stackHandle, int value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushInt32OntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushInt64OntoStack_internal(int stackHandle, long long int value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushInt64OntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushFloatOntoStack_internal(int stackHandle, float value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushFloatOntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushDoubleOntoStack_internal(int stackHandle, double value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushDoubleOntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushTextOntoStack_internal(int stackHandle, const char* value)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushTextOntoStack(value);
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushStringOntoStack_internal(int stackHandle, const char* value, int stringSize)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stringSize == 0)
                stack->pushTextOntoStack(value);
            else
                stack->pushBinaryStringOntoStack(value, size_t(stringSize));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushBufferOntoStack_internal(int stackHandle, const char* value, int stringSize)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushBufferOntoStack(value, size_t(stringSize));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushUInt8TableOntoStack_internal(int stackHandle, const unsigned char* values, int valueCnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushUCharArrayOntoStack(values, size_t(valueCnt));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushInt32TableOntoStack_internal(int stackHandle, const int* values, int valueCnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushInt32ArrayOntoStack(values, size_t(valueCnt));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushInt64TableOntoStack_internal(int stackHandle, const long long int* values, int valueCnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushInt64ArrayOntoStack(values, size_t(valueCnt));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushFloatTableOntoStack_internal(int stackHandle, const float* values, int valueCnt)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushFloatArrayOntoStack(values, size_t(valueCnt));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushDoubleTableOntoStack_internal(int stackHandle, const double* values, int valueCnt)
{
    C_API_START;

    if (!isFloatArrayOk(values, valueCnt))
    {
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
        return (-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushDoubleArrayOntoStack(values, size_t(valueCnt));
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPushTableOntoStack_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->pushTableOntoStack();
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simInsertDataIntoStackTable_internal(int stackHandle)
{ // stack should have at least: table,key,value (where value is on top of stack)
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->insertDataIntoStackTable())
                return (1);
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackSize_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
            return (stack->getStackSize());
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simPopStackItem_internal(int stackHandle, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            stack->popStackValue(count);
            return (stack->getStackSize());
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simMoveStackItemToTop_internal(int stackHandle, int cIndex)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (cIndex < 0)
                cIndex = stack->getStackSize() + cIndex;
            if (stack->moveStackItemToTop(cIndex))
                return (1);
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_INDEX);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackItemType_internal(int stackHandle, int cIndex)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (cIndex < 0)
                cIndex = stack->getStackSize() + cIndex;
            if ((cIndex >= 0) && (stack->getStackSize() > cIndex))
                return (stack->getStackItemType(cIndex));
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_INDEX);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackStringType_internal(int stackHandle, int cIndex)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (cIndex < 0)
                cIndex = stack->getStackSize() + cIndex;
            if ((cIndex >= 0) && (stack->getStackSize() > cIndex))
            {
                if (stack->getStackItemType(cIndex) == sim_stackitem_string)
                    return (stack->getStackStringType(cIndex));
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_NOT_A_STRING);
                return (-1);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_INDEX);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackBoolValue_internal(int stackHandle, bool* boolValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                bool v;
                if (stack->getStackBoolValue(v))
                {
                    boolValue[0] = 0;
                    if (v)
                        boolValue[0] = 1;
                    return (1);
                }
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackInt32Value_internal(int stackHandle, int* numberValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                int v;
                if (stack->getStackInt32Value(v))
                {
                    numberValue[0] = v;
                    return (1);
                }
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackInt64Value_internal(int stackHandle, long long int* numberValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                long long int v;
                if (stack->getStackInt64Value(v))
                {
                    numberValue[0] = v;
                    return (1);
                }
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackFloatValue_internal(int stackHandle, float* numberValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                float v;
                if (stack->getStackFloatValue(v))
                {
                    numberValue[0] = v;
                    return (1);
                }
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackDoubleValue_internal(int stackHandle, double* numberValue)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                double v;
                if (stack->getStackDoubleValue(v))
                {
                    numberValue[0] = v;
                    return (1);
                }
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

char* simGetStackStringValue_internal(int stackHandle, int* stringSize)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                std::string v;
                if (stack->getStackStringValue(v))
                {
                    if (stringSize != nullptr)
                        stringSize[0] = (int)v.size();
                    char* buff = new char[v.size() + 1];
                    for (size_t i = 0; i < v.size(); i++)
                        buff[i] = v[i];
                    buff[v.size()] = 0;
                    return (buff);
                }
                if (stringSize != nullptr)
                    stringSize[0] = 0;
                return (nullptr);
            }
            if (stringSize != nullptr)
                stringSize[0] = -1;
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (nullptr);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        if (stringSize != nullptr)
            stringSize[0] = -1;
        return (nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    if (stringSize != nullptr)
        stringSize[0] = -1;
    return (nullptr);
}

int simGetStackTableInfo_internal(int stackHandle, int infoType)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                int retVal = stack->getStackTableInfo(infoType);
                return (retVal);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackUInt8Table_internal(int stackHandle, unsigned char* array, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->getStackUCharArray(array, count))
                    return (1);
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackInt32Table_internal(int stackHandle, int* array, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->getStackInt32Array(array, count))
                    return (1);
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackInt64Table_internal(int stackHandle, long long int* array, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->getStackInt64Array(array, count))
                    return (1);
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackFloatTable_internal(int stackHandle, float* array, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->getStackFloatArray(array, count))
                    return (1);
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetStackDoubleTable_internal(int stackHandle, double* array, int count)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->getStackDoubleArray(array, count))
                    return (1);
                return (0);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simUnfoldStackTable_internal(int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                if (stack->unfoldStackTable())
                    return (1);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simDebugStack_internal(int stackHandle, int cIndex)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            std::string buffer;
            stack->printContent(cIndex, buffer);
            App::logMsg(sim_verbosity_none, buffer.c_str());
            return (1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simCreateOctree_internal(double voxelSize, int options, double pointSize, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        COcTree* it = new COcTree();
        it->setCellSize(voxelSize);
        it->setPointSize(int(pointSize + 0.5));
        it->setUseRandomColors(options & 1);
        it->setShowOctree(options & 2);
        it->setUsePointsInsteadOfCubes(options & 4);
        it->setColorIsEmissive(options & 16);
        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCreatePointCloud_internal(double maxVoxelSize, int maxPtCntPerVoxel, int options, double pointSize,
                                 void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPointCloud* it = new CPointCloud();
        it->setCellSize(maxVoxelSize);
        it->setMaxPointCountPerCell(maxPtCntPerVoxel);
        it->setPointSize(int(pointSize + 0.5));
        it->setUseRandomColors(options & 1);
        it->setShowOctree(options & 2);
        it->setDoNotUseCalculationStructure(options & 8);
        it->setColorIsEmissive(options & 16);
        App::currentWorld->sceneObjects->addObjectToScene(it, false, true);
        int retVal = it->getObjectHandle();
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simSetPointCloudOptions_internal(int pointCloudHandle, double maxVoxelSize, int maxPtCntPerVoxel, int options,
                                     double pointSize, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        it->setCellSize(maxVoxelSize);
        it->setMaxPointCountPerCell(maxPtCntPerVoxel);
        it->setPointSize(int(pointSize + 0.5));
        it->setUseRandomColors(options & 1);
        it->setShowOctree(options & 2);
        it->setDoNotUseCalculationStructure(options & 8);
        it->setColorIsEmissive(options & 16);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetPointCloudOptions_internal(int pointCloudHandle, double* maxVoxelSize, int* maxPtCntPerVoxel, int* options,
                                     double* pointSize, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        maxVoxelSize[0] = it->getCellSize();
        maxPtCntPerVoxel[0] = it->getMaxPointCountPerCell();
        pointSize[0] = (double)it->getPointSize();
        options[0] = 0;
        if (it->getUseRandomColors())
            options[0] |= 1;
        if (it->getShowOctree())
            options[0] |= 2;
        if (it->getDoNotUseCalculationStructure())
            options[0] |= 8;
        if (it->getColorIsEmissive())
            options[0] |= 16;
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simInsertVoxelsIntoOctree_internal(int octreeHandle, int options, const double* pts, int ptCnt,
                                       const unsigned char* color, const unsigned int* tag, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (-1);
        if (!isFloatArrayOk(pts, ptCnt * 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if ((tag == nullptr) || (color == nullptr))
        {
            if (color == nullptr)
                it->insertPoints(pts, ptCnt, options & 1, nullptr, options & 2, nullptr, 0);
            else
            {
                std::vector<unsigned int> tags;
                if (options & 2)
                    tags.resize(ptCnt, 0);
                else
                    tags.push_back(0);
                it->insertPoints(pts, ptCnt, options & 1, color, options & 2, &tags[0], 0);
            }
        }
        else
            it->insertPoints(pts, ptCnt, options & 1, color, options & 2, tag, 0);
        int retVal = int(it->getCubePositions()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simRemoveVoxelsFromOctree_internal(int octreeHandle, int options, const double* pts, int ptCnt, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (-1);
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if (pts == nullptr)
            it->clear();
        else
        {
            if (!isFloatArrayOk(pts, ptCnt * 3))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            it->subtractPoints(pts, ptCnt, options & 1);
        }
        int retVal = int(it->getCubePositions()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simInsertPointsIntoPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                           const unsigned char* color, void* optionalValues)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        if (!isFloatArrayOk(pts, ptCnt * 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        double insertionToleranceSaved = it->getInsertionDistanceTolerance();
        int optionalValuesBits = 0;
        if (optionalValues != nullptr)
            optionalValuesBits = ((int*)optionalValues)[0];
        if (optionalValuesBits & 1)
            it->setInsertionDistanceTolerance((double)((float*)optionalValues)[1]);
        it->insertPoints(pts, ptCnt, options & 1, color, options & 2);
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        int retVal = int(it->getPoints()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simRemovePointsFromPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                           double tolerance, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        if (pts == nullptr)
            it->clear();
        else
        {
            if (!isFloatArrayOk(pts, ptCnt * 3))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            it->removePoints(pts, ptCnt, options & 1, tolerance);
        }
        int retVal = int(it->getPoints()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simIntersectPointsWithPointCloud_internal(int pointCloudHandle, int options, const double* pts, int ptCnt,
                                              double tolerance, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        if (pts == nullptr)
            it->clear();
        else
        {
            if (!isFloatArrayOk(pts, ptCnt * 3))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
                return (-1);
            }
            it->intersectPoints(pts, ptCnt, options & 1, tolerance);
        }
        int retVal = int(it->getPoints()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

const double* simGetOctreeVoxels_internal(int octreeHandle, int* ptCnt, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (nullptr);
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        const std::vector<double>* p = it->getCubePositions();
        if (p->size() == 0)
        {
            ptCnt[0] = 0;
            return (nullptr);
        }
        ptCnt[0] = int(p->size()) / 3;
        return (&(p[0])[0]);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

const double* simGetPointCloudPoints_internal(int pointCloudHandle, int* ptCnt, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (nullptr);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        const std::vector<double>* p = it->getPoints();
        if (p->size() == 0)
        {
            ptCnt[0] = 0;
            return (nullptr);
        }
        ptCnt[0] = int(p->size()) / 3;
        return (&(p[0])[0]);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simInsertObjectIntoOctree_internal(int octreeHandle, int objectHandle, int options, const unsigned char* color,
                                       unsigned int tag, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (-1);
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);

        float savedCols[3];
        it->getColor()->getColor(savedCols, sim_colorcomponent_ambient_diffuse);
        float* cptr = it->getColor()->getColorsPtr();
        if (color != nullptr)
        {
            cptr[0] = float(color[0]) / 255.1;
            cptr[1] = float(color[1]) / 255.1;
            cptr[2] = float(color[2]) / 255.1;
        }
        it->insertObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle), tag);
        cptr[0] = savedCols[0];
        cptr[1] = savedCols[1];
        cptr[2] = savedCols[2];
        int retVal = int(it->getCubePositions()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simSubtractObjectFromOctree_internal(int octreeHandle, int objectHandle, int options, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (-1);
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        it->subtractObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle));
        int retVal = int(it->getCubePositions()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simInsertObjectIntoPointCloud_internal(int pointCloudHandle, int objectHandle, int options, double gridSize,
                                           const unsigned char* color, void* optionalValues)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__, pointCloudHandle))
            return (-1);
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        double savedGridSize = it->getBuildResolution();
        it->setBuildResolution(gridSize);
        float savedCols[3];
        it->getColor()->getColor(savedCols, sim_colorcomponent_ambient_diffuse);
        if (color != nullptr)
        {
            it->getColor()->getColorsPtr()[0] = float(color[0]) / 255.1;
            it->getColor()->getColorsPtr()[1] = float(color[1]) / 255.1;
            it->getColor()->getColorsPtr()[2] = float(color[2]) / 255.1;
        }
        double insertionToleranceSaved = it->getInsertionDistanceTolerance();
        int optionalValuesBits = 0;
        if (optionalValues != nullptr)
            optionalValuesBits = ((int*)optionalValues)[0];
        if (optionalValuesBits & 1)
            it->setInsertionDistanceTolerance((double)((float*)optionalValues)[1]);
        it->insertObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle));
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        it->setBuildResolution(savedGridSize);
        it->getColor()->getColorsPtr()[0] = savedCols[0];
        it->getColor()->getColorsPtr()[1] = savedCols[1];
        it->getColor()->getColorsPtr()[2] = savedCols[2];
        int retVal = int(it->getPoints()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simSubtractObjectFromPointCloud_internal(int pointCloudHandle, int objectHandle, int options, double tolerance,
                                             void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__, pointCloudHandle))
            return (-1);
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        it->subtractObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle), tolerance);
        int retVal = int(it->getPoints()->size()) / 3;
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simCheckOctreePointOccupancy_internal(int octreeHandle, int options, const double* points, int ptCnt,
                                          unsigned int* tag, unsigned long long int* location, void* reserved)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__, octreeHandle))
            return (-1);
        if (ptCnt <= 0)
            return (-1);
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if (it->getOctreeInfo() == nullptr)
            return (0);
        if (!isFloatArrayOk(points, ptCnt * 3))
        {
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        const double* _pts = points;
        std::vector<double> __pts;
        if (options & 1)
        {
            C7Vector tr(it->getFullCumulativeTransformation());
            for (int i = 0; i < ptCnt; i++)
            {
                C3Vector v(&points[3 * i]);
                v *= tr;
                __pts.push_back(v(0));
                __pts.push_back(v(1));
                __pts.push_back(v(2));
            }
            _pts = &__pts[0];
        }
        if (ptCnt == 1)
        {
            if (App::worldContainer->pluginContainer->geomPlugin_getOctreePointCollision(
                    it->getOctreeInfo(), it->getFullCumulativeTransformation(), C3Vector(_pts), tag, location))
                return (1);
        }
        else
        {
            if (App::worldContainer->pluginContainer->geomPlugin_getOctreePointsCollision(
                    it->getOctreeInfo(), it->getFullCumulativeTransformation(), _pts, ptCnt))
                return (1);
        }
        return (0);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

char* simOpenTextEditor_internal(const char* initText, const char* xml, int* various)
{
    C_API_START;

    char* retVal = nullptr;
#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
    {
        std::string txt = GuiApp::mainWindow->codeEditorContainer->openModalTextEditor(initText, xml, various, true);
        retVal = new char[txt.size() + 1];
        for (size_t i = 0; i < txt.size(); i++)
            retVal[i] = txt[i];
        retVal[txt.size()] = 0;
    }
#endif
    return (retVal);
}

char* simPackTable_internal(int stackHandle, int* bufferSize)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->getStackSize() > 0)
            {
                std::string buff(stack->getBufferFromTable());
                if (buff.size() > 0)
                {
                    char* bu = new char[buff.size()];
                    bufferSize[0] = int(buff.size());
                    for (size_t i = 0; i < buff.size(); i++)
                        bu[i] = buff[i];
                    return (bu);
                }
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_STACK_CONTENT);
            return (nullptr);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simUnpackTable_internal(int stackHandle, const char* buffer, int bufferSize)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack != nullptr)
        {
            if (stack->pushTableFromBuffer(buffer, bufferSize))
                return (0);
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_DATA);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
        return (-1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simSetReferencedHandles_internal(int objectHandle, int count, const int* referencedHandles, const char* tag,
                                     const int* reserved2)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        std::string ttag("");
        if (tag != nullptr)
            ttag = tag;
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if ((handleFlags & sim_handleflag_keeporiginal) == 0)
            it->setReferencedHandles(size_t(count), referencedHandles, ttag.c_str());
        else
            it->setReferencedOriginalHandles(count, referencedHandles, ttag.c_str());
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetReferencedHandles_internal(int objectHandle, int** referencedHandles, const char* tag, int** reserved2)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = objectHandle & 0xff00000;
        objectHandle = objectHandle & 0xfffff;
        if (!doesObjectExist(__func__, objectHandle))
            return (-1);
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int* handles = nullptr;
        int cnt;
        std::string ttag("");
        if (tag != nullptr)
            ttag = tag;
        if ((handleFlags & sim_handleflag_keeporiginal) == 0)
        {
            cnt = int(it->getReferencedHandlesCount(ttag.c_str()));
            if (cnt > 0)
            {
                handles = new int[cnt];
                it->getReferencedHandles(handles, ttag.c_str());
            }
        }
        else
        {
            cnt = int(it->getReferencedOriginalHandlesCount(ttag.c_str()));
            if (cnt > 0)
            {
                handles = new int[cnt];
                it->getReferencedOriginalHandles(handles, ttag.c_str());
            }
        }
        referencedHandles[0] = handles;
        return (cnt);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetShapeViz_internal(int shapeHandle, int index, struct SShapeVizInfo* info)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = shapeHandle & 0x0ff00000;
        shapeHandle = shapeHandle & 0x000fffff;

        if (!isShape(__func__, shapeHandle))
            return (-1);
        int retVal = 0;
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<CMesh*> all;
        std::vector<C7Vector> allTr;
        it->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, &allTr);
        if ((index >= 0) && (index < int(all.size())))
        {
            CMesh* geom = all[index];

            if ((handleFlags & sim_handleflag_extended) != 0)
            {
                info->transparency = 0.0;
                if (geom->color.getTranslucid())
                    info->transparency = (double)geom->color.getOpacity();
                info->options = 0;
                if (geom->getCulling())
                    info->options |= 1;
                if (geom->getWireframe_OLD())
                    info->options |= 2;
            }

            C7Vector tr(allTr[index]);
            const std::vector<double>* wvert = geom->getVertices();
            const std::vector<int>* wind = geom->getIndices();
            const std::vector<double>* wnorm = geom->getNormals();
            info->verticesSize = int(wvert->size());
            info->vertices = new double[wvert->size()];
            for (size_t i = 0; i < wvert->size() / 3; i++)
            {
                C3Vector v;
                v.setData((&wvert[0][0]) + i * 3);
                v = tr * v;
                info->vertices[3 * i + 0] = v(0);
                info->vertices[3 * i + 1] = v(1);
                info->vertices[3 * i + 2] = v(2);
            }
            info->indicesSize = int(wind->size());
            info->indices = new int[wind->size()];
            info->normals = new double[wind->size() * 3];
            for (size_t i = 0; i < wind->size(); i++)
            {
                info->indices[i] = wind->at(i);
                C3Vector n;
                n.setData(&(wnorm[0])[0] + i * 3);
                n = tr.Q * n; // only orientation
                info->normals[3 * i + 0] = n(0);
                info->normals[3 * i + 1] = n(1);
                info->normals[3 * i + 2] = n(2);
            }
            geom->color.getColor(info->colors + 0, sim_colorcomponent_ambient_diffuse);
            geom->color.getColor(info->colors + 3, sim_colorcomponent_specular);
            geom->color.getColor(info->colors + 6, sim_colorcomponent_emission);
            info->shadingAngle = geom->getShadingAngle();

            CTextureProperty* tp = geom->getTextureProperty();
            CTextureObject* to = nullptr;
            const std::vector<float>* tc = nullptr;
            if (tp != nullptr)
            {
                to = tp->getTextureObject();
                tc = tp->getTextureCoordinates(-1, geom->getVerticesForDisplayAndDisk()[0], wind[0]);
            }

            if ((to != nullptr) && (tc != nullptr))
            {
                retVal = 2;
                to->getTextureSize(info->textureRes[0], info->textureRes[1]);
                size_t totBytes = 4 * info->textureRes[0] * info->textureRes[1];
                info->texture = new char[totBytes];
                const char* ob = (char*)to->getTextureBufferPointer();
                for (size_t i = 0; i < totBytes; i++)
                    info->texture[i] = ob[i];
                info->textureCoords = new float[tc->size()];
                for (size_t i = 0; i < tc->size(); i++)
                    info->textureCoords[i] = tc->at(i);
                info->textureApplyMode = tp->getApplyMode();
                info->textureOptions = 0;
                if (tp->getRepeatU())
                    info->textureOptions |= 1;
                if (tp->getRepeatV())
                    info->textureOptions |= 2;
                if (tp->getInterpolateColors())
                    info->textureOptions |= 4;
                if (geom->getWireframe_OLD())
                    info->textureOptions |= 8;
                info->textureId = tp->getTextureObjectID();
            }
            else
            {
                retVal = 1;
                info->texture = nullptr;
                info->textureCoords = nullptr;
                info->textureId = -1;
                info->textureOptions = 0;
                if (geom->getWireframe_OLD())
                    info->textureOptions |= 8;
            }
            return (retVal);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simGetShapeVizf_internal(int shapeHandle, int index, struct SShapeVizInfof* info)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags = shapeHandle & 0x0ff00000;
        shapeHandle = shapeHandle & 0x000fffff;

        if (!isShape(__func__, shapeHandle))
            return (-1);
        int retVal = 0;
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<CMesh*> all;
        std::vector<C7Vector> allTr;
        it->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, &allTr);
        if ((index >= 0) && (index < int(all.size())))
        {
            CMesh* geom = all[index];

            if ((handleFlags & sim_handleflag_extended) != 0)
            {
                info->transparency = 0.0;
                if (geom->color.getTranslucid())
                    info->transparency = geom->color.getOpacity();
                info->options = 0;
                if (geom->getCulling())
                    info->options |= 1;
                if (geom->getWireframe_OLD())
                    info->options |= 2;
            }

            C7Vector tr(allTr[index]);
            const std::vector<float>* wvert = geom->getVerticesForDisplayAndDisk();
            const std::vector<int>* wind = geom->getIndices();
            const std::vector<float>* wnorm = geom->getNormalsForDisplayAndDisk();
            info->verticesSize = int(wvert->size());
            info->vertices = new float[wvert->size()];
            for (size_t i = 0; i < wvert->size() / 3; i++)
            {
                C3Vector v;
                v.setData((&wvert[0][0]) + i * 3);
                v = tr * v;
                info->vertices[3 * i + 0] = (float)v(0);
                info->vertices[3 * i + 1] = (float)v(1);
                info->vertices[3 * i + 2] = (float)v(2);
            }
            info->indicesSize = int(wind->size());
            info->indices = new int[wind->size()];
            info->normals = new float[wind->size() * 3];
            for (size_t i = 0; i < wind->size(); i++)
            {
                info->indices[i] = wind->at(i);
                C3Vector n;
                n.setData(&(wnorm[0])[0] + i * 3);
                n = tr.Q * n; // only orientation
                info->normals[3 * i + 0] = (float)n(0);
                info->normals[3 * i + 1] = (float)n(1);
                info->normals[3 * i + 2] = (float)n(2);
            }
            geom->color.getColor(info->colors + 0, sim_colorcomponent_ambient_diffuse);
            geom->color.getColor(info->colors + 3, sim_colorcomponent_specular);
            geom->color.getColor(info->colors + 6, sim_colorcomponent_emission);
            info->shadingAngle = (float)geom->getShadingAngle();

            CTextureProperty* tp = geom->getTextureProperty();
            CTextureObject* to = nullptr;
            const std::vector<float>* tc = nullptr;
            if (tp != nullptr)
            {
                to = tp->getTextureObject();
                tc = tp->getTextureCoordinates(-1, wvert[0], wind[0]);
            }

            if ((to != nullptr) && (tc != nullptr))
            {
                retVal = 2;
                to->getTextureSize(info->textureRes[0], info->textureRes[1]);
                size_t totBytes = 4 * info->textureRes[0] * info->textureRes[1];
                info->texture = new char[totBytes];
                const char* ob = (char*)to->getTextureBufferPointer();
                for (size_t i = 0; i < totBytes; i++)
                    info->texture[i] = ob[i];
                info->textureCoords = new float[tc->size()];
                for (size_t i = 0; i < tc->size(); i++)
                    info->textureCoords[i] = tc->at(i);
                info->textureApplyMode = tp->getApplyMode();
                info->textureOptions = 0;
                if (tp->getRepeatU())
                    info->textureOptions |= 1;
                if (tp->getRepeatV())
                    info->textureOptions |= 2;
                if (tp->getInterpolateColors())
                    info->textureOptions |= 4;
                if (geom->getWireframe_OLD())
                    info->textureOptions |= 8;
                info->textureId = tp->getTextureObjectID();
            }
            else
            {
                retVal = 1;
                info->texture = nullptr;
                info->textureCoords = nullptr;
                info->textureId = -1;
                info->textureOptions = 0;
                if (geom->getWireframe_OLD())
                    info->textureOptions |= 8;
            }
            return (retVal);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simExecuteScriptString_internal(int scriptHandle, const char* stringToExecute, int stackHandle)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* script = nullptr;
        std::string stringToExec;

        std::string strAtScriptName(stringToExecute);
        int lang = sim_lang_undefined;
        if (boost::algorithm::ends_with(strAtScriptName.c_str(), "@lua"))
        {
            lang = sim_lang_lua;
            strAtScriptName.resize(strAtScriptName.size() - 4);
        }
        else if (boost::algorithm::ends_with(strAtScriptName.c_str(), "@python"))
        {
            lang = sim_lang_python;
            strAtScriptName.resize(strAtScriptName.size() - 7);
        }

        size_t p = strAtScriptName.rfind('@'); // back compat.
        if (p != std::string::npos)
            stringToExec.assign(strAtScriptName.begin(), strAtScriptName.begin() + p);
        else
            stringToExec = strAtScriptName;
        script = App::worldContainer->getScriptObjectFromHandle(scriptHandle);

        if (script != nullptr)
        {
            bool noReturnDesired = (stackHandle == 0);
            CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->getStack(stackHandle);
            if ((stack == nullptr) && (!noReturnDesired))
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_HANDLE);
                return (-1);
            }
            int retVal = -1; // error
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // OLD, very special handling here!
                if (VThread::areThreadIdsSame(script->getThreadedScriptThreadId_old(), VThread::getCurrentThreadId()))
                    retVal = script->executeScriptString(stringToExec.c_str(), stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType = 3;
                    d[0] = &callType;
                    d[1] = script;
                    d[2] = (void*)stringToExec.c_str();
                    d[3] = stack;

                    retVal = CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(), d);
                }
            }
            else
            {
                if (VThread::isSimThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    if ((script->getLang() == "lua") || (lang == sim_lang_lua))
                        retVal = script->executeScriptString(stringToExec.c_str(), stack);
                    else
                    {
                        if (script->getLang() == "python")
                        {
                            if (script->getScriptState() == CScriptObject::scriptState_initialized)
                            {
                                CInterfaceStack* tmpStack = App::worldContainer->interfaceStackContainer->createStack();
                                tmpStack->pushTextOntoStack(stringToExec.c_str());
                                retVal = script->callCustomScriptFunction("_evalExecRet", tmpStack);
                                if (stack != nullptr)
                                    stack->copyFrom(tmpStack);
                                App::worldContainer->interfaceStackContainer->destroyStack(tmpStack);
                                if (retVal == 1)
                                {
                                    retVal = 0;
                                    if (stack != nullptr)
                                    {
                                        CInterfaceStackObject* obj = stack->getStackObjectFromIndex(0);
                                        if (obj->getObjectType() == sim_stackitem_string)
                                        {
                                            CInterfaceStackString* str = (CInterfaceStackString*)obj;
                                            std::string tmp(str->getValue(nullptr));
                                            if (tmp == "_*empty*_")
                                                stack->clear();
                                        }
                                    }
                                }
                                else
                                    retVal = -1; // error
                            }
                            else
                                retVal = -2; // script not initialized
                        }
                    }
                }
            }

            if (retVal != 0)
            {
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_OPERATION_FAILED);
                retVal = -1;
            }
            return (retVal);
        }

        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_SCRIPT_INEXISTANT);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

char* simGetApiFunc_internal(int scriptHandle, const char* apiWord)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* script = nullptr;
        if (scriptHandle >= SIM_IDSTART_LUASCRIPT)
            script = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
        std::string apiW(apiWord);
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
            CScriptObject::getMatchingFunctions(apiW.c_str(), t, script);
        if (vars)
            CScriptObject::getMatchingConstants(apiW.c_str(), t, script);
        std::string theWords;
        for (const auto& str : t)
        {
            theWords += str;
            theWords += ' ';
        }
        char* buff = nullptr;
        if (theWords.size() > 0)
        {
            theWords.pop_back();
            buff = new char[theWords.size() + 1];
            strcpy(buff, theWords.c_str());
        }
        return (buff);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

char* simGetApiInfo_internal(int scriptHandle, const char* apiWord)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* script = nullptr;
        if (scriptHandle >= SIM_IDSTART_LUASCRIPT)
            script = App::worldContainer->getScriptObjectFromHandle(scriptHandle);
        if (strlen(apiWord) > 0)
        {
            std::string tip(CScriptObject::getFunctionCalltip(apiWord, script));
            char* buff = new char[tip.size() + 1];
            strcpy(buff, tip.c_str());
            return (buff);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENTS);
        return (nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (nullptr);
}

int simSetPluginInfo_internal(const char* pluginName, int infoType, const char* stringInfo, int intInfo)
{
    C_API_START;
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPlugin* plug = nullptr;
        if (pluginName == nullptr)
            plug = App::worldContainer->pluginContainer->getCurrentPlugin();
        else
        {
            plug = App::worldContainer->pluginContainer->getPluginFromName(pluginName);
            if (plug == nullptr)
                plug = App::worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        }
        if (plug != nullptr)
        {
            if (infoType == sim_moduleinfo_extversionstr)
            {
                plug->setExtendedVersionString(stringInfo);
                return (1);
            }
            if (infoType == sim_moduleinfo_builddatestr)
            {
                plug->setBuildDateString(stringInfo);
                return (1);
            }
            if (infoType == sim_moduleinfo_extversionint)
            {
                plug->setExtendedVersionInt(intInfo);
                return (1);
            }
            if (infoType == sim_moduleinfo_verbosity)
            {
                App::setConsoleVerbosity(intInfo, pluginName);
                return (1);
            }
            if (infoType == sim_moduleinfo_statusbarverbosity)
            {
                App::setStatusbarVerbosity(intInfo, pluginName);
                return (1);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PLUGIN_NAME);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (-1);
}

int simGetPluginInfo_internal(const char* pluginName, int infoType, char** stringInfo, int* intInfo)
{
    C_API_START;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug = nullptr;
        if (pluginName == nullptr)
            plug = App::worldContainer->pluginContainer->getCurrentPlugin();
        else
        {
            plug = App::worldContainer->pluginContainer->getPluginFromName(pluginName);
            if (plug == nullptr)
                plug = App::worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        }
        if (plug != nullptr)
        {
            if (infoType == sim_moduleinfo_extversionstr)
            {
                std::string str(plug->getExtendedVersionString());
                char* txt = new char[str.length() + 1];
                strcpy(txt, str.c_str());
                if (stringInfo != nullptr)
                    stringInfo[0] = txt;
                else
                    delete[] txt;
                return (1);
            }
            if (infoType == sim_moduleinfo_builddatestr)
            {
                std::string str(plug->getBuildDateString());
                char* txt = new char[str.length() + 1];
                strcpy(txt, str.c_str());
                if (stringInfo != nullptr)
                    stringInfo[0] = txt;
                else
                    delete[] txt;
                return (1);
            }
            if (infoType == sim_moduleinfo_extversionint)
            {
                intInfo[0] = plug->getExtendedVersionInt();
                return (1);
            }
            if (infoType == sim_moduleinfo_verbosity)
            {
                intInfo[0] = App::getConsoleVerbosity(pluginName);
                return (1);
            }
            if (infoType == sim_moduleinfo_statusbarverbosity)
            {
                intInfo[0] = App::getStatusbarVerbosity(pluginName);
                return (1);
            }
            CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_ARGUMENT);
            return (-1);
        }
        CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_INVALID_PLUGIN_NAME);
        return (-1);
    }

    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

int simEventNotification_internal(const char* event)
{
    C_API_START;
    int retVal = -1;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        tinyxml2::XMLDocument xmldoc;
        tinyxml2::XMLError error = xmldoc.Parse(event);
        if (error == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement* rootElement = xmldoc.FirstChildElement();
            const char* origin = rootElement->Attribute("origin");
            if (origin != nullptr)
            {
                if (strcmp(origin, "codeEditor") == 0)
                {
                    const char* msg = rootElement->Attribute("msg");
                    const char* handle = rootElement->Attribute("handle");
                    const char* data = rootElement->Attribute("data");
#ifdef SIM_WITH_GUI
                    if ((msg != nullptr) && (handle != nullptr) && (data != nullptr) && (GuiApp::mainWindow != nullptr))
                    {
                        if (strcmp(msg, "closeEditor") == 0)
                        {
                            int h;
                            if (tt::stringToInt(handle, h))
                            {
                                if (strlen(data) != 0)
                                {
                                    int callingScript = GuiApp::mainWindow->codeEditorContainer->getCallingScriptHandle(h);
                                    CInterfaceStack* stack = App::worldContainer->interfaceStackContainer->createStack();
                                    int posAndSize[4];
                                    std::string txt = GuiApp::mainWindow->codeEditorContainer->getText(h, posAndSize);
                                    stack->pushTextOntoStack(txt.c_str());
                                    stack->pushInt32ArrayOntoStack(posAndSize + 0, 2);
                                    stack->pushInt32ArrayOntoStack(posAndSize + 2, 2);
                                    simCallScriptFunctionEx_internal(callingScript, data, stack->getId());
                                    App::worldContainer->interfaceStackContainer->destroyStack(stack);
                                }
                                if ((strlen(data) == 0) ||
                                    GuiApp::mainWindow->codeEditorContainer->getCloseAfterCallbackCalled(h))
                                    GuiApp::mainWindow->codeEditorContainer->close(h, nullptr, nullptr, nullptr);
                                retVal = 1;
                            }
                        }
                        if (strcmp(msg, "restartScript") == 0)
                        {
                            int h;
                            if (tt::stringToInt(handle, h))
                            {
                                GuiApp::mainWindow->codeEditorContainer->restartScript(h);
                                retVal = 1;
                            }
                        }
                    }
#endif
                }
            }
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (retVal);
}

int simApplyTexture_internal(int shapeHandle, const double* textureCoordinates, int textCoordSize,
                             const unsigned char* texture, const int* textureResolution, int options)
{
    C_API_START;
    int retVal = -1;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__, shapeHandle))
        {
            CShape* shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMesh()->isMesh())
            {
                // first remove any existing texture:
                CTextureProperty* tp = shape->getSingleMesh()->getTextureProperty();
                if (tp != nullptr)
                {
                    App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(), -1);
                    delete tp;
                    shape->getSingleMesh()->setTextureProperty(nullptr);
                }
                if (shape->getSingleMesh()->getIndices()->size() * 2 == textCoordSize)
                {
                    // Now create and attach the texture:
                    CTextureObject* textureObj = new CTextureObject(textureResolution[0], textureResolution[1]);
                    textureObj->setImage(options & 16, options & 32, (options & 64) == 0, texture);
                    textureObj->setObjectName("importedTexture");
                    textureObj->addDependentObject(shape->getObjectHandle(), shape->getSingleMesh()->getUniqueID());
                    retVal = App::currentWorld->textureContainer->addObject(
                        textureObj, false); // might erase the textureObj and return a similar object already present!!
                    tp = new CTextureProperty(retVal);
                    shape->getSingleMesh()->setTextureProperty(tp);
                    shape->getSingleMesh()->setTextureInterpolate((options & 1) == 0);
                    if ((options & 2) != 0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    std::vector<float> c;
                    c.resize(textCoordSize);
                    for (int i = 0; i < textCoordSize; i++)
                        c[i] = (float)textureCoordinates[i];
                    tp->setFixedCoordinates(&c);
                }
                else
                    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_BAD_TEXTURE_COORD_SIZE);
            }
            else
                CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_CANNOT_BE_COMPOUND_SHAPE);
        }
        return (retVal);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (retVal);
}

int simSetJointDependency_internal(int jointHandle, int masterJointHandle, double offset, double multCoeff)
{
    C_API_START;
    int retVal = -1;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isJoint(__func__, jointHandle))
        {
            if ((masterJointHandle == -1) || isJoint(__func__, masterJointHandle))
            {
                CJoint* joint = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
                joint->setDependencyMasterJointHandle(masterJointHandle);
                if (joint->getDependencyMasterJointHandle() == masterJointHandle)
                {
                    joint->setDependencyParams(offset, multCoeff);
                    retVal = 0;
                    return (retVal);
                }
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (retVal);
}

int simGetJointDependency_internal(int jointHandle, int* masterJointHandle, double* offset, double* multCoeff)
{
    C_API_START;
    int retVal = -1;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isJoint(__func__, jointHandle))
        {
            CJoint* joint = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            masterJointHandle[0] = joint->getDependencyMasterJointHandle();
            joint->getDependencyParams(offset[0], multCoeff[0]);
            retVal = 0;
            return (retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return (retVal);
}

//************************************************************************************************************
//************************************************************************************************************
// FOLLOWING FUNCTIONS ARE VERY FAST, BUT NO SPECIFIC CHECKING IS DONE. ALSO, MANY OPERATE ON OBJECT POINTERS!
//************************************************************************************************************
//************************************************************************************************************

const void* _simGetGeomWrapFromGeomProxy_internal(const void* geomData)
{
    C_API_START;
    return (((CShape*)geomData)->getMesh());
}

double _simGetMass_internal(const void* geomInfo)
{
    C_API_START;
    return (((CMeshWrapper*)geomInfo)->getMass());
}

double _simGetLocalInertiaInfo_internal(const void* object, double* pos, double* quat, double* diag)
{ // returns the diag inertia (with mass!)
    CShape* shape = (CShape*)object;
    double mass = shape->getMesh()->getMass();
    C3Vector diagI;
    C7Vector localTr(shape->getMesh()->getDiagonalInertiaInfo(diagI));
    if (App::currentWorld->dynamicsContainer->getComputeInertias())
    {
        if (shape->getMesh()->isPure())
            mass = App::worldContainer->pluginContainer->dyn_computeInertia(shape->getObjectHandle(), localTr, diagI);
        else
        { // we use the convex hull
            std::vector<double> vert;
            shape->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation, vert, nullptr, nullptr);
            std::vector<double> hull;
            std::vector<int> indices;
            if (CMeshRoutines::getConvexHull(vert, hull, indices))
                mass = App::worldContainer->pluginContainer->dyn_computePMI(hull, indices, localTr, diagI);
        }
    }
    if (mass > 0.0)
    {
        localTr.X.getData(pos);
        localTr.Q.getData(quat);
        diagI = diagI * mass;
        diagI.getData(diag);
    }
    return (mass);
}

int _simGetPurePrimitiveType_internal(const void* geomInfo)
{
    C_API_START;
    return (((CMeshWrapper*)geomInfo)->getPurePrimitiveType());
}

void _simGetPurePrimitiveSizes_internal(const void* geometric, double* sizes)
{
    C_API_START;
    C3Vector s;
    ((CMesh*)geometric)->getPurePrimitiveSizes(s);
    s.getData(sizes);
}

bool _simIsGeomWrapGeometric_internal(const void* geomInfo)
{
    C_API_START;
    return (((CMeshWrapper*)geomInfo)->isMesh());
}

bool _simIsGeomWrapConvex_internal(const void* geomInfo)
{
    C_API_START;
    return (((CMeshWrapper*)geomInfo)->isConvex());
}

int _simGetGeometricCount_internal(const void* geomInfo)
{
    C_API_START;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all);
    return ((int)all.size());
}

void _simGetAllGeometrics_internal(const void* geomInfo, void** allGeometrics)
{
    C_API_START;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all);
    for (size_t i = 0; i < all.size(); i++)
        allGeometrics[i] = all[i];
}

void _simMakeDynamicAnnouncement_internal(int announceType)
{
    C_API_START;
    if (announceType == sim_announce_pureconenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureConeNotSupported();
    if (announceType == sim_announce_purespheroidnotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureSpheroidNotSupported();
    if (announceType == sim_announce_containsnonpurenonconvexshapes)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsNonPureNonConvexShapes();
    if (announceType == sim_announce_containsstaticshapesondynamicconstruction)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsStaticShapesOnDynamicConstruction();
    if (announceType == sim_announce_purehollowshapenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureHollowShapeNotSupported();
    if (announceType == sim_announce_vortexpluginisdemo)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_vortexPluginIsDemo();
}

void _simGetVerticesLocalFrame_internal(const void* shape, const void* geometric, double* pos, double* quat)
{
    C_API_START;
    C7Vector tr;
    ((CShape*)shape)
        ->getMesh()
        ->getShapeRelBB(C7Vector::identityTransformation, (CMeshWrapper*)geometric, tr, nullptr);
    tr.Q.getData(quat);
    tr.X.getData(pos);
}

const double* _simGetHeightfieldData_internal(const void* geometric, int* xCount, int* yCount, double* minHeight,
                                              double* maxHeight)
{
    C_API_START;
    return (((CMesh*)geometric)->getHeightfieldData(xCount[0], yCount[0], minHeight[0], maxHeight[0]));
}

void _simGetCumulativeMeshes_internal(const void* shape, const void* geomInfo, double** vertices, int* verticesSize,
                                      int** indices, int* indicesSize)
{
    C_API_START;
    std::vector<double> vert;
    std::vector<int> ind;
    ((CShape*)shape)
        ->getMesh()
        ->getCumulativeMeshes(C7Vector::identityTransformation, (CMeshWrapper*)geomInfo, vert, &ind, nullptr);

    vertices[0] = new double[vert.size()];
    verticesSize[0] = (int)vert.size();
    for (size_t i = 0; i < vert.size(); i++)
        vertices[0][i] = vert[i];
    indices[0] = new int[ind.size()];
    indicesSize[0] = (int)ind.size();
    for (size_t i = 0; i < ind.size(); i++)
        indices[0][i] = ind[i];
}

int _simGetObjectID_internal(const void* object)
{
    C_API_START;
    return (((CSceneObject*)object)->getObjectHandle());
}

void _simGetObjectLocalTransformation_internal(const void* object, double* pos, double* quat,
                                               bool excludeFirstJointTransformation)
{
    C_API_START;
    C7Vector tr;
    if (excludeFirstJointTransformation)
        tr = ((CSceneObject*)object)->getLocalTransformation();
    else
        tr = ((CSceneObject*)object)->getFullLocalTransformation();
    tr.X.getData(pos);
    tr.Q.getData(quat);
}

void _simSetObjectLocalTransformation_internal(void* object, const double* pos, const double* quat, double simTime)
{
    C_API_START;
    C7Vector tr;
    tr.X.setData(pos);
    tr.Q.setData(quat);
    ((CSceneObject*)object)->setLocalTransformation(tr);
}

void _simGetObjectCumulativeTransformation_internal(const void* object, double* pos, double* quat,
                                                    bool excludeFirstJointTransformation)
{
    C_API_START;
    C7Vector tr;
    CSceneObject* obj = (CSceneObject*)object;
    if (excludeFirstJointTransformation != 0)
        tr = obj->getCumulativeTransformation();
    else
        tr = obj->getFullCumulativeTransformation();

    if (obj->getObjectType() == sim_sceneobject_shape)
    {
        CShape* shape = (CShape*)obj;
        if (shape->getMesh()->getPurePrimitiveType() == sim_primitiveshape_heightfield)
        { // Special handling with Heightfields (all data for physics engines need to be centered with heightfields)
            tr *= shape->getMesh()->getBB(nullptr);
        }
    }

    if (pos != nullptr)
        tr.X.getData(pos);
    if (quat != nullptr)
        tr.Q.getData(quat);
}

bool _simIsShapeDynamicallyStatic_internal(const void* shape)
{
    C_API_START;
    return (((CShape*)shape)->getStatic());
}

void _simGetInitialDynamicVelocity_internal(const void* shape, double* vel)
{
    C_API_START;
    ((CShape*)shape)->getInitialDynamicLinearVelocity().getData(vel);
}

void _simSetInitialDynamicVelocity_internal(void* shape, const double* vel)
{
    C_API_START;
    if (!isNanOrInf(vel, 3))
        ((CShape*)shape)->setInitialDynamicLinearVelocity(C3Vector(vel));
}

void _simGetInitialDynamicAngVelocity_internal(const void* shape, double* angularVel)
{
    C_API_START;
    ((CShape*)shape)->getInitialDynamicAngularVelocity().getData(angularVel);
}

void _simSetInitialDynamicAngVelocity_internal(void* shape, const double* angularVel)
{
    C_API_START;
    if (!isNanOrInf(angularVel, 3))
        ((CShape*)shape)->setInitialDynamicAngularVelocity(C3Vector(angularVel));
}

bool _simGetStartSleeping_internal(const void* shape)
{
    C_API_START;
    return (((CShape*)shape)->getStartInDynamicSleeping());
}

bool _simGetWasPutToSleepOnce_internal(const void* shape)
{ // flag is set to true whenever called!!!
    C_API_START;
    bool a = ((CShape*)shape)->getRigidBodyWasAlreadyPutToSleepOnce();
    ((CShape*)shape)->setRigidBodyWasAlreadyPutToSleepOnce(true);
    return (a);
}

bool _simIsShapeDynamicallyRespondable_internal(const void* shape)
{
    C_API_START;
    return (((CShape*)shape)->getRespondable());
}

int _simGetDynamicCollisionMask_internal(const void* shape)
{
    C_API_START;
    return (((CShape*)shape)->getRespondableMask());
}

const void* _simGetLastParentForLocalGlobalCollidable_internal(const void* shape)
{
    C_API_START;
    return (((CShape*)shape)->getLastParentForLocalGlobalRespondable());
}

bool _simGetDynamicsFullRefreshFlag_internal(const void* object)
{
    C_API_START;
    return (((CSceneObject*)object)->getDynamicsResetFlag());
}

void _simSetDynamicsFullRefreshFlag_internal(const void* object, bool flag)
{
    C_API_START;
    ((CSceneObject*)object)->setDynamicsResetFlag(flag != 0, false);
}

const void* _simGetParentObject_internal(const void* object)
{
    C_API_START;
    return (((CSceneObject*)object)->getParent());
}

void _simDynReportObjectCumulativeTransformation_internal(void* obj, const double* pos, const double* quat, double simTime)
{ // obj is always a shape. Used by the physics engines. The joints and force sensors's internal errors are updated
    // accordingly
    C_API_START;
    if ( (!isNanOrInf(pos, 3)) && (!isNanOrInf(quat, 4)) )
    {
        CSceneObject* object = (CSceneObject*)obj;
        CSceneObject* parent = object->getParent();
        C7Vector tr;
        tr.X.setData(pos);
        tr.Q.setData(quat);
        if (parent != nullptr)
        {
            if (parent->getObjectType() == sim_sceneobject_joint)
            {
                CJoint* joint = (CJoint*)parent;
                C7Vector x(joint->getIntrinsicTransformation(false).getInverse() *
                           joint->getCumulativeTransformation().getInverse() * tr *
                           object->getLocalTransformation().getInverse());
                joint->setIntrinsicTransformationError(x);
            }
            else if (parent->getObjectType() == sim_sceneobject_forcesensor)
            {
                CForceSensor* sensor = (CForceSensor*)parent;
                C7Vector x(sensor->getCumulativeTransformation().getInverse() * tr *
                           object->getLocalTransformation().getInverse());
                sensor->setIntrinsicTransformationError(x);
            }
            else
                App::currentWorld->sceneObjects->setObjectAbsolutePose(object->getObjectHandle(), tr, false);
        }
        else
            object->setLocalTransformation(tr);
    }
}

void _simSetObjectCumulativeTransformation_internal(void* object, const double* pos, const double* quat,
                                                    bool keepChildrenInPlace)
{
    C_API_START;
    C7Vector tr;
    tr.X.setData(pos);
    tr.Q.setData(quat);
    App::currentWorld->sceneObjects->setObjectAbsolutePose(((CSceneObject*)object)->getObjectHandle(), tr,
                                                           keepChildrenInPlace != 0);
}

void _simSetShapeDynamicVelocity_internal(void* shape, const double* linear, const double* angular, double simTime)
{
    C_API_START;
    if ( (!isNanOrInf(linear, 3)) && (!isNanOrInf(angular, 3)) )
        ((CShape*)shape)->setDynamicVelocity(linear, angular);
}

void _simGetAdditionalForceAndTorque_internal(const void* shape, double* force, double* torque)
{
    C_API_START;
    ((CShape*)shape)->getAdditionalForce().getData(force);
    ((CShape*)shape)->getAdditionalTorque().getData(torque);
}

void _simClearAdditionalForceAndTorque_internal(const void* shape)
{
    C_API_START;
    ((CShape*)shape)->clearAdditionalForceAndTorque();
}

bool _simGetJointPositionInterval_internal(const void* joint, double* minValue, double* rangeValue)
{
    C_API_START;
    double minV, maxV;
    ((CJoint*)joint)->getInterval(minV, maxV);
    if (minValue != nullptr)
        minValue[0] = minV;
    if (rangeValue != nullptr)
        rangeValue[0] = maxV - minV;
    return (!((CJoint*)joint)->getIsCyclic());
}

const void* _simGetObject_internal(int objID)
{
    C_API_START;
    return (App::currentWorld->sceneObjects->getObjectFromHandle(objID));
}

const void* _simGetIkGroupObject_internal(int ikGroupID)
{
    C_API_START;
    return (App::currentWorld->ikGroups_old->getObjectFromHandle(ikGroupID));
}

int _simMpHandleIkGroupObject_internal(const void* ikGroup)
{
    C_API_START;
    return (((CIkGroup_old*)ikGroup)->computeGroupIk(true));
}

int _simGetJointType_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getJointType());
}

double _simGetDynamicMotorTargetPosition_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getTargetPosition());
}

double _simGetDynamicMotorTargetVelocity_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getTargetVelocity());
}

double _simGetDynamicMotorMaxForce_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getTargetForce(false));
}

double _simGetDynamicMotorUpperLimitVelocity_internal(const void* joint)
{
    C_API_START;
    double maxVelAccelJerk[3];
    ((CJoint*)joint)->getMaxVelAccelJerk(maxVelAccelJerk);
    return (maxVelAccelJerk[0]);
}

void _simSetJointSphericalTransformation_internal(void* joint, const double* quat, double simTime)
{
    C_API_START;
    if (!isNanOrInf(quat, 4))
        ((CJoint*)joint)->setSphericalTransformation(quat);
}

void _simAddForceSensorCumulativeForcesAndTorques_internal(void* forceSensor, const double* force, const double* torque, int totalPassesCount, double simTime)
{
    C_API_START;
    if ((!isNanOrInf(force, 3)) && (!isNanOrInf(torque, 3)))
        ((CForceSensor*)forceSensor)->addCumulativeForcesAndTorques(force, torque, totalPassesCount);
}

void _simAddJointCumulativeForcesOrTorques_internal(void* joint, double forceOrTorque, int totalPassesCount,
                                                    double simTime)
{
    C_API_START;
    if (!isNanOrInf(forceOrTorque))
        ((CJoint*)joint)->addCumulativeForceOrTorque(forceOrTorque, totalPassesCount);
}

int _simGetObjectListSize_internal(int objType)
{
    C_API_START;
    if (objType == sim_sceneobject_shape)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape)));
    if (objType == sim_sceneobject_joint)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_joint)));
    if (objType == sim_handle_all)
        return (int(App::currentWorld->sceneObjects->getObjectCount())); // we put it also here for faster access!
    if (objType == sim_sceneobject_dummy)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_dummy)));
    if (objType == sim_sceneobject_octree)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_octree)));
    if (objType == sim_sceneobject_pointcloud)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_pointcloud)));
    if (objType == sim_sceneobject_graph)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_graph)));
    if (objType == sim_sceneobject_camera)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_camera)));
    if (objType == sim_sceneobject_proximitysensor)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_proximitysensor)));
    if (objType == sim_sceneobject_path)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_path)));
    if (objType == sim_sceneobject_visionsensor)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_visionsensor)));
    if (objType == sim_sceneobject_mill)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_mill)));
    if (objType == sim_sceneobject_forcesensor)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_forcesensor)));
    if (objType == sim_sceneobject_light)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_light)));
    if (objType == sim_sceneobject_mirror)
        return (int(App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_mirror)));
    if (objType == -1)
        return (int(App::currentWorld->sceneObjects->getOrphanCount()));
    return (int(App::currentWorld->sceneObjects->getObjectCount()));
}

const void* _simGetObjectFromIndex_internal(int objType, int index)
{
    C_API_START;
    if (objType == sim_sceneobject_shape)
        return (App::currentWorld->sceneObjects->getShapeFromIndex(index));
    if (objType == sim_sceneobject_joint)
        return (App::currentWorld->sceneObjects->getJointFromIndex(index));
    if (objType == sim_handle_all)
        return (App::currentWorld->sceneObjects->getObjectFromIndex(index)); // we put it also here for faster access!
    if (objType == sim_sceneobject_dummy)
        return (App::currentWorld->sceneObjects->getDummyFromIndex(index));
    if (objType == sim_sceneobject_octree)
        return (App::currentWorld->sceneObjects->getOctreeFromIndex(index));
    if (objType == sim_sceneobject_pointcloud)
        return (App::currentWorld->sceneObjects->getPointCloudFromIndex(index));
    if (objType == sim_sceneobject_graph)
        return (App::currentWorld->sceneObjects->getGraphFromIndex(index));
    if (objType == sim_sceneobject_camera)
        return (App::currentWorld->sceneObjects->getCameraFromIndex(index));
    if (objType == sim_sceneobject_proximitysensor)
        return (App::currentWorld->sceneObjects->getProximitySensorFromIndex(index));
    if (objType == sim_sceneobject_path)
        return (App::currentWorld->sceneObjects->getPathFromIndex(index));
    if (objType == sim_sceneobject_visionsensor)
        return (App::currentWorld->sceneObjects->getVisionSensorFromIndex(index));
    if (objType == sim_sceneobject_mill)
        return (App::currentWorld->sceneObjects->getMillFromIndex(index));
    if (objType == sim_sceneobject_forcesensor)
        return (App::currentWorld->sceneObjects->getForceSensorFromIndex(index));
    if (objType == sim_sceneobject_light)
        return (App::currentWorld->sceneObjects->getLightFromIndex(index));
    if (objType == sim_sceneobject_mirror)
        return (App::currentWorld->sceneObjects->getMirrorFromIndex(index));
    if (objType == -1)
        return (App::currentWorld->sceneObjects->getOrphanFromIndex(index));
    return (App::currentWorld->sceneObjects->getObjectFromIndex(index));
}

void _simSetDynamicSimulationIconCode_internal(void* object, int code)
{
    C_API_START;
    ((CSceneObject*)object)->setDynamicSimulationIconCode(code);
}

void _simSetDynamicObjectFlagForVisualization_internal(void* object, int flag)
{
    C_API_START;
    ((CSceneObject*)object)->setDynamicFlag(flag);
}

int _simGetTreeDynamicProperty_internal(const void* object)
{
    C_API_START;
    return (((CSceneObject*)object)->getTreeDynamicProperty());
}

int _simGetObjectType_internal(const void* object)
{
    C_API_START;
    return (((CSceneObject*)object)->getObjectType());
}

const void** _simGetObjectChildren_internal(const void* object, int* count)
{
    C_API_START;
    CSceneObject* it = (CSceneObject*)object;
    count[0] = int(it->getChildCount());
    if (count[0] != 0)
        return ((const void**)&it->getChildren()->at(0));
    return (nullptr);
}

int _simGetDummyLinkType_internal(const void* dummy, int* linkedDummyID)
{
    C_API_START;
    int dType = ((CDummy*)dummy)->getDummyType();
    if (linkedDummyID != nullptr)
        linkedDummyID[0] = ((CDummy*)dummy)->getLinkedDummyHandle();
    return (dType);
}

int _simGetJointMode_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getJointMode());
}

bool _simIsJointInHybridOperation_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getHybridFunctionality_old());
}

void _simDisableDynamicTreeForManipulation_internal(const void* object, bool disableFlag)
{
    C_API_START;
    ((CSceneObject*)object)->temporarilyDisableDynamicTree();
}

void _simSetJointVelocity_internal(const void* joint, double vel)
{ // only used by MuJoCo. Other engines have the joint velocity computed via
    // _simSetDynamicMotorReflectedPositionFromDynamicEngine
    C_API_START;
    if (!isNanOrInf(vel))
        ((CJoint*)joint)->setVelocity(vel);
}

void _simSetJointPosition_internal(const void* joint, double pos)
{ // only used by MuJoCo. Other engines have the joint position set via
    // _simSetDynamicMotorReflectedPositionFromDynamicEngine
    C_API_START;
    if (!isNanOrInf(pos))
    {
        CJoint* j = (CJoint*)joint;
        j->setPosition(pos, nullptr, !j->getEnforceLimits()); // here we should set what the engine tells us, and not enforce the joint limits!! (otherwise we have discrepancies between dyn. state and visuals)
    }
}

void _simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(void* joint, double pos, double simTime)
{ // only from non-MuJoCo engines. MuJoCo uses above 2 functions instead
    C_API_START;
    ((CJoint*)joint)->setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(pos, simTime); // here we should set what the engine tells us, and not enforce the joint limits!! (otherwise we have discrepancies between dyn. state and visuals)
}

double _simGetJointPosition_internal(const void* joint)
{
    C_API_START;
    return (((CJoint*)joint)->getPosition());
}

void _simSetDynamicMotorPositionControlTargetPosition_internal(const void* joint, double pos)
{ // OLD, for backward compatibility. Only joints in hybrid operation are called here
    if (_simGetJointMode_internal(joint) != sim_jointmode_dynamic)
        ((CJoint*)joint)->setTargetPosition(pos);
}

void _simGetGravity_internal(double* gravity)
{
    C_API_START;
    App::currentWorld->dynamicsContainer->getGravity().getData(gravity);
}

int _simGetTimeDiffInMs_internal(int previousTime)
{
    C_API_START;
    return (VDateTime::getTimeDiffInMs(previousTime));
}

bool _simDoEntitiesCollide_internal(int entity1ID, int entity2ID, int* cacheBuffer, bool overrideCollidableFlagIfShape1,
                                    bool overrideCollidableFlagIfShape2, bool pathOrMotionPlanningRoutineCalling)
{
    C_API_START;
    return (CCollisionRoutine::doEntitiesCollide(entity1ID, entity2ID, nullptr, overrideCollidableFlagIfShape1 != 0,
                                                 overrideCollidableFlagIfShape2 != 0, nullptr));
}

bool _simGetDistanceBetweenEntitiesIfSmaller_internal(int entity1ID, int entity2ID, double* distance, double* ray,
                                                      int* cacheBuffer, bool overrideMeasurableFlagIfNonCollection1,
                                                      bool overrideMeasurableFlagIfNonCollection2,
                                                      bool pathPlanningRoutineCalling)
{
    C_API_START;
    return (CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(
        entity1ID, entity2ID, distance[0], ray, cacheBuffer, cacheBuffer + 2,
        overrideMeasurableFlagIfNonCollection1 != 0, overrideMeasurableFlagIfNonCollection2 != 0));
}

int _simHandleJointControl_internal(const void* joint, int auxV, const int* inputValuesInt, const double* inputValuesFloat, double* outputValues)
{
    C_API_START;
    if (!isNanOrInf(inputValuesFloat, 6))
    {
        double currentPosVelAccel[3] = {inputValuesFloat[0], inputValuesFloat[4], inputValuesFloat[5]};
        return (((CJoint*)joint)->handleDynJoint(auxV, inputValuesInt, currentPosVelAccel, inputValuesFloat[1], inputValuesFloat[2], inputValuesFloat[3], outputValues));
    }
    return 0;
}

int _simGetJointDynCtrlMode_internal(const void* joint)
{
    C_API_START;
    int retVal = ((CJoint*)joint)->getDynCtrlMode();
    return (retVal);
}

int _simHandleCustomContact_internal(int objHandle1, int objHandle2, int engine, int* dataInt, double* dataFloat)
{ // Careful with this function: it can also be called from any other thread (e.g. generated by the physics engine)
    C_API_START;

    // 1. We handle the new calling method:
    if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_contact) > 0)
    {
        if ((engine & 1024) == 0) // the engine flag 1024 means: the calling thread is not the simulation thread. We
                                  // would have problems with the scripts
        {
            CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->insertKeyInt32IntoStackTable("handle1", objHandle1);
            inStack->insertKeyInt32IntoStackTable("handle2", objHandle2);
            inStack->insertKeyInt32IntoStackTable("engine", engine);
            CInterfaceStack* outStack = App::worldContainer->interfaceStackContainer->createStack();
            App::worldContainer->callScripts(sim_syscb_contact, inStack, outStack);
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);

            bool ignoreContact;
            if (outStack->getStackMapBoolValue("ignoreContact", ignoreContact))
            {
                dataInt[0] = 0;
                if (!ignoreContact)
                {
                    bool collisionResponse = false;
                    outStack->getStackMapBoolValue("collisionResponse", collisionResponse);
                    if (collisionResponse)
                    {
                        if (engine == sim_physics_ode)
                        {
                            outStack->getStackMapInt32Value("ode.maxContacts", dataInt[1]);
                            outStack->getStackMapInt32Value("ode.contactMode", dataInt[2]);
                        }
                        if (engine == sim_physics_bullet)
                        {
                            outStack->getStackMapDoubleValue("bullet.friction", dataFloat[0]);
                            outStack->getStackMapDoubleValue("bullet.restitution", dataFloat[1]);
                        }
                        if (engine == sim_physics_ode)
                        {
                            outStack->getStackMapDoubleValue("ode.mu", dataFloat[0]);
                            outStack->getStackMapDoubleValue("ode.mu2", dataFloat[1]);
                            outStack->getStackMapDoubleValue("ode.bounce", dataFloat[2]);
                            outStack->getStackMapDoubleValue("ode.bounceVel", dataFloat[3]);
                            outStack->getStackMapDoubleValue("ode.softCfm", dataFloat[4]);
                            outStack->getStackMapDoubleValue("ode.softErp", dataFloat[5]);
                            outStack->getStackMapDoubleValue("ode.motion1", dataFloat[6]);
                            outStack->getStackMapDoubleValue("ode.motion2", dataFloat[7]);
                            outStack->getStackMapDoubleValue("ode.motionN", dataFloat[8]);
                            outStack->getStackMapDoubleValue("ode.slip1", dataFloat[9]);
                            outStack->getStackMapDoubleValue("ode.slip2", dataFloat[10]);
                            outStack->getStackMapDoubleArray("ode.fDir1", dataFloat + 11, 3);
                        }
                        if (engine == sim_physics_vortex)
                        {
                            // outStack->getStackMapDoubleValue("vortex.xxxx",dataFloat[0]);
                        }
                        if (engine == sim_physics_newton)
                        {
                            outStack->getStackMapDoubleValue("newton.staticFriction", dataFloat[0]);
                            outStack->getStackMapDoubleValue("newton.kineticFriction", dataFloat[1]);
                            outStack->getStackMapDoubleValue("newton.restitution", dataFloat[2]);
                        }
                        if (engine == sim_physics_mujoco)
                        {
                        }
                        App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                        return (1); // collision
                    }
                    else
                    {
                        App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                        return (0); // no collision
                    }
                }
            }
            App::worldContainer->interfaceStackContainer->destroyStack(outStack);
        }
    }
    return (-1); // we let CoppeliaSim handle the contact
}

double _simGetPureHollowScaling_internal(const void* geometric)
{
    C_API_START;
    return (((CMesh*)geometric)->getPurePrimitiveInsideScaling_OLD());
}

void _simDynCallback_internal(const int* intData, const double* floatData)
{
    C_API_START;

    if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_dyn) > 0)
    { // to make it a bit faster than blindly parsing the whole object hierarchy
        CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
        inStack->pushTableOntoStack();

        inStack->insertKeyInt32IntoStackTable("passCnt", intData[1]);
        inStack->insertKeyInt32IntoStackTable("totalPasses", intData[2]);
        inStack->insertKeyFloatIntoStackTable("dynStepSize", floatData[0]); // deprecated
        inStack->insertKeyFloatIntoStackTable("dt", floatData[0]);
        inStack->insertKeyBoolIntoStackTable("afterStep", intData[3] != 0);
        App::worldContainer->callScripts(sim_syscb_dyn, inStack, nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(inStack);
    }
}

int simGetVisionSensorRes_internal(int sensorHandle, int* resolution)
{
    C_API_START;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__, sensorHandle))
            return (-1);
        if (!isVisionSensor(__func__, sensorHandle))
            return (-1);
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        it->getResolution(resolution);
        return (1);
    }
    CApiErrors::setLastWarningOrError(__func__, SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return (-1);
}

#include <simInternal-old.cpp>
