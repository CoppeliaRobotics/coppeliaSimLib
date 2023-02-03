#include <easyLock.h>
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
#include <pluginContainer.h>
#include <mesh.h>
#include <vDateTime.h>
#include <ttUtil.h>
#include <vVarious.h>
#include <imgLoaderSaver.h>
#include <apiErrors.h>
#include <sigHandler.h>
#include <memorizedConf_old.h>
#include <algorithm>
#include <iostream>
#include <tinyxml2.h>
#include <simFlavor.h>
#include <regex>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <gm.h>
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
        #include <_dirent.h>
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
    /*
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
    */

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
        char* data=simGetSimulatorMessage_internal(&messageID,auxValues,&dataSize);
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
            if ((firstSimulationStopDelay>0)&&(simGetSimulationTime_internal()>=double(firstSimulationStopDelay)/1000.0))
            {
                firstSimulationStopDelay=0;
                simStopSimulation_internal();
            }
        }
        else
            App::worldContainer->callScripts(sim_syscb_realtimeidle,nullptr,nullptr);
    }
    if ( (simGetSimulationState_internal()==sim_simulation_stopped)&&wasRunning&&firstSimulationAutoQuit )
    {
        wasRunning=false;
        simQuitSimulator_internal(true); // will post the quit command
    }
}

//********************************************
// Following courtesy of Stephen James:
int simExtLaunchUIThread_internal(const char* applicationName,int options,const char* sceneOrModelToLoad_,const char* applicationDir_)
{
    std::string applicationDir__(applicationDir_);
    applicationDir = applicationDir__;
    return(simRunSimulator_internal(applicationName,options,simulatorInit,simulatorLoop,simulatorDeinit,0,sceneOrModelToLoad_,false));
}

int simExtGetExitRequest_internal()
{
    return(App::getExitRequest());
}

int simExtStep_internal(bool stepIfRunning)
{
    stepSimIfRunning = stepIfRunning;
    App::simulationThreadLoop();
    return(1);
}

int simExtCanInitSimThread_internal()
{
    if (App::canInitSimThread())
        return(1);
    return(0);
}

int simExtSimThreadInit_internal()
{
    App::simulationThreadInit();
    return(1);
}

int simExtSimThreadDestroy_internal()
{
    // If already called, then means we closed from the UI and dont need to post another request
    if(!App::getExitRequest())
        App::postExitRequest();
    App::simulationThreadDestroy();
    return(1);
}

int simExtPostExitRequest_internal()
{
    // If already called, then means we closed from the UI and dont need to post another request
    if(!App::getExitRequest())
        App::postExitRequest();
    return (1);
}

int simExtCallScriptFunction_internal(int scriptHandleOrType, const char* functionNameAtScriptName,
                                         const int* inIntData, int inIntCnt,
                                         const double* inFloatData, int inFloatCnt,
                                         const char** inStringData, int inStringCnt,
                                         const char* inBufferData, int inBufferCnt,
                                         int** outIntData, int* outIntCnt,
                                         double** outFloatData, int* outFloatCnt,
                                         char*** outStringData, int* outStringCnt,
                                         char** outBufferData, int* outBufferSize)
{
    int stack=simCreateStack_internal();
    simPushInt32TableOntoStack_internal(stack,inIntData,inIntCnt);
    simPushDoubleTableOntoStack_internal(stack,inFloatData,inFloatCnt);
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
        // at pos 2 we are expecting a double table:
        outFloatCnt[0]=-1;
        if (simGetStackSize_internal(stack)==2)
        {
            int tableSize=simGetStackTableInfo_internal(stack,0);
            if (tableSize>0)
            {
                outFloatCnt[0]=tableSize;
                outFloatData[0]=new double[tableSize];
                simGetStackDoubleTable_internal(stack,outFloatData[0],tableSize);
            }
            simPopStackItem_internal(stack,1);
        }
        if (outFloatCnt[0]==-1)
        {
            outFloatCnt[0]=0;
            outFloatData[0]=new double[0];
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
int simRunSimulator_internal(const char* applicationName,int options,void(*initCallBack)(),void(*loopCallBack)(),void(*deinitCallBack)(),int stopDelay,const char* sceneOrModelToLoad,bool launchSimThread)
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

void* simGetMainWindow_internal(int type)
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
                return((void*)App::mainWindow->winId());
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

int simRefreshDialogs_internal(int refreshDegree)
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

char* simGetLastError_internal()
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

int simSetLastError_internal(const char* funcName,const char* errorMessage)
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

int simGetObject_internal(const char* objectPath,int index,int proxy,int options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CSceneObject* it=nullptr;
        CSceneObject* prox=nullptr;
        if (proxy!=-1)
        {
            prox=App::currentWorld->sceneObjects->getObjectFromHandle(proxy);
            if (prox==nullptr)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PROXY_OBJECT);
                return(-1);
            }
        }
        const CSceneObject* emittingObj=nullptr;
        if (prox==nullptr)
        {
            int objHandle=App::currentWorld->embeddedScriptContainer->getObjectHandleFromScriptHandle(_currentScriptHandle);
            emittingObj=App::currentWorld->sceneObjects->getObjectFromHandle(objHandle);
        }
        else
            emittingObj=prox;
        it=App::currentWorld->sceneObjects->getObjectFromPath(emittingObj,objectPath,index);

        if (it!=nullptr)
            retVal=it->getObjectHandle();
        else
        {
            if ((options&1)==0)
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_OBJECT_INEXISTANT_OR_ILL_FORMATTED_PATH);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

long long int simGetObjectUid_internal(int objectHandle)
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

int simGetObjectFromUid_internal(long long int uid,int options)
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

int simGetScriptHandleEx_internal(int scriptType,int objectHandle,const char* scriptName)
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

int simRemoveObjects_internal(const int* objectHandles,int count)
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

int simRemoveModel_internal(int objectHandle)
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

char* simGetObjectAlias_internal(int objectHandle,int options)
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
            if (App::currentWorld->sceneObjects->getObjectFromPath(nullptr,(std::string("/")+it->getObjectAlias()).c_str(),1)==nullptr)
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
        if (options==6)
            nm=it->getObjectPathAndIndex(0); // the path with index, e.g. "/alias{3}"
        if (options==7)
            nm=it->getObjectPathAndIndex(1); // the path with index, e.g. "/parentModel{1}/alias{3}"
        if (options==8)
            nm=it->getObjectPathAndIndex(2); // the path with index, e.g. "/greatParent{0}/parentModel{1}/alias{3}"
        if (options==9)
            nm=it->getObjectPathAndIndex(999); // the path with index, e.g. "/.../.../greatParent{0}/parentModel{1}/alias{3}"
        char* retVal=new char[nm.length()+1];
        for (size_t i=0;i<nm.length();i++)
            retVal[i]=nm[i];
        retVal[nm.length()]=0;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetObjectAlias_internal(int objectHandle,const char* objectAlias,int options)
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

int simGetObjectMatrix_internal(int objectHandle,int relativeToObjectHandle,double* matrix)
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
        tr.getMatrix().getData(matrix);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectMatrix_internal(int objectHandle,int relativeToObjectHandle,const double* matrix)
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
        m.setData(matrix);
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

int simGetObjectPose_internal(int objectHandle,int relativeToObjectHandle,double* pose)
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
        tr.getData(pose,(handleFlags&sim_handleflag_wxyzquat)==0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectPose_internal(int objectHandle,int relativeToObjectHandle,const double* pose)
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
        tr.setData(pose,(handleFlags&sim_handleflag_wxyzquat)==0);
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

int simGetObjectPosition_internal(int objectHandle,int relativeToObjectHandle,double* position)
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
        tr.X.getData(position);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectPosition_internal(int objectHandle,int relativeToObjectHandle,const double* position)
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
                x.X.setData(position);
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
                    x.X.setData(position);
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

int simGetObjectOrientation_internal(int objectHandle,int relativeToObjectHandle,double* eulerAngles)
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
        C3Vector(tr.Q.getEulerAngles()).getData(eulerAngles);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectOrientation_internal(int objectHandle,int relativeToObjectHandle,const double* eulerAngles)
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

int simGetJointPosition_internal(int objectHandle,double* position)
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

int simSetJointPosition_internal(int objectHandle,double position)
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
        it->setPosition(position);
        it->setKinematicMotionType(0,true); // reset
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetJointTargetPosition_internal(int objectHandle,double targetPosition)
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

int simGetJointTargetPosition_internal(int objectHandle,double* targetPosition)
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

int simSetJointTargetVelocity_internal(int objectHandle,double targetVelocity)
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

int simGetJointTargetVelocity_internal(int objectHandle,double* targetVelocity)
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

int simGetObjectChildPose_internal(int objectHandle,double* pose)
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
        tr.getData(pose,true);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectChildPose_internal(int objectHandle,const double* pose)
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
                tr.setData(pose,true);
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

int simGetJointInterval_internal(int objectHandle,bool* cyclic,double* interval)
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

int simSetJointInterval_internal(int objectHandle,bool cyclic,const double* interval)
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
            double previousPos=it->getPosition();
            it->setIsCyclic(cyclic!=0);
            it->setPositionMin(interval[0]);
            it->setPositionRange(interval[1]);
            it->setPosition(previousPos);
            return(1);
        }
//        return(-1);
//        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectParent_internal(int objectHandle)
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

int simGetObjectChild_internal(int objectHandle,int index)
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

int simSetObjectParent_internal(int objectHandle,int parentObjectHandle,bool keepInPlace)
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

int simGetObjectType_internal(int objectHandle)
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

int simGetJointType_internal(int objectHandle)
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

int simBuildIdentityMatrix_internal(double* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.setIdentity();
    m.getData(matrix);
    return(1);
}

int simBuildMatrix_internal(const double* position,const double* eulerAngles,double* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.M.setEulerAngles(C3Vector(eulerAngles));
    m.X.setData(position);
    m.getData(matrix);
    return(1);
}

int simBuildPose_internal(const double* position,const double* eulerAngles,double* pose)
{
    // CoppeliaSim quaternion, internally: w x y z
    // CoppeliaSim quaternion, at interfaces: x y z w
    TRACE_C_API;

    C7Vector tr(C4Vector(eulerAngles[0],eulerAngles[1],eulerAngles[2]),C3Vector(position));
    tr.getData(pose,true);
    return(1);
}

int simGetEulerAnglesFromMatrix_internal(const double* matrix,double* eulerAngles)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.setData(matrix);
    m.M.getEulerAngles().getData(eulerAngles);
    return(1);
}

int simInvertMatrix_internal(double* matrix)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.setData(matrix);
    m.inverse();
    m.getData(matrix);
    return(1);
}

int simInvertPose_internal(double* pose)
{
    TRACE_C_API;

    C7Vector p;
    p.setData(pose,true);
    p.inverse();
    p.getData(pose,true);
    return(1);
}

int simMultiplyMatrices_internal(const double* matrixIn1,const double* matrixIn2,double* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn1;
    mIn1.setData(matrixIn1);
    C4X4Matrix mIn2;
    mIn2.setData(matrixIn2);
    (mIn1*mIn2).getData(matrixOut);
    return(1);
}

int simMultiplyPoses_internal(const double* poseIn1,const double* poseIn2,double* poseOut)
{
    TRACE_C_API;

    C7Vector pIn1;
    pIn1.setData(poseIn1,true);
    C7Vector pIn2;
    pIn2.setData(poseIn2,true);
    (pIn1*pIn2).getData(poseOut,true);
    return(1);
}

int simPoseToMatrix_internal(const double* poseIn,double* matrixOut)
{
    TRACE_C_API;

    C7Vector pIn;
    pIn.setData(poseIn,true);
    pIn.getMatrix().getData(matrixOut);
    return(1);
}

int simMatrixToPose_internal(const double* matrixIn,double* poseOut)
{
    TRACE_C_API;

    C4X4Matrix mIn;
    mIn.setData(matrixIn);
    mIn.getTransformation().getData(poseOut,true);
    return(1);
}

int simInterpolateMatrices_internal(const double* matrixIn1,const double* matrixIn2,double interpolFactor,double* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn1;
    mIn1.setData(matrixIn1);
    C4X4Matrix mIn2;
    mIn2.setData(matrixIn2);
    C7Vector tr;
    tr.buildInterpolation(mIn1.getTransformation(),mIn2.getTransformation(),interpolFactor);
    (tr.getMatrix()).getData(matrixOut);
    return(1);
}

int simInterpolatePoses_internal(const double* poseIn1,const double* poseIn2,double interpolFactor,double* poseOut)
{
    TRACE_C_API;

    C7Vector pIn1;
    pIn1.setData(poseIn1,true);
    C7Vector pIn2;
    pIn2.setData(poseIn2,true);
    C7Vector tr;
    tr.buildInterpolation(pIn1,pIn2,interpolFactor);
    tr.getData(poseOut,true);
    return(1);
}

