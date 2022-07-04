#include "easyLock.h"
#include "simInternal.h"
#include "simulation.h"
#include "collisionRoutines.h"
#include "distanceRoutines.h"
#include "proxSensorRoutine.h"
#include "meshRoutines.h"
#include "tt.h"
#include "fileOperations.h"
#include "persistentDataContainer.h"
#include "graphingRoutines_old.h"
#include "sceneObjectOperations.h"
#include "threadPool_old.h"
#include "addOperations.h"
#include "app.h"
#include "pluginContainer.h"
#include "mesh.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "vVarious.h"
#include "volInt.h"
#include "imgLoaderSaver.h"
#include "apiErrors.h"
#include "sigHandler.h"
#include "memorizedConf_old.h"
#include <algorithm>
#include <iostream>
#include "tinyxml2.h"
#include "simFlavor.h"
#include <regex>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "gm.h"
#ifdef SIM_WITH_GUI
    #include <QSplashScreen>
#endif
#ifdef WIN_SIM
    #include <windows.h>
    #include <dbghelp.h>
#else
    #include <execinfo.h>
    #include <signal.h>
#endif

#ifndef SIM_WITH_QT
    #ifdef WIN_SIM
        #include "_dirent.h"
    #else
        #include <dirent.h>
    #endif
#else
    VMutex _lockForExtLockList;
    std::vector<CSimAndUiThreadSync*> _extLockList;
#endif

int _currentScriptNameIndex=-1;
int _currentScriptHandle=-1;

bool outputSceneOrModelLoadMessagesWithApiCall=false;
bool fullModelCopyFromApi=true;
bool waitingForTrigger=false;
bool doNotRunMainScriptFromRosInterface=false;

std::vector<int> pluginHandles;
std::string initialSceneOrModelToLoad;
std::string applicationDir;
bool stepSimIfRunning=true;
bool firstSimulationAutoStart=false;
int firstSimulationStopDelay=0;
bool firstSimulationAutoQuit=false;

void simulatorInit()
{
    App::logMsg(sim_verbosity_loadinfos,"simulator launched.");
    std::vector<std::string> theNames;
    std::vector<std::string> theDirAndNames;
#ifndef SIM_WITH_QT
    char curDirAndFile[2048];
    #ifdef WIN_SIM
        GetModuleFileNameA(NULL,curDirAndFile,2000);
        int i=0;
        while (true)
        {
            if (curDirAndFile[i]==0)
                break;
            if (curDirAndFile[i]=='\\')
                curDirAndFile[i]='/';
            i++;
        }
        std::string theDir(curDirAndFile);
        while ( (theDir.size()>0)&&(theDir[theDir.size()-1]!='/') )
            theDir.erase(theDir.end()-1);
        if (theDir.size()>0)
            theDir.erase(theDir.end()-1);
    #else
        getcwd(curDirAndFile,2000);
        std::string theDir(curDirAndFile);
    #endif

    DIR* dir;
    struct dirent* ent;
    if ( (dir=opendir(theDir.c_str()))!=NULL )
    {
        while ( (ent=readdir(dir))!=NULL )
        {
            if ( (ent->d_type==DT_LNK)||(ent->d_type==DT_REG) )
            {
                std::string nm(ent->d_name);
                std::transform(nm.begin(),nm.end(),nm.begin(),::tolower);
                int pre=0;
                int po=0;
                #ifdef WIN_SIM
                if ( boost::algorithm::starts_with(nm,"v_repext")&&boost::algorithm::ends_with(nm,".dll") )
                    pre=8;po=4;
                if ( boost::algorithm::starts_with(nm,"simext")&&boost::algorithm::ends_with(nm,".dll") )
                    pre=6;po=4;
                #endif
                #ifdef LIN_SIM
                if ( boost::algorithm::starts_with(nm,"libv_repext")&&boost::algorithm::ends_with(nm,".so") )
                    pre=11;po=3;
                if ( boost::algorithm::starts_with(nm,"libsimext")&&boost::algorithm::ends_with(nm,".so") )
                    pre=9;po=3;
                #endif
                #ifdef MAC_SIM
                if ( boost::algorithm::starts_with(nm,"libv_repext")&&boost::algorithm::ends_with(nm,".dylib") )
                    pre=11;po=6;
                if ( boost::algorithm::starts_with(nm,"libsimext")&&boost::algorithm::ends_with(nm,".dylib") )
                    pre=9;po=6;
                #endif
                if (pre!=0)
                {
                    if (nm.find('_',6)==std::string::npos)
                    {
                        nm=ent->d_name;
                        nm.assign(nm.begin()+pre,nm.end()-po);
                        theNames.push_back(nm);
                        theDirAndNames.push_back(theDir+'/'+ent->d_name);
                    }
                }
            }
        }
        closedir(dir);
    }
#else

    {
        QDir dir(applicationDir.c_str());
        dir.setFilter(QDir::Files|QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl=8;
        #ifdef WIN_SIM
            std::string tmp("v_repExt*.dll");
        #endif
        #ifdef MAC_SIM
            std::string tmp("libv_repExt*.dylib");
            bnl=11;
        #endif
        #ifdef LIN_SIM
            std::string tmp("libv_repExt*.so");
            bnl=11;
        #endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list=dir.entryInfoList();
        for (int i=0;i<list.size();++i)
        {
            QFileInfo fileInfo=list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin()+bnl,bla.end());
            if (tmp.find('_')==std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

    {
        QDir dir(applicationDir.c_str());
        dir.setFilter(QDir::Files|QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl=6;
        #ifdef WIN_SIM
            std::string tmp("simExt*.dll");
        #endif
        #ifdef MAC_SIM
            std::string tmp("libsimExt*.dylib");
            bnl=9;
        #endif
        #ifdef LIN_SIM
            std::string tmp("libsimExt*.so");
            bnl=9;
        #endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list=dir.entryInfoList();
        for (int i=0;i<list.size();++i)
        {
            QFileInfo fileInfo=list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin()+bnl,bla.end());
            if (tmp.find('_')==std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

#endif

     // Load the system plugins first:
    for (size_t i=0;i<theNames.size();i++)
    {
        if ( (theNames[i].compare("Geometric")==0)||boost::algorithm::starts_with(theNames[i],"Dynamics") )
        {
            int pluginHandle=simLoadModule_internal(theDirAndNames[i].c_str(),theNames[i].c_str());
            if (pluginHandle>=0)
                pluginHandles.push_back(pluginHandle);
            theDirAndNames[i]=""; // mark as 'already loaded'
        }
    }

     // Now load the other plugins too:
    for (size_t i=0;i<theNames.size();i++)
    {
        if (theDirAndNames[i].compare("")!=0)
        { // not yet loaded
            int pluginHandle=simLoadModule_internal(theDirAndNames[i].c_str(),theNames[i].c_str());
            if (pluginHandle>=0)
                pluginHandles.push_back(pluginHandle);
        }
    }

    if (initialSceneOrModelToLoad.length()!=0)
    { // Here we double-clicked a CoppeliaSim file or dragged-and-dropped it onto this application
        if ( boost::algorithm::ends_with(initialSceneOrModelToLoad.c_str(),".ttt")||boost::algorithm::ends_with(initialSceneOrModelToLoad.c_str(),".simscene.xml") )
        {
            if (simLoadScene_internal(initialSceneOrModelToLoad.c_str())==-1)
                App::logMsg(sim_verbosity_errors,"scene could not be opened.");
        }
        if ( boost::algorithm::ends_with(initialSceneOrModelToLoad.c_str(),".ttm")||boost::algorithm::ends_with(initialSceneOrModelToLoad.c_str(),".simmodel.xml"))
        {
            if (simLoadModel_internal(initialSceneOrModelToLoad.c_str())==-1)
                App::logMsg(sim_verbosity_errors,"model could not be opened.");
        }
    }
}

void simulatorDeinit()
{
    // Unload all plugins:
    while (pluginHandles.size()>0)
    {
        simUnloadModule_internal(pluginHandles[pluginHandles.size()-1]);
        pluginHandles.pop_back();
    }
    App::logMsg(sim_verbosity_loadinfos,"simulator ended.");
}

void simulatorLoop()
{   // The main simulation loop
    static bool wasRunning=false;
    int auxValues[4];
    int messageID=0;
    int dataSize;
    if (firstSimulationAutoStart)
    {
        simStartSimulation_internal();
        firstSimulationAutoStart=false;
    }
    while (messageID!=-1)
    {
        simChar* data=simGetSimulatorMessage_internal(&messageID,auxValues,&dataSize);
        if (messageID!=-1)
        {
            if (messageID==sim_message_simulation_start_resume_request)
                simStartSimulation_internal();
            if (messageID==sim_message_simulation_pause_request)
                simPauseSimulation_internal();
            if (messageID==sim_message_simulation_stop_request)
                simStopSimulation_internal();
            if (data!=NULL)
                simReleaseBuffer_internal(data);
        }
    }

     // Handle a running simulation:
    if ( stepSimIfRunning && (simGetSimulationState_internal()&sim_simulation_advancing)!=0 )
    {
        wasRunning=true;
        if ( (simGetRealTimeSimulation_internal()!=1)||(simIsRealTimeSimulationStepNeeded_internal()==1) )
        {
            if ((simHandleMainScript_internal()&sim_script_main_script_not_called)==0)
                simAdvanceSimulationByOneStep_internal();
            if ((firstSimulationStopDelay>0)&&(simGetSimulationTime_internal()>=float(firstSimulationStopDelay)/1000.0f))
            {
                firstSimulationStopDelay=0;
                simStopSimulation_internal();
            }
        }
        else
            App::worldContainer->callScripts(sim_syscb_realtimeidle,nullptr);
    }
    if ( (simGetSimulationState_internal()==sim_simulation_stopped)&&wasRunning&&firstSimulationAutoQuit )
    {
        wasRunning=false;
        simQuitSimulator_internal(true); // will post the quit command
    }
}

//********************************************
// Following courtesy of Stephen James:
simInt simExtLaunchUIThread_internal(const simChar* applicationName,simInt options,const simChar* sceneOrModelToLoad_,const simChar* applicationDir_)
{
    std::string applicationDir__(applicationDir_);
    applicationDir = applicationDir__;
    return(simRunSimulator_internal(applicationName,options,simulatorInit,simulatorLoop,simulatorDeinit,0,sceneOrModelToLoad_,false));
}

simInt simExtGetExitRequest_internal()
{
    return(App::getExitRequest());
}

simInt simExtStep_internal(simBool stepIfRunning)
{
    stepSimIfRunning = stepIfRunning;
    App::simulationThreadLoop();
    return(1);
}

simInt simExtCanInitSimThread_internal()
{
    if (App::canInitSimThread())
        return(1);
    return(0);
}

simInt simExtSimThreadInit_internal()
{
    App::simulationThreadInit();
    return(1);
}

simInt simExtSimThreadDestroy_internal()
{
    // If already called, then means we closed from the UI and dont need to post another request
    if(!App::getExitRequest())
        App::postExitRequest();
    App::simulationThreadDestroy();
    return(1);
}

simInt simExtPostExitRequest_internal()
{
    // If already called, then means we closed from the UI and dont need to post another request
    if(!App::getExitRequest())
        App::postExitRequest();
    return (1);
}

simInt simExtCallScriptFunction_internal(simInt scriptHandleOrType, const simChar* functionNameAtScriptName,
                                         const simInt* inIntData, simInt inIntCnt,
                                         const simFloat* inFloatData, simInt inFloatCnt,
                                         const simChar** inStringData, simInt inStringCnt,
                                         const simChar* inBufferData, simInt inBufferCnt,
                                         simInt** outIntData, simInt* outIntCnt,
                                         simFloat** outFloatData, simInt* outFloatCnt,
                                         simChar*** outStringData, simInt* outStringCnt,
                                         simChar** outBufferData, simInt* outBufferSize)
{
    int stack=simCreateStack_internal();
    simPushInt32TableOntoStack_internal(stack,inIntData,inIntCnt);
    simPushFloatTableOntoStack_internal(stack,inFloatData,inFloatCnt);
    simPushTableOntoStack_internal(stack);
    for (int i=0;i<inStringCnt;i++)
    {
        simPushInt32OntoStack_internal(stack,i+1);
        simPushStringOntoStack_internal(stack,inStringData[i],0);
        simInsertDataIntoStackTable_internal(stack);
    }
    simPushStringOntoStack_internal(stack,inBufferData,inBufferCnt);

    int ret = simCallScriptFunctionEx_internal(scriptHandleOrType,functionNameAtScriptName,stack);
    if (ret!=-1)
    { // success!
        // Get the return arguments. Make sure we have 4 or less:
        while (simGetStackSize_internal(stack)>4)
            simPopStackItem_internal(stack,1);
        // at pos 4 we are expecting a string (i.e. a buffer):
        outBufferSize[0]=-1;
        if (simGetStackSize_internal(stack)==4)
        {
            int bs;
            char* buffer=simGetStackStringValue_internal(stack,&bs);
            if ( (buffer!=nullptr)&&(bs>0) )
            {
                outBufferSize[0]=bs;
                outBufferData[0]=buffer;
            }
            simPopStackItem_internal(stack,1);
        }
        if (outBufferSize[0]==-1)
        {
            outBufferSize[0]=0;
            outBufferData[0]=new char[0];
        }
        // at pos 3 we are expecting a string table:
        outStringCnt[0]=-1;
        if (simGetStackSize_internal(stack)==3)
        {
            int tableSize=simGetStackTableInfo_internal(stack,0);
            if (tableSize>0)
            {
                int info=simGetStackTableInfo_internal(stack,4);
                if (info==1)
                {
                    outStringCnt[0]=tableSize;
                    outStringData[0]=new char*[tableSize];
                    simUnfoldStackTable_internal(stack);
                    for (int i=0;i<tableSize;i++)
                    {
                        int l;
                        char* str=simGetStackStringValue_internal(stack,&l);
                        outStringData[0][i]=str;
                        simPopStackItem_internal(stack,2);
                    }
                }
                else
                    simPopStackItem_internal(stack,1);
            }
            else
                simPopStackItem_internal(stack,1);
        }
        if (outStringCnt[0]==-1)
        {
            outStringCnt[0]=0;
            outStringData[0]=new char*[0];
        }
        // at pos 2 we are expecting a float table:
        outFloatCnt[0]=-1;
        if (simGetStackSize_internal(stack)==2)
        {
            int tableSize=simGetStackTableInfo_internal(stack,0);
            if (tableSize>0)
            {
                outFloatCnt[0]=tableSize;
                outFloatData[0]=new float[tableSize];
                simGetStackFloatTable_internal(stack,outFloatData[0],tableSize);
            }
            simPopStackItem_internal(stack,1);
        }
        if (outFloatCnt[0]==-1)
        {
            outFloatCnt[0]=0;
            outFloatData[0]=new float[0];
        }
        // at pos 1 we are expecting an int32 table:
        outIntCnt[0]=-1;
        if (simGetStackSize_internal(stack)==1)
        {
            int tableSize=simGetStackTableInfo_internal(stack,0);
            if (tableSize>0)
            {
                outIntCnt[0]=tableSize;
                outIntData[0]=new int[tableSize];
                simGetStackInt32Table_internal(stack,outIntData[0],tableSize);
            }
            simPopStackItem_internal(stack,1);
        }
        if (outIntCnt[0]==-1)
        {
            outIntCnt[0]=0;
            outIntData[0]=new int[0];
        }
    }
    simReleaseStack_internal(stack);
    return ret;
}
//********************************************

std::string getIndexAdjustedObjectName(const char* nm)
{
    std::string retVal;
    if (strlen(nm)!=0)
    {
        retVal=nm;
        if (retVal.find('#')==std::string::npos)
        { // e.g. "myObject42"
            if (_currentScriptNameIndex!=-1)
            { // for backward compatibility
                retVal+="#";
                retVal+=boost::lexical_cast<std::string>(_currentScriptNameIndex);
            }
        }
        if ( (retVal.length()!=0)&&(retVal[retVal.length()-1]=='#') ) // e.g. "myObject#"
            retVal.erase(retVal.end()-1);
    }
    return(retVal);
}

void setCurrentScriptInfo_cSide(int scriptHandle,int scriptNameIndex)
{
    _currentScriptHandle=scriptHandle;
    _currentScriptNameIndex=scriptNameIndex;
}

int getCurrentScriptNameIndex_cSide()
{
    return(_currentScriptNameIndex);
}

bool isSimulatorInitialized(const char* functionName)
{
    if (!App::isSimulatorRunning())
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_SIMULATOR_NOT_INITIALIZED);
        return(false);
    }
    return(true);
}

bool ifEditModeActiveGenerateErrorAndReturnTrue(const char* functionName)
{
    if (App::getEditModeType()!=NO_EDIT_MODE)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERRROR_EDIT_MODE_ACTIVE);
        return(true);
    }
    return(false);
}

bool canBoolIntOrFloatParameterBeSetOrGet(const char* functionName,int when)
{ // 'when' coding: bit0: sim not launched(1), bit1: sim launched(2), bit2: mainWindow not present(4),
    //bit3: mainWIndow present(8),bit4: sim not running(16), bit5: sim running(32)
    int st=0;
    if (App::isSimulatorRunning())
    {
        st|=2;
#ifdef SIM_WITH_GUI
        if (App::mainWindow==nullptr)
            st|=4;
        else
            st|=8;
#else
        st|=4;
#endif
        if (App::currentWorld->simulation->isSimulationStopped())
            st|=16;
        else
            st|=32;
    }
    else
        st|=1+4+16;
    if (((st&3)&when)==0)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_LAUNCH);
        return(false);
    }
    if (((st&12)&when)==0)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_WINDOW);
        return(false);
    }
    if (((st&48)&when)==0)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_SIM);
        return(false);
    }
    return(true);
}

bool doesObjectExist(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool doesEntityExist(const char* functionName,int identifier)
{
    if (identifier>=SIM_IDSTART_COLLECTION)
    {
        if (App::currentWorld->collections->getObjectFromHandle(identifier)==nullptr)
        {
            CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_ENTITY_INEXISTANT);
            return(false);
        }
        return(true);
    }
    else
    {
        if (App::currentWorld->sceneObjects->getObjectFromHandle(identifier)==nullptr)
        {
            CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_ENTITY_INEXISTANT);
            return(false);
        }
        return(true);
    }
}

bool doesCollectionExist(const char* functionName,int identifier)
{
    if (App::currentWorld->collections->getObjectFromHandle(identifier)==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_COLLECTION_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesCollisionObjectExist(const char* functionName,int identifier)
{
    if (App::currentWorld->collisions->getObjectFromHandle(identifier)==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_COLLISION_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool doesDistanceObjectExist(const char* functionName,int identifier)
{
    if (App::currentWorld->distances->getObjectFromHandle(identifier)==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_DISTANCE_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool isJoint(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_joint_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_JOINT);
        return(false);
    }
    return(true);
}
bool isShape(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_shape_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_SHAPE);
        return(false);
    }
    return(true);
}
bool isSensor(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_proximitysensor_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_PROX_SENSOR);
        return(false);
    }
    return(true);
}
bool isMill(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_mill_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_MILL);
        return(false);
    }
    return(true);
}
bool isForceSensor(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_forcesensor_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_FORCE_SENSOR);
        return(false);
    }
    return(true);
}
bool isVisionSensor(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_visionsensor_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_VISION_SENSOR);
        return(false);
    }
    return(true);
}
bool isCamera(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_camera_type)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_CAMERA);
        return(false);
    }
    return(true);
}
bool isGraph(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_graph_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_GRAPH);
        return(false);
    }
    return(true);
}
bool isPath(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_path_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_PATH);
        return(false);
    }
    return(true);
}
bool isLight(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_light_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_LIGHT);
        return(false);
    }
    return(true);
}
bool isDummy(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_dummy_type)
    { 
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_DUMMY);
        return(false);
    }
    return(true);
}
bool isOctree(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_octree_type)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_OCTREE);
        return(false);
    }
    return(true);
}
bool isPointCloud(const char* functionName,int identifier)
{
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_pointcloud_type)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_OBJECT_NOT_POINTCLOUD);
        return(false);
    }
    return(true);
}
bool doesUIExist(const char* functionName,int elementHandle)
{
    CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_UI_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesUIButtonExist(const char* functionName,int elementHandle,int buttonHandle)
{
    CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_UI_INEXISTANT);
        return(false);
    }
    else if (it->getButtonWithID(buttonHandle)==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_UI_BUTTON_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesIKGroupExist(const char* functionName,int identifier)
{
    CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setLastWarningOrError(functionName,SIM_ERROR_IK_GROUP_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

char* _extractCustomDataFromBuffer(std::vector<char>& buffer,const char* dataName,int* extractedBufSize)
{
    int off=0;
    while (off+8<int(buffer.size()))
    {
        int sizeIncr=((int*)((&buffer[0])+off))[0];
        int nameLength=((int*)((&buffer[0])+off))[1];
        std::string datName(((&buffer[0])+off)+4+4);
        if (datName.compare(dataName)==0)
        { // ok, we have the correct data here
            int datLength=sizeIncr-4-4-nameLength;
            char* returnBuffer=new char[datLength];
            extractedBufSize[0]=datLength;
            for (int i=0;i<datLength;i++)
                returnBuffer[i]=buffer[off+4+4+nameLength+i];
            buffer.erase(buffer.begin()+off,buffer.begin()+off+sizeIncr);
            return(returnBuffer);
        }
        // continue searching
        off+=sizeIncr;
    }
    extractedBufSize[0]=0;
    return(nullptr);
}

void _extractCustomDataTagsFromBuffer(std::vector<char>& buffer,std::vector<std::string>& tags)
{
    int off=0;
    while (off+8<int(buffer.size()))
    {
        int sizeIncr=((int*)((&buffer[0])+off))[0];
        //int nameLength=((int*)((&buffer[0])+off))[1];
        std::string datName(((&buffer[0])+off)+4+4);
        tags.push_back(datName);
        off+=sizeIncr;
    }
}

void _appendCustomDataToBuffer(std::vector<char>& buffer,const char* dataName,const char* data,int dataSize)
{
    if ((data!=nullptr)&&(dataSize>0))
    {
        int nameLength=int(strlen(dataName)+1); // with zero char
        int sizeIncr=4+4+nameLength+dataSize;
        buffer.push_back(((char*)&sizeIncr)[0]);
        buffer.push_back(((char*)&sizeIncr)[1]);
        buffer.push_back(((char*)&sizeIncr)[2]);
        buffer.push_back(((char*)&sizeIncr)[3]);
        buffer.push_back(((char*)&nameLength)[0]);
        buffer.push_back(((char*)&nameLength)[1]);
        buffer.push_back(((char*)&nameLength)[2]);
        buffer.push_back(((char*)&nameLength)[3]);
        for (int i=0;i<nameLength;i++)
            buffer.push_back(dataName[i]);
        for (int i=0;i<dataSize;i++)
            buffer.push_back(data[i]);
    }
}

#ifdef WIN_SIM
LONG WINAPI _winExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    void* stack[62];
    HANDLE process=GetCurrentProcess();
    SymInitialize(process,0,TRUE);
    unsigned short fr=CaptureStackBackTrace(0,62,stack,nullptr);
    SYMBOL_INFO* symb=(SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO)+1024*sizeof(char),1);
    symb->MaxNameLen=1023;
    symb->SizeOfStruct=sizeof(SYMBOL_INFO);
    for (size_t i=0;i<fr;i++)
    {
        SymFromAddr(process,(DWORD64)(stack[i]),0,symb);
        printf("CoppeliaSim: debug: %zu: %s - 0x%0I64X\n",fr-i-1,symb->Name,symb->Address);
    }
    free(symb);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
void _segHandler(int sig)
{
    void* arr[10];
    size_t s=backtrace(arr,10);
    fprintf(stderr,"\n\nError: signal %d:\n\n",sig);
    backtrace_symbols_fd(arr,s,STDERR_FILENO);
    exit(1);
}
#endif
simInt simRunSimulator_internal(const simChar* applicationName,simInt options,simVoid(*initCallBack)(),simVoid(*loopCallBack)(),simVoid(*deinitCallBack)(),simInt stopDelay,const simChar* sceneOrModelToLoad,bool launchSimThread)
{
    CGm gm;
    App::gm=&gm;
    firstSimulationStopDelay=stopDelay;
    initialSceneOrModelToLoad=sceneOrModelToLoad;
    if ( (options&sim_autostart)!=0 )
        firstSimulationAutoStart=true;
    if ( (options&sim_autoquit)!=0 )
        firstSimulationAutoQuit=true;
    if (initCallBack==nullptr)
        initCallBack=simulatorInit;
    if (loopCallBack==nullptr)
        loopCallBack=simulatorLoop;
    if (deinitCallBack==nullptr)
        deinitCallBack=simulatorDeinit;

#ifdef WIN_SIM
    SetUnhandledExceptionFilter(_winExceptionHandler);
#else
    signal(SIGSEGV,_segHandler);
#endif
    SignalHandler sigH(SignalHandler::SIG_INT | SignalHandler::SIG_TERM | SignalHandler::SIG_CLOSE);

    std::string flav("n/a");
#ifdef SIM_FL
    flav=std::to_string(SIM_FL).c_str();
#endif
    std::string str("CoppeliaSim v");
    str+=SIM_PROGRAM_VERSION;
    str+=", ";
    str+=SIM_PROGRAM_REVISION;
    str+=", flavor: ";
    str+=flav+", ";
    str+=SIM_PLATFORM;
    App::logMsg(sim_verbosity_loadinfos,str.c_str());
    CSimFlavor::run(0);

    TRACE_C_API;
    for (int i=0;i<9;i++)
    {
        std::string str(App::getApplicationArgument(i));
        if ( (str.compare(0,9,"GUIITEMS_")==0)&&(str.length()>9) )
        {
            str.erase(str.begin(),str.begin()+9);
            int val=0;
            if (tt::stringToInt(str.c_str(),val))
            {
                options=val;
                break;
            }
        }
    }

    App::operationalUIParts=options;
    if (App::operationalUIParts&sim_gui_headless)
        App::operationalUIParts=sim_gui_headless;
    App* applicationBasicInitialization=new App((App::operationalUIParts&sim_gui_headless)!=0);
    if (!applicationBasicInitialization->wasInitSuccessful())
    {
        delete applicationBasicInitialization;
        return(0);
    }
#ifdef SIM_WITH_QT
    if (launchSimThread)
    {
        QFileInfo pathInfo(QCoreApplication::applicationFilePath());
        applicationDir=pathInfo.path().toStdString();
    }
#endif

#ifdef SIM_WITH_GUI
    // Browser and hierarchy visibility is set in userset.txt. We can override it here:
    if ((App::operationalUIParts&sim_gui_hierarchy)==0)
        COglSurface::_hierarchyEnabled=false;
    if ((App::operationalUIParts&sim_gui_browser)==0)
    {
        //OLDMODELBROWSER COglSurface::_browserEnabled=false;
        App::setBrowserEnabled(false);
    }
    App::setIcon();
#endif

    App::setApplicationName(CSimFlavor::getStringVal(2).c_str());

#ifdef SIM_WITH_GUI
    if ((App::operationalUIParts&sim_gui_headless)==0)
        App::showSplashScreen();
#endif

    App::createWorldsContainer();
    CFileOperations::createNewScene(true,false);

#ifdef SIM_WITH_GUI
    if ((App::operationalUIParts&sim_gui_headless)==0)
    {
        App::createMainWindow();
        App::mainWindow->oglSurface->adjustBrowserAndHierarchySizesToDefault();
    }
#endif

    App::run(initCallBack,loopCallBack,deinitCallBack,launchSimThread); // We stay in here until we quit the application!
    App::logMsg(sim_verbosity_loadinfos,"4");
#ifdef SIM_WITH_GUI
    App::deleteMainWindow();
#endif
    App::logMsg(sim_verbosity_loadinfos,"3");
    App::deleteWorldsContainer();
    App::logMsg(sim_verbosity_loadinfos,"2");
    CSimFlavor::run(3);
    App::logMsg(sim_verbosity_loadinfos,"1");
    CThreadPool_old::cleanUp();
    delete applicationBasicInitialization;
    App::logMsg(sim_verbosity_loadinfos,"done.");
    return(1);
}

simVoid* simGetMainWindow_internal(simInt type)
{ // 0=window handle , otherwise Qt pointer 
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

//    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if (type==0)
            {
#ifdef WIN_SIM
                return((simVoid*)App::mainWindow->winId());
#endif
            }
            if (type==1)
            {
                return(App::mainWindow);
            }
        }
#endif
        return(nullptr);
    }
//    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simRefreshDialogs_internal(simInt refreshDegree)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (refreshDegree==0)
            App::setLightDialogRefreshFlag();
        if (refreshDegree==2)
            App::setFullDialogRefreshFlag();
        App::setDialogRefreshDontPublishFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetLastError_internal()
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string lastErr(CApiErrors::getAndClearLastWarningOrError());
        if (lastErr.size()==0)
            return(nullptr);
        char* retVal=new char[lastErr.length()+1];
        for (unsigned int i=0;i<lastErr.length();i++)
            retVal[i]=lastErr[i];
        retVal[lastErr.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetLastError_internal(const simChar* funcName,const simChar* errorMessage)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string func(funcName);
        if (func.compare(0,8,"warning@")==0)
            CApiErrors::setThreadBasedFirstCapiWarning_old(errorMessage);
        else
            CApiErrors::setThreadBasedFirstCapiError_old(errorMessage);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObject_internal(const simChar* objectAlias,int index,int proxy,int options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=nullptr;
        std::string nm(objectAlias);
        if ( (nm.size()>0)&&((nm[0]=='.')||(nm[0]==':')||(nm[0]=='/')) )
        {
            int objHandle=App::currentWorld->embeddedScriptContainer->getObjectHandleFromScriptHandle(_currentScriptHandle);
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
            CSceneObject* prox=App::currentWorld->sceneObjects->getObjectFromHandle(proxy);
            it=App::currentWorld->sceneObjects->getObjectFromPath(obj,nm.c_str(),index,prox);
        }

        if (it==nullptr)
        {
            if ((options&1)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt64 simGetObjectUid_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        return(it->getObjectUid());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectFromUid_internal(simInt64 uid,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromUid(uid);
        if (it!=nullptr)
            return(it->getObjectHandle());
        if ((options&1)==0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptHandleEx_internal(simInt scriptType,simInt objectHandle,const simChar* scriptName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=nullptr;
        if (scriptType==sim_scripttype_mainscript)
            it=App::currentWorld->embeddedScriptContainer->getMainScript();
        if (scriptType==sim_scripttype_sandboxscript)
            it=App::worldContainer->sandboxScript;
        if (scriptType==sim_scripttype_childscript)
        {
            if ( (objectHandle<0)&&(scriptName!=nullptr) )
                objectHandle=simGetObjectHandleEx_internal(scriptName,-1,-1,0);
            it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objectHandle);
        }
        if (scriptType==sim_scripttype_customizationscript)
        {
            if ( (objectHandle<0)&&(scriptName!=nullptr) )
                objectHandle=simGetObjectHandleEx_internal(scriptName,-1,-1,0);
            it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objectHandle);
        }
        if ( (scriptType==sim_scripttype_addonscript)&&(scriptName!=nullptr) )
            it=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName);
        if ( (it!=nullptr)&&(!it->getFlaggedForDestruction()) )
            return(it->getScriptHandle());
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRemoveObjects_internal(const simInt* objectHandles,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        // Memorize the selection:
        std::vector<int> initSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        // Erase the objects:
        std::vector<int> sel;
        for (int i=0;i<count;i++)
            sel.push_back(objectHandles[size_t(i)]);
        App::currentWorld->sceneObjects->eraseObjects(sel,true);

        // Restore the initial selection:
        App::currentWorld->sceneObjects->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveModel_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }
        if (!it->getModelBase())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_MODEL_BASE);
            return(-1);
        }

        // memorize current selection:
        std::vector<int> initSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        // Erase the objects:
        std::vector<int> sel;
        sel.push_back(objectHandle);
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

        App::currentWorld->sceneObjects->eraseObjects(sel,true);

        // Restore the initial selection:
        App::currentWorld->sceneObjects->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

        return((int)sel.size());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetObjectAlias_internal(simInt objectHandle,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string nm;
        if (options==-1)
            nm=it->getObjectAlias(); // just the alias, e.g. "alias"
        if (options==0)
            nm=it->getObjectAliasAndOrderIfRequired(); // the alias with order, e.g. "alias[0]"
        if (options==1)
            nm=it->getObjectAlias_shortPath(); // the alias with unique path, short, e.g. "/obj/alias[0]"
        if (options==2)
            nm=it->getObjectAlias_fullPath(); // the alias with full path, e.g. "/obj/obj2/alias[0]"
        if (options==3)
        { // just the alias, if unique, e.g. "alias", otherwise the alias with handle, e.g. "alias__42__"
            if (App::currentWorld->sceneObjects->getObjectFromPath(nullptr,(std::string("/")+it->getObjectAlias()).c_str(),1,nullptr)==nullptr)
                nm=it->getObjectAlias();
            else
                options=4;
        }
        if (options==4)
        { // the alias with object handle, e.g. "alias__42__"
            nm=it->getObjectAlias()+"__";
            nm+=std::to_string(it->getObjectHandle());
            nm+="__";
        }
        if (options==5)
            nm=it->getObjectAlias_printPath(); // the print version, not guaranteed to be unique, e.g. "/obj/.../alias[0]"
        char* retVal=new char[nm.length()+1];
        for (size_t i=0;i<nm.length();i++)
            retVal[i]=nm[i];
        retVal[nm.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectAlias_internal(simInt objectHandle,const simChar* objectAlias,int options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        App::currentWorld->sceneObjects->setObjectAlias(it,objectAlias,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectMatrix_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* matrix)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        bool inverse=false;
        if (relativeToObjectHandle==sim_handle_inverse)
        {
            inverse=true;
            relativeToObjectHandle=sim_handle_world;
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformation();
        else
        {
            C7Vector relTr;
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                relTr=relObj->getCumulativeTransformation();
            else
                relTr=relObj->getFullCumulativeTransformation();
            tr=relTr.getInverse()*it->getCumulativeTransformation();
        }
        if (inverse)
            tr.inverse();
        tr.getMatrix().copyToInterface(matrix);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectMatrix_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* matrix)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        bool inverse=false;
        if (relativeToObjectHandle==sim_handle_inverse)
        {
            inverse=true;
            relativeToObjectHandle=sim_handle_world;
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true,true);
        C4X4Matrix m;
        m.copyFromInterface(matrix);
        if (inverse)
            m.inverse();
        CSceneObject* objRel=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (objRel==nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(),m.getTransformation(),false);
        else
        {
            C7Vector relTr;
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                relTr=objRel->getCumulativeTransformation();
            else
                relTr=objRel->getFullCumulativeTransformation();
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(),relTr*m.getTransformation(),false);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectPose_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* pose)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        bool inverse=false;
        if (relativeToObjectHandle==sim_handle_inverse)
        {
            inverse=true;
            relativeToObjectHandle=sim_handle_world;
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformation();
        else
        {
            C7Vector relTr;
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                relTr=relObj->getCumulativeTransformation();
            else
                relTr=relObj->getFullCumulativeTransformation();
            tr=relTr.getInverse()*it->getCumulativeTransformation();
        }
        if (inverse)
            tr.inverse();
        tr.getInternalData(pose,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectPose_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* pose)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        bool inverse=false;
        if (relativeToObjectHandle==sim_handle_inverse)
        {
            inverse=true;
            relativeToObjectHandle=sim_handle_world;
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true,true);
        C7Vector tr;
        tr.setInternalData(pose,true);
        if (inverse)
            tr.inverse();
        CSceneObject* objRel=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (objRel==nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(),tr,false);
        else
        {
            C7Vector relTr;
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                relTr=objRel->getCumulativeTransformation();
            else
                relTr=objRel->getFullCumulativeTransformation();
            App::currentWorld->sceneObjects->setObjectAbsolutePose(it->getObjectHandle(),relTr*tr,false);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectPosition_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformation();
        else
        {
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr=relTr.getInverse()*it->getCumulativeTransformation();
            }
            else
            {
                if (it->getParent()==relObj)
                    tr=it->getLocalTransformation(); // in case of a series of get/set, not losing precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr=relTr.getInverse()*it->getCumulativeTransformation();
                }
            }
        }
        tr.X.copyTo(position);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectPosition_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true,true);
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj==nullptr)
            App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(),C3Vector(position));
        else
        {
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
            {
                C7Vector absTr(it->getCumulativeTransformation());
                C7Vector relTr(relObj->getCumulativeTransformation());
                C7Vector x(relTr.getInverse()*absTr);
                x.X.set(position);
                absTr=relTr*x;
                App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(),absTr.X);
            }
            else
            {
                if (it->getParent()==relObj)
                { // special here, in order to not lose precision in a series of get/set
                    C7Vector tr(it->getLocalTransformation());
                    tr.X=position;
                    it->setLocalTransformation(tr);
                }
                else
                {
                    C7Vector absTr(it->getCumulativeTransformation());
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    C7Vector x(relTr.getInverse()*absTr);
                    x.X.set(position);
                    absTr=relTr*x;
                    App::currentWorld->sceneObjects->setObjectAbsolutePosition(it->getObjectHandle(),absTr.X);
                }
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectOrientation_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* eulerAngles)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformation();
        else
        {
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr=relTr.getInverse()*it->getCumulativeTransformation();
            }
            else
            {
                C7Vector relTr(relObj->getFullCumulativeTransformation());
                tr=relTr.getInverse()*it->getCumulativeTransformation();
            }
        }
        C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectOrientation_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* eulerAngles)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true,true);
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj==nullptr)
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),C3Vector(eulerAngles));
        else
        {
            C7Vector absTr(it->getCumulativeTransformation());
            C7Vector relTr;
            if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                relTr=relObj->getCumulativeTransformation();
            else
                relTr=relObj->getFullCumulativeTransformation();
            C7Vector x(relTr.getInverse()*absTr);
            x.Q.setEulerAngles(eulerAngles);
            absTr=relTr*x;
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),absTr.Q.getEulerAngles());
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointPosition_internal(simInt objectHandle,simFloat* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        position[0]=it->getPosition();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointPosition_internal(simInt objectHandle,simFloat position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        // info: do not try to trigger a sysCall_jointCallback call for that function, it really doesn't make sense
        it->setPosition(position,false);
        it->setKinematicMotionType(0,true); // reset
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointTargetPosition_internal(simInt objectHandle,simFloat targetPosition)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        if (it->getJointMode()==sim_jointmode_dynamic)
        {
            it->setTargetPosition(targetPosition);
            it->setKinematicMotionType(0,true); // reset
        }
        else
        {
            if (it->getJointMode()==sim_jointmode_kinematic)
            {
                it->setTargetPosition(targetPosition);
                it->setKinematicMotionType(1,false); // pos
            }
            else
                it->setKinematicMotionType(0,true); // reset
            return(-1);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointTargetPosition_internal(simInt objectHandle,simFloat* targetPosition)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        targetPosition[0]=it->getTargetPosition();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointTargetVelocity_internal(simInt objectHandle,simFloat targetVelocity)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        if (it->getJointMode()==sim_jointmode_dynamic)
        {
            it->setTargetVelocity(targetVelocity);
            it->setKinematicMotionType(0,true); // reset
        }
        else
        {
            if (it->getJointMode()==sim_jointmode_kinematic)
            {
                it->setTargetVelocity(targetVelocity);
                it->setKinematicMotionType(2,false); // vel
            }
            else
                it->setKinematicMotionType(0,true); // reset
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointTargetVelocity_internal(simInt objectHandle,simFloat* targetVelocity)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        targetVelocity[0]=it->getTargetVelocity();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectChildPose_internal(simInt objectHandle,simFloat* pose)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        C7Vector tr;
        tr.setIdentity();
        if (obj->getObjectType()==sim_object_joint_type)
            tr=((CJoint*)obj)->getIntrinsicTransformation(true);
        if (obj->getObjectType()==sim_object_forcesensor_type)
            tr=((CForceSensor*)obj)->getIntrinsicTransformation(true);
        tr.getInternalData(pose,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectChildPose_internal(simInt objectHandle,const simFloat* pose)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* it=(CJoint*)obj;
            if (it->getJointType()==sim_joint_spherical_subtype)
            {
                C7Vector tr;
                tr.setInternalData(pose,true);
                it->setSphericalTransformation(C4Vector(tr.Q));
                it->setIntrinsicTransformationError(C7Vector::identityTransformation);
            }
        }
        if (obj->getObjectType()==sim_object_forcesensor_type)
        {
            CForceSensor* it=(CForceSensor*)obj;
            it->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointInterval_internal(simInt objectHandle,simBool* cyclic,simFloat* interval)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        cyclic[0]=0;
        if (it->getIsCyclic())
            cyclic[0]=1;
        interval[0]=it->getPositionMin();
        interval[1]=it->getPositionRange();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointInterval_internal(simInt objectHandle,simBool cyclic,const simFloat* interval)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
// Some models need to modify that
//        if ( App::currentWorld->simulation->isSimulationStopped()||((it->getJointMode()!=sim_jointmode_dynamic)&&(!it->getHybridFunctionality_old())) )
        {
            float previousPos=it->getPosition();
            it->setIsCyclic(cyclic!=0);
            it->setPositionMin(interval[0]);
            it->setPositionRange(interval[1]);
            it->setPosition(previousPos,false);
            return(1);
        }
//        return(-1);
//        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectParent_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=-1;
        if (it->getParent()!=nullptr)
            retVal=it->getParent()->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectChild_internal(simInt objectHandle,simInt index)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=-1;
        if (int(it->getChildCount())>index)
            retVal=it->getChildFromIndex(index)->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectParent_internal(simInt objectHandle,simInt parentObjectHandle,simBool keepInPlace)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0xff00000;
            objectHandle=objectHandle&0xfffff;
        }
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (parentObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,parentObjectHandle))
                return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CSceneObject* parentIt=App::currentWorld->sceneObjects->getObjectFromHandle(parentObjectHandle);
        CSceneObject* pp=parentIt;
        while (pp!=nullptr)
        {
            if (pp==it)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_IS_ANCESTOR_OF_DESIRED_PARENT);
                return(-1);
            }
            pp=pp->getParent();
        }
        if (keepInPlace)
            App::currentWorld->sceneObjects->setObjectParent(it,parentIt,true);
        else
        {
            if ( (handleFlags&sim_handleflag_assembly)&&(parentIt!=nullptr) )
            { // only assembling
                // There is another such similar routine!! XXBFVGA
                std::vector<CSceneObject*> potParents;
                parentIt->getAllChildrenThatMayBecomeAssemblyParent(it->getChildAssemblyMatchValuesPointer(),potParents);
                bool directAssembly=parentIt->doesParentAssemblingMatchValuesMatchWithChild(it->getChildAssemblyMatchValuesPointer());
                if ( directAssembly||(potParents.size()==1) )
                {
                    if (directAssembly)
                        App::currentWorld->sceneObjects->setObjectParent(it,parentIt,true);
                    else
                        App::currentWorld->sceneObjects->setObjectParent(it,potParents[0],true);
                    if (it->getAssemblingLocalTransformationIsUsed())
                        it->setLocalTransformation(it->getAssemblingLocalTransformation());
                }
                else
                    App::currentWorld->sceneObjects->setObjectParent(it,parentIt,false);
            }
            else
                App::currentWorld->sceneObjects->setObjectParent(it,parentIt,false);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectType_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=it->getObjectType();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointType_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        if (!isJoint(__func__,objectHandle))
        {
            return(-1);
        }
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        int retVal=it->getJointType();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simBuildIdentityMatrix_internal(simFloat* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.setIdentity();
    m.copyToInterface(matrix);
    return(1);
}

simInt simBuildMatrix_internal(const simFloat* position,const simFloat* eulerAngles,simFloat* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.M.setEulerAngles(C3Vector(eulerAngles));
    m.X.set(position);
    m.copyToInterface(matrix);
    return(1);
}

simInt simBuildPose_internal(const simFloat* position,const simFloat* eulerAngles,simFloat* pose)
{
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces: x y z w
    TRACE_C_API;

    C7Vector tr(C4Vector(eulerAngles[0],eulerAngles[1],eulerAngles[2]),C3Vector(position));
    tr.getInternalData(pose,true);
    return(1);
}

simInt simBuildMatrixQ_internal(const simFloat* position,const simFloat* quaternion,simFloat* matrix)
{
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces: x y z w
    TRACE_C_API;

    C4X4Matrix m;
    C4Vector q(quaternion[3],quaternion[0],quaternion[1],quaternion[2]);
    m.M=q.getMatrix();
    m.X.set(position);
    m.copyToInterface(matrix);
    return(1);
}

simInt simGetEulerAnglesFromMatrix_internal(const simFloat* matrix,simFloat* eulerAngles)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    m.M.getEulerAngles().copyTo(eulerAngles);
    return(1);
}

simInt simGetQuaternionFromMatrix_internal(const simFloat* matrix,simFloat* quaternion)
{
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces: x y z w
    TRACE_C_API;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    C4Vector q(m.M.getQuaternion());
    quaternion[0]=q(1);
    quaternion[1]=q(2);
    quaternion[2]=q(3);
    quaternion[3]=q(0);
    return(1);
}

simInt simInvertMatrix_internal(simFloat* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    m.inverse();
    m.copyToInterface(matrix);
    return(1);
}

simInt simMultiplyMatrices_internal(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn1;
    mIn1.copyFromInterface(matrixIn1);
    C4X4Matrix mIn2;
    mIn2.copyFromInterface(matrixIn2);
    (mIn1*mIn2).copyToInterface(matrixOut);
    return(1);
}

simInt simInterpolateMatrices_internal(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat interpolFactor,simFloat* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn1;
    mIn1.copyFromInterface(matrixIn1);
    C4X4Matrix mIn2;
    mIn2.copyFromInterface(matrixIn2);
    C7Vector tr;
    tr.buildInterpolation(mIn1.getTransformation(),mIn2.getTransformation(),interpolFactor);
    (tr.getMatrix()).copyToInterface(matrixOut);
    return(1);
}

simInt simTransformVector_internal(const simFloat* matrix,simFloat* vect)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    C3Vector v(vect);
    (m*v).copyTo(vect);
    return(1);
}

simInt simReservedCommand_internal(simInt v,simInt w)
{
    TRACE_C_API;

    if (v==0)
        VThread::sleep(w);
    if (v==2)
    {
        App::logMsg(sim_verbosity_errors,"functionality deprecated! (simReservedCommand, v=2)");
        int retVal=App::worldContainer->getModificationFlags(true);
        return(retVal);
    }
    return(CSimFlavor::getIntVal_2int(0,v,w));
}

simInt simSetBoolParam_internal(simInt parameter,simBool boolState)
{
    TRACE_C_API;
    bool couldNotLock=true;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        couldNotLock=false;
        if (parameter==sim_boolparam_exit_request)
        {
            if ( App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=EXIT_REQUEST_CMD;
                App::appendSimulationThreadCommand(cmd);
                return(1);
            }
            return(-1);
        }
        if (parameter==sim_boolparam_hierarchy_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                if (App::mainWindow->dlgCont->isVisible(HIERARCHY_DLG)!=(boolState!=0))
                    App::mainWindow->dlgCont->toggle(HIERARCHY_DLG);
            }
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_hierarchy_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setHierarchyToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_browser_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                if (App::mainWindow->dlgCont->isVisible(BROWSER_DLG)!=(boolState!=0))
                    App::mainWindow->dlgCont->toggle(BROWSER_DLG);
            }
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_browser_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setBrowserToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_play_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setPlayViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_pause_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setPauseViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_stop_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setStopViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_objproperties_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setObjPropToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_calcmodules_toolbarbutton_enabled)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setCalcModulesToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_rosinterface_donotrunmainscript)
        {
            doNotRunMainScriptFromRosInterface=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_waiting_for_trigger)
        {
            waitingForTrigger=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_objectshift_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setObjectShiftToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_objectrotate_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->setObjectRotateToggleViaGuiEnabled(boolState!=0);
            else
#endif
                return(-1);
            return(1);
        }
        if (parameter==sim_boolparam_shape_textures_are_visible)
        {
            if (App::currentWorld->environment==nullptr)
                return(-1);
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            App::currentWorld->environment->setShapeTexturesEnabled(boolState!=0);
            return(1);
        }
        if ( (parameter==sim_boolparam_show_w_emitters)||(parameter==sim_boolparam_show_w_receivers) )
        {
            if (App::currentWorld->environment==nullptr)
                return(-1);
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (parameter==sim_boolparam_show_w_emitters)
                App::currentWorld->environment->setVisualizeWirelessEmitters(boolState!=0);
            else
                App::currentWorld->environment->setVisualizeWirelessReceivers(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_display_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                if (VThread::isCurrentThreadTheUiThread())
                { // We are in the UI thread. We execute the command now:
                    App::mainWindow->setOpenGlDisplayEnabled(boolState!=0);
                }
                else
                { // We are not in the UI thread. Execute the command via the UI thread:
                    SUIThreadCommand cmdIn;
                    SUIThreadCommand cmdOut;
                    if (boolState!=0)
                        cmdIn.cmdId=VISUALIZATION_ON_UITHREADCMD;
                    else
                        cmdIn.cmdId=VISUALIZATION_OFF_UITHREADCMD;
                    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
                return(1);
            }
            else
#endif
                return(-1);
        }

        if (parameter==sim_boolparam_infotext_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->infoWindowOpenState=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_fullscreen)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+32))
                return(-1);
            if (App::isFullScreen())
            {
                if (boolState==0)
                    App::setFullScreen(false);
            }
            else
            {
                if (boolState!=0)
                    App::setFullScreen(true);
            }
            return(1);
        }

        if (parameter==sim_boolparam_statustext_open)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->statusBoxOpenState=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_fog_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->setFogEnabled(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_scene_and_model_load_messages)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,1+2+4+8+16+32))
                return(-1);
            outputSceneOrModelLoadMessagesWithApiCall=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_console_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            App::userSettings->alwaysShowConsole=(boolState!=0);
#ifdef SIM_WITH_GUI
            App::setShowConsole(App::userSettings->alwaysShowConsole);
#endif
            return(1);
        }

        if (parameter==sim_boolparam_collision_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->collisionDetectionEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_distance_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->distanceCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_ik_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->ikCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_gcs_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->gcsCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_dynamics_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            App::currentWorld->dynamicsContainer->setDynamicsEnabled(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_joint_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->jointMotionHandlingEnabled_DEPRECATED=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_path_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->pathMotionHandlingEnabled_DEPRECATED=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_proximity_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->proximitySensorsEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_vision_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->visionSensorsEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_mirrors_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->mirrorsDisabled=(boolState==0);
            return(1);
        }

        if (parameter==sim_boolparam_aux_clip_planes_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->clippingPlanesDisabled=(boolState==0);
            return(1);
        }
        if (parameter==sim_boolparam_reserved3)
        {
            fullModelCopyFromApi=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_realtime_simulation)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                App::currentWorld->simulation->setRealTimeSimulation(boolState!=0);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_boolparam_use_glfinish_cmd)
        {
            App::userSettings->useGlFinish=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_force_show_wireless_emission)
        {
            CBroadcastDataContainer::setWirelessForceShow_emission(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_force_show_wireless_reception)
        {
            CBroadcastDataContainer::setWirelessForceShow_reception(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_mill_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->millsEnabled=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_video_recording_triggered)
        {
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&(!App::mainWindow->simulationRecorder->getIsRecording())&&App::currentWorld->simulation->isSimulationStopped() )
            {
                App::mainWindow->simulationRecorder->setRecorderEnabled(boolState!=0);
                App::mainWindow->simulationRecorder->setShowSavedMessage(boolState==0); // avoid displaying a message at simulation end in this case!
                return(1);
            }
#endif
            return(-1);
        }
    }
    if (couldNotLock)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);

    couldNotLock=true;
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::currentWorld->sceneObjects==nullptr)
            return(-1);
        couldNotLock=false;
        if ( (parameter==sim_boolparam_force_calcstruct_all_visible)||(parameter==sim_boolparam_force_calcstruct_all) )
        {
            int displayAttrib=sim_displayattribute_renderpass;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
            {
                int shapeHandle=App::currentWorld->sceneObjects->getShapeFromIndex(i)->getObjectHandle();
                CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                if (shape->getShouldObjectBeDisplayed(-1,displayAttrib)||(parameter==sim_boolparam_force_calcstruct_all))
                    shape->initializeMeshCalculationStructureIfNeeded();
            }
            return(1);
        }
    }

    if (couldNotLock)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
    return(-1);
}

simInt simGetBoolParam_internal(simInt parameter)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_boolparam_realtime_simulation)
        {
            int retVal=0;
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->getRealTimeSimulation())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_scene_closing)
        {
            int retVal=0;
            if (App::currentWorld->environment==nullptr)
                return(-1);
            if (App::currentWorld->environment->getSceneIsClosingFlag())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_use_glfinish_cmd)
        {
            int retVal=0;
            if (App::userSettings->useGlFinish)
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_force_show_wireless_emission)
        {
            int retVal=0;
            if (CBroadcastDataContainer::getWirelessForceShow_emission())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_force_show_wireless_reception)
        {
            int retVal=0;
            if (CBroadcastDataContainer::getWirelessForceShow_reception())
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_hierarchy_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->dlgCont->isVisible(HIERARCHY_DLG))
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_hierarchy_toolbarbutton_enabled)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getHierarchyToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_play_toolbarbutton_enabled)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getPlayViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_pause_toolbarbutton_enabled)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getPauseViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_stop_toolbarbutton_enabled)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getStopViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_waiting_for_trigger)
        {
            int retVal=0;
            if (waitingForTrigger)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_rosinterface_donotrunmainscript)
        {
            int retVal=0;
            if (doNotRunMainScriptFromRosInterface)
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_objectshift_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getObjectShiftToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_objectrotate_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getObjectRotateToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_browser_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->dlgCont->isVisible(BROWSER_DLG))
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_browser_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getBrowserToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_objproperties_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getObjPropToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_calcmodules_toolbarbutton_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getCalcModulesToggleViaGuiEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_shape_textures_are_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->environment->getShapeTexturesEnabled())
                retVal=1;
            return(retVal);
        }
        if ( (parameter==sim_boolparam_show_w_emitters)||(parameter==sim_boolparam_show_w_receivers) )
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            int retVal=0;
            if (parameter==sim_boolparam_show_w_emitters)
            {
                if (App::currentWorld->environment->getVisualizeWirelessEmitters())
                    retVal=1;
            }
            else
            {
                if (App::currentWorld->environment->getVisualizeWirelessReceivers())
                    retVal=1;
            }
            return(retVal);
        }
        if (parameter==sim_boolparam_fullscreen)
        {
            int retVal=0;
            if (App::isFullScreen())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_headless)
        {
#ifdef SIM_WITH_GUI
            int retVal=0;
            if (App::mainWindow==nullptr)
                retVal=1;
            return(retVal);
#else
            return(1);
#endif
        }
        if (parameter==sim_boolparam_rayvalid)
        {
#ifdef SIM_WITH_GUI
            int retVal=0;
            C3Vector orig,dir;
            if ( (App::mainWindow!=nullptr)&&(App::mainWindow->getMouseRay(orig,dir)) )
                retVal=1;
            return(retVal);
#endif
            return(0);
        }
        if (parameter==sim_boolparam_display_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getOpenGlDisplayEnabled())
                retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_boolparam_infotext_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->infoWindowOpenState)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_statustext_open)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->statusBoxOpenState)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_fog_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->environment->getFogEnabled())
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_rml2_available)
        {
            int retVal=0;
            if (CPluginContainer::currentRuckigPlugin!=nullptr)
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_rml4_available)
        {
            int retVal=0;
            if (CPluginContainer::currentRuckigPlugin!=nullptr)
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_scene_and_model_load_messages)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,1+2+4+8+16+32))
                return(-1);
            int retVal=0;
            if (outputSceneOrModelLoadMessagesWithApiCall)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_console_visible)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int retVal=0;
            if (App::userSettings->alwaysShowConsole)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_collision_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->collisionDetectionEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_distance_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->distanceCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_ik_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->ikCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_gcs_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->gcsCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_dynamics_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->dynamicsContainer->getDynamicsEnabled())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_joint_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->jointMotionHandlingEnabled_DEPRECATED)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_path_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->pathMotionHandlingEnabled_DEPRECATED)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_proximity_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->proximitySensorsEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_vision_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->visionSensorsEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_mirrors_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=1;
            if (App::currentWorld->mainSettings->mirrorsDisabled)
                retVal=0;
            return(retVal);
        }
        if (parameter==sim_boolparam_aux_clip_planes_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=1;
            if (App::currentWorld->mainSettings->clippingPlanesDisabled)
                retVal=0;
            return(retVal);
        }
        if (parameter==sim_boolparam_reserved3)
        {
            int retVal=0;
            if (fullModelCopyFromApi)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_mill_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::currentWorld->mainSettings->millsEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_video_recording_triggered)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&App::mainWindow->simulationRecorder->getRecorderEnabled())
                retVal=1;
#endif
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetArrayParam_internal(simInt parameter,const simVoid* arrayOfValues)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_arrayparam_gravity)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            App::currentWorld->dynamicsContainer->setGravity(C3Vector((float*)arrayOfValues));
            return(1);
        }

        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->setFogStart(((float*)arrayOfValues)[0]);
            App::currentWorld->environment->setFogEnd(((float*)arrayOfValues)[1]);
            App::currentWorld->environment->setFogDensity(((float*)arrayOfValues)[2]);
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->fogBackgroundColor[0]=((float*)arrayOfValues)[0];
            App::currentWorld->environment->fogBackgroundColor[1]=((float*)arrayOfValues)[1];
            App::currentWorld->environment->fogBackgroundColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->backGroundColorDown[0]=((float*)arrayOfValues)[0];
            App::currentWorld->environment->backGroundColorDown[1]=((float*)arrayOfValues)[1];
            App::currentWorld->environment->backGroundColorDown[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->backGroundColor[0]=((float*)arrayOfValues)[0];
            App::currentWorld->environment->backGroundColor[1]=((float*)arrayOfValues)[1];
            App::currentWorld->environment->backGroundColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->ambientLightColor[0]=((float*)arrayOfValues)[0];
            App::currentWorld->environment->ambientLightColor[1]=((float*)arrayOfValues)[1];
            App::currentWorld->environment->ambientLightColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetArrayParam_internal(simInt parameter,simVoid* arrayOfValues)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_arrayparam_gravity)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            C3Vector g(App::currentWorld->dynamicsContainer->getGravity());
            g.copyTo((float*)arrayOfValues);
            return(1);
        }
        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::currentWorld->environment->getFogStart();
            ((float*)arrayOfValues)[1]=App::currentWorld->environment->getFogEnd();
            ((float*)arrayOfValues)[2]=App::currentWorld->environment->getFogDensity();
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::currentWorld->environment->fogBackgroundColor[0];
            ((float*)arrayOfValues)[1]=App::currentWorld->environment->fogBackgroundColor[1];
            ((float*)arrayOfValues)[2]=App::currentWorld->environment->fogBackgroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::currentWorld->environment->backGroundColorDown[0];
            ((float*)arrayOfValues)[1]=App::currentWorld->environment->backGroundColorDown[1];
            ((float*)arrayOfValues)[2]=App::currentWorld->environment->backGroundColorDown[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::currentWorld->environment->backGroundColor[0];
            ((float*)arrayOfValues)[1]=App::currentWorld->environment->backGroundColor[1];
            ((float*)arrayOfValues)[2]=App::currentWorld->environment->backGroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::currentWorld->environment->ambientLightColor[0];
            ((float*)arrayOfValues)[1]=App::currentWorld->environment->ambientLightColor[1];
            ((float*)arrayOfValues)[2]=App::currentWorld->environment->ambientLightColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_random_euler)
        {
            C4Vector r;
            r.buildRandomOrientation();
            C3Vector euler(r.getEulerAngles());
            ((float*)arrayOfValues)[0]=euler(0);
            ((float*)arrayOfValues)[1]=euler(1);
            ((float*)arrayOfValues)[2]=euler(2);
            return(1);
        }
        if (parameter==sim_arrayparam_rayorigin)
        {
            ((float*)arrayOfValues)[0]=0.0f;
            ((float*)arrayOfValues)[1]=0.0f;
            ((float*)arrayOfValues)[2]=0.0f;
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                C3Vector orig,dir;
                if (App::mainWindow->getMouseRay(orig,dir))
                {
                    orig.getInternalData(((float*)arrayOfValues));
                    return(1);
                }
            }
#endif
            return(0);
        }
        if (parameter==sim_arrayparam_raydirection)
        {
            ((float*)arrayOfValues)[0]=0.0f;
            ((float*)arrayOfValues)[1]=0.0f;
            ((float*)arrayOfValues)[2]=1.0f;
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                C3Vector orig,dir;
                if (App::mainWindow->getMouseRay(orig,dir))
                {
                    dir.getInternalData(((float*)arrayOfValues));
                    return(1);
                }
            }
#endif
            return(0);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



simInt simSetInt32Param_internal(simInt parameter,simInt intState)
{
    if (parameter==sim_intparam_verbosity)
    { // called by client app when lib not yet initialized
        App::setConsoleVerbosity(intState);
        return(1);
    }
    if (parameter==sim_intparam_statusbarverbosity)
    { // called by client app when lib not yet initialized
        App::setStatusbarVerbosity(intState);
        return(1);
    }
    if (parameter==sim_intparam_dlgverbosity)
    { // called by client app when lib not yet initialized
        App::setDlgVerbosity(intState);
        return(1);
    }
    if (parameter==sim_intparam_exitcode)
    {
        App::setExitCode(intState);
        return(1);
    }
    if (parameter==sim_intparam_error_report_mode)
    { // keep for backward compatibility
        return(1);
    }
    TRACE_C_API;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_intparam_server_port_next)
        {
            App::userSettings->setNextFreeServerPortToUse(intState);
            return(1);
        }
        if (parameter==sim_intparam_videoencoder_index)
        {
#ifdef SIM_WITH_GUI
            if ( (App::mainWindow!=nullptr)&&(App::mainWindow->simulationRecorder!=nullptr) )
            {
                App::mainWindow->simulationRecorder->setEncoderIndex(intState);
                return(1);
            }
#endif
            return(0);
        }
        if (parameter==sim_intparam_current_page)
        {
            if (App::currentWorld->pageContainer==nullptr)
                return(-1);
#ifdef SIM_WITH_GUI
            App::currentWorld->pageContainer->setActivePage(intState);
#endif
            return(1);
        }
        if (parameter==sim_intparam_visible_layers)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->environment->setActiveLayers((unsigned short)intState);
            return(1);
        }
        if (parameter==sim_intparam_infotext_style)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            App::currentWorld->mainSettings->infoWindowColorStyle=intState;
            return(1);
        }
        if (parameter==sim_intparam_settings)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            App::userSettings->antiAliasing=(intState&1);
            App::userSettings->displayWorldReference=((intState&2)!=0);
            App::userSettings->displayBoundingBoxeWhenObjectSelected=((intState&4)!=0);
            // reserved
            App::userSettings->setUndoRedoEnabled((intState&16)!=0);
            return(1);
        }
        if (parameter==sim_intparam_work_thread_count)
            return(1); // for backward compatibility
        if (parameter==sim_intparam_work_thread_calc_time_ms)
            return(1); // for backward compatibility
        if (parameter==sim_intparam_speedmodifier)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
                return(-1);
            App::currentWorld->simulation->setSpeedModifierIndexOffset(intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
                return(-1);
            if (App::currentWorld->dynamicsContainer->setCurrentIterationCount(intState))
                return(1);
            return(-1);
        }
        if (parameter==sim_intparam_scene_index)
        {
            App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD,intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_engine)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                App::currentWorld->dynamicsContainer->setDynamicEngineType(intState,0);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_intparam_idle_fps)
        {
            App::userSettings->setIdleFps_session(intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_warning_disabled_mask)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            App::currentWorld->dynamicsContainer->setTempDisabledWarnings(intState);
            return(1);
        }
        if (parameter==sim_intparam_simulation_warning_disabled_mask)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            App::currentWorld->simulation->setDisableWarningsFlags(intState);
            return(1);
       }

        if ( (parameter==sim_intparam_prox_sensor_select_down)||(parameter==sim_intparam_prox_sensor_select_up) )
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                if (parameter==sim_intparam_prox_sensor_select_down)
                    App::mainWindow->setProxSensorClickSelectDown(intState);
                else
                    App::mainWindow->setProxSensorClickSelectUp(intState);
            }
#endif
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUInt64Param_internal(simInt parameter,simUInt64* intState)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_uint64param_simulation_time_step_ns)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=App::currentWorld->simulation->getSimulationTimeStep_speedModified_us()*1000;
            return(1);
        }
        if (parameter==sim_uint64param_simulation_time_ns)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=App::currentWorld->simulation->getSimulationTime_us()*1000;
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetInt32Param_internal(simInt parameter,simInt* intState)
{
    TRACE_C_API;
    if (parameter==sim_intparam_error_report_mode)
    {
        intState[0]=1+2+4; // deprecated. Keep for backw. compatibility
        return(1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_intparam_stop_request_counter)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=App::currentWorld->simulation->getStopRequestCounter();
            return(1);
        }
        if (parameter==sim_intparam_program_version)
        {
            intState[0]=SIM_PROGRAM_VERSION_NB;
            return(1);
        }
        if (parameter==sim_intparam_program_revision)
        {
            intState[0]=SIM_PROGRAM_REVISION_NB;
            return(1);
        }
        if (parameter==sim_intparam_program_full_version)
        {
            intState[0]=SIM_PROGRAM_FULL_VERSION_NB;
            return(1);
        }
        if (parameter==sim_intparam_verbosity)
        {
            intState[0]=App::getConsoleVerbosity();
            return(1);
        }
        if (parameter==sim_intparam_exitcode)
        {
            intState[0]=App::getExitCode();
            return(1);
        }
        if (parameter==sim_intparam_mouseclickcounterdown)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            int v=0;
            if (App::mainWindow!=nullptr)
                v=App::mainWindow->getMouseClickActionCounter(true);
            intState[0]=v;
            retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_intparam_mouseclickcounterup)
        {
            int retVal=0;
#ifdef SIM_WITH_GUI
            int v=0;
            if (App::mainWindow!=nullptr)
                v=App::mainWindow->getMouseClickActionCounter(false);
            intState[0]=v;
            retVal=1;
#endif
            return(retVal);
        }
        if (parameter==sim_intparam_bugfix1)
        {
            intState[0]=App::userSettings->bugFix1;
            return(1);
        }
        if (parameter==sim_intparam_dlgverbosity)
        {
            intState[0]=App::getDlgVerbosity();
            return(1);
        }
        if (parameter==sim_intparam_statusbarverbosity)
        {
            intState[0]=App::getStatusbarVerbosity();
            return(1);
        }

        if (parameter==sim_intparam_scene_unique_id)
        {
            if (App::currentWorld->environment==nullptr)
                return(-1);
            intState[0]=App::currentWorld->environment->getSceneUniqueID();
            return(1);
        }
        if (parameter==sim_intparam_platform)
        {
    #ifdef WIN_SIM
            intState[0]=0;
    #endif
    #ifdef MAC_SIM
            intState[0]=1;
    #endif
    #ifdef LIN_SIM
            intState[0]=2;
    #endif
            return(1);
        }
        if (parameter==sim_intparam_event_flags_read_old)
        {
            intState[0]=App::worldContainer->getModificationFlags(false);
            return(1);
        }
        if (parameter==sim_intparam_event_flags_read_clear_old)
        {
            intState[0]=App::worldContainer->getModificationFlags(true);
            return(1);
        }

        if (parameter==sim_intparam_qt_version)
        {
            intState[0]=0;
#ifdef SIM_WITH_QT
            intState[0]=(QT_VERSION>>16)*10000+((QT_VERSION>>8)&255)*100+(QT_VERSION&255)*1;
#endif
            return(1);
        }
        if (parameter==sim_intparam_compilation_version)
        { // for backw. compatibility:
            int v=CSimFlavor::getIntVal(2);
            if (v==-1)
                intState[0]=6;
            if (v==0)
                intState[0]=2;
            if (v==1)
                intState[0]=0;
            if (v==2)
                intState[0]=1;
            if (v==3)
                intState[0]=3;
            return(1);
        }
        if (parameter==sim_intparam_instance_count)
        {
            intState[0]=1;
            return(1);
        }
        if (parameter==sim_intparam_custom_cmd_start_id)
        {
            intState[0]=OUTSIDE_CUSTOMIZED_COMMANDS_START_CMD;
            return(1);
        }
        if (parameter==sim_intparam_current_page)
        {
            if (App::currentWorld->pageContainer==nullptr)
                return(-1);
#ifdef SIM_WITH_GUI
            intState[0]=App::currentWorld->pageContainer->getActivePageIndex();
#else
            intState[0]=0;
#endif
            return(1);
        }
        if (parameter==sim_intparam_visible_layers)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            intState[0]=int(App::currentWorld->environment->getActiveLayers());
            return(1);
        }
        if (parameter==sim_intparam_infotext_style)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::currentWorld->mainSettings==nullptr)
                return(-1);
            intState[0]=int(App::currentWorld->mainSettings->infoWindowColorStyle);
            return(1);
        }
        if (parameter==sim_intparam_edit_mode_type)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            int editMode=App::getEditModeType();
            if (editMode==NO_EDIT_MODE)
                intState[0]=0;
            if (editMode==TRIANGLE_EDIT_MODE)
                intState[0]=1;
            if (editMode==VERTEX_EDIT_MODE)
                intState[0]=2;
            if (editMode==EDGE_EDIT_MODE)
                intState[0]=3;
            if (editMode==PATH_EDIT_MODE_OLD)
                intState[0]=4;
            if (editMode==MULTISHAPE_EDIT_MODE)
                intState[0]=6;
            return(1);
        }
        if (parameter==sim_intparam_work_thread_count)
        {
            intState[0]=0; // for backward compatibility
            return(1);
        }
        if (parameter==sim_intparam_work_thread_calc_time_ms)
        {
            intState[0]=0; // for backward compatibility
            return(1);
        }
        if (parameter==sim_intparam_core_count)
        {
            intState[0]=VThread::getCoreCount();
            return(1);
        }
        if (parameter==sim_intparam_idle_fps)
        {
            intState[0]=App::userSettings->getIdleFps();
            return(1);
        }
        if (parameter==sim_intparam_prox_sensor_select_down)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                intState[0]=App::mainWindow->getProxSensorClickSelectDown();
#endif
            return(1);
        }
        if (parameter==sim_intparam_prox_sensor_select_up)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                intState[0]=App::mainWindow->getProxSensorClickSelectUp();
#endif
            return(1);
        }
        if (parameter==sim_intparam_mouse_buttons)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                intState[0]=App::mainWindow->getMouseButtonState();
                return(1);
            }
            else
#endif
                return(-1);
        }
        if (parameter==sim_intparam_dynamic_warning_disabled_mask)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::currentWorld->dynamicsContainer->getTempDisabledWarnings();
            return(1);
        }
        if (parameter==sim_intparam_simulation_warning_disabled_mask)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=App::currentWorld->simulation->getDisableWarningsFlags();
            return(1);
        }

        if (parameter==sim_intparam_mouse_x)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                int p[2];
                App::mainWindow->getMouseRenderingPos(p);
                intState[0]=p[0];
                return(1);
            }
            else
#endif
                return(-1);
        }
        if (parameter==sim_intparam_mouse_y)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                int p[2];
                App::mainWindow->getMouseRenderingPos(p);
                intState[0]=p[1];
                return(1);
            }
            else
#endif
                return(-1);
        }
        if (parameter==sim_intparam_settings)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            intState[0]=0;
            if (App::userSettings->antiAliasing)
                intState[0]|=1;
            if (App::userSettings->displayWorldReference)
                intState[0]|=2;
            if (App::userSettings->displayBoundingBoxeWhenObjectSelected)
                intState[0]|=4;
            // reserved     intState[0]|=8;
            if (App::userSettings->getUndoRedoEnabled())
                intState[0]|=16;
            return(1);
        }
        if (parameter==sim_intparam_speedmodifier)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
                return(-1);
            intState[0]=App::currentWorld->simulation->getSpeedModifierIndexOffset();
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::currentWorld->dynamicsContainer->getCurrentIterationCount();
            return(1);
        }
        if (parameter==sim_intparam_scene_index)
        {
            intState[0]=App::worldContainer->getCurrentWorldIndex();
            return(1);
        }
#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(parameter==sim_intparam_flymode_camera_handle) )
        {
            intState[0]=-1;
            return(1);
        }
#endif
        if (parameter==sim_intparam_dynamic_step_divider)
        {
            intState[0]=CPluginContainer::dyn_getDynamicStepDivider();
            if (intState[0]>0)
                return(1);
            return(-1);
        }
        if (parameter==sim_intparam_dynamic_engine)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
            return(1);
        }
        if (parameter==sim_intparam_server_port_start)
        {
            intState[0]=App::userSettings->freeServerPortStart;
            return(1);
        }
        if (parameter==sim_intparam_server_port_next)
        {
            intState[0]=App::userSettings->getNextFreeServerPortToUse();
            return(1);
        }
        if (parameter==sim_intparam_videoencoder_index)
        {
#ifdef SIM_WITH_GUI
            if ( (App::mainWindow!=nullptr)&&(App::mainWindow->simulationRecorder!=nullptr) )
            {
                intState[0]=App::mainWindow->simulationRecorder->getEncoderIndex();
                return(1);
            }
#endif
            return(0);
        }
        if (parameter==sim_intparam_server_port_range)
        {
            intState[0]=App::userSettings->freeServerPortRange;
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetFloatParam_internal(simInt parameter,simFloat floatState)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_floatparam_simulation_time_step)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                App::currentWorld->simulation->setSimulationTimeStep_raw_us(quint64(floatState*1000000.0f));
                return(1);
            }
            return(0);
        }
        if (parameter==sim_floatparam_stereo_distance)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                App::mainWindow->setStereoDistance(floatState);
                return(1);
            }
#endif
            return(0);
        }
        if (parameter==sim_floatparam_dynamic_step_size)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                if (App::currentWorld->dynamicsContainer->setCurrentDynamicStepSize(floatState))
                    return(1);
            }
            return(0);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetFloatParam_internal(simInt parameter,simFloat* floatState)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_floatparam_rand)
        {
            floatState[0]=SIM_RAND_FLOAT;
            return(1);
        }
        if (parameter==sim_floatparam_simulation_time_step)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            floatState[0]=float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_us())/1000000.0f;
            return(1);
        }
        if (parameter==sim_floatparam_stereo_distance)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                floatState[0]=App::mainWindow->getStereoDistance();
                return(1);
            }
#endif
            return(0);
        }
        if (parameter==sim_floatparam_dynamic_step_size)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            floatState[0]=App::currentWorld->dynamicsContainer->getCurrentDynamicStepSize();
            return(1);
        }
        if (parameter==sim_floatparam_mouse_wheel_zoom_factor)
        {
            floatState[0]=App::userSettings->mouseWheelZoomFactor;
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetStringParam_internal(simInt parameter,const simChar* str)
{
    if ( (parameter==sim_stringparam_verbosity)||(parameter==sim_stringparam_statusbarverbosity)||(parameter==sim_stringparam_dlgverbosity) )
    { // called by client app when lib not yet initialized
        int v=sim_verbosity_none;
        if (strcmp(str,"errors")==0)
            v=sim_verbosity_errors;
        if (strcmp(str,"warnings")==0)
            v=sim_verbosity_warnings;
        if (strcmp(str,"loadinfos")==0)
            v=sim_verbosity_loadinfos;
        if (strcmp(str,"questions")==0)
            v=sim_verbosity_questions;
        if (strcmp(str,"scripterrors")==0)
            v=sim_verbosity_scripterrors;
        if (strcmp(str,"scriptwarnings")==0)
            v=sim_verbosity_scriptwarnings;
        if (strcmp(str,"scriptinfos")==0)
            v=sim_verbosity_scriptinfos;
        if (strcmp(str,"msgs")==0)
            v=sim_verbosity_msgs;
        if (strcmp(str,"infos")==0)
            v=sim_verbosity_infos;
        if (strcmp(str,"debug")==0)
            v=sim_verbosity_debug;
        if (strcmp(str,"trace")==0)
            v=sim_verbosity_trace;
        if (strcmp(str,"tracelua")==0)
            v=sim_verbosity_tracelua;
        if (strcmp(str,"traceall")==0)
            v=sim_verbosity_traceall;
        if (parameter==sim_stringparam_verbosity)
            App::setConsoleVerbosity(v);
        if (parameter==sim_stringparam_statusbarverbosity)
            App::setStatusbarVerbosity(v);
        if (parameter==sim_stringparam_dlgverbosity)
            App::setDlgVerbosity(v);
        return(1);
    }

    if (parameter==sim_stringparam_startupscriptstring)
    { // called by client app when lib not yet initialized
        App::setStartupScriptString(str);
        return(1);
    }
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_stringparam_video_filename)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                App::mainWindow->simulationRecorder->setPathAndFilename(str);
                return(1);
            }
            else
#endif
            {
                return(0);
            }
        }
        if ((parameter>=sim_stringparam_app_arg1)&&(parameter<=sim_stringparam_app_arg9))
        {
            App::setApplicationArgument(parameter-sim_stringparam_app_arg1,str);
            return(1);
        }
        if (parameter==sim_stringparam_additional_addonscript1)
        {
            App::setAdditionalAddOnScript1(str);
            return(1);
        }
        if (parameter==sim_stringparam_additional_addonscript2)
        {
            App::setAdditionalAddOnScript2(str);
            return(1);
        }
        if (parameter==sim_stringparam_consolelogfilter)
        {
            App::setConsoleLogFilter(str);
            return(1);
        }
        if (parameter==sim_stringparam_importexportdir)
        {
            App::folders->setImportExportPath(str);
            return(1);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStringParam_internal(simInt parameter)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string retVal(CSimFlavor::getStringVal_int(0,parameter));
        bool validParam=(retVal.length()>0);
        if (parameter==sim_stringparam_application_path)
        {
            validParam=true;
            retVal=App::folders->getExecutablePath();
        }
        if (parameter==sim_stringparam_uniqueid)
        {
            validParam=true;
            retVal=CTTUtil::generateUniqueReadableString();
        }
        if (parameter==sim_stringparam_tempdir)
        {
            validParam=true;
            retVal=App::folders->getTempDataPath();
        }
        if (parameter==sim_stringparam_tempscenedir)
        {
            validParam=true;
            retVal=App::folders->getSceneTempDataPath();
        }
        if (parameter==sim_stringparam_datadir)
        {
            validParam=true;
            retVal=App::folders->getAppDataPath();
        }
        if (parameter==sim_stringparam_importexportdir)
        {
            validParam=true;
            retVal=App::folders->getImportExportPath();
        }
        if (parameter==sim_stringparam_scenedefaultdir)
        {
            validParam=true;
            retVal=App::folders->getScenesDefaultPath();
        }
        if (parameter==sim_stringparam_modeldefaultdir)
        {
            validParam=true;
            retVal=App::folders->getModelsDefaultPath();
        }
        if (parameter==sim_stringparam_defaultpython)
        {
            validParam=true;
            retVal=App::userSettings->defaultPython;
        }
        if (parameter==sim_stringparam_additionalpythonpath)
        {
            validParam=true;
            retVal=App::userSettings->additionalPythonPath;
        }
        if (parameter==sim_stringparam_luadir)
        {
            validParam=true;
            retVal=App::folders->getLuaPath();
        }
        if (parameter==sim_stringparam_pythondir)
        {
            validParam=true;
            retVal=App::folders->getPythonPath();
        }
        if (parameter==sim_stringparam_scene_path_and_name)
        {
            validParam=true;
            if (App::currentWorld->mainSettings==nullptr)
                return(nullptr);
            retVal=App::currentWorld->mainSettings->getScenePathAndName();
        }
        if (parameter==sim_stringparam_scene_name)
        {
            validParam=true;
            if (App::currentWorld->mainSettings==nullptr)
                return(nullptr);
            retVal=App::currentWorld->mainSettings->getSceneNameWithExt();
        }
        if (parameter==sim_stringparam_scene_unique_id)
        {
            validParam=true;
            if (App::currentWorld->environment==nullptr)
                return(nullptr);
            retVal=App::currentWorld->environment->getUniquePersistentIdString();
            retVal=CTTUtil::encode64(retVal);
        }
        if (parameter==sim_stringparam_scene_path)
        {
            validParam=true;
            if (App::currentWorld->mainSettings==nullptr)
                return(nullptr);
            retVal=App::currentWorld->mainSettings->getScenePath();
        }
        if (parameter==sim_stringparam_remoteapi_temp_file_dir)
        {
            validParam=true;
            retVal=App::folders->getTempDataPath();
        }
        if (parameter==sim_stringparam_video_filename)
        {
            validParam=true;
#ifdef SIM_WITH_GUI
            if (App::mainWindow==nullptr)
                return(nullptr);
            char userSet;
            retVal=App::mainWindow->simulationRecorder->getPath(&userSet);
            if (userSet==0)
                retVal+="/";
#else
            return(nullptr);
#endif
        }
        if ((parameter>=sim_stringparam_app_arg1)&&(parameter<=sim_stringparam_app_arg9))
        {
            validParam=true;
            retVal=App::getApplicationArgument(parameter-sim_stringparam_app_arg1);
        }
        if (parameter==sim_stringparam_consolelogfilter)
        {
            validParam=true;
            retVal=App::getConsoleLogFilter();
        }
        if (validParam)
        {
            char* retVal2=new char[retVal.length()+1];
            for (size_t i=0;i<retVal.length();i++)
                retVal2[i]=retVal[i];
            retVal2[retVal.length()]=0;
            return(retVal2);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simFloat simGetSimulationTime_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0f);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        float retVal=float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0f);
}

simInt simGetSimulationState_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::currentWorld->simulation->getSimulationState();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simDouble simGetSystemTime_internal()
{
    return(VDateTime::getTime());
}

simInt simLoadScene_internal(const simChar* filename)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }

        std::string nm(filename);
        size_t keepCurrentPos=nm.find("@keepCurrent");
        bool keepCurrent=(keepCurrentPos!=std::string::npos);
        if (keepCurrent)
            nm.erase(nm.begin()+keepCurrentPos,nm.end());

        if (nm.size()!=0)
        {
            if (!VFile::doesFileExist(nm.c_str()))
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FILE_NOT_FOUND);
                return(-1);
            }
        }

        if (keepCurrent)
            CFileOperations::createNewScene(outputSceneOrModelLoadMessagesWithApiCall,true);

        if (!CFileOperations::loadScene(nm.c_str(),outputSceneOrModelLoadMessagesWithApiCall,false))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_COULD_NOT_BE_READ);
            return(-1);
        }
        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCloseScene_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        CFileOperations::closeScene(false);
        return(App::worldContainer->getCurrentWorldIndex());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simLoadModel_internal(const simChar* filename)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string nm(filename);
        size_t atCopyPos=nm.find("@copy"); // deprecated option
        bool forceAsCopy=(atCopyPos!=std::string::npos);
        if (forceAsCopy)
            nm.erase(nm.begin()+atCopyPos,nm.end());

        if (!VFile::doesFileExist(nm.c_str()))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FILE_NOT_FOUND);
            return(-1);
        }
        if (!CFileOperations::loadModel(nm.c_str(),outputSceneOrModelLoadMessagesWithApiCall,false,true,nullptr,false,forceAsCopy))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_MODEL_COULD_NOT_BE_READ);
            return(-1);
        }
        int retVal=App::currentWorld->sceneObjects->getLastSelectionHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetSimulatorMessage_internal(simInt* messageID,simInt* auxiliaryData,simInt* returnedDataSize)
{
    TRACE_C_API;

    char* retVal=App::worldContainer->simulatorMessageQueue->extractOneCommand(messageID[0],auxiliaryData,returnedDataSize[0]);
    return(retVal);
}

simInt simSaveScene_internal(const simChar* filename)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    if (App::currentWorld->environment->getSceneLocked())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::currentWorld->environment->getRequestFinalSave())
            App::currentWorld->environment->setSceneLocked(); // silent locking!

        if (!CFileOperations::saveScene(filename,false,false,false))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_COULD_NOT_BE_SAVED);
            return(-1);
        }
        CFileOperations::addToRecentlyOpenedScenes(App::currentWorld->mainSettings->getScenePathAndName());
        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveModel_internal(int baseOfModelHandle,const simChar* filename)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    if (App::currentWorld->environment->getSceneLocked())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
        return(-1);
    }
    if (!doesObjectExist(__func__,baseOfModelHandle))
        return(-1);
    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(baseOfModelHandle);
    if (!it->getModelBase())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_MODEL_BASE);
        return(-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        const std::vector<int>* initSelection=App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr();
        if (!CFileOperations::saveModel(baseOfModelHandle,filename,false,false))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_MODEL_COULD_NOT_BE_SAVED);
            return(-1);
        }
        App::currentWorld->sceneObjects->setSelectedObjectHandles(initSelection);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simDoesFileExist_internal(const simChar* filename)
{
    TRACE_C_API;

    if (!VFile::doesFileExist(filename))
        return(0);
    return(1);
}

simInt* simGetObjectSel_internal(simInt* cnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        const std::vector<int>* handles=App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr();
        int* retVal=new int[handles->size()];
        for (size_t i=0;i<handles->size();i++)
            retVal[i]=handles->at(i);
        cnt[0]=int(handles->size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectSel_internal(const simInt* handles,simInt cnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::currentWorld->sceneObjects->deselectObjects();
        if ((handles!=nullptr)&&(cnt>0))
        {
            for (int i=0;i<cnt;i++)
                App::currentWorld->sceneObjects->addObjectToSelection(handles[i]);
        }
        return(int(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()->size()));
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simHandleProximitySensor_internal(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (sensorHandle>=0)
        { // handle just one sensor (this is explicit handling)
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
            if (it==nullptr)
            {
                return(-1);
            }
            else
            {
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }

                int retVal=0;
                C3Vector smallest;
                float smallestL=SIM_MAX_FLOAT;
                int detectedObj;
                C3Vector detectedSurf;
                bool detected=it->handleSensor(false,detectedObj,detectedSurf);
                if (detected)
                {
                    smallest=it->getDetectedPoint();
                    smallestL=smallest.getLength();
                    retVal=1;
                    if (detectedPoint!=nullptr)
                    {
                        detectedPoint[0]=smallest(0);
                        detectedPoint[1]=smallest(1);
                        detectedPoint[2]=smallest(2);
                        detectedPoint[3]=smallestL;
                    }
                    if (detectedObjectHandle!=nullptr)
                        detectedObjectHandle[0]=detectedObj;
                    if (normalVector!=nullptr)
                    {
                        normalVector[0]=detectedSurf(0);
                        normalVector[1]=detectedSurf(1);
                        normalVector[2]=detectedSurf(2);
                    }
                }
                return(retVal);
            }
        }
        else
        { // handle several sensors at once (with sim_handle_all or sim_handle_all_except_explicit
            int retVal=0;
            C3Vector allSmallest;
            int detectedObjectID=-1;
            C3Vector detectedSurfaceNormal;
            float allSmallestL=SIM_MAX_FLOAT;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getProximitySensorCount();i++)
            {
                int detectedObj;
                C3Vector detectedSurf;
                CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromIndex(i);
                bool detected=false;
                detected=it->handleSensor(sensorHandle==sim_handle_all_except_explicit,detectedObj,detectedSurf);
                if (detected)
                {
                    C3Vector smallest(it->getDetectedPoint());
                    float smallestL=smallest.getLength();

                    if (smallestL<allSmallestL)
                    {
                        allSmallest=smallest;
                        allSmallestL=smallestL;
                        detectedObjectID=detectedObj;
                        detectedSurfaceNormal=detectedSurf;
                        retVal=1;
                    }
                }
            }
            if (retVal==1)
            {
                if (detectedPoint!=nullptr)
                {
                    detectedPoint[0]=allSmallest(0);
                    detectedPoint[1]=allSmallest(1);
                    detectedPoint[2]=allSmallest(2);
                    detectedPoint[3]=allSmallestL;
                }
                if (detectedObjectHandle!=nullptr)
                    detectedObjectHandle[0]=detectedObjectID;
                if (normalVector!=nullptr)
                {
                    normalVector[0]=detectedSurfaceNormal(0);
                    normalVector[1]=detectedSurfaceNormal(1);
                    normalVector[2]=detectedSurfaceNormal(2);
                }
            }
            return(retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simReadProximitySensor_internal(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isSensor(__func__,sensorHandle))
            return(-1);
        CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        C3Vector dPt;
        int dObjHandle;
        C3Vector nVect;
        int retVal=it->readSensor(dPt,dObjHandle,nVect);
        if (retVal==1)
        {
            if (detectedPoint!=nullptr)
            {
                dPt.copyTo(detectedPoint);
                detectedPoint[3]=dPt.getLength();
            }
            if (detectedObjectHandle!=nullptr)
                detectedObjectHandle[0]=dObjHandle;
            if (normalVector!=nullptr)
                nVect.copyTo(normalVector);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simHandleDynamics_internal(simFloat deltaTime)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->dynamicsContainer->handleDynamics(deltaTime);
        if ((!App::currentWorld->dynamicsContainer->isWorldThere())&&App::currentWorld->dynamicsContainer->getDynamicsEnabled())
        {
            App::currentWorld->dynamicsContainer->markForWarningDisplay_physicsEngineNotSupported();
            return(0);
        }
        return(CPluginContainer::dyn_getDynamicStepDivider());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleMainScript_internal()
{
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);

    int retVal=0;

    // Plugins:
    int data[4]={0,0,0,0};
    int rtVal[4]={-1,-1,-1,-1};
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_mainscriptabouttobecalled,data,nullptr,rtVal);
    delete[] (char*)returnVal;

    // Child scripts & customization scripts:
    bool cs=!App::currentWorld->embeddedScriptContainer->shouldTemporarilySuspendMainScript();

    // Add-on scripts:
    bool as=!App::worldContainer->addOnScriptContainer->shouldTemporarilySuspendMainScript();

    // Sandbox script:
    bool ss=true;
    if (App::worldContainer->sandboxScript!=nullptr)
        ss=!App::worldContainer->sandboxScript->shouldTemporarilySuspendMainScript();

    if ( ( (rtVal[0]==-1)&&cs&&as&&ss )||App::currentWorld->simulation->didStopRequestCounterChangeSinceSimulationStart() )
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getMainScript();
        if (it!=nullptr)
        {
            App::worldContainer->calcInfo->simulationPassStart();

            App::currentWorld->embeddedScriptContainer->broadcastDataContainer.removeTimedOutObjects(float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f); // remove invalid elements
            CThreadPool_old::prepareAllThreadsForResume_calledBeforeMainScript();

            if (it->systemCallMainScript(-1,nullptr,nullptr)>0)
                retVal=sim_script_no_error;
            else
                retVal=sim_script_lua_error;
            App::worldContainer->calcInfo->simulationPassEnd();
        }
        else
        { // we don't have a main script
            retVal=sim_script_main_script_nonexistent; // this should not generate an error
        }
    }
    else
    { // a plugin or customization script doesn't want to run the main script!
        retVal=sim_script_main_script_not_called; // this should not generate an error
    }

    // Following for backward compatibility:
    App::worldContainer->addOnScriptContainer->callScripts(sim_syscb_aos_run_old,nullptr,nullptr);

    return(retVal);
}

simInt simResetScript_internal(simInt scriptHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle==sim_handle_all)
        {
            App::currentWorld->embeddedScriptContainer->killAllSimulationLuaStates();
            return(1);
        }
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        it->resetScript();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAssociateScriptWithObject_internal(simInt scriptHandle,simInt associatedObjectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1;
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        if (it!=nullptr)
        {
            if ( (it->getScriptType()==sim_scripttype_childscript)||(it->getScriptType()==sim_scripttype_customizationscript) )
            {
                if (associatedObjectHandle==-1)
                { // remove association
                    it->setObjectHandleThatScriptIsAttachedTo(-1);
                    App::setLightDialogRefreshFlag();
                    retVal=1;
                }
                else
                { // set association
                    if (doesObjectExist(__func__,associatedObjectHandle))
                    { // object does exist
                        if (it->getObjectHandleThatScriptIsAttachedTo(-1)==-1)
                        { // script not yet associated
                            CScriptObject* currentSimilarObj=nullptr;
                            if (it->getScriptType()==sim_scripttype_childscript)
                                currentSimilarObj=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,associatedObjectHandle);
                            if (it->getScriptType()==sim_scripttype_customizationscript)
                                currentSimilarObj=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,associatedObjectHandle);
                            if (currentSimilarObj==nullptr)
                            {
                                it->setObjectHandleThatScriptIsAttachedTo(associatedObjectHandle);
                                App::setLightDialogRefreshFlag();
                                retVal=1;
                            }
                            else
                                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_ALREADY_ASSOCIATED_WITH_SCRIPT_TYPE);
                        }
                    }
                }
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_NOT_CHILD_OR_CUSTOMIZATION_SCRIPT);
        }
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simAddScript_internal(simInt scriptProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int scriptType=scriptProperty;
        if (scriptProperty&sim_scripttype_threaded_old)
            scriptType=scriptProperty-sim_scripttype_threaded_old;
        CScriptObject* it=new CScriptObject(scriptType);
        if (App::userSettings->keepOldThreadedScripts)
        {
            if (scriptProperty&sim_scripttype_threaded_old)
            {
                it->setThreadedExecution_oldThreads(true);
                it->setExecuteJustOnce_oldThreads(true);
            }
        }
        int retVal=App::currentWorld->embeddedScriptContainer->insertScript(it);
        App::setFullDialogRefreshFlag();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveScript_internal(simInt scriptHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle==sim_handle_all)
        { // We wanna remove all scripts!
            if (!App::currentWorld->simulation->isSimulationStopped())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
                return(-1);
            }
            App::currentWorld->embeddedScriptContainer->removeAllScripts();
            App::setFullDialogRefreshFlag();
            return(1);
        }
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,true);
#endif
        App::currentWorld->embeddedScriptContainer->removeScript_safe(scriptHandle);
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simResetProximitySensor_internal(simInt sensorHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (sensorHandle!=sim_handle_all)&&(sensorHandle!=sim_handle_all_except_explicit) )
        {
            if (!isSensor(__func__,sensorHandle))
            {
                return(-1);
            }
        }
        for (size_t i=0;i<App::currentWorld->sceneObjects->getProximitySensorCount();i++)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromIndex(i);
            if (sensorHandle>=0)
            { // Explicit handling
                it=(CProxSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(sensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                it->resetSensor(false);
                break;
            }
            else
                it->resetSensor(sensorHandle==sim_handle_all_except_explicit);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckProximitySensor_internal(simInt sensorHandle,simInt entityHandle,simFloat* detectedPoint)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(-1);
        if (!isSensor(__func__,sensorHandle))
            return(-1);
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
            return(-1);
        CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        int options=0;
        if (it->getFrontFaceDetection())
            options=options|1;
        if (it->getBackFaceDetection())
            options=options|2;
        if (!it->getClosestObjectMode())
            options=options|4;
        if (it->getNormalCheck())
            options=options|8;
        int retVal=simCheckProximitySensorEx_internal(sensorHandle,entityHandle,options,SIM_MAX_FLOAT,it->getAllowedNormal(),detectedPoint,nullptr,nullptr);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckProximitySensorEx_internal(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(-1);
        if (!isSensor(__func__,sensorHandle))
            return(-1);
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
            return(-1);
        if (entityHandle==sim_handle_all)
            entityHandle=-1;
        if (!App::currentWorld->mainSettings->proximitySensorsEnabled)
            return(0);

        bool frontFace=SIM_IS_BIT_SET(detectionMode,0);
        bool backFace=SIM_IS_BIT_SET(detectionMode,1);
        bool fastDetection=SIM_IS_BIT_SET(detectionMode,2);
        bool limitedAngle=SIM_IS_BIT_SET(detectionMode,3);
        if (!(frontFace||backFace))
            frontFace=true;
        if (detectionThreshold<0.0f)
            detectionThreshold=0.0f;
        tt::limitValue(0.0f,piValD2_f,maxAngle);
        int detectedObj;
        C3Vector dPoint;
        float minThreshold=-1.0f;
        CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        if ( (it!=nullptr)&&(it->convexVolume->getSmallestDistanceEnabled()) )
            minThreshold=it->convexVolume->getSmallestDistanceAllowed();
        C3Vector normV;
        bool returnValue;
        returnValue=CProxSensorRoutine::detectEntity(sensorHandle,entityHandle,!fastDetection,limitedAngle,maxAngle,dPoint,detectionThreshold,frontFace,backFace,detectedObj,minThreshold,normV,true);

        if (returnValue)
        {
            if (detectedPoint!=nullptr)
            {
                detectedPoint[0]=dPoint(0);
                detectedPoint[1]=dPoint(1);
                detectedPoint[2]=dPoint(2);
                detectedPoint[3]=detectionThreshold;
            }
            if (detectedObjectHandle!=nullptr)
                detectedObjectHandle[0]=detectedObj;
            if (normalVector!=nullptr)
            {
                normalVector[0]=normV(0);
                normalVector[1]=normV(1);
                normalVector[2]=normV(2);
            }
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckProximitySensorEx2_internal(simInt sensorHandle,simFloat* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simFloat* normalVector)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isSensor(__func__,sensorHandle))
        {
            return(-1);
        }

        if (!App::currentWorld->mainSettings->proximitySensorsEnabled)
        {
            return(0);
        }
        bool frontFace=SIM_IS_BIT_SET(detectionMode,0);
        bool backFace=SIM_IS_BIT_SET(detectionMode,1);
        bool fastDetection=SIM_IS_BIT_SET(detectionMode,2);
        bool limitedAngle=SIM_IS_BIT_SET(detectionMode,3);
        if (!(frontFace||backFace))
            frontFace=true;
        if (detectionThreshold<0.0f)
            detectionThreshold=0.0f;
        tt::limitValue(0.0f,piValD2_f,maxAngle);
        C3Vector dPoint;
        float minThreshold=-1.0f;
        CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorHandle);
        if ( (it!=nullptr)&&(it->convexVolume->getSmallestDistanceEnabled()) )
            minThreshold=it->convexVolume->getSmallestDistanceAllowed();
        C3Vector normV;
        bool returnValue;
        returnValue=CProxSensorRoutine::detectPrimitive(sensorHandle,vertexPointer,itemType,itemCount,!fastDetection,limitedAngle,maxAngle,dPoint,detectionThreshold,frontFace,backFace,minThreshold,normV);
        if (returnValue)
        {
            if (detectedPoint!=nullptr)
            {
                detectedPoint[0]=dPoint(0);
                detectedPoint[1]=dPoint(1);
                detectedPoint[2]=dPoint(2);
                detectedPoint[3]=detectionThreshold;
            }
            if (normalVector!=nullptr)
            {
                normalVector[0]=normV(0);
                normalVector[1]=normV(1);
                normalVector[2]=normV(2);
            }
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLoadModule_internal(const simChar* filenameAndPath,const simChar* pluginName)
{ // -3: could not load, -2: missing entry points, -1: could not initialize. 0=< : handle of the plugin
  // we cannot lock/unlock, because this function might trigger another thread (GUI) that itself will initialize the plugin and call sim-functions --> forever locked!!
    TRACE_C_API;
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=PLUGIN_LOAD_AND_START_PLUGUITHREADCMD;
    cmdIn.stringParams.push_back(filenameAndPath);
    cmdIn.stringParams.push_back(pluginName);
    App::logMsg(sim_verbosity_loadinfos,"plugin '%s': loading...",pluginName);
    if (VThread::isCurrentThreadTheUiThread())
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    else
    {
        SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING; // Needed when a plugin is loaded on-the-fly
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    int handle=cmdOut.intParams[0];
    if (handle==-3)
    {
    #ifdef WIN_SIM
        App::logMsg(sim_verbosity_errors,"plugin '%s': load failed (could not load). The plugin probably couldn't load dependency libraries. Try rebuilding the plugin.",pluginName);
    #endif
    #ifdef MAC_SIM
        App::logMsg(sim_verbosity_errors,"plugin '%s': load failed (could not load). The plugin probably couldn't load dependency libraries. Try 'otool -L pluginName.dylib' for more infos, or simply rebuild the plugin.",pluginName);
    #endif
    #ifdef LIN_SIM
        App::logMsg(sim_verbosity_errors,"plugin '%s': load failed (could not load). The plugin probably couldn't load dependency libraries. For additional infos, modify the script 'libLoadErrorCheck.sh', run it and inspect the output.",pluginName);
    #endif
    }

    if (handle==-2)
        App::logMsg(sim_verbosity_errors,"plugin '%s': load failed (missing entry points).",pluginName);
    if (handle==-1)
        App::logMsg(sim_verbosity_errors,"plugin '%s': load failed (failed initialization).",pluginName);
    if (handle>=0)
        App::logMsg(sim_verbosity_loadinfos,"plugin '%s': load succeeded.",pluginName);
    return(handle);
}

simInt simUnloadModule_internal(simInt pluginhandle)
{ // we cannot lock/unlock, because this function might trigger another thread (GUI) that itself will initialize the plugin and call sim-functions --> forever locked!!
    TRACE_C_API;
    int retVal=0;
    CPlugin* pl=CPluginContainer::getPluginFromHandle(pluginhandle);
    if (pl!=nullptr)
    {
        std::string nm(pl->getName());
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD;
        cmdIn.intParams.push_back(pluginhandle);
        App::logMsg(sim_verbosity_loadinfos,"plugin '%s': unloading...",nm.c_str());
        if (VThread::isCurrentThreadTheUiThread())
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        else
        {
            SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING; // Needed when a plugin is unloaded on-the-fly
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
        App::logMsg(sim_verbosity_loadinfos,"plugin '%s': done.",nm.c_str());
        if (cmdOut.boolParams[0])
            retVal=1;
    }
    return(retVal);
}

simInt simRegisterScriptCallbackFunction_internal(const simChar* funcNameAtPluginName,const simChar* callTips,simVoid(*callBack)(struct SScriptCallBack* cb))
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {

        std::string funcName;
        std::string pluginName;

        std::string funcNameAtPluginNm(funcNameAtPluginName);
        size_t p=funcNameAtPluginNm.find('@');
        if (p!=std::string::npos)
        {
            pluginName.assign(funcNameAtPluginNm.begin()+p+1,funcNameAtPluginNm.end());
            funcName.assign(funcNameAtPluginNm.begin(),funcNameAtPluginNm.begin()+p);
        }
        if (pluginName.size()<1)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_MISSING_PLUGIN_NAME);
            return(-1);
        }

        bool retVal=1;
        if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomFunction(funcNameAtPluginName))
            retVal=0;// that function already existed. We remove it and replace it!
        CScriptCustomFunction* newFunction=new CScriptCustomFunction(funcNameAtPluginName,callTips,callBack);
        if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomFunction(newFunction))
        {
            delete newFunction;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRegisterScriptVariable_internal(const simChar* varNameAtPluginName,const simChar* varValue,simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        bool retVal=1;
        if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomVariable(varNameAtPluginName))
            retVal=0;// that variable already existed. We remove it and replace it!
        if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomVariable(varNameAtPluginName,varValue,stackHandle))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CUSTOM_LUA_VAR_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRegisterScriptFuncHook_internal(simInt scriptHandle,const simChar* funcToHook,const simChar* userFunction,simBool executeBefore,simInt options)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it!=nullptr)
            retVal=it->registerFunctionHook(funcToHook,userFunction,executeBefore);
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simCreateBuffer_internal(simInt size)
{
    TRACE_C_API;

    simChar* retVal=new char[size];
    return(retVal);
}

simInt simReleaseBuffer_internal(const simChar* buffer)
{
    TRACE_C_API;

    delete[] buffer;
    return(1);
}

simInt simCheckCollision_internal(simInt entity1Handle,simInt entity2Handle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (!doesEntityExist(__func__,entity1Handle))||
            ((entity2Handle!=sim_handle_all)&&(!doesEntityExist(__func__,entity2Handle))))
                return(-1);
        if (entity2Handle==sim_handle_all)
            entity2Handle=-1;

        if (!App::currentWorld->mainSettings->collisionDetectionEnabled)
            return(0);
        bool returnValue=CCollisionRoutine::doEntitiesCollide(entity1Handle,entity2Handle,nullptr,true,true,nullptr);
        return(returnValue);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckCollisionEx_internal(simInt entity1Handle,simInt entity2Handle,simFloat** intersectionSegments)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (!doesEntityExist(__func__,entity1Handle))||
            ((entity2Handle!=sim_handle_all)&&(!doesEntityExist(__func__,entity2Handle))))
        {
            return(-1);
        }
        if (entity2Handle==sim_handle_all)
            entity2Handle=-1;

        if (!App::currentWorld->mainSettings->collisionDetectionEnabled)
        {
            return(0);
        }

        std::vector<float> intersect;
        CCollisionRoutine::doEntitiesCollide(entity1Handle,entity2Handle,&intersect,true,true,nullptr);
        if ( (intersectionSegments!=nullptr)&&(intersect.size()!=0) )
        {
            intersectionSegments[0]=new float[intersect.size()];
            for (int i=0;i<int(intersect.size());i++)
                (*intersectionSegments)[i]=intersect[i];
        }
        return((int)intersect.size()/6);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckDistance_internal(simInt entity1Handle,simInt entity2Handle,simFloat threshold,simFloat* distanceData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (!doesEntityExist(__func__,entity1Handle))||
            ((entity2Handle!=sim_handle_all)&&(!doesEntityExist(__func__,entity2Handle))) )
        {
            return(-1);
        }
        if (entity2Handle==sim_handle_all)
            entity2Handle=-1;

        if (!App::currentWorld->mainSettings->distanceCalculationEnabled)
            return(0);

        int buffer[4];
        App::currentWorld->cacheData->getCacheDataDist(entity1Handle,entity2Handle,buffer);
        if (threshold<=0.0f)
            threshold=SIM_MAX_FLOAT;
        bool result=CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1Handle,entity2Handle,threshold,distanceData,buffer,buffer+2,true,true);
        App::currentWorld->cacheData->setCacheDataDist(entity1Handle,entity2Handle,buffer);
        if (result)
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAdvanceSimulationByOneStep_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        App::currentWorld->simulation->advanceSimulationByOneStep();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSimulationTimeStep_internal(simFloat timeStep)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        App::currentWorld->simulation->setSimulationTimeStep_raw_us(quint64(timeStep*1000000.0f));
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat simGetSimulationTimeStep_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0f);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        float retVal=float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_us())/1000000.0f;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0f);
}

simInt simGetRealTimeSimulation_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->simulation->getRealTimeSimulation())
        {
            return(1);
        }
        else
        {
            return(0);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAdjustRealTimeTimer_internal(simInt instanceIndex,simFloat deltaTime)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->simulation->adjustRealTimeTimer_us(quint64(deltaTime*1000000.0f));
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simIsRealTimeSimulationStepNeeded_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        if (!App::currentWorld->simulation->getRealTimeSimulation())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_REAL_TIME);
            return(-1);
        }
        if (App::currentWorld->simulation->isRealTimeCalculationStepNeeded())
        {
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetSimulationPassesPerRenderingPass_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::currentWorld->simulation->getSimulationPassesPerRendering_speedModified();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSimulationPassesPerRenderingPass_internal(int p)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        p=tt::getLimitedInt(1,512,p);
        App::currentWorld->simulation->setSimulationPassesPerRendering_raw(p);
        return(App::currentWorld->simulation->getSimulationPassesPerRendering_raw());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simStartSimulation_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            App::currentWorld->simulation->startOrResumeSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simStopSimulation_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            App::currentWorld->simulation->incrementStopRequestCounter();
            App::currentWorld->simulation->stopSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simPauseSimulation_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->simulation->isSimulationRunning())
        {
            App::currentWorld->simulation->pauseSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleGraph_internal(simInt graphHandle,simFloat simulationTime)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (graphHandle!=sim_handle_all)&&(graphHandle!=sim_handle_all_except_explicit) )
        {
            if (!isGraph(__func__,graphHandle))
                return(-1);
        }
        if (graphHandle<0)
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getGraphCount();i++)
            {
                CGraph* it=App::currentWorld->sceneObjects->getGraphFromIndex(i);
                if ( (!it->getExplicitHandling())||(graphHandle==sim_handle_all) )
                    it->addNextPoint(simulationTime);
            }
        }
        else
        { // explicit handling
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->addNextPoint(simulationTime);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetGraph_internal(simInt graphHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (graphHandle!=sim_handle_all)&&(graphHandle!=sim_handle_all_except_explicit) )
        {
            if (!isGraph(__func__,graphHandle))
                return(-1);
        }
        if (graphHandle<0)
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getGraphCount();i++)
            {
                CGraph* it=App::currentWorld->sceneObjects->getGraphFromIndex(i);
                if ( (!it->getExplicitHandling())||(graphHandle==sim_handle_all) )
                    it->resetGraph();
            }
        }
        else
        { // explicit handling
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
//            if (!it->getExplicitHandling())
//            {
//                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
//                return(-1);
//            }
            it->resetGraph();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddGraphStream_internal(simInt graphHandle,const simChar* streamName,const simChar* unitStr,simInt options,const simFloat* color,simFloat cyclicRange)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        if (strlen(streamName)==0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_EMPTY_STRING_NOT_ALLOWED);
            return(-1);
        }
        std::string nm(streamName);
        tt::removeIllegalCharacters(nm,false);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphDataStream* str=new CGraphDataStream(nm.c_str(),unitStr,options,color,cyclicRange,_currentScriptHandle);
        int retVal=it->addOrUpdateDataStream(str);
        if (retVal==-1)
        {
            delete str;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDestroyGraphCurve_internal(simInt graphHandle,simInt curveId)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (curveId==-1)
        {
            it->removeAllStreamsAndCurves();
            it->removeAllStreamsAndCurves_old();
            return(1);
        }
        else
        {
            if (it->removeGraphCurve(curveId))
                return(1);
            if (it->removeGraphDataStream(curveId))
                return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CURVE_ID);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetGraphStreamTransformation_internal(simInt graphHandle,simInt streamId,simInt trType,simFloat mult,simFloat off,simInt movingAvgPeriod)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (it->setDataStreamTransformation(streamId,trType,mult,off,movingAvgPeriod))
            return(1);
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CURVE_ID);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDuplicateGraphCurveToStatic_internal(simInt graphHandle,simInt curveId,const simChar* curveName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        std::string nm(curveName);
        tt::removeIllegalCharacters(nm,false);
        int retVal=it->duplicateCurveToStatic(curveId,nm.c_str());
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CURVE_ID);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddGraphCurve_internal(simInt graphHandle,const simChar* curveName,simInt dim,const simInt* streamIds,const simFloat* defaultValues,const simChar* unitStr,simInt options,const simFloat* color,simInt curveWidth)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        if (strlen(curveName)==0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_EMPTY_STRING_NOT_ALLOWED);
            return(-1);
        }
        std::string nm(curveName);
        tt::removeIllegalCharacters(nm,false);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphCurve* curve=new CGraphCurve(dim,streamIds,defaultValues,nm.c_str(),unitStr,options,color,curveWidth,_currentScriptHandle);
        int retVal=it->addOrUpdateCurve(curve);
        if (retVal==-1)
        {
            delete curve;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_OVERWRITE_STATIC_CURVE);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetGraphStreamValue_internal(simInt graphHandle,simInt streamId,simFloat value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        if (it->setNextValueToInsert(streamId,value))
            return(1);
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CURVE_ID);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetModuleName_internal(simInt index,simUChar* moduleVersion)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromIndex(index);
        if (plug==nullptr)
            return(nullptr);
        char* name=new char[plug->getName().length()+1];
        for (size_t i=0;i<plug->getName().length();i++)
            name[i]=plug->getName()[i];
        name[plug->getName().length()]=0;
        if (moduleVersion!=nullptr)
            moduleVersion[0]=plug->pluginVersion;
        return(name);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simAddLog_internal(const simChar* pluginName,simInt verbosityLevel,const simChar* logMsg)
{ // keep this as simple as possible (no trace, no thread checking). For now
    if (App::logPluginMsg(pluginName,verbosityLevel,logMsg))
        return(1);
    return(0);
}

simInt simSetNavigationMode_internal(simInt navigationMode)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    App::setMouseMode(navigationMode);
    return(1);
}

simInt simGetNavigationMode_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    int retVal=App::getMouseMode();
    return(retVal);
}

simInt simSetPage_internal(simInt index)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        App::currentWorld->pageContainer->setActivePage(index);
#endif
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPage_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        int retVal=App::currentWorld->pageContainer->getActivePageIndex();
        return(retVal);
#else
        return(0);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCopyPasteObjects_internal(simInt* objectHandles,simInt objectCount,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        int retVal=0;
        if (objectCount>0)
        {
            // memorize current selection:
            std::vector<int> initSel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                App::currentWorld->sceneObjects->getObjectFromIndex(i)->setCopyString("");

            // adjust the selection to copy:
            std::vector<int> selT;
            for (int i=0;i<objectCount;i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
                if (it!=nullptr)
                {
                    if (((options&1)==0)||it->getModelBase())
                        selT.push_back(objectHandles[i]);
                    // Here we can't use custom data, dna, etc. since it might be stripped away during the copy, dep. on the options
                    it->setCopyString(std::to_string(objectHandles[i]).c_str());
                }
            }
            // if we just wanna handle models, make sure no model has a parent that will also be copied:
            std::vector<int> sel;
            if (options&1)
            {
                for (size_t i=0;i<selT.size();i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(selT[i]);
                    bool ok=true;
                    if (it->getParent()!=nullptr)
                    {
                        for (size_t j=0;j<selT.size();j++)
                        {
                            CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromHandle(selT[j]);
                            if (it!=it2)
                            {
                                if (it->isObjectParentedWith(it2))
                                {
                                    ok=false;
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
                sel.assign(selT.begin(),selT.end());

            if (options&1)
                CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            App::worldContainer->copyBuffer->memorizeBuffer();
            App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),options>>1);
            App::currentWorld->sceneObjects->deselectObjects();
            App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),0);
            App::worldContainer->copyBuffer->restoreBuffer();
            App::worldContainer->copyBuffer->clearMemorizedBuffer();

            // Restore the initial selection:
            App::currentWorld->sceneObjects->deselectObjects();
            for (size_t i=0;i<initSel.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

            for (int i=0;i<objectCount;i++)
            { // now return the handles of the copies. Each input handle has a corresponding output handle:
                CSceneObject* original=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
                if (original!=nullptr)
                {
                    std::string str=original->getCopyString();
                    original->setCopyString("");
                    for (size_t j=0;j<App::currentWorld->sceneObjects->getObjectCount();j++)
                    {
                        CSceneObject* potentialCopy=App::currentWorld->sceneObjects->getObjectFromIndex(j);
                        if (potentialCopy->getCopyString().compare(str)==0)
                        {
                            objectHandles[i]=potentialCopy->getObjectHandle();
                            retVal++;
                            break;
                        }
                    }
                }
                else
                    objectHandles[i]=-1;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simScaleObjects_internal(const simInt* objectHandles,simInt objectCount,simFloat scalingFactor,simBool scalePositionsToo)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        sel.assign(objectHandles,objectHandles+objectCount);
        CSceneObjectOperations::scaleObjects(sel,scalingFactor,scalePositionsToo!=0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddDrawingObject_internal(simInt objectType,simFloat size,simFloat duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (parentObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,parentObjectHandle))
                return(-1);
        }
        int creatorHandle=-1;
        if ( (objectType&sim_drawing_persistent)==0 )
            creatorHandle=_currentScriptHandle;
        CDrawingObject* it=new CDrawingObject(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,creatorHandle);
        if (ambient_diffuse!=nullptr)
            it->color.setColor(ambient_diffuse,sim_colorcomponent_ambient_diffuse);
        if (specular!=nullptr)
            it->color.setColor(specular,sim_colorcomponent_specular);
        if (emission!=nullptr)
        {
            it->color.setColor(emission,sim_colorcomponent_emission);
            if ((objectType&sim_drawing_auxchannelcolor1)!=0)
                it->color.setColor(emission+3,sim_colorcomponent_auxiliary);
        }
        int retVal=App::currentWorld->drawingCont->addObject(it);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveDrawingObject_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }

        if (handle==sim_handle_all)
            App::currentWorld->drawingCont->eraseAllObjects();
        else
        {
            CDrawingObject* it=App::currentWorld->drawingCont->getObject(handle);
            if (it!=nullptr)
                App::currentWorld->drawingCont->removeObject(handle);
            else
            {
                if (handleFlags!=sim_handleflag_silenterror)
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
                return(-1);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddDrawingObjectItem_internal(simInt objectHandle,const simFloat* itemData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
       return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    { // protected with an additional mutex in CDrawingObject
        CDrawingObject* it=App::currentWorld->drawingCont->getObject(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }
        if (it->addItem(itemData))
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}
simInt simAddParticleObject_internal(simInt objectType,simFloat size,simFloat massOverVolume,const simVoid* params,simFloat lifeTime,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        int retVal=CPluginContainer::dyn_addParticleObject(objectType,size,massOverVolume,params,lifeTime,maxItemCount,ambient_diffuse,nullptr,specular,emission);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveParticleObject_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::currentWorld->simulation->isSimulationRunning())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        int retVal=-1;
        if (CPluginContainer::dyn_removeParticleObject(objectHandle)!=0)
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddParticleObjectItem_internal(simInt objectHandle,const simFloat* itemData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    if (!App::currentWorld->simulation->isSimulationRunning())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1;
        if (CPluginContainer::dyn_addParticleObjectItem(objectHandle,itemData,float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f)!=0)
            retVal=1;
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat simGetObjectSizeFactor_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0f);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1.0f);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

        float retVal=it->getSizeFactor();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAnnounceSceneContentChange_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->undoBufferContainer->announceChange())
            return(-1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetInt32Signal_internal(const simChar* signalName,simInt signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->signalContainer->setIntegerSignal(signalName,signalValue,_currentScriptHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetInt32Signal_internal(const simChar* signalName,simInt* signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::currentWorld->signalContainer->getIntegerSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearInt32Signal_internal(const simChar* signalName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::currentWorld->signalContainer->clearAllIntegerSignals();
        else
            retVal=App::currentWorld->signalContainer->clearIntegerSignal(signalName);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetFloatSignal_internal(const simChar* signalName,simFloat signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->signalContainer->setFloatSignal(signalName,signalValue,_currentScriptHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetFloatSignal_internal(const simChar* signalName,simFloat* signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::currentWorld->signalContainer->getFloatSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearFloatSignal_internal(const simChar* signalName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::currentWorld->signalContainer->clearAllFloatSignals();
        else
            retVal=App::currentWorld->signalContainer->clearFloatSignal(signalName);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetDoubleSignal_internal(const simChar* signalName,simDouble signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->signalContainer->setDoubleSignal(signalName,signalValue,_currentScriptHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDoubleSignal_internal(const simChar* signalName,simDouble* signalValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::currentWorld->signalContainer->getDoubleSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearDoubleSignal_internal(const simChar* signalName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::currentWorld->signalContainer->clearAllDoubleSignals();
        else
            retVal=App::currentWorld->signalContainer->clearDoubleSignal(signalName);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetStringSignal_internal(const simChar* signalName,const simChar* signalValue,simInt stringLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->signalContainer->setStringSignal(signalName,std::string(signalValue,stringLength),_currentScriptHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStringSignal_internal(const simChar* signalName,simInt* stringLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string sigVal;

        if (App::currentWorld->signalContainer->getStringSignal(signalName,sigVal))
        {
            char* retVal=new char[sigVal.length()];
            for (unsigned int i=0;i<sigVal.length();i++)
                retVal[i]=sigVal[i];
            stringLength[0]=(int)sigVal.length();
            return(retVal);
        }

        return(nullptr); // signal does not exist
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simClearStringSignal_internal(const simChar* signalName)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::currentWorld->signalContainer->clearAllStringSignals();
        else
            retVal=App::currentWorld->signalContainer->clearStringSignal(signalName);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetSignalName_internal(simInt signalIndex,simInt signalType)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (signalType!=0)&&(signalType!=1)&&(signalType!=2)&&(signalType!=3) )
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(nullptr);
        }
        std::string sigName;
        bool res=false;

        if (signalType==0)
            res=App::currentWorld->signalContainer->getIntegerSignalNameAtIndex(signalIndex,sigName);
        if (signalType==1)
            res=App::currentWorld->signalContainer->getFloatSignalNameAtIndex(signalIndex,sigName);
        if (signalType==2)
            res=App::currentWorld->signalContainer->getStringSignalNameAtIndex(signalIndex,sigName);
        if (signalType==3)
            res=App::currentWorld->signalContainer->getDoubleSignalNameAtIndex(signalIndex,sigName);

        if (res)
        {
            char* retVal=new char[sigName.length()+1];
            for (unsigned int i=0;i<sigName.length();i++)
                retVal[i]=sigName[i];
            retVal[sigName.length()]=0;
            return(retVal);
        }
        return(nullptr); // signal does not exist
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectProperty_internal(simInt objectHandle,simInt prop)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        it->setObjectProperty(prop);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectProperty_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=it->getObjectProperty();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectSpecialProperty_internal(simInt objectHandle,simInt prop)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        it->setLocalObjectSpecialProperty(prop);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectSpecialProperty_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=it->getLocalObjectSpecialProperty();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetModelProperty_internal(simInt objectHandle,simInt modelProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if ((modelProperty&sim_modelproperty_not_model)!=0)
            it->setModelBase(false);
        else
        {
            if (!it->getModelBase())
                it->setModelBase(true);
            it->setModelProperty(modelProperty);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetModelProperty_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal;
        if (it->getModelBase())
            retVal=it->getModelProperty();
        else
            retVal=sim_modelproperty_not_model;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadForceSensor_internal(simInt objectHandle,simFloat* forceVector,simFloat* torqueVector)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }

        if (!doesObjectExist(__func__,handle))
            return(-1);
        if (!isForceSensor(__func__,handle))
            return(-1);
        if (App::currentWorld->simulation->isSimulationStopped())
        {
//            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(0);
        }
        CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(handle);
        int retVal=0;

        C3Vector f,t;
        if (it->getDynamicForces(f,(handleFlags&sim_handleflag_rawvalue)!=0))
        {
            it->getDynamicTorques(t,(handleFlags&sim_handleflag_rawvalue)!=0);
            if (forceVector!=nullptr)
                f.copyTo(forceVector);
            if (torqueVector!=nullptr)
                t.copyTo(torqueVector);
            retVal|=1;
        }

        CSceneObject* child=it->getChildFromIndex(0);
        if (child==nullptr)
            retVal|=2;

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeVertex_internal(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simFloat* relativePosition)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
        {
            return(-1);
        }
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        CMesh* cc=it->getMeshWrapper()->getShapeComponentAtIndex(groupElementIndex);
        if (cc==nullptr)
        {
            return(0);
        }
        std::vector<float> wvert;
        cc->getCumulativeMeshes(wvert,nullptr,nullptr);
        if ( (vertexIndex<0)||(vertexIndex>=int(wvert.size())/3) )
        {
            return(0);
        }
        relativePosition[0]=wvert[3*vertexIndex+0];
        relativePosition[1]=wvert[3*vertexIndex+1];
        relativePosition[2]=wvert[3*vertexIndex+2];
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeTriangle_internal(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simFloat* triangleNormals)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
        {
            return(-1);
        }
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        CMesh* cc=it->getMeshWrapper()->getShapeComponentAtIndex(groupElementIndex);
        if (cc==nullptr)
        {
            return(0);
        }
        std::vector<float> wvert;
        std::vector<int> wind;
        std::vector<float> wnorm;
        cc->getCumulativeMeshes(wvert,&wind,&wnorm);
        if ( (triangleIndex<0)||(triangleIndex>=int(wind.size())/3) )
        {
            return(0);
        }
        if (vertexIndices!=nullptr)
        {
            vertexIndices[0]=wind[3*triangleIndex+0];
            vertexIndices[1]=wind[3*triangleIndex+1];
            vertexIndices[2]=wind[3*triangleIndex+2];
        }
        if (triangleNormals!=nullptr)
        {
            triangleNormals[0]=wnorm[9*triangleIndex+0];
            triangleNormals[1]=wnorm[9*triangleIndex+1];
            triangleNormals[2]=wnorm[9*triangleIndex+2];
            triangleNormals[3]=wnorm[9*triangleIndex+3];
            triangleNormals[4]=wnorm[9*triangleIndex+4];
            triangleNormals[5]=wnorm[9*triangleIndex+5];
            triangleNormals[6]=wnorm[9*triangleIndex+6];
            triangleNormals[7]=wnorm[9*triangleIndex+7];
            triangleNormals[8]=wnorm[9*triangleIndex+8];
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetLightParameters_internal(simInt objectHandle,simFloat* setToNULL,simFloat* diffusePart,simFloat* specularPart)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isLight(__func__,objectHandle))
            return(-1);
        CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        int retVal=0;
        if (it->getLightActive())
            retVal|=1;
        for (int i=0;i<3;i++)
        {
            if (setToNULL!=nullptr)
                setToNULL[0+i]=0.0f;
            if (diffusePart!=nullptr)
                diffusePart[0+i]=it->getColor(true)->getColorsPtr()[3+i];
            if (specularPart!=nullptr)
                specularPart[0+i]=it->getColor(true)->getColorsPtr()[6+i];
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetLightParameters_internal(simInt objectHandle,simInt state,const simFloat* setToNULL,const simFloat* diffusePart,const simFloat* specularPart)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isLight(__func__,objectHandle))
            return(-1);
        CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        it->setLightActive(state&1);
        if (diffusePart!=nullptr)
            it->getColor(true)->setColor(diffusePart,sim_colorcomponent_diffuse);
        if (specularPart!=nullptr)
            it->getColor(true)->setColor(specularPart,sim_colorcomponent_specular);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetVelocity_internal(simInt shapeHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
            return(-1);
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        C3Vector lv(it->getDynamicLinearVelocity());
        C3Vector av(it->getDynamicAngularVelocity());
        if (linearVelocity!=nullptr)
            lv.copyTo(linearVelocity);
        if (angularVelocity!=nullptr)
            av.copyTo(angularVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectVelocity_internal(simInt objectHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        C3Vector lv(it->getMeasuredLinearVelocity());
        C3Vector av;
        if ((handleFlags&sim_handleflag_axis)!=0)
            av=it->getMeasuredAngularVelocityAxis();
        else
            av=it->getMeasuredAngularVelocity3();
        if (linearVelocity!=nullptr)
            lv.copyTo(linearVelocity);
        if (angularVelocity!=nullptr)
            av.copyTo(angularVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointVelocity_internal(simInt jointHandle,simFloat* velocity)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__,jointHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        velocity[0]=it->getMeasuredJointVelocity();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddForceAndTorque_internal(simInt shapeHandle,const simFloat* force,const simFloat* torque)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=shapeHandle;
        int handleFlags=0;
        if (shapeHandle>=0)
        {
            handleFlags=shapeHandle&0x0ff00000;
            handle=shapeHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        if (!isShape(__func__,handle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(handle);
        C3Vector f;
        C3Vector t;
        f.clear();
        t.clear();
        if (force!=nullptr)
            f.set(force);
        if (torque!=nullptr)
            t.set(torque);

        if ((handleFlags&sim_handleflag_resetforce)!=0)
            it->clearAdditionalForce();
        if ((handleFlags&sim_handleflag_resettorque)!=0)
            it->clearAdditionalTorque();

        it->addAdditionalForceAndTorque(f,t);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddForce_internal(simInt shapeHandle,const simFloat* position,const simFloat* force)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=shapeHandle;
        int handleFlags=0;
        if (shapeHandle>=0)
        {
            handleFlags=shapeHandle&0x0ff00000;
            handle=shapeHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        if (!isShape(__func__,handle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(handle);
        C3Vector r(position);
        C3Vector f(force);
        C3Vector t(r^f);
        // f & t are relative to the shape's frame now. We have to make them absolute:
        C4Vector q(it->getCumulativeTransformation().Q);
        f=q*f;
        t=q*t;
        if ((handleFlags&sim_handleflag_resetforcetorque)!=0)
        {
            it->clearAdditionalForce();
            it->clearAdditionalTorque();
        }
        it->addAdditionalForceAndTorque(f,t);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simSetExplicitHandling_internal(simInt objectHandle,int explicitFlags)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (objectHandle>=SIM_IDSTART_SCENEOBJECT)&&(objectHandle<=SIM_IDEND_SCENEOBJECT) )
        { // scene objects
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it->getObjectType()==sim_object_joint_type)
            { // Joints
                ((CJoint*)it)->setExplicitHandling_DEPRECATED(explicitFlags&1);
                return(1);
            }
            if (it->getObjectType()==sim_object_graph_type)
            { // Graphs
                ((CGraph*)it)->setExplicitHandling(explicitFlags&1);
                return(1);
            }
            if (it->getObjectType()==sim_object_mill_type)
            { // Mills
                ((CMill*)it)->setExplicitHandling(explicitFlags&1);
                return(1);
            }
            if (it->getObjectType()==sim_object_path_type)
            { // Paths
                ((CPath_old*)it)->setExplicitHandling(explicitFlags&1);
                return(1);
            }
            if (it->getObjectType()==sim_object_visionsensor_type)
            { // vision sensors
                ((CVisionSensor*)it)->setExplicitHandling(explicitFlags&1);
                return(1);
            }
            if (it->getObjectType()==sim_object_proximitysensor_type)
            { // Proximity sensors
                ((CProxSensor*)it)->setExplicitHandling(explicitFlags&1);
                return(1);
            }
        }
        // Following for backward compatibility (03.11.2020)
        // -------------------------------------------------------
        if ( (objectHandle>=SIM_IDSTART_COLLISION)&&(objectHandle<SIM_IDEND_COLLISION) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (objectHandle>=SIM_IDSTART_DISTANCE)&&(objectHandle<SIM_IDEND_DISTANCE) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (objectHandle>=SIM_IDSTART_IKGROUP)&&(objectHandle<SIM_IDEND_IKGROUP) )
        { // IK objects
            if (!doesIKGroupExist(__func__,objectHandle))
            {
                return(-1);
            }
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        // -------------------------------------------------------
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetExplicitHandling_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (objectHandle>=SIM_IDSTART_SCENEOBJECT)&&(objectHandle<=SIM_IDEND_SCENEOBJECT) )
        { // scene objects
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it->getObjectType()==sim_object_joint_type)
            { // Joints
                bool exp=((CJoint*)it)->getExplicitHandling_DEPRECATED();
                return(exp);
            }
            if (it->getObjectType()==sim_object_graph_type)
            { // Graphs
                bool exp=((CGraph*)it)->getExplicitHandling();
                return(exp);
            }
            if (it->getObjectType()==sim_object_mill_type)
            { // Mills
                bool exp=((CMill*)it)->getExplicitHandling();
                return(exp);
            }
            if (it->getObjectType()==sim_object_path_type)
            { // Paths
                bool exp=((CPath_old*)it)->getExplicitHandling();
                return(exp);
            }
            if (it->getObjectType()==sim_object_visionsensor_type)
            { // vision sensors
                bool exp=((CVisionSensor*)it)->getExplicitHandling();
                return(exp);
            }
            if (it->getObjectType()==sim_object_proximitysensor_type)
            { // Proximity sensors
                bool exp=((CProxSensor*)it)->getExplicitHandling();
                return(exp);
            }
        }
        // Following for backward compatibility (03.11.2020)
        // -------------------------------------------------------
        if ( (objectHandle>=SIM_IDSTART_COLLISION)&&(objectHandle<SIM_IDEND_COLLISION) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(objectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (objectHandle>=SIM_IDSTART_DISTANCE)&&(objectHandle<SIM_IDEND_DISTANCE) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(objectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (objectHandle>=SIM_IDSTART_IKGROUP)&&(objectHandle<SIM_IDEND_IKGROUP) )
        { // IK objects
            if (!doesIKGroupExist(__func__,objectHandle))
            {
                return(-1);
            }
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(objectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        // -------------------------------------------------------
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetLinkDummy_internal(simInt dummyHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isDummy(__func__,dummyHandle))
        {
            return(-1);
        }
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);
        int retVal=it->getLinkedDummyHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetLinkDummy_internal(simInt dummyHandle,simInt linkedDummyHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isDummy(__func__,dummyHandle))
            return(-1);
        if (linkedDummyHandle!=-1)
        {
            if (!isDummy(__func__,linkedDummyHandle))
                return(-1);
        }
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);
        it->setLinkedDummyHandle(linkedDummyHandle,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectColor_internal(simInt objectHandle,simInt index,simInt colorComponent,const simFloat* rgbData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=0;
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            std::vector<CMesh*> all;
            shape->getMeshWrapper()->getAllShapeComponentsCumulative(all);
            if ( (index>=0)&&(index<int(all.size()))&&(colorComponent<=sim_colorcomponent_auxiliary) )
            {
                CMesh* geom=all[index];
                if (colorComponent==sim_colorcomponent_transparency)
                {
                    geom->color.setTranslucid(rgbData[0]!=0.0f);
                    geom->color.setOpacity(rgbData[0]);
                }
                else
                    geom->color.setColor(rgbData,colorComponent);
                geom->color.pushShapeColorChangeEvent(objectHandle,index);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_dummy_type)
        {
            CDummy* dummy=(CDummy*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                dummy->getDummyColor()->setColor(rgbData,colorComponent);
                float cols[9];
                dummy->getDummyColor()->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle,cols);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_camera_type)
        {
            CCamera* camera=(CCamera*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                camera->getColor(false)->setColor(rgbData,colorComponent);
                float cols[9];
                camera->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle,cols);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                joint->getColor(false)->setColor(rgbData,colorComponent);
                float cols[9];
                joint->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle,cols);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_light_type)
        {
            CLight* light=(CLight*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                light->getColor(index==1)->setColor(rgbData,colorComponent);
                retVal=1;
            }
            float cols[2*9];
            light->getColor(false)->getNewColors(cols);
            light->getColor(true)->getNewColors(cols+9);
            CColorObject::pushColorChangeEvent(objectHandle,cols,cols+9);
        }
        if (it->getObjectType()==sim_object_proximitysensor_type)
        {
            CProxSensor* sensor=(CProxSensor*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor(index)->setColor(rgbData,colorComponent);
                retVal=1;
            }
            float cols[2*9];
            sensor->getColor(0)->getNewColors(cols);
            sensor->getColor(1)->getNewColors(cols+9);
            CColorObject::pushColorChangeEvent(objectHandle,cols,cols+9);
        }
        if (it->getObjectType()==sim_object_visionsensor_type)
        {
            CVisionSensor* sensor=(CVisionSensor*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor()->setColor(rgbData,colorComponent);
                float cols[9];
                sensor->getColor()->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle,cols);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_forcesensor_type)
        {
            CForceSensor* sensor=(CForceSensor*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor(false)->setColor(rgbData,colorComponent);
                float cols[9];
                sensor->getColor(false)->getNewColors(cols);
                CColorObject::pushColorChangeEvent(objectHandle,cols);
                retVal=1;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectColor_internal(simInt objectHandle,simInt index,simInt colorComponent,simFloat* rgbData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=0;
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            std::vector<CMesh*> all;
            shape->getMeshWrapper()->getAllShapeComponentsCumulative(all);
            if ( (index>=0)&&(index<int(all.size()))&&(colorComponent<=sim_colorcomponent_auxiliary) )
            {
                CMesh* geom=all[index];
                geom->color.getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_dummy_type)
        {
            CDummy* dummy=(CDummy*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                dummy->getDummyColor()->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_camera_type)
        {
            CCamera* camera=(CCamera*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                camera->getColor(index==1)->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                if (index==0)
                    joint->getColor(false)->getColor(rgbData,colorComponent);
                if (index==1)
                    joint->getColor(true)->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_light_type)
        {
            CLight* light=(CLight*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                light->getColor(index==1)->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_proximitysensor_type)
        {
            CProxSensor* sensor=(CProxSensor*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor(index)->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_visionsensor_type)
        {
            CVisionSensor* sensor=(CVisionSensor*)it;
            if ( (index==0)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor()->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        if (it->getObjectType()==sim_object_forcesensor_type)
        {
            CForceSensor* sensor=(CForceSensor*)it;
            if ( (index>=0)&&(index<=1)&&(colorComponent<=sim_colorcomponent_emission) )
            {
                sensor->getColor(index==1)->getColor(rgbData,colorComponent);
                retVal=1;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetShapeColor_internal(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simFloat* rgbData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (shapeHandle!=sim_handle_all)&&(!isShape(__func__,shapeHandle)) )
            return(-1);
        if (shapeHandle==sim_handle_all)
        { // deprecated functionality
            for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
            {
                CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
                it->setColor(colorName,colorComponent,rgbData);
            }
        }
        else
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            it->setColor(colorName,colorComponent,rgbData);
        }
       return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeColor_internal(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simFloat* rgbData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        int retVal=0;
        if (it->getColor(colorName,colorComponent,rgbData))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetDynamicObject_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0xff00000;
            objectHandle=objectHandle&0xfffff;
        }

        if ( (objectHandle!=sim_handle_all)&&(!doesObjectExist(__func__,objectHandle)) )
            return(-1);
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if ( (it->getObjectHandle()==objectHandle)||(objectHandle==sim_handle_all) )
            {
                if (it->getObjectHandle()==objectHandle)
                    it->setDynamicsResetFlag(true,handleFlags&sim_handleflag_model);
                else
                    it->setDynamicsResetFlag(true,false);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointMode_internal(simInt jointHandle,simInt jointMode,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__,jointHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        it->setJointMode(jointMode);
        it->setHybridFunctionality_old(options&1);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointMode_internal(simInt jointHandle,simInt* options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__,jointHandle))
        {
            return(-1);
        }
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        int retVal=it->getJointMode();
        options[0]=0;
        if (it->getHybridFunctionality_old())
            options[0]|=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialOpen_internal(const simChar* portString,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=-1;
#ifdef SIM_WITH_SERIAL
        handle=App::worldContainer->serialPortContainer->serialPortOpen(false,portString,baudRate);
#endif
        return(handle);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialClose_internal(simInt portHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
#ifdef SIM_WITH_SERIAL
        if (App::worldContainer->serialPortContainer->serialPortClose(portHandle))
            retVal=1;
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialSend_internal(simInt portHandle,const simChar* data,simInt dataLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    std::string dat(data,data+dataLength);
    retVal=App::worldContainer->serialPortContainer->serialPortSend(portHandle,dat);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simSerialRead_internal(simInt portHandle,simChar* buffer,simInt dataLengthToRead)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    std::string data;
    data.resize(dataLengthToRead);
    retVal=App::worldContainer->serialPortContainer->serialPortReceive(portHandle,data,dataLengthToRead);
    if (retVal>0)
    {
        for (int i=0;i<retVal;i++)
            buffer[i]=data[i];
    }
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simSerialCheck_internal(simInt portHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::worldContainer->serialPortContainer->serialPortCheck(portHandle);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simGetContactInfo_internal(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simFloat* contactInfo)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::currentWorld->dynamicsContainer->getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo)!=0)
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAuxiliaryConsoleOpen_internal(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simFloat* textColor,const simFloat* backgroundColor)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=1; // in headless mode, we just return a random handle
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            int tCol[3]={0,0,0};
            int bCol[3]={255,255,255};
            for (size_t i=0;i<3;i++)
            {
                if (textColor!=nullptr)
                    tCol[i]=int(textColor[i]*255.1f);
                if (backgroundColor!=nullptr)
                    bCol[i]=int(backgroundColor[i]*255.1f);
            }
            retVal=App::mainWindow->codeEditorContainer->openConsole(title,maxLines,mode,position,size,tCol,bCol,-1);
        }
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAuxiliaryConsoleClose_internal(simInt consoleHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(App::mainWindow->codeEditorContainer->close(consoleHandle,nullptr,nullptr,nullptr)) )
            return(1);
#endif
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAuxiliaryConsoleShow_internal(simInt consoleHandle,simBool showState)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        int handleFlags=consoleHandle&0x0ff00000;
        int handle=consoleHandle&0x000fffff;
        if ((handleFlags&sim_handleflag_extended)!=0)
        { // we just wanna now if the console is still open
            if ( (App::mainWindow!=nullptr)&&(App::mainWindow->codeEditorContainer->isHandleValid(handle)) )
                return(1);
        }
        else
        { // normal operation
            if (App::mainWindow!=nullptr)
                return(App::mainWindow->codeEditorContainer->showOrHide(handle,showState!=0));
        }
#endif
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAuxiliaryConsolePrint_internal(simInt consoleHandle,const simChar* text)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if (text==nullptr)
                App::mainWindow->codeEditorContainer->setText(consoleHandle,"");
            if (App::mainWindow->codeEditorContainer->appendText(consoleHandle,text))
                return(1);
            return(0);
        }
#endif
        return(1); // in headless mode we fake success
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simImportShape_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return(-1);
        }
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FILE_NOT_FOUND);
            return(-1);
        }
        int op=32;
        if ((options&1)!=0)
            op|=16;
        if ((options&8)!=0)
            op|=2;
        if ((options&16)==0)
            op|=1;
        if ((options&32)!=0)
            op|=64;
        if ((options&128)!=0)
            op|=128;
        int h=-1;
        int cnt=0;
        int* shapes=CPluginContainer::assimp_importShapes(pathAndFilename,512,scalingFactor,1,op,&cnt);
        if (cnt>0)
        {
            h=shapes[0];
            delete[] shapes;
        }
        App::currentWorld->sceneObjects->deselectObjects();
        return(h);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simImportMesh_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor,simFloat*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simFloat*** reserved,simChar*** names)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FILE_NOT_FOUND);
            return(-1);
        }
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return(-1);
        }
        int op=0;
        if ((options&1)!=0)
            op|=16;
        if ((options&128)!=0)
            op|=128;
        int retVal=CPluginContainer::assimp_importMeshes(pathAndFilename,scalingFactor,1,op,vertices,verticesSizes,indices,indicesSizes);
        if (names!=nullptr)
            names[0]=new char*[retVal];
        for (int i=0;i<retVal;i++)
        {
            if (names!=nullptr)
            {
                names[0][i]=new char[1];
                names[0][i][0]=0;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simExportMesh_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat scalingFactor,simInt elementCount,const simFloat** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simFloat** reserved,const simChar** names)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return(-1);
        }
        if (CSimFlavor::getBoolVal(15))
            return(-1);
        std::string format;
        if (fileformat==0)
            format="obj";
        if (fileformat==3)
            format="stl";
        if (fileformat==4)
            format="stlb";
        if (fileformat==5)
            format="collada";
        if (fileformat==6)
            format="ply";
        if (fileformat==7)
            format="plyb";
        if (format.size()==0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_FILE_FORMAT);
            return(-1);
        }
        bool invalidValues=false;
        if (elementCount<1)
            invalidValues=true;
        if (!invalidValues)
        {
            for (int i=0;i<elementCount;i++)
            {
                if ( (verticesSizes[i]<9)||((verticesSizes[i]/3)*3!=verticesSizes[i]) )
                    invalidValues=true;
                if ( (indicesSizes[i]<3)||((indicesSizes[i]/3)*3!=indicesSizes[i]) )
                    invalidValues=true;
            }
        }
        if (invalidValues)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        int op=0;
        CPluginContainer::assimp_exportMeshes(elementCount,vertices,verticesSizes,indices,indicesSizes,pathAndFilename,format.c_str(),scalingFactor,1,op);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simCreateMeshShape_internal(simInt options,simFloat shadingAngle,const simFloat* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simFloat* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ( (indicesSize>=3)&&((indicesSize/3)*3==indicesSize) )
        {
            bool badIndices=false;
            for (int i=0;i<indicesSize;i++)
            {
                if ( (indices[i]<0)||(indices[i]>=verticesSize/3) )
                {
                    badIndices=true;
                    break;
                }
            }
            if (!badIndices)
            {
                if ( (verticesSize>=9)&&((verticesSize/3)*3==verticesSize) )
                {
                    std::vector<float> vert(vertices,vertices+verticesSize);
                    std::vector<int> ind(indices,indices+indicesSize);
                    CShape* shape=new CShape(nullptr,vert,ind,nullptr,nullptr);
                    shape->getSingleMesh()->setShadingAngle(shadingAngle);
                    shape->getSingleMesh()->setEdgeThresholdAngle(shadingAngle);
                    shape->getMeshWrapper()->setLocalInertiaFrame(C7Vector::identityTransformation);
                    shape->setCulling((options&1)!=0);
                    shape->setVisibleEdges((options&2)!=0);
                    App::currentWorld->sceneObjects->addObjectToScene(shape,false,true);
                    return(shape->getObjectHandle());
                }
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BAD_VERTICES);
                return(-1);
            }
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BAD_INDICES);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetShapeMesh_internal(simInt shapeHandle,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simFloat** normals)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<float> wvert;
        std::vector<int> wind;
        std::vector<float> wnorm;
        it->getMeshWrapper()->getCumulativeMeshes(wvert,&wind,&wnorm);
        vertices[0]=new float[wvert.size()];
        verticesSize[0]=int(wvert.size());
        indices[0]=new int[wind.size()];
        indicesSize[0]=int(wind.size());
        if (normals!=nullptr)
            normals[0]=new float[wnorm.size()];
        for (size_t i=0;i<wvert.size();i++)
            vertices[0][i]=wvert[i];
        for (size_t i=0;i<wind.size();i++)
            indices[0][i]=wind[i];
        if (normals!=nullptr)
        {
            for (size_t i=0;i<wnorm.size();i++)
                normals[0][i]=wnorm[i];
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreatePrimitiveShape_internal(simInt primitiveType,const simFloat* sizes,simInt options)
{ // options: bit: 0=culling, 1=sharp edges, 2=open
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        C3Vector s(tt::getLimitedFloat(0.00001f,100000.0f,sizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[1]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[2]));
        CShape* shape=CAddOperations::addPrimitiveShape(primitiveType,s,options,nullptr,0,32,0,false,1);
        int retVal=-1;
        if (shape!=nullptr)
        {
            shape->setLocalTransformation(C7Vector::identityTransformation);
            retVal=shape->getObjectHandle();
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateDummy_internal(simFloat size,const simFloat* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CDummy* it=new CDummy();
        it->setDummySize(size);
        if (reserved!=nullptr)
        {
            it->getDummyColor()->setColor(reserved+0,sim_colorcomponent_ambient_diffuse);
            it->getDummyColor()->setColor(reserved+6,sim_colorcomponent_specular);
            it->getDummyColor()->setColor(reserved+9,sim_colorcomponent_emission);
        }
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateProximitySensor_internal(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CProxSensor* it=new CProxSensor(sensorType);
        it->setSensableType(subType);
        it->setExplicitHandling((options&1)!=0);
        it->setShowVolume((options&4)==0);
        it->setFrontFaceDetection((options&8)==0);
        it->setBackFaceDetection((options&16)==0);
        it->setClosestObjectMode((options&32)==0);
        it->setNormalCheck((options&64)!=0);
        it->convexVolume->setSmallestDistanceEnabled((options&256)!=0);
        it->setRandomizedDetection((sensorType==sim_proximitysensor_ray_subtype)&&(options&512)!=0);

        if ( (sensorType==sim_proximitysensor_cylinder_subtype)||(sensorType==sim_proximitysensor_disc_subtype)||(sensorType==sim_proximitysensor_cone_subtype) )
            it->convexVolume->setFaceNumber(intParams[0]);
        if (sensorType==sim_proximitysensor_disc_subtype)
            it->convexVolume->setFaceNumberFar(intParams[1]);
        if (sensorType==sim_proximitysensor_cone_subtype)
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
        if (sensorType==sim_proximitysensor_pyramid_subtype)
            it->convexVolume->setXSize(floatParams[2]);
        if ( (sensorType==sim_proximitysensor_pyramid_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setYSize(floatParams[3]);
        if (sensorType==sim_proximitysensor_pyramid_subtype)
        {
            it->convexVolume->setXSizeFar(floatParams[4]);
            it->convexVolume->setYSizeFar(floatParams[5]);
        }
        if ( (sensorType==sim_proximitysensor_cone_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setInsideAngleThing(floatParams[6]);

        if ( ((sensorType==sim_proximitysensor_ray_subtype)&&it->getRandomizedDetection())||(sensorType==sim_proximitysensor_cylinder_subtype)||(sensorType==sim_proximitysensor_cone_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setRadius(floatParams[7]);
        if (sensorType==sim_proximitysensor_cylinder_subtype)
            it->convexVolume->setRadiusFar(floatParams[8]);
        if ( ((sensorType==sim_proximitysensor_ray_subtype)&&it->getRandomizedDetection())||(sensorType==sim_proximitysensor_cone_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setAngle(floatParams[9]);

        it->setAllowedNormal(floatParams[10]);
        it->convexVolume->setSmallestDistanceAllowed(floatParams[11]);
        it->setProxSensorSize(floatParams[12]);

        if (reserved!=nullptr)
        {
            it->getColor(0)->setColor(reserved+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(0)->setColor(reserved+6,sim_colorcomponent_specular);
            it->getColor(0)->setColor(reserved+9,sim_colorcomponent_emission);

            it->getColor(1)->setColor(reserved+12,sim_colorcomponent_ambient_diffuse);
            it->getColor(1)->setColor(reserved+18,sim_colorcomponent_specular);
            it->getColor(1)->setColor(reserved+21,sim_colorcomponent_emission);
        }

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateForceSensor_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CForceSensor* it=new CForceSensor();

        it->setEnableForceThreshold((options&1)!=0);
        it->setEnableTorqueThreshold((options&2)!=0);

        it->setFilterType(intParams[0]);
        it->setValueCountForFilter(intParams[1]);
        it->setConsecutiveThresholdViolationsForBreaking(intParams[2]);

        it->setForceSensorSize(floatParams[0]);
        it->setForceThreshold(floatParams[1]);
        it->setTorqueThreshold(floatParams[2]);

        if (reserved!=nullptr)
        {
            it->getColor(false)->setColor(reserved+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(false)->setColor(reserved+6,sim_colorcomponent_specular);
            it->getColor(false)->setColor(reserved+9,sim_colorcomponent_emission);

            it->getColor(true)->setColor(reserved+12,sim_colorcomponent_ambient_diffuse);
            it->getColor(true)->setColor(reserved+18,sim_colorcomponent_specular);
            it->getColor(true)->setColor(reserved+21,sim_colorcomponent_emission);
        }

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateVisionSensor_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CVisionSensor* it=new CVisionSensor();

        it->setExplicitHandling((options&1)!=0);
        it->setPerspective((options&2)!=0);
        it->setShowVolume((options&4)==0);
        it->setUseExternalImage((options&16)!=0);
        it->setUseLocalLights((options&32)!=0);
        it->setShowFogIfAvailable((options&64)==0);
        it->setUseEnvironmentBackgroundColor((options&128)==0);
        it->setResolution(intParams);

        it->setNearClippingPlane(floatParams[0]);
        it->setFarClippingPlane(floatParams[1]);
        if (it->getPerspective())
            it->setViewAngle(floatParams[2]);
        else
            it->setOrthoViewSize(floatParams[2]);
        it->setVisionSensorSize(floatParams[3]);
        it->setDefaultBufferValues(floatParams+6);

        if (reserved!=nullptr)
        {
            it->getColor()->setColor(reserved+0,sim_colorcomponent_ambient_diffuse);
            it->getColor()->setColor(reserved+6,sim_colorcomponent_specular);
            it->getColor()->setColor(reserved+9,sim_colorcomponent_emission);
        }

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}


simInt simCreateJoint_internal(simInt jointType,simInt jointMode,simInt options,const simFloat* sizes,const simFloat* reservedA,const simFloat* reservedB)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CJoint* it=new CJoint(jointType);
        it->setJointMode(jointMode);
        it->setHybridFunctionality_old(options&1);
        if (sizes!=nullptr)
        {
            it->setLength(sizes[0]);
            it->setDiameter(sizes[1]);
        }
        if (reservedA!=nullptr)
        {
            it->getColor(false)->setColor(reservedA+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(false)->setColor(reservedA+6,sim_colorcomponent_specular);
            it->getColor(false)->setColor(reservedA+9,sim_colorcomponent_emission);
        }
        if (reservedB!=nullptr)
        {
            it->getColor(true)->setColor(reservedB+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(true)->setColor(reservedB+6,sim_colorcomponent_specular);
            it->getColor(true)->setColor(reservedB+9,sim_colorcomponent_emission);
        }
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simFloatingViewAdd_internal(simFloat posX,simFloat posY,simFloat sizeX,simFloat sizeY,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
        if (page==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PAGE_INEXISTANT);
            return(-1);
        }
        CSView* theFloatingView=new CSView(-1);
        posX=1.0f-posX;
        posY=1.0f-posY;
        if (posX<0.01f)
            posX=0.01f;
        if (posX>0.99f)
            posX=0.99f;
        if (posY<0.01f)
            posY=0.01f;
        if (posY>0.99f)
            posY=0.99f;
        sizeX=std::min<float>(sizeX,2.0f*std::min<float>(posX,1.0f-posX));
        sizeY=std::min<float>(sizeY,2.0f*std::min<float>(posY,1.0f-posY));
        float sizes[2]={sizeX,sizeY};
        float positions[2]={posX-sizeX*0.5f,posY-sizeY*0.5f};
        page->addFloatingView(theFloatingView,sizes,positions);
        theFloatingView->setCanSwapViewWithMainView(false);
        if (options&1)
            theFloatingView->setCanSwapViewWithMainView(true);
        if (options&2)
            theFloatingView->setCanBeClosed(false);
        if (options&4)
            theFloatingView->setCanBeShifted(false);
        if (options&8)
            theFloatingView->setCanBeResized(false);
        int retVal=theFloatingView->getUniqueID();
        return(retVal);
#else
        return(-1);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simFloatingViewRemove_internal(simInt floatingViewHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        for (int i=0;i<8;i++)
        {
            CSPage* page=App::currentWorld->pageContainer->getPage(i);
            if (page!=nullptr)
            {
                int viewIndex=page->getViewIndexFromViewUniqueID(floatingViewHandle);
                if (viewIndex!=-1)
                {
                    if (size_t(viewIndex)>=page->getRegularViewCount())
                    {
                        page->removeFloatingView(size_t(viewIndex));
                        return(1);
                    }
                    break; // We can't remove the view because it is not floating (anymore?)
                }
            }
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCameraFitToView_internal(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simFloat scaling)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=0;
        if (viewHandleOrIndex>=0)
        {
            handleFlags=viewHandleOrIndex&0xff00000;
            viewHandleOrIndex=viewHandleOrIndex&0xfffff;
        }

        CSView* view=nullptr;
        CCamera* camera=nullptr;
        if ((handleFlags&sim_handleflag_camera)==0)
        { // normal operation: we provide a view
            if (viewHandleOrIndex>=10000)
            {
                for (int i=0;i<8;i++)
                {
                    CSPage* page=App::currentWorld->pageContainer->getPage(i);
                    int index=page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                    if (index!=-1)
                    {
                        view=page->getView(size_t(index));
                        break;
                    }
                }
            }
            else
            {
#ifdef SIM_WITH_GUI
                CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
#else
                CSPage* page=App::currentWorld->pageContainer->getPage(0);
#endif
                if (page==nullptr)
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PAGE_INEXISTANT);
                    return(-1);
                }
                else
                    view=page->getView(size_t(viewHandleOrIndex));
            }
            if (view==nullptr)
            { // silent error
                return(0);
            }
            camera=App::currentWorld->sceneObjects->getCameraFromHandle(view->getLinkedObjectID());
            if (camera==nullptr)
            { // silent error
                return(0);
            }
        }
        else
        { // special operation: we provide a camera
            if (!isCamera(__func__,viewHandleOrIndex))
                return(-1);
            options|=2; // 1x1 proportions
            camera=App::currentWorld->sceneObjects->getCameraFromHandle(viewHandleOrIndex);
        }

        std::vector<int> objectsToFrame;
        if ( (objectCount!=0)&&(objectHandles!=nullptr) )
        {
            for (int i=0;i<objectCount;i++)
                objectsToFrame.push_back(objectHandles[i]);
        }
        float xByY=1.0f;
        if ((options&2)==0)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                int vs[2];
                view->getViewSize(vs);
                xByY=float(vs[0])/float(vs[1]);
            }
            else
#endif
                xByY=455.0f/256.0f; // in headless mode
        }
        bool perspective=true;
        if (view!=nullptr)
            perspective=view->getPerspectiveDisplay();
        camera->frameSceneOrSelectedObjects(xByY,perspective,&objectsToFrame,false,(options&1)==0,scaling,view);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simAdjustView_internal(simInt viewHandleOrIndex,simInt associatedViewableObjectHandle,simInt options,const simChar* viewLabel)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSView* view=nullptr;
        if (viewHandleOrIndex>=10000)
        {
            for (int i=0;i<8;i++)
            {
                CSPage* page=App::currentWorld->pageContainer->getPage(i);
                int index=page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                if (index!=-1)
                {
                    view=page->getView(size_t(index));
                    break;
                }
            }
        }
        else
        {
#ifdef SIM_WITH_GUI
            CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
#else
            CSPage* page=App::currentWorld->pageContainer->getPage(0);
#endif
            if (page==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PAGE_INEXISTANT);
                return(-1);
            }
            else
                view=page->getView(size_t(viewHandleOrIndex));
        }
        if (view==nullptr)
        {
            if (options&0x100)
                return(1);
            return(0);
        }
        if (options&0x100)
            return(2);
        if (options&0x200)
        { // just return the object associated with the view:
            return(view->getLinkedObjectID());
        }
        if (associatedViewableObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,associatedViewableObjectHandle))
            {
                return(-1);
            }
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(associatedViewableObjectHandle);
            int objType=it->getObjectType();
            if ( (objType!=sim_object_camera_type)&&(objType!=sim_object_graph_type)&&(objType!=sim_object_visionsensor_type) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_IS_NOT_VIEWABLE);
                return(-1);
            }
        }
        view->setLinkedObjectID(associatedViewableObjectHandle,true);
        view->setRenderingMode(options&0x000f);
        view->setPerspectiveDisplay((options&0x0010)==0);
        view->setTimeGraph((options&0x0020)==0);
        view->setXYGraphIsOneOneProportional((options&0x0400)!=0);
        view->setRemoveFloatingViewAtSimulationEnd((options&0x0040)!=0);
        view->setDoNotSaveFloatingView((options&0x0080)!=0);

        if (viewLabel!=nullptr)
            view->setAlternativeViewName(viewLabel);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateHeightfieldShape_internal(simInt options,simFloat shadingAngle,simInt xPointCount,simInt yPointCount,simFloat xSize,const simFloat* heights)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ( (xPointCount<2)||(xPointCount>2048)||(yPointCount<2)||(yPointCount>2048)||(xSize<0.00001f) )
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
            return(-1);
        }
        std::vector<std::vector<float>*> allData;
        for (int i=0;i<yPointCount;i++)
        {
            std::vector<float>* vect=new std::vector<float>;
            for (int j=0;j<xPointCount;j++)
                vect->push_back(heights[i*xPointCount+j]);
            allData.push_back(vect);
        }
        int retVal=CFileOperations::apiAddHeightfieldToScene(xPointCount,xSize/(xPointCount-1),allData,shadingAngle,options);
        for (int i=0;i<int(allData.size());i++)
            delete allData[i];
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectInt32Param_internal(simInt objectHandle,simInt parameterID,simInt* parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not retrieved
        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        CGraph* graph=App::currentWorld->sceneObjects->getGraphFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CMirror* mirror=App::currentWorld->sceneObjects->getMirrorFromHandle(objectHandle);
        CProxSensor* proximitySensor=App::currentWorld->sceneObjects->getProximitySensorFromHandle(objectHandle);
        CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (parameterID==sim_objintparam_visibility_layer)
                {
                    parameter[0]=it->getVisibilityLayer();
                    retVal=1;
                }
                if (parameterID==sim_objintparam_visible)
                {
                    if (it->isObjectVisible())
                        parameter[0]=1;
                    else
                        parameter[0]=0;
                    retVal=1;
                }
                if (parameterID==sim_objintparam_unique_id)
                {
                    parameter[0]=int(it->getObjectUid());
                    retVal=1;
                }
                if (parameterID==sim_objintparam_collection_self_collision_indicator)
                {
                    parameter[0]=it->getCollectionSelfCollisionIndicator();
                    retVal=1;
                }
                if (parameterID==sim_objintparam_child_role)
                {
                    parameter[0]=0;
                    if (it->getAssemblyMatchValues(true).size()>0)
                        parameter[0]=1;
                    retVal=1;
                }
                if (parameterID==sim_objintparam_parent_role)
                {
                    parameter[0]=0;
                    if (it->getAssemblyMatchValues(false).size()>0)
                        parameter[0]=1;
                    retVal=1;
                }
                if (parameterID==sim_objintparam_manipulation_permissions)
                {
                    int a=it->getObjectMovementOptions();
                    parameter[0]=(a&0xffff0);
                    if ((a&1)==0)
                        parameter[0]|=1;
                    if ((a&2)==0)
                        parameter[0]|=2;
                    if ((a&4)==0)
                        parameter[0]|=4;
                    if ((a&8)==0)
                        parameter[0]|=8;
                    retVal=1;
                }
                if (parameterID==sim_objintparam_illumination_handle)
                {
                    parameter[0]=it->getSpecificLight();
                    retVal=1;
                }
            }
        }
        if (rendSens!=nullptr)
        {
            if (parameterID==sim_visionintparam_resolution_x)
            {
                int r[2];
                rendSens->getResolution(r);
                parameter[0]=r[0];
                retVal=1;
            }
            if (parameterID==sim_visionintparam_resolution_y)
            {
                int r[2];
                rendSens->getResolution(r);
                parameter[0]=r[1];
                retVal=1;
            }
            if (parameterID==sim_visionintparam_disabled_light_components)
            {
                parameter[0]=rendSens->getDisabledColorComponents();
                retVal=1;
            }
            if (parameterID==sim_visionintparam_rendering_attributes)
            {
                parameter[0]=rendSens->getAttributesForRendering();
                retVal=1;
            }
            if (parameterID==sim_visionintparam_entity_to_render)
            {
                parameter[0]=rendSens->getDetectableEntityHandle();
                retVal=1;
            }
            if ((parameterID>=sim_visionintparam_windowed_size_x)&&(parameterID<=sim_visionintparam_windowed_pos_y))
            {
                int sizeX,sizeY,posX,posY;
                rendSens->getExtWindowSizeAndPos(sizeX,sizeY,posX,posY);
                if (parameterID==sim_visionintparam_windowed_size_x)
                    parameter[0]=sizeX;
                if (parameterID==sim_visionintparam_windowed_size_y)
                    parameter[0]=sizeY;
                if (parameterID==sim_visionintparam_windowed_pos_x)
                    parameter[0]=posX;
                if (parameterID==sim_visionintparam_windowed_pos_y)
                    parameter[0]=posY;
                retVal=1;
            }
            if (parameterID==sim_visionintparam_pov_focal_blur)
            {
                std::string extensionString(rendSens->getExtensionString());
                std::string val;
                parameter[0]=0;
                if (tt::getValueOfKey("focalBlur@povray",extensionString.c_str(),val))
                {
                    if (tt::getLowerUpperCaseString(val,false).compare("true")==0)
                        parameter[0]=1;
                }
                retVal=1;
            }
            if (parameterID==sim_visionintparam_pov_blur_sampled)
            {
                std::string extensionString(rendSens->getExtensionString());
                std::string val;
                parameter[0]=10;
                if (tt::getValueOfKey("blurSamples@povray",extensionString.c_str(),val))
                {
                    int samples;
                    if (tt::getValidInt(val.c_str(),samples))
                        parameter[0]=samples;
                }
                retVal=1;
            }
            if (parameterID==sim_visionintparam_render_mode)
            {
                parameter[0]=rendSens->getRenderMode();
                retVal=1;
            }
            if (parameterID==sim_visionintparam_perspective_operation)
            {
                parameter[0]=0;
                if (rendSens->getPerspective())
                    parameter[0]=1;
                retVal=1;
            }
        }
        if (proximitySensor!=nullptr)
        {
            if (parameterID==sim_proxintparam_ray_invisibility)
            {
                parameter[0]=proximitySensor->getHideDetectionRay();
                retVal=1;
            }
            if (parameterID==sim_proxintparam_volume_type)
            {
                parameter[0]=proximitySensor->convexVolume->getVolumeType()-PYRAMID_TYPE_CONVEX_VOLUME+sim_volume_pyramid;
                if (parameter[0]==sim_volume_ray)
                {
                    if (proximitySensor->getRandomizedDetection())
                        parameter[0]=sim_volume_randomizedray;
                }
                retVal=1;
            }
            if (parameterID==sim_proxintparam_entity_to_detect)
            {
                parameter[0]=proximitySensor->getSensableObject();
                retVal=1;
            }
        }
        if (mill!=nullptr)
        {
            if (parameterID==sim_millintparam_volume_type)
            {
                parameter[0]=mill->convexVolume->getVolumeType()-PYRAMID_TYPE_CONVEX_VOLUME+sim_volume_pyramid;
                retVal=1;
            }
        }
        if (light!=nullptr)
        {
            if (parameterID==sim_lightintparam_pov_casts_shadows)
            {
                std::string extensionString(light->getExtensionString());
                std::string val;
                parameter[0]=1;
                if (tt::getValueOfKey("shadow@povray",extensionString.c_str(),val))
                {
                    if (tt::getLowerUpperCaseString(val,false).compare("false")==0)
                        parameter[0]=0;
                }
                retVal=1;
            }
        }
        if (camera!=nullptr)
        {
            if (parameterID==sim_cameraintparam_perspective_operation)
            {
                parameter[0]=0;
                if (camera->getPerspective())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_remotecameramode)
            {
                parameter[0]=camera->getRemoteCameraMode();
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_trackedobject)
            {
                parameter[0]=camera->getTrackedObjectHandle();
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_disabled_light_components)
            {
                parameter[0]=camera->getDisabledColorComponents();
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_rendering_attributes)
            {
#ifdef SIM_WITH_GUI
                parameter[0]=camera->getAttributesForRendering();
#endif
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_pov_focal_blur)
            {
                std::string extensionString(camera->getExtensionString());
                std::string val;
                parameter[0]=0;
                if (tt::getValueOfKey("focalBlur@povray",extensionString.c_str(),val))
                {
                    if (tt::getLowerUpperCaseString(val,false).compare("true")==0)
                        parameter[0]=1;
                }
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_pov_blur_samples)
            {
                std::string extensionString(camera->getExtensionString());
                std::string val;
                parameter[0]=10;
                if (tt::getValueOfKey("blurSamples@povray",extensionString.c_str(),val))
                {
                    int samples;
                    if (tt::getValidInt(val.c_str(),samples))
                        parameter[0]=samples;
                }
                retVal=1;
            }
        }
        if (dummy!=nullptr)
        {
            if (parameterID==sim_dummyintparam_link_type)
            {
                parameter[0]=dummy->getLinkType();
                retVal=1;
            }
            if (parameterID==sim_dummyintparam_follow_path)
            {
                parameter[0]=dummy->getAssignedToParentPath();
                retVal=1;
            }
        }
        if (graph!=nullptr)
        {
            if (parameterID==sim_graphintparam_needs_refresh)
            {
                parameter[0]=graph->getNeedsRefresh();
                retVal=1;
            }
        }
        if (joint!=nullptr)
        {
            if (parameterID==sim_jointintparam_motor_enabled)
            {
                parameter[0]=0;
                if (joint->getDynCtrlMode()!=sim_jointdynctrl_free)
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_jointintparam_ctrl_enabled)
            {
                parameter[0]=0;
                if (joint->getDynCtrlMode()>=sim_jointdynctrl_position)
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==2020)
            { // deprecated functionality
                parameter[0]=0;
                retVal=1;
            }
            if (parameterID==sim_jointintparam_velocity_lock)
            {
                parameter[0]=0;
                if (joint->getMotorLock())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_jointintparam_vortex_dep_handle)
            {
                parameter[0]=joint->getVortexDependentJointId();
                retVal=1;
            }
            if (parameterID==sim_jointintparam_dynctrlmode)
            {
                parameter[0]=joint->getDynCtrlMode();
                retVal=1;
            }
            if (parameterID==sim_jointintparam_dynvelctrltype)
            {
                parameter[0]=joint->getDynVelCtrlType();
                retVal=1;
            }
            if (parameterID==sim_jointintparam_dynposctrltype)
            {
                parameter[0]=joint->getDynPosCtrlType();
                retVal=1;
            }
        }
        if (shape!=nullptr)
        {
            if (parameterID==sim_shapeintparam_sleepmodestart)
            {
                parameter[0]=0;
                if (shape->getStartInDynamicSleeping())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_static)
            {
                parameter[0]=0;
                if (shape->getShapeIsDynamicallyStatic())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_respondable)
            {
                parameter[0]=0;
                if (shape->getRespondable())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_culling)
            {
                parameter[0]=0;
                if (shape->getCulling())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_wireframe)
            {
                parameter[0]=0;
                if (shape->getShapeWireframe_OLD())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_compound)
            {
                parameter[0]=0;
                if (!shape->getMeshWrapper()->isMesh())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex)
            {
                parameter[0]=0;
                if (shape->getMeshWrapper()->isConvex())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_respondable_mask)
            {
                parameter[0]=int(shape->getDynamicCollisionMask());
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_edge_visibility)
            {
                parameter[0]=0;
                if (shape->getVisibleEdges())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_edge_borders_hidden)
            {
                parameter[0]=0;
                if (shape->getHideEdgeBorders_OLD())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_component_cnt)
            {
                parameter[0]=shape->getComponentCount();
                retVal=1;
            }

        }
        if (mirror!=nullptr)
        {
            if (parameterID==sim_mirrorintparam_enable)
            {
                parameter[0]=mirror->getActive();
                retVal=1;
            }
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectInt32Param_internal(simInt objectHandle,simInt parameterID,simInt parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not set
        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CMirror* mirror=App::currentWorld->sceneObjects->getMirrorFromHandle(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        CProxSensor* proximitySensor=App::currentWorld->sceneObjects->getProximitySensorFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (parameterID==sim_objintparam_visibility_layer)
                {
                    it->setVisibilityLayer(tt::getLimitedInt(0,65535,parameter));
                    retVal=1;
                }
                if (parameterID==sim_objintparam_collection_self_collision_indicator)
                {
                    it->setCollectionSelfCollisionIndicator(parameter);
                    retVal=1;
                }
                if (parameterID==sim_objintparam_child_role)
                {
                    if (parameter==0)
                        it->setAssemblyMatchValues(true,"");
                    retVal=1;
                }
                if (parameterID==sim_objintparam_parent_role)
                {
                    if (parameter==0)
                        it->setAssemblyMatchValues(false,"");
                    retVal=1;
                }
                if (parameterID==sim_objintparam_manipulation_permissions)
                {
                    int a=parameter&(0xffff0);
                    if ((parameter&1)==0)
                        a=a|1;
                    if ((parameter&2)==0)
                        a=a|2;
                    if ((parameter&4)==0)
                        a=a|4;
                    if ((parameter&8)==0)
                        a=a|8;
                    it->setObjectMovementOptions(a);
                    retVal=1;
                }
                if (parameterID==sim_objintparam_illumination_handle)
                {
                    it->setSpecificLight(parameter);
                    retVal=1;
                }
            }
        }
        if (rendSens!=nullptr)
        {
            if ((parameterID==sim_visionintparam_resolution_x)||(parameterID==sim_visionintparam_resolution_y))
            {
                int r[2];
                rendSens->getResolution(r);
                if (parameterID==sim_visionintparam_resolution_x)
                    r[0]=parameter;
                else
                    r[1]=parameter;
                rendSens->setResolution(r);
                retVal=1;
            }
            if (parameterID==sim_visionintparam_disabled_light_components)
            {
                rendSens->setDisabledColorComponents(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionintparam_rendering_attributes)
            {
                rendSens->setAttributesForRendering(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionintparam_entity_to_render)
            {
                rendSens->setDetectableEntityHandle(parameter);
                retVal=1;
            }
            if ((parameterID>=sim_visionintparam_windowed_size_x)&&(parameterID<=sim_visionintparam_windowed_pos_y))
            {
                int sizeX,sizeY,posX,posY;
                rendSens->getExtWindowSizeAndPos(sizeX,sizeY,posX,posY);
                if (parameterID==sim_visionintparam_windowed_size_x)
                    sizeX=parameter;
                if (parameterID==sim_visionintparam_windowed_size_y)
                    sizeY=parameter;
                if (parameterID==sim_visionintparam_windowed_pos_x)
                    posX=parameter;
                if (parameterID==sim_visionintparam_windowed_pos_y)
                    posY=parameter;
                rendSens->setExtWindowSizeAndPos(sizeX,sizeY,posX,posY);
                retVal=1;
            }
            if (parameterID==sim_visionintparam_pov_focal_blur)
            {
                std::string extensionString(rendSens->getExtensionString());
                if (parameter!=0)
                    tt::insertKeyAndValue("focalBlur@povray","true",extensionString);
                else
                    tt::insertKeyAndValue("focalBlur@povray","false",extensionString);
                rendSens->setExtensionString(extensionString.c_str());
                retVal=1;
            }
            if (parameterID==sim_visionintparam_pov_blur_sampled)
            {
                std::string extensionString(rendSens->getExtensionString());
                tt::insertKeyAndValue("blurSamples@povray",tt::FNb(0,parameter,false).c_str(),extensionString);
                rendSens->setExtensionString(extensionString.c_str());
                retVal=1;
            }
            if (parameterID==sim_visionintparam_render_mode)
            {
                rendSens->setRenderMode(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionintparam_perspective_operation)
            {
                rendSens->setPerspective(parameter!=0);
                retVal=1;
            }
        }
        if (proximitySensor!=nullptr)
        {
            if (parameterID==sim_proxintparam_ray_invisibility)
            {
                proximitySensor->setHideDetectionRay(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_proxintparam_entity_to_detect)
            {
                proximitySensor->setSensableObject(parameter);
                retVal=1;
            }
        }
        if (light!=nullptr)
        {
            if (parameterID==sim_lightintparam_pov_casts_shadows)
            {
                std::string extensionString(light->getExtensionString());
                if (parameter!=0)
                    tt::insertKeyAndValue("shadow@povray","true",extensionString);
                else
                    tt::insertKeyAndValue("shadow@povray","false",extensionString);
                light->setExtensionString(extensionString.c_str());
                retVal=1;
            }
        }
        if (camera!=nullptr)
        {
            if (parameterID==sim_cameraintparam_remotecameramode)
            {
                camera->setRemoteCameraMode(parameter);
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_perspective_operation)
            {
                if (parameter!=0)
                    camera->setPerspectiveOperation(true);
                else
                    camera->setPerspectiveOperation(false);
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_trackedobject)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(parameter);
                if ( (it==nullptr)||(it==camera) )
                    parameter=-1;
                camera->setTrackedObjectHandle(parameter);
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_disabled_light_components)
            {
                camera->setDisabledColorComponents(parameter);
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_rendering_attributes)
            {
#ifdef SIM_WITH_GUI
                camera->setAttributesForRendering(parameter);
#endif
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_pov_focal_blur)
            {
                std::string extensionString(camera->getExtensionString());
                if (parameter!=0)
                    tt::insertKeyAndValue("focalBlur@povray","true",extensionString);
                else
                    tt::insertKeyAndValue("focalBlur@povray","false",extensionString);
                camera->setExtensionString(extensionString.c_str());
                retVal=1;
            }
            if (parameterID==sim_cameraintparam_pov_blur_samples)
            {
                std::string extensionString(camera->getExtensionString());
                tt::insertKeyAndValue("blurSamples@povray",tt::FNb(0,parameter,false).c_str(),extensionString);
                camera->setExtensionString(extensionString.c_str());
                retVal=1;
            }
        }
        if (dummy!=nullptr)
        {
            if (parameterID==sim_dummyintparam_link_type)
            {
                dummy->setLinkType(parameter,true);
                retVal=1;
            }
            if (parameterID==sim_dummyintparam_follow_path)
            {
                dummy->setAssignedToParentPath(parameter!=0);
                retVal=1;
            }
        }
        if (joint!=nullptr)
        {
            if (parameterID==sim_jointintparam_motor_enabled)
            { // backward compat. 18.05.2022
                int c=joint->getDynCtrlMode();
                if (c==sim_jointdynctrl_free)
                {
                    if (parameter>0)
                        joint->setDynCtrlMode(sim_jointdynctrl_velocity);
                }
                else
                {
                    if (parameter==0)
                        joint->setDynCtrlMode(sim_jointdynctrl_free);
                }
                retVal=1;
            }
            if (parameterID==sim_jointintparam_ctrl_enabled)
            { // backward compat. 18.05.2022
                int c=joint->getDynCtrlMode();
                if (c<sim_jointdynctrl_position)
                {
                    if (parameter>0)
                        joint->setDynCtrlMode(sim_jointdynctrl_position);
                }
                else
                {
                    if (parameter==0)
                        joint->setDynCtrlMode(sim_jointdynctrl_velocity);
                }
                retVal=1;
            }
            if (parameterID==2020)
            { // deprecated command
                retVal=1;
            }
            if (parameterID==sim_jointintparam_velocity_lock)
            {
                joint->setMotorLock(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_jointintparam_vortex_dep_handle)
            {
                if (joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,parameter))
                    retVal=1;
            }
            if (parameterID==sim_jointintparam_dynctrlmode)
            {
                joint->setDynCtrlMode(parameter);
                retVal=1;
            }
            if (parameterID==sim_jointintparam_dynvelctrltype)
            {
                joint->setDynVelCtrlType(parameter);
                retVal=1;
            }
            if (parameterID==sim_jointintparam_dynposctrltype)
            {
                joint->setDynPosCtrlType(parameter);
                retVal=1;
            }
        }
        if (shape!=nullptr)
        {
            if (parameterID==sim_shapeintparam_sleepmodestart)
            {
                shape->setStartInDynamicSleeping(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_static)
            {
                shape->setShapeIsDynamicallyStatic(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_respondable)
            {
                shape->setRespondable(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_culling)
            {
                shape->setCulling(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_wireframe)
            {
                shape->setShapeWireframe_OLD(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex)
            { // very careful when setting this!
                shape->getMeshWrapper()->setConvex(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex_check)
            {
                shape->getMeshWrapper()->checkIfConvex();
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_respondable_mask)
            {
                shape->setDynamicCollisionMask((unsigned short)parameter);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_edge_visibility)
            {
                shape->setVisibleEdges(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_edge_borders_hidden)
            {
                shape->setHideEdgeBorders_OLD(parameter!=0);
                retVal=1;
            }
        }
        if (mirror!=nullptr)
        {
            if (parameterID==sim_mirrorintparam_enable)
            {
                mirror->setActive(parameter!=0);
                retVal=1;
            }
        }
        if (retVal==0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_SET_PARAMETER);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectFloatParam_internal(simInt objectHandle,simInt parameterID,simFloat* parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not retrieved
        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CForceSensor* forceSensor=App::currentWorld->sceneObjects->getForceSensorFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        CMirror* mirror=App::currentWorld->sceneObjects->getMirrorFromHandle(objectHandle);
        CPathPlanningTask* pathPlanningObject=App::currentWorld->pathPlanning->getObject(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if ((parameterID>=sim_objfloatparam_abs_x_velocity)&&(parameterID<=sim_objfloatparam_abs_z_velocity))
                {
                    parameter[0]=it->getMeasuredLinearVelocity()(parameterID-sim_objfloatparam_abs_x_velocity);
                    retVal=1;
                }
                if (parameterID==sim_objfloatparam_abs_rot_velocity)
                {
                    parameter[0]=it->getMeasuredAngularVelocity();
                    retVal=1;
                }
                if ((parameterID>=sim_objfloatparam_objbbox_min_x)&&(parameterID<=sim_objfloatparam_objbbox_max_z))
                {
                    C3Vector minV,maxV;
                    it->getBoundingBox(minV,maxV);
                    if (parameterID<=sim_objfloatparam_objbbox_min_z)
                        parameter[0]=minV(parameterID-sim_objfloatparam_objbbox_min_x);
                    else
                        parameter[0]=maxV(parameterID-sim_objfloatparam_objbbox_max_x);
                    retVal=1;
                }
                if ((parameterID>=sim_objfloatparam_modelbbox_min_x)&&(parameterID<=sim_objfloatparam_modelbbox_max_z))
                {
                    C3Vector minV,maxV;
                    C7Vector ctmi(it->getCumulativeTransformation().getInverse());
                    bool b=true;
                    if (!it->getGlobalMarkingBoundingBox(ctmi,minV,maxV,b,true,false))
                        retVal=0;
                    else
                    {
                        if (parameterID<=sim_objfloatparam_modelbbox_min_z)
                            parameter[0]=minV(parameterID-sim_objfloatparam_modelbbox_min_x);
                        else
                            parameter[0]=maxV(parameterID-sim_objfloatparam_modelbbox_max_x);
                        retVal=1;
                    }
                }
                if (parameterID==sim_objfloatparam_transparency_offset)
                {
                    parameter[0]=it->getTransparentObjectDistanceOffset();
                    retVal=1;
                }
                if (parameterID==sim_objfloatparam_size_factor)
                {
                    parameter[0]=it->getSizeFactor();
                    retVal=1;
                }
            }
        }
        if (light!=nullptr)
        {
            if (parameterID==sim_lightfloatparam_spot_exponent)
            {
                parameter[0]=float(light->getSpotExponent());
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_spot_cutoff)
            {
                parameter[0]=light->getSpotCutoffAngle();
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_const_attenuation)
            {
                parameter[0]=light->getAttenuationFactor(CONSTANT_ATTENUATION);
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_lin_attenuation)
            {
                parameter[0]=light->getAttenuationFactor(LINEAR_ATTENUATION);
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_quad_attenuation)
            {
                parameter[0]=light->getAttenuationFactor(QUADRATIC_ATTENUATION);
                retVal=1;
            }
        }
        if (rendSens!=nullptr)
        {
            if (parameterID==sim_visionfloatparam_near_clipping)
            {
                parameter[0]=rendSens->getNearClippingPlane();
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_far_clipping)
            {
                parameter[0]=rendSens->getFarClippingPlane();
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_perspective_angle)
            {
                parameter[0]=rendSens->getViewAngle();
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_ortho_size)
            {
                parameter[0]=rendSens->getOrthoViewSize();
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_pov_blur_distance)
            {
                std::string extensionString(rendSens->getExtensionString());
                std::string val;
                parameter[0]=2.0f;
                if (tt::getValueOfKey("focalDist@povray",extensionString.c_str(),val))
                {
                    float dist;
                    if (tt::getValidFloat(val.c_str(),dist))
                        parameter[0]=dist;
                }
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_pov_aperture)
            {
                std::string extensionString(rendSens->getExtensionString());
                std::string val;
                parameter[0]=0.05f;
                if (tt::getValueOfKey("aperture@povray",extensionString.c_str(),val))
                {
                    float ap;
                    if (tt::getValidFloat(val.c_str(),ap))
                        parameter[0]=ap;
                }
                retVal=1;
            }
        }
        if (joint!=nullptr)
        {
            if ((parameterID==sim_jointfloatparam_pid_p)||(parameterID==sim_jointfloatparam_pid_i)||(parameterID==sim_jointfloatparam_pid_d))
            {
                float pp,ip,dp;
                joint->getPid(pp,ip,dp);
                if (parameterID==sim_jointfloatparam_pid_p)
                    parameter[0]=pp;
                if (parameterID==sim_jointfloatparam_pid_i)
                    parameter[0]=ip;
                if (parameterID==sim_jointfloatparam_pid_d)
                    parameter[0]=dp;
                retVal=1;
            }
            if ((parameterID==sim_jointfloatparam_kc_k)||(parameterID==sim_jointfloatparam_kc_c))
            {
                float kp,cp;
                joint->getKc(kp,cp);
                if (parameterID==sim_jointfloatparam_kc_k)
                    parameter[0]=kp;
                if (parameterID==sim_jointfloatparam_kc_c)
                    parameter[0]=cp;
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_ik_weight)
            {
                parameter[0]=joint->getIKWeight_old();
                retVal=1;
            }
            if ( (parameterID>=sim_jointfloatparam_error_x)&&(parameterID<=sim_jointfloatparam_error_g) )
            {
                C3Vector p,o;
                joint->getDynamicJointErrorsFull(p,o);
                if (parameterID==sim_jointfloatparam_error_x)
                    parameter[0]=p(0);
                if (parameterID==sim_jointfloatparam_error_y)
                    parameter[0]=p(1);
                if (parameterID==sim_jointfloatparam_error_z)
                    parameter[0]=p(2);
                if (parameterID==sim_jointfloatparam_error_a)
                    parameter[0]=o(0);
                if (parameterID==sim_jointfloatparam_error_b)
                    parameter[0]=o(1);
                if (parameterID==sim_jointfloatparam_error_g)
                    parameter[0]=o(2);
                retVal=1;
            }
            if ( (parameterID==sim_jointfloatparam_error_pos)||(parameterID==sim_jointfloatparam_error_angle) )
            {
                float p,o;
                joint->getDynamicJointErrors(p,o);
                if (parameterID==sim_jointfloatparam_error_pos)
                    parameter[0]=p;
                if (parameterID==sim_jointfloatparam_error_angle)
                    parameter[0]=o;
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_vortex_dep_multiplication)
            {
                bool ok;
                parameter[0]=joint->getEngineFloatParam(sim_vortex_joint_dependencyfactor,&ok);
                if (ok)
                    retVal=1;
            }
            if (parameterID==sim_jointfloatparam_vortex_dep_offset)
            {
                bool ok;
                parameter[0]=joint->getEngineFloatParam(sim_vortex_joint_dependencyoffset,&ok);
                if (ok)
                    retVal=1;
            }
            if (parameterID==sim_jointfloatparam_screw_pitch)
            {
                parameter[0]=joint->getScrewPitch();
                retVal=1;
            }
            if ( (parameterID>=sim_jointfloatparam_maxvel)&&(parameterID<=sim_jointfloatparam_maxjerk) )
            {
                float v[3];
                joint->getMaxVelAccelJerk(v);
                parameter[0]=v[parameterID-sim_jointfloatparam_maxvel];
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_step_size)
            {
                parameter[0]=joint->getMaxStepSize_old();
                retVal=1;
            }
            if ((parameterID>=sim_jointfloatparam_intrinsic_x)&&(parameterID<=sim_jointfloatparam_intrinsic_qw))
            {
                C7Vector trFull(joint->getFullLocalTransformation());
                C7Vector trPart1(joint->getLocalTransformation());
                C7Vector tr(trPart1.getInverse()*trFull);
                if (parameterID==sim_jointfloatparam_intrinsic_x)
                    parameter[0]=tr.X(0);
                if (parameterID==sim_jointfloatparam_intrinsic_y)
                    parameter[0]=tr.X(1);
                if (parameterID==sim_jointfloatparam_intrinsic_z)
                    parameter[0]=tr.X(2);
                if (parameterID==sim_jointfloatparam_intrinsic_qx)
                    parameter[0]=tr.Q(1);
                if (parameterID==sim_jointfloatparam_intrinsic_qy)
                    parameter[0]=tr.Q(2);
                if (parameterID==sim_jointfloatparam_intrinsic_qz)
                    parameter[0]=tr.Q(3);
                if (parameterID==sim_jointfloatparam_intrinsic_qw)
                    parameter[0]=tr.Q(0);
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_velocity)
            {
                parameter[0]=joint->getMeasuredJointVelocity();
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_upper_limit)
            {
                float maxVelAccelJerk[3];
                joint->getMaxVelAccelJerk(maxVelAccelJerk);
                parameter[0]=maxVelAccelJerk[0];
                retVal=1;
            }
        }
        if (shape!=nullptr)
        {
            if ((parameterID==sim_shapefloatparam_init_velocity_x)||(parameterID==sim_shapefloatparam_init_velocity_y)||(parameterID==sim_shapefloatparam_init_velocity_z))
            {
                C3Vector v(shape->getInitialDynamicLinearVelocity());
                if (parameterID==sim_shapefloatparam_init_velocity_x)
                    parameter[0]=v(0);
                if (parameterID==sim_shapefloatparam_init_velocity_y)
                    parameter[0]=v(1);
                if (parameterID==sim_shapefloatparam_init_velocity_z)
                    parameter[0]=v(2);
                retVal=1;
            }
            if ((parameterID==sim_shapefloatparam_init_ang_velocity_x)||(parameterID==sim_shapefloatparam_init_ang_velocity_y)||(parameterID==sim_shapefloatparam_init_ang_velocity_z))
            {
                C3Vector v(shape->getInitialDynamicAngularVelocity());
                if (parameterID==sim_shapefloatparam_init_ang_velocity_x)
                    parameter[0]=v(0);
                if (parameterID==sim_shapefloatparam_init_ang_velocity_y)
                    parameter[0]=v(1);
                if (parameterID==sim_shapefloatparam_init_ang_velocity_z)
                    parameter[0]=v(2);
                retVal=1;
            }
            if (parameterID==sim_shapefloatparam_mass)
            {
                parameter[0]=shape->getMeshWrapper()->getMass();
                retVal=1;
            }
            if ((parameterID>=sim_shapefloatparam_texture_x)&&(parameterID<=sim_shapefloatparam_texture_scaling_y))
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    CTextureProperty* tp=shape->getSingleMesh()->getTextureProperty();
                    if (tp!=nullptr)
                    {
                        if (parameterID==sim_shapefloatparam_texture_x)
                            parameter[0]=tp->getTextureRelativeConfig().X(0);
                        if (parameterID==sim_shapefloatparam_texture_y)
                            parameter[0]=tp->getTextureRelativeConfig().X(1);
                        if (parameterID==sim_shapefloatparam_texture_z)
                            parameter[0]=tp->getTextureRelativeConfig().X(2);
                        if (parameterID==sim_shapefloatparam_texture_a)
                            parameter[0]=tp->getTextureRelativeConfig().Q.getEulerAngles()(0);
                        if (parameterID==sim_shapefloatparam_texture_b)
                            parameter[0]=tp->getTextureRelativeConfig().Q.getEulerAngles()(1);
                        if (parameterID==sim_shapefloatparam_texture_g)
                            parameter[0]=tp->getTextureRelativeConfig().Q.getEulerAngles()(2);
                        if (parameterID==sim_shapefloatparam_texture_scaling_x)
                        {
                            float dummyFloat;
                            tp->getTextureScaling(parameter[0],dummyFloat);
                        }
                        if (parameterID==sim_shapefloatparam_texture_scaling_y)
                        {
                            float dummyFloat;
                            tp->getTextureScaling(dummyFloat,parameter[0]);
                        }
                        retVal=1;
                    }
                    else
                        retVal=0;
                }
                else
                    retVal=0;
            }
            if (parameterID==sim_shapefloatparam_shading_angle)
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    parameter[0]=shape->getSingleMesh()->getShadingAngle();
                    retVal=1;
                }
                else
                    retVal=0;
            }
            if (parameterID==sim_shapefloatparam_edge_angle)
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    parameter[0]=shape->getSingleMesh()->getEdgeThresholdAngle();
                    retVal=1;
                }
                else
                    retVal=0;
            }
        }
        if (mirror!=nullptr)
        {
            if (parameterID==sim_mirrorfloatparam_width)
            {
                parameter[0]=mirror->getMirrorWidth();
                retVal=1;
            }
            if (parameterID==sim_mirrorfloatparam_height)
            {
                parameter[0]=mirror->getMirrorHeight();
                retVal=1;
            }
            if (parameterID==sim_mirrorfloatparam_reflectance)
            {
                parameter[0]=mirror->getReflectance();
                retVal=1;
            }
        }
        if (pathPlanningObject!=nullptr)
        {
            if ((parameterID>=sim_pplanfloatparam_x_min)&&(parameterID<=sim_pplanfloatparam_delta_range))
            {
                float sMin[4];
                float sRange[4];
                pathPlanningObject->getSearchRange(sMin,sRange);
                if (parameterID==sim_pplanfloatparam_x_min)
                    parameter[0]=sMin[0];
                if (parameterID==sim_pplanfloatparam_x_range)
                    parameter[0]=sRange[0];
                if (parameterID==sim_pplanfloatparam_y_min)
                    parameter[0]=sMin[1];
                if (parameterID==sim_pplanfloatparam_y_range)
                    parameter[0]=sRange[1];
                if (parameterID==sim_pplanfloatparam_z_min)
                    parameter[0]=sMin[2];
                if (parameterID==sim_pplanfloatparam_z_range)
                    parameter[0]=sRange[2];
                if (parameterID==sim_pplanfloatparam_delta_min)
                    parameter[0]=sMin[3];
                if (parameterID==sim_pplanfloatparam_delta_range)
                    parameter[0]=sRange[3];
                retVal=1;
            }
        }
        if (forceSensor!=nullptr)
        {
            if ( (parameterID>=sim_forcefloatparam_error_x)&&(parameterID<=sim_forcefloatparam_error_g) )
            {
                C3Vector p,o;
                forceSensor->getDynamicErrorsFull(p,o);
                if (parameterID==sim_forcefloatparam_error_x)
                    parameter[0]=p(0);
                if (parameterID==sim_forcefloatparam_error_y)
                    parameter[0]=p(1);
                if (parameterID==sim_forcefloatparam_error_z)
                    parameter[0]=p(2);
                if (parameterID==sim_forcefloatparam_error_a)
                    parameter[0]=o(0);
                if (parameterID==sim_forcefloatparam_error_b)
                    parameter[0]=o(1);
                if (parameterID==sim_forcefloatparam_error_g)
                    parameter[0]=o(2);
                retVal=1;
            }
            if ( (parameterID==sim_forcefloatparam_error_pos)||(parameterID==sim_forcefloatparam_error_angle) )
            {
                float p=forceSensor->getDynamicPositionError();
                float o=forceSensor->getDynamicOrientationError();
                if (parameterID==sim_forcefloatparam_error_pos)
                    parameter[0]=p;
                if (parameterID==sim_forcefloatparam_error_angle)
                    parameter[0]=o;
                retVal=1;
            }
        }
        if (dummy!=nullptr)
        {
            if (parameterID==sim_dummyfloatparam_follow_path_offset)
            {
                parameter[0]=dummy->getVirtualDistanceOffsetOnPath();
                retVal=1;
            }
            if (parameterID==sim_dummyfloatparam_size)
            {
                parameter[0]=dummy->getDummySize();
                retVal=1;
            }
        }
        if (camera!=nullptr)
        {
            if (parameterID==sim_camerafloatparam_near_clipping)
            {
                parameter[0]=camera->getNearClippingPlane();
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_far_clipping)
            {
                parameter[0]=camera->getFarClippingPlane();
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_perspective_angle)
            {
                parameter[0]=camera->getViewAngle();
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_ortho_size)
            {
                parameter[0]=camera->getOrthoViewSize();
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_pov_blur_distance)
            {
                std::string extensionString(camera->getExtensionString());
                std::string val;
                parameter[0]=2.0f;
                if (tt::getValueOfKey("focalDist@povray",extensionString.c_str(),val))
                {
                    float dist;
                    if (tt::getValidFloat(val.c_str(),dist))
                        parameter[0]=dist;
                }
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_pov_aperture)
            {
                std::string extensionString(camera->getExtensionString());
                std::string val;
                parameter[0]=0.05f;
                if (tt::getValueOfKey("aperture@povray",extensionString.c_str(),val))
                {
                    float ap;
                    if (tt::getValidFloat(val.c_str(),ap))
                        parameter[0]=ap;
                }
                retVal=1;
            }
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectFloatParam_internal(simInt objectHandle,simInt parameterID,simFloat parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not set
        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        CMirror* mirror=App::currentWorld->sceneObjects->getMirrorFromHandle(objectHandle);
        CPathPlanningTask* pathPlanningObject=App::currentWorld->pathPlanning->getObject(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (parameterID==sim_objfloatparam_transparency_offset)
                {
                    it->setTransparentObjectDistanceOffset(parameter);
                    retVal=1;
                }
                if (parameterID==sim_objfloatparam_size_factor)
                {
                    it->setSizeFactor(parameter);
                    retVal=1;
                }
            }
        }
        if (light!=nullptr)
        {
            if (parameterID==sim_lightfloatparam_spot_exponent)
            {
                light->setSpotExponent(int(parameter+0.5f));
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_spot_cutoff)
            {
                light->setSpotCutoffAngle(parameter);
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_const_attenuation)
            {
                light->setAttenuationFactor(CONSTANT_ATTENUATION,parameter);
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_lin_attenuation)
            {
                light->setAttenuationFactor(LINEAR_ATTENUATION,parameter);
                retVal=1;
            }
            if (parameterID==sim_lightfloatparam_quad_attenuation)
            {
                light->setAttenuationFactor(QUADRATIC_ATTENUATION,parameter);
                retVal=1;
            }
        }
        if (rendSens!=nullptr)
        {
            if (parameterID==sim_visionfloatparam_near_clipping)
            {
                rendSens->setNearClippingPlane(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_far_clipping)
            {
                rendSens->setFarClippingPlane(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_perspective_angle)
            {
                rendSens->setViewAngle(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_ortho_size)
            {
                rendSens->setOrthoViewSize(parameter);
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_pov_blur_distance)
            {
                std::string extensionString(rendSens->getExtensionString());
                tt::insertKeyAndValue("blurDist@povray",tt::FNb(0,parameter,3,false).c_str(),extensionString);
                rendSens->setExtensionString(extensionString.c_str());
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_pov_aperture)
            {
                std::string extensionString(rendSens->getExtensionString());
                tt::insertKeyAndValue("aperture@povray",tt::FNb(0,parameter,3,false).c_str(),extensionString);
                rendSens->setExtensionString(extensionString.c_str());
                retVal=1;
            }
        }
        if (joint!=nullptr)
        {
            if ((parameterID==sim_jointfloatparam_pid_p)||(parameterID==sim_jointfloatparam_pid_i)||(parameterID==sim_jointfloatparam_pid_d))
            {
                float pp,ip,dp;
                joint->getPid(pp,ip,dp);
                if (parameterID==sim_jointfloatparam_pid_p)
                    pp=parameter;
                if (parameterID==sim_jointfloatparam_pid_i)
                    ip=parameter;
                if (parameterID==sim_jointfloatparam_pid_d)
                    dp=parameter;
                joint->setPid(pp,ip,dp);
                retVal=1;
            }
            if ((parameterID==sim_jointfloatparam_kc_k)||(parameterID==sim_jointfloatparam_kc_c))
            {
                float kp,cp;
                joint->getKc(kp,cp);
                if (parameterID==sim_jointfloatparam_kc_k)
                    kp=parameter;
                if (parameterID==sim_jointfloatparam_kc_c)
                    cp=parameter;
                joint->setKc(kp,cp);
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_ik_weight)
            {
                joint->setIKWeight_old(parameter);
                retVal=1;
            }
            if ((parameterID>=sim_jointfloatparam_spherical_qx)&&(parameterID<=sim_jointfloatparam_spherical_qw))
            {
                if (joint->getJointType()==sim_joint_spherical_subtype)
                {
                    static float buff[3];
                    if (parameterID==sim_jointfloatparam_spherical_qx)
                        buff[0]=parameter;
                    if (parameterID==sim_jointfloatparam_spherical_qy)
                        buff[1]=parameter;
                    if (parameterID==sim_jointfloatparam_spherical_qz)
                        buff[2]=parameter;
                    if (parameterID==sim_jointfloatparam_spherical_qw)
                    {
                        C4Vector q(parameter,buff[0],buff[1],buff[2]);
                        joint->setSphericalTransformation(q);
                    }
                    retVal=1;
                }
                else
                    retVal=0;
            }
            if (parameterID==sim_jointfloatparam_upper_limit)
            {
                float maxVelAccelJerk[3];
                joint->getMaxVelAccelJerk(maxVelAccelJerk);
                maxVelAccelJerk[0]=parameter;
                joint->setMaxVelAccelJerk(maxVelAccelJerk);
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_vortex_dep_multiplication)
            {
                if (joint->setEngineFloatParam(sim_vortex_joint_dependencyfactor,parameter))
                    retVal=1;
            }
            if (parameterID==sim_jointfloatparam_vortex_dep_offset)
            {
                if (joint->setEngineFloatParam(sim_vortex_joint_dependencyoffset,parameter))
                    retVal=1;
            }
            if (parameterID==sim_jointfloatparam_screw_pitch)
            {
                if (joint->setScrewPitch(parameter))
                    retVal=1;
            }
            if ( (parameterID>=sim_jointfloatparam_maxvel)&&(parameterID<=sim_jointfloatparam_maxjerk) )
            {
                float v[3];
                joint->getMaxVelAccelJerk(v);
                v[parameterID-sim_jointfloatparam_maxvel]=parameter;
                joint->setMaxVelAccelJerk(v);
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_step_size)
            {
                joint->setMaxStepSize_old(parameter);
                retVal=1;
            }
        }
        if (shape!=nullptr)
        {
            if ((parameterID==sim_shapefloatparam_init_velocity_x)||(parameterID==sim_shapefloatparam_init_velocity_y)||(parameterID==sim_shapefloatparam_init_velocity_z))
            {
                C3Vector v(shape->getInitialDynamicLinearVelocity());
                if (parameterID==sim_shapefloatparam_init_velocity_x)
                    v(0)=parameter;
                if (parameterID==sim_shapefloatparam_init_velocity_y)
                    v(1)=parameter;
                if (parameterID==sim_shapefloatparam_init_velocity_z)
                    v(2)=parameter;
                shape->setInitialDynamicLinearVelocity(v);
                retVal=1;
            }
            if ((parameterID==sim_shapefloatparam_init_ang_velocity_x)||(parameterID==sim_shapefloatparam_init_ang_velocity_y)||(parameterID==sim_shapefloatparam_init_ang_velocity_z))
            {
                C3Vector v(shape->getInitialDynamicAngularVelocity());
                if (parameterID==sim_shapefloatparam_init_ang_velocity_x)
                    v(0)=parameter;
                if (parameterID==sim_shapefloatparam_init_ang_velocity_y)
                    v(1)=parameter;
                if (parameterID==sim_shapefloatparam_init_ang_velocity_z)
                    v(2)=parameter;
                shape->setInitialDynamicAngularVelocity(v);
                retVal=1;
            }
            if (parameterID==sim_shapefloatparam_mass)
            {
                shape->getMeshWrapper()->setMass(parameter);
                retVal=1;
            }
            if ((parameterID>=sim_shapefloatparam_texture_x)&&(parameterID<=sim_shapefloatparam_texture_scaling_y))
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    CTextureProperty* tp=shape->getSingleMesh()->getTextureProperty();
                    if (tp!=nullptr)
                    {
                        C3Vector pos(tp->getTextureRelativeConfig().X);
                        C3Vector euler(tp->getTextureRelativeConfig().Q.getEulerAngles());
                        float scalingX,scalingY;
                        tp->getTextureScaling(scalingX,scalingY);
                        if (parameterID==sim_shapefloatparam_texture_x)
                            pos(0)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_y)
                            pos(1)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_z)
                            pos(2)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_a)
                            euler(0)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_b)
                            euler(1)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_g)
                            euler(2)=parameter;
                        if (parameterID==sim_shapefloatparam_texture_scaling_x)
                            scalingX=parameter;
                        if (parameterID==sim_shapefloatparam_texture_scaling_y)
                            scalingY=parameter;
                        tp->setTextureRelativeConfig(C7Vector(C4Vector(euler),pos));
                        tp->setTextureScaling(scalingX,scalingY);
                        retVal=1;
                    }
                }
            }
            if (parameterID==sim_shapefloatparam_shading_angle)
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    if (!VThread::isCurrentThreadTheUiThread())
                    { // we are NOT in the UI thread. We execute the command now:
                        shape->getSingleMesh()->setShadingAngle(parameter);
                    }
                    else
                    { // We are in the UI thread. Execute the command via the main thread:
                        SSimulationThreadCommand cmd;
                        cmd.cmdId=SET_SHAPE_SHADING_ANGLE_CMD;
                        cmd.intParams.push_back(shape->getObjectHandle());
                        cmd.floatParams.push_back(parameter);
                        App::appendSimulationThreadCommand(cmd);
                    }
                    retVal=1;
                }
                else
                    retVal=0;
            }
            if (parameterID==sim_shapefloatparam_edge_angle)
            {
                if (shape->getMeshWrapper()->isMesh())
                {
                    if (!VThread::isCurrentThreadTheUiThread())
                    { // we are NOT in the UI thread. We execute the command now:
                        shape->getSingleMesh()->setEdgeThresholdAngle(parameter);
                    }
                    else
                    { // We are in the UI thread. Execute the command via the main thread:
                        SSimulationThreadCommand cmd;
                        cmd.cmdId=SET_SHAPE_EDGE_ANGLE_CMD;
                        cmd.intParams.push_back(shape->getObjectHandle());
                        cmd.floatParams.push_back(parameter);
                        App::appendSimulationThreadCommand(cmd);
                    }
                    retVal=1;
                }
                else
                    retVal=0;
            }
        }
        if (mirror!=nullptr)
        {
            if (parameterID==sim_mirrorfloatparam_width)
            {
                mirror->setMirrorWidth(parameter);
                retVal=1;
            }
            if (parameterID==sim_mirrorfloatparam_height)
            {
                mirror->setMirrorHeight(parameter);
                retVal=1;
            }
            if (parameterID==sim_mirrorfloatparam_reflectance)
            {
                mirror->setReflectance(parameter);
                retVal=1;
            }
        }
        if (pathPlanningObject!=nullptr)
        {
            if ((parameterID>=sim_pplanfloatparam_x_min)&&(parameterID<=sim_pplanfloatparam_delta_range))
            {
                float sMin[4];
                float sRange[4];
                pathPlanningObject->getSearchRange(sMin,sRange);
                if (parameterID==sim_pplanfloatparam_x_min)
                    sMin[0]=parameter;
                if (parameterID==sim_pplanfloatparam_x_range)
                    sRange[0]=parameter;
                if (parameterID==sim_pplanfloatparam_y_min)
                    sMin[1]=parameter;
                if (parameterID==sim_pplanfloatparam_y_range)
                    sRange[1]=parameter;
                if (parameterID==sim_pplanfloatparam_z_min)
                    sMin[2]=parameter;
                if (parameterID==sim_pplanfloatparam_z_range)
                    sRange[2]=parameter;
                if (parameterID==sim_pplanfloatparam_delta_min)
                    sMin[3]=parameter;
                if (parameterID==sim_pplanfloatparam_delta_range)
                    sRange[3]=parameter;
                pathPlanningObject->setSearchRange(sMin,sRange);
                retVal=1;
            }
        }
        if (dummy!=nullptr)
        {
            if (parameterID==sim_dummyfloatparam_follow_path_offset)
            {
                dummy->setVirtualDistanceOffsetOnPath(parameter);
                retVal=1;
            }
        }
        if (dummy!=nullptr)
        {
            if (parameterID==sim_dummyfloatparam_size)
            {
                dummy->setDummySize(parameter);
                retVal=1;
            }
        }
        if (camera!=nullptr)
        {
            if (parameterID==sim_camerafloatparam_near_clipping)
            {
                camera->setNearClippingPlane(parameter);
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_far_clipping)
            {
                camera->setFarClippingPlane(parameter);
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_perspective_angle)
            {
                camera->setViewAngle(parameter);
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_ortho_size)
            {
                camera->setOrthoViewSize(parameter);
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_pov_blur_distance)
            {
                std::string extensionString(camera->getExtensionString());
                tt::insertKeyAndValue("blurDist@povray",tt::FNb(0,parameter,3,false).c_str(),extensionString);
                camera->setExtensionString(extensionString.c_str());
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_pov_aperture)
            {
                std::string extensionString(camera->getExtensionString());
                tt::insertKeyAndValue("aperture@povray",tt::FNb(0,parameter,3,false).c_str(),extensionString);
                camera->setExtensionString(extensionString.c_str());
                retVal=1;
            }
        }
        if (retVal==0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_SET_PARAMETER);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetObjectStringParam_internal(simInt objectHandle,simInt parameterID,simInt* parameterLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    if (!doesObjectExist(__func__,objectHandle))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);
        simChar* retVal=nullptr; // Means the parameter was not retrieved
        parameterLength[0]=0;
        CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        if (parameterID==sim_objstringparam_dna)
        {
            std::string s(object->getDnaString());
            retVal=new char[s.length()+1];
            for (size_t i=0;i<s.length();i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            parameterLength[0]=(int)s.length();
        }
        if (parameterID==sim_objstringparam_unique_id)
        {
            std::string s(object->getUniquePersistentIdString());
            retVal=new char[s.length()+1];
            for (size_t i=0;i<s.length();i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            parameterLength[0]=(int)s.length();
        }

        if (rendSens!=nullptr)
        {

        }
        if (shape!=nullptr)
        {
            if (parameterID==sim_shapestringparam_color_name)
            {
                std::string colorNames;
                shape->getMeshWrapper()->getColorStrings(colorNames);
                if (colorNames.length()>0)
                {
                    retVal=new char[colorNames.length()+1];
                    for (unsigned int i=0;i<colorNames.length();i++)
                        retVal[i]=colorNames[i];
                    retVal[colorNames.length()]=0;
                    parameterLength[0]=(int)colorNames.length();
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectStringParam_internal(simInt objectHandle,simInt parameterID,const simChar* parameter,simInt parameterLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        int retVal=0; // Means the parameter was not set


        CVisionSensor* rendSens=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        if (rendSens!=nullptr)
        {

        }
        if (shape!=nullptr)
        {
            if (parameterID==sim_shapestringparam_color_name)
            {
                if (!shape->isCompound())
                {
                    if ( (parameter==nullptr)||(parameterLength<1) )
                        shape->getSingleMesh()->color.setColorName("");
                    else
                        shape->getSingleMesh()->color.setColorName(parameter);
                    retVal=1;
                }
            }
        }
        if (retVal==0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_SET_PARAMETER);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptInt32Param_internal(simInt scriptHandle,simInt parameterID,simInt* parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (parameterID==sim_scriptintparam_execorder)
        {
            parameter[0]=it->getExecutionPriority();
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_execcount)
        {
            parameter[0]=it->getNumberOfPasses();
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_type)
        {
            parameter[0]=it->getScriptType();
            if (it->getThreadedExecution_oldThreads())
                parameter[0]|=sim_scripttype_threaded_old;
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_handle)
        {
            parameter[0]=it->getScriptHandle();
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_objecthandle)
        {
            parameter[0]=it->getObjectHandleThatScriptIsAttachedTo(-1);
            retVal=1;
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetScriptInt32Param_internal(simInt scriptHandle,simInt parameterID,simInt parameter)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (parameterID==sim_scriptintparam_execorder)
        {
            it->setExecutionPriority(parameter);
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_execcount)
        {
            it->setNumberOfPasses(parameter);
            retVal=1;
        }
        if (parameterID==sim_scriptintparam_enabled)//&&(it->getScriptType()==sim_scripttype_childscript) )
        {
            it->setScriptIsDisabled(parameter==0);
            retVal=1;
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetScriptStringParam_internal(simInt scriptHandle,simInt parameterID,simInt* parameterLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        char* retVal=nullptr;
        if (parameterID==sim_scriptstringparam_name)
        {
            std::string s(it->getScriptName());
            retVal=new char[s.length()+1];
            for (size_t i=0;i<s.length();i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            parameterLength[0]=(int)s.length();
        }
        if (parameterID==sim_scriptstringparam_description)
        {
            std::string s(it->getDescriptiveName());
            retVal=new char[s.length()+1];
            for (size_t i=0;i<s.length();i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            parameterLength[0]=(int)s.length();
        }
        if (parameterID==sim_scriptstringparam_text)
        {
            std::string s(it->getScriptText());
            retVal=new char[s.length()+1];
            for (size_t i=0;i<s.length();i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            parameterLength[0]=(int)s.length();
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetScriptStringParam_internal(simInt scriptHandle,simInt parameterID,const simChar* parameter,simInt parameterLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (parameterID==sim_scriptstringparam_text)
        {
            std::string s(parameter);
            if (s.size()<parameterLength)
                s.assign(parameter,parameter+parameterLength);
            it->setScriptText(s.c_str());
            retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetRotationAxis_internal(const simFloat* matrixStart,const simFloat* matrixGoal,simFloat* axis,simFloat* angle)
{
    TRACE_C_API;

    C4X4Matrix mStart;
    mStart.copyFromInterface(matrixStart);
    C4X4Matrix mGoal;
    mGoal.copyFromInterface(matrixGoal);

    // Following few lines taken from the quaternion interpolation part:
    C4Vector AA(mStart.M.getQuaternion());
    C4Vector BB(mGoal.M.getQuaternion());
    if (AA(0)*BB(0)+AA(1)*BB(1)+AA(2)*BB(2)+AA(3)*BB(3)<0.0f)
        AA=AA*-1.0f;
    C4Vector r((AA.getInverse()*BB).getAngleAndAxis());

    C3Vector v(r(1),r(2),r(3));
    v=AA*v;

    axis[0]=v(0);
    axis[1]=v(1);
    axis[2]=v(2);
    float l=sqrt(v(0)*v(0)+v(1)*v(1)+v(2)*v(2));
    if (l!=0.0f)
    {
        axis[0]/=l;
        axis[1]/=l;
        axis[2]/=l;
    }
    angle[0]=r(0);

    return(1);
}

simInt simRotateAroundAxis_internal(const simFloat* matrixIn,const simFloat* axis,const simFloat* axisPos,simFloat angle,simFloat* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn;
    mIn.copyFromInterface(matrixIn);
    C7Vector m(mIn);
    C3Vector ax(axis);
    C3Vector pos(axisPos);

    float alpha=-atan2(ax(1),ax(0));
    float beta=atan2(-sqrt(ax(0)*ax(0)+ax(1)*ax(1)),ax(2));
    m.X-=pos;
    C7Vector r;
    r.X.clear();
    r.Q.setEulerAngles(0.0f,0.0f,alpha);
    m=r*m;
    r.Q.setEulerAngles(0.0f,beta,0.0f);
    m=r*m;
    r.Q.setEulerAngles(0.0f,0.0f,angle);
    m=r*m;
    r.Q.setEulerAngles(0.0f,-beta,0.0f);
    m=r*m;
    r.Q.setEulerAngles(0.0f,0.0f,-alpha);
    m=r*m;
    m.X+=pos;
    m.getMatrix().copyToInterface(matrixOut);

    return(1);
}

simInt simGetJointForce_internal(simInt jointHandle,simFloat* forceOrTorque)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=jointHandle;
        int handleFlags=0;
        if (jointHandle>=0)
        {
            handleFlags=jointHandle&0x0ff00000;
            handle=jointHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        if (!isJoint(__func__,handle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(handle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        float f;
        forceOrTorque[0]=0.0f;
        if (it->getDynamicForceOrTorque(f,(handleFlags&sim_handleflag_rawvalue)!=0))
        {
            forceOrTorque[0]=f;
            return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointTargetForce_internal(simInt jointHandle,simFloat* forceOrTorque)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,jointHandle))
            return(-1);
        if (!isJoint(__func__,jointHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        forceOrTorque[0]=it->getTargetForce(true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointTargetForce_internal(simInt objectHandle,simFloat forceOrTorque,simBool signedValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        it->setTargetForce(forceOrTorque,signedValue);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPersistentDataWrite_internal(const simChar* dataTag,const simChar* dataValue,simInt dataLength,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::worldContainer->persistentDataContainer->writeData(dataTag,std::string(dataValue,dataLength),(options&1)!=0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simPersistentDataRead_internal(const simChar* dataTag,simInt* dataLength)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string sigVal;
        if (App::worldContainer->persistentDataContainer->readData(dataTag,sigVal))
        {
            char* retVal=new char[sigVal.length()];
            for (unsigned int i=0;i<sigVal.length();i++)
                retVal[i]=sigVal[i];
            dataLength[0]=(int)sigVal.length();
            return(retVal);
        }
        return(nullptr); // data does not exist
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simIsHandle_internal(simInt generalObjectHandle,simInt generalObjectType)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_object_type))&&(App::currentWorld->sceneObjects->getObjectFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_collection_type))&&(App::currentWorld->collections->getObjectFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_script_type))&&(App::worldContainer->getScriptFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_texture_type))&&(App::currentWorld->textureContainer->getObject(generalObjectHandle)!=nullptr))
            return(1);

        // Old:
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_ui_type))&&(App::currentWorld->buttonBlockContainer->getBlockWithID(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_collision_type))&&(App::currentWorld->collisions->getObjectFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_distance_type))&&(App::currentWorld->distances->getObjectFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_ik_type))&&(App::currentWorld->ikGroups->getObjectFromHandle(generalObjectHandle)!=nullptr))
            return(1);
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_pathplanning_type))&&(App::currentWorld->pathPlanning->getObject(generalObjectHandle)!=nullptr))
            return(1);
        return(0); // handle is not valid!
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleVisionSensor_internal(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (visionSensorHandle!=sim_handle_all)&&(visionSensorHandle!=sim_handle_all_except_explicit) )
        {
            if (!isVisionSensor(__func__,visionSensorHandle))
                return(-1);
        }
        if (auxValues!=nullptr)
            auxValues[0]=nullptr;
        if (auxValuesCount!=nullptr)
            auxValuesCount[0]=nullptr;
        int retVal=0;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getVisionSensorCount();i++)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromIndex(i);
            if (visionSensorHandle>=0)
            { // explicit handling
                it=(CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                retVal=it->handleSensor();
                if ( (auxValues!=nullptr)&&(auxValuesCount!=nullptr) )
                {
                    auxValuesCount[0]=new int[1+int(it->sensorAuxiliaryResult.size())];
                    auxValuesCount[0][0]=int(it->sensorAuxiliaryResult.size());
                    int fvs=0;
                    for (size_t j=0;j<it->sensorAuxiliaryResult.size();j++)
                        fvs+=(int)it->sensorAuxiliaryResult[j].size();
                    auxValues[0]=new float[fvs];
                    fvs=0;
                    for (size_t j=0;j<it->sensorAuxiliaryResult.size();j++)
                    {
                        auxValuesCount[0][j+1]=int(it->sensorAuxiliaryResult[j].size());
                        for (size_t k=0;k<it->sensorAuxiliaryResult[j].size();k++)
                            auxValues[0][fvs++]=it->sensorAuxiliaryResult[j][k];
                    }
                }
            }
            else
            {
                if ( (!it->getExplicitHandling())||(visionSensorHandle==sim_handle_all) )
                    retVal+=it->handleSensor();
            }
            if (visionSensorHandle>=0)
                break;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadVisionSensor_internal(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isVisionSensor(__func__,visionSensorHandle))
            return(-1);
        if (auxValues!=nullptr)
            auxValues[0]=nullptr;
        if (auxValuesCount!=nullptr)
            auxValuesCount[0]=nullptr;
        CVisionSensor* it=(CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
        int retVal=-1;
        if (it->sensorResult.sensorResultIsValid)
        {
            if (it->sensorResult.sensorWasTriggered)
                retVal=1;
            else
                retVal=0;
            if ( (auxValues!=nullptr)&&(auxValuesCount!=nullptr) )
            {
                auxValuesCount[0]=new int[1+int(it->sensorAuxiliaryResult.size())];
                auxValuesCount[0][0]=int(it->sensorAuxiliaryResult.size());
                int fvs=0;
                for (int j=0;j<int(it->sensorAuxiliaryResult.size());j++)
                    fvs+=(int)it->sensorAuxiliaryResult[j].size();
                auxValues[0]=new float[fvs];
                fvs=0;
                for (int j=0;j<int(it->sensorAuxiliaryResult.size());j++)
                {
                    auxValuesCount[0][j+1]=int(it->sensorAuxiliaryResult[j].size());
                    for (int k=0;k<int(it->sensorAuxiliaryResult[j].size());k++)
                        auxValues[0][fvs++]=it->sensorAuxiliaryResult[j][k];
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetVisionSensor_internal(simInt visionSensorHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (visionSensorHandle!=sim_handle_all)&&(visionSensorHandle!=sim_handle_all_except_explicit) )
        {
            if (!isVisionSensor(__func__,visionSensorHandle))
            {
                return(-1);
            }
        }
        for (size_t i=0;i<App::currentWorld->sceneObjects->getVisionSensorCount();i++)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromIndex(i);
            if (visionSensorHandle>=0)
            { // Explicit handling
                it=(CVisionSensor*)App::currentWorld->sceneObjects->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                it->resetSensor();
                break;
            }
            else
            {
                if ( (!it->getExplicitHandling())||(visionSensorHandle==sim_handle_all) )
                    it->resetSensor();
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simCheckVisionSensor_internal(simInt sensorHandle,simInt entityHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(-1);
        if (!isVisionSensor(__func__,sensorHandle))
            return(-1);
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
            return(-1);
        if (entityHandle==sim_handle_all)
            entityHandle=-1;

        if (!App::currentWorld->mainSettings->visionSensorsEnabled)
            return(0);

        if (auxValues!=nullptr)
            auxValues[0]=nullptr;
        if (auxValuesCount!=nullptr)
            auxValuesCount[0]=nullptr;
        int retVal=0;

        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        if (it->checkSensor(entityHandle,true))
            retVal=1;

        if ( (auxValues!=nullptr)&&(auxValuesCount!=nullptr) )
        {
            auxValuesCount[0]=new int[1+int(it->sensorAuxiliaryResult.size())];
            auxValuesCount[0][0]=int(it->sensorAuxiliaryResult.size());
            int fvs=0;
            for (int j=0;j<int(it->sensorAuxiliaryResult.size());j++)
                fvs+=(int)it->sensorAuxiliaryResult[j].size();
            auxValues[0]=new float[fvs];
            fvs=0;
            for (int j=0;j<int(it->sensorAuxiliaryResult.size());j++)
            {
                auxValuesCount[0][j+1]=int(it->sensorAuxiliaryResult[j].size());
                for (int k=0;k<int(it->sensorAuxiliaryResult[j].size());k++)
                    auxValues[0][fvs++]=it->sensorAuxiliaryResult[j][k];
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simCheckVisionSensorEx_internal(simInt sensorHandle,simInt entityHandle,simBool returnImage)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
            return(nullptr);
        if (entityHandle==sim_handle_all)
            entityHandle=-1;

        if (!App::currentWorld->mainSettings->visionSensorsEnabled)
            return(nullptr);

        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        float* retBuffer=it->checkSensorEx(entityHandle,returnImage!=0,false,false,true);
        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}


simUChar* simGetVisionSensorImg_internal(simInt sensorHandle,simInt options,simFloat rgbaCutOff,const simInt* pos,const simInt* size,simInt* resolution)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolution!=nullptr)
        {
            resolution[0]=res[0];
            resolution[1]=res[1];
        }
        int posX=0;
        int posY=0;
        if (pos!=nullptr)
        {
            posX=pos[0];
            posY=pos[1];
        }
        int sizeX=res[0];
        int sizeY=res[1];
        if ( (size!=nullptr)&&(size[0]>0) )
        {
            sizeX=size[0];
            sizeY=size[1];
        }
        unsigned char* img=it->readPortionOfCharImage(posX,posY,sizeX,sizeY,rgbaCutOff,options);
        if (img==nullptr)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        return(img);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetVisionSensorImg_internal(simInt sensorHandle,const simUChar* img,simInt options,const simInt* pos,const simInt* size)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(-1);
        if (!isVisionSensor(__func__,sensorHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        int posX=0;
        int posY=0;
        if (pos!=nullptr)
        {
            posX=pos[0];
            posY=pos[1];
        }
        int sizeX=res[0];
        int sizeY=res[1];
        if ( (size!=nullptr)&&(size[0]>0) )
        {
            sizeX=size[0];
            sizeY=size[1];
        }
        if (!it->writePortionOfCharImage(img,posX,posY,sizeX,sizeY,options))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
            return(-1);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat* simGetVisionSensorDepth_internal(simInt sensorHandle,simInt options,const simInt* pos,const simInt* size,simInt* resolution)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolution!=nullptr)
        {
            resolution[0]=res[0];
            resolution[1]=res[1];
        }
        int posX=0;
        int posY=0;
        if (pos!=nullptr)
        {
            posX=pos[0];
            posY=pos[1];
        }
        int sizeX=res[0];
        int sizeY=res[1];
        if ( (size!=nullptr)&&(size[0]>0) )
        {
            sizeX=size[0];
            sizeY=size[1];
        }
        float* retBuff=it->readPortionOfImage(posX,posY,sizeX,sizeY,2);
        if ((options&1)!=0)
        {
            float n=it->getNearClippingPlane();
            float f=it->getFarClippingPlane();
            float fmn=f-n;
            for (int i=0;i<sizeX*sizeY;i++)
                retBuff[i]=n+fmn*retBuff[i];
        }
        return(retBuff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simRuckigPos_internal(simInt dofs,simDouble baseCycleTime,simInt flags,const simDouble* currentPos,const simDouble* currentVel,const simDouble* currentAccel,const simDouble* maxVel,const simDouble* maxAccel,const simDouble* maxJerk,const simBool* selection,const simDouble* targetPos,const simDouble* targetVel,simDouble* reserved1,simInt* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=CPluginContainer::ruckigPlugin_pos(_currentScriptHandle,dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxVel,maxAccel,maxJerk,selection,targetPos,targetVel);
        if (retVal==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRuckigVel_internal(simInt dofs,simDouble baseCycleTime,simInt flags,const simDouble* currentPos,const simDouble* currentVel,const simDouble* currentAccel,const simDouble* maxAccel,const simDouble* maxJerk,const simBool* selection,const simDouble* targetVel,simDouble* reserved1,simInt* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=CPluginContainer::ruckigPlugin_vel(_currentScriptHandle,dofs,baseCycleTime,flags,currentPos,currentVel,currentAccel,maxAccel,maxJerk,selection,targetVel);
        if (retVal==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRuckigStep_internal(simInt objHandle,simDouble cycleTime,simDouble* newPos,simDouble* newVel,simDouble* newAccel,simDouble* syncTime,simDouble* reserved1,simInt* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=CPluginContainer::ruckigPlugin_step(objHandle,cycleTime,newPos,newVel,newAccel,syncTime);
        if (retVal==-3)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_RUCKIG_CYCLETIME_ERROR);
        if (retVal==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_RUCKIG_OBJECT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRuckigRemove_internal(simInt objHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=CPluginContainer::ruckigPlugin_remove(objHandle);
        if (retVal==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_FIND_RUCKIG);
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_RUCKIG_OBJECT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectQuaternion_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* quaternion)
{ 
    TRACE_C_API;
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces (default): x y z w

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;

        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformation();
        else
        {
            if ((handleFlags&sim_handleflag_reljointbaseframe)!=0)
            {
                C7Vector relTr(relObj->getCumulativeTransformation());
                tr=relTr.getInverse()*it->getCumulativeTransformation();
            }
            else
            {
                if (it->getParent()==relObj)
                    tr=it->getLocalTransformation(); // in case of a series get/set, not to lose precision
                else
                {
                    C7Vector relTr(relObj->getFullCumulativeTransformation());
                    tr=relTr.getInverse()*it->getCumulativeTransformation();
                }
            }
        }
        if ((handleFlags&sim_handleflag_wxyzquaternion)!=0)
            tr.Q.getInternalData(quaternion);
        else
        {
            quaternion[0]=tr.Q(1);
            quaternion[1]=tr.Q(2);
            quaternion[2]=tr.Q(3);
            quaternion[3]=tr.Q(0);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectQuaternion_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* quaternion)
{
    TRACE_C_API;
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces (default): x y z w

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;

        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=sim_handle_world;
            CSceneObject* parent=it->getParent();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=sim_handle_world)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getDynamicFlag()>1) // for non-static shapes, and other objects that are in the dyn. world
            it->setDynamicsResetFlag(true,true);
        CSceneObject* relObj=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToObjectHandle);
        if (relObj==nullptr)
        {
            C4Vector q;
            if ((handleFlags&sim_handleflag_wxyzquaternion)!=0)
                q.setInternalData(quaternion);
            else
            {
                q(0)=quaternion[3];
                q(1)=quaternion[0];
                q(2)=quaternion[1];
                q(3)=quaternion[2];
            }
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),q.getEulerAngles());
        }
        else
        {
            if ( (it->getParent()==relObj)&&((handleFlags&sim_handleflag_reljointbaseframe)==0) )
            { // special here, in order to not lose precision in a series of get/set
                C7Vector tr(it->getLocalTransformation());
                if ((handleFlags&sim_handleflag_wxyzquaternion)!=0)
                    tr.Q.setInternalData(quaternion);
                else
                {
                    tr.Q(0)=quaternion[3];
                    tr.Q(1)=quaternion[0];
                    tr.Q(2)=quaternion[1];
                    tr.Q(3)=quaternion[2];
                }
                it->setLocalTransformation(tr);
            }
            else
            {
                C7Vector absTr(it->getCumulativeTransformation());
                C7Vector relTr;
                if ( (handleFlags&sim_handleflag_reljointbaseframe)!=0)
                    relTr=relObj->getCumulativeTransformation();
                else
                    relTr=relObj->getFullCumulativeTransformation();
                C7Vector x(relTr.getInverse()*absTr);
                if ((handleFlags&sim_handleflag_wxyzquaternion)!=0)
                    x.Q.setInternalData(quaternion);
                else
                {
                    x.Q(0)=quaternion[3];
                    x.Q(1)=quaternion[0];
                    x.Q(2)=quaternion[1];
                    x.Q(3)=quaternion[2];
                }
                absTr=relTr*x;
                App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),absTr.Q.getEulerAngles());
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeMass_internal(simInt shapeHandle,simFloat* mass)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        mass[0]=it->getMeshWrapper()->getMass();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetShapeMass_internal(simInt shapeHandle,simFloat mass)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        if (mass<0.0000001f)
            mass=0.0000001f;
        it->getMeshWrapper()->setMass(mass);
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeInertia_internal(simInt shapeHandle,simFloat* inertiaMatrix,simFloat* transformationMatrix)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        C4X4Matrix tr(it->getMeshWrapper()->getLocalInertiaFrame());
        tr.copyToInterface(transformationMatrix);

        C3X3Matrix m;
        m.clear();
        m.axis[0](0)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(0);
        m.axis[1](1)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(1);
        m.axis[2](2)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(2);
        m*=it->getMeshWrapper()->getMass(); // in CoppeliaSim we work with the "massless inertia"
        m.copyToInterface(inertiaMatrix);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetShapeInertia_internal(simInt shapeHandle,const simFloat* inertiaMatrix,const simFloat* transformationMatrix)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        C4X4Matrix tr;
        tr.copyFromInterface(transformationMatrix);

        C3X3Matrix m;
        m.copyFromInterface(inertiaMatrix);
        m.axis[0](1)=m.axis[1](0);
        m.axis[0](2)=m.axis[2](0);
        m.axis[1](2)=m.axis[2](1);
        m/=it->getMeshWrapper()->getMass(); // in CoppeliaSim we work with the "massless inertia"

        C7Vector corr;
        corr.setIdentity();
        C3Vector pmoment;
        CMeshWrapper::findPrincipalMomentOfInertia(m,corr.Q,pmoment);

        if (pmoment(0)<0.0000001f)
            pmoment(0)=0.0000001f;
        if (pmoment(1)<0.0000001f)
            pmoment(1)=0.0000001f;
        if (pmoment(2)<0.0000001f)
            pmoment(0)=0.0000001f;
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
        it->getMeshWrapper()->setLocalInertiaFrame(tr.getTransformation()*corr);
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simIsDynamicallyEnabled_internal(simInt objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int retVal=0;
        if (it->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)it;
            if ( (joint->getJointMode()==sim_jointmode_dynamic)&&(it->getDynamicSimulationIconCode()==sim_dynamicsimicon_objectisdynamicallysimulated) )
                retVal=1; // we do not consider a joint dyn. enabled when in deprecated hybrid mode
        }
        /*
        if (it->getObjectType()==sim_object_shape_type)
        {
            if (((CShape*)it)->xxxxx())
                retVal=1;
        }
        if (it->getObjectType()==sim_object_joint_type)
        {
            float dummyVal;
            if (((CJoint*)it)->getDynamicForceOrTorque(dummyVal,false))
                retVal=1;
        }
        if (it->getObjectType()==sim_object_forcesensor_type)
        {
            C3Vector dummyVal;
            if (((CForceSensor*)it)->getDynamicForces(dummyVal,false))
                retVal=1;
        }
        */
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGenerateShapeFromPath_internal(const simFloat* pppath,simInt pathSize,const simFloat* section,simInt sectionSize,simInt options,const simFloat* upVector,simFloat reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        // First make sure the points are not coincident:
        std::vector<float> ppath;
        C3Vector prevV;
        prevV.clear();
        C4Vector prevQ;
        prevQ.clear();
        for (int i=0;i<pathSize/7;i++)
        {
            C3Vector v(pppath+7*i);
            C4Vector q(pppath+7*i+3,true);
            float d=(prevV-v).getLength();
            if ( (d>=0.0005f)||(i==0) )
            {
                prevV=v;
                prevQ=q;
                for (size_t j=0;j<3;j++)
                    ppath.push_back(v(j));
                for (size_t j=0;j<4;j++)
                    ppath.push_back(q(j));
            }
        }
        pathSize=int(ppath.size());
        if (pathSize>=7*2)
        {
            C3Vector zvect;
            if (upVector!=nullptr)
                zvect.set(upVector);
            else
                zvect=C3Vector::unitZVector;
            bool closedPath=(options&4)!=0;
            int axis=options&3;

            size_t confCnt=size_t(pathSize)/7;
            size_t elementCount=confCnt;
            size_t secVertCnt=size_t(sectionSize)/2;
            std::vector<float> path;
            for (size_t i=0;i<confCnt;i++)
            {
                C3Vector p0,p1,p2;
                if (i!=0)
                    p0=C3Vector(&ppath[0]+7*(i-1));
                else
                {
                    if (closedPath)
                        p0=C3Vector(&ppath[0]+pathSize-7);
                }
                p1=C3Vector(&ppath[0]+7*i);
                C4Vector q(&ppath[0]+7*i+3,false); // Quaternion notation was changed above!
                if (axis!=0)
                    zvect=q.getAxis(axis-1);
                if (i!=(confCnt-1))
                    p2=C3Vector(&ppath[0]+7*(i+1));
                else
                {
                    if (closedPath)
                        p2=C3Vector(&ppath[0]+7*1);
                }
                C3Vector vy;
                if ( closedPath||((i!=0)&&(i!=(confCnt-1))) )
                    vy=(p1-p0)+(p2-p1);
                else
                {
                    if (i==0)
                        vy=(p2-p1);
                    else
                        vy=(p1-p0);
                }
                vy.normalize();
                C3Vector vx=vy^zvect;
                vx.normalize();
                C4X4Matrix m;
                m.X=p1;
                m.M.axis[0]=vx;
                m.M.axis[1]=vy;
                m.M.axis[2]=vx^vy;
                C7Vector p(m.getTransformation());
                for (size_t j=0;j<7;j++)
                    path.push_back(p(j));
            }

            bool sectionClosed=( (section[0]==section[sectionSize-2])&&(section[1]==section[sectionSize-1]) );
            if (sectionClosed)
                secVertCnt--;

            std::vector<float> vertices;
            std::vector<int> indices;
            C7Vector tr0;
            tr0.setInternalData(&path[0]);
            for (size_t i=0;i<=secVertCnt-1;i++)
            {
                C3Vector v(section[i*2+0],0.0f,section[i*2+1]);
                v=tr0*v;
                vertices.push_back(v(0));
                vertices.push_back(v(1));
                vertices.push_back(v(2));
            }

            int previousVerticesOffset=0;
            for (size_t ec=1;ec<elementCount;ec++)
            {
                C7Vector tr;
                tr.setInternalData(&path[ec*7]);
                int forwOff=int(secVertCnt);
                for (int i=0;i<=int(secVertCnt)-1;i++)
                {
                    C3Vector v(section[i*2+0],0.0f,section[i*2+1]);
                    if ( closedPath&&(ec==(elementCount-1)) )
                        forwOff=-previousVerticesOffset;
                    else
                    {
                        v=tr*v;
                        vertices.push_back(v(0));
                        vertices.push_back(v(1));
                        vertices.push_back(v(2));
                    }
                    if (i!=int(secVertCnt-1))
                    {
                        indices.push_back(previousVerticesOffset+0+i);
                        indices.push_back(previousVerticesOffset+forwOff+i);
                        indices.push_back(previousVerticesOffset+1+i);
                        indices.push_back(previousVerticesOffset+1+i);
                        indices.push_back(previousVerticesOffset+forwOff+i);
                        indices.push_back(previousVerticesOffset+forwOff+i+1);
                    }
                    else
                    {
                        if (sectionClosed)
                        {
                            indices.push_back(previousVerticesOffset+0+i);
                            indices.push_back(previousVerticesOffset+forwOff+i);
                            indices.push_back(previousVerticesOffset+0);
                            indices.push_back(previousVerticesOffset+0);
                            indices.push_back(previousVerticesOffset+forwOff+i);
                            indices.push_back(previousVerticesOffset+forwOff+0);
                        }
                    }
                }
                previousVerticesOffset+=int(secVertCnt);
            }
            int h=simCreateMeshShape_internal(0,0.0f,&vertices[0],int(vertices.size()),&indices[0],int(indices.size()),nullptr);
            return(h);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PATH);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat simGetClosestPosOnPath_internal(const simFloat* path,simInt pathSize,const simFloat* pathLengths,const simFloat* absPt)
{
    TRACE_C_API;
    float retVal=0.0f;

    if (pathSize>=6)
    {
        float d=SIM_MAX_FLOAT;
        C3Vector pppt(absPt);
        for (int i=0;i<(pathSize/3)-1;i++)
        {
            C3Vector v0(path+i*3);
            C3Vector v1(path+(i+1)*3);
            C3Vector vd(v1-v0);
            C3Vector theSearchedPt;
            if (CMeshRoutines::getMinDistBetweenSegmentAndPoint_IfSmaller(v0,vd,pppt,d,theSearchedPt))
            {
                float vdL=vd.getLength();
                if (vdL==0.0f)
                    retVal=pathLengths[i]; // // Coinciding points
                else
                {
                    float l=(theSearchedPt-v0).getLength();
                    float c=l/vdL;
                    retVal=pathLengths[i]*(1.0f-c)+pathLengths[i+1]*c;
                }
            }
        }
    }
    return(retVal);
}

simInt simInitScript_internal(simInt scriptHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it!=nullptr)
        {
            if ( (it->getScriptType()==sim_scripttype_childscript)||(it->getScriptType()==sim_scripttype_customizationscript)||(it->getScriptType()==sim_scripttype_sandboxscript) )
            {
                it->resetScript();
                if (it->initScript())
                    return(1);
                return(0);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_SCRIPT_TYPE);
                return(-1);
            }
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simModuleEntry_internal(simInt handle,const simChar* label,simInt state)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool first=false;
        if (handle==-1)
        {
            first=true;
            handle=App::worldContainer->moduleMenuItemContainer->addMenuItem(label,_currentScriptHandle);
        }
        CModuleMenuItem* item=App::worldContainer->moduleMenuItemContainer->getItemFromHandle(handle);
        if (item!=nullptr)
        {
            if (state==-2)
                App::worldContainer->moduleMenuItemContainer->removeMenuItem(handle);
            else
            {
                if ( (!first)&&(label!=nullptr) )
                    item->setLabel(label);
                if (state!=-1)
                    item->setState(state);
            }
            return(handle);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCheckExecAuthorization_internal(const simChar* what,const simChar* args,simInt scriptHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=0;
        if (App::userSettings->executeUnsafe)
            retVal=1;
        else
        {
            bool auth=false;
            int h=scriptHandle;
            if (h<0)
                h=CScriptObject::getInExternalCall();
            CScriptObject* it=nullptr;
            std::string x,y;
            if (h>=0)
            {
                it=App::worldContainer->getScriptFromHandle(h);
                if (it!=nullptr)
                {
                    x=x+args+" ";
                    x=std::regex_replace(x,std::regex(" ([0-9]+) ")," X ");
                    x=std::regex_replace(x,std::regex("([0-9]{5}) "),"X ");
                    x=std::regex_replace(x,std::regex(" "),"_");
                    y=x+std::to_string(it->getSimpleHash());
                    std::hash<std::string> hasher;
                    y=std::to_string(hasher(y))+"EXECUNSAFE";
                    CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
                    std::string val;
                    if (cont.readData(y.c_str(),val))
                        auth=true;
                }
            }
#ifdef SIM_WITH_GUI
            if ( (!auth)&&(App::mainWindow!=nullptr) )
            {
                if (App::uiThread->checkExecuteUnsafeOk(what,args,x.c_str()))
                {
                    auth=true;
                    if (it!=nullptr)
                    {
                        CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
                        cont.writeData(y.c_str(),"OK",true);
                    }
                }
            }
#endif
            if (auth)
                retVal=1;
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_EXEC_UNSAFE_FAILED);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGroupShapes_internal(const simInt* shapeHandles,simInt shapeCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> shapes;
        bool merging=(shapeCount<0);
        if (shapeCount<0)
            shapeCount=-shapeCount;
        for (int i=0;i<shapeCount;i++)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandles[i]);
            if (it!=nullptr)
                shapes.push_back(it->getObjectHandle());
        }
        if (shapes.size()<2)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_NOT_ENOUGH_SHAPES);
            return(-1);
        }
        const std::vector<int> initSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
        int retVal;
        if (merging)
            retVal=CSceneObjectOperations::mergeSelection(&shapes,false);
        else
            retVal=CSceneObjectOperations::groupSelection(&shapes,false);
        App::currentWorld->sceneObjects->setSelectedObjectHandles(&initSelection);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt* simUngroupShape_internal(simInt shapeHandle,simInt* shapeCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        shapeCount[0]=0;
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        bool dividing=false;
        if (shapeHandle<=-2)
        {
            shapeHandle=-shapeHandle-2;
            dividing=true;
        }

        if (!isShape(__func__,shapeHandle))
        {
            shapeCount[0]=0;
            return(nullptr);
        }
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        if (it->getMeshWrapper()->isMesh())
        {
            if (dividing)
            {
                std::vector<int> finalSel;
                std::vector<int> previousSel;
                std::vector<int> sel;
                previousSel.push_back(shapeHandle);
                sel.push_back(shapeHandle);
                CSceneObjectOperations::divideSelection(&sel,false);
                for (size_t j=0;j<sel.size();j++)
                {
                    if (sel[j]!=shapeHandle)
                        finalSel.push_back(sel[j]);
                }
                finalSel.push_back(shapeHandle); // the original shape is added at the very end for correct ordering

                int* retVal=new int[finalSel.size()];
                for (int i=0;i<int(finalSel.size());i++)
                    retVal[i]=finalSel[i];
                shapeCount[0]=int(finalSel.size());
                return(retVal);
            }
            else
            {
                shapeCount[0]=1;
                int* retVal=new int[1];
                retVal[0]=shapeHandle;
                return(retVal);
            }
        }
        else
        {
            if (dividing)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE);
                shapeCount[0]=0;
                return(nullptr);
            }
        }
        std::vector<int> finalSel;
        std::vector<int> previousSel;
        std::vector<int> sel;
        previousSel.push_back(shapeHandle);
        sel.push_back(shapeHandle);
        while (sel.size()!=0)
        {
            CSceneObjectOperations::ungroupSelection(&sel,false);
            for (int i=0;i<int(previousSel.size());i++)
            {
                int previousID=previousSel[i];
                bool present=false;
                for (int j=0;j<int(sel.size());j++)
                {
                    if (sel[j]==previousID)
                    {
                        present=true;
                        break;
                    }
                }
                if ((!present)&&(shapeHandle!=previousID)) // the original shape will be added at the very end for correct ordering (see below)
                    finalSel.push_back(previousID); // this is a simple shape (not a group)
            }
            previousSel.assign(sel.begin(),sel.end());
        }
        finalSel.push_back(shapeHandle);
        int* retVal=new int[finalSel.size()];
        for (int i=0;i<int(finalSel.size());i++)
            retVal[i]=finalSel[i];
        shapeCount[0]=int(finalSel.size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simConvexDecompose_internal(simInt shapeHandle,simInt options,const simInt* intParams,const simFloat* floatParams)
{ // one shape at a time!
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    int retVal=CSceneObjectOperations::convexDecompose_apiVersion(shapeHandle,options,intParams,floatParams);
    return(retVal);
}

simVoid simQuitSimulator_internal(simBool ignoredArgument)
{
    TRACE_C_API;
    SSimulationThreadCommand cmd;
    cmd.cmdId=EXIT_REQUEST_CMD;
    App::appendSimulationThreadCommand(cmd);
}

simInt simSetShapeMaterial_internal(simInt shapeHandle,simInt materialIdOrShapeHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1; // means error
        if (isShape(__func__,shapeHandle))
        {
            // For backward compatibility (28/10/2016)
            // We now do not share materials anymore: each shape has its own material, so
            // the material of a shape is identified by the shape handle itself
            if (materialIdOrShapeHandle!=-1)
            {
                if (materialIdOrShapeHandle>=sim_dynmat_default)
                { // set a predefined material
                    CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                    shape->getDynMaterial()->generateDefaultMaterial(materialIdOrShapeHandle);
                }
                else
                { // set the same material as another shape
                    CShape* matShape=App::currentWorld->sceneObjects->getShapeFromHandle(materialIdOrShapeHandle);
                    if (matShape!=nullptr)
                    {
                        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
                        shape->setDynMaterial(matShape->getDynMaterial()->copyYourself());
                        retVal=1;
                    }
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetTextureId_internal(const simChar* textureName,simInt* resolution)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        CTextureObject* to=App::currentWorld->textureContainer->getObject(textureName);
        if (to!=nullptr)
        {
            retVal=to->getObjectID();
            if (resolution!=nullptr)
                to->getTextureSize(resolution[0],resolution[1]);
        }
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReadTexture_internal(simInt textureId,simInt options,simInt posX,simInt posY,simInt sizeX,simInt sizeY)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to=App::currentWorld->textureContainer->getObject(textureId);
        if (to!=nullptr)
        {
            int resX,resY;
            to->getTextureSize(resX,resY);
            if ( (posX>=0)&&(posY>=0)&&(sizeX>=0)&&(sizeY>=0)&&(posX+sizeX<=resX)&&(posY+sizeY<=resY) )
            {
                if (sizeX==0)
                {
                    posX=0;
                    sizeX=resX;
                }
                if (sizeY==0)
                {
                    posY=0;
                    sizeY=resY;
                }
                char* retVal=to->readPortionOfTexture(posX,posY,sizeX,sizeY);
                return(retVal);
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simWriteTexture_internal(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simFloat interpol)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to=App::currentWorld->textureContainer->getObject(textureId);
        if (to!=nullptr)
        {
            int resX,resY;
            to->getTextureSize(resX,resY);
            if ( (sizeX>=0)&&(sizeY>=0) )
            {
                if (sizeX==0)
                {
                    posX=0;
                    sizeX=resX;
                }
                if (sizeY==0)
                {
                    posY=0;
                    sizeY=resY;
                }
                int retVal=-1;
                if (to->writePortionOfTexture((unsigned char*)data,posX,posY,sizeX,sizeY,(options&4)!=0,interpol))
                    retVal=1;
                return(retVal);
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateTexture_internal(const simChar* fileName,simInt options,const simFloat* planeSizes,const simFloat* scalingUV,const simFloat* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (strlen(fileName)!=0)
        { // loading a texture
            if (VFile::doesFileExist(fileName))
            {
                int resX,resY,n;
                unsigned char* data=CImageLoaderSaver::load(fileName,&resX,&resY,&n,0,fixedResolution);
                bool rgba=(n==4);
                if (n<3)
                {
                    delete[] data;
                    data=nullptr;
                }
                if (data!=nullptr)
                {
                    C3Vector s(0.1f,0.1f,0.00001f);
                    if (planeSizes!=nullptr)
                        s=C3Vector(tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[1]),0.00001f);
                    CShape* shape=CAddOperations::addPrimitiveShape(sim_primitiveshape_plane,s);

                    C7Vector identity;
                    identity.setIdentity();
                    shape->setLocalTransformation(identity);
                    shape->setCulling(false);
                    shape->setVisibleEdges(false);
                    shape->setRespondable(false);
                    shape->setShapeIsDynamicallyStatic(true);
                    shape->getMeshWrapper()->setMass(1.0f);

                    if (resolution!=nullptr)
                    {
                        resolution[0]=resX;
                        resolution[1]=resY;
                    }

                    CTextureObject* textureObj=new CTextureObject(resX,resY);
                    textureObj->setImage(rgba,false,false,data); // keep false,false
                    textureObj->setObjectName(App::folders->getNameFromFull(fileName).c_str());
                    delete[] data;
                    textureObj->addDependentObject(shape->getObjectHandle(),shape->getSingleMesh()->getUniqueID());
                    int texID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                    CTextureProperty* tp=new CTextureProperty(texID);
                    shape->getSingleMesh()->setTextureProperty(tp);
                    tp->setInterpolateColors((options&1)==0);
                    if ((options&2)!=0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    tp->setRepeatU((options&4)!=0);
                    tp->setRepeatV((options&8)!=0);
                    if (scalingUV!=nullptr)
                        tp->setTextureScaling(scalingUV[0],scalingUV[1]);
                    else
                        tp->setTextureScaling(s(0),s(1));
                    if (xy_g!=nullptr)
                    {
                        C7Vector tr;
                        tr.setIdentity();
                        tr.X(0)=xy_g[0];
                        tr.X(1)=xy_g[1];
                        tr.Q=C4Vector(0.0f,0.0f,xy_g[2]);
                        tp->setTextureRelativeConfig(tr);
                    }
                    if (textureId!=nullptr)
                        textureId[0]=texID;
                    return(shape->getObjectHandle());
                }
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FILE_NOT_FOUND);
        }
        else
        { // just creating a texture (not loading it)
            if (resolution!=nullptr)
            {
                C3Vector s(0.1f,0.1f,0.00001f);
                if (planeSizes!=nullptr)
                    s=C3Vector(tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[1]),0.00001f);
                CShape* shape=CAddOperations::addPrimitiveShape(sim_primitiveshape_plane,s);
                C7Vector identity;
                identity.setIdentity();
                shape->setLocalTransformation(identity);
                shape->setCulling(false);
                shape->setVisibleEdges(false);
                shape->setRespondable(false);
                shape->setShapeIsDynamicallyStatic(true);
                shape->getMeshWrapper()->setMass(1.0f);

                CTextureObject* textureObj=new CTextureObject(resolution[0],resolution[1]);
                textureObj->setRandomContent();
                textureObj->setObjectName(App::folders->getNameFromFull(fileName).c_str());
                textureObj->addDependentObject(shape->getObjectHandle(),shape->getSingleMesh()->getUniqueID());
                int texID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                CTextureProperty* tp=new CTextureProperty(texID);
                shape->getSingleMesh()->setTextureProperty(tp);
                tp->setInterpolateColors((options&1)==0);
                if ((options&2)!=0)
                    tp->setApplyMode(1);
                else
                    tp->setApplyMode(0);
                tp->setRepeatU((options&4)!=0);
                tp->setRepeatV((options&8)!=0);
                if (scalingUV!=nullptr)
                    tp->setTextureScaling(scalingUV[0],scalingUV[1]);
                else
                    tp->setTextureScaling(s(0),s(1));
                if (xy_g!=nullptr)
                {
                    C7Vector tr;
                    tr.setIdentity();
                    tr.X(0)=xy_g[0];
                    tr.X(1)=xy_g[1];
                    tr.Q=C4Vector(0.0f,0.0f,xy_g[2]);
                    tp->setTextureRelativeConfig(tr);
                }
                if (textureId!=nullptr)
                    textureId[0]=texID;
                return(shape->getObjectHandle());
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simWriteCustomDataBlock_internal(simInt objectHandle,const simChar* tagName,const simChar* data,simInt dataSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::vector<char> buffer;
        bool useTempBuffer=false;
        size_t l=std::strlen(tagName);
        if (l>4)
        {
            useTempBuffer=( (tagName[l-4]=='@')&&(tagName[l-3]=='t')&&(tagName[l-2]=='m')&&(tagName[l-1]=='p') );
            useTempBuffer=useTempBuffer||( (tagName[0]=='@')&&(tagName[1]=='t')&&(tagName[2]=='m')&&(tagName[3]=='p') ); // backw. compatibility
        }
        if (data==nullptr)
            dataSize=0;

        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            it->writeCustomDataBlock(useTempBuffer,tagName,data,dataSize);
            // ---------------------- Old -----------------------------
            if (!useTempBuffer)
            {
                if (strlen(tagName)!=0)
                {
                    int l=it->getObjectCustomDataLength_old(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        it->getObjectCustomData_old(356248756,&buffer[0]);
                    }
                    int extractedBufSize;
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        it->setObjectCustomData_old(356248756,&buffer[0],(int)buffer.size());
                    else
                        it->setObjectCustomData_old(356248756,nullptr,0);
                }
                else
                    it->setObjectCustomData_old(356248756,nullptr,0);
            }
            // ---------------------- Old -----------------------------
        }

        if (objectHandle==sim_handle_scene)
        {
            if (useTempBuffer)
                App::currentWorld->customSceneData_tempData.setData(tagName,data,dataSize);
            else
                App::currentWorld->customSceneData.setData(tagName,data,dataSize);
            // ---------------------- Old -----------------------------
            if (!useTempBuffer)
            {
                if (strlen(tagName)!=0)
                {
                    int l=App::currentWorld->customSceneData_old->getDataLength(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        App::currentWorld->customSceneData_old->getData(356248756,&buffer[0]);
                    }
                    int extractedBufSize;
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        App::currentWorld->customSceneData_old->setData(356248756,&buffer[0],(int)buffer.size());
                    else
                        App::currentWorld->customSceneData_old->setData(356248756,nullptr,0);
                }
                else
                    App::currentWorld->customSceneData_old->setData(356248756,nullptr,0);
            }
            // ---------------------- Old -----------------------------
        }

        if (objectHandle==sim_handle_app)
        { // here we have the app
            App::worldContainer->customAppData.setData(tagName,data,dataSize);
            // ---------------------- Old -----------------------------
            if (strlen(tagName)!=0)
            {
                int l=App::worldContainer->customAppData_old->getDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    App::worldContainer->customAppData_old->getData(356248756,&buffer[0]);
                }
                int extractedBufSize;
                delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                if (buffer.size()>0)
                    App::worldContainer->customAppData_old->setData(356248756,&buffer[0],(int)buffer.size());
                else
                    App::worldContainer->customAppData_old->setData(356248756,nullptr,0);
            }
            else
                App::worldContainer->customAppData_old->setData(356248756,nullptr,0);
            // ---------------------- Old -----------------------------
        }

        // ---------------------- Old -----------------------------
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            if (!App::userSettings->compatibilityFix1)
            {
                CApiErrors::setLastWarningOrError(__func__,"targetting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in 'system/usrset.txt'");
                return(-1);
            }

            CScriptObject* script=App::worldContainer->getScriptFromHandle(objectHandle);
            if (script!=nullptr)
            { // here we have a script
                if (useTempBuffer)
                {
                    if (strlen(tagName)!=0)
                    {
                        int l=script->getObjectCustomDataLength_tempData_old(356248756);
                        if (l>0)
                        {
                            buffer.resize(l,' ');
                            script->getObjectCustomData_tempData_old(356248756,&buffer[0]);
                        }
                        int extractedBufSize;
                        delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                        _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                        if (buffer.size()>0)
                            script->setObjectCustomData_tempData_old(356248756,&buffer[0],(int)buffer.size());
                        else
                            script->setObjectCustomData_tempData_old(356248756,nullptr,0);
                    }
                    else
                        script->setObjectCustomData_tempData_old(356248756,nullptr,0);
                }
                else
                {
                    if (strlen(tagName)!=0)
                    {
                        int l=script->getObjectCustomDataLength_old(356248756);
                        if (l>0)
                        {
                            buffer.resize(l,' ');
                            script->getObjectCustomData_old(356248756,&buffer[0]);
                        }
                        int extractedBufSize;
                        delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                        _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                        if (buffer.size()>0)
                            script->setObjectCustomData_old(356248756,&buffer[0],(int)buffer.size());
                        else
                            script->setObjectCustomData_old(356248756,nullptr,0);
                    }
                    else
                        script->setObjectCustomData_old(356248756,nullptr,0);
                }
            }
        }
        // ---------------------- Old -----------------------------
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReadCustomDataBlock_internal(simInt objectHandle,const simChar* tagName,simInt* dataSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retBuffer=nullptr;
        dataSize[0]=0;
        bool useTempBuffer=false;
        size_t l=std::strlen(tagName);
        if (l>4)
        {
            useTempBuffer=( (tagName[l-4]=='@')&&(tagName[l-3]=='t')&&(tagName[l-2]=='m')&&(tagName[l-1]=='p') );
            useTempBuffer=useTempBuffer||( (tagName[0]=='@')&&(tagName[1]=='t')&&(tagName[2]=='m')&&(tagName[3]=='p') ); // backw. compatibility
        }

        std::string rrr;
        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // Here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            rrr=it->readCustomDataBlock(useTempBuffer,tagName);
        }

        if (objectHandle==sim_handle_scene)
        { // here we have a scene
            if (useTempBuffer)
                rrr=App::currentWorld->customSceneData_tempData.getData(tagName);
            else
                rrr=App::currentWorld->customSceneData.getData(tagName);
        }

        if (objectHandle==sim_handle_app)
            rrr=App::worldContainer->customAppData.getData(tagName); // here we have the app

        if (rrr.size()>0)
        {
            retBuffer=new char[rrr.size()];
            for (size_t i=0;i<rrr.size();i++)
                retBuffer[i]=rrr[i];
            dataSize[0]=int(rrr.size());
        }

        // ---------------------- Old -----------------------------
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            if (!App::userSettings->compatibilityFix1)
            {
                CApiErrors::setLastWarningOrError(__func__,"targetting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in 'system/usrset.txt'");
                return(nullptr);
            }
            CScriptObject* script=App::worldContainer->getScriptFromHandle(objectHandle);
            if (script!=nullptr)
            {
                std::vector<char> buffer;
                if (useTempBuffer)
                {
                    int l=script->getObjectCustomDataLength_tempData_old(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        script->getObjectCustomData_tempData_old(356248756,&buffer[0]);
                    }
                }
                else
                {
                    int l=script->getObjectCustomDataLength_old(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        script->getObjectCustomData_old(356248756,&buffer[0]);
                    }
                }
                retBuffer=_extractCustomDataFromBuffer(buffer,tagName,dataSize);
            }
        }
        // ---------------------- Old -----------------------------
        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simReadCustomDataBlockTags_internal(simInt objectHandle,simInt* tagCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retBuffer=nullptr;
        tagCount[0]=0;
        std::string tags;
        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            size_t tc;
            tags=it->getAllCustomDataBlockTags(false,&tc);
            tagCount[0]+=int(tc);
            tags+=it->getAllCustomDataBlockTags(true,&tc);
            tagCount[0]+=int(tc);
        }

        if (objectHandle==sim_handle_scene)
        { // here we have a scene
            size_t tc;
            tags=App::currentWorld->customSceneData.getAllTags(&tc);
            tagCount[0]+=int(tc);
            tags+=App::currentWorld->customSceneData_tempData.getAllTags(&tc);
            tagCount[0]+=int(tc);
        }

        if (objectHandle==sim_handle_app)
        { // here we have the application
            size_t tc;
            tags=App::worldContainer->customAppData.getAllTags(&tc);
            tagCount[0]+=int(tc);
        }

        if (tagCount[0]>0)
        {
            retBuffer=new char[tags.size()];
            for (size_t i=0;i<tags.size();i++)
                retBuffer[i]=tags[i];
        }

        // ---------------------- Old -----------------------------
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            std::vector<std::string> allTags;
            if (!App::userSettings->compatibilityFix1)
            {
                CApiErrors::setLastWarningOrError(__func__,"targetting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in 'system/usrset.txt'");
                return(nullptr);
            }
            CScriptObject* script=App::worldContainer->getScriptFromHandle(objectHandle);
            if (script!=nullptr)
            {
                std::vector<char> buffer;
                int l=script->getObjectCustomDataLength_tempData_old(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    script->getObjectCustomData_tempData_old(356248756,&buffer[0]);
                    _extractCustomDataTagsFromBuffer(buffer,allTags);
                }

                l=script->getObjectCustomDataLength_old(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    script->getObjectCustomData_old(356248756,&buffer[0]);
                    _extractCustomDataTagsFromBuffer(buffer,allTags);
                }
            }
            if (allTags.size()>0)
            {
                tagCount[0]=int(allTags.size());
                int totChars=0;
                for (size_t i=0;i<allTags.size();i++)
                    totChars+=(int)allTags[i].length()+1;
                retBuffer=new char[totChars];
                totChars=0;
                for (size_t i=0;i<allTags.size();i++)
                {
                    for (size_t j=0;j<allTags[i].length();j++)
                        retBuffer[totChars+j]=allTags[i][j];
                    retBuffer[totChars+allTags[i].length()]=0;
                    totChars+=(int)allTags[i].length()+1;
                }
            }
        }
        // ---------------------- Old -----------------------------

        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetShapeGeomInfo_internal(simInt shapeHandle,simInt* intData,simFloat* floatData,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            retVal=0;
            if (shape->getMeshWrapper()->isMesh())
            {
                CMesh* geom=shape->getSingleMesh();
                if (intData!=nullptr)
                {
                    intData[0]=geom->getPurePrimitiveType();
                }
                if (floatData!=nullptr)
                {
                    C3Vector s;
                    geom->getPurePrimitiveSizes(s);
                    floatData[0]=s(0);
                    floatData[1]=s(1);
                    floatData[2]=s(2);
                    floatData[3]=geom->getPurePrimitiveInsideScaling_OLD();
                }
                if (geom->isPure())
                    retVal|=2;
                if (geom->isConvex())
                    retVal|=4;
            }
            else
            { // we have a compound...
                retVal|=1;
                if (shape->getMeshWrapper()->isPure())
                    retVal|=2;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjects_internal(simInt index,simInt objectType)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int cnter=0;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if (objectType!=sim_handle_all)
            {
                if (it->getObjectType()!=objectType)
                    it=nullptr;
            }
            if (it!=nullptr)
            {
                if (cnter==index)
                {
                    int retVal=it->getObjectHandle();
                    return(retVal);
                }
                cnter++;
            }
        }
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt* simGetObjectsInTree_internal(simInt treeBaseHandle,simInt objectType,simInt options,simInt* objectCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int* retVal=nullptr;
        if ( (treeBaseHandle==sim_handle_scene)||doesObjectExist(__func__,treeBaseHandle) )
        {
            std::vector<CSceneObject*> toExplore;
            if (treeBaseHandle!=sim_handle_scene)
            {
                CSceneObject* baseObj=App::currentWorld->sceneObjects->getObjectFromHandle(treeBaseHandle);
                toExplore.push_back(baseObj);
            }
            else
            {
                for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromIndex(i);
                    toExplore.push_back(obj);
                }
            }
            std::vector<int> outHandles;
            bool firstChildrenDone=false;
            while (toExplore.size()!=0)
            {
                CSceneObject* obj=toExplore[0];
                toExplore.erase(toExplore.begin(),toExplore.begin()+1);
                if (treeBaseHandle!=sim_handle_scene)
                {
                    if ( (!firstChildrenDone)||((options&2)==0) )
                    {
                        for (size_t i=0;i<obj->getChildCount();i++)
                            toExplore.push_back(obj->getChildFromIndex(i));
                    }
                    firstChildrenDone=true;
                }
                if ((obj->getObjectType()==objectType)||(objectType==sim_handle_all))
                {
                    if ( ((options&1)==0)||(obj->getObjectHandle()!=treeBaseHandle) )
                    {
                        if ( ((options&2)==0)||(treeBaseHandle!=sim_handle_scene) )
                            outHandles.push_back(obj->getObjectHandle());
                        else
                        { // only first children! Just handle the scene part here
                            if (obj->getParent()==nullptr)
                                outHandles.push_back(obj->getObjectHandle());
                        }
                    }
                }
            }
            retVal=new int[outHandles.size()];
            for (int i=0;i<int(outHandles.size());i++)
                retVal[i]=outHandles[i];
            objectCount[0]=int(outHandles.size());
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simScaleObject_internal(simInt objectHandle,simFloat xScale,simFloat yScale,simFloat zScale,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            obj->scaleObjectNonIsometrically(xScale,yScale,zScale);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetShapeTextureId_internal(simInt shapeHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMeshWrapper()->isMesh())
            {
                CTextureProperty* tp=shape->getSingleMesh()->getTextureProperty();
                if (tp!=nullptr)
                {
                    CTextureObject* to=App::currentWorld->textureContainer->getObject(tp->getTextureObjectID());
                    return(to->getObjectID());
                }
                return(-1);
            }
            return(-1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simSetShapeTexture_internal(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simFloat* uvScaling,const simFloat* position,const simFloat* orientation)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMeshWrapper()->isMesh())
            {
                CTextureProperty* tp=shape->getSingleMesh()->getTextureProperty();
                if (tp!=nullptr)
                { // first remove any existing texture:
           //         App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                    delete tp;
                    shape->getSingleMesh()->setTextureProperty(nullptr);
                }
                if (textureId==-1)
                    return(1);
                CTextureObject* to=App::currentWorld->textureContainer->getObject(textureId);
                if (to!=nullptr)
                {
                    to->addDependentObject(shape->getObjectHandle(),shape->getSingleMesh()->getUniqueID());
                    tp=new CTextureProperty(textureId);
                    shape->getSingleMesh()->setTextureProperty(tp);
                    tp->setTextureMapMode(mappingMode);
                    tp->setInterpolateColors((options&1)==0);
                    if ((options&2)!=0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    tp->setRepeatU((options&4)!=0);
                    tp->setRepeatV((options&8)!=0);
                    tp->setTextureScaling(uvScaling[0],uvScaling[1]);
                    C7Vector tr;
                    tr.setIdentity();
                    if (position!=nullptr)
                        tr.X.setInternalData(position);
                    if (orientation!=nullptr)
                        tr.Q=C4Vector(orientation[0],orientation[1],orientation[2]);
                    tp->setTextureRelativeConfig(tr);
                    return(1);
                }
                else
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
            }
        }
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateCollectionEx_internal(simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CCollection* it=new CCollection(_currentScriptHandle);
        it->setCollectionName("___col___",false); // is actually not used anymore
        App::currentWorld->collections->addCollection(it,false);
        it->setOverridesObjectMainProperties((options&1)!=0);
        return(it->getCollectionHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddItemToCollection_internal(simInt collectionHandle,simInt what,simInt objectHandle,simInt options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(-1);
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (what!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
        }
        CCollectionElement* el=nullptr;
        if (what==sim_handle_all)
            el=new CCollectionElement(-1,sim_collectionelement_all,true);
        if (what==sim_handle_single)
            el=new CCollectionElement(objectHandle,sim_collectionelement_loose,(options&1)==0);
        if (what==sim_handle_tree)
        {
            int what=sim_collectionelement_frombaseincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_frombaseexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (what==sim_handle_chain)
        {
            int what=sim_collectionelement_fromtipincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_fromtipexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (el==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        it->addCollectionElement(el);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simDestroyCollection_internal(simInt collectionHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        App::currentWorld->collections->removeCollection(collectionHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt* simGetCollectionObjects_internal(simInt collectionHandle,simInt* objectCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesCollectionExist(__func__,collectionHandle))
        {
            CCollection* coll=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
            objectCount[0]=int(coll->getSceneObjectCountInCollection());
            int* retVal=new int[objectCount[0]];
            for (int i=0;i<objectCount[0];i++)
                retVal[i]=coll->getSceneObjectHandleFromIndex(i);
            return(retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simReorientShapeBoundingBox_internal(simInt shapeHandle,simInt relativeToHandle,simInt reservedSetToZero)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        if ( (relativeToHandle!=-1)&&(relativeToHandle!=sim_handle_self) )
        {
            if (!doesObjectExist(__func__,relativeToHandle))
                return(-1);
        }

        CShape* theShape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        CSceneObject* theObjectRelativeTo=App::currentWorld->sceneObjects->getObjectFromHandle(relativeToHandle);
        if ( (!theShape->getMeshWrapper()->isPure())||(theShape->isCompound()) )
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (relativeToHandle==-1)
                theShape->alignBoundingBoxWithWorld();
            else if (relativeToHandle==sim_handle_self)
                theShape->alignBoundingBoxWithMainAxis();
            else
            {
                C7Vector oldAbsTr(theShape->getCumulativeTransformation());
                C7Vector oldAbsTr2(theObjectRelativeTo->getCumulativeTransformation().getInverse()*oldAbsTr);
                C7Vector x(oldAbsTr2*oldAbsTr.getInverse());
                theShape->setLocalTransformation(theShape->getFullParentCumulativeTransformation().getInverse()*oldAbsTr2);
                theShape->alignBoundingBoxWithWorld();
                C7Vector newAbsTr2(theShape->getCumulativeTransformation());
                C7Vector newAbsTr(x.getInverse()*newAbsTr2);
                theShape->setLocalTransformation(theShape->getFullParentCumulativeTransformation().getInverse()*newAbsTr);
            }
        }
        else
            return(0);
        return(1); // success
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveImage_internal(const simUChar* image,const simInt* resolution,simInt options,const simChar* filename,simInt quality,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        if (CImageLoaderSaver::save(image,resolution,options,filename,quality,nullptr))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simUChar* simLoadImage_internal(simInt* resolution,simInt options,const simChar* filename,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal=CImageLoaderSaver::load(resolution,options,filename,reserved);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simUChar* simGetScaledImage_internal(const simUChar* imageIn,const simInt* resolutionIn,simInt* resolutionOut,simInt options,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal=CImageLoaderSaver::getScaledImage(imageIn,resolutionIn,resolutionOut,options);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simTransformImage_internal(simUChar* image,const simInt* resolution,simInt options,const simFloat* floatParams,const simInt* intParams,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (CImageLoaderSaver::transformImage(image,resolution[0],resolution[1],options))
            return(1);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetQHull_internal(const simFloat* inVertices,simInt inVerticesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simFloat* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        std::vector<float> vOut;
        std::vector<int> iOut;
        bool res=CMeshRoutines::getConvexHull(inVertices,inVerticesL,&vOut,&iOut);
        if (res)
        {
            verticesOut[0]=new float[vOut.size()];
            verticesOutL[0]=(int)vOut.size();
            for (size_t i=0;i<vOut.size();i++)
                verticesOut[0][i]=vOut[i];
            if (indicesOut!=nullptr)
            {
                indicesOut[0]=new int[iOut.size()];
                indicesOutL[0]=(int)iOut.size();
                for (size_t i=0;i<iOut.size();i++)
                    indicesOut[0][i]=iOut[i];
            }
            retVal=1;
        }
        else
        {
            verticesOut[0]=nullptr;
            verticesOutL[0]=0;
            if (indicesOut!=nullptr)
            {
                indicesOut[0]=nullptr;
                indicesOutL[0]=0;
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDecimatedMesh_internal(const simFloat* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simFloat decimationPercent,simInt reserved1,const simFloat* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        std::vector<float> vOut;
        std::vector<int> iOut;
        std::vector<float> vIn(inVertices,inVertices+inVerticesL);
        std::vector<int> iIn(inIndices,inIndices+inIndicesL);
        bool res=CMeshRoutines::getDecimatedMesh(vIn,iIn,decimationPercent,vOut,iOut);
        if (res)
        {
            verticesOut[0]=new float[vOut.size()];
            verticesOutL[0]=(int)vOut.size();
            for (size_t i=0;i<vOut.size();i++)
                verticesOut[0][i]=vOut[i];
            indicesOut[0]=new int[iOut.size()];
            indicesOutL[0]=(int)iOut.size();
            for (size_t i=0;i<iOut.size();i++)
                indicesOut[0][i]=iOut[i];
            retVal=1;
        }
        else
        {
            verticesOut[0]=nullptr;
            verticesOutL[0]=0;
            indicesOut[0]=nullptr;
            indicesOutL[0]=0;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCallScriptFunctionEx_internal(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,simInt stackId)
{
    TRACE_C_API;
    CScriptObject* script=nullptr;
    std::string funcName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        else
            funcName=funcNameAtScriptName;
        script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
    }
    else
    { // script is identified by a script type and sometimes also a script name
        std::string scriptName;
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
        {
            scriptName.assign(funcNameAtScriptName.begin()+p+1,funcNameAtScriptName.end());
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        }
        else
            funcName=funcNameAtScriptName;
        if (scriptHandleOrType==sim_scripttype_mainscript)
            script=App::currentWorld->embeddedScriptContainer->getMainScript();
        if (scriptHandleOrType==sim_scripttype_sandboxscript)
            script=App::worldContainer->sandboxScript;
        if (scriptHandleOrType==sim_scripttype_addonscript)
            script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
        if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==(sim_scripttype_childscript|sim_scripttype_threaded_old))||(scriptHandleOrType==sim_scripttype_customizationscript) )
        {
            int objId=-1;
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
            if (obj!=nullptr)
                objId=obj->getObjectHandle();
            else
                objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
            if (scriptHandleOrType==sim_scripttype_customizationscript)
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            else
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
        }
    }

    std::string tmp("External call to simCallScriptFunction failed (");
    tmp+=functionNameAtScriptName;
    tmp+="): ";
    if (script!=nullptr)
    {
        int retVal=-1; // error
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackId);
        if (stack!=nullptr)
        {
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                    retVal=script->callCustomScriptFunction(funcName.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=1;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)funcName.c_str();
                    d[3]=stack;

                    retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                    retVal=script->callCustomScriptFunction(funcName.c_str(),stack);
            }
            if (retVal==-1)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION);
                tmp+=SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION;
                App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)
            }
            if (retVal==0)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
                tmp+=SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)
                retVal=-1; // to stay backward compatible
            }
            if (retVal==1)
                retVal=0; // to stay backward compatible
        }
        else
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
            tmp+=SIM_ERROR_INVALID_HANDLE;
            App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
    tmp+=SIM_ERROR_SCRIPT_INEXISTANT;
    App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)

    return(-1);
}

simChar* simGetExtensionString_internal(simInt objectHandle,simInt index,const char* key)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal=nullptr;
        std::string extensionString;
        if (objectHandle==-1)
            extensionString=App::currentWorld->environment->getExtensionString();
        else
        {
            if (doesObjectExist(__func__,objectHandle))
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if ( (it->getObjectType()==sim_object_shape_type)&&(index>=0) )
                {
                    CMesh* geom=((CShape*)it)->getMeshWrapper()->getShapeComponentAtIndex(index);
                    if (geom!=nullptr)
                        extensionString=geom->color.getExtensionString();
                }
                else
                    extensionString=it->getExtensionString();
            }
        }
        if ( (key!=nullptr)&&(strlen(key)!=0)&&(extensionString.size()>0) )
        {
            std::string value;
            if (tt::getValueOfKey(key,extensionString.c_str(),value))
                extensionString=value;
            else
                extensionString.clear();
        }
        if (extensionString.size()>0)
        {
            retVal=new char[extensionString.size()+1];
            for (size_t i=0;i<extensionString.size();i++)
                retVal[i]=extensionString[i];
            retVal[extensionString.size()]=0;
        }
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simComputeMassAndInertia_internal(simInt shapeHandle,simFloat density)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=(CShape*)App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMeshWrapper()->isConvex())
            {
                std::vector<float> vert;
                std::vector<int> ind;
                shape->getMeshWrapper()->getCumulativeMeshes(vert,&ind,nullptr);
                C3Vector com;
                C3X3Matrix tensor;
                float mass=CVolInt::getMassCenterOfMassAndInertiaTensor(&vert[0],(int)vert.size()/3,&ind[0],(int)ind.size()/3,density,com,tensor);
                C4Vector rot;
                C3Vector pmoment;
                CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,pmoment);
                shape->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
                shape->getMeshWrapper()->setLocalInertiaFrame(C7Vector(rot,com));
                shape->getMeshWrapper()->setMass(mass);
                App::undoRedo_sceneChanged(""); // **************** UNDO THINGY ****************
                return(1);
            }
            return(0);
        }
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateStack_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        return(stack->getId());
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReleaseStack_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::worldContainer->interfaceStackContainer->destroyStack(stackHandle))
            return(1);
        return(0);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCopyStack_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
            return(App::worldContainer->interfaceStackContainer->createStackCopy(stack)->getId());
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushNullOntoStack_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushNullOntoStack();
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushBoolOntoStack_internal(simInt stackHandle,simBool value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushBoolOntoStack(value);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt32OntoStack_internal(simInt stackHandle,simInt value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushInt32OntoStack(value);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt64OntoStack_internal(simInt stackHandle,simInt64 value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushInt64OntoStack(value);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushFloatOntoStack_internal(simInt stackHandle,simFloat value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushFloatOntoStack(value);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushDoubleOntoStack_internal(simInt stackHandle,simDouble value)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushDoubleOntoStack(value);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushStringOntoStack_internal(simInt stackHandle,const simChar* value,simInt stringSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushStringOntoStack(value,size_t(stringSize));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushUInt8TableOntoStack_internal(simInt stackHandle,const simUChar* values,simInt valueCnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushUCharArrayOntoStack(values,size_t(valueCnt));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt32TableOntoStack_internal(simInt stackHandle,const simInt* values,simInt valueCnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushInt32ArrayOntoStack(values,size_t(valueCnt));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt64TableOntoStack_internal(simInt stackHandle,const simInt64* values,simInt valueCnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushInt64ArrayOntoStack(values,size_t(valueCnt));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushFloatTableOntoStack_internal(simInt stackHandle,const simFloat* values,simInt valueCnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushFloatArrayOntoStack(values,size_t(valueCnt));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushDoubleTableOntoStack_internal(simInt stackHandle,const simDouble* values,simInt valueCnt)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushDoubleArrayOntoStack(values,size_t(valueCnt));
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushTableOntoStack_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushTableOntoStack();
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simInsertDataIntoStackTable_internal(simInt stackHandle)
{ // stack should have at least: table,key,value (where value is on top of stack)
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->insertDataIntoStackTable())
                return(1);
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackSize_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
            return(stack->getStackSize());
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPopStackItem_internal(simInt stackHandle,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->popStackValue(count);
            return(stack->getStackSize());
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simMoveStackItemToTop_internal(simInt stackHandle,simInt cIndex)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->moveStackItemToTop(cIndex))
                return(1);
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_INDEX);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simIsStackValueNull_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->isStackValueNull())
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackBoolValue_internal(simInt stackHandle,simBool* boolValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                bool v;
                if (stack->getStackBoolValue(v))
                {
                    boolValue[0]=0;
                    if (v)
                        boolValue[0]=1;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt32Value_internal(simInt stackHandle,simInt* numberValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                int v;
                if (stack->getStackInt32Value(v))
                {
                    numberValue[0]=v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt64Value_internal(simInt stackHandle,simInt64* numberValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                simInt64 v;
                if (stack->getStackInt64Value(v))
                {
                    numberValue[0]=v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackFloatValue_internal(simInt stackHandle,simFloat* numberValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                float v;
                if (stack->getStackFloatValue(v))
                {
                    numberValue[0]=v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackDoubleValue_internal(simInt stackHandle,simDouble* numberValue)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                double v;
                if (stack->getStackDoubleValue(v))
                {
                    numberValue[0]=v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStackStringValue_internal(simInt stackHandle,simInt* stringSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        if (stringSize!=nullptr)
            stringSize[0]=-1;
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                std::string v;
                if (stack->getStackStringValue(v))
                {
                    if (stringSize!=nullptr)
                        stringSize[0]=(int)v.size();
                    char* buff=new char[v.size()+1];
                    for (size_t i=0;i<v.size();i++)
                        buff[i]=v[i];
                    buff[v.size()]=0;
                    return(buff);
                }
                if (stringSize!=nullptr)
                    stringSize[0]=0;
                return(nullptr);
            }
            if (stringSize!=nullptr)
                stringSize[0]=-1;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(nullptr);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        if (stringSize!=nullptr)
            stringSize[0]=-1;
        return(nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    if (stringSize!=nullptr)
        stringSize[0]=-1;
    return(nullptr);
}

simInt simGetStackTableInfo_internal(simInt stackHandle,simInt infoType)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                int retVal=stack->getStackTableInfo(infoType);
                return(retVal);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackUInt8Table_internal(simInt stackHandle,simUChar* array,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackUCharArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt32Table_internal(simInt stackHandle,simInt* array,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackInt32Array(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt64Table_internal(simInt stackHandle,simInt64* array,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackInt64Array(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackFloatTable_internal(simInt stackHandle,simFloat* array,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackFloatArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackDoubleTable_internal(simInt stackHandle,simDouble* array,simInt count)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackDoubleArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simUnfoldStackTable_internal(simInt stackHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->unfoldStackTable())
                    return(1);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDebugStack_internal(simInt stackHandle,simInt cIndex)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            std::string buffer;
            stack->printContent(cIndex,buffer);
            App::logMsg(sim_verbosity_none,buffer.c_str());
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat simGetEngineFloatParam_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    bool success=true;
    float retVal=0.0f;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                retVal=App::currentWorld->dynamicsContainer->getEngineFloatParam(paramId,&success);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    retVal=joint->getEngineFloatParam(paramId,&success);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    retVal=mat->getEngineFloatParam(paramId,&success);
                }
            }
        }
        if (ok!=nullptr)
            ok[0]=success;
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simInt simGetEngineInt32Param_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    bool success=true;
    int retVal=0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                retVal=App::currentWorld->dynamicsContainer->getEngineIntParam(paramId,&success);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    retVal=joint->getEngineIntParam(paramId,&success);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    retVal=mat->getEngineIntParam(paramId,&success);
                }
            }
        }
        if (ok!=nullptr)
            ok[0]=success;
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simBool simGetEngineBoolParam_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    bool success=true;
    simBool retVal=0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                retVal=App::currentWorld->dynamicsContainer->getEngineBoolParam(paramId,&success);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    retVal=joint->getEngineBoolParam(paramId,&success);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    retVal=mat->getEngineBoolParam(paramId,&success);
                }
            }
        }
        if (ok!=nullptr)
            ok[0]=success;
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simInt simSetEngineFloatParam_internal(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=(CSceneObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineFloatParam(paramId,val,false);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    success=joint->setEngineFloatParam(paramId,val);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    success=mat->setEngineFloatParam(paramId,val);
                }
            }
        }
        if (success)
            return(1);
        return(0);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetEngineInt32Param_internal(simInt paramId,simInt objectHandle,const simVoid* object,simInt val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=(CSceneObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineIntParam(paramId,val,false);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    success=joint->setEngineIntParam(paramId,val);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    success=mat->setEngineIntParam(paramId,val);
                }
            }
        }
        if (success)
            return(1);
        return(0);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetEngineBoolParam_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=(CSceneObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineBoolParam(paramId,val,false);
            else
            {
                if (it->getObjectType()==sim_object_joint_type)
                {
                    CJoint* joint=(CJoint*)it;
                    success=joint->setEngineBoolParam(paramId,val);
                }
                if (it->getObjectType()==sim_object_shape_type)
                {
                    CShape* shape=(CShape*)it;
                    CDynMaterialObject* mat=shape->getDynMaterial();
                    success=mat->setEngineBoolParam(paramId,val);
                }
            }
        }
        if (success)
            return(1);
        return(0);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateOctree_internal(simFloat voxelSize,simInt options,simFloat pointSize,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        COctree* it=new COctree();
        it->setCellSize(voxelSize);
        it->setPointSize(int(pointSize+0.5));
        it->setUseRandomColors(options&1);
        it->setShowOctree(options&2);
        it->setUsePointsInsteadOfCubes(options&4);
        it->setColorIsEmissive(options&16);
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreatePointCloud_internal(simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPointCloud* it=new CPointCloud();
        it->setCellSize(maxVoxelSize);
        it->setMaxPointCountPerCell(maxPtCntPerVoxel);
        it->setPointSize(int(pointSize+0.5));
        it->setUseRandomColors(options&1);
        it->setShowOctree(options&2);
        it->setDoNotUseCalculationStructure(options&8);
        it->setColorIsEmissive(options&16);
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetPointCloudOptions_internal(simInt pointCloudHandle,simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        it->setCellSize(maxVoxelSize);
        it->setMaxPointCountPerCell(maxPtCntPerVoxel);
        it->setPointSize(int(pointSize+0.5));
        it->setUseRandomColors(options&1);
        it->setShowOctree(options&2);
        it->setDoNotUseCalculationStructure(options&8);
        it->setColorIsEmissive(options&16);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetPointCloudOptions_internal(simInt pointCloudHandle,simFloat* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simFloat* pointSize,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        maxVoxelSize[0]=it->getCellSize();
        maxPtCntPerVoxel[0]=it->getMaxPointCountPerCell();
        pointSize[0]=(float)it->getPointSize();
        options[0]=0;
        if (it->getUseRandomColors())
            options[0]|=1;
        if (it->getShowOctree())
            options[0]|=2;
        if (it->getDoNotUseCalculationStructure())
            options[0]|=8;
        if (it->getColorIsEmissive())
            options[0]|=16;
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simInsertVoxelsIntoOctree_internal(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if ( (tag==nullptr)||(color==nullptr) )
        {
            if (color==nullptr)
                it->insertPoints(pts,ptCnt,options&1,nullptr,options&2,nullptr,0);
            else
            {
                std::vector<unsigned int> tags;
                if (options&2)
                    tags.resize(ptCnt,0);
                else
                    tags.push_back(0);
                it->insertPoints(pts,ptCnt,options&1,color,options&2,&tags[0],0);
            }
        }
        else
            it->insertPoints(pts,ptCnt,options&1,color,options&2,tag,0);
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveVoxelsFromOctree_internal(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->subtractPoints(pts,ptCnt,options&1);
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertPointsIntoPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        float insertionToleranceSaved=it->getInsertionDistanceTolerance();
        int optionalValuesBits=0;
        if (optionalValues!=nullptr)
            optionalValuesBits=((int*)optionalValues)[0];
        if (optionalValuesBits&1)
            it->setInsertionDistanceTolerance(((float*)optionalValues)[1]);
        it->insertPoints(pts,ptCnt,options&1,color,options&2);
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemovePointsFromPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->removePoints(pts,ptCnt,options&1,tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simIntersectPointsWithPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->intersectPoints(pts,ptCnt,options&1,tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

const float* simGetOctreeVoxels_internal(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(nullptr);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        const std::vector<float>* p=it->getCubePositions();
        if (p->size()==0)
        {
            ptCnt[0]=0;
            return(nullptr);
        }
        ptCnt[0]=int(p->size())/3;
        return(&(p[0])[0]);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

const float* simGetPointCloudPoints_internal(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(nullptr);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        const std::vector<float>* p=it->getPoints();
        if (p->size()==0)
        {
            ptCnt[0]=0;
            return(nullptr);
        }
        ptCnt[0]=int(p->size())/3;
        return(&(p[0])[0]);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simInsertObjectIntoOctree_internal(simInt octreeHandle,simInt objectHandle,simInt options,const simUChar* color,simUInt tag,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);

        float savedCols[3];
        it->getColor()->getColor(savedCols,sim_colorcomponent_ambient_diffuse);
        float* cptr=it->getColor()->getColorsPtr();
        if (color!=nullptr)
        {
            cptr[0]=float(color[0])/255.1f;
            cptr[1]=float(color[1])/255.1f;
            cptr[2]=float(color[2])/255.1f;
        }
        it->insertObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle),tag);
        cptr[0]=savedCols[0];
        cptr[1]=savedCols[1];
        cptr[2]=savedCols[2];
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSubtractObjectFromOctree_internal(simInt octreeHandle,simInt objectHandle,simInt options,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        it->subtractObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle));
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertObjectIntoPointCloud_internal(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat gridSize,const simUChar* color,simVoid* optionalValues)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        float savedGridSize=it->getBuildResolution();
        it->setBuildResolution(gridSize);
        float savedCols[3];
        it->getColor()->getColor(savedCols,sim_colorcomponent_ambient_diffuse);
        if (color!=nullptr)
        {
            it->getColor()->getColorsPtr()[0]=float(color[0])/255.1f;
            it->getColor()->getColorsPtr()[1]=float(color[1])/255.1f;
            it->getColor()->getColorsPtr()[2]=float(color[2])/255.1f;
        }
        float insertionToleranceSaved=it->getInsertionDistanceTolerance();
        int optionalValuesBits=0;
        if (optionalValues!=nullptr)
            optionalValuesBits=((int*)optionalValues)[0];
        if (optionalValuesBits&1)
            it->setInsertionDistanceTolerance(((float*)optionalValues)[1]);
        it->insertObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle));
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        it->setBuildResolution(savedGridSize);
        it->getColor()->getColorsPtr()[0]=savedCols[0];
        it->getColor()->getColorsPtr()[1]=savedCols[1];
        it->getColor()->getColorsPtr()[2]=savedCols[2];
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSubtractObjectFromPointCloud_internal(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat tolerance,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,pointCloudHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        it->subtractObject(App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle),tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCheckOctreePointOccupancy_internal(simInt octreeHandle,simInt options,const simFloat* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (ptCnt<=0)
            return(-1);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        if (it->getOctreeInfo()==nullptr)
            return(0);
        const float* _pts=points;
        std::vector<float> __pts;
        if (options&1)
        {
            C7Vector tr(it->getFullCumulativeTransformation());
            for (int i=0;i<ptCnt;i++)
            {
                C3Vector v(&points[3*i]);
                v*=tr;
                __pts.push_back(v(0));
                __pts.push_back(v(1));
                __pts.push_back(v(2));
            }
            _pts=&__pts[0];
        }
        if (ptCnt==1)
        {
            if (CPluginContainer::geomPlugin_getOctreePointCollision(it->getOctreeInfo(),it->getFullCumulativeTransformation(),C3Vector(_pts),tag,location))
                return(1);
        }
        else
        {
            if (CPluginContainer::geomPlugin_getOctreePointsCollision(it->getOctreeInfo(),it->getFullCumulativeTransformation(),_pts,ptCnt))
                return(1);
        }
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simOpenTextEditor_internal(const simChar* initText,const simChar* xml,simInt* various)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    char* retVal=nullptr;
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
    {
        std::string txt=App::mainWindow->codeEditorContainer->openModalTextEditor(initText,xml,various);
        retVal=new char[txt.size()+1];
        for (size_t i=0;i<txt.size();i++)
            retVal[i]=txt[i];
        retVal[txt.size()]=0;
    }
#endif
    return(retVal);
}

simChar* simPackTable_internal(simInt stackHandle,simInt* bufferSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                std::string buff(stack->getBufferFromTable());
                if (buff.size()>0)
                {
                    char* bu=new char[buff.size()];
                    bufferSize[0]=int(buff.size());
                    for (size_t i=0;i<buff.size();i++)
                        bu[i]=buff[i];
                    return(bu);
                }
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(nullptr);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simUnpackTable_internal(simInt stackHandle,const simChar* buffer,simInt bufferSize)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->pushTableFromBuffer(buffer,bufferSize))
                return(0);
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetReferencedHandles_internal(simInt objectHandle,simInt count,const simInt* referencedHandles,const simInt* reserved1,const simInt* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if ((handleFlags&sim_handleflag_keeporiginal)==0)
            it->setReferencedHandles(size_t(count),referencedHandles);
        else
            it->setReferencedOriginalHandles(count,referencedHandles);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetReferencedHandles_internal(simInt objectHandle,simInt** referencedHandles,simInt** reserved1,simInt** reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        int* handles=nullptr;
        int cnt;
        if ((handleFlags&sim_handleflag_keeporiginal)==0)
        {
            cnt=int(it->getReferencedHandlesCount());
            if (cnt>0)
            {
                handles=new int[cnt];
                it->getReferencedHandles(handles);
            }
        }
        else
        {
            cnt=int(it->getReferencedOriginalHandlesCount());
            if (cnt>0)
            {
                handles=new int[cnt];
                it->getReferencedOriginalHandles(handles);
            }
        }
        referencedHandles[0]=handles;
        return(cnt);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeViz_internal(simInt shapeHandle,simInt index,struct SShapeVizInfo* info)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=shapeHandle&0x0ff00000;
        shapeHandle=shapeHandle&0x000fffff;

        if (!isShape(__func__,shapeHandle))
            return(-1);
        int retVal=0;
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<CMesh*> all;
        it->getMeshWrapper()->getAllShapeComponentsCumulative(all);
        if ( (index>=0)&&(index<int(all.size())) )
        {
            CMesh* geom=all[index];

            if ((handleFlags&sim_handleflag_extended)!=0)
            {
                info->transparency=0.0f;
                if (geom->color.getTranslucid())
                    info->transparency=geom->color.getOpacity();
                info->options=0;
                if (geom->getCulling())
                    info->options|=1;
                if (geom->getWireframe_OLD())
                    info->options|=2;
            }

            C7Vector tr(geom->getVerticeLocalFrame());
            const std::vector<float>* wvert=geom->getVertices();
            const std::vector<int>* wind=geom->getIndices();
            const std::vector<float>* wnorm=geom->getNormals();
            info->verticesSize=int(wvert->size());
            info->vertices=new float[wvert->size()];
            for (size_t i=0;i<wvert->size()/3;i++)
            {
                C3Vector v((&wvert[0][0])+i*3);
                v=tr*v;
                info->vertices[3*i+0]=v(0);
                info->vertices[3*i+1]=v(1);
                info->vertices[3*i+2]=v(2);
            }
            info->indicesSize=int(wind->size());
            info->indices=new int[wind->size()];
            info->normals=new float[wind->size()*3];
            for (size_t i=0;i<wind->size();i++)
            {
                info->indices[i]=wind->at(i);
                C3Vector n(&(wnorm[0])[0]+i*3);
                n=tr.Q*n; // only orientation
                info->normals[3*i+0]=n(0);
                info->normals[3*i+1]=n(1);
                info->normals[3*i+2]=n(2);
            }
            geom->color.getColor(info->colors+0,sim_colorcomponent_ambient_diffuse);
            geom->color.getColor(info->colors+3,sim_colorcomponent_specular);
            geom->color.getColor(info->colors+6,sim_colorcomponent_emission);
            info->shadingAngle=geom->getShadingAngle();

            CTextureProperty* tp=geom->getTextureProperty();
            CTextureObject* to=nullptr;
            const std::vector<float>* tc=nullptr;
            if (tp!=nullptr)
            {
                to=tp->getTextureObject();
                tc=tp->getTextureCoordinates(-1,tr,wvert[0],wind[0]);
            }

            if ( (to!=nullptr)&&(tc!=nullptr) )
            {
                retVal=2;
                to->getTextureSize(info->textureRes[0],info->textureRes[1]);
                size_t totBytes=4*info->textureRes[0]*info->textureRes[1];
                info->texture=new char[totBytes];
                const char* ob=(char*)to->getTextureBufferPointer();
                for (size_t i=0;i<totBytes;i++)
                    info->texture[i]=ob[i];
                info->textureCoords=new float[tc->size()];
                for (size_t i=0;i<tc->size();i++)
                    info->textureCoords[i]=tc->at(i);
                info->textureApplyMode=tp->getApplyMode();
                info->textureOptions=0;
                if (tp->getRepeatU())
                    info->textureOptions|=1;
                if (tp->getRepeatV())
                    info->textureOptions|=2;
                if (tp->getInterpolateColors())
                    info->textureOptions|=4;
                if (geom->getWireframe_OLD())
                    info->textureOptions|=8;
                info->textureId=tp->getTextureObjectID();
            }
            else
            {
                retVal=1;
                info->texture=nullptr;
                info->textureCoords=nullptr;
                info->textureId=-1;
                info->textureOptions=0;
                if (geom->getWireframe_OLD())
                    info->textureOptions|=8;
            }
            return(retVal);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simExecuteScriptString_internal(simInt scriptHandleOrType,const simChar* stringAtScriptName,simInt stackHandle)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* script=nullptr;
        std::string stringToExecute;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string strAtScriptName(stringAtScriptName);
            size_t p=strAtScriptName.rfind('@');
            if (p!=std::string::npos)
                stringToExecute.assign(strAtScriptName.begin(),strAtScriptName.begin()+p);
            else
                stringToExecute=strAtScriptName;
            script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
        }
        else
        {
            std::string scriptName;
            std::string strAtScriptName(stringAtScriptName);
            size_t p=strAtScriptName.rfind('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(strAtScriptName.begin()+p+1,strAtScriptName.end());
                stringToExecute.assign(strAtScriptName.begin(),strAtScriptName.begin()+p);
            }
            else
                stringToExecute=strAtScriptName;

            if (scriptHandleOrType==sim_scripttype_mainscript)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::worldContainer->sandboxScript;
            if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==sim_scripttype_customizationscript) )
            {
                if (scriptName.size()>0)
                {
                    int objId=-1;
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                    if (obj!=nullptr)
                        objId=obj->getObjectHandle();
                    else
                        objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                    if (scriptHandleOrType==sim_scripttype_customizationscript)
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                    else
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
            }
        }

        if (script!=nullptr)
        {
            bool noReturnDesired=(stackHandle==0);
            CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
            if ( (stack==nullptr)&&(!noReturnDesired) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
                return(-1);
            }
            int retVal=-1; // error
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // OLD, very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                    retVal=script->executeScriptString(stringToExecute.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=3;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)stringToExecute.c_str();
                    d[3]=stack;

                    retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->executeScriptString(stringToExecute.c_str(),stack);
                }
            }

            if (retVal!=0)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
                retVal=-1;
            }
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetApiFunc_internal(simInt scriptHandleOrType,const simChar* apiWord)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int scriptType=-1;
        bool threaded=false;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            CScriptObject* script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
            if (script!=nullptr)
            {
                scriptType=script->getScriptType();
                threaded=script->getThreadedExecution_oldThreads();
            }
        }
        else
            scriptType=scriptHandleOrType;
        std::string apiW(apiWord);
        bool funcs=true;
        bool vars=true;
        if (apiW.size()>0)
        {
            if ( (apiW[0]=='+')||(apiW[0]=='-') )
            {
                vars=(apiW[0]!='+');
                funcs=(apiW[0]!='-');
                apiW.erase(0,1);
            }
        }
        std::vector<std::string> t;
        if (funcs)
            CScriptObject::getMatchingFunctions(apiW.c_str(),t);
        if (vars)
            CScriptObject::getMatchingConstants(apiW.c_str(),t);
        std::string theWords;
        for (size_t i=0;i<t.size();i++)
        {
            theWords+=t[i];
            if (i!=t.size()-1)
                theWords+=' ';
        }
        char* buff=nullptr;
        if (theWords.size()>0)
        {
            buff=new char[theWords.size()+1];
            strcpy(buff,theWords.c_str());
        }
        return(buff);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simGetApiInfo_internal(simInt scriptHandleOrType,const simChar* apiWord)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int scriptType=-1;
        bool threaded=false;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            CScriptObject* script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
            if (script!=nullptr)
            {
                scriptType=script->getScriptType();
                threaded=script->getThreadedExecution_oldThreads();
            }
        }
        else
            scriptType=scriptHandleOrType;
        if (strlen(apiWord)>0)
        {
            std::string tip(CScriptObject::getFunctionCalltip(apiWord));
            char* buff=nullptr;
            if (tip.size()>0)
            {
                buff=new char[tip.size()+1];
                strcpy(buff,tip.c_str());
            }
            return(buff);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        return(nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetModuleInfo_internal(const simChar* moduleName,simInt infoType,const simChar* stringInfo,simInt intInfo)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromName(moduleName,true);
        if (plug!=nullptr)
        {
            if (infoType==sim_moduleinfo_extversionstr)
            {
                plug->extendedVersionString=stringInfo;
                return(1);
            }
            if (infoType==sim_moduleinfo_builddatestr)
            {
                plug->buildDateString=stringInfo;
                return(1);
            }
            if (infoType==sim_moduleinfo_extversionint)
            {
                plug->extendedVersionInt=intInfo;
                return(1);
            }
            if (infoType==sim_moduleinfo_verbosity)
            {
                App::setConsoleVerbosity(intInfo,moduleName);
                return(1);
            }
            if (infoType==sim_moduleinfo_statusbarverbosity)
            {
                App::setStatusbarVerbosity(intInfo,moduleName);
                return(1);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PLUGIN_NAME);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetModuleInfo_internal(const simChar* moduleName,simInt infoType,simChar** stringInfo,simInt* intInfo)
{
    TRACE_C_API;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromName(moduleName,true);
        if (plug!=nullptr)
        {
            if (infoType==sim_moduleinfo_extversionstr)
            {
                char* txt=new char[plug->extendedVersionString.length()+1];
                strcpy(txt,plug->extendedVersionString.c_str());
                if (stringInfo!=nullptr)
                    stringInfo[0]=txt;
                else
                    delete[] txt;
                return(1);
            }
            if (infoType==sim_moduleinfo_builddatestr)
            {
                char* txt=new char[plug->buildDateString.length()+1];
                strcpy(txt,plug->buildDateString.c_str());
                if (stringInfo!=nullptr)
                    stringInfo[0]=txt;
                else
                    delete[] txt;
                return(1);
            }
            if (infoType==sim_moduleinfo_extversionint)
            {
                intInfo[0]=plug->extendedVersionInt;
                return(1);
            }
            if (infoType==sim_moduleinfo_verbosity)
            {
                intInfo[0]=App::getConsoleVerbosity(moduleName);
                return(1);
            }
            if (infoType==sim_moduleinfo_statusbarverbosity)
            {
                intInfo[0]=App::getStatusbarVerbosity(moduleName);
                return(1);
            }
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PLUGIN_NAME);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simIsDeprecated_internal(const simChar* funcOrConst)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=CScriptObject::isFunctionOrConstDeprecated(funcOrConst);
        if (retVal<0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_STRING_NOT_RECOGNIZED_AS_FUNC_OR_CONST);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}
simChar* simGetPersistentDataTags_internal(simInt* tagCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<std::string> allTags;
        tagCount[0]=App::worldContainer->persistentDataContainer->getAllDataNames(allTags);
        char* retBuffer=nullptr;
        if (allTags.size()>0)
        {
            tagCount[0]=int(allTags.size());
            int totChars=0;
            for (size_t i=0;i<allTags.size();i++)
                totChars+=(int)allTags[i].length()+1;
            retBuffer=new char[totChars];
            totChars=0;
            for (size_t i=0;i<allTags.size();i++)
            {
                for (size_t j=0;j<allTags[i].length();j++)
                    retBuffer[totChars+j]=allTags[i][j];
                retBuffer[totChars+allTags[i].length()]=0;
                totChars+=(int)allTags[i].length()+1;
            }
        }
        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simEventNotification_internal(const simChar* event)
{
    TRACE_C_API;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        sim::tinyxml2::XMLDocument xmldoc;
        sim::tinyxml2::XMLError error=xmldoc.Parse(event);
        if(error==sim::tinyxml2::XML_NO_ERROR)
        {
            sim::tinyxml2::XMLElement* rootElement=xmldoc.FirstChildElement();
            const char* origin=rootElement->Attribute("origin");
            if (origin!=nullptr)
            {
                if (strcmp(origin,"codeEditor")==0)
                {
                    const char* msg=rootElement->Attribute("msg");
                    const char* handle=rootElement->Attribute("handle");
                    const char* data=rootElement->Attribute("data");
#ifdef SIM_WITH_GUI
                    if ((msg!=nullptr)&&(handle!=nullptr)&&(data!=nullptr)&&(App::mainWindow!=nullptr))
                    {
                        if (strcmp(msg,"closeEditor")==0)
                        {
                            int h;
                            if (tt::stringToInt(handle,h))
                            {
                                if (strlen(data)!=0)
                                {
                                    int callingScript=App::mainWindow->codeEditorContainer->getCallingScriptHandle(h);
                                    CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
                                    int posAndSize[4];
                                    std::string txt=App::mainWindow->codeEditorContainer->getText(h,posAndSize);
                                    stack->pushStringOntoStack(txt.c_str(),0);
                                    stack->pushInt32ArrayOntoStack(posAndSize+0,2);
                                    stack->pushInt32ArrayOntoStack(posAndSize+2,2);
                                    simCallScriptFunctionEx_internal(callingScript,data,stack->getId());
                                    App::worldContainer->interfaceStackContainer->destroyStack(stack);
                                }
                                if ( (strlen(data)==0)||App::mainWindow->codeEditorContainer->getCloseAfterCallbackCalled(h) )
                                    App::mainWindow->codeEditorContainer->close(h,nullptr,nullptr,nullptr);
                                retVal=1;
                            }
                        }
                        if (strcmp(msg,"restartScript")==0)
                        {
                            int h;
                            if (tt::stringToInt(handle,h))
                                App::mainWindow->codeEditorContainer->restartScript(h);
                        }
                    }
#endif
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simApplyTexture_internal(simInt shapeHandle,const simFloat* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    TRACE_C_API;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            if (shape->getMeshWrapper()->isMesh())
            {
                // first remove any existing texture:
                CTextureProperty* tp=shape->getSingleMesh()->getTextureProperty();
                if (tp!=nullptr)
                {
                    App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                    delete tp;
                    shape->getSingleMesh()->setTextureProperty(nullptr);
                }
                if (shape->getSingleMesh()->getIndices()->size()*2==textCoordSize)
                {
                    // Now create and attach the texture:
                    CTextureObject* textureObj=new CTextureObject(textureResolution[0],textureResolution[1]);
                    textureObj->setImage(options&16,options&32,(options&64)==0,texture);
                    textureObj->setObjectName("importedTexture");
                    textureObj->addDependentObject(shape->getObjectHandle(),shape->getSingleMesh()->getUniqueID());
                    retVal=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                    tp=new CTextureProperty(retVal);
                    shape->getSingleMesh()->setTextureProperty(tp);
                    tp->setInterpolateColors((options&1)==0);
                    if ((options&2)!=0)
                        tp->setApplyMode(1);
                    else
                        tp->setApplyMode(0);
                    std::vector<float> c;
                    c.assign(textureCoordinates,textureCoordinates+textCoordSize);
                    tp->setFixedCoordinates(&c);
                }
                else
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BAD_TEXTURE_COORD_SIZE);
            }
            else
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_BE_COMPOUND_SHAPE);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simSetJointDependency_internal(simInt jointHandle,simInt masterJointHandle,simFloat offset,simFloat multCoeff)
{
    TRACE_C_API;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isJoint(__func__,jointHandle))
        {
            if ( (masterJointHandle==-1)||isJoint(__func__,masterJointHandle) )
            {
                CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
                joint->setDependencyMasterJointHandle(masterJointHandle);
                if (joint->getDependencyMasterJointHandle()==masterJointHandle)
                {
                    joint->setDependencyJointOffset(offset);
                    joint->setDependencyJointMult(multCoeff);
                    retVal=0;
                    return(retVal);
                }
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simGetJointDependency_internal(simInt jointHandle,simInt* masterJointHandle,simFloat* offset,simFloat* multCoeff)
{
    TRACE_C_API;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isJoint(__func__,jointHandle))
        {
            CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            masterJointHandle[0]=joint->getDependencyMasterJointHandle();
            offset[0]=joint->getDependencyJointOffset();
            multCoeff[0]=joint->getDependencyJointMult();
            retVal=0;
            return(retVal);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simSetNamedStringParam_internal(const simChar* paramName,const simChar* stringParam,simInt paramLength)
{
    TRACE_C_API;
    int retVal=-1;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        retVal=App::setApplicationNamedParam(paramName,stringParam,paramLength);
        if (retVal>=0)
            return(retVal);
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simChar* simGetNamedStringParam_internal(const simChar* paramName,simInt* paramLength)
{
    TRACE_C_API;
    char* retVal=nullptr;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string val=App::getApplicationNamedParam(paramName);
        if (val.size()>0)
        {
            retVal=new char[val.size()+1];
            for (size_t i=0;i<val.size();i++)
                retVal[i]=val[i];
            retVal[val.size()]=0;
            if (paramLength!=nullptr)
                paramLength[0]=int(val.size());
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

//************************************************************************************************************
//************************************************************************************************************
// FOLLOWING FUNCTIONS ARE VERY FAST, BUT NO SPECIFIC CHECKING IS DONE. ALSO, MANY OPERATE ON OBJECT POINTERS!
//************************************************************************************************************
//************************************************************************************************************


const simVoid* _simGetGeomWrapFromGeomProxy_internal(const simVoid* geomData)
{
    TRACE_C_API;
    return(((CShape*)geomData)->getMeshWrapper());
}

simVoid _simGetLocalInertiaFrame_internal(const simVoid* geomInfo,simFloat* pos,simFloat* quat)
{
    TRACE_C_API;
    C7Vector tr(((CMeshWrapper*)geomInfo)->getLocalInertiaFrame());
    tr.Q.getInternalData(quat);
    tr.X.getInternalData(pos);
}

simVoid _simGetPrincipalMomentOfInertia_internal(const simVoid* geomInfo,simFloat* inertia)
{
    TRACE_C_API;
    ((CMeshWrapper*)geomInfo)->getPrincipalMomentsOfInertia().getInternalData(inertia);
}


simInt _simGetPurePrimitiveType_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->getPurePrimitiveType());
}

simVoid _simGetPurePrimitiveSizes_internal(const simVoid* geometric,simFloat* sizes)
{
    TRACE_C_API;
    C3Vector s;
    ((CMesh*)geometric)->getPurePrimitiveSizes(s);
    s.getInternalData(sizes);
}

simBool _simIsGeomWrapGeometric_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->isMesh());
}

simBool _simIsGeomWrapConvex_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->isConvex());
}

simInt _simGetGeometricCount_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllShapeComponentsCumulative(all);
    return((int)all.size());
}

simVoid _simGetAllGeometrics_internal(const simVoid* geomInfo,simVoid** allGeometrics)
{
    TRACE_C_API;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllShapeComponentsCumulative(all);
    for (size_t i=0;i<all.size();i++)
        allGeometrics[i]=all[i];
}

simVoid _simMakeDynamicAnnouncement_internal(int announceType)
{
    TRACE_C_API;
    if (announceType==sim_announce_pureconenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureConeNotSupported();
    if (announceType==sim_announce_purespheroidnotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureSpheroidNotSupported();
    if (announceType==sim_announce_newtondynamicrandommeshnotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_newtonDynamicRandomMeshNotSupported();
    if (announceType==sim_announce_containsnonpurenonconvexshapes)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsNonPureNonConvexShapes();
    if (announceType==sim_announce_containsstaticshapesondynamicconstruction)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsStaticShapesOnDynamicConstruction();
    if (announceType==sim_announce_purehollowshapenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureHollowShapeNotSupported();
    if (announceType==sim_announce_vortexpluginisdemo)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_vortexPluginIsDemo();
}

simVoid _simGetVerticesLocalFrame_internal(const simVoid* geometric,simFloat* pos,simFloat* quat)
{
    TRACE_C_API;
    C7Vector tr(((CMesh*)geometric)->getVerticeLocalFrame());
    tr.Q.getInternalData(quat);
    tr.X.getInternalData(pos);
}

const simFloat* _simGetHeightfieldData_internal(const simVoid* geometric,simInt* xCount,simInt* yCount,simFloat* minHeight,simFloat* maxHeight)
{
    TRACE_C_API;
    return(((CMesh*)geometric)->getHeightfieldData(xCount[0],yCount[0],minHeight[0],maxHeight[0]));
}

simVoid _simGetCumulativeMeshes_internal(const simVoid* geomInfo,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{
    TRACE_C_API;
    std::vector<float> vert;
    std::vector<int> ind;
    ((CMeshWrapper*)geomInfo)->getCumulativeMeshes(vert,&ind,nullptr);

    vertices[0]=new float[vert.size()];
    verticesSize[0]=(int)vert.size();
    for (size_t i=0;i<vert.size();i++)
        vertices[0][i]=vert[i];
    indices[0]=new int[ind.size()];
    indicesSize[0]=(int)ind.size();
    for (size_t i=0;i<ind.size();i++)
        indices[0][i]=ind[i];
}

simInt _simGetObjectID_internal(const simVoid* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getObjectHandle());
}

simVoid _simGetObjectLocalTransformation_internal(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    TRACE_C_API;
    C7Vector tr;
    if (excludeFirstJointTransformation)
        tr=((CSceneObject*)object)->getLocalTransformation();
    else
        tr=((CSceneObject*)object)->getFullLocalTransformation();
    tr.X.getInternalData(pos);
    tr.Q.getInternalData(quat);
}

simVoid _simSetObjectLocalTransformation_internal(simVoid* object,const simFloat* pos,const simFloat* quat,simFloat simTime)
{
    TRACE_C_API;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    ((CSceneObject*)object)->setLocalTransformation(tr);
}

simVoid _simGetObjectCumulativeTransformation_internal(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    TRACE_C_API;
    C7Vector tr;
    if (excludeFirstJointTransformation!=0)
        tr=((CSceneObject*)object)->getCumulativeTransformation();
    else
        tr=((CSceneObject*)object)->getFullCumulativeTransformation();
    if (pos!=nullptr)
        tr.X.getInternalData(pos);
    if (quat!=nullptr)
        tr.Q.getInternalData(quat);
}

const simVoid* _simGetGeomProxyFromShape_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(shape);
}

simFloat _simGetMass_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->getMass());
}

simBool _simIsShapeDynamicallyStatic_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getShapeIsDynamicallyStatic());
}

simVoid _simGetInitialDynamicVelocity_internal(const simVoid* shape,simFloat* vel)
{
    TRACE_C_API;
    ((CShape*)shape)->getInitialDynamicLinearVelocity().getInternalData(vel);
}

simVoid _simSetInitialDynamicVelocity_internal(simVoid* shape,const simFloat* vel)
{
    TRACE_C_API;
    ((CShape*)shape)->setInitialDynamicLinearVelocity(C3Vector(vel));
}

simVoid _simGetInitialDynamicAngVelocity_internal(const simVoid* shape,simFloat* angularVel)
{
    TRACE_C_API;
    ((CShape*)shape)->getInitialDynamicAngularVelocity().getInternalData(angularVel);
}

simVoid _simSetInitialDynamicAngVelocity_internal(simVoid* shape,const simFloat* angularVel)
{
    TRACE_C_API;
    ((CShape*)shape)->setInitialDynamicAngularVelocity(C3Vector(angularVel));
}

simBool _simGetStartSleeping_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getStartInDynamicSleeping());
}

simBool _simGetWasPutToSleepOnce_internal(const simVoid* shape)
{ // flag is set to true whenever called!!!
    TRACE_C_API;
    bool a=((CShape*)shape)->getRigidBodyWasAlreadyPutToSleepOnce();
    ((CShape*)shape)->setRigidBodyWasAlreadyPutToSleepOnce(true);
    return(a);
}

simBool _simIsShapeDynamicallyRespondable_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getRespondable());
}

simInt _simGetDynamicCollisionMask_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getDynamicCollisionMask());
}

const simVoid* _simGetLastParentForLocalGlobalCollidable_internal(const simVoid* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getLastParentForLocalGlobalRespondable());
}

simBool _simGetDynamicsFullRefreshFlag_internal(const simVoid* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getDynamicsResetFlag());
}

simVoid _simSetDynamicsFullRefreshFlag_internal(const simVoid* object,simBool flag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicsResetFlag(flag!=0,false);
}

const simVoid* _simGetParentObject_internal(const simVoid* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getParent());
}

simVoid _simDynReportObjectCumulativeTransformation_internal(simVoid* obj,const simFloat* pos,const simFloat* quat,simFloat simTime)
{ // obj is always a shape
    TRACE_C_API;
    CSceneObject* object=(CSceneObject*)obj;
    CSceneObject* parent=object->getParent();
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    if (parent!=nullptr)
    {
        if (parent->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)parent;
            C7Vector x(joint->getIntrinsicTransformation(false).getInverse()*joint->getCumulativeTransformation().getInverse()*tr*object->getLocalTransformation().getInverse());
            joint->setIntrinsicTransformationError(x);
        }
        else if (parent->getObjectType()==sim_object_forcesensor_type)
        {
            CForceSensor* sensor=(CForceSensor*)parent;
            C7Vector x(sensor->getCumulativeTransformation().getInverse()*tr*object->getLocalTransformation().getInverse());
            sensor->setIntrinsicTransformationError(x);
        }
        else
            App::currentWorld->sceneObjects->setObjectAbsolutePose(object->getObjectHandle(),tr,false);
    }
    else
        object->setLocalTransformation(tr);
}

simVoid _simSetObjectCumulativeTransformation_internal(simVoid* object,const simFloat* pos,const simFloat* quat,simBool keepChildrenInPlace)
{
    TRACE_C_API;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    App::currentWorld->sceneObjects->setObjectAbsolutePose(((CSceneObject*)object)->getObjectHandle(),tr,keepChildrenInPlace!=0);
}

simVoid _simSetShapeDynamicVelocity_internal(simVoid* shape,const simFloat* linear,const simFloat* angular,simFloat simTime)
{
    TRACE_C_API;
    ((CShape*)shape)->setDynamicVelocity(linear,angular);
}

simVoid _simGetAdditionalForceAndTorque_internal(const simVoid* shape,simFloat* force,simFloat* torque)
{
    TRACE_C_API;
    ((CShape*)shape)->getAdditionalForce().getInternalData(force);
    ((CShape*)shape)->getAdditionalTorque().getInternalData(torque);
}

simVoid _simClearAdditionalForceAndTorque_internal(const simVoid* shape)
{
    TRACE_C_API;
    ((CShape*)shape)->clearAdditionalForceAndTorque();
}

simBool _simGetJointPositionInterval_internal(const simVoid* joint,simFloat* minValue,simFloat* rangeValue)
{
    TRACE_C_API;
    if (minValue!=nullptr)
        minValue[0]=((CJoint*)joint)->getPositionMin();
    if (rangeValue!=nullptr)
        rangeValue[0]=((CJoint*)joint)->getPositionRange();
    return(!((CJoint*)joint)->getIsCyclic());
}

const simVoid* _simGetObject_internal(int objID)
{
    TRACE_C_API;
    return(App::currentWorld->sceneObjects->getObjectFromHandle(objID));
}

const simVoid* _simGetIkGroupObject_internal(int ikGroupID)
{
    TRACE_C_API;
    return(App::currentWorld->ikGroups->getObjectFromHandle(ikGroupID));
}

simInt _simMpHandleIkGroupObject_internal(const simVoid* ikGroup)
{
    TRACE_C_API;
    return(((CIkGroup_old*)ikGroup)->computeGroupIk(true));
}

simInt _simGetJointType_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getJointType());
}

simFloat _simGetDynamicMotorTargetPosition_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetPosition());
}

simFloat _simGetDynamicMotorTargetVelocity_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetVelocity());
}

simFloat _simGetDynamicMotorMaxForce_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetForce(false));
}

simFloat _simGetDynamicMotorUpperLimitVelocity_internal(const simVoid* joint)
{
    TRACE_C_API;
    float maxVelAccelJerk[3];
    ((CJoint*)joint)->getMaxVelAccelJerk(maxVelAccelJerk);
    return(maxVelAccelJerk[0]);
}

simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(simVoid* joint,simFloat pos,simFloat simTime)
{
    TRACE_C_API;
    ((CJoint*)joint)->setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(pos,simTime);
}

simVoid _simSetJointSphericalTransformation_internal(simVoid* joint,const simFloat* quat,simFloat simTime)
{
    TRACE_C_API;
    ((CJoint*)joint)->setSphericalTransformation(quat);
}

simVoid _simAddForceSensorCumulativeForcesAndTorques_internal(simVoid* forceSensor,const simFloat* force,const simFloat* torque,int totalPassesCount,simFloat simTime)
{
    TRACE_C_API;
    ((CForceSensor*)forceSensor)->addCumulativeForcesAndTorques(force,torque,totalPassesCount);
}

simVoid _simAddJointCumulativeForcesOrTorques_internal(simVoid* joint,simFloat forceOrTorque,int totalPassesCount,simFloat simTime)
{
    TRACE_C_API;
    ((CJoint*)joint)->addCumulativeForceOrTorque(forceOrTorque,totalPassesCount);
}

simInt _simGetObjectListSize_internal(simInt objType)
{
    TRACE_C_API;
    if (objType==sim_object_shape_type)
        return(int(App::currentWorld->sceneObjects->getShapeCount()));
    if (objType==sim_object_joint_type)
        return(int(App::currentWorld->sceneObjects->getJointCount()));
    if (objType==sim_handle_all)
        return(int(App::currentWorld->sceneObjects->getObjectCount())); // we put it also here for faster access!
    if (objType==sim_object_dummy_type)
        return(int(App::currentWorld->sceneObjects->getDummyCount()));
    if (objType==sim_object_octree_type)
        return(int(App::currentWorld->sceneObjects->getOctreeCount()));
    if (objType==sim_object_pointcloud_type)
        return(int(App::currentWorld->sceneObjects->getPointCloudCount()));
    if (objType==sim_object_graph_type)
        return(int(App::currentWorld->sceneObjects->getGraphCount()));
    if (objType==sim_object_camera_type)
        return(int(App::currentWorld->sceneObjects->getCameraCount()));
    if (objType==sim_object_proximitysensor_type)
        return(int(App::currentWorld->sceneObjects->getProximitySensorCount()));
    if (objType==sim_object_path_type)
        return(int(App::currentWorld->sceneObjects->getPathCount()));
    if (objType==sim_object_visionsensor_type)
        return(int(App::currentWorld->sceneObjects->getVisionSensorCount()));
    if (objType==sim_object_mill_type)
        return(int(App::currentWorld->sceneObjects->getMillCount()));
    if (objType==sim_object_forcesensor_type)
        return(int(App::currentWorld->sceneObjects->getForceSensorCount()));
    if (objType==sim_object_light_type)
        return(int(App::currentWorld->sceneObjects->getLightCount()));
    if (objType==sim_object_mirror_type)
        return(int(App::currentWorld->sceneObjects->getMirrorCount()));
    if (objType==-1)
        return(int(App::currentWorld->sceneObjects->getOrphanCount()));
    return(int(App::currentWorld->sceneObjects->getObjectCount()));
}

const simVoid* _simGetObjectFromIndex_internal(simInt objType,simInt index)
{
    TRACE_C_API;
    if (objType==sim_object_shape_type)
        return(App::currentWorld->sceneObjects->getShapeFromIndex(index));
    if (objType==sim_object_joint_type)
        return(App::currentWorld->sceneObjects->getJointFromIndex(index));
    if (objType==sim_handle_all)
        return(App::currentWorld->sceneObjects->getObjectFromIndex(index)); // we put it also here for faster access!
    if (objType==sim_object_dummy_type)
        return(App::currentWorld->sceneObjects->getDummyFromIndex(index));
    if (objType==sim_object_octree_type)
        return(App::currentWorld->sceneObjects->getOctreeFromIndex(index));
    if (objType==sim_object_pointcloud_type)
        return(App::currentWorld->sceneObjects->getPointCloudFromIndex(index));
    if (objType==sim_object_graph_type)
        return(App::currentWorld->sceneObjects->getGraphFromIndex(index));
    if (objType==sim_object_camera_type)
        return(App::currentWorld->sceneObjects->getCameraFromIndex(index));
    if (objType==sim_object_proximitysensor_type)
        return(App::currentWorld->sceneObjects->getProximitySensorFromIndex(index));
    if (objType==sim_object_path_type)
        return(App::currentWorld->sceneObjects->getPathFromIndex(index));
    if (objType==sim_object_visionsensor_type)
        return(App::currentWorld->sceneObjects->getVisionSensorFromIndex(index));
    if (objType==sim_object_mill_type)
        return(App::currentWorld->sceneObjects->getMillFromIndex(index));
    if (objType==sim_object_forcesensor_type)
        return(App::currentWorld->sceneObjects->getForceSensorFromIndex(index));
    if (objType==sim_object_light_type)
        return(App::currentWorld->sceneObjects->getLightFromIndex(index));
    if (objType==sim_object_mirror_type)
        return(App::currentWorld->sceneObjects->getMirrorFromIndex(index));
    if (objType==-1)
        return(App::currentWorld->sceneObjects->getOrphanFromIndex(index));
    return(App::currentWorld->sceneObjects->getObjectFromIndex(index));
}

simVoid _simSetDynamicSimulationIconCode_internal(simVoid* object,simInt code)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicSimulationIconCode(code);
}

simVoid _simSetDynamicObjectFlagForVisualization_internal(simVoid* object,simInt flag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicFlag(flag);
}

simInt _simGetTreeDynamicProperty_internal(const simVoid* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getTreeDynamicProperty());
}

simInt _simGetObjectType_internal(const simVoid* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getObjectType());
}

const simVoid** _simGetObjectChildren_internal(const simVoid* object,simInt* count)
{
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    count[0]=int(it->getChildCount());
    if (count[0]!=0)
        return((const void**)&it->getChildren()->at(0));
    return(nullptr);
}

simInt _simGetDummyLinkType_internal(const simVoid* dummy,simInt* linkedDummyID)
{
    TRACE_C_API;
    int dType=((CDummy*)dummy)->getLinkType();
    if (linkedDummyID!=nullptr)
        linkedDummyID[0]=((CDummy*)dummy)->getLinkedDummyHandle();
    return(dType);
}

simInt _simGetJointMode_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getJointMode());
}

simBool _simIsJointInHybridOperation_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getHybridFunctionality_old());
}

simVoid _simDisableDynamicTreeForManipulation_internal(const simVoid* object,simBool disableFlag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->temporarilyDisableDynamicTree();
}

simVoid _simSetJointPosition_internal(const simVoid* joint,simFloat pos)
{
    TRACE_C_API;
    ((CJoint*)joint)->setPosition(pos,false);
}

simFloat _simGetJointPosition_internal(const simVoid* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getPosition());
}

simVoid _simSetDynamicMotorPositionControlTargetPosition_internal(const simVoid* joint,simFloat pos)
{
    TRACE_C_API;
    ((CJoint*)joint)->setTargetPosition(pos);
}

simVoid _simGetGravity_internal(simFloat* gravity)
{
    TRACE_C_API;
    App::currentWorld->dynamicsContainer->getGravity().getInternalData(gravity);
}

simInt _simGetTimeDiffInMs_internal(simInt previousTime)
{
    TRACE_C_API;
    return(VDateTime::getTimeDiffInMs(previousTime));
}

simBool _simDoEntitiesCollide_internal(simInt entity1ID,simInt entity2ID,simInt* cacheBuffer,simBool overrideCollidableFlagIfShape1,simBool overrideCollidableFlagIfShape2,simBool pathOrMotionPlanningRoutineCalling)
{
    TRACE_C_API;
    return(CCollisionRoutine::doEntitiesCollide(entity1ID,entity2ID,nullptr,overrideCollidableFlagIfShape1!=0,overrideCollidableFlagIfShape2!=0,nullptr));
}

simBool _simGetDistanceBetweenEntitiesIfSmaller_internal(simInt entity1ID,simInt entity2ID,simFloat* distance,simFloat* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{
    TRACE_C_API;
    return(CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1ID,entity2ID,distance[0],ray,cacheBuffer,cacheBuffer+2,overrideMeasurableFlagIfNonCollection1!=0,overrideMeasurableFlagIfNonCollection2!=0));
}

simInt _simHandleJointControl_internal(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simFloat* inputValuesFloat,simFloat* outputValues)
{
    TRACE_C_API;
    return(((CJoint*)joint)->handleDynJoint((auxV&1)!=0,inputValuesInt[0],inputValuesInt[1],inputValuesFloat[0],inputValuesFloat[1],inputValuesFloat[2],inputValuesFloat[3],outputValues));
}

simInt _simGetJointCallbackCallOrder_internal(const simVoid* joint)
{
    TRACE_C_API;
    int retVal=sim_scriptexecorder_normal;
    CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,((CJoint*)joint)->getObjectHandle());
    if (it!=nullptr)
        retVal=it->getExecutionPriority();
    else
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,((CJoint*)joint)->getObjectHandle());
        if (it!=nullptr)
            retVal=it->getExecutionPriority();
    }
    return(retVal);
}

simInt _simGetJointDynCtrlMode_internal(const simVoid* joint)
{
    TRACE_C_API;
    int retVal=((CJoint*)joint)->getDynCtrlMode();
    return(retVal);
}

simInt _simHandleCustomContact_internal(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simFloat* dataFloat)
{ // Careful with this function: it can also be called from any other thread (e.g. generated by the physics engine)
    TRACE_C_API;

    // 1. We handle the new calling method:
    if ( ((engine&1024)==0)&&App::currentWorld->embeddedScriptContainer->isContactCallbackFunctionAvailable() ) // the engine flag 1024 means: the calling thread is not the simulation thread. We would have problems with the scripts
    {
        CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
        inStack->pushTableOntoStack();
        inStack->insertKeyInt32IntoStackTable("handle1",objHandle1);
        inStack->insertKeyInt32IntoStackTable("handle2",objHandle2);
        inStack->insertKeyInt32IntoStackTable("engine",engine);
        CInterfaceStack* outStack=App::worldContainer->interfaceStackContainer->createStack();
        int retInfo=0;
        App::currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_childscript,sim_syscb_contactcallback,inStack,outStack,&retInfo);
        if (retInfo>0)
            App::currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_contactcallback,inStack,outStack,&retInfo);
        App::worldContainer->interfaceStackContainer->destroyStack(inStack);

        bool ignoreContact;
        if (outStack->getStackMapBoolValue("ignoreContact",ignoreContact))
        {
            dataInt[0]=0;
            if (!ignoreContact)
            {
                bool collisionResponse=false;
                outStack->getStackMapBoolValue("collisionResponse",collisionResponse);
                if (collisionResponse)
                {
                    if (engine==sim_physics_ode)
                    {
                        outStack->getStackMapInt32Value("ode.maxContacts",dataInt[1]);
                        outStack->getStackMapInt32Value("ode.contactMode",dataInt[2]);
                    }
                    if (engine==sim_physics_bullet)
                    {
                        outStack->getStackMapFloatValue("bullet.friction",dataFloat[0]);
                        outStack->getStackMapFloatValue("bullet.restitution",dataFloat[1]);
                    }
                    if (engine==sim_physics_ode)
                    {
                        outStack->getStackMapFloatValue("ode.mu",dataFloat[0]);
                        outStack->getStackMapFloatValue("ode.mu2",dataFloat[1]);
                        outStack->getStackMapFloatValue("ode.bounce",dataFloat[2]);
                        outStack->getStackMapFloatValue("ode.bounceVel",dataFloat[3]);
                        outStack->getStackMapFloatValue("ode.softCfm",dataFloat[4]);
                        outStack->getStackMapFloatValue("ode.softErp",dataFloat[5]);
                        outStack->getStackMapFloatValue("ode.motion1",dataFloat[6]);
                        outStack->getStackMapFloatValue("ode.motion2",dataFloat[7]);
                        outStack->getStackMapFloatValue("ode.motionN",dataFloat[8]);
                        outStack->getStackMapFloatValue("ode.slip1",dataFloat[9]);
                        outStack->getStackMapFloatValue("ode.slip2",dataFloat[10]);
                        outStack->getStackMapFloatArray("ode.fDir1",dataFloat+11,3);
                    }
                    if (engine==sim_physics_vortex)
                    {
                        //outStack->getStackMapFloatValue("vortex.xxxx",dataFloat[0]);
                    }
                    if (engine==sim_physics_newton)
                    {
                        outStack->getStackMapFloatValue("newton.staticFriction",dataFloat[0]);
                        outStack->getStackMapFloatValue("newton.kineticFriction",dataFloat[1]);
                        outStack->getStackMapFloatValue("newton.restitution",dataFloat[2]);
                    }
                    if (engine==sim_physics_mujoco)
                    {
                    }
                    App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                    return(1); // collision
                }
                else
                {
                    App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                    return(0); // no collision
                }
            }
        }
        App::worldContainer->interfaceStackContainer->destroyStack(outStack);
    }
    return(-1); // we let CoppeliaSim handle the contact
}

simFloat _simGetPureHollowScaling_internal(const simVoid* geometric)
{
    TRACE_C_API;
    return(((CMesh*)geometric)->getPurePrimitiveInsideScaling_OLD());
}

simVoid _simDynCallback_internal(const simInt* intData,const simFloat* floatData)
{
    TRACE_C_API;

    if (App::currentWorld->embeddedScriptContainer->isDynCallbackFunctionAvailable())
    { // to make it a bit faster than blindly parsing the whole object hierarchy
        CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
        inStack->pushTableOntoStack();

        inStack->insertKeyInt32IntoStackTable("passCnt",intData[1]);
        inStack->insertKeyInt32IntoStackTable("totalPasses",intData[2]);
        inStack->insertKeyFloatIntoStackTable("dynStepSize",floatData[0]);
        inStack->insertKeyBoolIntoStackTable("afterStep",intData[3]!=0);
        App::currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_childscript,sim_syscb_dyncallback,inStack,nullptr,nullptr);
        App::currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_dyncallback,inStack,nullptr,nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(inStack);
    }
}


//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************

simInt simGetMaterialId_internal(const simChar* materialName)
{ // DEPRECATED since 29/10/2016.
    TRACE_C_API;
    // For backward compatibility (28/10/2016)
    // We now do not share materials anymore: each shape has its own material, so
    // the material of a shape is identified by the shape handle itself
    return(-1);
}

simInt simGetShapeMaterial_internal(simInt shapeHandle)
{ // DEPRECATED since 29/10/2016.
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        if (isShape(__func__,shapeHandle))
        { // since 28/10/2016 there is no more sharing of materials. So each shape has an individual material.
          // Here we return simply the shape handle, which can be used for simSetShapeMaterial!
            retVal=shapeHandle;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleVarious_internal()
{ // DEPRECATED since 29/10/2016. use simHandleSimulationStart and simHandleSensingStart instead!
    // HandleVarious should be the last function call before the sensing phase
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        // Following is for camera tracking!
        for (size_t i=0;i<App::currentWorld->sceneObjects->getCameraCount();i++)
        {
            CCamera*  it=App::currentWorld->sceneObjects->getCameraFromIndex(i);
            it->handleTrackingAndHeadAlwaysUp();
        }

        // Following is for velocity measurement:
        float dt=float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_us())/1000000.0f;
        float t=dt+float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            App::currentWorld->sceneObjects->getJointFromIndex(i)->measureJointVelocity(t);
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            App::currentWorld->sceneObjects->getObjectFromIndex(i)->measureVelocity(dt);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetMpConfigForTipPose_internal(simInt motionPlanningObjectHandle,simInt options,simFloat closeNodesDistance,simInt trialCount,const simFloat* tipPose,simInt maxTimeInMs,simFloat* outputJointPositions,const simFloat* referenceConfigs,simInt referenceConfigCount,const simFloat* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{ // DEPRECATED since 21/1/2016   referenceConfigs can be nullptr, as well as jointWeights
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simResetPath_internal(simInt pathHandle)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (pathHandle!=sim_handle_all)&&(pathHandle!=sim_handle_all_except_explicit) )
        {
            if (!isPath(__func__,pathHandle))
            {
                return(-1);
            }
        }
        if (pathHandle>=0)
        { // Explicit handling
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetPath();
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
            {
                CPath_old* p=App::currentWorld->sceneObjects->getPathFromIndex(i);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->resetPath();
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandlePath_internal(simInt pathHandle,simFloat deltaTime)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (pathHandle!=sim_handle_all)&&(pathHandle!=sim_handle_all_except_explicit) )
        {
            if (!isPath(__func__,pathHandle))
            {
                return(-1);
            }
        }
        if (pathHandle>=0)
        { // explicit handling
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handlePath(deltaTime);
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
            {
                CPath_old* p=App::currentWorld->sceneObjects->getPathFromIndex(i);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->handlePath(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetJoint_internal(simInt jointHandle)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
                return(-1);
        }
        if (jointHandle>=0)
        { // Explicit handling
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetJoint_DEPRECATED();
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* p=App::currentWorld->sceneObjects->getJointFromIndex(i);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->resetJoint_DEPRECATED();
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleJoint_internal(simInt jointHandle,simFloat deltaTime)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
                return(-1);
        }
        if (jointHandle>=0)
        { // explicit handling
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handleJoint_DEPRECATED(deltaTime);
        }
        else
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* p=App::currentWorld->sceneObjects->getJointFromIndex(i);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->handleJoint_DEPRECATED(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathPlanningHandle_internal(const simChar* pathPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    std::string pathPlanningObjectNameAdjusted=getIndexAdjustedObjectName(pathPlanningObjectName);
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetMotionPlanningHandle_internal(const simChar* motionPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simFloat* simFindMpPath_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

simFloat* simSimplifyMpPath_internal(simInt motionPlanningObjectHandle,const simFloat* pathBuffer,simInt configCnt,simInt options,simFloat stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

simFloat* simFindIkPath_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalPose,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

simFloat* simGetMpConfigTransition_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,const simInt* select,simFloat calcStepSize,simFloat maxOutStepSize,simInt wayPointCnt,const simFloat* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(nullptr);
}

simInt simCreateMotionPlanning_internal(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simFloat* jointMetricWeights,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simRemoveMotionPlanning_internal(simInt motionPlanningHandle)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simSearchPath_internal(simInt pathPlanningObjectHandle,simFloat maximumSearchTime)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=0;
        if (it->performSearch(false,maximumSearchTime))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simInitializePathSearch_internal(simInt pathPlanningObjectHandle,simFloat maximumSearchTime,simFloat searchTimeStep)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        if (App::currentWorld->pathPlanning->getTemporaryPathSearchObjectCount()>100)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TOO_MANY_TEMP_OBJECTS);
            return(-1);
        }

        maximumSearchTime=tt::getLimitedFloat(0.01f,36000.0f,maximumSearchTime);
        searchTimeStep=tt::getLimitedFloat(0.001f,std::min<float>(1.0f,maximumSearchTime),searchTimeStep);
        CPathPlanningTask* oldIt=it;
        it=oldIt->copyYourself(); // we copy it because the original might be destroyed at any time
        it->setOriginalTask(oldIt);
        int retVal=-1; // error
        if (it->initiateSteppedSearch(false,maximumSearchTime,searchTimeStep))
            retVal=App::currentWorld->pathPlanning->addTemporaryPathSearchObject(it);
        else
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_NOT_CONSISTENT);
            delete it;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPerformPathSearchStep_internal(simInt temporaryPathSearchObject,simBool abortSearch)
{ // DEPRECATED since release 3.3.0
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::currentWorld->pathPlanning->getTemporaryPathSearchObject(temporaryPathSearchObject);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_TEMP_PATH_SEARCH_OBJECT_INEXISTANT);
            return(-1);
        }
        if (abortSearch)
        {
            App::currentWorld->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            delete it;
            return(0);
        }
        int retVal=it->performSteppedSearch();
        if (retVal!=-2)
        {
            App::currentWorld->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            CPathPlanningTask* originalIt=it->getOriginalTask();
            int tree1Handle,tree2Handle;
            it->getAndDisconnectSearchTrees(tree1Handle,tree2Handle); // to keep trees visible!
            delete it;
            // Now we connect the trees only if the originalTask still exists:
            bool found=false;
            for (int ot=0;ot<int(App::currentWorld->pathPlanning->allObjects.size());ot++)
            {
                if (App::currentWorld->pathPlanning->allObjects[ot]==originalIt)
                {
                    found=true;
                    break;
                }
            }
            if (found)
                originalIt->connectExternalSearchTrees(tree1Handle,tree2Handle);
            else
            {
                App::currentWorld->drawingCont->removeObject(tree1Handle);
                App::currentWorld->drawingCont->removeObject(tree2Handle);
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLockInterface_internal(simBool locked)
{ // DEPRECATED since release 3.1.0
    return(0);
}

simInt simCopyPasteSelectedObjects_internal()
{ // deprecated since 3.1.3
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        if (sel.size()>0)
        {
            if (fullModelCopyFromApi)
                CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            App::worldContainer->copyBuffer->memorizeBuffer();
            App::worldContainer->copyBuffer->copyCurrentSelection(&sel,App::currentWorld->environment->getSceneLocked(),0);
            App::currentWorld->sceneObjects->deselectObjects();
            App::worldContainer->copyBuffer->pasteBuffer(App::currentWorld->environment->getSceneLocked(),3);
            App::worldContainer->copyBuffer->restoreBuffer();
            App::worldContainer->copyBuffer->clearMemorizedBuffer();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSerialPortOpen_internal(simInt portNumber,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::worldContainer->serialPortContainer->serialPortOpen_old(false,portNumber,baudRate))
        return(1);
#endif
    return(-1);
}

simInt simSerialPortClose_internal(simInt portNumber)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::worldContainer->serialPortContainer->serialPortClose_old(portNumber))
        return(1);
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(-1);
}

simInt simSerialPortSend_internal(simInt portNumber,const simChar* data,simInt dataLength)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::worldContainer->serialPortContainer->serialPortSend_old(portNumber,data,dataLength);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

simInt simSerialPortRead_internal(simInt portNumber,simChar* buffer,simInt dataLengthToRead)
{ // deprecated (10/04/2012)
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::worldContainer->serialPortContainer->serialPortReceive_old(portNumber,buffer,dataLengthToRead);
    if (retVal==-1)
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

simInt simAppendScriptArrayEntry_internal(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* arrayNameAtScriptName,const simChar* keyName,const simChar* data,const simInt* what)
{ // deprecated (23/02/2016)
    TRACE_C_API;
    CScriptObject* script=nullptr;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string arrayName;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string arrNameAtScriptName(arrayNameAtScriptName);
            size_t p=arrNameAtScriptName.find('@');
            if (p!=std::string::npos)
                arrayName.assign(arrNameAtScriptName.begin(),arrNameAtScriptName.begin()+p);
            else
                arrayName=arrNameAtScriptName;
            script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
        }
        else
        {
            if (reservedSetToNull==nullptr)
            { // this can be the old or new way of doing it (the old way was active only 2 months, and not officially):
                std::string scriptName;
                std::string arrNameAtScriptName(arrayNameAtScriptName);
                size_t p=arrNameAtScriptName.find('@');
                if (p!=std::string::npos)
                {
                    scriptName.assign(arrNameAtScriptName.begin()+p+1,arrNameAtScriptName.end());
                    arrayName.assign(arrNameAtScriptName.begin(),arrNameAtScriptName.begin()+p);
                }
                else
                    arrayName=arrNameAtScriptName;

                if (scriptHandleOrType==sim_scripttype_mainscript) // new and old way (same coding)
                    script=App::currentWorld->embeddedScriptContainer->getMainScript();
                if (scriptHandleOrType==sim_scripttype_addonscript)
                {
                    if (scriptName.size()>0)
                        script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
                }
                if (scriptHandleOrType==sim_scripttype_sandboxscript)
                    script=App::worldContainer->sandboxScript;
                if (scriptHandleOrType==sim_scripttype_childscript)
                {
                    if (scriptName.size()>0)
                    { // new way
                        int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                    }
                }
                if (scriptHandleOrType==sim_scripttype_customizationscript)
                { // new way only possible (6 was not available in the old way)
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                }
            }
            else
            { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
                arrayName=arrayNameAtScriptName;
                if (scriptHandleOrType==0) // main script
                    script=App::currentWorld->embeddedScriptContainer->getMainScript();
                if (scriptHandleOrType==3) // child script
                {
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
                if (scriptHandleOrType==5) // customization
                {
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                }
            }
        }

        if (script!=nullptr)
        {
            int retVal=script->appendTableEntry_DEPRECATED(arrayName.c_str(),keyName,data,what);
            if (retVal==-1)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearScriptVariable_internal(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* variableNameAtScriptName)
{ // DEPRECATED (23/02/2016)
    TRACE_C_API;
    CScriptObject* script=nullptr;

    std::string variableName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string varNameAtScriptName(variableNameAtScriptName);
        size_t p=varNameAtScriptName.find('@');
        if (p!=std::string::npos)
            variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
        else
            variableName=varNameAtScriptName;
        script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
    }
    else
    {
        if (reservedSetToNull==nullptr)
        { // this can be the old or new way of doing it (the old way was active only 2 months, and not officially):
            std::string scriptName;
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(varNameAtScriptName.begin()+p+1,varNameAtScriptName.end());
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            }
            else
                variableName=varNameAtScriptName;

            if (scriptHandleOrType==sim_scripttype_mainscript) // new and old way (same coding)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::worldContainer->sandboxScript;
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                if (scriptName.size()>0)
                { // new way
                    int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            { // new way only possible (6 was not available in the old way)
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            variableName=variableNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=script->clearScriptVariable_DEPRECATED(variableName.c_str());
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

simVoid _simGetVortexParameters_internal(const simVoid* object,simInt version,simFloat* floatParams,simInt* intParams)
{ // if object is nullptr, we return general engine settings, if object is a shape, we return shape settings, otherwise joint settings
    // Version allows to adjust for future extensions.
    TRACE_C_API;
    std::vector<float> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::currentWorld->dynamicsContainer->getVortexFloatParams(fparams);
        App::currentWorld->dynamicsContainer->getVortexIntParams(iparams);
        if (version==0)
        {
            fcnt=10;
            icnt=1;
        }
        if (version==1)
        {
            fcnt=10;
            icnt=1;
        }
        if (version==2)
        {
            fcnt=10;
            icnt=1;
        }
        if (version>=3)
        { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
            fcnt=10;
            icnt=1;
        }
    }
    else
    {
        CSceneObject* obj=(CSceneObject*)object;
        if (obj->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)object;
            CDynMaterialObject* mat=shape->getDynMaterial();
            mat->getVortexFloatParams(fparams);
            mat->getVortexIntParams(iparams);
            if (version==0)
            {
                fcnt=32;
                icnt=8;
            }
            if (version==1)
            {
                fcnt=33;
                icnt=8;
            }
            if (version==2)
            {
                fcnt=36;
                icnt=8;
            }
            if (version>=3)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=36;
                icnt=8;
            }
        }
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)object;
            joint->getVortexFloatParams(fparams);
            joint->getVortexIntParams(iparams);
            if (version==0)
            {
                fcnt=47;
                icnt=4;
            }
            if (version==1)
            {
                fcnt=47;
                icnt=4;
            }
            if (version==2)
            {
                fcnt=47;
                icnt=4;
            }
            if (version>=3)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=51;
                icnt=7;
            }
        }
    }
    for (int i=0;i<fcnt;i++)
        floatParams[i]=fparams[i];
    for (int i=0;i<icnt;i++)
        intParams[i]=iparams[i];
}

simVoid _simGetNewtonParameters_internal(const simVoid* object,simInt* version,simFloat* floatParams,simInt* intParams)
{ // if object is nullptr, we return general engine settings, if object is a shape, we return shape settings, otherwise joint settings
    // Version allows to adjust for future extensions.
    TRACE_C_API;
    std::vector<float> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::currentWorld->dynamicsContainer->getNewtonFloatParams(fparams);
        App::currentWorld->dynamicsContainer->getNewtonIntParams(iparams);
        if (version[0]>=0)
        { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
            fcnt=2;
            icnt=2;
        }
        version[0]=0;
    }
    else
    {
        CSceneObject* obj=(CSceneObject*)object;
        if (obj->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)object;
            CDynMaterialObject* mat=shape->getDynMaterial();
            mat->getNewtonFloatParams(fparams);
            mat->getNewtonIntParams(iparams);
            if (version[0]>=0)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=5;
                icnt=1;
            }
            version[0]=0;
        }
        if (obj->getObjectType()==sim_object_joint_type)
        {
            CJoint* joint=(CJoint*)object;
            joint->getNewtonFloatParams(fparams);
            joint->getNewtonIntParams(iparams);
            if (version[0]>=0)
            { // when the dynamics plugin is the same version as CoppeliaSim, or newer!
                fcnt=2;
                icnt=2;
            }
            version[0]=0;
        }
    }
    for (int i=0;i<fcnt;i++)
        floatParams[i]=fparams[i];
    for (int i=0;i<icnt;i++)
        intParams[i]=iparams[i];
}

simVoid _simGetJointOdeParameters_internal(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* bounce,simFloat* fudge,simFloat* normalCFM)
{
    TRACE_C_API;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr);
    bounce[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_bounce,nullptr);
    fudge[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr);
}

simVoid _simGetJointBulletParameters_internal(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* normalCFM)
{
    TRACE_C_API;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr);
}

CShape* __getShapeFromGeomInfo(const simVoid* geomInfo)
{
    for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
    {
        CShape* sh=App::currentWorld->sceneObjects->getShapeFromIndex(i);
        if (sh->getMeshWrapper()==(CMeshWrapper*)geomInfo)
            return(sh);
    }
    return(nullptr);
}

simVoid _simGetOdeMaxContactFrictionCFMandERP_internal(const simVoid* geomInfo,simInt* maxContacts,simFloat* friction,simFloat* cfm,simFloat* erp)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    maxContacts[0]=mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr);
    friction[0]=mat->getEngineFloatParam(sim_ode_body_friction,nullptr);
    cfm[0]=mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr);
    erp[0]=mat->getEngineFloatParam(sim_ode_body_softerp,nullptr);
}

simBool _simGetBulletCollisionMargin_internal(const simVoid* geomInfo,simFloat* margin,simInt* otherProp)
{
    TRACE_C_API;
    CMeshWrapper* geomWrap=(CMeshWrapper*)geomInfo;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    bool convexAndNotPure=(geomWrap->isConvex()&&(!geomWrap->isPure()));
    if (convexAndNotPure)
        margin[0]=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,nullptr);
    else
        margin[0]=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,nullptr);

    if (otherProp!=nullptr)
    {
        otherProp[0]=0;
        if (mat->getEngineBoolParam(sim_bullet_body_autoshrinkconvex,nullptr))
            otherProp[0]|=1;
    }
    bool retVal=false;
    if (convexAndNotPure)
    {
        if (mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,nullptr))
            retVal=true;
    }
    else
    {
        if (mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,nullptr))
            retVal=true;
    }
    return(retVal);
}

simBool _simGetBulletStickyContact_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr));
}

simFloat _simGetBulletRestitution_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr));
}

simVoid _simGetDamping_internal(const simVoid* geomInfo,simFloat* linDamping,simFloat* angDamping)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (eng==sim_physics_bullet)
    {
        linDamping[0]=mat->getEngineFloatParam(sim_bullet_body_lineardamping,nullptr);
        angDamping[0]=mat->getEngineFloatParam(sim_bullet_body_angulardamping,nullptr);
    }
    if (eng==sim_physics_ode)
    {
        linDamping[0]=mat->getEngineFloatParam(sim_ode_body_lineardamping,nullptr);
        angDamping[0]=mat->getEngineFloatParam(sim_ode_body_angulardamping,nullptr);
    }
}

simFloat _simGetFriction_internal(const simVoid* geomInfo)
{
    TRACE_C_API;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
    if (eng==sim_physics_bullet)
        return(mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr));
    if (eng==sim_physics_ode)
        return(mat->getEngineFloatParam(sim_ode_body_friction,nullptr));
    return(0.0f);
}

simInt simAddSceneCustomData_internal(simInt header,const simChar* data,simInt dataLength)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        App::currentWorld->customSceneData.setData(hh.c_str(),data,dataLength);
        // ---------------------- Old -----------------------------
        App::currentWorld->customSceneData_old->setData(header,data,dataLength);
        // ---------------------- Old -----------------------------
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetSceneCustomDataLength_internal(simInt header)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string data=App::currentWorld->customSceneData.getData(hh.c_str());
        int retVal=int(data.size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetSceneCustomData_internal(simInt header,simChar* data)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string hh("_oldSceneCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string dat=App::currentWorld->customSceneData.getData(hh.c_str());
        for (size_t i=0;i<dat.size();i++)
            data[i]=dat[i];
        App::currentWorld->customSceneData_old->getData(header,data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



simInt simAddObjectCustomData_internal(simInt objectHandle,simInt header,const simChar* data,simInt dataLength)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        it->writeCustomDataBlock(false,hh.c_str(),data,dataLength);
        // ---------------------- Old -----------------------------
        it->setObjectCustomData_old(header,data,dataLength);
        // ---------------------- Old -----------------------------
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectCustomDataLength_internal(simInt objectHandle,simInt header)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string data=it->readCustomDataBlock(false,hh.c_str());
        int retVal=int(data.size());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectCustomData_internal(simInt objectHandle,simInt header,simChar* data)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::string hh("_oldObjectCustomData_");
        hh+=std::to_string(header);
        hh+="_";
        std::string dat=it->readCustomDataBlock(false,hh.c_str());
        for (size_t i=0;i<dat.size();i++)
            data[i]=dat[i];
        App::currentWorld->customSceneData_old->getData(header,data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLoadUI_internal(const simChar* filename,int maxCount,int* uiHandles)
{
    TRACE_C_API;
    return(-1);
}

simInt simCreateUI_internal(const simChar* elementName,simInt menuAttributes,const simInt* clientSize,const simInt* cellSize,simInt* buttonHandles)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int s[2]={clientSize[0],clientSize[1]};
        if (menuAttributes!=0)
            s[1]++;
        int b=0;
        for (int i=0;i<8;i++)
        {
            if (menuAttributes&(1<<i))
                b++;
        }
        if (b>s[0])
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DIMENSIONS);
            return(-1);
        }
        CButtonBlock* it=new CButtonBlock(s[0],s[1],cellSize[0],cellSize[1],-1);
        it->setBlockName(elementName);
        if ((menuAttributes&sim_ui_menu_systemblock)!=0)
            it->setAttributes(it->getAttributes()|sim_ui_property_systemblock);
        App::currentWorld->buttonBlockContainer->insertBlock(it,false);
        int retVal=it->getBlockID();
        int retHandlesP=0;
        float white[3]={1.0f,1.0f,1.0f};
        if (menuAttributes&sim_ui_menu_title)
        { // We have a title bar:
            int p[2]={0,0};
            int s2[2]={s[0]-b+1,1};
            float blue[3]={0.36f,0.35f,0.87f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_label|sim_buttonproperty_enabled|sim_buttonproperty_verticallycentered);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],blue,blue,white);
            retHandlesP++;
        }
        if (menuAttributes&sim_ui_menu_minimize)
        { // We have a minimize button:
            int p[2]={s[0]-b,0};
            if (menuAttributes&sim_ui_menu_title)
                p[0]++;
            int s2[2]={1,1};
            float blue[3]={0.18f,0.16f,0.84f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_staydown|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered|sim_buttonproperty_isdown|sim_buttonproperty_rollupaction);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],blue,blue,white);
            simSetUIButtonLabel_internal(retVal,buttonHandles[retHandlesP],"&&fg999&&Square","&&fg999&&Minimize");
            it->setRollupMin(VPoint(0,0));
            it->setRollupMax(VPoint(s[0],0));
            retHandlesP++;
        }
        if (menuAttributes&sim_ui_menu_close)
        { // We have a close button:
            int p[2]={s[0]-1,0};
            int s2[2]={1,1};
            float red[3]={0.84f,0.16f,0.17f};
            buttonHandles[retHandlesP]=simCreateUIButton_internal(retVal,p,s2,sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered|sim_buttonproperty_closeaction);
            simSetUIButtonColor_internal(retVal,buttonHandles[retHandlesP],red,red,white);
            simSetUIButtonLabel_internal(retVal,buttonHandles[retHandlesP],"&&fg999&&Check","");
            retHandlesP++;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateUIButton_internal(simInt elementHandle,const simInt* position,const simInt* size,simInt buttonProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=new CSoftButton("",position[0],position[1],size[0],size[1]);
        if (!it->insertButton(but))
        {
            delete but;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_WRONG_POS_SIZE_PARAMS);
            return(-1);
        }
        int retVal=but->buttonID;
        if (simSetUIButtonProperty_internal(elementHandle,retVal,buttonProperty)==-1)
            retVal=-1; // should not happen!
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetUIHandle_internal(const simChar* elementName)
{
    TRACE_C_API;

    std::string elementNameAdjusted=getIndexAdjustedObjectName(elementName);
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithName(elementNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        int retVal=it->getBlockID();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIProperty_internal(simInt elementHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        int retVal=it->getAttributes();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIEventButton_internal(simInt elementHandle,simInt* auxiliaryValues)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        int retVal=-1;
#ifdef SIM_WITH_GUI
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        retVal=it->getLastEventButtonID(auxiliaryValues);
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIProperty_internal(simInt elementHandle,simInt elementProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        // Following few new since 4/2/2013 (to bring newly made visible UI to the front)
        int attrib=it->getAttributes();
        it->setAttributes(elementProperty);
        int attribNew=it->getAttributes();
        if ( ((attrib&sim_ui_property_visible)==0)&&((attribNew&sim_ui_property_visible)!=0) )
            App::currentWorld->buttonBlockContainer->sendBlockToFront(it->getBlockID());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIButtonSize_internal(simInt elementHandle,simInt buttonHandle,simInt* size)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        size[0]=but->getLength();
        size[1]=but->getHeight();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetUIButtonProperty_internal(simInt elementHandle,simInt buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        int retVal=but->getAttributes();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonProperty_internal(simInt elementHandle,simInt buttonHandle,simInt buttonProperty)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->setAttributes(buttonProperty);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonLabel_internal(simInt elementHandle,simInt buttonHandle,const simChar* upStateLabel,const simChar* downStateLabel)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (upStateLabel!=nullptr)
            but->label=std::string(upStateLabel);
        if (downStateLabel!=nullptr)
            but->downLabel=std::string(downStateLabel);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetUIButtonLabel_internal(simInt elementHandle,simInt buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(nullptr);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        char* retVal=new char[but->label.length()+1];
        for (unsigned int i=0;i<but->label.length();i++)
            retVal[i]=but->label[i];
        retVal[but->label.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetUISlider_internal(simInt elementHandle,simInt buttonHandle,simInt position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        but->setSliderPos((float(position)/500.0f)-1.0f);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUISlider_internal(simInt elementHandle,simInt buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        int retVal=int((but->getSliderPos()+1.0f)*500.0f);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonColor_internal(simInt elementHandle,simInt buttonHandle,const simFloat* upStateColor,const simFloat* downStateColor,const simFloat* labelColor)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        for (int i=0;i<3;i++)
        {
            if (upStateColor!=nullptr)
                but->backgroundColor[i]=upStateColor[i];
            if (downStateColor!=nullptr)
                but->downBackgroundColor[i]=downStateColor[i];
            if (labelColor!=nullptr)
                but->textColor[i]=labelColor[i];
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRemoveUI_internal(simInt elementHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (elementHandle==sim_handle_all)
        {
            App::currentWorld->buttonBlockContainer->removeAllBlocks(false);
            return(1);
        }
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        App::currentWorld->buttonBlockContainer->removeBlockFromID(elementHandle);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateUIButtonArray_internal(simInt elementHandle,simInt buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetUIButtonArrayColor_internal(simInt elementHandle,simInt buttonHandle,const simInt* position,const simFloat* color)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (!but->setArrayColor(position[0],position[1],color))
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(-1);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDeleteUIButtonArray_internal(simInt elementHandle,simInt buttonHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetUIButtonTexture_internal(simInt elementHandle,simInt buttonHandle,const simInt* size,const simChar* textureData)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        CTextureProperty* tp=but->getTextureProperty();
        if (tp!=nullptr)
        { // We already have a texture. Is it the same size/type? or do we wanna remove the texture anyway?
            int tob=tp->getTextureObjectID();
            bool remove=true;
            if ((tob>=SIM_IDSTART_TEXTURE)&&(tob<=SIM_IDSTART_TEXTURE)&&(size!=nullptr))
            { // we have the correct type (i.e. non-vision sensor)
                CTextureObject* to=App::currentWorld->textureContainer->getObject(tob);
                if (to!=nullptr)
                {
                    int sizeX,sizeY;
                    to->getTextureSize(sizeX,sizeY);
                    if ( (size[0]==sizeX)&&(size[1]==sizeY) )
                    { // we just need to actualize the texture content:
                        to->setImage(false,false,true,(unsigned char*)textureData);
                        remove=false;
                    }
                }
            }
            if (remove)
            {
                App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(elementHandle,but->getUniqueID());
                delete tp;
                tp=nullptr;
                but->setTextureProperty(nullptr);
            }
        }
        if ((tp==nullptr)&&(size!=nullptr))
        { // add an existing texture
            CTextureObject* textureObj=new CTextureObject(size[0],size[1]);
            textureObj->setImage(false,false,true,(unsigned char*)textureData); // keep false,true
            textureObj->setObjectName("textureSetThroughAPI");
            textureObj->addDependentObject(it->getBlockID(),but->getUniqueID()); // Unique ID starts exceptionnally at 1
            int textureID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
            tp=new CTextureProperty(textureID);
            but->setTextureProperty(tp);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveUI_internal(int count,const int* uiHandles,const simChar* filename)
{
    TRACE_C_API;
    return(-1);
}

simInt simGetUIPosition_internal(simInt elementHandle,simInt* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);

        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        VPoint p;
        it->getBlockPositionAbsolute(p);
        position[0]=p.x;
        position[1]=p.y;
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIPosition_internal(simInt elementHandle,const simInt* position)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);
        CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(elementHandle);
        it->setDesiredBlockPosition(position[0],position[1]);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleGeneralCallbackScript_internal(simInt callbackId,simInt callbackTag,simVoid* additionalData)
{ // Deprecated since release 3.4.1
    TRACE_C_API;
    return(-1);
}

simInt simRegisterCustomLuaFunction_internal(const simChar* funcName,const simChar* callTips,const simInt* inputArgumentTypes,simVoid(*callBack)(struct SLuaCallBack* p))
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        bool retVal=1;
        if (App::worldContainer->scriptCustomFuncAndVarContainer->removeCustomFunction(funcName))
            retVal=0;// that function already existed. We remove it and replace it!
        std::vector<int> inputV;
        if (inputArgumentTypes!=nullptr)
        {
            for (int i=0;i<inputArgumentTypes[0];i++)
                inputV.push_back(inputArgumentTypes[i+1]);
        }
        CScriptCustomFunction* newFunction=new CScriptCustomFunction(funcName,callTips,inputV,callBack);
        if (!App::worldContainer->scriptCustomFuncAndVarContainer->insertCustomFunction(newFunction))
        {
            delete newFunction;
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRegisterContactCallback_internal(simInt(*callBack)(simInt,simInt,simInt,simInt*,simFloat*))
{ // deprecated. Disabled on 18.05.2022
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simRegisterJointCtrlCallback_internal(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simFloat*,simFloat*))
{ // deprecated. Disabled on 18.05.2022
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simGetMechanismHandle_internal(const simChar* mechanismName)
{ // deprecated
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simHandleMechanism_internal(simInt mechanismHandle)
{ // deprecated
    TRACE_C_API;
    CApiErrors::setLastWarningOrError(__func__,"not supported anymore.");
    return(-1);
}

simInt simHandleCustomizationScripts_internal(simInt callType)
{ // deprecated
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            retVal=App::currentWorld->embeddedScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,callType,nullptr,nullptr,nullptr);
            App::currentWorld->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customizationscript);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCallScriptFunction_internal(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,SLuaCallBack* data,const simChar* reservedSetToNull)
{ // DEPRECATED
    TRACE_C_API;
    CScriptObject* script=nullptr;

    std::string funcName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        else
            funcName=funcNameAtScriptName;
        script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
    }
    else
    { // script is identified by a script type and sometimes also a script name
        if (reservedSetToNull==nullptr)
        {
            std::string scriptName;
            std::string funcNameAtScriptName(functionNameAtScriptName);
            size_t p=funcNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(funcNameAtScriptName.begin()+p+1,funcNameAtScriptName.end());
                funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
            }
            else
                funcName=funcNameAtScriptName;
            if (scriptHandleOrType==sim_scripttype_mainscript)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==(sim_scripttype_childscript|sim_scripttype_threaded_old))||(scriptHandleOrType==sim_scripttype_customizationscript) )
            {
                int objId=-1;
                CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                if (obj!=nullptr)
                    objId=obj->getObjectHandle();
                else
                    objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                if (scriptHandleOrType==sim_scripttype_customizationscript)
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                else
                    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==sim_scripttype_addonscript)
                script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            funcName=functionNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(reservedSetToNull);
                script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=-1; // error
        if (script->getThreadedExecutionIsUnderWay_oldThreads())
        { // very special handling here!
            if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                retVal=script->callScriptFunction_DEPRECATED(funcName.c_str(),data);
            else
            { // we have to execute that function via another thread!
                void* d[4];
                int callType=0;
                d[0]=&callType;
                d[1]=script;
                d[2]=(void*)funcName.c_str();
                d[3]=data;

                retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
            }
        }
        else
        {
            if (VThread::isCurrentThreadTheMainSimulationThread())
            { // For now we don't allow non-main threads to call non-threaded scripts!
                retVal=script->callScriptFunction_DEPRECATED(funcName.c_str(),data);
            }
        }
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

simInt simSetVisionSensorFilter_internal(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simFloat* floats,const simUChar* custom)
{ // DEPRECATED
    TRACE_C_API;

    return(-1);
}

simInt simGetVisionSensorFilter_internal(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simFloat** floats,simUChar** custom)
{ // DEPRECATED
    TRACE_C_API;

    return(-1);
}

simChar* simGetScriptSimulationParameter_internal(simInt scriptHandle,const simChar* parameterName,simInt* parameterLength)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(scriptHandle);
        if ( (it!=nullptr)||(obj!=nullptr) )
        {
            if (obj==nullptr)
                obj=App::currentWorld->sceneObjects->getObjectFromHandle(it->getObjectHandleThatScriptIsAttachedTo(-1));
            if (obj!=nullptr)
            {
                CUserParameters* uso=obj->getUserScriptParameterObject();
                if (uso!=nullptr)
                {
                    std::string parameterValue;
                    if (uso->getParameterValue(parameterName,parameterValue))
                    {
                        char* retVal=new char[parameterValue.length()+1];
                        for (size_t i=0;i<parameterValue.length();i++)
                            retVal[i]=parameterValue[i];
                        retVal[parameterValue.length()]=0;
                        parameterLength[0]=(int)parameterValue.length();
                        return(retVal);
                    }
                }
            }
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetScriptSimulationParameter_internal(simInt scriptHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{ // DEPRECATED
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(scriptHandle);
        int retVal=-1;
        if ( (it!=nullptr)||(obj!=nullptr) )
        {
            if (obj==nullptr)
                obj=App::currentWorld->sceneObjects->getObjectFromHandle(it->getObjectHandleThatScriptIsAttachedTo(-1));
            if (obj!=nullptr)
            {
                retVal=0;
                CUserParameters* uso=obj->getUserScriptParameterObject();
                bool s=false;
                if (uso==nullptr)
                {
                    uso=new CUserParameters();
                    s=true;
                }
                uso->setParameterValue(parameterName,parameterValue,size_t(parameterLength));
                if (s)
                    obj->setUserScriptParameterObject(uso);
            }
        }
        if (retVal==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetNameSuffix_internal(const simChar* name)
{ // DEPRECATED in 2020
    TRACE_C_API;

    int suffixNumber;
    if (name!=nullptr)
        suffixNumber=tt::getNameSuffixNumber(name,true);
    else
        suffixNumber=getCurrentScriptNameIndex_cSide();
    return(suffixNumber);
}

simInt simSetNameSuffix_internal(simInt nameSuffixNumber)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (nameSuffixNumber<-1)
        nameSuffixNumber=-1;
    setCurrentScriptInfo_cSide(_currentScriptHandle,nameSuffixNumber);
    return(1);
}

simInt simAddStatusbarMessage_internal(const simChar* message)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (message!=nullptr)
        {
//            App::addStatusbarMessage(message,false);
            int v=sim_verbosity_msgs;
            if (std::string(message).compare(0,18,"Lua runtime error:")==0) // probably not used at all.
                v=sim_verbosity_errors;
            App::logScriptMsg(nullptr,v|sim_verbosity_undecorated,message);
        }
        else
            App::clearStatusbar();

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetScriptRawBuffer_internal(simInt scriptHandle,simInt bufferHandle)
{ // DEPRECATED in 2020
    return(nullptr);
}

simInt simSetScriptRawBuffer_internal(simInt scriptHandle,const simChar* buffer,simInt bufferSize)
{ // DEPRECATED in 2020
    return(-1);
}

simInt simReleaseScriptRawBuffer_internal(simInt scriptHandle,simInt bufferHandle)
{ // DEPRECATED in 2020
    return(-1);
}

simInt simSetShapeMassAndInertia_internal(simInt shapeHandle,simFloat mass,const simFloat* inertiaMatrix,const simFloat* centerOfMass,const simFloat* transformation)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        if (mass<0.0000001f)
            mass=0.0000001f;
        C3X3Matrix m;
        m.copyFromInterface(inertiaMatrix);
        m.axis[0](1)=m.axis[1](0);
        m.axis[0](2)=m.axis[2](0);
        m.axis[1](2)=m.axis[2](1);
        m/=mass; // in CoppeliaSim we work with the "massless inertia"
        it->getMeshWrapper()->setMass(mass);
        C3Vector com(centerOfMass);
        C4X4Matrix tr;
        if (transformation==nullptr)
            tr.setIdentity();
        else
            tr.copyFromInterface(transformation);

        C4Vector rot;
        C3Vector pmoment;
        CMeshWrapper::findPrincipalMomentOfInertia(m,rot,pmoment);
        if (pmoment(0)<0.0000001f)
            pmoment(0)=0.0000001f;
        if (pmoment(1)<0.0000001f)
            pmoment(1)=0.0000001f;
        if (pmoment(2)<0.0000001f)
            pmoment(0)=0.0000001f;
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
        it->getMeshWrapper()->setLocalInertiaFrame(it->getFullCumulativeTransformation().getInverse()*tr.getTransformation()*C7Vector(rot,com));
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeMassAndInertia_internal(simInt shapeHandle,simFloat* mass,simFloat* inertiaMatrix,simFloat* centerOfMass,const simFloat* transformation)
{ // DEPRECATED in 2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        mass[0]=it->getMeshWrapper()->getMass();
        //float mmm=it->geomInfo->getMass();

        C7Vector tr(it->getFullCumulativeTransformation()*it->getMeshWrapper()->getLocalInertiaFrame());
        C4X4Matrix ref;
        if (transformation==nullptr)
            ref.setIdentity();
        else
            ref.copyFromInterface(transformation);
        C3X3Matrix m(CMeshWrapper::getNewTensor(it->getMeshWrapper()->getPrincipalMomentsOfInertia(),ref.getTransformation().getInverse()*tr));
        m*=mass[0]; // in CoppeliaSim we work with the "massless inertia"
        m.copyToInterface(inertiaMatrix);
        (ref.getTransformation().getInverse()*tr).X.copyTo(centerOfMass);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckIkGroup_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat* jointValues,const simInt* jointOptions)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        int retVal=-1;
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        int r=it->checkIkGroup(jointCnt,jointHandles,jointValues,jointOptions);
        if (r==-1)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
        if (r==-2)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLES);
        if (r>=sim_ikresult_not_performed)
            retVal=r;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateIkGroup_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* ikGroup=new CIkGroup_old();
        ikGroup->setObjectName("IK_Group",false);
        App::currentWorld->ikGroups->addIkGroup(ikGroup,false);
        ikGroup->setEnabled((options&1)==0);
        ikGroup->setRestoreIfPositionNotReached((options&4)!=0);
        ikGroup->setRestoreIfOrientationNotReached((options&8)!=0);
        ikGroup->setIgnoreMaxStepSizes((options&16)==0);
        ikGroup->setExplicitHandling((options&32)!=0);
        if (intParams!=nullptr)
        {
            ikGroup->setCalculationMethod(intParams[0]);
            ikGroup->setMaxIterations(intParams[1]);
        }
        if (floatParams!=nullptr)
            ikGroup->setDampingFactor(floatParams[0]);
        return(ikGroup->getObjectHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveIkGroup_internal(simInt ikGroupHandle)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        App::currentWorld->ikGroups->removeIkGroup(it->getObjectHandle());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateIkElement_internal(simInt ikGroupHandle,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int tip=intParams[0];
        int base=intParams[1];
        int constrBase=intParams[2];
        int constraints=intParams[3];
        if (!isDummy(__func__,tip))
            return(-1);
        if (App::currentWorld->sceneObjects->getObjectFromHandle(base)==nullptr)
            base=-1;
        if (App::currentWorld->sceneObjects->getObjectFromHandle(constrBase)==nullptr)
            constrBase=-1;
        CIkElement_old* ikEl=new CIkElement_old(tip);
        ikEl->setEnabled((options&1)==0);
        ikEl->setBase(base);
        ikEl->setAlternativeBaseForConstraints(constrBase);
        ikEl->setConstraints(constraints);
        if (floatParams!=nullptr)
        {
            ikEl->setMinLinearPrecision(floatParams[0]);
            ikEl->setMinAngularPrecision(floatParams[1]);
            ikEl->setPositionWeight(floatParams[2]);
            ikEl->setOrientationWeight(floatParams[3]);
        }
        it->addIkElement(ikEl);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simExportIk_internal(const simChar* pathAndFilename,simInt reserved1,simVoid* reserved2)
{ // deprecated on 29.09.2020
    CApiErrors::setLastWarningOrError(__func__,"Not supported anymore. Use CoppeliaSim V4.2.0 or earlier.");
    return(-1);
}

simInt simComputeJacobian_internal(simInt ikGroupHandle,simInt options,simVoid* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        int returnValue=-1;
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it->computeOnlyJacobian(options))
            returnValue=0;
        return(returnValue);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetConfigForTipPose_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat thresholdDist,simInt maxTimeInMs,simFloat* retConfig,const simFloat* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simFloat* lowLimits,const simFloat* ranges,simVoid* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);

        CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        std::string err;
        int retVal=ikGroup->getConfigForTipPose(jointCnt,jointHandles,thresholdDist,maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,err);
        if (retVal<0)
            CApiErrors::setLastWarningOrError(__func__,err.c_str());
        return(retVal);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simGenerateIkPath_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(nullptr);
        std::vector<CJoint*> joints;
        CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        bool err=false;
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* aJoint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
            if (aJoint==nullptr)
                err=true;
            else
                joints.push_back(aJoint);
        }
        if (err)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLES);
        std::vector<CDummy*> tips;
        std::vector<CDummy*> targets;
        std::vector<C7Vector> startTrs;
        std::vector<C7Vector> goalTrs;
        if (!err)
        {
            if (ikGroup->getIkElementCount()>0)
            {
                for (size_t i=0;i<ikGroup->getIkElementCount();i++)
                {
                    CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                    CDummy* tip=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTipHandle());
                    CDummy* target=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTargetHandle());
                    if ((tip==nullptr)||(target==nullptr))
                        err=true;
                    tips.push_back(tip);
                    targets.push_back(target);
                    startTrs.push_back(tip->getFullCumulativeTransformation());
                    goalTrs.push_back(target->getFullCumulativeTransformation());
                }
            }
            else
            {
                err=true;
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            }
        }
        if (!err)
        {
            if (ptCnt<2)
            {
                err=true;
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            }
        }
        if ( (!err)&&(collisionPairCnt>0)&&(collisionPairs!=nullptr) )
        {
            for (int i=0;i<collisionPairCnt;i++)
            {
                if (collisionPairs[2*i+0]!=-1)
                {
                    if (!doesCollectionExist(__func__,collisionPairs[2*i+0]))
                        err=true;
                    else
                    {
                        if (collisionPairs[2*i+1]!=sim_handle_all)
                        {
                            if (!doesCollectionExist(__func__,collisionPairs[2*i+1]))
                                err=true;
                        }
                    }
                }
            }
        }
        if (!err)
        {
            // Save joint positions/modes (all of them, just in case)
            std::vector<CJoint*> sceneJoints;
            std::vector<float> initSceneJointValues;
            std::vector<int> initSceneJointModes;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            {
                CJoint* aj=App::currentWorld->sceneObjects->getJointFromIndex(i);
                sceneJoints.push_back(aj);
                initSceneJointValues.push_back(aj->getPosition());
                initSceneJointModes.push_back(aj->getJointMode());
            }

            ikGroup->setAllInvolvedJointsToNewJointMode(sim_jointmode_kinematic);

            bool ikGroupWasActive=ikGroup->getEnabled();
            if (!ikGroupWasActive)
                ikGroup->setEnabled(true);

            // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
            std::vector<bool> enabledElements;
            for (size_t i=0;i<ikGroup->getIkElementCount();i++)
            {
                CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                enabledElements.push_back(ikElement->getEnabled());
            }

            // Set the correct mode for the joints involved:
            for (int i=0;i<jointCnt;i++)
            {
                if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                    joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_ik_deprecated);
                else
                    joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_dependent);
            }

            // do the calculation:
            float t=0.0;
            float dt=1.0/(ptCnt-1);
            bool failed=false;
            std::vector<float> thePath;
            for (int iterCnt=0;iterCnt<ptCnt;iterCnt++)
            {
                for (size_t el=0;el<ikGroup->getIkElementCount();el++)
                { // set all targets to an interpolated pose
                    C7Vector tr;
                    tr.buildInterpolation(startTrs[el],goalTrs[el],t);
                    targets[el]->setAbsoluteTransformation(tr);
                }

                // Try to perform IK:
                if (sim_ikresult_success==ikGroup->computeGroupIk(true))
                {
                    bool colliding=false;
                    if ( (collisionPairCnt>0)&&(collisionPairs!=nullptr) )
                    { // we need to check if this state collides:
                        for (int i=0;i<collisionPairCnt;i++)
                        {
                            if (collisionPairs[2*i+0]>=0)
                            {
                                int env=collisionPairs[2*i+1];
                                if (env==sim_handle_all)
                                    env=-1;
                                if (CCollisionRoutine::doEntitiesCollide(collisionPairs[2*i+0],env,nullptr,false,false,nullptr))
                                {
                                    colliding=true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!colliding)
                    { // we save this path point
                        for (int i=0;i<jointCnt;i++)
                            thePath.push_back(joints[i]->getPosition());
                    }
                    else
                        failed=true;
                }
                else
                    failed=true;
                if (failed)
                    break;
                t+=dt;
            }

            if (!ikGroupWasActive)
                ikGroup->setEnabled(false);

            // Restore the IK element activation state:
            for (size_t i=0;i<ikGroup->getIkElementCount();i++)
            {
                CIkElement_old* ikElement=ikGroup->getIkElementFromIndex(i);
                ikElement->setEnabled(enabledElements[i]);
            }

            // Restore joint positions/modes:
            for (size_t i=0;i<sceneJoints.size();i++)
            {
                if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                    sceneJoints[i]->setPosition(initSceneJointValues[i],false);
                if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                    sceneJoints[i]->setJointMode_noDynMotorTargetPosCorrection(initSceneJointModes[i]);
            }

            // Restore target dummies:
            for (size_t el=0;el<ikGroup->getIkElementCount();el++)
                targets[el]->setAbsoluteTransformation(goalTrs[el]);

            if (!failed)
            {
                float* retVal=new float[jointCnt*ptCnt];
                for (int i=0;i<jointCnt*ptCnt;i++)
                    retVal[i]=thePath[i];
                return(retVal);
            }
            return(nullptr);
        }
        return(nullptr);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetIkGroupHandle_internal(const simChar* ikGroupName)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(ikGroupName).find("@silentError");
    std::string nm(ikGroupName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string ikGroupNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromName(ikGroupNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simGetIkGroupMatrix_internal(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(nullptr);
        }
        float* retData=nullptr;
        if (options==0)
            retData=it->getLastJacobianData(matrixSize);
        if (options==1)
            retData=it->getLastManipulabilityValue(matrixSize);
        return(retData);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simHandleIkGroup_internal(simInt ikGroupHandle)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (ikGroupHandle!=sim_handle_all)&&(ikGroupHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesIKGroupExist(__func__,ikGroupHandle))
                return(-1);
        }
        int returnValue=0;
        if (ikGroupHandle<0)
            returnValue=App::currentWorld->ikGroups->computeAllIkGroups(ikGroupHandle==sim_handle_all_except_explicit);
        else
        { // explicit handling
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            returnValue=it->computeGroupIk(false);
        }
        return(returnValue);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetIkGroupProperties_internal(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simFloat damping,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        it->setCalculationMethod(resolutionMethod);
        it->setMaxIterations(maxIterations);
        it->setDampingFactor(damping);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetIkElementProperties_internal(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simFloat* precision,const simFloat* weight,void* reserved)
{ // deprecated on 29.09.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        if (!isDummy(__func__,tipDummyHandle))
            return(-1);
        CIkElement_old* el=it->getIkElementFromTipHandle(tipDummyHandle);
        if (el==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            return(-1);
        }
        el->setConstraints(constraints);
        if (precision!=nullptr)
        {
            el->setMinLinearPrecision(precision[0]);
            el->setMinAngularPrecision(precision[1]);
        }
        if (weight!=nullptr)
        {
            el->setPositionWeight(weight[0]);
            el->setOrientationWeight(weight[1]);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetThreadIsFree_internal(simBool freeMode)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (VThread::isCurrentThreadTheMainSimulationThread())
    {

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_CANNOT_BE_CALLED_FROM_MAIN_THREAD);
        return(-1);
    }
    if (CThreadPool_old::setThreadFreeMode(freeMode!=0))
        return(1);
    return(0);
}

simInt simTubeOpen_internal(simInt dataHeader,const simChar* dataName,simInt readBufferSize,simBool notUsedButKeepFalse)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;
        retVal=App::currentWorld->commTubeContainer->openTube(dataHeader,dataName,false,readBufferSize);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simTubeClose_internal(simInt tubeHandle)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (tubeHandle&1)
        { // not used anymore
        }
        else
        {
            if (App::currentWorld->commTubeContainer->closeTube(tubeHandle))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simTubeWrite_internal(simInt tubeHandle,const simChar* data,simInt dataLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (tubeHandle&1)
        { // not used anymore
        }
        else
        {
            if (App::currentWorld->commTubeContainer->writeToTube_copyBuffer(tubeHandle,data,dataLength))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simTubeRead_internal(simInt tubeHandle,simInt* dataLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal;
        retVal=App::currentWorld->commTubeContainer->readFromTube_bufferNotCopied(tubeHandle,dataLength[0]);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simTubeStatus_internal(simInt tubeHandle,simInt* readPacketsCount,simInt* writePacketsCount)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int readP=0;
        int writeP=0;
        int retVal;
        retVal=App::currentWorld->commTubeContainer->getTubeStatus(tubeHandle,readP,writeP);
        if (readPacketsCount!=nullptr)
            readPacketsCount[0]=readP;
        if (writePacketsCount!=nullptr)
            writePacketsCount[0]=writeP;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSendData_internal(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simFloat actionRadius,simFloat emissionAngle1,simFloat emissionAngle2,simFloat persistence)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (App::currentWorld->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (targetID!=0)&&(targetID!=sim_handle_all) )
        {
            CScriptObject* it=App::worldContainer->getScriptFromHandle(targetID);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_TARGET_HANDLE);
                return(-1);
            }
        }
        if (dataHeader<0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_HEADER);
            return(-1);
        }
        if (strlen(dataName)<1)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_NAME);
            return(-1);
        }
        if (dataLength<1)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        if (antennaHandle!=sim_handle_default)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
                return(-1);
            }
        }
        actionRadius=tt::getLimitedFloat(0.0f,SIM_MAX_FLOAT,actionRadius);
        emissionAngle1=tt::getLimitedFloat(0.0f,piValue_f,emissionAngle1);
        emissionAngle2=tt::getLimitedFloat(0.0f,piValTimes2_f,emissionAngle2);
        persistence=tt::getLimitedFloat(0.0f,99999999999999.9f,persistence);
        if (persistence==0.0f)
            persistence=float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_us())*1.5f/1000000.0f;
        std::string datN(dataName);
        App::currentWorld->embeddedScriptContainer->broadcastDataContainer.broadcastData(0,targetID,dataHeader,datN,
                        float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f+persistence,actionRadius,antennaHandle,
                        emissionAngle1,emissionAngle2,data,dataLength);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReceiveData_internal(simInt dataHeader,const simChar* dataName,simInt antennaHandle,simInt index,simInt* dataLength,simInt* senderID,simInt* dataHeaderR,simChar** dataNameR)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (App::currentWorld->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (dataHeader<0)
            dataHeader=-1;
        if (dataName!=nullptr)
        {
            if (strlen(dataName)<1)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_NAME);
                return(nullptr);
            }
        }
        if (antennaHandle!=sim_handle_default)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
                return(nullptr);
            }
        }
        if (index<0)
            index=-1;
        std::string datNm;
        if (dataName!=nullptr)
            datNm=dataName;
        int theIndex=index;
        int theSenderID;
        int theDataHeader;
        std::string theDataName;
        char* data0=App::currentWorld->embeddedScriptContainer->broadcastDataContainer.receiveData(0,float(App::currentWorld->simulation->getSimulationTime_us())/1000000.0f,
                dataHeader,datNm,antennaHandle,dataLength[0],theIndex,theSenderID,theDataHeader,theDataName);
        char* retData=nullptr;
        if (data0!=nullptr)
        {
            retData=new char[dataLength[0]];
            for (int i=0;i<dataLength[0];i++)
                retData[i]=data0[i];
            if (senderID!=nullptr)
                senderID[0]=theSenderID;
            if (dataHeaderR!=nullptr)
                dataHeaderR[0]=theDataHeader;
            if (dataNameR!=nullptr)
            {
                dataNameR[0]=new char[theDataName.length()+1];
                for (int i=0;i<int(theDataName.length());i++)
                    dataNameR[0][i]=theDataName[i];
                dataNameR[0][theDataName.length()]=0; // terminal zero
            }
        }
        return(retData);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetDataOnPath_internal(simInt pathHandle,simFloat relativeDistance,simInt dataType,simInt* intData,simFloat* floatData)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        float auxChannels[4];
        int auxFlags;
        if (dataType==0)
        {
            if (relativeDistance>-0.5f)
            { // regular use of the function
                if (it->pathContainer->getAuxDataOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,auxFlags,auxChannels))
                {
                    intData[0]=auxFlags;
                    for (int i=0;i<4;i++)
                        floatData[i]=auxChannels[i];
                    return(1);
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                    return(-1);
                }
            }
            else
            { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
                CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
                if (ctrlPt!=nullptr)
                {
                    intData[0]=ctrlPt->getAuxFlags();
                    ctrlPt->getAuxChannels(floatData);
                    return(1);
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                    return(-1);
                }
            }
        }
        else
            return(-1);

    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetPositionOnPath_internal(simInt pathHandle,simFloat relativeDistance,simFloat* position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C7Vector tr;

        if (relativeDistance>-0.5f)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformation()*tr;
                tr.X.copyTo(position);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformation()*tr;
                tr.X.copyTo(position);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetOrientationOnPath_internal(simInt pathHandle,simFloat relativeDistance,simFloat* eulerAngles)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C7Vector tr;
        if (relativeDistance>-0.5f)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformation()*tr;
                C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint_old* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformation()*tr;
                C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetClosestPositionOnPath_internal(simInt pathHandle,simFloat* absolutePosition,simFloat* pathPosition)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        C3Vector p(absolutePosition);
        if (it->pathContainer->getPositionOnPathClosestTo(p,*pathPosition))
        {
            float pl=it->pathContainer->getBezierVirtualPathLength();
            if (pl!=0.0f)
                *pathPosition/=pl;
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_EMPTY);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathPosition_internal(simInt objectHandle,simFloat* position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        position[0]=float(it->pathContainer->getPosition());
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetPathPosition_internal(simInt objectHandle,simFloat position)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        it->pathContainer->setPosition(position);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathLength_internal(simInt objectHandle,simFloat* length)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        length[0]=it->pathContainer->getBezierVirtualPathLength();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreatePath_internal(simInt attributes,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* newObject=new CPath_old();
        App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
        if (attributes!=-1)
            newObject->pathContainer->setAttributes(attributes);

        if (intParams!=nullptr)
        {
            newObject->pathContainer->setLineSize(intParams[0]);
            newObject->pathContainer->setPathLengthCalculationMethod(intParams[1]);
        }

        if (floatParams!=nullptr)
        {
            newObject->pathContainer->setSquareSize(floatParams[0]);
            newObject->pathContainer->setAngleVarToDistanceCoeff(floatParams[1]);
            newObject->pathContainer->setOnSpotDistanceToDistanceCoeff(floatParams[2]);
        }

        if (color!=nullptr)
        {
            newObject->pathContainer->_lineColor.setColor(color+0,sim_colorcomponent_ambient_diffuse);
            newObject->pathContainer->_lineColor.setColor(color+6,sim_colorcomponent_specular);
            newObject->pathContainer->_lineColor.setColor(color+9,sim_colorcomponent_emission);
        }

        int retVal=newObject->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertPathCtrlPoints_internal(simInt pathHandle,simInt options,simInt startIndex,simInt ptCnt,const simVoid* ptData)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* path=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_INEXISTANT);
            return(-1);
        }

        path->pathContainer->enableActualization(false);
        int fiCnt=11;
        if (options&2)
            fiCnt=16;

        for (int i=0;i<ptCnt;i++)
        {
            CSimplePathPoint_old* pt=new CSimplePathPoint_old();
            C7Vector tr(C4Vector(((float*)ptData)[fiCnt*i+3],((float*)ptData)[fiCnt*i+4],((float*)ptData)[fiCnt*i+5]),C3Vector(((float*)ptData)+fiCnt*i+0));
            pt->setTransformation(tr,path->pathContainer->getAttributes());
            pt->setMaxRelAbsVelocity(((float*)ptData)[fiCnt*i+6]);
            pt->setOnSpotDistance(((float*)ptData)[fiCnt*i+7]);
            pt->setBezierPointCount(((int*)ptData)[fiCnt*i+8]);
            pt->setBezierFactors(((float*)ptData)[fiCnt*i+9],((float*)ptData)[fiCnt*i+10]);
            if (options&2)
            {
                pt->setAuxFlags(((int*)ptData)[fiCnt*i+11]);
                pt->setAuxChannels(((float*)ptData)+fiCnt*i+12);
            }
            path->pathContainer->insertSimplePathPoint(pt,startIndex+i);
        }
        if (options&1)
            path->pathContainer->setAttributes(path->pathContainer->getAttributes()|sim_pathproperty_closed_path);
        else
            path->pathContainer->setAttributes((path->pathContainer->getAttributes()|sim_pathproperty_closed_path)-sim_pathproperty_closed_path);
        path->pathContainer->enableActualization(true);
        path->pathContainer->actualizePath();
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCutPathCtrlPoints_internal(simInt pathHandle,simInt startIndex,simInt ptCnt)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath_old* path=App::currentWorld->sceneObjects->getPathFromHandle(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_PATH_INEXISTANT);
            return(-1);
        }
        if ((startIndex<0)||(ptCnt<0))
            path->pathContainer->removeAllSimplePathPoints();
        else
        {
            path->pathContainer->enableActualization(false);
            for (int i=0;i<ptCnt;i++)
                path->pathContainer->removeSimplePathPoint(startIndex);
            path->pathContainer->enableActualization(true);
            path->pathContainer->actualizePath();
        }
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetThreadId_internal()
{ // deprecated on 01.10.2020
    TRACE_C_API;

    int retVal=VThread::getThreadId_apiQueried();
    return(retVal); // 0=GUI thread, 1=main sim thread, 2-n=aux. sim threads
}

simInt simSwitchThread_internal()
{ // deprecated on 01.10.2020
    TRACE_C_API;
    if (CThreadPool_old::getThreadAutomaticSwitch())
    { // Important: when a script forbids thread switching, we don't want that a plugin switches anyways
        if (CThreadPool_old::switchBackToPreviousThread())
            return(1);
    }
    return(0);
}

simInt simLockResources_internal(simInt lockType,simInt reserved)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

#ifdef SIM_WITH_QT
    CSimAndUiThreadSync* obj=new CSimAndUiThreadSync(__func__);
    bool res=false;
    int retVal=-1; // fail
    if (lockType==sim_lock_ui_wants_to_read)
        res=obj->uiThread_tryToLockForUiEventRead(5);
    if (lockType==sim_lock_ui_wants_to_write)
        res=obj->uiThread_tryToLockForUiEventWrite(800);
    if (lockType==sim_lock_nonui_wants_to_write)
    {
        obj->simThread_lockForSimThreadWrite();
        res=true;
    }
    if (res)
    {
        EASYLOCK(_lockForExtLockList);
        retVal=obj->getObjectHandle();
        _extLockList.push_back(obj);
    }
    return(retVal);
#else
    return(0);
#endif
}

simInt simUnlockResources_internal(simInt lockHandle)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

#ifdef SIM_WITH_QT
    int retVal=0;
    { // scope parenthesis are important here!
        EASYLOCK(_lockForExtLockList);
        for (int i=0;i<int(_extLockList.size());i++)
        {
            if (_extLockList[i]->getObjectHandle()==lockHandle)
            {
                delete _extLockList[i];
                _extLockList.erase(_extLockList.begin()+i);
                retVal=1;
                break;
            }
        }
    }
    return(retVal);
#else
    return(1);
#endif
}

simChar* simGetUserParameter_internal(simInt objectHandle,const simChar* parameterName,simInt* parameterLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CUserParameters* uso=obj->getUserScriptParameterObject();
        if (uso!=nullptr)
        {
            std::string parameterValue;
            if (uso->getParameterValue(parameterName,parameterValue))
            {
                char* retVal=new char[parameterValue.length()+1];
                for (size_t i=0;i<parameterValue.length();i++)
                    retVal[i]=parameterValue[i];
                retVal[parameterValue.length()]=0;
                parameterLength[0]=(int)parameterValue.length();
                return(retVal);
            }
        }
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetUserParameter_internal(simInt objectHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{ // deprecated on 01.10.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        CUserParameters* uso=obj->getUserScriptParameterObject();
        bool s=false;
        if (uso==nullptr)
        {
            uso=new CUserParameters();
            s=true;
        }
        if (std::string(parameterName).compare("@enable")==0)
            uso->addParameterValue("exampleParameter","string","Hello World!",strlen("Hello World!"));
        else
            uso->setParameterValue(parameterName,parameterValue,size_t(parameterLength));
        if (s)
            obj->setUserScriptParameterObject(uso);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetPathTargetNominalVelocity_internal(simInt objectHandle,simFloat targetNominalVelocity)
{ // deprecated probably around 2015 or earlier
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(objectHandle);
        it->pathContainer->setTargetNominalVelocity(targetNominalVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetCollectionHandle_internal(const simChar* collectionName)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(collectionName).find("@silentError");
    std::string nm(collectionName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collectionNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CCollection* it=App::currentWorld->collections->getObjectFromName(collectionNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getCollectionHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRemoveCollection_internal(simInt collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> memSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            memSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        if (collectionHandle==sim_handle_all)
        {
            App::currentWorld->sceneObjects->deselectObjects();
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
                App::currentWorld->collections->addCollectionToSelection(App::currentWorld->collections->getObjectFromIndex(i)->getCollectionHandle());
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->sceneObjects->eraseObjects(sel,true);
            App::currentWorld->collections->removeAllCollections();
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(memSel[i]);
            return(1);
        }
        else
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
                return(-1);
            }
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->collections->addCollectionToSelection(it->getCollectionHandle());
            std::vector<int> sel;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::currentWorld->sceneObjects->deselectObjects();
            App::currentWorld->sceneObjects->eraseObjects(sel,true);
            App::currentWorld->collections->removeCollection(collectionHandle);
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(memSel[i]);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simEmptyCollection_internal(simInt collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (collectionHandle==sim_handle_all)
        {
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
                App::currentWorld->collections->getObjectFromIndex(i)->emptyCollection();
            return(1);
        }
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        it->emptyCollection();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetCollectionName_internal(simInt collectionHandle)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(nullptr);
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        char* retVal=new char[it->getCollectionName().length()+1];
        for (unsigned int i=0;i<it->getCollectionName().length();i++)
            retVal[i]=it->getCollectionName()[i];
        retVal[it->getCollectionName().length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetCollectionName_internal(simInt collectionHandle,const simChar* collectionName)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(-1);
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        std::string originalText(collectionName);
        if (originalText.length()>127)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        std::string text(collectionName);
        tt::removeIllegalCharacters(text,true);
        if (originalText!=text)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        if (it->getCollectionName().compare(text)==0)
            return(1);
        if (App::currentWorld->collections->getObjectFromName(text.c_str())!=nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        it->setCollectionName(originalText.c_str(),true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simCreateCollection_internal(const simChar* collectionName,simInt options)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string originalText;
        if (collectionName!=nullptr)
            originalText=collectionName;
        if (originalText.length()!=0)
        {
            if ( (originalText.length()<=0)||(originalText.length()>127) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            std::string text(collectionName);
            tt::removeIllegalCharacters(text,true);
            if (originalText!=text)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            if (App::currentWorld->collections->getObjectFromName(text.c_str())!=nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
        }
        else
            originalText="collection"; // default name

        CCollection* it=new CCollection(-2);
        it->setCollectionName(originalText.c_str(),false);
        App::currentWorld->collections->addCollection(it,false);
        it->setOverridesObjectMainProperties((options&1)!=0);
        return(it->getCollectionHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddObjectToCollection_internal(simInt collectionHandle,simInt objectHandle,simInt what,simInt options)
{ // deprecated on 17.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
        {
            return(-1);
        }
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(collectionHandle);
        if (what!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
        }
        CCollectionElement* el=nullptr;
        if (what==sim_handle_all)
        {
            el=new CCollectionElement(-1,sim_collectionelement_all,true);
        }
        if (what==sim_handle_single)
        {
            el=new CCollectionElement(objectHandle,sim_collectionelement_loose,(options&1)==0);
        }
        if (what==sim_handle_tree)
        {
            int what=sim_collectionelement_frombaseincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_frombaseexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (what==sim_handle_chain)
        {
            int what=sim_collectionelement_fromtipincluded;
            if ((options&2)!=0)
                what=sim_collectionelement_fromtipexcluded;
            el=new CCollectionElement(objectHandle,what,(options&1)==0);
        }
        if (el==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        it->addCollectionElement(el);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simHandleCollision_internal(simInt collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (collisionObjectHandle!=sim_handle_all)&&(collisionObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
            {
                return(-1);
            }
        }
        int colCnt=0;
        if (collisionObjectHandle<0)
            colCnt=App::currentWorld->collisions->handleAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            if (it->handleCollision())
                colCnt++;
        }
        return(colCnt);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadCollision_internal(simInt collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
        {
            return(-1);
        }
        CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
        int retVal=it->readCollision(nullptr);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleDistance_internal(simInt distanceObjectHandle,simFloat* smallestDistance)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (distanceObjectHandle!=sim_handle_all)&&(distanceObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
                return(-1);
        }
        float d;
        if (distanceObjectHandle<0)
            d=App::currentWorld->distances->handleAllDistances(distanceObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            d=it->handleDistance();
        }
        if (d>=0.0f)
        {
            if (smallestDistance!=nullptr)
                smallestDistance[0]=d;
            return(1);
        }
        else
            return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadDistance_internal(simInt distanceObjectHandle,simFloat* smallestDistance)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
            return(-1);
        float d;
        CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
        d=it->readDistance();
        if (d>=0.0f)
        {
            smallestDistance[0]=d;
            return(1);
        }
        smallestDistance[0]=SIM_MAX_FLOAT; // new for V3.3.2 rev2
        return(0); // from -1 to 0 for V3.3.2 rev2
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetCollisionHandle_internal(const simChar* collisionObjectName)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(collisionObjectName).find("@silentError");
    std::string nm(collisionObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collisionObjectNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromName(collisionObjectNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COLLISION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDistanceHandle_internal(const simChar* distanceObjectName)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    size_t silentErrorPos=std::string(distanceObjectName).find("@silentError");
    std::string nm(distanceObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string distanceObjectNameAdjusted=getIndexAdjustedObjectName(nm.c_str());
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CDistanceObject_old* it=App::currentWorld->distances->getObjectFromName(distanceObjectNameAdjusted.c_str());
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_DISTANCE_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetCollision_internal(simInt collisionObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (collisionObjectHandle!=sim_handle_all)&&(collisionObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
                return(-1);
        }
        if (collisionObjectHandle<0)
            App::currentWorld->collisions->resetAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearCollisionResult();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetDistance_internal(simInt distanceObjectHandle)
{ // deprecated on 20.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (distanceObjectHandle!=sim_handle_all)&&(distanceObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
                return(-1);
        }
        if (distanceObjectHandle<0)
            App::currentWorld->distances->resetAllDistances(distanceObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearDistanceResult();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddBanner_internal(const simChar* label,simFloat size,simInt options,const simFloat* positionAndEulerAngles,simInt parentObjectHandle,const simFloat* labelColors,const simFloat* backgroundColors)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1; // means error

        CBannerObject* it=new CBannerObject(label,options,parentObjectHandle,positionAndEulerAngles,labelColors,backgroundColors,size);
        retVal=App::currentWorld->bannerCont->addObject(it);

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveBanner_internal(simInt bannerID)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (bannerID==sim_handle_all)
            App::currentWorld->bannerCont->eraseAllObjects(false);
        else
        {
            int handleFlags=0;
            if (bannerID>=0)
            {
                handleFlags=bannerID&0xff00000;
                bannerID=bannerID&0xfffff;
            }
            CBannerObject* it=App::currentWorld->bannerCont->getObject(bannerID);
            if (it==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
                return(-1);
            }
            if (handleFlags&sim_handleflag_togglevisibility)
            {
                if (it->toggleVisibility())
                    return(1);
                return(0);
            }
            else
                App::currentWorld->bannerCont->removeObject(bannerID);
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddGhost_internal(simInt ghostGroup,simInt objectHandle,simInt options,simFloat startTime,simFloat endTime,const simFloat* color)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        int retVal=App::currentWorld->ghostObjectCont->addGhost(ghostGroup,objectHandle,options,startTime,endTime,color);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simModifyGhost_internal(simInt ghostGroup,simInt ghostId,simInt operation,simFloat floatValue,simInt options,simInt optionsMask,const simFloat* colorOrTransformation)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::currentWorld->ghostObjectCont->modifyGhost(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetGraphUserData_internal(simInt graphHandle,const simChar* streamName,simFloat data)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!App::currentWorld->simulation->isSimulationRunning())
    {
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
            return(-1);
        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(graphHandle);
        CGraphData_old* stream=it->getGraphData(streamName);
        if (stream==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_DATA_STREAM);
            return(-1);
        }
        if (stream->getDataType()!=GRAPH_NOOBJECT_USER_DEFINED)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_DATA_STREAM_NOT_USER_DEFINED);
            return(-1);
        }
        stream->setUserData(data);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddPointCloud_internal(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simFloat pointSize,simInt ptCnt,const simFloat* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simFloat* pointNormals)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CPtCloud_old* ptCloud=new CPtCloud_old(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,(unsigned char*)pointColors,pointNormals,(unsigned char*)defaultColors);
        retVal=App::currentWorld->pointCloudCont->addObject(ptCloud);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simModifyPointCloud_internal(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simFloat* floatParam)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (operation==0)
        {
            if (App::currentWorld->pointCloudCont->removeObject(pointCloudHandle))
                return(1);
        }
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCopyMatrix_internal(const simFloat* matrixIn,simFloat* matrixOut)
{ // deprecated on 23.11.2020
    TRACE_C_API;

    for (int i=0;i<12;i++)
        matrixOut[i]=matrixIn[i];
    return(1);
}

simInt simAddModuleMenuEntry_internal(const simChar* entryLabel,simInt itemCount,simInt* itemHandles)
{ // deprecated on 04.05.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (itemCount==1)
            itemHandles[0]=App::worldContainer->moduleMenuItemContainer->addMenuItem(entryLabel,-1);
        else
        {
            std::string s(entryLabel);
            s+="\n";
            for (int i=0;i<itemCount;i++)
                itemHandles[i]=App::worldContainer->moduleMenuItemContainer->addMenuItem(s.c_str(),-1);
        }
        return(1);
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetModuleMenuItemState_internal(simInt itemHandle,simInt state,const simChar* label)
{ // deprecated on 04.05.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CModuleMenuItem* it=App::worldContainer->moduleMenuItemContainer->getItemFromHandle(itemHandle);
        if (it!=nullptr)
        {
            if (state!=-1)
                it->setState(state);
            if (label!=nullptr)
                it->setLabel(label);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ITEM_HANDLE);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetObjectName_internal(simInt objectHandle)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(nullptr);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        std::string nm;
        if ((handleFlags&sim_handleflag_altname)!=0)
            nm=it->getObjectAltName_old();
        else
            nm=it->getObjectName_old();
        char* retVal=new char[nm.length()+1];
        for (size_t i=0;i<nm.length();i++)
            retVal[i]=nm[i];
        retVal[nm.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectName_internal(simInt objectHandle,const simChar* objectName)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handle=objectHandle;
        int handleFlags=0;
        if (objectHandle>=0)
        {
            handleFlags=objectHandle&0x0ff00000;
            handle=objectHandle&0x000fffff;
        }
        if (!doesObjectExist(__func__,handle))
            return(-1);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(handle);
        std::string originalText(objectName);
        if (originalText.length()>127)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if (originalText.length()<1)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        std::string text(objectName);
        if ((handleFlags&sim_handleflag_altname)!=0)
            tt::removeAltNameIllegalCharacters(text);
        else
            tt::removeIllegalCharacters(text,true);
        if (originalText!=text)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        std::string oldName;
        if ((handleFlags&sim_handleflag_altname)!=0)
            oldName=it->getObjectAltName_old();
        else
            oldName=it->getObjectName_old();

        if (oldName.compare(text)==0)
            return(1);
        bool err;
        if ((handleFlags&sim_handleflag_altname)!=0)
            err=(App::currentWorld->sceneObjects->getObjectFromAltName_old(text.c_str())!=nullptr);
        else
            err=(App::currentWorld->sceneObjects->getObjectFromName_old(text.c_str())!=nullptr);
        if (err)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if ((handleFlags&sim_handleflag_altname)!=0)
            App::currentWorld->sceneObjects->setObjectAltName_old(it,text.c_str(),true);
        else
        {
            App::currentWorld->sceneObjects->setObjectName_old(it,text.c_str(),true);
            App::setFullDialogRefreshFlag();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetScriptName_internal(simInt scriptHandle)
{ // deprecated on 08.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        std::string name(it->getScriptPseudoName_old());
        char* retVal=new char[name.length()+1];
        for (int i=0;i<int(name.length());i++)
            retVal[i]=name[i];
        retVal[name.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetScriptHandle_internal(const simChar* targetAtScriptName)
{ // deprecated on 16.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    std::string scriptName(targetAtScriptName);
    std::string targetName;

    std::string targetAtScriptNm(targetAtScriptName);
    size_t p=targetAtScriptNm.find('@');
    if (p!=std::string::npos)
    {
        scriptName.assign(targetAtScriptNm.begin()+p+1,targetAtScriptNm.end());
        targetName.assign(targetAtScriptNm.begin(),targetAtScriptNm.begin()+p);
    }

    bool useAlias=( (scriptName.size()>0)&&((scriptName[0]=='/')||(scriptName[0]=='.')||(scriptName[0]==':')) );
    if (!useAlias)
        scriptName=getIndexAdjustedObjectName(scriptName.c_str());

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=nullptr;
        if ((targetName.length()==0)||(targetName.compare("child")==0)||(targetName.compare("main")==0))
        {
            if (scriptName.length()==0)
                it=App::currentWorld->embeddedScriptContainer->getMainScript();
            else
            {
                CSceneObject* obj=nullptr;
                if (useAlias)
                    obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                else
                    obj=App::currentWorld->sceneObjects->getObjectFromName_old(scriptName.c_str());
                if (obj!=nullptr)
                    it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,obj->getObjectHandle());
            }
        }
        if (targetName.compare("customization")==0)
        {
            CSceneObject* obj=nullptr;
            if (useAlias)
                obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
            else
                obj=App::currentWorld->sceneObjects->getObjectFromName_old(scriptName.c_str());
            if (obj!=nullptr)
                it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,obj->getObjectHandle());
        }
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        if (it->getFlaggedForDestruction())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_WAS_DESTROYED);
            return(-1);
        }
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetScriptVariable_internal(simInt scriptHandleOrType,const simChar* variableNameAtScriptName,simInt stackHandle)
{ // deprecated on 16.06.2021
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* script=nullptr;
        std::string variableName;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            else
                variableName=varNameAtScriptName;
            script=App::worldContainer->getScriptFromHandle(scriptHandleOrType);
        }
        else
        {
            std::string scriptName;
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
            {
                scriptName.assign(varNameAtScriptName.begin()+p+1,varNameAtScriptName.end());
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            }
            else
                variableName=varNameAtScriptName;

            if (scriptHandleOrType==sim_scripttype_mainscript)
                script=App::currentWorld->embeddedScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::worldContainer->addOnScriptContainer->getAddOnFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::worldContainer->sandboxScript;
            if ( (scriptHandleOrType==sim_scripttype_childscript)||(scriptHandleOrType==sim_scripttype_customizationscript) )
            {
                if (scriptName.size()>0)
                {
                    int objId=-1;
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0,nullptr);
                    if (obj!=nullptr)
                        objId=obj->getObjectHandle();
                    else
                        objId=App::currentWorld->sceneObjects->getObjectHandleFromName_old(scriptName.c_str());
                    if (scriptHandleOrType==sim_scripttype_customizationscript)
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objId);
                    else
                        script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objId);
                }
            }
        }

        if (script!=nullptr)
        {
            bool doAClear=(stackHandle==0);
            CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
            if ( (stack==nullptr)&&(!doAClear) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_HANDLE);
                return(-1);
            }
            if ( (stack!=nullptr)&&(stack->getStackSize()==0) )
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
                return(-1);
            }
            int retVal=-1; // error
            if (script->getThreadedExecutionIsUnderWay_oldThreads())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId_old(),VThread::getCurrentThreadId()))
                    retVal=script->setScriptVariable_old(variableName.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=2;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)variableName.c_str();
                    d[3]=stack;

                    retVal=CThreadPool_old::callRoutineViaSpecificThread(script->getThreadedScriptThreadId_old(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->setScriptVariable_old(variableName.c_str(),stack);
                }
            }

            if (retVal==-1)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScript_internal(simInt index)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (index<0)||(index>=int(App::currentWorld->embeddedScriptContainer->allScripts.size())) )
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->allScripts[index];
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptAssociatedWithObject_internal(simInt objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,objectHandle);
        if (it==nullptr)
            return(-1);
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetCustomizationScriptAssociatedWithObject_internal(simInt objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,objectHandle);
        if (it==nullptr)
            return(-1);
        int retVal=it->getScriptHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectAssociatedWithScript_internal(simInt scriptHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (it->getScriptType()==sim_scripttype_childscript)
                retVal=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_childscript);
        if (it->getScriptType()==sim_scripttype_customizationscript)
                retVal=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_customizationscript);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetObjectConfiguration_internal(simInt objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);

        std::vector<char> data;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it!=nullptr)
        {
            CMemorizedConf_old temp(it);
            temp.serializeToMemory(data);
            char* retBuffer=new char[data.size()+sizeof(int)];
            ((int*)retBuffer)[0]=int(data.size());
            for (size_t i=0;i<data.size();i++)
                retBuffer[sizeof(int)+i]=data[i];
            return(retBuffer);
        }
        return(nullptr);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectConfiguration_internal(const simChar* data)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (data!=nullptr)
        {
            std::vector<char> arr;
            int l=((int*)data)[0];
            for (int i=0;i<l;i++)
                arr.push_back(data[i+sizeof(int)]);
            CMemorizedConf_old temp;
            temp.serializeFromMemory(arr);
            temp.restore();
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetConfigurationTree_internal(simInt objectHandle)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (objectHandle!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
        }
        if (objectHandle==sim_handle_all)
            objectHandle=-1;

        std::vector<char> data;
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        std::vector<CSceneObject*> sel;
        if (it==nullptr)
        { // We memorize everything:
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectFromIndex(i));
        }
        else
        { // We memorize just the object and all its children:
            it->getAllObjectsRecursive(&sel,true,true);
        }
        for (size_t i=0;i<sel.size();i++)
        {
            CMemorizedConf_old temp(sel[i]);
            temp.serializeToMemory(data);
        }
        char* retBuffer=new char[data.size()+sizeof(int)];
        ((int*)retBuffer)[0]=int(data.size());
        for (size_t i=0;i<data.size();i++)
            retBuffer[sizeof(int)+i]=data[i];
        return(retBuffer);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetConfigurationTree_internal(const simChar* data)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (data!=nullptr)
        {
            int l=((int*)data)[0];
            std::vector<char> arr;
            for (int i=0;i<l;i++)
                arr.push_back(data[i+sizeof(int)]);
            std::vector<CMemorizedConf_old*> allConfs;
            std::vector<int> parentCount;
            std::vector<int> index;
            while (arr.size()!=0)
            {
                CMemorizedConf_old* temp=new CMemorizedConf_old();
                temp->serializeFromMemory(arr);
                parentCount.push_back(temp->getParentCount());
                index.push_back((int)index.size());
                allConfs.push_back(temp);
            }
            tt::orderAscending(parentCount,index);
            for (size_t i=0;i<index.size();i++)
            {
                allConfs[index[i]]->restore();
                delete allConfs[index[i]];
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simEnableEventCallback_internal(simInt eventCallbackType,const simChar* plugin,simInt reserved)
{ // deprecated on 18.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (CPluginContainer::enableOrDisableSpecificEventCallback(eventCallbackType,plugin))
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetObjectSizeValues_internal(simInt objectHandle,const simFloat* sizeValues)
{ // deprecated on 28.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            obj->setSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectSizeValues_internal(simInt objectHandle,simFloat* sizeValues)
{ // deprecated on 28.06.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            obj->getSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRMLPosition_internal(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simDouble* newPosVelAccel,simVoid* auxData)
{ // deprecated sometime in 2015-2016
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    unsigned char auxDataCount=0;
    if (auxData!=nullptr)
        auxDataCount=((unsigned char*)auxData)[0]; // the first byte indicates how many values we have or we wanna have set!

    char* data=new char[4+8+24*dofs+24*dofs+dofs+16*dofs+4+1]; // for now we have no aux data to transmit (so just +1)
    int off=0;

    // dofs
    ((int*)(data+off))[0]=dofs;
    off+=4;

    // timeStep
    ((double*)(data+off))[0]=timeStep;
    off+=8;

    // currentPosVelAccel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[2*dofs+i];
        off+=8;
    }

    // maxVelAccelJerk
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[2*dofs+i];
        off+=8;
    }

    // selection
    for (int i=0;i<dofs;i++)
    {
        ((char*)(data+off))[0]=selection[0+i];
        off++;
    }

    // targetPosVel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetPosVel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetPosVel[dofs+i];
        off+=8;
    }

    // Flags:
    ((int*)(data+off))[0]=flags;
    off+=4;

    // Number of extension bytes (not used for now)
    data[off]=0;
    off++;

    int retVal=-42; // means no plugin
    int auxVals[4]={0,0,0,0}; // if first value is diff. from 0, we use the type 4 lib
        auxVals[0]=1;
    int replyData[4]={-1,-1,-1,-1};

    void* returnData=nullptr;
    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        returnData=plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlposition,auxVals,data,replyData);
    if (returnData!=nullptr)
    {
        retVal=replyData[0];
        off=0;
        char* returnDat=(char*)returnData;
        // newPosVelAccel
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[0+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[2*dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }

        // here we have 8*8 bytes for future extensions. 1*8 bytes are already used:
        double synchronizationTime=((double*)(returnDat+off))[0];
        off+=8*8;

        if (auxDataCount>=1)
            ((double*)(((char*)auxData)+1))[0]=synchronizationTime;

        delete[] (char*)returnData;
    }

    delete[] data;
    return(retVal);
}

simInt simRMLVelocity_internal(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simDouble* newPosVelAccel,simVoid* auxData)
{ // deprecated sometime in 2015-2016
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    unsigned char auxDataCount=0;
    if (auxData!=nullptr)
        auxDataCount=((unsigned char*)auxData)[0]; // the first byte indicates how many values we have or we wanna have set!

    char* data=new char[4+8+24*dofs+16*dofs+dofs+8*dofs+4+1]; // for now we have no aux data to transmit (so just +1)
    int off=0;

    // dofs
    ((int*)(data+off))[0]=dofs;
    off+=4;

    // timeStep
    ((double*)(data+off))[0]=timeStep;
    off+=8;

    // currentPosVelAccel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[2*dofs+i];
        off+=8;
    }

    // maxAccelJerk
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxAccelJerk[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxAccelJerk[dofs+i];
        off+=8;
    }

    // selection
    for (int i=0;i<dofs;i++)
    {
        ((char*)(data+off))[0]=selection[0+i];
        off++;
    }

    // targetVel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetVel[0+i];
        off+=8;
    }

    // Flags:
    ((int*)(data+off))[0]=flags;
    off+=4;

    // Number of extension bytes (not used for now)
    data[off]=0;
    off++;

    int retVal=-42; // means no plugin
    int auxVals[4]={0,0,0,0}; // if first value is diff. from 0, we use the type 4 lib!
        auxVals[0]=1;
    int replyData[4]={-1,-1,-1,-1};


    void* returnData=nullptr;
    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        returnData=plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlvelocity,auxVals,data,replyData);
    if (returnData!=nullptr)
    {
        retVal=replyData[0];
        off=0;
        char* returnDat=(char*)returnData;
        // newPosVelAccel
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[0+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[2*dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }

        // here we have 8*8 bytes for future extensions. 1*8 bytes are already used:
        double synchronizationTime=((double*)(returnDat+off))[0];
        if (auxDataCount>=1)
            ((double*)(((char*)auxData)+1))[0]=synchronizationTime;
        off+=8*8;

        delete[] (char*)returnData;
    }

    delete[] data;
    return(retVal);
}

simInt simRMLPos_internal(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simVoid* auxData)
{ // deprecated on 10.08.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    char* data=new char[4+8+24*dofs+24*dofs+dofs+16*dofs+4+1]; // for now we have no aux data to transmit (so just +1)
    int off=0;

    // dofs
    ((int*)(data+off))[0]=dofs;
    off+=4;

    // timeStep
    ((double*)(data+off))[0]=smallestTimeStep;
    off+=8;

    // currentPosVelAccel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[2*dofs+i];
        off+=8;
    }

    // maxVelAccelJerk
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxVelAccelJerk[2*dofs+i];
        off+=8;
    }

    // selection
    for (int i=0;i<dofs;i++)
    {
        ((char*)(data+off))[0]=selection[0+i];
        off++;
    }

    // targetPosVel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetPosVel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetPosVel[dofs+i];
        off+=8;
    }

    // Flags:
    ((int*)(data+off))[0]=flags;
    off+=4;

    // Number of extension bytes (not used for now)
    data[off]=0;
    off++;

    int auxVals[4]={0,0,0,0}; // if first value is diff. from 0, we use the type 4 lib
    auxVals[0]=1;
    auxVals[1]=0; // do not destroy at simulation end
    if (auxData!=nullptr)
    {
        if (((unsigned char*)auxData)[0]>=1)
            auxVals[1]=((int*)(((unsigned char*)auxData)+1))[0];
    }
    int replyData[4]={-1,-1,-1,-1};
    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlpos,auxVals,data,replyData);
    delete[] data;
    return(replyData[1]);
}

simInt simRMLVel_internal(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simVoid* auxData)
{ // deprecated on 10.08.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    char* data=new char[4+8+24*dofs+16*dofs+dofs+8*dofs+4+1]; // for now we have no aux data to transmit (so just +1)
    int off=0;

    // dofs
    ((int*)(data+off))[0]=dofs;
    off+=4;

    // timeStep
    ((double*)(data+off))[0]=smallestTimeStep;
    off+=8;

    // currentPosVelAccel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[dofs+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=currentPosVelAccel[2*dofs+i];
        off+=8;
    }

    // maxAccelJerk
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxAccelJerk[0+i];
        off+=8;
    }
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=maxAccelJerk[dofs+i];
        off+=8;
    }

    // selection
    for (int i=0;i<dofs;i++)
    {
        ((char*)(data+off))[0]=selection[0+i];
        off++;
    }

    // targetVel
    for (int i=0;i<dofs;i++)
    {
        ((double*)(data+off))[0]=targetVel[0+i];
        off+=8;
    }

    // Flags:
    ((int*)(data+off))[0]=flags;
    off+=4;

    // Number of extension bytes (not used for now)
    data[off]=0;
    off++;

    int auxVals[4]={0,0,0,0}; // if first value is diff. from 0, we use the type 4 lib!
    auxVals[0]=1;
    auxVals[1]=0; // do not destroy at simulation end
    if (auxData!=nullptr)
    {
        if (((unsigned char*)auxData)[0]>=1)
            auxVals[1]=((int*)(((unsigned char*)auxData)+1))[0];
    }
    int replyData[4]={-1,-1,-1,-1};

    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlvel,auxVals,data,replyData);

    delete[] data;
    return(replyData[1]);
}

simInt simRMLStep_internal(simInt handle,simDouble timeStep,simDouble* newPosVelAccel,simVoid* auxData,simVoid* reserved)
{ // deprecated on 10.08.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    unsigned char auxDataCount=0;
    if (auxData!=nullptr)
        auxDataCount=((unsigned char*)auxData)[0]; // the first byte indicates how many values we have or we wanna have set!

    int retVal=-1;
    int auxVals[4]={0,0,0,0}; // if first value is diff. from 0, we use the type 4 lib
    auxVals[0]=1;
    auxVals[1]=handle;
    auxVals[2]=int(timeStep*100000.0);
    int replyData[4]={-1,-1,-1,-1};


    void* returnData=nullptr;
    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        returnData=plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlstep,auxVals,nullptr,replyData);
    if (returnData!=nullptr)
    {
        retVal=replyData[0];
        int dofs=replyData[1];
        int off=0;
        char* returnDat=(char*)returnData;
        // newPosVelAccel
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[0+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }
        for (int i=0;i<dofs;i++)
        {
            newPosVelAccel[2*dofs+i]=((double*)(returnDat+off))[0];
            off+=8;
        }

        // here we have 8*8 bytes for future extensions. 1*8 bytes are already used:
        double synchronizationTime=((double*)(returnDat+off))[0];
        off+=8*8;

        if (auxDataCount>=1)
            ((double*)(((char*)auxData)+1))[0]=synchronizationTime;

        delete[] (char*)returnData;
    }

    return(retVal);
}

simInt simRMLRemove_internal(simInt handle)
{ // deprecated on 10.08.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    int auxVals[4]={0,0,0,0};
    auxVals[1]=handle;
    int replyData[4]={-1,-1,-1,-1};

    CPlugin* plugin=CPluginContainer::getPluginFromFunc("rml");
    if (plugin!=nullptr)
        plugin->sendEventCallbackMessage(sim_message_eventcallback_rmlremove,auxVals,nullptr,replyData);
    return(replyData[1]);
}

simChar* simFileDialog_internal(simInt mode,const simChar* title,const simChar* startPath,const simChar* initName,const simChar* extName,const simChar* ext)
{ // deprecated on 07.09.2021
    TRACE_C_API;
#ifdef SIM_WITH_GUI
    if (!isSimulatorInitialized(__func__))
        return(nullptr);
    char* retVal=nullptr;

    std::string nameAndPath;
    bool native=1;
    #ifndef WIN_SIM // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
        native=0;
    #endif
    CPluginContainer::customUi_fileDialog(mode,title,startPath,initName,extName,ext,native,nameAndPath);

/*
    std::string stPath(startPath);
    if (stPath.length()==0)
        stPath=App::directories->executableDirectory;
    nameAndPath=App::uiThread->getOpenOrSaveFileName_api(mode,title,stPath.c_str(),initName,extName,ext);
    */
    if (nameAndPath.length()!=0)
    {
        retVal=new char[nameAndPath.length()+1];
        for (size_t i=0;i<nameAndPath.length();i++)
            retVal[i]=nameAndPath[i];
        retVal[nameAndPath.length()]=0; // terminal 0
    }
    return(retVal);
#else
    return(nullptr);
#endif
}

simInt simMsgBox_internal(simInt dlgType,simInt buttons,const simChar* title,const simChar* message)
{ // deprecated on 07.09.2021
    TRACE_C_API;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=sim_msgbox_return_ok;
#ifdef SIM_WITH_GUI
    retVal=CPluginContainer::customUi_msgBox(dlgType,buttons,title,message,sim_msgbox_return_ok);
#endif
    return(retVal);
}

simInt simDisplayDialog_internal(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simFloat* titleColors,const simFloat* dialogColors,simInt* elementHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushStringOntoStack(titleText,0);
        stack->pushStringOntoStack(mainText,0);
        stack->pushInt32OntoStack(dialogType);
        stack->pushBoolOntoStack(false);
        if (initialText!=nullptr)
            stack->pushStringOntoStack(initialText,0);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.displayDialog",stack->getId());
        int retVal;
        stack->getStackInt32Value(retVal);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        return(retVal);
#else
        return(1);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetDialogResult_internal(simInt genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();;
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogResult",stack->getId());
        int retVal;
        stack->getStackInt32Value(retVal);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        return(retVal);
#else
        return(sim_dlgret_cancel);
#endif
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetDialogInput_internal(simInt genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string tmp;
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogInput",stack->getId());
        bool r=stack->getStackStringValue(tmp);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
        if (!r)
            return(nullptr);
#else
        return(nullptr);
#endif
        char* retVal=new char[tmp.length()+1];
        retVal[tmp.length()]=0;
        for (int i=0;i<int(tmp.length());i++)
            retVal[i]=tmp[i];
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simEndDialog_internal(simInt genericDialogHandle)
{ // deprecated on 07.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->createStack();
        stack->pushInt32OntoStack(genericDialogHandle);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.endDialog",stack->getId());
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
#endif
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simIsObjectInSelection_internal(simInt objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::currentWorld->sceneObjects->isObjectInSelection(objectHandle))
            retVal|=1;
        CSceneObject* lastSel=App::currentWorld->sceneObjects->getLastSelectionObject();
        if (lastSel!=nullptr)
        {
            if (lastSel->getObjectHandle()==objectHandle)
                retVal|=2;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddObjectToSelection_internal(simInt what,simInt objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        if (what==sim_handle_all)
            App::currentWorld->sceneObjects->selectAllObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::currentWorld->sceneObjects->addObjectToSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<CSceneObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::currentWorld->sceneObjects->addObjectToSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simRemoveObjectFromSelection_internal(simInt what,simInt objectHandle)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        if (what==sim_handle_all)
            App::currentWorld->sceneObjects->deselectObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::currentWorld->sceneObjects->removeObjectFromSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<CSceneObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::currentWorld->sceneObjects->removeObjectFromSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simGetObjectSelectionSize_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=int(App::currentWorld->sceneObjects->getSelectionCount());
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectLastSelection_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
        if (it!=nullptr)
            retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetObjectSelection_internal(simInt* objectHandles)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            objectHandles[i]=App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i);
        return(int(App::currentWorld->sceneObjects->getSelectionCount()));
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simScaleSelectedObjects_internal(simFloat scalingFactor,simBool scalePositionsToo)
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        CSceneObjectOperations::scaleObjects(sel,scalingFactor,scalePositionsToo!=0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simDeleteSelectedObjects_internal()
{ // deprecated on 24.09.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
        App::currentWorld->sceneObjects->deselectObjects();
        App::currentWorld->sceneObjects->eraseObjects(sel,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectUniqueIdentifier_internal(simInt objectHandle,simInt* uniqueIdentifier)
{ // deprecated on 08.10.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (objectHandle!=sim_handle_all)&&(!doesObjectExist(__func__,objectHandle)) )
            return(-1);
        if (objectHandle!=sim_handle_all)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            uniqueIdentifier[0]=int(it->getObjectUid());
        }
        else
        { // for backward compatibility
            int p=0;
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromIndex(i);
                if ( (it->getObjectHandle()==objectHandle)||(objectHandle==sim_handle_all) )
                    uniqueIdentifier[p++]=int(it->getObjectUid());
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simVoid _simSetDynamicJointLocalTransformationPart2_internal(simVoid* joint,const simFloat* pos,const simFloat* quat)
{ // deprecated on 08.11.2021
}

simVoid _simSetDynamicForceSensorLocalTransformationPart2_internal(simVoid* forceSensor,const simFloat* pos,const simFloat* quat)
{ // deprecated on 08.11.2021
}

simVoid _simSetDynamicJointLocalTransformationPart2IsValid_internal(simVoid* joint,simBool valid)
{ // deprecated on 08.11.2021
}

simVoid _simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(simVoid* forceSensor,simBool valid)
{ // deprecated on 08.11.2021
}

simInt simBreakForceSensor_internal(simInt objectHandle)
{ // deprecated on 08.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isForceSensor(__func__,objectHandle))
            return(-1);
        if (App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(objectHandle);
        CSceneObject* child=it->getChildFromIndex(0);
        if (child!=nullptr)
            App::currentWorld->sceneObjects->setObjectParent(child,nullptr,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simBool _simIsForceSensorBroken_internal(const simVoid* forceSensor)
{ // deprecated on 08.11.2021
    TRACE_C_API;
    CSceneObject* child=((CForceSensor*)forceSensor)->getChildFromIndex(0);
    return(child==nullptr);
}

simVoid _simGetDynamicForceSensorLocalTransformationPart2_internal(const simVoid* forceSensor,simFloat* pos,simFloat* quat)
{ // deprecated on 08.11.2021
    TRACE_C_API;
    C7Vector tr;
    tr.setIdentity();
    tr.X.getInternalData(pos);
    tr.Q.getInternalData(quat);
}

simInt simGetJointMatrix_internal(simInt objectHandle,simFloat* matrix)
{ // deprecated on 09.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        C7Vector trFull(it->getFullLocalTransformation());
        C7Vector trPart1(it->getLocalTransformation());
        C7Vector tr(trPart1.getInverse()*trFull);
        tr.getMatrix().copyToInterface(matrix);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSphericalJointMatrix_internal(simInt objectHandle,const simFloat* matrix)
{ // deprecated on 09.11.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        if (it->getJointType()!=sim_joint_spherical_subtype)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_JOINT_NOT_SPHERICAL);
            return(-1);
        }
        C4X4Matrix m;
        m.copyFromInterface(matrix);
        it->setSphericalTransformation(C4Vector(m.M.getQuaternion()));
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectHandleEx_internal(const simChar* objectAlias,int index,int proxy,int options)
{ // deprecated on 03.12.2021
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSceneObject* it=nullptr;
        std::string nm(objectAlias);
        size_t silentErrorPos=std::string(objectAlias).find("@silentError"); // Old, for backcompatibility
        std::string additionalMessage_backCompatibility;
        if ( (nm.size()>0)&&((nm[0]=='.')||(nm[0]==':')||(nm[0]=='/')) )
        {
            int objHandle=App::currentWorld->embeddedScriptContainer->getObjectHandleFromScriptHandle(_currentScriptHandle);
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
            CSceneObject* prox=App::currentWorld->sceneObjects->getObjectFromHandle(proxy);
            it=App::currentWorld->sceneObjects->getObjectFromPath(obj,nm.c_str(),index,prox);
        }
        else
        { // Old, for backcompatibility:
            size_t altPos=std::string(objectAlias).find("@alt");
            size_t firstAtPos=std::string(objectAlias).find("@");
            if (firstAtPos!=std::string::npos)
                nm.erase(nm.begin()+firstAtPos,nm.end());
            if (altPos==std::string::npos)
            { // handle retrieval via regular name
                nm=getIndexAdjustedObjectName(nm.c_str());
                it=App::currentWorld->sceneObjects->getObjectFromName_old(nm.c_str());
                if (it==nullptr)
                {
                    additionalMessage_backCompatibility+="\n\nSince CoppeliaSim V4.3.0, objects should be retrieved via a path and alias, e.g. \"./path/to/alias\", \":/path/to/alias\", \"/path/to/alias\", etc.";
                    additionalMessage_backCompatibility+="\nYou however tried to access an object in a way that doesn't follow the new notation, i.e. \"";
                    additionalMessage_backCompatibility+=objectAlias;
                    additionalMessage_backCompatibility+="\" wasn't found.";
                    additionalMessage_backCompatibility+="\nNote also that object aliases are distinct from object names, which are deprecated and not displayed anymore. Additionally, sim.getObject is preferred over sim.getObjectHandle.";
                    additionalMessage_backCompatibility+="\nMake sure to read the following page for additional details: https://www.coppeliarobotics.com/helpFiles/en/accessingSceneObjects.htm";
                }
            }
            else
                it=App::currentWorld->sceneObjects->getObjectFromAltName_old(nm.c_str()); // handle retrieval via alt name
        }

        if (it==nullptr)
        {
            if ( (silentErrorPos==std::string::npos)&&((options&1)==0) )
            {
                additionalMessage_backCompatibility=SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH+additionalMessage_backCompatibility;
                CApiErrors::setLastWarningOrError(__func__,additionalMessage_backCompatibility.c_str());
            }
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetScriptAttribute_internal(simInt scriptHandle,simInt attributeID,simFloat floatVal,simInt intOrBoolVal)
{ // deprecated on 05.01.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(intOrBoolVal==0);
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustomizationScriptCleanupBeforeSave_DEPRECATED(intOrBoolVal!=0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            it->setExecutionPriority(intOrBoolVal);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            it->setNumberOfPasses(intOrBoolVal);
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution_oldThreads()) )
        {
            it->setAutomaticCascadingCallsDisabled_old(intOrBoolVal==0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_enabled)//&&(it->getScriptType()==sim_scripttype_childscript) )
        {
            it->setScriptIsDisabled(intOrBoolVal==0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
            retVal=1; // deprecated. Doesn't do anything


        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptAttribute_internal(simInt scriptHandle,simInt attributeID,simFloat* floatVal,simInt* intOrBoolVal)
{ // deprecated on 05.01.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustScriptDisabledDSim_compatibilityMode_DEPRECATED())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustomizationScriptCleanupBeforeSave_DEPRECATED())
                intOrBoolVal[0]=1;
            else
                intOrBoolVal[0]=0;
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution_oldThreads()) )
        {
            if (it->getAutomaticCascadingCallsDisabled_old())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            intOrBoolVal[0]=it->getExecutionPriority();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            intOrBoolVal[0]=it->getNumberOfPasses();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
        {
            intOrBoolVal[0]=sim_scriptdebug_none; // deprecated, doesn't work anymore
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_scripttype)
        {
            intOrBoolVal[0]=it->getScriptType();
            if (it->getThreadedExecution_oldThreads())
                intOrBoolVal[0]|=sim_scripttype_threaded_old;
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_scripthandle)
        {
            intOrBoolVal[0]=it->getScriptHandle();
            retVal=1;
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetScriptText_internal(simInt scriptHandle,const simChar* scriptText)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,true);
#endif
        it->setScriptText(scriptText);
        if ( (it->getScriptType()!=sim_scripttype_childscript)||(!it->getThreadedExecution_oldThreads())||App::currentWorld->simulation->isSimulationStopped() )
            it->resetScript();
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

const simChar* simGetScriptText_internal(simInt scriptHandle)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->environment->getSceneLocked())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCENE_LOCKED);
            return(nullptr);
        }
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        const char* retVal=nullptr;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,false);
        else
#endif
            retVal=it->getScriptText();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetScriptProperty_internal(simInt scriptHandle,simInt* scriptProperty,simInt* associatedObjectHandle)
{ // deprecated on 04.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        scriptProperty[0]=it->getScriptType();
        associatedObjectHandle[0]=it->getObjectHandleThatScriptIsAttachedTo(sim_scripttype_childscript);
        if (it->getThreadedExecution_oldThreads())
            scriptProperty[0]|=sim_scripttype_threaded_old;
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointMaxForce_internal(simInt jointHandle,simFloat* forceOrTorque)
{ // deprecated on 24.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,jointHandle))
            return(-1);
        if (!isJoint(__func__,jointHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
        forceOrTorque[0]=it->getTargetForce(false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointMaxForce_internal(simInt objectHandle,simFloat forceOrTorque)
{ // deprecated on 24.02.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        it->setTargetForce(forceOrTorque,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simVoid _simSetGeomProxyDynamicsFullRefreshFlag_internal(simVoid* geomData,simBool flag)
{ // deprecated on 03.03.2022. Has no effect
}

simBool _simGetGeomProxyDynamicsFullRefreshFlag_internal(const simVoid* geomData)
{ // deprecated on 03.03.2022. Has no effect
    return(0);
}

simInt simRemoveObject_internal(simInt objectHandle)
{ // deprecated on 07.03.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle==sim_handle_all)
        {
            App::currentWorld->sceneObjects->eraseAllObjects(true);
            return(1);
        }
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }

        // Memorize the selection:
        std::vector<int> initSel;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
            initSel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));

        App::currentWorld->sceneObjects->eraseObject(it,true);

        // Restore the initial selection:
        App::currentWorld->sceneObjects->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::currentWorld->sceneObjects->addObjectToSelection(initSel[i]);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simUInt simGetSystemTimeInMs_internal(simInt previousTime)
{ // deprecated on 01.04.2022
    TRACE_C_API;
    unsigned int retVal;
    if (previousTime<-1)
        retVal=VDateTime::getOSTimeInMs();
    else
    {
        if (previousTime==-1)
            retVal=(unsigned int)VDateTime::getTimeInMs();
        else
            retVal=(unsigned int)VDateTime::getTimeDiffInMs(previousTime);
    }
    return(retVal);
}

simInt simGetVisionSensorResolution_internal(simInt sensorHandle,simInt* resolution)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
            return(-1);
        if (!isVisionSensor(__func__,sensorHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        it->getResolution(resolution);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simGetVisionSensorImage_internal(simInt sensorHandle)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        int valPerPixel=3;
        if ((handleFlags&sim_handleflag_greyscale)!=0)
            valPerPixel=1;
        float* buff=new float[res[0]*res[1]*valPerPixel];
        unsigned char* imgBuff=it->getRgbBufferPointer();
        if ((handleFlags&sim_handleflag_greyscale)!=0)
        {
            for (int i=0;i<res[0]*res[1];i++)
            {
                float v=float(imgBuff[3*i+0])/255.0f;
                v+=float(imgBuff[3*i+1])/255.0f;
                v+=float(imgBuff[3*i+2])/255.0f;
                buff[i]=v/3.0f;
            }
        }
        else
        {
            for (int i=0;i<res[0]*res[1]*3;i++)
                buff[i]=float(imgBuff[i])/255.0f;
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetVisionSensorImage_internal(simInt sensorHandle,const simFloat* image)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isVisionSensor(__func__,objectHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        int retVal=0;
        if (handleFlags&sim_handleflag_depthbuffer)
            it->setDepthBuffer(image);
        else
        {
            if (it->setExternalImage_old(image,(handleFlags&sim_handleflag_greyscale)!=0,(handleFlags&sim_handleflag_rawvalue)!=0))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simUChar* simGetVisionSensorCharImage_internal(simInt sensorHandle,simInt* resolutionX,simInt* resolutionY)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        if (resolutionX!=nullptr)
            resolutionX[0]=res[0];
        if (resolutionY!=nullptr)
            resolutionY[0]=res[1];

        int valPerPixel=3;
        if ((handleFlags&sim_handleflag_greyscale)!=0)
            valPerPixel=1;
        unsigned char* buff=new unsigned char[res[0]*res[1]*valPerPixel];
        unsigned char* imgBuff=it->getRgbBufferPointer();
        if ((handleFlags&sim_handleflag_greyscale)!=0)
        {
            int n=res[0]*res[1];
            for (int i=0;i<n;i++)
            {
                unsigned int v=imgBuff[3*i+0];
                v+=imgBuff[3*i+1];
                v+=imgBuff[3*i+2];
                buff[i]=(unsigned char)(v/3);
            }
        }
        else
        {
            int n=res[0]*res[1]*3;
            for (int i=0;i<n;i++)
                buff[i]=imgBuff[i];
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetVisionSensorCharImage_internal(simInt sensorHandle,const simUChar* image)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isVisionSensor(__func__,objectHandle))
            return(-1);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        int retVal=0;
        if (it->setExternalCharImage_old(image,(handleFlags&sim_handleflag_greyscale)!=0,(handleFlags&sim_handleflag_rawvalue)!=0))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat* simGetVisionSensorDepthBuffer_internal(simInt sensorHandle)
{ // deprecated on 11.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
            return(nullptr);
        if (!isVisionSensor(__func__,sensorHandle))
            return(nullptr);
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        int res[2];
        it->getResolution(res);
        float* buff=new float[res[0]*res[1]];
        float* depthBuff=it->getDepthBufferPointer();
        if ((handleFlags&sim_handleflag_depthbuffermeters)!=0)
        { // Here we need to convert values to distances in meters:
            float n=it->getNearClippingPlane();
            float f=it->getFarClippingPlane();
            float fmn=f-n;
            for (int i=0;i<res[0]*res[1];i++)
                buff[i]=n+fmn*depthBuff[i];
        }
        else
        { // values are: 0=on the close clipping plane, 1=on the far clipping plane
            for (int i=0;i<res[0]*res[1];i++)
                buff[i]=depthBuff[i];
        }
        return(buff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simCreatePureShape_internal(simInt primitiveType,simInt options,const simFloat* sizes,simFloat mass,const simInt* precision)
{ // deprecated on 27.04.2022
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int pType=0;
        C3Vector s(tt::getLimitedFloat(0.00001f,100000.0f,sizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[1]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[2]));
        int sides=32;
        if (precision!=nullptr)
        {
            if (pType==2)
                sides=tt::getLimitedInt(3,50,precision[1]); // sphere
            else
                sides=tt::getLimitedInt(0,50,precision[1]);
        }

        if (primitiveType==0) // cuboid
            pType=sim_primitiveshape_cuboid;
        if (primitiveType==1) // sphere
        {
            pType=sim_primitiveshape_spheroid;
            s(1)=s(0);
            s(2)=s(0);
        }
        if (primitiveType==2) // cylinder
        {
            pType=sim_primitiveshape_cylinder;
            s(1)=s(0);
        }
        if (primitiveType==3) // cone
        {
            pType=sim_primitiveshape_cone;
            s(1)=s(0);
        }
        if (pType==0)
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_TYPE);
            return(-1);
        }
        int op=0;
        if ((options&1)!=0)
            op|=1;
        if ((options&4)==0)
            op|=2;
        if ((options&32)!=0)
            op|=4;
        CShape* shape=CAddOperations::addPrimitiveShape(pType,s,op,nullptr,0,sides,0,(options&16)==0,true);
        shape->setLocalTransformation(C7Vector::identityTransformation);
        shape->setVisibleEdges((options&2)!=0);
        shape->setRespondable((options&8)!=0);
        shape->getMeshWrapper()->setMass(tt::getLimitedFloat(0.000001f,10000.0f,mass));
        return(shape->getObjectHandle());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simVoid* simBroadcastMessage_internal(simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{ // deprecated on 04.05.2022
    TRACE_C_API;

    if (replyData!=nullptr)
    {
        replyData[0]=-1;
        replyData[1]=-1;
        replyData[2]=-1;
        replyData[3]=-1;
    }
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_broadcast,auxiliaryData,customData,replyData);
    return(retVal);
}

simVoid* simSendModuleMessage_internal(simInt message,simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{ // deprecated on 04.05.2022
    TRACE_C_API;

    if (replyData!=nullptr)
    {
        replyData[0]=-1;
        replyData[1]=-1;
        replyData[2]=-1;
        replyData[3]=-1;
    }
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(message,auxiliaryData,customData,replyData);
    return(retVal);
}