int simTransformVector_internal(const double* matrix,double* vect)
{
    TRACE_C_API;

    C4X4Matrix m;
    m.setData(matrix);
    C3Vector v(vect);
    (m*v).getData(vect);
    return(1);
}

int simReservedCommand_internal(int v,int w)
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

int simSetBoolParam_internal(int parameter,bool boolState)
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
                App::mainWindow->setCalcModulesToggleViaGuiEnabled_OLD(boolState!=0);
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
                App::currentWorld->simulation->setIsRealTimeSimulation(boolState!=0);
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

int simGetBoolParam_internal(int parameter)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_boolparam_realtime_simulation)
        {
            int retVal=0;
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->getIsRealTimeSimulation())
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
            if ((App::mainWindow!=nullptr)&&App::mainWindow->getCalcModulesToggleViaGuiEnabled_OLD())
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

int simSetArrayParam_internal(int parameter,const double* arrayOfValues)
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
            App::currentWorld->dynamicsContainer->setGravity(C3Vector(arrayOfValues));
            return(1);
        }

        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->setFogStart(arrayOfValues[0]);
            App::currentWorld->environment->setFogEnd(arrayOfValues[1]);
            App::currentWorld->environment->setFogDensity(arrayOfValues[2]);
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->fogBackgroundColor[0]=arrayOfValues[0];
            App::currentWorld->environment->fogBackgroundColor[1]=arrayOfValues[1];
            App::currentWorld->environment->fogBackgroundColor[2]=arrayOfValues[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->backGroundColorDown[0]=arrayOfValues[0];
            App::currentWorld->environment->backGroundColorDown[1]=arrayOfValues[1];
            App::currentWorld->environment->backGroundColorDown[2]=arrayOfValues[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->backGroundColor[0]=arrayOfValues[0];
            App::currentWorld->environment->backGroundColor[1]=arrayOfValues[1];
            App::currentWorld->environment->backGroundColor[2]=arrayOfValues[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            App::currentWorld->environment->ambientLightColor[0]=arrayOfValues[0];
            App::currentWorld->environment->ambientLightColor[1]=arrayOfValues[1];
            App::currentWorld->environment->ambientLightColor[2]=arrayOfValues[2];
            return(1);
        }

        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetArrayParam_internal(int parameter,double* arrayOfValues)
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
            g.getData(arrayOfValues);
            return(1);
        }
        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            arrayOfValues[0]=App::currentWorld->environment->getFogStart();
            arrayOfValues[1]=App::currentWorld->environment->getFogEnd();
            arrayOfValues[2]=App::currentWorld->environment->getFogDensity();
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            arrayOfValues[0]=App::currentWorld->environment->fogBackgroundColor[0];
            arrayOfValues[1]=App::currentWorld->environment->fogBackgroundColor[1];
            arrayOfValues[2]=App::currentWorld->environment->fogBackgroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            arrayOfValues[0]=App::currentWorld->environment->backGroundColorDown[0];
            arrayOfValues[1]=App::currentWorld->environment->backGroundColorDown[1];
            arrayOfValues[2]=App::currentWorld->environment->backGroundColorDown[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            arrayOfValues[0]=App::currentWorld->environment->backGroundColor[0];
            arrayOfValues[1]=App::currentWorld->environment->backGroundColor[1];
            arrayOfValues[2]=App::currentWorld->environment->backGroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::currentWorld->environment==nullptr)
                return(-1);
            arrayOfValues[0]=App::currentWorld->environment->ambientLightColor[0];
            arrayOfValues[1]=App::currentWorld->environment->ambientLightColor[1];
            arrayOfValues[2]=App::currentWorld->environment->ambientLightColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_random_euler)
        {
            C4Vector r;
            r.buildRandomOrientation();
            C3Vector euler(r.getEulerAngles());
            arrayOfValues[0]=euler(0);
            arrayOfValues[1]=euler(1);
            arrayOfValues[2]=euler(2);
            return(1);
        }
        if (parameter==sim_arrayparam_rayorigin)
        {
            arrayOfValues[0]=0.0;
            arrayOfValues[1]=0.0;
            arrayOfValues[2]=0.0;
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                C3Vector orig,dir;
                if (App::mainWindow->getMouseRay(orig,dir))
                {
                    orig.getData(arrayOfValues);
                    return(1);
                }
            }
#endif
            return(0);
        }
        if (parameter==sim_arrayparam_raydirection)
        {
            arrayOfValues[0]=0.0;
            arrayOfValues[1]=0.0;
            arrayOfValues[2]=1.0;
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                C3Vector orig,dir;
                if (App::mainWindow->getMouseRay(orig,dir))
                {
                    dir.getData(arrayOfValues);
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



int simSetInt32Param_internal(int parameter,int intState)
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
            App::currentWorld->simulation->setSpeedModifierCount(intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
                return(-1);
            if (App::currentWorld->dynamicsContainer->setIterationCount(intState))
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
        { // deprecated. Does nothing, and doesn't generate an error
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

int simGetUInt64Param_internal(int parameter,unsigned long long int* intState)
{
    TRACE_C_API;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_uint64param_simulation_time_step_ns)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=quint64(App::currentWorld->simulation->getTimeStep()*1000000000.0);
            return(1);
        }
        if (parameter==sim_uint64param_simulation_time_ns)
        {
            if (App::currentWorld->simulation==nullptr)
                return(-1);
            intState[0]=quint64(App::currentWorld->simulation->getSimulationTime()*1000000000.0);
            return(1);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetInt32Param_internal(int parameter,int* intState)
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

        if (parameter==sim_intparam_objectcreationcounter)
        {
            intState[0]=App::currentWorld->sceneObjects->getObjectCreationCounter();
            return(1);
        }
        if (parameter==sim_intparam_objectdestructioncounter)
        {
            intState[0]=App::currentWorld->sceneObjects->getObjectDestructionCounter();
            return(1);
        }
        if (parameter==sim_intparam_hierarchychangecounter)
        {
            intState[0]=App::currentWorld->sceneObjects->getHierarchyChangeCounter();
            return(1);
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
        { // deprecated. Does nothing, and doesn't generate an error
            intState[0]=0;
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
            intState[0]=App::currentWorld->simulation->getSpeedModifierCount();
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::currentWorld->dynamicsContainer->getIterationCount();
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

int simSetFloatParam_internal(int parameter,double floatState)
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
                App::currentWorld->simulation->setTimeStep(floatState);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_floatparam_physicstimestep)
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                App::currentWorld->dynamicsContainer->setDesiredStepSize(floatState);
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
                if (App::currentWorld->dynamicsContainer->setDesiredStepSize(floatState))
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

int simGetFloatParam_internal(int parameter,double* floatState)
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
            floatState[0]=App::currentWorld->simulation->getTimeStep();
            return(1);
        }
        if ( (parameter==sim_floatparam_physicstimestep)||(parameter==sim_floatparam_dynamic_step_size) )
        {
            if (App::currentWorld->dynamicsContainer==nullptr)
                return(-1);
            floatState[0]=App::currentWorld->dynamicsContainer->getEffectiveStepSize();
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

int simSetStringParam_internal(int parameter,const char* str)
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

char* simGetStringParam_internal(int parameter)
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
        if (parameter==sim_stringparam_mujocodir)
        {
            validParam=true;
            retVal=App::folders->getMujocoPath();
        }
        if (parameter==sim_stringparam_usersettingsdir)
        {
            validParam=true;
            retVal=App::folders->getUserSettingsPath();
        }
        if (parameter==sim_stringparam_systemdir)
        {
            validParam=true;
            retVal=App::folders->getSystemPath();
        }
        if (parameter==sim_stringparam_resourcesdir)
        {
            validParam=true;
            retVal=App::folders->getResourcesPath();
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

double simGetSimulationTime_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
        return(App::currentWorld->simulation->getSimulationTime());
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0);
}

int simGetSimulationState_internal()
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

double simGetSystemTime_internal()
{
    return(VDateTime::getTime());
}

int simLoadScene_internal(const char* filename)
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

int simCloseScene_internal()
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

int simLoadModel_internal(const char* filename)
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

char* simGetSimulatorMessage_internal(int* messageID,int* auxiliaryData,int* returnedDataSize)
{
    TRACE_C_API;

    char* retVal=App::worldContainer->simulatorMessageQueue->extractOneCommand(messageID[0],auxiliaryData,returnedDataSize[0]);
    return(retVal);
}

int simSaveScene_internal(const char* filename)
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

int simSaveModel_internal(int baseOfModelHandle,const char* filename)
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

int simDoesFileExist_internal(const char* filename)
{
    TRACE_C_API;

    if (!VFile::doesFileExist(filename))
        return(0);
    return(1);
}

int* simGetObjectSel_internal(int* cnt)
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

int simSetObjectSel_internal(const int* handles,int cnt)
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

int simHandleProximitySensor_internal(int sensorHandle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
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
                double smallestL=FLOAT_MAX;
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
            double allSmallestL=FLOAT_MAX;
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
                    double smallestL=smallest.getLength();

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


int simReadProximitySensor_internal(int sensorHandle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
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
                dPt.getData(detectedPoint);
                detectedPoint[3]=dPt.getLength();
            }
            if (detectedObjectHandle!=nullptr)
                detectedObjectHandle[0]=dObjHandle;
            if (normalVector!=nullptr)
                nVect.getData(normalVector);
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simHandleDynamics_internal(double deltaTime)
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

int simHandleMainScript_internal()
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

            App::currentWorld->embeddedScriptContainer->broadcastDataContainer.removeTimedOutObjects(App::currentWorld->simulation->getSimulationTime()); // remove invalid elements
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

int simResetScript_internal(int scriptHandle)
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

int simAssociateScriptWithObject_internal(int scriptHandle,int associatedObjectHandle)
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

int simAddScript_internal(int scriptProperty)
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

int simRemoveScript_internal(int scriptHandle)
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

int simResetProximitySensor_internal(int sensorHandle)
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

int simCheckProximitySensor_internal(int sensorHandle,int entityHandle,double* detectedPoint)
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
        int retVal=simCheckProximitySensorEx_internal(sensorHandle,entityHandle,options,FLOAT_MAX,it->getAllowedNormal(),detectedPoint,nullptr,nullptr);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCheckProximitySensorEx_internal(int sensorHandle,int entityHandle,int detectionMode,double detectionThreshold,double maxAngle,double* detectedPoint,int* detectedObjectHandle,double* normalVector)
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
        if (detectionThreshold<0.0)
            detectionThreshold=0.0;
        tt::limitValue(0.0,piValD2,maxAngle);
        int detectedObj;
        C3Vector dPoint;
        double minThreshold=-1.0;
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

int simCheckProximitySensorEx2_internal(int sensorHandle,double* vertexPointer,int itemType,int itemCount,int detectionMode,double detectionThreshold,double maxAngle,double* detectedPoint,double* normalVector)
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
        if (detectionThreshold<0.0)
            detectionThreshold=0.0;
        tt::limitValue(0.0,piValD2,maxAngle);
        C3Vector dPoint;
        double minThreshold=-1.0;
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

int simLoadModule_internal(const char* filenameAndPath,const char* pluginName)
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

int simUnloadModule_internal(int pluginhandle)
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

int simRegisterScriptCallbackFunction_internal(const char* funcNameAtPluginName,const char* callTips,void(*callBack)(struct SScriptCallBack* cb))
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

int simRegisterScriptVariable_internal(const char* varNameAtPluginName,const char* varValue,int stackHandle)
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

int simRegisterScriptFuncHook_internal(int scriptHandle,const char* funcToHook,const char* userFunction,bool executeBefore,int options)
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

void* simCreateBuffer_internal(int size)
{
    TRACE_C_API;

    void* retVal=(void*)new char[size];
    return(retVal);
}

int simReleaseBuffer_internal(const void* buffer)
{
    TRACE_C_API;

    delete[] buffer;
    return(1);
}

int simCheckCollision_internal(int entity1Handle,int entity2Handle)
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

int simCheckCollisionEx_internal(int entity1Handle,int entity2Handle,double** intersectionSegments)
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

        std::vector<double> intersect;
        CCollisionRoutine::doEntitiesCollide(entity1Handle,entity2Handle,&intersect,true,true,nullptr);
        if ( (intersectionSegments!=nullptr)&&(intersect.size()!=0) )
        {
            intersectionSegments[0]=new double[intersect.size()];
            for (int i=0;i<int(intersect.size());i++)
                (*intersectionSegments)[i]=intersect[i];
        }
        return((int)intersect.size()/6);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCheckDistance_internal(int entity1Handle,int entity2Handle,double threshold,double* distanceData)
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
        if (threshold<=0.0)
            threshold=FLOAT_MAX;
        bool result=CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1Handle,entity2Handle,threshold,distanceData,buffer,buffer+2,true,true);
        App::currentWorld->cacheData->setCacheDataDist(entity1Handle,entity2Handle,buffer);
        if (result)
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simAdvanceSimulationByOneStep_internal()
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

int simSetSimulationTimeStep_internal(double timeStep)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        App::currentWorld->simulation->setTimeStep(timeStep);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

double simGetSimulationTimeStep_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
        return(App::currentWorld->simulation->getTimeStep());
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0);
}

int simGetRealTimeSimulation_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->simulation->getIsRealTimeSimulation())
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

int simAdjustRealTimeTimer_internal(int instanceIndex,double deltaTime)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::currentWorld->simulation->adjustRealTimeTimer(deltaTime);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simIsRealTimeSimulationStepNeeded_internal()
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
        if (!App::currentWorld->simulation->getIsRealTimeSimulation())
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

int simGetSimulationPassesPerRenderingPass_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::currentWorld->simulation->getPassesPerRendering();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetSimulationPassesPerRenderingPass_internal(int p)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        p=tt::getLimitedInt(1,512,p);
        App::currentWorld->simulation->setPassesPerRendering(p);
        return(App::currentWorld->simulation->getPassesPerRendering());
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


int simStartSimulation_internal()
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

int simStopSimulation_internal()
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

int simPauseSimulation_internal()
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

int simHandleGraph_internal(int graphHandle,double simulationTime)
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

int simResetGraph_internal(int graphHandle)
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

int simAddGraphStream_internal(int graphHandle,const char* streamName,const char* unitStr,int options,const float* color,double cyclicRange)
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

int simDestroyGraphCurve_internal(int graphHandle,int curveId)
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

int simSetGraphStreamTransformation_internal(int graphHandle,int streamId,int trType,double mult,double off,int movingAvgPeriod)
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

int simDuplicateGraphCurveToStatic_internal(int graphHandle,int curveId,const char* curveName)
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

int simAddGraphCurve_internal(int graphHandle,const char* curveName,int dim,const int* streamIds,const double* defaultValues,const char* unitStr,int options,const float* color,int curveWidth)
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

int simSetGraphStreamValue_internal(int graphHandle,int streamId,double value)
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

char* simGetModuleName_internal(int index,unsigned char* moduleVersion)
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

int simAddLog_internal(const char* pluginName,int verbosityLevel,const char* logMsg)
{ // keep this as simple as possible (no trace, no thread checking). For now
    int retVal=0;
    if ( (pluginName==nullptr)||(logMsg==nullptr) )
    {
        App::clearStatusbar();
        retVal=1;
    }
    else
    {
        if (App::logPluginMsg(pluginName,verbosityLevel,logMsg))
            retVal=1;
    }
    return(retVal);
}

int simSetNavigationMode_internal(int navigationMode)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    App::setMouseMode(navigationMode);
    return(1);
}

int simGetNavigationMode_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    int retVal=App::getMouseMode();
    return(retVal);
}

int simSetPage_internal(int index)
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

int simGetPage_internal()
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

int simCopyPasteObjects_internal(int* objectHandles,int objectCount,int options)
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

int simScaleObjects_internal(const int* objectHandles,int objectCount,double scalingFactor,bool scalePositionsToo)
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

int simAddDrawingObject_internal(int objectType,double size,double duplicateTolerance,int parentObjectHandle,int maxItemCount,const float* color,const float* setToNULL,const float* setToNULL2,const float* setToNULL3)
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
        if (color!=nullptr)
            it->color.setColor(color,sim_colorcomponent_ambient_diffuse);
        if (setToNULL2!=nullptr)
            it->color.setColor(setToNULL2,sim_colorcomponent_specular);
        if (setToNULL3!=nullptr)
        {
            it->color.setColor(setToNULL3,sim_colorcomponent_emission);
            if ((objectType&sim_drawing_auxchannelcolor1)!=0)
                it->color.setColor(setToNULL3+3,sim_colorcomponent_auxiliary);
        }
        int retVal=App::currentWorld->drawingCont->addObject(it);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simRemoveDrawingObject_internal(int objectHandle)
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

int simAddDrawingObjectItem_internal(int objectHandle,const double* itemData)
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

double simGetObjectSizeFactor_internal(int objectHandle)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1.0);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1.0);
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

        double retVal=it->getSizeFactor();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simAnnounceSceneContentChange_internal()
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::currentWorld->undoBufferContainer->announceChange())
            return(1);
        return(0);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetInt32Signal_internal(const char* signalName,int signalValue)
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

int simGetInt32Signal_internal(const char* signalName,int* signalValue)
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

int simClearInt32Signal_internal(const char* signalName)
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

int simSetFloatSignal_internal(const char* signalName,double signalValue)
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

int simGetFloatSignal_internal(const char* signalName,double* signalValue)
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

int simClearFloatSignal_internal(const char* signalName)
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

int simSetStringSignal_internal(const char* signalName,const char* signalValue,int stringLength)
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

char* simGetStringSignal_internal(const char* signalName,int* stringLength)
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

int simClearStringSignal_internal(const char* signalName)
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

char* simGetSignalName_internal(int signalIndex,int signalType)
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
            res=App::currentWorld->signalContainer->getDoubleSignalNameAtIndex_old(signalIndex,sigName);

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

int simSetObjectProperty_internal(int objectHandle,int prop)
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

int simGetObjectProperty_internal(int objectHandle)
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

int simSetObjectSpecialProperty_internal(int objectHandle,int prop)
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

int simGetObjectSpecialProperty_internal(int objectHandle)
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

int simSetModelProperty_internal(int objectHandle,int modelProperty)
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

int simGetModelProperty_internal(int objectHandle)
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

int simReadForceSensor_internal(int objectHandle,double* forceVector,double* torqueVector)
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
                f.getData(forceVector);
            if (torqueVector!=nullptr)
                t.getData(torqueVector);
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

int simGetLightParameters_internal(int objectHandle,double* setToNULL,double* diffusePart,double* specularPart)
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
                setToNULL[0+i]=0.0;
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

int simSetLightParameters_internal(int objectHandle,int state,const float* setToNULL,const float* diffusePart,const float* specularPart)
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

int simGetVelocity_internal(int shapeHandle,double* linearVelocity,double* angularVelocity)
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
            lv.getData(linearVelocity);
        if (angularVelocity!=nullptr)
            av.getData(angularVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetObjectVelocity_internal(int objectHandle,double* linearVelocity,double* angularVelocity)
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
            lv.getData(linearVelocity);
        if (angularVelocity!=nullptr)
            av.getData(angularVelocity);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetJointVelocity_internal(int jointHandle,double* velocity)
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

int simAddForceAndTorque_internal(int shapeHandle,const double* force,const double* torque)
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
            f.setData(force);
        if (torque!=nullptr)
            t.setData(torque);

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

int simAddForce_internal(int shapeHandle,const double* position,const double* force)
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


int simSetExplicitHandling_internal(int objectHandle,int explicitFlags)
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
        if ( (objectHandle>=SIM_IDSTART_COLLISION_old)&&(objectHandle<SIM_IDEND_COLLISION_old) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (objectHandle>=SIM_IDSTART_DISTANCE_old)&&(objectHandle<SIM_IDEND_DISTANCE_old) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(objectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (objectHandle>=SIM_IDSTART_IKGROUP_old)&&(objectHandle<SIM_IDEND_IKGROUP_old) )
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

int simGetExplicitHandling_internal(int objectHandle)
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
        if ( (objectHandle>=SIM_IDSTART_COLLISION_old)&&(objectHandle<SIM_IDEND_COLLISION_old) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(objectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (objectHandle>=SIM_IDSTART_DISTANCE_old)&&(objectHandle<SIM_IDEND_DISTANCE_old) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(objectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (objectHandle>=SIM_IDSTART_IKGROUP_old)&&(objectHandle<SIM_IDEND_IKGROUP_old) )
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

int simGetLinkDummy_internal(int dummyHandle)
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

int simSetLinkDummy_internal(int dummyHandle,int linkedDummyHandle)
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

int simSetObjectColor_internal(int objectHandle,int index,int colorComponent,const float* rgbData)
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
                    geom->color.setTranslucid(rgbData[0]!=0.0);
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

int simGetObjectColor_internal(int objectHandle,int index,int colorComponent,float* rgbData)
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

int simSetShapeColor_internal(int shapeHandle,const char* colorName,int colorComponent,const float* rgbData)
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

int simGetShapeColor_internal(int shapeHandle,const char* colorName,int colorComponent,float* rgbData)
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

int simResetDynamicObject_internal(int objectHandle)
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

int simSetJointMode_internal(int jointHandle,int jointMode,int options)
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

int simGetJointMode_internal(int jointHandle,int* options)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isJoint(__func__,jointHandle))
            return(-1);
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

int simSerialOpen_internal(const char* portString,int baudRate,void* reserved1,void* reserved2)
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

int simSerialClose_internal(int portHandle)
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

int simSerialSend_internal(int portHandle,const char* data,int dataLength)
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

int simSerialRead_internal(int portHandle,char* buffer,int dataLengthToRead)
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

int simSerialCheck_internal(int portHandle)
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

int simGetContactInfo_internal(int dynamicPass,int objectHandle,int index,int* objectHandles,double* contactInfo)
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

int simAuxiliaryConsoleOpen_internal(const char* title,int maxLines,int mode,const int* position,const int* size,const float* textColor,const float* backgroundColor)
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
                    tCol[i]=int(textColor[i]*255.1);
                if (backgroundColor!=nullptr)
                    bCol[i]=int(backgroundColor[i]*255.1);
            }
            retVal=App::mainWindow->codeEditorContainer->openConsole(title,maxLines,mode,position,size,tCol,bCol,-1);
        }
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simAuxiliaryConsoleClose_internal(int consoleHandle)
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

int simAuxiliaryConsoleShow_internal(int consoleHandle,bool showState)
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

int simAuxiliaryConsolePrint_internal(int consoleHandle,const char* text)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=1; // in headless mode we fake success
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            retVal=0;
            if (text==nullptr)
            {
                if (App::mainWindow->codeEditorContainer->setText(consoleHandle,""))
                    retVal=1;
            }
            else
            {
                if (App::mainWindow->codeEditorContainer->appendText(consoleHandle,text))
                    retVal=1;
            }
        }
#endif
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simImportShape_internal(int fileformat,const char* pathAndFilename,int options,double identicalVerticeTolerance,double scalingFactor)
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

int simImportMesh_internal(int fileformat,const char* pathAndFilename,int options,double identicalVerticeTolerance,double scalingFactor,double*** vertices,int** verticesSizes,int*** indices,int** indicesSizes,double*** reserved,char*** names)
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

int simExportMesh_internal(int fileformat,const char* pathAndFilename,int options,double scalingFactor,int elementCount,const double** vertices,const int* verticesSizes,const int** indices,const int* indicesSizes,double** reserved,const char** names)
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


int simCreateMeshShape_internal(int options,double shadingAngle,const double* vertices,int verticesSize,const int* indices,int indicesSize,double* reserved)
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
                    std::vector<double> vert(vertices,vertices+verticesSize);
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

int simGetShapeMesh_internal(int shapeHandle,double** vertices,int* verticesSize,int** indices,int* indicesSize,double** normals)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        std::vector<double> wvert;
        std::vector<int> wind;
        std::vector<double> wnorm;
        it->getMeshWrapper()->getCumulativeMeshes(wvert,&wind,&wnorm);
        vertices[0]=new double[wvert.size()];
        verticesSize[0]=int(wvert.size());
        indices[0]=new int[wind.size()];
        indicesSize[0]=int(wind.size());
        if (normals!=nullptr)
            normals[0]=new double[wnorm.size()];
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

int simCreatePrimitiveShape_internal(int primitiveType,const double* sizes,int options)
{ // options: bit: 0=culling, 1=sharp edges, 2=open
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        C3Vector s(tt::getLimitedFloat(0.00001,100000.0,sizes[0]),tt::getLimitedFloat(0.00001,100000.0,sizes[1]),tt::getLimitedFloat(0.00001,100000.0,sizes[2]));
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

int simCreateDummy_internal(double size,const float* reserved)
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

int simCreateProximitySensor_internal(int sensorType,int subType,int options,const int* intParams,const double* floatParams,const double* reserved)
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

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCreateForceSensor_internal(int options,const int* intParams,const double* floatParams,const double* reserved)
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

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simCreateVisionSensor_internal(int options,const int* intParams,const double* floatParams,const double* reserved)
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
        float w[3]={(float)floatParams[6],(float)floatParams[6],(float)floatParams[6]};
        it->setDefaultBufferValues(w);

        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}


int simCreateJoint_internal(int jointType,int jointMode,int options,const double* sizes,const double* reservedA,const double* reservedB)
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
        App::currentWorld->sceneObjects->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simFloatingViewAdd_internal(double posX,double posY,double sizeX,double sizeY,int options)
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
        posX=1.0-posX;
        posY=1.0-posY;
        if (posX<0.01)
            posX=0.01;
        if (posX>0.99)
            posX=0.99;
        if (posY<0.01)
            posY=0.01;
        if (posY>0.99)
            posY=0.99;
        sizeX=std::min<double>(sizeX,2.0*std::min<double>(posX,1.0-posX));
        sizeY=std::min<double>(sizeY,2.0*std::min<double>(posY,1.0-posY));
        double sizes[2]={sizeX,sizeY};
        double positions[2]={posX-sizeX*0.5,posY-sizeY*0.5};
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

int simFloatingViewRemove_internal(int floatingViewHandle)
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

int simCameraFitToView_internal(int viewHandleOrIndex,int objectCount,const int* objectHandles,int options,double scaling)
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
        double xByY=1.0;
        if ((options&2)==0)
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                int vs[2];
                view->getViewSize(vs);
                xByY=double(vs[0])/double(vs[1]);
            }
            else
#endif
                xByY=455.0/256.0; // in headless mode
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


int simAdjustView_internal(int viewHandleOrIndex,int associatedViewableObjectHandle,int options,const char* viewLabel)
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

int simCreateHeightfieldShape_internal(int options,double shadingAngle,int xPointCount,int yPointCount,double xSize,const double* heights)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ( (xPointCount<2)||(xPointCount>2048)||(yPointCount<2)||(yPointCount>2048)||(xSize<0.00001) )
        {
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_ARGUMENTS);
            return(-1);
        }
        std::vector<std::vector<double>*> allData;
        for (int i=0;i<yPointCount;i++)
        {
            std::vector<double>* vect=new std::vector<double>;
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

int simGetObjectInt32Param_internal(int objectHandle,int parameterID,int* parameter)
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
            if (parameterID==sim_shapeintparam_kinematic)
            {
                parameter[0]=0;
                if (shape->getShapeIsDynamicallyKinematic())
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

int simSetObjectInt32Param_internal(int objectHandle,int parameterID,int parameter)
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
            if (parameterID==sim_shapeintparam_kinematic)
            {
                shape->setShapeIsDynamicallyKinematic(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_respondablesuspendcnt)
            {
                shape->setRespondableSuspendCount(parameter);
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

int simGetObjectFloatParam_internal(int objectHandle,int parameterID,double* parameter)
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
                parameter[0]=double(light->getSpotExponent());
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
                parameter[0]=2.0;
                if (tt::getValueOfKey("focalDist@povray",extensionString.c_str(),val))
                {
                    double dist;
                    if (tt::getValidFloat(val.c_str(),dist))
                        parameter[0]=dist;
                }
                retVal=1;
            }
            if (parameterID==sim_visionfloatparam_pov_aperture)
            {
                std::string extensionString(rendSens->getExtensionString());
                std::string val;
                parameter[0]=0.05;
                if (tt::getValueOfKey("aperture@povray",extensionString.c_str(),val))
                {
                    double ap;
                    if (tt::getValidFloat(val.c_str(),ap))
                        parameter[0]=ap;
                }
                retVal=1;
            }
        }
        if (joint!=nullptr)
        {
            if ((parameterID==sim_jointfloatparam_pid_p)||(parameterID==sim_jointfloatparam_pid_i)||(parameterID==sim_jointfloatparam_pid_d))
            { // deprecated parameter
                double pp,ip,dp;
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
                double kp,cp;
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
                double p,o;
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
                double v[3];
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
                double maxVelAccelJerk[3];
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
                            double dummyFloat;
                            tp->getTextureScaling(parameter[0],dummyFloat);
                        }
                        if (parameterID==sim_shapefloatparam_texture_scaling_y)
                        {
                            double dummyFloat;
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
                double sMin[4];
                double sRange[4];
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
                double p=forceSensor->getDynamicPositionError();
                double o=forceSensor->getDynamicOrientationError();
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
                parameter[0]=2.0;
                if (tt::getValueOfKey("focalDist@povray",extensionString.c_str(),val))
                {
                    double dist;
                    if (tt::getValidFloat(val.c_str(),dist))
                        parameter[0]=dist;
                }
                retVal=1;
            }
            if (parameterID==sim_camerafloatparam_pov_aperture)
            {
                std::string extensionString(camera->getExtensionString());
                std::string val;
                parameter[0]=0.05;
                if (tt::getValueOfKey("aperture@povray",extensionString.c_str(),val))
                {
                    double ap;
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

int simSetObjectFloatParam_internal(int objectHandle,int parameterID,double parameter)
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
                light->setSpotExponent(int(parameter+0.5));
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
            { // deprecated parameter
                double pp,ip,dp;
                joint->getPid(pp,ip,dp);
                if (parameterID==sim_jointfloatparam_pid_p)
                    pp=parameter;
                if (parameterID==sim_jointfloatparam_pid_i)
                    ip=parameter;
                if (parameterID==sim_jointfloatparam_pid_d)
                    dp=parameter;
                joint->setPid_old(pp,ip,dp);
                retVal=1;
            }
            if ((parameterID==sim_jointfloatparam_kc_k)||(parameterID==sim_jointfloatparam_kc_c))
            {
                double kp,cp;
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
                    static double buff[3];
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
                double maxVelAccelJerk[3];
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
                double v[3];
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
                        double scalingX,scalingY;
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
                double sMin[4];
                double sRange[4];
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

double* simGetObjectFloatArrayParam_internal(int objectHandle,int parameterID,int* size)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    if (!doesObjectExist(__func__,objectHandle))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        double* retVal=nullptr; // Means the parameter was not retrieved
        CVisionSensor* vision=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CForceSensor* forceSensor=App::currentWorld->sceneObjects->getForceSensorFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
            }
        }
        if (light!=nullptr)
        {
        }
        if (vision!=nullptr)
        {
            if (parameterID==sim_visionfarrayparam_viewfrustum)
            {
                C3Vector nearV,farV;
                vision->getVolumeVectors(nearV,farV);
                retVal=new double[6];
                for (size_t i=0;i<3;i++)
                {
                    retVal[i]=nearV(i);
                    retVal[i+3]=farV(i);
                }
                if (size!=nullptr)
                    size[0]=6;
            }
        }
        if (camera!=nullptr)
        {
            if (parameterID==sim_camerafarrayparam_viewfrustum)
            {
                C3Vector nearV,farV;
                camera->getVolumeVectors(nearV,farV);
                retVal=new double[6];
                for (size_t i=0;i<3;i++)
                {
                    retVal[i]=nearV(i);
                    retVal[i+3]=farV(i);
                }
                if (size!=nullptr)
                    size[0]=6;
            }
        }
        if (joint!=nullptr)
        {
        }
        if (shape!=nullptr)
        {
        }
        if (forceSensor!=nullptr)
        {
        }
        if (dummy!=nullptr)
        {
        }

        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int simSetObjectFloatArrayParam_internal(int objectHandle,int parameterID,const double* params,int size)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    if (!doesObjectExist(__func__,objectHandle))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not set
        CVisionSensor* vision=App::currentWorld->sceneObjects->getVisionSensorFromHandle(objectHandle);
        CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(objectHandle);
        CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(objectHandle);
        CLight* light=App::currentWorld->sceneObjects->getLightFromHandle(objectHandle);
        CCamera* camera=App::currentWorld->sceneObjects->getCameraFromHandle(objectHandle);
        CDummy* dummy=App::currentWorld->sceneObjects->getDummyFromHandle(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
            }
        }
        if (light!=nullptr)
        {
        }
        if (vision!=nullptr)
        {
        }
        if (camera!=nullptr)
        {
        }
        if (joint!=nullptr)
        {
        }
        if (shape!=nullptr)
        {
        }
        if (dummy!=nullptr)
        {
        }
        if (retVal==0)
            CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_SET_PARAMETER);
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

char* simGetObjectStringParam_internal(int objectHandle,int parameterID,int* parameterLength)
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
        char* retVal=nullptr; // Means the parameter was not retrieved
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

int simSetObjectStringParam_internal(int objectHandle,int parameterID,const char* parameter,int parameterLength)
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

int simGetScriptInt32Param_internal(int scriptHandle,int parameterID,int* parameter)
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
            parameter[0]=it->getScriptExecPriority();
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

int simSetScriptInt32Param_internal(int scriptHandle,int parameterID,int parameter)
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
            it->setScriptExecPriority(parameter);
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

char* simGetScriptStringParam_internal(int scriptHandle,int parameterID,int* parameterLength)
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

int simSetScriptStringParam_internal(int scriptHandle,int parameterID,const char* parameter,int parameterLength)
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

int simGetRotationAxis_internal(const double* matrixStart,const double* matrixGoal,double* axis,double* angle)
{
    TRACE_C_API;

    C4X4Matrix mStart;
    mStart.setData(matrixStart);
    C4X4Matrix mGoal;
    mGoal.setData(matrixGoal);

    // Following few lines taken from the quaternion interpolation part:
    C4Vector AA(mStart.M.getQuaternion());
    C4Vector BB(mGoal.M.getQuaternion());
    if (AA(0)*BB(0)+AA(1)*BB(1)+AA(2)*BB(2)+AA(3)*BB(3)<0.0)
        AA=AA*-1.0;
    C4Vector r((AA.getInverse()*BB).getAngleAndAxis());

    C3Vector v(r(1),r(2),r(3));
    v=AA*v;

    axis[0]=v(0);
    axis[1]=v(1);
    axis[2]=v(2);
    double l=sqrt(v(0)*v(0)+v(1)*v(1)+v(2)*v(2));
    if (l!=0.0)
    {
        axis[0]/=l;
        axis[1]/=l;
        axis[2]/=l;
    }
    angle[0]=r(0);

    return(1);
}

int simRotateAroundAxis_internal(const double* matrixIn,const double* axis,const double* axisPos,double angle,double* matrixOut)
{
    TRACE_C_API;

    C4X4Matrix mIn;
    mIn.setData(matrixIn);
    C7Vector m(mIn);
    C3Vector ax(axis);
    C3Vector pos(axisPos);

    double alpha=-atan2(ax(1),ax(0));
    double beta=atan2(-sqrt(ax(0)*ax(0)+ax(1)*ax(1)),ax(2));
    m.X-=pos;
    C7Vector r;
    r.X.clear();
    r.Q.setEulerAngles(0.0,0.0,alpha);
    m=r*m;
    r.Q.setEulerAngles(0.0,beta,0.0);
    m=r*m;
    r.Q.setEulerAngles(0.0,0.0,angle);
    m=r*m;
    r.Q.setEulerAngles(0.0,-beta,0.0);
    m=r*m;
    r.Q.setEulerAngles(0.0,0.0,-alpha);
    m=r*m;
    m.X+=pos;
    m.getMatrix().getData(matrixOut);

    return(1);
}

int simGetJointForce_internal(int jointHandle,double* forceOrTorque)
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
        double f;
        forceOrTorque[0]=0.0;
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

int simGetJointTargetForce_internal(int jointHandle,double* forceOrTorque)
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

int simSetJointTargetForce_internal(int objectHandle,double forceOrTorque,bool signedValue)
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

int simPersistentDataWrite_internal(const char* dataTag,const char* dataValue,int dataLength,int options)
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

char* simPersistentDataRead_internal(const char* dataTag,int* dataLength)
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

int simIsHandle_internal(int generalObjectHandle,int generalObjectType)
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

int simHandleVisionSensor_internal(int visionSensorHandle,double** auxValues,int** auxValuesCount)
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
                    auxValues[0]=new double[fvs];
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

int simReadVisionSensor_internal(int visionSensorHandle,double** auxValues,int** auxValuesCount)
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
                auxValues[0]=new double[fvs];
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

int simResetVisionSensor_internal(int visionSensorHandle)
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


int simCheckVisionSensor_internal(int sensorHandle,int entityHandle,double** auxValues,int** auxValuesCount)
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
            auxValues[0]=new double[fvs];
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

float* simCheckVisionSensorEx_internal(int sensorHandle,int entityHandle,bool returnImage)
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


unsigned char* simGetVisionSensorImg_internal(int sensorHandle,int options,double rgbaCutOff,const int* pos,const int* size,int* resolution)
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

int simSetVisionSensorImg_internal(int sensorHandle,const unsigned char* img,int options,const int* pos,const int* size)
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

float* simGetVisionSensorDepth_internal(int sensorHandle,int options,const int* pos,const int* size,int* resolution)
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
            float n=(float)it->getNearClippingPlane();
            float f=(float)it->getFarClippingPlane();
            float fmn=f-n;
            for (int i=0;i<sizeX*sizeY;i++)
                retBuff[i]=n+fmn*retBuff[i];
        }
        return(retBuff);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

int _simSetVisionSensorDepth_internal(int sensorHandle,int options,const float* depth)
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        it->writeImage(depth,2);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simRuckigPos_internal(int dofs,double baseCycleTime,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetPos,const double* targetVel,double* reserved1,int* reserved2)
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

int simRuckigVel_internal(int dofs,double baseCycleTime,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetVel,double* reserved1,int* reserved2)
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

int simRuckigStep_internal(int objHandle,double cycleTime,double* newPos,double* newVel,double* newAccel,double* syncTime,double* reserved1,int* reserved2)
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

int simRuckigRemove_internal(int objHandle)
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

int simGetObjectQuaternion_internal(int objectHandle,int relativeToObjectHandle,double* quaternion)
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
        tr.Q.getData(quaternion,(handleFlags&sim_handleflag_wxyzquat)==0);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetObjectQuaternion_internal(int objectHandle,int relativeToObjectHandle,const double* quaternion)
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
            q.setData(quaternion,(handleFlags&sim_handleflag_wxyzquat)==0);
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),q.getEulerAngles());
        }
        else
        {
            if ( (it->getParent()==relObj)&&((handleFlags&sim_handleflag_reljointbaseframe)==0) )
            { // special here, in order to not lose precision in a series of get/set
                C7Vector tr(it->getLocalTransformation());
                tr.Q.setData(quaternion,(handleFlags&sim_handleflag_wxyzquat)==0);
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
                x.Q.setData(quaternion,(handleFlags&sim_handleflag_wxyzquat)==0);
                absTr=relTr*x;
                App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(it->getObjectHandle(),absTr.Q.getEulerAngles());
            }
        }
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetShapeMass_internal(int shapeHandle,double* mass)
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

int simSetShapeMass_internal(int shapeHandle,double mass)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);

        if (mass<0.0000001)
            mass=0.0000001;
        it->getMeshWrapper()->setMass(mass);
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGetShapeInertia_internal(int shapeHandle,double* inertiaMatrix,double* transformationMatrix)
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
        tr.getData(transformationMatrix);

        C3X3Matrix m;
        m.clear();
        m.axis[0](0)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(0);
        m.axis[1](1)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(1);
        m.axis[2](2)=it->getMeshWrapper()->getPrincipalMomentsOfInertia()(2);
        m*=it->getMeshWrapper()->getMass(); // in CoppeliaSim we work with the "massless inertia"
        m.getData(inertiaMatrix);

        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simSetShapeInertia_internal(int shapeHandle,const double* inertiaMatrix,const double* transformationMatrix)
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
        tr.setData(transformationMatrix);

        C3X3Matrix m;
        m.setData(inertiaMatrix);
        m.axis[0](1)=m.axis[1](0);
        m.axis[0](2)=m.axis[2](0);
        m.axis[1](2)=m.axis[2](1);
        m/=it->getMeshWrapper()->getMass(); // in CoppeliaSim we work with the "massless inertia"

        C7Vector corr;
        corr.setIdentity();
        C3Vector pmoment;
        CMeshWrapper::findPrincipalMomentOfInertia(m,corr.Q,pmoment);

        if (pmoment(0)<0.0000001)
            pmoment(0)=0.0000001;
        if (pmoment(1)<0.0000001)
            pmoment(1)=0.0000001;
        if (pmoment(2)<0.0000001)
            pmoment(0)=0.0000001;
        it->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
        it->getMeshWrapper()->setLocalInertiaFrame(tr.getTransformation()*corr);
        it->setDynamicsResetFlag(true,false);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simIsDynamicallyEnabled_internal(int objectHandle)
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
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simGenerateShapeFromPath_internal(const double* pppath,int pathSize,const double* section,int sectionSize,int options,const double* upVector,double reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        // First make sure the points are not coincident:
        std::vector<double> ppath;
        C3Vector prevV;
        prevV.clear();
        C4Vector prevQ;
        prevQ.clear();
        for (int i=0;i<pathSize/7;i++)
        {
            C3Vector v(pppath+7*i);
            C4Vector q(pppath+7*i+3,true);
            double d=(prevV-v).getLength();
            if ( (d>=0.0005)||(i==0) )
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
                zvect.setData(upVector);
            else
                zvect=C3Vector::unitZVector;
            bool closedPath=(options&4)!=0;
            int axis=options&3;

            size_t confCnt=size_t(pathSize)/7;
            size_t elementCount=confCnt;
            size_t secVertCnt=size_t(sectionSize)/2;
            std::vector<double> path;
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

            std::vector<double> vertices;
            std::vector<int> indices;
            C7Vector tr0;
            tr0.setData(&path[0]);
            for (size_t i=0;i<=secVertCnt-1;i++)
            {
                C3Vector v(section[i*2+0],0.0,section[i*2+1]);
                v=tr0*v;
                vertices.push_back(v(0));
                vertices.push_back(v(1));
                vertices.push_back(v(2));
            }

            int previousVerticesOffset=0;
            for (size_t ec=1;ec<elementCount;ec++)
            {
                C7Vector tr;
                tr.setData(&path[ec*7]);
                int forwOff=int(secVertCnt);
                for (int i=0;i<=int(secVertCnt)-1;i++)
                {
                    C3Vector v(section[i*2+0],0.0,section[i*2+1]);
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
            int h=simCreateMeshShape_internal(0,0.0,&vertices[0],int(vertices.size()),&indices[0],int(indices.size()),nullptr);
            return(h);
        }
        CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_INVALID_PATH);
        return(-1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

double simGetClosestPosOnPath_internal(const double* path,int pathSize,const double* pathLengths,const double* absPt)
{
    TRACE_C_API;
    double retVal=0.0;

    if (pathSize>=6)
    {
        double d=FLOAT_MAX;
        C3Vector pppt(absPt);
        for (int i=0;i<(pathSize/3)-1;i++)
        {
            C3Vector v0(path+i*3);
            C3Vector v1(path+(i+1)*3);
            C3Vector vd(v1-v0);
            C3Vector theSearchedPt;
            if (CMeshRoutines::getMinDistBetweenSegmentAndPoint_IfSmaller(v0,vd,pppt,d,theSearchedPt))
            {
                double vdL=vd.getLength();
                if (vdL==0.0)
                    retVal=pathLengths[i]; // // Coinciding points
                else
                {
                    double l=(theSearchedPt-v0).getLength();
                    double c=l/vdL;
                    retVal=pathLengths[i]*(1.0-c)+pathLengths[i+1]*c;
                }
            }
        }
    }
    return(retVal);
}

int simInitScript_internal(int scriptHandle)
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

int simModuleEntry_internal(int handle,const char* label,int state)
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

int simCheckExecAuthorization_internal(const char* what,const char* args,int scriptHandle)
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
                    CPersistentDataContainer cont;
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
                        CPersistentDataContainer cont;
                        cont.writeData(y.c_str(),"OK",true);
                    }
                }
            }
#endif
            if (auth)
                retVal=1;
            else
            {
                std::string tmp("function was hindered to execute for your safety. You can enable its execution and every other unsafe function with 'executeUnsafe=true' in ");
                tmp+=App::folders->getUserSettingsPath()+"/usrset.txt, at your own risk!";
                CApiErrors::setLastWarningOrError(__func__,tmp.c_str());
            }
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simGroupShapes_internal(const int* shapeHandles,int shapeCount)
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

int* simUngroupShape_internal(int shapeHandle,int* shapeCount)
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

int simConvexDecompose_internal(int shapeHandle,int options,const int* intParams,const double* floatParams)
{ // one shape at a time!
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    int retVal=CSceneObjectOperations::convexDecompose_apiVersion(shapeHandle,options,intParams,floatParams);
    return(retVal);
}

void simQuitSimulator_internal(bool ignoredArgument)
{
    TRACE_C_API;
    SSimulationThreadCommand cmd;
    cmd.cmdId=EXIT_REQUEST_CMD;
    App::appendSimulationThreadCommand(cmd);
}

int simSetShapeMaterial_internal(int shapeHandle,int materialIdOrShapeHandle)
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

int simGetTextureId_internal(const char* textureName,int* resolution)
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

unsigned char* simReadTexture_internal(int textureId,int options,int posX,int posY,int sizeX,int sizeY)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

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
                unsigned char* retVal=to->readPortionOfTexture(posX,posY,sizeX,sizeY);
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

int simWriteTexture_internal(int textureId,int options,const char* data,int posX,int posY,int sizeX,int sizeY,double interpol)
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

int simCreateTexture_internal(const char* fileName,int options,const double* planeSizes,const double* scalingUV,const double* xy_g,int fixedResolution,int* textureId,int* resolution,const void* reserved)
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
                    C3Vector s(0.1,0.1,0.00001);
                    if (planeSizes!=nullptr)
                        s=C3Vector(tt::getLimitedFloat(0.00001,100000.0,planeSizes[0]),tt::getLimitedFloat(0.00001,100000.0,planeSizes[1]),0.00001);
                    CShape* shape=CAddOperations::addPrimitiveShape(sim_primitiveshape_plane,s);

                    C7Vector identity;
                    identity.setIdentity();
                    shape->setLocalTransformation(identity);
                    shape->setCulling(false);
                    shape->setVisibleEdges(false);
                    shape->setRespondable(false);
                    shape->setShapeIsDynamicallyStatic(true);
                    shape->getMeshWrapper()->setMass(1.0);

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
                        tr.Q=C4Vector(0.0,0.0,xy_g[2]);
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
                C3Vector s(0.1,0.1,0.00001);
                if (planeSizes!=nullptr)
                    s=C3Vector(tt::getLimitedFloat(0.00001,100000.0,planeSizes[0]),tt::getLimitedFloat(0.00001,100000.0,planeSizes[1]),0.00001);
                CShape* shape=CAddOperations::addPrimitiveShape(sim_primitiveshape_plane,s);
                C7Vector identity;
                identity.setIdentity();
                shape->setLocalTransformation(identity);
                shape->setCulling(false);
                shape->setVisibleEdges(false);
                shape->setRespondable(false);
                shape->setShapeIsDynamicallyStatic(true);
                shape->getMeshWrapper()->setMass(1.0);

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
                    tr.Q=C4Vector(0.0,0.0,xy_g[2]);
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

int simWriteCustomDataBlock_internal(int objectHandle,const char* tagName,const char* data,int dataSize)
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

        if ( (objectHandle>=SIM_IDSTART_SCENEOBJECT)&&(objectHandle<=SIM_IDEND_SCENEOBJECT) )
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
        if ( (objectHandle>=SIM_IDSTART_LUASCRIPT)&&(objectHandle<=SIM_IDEND_LUASCRIPT) )
        { // here we have a script
            if (!App::userSettings->compatibilityFix1)
            {
                std::string tmp("targeting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in ");
                tmp+=App::folders->getUserSettingsPath()+"/usrset.txt";
                CApiErrors::setLastWarningOrError(__func__,tmp.c_str());
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

char* simReadCustomDataBlock_internal(int objectHandle,const char* tagName,int* dataSize)
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
        if ( (objectHandle>=SIM_IDSTART_SCENEOBJECT)&&(objectHandle<=SIM_IDEND_SCENEOBJECT) )
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
        if ( (objectHandle>=SIM_IDSTART_LUASCRIPT)&&(objectHandle<=SIM_IDEND_LUASCRIPT) )
        { // here we have a script
            if (!App::userSettings->compatibilityFix1)
            {
                std::string tmp("targeting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in ");
                tmp+=App::folders->getUserSettingsPath()+"/usrset.txt";
                CApiErrors::setLastWarningOrError(__func__,tmp.c_str());
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

char* simReadCustomDataBlockTags_internal(int objectHandle,int* tagCount)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retBuffer=nullptr;
        tagCount[0]=0;
        std::string tags;
        if ( (objectHandle>=SIM_IDSTART_SCENEOBJECT)&&(objectHandle<=SIM_IDEND_SCENEOBJECT) )
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
        if ( (objectHandle>=SIM_IDSTART_LUASCRIPT)&&(objectHandle<=SIM_IDEND_LUASCRIPT) )
        { // here we have a script
            std::vector<std::string> allTags;
            if (!App::userSettings->compatibilityFix1)
            {
                std::string tmp("targeting a script is not supported anymore. Please adjust your code. Temporarily (until next release), you can keep backward compatibility by adding 'compatibilityFix1=true' in ");
                tmp+=App::folders->getUserSettingsPath()+"/usrset.txt";
                CApiErrors::setLastWarningOrError(__func__,tmp.c_str());
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

int simGetShapeGeomInfo_internal(int shapeHandle,int* intData,double* floatData,void* reserved)
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

int simGetObjects_internal(int index,int objectType)
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

int* simGetObjectsInTree_internal(int treeBaseHandle,int objectType,int options,int* objectCount)
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

int simScaleObject_internal(int objectHandle,double xScale,double yScale,double zScale,int options)
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

int simGetShapeTextureId_internal(int shapeHandle)
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


int simSetShapeTexture_internal(int shapeHandle,int textureId,int mappingMode,int options,const double* uvScaling,const double* position,const double* orientation)
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
                        tr.X.setData(position);
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

int simCreateCollectionEx_internal(int options)
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

int simAddItemToCollection_internal(int collectionHandle,int what,int objectHandle,int options)
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

int simDestroyCollection_internal(int collectionHandle)
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

int* simGetCollectionObjects_internal(int collectionHandle,int* objectCount)
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

int simReorientShapeBoundingBox_internal(int shapeHandle,int relativeToHandle,int reservedSetToZero)
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

int simSaveImage_internal(const unsigned char* image,const int* resolution,int options,const char* filename,int quality,void* reserved)
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

unsigned char* simLoadImage_internal(int* resolution,int options,const char* filename,void* reserved)
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

unsigned char* simGetScaledImage_internal(const unsigned char* imageIn,const int* resolutionIn,int* resolutionOut,int options,void* reserved)
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

int simTransformImage_internal(unsigned char* image,const int* resolution,int options,const double* floatParams,const int* intParams,void* reserved)
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

int simGetQHull_internal(const double* inVertices,int inVerticesL,double** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,int reserved1,const double* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        std::vector<double> vOut;
        std::vector<int> iOut;
        bool res=CMeshRoutines::getConvexHull(inVertices,inVerticesL,&vOut,&iOut);
        if (res)
        {
            verticesOut[0]=new double[vOut.size()];
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

int simGetDecimatedMesh_internal(const double* inVertices,int inVerticesL,const int* inIndices,int inIndicesL,double** verticesOut,int* verticesOutL,int** indicesOut,int* indicesOutL,double decimationPercent,int reserved1,const double* reserved2)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        std::vector<double> vOut;
        std::vector<int> iOut;
        std::vector<double> vIn(inVertices,inVertices+inVerticesL);
        std::vector<int> iIn(inIndices,inIndices+inIndicesL);
        bool res=CMeshRoutines::getDecimatedMesh(vIn,iIn,decimationPercent,vOut,iOut);
        if (res)
        {
            verticesOut[0]=new double[vOut.size()];
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

int simCallScriptFunctionEx_internal(int scriptHandleOrType,const char* functionNameAtScriptName,int stackId)
{
    TRACE_C_API;
    CScriptObject* script=nullptr;
    std::string funcName;
    int handleFlags=scriptHandleOrType&0x0ff00000;
    scriptHandleOrType=scriptHandleOrType&0x000fffff;
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
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0);
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

    std::string tmp("External call to simCallScriptFunction failed ('");
    tmp+=functionNameAtScriptName;
    tmp+="'): ";
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
                if ((handleFlags&sim_handleflag_silenterror)==0)
                    App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)
            }
            if (retVal==0)
            {
                CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
                tmp+=SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION;
                if ((handleFlags&sim_handleflag_silenterror)==0)
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
            if ((handleFlags&sim_handleflag_silenterror)==0)
                App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)
        }
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
    tmp+=SIM_ERROR_SCRIPT_INEXISTANT;
    if ((handleFlags&sim_handleflag_silenterror)==0)
        App::logMsg(sim_verbosity_errors,tmp.c_str()); // log error here (special, for easier debugging)

    return(-1);
}

char* simGetExtensionString_internal(int objectHandle,int index,const char* key)
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

int simComputeMassAndInertia_internal(int shapeHandle,double density)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=(CShape*)App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
            C7Vector localTr;
            C3Vector diagI; // massless
            double mass=CPluginContainer::dyn_computeInertia(shape->getObjectHandle(),localTr,diagI);
            if (mass>0.0)
            {
                mass=density*mass/1000.0;
                shape->getMeshWrapper()->setPrincipalMomentsOfInertia(diagI);
                shape->getMeshWrapper()->setLocalInertiaFrame(localTr);
                shape->getMeshWrapper()->setMass(mass);
                App::undoRedo_sceneChanged("");
                return(1);
            }
            return(0);
        }
        return(-1);
    }

    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

int simCreateStack_internal()
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

int simReleaseStack_internal(int stackHandle)
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

int simCopyStack_internal(int stackHandle)
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

int simPushNullOntoStack_internal(int stackHandle)
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

int simPushBoolOntoStack_internal(int stackHandle,bool value)
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

int simPushInt32OntoStack_internal(int stackHandle,int value)
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

int simPushInt64OntoStack_internal(int stackHandle,long long int value)
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

int simPushFloatOntoStack_internal(int stackHandle,float value)
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

int simPushDoubleOntoStack_internal(int stackHandle,double value)
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

int simPushStringOntoStack_internal(int stackHandle,const char* value,int stringSize)
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

int simPushUInt8TableOntoStack_internal(int stackHandle,const unsigned char* values,int valueCnt)
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

int simPushInt32TableOntoStack_internal(int stackHandle,const int* values,int valueCnt)
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

int simPushInt64TableOntoStack_internal(int stackHandle,const long long int* values,int valueCnt)
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

int simPushFloatTableOntoStack_internal(int stackHandle,const float* values,int valueCnt)
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

int simPushDoubleTableOntoStack_internal(int stackHandle,const double* values,int valueCnt)
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

int simPushTableOntoStack_internal(int stackHandle)
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

int simInsertDataIntoStackTable_internal(int stackHandle)
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

int simGetStackSize_internal(int stackHandle)
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

int simPopStackItem_internal(int stackHandle,int count)
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

int simMoveStackItemToTop_internal(int stackHandle,int cIndex)
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

int simIsStackValueNull_internal(int stackHandle)
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

int simGetStackBoolValue_internal(int stackHandle,bool* boolValue)
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

int simGetStackInt32Value_internal(int stackHandle,int* numberValue)
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

int simGetStackInt64Value_internal(int stackHandle,long long int* numberValue)
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
                long long int v;
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

int simGetStackFloatValue_internal(int stackHandle,float* numberValue)
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

int simGetStackDoubleValue_internal(int stackHandle,double* numberValue)
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

char* simGetStackStringValue_internal(int stackHandle,int* stringSize)
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

int simGetStackTableInfo_internal(int stackHandle,int infoType)
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

int simGetStackUInt8Table_internal(int stackHandle,unsigned char* array,int count)
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

int simGetStackInt32Table_internal(int stackHandle,int* array,int count)
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

int simGetStackInt64Table_internal(int stackHandle,long long int* array,int count)
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

int simGetStackFloatTable_internal(int stackHandle,float* array,int count)
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

int simGetStackDoubleTable_internal(int stackHandle,double* array,int count)
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

int simUnfoldStackTable_internal(int stackHandle)
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

int simDebugStack_internal(int stackHandle,int cIndex)
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

double simGetEngineFloatParam_internal(int paramId,int objectHandle,const void* object,bool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    bool success=true;
    double retVal=0.0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    retVal=dummy->getEngineFloatParam(paramId,&success);
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

int simGetEngineInt32Param_internal(int paramId,int objectHandle,const void* object,bool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
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
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    retVal=dummy->getEngineIntParam(paramId,&success);
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

bool simGetEngineBoolParam_internal(int paramId,int objectHandle,const void* object,bool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
    // this function doesn't generate any error messages
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    bool success=true;
    bool retVal=0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
                if (it!=nullptr)
                {
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    retVal=dummy->getEngineBoolParam(paramId,&success);
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

int simSetEngineFloatParam_internal(int paramId,int objectHandle,const void* object,double val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
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
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineFloatParam(paramId,val);
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    success=dummy->setEngineFloatParam(paramId,val);
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

int simSetEngineInt32Param_internal(int paramId,int objectHandle,const void* object,int val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
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
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineIntParam(paramId,val);
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    success=dummy->setEngineIntParam(paramId,val);
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

int simSetEngineBoolParam_internal(int paramId,int objectHandle,const void* object,bool val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint, shape or dummy parameter
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
                    if ( (it->getObjectType()!=sim_object_joint_type)&&(it->getObjectType()!=sim_object_shape_type)&&(it->getObjectType()!=sim_object_dummy_type) )
                        success=false;
                }
                else
                    success=false;
            }
        }
        if (success)
        {
            if (it==nullptr)
                success=App::currentWorld->dynamicsContainer->setEngineBoolParam(paramId,val);
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
                if (it->getObjectType()==sim_object_dummy_type)
                {
                    CDummy* dummy=(CDummy*)it;
                    success=dummy->setEngineBoolParam(paramId,val);
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

int simCreateOctree_internal(double voxelSize,int options,double pointSize,void* reserved)
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

int simCreatePointCloud_internal(double maxVoxelSize,int maxPtCntPerVoxel,int options,double pointSize,void* reserved)
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

int simSetPointCloudOptions_internal(int pointCloudHandle,double maxVoxelSize,int maxPtCntPerVoxel,int options,double pointSize,void* reserved)
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

int simGetPointCloudOptions_internal(int pointCloudHandle,double* maxVoxelSize,int* maxPtCntPerVoxel,int* options,double* pointSize,void* reserved)
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
        pointSize[0]=(double)it->getPointSize();
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

int simInsertVoxelsIntoOctree_internal(int octreeHandle,int options,const double* pts,int ptCnt,const unsigned char* color,const unsigned int* tag,void* reserved)
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

int simRemoveVoxelsFromOctree_internal(int octreeHandle,int options,const double* pts,int ptCnt,void* reserved)
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

int simInsertPointsIntoPointCloud_internal(int pointCloudHandle,int options,const double* pts,int ptCnt,const unsigned char* color,void* optionalValues)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        double insertionToleranceSaved=it->getInsertionDistanceTolerance();
        int optionalValuesBits=0;
        if (optionalValues!=nullptr)
            optionalValuesBits=((int*)optionalValues)[0];
        if (optionalValuesBits&1)
            it->setInsertionDistanceTolerance((double)((float*)optionalValues)[1]);
        it->insertPoints(pts,ptCnt,options&1,color,options&2);
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

int simRemovePointsFromPointCloud_internal(int pointCloudHandle,int options,const double* pts,int ptCnt,double tolerance,void* reserved)
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

int simIntersectPointsWithPointCloud_internal(int pointCloudHandle,int options,const double* pts,int ptCnt,double tolerance,void* reserved)
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

const double* simGetOctreeVoxels_internal(int octreeHandle,int* ptCnt,void* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(nullptr);
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(octreeHandle);
        const std::vector<double>* p=it->getCubePositions();
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

const double* simGetPointCloudPoints_internal(int pointCloudHandle,int* ptCnt,void* reserved)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(nullptr);
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(pointCloudHandle);
        const std::vector<double>* p=it->getPoints();
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

int simInsertObjectIntoOctree_internal(int octreeHandle,int objectHandle,int options,const unsigned char* color,unsigned int tag,void* reserved)
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
            cptr[0]=float(color[0])/255.1;
            cptr[1]=float(color[1])/255.1;
            cptr[2]=float(color[2])/255.1;
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

int simSubtractObjectFromOctree_internal(int octreeHandle,int objectHandle,int options,void* reserved)
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

int simInsertObjectIntoPointCloud_internal(int pointCloudHandle,int objectHandle,int options,double gridSize,const unsigned char* color,void* optionalValues)
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
        double savedGridSize=it->getBuildResolution();
        it->setBuildResolution(gridSize);
        float savedCols[3];
        it->getColor()->getColor(savedCols,sim_colorcomponent_ambient_diffuse);
        if (color!=nullptr)
        {
            it->getColor()->getColorsPtr()[0]=float(color[0])/255.1;
            it->getColor()->getColorsPtr()[1]=float(color[1])/255.1;
            it->getColor()->getColorsPtr()[2]=float(color[2])/255.1;
        }
        double insertionToleranceSaved=it->getInsertionDistanceTolerance();
        int optionalValuesBits=0;
        if (optionalValues!=nullptr)
            optionalValuesBits=((int*)optionalValues)[0];
        if (optionalValuesBits&1)
            it->setInsertionDistanceTolerance((double)((float*)optionalValues)[1]);
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

int simSubtractObjectFromPointCloud_internal(int pointCloudHandle,int objectHandle,int options,double tolerance,void* reserved)
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

int simCheckOctreePointOccupancy_internal(int octreeHandle,int options,const double* points,int ptCnt,unsigned int* tag,unsigned long long int* location,void* reserved)
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
        const double* _pts=points;
        std::vector<double> __pts;
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

char* simOpenTextEditor_internal(const char* initText,const char* xml,int* various)
{
    TRACE_C_API;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    char* retVal=nullptr;
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
    {
        std::string txt=App::mainWindow->codeEditorContainer->openModalTextEditor(initText,xml,various,true);
        retVal=new char[txt.size()+1];
        for (size_t i=0;i<txt.size();i++)
            retVal[i]=txt[i];
        retVal[txt.size()]=0;
    }
#endif
    return(retVal);
}

char* simPackTable_internal(int stackHandle,int* bufferSize)
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

int simUnpackTable_internal(int stackHandle,const char* buffer,int bufferSize)
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

int simSetReferencedHandles_internal(int objectHandle,int count,const int* referencedHandles,const int* reserved1,const int* reserved2)
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

int simGetReferencedHandles_internal(int objectHandle,int** referencedHandles,int** reserved1,int** reserved2)
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

int simGetShapeViz_internal(int shapeHandle,int index,struct SShapeVizInfo* info)
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
                info->transparency=0.0;
                if (geom->color.getTranslucid())
                    info->transparency=(double)geom->color.getOpacity();
                info->options=0;
                if (geom->getCulling())
                    info->options|=1;
                if (geom->getWireframe_OLD())
                    info->options|=2;
            }

            C7Vector tr(geom->getVerticeLocalFrame());
            const std::vector<double>* wvert=geom->getVertices();
            const std::vector<int>* wind=geom->getIndices();
            const std::vector<double>* wnorm=geom->getNormals();
            info->verticesSize=int(wvert->size());
            info->vertices=new double[wvert->size()];
            for (size_t i=0;i<wvert->size()/3;i++)
            {
                C3Vector v;
                v.setData((&wvert[0][0])+i*3);
                v=tr*v;
                info->vertices[3*i+0]=v(0);
                info->vertices[3*i+1]=v(1);
                info->vertices[3*i+2]=v(2);
            }
            info->indicesSize=int(wind->size());
            info->indices=new int[wind->size()];
            info->normals=new double[wind->size()*3];
            for (size_t i=0;i<wind->size();i++)
            {
                info->indices[i]=wind->at(i);
                C3Vector n;
                n.setData(&(wnorm[0])[0]+i*3);
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
                tc=tp->getTextureCoordinates(-1,tr,geom->getVerticesForDisplayAndDisk()[0],wind[0]);
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
                info->textureCoords=new double[tc->size()];
                for (size_t i=0;i<tc->size();i++)
                    info->textureCoords[i]=(double)tc->at(i);
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

int simGetShapeVizf_internal(int shapeHandle,int index,struct SShapeVizInfof* info)
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
                info->transparency=0.0;
                if (geom->color.getTranslucid())
                    info->transparency=geom->color.getOpacity();
                info->options=0;
                if (geom->getCulling())
                    info->options|=1;
                if (geom->getWireframe_OLD())
                    info->options|=2;
            }

            C7Vector tr(geom->getVerticeLocalFrame());
            const std::vector<float>* wvert=geom->getVerticesForDisplayAndDisk();
            const std::vector<int>* wind=geom->getIndices();
            const std::vector<float>* wnorm=geom->getNormalsForDisplayAndDisk();
            info->verticesSize=int(wvert->size());
            info->vertices=new float[wvert->size()];
            for (size_t i=0;i<wvert->size()/3;i++)
            {
                C3Vector v;
                v.setData((&wvert[0][0])+i*3);
                v=tr*v;
                info->vertices[3*i+0]=(float)v(0);
                info->vertices[3*i+1]=(float)v(1);
                info->vertices[3*i+2]=(float)v(2);
            }
            info->indicesSize=int(wind->size());
            info->indices=new int[wind->size()];
            info->normals=new float[wind->size()*3];
            for (size_t i=0;i<wind->size();i++)
            {
                info->indices[i]=wind->at(i);
                C3Vector n;
                n.setData(&(wnorm[0])[0]+i*3);
                n=tr.Q*n; // only orientation
                info->normals[3*i+0]=(float)n(0);
                info->normals[3*i+1]=(float)n(1);
                info->normals[3*i+2]=(float)n(2);
            }
            geom->color.getColor(info->colors+0,sim_colorcomponent_ambient_diffuse);
            geom->color.getColor(info->colors+3,sim_colorcomponent_specular);
            geom->color.getColor(info->colors+6,sim_colorcomponent_emission);
            info->shadingAngle=(float)geom->getShadingAngle();

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

int simExecuteScriptString_internal(int scriptHandleOrType,const char* stringAtScriptName,int stackHandle)
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
        { // script is identified by its type
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
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,scriptName.c_str(),0);
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

char* simGetApiFunc_internal(int scriptHandleOrType,const char* apiWord)
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

char* simGetApiInfo_internal(int scriptHandleOrType,const char* apiWord)
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

int simSetModuleInfo_internal(const char* moduleName,int infoType,const char* stringInfo,int intInfo)
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

int simGetModuleInfo_internal(const char* moduleName,int infoType,char** stringInfo,int* intInfo)
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

int simIsDeprecated_internal(const char* funcOrConst)
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
char* simGetPersistentDataTags_internal(int* tagCount)
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

int simEventNotification_internal(const char* event)
{
    //printf("event: %s\n",event);
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

int simApplyTexture_internal(int shapeHandle,const double* textureCoordinates,int textCoordSize,const unsigned char* texture,const int* textureResolution,int options)
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
                    c.resize(textCoordSize);
                    for (int i=0;i<textCoordSize;i++)
                        c[i]=(float)textureCoordinates[i];
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

int simSetJointDependency_internal(int jointHandle,int masterJointHandle,double offset,double multCoeff)
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

int simGetJointDependency_internal(int jointHandle,int* masterJointHandle,double* offset,double* multCoeff)
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

int simSetNamedStringParam_internal(const char* paramName,const char* stringParam,int paramLength)
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

char* simGetNamedStringParam_internal(const char* paramName,int* paramLength)
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


const void* _simGetGeomWrapFromGeomProxy_internal(const void* geomData)
{
    TRACE_C_API;
    return(((CShape*)geomData)->getMeshWrapper());
}

double _simGetMass_internal(const void* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->getMass());
}

double _simGetLocalInertiaInfo_internal(const void* object,double* pos,double* quat,double* diagI)
{ // returns the diag inertia (with mass!). diagI can be NULL
    CShape* shape=(CShape*)object;
    CMeshWrapper* geomInfo=shape->getMeshWrapper();
    C7Vector tr(geomInfo->getLocalInertiaFrame());
    double m=geomInfo->getMass();
    if (diagI!=nullptr)
    {
        C3Vector diag(geomInfo->getPrincipalMomentsOfInertia());
        if (App::currentWorld->dynamicsContainer->getComputeInertias())
            CPluginContainer::dyn_computeInertia(shape->getObjectHandle(),tr,diag);
        tr.X.getData(pos);
        tr.Q.getData(quat);
        diag=diag*m;
        diag.getData(diagI);
    }
    else
    {
        tr.X.getData(pos);
        tr.Q.getData(quat);
    }
    return(m);
}

int _simGetPurePrimitiveType_internal(const void* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->getPurePrimitiveType());
}

void _simGetPurePrimitiveSizes_internal(const void* geometric,double* sizes)
{
    TRACE_C_API;
    C3Vector s;
    ((CMesh*)geometric)->getPurePrimitiveSizes(s);
    s.getData(sizes);
}

bool _simIsGeomWrapGeometric_internal(const void* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->isMesh());
}

bool _simIsGeomWrapConvex_internal(const void* geomInfo)
{
    TRACE_C_API;
    return(((CMeshWrapper*)geomInfo)->isConvex());
}

int _simGetGeometricCount_internal(const void* geomInfo)
{
    TRACE_C_API;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllShapeComponentsCumulative(all);
    return((int)all.size());
}

void _simGetAllGeometrics_internal(const void* geomInfo,void** allGeometrics)
{
    TRACE_C_API;
    std::vector<CMesh*> all;
    ((CMeshWrapper*)geomInfo)->getAllShapeComponentsCumulative(all);
    for (size_t i=0;i<all.size();i++)
        allGeometrics[i]=all[i];
}

void _simMakeDynamicAnnouncement_internal(int announceType)
{
    TRACE_C_API;
    if (announceType==sim_announce_pureconenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureConeNotSupported();
    if (announceType==sim_announce_purespheroidnotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureSpheroidNotSupported();
    if (announceType==sim_announce_containsnonpurenonconvexshapes)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsNonPureNonConvexShapes();
    if (announceType==sim_announce_containsstaticshapesondynamicconstruction)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_containsStaticShapesOnDynamicConstruction();
    if (announceType==sim_announce_purehollowshapenotsupported)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_pureHollowShapeNotSupported();
    if (announceType==sim_announce_vortexpluginisdemo)
        App::currentWorld->dynamicsContainer->markForWarningDisplay_vortexPluginIsDemo();
}

void _simGetVerticesLocalFrame_internal(const void* geometric,double* pos,double* quat)
{
    TRACE_C_API;
    C7Vector tr(((CMesh*)geometric)->getVerticeLocalFrame());
    tr.Q.getData(quat);
    tr.X.getData(pos);
}

const double* _simGetHeightfieldData_internal(const void* geometric,int* xCount,int* yCount,double* minHeight,double* maxHeight)
{
    TRACE_C_API;
    return(((CMesh*)geometric)->getHeightfieldData(xCount[0],yCount[0],minHeight[0],maxHeight[0]));
}

void _simGetCumulativeMeshes_internal(const void* geomInfo,double** vertices,int* verticesSize,int** indices,int* indicesSize)
{
    TRACE_C_API;
    std::vector<double> vert;
    std::vector<int> ind;
    ((CMeshWrapper*)geomInfo)->getCumulativeMeshes(vert,&ind,nullptr);

    vertices[0]=new double[vert.size()];
    verticesSize[0]=(int)vert.size();
    for (size_t i=0;i<vert.size();i++)
        vertices[0][i]=vert[i];
    indices[0]=new int[ind.size()];
    indicesSize[0]=(int)ind.size();
    for (size_t i=0;i<ind.size();i++)
        indices[0][i]=ind[i];
}

int _simGetObjectID_internal(const void* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getObjectHandle());
}

void _simGetObjectLocalTransformation_internal(const void* object,double* pos,double* quat,bool excludeFirstJointTransformation)
{
    TRACE_C_API;
    C7Vector tr;
    if (excludeFirstJointTransformation)
        tr=((CSceneObject*)object)->getLocalTransformation();
    else
        tr=((CSceneObject*)object)->getFullLocalTransformation();
    tr.X.getData(pos);
    tr.Q.getData(quat);
}

void _simSetObjectLocalTransformation_internal(void* object,const double* pos,const double* quat,double simTime)
{
    TRACE_C_API;
    C7Vector tr;
    tr.X.setData(pos);
    tr.Q.setData(quat);
    ((CSceneObject*)object)->setLocalTransformation(tr);
}

void _simGetObjectCumulativeTransformation_internal(const void* object,double* pos,double* quat,bool excludeFirstJointTransformation)
{
    TRACE_C_API;
    C7Vector tr;
    if (excludeFirstJointTransformation!=0)
        tr=((CSceneObject*)object)->getCumulativeTransformation();
    else
        tr=((CSceneObject*)object)->getFullCumulativeTransformation();
    if (pos!=nullptr)
        tr.X.getData(pos);
    if (quat!=nullptr)
        tr.Q.getData(quat);
}

const void* _simGetGeomProxyFromShape_internal(const void* shape)
{
    TRACE_C_API;
    return(shape);
}

bool _simIsShapeDynamicallyStatic_internal(const void* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getShapeIsDynamicallyStatic());
}

void _simGetInitialDynamicVelocity_internal(const void* shape,double* vel)
{
    TRACE_C_API;
    ((CShape*)shape)->getInitialDynamicLinearVelocity().getData(vel);
}

void _simSetInitialDynamicVelocity_internal(void* shape,const double* vel)
{
    TRACE_C_API;
    ((CShape*)shape)->setInitialDynamicLinearVelocity(C3Vector(vel));
}

void _simGetInitialDynamicAngVelocity_internal(const void* shape,double* angularVel)
{
    TRACE_C_API;
    ((CShape*)shape)->getInitialDynamicAngularVelocity().getData(angularVel);
}

void _simSetInitialDynamicAngVelocity_internal(void* shape,const double* angularVel)
{
    TRACE_C_API;
    ((CShape*)shape)->setInitialDynamicAngularVelocity(C3Vector(angularVel));
}

bool _simGetStartSleeping_internal(const void* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getStartInDynamicSleeping());
}

bool _simGetWasPutToSleepOnce_internal(const void* shape)
{ // flag is set to true whenever called!!!
    TRACE_C_API;
    bool a=((CShape*)shape)->getRigidBodyWasAlreadyPutToSleepOnce();
    ((CShape*)shape)->setRigidBodyWasAlreadyPutToSleepOnce(true);
    return(a);
}

bool _simIsShapeDynamicallyRespondable_internal(const void* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getRespondable());
}

int _simGetDynamicCollisionMask_internal(const void* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getDynamicCollisionMask());
}

const void* _simGetLastParentForLocalGlobalCollidable_internal(const void* shape)
{
    TRACE_C_API;
    return(((CShape*)shape)->getLastParentForLocalGlobalRespondable());
}

bool _simGetDynamicsFullRefreshFlag_internal(const void* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getDynamicsResetFlag());
}

void _simSetDynamicsFullRefreshFlag_internal(const void* object,bool flag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicsResetFlag(flag!=0,false);
}

const void* _simGetParentObject_internal(const void* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getParent());
}

void _simDynReportObjectCumulativeTransformation_internal(void* obj,const double* pos,const double* quat,double simTime)
{ // obj is always a shape. Used by the physics engines. The joints and force sensors's internal errors are updated accordingly
    TRACE_C_API;
    CSceneObject* object=(CSceneObject*)obj;
    CSceneObject* parent=object->getParent();
    C7Vector tr;
    tr.X.setData(pos);
    tr.Q.setData(quat);
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

void _simSetObjectCumulativeTransformation_internal(void* object,const double* pos,const double* quat,bool keepChildrenInPlace)
{
    TRACE_C_API;
    C7Vector tr;
    tr.X.setData(pos);
    tr.Q.setData(quat);
    App::currentWorld->sceneObjects->setObjectAbsolutePose(((CSceneObject*)object)->getObjectHandle(),tr,keepChildrenInPlace!=0);
}

void _simSetShapeDynamicVelocity_internal(void* shape,const double* linear,const double* angular,double simTime)
{
    TRACE_C_API;
    ((CShape*)shape)->setDynamicVelocity(linear,angular);
}

void _simGetAdditionalForceAndTorque_internal(const void* shape,double* force,double* torque)
{
    TRACE_C_API;
    ((CShape*)shape)->getAdditionalForce().getData(force);
    ((CShape*)shape)->getAdditionalTorque().getData(torque);
}

void _simClearAdditionalForceAndTorque_internal(const void* shape)
{
    TRACE_C_API;
    ((CShape*)shape)->clearAdditionalForceAndTorque();
}

bool _simGetJointPositionInterval_internal(const void* joint,double* minValue,double* rangeValue)
{
    TRACE_C_API;
    if (minValue!=nullptr)
        minValue[0]=((CJoint*)joint)->getPositionMin();
    if (rangeValue!=nullptr)
        rangeValue[0]=((CJoint*)joint)->getPositionRange();
    return(!((CJoint*)joint)->getIsCyclic());
}

const void* _simGetObject_internal(int objID)
{
    TRACE_C_API;
    return(App::currentWorld->sceneObjects->getObjectFromHandle(objID));
}

const void* _simGetIkGroupObject_internal(int ikGroupID)
{
    TRACE_C_API;
    return(App::currentWorld->ikGroups->getObjectFromHandle(ikGroupID));
}

int _simMpHandleIkGroupObject_internal(const void* ikGroup)
{
    TRACE_C_API;
    return(((CIkGroup_old*)ikGroup)->computeGroupIk(true));
}

int _simGetJointType_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getJointType());
}

double _simGetDynamicMotorTargetPosition_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetPosition());
}

double _simGetDynamicMotorTargetVelocity_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetVelocity());
}

double _simGetDynamicMotorMaxForce_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getTargetForce(false));
}

double _simGetDynamicMotorUpperLimitVelocity_internal(const void* joint)
{
    TRACE_C_API;
    double maxVelAccelJerk[3];
    ((CJoint*)joint)->getMaxVelAccelJerk(maxVelAccelJerk);
    return(maxVelAccelJerk[0]);
}

void _simSetJointSphericalTransformation_internal(void* joint,const double* quat,double simTime)
{
    TRACE_C_API;
    ((CJoint*)joint)->setSphericalTransformation(quat);
}

void _simAddForceSensorCumulativeForcesAndTorques_internal(void* forceSensor,const double* force,const double* torque,int totalPassesCount,double simTime)
{
    TRACE_C_API;
    ((CForceSensor*)forceSensor)->addCumulativeForcesAndTorques(force,torque,totalPassesCount);
}

void _simAddJointCumulativeForcesOrTorques_internal(void* joint,double forceOrTorque,int totalPassesCount,double simTime)
{
    TRACE_C_API;
    ((CJoint*)joint)->addCumulativeForceOrTorque(forceOrTorque,totalPassesCount);
}

int _simGetObjectListSize_internal(int objType)
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

const void* _simGetObjectFromIndex_internal(int objType,int index)
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

void _simSetDynamicSimulationIconCode_internal(void* object,int code)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicSimulationIconCode(code);
}

void _simSetDynamicObjectFlagForVisualization_internal(void* object,int flag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->setDynamicFlag(flag);
}

int _simGetTreeDynamicProperty_internal(const void* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getTreeDynamicProperty());
}

int _simGetObjectType_internal(const void* object)
{
    TRACE_C_API;
    return(((CSceneObject*)object)->getObjectType());
}

const void** _simGetObjectChildren_internal(const void* object,int* count)
{
    TRACE_C_API;
    CSceneObject* it=(CSceneObject*)object;
    count[0]=int(it->getChildCount());
    if (count[0]!=0)
        return((const void**)&it->getChildren()->at(0));
    return(nullptr);
}

int _simGetDummyLinkType_internal(const void* dummy,int* linkedDummyID)
{
    TRACE_C_API;
    int dType=((CDummy*)dummy)->getLinkType();
    if (linkedDummyID!=nullptr)
        linkedDummyID[0]=((CDummy*)dummy)->getLinkedDummyHandle();
    return(dType);
}

int _simGetJointMode_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getJointMode());
}

bool _simIsJointInHybridOperation_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getHybridFunctionality_old());
}

void _simDisableDynamicTreeForManipulation_internal(const void* object,bool disableFlag)
{
    TRACE_C_API;
    ((CSceneObject*)object)->temporarilyDisableDynamicTree();
}

void _simSetJointVelocity_internal(const void* joint,double vel)
{ // only used by MuJoCo. Other engines have the joint velocity computed via _simSetDynamicMotorReflectedPositionFromDynamicEngine
    TRACE_C_API;
    ((CJoint*)joint)->setVelocity(vel);
}

void _simSetJointPosition_internal(const void* joint,double pos)
{ // only used by MuJoCo. Other engines have the joint position set via _simSetDynamicMotorReflectedPositionFromDynamicEngine
    TRACE_C_API;
    ((CJoint*)joint)->setPosition(pos);
}

void _simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(void* joint,double pos,double simTime)
{ // only from non-MuJoCo engines. MuJoCo uses above 2 functions instead
    TRACE_C_API;
    ((CJoint*)joint)->setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(pos,simTime);
}

double _simGetJointPosition_internal(const void* joint)
{
    TRACE_C_API;
    return(((CJoint*)joint)->getPosition());
}

void _simSetDynamicMotorPositionControlTargetPosition_internal(const void* joint,double pos)
{ // OLD, for backward compatibility. Only joints in hybrid operation are called here
    if (_simGetJointMode_internal(joint)!=sim_jointmode_dynamic)
        ((CJoint*)joint)->setTargetPosition(pos);
}

void _simGetGravity_internal(double* gravity)
{
    TRACE_C_API;
    App::currentWorld->dynamicsContainer->getGravity().getData(gravity);
}

int _simGetTimeDiffInMs_internal(int previousTime)
{
    TRACE_C_API;
    return(VDateTime::getTimeDiffInMs(previousTime));
}

bool _simDoEntitiesCollide_internal(int entity1ID,int entity2ID,int* cacheBuffer,bool overrideCollidableFlagIfShape1,bool overrideCollidableFlagIfShape2,bool pathOrMotionPlanningRoutineCalling)
{
    TRACE_C_API;
    return(CCollisionRoutine::doEntitiesCollide(entity1ID,entity2ID,nullptr,overrideCollidableFlagIfShape1!=0,overrideCollidableFlagIfShape2!=0,nullptr));
}

bool _simGetDistanceBetweenEntitiesIfSmaller_internal(int entity1ID,int entity2ID,double* distance,double* ray,int* cacheBuffer,bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2,bool pathPlanningRoutineCalling)
{
    TRACE_C_API;
    return(CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1ID,entity2ID,distance[0],ray,cacheBuffer,cacheBuffer+2,overrideMeasurableFlagIfNonCollection1!=0,overrideMeasurableFlagIfNonCollection2!=0));
}

int _simHandleJointControl_internal(const void* joint,int auxV,const int* inputValuesInt,const double* inputValuesFloat,double* outputValues)
{
    TRACE_C_API;
    double currentPosVelAccel[3]={inputValuesFloat[0],inputValuesFloat[4],inputValuesFloat[5]};
    return(((CJoint*)joint)->handleDynJoint(auxV,inputValuesInt,currentPosVelAccel,inputValuesFloat[1],inputValuesFloat[2],inputValuesFloat[3],outputValues));
}

int _simGetJointDynCtrlMode_internal(const void* joint)
{
    TRACE_C_API;
    int retVal=((CJoint*)joint)->getDynCtrlMode();
    return(retVal);
}

int _simHandleCustomContact_internal(int objHandle1,int objHandle2,int engine,int* dataInt,double* dataFloat)
{ // Careful with this function: it can also be called from any other thread (e.g. generated by the physics engine)
    TRACE_C_API;

    // 1. We handle the new calling method:
    if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_contact)>0)
    {
        if ((engine&1024)==0) // the engine flag 1024 means: the calling thread is not the simulation thread. We would have problems with the scripts
        {
            CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->insertKeyInt32IntoStackTable("handle1",objHandle1);
            inStack->insertKeyInt32IntoStackTable("handle2",objHandle2);
            inStack->insertKeyInt32IntoStackTable("engine",engine);
            CInterfaceStack* outStack=App::worldContainer->interfaceStackContainer->createStack();
            App::worldContainer->callScripts(sim_syscb_contact,inStack,outStack);
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
                            outStack->getStackMapDoubleValue("bullet.friction",dataFloat[0]);
                            outStack->getStackMapDoubleValue("bullet.restitution",dataFloat[1]);
                        }
                        if (engine==sim_physics_ode)
                        {
                            outStack->getStackMapDoubleValue("ode.mu",dataFloat[0]);
                            outStack->getStackMapDoubleValue("ode.mu2",dataFloat[1]);
                            outStack->getStackMapDoubleValue("ode.bounce",dataFloat[2]);
                            outStack->getStackMapDoubleValue("ode.bounceVel",dataFloat[3]);
                            outStack->getStackMapDoubleValue("ode.softCfm",dataFloat[4]);
                            outStack->getStackMapDoubleValue("ode.softErp",dataFloat[5]);
                            outStack->getStackMapDoubleValue("ode.motion1",dataFloat[6]);
                            outStack->getStackMapDoubleValue("ode.motion2",dataFloat[7]);
                            outStack->getStackMapDoubleValue("ode.motionN",dataFloat[8]);
                            outStack->getStackMapDoubleValue("ode.slip1",dataFloat[9]);
                            outStack->getStackMapDoubleValue("ode.slip2",dataFloat[10]);
                            outStack->getStackMapDoubleArray("ode.fDir1",dataFloat+11,3);
                        }
                        if (engine==sim_physics_vortex)
                        {
                            //outStack->getStackMapDoubleValue("vortex.xxxx",dataFloat[0]);
                        }
                        if (engine==sim_physics_newton)
                        {
                            outStack->getStackMapDoubleValue("newton.staticFriction",dataFloat[0]);
                            outStack->getStackMapDoubleValue("newton.kineticFriction",dataFloat[1]);
                            outStack->getStackMapDoubleValue("newton.restitution",dataFloat[2]);
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
    }
    return(-1); // we let CoppeliaSim handle the contact
}

double _simGetPureHollowScaling_internal(const void* geometric)
{
    TRACE_C_API;
    return(((CMesh*)geometric)->getPurePrimitiveInsideScaling_OLD());
}

void _simDynCallback_internal(const int* intData,const double* floatData)
{
    TRACE_C_API;

    if (App::worldContainer->getSysFuncAndHookCnt(sim_syscb_dyn)>0)
    { // to make it a bit faster than blindly parsing the whole object hierarchy
        CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
        inStack->pushTableOntoStack();

        inStack->insertKeyInt32IntoStackTable("passCnt",intData[1]);
        inStack->insertKeyInt32IntoStackTable("totalPasses",intData[2]);
        inStack->insertKeyFloatIntoStackTable("dynStepSize",floatData[0]); // deprecated
        inStack->insertKeyFloatIntoStackTable("dt",floatData[0]);
        inStack->insertKeyBoolIntoStackTable("afterStep",intData[3]!=0);
        App::worldContainer->callScripts(sim_syscb_dyn,inStack,nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(inStack);
    }
}

int simGetVisionSensorRes_internal(int sensorHandle,int* resolution)
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(sensorHandle);
        it->getResolution(resolution);
        return(1);
    }
    CApiErrors::setLastWarningOrError(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

#include <simInternal-old.cpp>
