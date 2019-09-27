#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "easyLock.h"
#include "v_rep_internal.h"
#include "simulation.h"
#include "collisionRoutine.h"
#include "distanceRoutine.h"
#include "proxSensorRoutine.h"
#include "meshRoutines.h"
#include "tt.h"
#include "fileOperations.h"
#include "persistentDataContainer.h"
#include "graphingRoutines.h"
#include "sceneObjectOperations.h"
#include "threadPool.h"
#include "addOperations.h"
#include "app.h"
#include "pluginContainer.h"
#include "geometric.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "vVarious.h"
#include "volInt.h"
#include "xmlSer.h"
#include "imgLoaderSaver.h"
#include "apiErrors.h"
#include "sigHandler.h"
#include "luaScriptFunctions.h"
#include <algorithm>
#include <iostream>
#include "v_rep_internalBase.h"
#ifdef SIM_WITH_GUI
    #include <QSplashScreen>
#endif
#ifdef WIN_VREP
    #include <dbghelp.h>
#else
    #include <execinfo.h>
    #include <signal.h>
#endif

#ifdef SIM_WITHOUT_QT_AT_ALL
    #ifdef WIN_VREP
        #include "_dirent.h"
    #else
        #include <dirent.h>
    #endif
#else
    VMutex _lockForExtLockList;
    std::vector<CSimAndUiThreadSync*> _extLockList;
#endif // SIM_WITHOUT_QT_AT_ALL

bool cNameSuffixAdjustmentTemporarilyDisabled=false;
int cNameSuffixNumber=-1;
bool outputSceneOrModelLoadMessagesWithApiCall=false;
bool fullModelCopyFromApi=true;
bool waitingForTrigger=false;
bool doNotRunMainScriptFromRosInterface=false;

std::vector<contactCallback> allContactCallbacks;
std::vector<jointCtrlCallback> allJointCtrlCallbacks;


std::vector<int> pluginHandles;
std::string initialSceneOrModelToLoad;
std::string applicationDir;
bool stepSimIfRunning=true;
bool firstSimulationAutoStart=false;
int firstSimulationStopDelay=0;
bool firstSimulationAutoQuit=false;

int loadPlugin(const char* theName,const char* theDirAndName)
{
    std::cout << "Plugin '" << theName << "': loading...\n";
    int pluginHandle=simLoadModule_internal(theDirAndName,theName);
    if (pluginHandle==-3)
    #ifdef WIN_VREP
        std::cout << "Error with plugin '" << theName << "': load failed (could not load). The plugin probably couldn't load dependency libraries. Try rebuilding the plugin.\n";
    #endif
    #ifdef MAC_VREP
        std::cout << "Error with plugin '" << theName << "': load failed (could not load). The plugin probably couldn't load dependency libraries. Try 'otool -L pluginName.dylib' for more infos, or simply rebuild the plugin.\n";
    #endif
    #ifdef LIN_VREP
        std::cout << "Error with plugin '" << theName << "': load failed (could not load). The plugin probably couldn't load dependency libraries. For additional infos, modify the script 'libLoadErrorCheck.sh', run it and inspect the output.\n";
    #endif

     if (pluginHandle==-2)
        std::cout << "Error with plugin '" << theName << "': load failed (missing entry points).\n";
    if (pluginHandle==-1)
        std::cout << "Error with plugin '" << theName << "': load failed (failed initialization).\n";
    if (pluginHandle>=0)
        std::cout << "Plugin '" << theName << "': load succeeded.\n";
    return(pluginHandle);
}

void simulatorInit()
{
    std::cout << "Simulator launched.\n";
    std::vector<std::string> theNames;
    std::vector<std::string> theDirAndNames;
#ifdef SIM_WITHOUT_QT_AT_ALL
    char curDirAndFile[2048];
    #ifdef WIN_VREP
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
                #ifdef WIN_VREP
                    pre=8;
                    po=4;
                    if ( (nm.compare(0,8,"v_repext")==0)&&(nm.compare(nm.size()-4,4,".dll")==0) )
                #endif
                #ifdef LIN_VREP
                    pre=11;
                    po=3;
                    if ( (nm.compare(0,11,"libv_repext")==0)&&(nm.compare(nm.size()-3,3,".so")==0) )
                #endif
                #ifdef MAC_VREP
                    pre=11;
                    po=6;
                    if ( (nm.compare(0,11,"libv_repext")==0)&&(nm.compare(nm.size()-6,6,".dylib")==0) )
                #endif
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
#else // SIM_WITHOUT_QT_AT_ALL

    QDir dir(applicationDir.c_str());
    dir.setFilter(QDir::Files|QDir::Hidden);
    dir.setSorting(QDir::Name);
    QStringList filters;
    int bnl=8;
    #ifdef WIN_VREP
        std::string tmp("v_repExt*.dll");
    #endif
    #ifdef MAC_VREP
        std::string tmp("libv_repExt*.dylib");
        bnl=11;
    #endif
    #ifdef LIN_VREP
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
#endif // SIM_WITHOUT_QT_AT_ALL

     // Load the system plugins first:
    for (size_t i=0;i<theNames.size();i++)
    {
        if ( (theNames[i].compare("MeshCalc")==0)||(theNames[i].compare("Dynamics")==0) )
        {
            int pluginHandle=loadPlugin(theNames[i].c_str(),theDirAndNames[i].c_str());
            if (pluginHandle>=0)
                pluginHandles.push_back(pluginHandle);
            theDirAndNames[i]=""; // mark as 'already loaded'
        }
    }
    simLoadModule_internal("",""); // indicate that we are done with the system plugins

     // Now load the other plugins too:
    for (size_t i=0;i<theNames.size();i++)
    {
        if (theDirAndNames[i].compare("")!=0)
        { // not yet loaded
            int pluginHandle=loadPlugin(theNames[i].c_str(),theDirAndNames[i].c_str());
            if (pluginHandle>=0)
                pluginHandles.push_back(pluginHandle);
        }
    }

    if (initialSceneOrModelToLoad.length()!=0)
    { // Here we double-clicked a V-REP file or dragged-and-dropped it onto this application
        int l=int(initialSceneOrModelToLoad.length());
        if ( (l>4)&&(initialSceneOrModelToLoad[l-4]=='.')&&(initialSceneOrModelToLoad[l-3]=='t')&&(initialSceneOrModelToLoad[l-2]=='t') )
        {
            simSetBoolParameter_internal(sim_boolparam_scene_and_model_load_messages,1);
            if (initialSceneOrModelToLoad[l-1]=='t') // trying to load a scene?
            {
                if (simLoadScene_internal(initialSceneOrModelToLoad.c_str())==-1)
                    simAddStatusbarMessage_internal("Scene could not be opened.");
            }
            if (initialSceneOrModelToLoad[l-1]=='m') // trying to load a model?
            {
                if (simLoadModel_internal(initialSceneOrModelToLoad.c_str())==-1)
                    simAddStatusbarMessage_internal("Model could not be loaded.");
            }
            simSetBoolParameter_internal(sim_boolparam_scene_and_model_load_messages,0);
        }
    }
}

void simulatorDeinit()
{
    // Unload all plugins:
    for (int i=0;i<int(pluginHandles.size());i++)
        simUnloadModule_internal(pluginHandles[i]);
    pluginHandles.clear();
    std::cout << "Simulator ended.\n";
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


std::vector<contactCallback>& getAllContactCallbacks()
{
    return(allContactCallbacks);
}

std::vector<jointCtrlCallback>& getAllJointCtrlCallbacks()
{
    return(allJointCtrlCallbacks);
}

void quicklyDisableAndAutomaticallyReenableCNameSuffixAdjustment()
{
    cNameSuffixAdjustmentTemporarilyDisabled=true;
}

void enableCNameSuffixAdjustment()
{
    cNameSuffixAdjustmentTemporarilyDisabled=false;
}

std::string getCNameSuffixAdjustedName(const char* name)
{
    if (strlen(name)==0)
        return("");
    std::string retVal(name);
    if ((!cNameSuffixAdjustmentTemporarilyDisabled)&&(retVal.find('#')==std::string::npos))
    { // Lua script calls never enter here
        if (cNameSuffixNumber>-1)
        {
            retVal=tt::getNameWithoutSuffixNumber(name,true);
            retVal=tt::generateNewName_dash(retVal,cNameSuffixNumber+1);
        }
    }

    if (retVal.length()!=0)
    {
        if (retVal[retVal.length()-1]=='#')
            retVal.erase(retVal.end()-1);
    }
    return(retVal);
}

void setCNameSuffixNumber(int number)
{
    cNameSuffixNumber=number;
}

int getCNameSuffixNumber()
{
    return(cNameSuffixNumber);
}

bool isSimulatorInitialized(const char* functionName)
{
    if (!App::isSimulatorRunning())
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_SIMULATOR_NOT_INITIALIZED);
        return(false);
    }
    return(true);
}

bool ifEditModeActiveGenerateErrorAndReturnTrue(const char* functionName)
{
    if (App::getEditModeType()!=NO_EDIT_MODE)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERRROR_EDIT_MODE_ACTIVE);
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
        if (App::ct->simulation->isSimulationStopped())
            st|=16;
        else
            st|=32;
    }
    else
        st|=1+4+16;
    if (((st&3)&when)==0)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_LAUNCH);
        return(false);
    }
    if (((st&12)&when)==0)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_WINDOW);
        return(false);
    }
    if (((st&48)&when)==0)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_CANNOT_SET_GET_PARAM_SIM);
        return(false);
    }
    return(true);
}

bool doesObjectExist(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool doesEntityExist(const char* functionName,int identifier)
{
    if (identifier>=SIM_IDSTART_COLLECTION)
    {
        if (App::ct->collections->getCollection(identifier)==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_ENTITY_INEXISTANT);
            return(false);
        }
        return(true);
    }
    else
    {
        if (App::ct->objCont->getObjectFromHandle(identifier)==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_ENTITY_INEXISTANT);
            return(false);
        }
        return(true);
    }
}

bool doesCollectionExist(const char* functionName,int identifier)
{
    if (App::ct->collections->getCollection(identifier)==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_COLLECTION_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesCollisionObjectExist(const char* functionName,int identifier)
{
    if (App::ct->collisions->getObject(identifier)==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_COLLISION_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool doesDistanceObjectExist(const char* functionName,int identifier)
{
    if (App::ct->distances->getObject(identifier)==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_DISTANCE_INEXISTANT);
        return(false);
    }
    else
        return(true);
}

bool isJoint(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_joint_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_JOINT);
        return(false);
    }
    return(true);
}
bool isShape(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_shape_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_SHAPE);
        return(false);
    }
    return(true);
}
bool isSensor(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_proximitysensor_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_PROX_SENSOR);
        return(false);
    }
    return(true);
}
bool isMill(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_mill_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_MILL);
        return(false);
    }
    return(true);
}
bool isForceSensor(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_forcesensor_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_FORCE_SENSOR);
        return(false);
    }
    return(true);
}
bool isVisionSensor(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_visionsensor_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_VISION_SENSOR);
        return(false);
    }
    return(true);
}
bool isCamera(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_camera_type)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_CAMERA);
        return(false);
    }
    return(true);
}
bool isGraph(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_graph_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_GRAPH);
        return(false);
    }
    return(true);
}
bool isPath(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_path_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_PATH);
        return(false);
    }
    return(true);
}
bool isLight(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_light_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_LIGHT);
        return(false);
    }
    return(true);
}
bool isDummy(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_dummy_type)
    { 
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_DUMMY);
        return(false);
    }
    return(true);
}
bool isOctree(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_octree_type)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_OCTREE);
        return(false);
    }
    return(true);
}
bool isPointCloud(const char* functionName,int identifier)
{
    C3DObject* it=App::ct->objCont->getObjectFromHandle(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_INEXISTANT);
        return(false);
    }
    else if (it->getObjectType()!=sim_object_pointcloud_type)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_OBJECT_NOT_POINTCLOUD);
        return(false);
    }
    return(true);
}
bool doesUIExist(const char* functionName,int elementHandle)
{
    CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_UI_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesUIButtonExist(const char* functionName,int elementHandle,int buttonHandle)
{
    CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_UI_INEXISTANT);
        return(false);
    }
    else if (it->getButtonWithID(buttonHandle)==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_UI_BUTTON_INEXISTANT);
        return(false);
    }
    return(true);
}

bool doesIKGroupExist(const char* functionName,int identifier)
{
    CikGroup* it=App::ct->ikGroups->getIkGroup(identifier);
    if (it==nullptr)
    {
        CApiErrors::setApiCallErrorMessage(functionName,SIM_ERROR_IK_GROUP_INEXISTANT);
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

#ifdef WIN_VREP
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
        printf("%zu: %s - 0x%0I64X\n",fr-i-1,symb->Name,symb->Address);
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

#ifdef WIN_VREP
    SetUnhandledExceptionFilter(_winExceptionHandler);
#else
    signal(SIGSEGV,_segHandler);
#endif
    SignalHandler sigH(SignalHandler::SIG_INT | SignalHandler::SIG_TERM | SignalHandler::SIG_CLOSE);

    handleVerSpecRunSimulator1();

    C_API_FUNCTION_DEBUG;
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
    App applicationBasicInitialization((App::operationalUIParts&sim_gui_headless)!=0);
    if (!applicationBasicInitialization.wasInitSuccessful())
        return(0);
#ifndef SIM_WITHOUT_QT_AT_ALL
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
    handleVerSpecRunSimulator2();
#endif

    handleVerSpecRunSimulator3();

    App::setApplicationName(handleVerSpec_getAppName().c_str()); // Make sure you mention in the name that it is a customized V-REP version (i.e. something like: "BenderSimulator (based on the V-REP platform)")

#ifdef SIM_WITH_GUI
    if ((App::operationalUIParts&sim_gui_headless)==0)
        App::showSplashScreen();
#endif

    handleVerSpecRunSimulator4();

    App::createMainContainer();
    CFileOperations::createNewScene(true,false);

#ifdef SIM_WITH_GUI
    if ((App::operationalUIParts&sim_gui_headless)==0)
    {
        App::createMainWindow();
        App::mainWindow->oglSurface->adjustBrowserAndHierarchySizesToDefault();
    }
#endif

    App::run(initCallBack,loopCallBack,deinitCallBack,launchSimThread); // We stay in here until we quit the application!
    printf(".");
#ifdef SIM_WITH_GUI
    App::deleteMainWindow();
#endif
    printf(".");
    App::deleteMainContainer();
    printf(".");
    handleVerSpecRunSimulator5();
    printf(".");
    CThreadPool::cleanUp();
    printf("done.\n");
    return(1);
}

simVoid* simGetMainWindow_internal(simInt type)
{ // 0=window handle , otherwise Qt pointer 
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

//    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if (type==0)
            {
#ifdef WIN_VREP
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
//    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simRefreshDialogs_internal(simInt refreshDegree)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetLastError_internal()
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string lastErr=CApiErrors::getApiCallErrorMessage();
        if (lastErr==SIM_API_CALL_NO_ERROR)
        {
            return(nullptr);
        }
        char* retVal=new char[lastErr.length()+1];
        for (unsigned int i=0;i<lastErr.length();i++)
            retVal[i]=lastErr[i];
        retVal[lastErr.length()]=0;
        CApiErrors::clearApiCallErrorMessage();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetLastError_internal(const simChar* funcName,const simChar* errorMessage)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string func(funcName);
        if (func.compare(0,8,"warning@")==0)
            CApiErrors::setLuaCallWarningMessage_fromPlugin(funcName,errorMessage);
        else
            CApiErrors::setLuaCallErrorMessage_fromPlugin(funcName,errorMessage);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetObjectHandle_internal(const simChar* objectName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        C3DObject* it;
        size_t silentErrorPos=std::string(objectName).find("@silentError");
        size_t altPos=std::string(objectName).find("@alt");
        size_t firstAtPos=std::string(objectName).find("@");
        std::string nm(objectName);
        if (firstAtPos!=std::string::npos)
            nm.erase(nm.begin()+firstAtPos,nm.end());
        if (altPos==std::string::npos)
        { // handle retrieval via regular name
            std::string objectNameAdjusted=getCNameSuffixAdjustedName(nm.c_str());
            enableCNameSuffixAdjustment();
            it=App::ct->objCont->getObjectFromName(objectNameAdjusted.c_str());
        }
        else
            it=App::ct->objCont->getObjectFromAltName(nm.c_str()); // handle retrieval via alt name

        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simRemoveObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle==sim_handle_all)
        {
            App::ct->objCont->removeAllObjects(true);
            return(1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }

        // Memorize the selection:
        std::vector<int> initSel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            initSel.push_back(App::ct->objCont->getSelID(i));

        App::ct->objCont->eraseObject(it,true);

        // Restore the initial selection:
        App::ct->objCont->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::ct->objCont->addObjectToSelection(initSel[i]);

        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveModel_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }
        if (!it->getModelBase())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_MODEL_BASE);
            return(-1);
        }

        // memorize current selection:
        std::vector<int> initSel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            initSel.push_back(App::ct->objCont->getSelID(i));

        // Erase the objects:
        std::vector<int> sel;
        sel.push_back(objectHandle);
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

        App::ct->objCont->eraseSeveralObjects(sel,true);

        // Restore the initial selection:
        App::ct->objCont->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::ct->objCont->addObjectToSelection(initSel[i]);

        return((int)sel.size());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetObjectName_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(handle);
        std::string nm;
        if ((handleFlags&sim_handleflag_altname)!=0)
            nm=it->getObjectAltName();
        else
            nm=it->getObjectName();
        char* retVal=new char[nm.length()+1];
        for (size_t i=0;i<nm.length();i++)
            retVal[i]=nm[i];
        retVal[nm.length()]=0;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetObjects_internal(simInt index,simInt objectType)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int cnter=0;
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectName_internal(simInt objectHandle,const simChar* objectName)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(handle);
        std::string originalText(objectName);
        if (originalText.length()>127)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if (originalText.length()<1)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        std::string oldName;
        if ((handleFlags&sim_handleflag_altname)!=0)
            oldName=it->getObjectAltName();
        else
            oldName=it->getObjectName();

        if (oldName.compare(text)==0)
            return(1);
        bool err;
        if ((handleFlags&sim_handleflag_altname)!=0)
            err=(App::ct->objCont->getObjectFromAltName(text.c_str())!=nullptr);
        else
            err=(App::ct->objCont->getObjectFromName(text.c_str())!=nullptr);
        if (err)
        {
            if ((handleFlags&sim_handleflag_silenterror)==0)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
            return(-1);
        }
        if ((handleFlags&sim_handleflag_altname)!=0)
            App::ct->objCont->altRenameObject(it->getObjectHandle(),text.c_str());
        else
        {
            if ( (VREP_LOWCASE_STRING_COMPARE("world",text.c_str())==0)||(VREP_LOWCASE_STRING_COMPARE("none",text.c_str())==0) )
            {
                if ((handleFlags&sim_handleflag_silenterror)==0)
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_OBJECT_NAME);
                return(-1);
            }
            App::ct->objCont->renameObject(it->getObjectHandle(),text.c_str());
            App::setFullDialogRefreshFlag();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simGetCollectionHandle_internal(const simChar* collectionName)
{
    C_API_FUNCTION_DEBUG;

    size_t silentErrorPos=std::string(collectionName).find("@silentError");
    std::string nm(collectionName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collectionNameAdjusted=getCNameSuffixAdjustedName(nm.c_str());
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CRegCollection* it=App::ct->collections->getCollection(collectionNameAdjusted);
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getCollectionID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRemoveCollection_internal(simInt collectionHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> memSel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            memSel.push_back(App::ct->objCont->getSelID(i));

        if (collectionHandle==sim_handle_all)
        {
            App::ct->objCont->deselectObjects();
            for (size_t i=0;i<App::ct->collections->allCollections.size();i++)
                App::ct->collections->addCollectionToSelection(App::ct->collections->allCollections[i]->getCollectionID());
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::ct->objCont->deselectObjects();
            App::ct->objCont->eraseSeveralObjects(sel,true);
            App::ct->collections->removeAllCollections();
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::ct->objCont->addObjectToSelection(memSel[i]);
            return(1);
        }
        else
        {
            CRegCollection* it=App::ct->collections->getCollection(collectionHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
                return(-1);
            }
            App::ct->objCont->deselectObjects();
            App::ct->collections->addCollectionToSelection(it->getCollectionID());
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::ct->objCont->deselectObjects();
            App::ct->objCont->eraseSeveralObjects(sel,true);
            App::ct->collections->removeCollection(collectionHandle);
            // Restore previous' selection state:
            for (size_t i=0;i<memSel.size();i++)
                App::ct->objCont->addObjectToSelection(memSel[i]);
            return(1);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simEmptyCollection_internal(simInt collectionHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (collectionHandle==sim_handle_all)
        {
            for (size_t i=0;i<App::ct->collections->allCollections.size();i++)
                App::ct->collections->allCollections[i]->emptyCollection();
            return(1);
        }
        CRegCollection* it=App::ct->collections->getCollection(collectionHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COLLECTION_INEXISTANT);
            return(-1);
        }
        it->emptyCollection();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetCollectionName_internal(simInt collectionHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(nullptr);
        CRegCollection* it=App::ct->collections->getCollection(collectionHandle);
        char* retVal=new char[it->getCollectionName().length()+1];
        for (unsigned int i=0;i<it->getCollectionName().length();i++)
            retVal[i]=it->getCollectionName()[i];
        retVal[it->getCollectionName().length()]=0;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetCollectionName_internal(simInt collectionHandle,const simChar* collectionName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesCollectionExist(__func__,collectionHandle))
            return(-1);
        CRegCollection* it=App::ct->collections->getCollection(collectionHandle);
        std::string originalText(collectionName);
        if (originalText.length()>127)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        std::string text(collectionName);
        tt::removeIllegalCharacters(text,true);
        if (originalText!=text)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        if (it->getCollectionName().compare(text)==0)
            return(1);
        if (App::ct->collections->getCollection(text)!=nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
            return(-1);
        }
        it->setCollectionName(originalText);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simGetObjectMatrix_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
        C7Vector tr;
        if (relObj==nullptr)
            tr=it->getCumulativeTransformationPart1();
        else
        {
            C7Vector relTr(relObj->getCumulativeTransformation());
            tr=relTr.getInverse()*it->getCumulativeTransformationPart1();
        }
        tr.getMatrix().copyToInterface(matrix);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectMatrix_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            if (!shape->getShapeIsDynamicallyStatic()) // condition new since 5/5/2013
                shape->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        }
        else
            it->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        C4X4Matrix m;
        m.copyFromInterface(matrix);
        if (relativeToObjectHandle==-1)
            App::ct->objCont->setAbsoluteConfiguration(it->getObjectHandle(),m.getTransformation(),false);
        else
        {
            C3DObject* objRel=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            C7Vector relTr(objRel->getCumulativeTransformation());
            App::ct->objCont->setAbsoluteConfiguration(it->getObjectHandle(),relTr*m.getTransformation(),false);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectPosition_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        C7Vector tr;
        if (relativeToObjectHandle==-1)
            tr=it->getCumulativeTransformationPart1();
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            if (it->getParentObject()==relObj)
            { // special here, in order to not lose precision in a series of get/set
                tr=it->getLocalTransformationPart1();
            }
            else
            {
                C7Vector relTr(relObj->getCumulativeTransformationPart1());
                tr=relTr.getInverse()*it->getCumulativeTransformationPart1();
            }
        }
        tr.X.copyTo(position);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectPosition_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            if (!shape->getShapeIsDynamicallyStatic()) // condition new since 5/5/2013
                shape->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        }
        else
            it->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        if (relativeToObjectHandle==-1)
            App::ct->objCont->setAbsolutePosition(it->getObjectHandle(),C3Vector(position));
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            if (it->getParentObject()==relObj)
            { // special here, in order to not lose precision in a series of get/set
                C7Vector tr(it->getLocalTransformationPart1());
                tr.X=position;
                it->setLocalTransformation(tr);
            }
            else
            {
                C7Vector absTr(it->getCumulativeTransformationPart1());
                C7Vector relTr(relObj->getCumulativeTransformationPart1());
                C7Vector x(relTr.getInverse()*absTr);
                x.X.set(position);
                absTr=relTr*x;
                App::ct->objCont->setAbsolutePosition(it->getObjectHandle(),absTr.X);
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectOrientation_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* eulerAngles)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        C7Vector tr;
        if (relativeToObjectHandle==-1)
            tr=it->getCumulativeTransformationPart1();
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            C7Vector relTr(relObj->getCumulativeTransformationPart1());
            tr=relTr.getInverse()*it->getCumulativeTransformationPart1();
        }
        C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectOrientation_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* eulerAngles)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            if (!shape->getShapeIsDynamicallyStatic()) // condition new since 5/5/2013
                shape->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        }
        else
            it->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        if (relativeToObjectHandle==-1)
            App::ct->objCont->setAbsoluteAngles(it->getObjectHandle(),C3Vector(eulerAngles));
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            C7Vector absTr(it->getCumulativeTransformationPart1());
            C7Vector relTr(relObj->getCumulativeTransformationPart1());
            C7Vector x(relTr.getInverse()*absTr);
            x.Q.setEulerAngles(eulerAngles);
            absTr=relTr*x;
            App::ct->objCont->setAbsoluteAngles(it->getObjectHandle(),absTr.Q.getEulerAngles());
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointPosition_internal(simInt objectHandle,simFloat* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        position[0]=it->getPosition();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointPosition_internal(simInt objectHandle,simFloat position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        it->setPosition(position);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointTargetPosition_internal(simInt objectHandle,simFloat targetPosition)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        if (it->getJointMode()==sim_jointmode_force)
            it->setDynamicMotorPositionControlTargetPosition(targetPosition);
        else
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_NOT_IN_FORCE_TORQUE_MODE);
            return(-1);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointTargetPosition_internal(simInt objectHandle,simFloat* targetPosition)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        targetPosition[0]=it->getDynamicMotorPositionControlTargetPosition();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointForce_internal(simInt objectHandle,simFloat forceOrTorque)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        it->setDynamicMotorMaximumForce(forceOrTorque);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathPosition_internal(simInt objectHandle,simFloat* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(objectHandle);
        position[0]=float(it->pathContainer->getPosition());
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetPathPosition_internal(simInt objectHandle,simFloat position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(objectHandle);
        it->pathContainer->setPosition(position);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathLength_internal(simInt objectHandle,simFloat* length)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isPath(__func__,objectHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(objectHandle);
        length[0]=it->pathContainer->getBezierVirtualPathLength();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointTargetVelocity_internal(simInt objectHandle,simFloat targetVelocity)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        if (it->getJointMode()==sim_jointmode_force)
            it->setDynamicMotorTargetVelocity(targetVelocity);
        else
            it->setTargetVelocity_DEPRECATED(targetVelocity);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointTargetVelocity_internal(simInt objectHandle,simFloat* targetVelocity)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
            return(-1);
        }
        if (it->getJointMode()==sim_jointmode_force)
            targetVelocity[0]=it->getDynamicMotorTargetVelocity();
        else
            targetVelocity[0]=it->getTargetVelocity_DEPRECATED();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetPathTargetNominalVelocity_internal(simInt objectHandle,simFloat targetNominalVelocity)
{
    C_API_FUNCTION_DEBUG;

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
        if (!isPath(__func__,objectHandle))
        {
            return(-1);
        }
        CPath* it=App::ct->objCont->getPath(objectHandle);
        it->pathContainer->setTargetNominalVelocity(targetNominalVelocity);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointMatrix_internal(simInt objectHandle,simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        C7Vector trFull(it->getLocalTransformation());
        C7Vector trPart1(it->getLocalTransformationPart1());
        C7Vector tr(trPart1.getInverse()*trFull);
        tr.getMatrix().copyToInterface(matrix);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSphericalJointMatrix_internal(simInt objectHandle,const simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        if (it->getJointType()!=sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_NOT_SPHERICAL);
            return(-1);
        }
        C4X4Matrix m;
        m.copyFromInterface(matrix);
        it->setSphericalTransformation(C4Vector(m.M.getQuaternion()));
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointInterval_internal(simInt objectHandle,simBool* cyclic,simFloat* interval)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        cyclic[0]=0;
        if (it->getPositionIsCyclic())
            cyclic[0]=1;
        interval[0]=it->getPositionIntervalMin();
        interval[1]=it->getPositionIntervalRange();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointInterval_internal(simInt objectHandle,simBool cyclic,const simFloat* interval)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        if (!isJoint(__func__,objectHandle))
            return(-1);
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
// Some models need to modify that
//        if ( App::ct->simulation->isSimulationStopped()||((it->getJointMode()!=sim_jointmode_force)&&(!it->getHybridFunctionality())) )
        {
            float previousPos=it->getPosition();
            it->setPositionIsCyclic(cyclic!=0);
            it->setPositionIntervalMin(interval[0]);
            it->setPositionIntervalRange(interval[1]);
            it->setPosition(previousPos);
            return(1);
        }
//        return(-1);
//        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectParent_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=-1;
        if (it->getParentObject()!=nullptr)
            retVal=it->getParentObject()->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectChild_internal(simInt objectHandle,simInt index)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=-1;
        if (int(it->childList.size())>index)
            retVal=it->childList[index]->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectParent_internal(simInt objectHandle,simInt parentObjectHandle,simBool keepInPlace)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        C3DObject* parentIt=App::ct->objCont->getObjectFromHandle(parentObjectHandle);
        if (keepInPlace)
            App::ct->objCont->makeObjectChildOf(it,parentIt);
        else
        {
            if ( (handleFlags&sim_handleflag_assembly)&&(parentIt!=nullptr) )
            { // only assembling
                // There is another such similar routine!! XXBFVGA
                std::vector<C3DObject*> potParents;
                parentIt->getAllChildrenThatMayBecomeAssemblyParent(it->getChildAssemblyMatchValuesPointer(),potParents);
                bool directAssembly=parentIt->doesParentAssemblingMatchValuesMatchWithChild(it->getChildAssemblyMatchValuesPointer());
                if ( directAssembly||(potParents.size()==1) )
                {
                    if (directAssembly)
                        App::ct->objCont->makeObjectChildOf(it,parentIt);
                    else
                        App::ct->objCont->makeObjectChildOf(it,potParents[0]);
                    if (it->getAssemblingLocalTransformationIsUsed())
                        it->setLocalTransformation(it->getAssemblingLocalTransformation());
                }
                else
                    it->setParentObject(parentIt);
            }
            else
                it->setParentObject(parentIt);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectType_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=it->getObjectType();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointType_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(objectHandle);
        int retVal=it->getJointType();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simBuildIdentityMatrix_internal(simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    m.setIdentity();
    m.copyToInterface(matrix);
    return(1);
}

simInt simCopyMatrix_internal(const simFloat* matrixIn,simFloat* matrixOut)
{
    C_API_FUNCTION_DEBUG;

    for (int i=0;i<12;i++)
        matrixOut[i]=matrixIn[i];
    return(1);
}

simInt simBuildMatrix_internal(const simFloat* position,const simFloat* eulerAngles,simFloat* matrix)
{
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    m.M.setEulerAngles(C3Vector(eulerAngles));
    m.X.set(position);
    m.copyToInterface(matrix);
    return(1);
}

simInt simBuildMatrixQ_internal(const simFloat* position,const simFloat* quaternion,simFloat* matrix)
{
    // V-REP quaternion, internally: w x y z
    // V-REP quaternion, at interfaces: x y z w
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    C4Vector q(quaternion[3],quaternion[0],quaternion[1],quaternion[2]);
    m.M=q.getMatrix();
    m.X.set(position);
    m.copyToInterface(matrix);
    return(1);
}

simInt simGetEulerAnglesFromMatrix_internal(const simFloat* matrix,simFloat* eulerAngles)
{
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    m.M.getEulerAngles().copyTo(eulerAngles);
    return(1);
}

simInt simGetQuaternionFromMatrix_internal(const simFloat* matrix,simFloat* quaternion)
{
    // V-REP quaternion, internally: w x y z
    // V-REP quaternion, at interfaces: x y z w
    C_API_FUNCTION_DEBUG;

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
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    m.inverse();
    m.copyToInterface(matrix);
    return(1);
}

simInt simMultiplyMatrices_internal(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat* matrixOut)
{
    C_API_FUNCTION_DEBUG;

    C4X4Matrix mIn1;
    mIn1.copyFromInterface(matrixIn1);
    C4X4Matrix mIn2;
    mIn2.copyFromInterface(matrixIn2);
    (mIn1*mIn2).copyToInterface(matrixOut);
    return(1);
}

simInt simInterpolateMatrices_internal(const simFloat* matrixIn1,const simFloat* matrixIn2,simFloat interpolFactor,simFloat* matrixOut)
{
    C_API_FUNCTION_DEBUG;

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
    C_API_FUNCTION_DEBUG;

    C4X4Matrix m;
    m.copyFromInterface(matrix);
    C3Vector v(vect);
    (m*v).copyTo(vect);
    return(1);
}

simInt simReservedCommand_internal(simInt v,simInt w)
{
    C_API_FUNCTION_DEBUG;

    if (v==0)
        VThread::sleep(w);
    if (v==2)
    {
        int retVal=App::ct->getModificationFlags(true);
        return(retVal);
    }
    return(handleVerSpec_handleReservedCmd1(v,w));
}

simInt simSetBoolParameter_internal(simInt parameter,simBool boolState)
{
    C_API_FUNCTION_DEBUG;
    bool couldNotLock=true;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        couldNotLock=false;
        if (parameter==sim_boolparam_exit_request)
        {
            if ( App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=FINAL_EXIT_REQUEST_CMD;
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
            if (App::ct->environment==nullptr)
                return(-1);
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            App::ct->environment->setShapeTexturesEnabled(boolState!=0);
            return(1);
        }
        if ( (parameter==sim_boolparam_show_w_emitters)||(parameter==sim_boolparam_show_w_receivers) )
        {
            if (App::ct->environment==nullptr)
                return(-1);
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (parameter==sim_boolparam_show_w_emitters)
                App::ct->environment->setVisualizeWirelessEmitters(boolState!=0);
            else
                App::ct->environment->setVisualizeWirelessReceivers(boolState!=0);
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->infoWindowOpenState=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_threaded_rendering_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+32))
                return(-1);
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->getThreadedRendering())
            {
                if (boolState==0)
                    App::ct->simulation->toggleThreadedRendering(true);
            }
            else
            {
                if (boolState!=0)
                    App::ct->simulation->toggleThreadedRendering(true);
            }
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->statusBoxOpenState=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_fog_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->setFogEnabled(boolState!=0);
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->collisionDetectionEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_distance_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->distanceCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_ik_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->ikCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_gcs_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->gcsCalculationEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_dynamics_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            App::ct->dynamicsContainer->setDynamicsEnabled(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_joint_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->jointMotionHandlingEnabled_DEPRECATED=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_path_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->pathMotionHandlingEnabled_DEPRECATED=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_proximity_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->proximitySensorsEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_vision_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->visionSensorsEnabled=(boolState!=0);
            return(1);
        }

        if (parameter==sim_boolparam_mirrors_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->mirrorsDisabled=(boolState==0);
            return(1);
        }

        if (parameter==sim_boolparam_aux_clip_planes_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->clippingPlanesDisabled=(boolState==0);
            return(1);
        }
        if (parameter==sim_boolparam_reserved3)
        {
            fullModelCopyFromApi=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_realtime_simulation)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
            {
                App::ct->simulation->setRealTimeSimulation(boolState!=0);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_boolparam_online_mode)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
            {
                App::ct->simulation->setOnlineMode(boolState!=0);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_boolparam_br_partrepository)
        {
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->setShowPartRepository(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_br_palletrepository)
        {
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->setShowPalletRepository(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_br_jobfunc)
        {
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->setJobFunctionalityEnabled(boolState!=0);
            return(1);
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->millsEnabled=(boolState!=0);
            return(1);
        }
        if (parameter==sim_boolparam_video_recording_triggered)
        {
#ifdef SIM_WITH_GUI
            if ((App::mainWindow!=nullptr)&&(!App::mainWindow->simulationRecorder->getIsRecording())&&App::ct->simulation->isSimulationStopped() )
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
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);

    couldNotLock=true;
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::ct->objCont==nullptr)
            return(-1);
        couldNotLock=false;
        if ( (parameter==sim_boolparam_force_calcstruct_all_visible)||(parameter==sim_boolparam_force_calcstruct_all) )
        {
            int displayAttrib=sim_displayattribute_renderpass;
            for (int i=0;i<int(App::ct->objCont->shapeList.size());i++)
            {
                int shapeHandle=App::ct->objCont->shapeList[i];
                CShape* shape=App::ct->objCont->getShape(shapeHandle);
                if (shape->getShouldObjectBeDisplayed(-1,displayAttrib)||(parameter==sim_boolparam_force_calcstruct_all))
                    shape->geomData->initializeCalculationStructureIfNeeded();
            }
            return(1);
        }
    }

    if (couldNotLock)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
    return(-1);
}

simInt simGetBoolParameter_internal(simInt parameter)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_boolparam_realtime_simulation)
        {
            int retVal=0;
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->getRealTimeSimulation())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_online_mode)
        {
            int retVal=0;
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->getOnlineMode())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_br_partrepository)
        {
            int retVal=0;
            if (App::ct->environment==nullptr)
                return(-1);
            if (App::ct->environment->getShowPartRepository())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_br_palletrepository)
        {
            int retVal=0;
            if (App::ct->environment==nullptr)
                return(-1);
            if (App::ct->environment->getShowPalletRepository())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_br_jobfunc)
        {
            int retVal=0;
            if (App::ct->environment==nullptr)
                return(-1);
            if (App::ct->environment->getJobFunctionalityEnabled())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_scene_closing)
        {
            int retVal=0;
            if (App::ct->environment==nullptr)
                return(-1);
            if (App::ct->environment->getSceneIsClosingFlag())
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
            if (App::ct->environment==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->environment->getShapeTexturesEnabled())
                retVal=1;
            return(retVal);
        }
        if ( (parameter==sim_boolparam_show_w_emitters)||(parameter==sim_boolparam_show_w_receivers) )
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            int retVal=0;
            if (parameter==sim_boolparam_show_w_emitters)
            {
                if (App::ct->environment->getVisualizeWirelessEmitters())
                    retVal=1;
            }
            else
            {
                if (App::ct->environment->getVisualizeWirelessReceivers())
                    retVal=1;
            }
            return(retVal);
        }
        if (parameter==sim_boolparam_threaded_rendering_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+32))
                return(-1);
            if (App::ct->simulation==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->simulation->getThreadedRendering())
                retVal=1;
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->infoWindowOpenState)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_statustext_open)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->statusBoxOpenState)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_fog_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->environment->getFogEnabled())
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_rml2_available)
        {
            int retVal=0;
            if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
                retVal=1;
            return(retVal);
        }

        if (parameter==sim_boolparam_rml4_available)
        {
            int retVal=0;
            if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->collisionDetectionEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_distance_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->distanceCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_ik_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->ikCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_gcs_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->gcsCalculationEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_dynamics_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->dynamicsContainer->getDynamicsEnabled())
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_joint_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->jointMotionHandlingEnabled_DEPRECATED)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_path_motion_handling_enabled_deprecated)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->pathMotionHandlingEnabled_DEPRECATED)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_proximity_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->proximitySensorsEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_vision_sensor_handling_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->visionSensorsEnabled)
                retVal=1;
            return(retVal);
        }
        if (parameter==sim_boolparam_mirrors_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=1;
            if (App::ct->mainSettings->mirrorsDisabled)
                retVal=0;
            return(retVal);
        }
        if (parameter==sim_boolparam_aux_clip_planes_enabled)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=1;
            if (App::ct->mainSettings->clippingPlanesDisabled)
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
            if (App::ct->mainSettings==nullptr)
                return(-1);
            int retVal=0;
            if (App::ct->mainSettings->millsEnabled)
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

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetArrayParameter_internal(simInt parameter,const simVoid* arrayOfValues)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_arrayparam_gravity)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            App::ct->dynamicsContainer->setGravity(C3Vector((float*)arrayOfValues));
            return(1);
        }

        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->setFogStart(((float*)arrayOfValues)[0]);
            App::ct->environment->setFogEnd(((float*)arrayOfValues)[1]);
            App::ct->environment->setFogDensity(((float*)arrayOfValues)[2]);
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->fogBackgroundColor[0]=((float*)arrayOfValues)[0];
            App::ct->environment->fogBackgroundColor[1]=((float*)arrayOfValues)[1];
            App::ct->environment->fogBackgroundColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->backGroundColorDown[0]=((float*)arrayOfValues)[0];
            App::ct->environment->backGroundColorDown[1]=((float*)arrayOfValues)[1];
            App::ct->environment->backGroundColorDown[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->backGroundColor[0]=((float*)arrayOfValues)[0];
            App::ct->environment->backGroundColor[1]=((float*)arrayOfValues)[1];
            App::ct->environment->backGroundColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            App::ct->environment->ambientLightColor[0]=((float*)arrayOfValues)[0];
            App::ct->environment->ambientLightColor[1]=((float*)arrayOfValues)[1];
            App::ct->environment->ambientLightColor[2]=((float*)arrayOfValues)[2];
            return(1);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetArrayParameter_internal(simInt parameter,simVoid* arrayOfValues)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_arrayparam_gravity)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            C3Vector g(App::ct->dynamicsContainer->getGravity());
            g.copyTo((float*)arrayOfValues);
            return(1);
        }
        if (parameter==sim_arrayparam_fog)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::ct->environment->getFogStart();
            ((float*)arrayOfValues)[1]=App::ct->environment->getFogEnd();
            ((float*)arrayOfValues)[2]=App::ct->environment->getFogDensity();
            return(1);
        }
        if (parameter==sim_arrayparam_fog_color)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::ct->environment->fogBackgroundColor[0];
            ((float*)arrayOfValues)[1]=App::ct->environment->fogBackgroundColor[1];
            ((float*)arrayOfValues)[2]=App::ct->environment->fogBackgroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color1)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::ct->environment->backGroundColorDown[0];
            ((float*)arrayOfValues)[1]=App::ct->environment->backGroundColorDown[1];
            ((float*)arrayOfValues)[2]=App::ct->environment->backGroundColorDown[2];
            return(1);
        }
        if (parameter==sim_arrayparam_background_color2)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::ct->environment->backGroundColor[0];
            ((float*)arrayOfValues)[1]=App::ct->environment->backGroundColor[1];
            ((float*)arrayOfValues)[2]=App::ct->environment->backGroundColor[2];
            return(1);
        }
        if (parameter==sim_arrayparam_ambient_light)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+4+8+16+32))
                return(-1);
            if (App::ct->environment==nullptr)
                return(-1);
            ((float*)arrayOfValues)[0]=App::ct->environment->ambientLightColor[0];
            ((float*)arrayOfValues)[1]=App::ct->environment->ambientLightColor[1];
            ((float*)arrayOfValues)[2]=App::ct->environment->ambientLightColor[2];
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


        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



simInt simSetInt32Parameter_internal(simInt parameter,simInt intState)
{
    C_API_FUNCTION_DEBUG;

    if (parameter==sim_intparam_error_report_mode)
    { // do not lock with this parameter: you should be able to call it anytime, from the UI or the SIM thread
        CApiErrors::setApiCallErrorReportMode(intState);
        return(1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_intparam_server_port_next)
        {
            App::userSettings->setNextFreeServerPortToUse(intState);
            return(1);
        }
        if (parameter==sim_intparam_current_page)
        {
            if (App::ct->pageContainer==nullptr)
                return(-1);
#ifdef SIM_WITH_GUI
            App::ct->pageContainer->setActivePage(intState);
#endif
            return(1);
        }
        if (parameter==sim_intparam_visible_layers)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->setActiveLayers((unsigned short)intState);
            return(1);
        }
        if (parameter==sim_intparam_infotext_style)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            App::ct->mainSettings->infoWindowColorStyle=intState;
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
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
                return(-1);
            App::ct->simulation->setSpeedModifierIndexOffset(intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
                return(-1);
            if (App::ct->dynamicsContainer->setCurrentIterationCount(intState))
                return(1);
            return(-1);
        }
        if (parameter==sim_intparam_scene_index)
        {
            App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD,intState);
            return(1);
        }
        if (parameter==sim_intparam_motionplanning_seed)
        {
            CMotionPlanningTask::randomSeed=intState;
            return(1);
        }
        if (parameter==sim_intparam_dynamic_engine)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
            {
                App::ct->dynamicsContainer->setDynamicEngineType(intState,0);
                return(1);
            }
            return(0);
        }
        if (parameter==sim_intparam_idle_fps)
        {
            App::userSettings->setIdleFps(intState);
            return(1);
        }
        if (parameter==sim_intparam_dynamic_warning_disabled_mask)
        {
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            App::ct->dynamicsContainer->setTempDisabledWarnings(intState);
            return(1);
        }
        if (parameter==sim_intparam_simulation_warning_disabled_mask)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            App::ct->simulation->setDisableWarningsFlags(intState);
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
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUInt64Parameter_internal(simInt parameter,simUInt64* intState)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_uint64param_simulation_time_step_ns)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            intState[0]=App::ct->simulation->getSimulationTimeStep_speedModified_ns();
            return(1);
        }
        if (parameter==sim_uint64param_simulation_time_ns)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            intState[0]=App::ct->simulation->getSimulationTime_ns();
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetInt32Parameter_internal(simInt parameter,simInt* intState)
{
    C_API_FUNCTION_DEBUG;
    if (parameter==sim_intparam_error_report_mode)
    { // do not lock with this parameter: you should be able to call it anytime, from the UI or the SIM thread
        intState[0]=CApiErrors::getApiCallErrorReportMode();
        return(1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_intparam_stop_request_counter)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            intState[0]=App::ct->simulation->getStopRequestCounter();
            return(1);
        }
        if (parameter==sim_intparam_program_version)
        {
            intState[0]=VREP_PROGRAM_VERSION_NB;
            return(1);
        }
        if (parameter==sim_intparam_program_revision)
        {
            intState[0]=VREP_PROGRAM_REVISION_NB;
            return(1);
        }
        if (parameter==sim_intparam_program_full_version)
        {
            intState[0]=VREP_PROGRAM_FULL_VERSION_NB;
            return(1);
        }

        if (parameter==sim_intparam_scene_unique_id)
        {
            if (App::ct->environment==nullptr)
                return(-1);
            intState[0]=App::ct->environment->getSceneUniqueID();
            return(1);
        }
        if (parameter==sim_intparam_platform)
        {
    #ifdef WIN_VREP
            intState[0]=0;
    #endif
    #ifdef MAC_VREP
            intState[0]=1;
    #endif
    #ifdef LIN_VREP
            intState[0]=2;
    #endif
            return(1);
        }
        if (parameter==sim_intparam_event_flags_read)
        {
            intState[0]=App::ct->getModificationFlags(false);
            return(1);
        }
        if (parameter==sim_intparam_event_flags_read_clear)
        {
            intState[0]=App::ct->getModificationFlags(true);
            return(1);
        }

        if (parameter==sim_intparam_qt_version)
        {
            intState[0]=0;
#ifndef SIM_WITHOUT_QT_AT_ALL
            intState[0]=(QT_VERSION>>16)*10000+((QT_VERSION>>8)&255)*100+(QT_VERSION&255)*1;
#endif
            return(1);
        }
        if (parameter==sim_intparam_compilation_version)
        {
            intState[0]=VREP_COMPILATION_VERSION;
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
            if (App::ct->pageContainer==nullptr)
                return(-1);
#ifdef SIM_WITH_GUI
            intState[0]=App::ct->pageContainer->getActivePageIndex();
#else
            intState[0]=0;
#endif
            return(1);
        }
        if (parameter==sim_intparam_visible_layers)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            intState[0]=int(App::ct->mainSettings->getActiveLayers());
            return(1);
        }
        if (parameter==sim_intparam_infotext_style)
        {
            if (!canBoolIntOrFloatParameterBeSetOrGet(__func__,2+8+16+32))
                return(-1);
            if (App::ct->mainSettings==nullptr)
                return(-1);
            intState[0]=int(App::ct->mainSettings->infoWindowColorStyle);
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
            if (editMode==PATH_EDIT_MODE)
                intState[0]=4;
            if (editMode==BUTTON_EDIT_MODE)
                intState[0]=5;
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
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::ct->dynamicsContainer->getTempDisabledWarnings();
            return(1);
        }
        if (parameter==sim_intparam_simulation_warning_disabled_mask)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            intState[0]=App::ct->simulation->getDisableWarningsFlags();
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
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
                return(-1);
            intState[0]=App::ct->simulation->getSpeedModifierIndexOffset();
            return(1);
        }
        if (parameter==sim_intparam_dynamic_iteration_count)
        {
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::ct->dynamicsContainer->getCurrentIterationCount();
            return(1);
        }
        if (parameter==sim_intparam_job_count)
        {
            if (App::ct->environment==nullptr)
                return(-1);
            intState[0]=App::ct->environment->getJobCount();
            return(1);
        }
        if (parameter==sim_intparam_scene_index)
        {
            intState[0]=App::ct->getCurrentInstanceIndex();
            return(1);
        }
#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(parameter==sim_intparam_flymode_camera_handle) )
        {
            intState[0]=App::mainWindow->getFlyModeCameraHandle();
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
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            intState[0]=App::ct->dynamicsContainer->getDynamicEngineType(nullptr);
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
        if (parameter==sim_intparam_server_port_range)
        {
            intState[0]=App::userSettings->freeServerPortRange;
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetFloatParameter_internal(simInt parameter,simFloat floatState)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_floatparam_simulation_time_step)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
            {
                App::ct->simulation->setSimulationTimeStep_raw_ns(quint64(floatState*1000000.0f));
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
            if (App::ct->simulation==nullptr)
                return(-1);
            if (App::ct->simulation->isSimulationStopped())
            {
                if (App::ct->dynamicsContainer->setCurrentDynamicStepSize(floatState))
                    return(1);
            }
            return(0);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetFloatParameter_internal(simInt parameter,simFloat* floatState)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (parameter==sim_floatparam_rand)
        {
            floatState[0]=SIM_RAND_FLOAT;
            return(1);
        }
        if (parameter==sim_floatparam_simulation_time_step)
        {
            if (App::ct->simulation==nullptr)
                return(-1);
            floatState[0]=float(App::ct->simulation->getSimulationTimeStep_speedModified_ns())/1000000.0f;
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
            if (App::ct->dynamicsContainer==nullptr)
                return(-1);
            floatState[0]=App::ct->dynamicsContainer->getCurrentDynamicStepSize();
            return(1);
        }
        if (parameter==sim_floatparam_mouse_wheel_zoom_factor)
        {
            floatState[0]=App::userSettings->mouseWheelZoomFactor;
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetStringParameter_internal(simInt parameter,const simChar* str)
{
    C_API_FUNCTION_DEBUG;

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
        if (parameter==sim_stringparam_additional_addonscript_firstscene)
        {
            App::ct->addOnScriptContainer->setAdditionalAddOnScript(str);
            return(1);
        }
        if (parameter==sim_stringparam_additional_addonscript)
        {
            App::ct->addOnScriptContainer->setAdditionalAddOnScript(str);
            return(1);
        }
        if (parameter==sim_stringparam_job)
        {
            bool r=App::ct->environment->setCurrentJob(str);
            if (r)
                return(1);
            return(0);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStringParameter_internal(simInt parameter)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* dval=handleVerSpec_getStringParam(parameter);
        if (dval!=nullptr)
            return(dval);
        if (parameter==sim_stringparam_application_path)
        {
            char* retVal=new char[App::directories->executableDirectory.length()+1];
            for (int i=0;i<int(App::directories->executableDirectory.length());i++)
                retVal[i]=App::directories->executableDirectory[i];
            retVal[App::directories->executableDirectory.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_scene_path_and_name)
        {
            if (App::ct->mainSettings==nullptr)
                return(nullptr);
            std::string pan(App::ct->mainSettings->getScenePathAndName());
            char* retVal=new char[pan.length()+1];
            for (size_t i=0;i<pan.length();i++)
                retVal[i]=pan[i];
            retVal[pan.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_scene_name)
        {
            if (App::ct->mainSettings==nullptr)
                return(nullptr);
            std::string tmp(App::ct->mainSettings->getSceneNameWithExt());
            char* retVal=new char[tmp.length()+1];
            for (int i=0;i<int(tmp.length());i++)
                retVal[i]=tmp[i];
            retVal[tmp.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_scene_unique_id)
        {
            if (App::ct->environment==nullptr)
                return(nullptr);
            std::string tmp(App::ct->environment->getUniquePersistentIdString());
            tmp=CTTUtil::encode64(tmp);
            char* retVal=new char[tmp.length()+1];
            for (int i=0;i<int(tmp.length());i++)
                retVal[i]=tmp[i];
            retVal[tmp.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_scene_path)
        {
            if (App::ct->mainSettings==nullptr)
                return(nullptr);
            std::string tmp(App::ct->mainSettings->getScenePath());
            char* retVal=new char[tmp.length()+1];
            for (int i=0;i<int(tmp.length());i++)
                retVal[i]=tmp[i];
            retVal[tmp.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_remoteapi_temp_file_dir)
        {
            char* retVal=new char[App::directories->remoteApiFileTransferDirectory.length()+1];
            for (int i=0;i<int(App::directories->remoteApiFileTransferDirectory.length());i++)
                retVal[i]=App::directories->remoteApiFileTransferDirectory[i];
            retVal[App::directories->remoteApiFileTransferDirectory.length()]=0;
            return(retVal);
        }
        if (parameter==sim_stringparam_video_filename)
        {
            char* retVal=nullptr;
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
            {
                char userSet;
                std::string s(App::mainWindow->simulationRecorder->getPath(&userSet));
                if (userSet==0)
                    s+="/";
                retVal=new char[s.length()+1];
                for (int i=0;i<int(s.length());i++)
                    retVal[i]=s[i];
                retVal[s.length()]=0;
            }
#endif
            return(retVal);
        }
        if (parameter==sim_stringparam_job)
        {
            std::string job(App::ct->environment->getCurrentJob());
            char* retVal=new char[job.length()+1];
            for (size_t i=0;i<job.length();i++)
                retVal[i]=job[i];
            retVal[job.length()]=0;
            return(retVal);
        }
        if ( (parameter>=sim_stringparam_job0)&&(parameter<=sim_stringparam_job99) )
        {
            std::string job(App::ct->environment->getJobAtIndex(parameter-sim_stringparam_job0));
            if (job.size()>0)
            {
                char* retVal=new char[job.length()+1];
                for (size_t i=0;i<job.length();i++)
                    retVal[i]=job[i];
                retVal[job.length()]=0;
                return(retVal);
            }
            return(nullptr);
        }
        if ((parameter>=sim_stringparam_app_arg1)&&(parameter<=sim_stringparam_app_arg9))
        {
            std::string s(App::getApplicationArgument(parameter-sim_stringparam_app_arg1));
            char* retVal=new char[s.length()+1];
            for (int i=0;i<int(s.length());i++)
                retVal[i]=s[i];
            retVal[s.length()]=0;
            return(retVal);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PARAMETER);
        return(nullptr);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simFloat simGetSimulationTime_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1.0f);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        float retVal=float(App::ct->simulation->getSimulationTime_ns())/1000000.0f;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0f);
}

simInt simGetSimulationState_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::ct->simulation->getSimulationState();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat simGetSystemTime_internal()
{
    C_API_FUNCTION_DEBUG;
//  float retVal=float(float(VDateTime::getTimeInMs())/1000.0f);
    float retVal=float(VDateTime::getOSTimeInMs())/1000.0f;
    return(retVal);
}

simInt simGetSystemTimeInMilliseconds_internal()
{
    C_API_FUNCTION_DEBUG;
//  int retVal=VDateTime::getTimeInMs();
    int retVal=int(VDateTime::getOSTimeInMs());
    return(retVal);
}

simUInt simGetSystemTimeInMs_internal(simInt previousTime)
{
    C_API_FUNCTION_DEBUG;
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

simInt simLoadScene_internal(const simChar* filename)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FILE_NOT_FOUND);
                return(-1);
            }
        }

        if (keepCurrent)
            CFileOperations::createNewScene(outputSceneOrModelLoadMessagesWithApiCall,true);

        if (!CFileOperations::loadScene(nm.c_str(),outputSceneOrModelLoadMessagesWithApiCall,outputSceneOrModelLoadMessagesWithApiCall,false))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_COULD_NOT_BE_READ);
            return(-1);
        }
        App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCloseScene_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        CFileOperations::closeScene(false,false);
        return(App::ct->getCurrentInstanceIndex());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simLoadModel_internal(const simChar* filename)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string nm(filename);
        size_t atCopyPos=nm.find("@copy");
        bool forceAsCopy=(atCopyPos!=std::string::npos);
        if (forceAsCopy)
            nm.erase(nm.begin()+atCopyPos,nm.end());

        if (!VFile::doesFileExist(nm))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FILE_NOT_FOUND);
            return(-1);
        }
        if (!CFileOperations::loadModel(nm.c_str(),outputSceneOrModelLoadMessagesWithApiCall,outputSceneOrModelLoadMessagesWithApiCall,false,nullptr,true,nullptr,false,forceAsCopy))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MODEL_COULD_NOT_BE_READ);
            return(-1);
        }
        int retVal=App::ct->objCont->getLastSelectionID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetSimulatorMessage_internal(simInt* messageID,simInt* auxiliaryData,simInt* returnedDataSize)
{
    C_API_FUNCTION_DEBUG;

    char* retVal=App::ct->simulatorMessageQueue->extractOneCommand(messageID[0],auxiliaryData,returnedDataSize[0]);
    return(retVal);
}

simInt simSaveScene_internal(const simChar* filename)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    if (App::ct->environment->getSceneLocked())
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::ct->environment->getRequestFinalSave())
            App::ct->environment->setSceneLocked(); // silent locking!

        if (!CFileOperations::saveScene(filename,false,false,false,false))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_COULD_NOT_BE_SAVED);
            return(-1);
        }
        CFileOperations::addToRecentlyOpenedScenes(App::ct->mainSettings->getScenePathAndName());
        App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveModel_internal(int baseOfModelHandle,const simChar* filename)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    if (App::ct->environment->getSceneLocked())
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
        return(-1);
    }
    if (!doesObjectExist(__func__,baseOfModelHandle))
        return(-1);
    C3DObject* it=App::ct->objCont->getObjectFromHandle(baseOfModelHandle);
    if (!it->getModelBase())
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_MODEL_BASE);
        return(-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<int> initSelection;
        App::ct->objCont->getSelectedObjects(initSelection);
        if (!CFileOperations::saveModel(baseOfModelHandle,filename,false,false,false))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MODEL_COULD_NOT_BE_SAVED);
            return(-1);
        }
        App::ct->objCont->setSelectedObjects(initSelection);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simDoesFileExist_internal(const simChar* filename)
{
    C_API_FUNCTION_DEBUG;

    if (!VFile::doesFileExist(filename))
    {
        return(0);
    }
    return(1);
}

simInt simIsObjectInSelection_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::ct->objCont->isObjectInSelection(objectHandle))
            retVal|=1;
        C3DObject* lastSel=App::ct->objCont->getLastSelection_object();
        if (lastSel!=nullptr)
        {
            if (lastSel->getObjectHandle()==objectHandle)
                retVal|=2;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddObjectToSelection_internal(simInt what,simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
        {
            return(-1);
        }
        if (what==sim_handle_all)
            App::ct->objCont->selectAllObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::ct->objCont->addObjectToSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<C3DObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::ct->objCont->addObjectToSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simRemoveObjectFromSelection_internal(simInt what,simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
        {
            return(-1);
        }
        if (what==sim_handle_all)
            App::ct->objCont->deselectObjects();
        else
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (what==sim_handle_single)
                App::ct->objCont->removeObjectFromSelection(objectHandle);
            else
            {
                if ((what==sim_handle_tree)||(what==sim_handle_chain))
                {
                    std::vector<C3DObject*> allObjects;
                    if (what==sim_handle_tree)
                        it->getAllObjectsRecursive(&allObjects,true,true);
                    if (what==sim_handle_chain)
                        it->getChain(allObjects,true,true);
                    for (int i=0;i<int(allObjects.size());i++)
                        App::ct->objCont->removeObjectFromSelection(allObjects[i]->getObjectHandle());
                }
                else
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
                    return(-1);
                }
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simGetObjectSelectionSize_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::ct->objCont->getSelSize();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectLastSelection_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        C3DObject* it=App::ct->objCont->getLastSelection_object();
        if (it!=nullptr)
            retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetObjectSelection_internal(simInt* objectHandles)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            objectHandles[i]=App::ct->objCont->getSelID(i);
        return(App::ct->objCont->getSelSize());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleCollision_internal(simInt collisionObjectHandle)
{
    C_API_FUNCTION_DEBUG;

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
            colCnt=App::ct->collisions->handleAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CRegCollision* it=App::ct->collisions->getObject(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            if (it->handleCollision())
                colCnt++;
        }
        return(colCnt);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadCollision_internal(simInt collisionObjectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CRegCollision* it=App::ct->collisions->getObject(collisionObjectHandle);
        int retVal=it->readCollision(nullptr);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleDistance_internal(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    C_API_FUNCTION_DEBUG;

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
            d=App::ct->distances->handleAllDistances(distanceObjectHandle==sim_handle_all_except_explicit); // implicit handling
        else
        { // explicit handling
            CRegDist* it=App::ct->distances->getObject(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadDistance_internal(simInt distanceObjectHandle,simFloat* smallestDistance)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
            return(-1);
        float d;
        CRegDist* it=App::ct->distances->getObject(distanceObjectHandle);
        d=it->readDistance();
        if (d>=0.0f)
        {
            smallestDistance[0]=d;
            return(1);
        }
        smallestDistance[0]=SIM_MAX_FLOAT; // new for V3.3.2 rev2
        return(0); // from -1 to 0 for V3.3.2 rev2
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleMill_internal(simInt millHandle,simFloat* removedSurfaceAndVolume)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    { // writing/reading of cut data is protected by another mutex!
        if ( (millHandle!=sim_handle_all)&&(millHandle!=sim_handle_all_except_explicit) )
        {
            if (!isMill(__func__,millHandle))
            {
                return(-1);
            }
        }
        int totalCutCount=0;
        float totalSurface=0.0f;
        float totalVolume=0.0f;
        for (int i=0;i<int(App::ct->objCont->millList.size());i++)
        {
            float surface=0.0f;
            float volume=0.0f;
            CMill* it=(CMill*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->millList[i]);
            if (millHandle>=0)
            { // explicit handling
                it=(CMill*)App::ct->objCont->getObjectFromHandle(millHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                totalCutCount+=it->handleMill(false,surface,volume,false);
            }
            else
                totalCutCount+=it->handleMill(millHandle==sim_handle_all_except_explicit,surface,volume,false);
            totalSurface+=surface;
            totalVolume+=volume;
            if (millHandle>=0)
                break;
        }
        if (removedSurfaceAndVolume!=nullptr)
        {
            removedSurfaceAndVolume[0]=totalSurface;
            removedSurfaceAndVolume[1]=totalVolume;
        }
        return(totalCutCount);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



simInt simHandleProximitySensor_internal(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (sensorHandle>=0)
        { // handle just one sensor (this is explicit handling)
            CProxSensor* it=App::ct->objCont->getProximitySensor(sensorHandle);
            if (it==nullptr)
            {
                return(-1);
            }
            else
            {
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
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
            for (int i=0;i<int(App::ct->objCont->proximitySensorList.size());i++)
            {
                int detectedObj;
                C3Vector detectedSurf;
                CProxSensor* it=(CProxSensor*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->proximitySensorList[i]);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simReadProximitySensor_internal(simInt sensorHandle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isSensor(__func__,sensorHandle))
            return(-1);
        CProxSensor* it=App::ct->objCont->getProximitySensor(sensorHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simHandleIkGroup_internal(simInt ikGroupHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (ikGroupHandle!=sim_handle_all)&&(ikGroupHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesIKGroupExist(__func__,ikGroupHandle))
            {
                return(-1);
            }
        }
        int returnValue=0;
        if (ikGroupHandle<0)
            returnValue=App::ct->ikGroups->computeAllIkGroups(ikGroupHandle==sim_handle_all_except_explicit);
        else
        { // explicit handling
            CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            returnValue=it->computeGroupIk(false);
        }
        return(returnValue);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckIkGroup_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat* jointValues,const simInt* jointOptions)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (!it->getExplicitHandling())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
            return(-1);
        }
        // Check if the handles are valid:
        bool ok=true;
        for (int i=0;i<jointCnt;i++)
        {
            if (App::ct->objCont->getJoint(jointHandles[i])==nullptr)
                ok=false;
        }
        if (!ok)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLES);
            return(-1);
        }
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
        {
            CJoint* aj=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
            sceneJoints.push_back(aj);
            initSceneJointValues.push_back(aj->getPosition());
            initSceneJointModes.push_back(aj->getJointMode());
        }

        it->setAllInvolvedJointsToPassiveMode();
        // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
        std::vector<bool> enabledElements;
        for (int i=0;i<int(it->ikElements.size());i++)
            enabledElements.push_back(it->ikElements[i]->getActive());

        // Set the correct mode for the joints involved:
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(jointHandles[i]);
            if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                joint->setJointMode(sim_jointmode_ik,false);
            else
                joint->setJointMode(sim_jointmode_dependent,false);
        }

        // do the calculation:
        int returnValue=it->computeGroupIk(false);

        // Prepare the return array with the computed joint values:
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(jointHandles[i]);
            jointValues[i]=joint->getPosition();
        }

        // Restore the IK element activation state:
        for (int i=0;i<int(it->ikElements.size());i++)
            it->ikElements[i]->setActive(enabledElements[i]);

        // Restore joint positions/modes:
        for (int i=0;i<int(sceneJoints.size());i++)
        {
            if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                sceneJoints[i]->setPosition(initSceneJointValues[i]);
            if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                sceneJoints[i]->setJointMode(initSceneJointModes[i],false);
        }

        return(returnValue);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleDynamics_internal(simFloat deltaTime)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->dynamicsContainer->handleDynamics(deltaTime);
        if ((!App::ct->dynamicsContainer->isWorldThere())&&App::ct->dynamicsContainer->getDynamicsEnabled())
        {
            App::ct->dynamicsContainer->markForWarningDisplay_physicsEngineNotSupported();
            return(0);
        }
        return(CPluginContainer::dyn_getDynamicStepDivider());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptHandle_internal(const simChar* targetAtScriptName)
{
    C_API_FUNCTION_DEBUG;

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



    std::string scriptNameAdjusted=getCNameSuffixAdjustedName(scriptName.c_str());
    enableCNameSuffixAdjustment();

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=nullptr;
        if ((targetName.length()==0)||(targetName.compare("child")==0)||(targetName.compare("main")==0))
        {
            if (scriptNameAdjusted.length()==0)
                it=App::ct->luaScriptContainer->getMainScript();
            else
            {
                C3DObject* obj=App::ct->objCont->getObjectFromName(scriptNameAdjusted.c_str());
                if (obj!=nullptr)
                    it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(obj->getObjectHandle());
            }
        }
        if (targetName.compare("customization")==0)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromName(scriptNameAdjusted.c_str());
            if (obj!=nullptr)
                it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(obj->getObjectHandle());
        }
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        if (it->getFlaggedForDestruction())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_WAS_DESTROYED);
            return(-1);
        }
        int retVal=it->getScriptID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleMainScript_internal()
{
    C_API_FUNCTION_DEBUG;
    if (!isSimulatorInitialized(__func__))
        return(-1);

    int retVal=0;

    // Plugins:
    int data[4]={0,0,0,0};
    int rtVal[4]={-1,-1,-1,-1};
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_mainscriptabouttobecalled,data,nullptr,rtVal);
    delete[] (char*)returnVal;

    // Customization scripts:
    int res=0;
    CInterfaceStack outStack;
    App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_beforemainscript,nullptr,&outStack,&res);
    bool cs=(res!=1);

    // Add-on scripts:
    bool as=App::ct->addOnScriptContainer->handleAddOnScriptExecution_beforeMainScript();

    // Sandbox script:
    bool ss=true;
    if (App::ct->sandboxScript!=nullptr)
        ss=App::ct->sandboxScript->runSandboxScript_beforeMainScript();

    if ( ( (rtVal[0]==-1)&&cs&&as&&ss )||App::ct->simulation->didStopRequestCounterChangeSinceSimulationStart() )
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getMainScript();
        if (it!=nullptr)
        {
            App::ct->calcInfo->simulationPassStart();
            retVal=it->runMainScript(-1,nullptr,nullptr,nullptr);
            App::ct->calcInfo->simulationPassEnd();
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

    // Handle add-on execution (during non-simulation, it happens elsewhere!):
    App::ct->addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_aos_run,nullptr,nullptr);

    return(retVal);
}

simInt simResetScript_internal(simInt scriptHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle==sim_handle_all)
        {
            App::ct->luaScriptContainer->killAllSimulationLuaStates();
            return(1);
        }
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        it->killLuaState();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetScriptText_internal(simInt scriptHandle,const simChar* scriptText)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptHandle,nullptr,true);
#endif
        it->setScriptText(scriptText);
        if ( (it->getScriptType()!=sim_scripttype_childscript)||(!it->getThreadedExecution())||App::ct->simulation->isSimulationStopped() )
            it->killLuaState();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

const simChar* simGetScriptText_internal(simInt scriptHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::ct->environment->getSceneLocked())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
            return(nullptr);
        }
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetScript_internal(simInt index)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (index<0)||(index>=int(App::ct->luaScriptContainer->allScripts.size())) )
        {
            return(-1);
        }
        CLuaScriptObject* it=App::ct->luaScriptContainer->allScripts[index];
        int retVal=it->getScriptID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptAssociatedWithObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objectHandle);
        if (it==nullptr)
            return(-1);
        int retVal=it->getScriptID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetCustomizationScriptAssociatedWithObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objectHandle);
        if (it==nullptr)
        {
            return(-1);
        }
        int retVal=it->getScriptID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectAssociatedWithScript_internal(simInt scriptHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if (it->getScriptType()==sim_scripttype_childscript)
                retVal=it->getObjectIDThatScriptIsAttachedTo_child();
        if (it->getScriptType()==sim_scripttype_customizationscript)
                retVal=it->getObjectIDThatScriptIsAttachedTo_customization();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetScriptName_internal(simInt scriptHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        std::string name(it->getScriptPseudoName());
        char* retVal=new char[name.length()+1];
        for (int i=0;i<int(name.length());i++)
            retVal[i]=name[i];
        retVal[name.length()]=0;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}


simInt simGetScriptProperty_internal(simInt scriptHandle,simInt* scriptProperty,simInt* associatedObjectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        scriptProperty[0]=it->getScriptType();
        associatedObjectHandle[0]=it->getObjectIDThatScriptIsAttachedTo_child();
        if (it->getThreadedExecution())
            scriptProperty[0]|=sim_scripttype_threaded;
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAssociateScriptWithObject_internal(simInt scriptHandle,simInt associatedObjectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1;
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
        if (it!=nullptr)
        {
            if ( (it->getScriptType()==sim_scripttype_childscript)||(it->getScriptType()==sim_scripttype_customizationscript) )
            {
                if (associatedObjectHandle==-1)
                { // remove association
                    it->setObjectIDThatScriptIsAttachedTo(-1);
                    App::setLightDialogRefreshFlag();
                    retVal=1;
                }
                else
                { // set association
                    if (doesObjectExist(__func__,associatedObjectHandle))
                    { // object does exist
                        if (it->getObjectIDThatScriptIsAttachedTo()==-1)
                        { // script not yet associated
                            if (it->getScriptType()==sim_scripttype_childscript)
                                it->setObjectIDThatScriptIsAttachedTo(associatedObjectHandle);
                            App::setLightDialogRefreshFlag();
                            retVal=1;
                        }
                    }
                }
            }
            else
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_NOT_CHILD_OR_CUSTOMIZATION_SCRIPT);
        }
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);

}

simInt simAddScript_internal(simInt scriptProperty)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int scriptType=scriptProperty;
        if (scriptProperty&sim_scripttype_threaded)
            scriptType=scriptProperty-sim_scripttype_threaded;
        CLuaScriptObject* it=new CLuaScriptObject(scriptType);
        if (scriptProperty&sim_scripttype_threaded)
        {
            it->setThreadedExecution(true);
            it->setExecuteJustOnce(true);
        }
        int retVal=App::ct->luaScriptContainer->insertScript(it);
        App::setFullDialogRefreshFlag();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveScript_internal(simInt scriptHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle==sim_handle_all)
        { // We wanna remove all scripts!
            if (!App::ct->simulation->isSimulationStopped())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
                return(-1);
            }
            App::ct->luaScriptContainer->removeAllScripts();
            App::setFullDialogRefreshFlag();
            return(1);
        }
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        App::ct->luaScriptContainer->removeScript_safe(scriptHandle);
        App::setFullDialogRefreshFlag();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetCollisionHandle_internal(const simChar* collisionObjectName)
{
    C_API_FUNCTION_DEBUG;

    size_t silentErrorPos=std::string(collisionObjectName).find("@silentError");
    std::string nm(collisionObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string collisionObjectNameAdjusted=getCNameSuffixAdjustedName(nm.c_str());
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CRegCollision* it=App::ct->collisions->getObject(collisionObjectNameAdjusted);
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COLLISION_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDistanceHandle_internal(const simChar* distanceObjectName)
{
    C_API_FUNCTION_DEBUG;

    size_t silentErrorPos=std::string(distanceObjectName).find("@silentError");
    std::string nm(distanceObjectName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string distanceObjectNameAdjusted=getCNameSuffixAdjustedName(nm.c_str());
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CRegDist* it=App::ct->distances->getObject(distanceObjectNameAdjusted);
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_DISTANCE_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetIkGroupHandle_internal(const simChar* ikGroupName)
{
    C_API_FUNCTION_DEBUG;

    size_t silentErrorPos=std::string(ikGroupName).find("@silentError");
    std::string nm(ikGroupName);
    if (silentErrorPos!=std::string::npos)
        nm.erase(nm.begin()+silentErrorPos,nm.end());

    std::string ikGroupNameAdjusted=getCNameSuffixAdjustedName(nm.c_str());
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupNameAdjusted);
        if (it==nullptr)
        {
            if (silentErrorPos==std::string::npos)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetIkGroupProperties_internal(simInt ikGroupHandle,simInt resolutionMethod,simInt maxIterations,simFloat damping,void* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        it->setCalculationMethod(resolutionMethod);
        it->setMaxIterations(maxIterations);
        it->setDlsFactor(damping);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetIkElementProperties_internal(simInt ikGroupHandle,simInt tipDummyHandle,simInt constraints,const simFloat* precision,const simFloat* weight,void* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        if (!isDummy(__func__,tipDummyHandle))
        {
            return(-1);
        }
        CikEl* el=it->getIkElementWithTooltipID(tipDummyHandle);
        if (el==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simResetCollision_internal(simInt collisionObjectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (collisionObjectHandle!=sim_handle_all)&&(collisionObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesCollisionObjectExist(__func__,collisionObjectHandle))
            {
                return(-1);
            }
        }
        if (collisionObjectHandle<0)
            App::ct->collisions->resetAllCollisions(collisionObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CRegCollision* it=App::ct->collisions->getObject(collisionObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearCollisionResult();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetDistance_internal(simInt distanceObjectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (distanceObjectHandle!=sim_handle_all)&&(distanceObjectHandle!=sim_handle_all_except_explicit) )
        {
            if (!doesDistanceObjectExist(__func__,distanceObjectHandle))
            {
                return(-1);
            }
        }
        if (distanceObjectHandle<0)
            App::ct->distances->resetAllDistances(distanceObjectHandle==sim_handle_all_except_explicit);
        else
        { // Explicit handling
            CRegDist* it=App::ct->distances->getObject(distanceObjectHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->clearDistanceResult();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetProximitySensor_internal(simInt sensorHandle)
{
    C_API_FUNCTION_DEBUG;

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
        for (int i=0;i<int(App::ct->objCont->proximitySensorList.size());i++)
        {
            CProxSensor* it=(CProxSensor*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->proximitySensorList[i]);
            if (sensorHandle>=0)
            { // Explicit handling
                it=(CProxSensor*)App::ct->objCont->getObjectFromHandle(sensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetMill_internal(simInt millHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ( (millHandle!=sim_handle_all)&&(millHandle!=sim_handle_all_except_explicit) )
        {
            if (!isMill(__func__,millHandle))
            {
                return(-1);
            }
        }
        for (int i=0;i<int(App::ct->objCont->millList.size());i++)
        {
            CMill* it=(CMill*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->millList[i]);
            if (millHandle>=0)
            { // Explicit handling
                it=(CMill*)App::ct->objCont->getObjectFromHandle(millHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                it->resetMill(false);
                break;
            }
            else
                it->resetMill(millHandle==sim_handle_all_except_explicit);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}


simInt simCheckProximitySensor_internal(simInt sensorHandle,simInt entityHandle,simFloat* detectedPoint)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(-1);
        }
        if (!isSensor(__func__,sensorHandle))
        {
            return(-1);
        }
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
        {
            return(-1);
        }
        int retVal=simCheckProximitySensorEx_internal(sensorHandle,entityHandle,3,SIM_MAX_FLOAT,0,detectedPoint,nullptr,nullptr);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckProximitySensorEx_internal(simInt sensorHandle,simInt entityHandle,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simInt* detectedObjectHandle,simFloat* normalVector)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(-1);
        }
        if (!isSensor(__func__,sensorHandle))
        {
            return(-1);
        }
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
        {
            return(-1);
        }
        if (entityHandle==sim_handle_all)
            entityHandle=-1;

        if (!App::ct->mainSettings->proximitySensorsEnabled)
        {
            return(0);
        }

        bool frontFace=SIM_IS_BIT_SET(detectionMode,0);
        bool backFace=SIM_IS_BIT_SET(detectionMode,1);
        bool fastDetection=SIM_IS_BIT_SET(detectionMode,2);
        bool limitedAngle=SIM_IS_BIT_SET(detectionMode,3);
        bool checkForOcclusions=SIM_IS_BIT_SET(detectionMode,4);
        if (!(frontFace||backFace))
            frontFace=true;
        if (detectionThreshold<0.0f)
            detectionThreshold=0.0f;
        tt::limitValue(0.0f,piValD2_f,maxAngle);
        int detectedObj;
        C3Vector dPoint;
        float minThreshold=-1.0f;
        CProxSensor* it=App::ct->objCont->getProximitySensor(sensorHandle);
        if ( (it!=nullptr)&&(it->convexVolume->getSmallestDistanceEnabled()) )
            minThreshold=it->convexVolume->getSmallestDistanceAllowed();
        C3Vector normV;
        bool returnValue;
        returnValue=CProxSensorRoutine::detectEntity(sensorHandle,entityHandle,!fastDetection,limitedAngle,maxAngle,dPoint,detectionThreshold,frontFace,backFace,detectedObj,minThreshold,normV,true,checkForOcclusions);

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckProximitySensorEx2_internal(simInt sensorHandle,simFloat* vertexPointer,simInt itemType,simInt itemCount,simInt detectionMode,simFloat detectionThreshold,simFloat maxAngle,simFloat* detectedPoint,simFloat* normalVector)
{
    C_API_FUNCTION_DEBUG;

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

        if (!App::ct->mainSettings->proximitySensorsEnabled)
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
        CProxSensor* it=App::ct->objCont->getProximitySensor(sensorHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLoadModule_internal(const simChar* filenameAndPath,const simChar* pluginName)
{ // -3: could not load, -2: missing entry points, -1: could not initialize. 0=< : handle of the plugin
  // we cannot lock/unlock, because this function might trigger another thread (GUI) that itself will initialize the plugin and call sim-functions --> forever locked!!
    C_API_FUNCTION_DEBUG;
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=PLUGIN_LOAD_AND_START_PLUGUITHREADCMD;
    cmdIn.stringParams.push_back(filenameAndPath);
    cmdIn.stringParams.push_back(pluginName);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    int handle=cmdOut.intParams[0];
    return(handle);
}

simInt simUnloadModule_internal(simInt pluginhandle)
{ // we cannot lock/unlock, because this function might trigger another thread (GUI) that itself will initialize the plugin and call sim-functions --> forever locked!!
    C_API_FUNCTION_DEBUG;
    int retVal=0;
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD;
    cmdIn.intParams.push_back(pluginhandle);
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    if (cmdOut.boolParams[0])
        retVal=1;
    return(retVal);
}

simVoid* simSendModuleMessage_internal(simInt message,simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    C_API_FUNCTION_DEBUG;

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

simVoid* simBroadcastMessage_internal(simInt* auxiliaryData,simVoid* customData,simInt* replyData)
{
    C_API_FUNCTION_DEBUG;

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

simInt simRegisterScriptCallbackFunction_internal(const simChar* funcNameAtPluginName,const simChar* callTips,simVoid(*callBack)(struct SScriptCallBack* cb))
{
    C_API_FUNCTION_DEBUG;

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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MISSING_PLUGIN_NAME);
            return(-1);
        }

        bool retVal=1;
        if (App::ct->luaCustomFuncAndVarContainer->removeCustomFunction(funcNameAtPluginName))
            retVal=0;// that function already existed. We remove it and replace it!
        CLuaCustomFunction* newFunction=new CLuaCustomFunction(funcNameAtPluginName,callTips,callBack);
        if (!App::ct->luaCustomFuncAndVarContainer->insertCustomFunction(newFunction))
        {
            delete newFunction;
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRegisterScriptVariable_internal(const simChar* varNameAtPluginName,const simChar* varValue,simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        bool retVal=1;
        if (App::ct->luaCustomFuncAndVarContainer->removeCustomVariable(varNameAtPluginName))
            retVal=0;// that variable already existed. We remove it and replace it!
        if (!App::ct->luaCustomFuncAndVarContainer->insertCustomVariable(varNameAtPluginName,varValue,stackHandle))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CUSTOM_LUA_VAR_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simCreateBuffer_internal(simInt size)
{
    C_API_FUNCTION_DEBUG;

    simChar* retVal=new char[size];
    return(retVal);
}

simInt simReleaseBuffer_internal(const simChar* buffer)
{
    C_API_FUNCTION_DEBUG;

    delete[] buffer;
    return(1);
}

simInt simCheckCollision_internal(simInt entity1Handle,simInt entity2Handle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (!doesEntityExist(__func__,entity1Handle))||
            ((entity2Handle!=sim_handle_all)&&(!doesEntityExist(__func__,entity2Handle))))
                return(-1);
        if (entity2Handle==sim_handle_all)
            entity2Handle=-1;

        if (!App::ct->mainSettings->collisionDetectionEnabled)
            return(0);
        bool returnValue=CCollisionRoutine::doEntitiesCollide(entity1Handle,entity2Handle,nullptr,true,true,nullptr);
        return(returnValue);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckCollisionEx_internal(simInt entity1Handle,simInt entity2Handle,simFloat** intersectionSegments)
{
    C_API_FUNCTION_DEBUG;

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

        if (!App::ct->mainSettings->collisionDetectionEnabled)
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCheckDistance_internal(simInt entity1Handle,simInt entity2Handle,simFloat threshold,simFloat* distanceData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (!doesEntityExist(__func__,entity1Handle))||
            ((entity2Handle!=sim_handle_all)&&(!doesEntityExist(__func__,entity2Handle))) )
        {
            return(-1);
        }
        if (entity2Handle==sim_handle_all)
            entity2Handle=-1;

        if (!App::ct->mainSettings->distanceCalculationEnabled)
        {
            return(0);
        }

        int buffer[4];
        App::ct->cacheData->getCacheDataDist(entity1Handle,entity2Handle,buffer);
        if (threshold<=0.0f)
            threshold=SIM_MAX_FLOAT;
        bool result=CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1Handle,entity2Handle,threshold,distanceData,buffer,buffer+2,true,true);
        App::ct->cacheData->setCacheDataDist(entity1Handle,entity2Handle,buffer);
        if (result)
        {
            return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetConfigurationTree_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (objectHandle!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(nullptr);
            }
        }
        if (objectHandle==sim_handle_all)
            objectHandle=-1;
        char* retVal=App::ct->confContainer->getConfigurationTree(objectHandle);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetConfigurationTree_internal(const simChar* data)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::ct->confContainer->setConfigurationTree(data);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simGetObjectConfiguration_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(nullptr);
        }
        char* retVal=App::ct->confContainer->getObjectConfiguration(objectHandle);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectConfiguration_internal(const simChar* data)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::ct->confContainer->setObjectConfiguration(data);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAdvanceSimulationByOneStep_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::ct->simulation->isSimulationRunning())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        App::ct->simulation->advanceSimulationByOneStep();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSimulationTimeStep_internal(simFloat timeStep)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        App::ct->simulation->setSimulationTimeStep_raw_ns(quint64(timeStep*1000000.0f));
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat simGetSimulationTimeStep_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1.0f);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        float retVal=float(App::ct->simulation->getSimulationTimeStep_speedModified_ns())/1000000.0f;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1.0f);
}

simInt simGetRealTimeSimulation_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::ct->simulation->getRealTimeSimulation())
        {
            return(1);
        }
        else
        {
            return(0);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAdjustRealTimeTimer_internal(simInt instanceIndex,simFloat deltaTime)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->simulation->adjustRealTimeTimer_ns(quint64(deltaTime*1000000.0f));
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simIsRealTimeSimulationStepNeeded_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::ct->simulation->isSimulationRunning())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        if (!App::ct->simulation->getRealTimeSimulation())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_REAL_TIME);
            return(-1);
        }
        if (App::ct->simulation->isRealTimeCalculationStepNeeded())
        {
            return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetSimulationPassesPerRenderingPass_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::ct->simulation->getSimulationPassesPerRendering_speedModified();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetSimulationPassesPerRenderingPass_internal(int p)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        p=tt::getLimitedInt(1,512,p);
        App::ct->simulation->setSimulationPassesPerRendering_raw(p);
        return(App::ct->simulation->getSimulationPassesPerRendering_raw());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simStartSimulation_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!App::ct->simulation->isSimulationRunning())
        {
            App::ct->simulation->startOrResumeSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simStopSimulation_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::ct->simulation->isSimulationStopped())
        {
            App::ct->simulation->incrementStopRequestCounter();
            App::ct->simulation->stopSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simPauseSimulation_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationRunning())
        {
            App::ct->simulation->pauseSimulation();
            return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddStatusbarMessage_internal(const simChar* message)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        /*
#ifdef SIM_WITH_GUI
        if (App::mainWindow==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MAIN_WINDOW_NOT_INITIALIZED);
            return(-1);
        }
#endif
*/
        if (message!=nullptr)
        {
            bool scriptErrorMsg=(std::string(message).compare(0,18,"Lua runtime error:")==0);
            App::addStatusbarMessage(message,scriptErrorMsg);
            if (scriptErrorMsg)
            { // this is to intercept the xpcall error message generated in a threaded child script, and to flash the status bar
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=FLASH_STATUSBAR_UITHREADCMD;
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            }
        }
        else
            App::clearStatusbar();

        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddModuleMenuEntry_internal(const simChar* entryLabel,simInt itemCount,simInt* itemHandles)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            std::vector<int> commandIDs;
            if (App::mainWindow->customMenuBarItemContainer->addMenuBarItem(entryLabel,itemCount,commandIDs))
            {
                for (unsigned int i=0;i<commandIDs.size();i++)
                    itemHandles[i]=commandIDs[i];
                App::mainWindow->createDefaultMenuBar();
                return(1);
            }
        }
        else
#endif
            return(1); // in headless mode we fake success
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetModuleMenuItemState_internal(simInt itemHandle,simInt state,const simChar* label)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if (App::mainWindow->customMenuBarItemContainer->setItemState(itemHandle,(state&2)!=0,(state&1)!=0,label))
                return(1);
        }
        else
#endif
            return(1); // in headless mode we fake success
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ITEM_HANDLE);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleGraph_internal(simInt graphHandle,simFloat simulationTime)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (graphHandle!=sim_handle_all)&&(graphHandle!=sim_handle_all_except_explicit) )
        {
            if (!isGraph(__func__,graphHandle))
            {
                return(-1);
            }
        }
        if (graphHandle<0)
            CGraphingRoutines::handleAllGraphs(graphHandle==sim_handle_all_except_explicit,simulationTime);
        else
        { // explicit handling
            CGraph* it=App::ct->objCont->getGraph(graphHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->addNextPoint(simulationTime);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetGraph_internal(simInt graphHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (graphHandle!=sim_handle_all)&&(graphHandle!=sim_handle_all_except_explicit) )
        {
            if (!isGraph(__func__,graphHandle))
            {
                return(-1);
            }
        }
        if (graphHandle<0)
            CGraphingRoutines::resetAllGraphs(graphHandle==sim_handle_all_except_explicit);
        else
        { // explicit handling
            CGraph* it=App::ct->objCont->getGraph(graphHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetGraph();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetModuleName_internal(simInt index,simUChar* moduleVersion)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromIndex(index);
        if (plug==nullptr)
        {
            return(nullptr);
        }
        char* name=new char[plug->name.length()+1];
        for (int i=0;i<int(plug->name.length());i++)
            name[i]=plug->name[i];
        name[plug->name.length()]=0;
        if (moduleVersion!=nullptr)
            moduleVersion[0]=plug->pluginVersion;
        return(name);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simGetScriptSimulationParameter_internal(simInt scriptHandle,const simChar* parameterName,simInt* parameterLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
        if ((scriptHandle!=sim_handle_main_script)&&(scriptHandle!=sim_handle_all))
        { // make sure we have indicated only a main or child script:
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
                return(nullptr);
            }
            if ( (it->getScriptType()!=sim_scripttype_mainscript)&&(it->getScriptType()!=sim_scripttype_childscript) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SPECIFIED_SCRIPT_IS_NOT_MAIN_OR_CHILD);
                return(nullptr);
            }
        }
        std::string parameterValue;
        int retVal=App::ct->luaScriptContainer->getScriptSimulationParameter_mainAndChildScriptsOnly(scriptHandle,parameterName,parameterValue);
        if (retVal!=0)
        {
            char* retVal=new char[parameterValue.length()+1];
            for (int i=0;i<int(parameterValue.length());i++)
                retVal[i]=parameterValue[i];
            retVal[parameterValue.length()]=0;
            parameterLength[0]=(int)parameterValue.length();
            return(retVal);
        }
        return(nullptr);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetScriptSimulationParameter_internal(simInt scriptHandle,const simChar* parameterName,const simChar* parameterValue,simInt parameterLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptHandle);
        if ((scriptHandle!=sim_handle_main_script)&&(scriptHandle!=sim_handle_all))
        { // make sure we have indicated only a main or child script:
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
                return(-1);
            }
            if ( (it->getScriptType()!=sim_scripttype_mainscript)&&(it->getScriptType()!=sim_scripttype_childscript) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SPECIFIED_SCRIPT_IS_NOT_MAIN_OR_CHILD);
                return(-1);
            }
        }
        int retVal=App::ct->luaScriptContainer->setScriptSimulationParameter_mainAndChildScriptsOnly(scriptHandle,parameterName,parameterValue,parameterLength);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDisplayDialog_internal(const simChar* titleText,const simChar* mainText,simInt dialogType,const simChar* initialText,const simFloat* titleColors,const simFloat* dialogColors,simInt* elementHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=new CInterfaceStack();
        stack->pushStringOntoStack(titleText,0);
        stack->pushStringOntoStack(mainText,0);
        stack->pushNumberOntoStack(dialogType);
        stack->pushBoolOntoStack(false);
        if (initialText!=nullptr)
            stack->pushStringOntoStack(initialText,0);
        int stackId=App::ct->interfaceStackContainer->addStack(stack);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.displayDialog",stackId);
        double d;
        stack->getStackNumberValue(d);
        App::ct->interfaceStackContainer->destroyStack(stackId);
        int retVal=int(d);
        return(retVal);
#else
        return(1);
#endif
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetDialogResult_internal(simInt genericDialogHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=new CInterfaceStack();
        stack->pushNumberOntoStack(genericDialogHandle);
        int stackId=App::ct->interfaceStackContainer->addStack(stack);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogResult",stackId);
        double d;
        stack->getStackNumberValue(d);
        App::ct->interfaceStackContainer->destroyStack(stackId);
        int retVal=int(d);
        return(retVal);
#else
        return(sim_dlgret_cancel);
#endif
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetDialogInput_internal(simInt genericDialogHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string tmp;
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=new CInterfaceStack();
        stack->pushNumberOntoStack(genericDialogHandle);
        int stackId=App::ct->interfaceStackContainer->addStack(stack);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.getDialogInput",stackId);
        bool r=stack->getStackStringValue(tmp);
        App::ct->interfaceStackContainer->destroyStack(stackId);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simEndDialog_internal(simInt genericDialogHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CInterfaceStack* stack=new CInterfaceStack();
        stack->pushNumberOntoStack(genericDialogHandle);
        int stackId=App::ct->interfaceStackContainer->addStack(stack);
        simCallScriptFunctionEx_internal(sim_scripttype_sandboxscript,"sim.endDialog",stackId);
        double d;
        stack->getStackNumberValue(d);
        App::ct->interfaceStackContainer->destroyStack(stackId);
        int retVal=int(d);
#endif
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetNavigationMode_internal(simInt navigationMode)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    App::setMouseMode(navigationMode);
    return(1);
}

simInt simGetNavigationMode_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    int retVal=App::getMouseMode();
    return(retVal);
}

simInt simSetPage_internal(simInt index)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        App::ct->pageContainer->setActivePage(index);
#endif
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPage_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
#ifdef SIM_WITH_GUI
        int retVal=App::ct->pageContainer->getActivePageIndex();
        return(retVal);
#else
        return(0);
#endif
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simChar* simGetScriptRawBuffer_internal(simInt scriptHandle,simInt bufferHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(nullptr);
        }
        char* buffer=(char*)it->getUserData(bufferHandle);
        if (buffer==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_BUFFER_INEXISTANT);
            return(nullptr);
        }
        return(buffer);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetScriptRawBuffer_internal(simInt scriptHandle,const simChar* buffer,simInt bufferSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        char* bufferCopy=new char[bufferSize];
        for (int i=0;i<bufferSize;i++)
            bufferCopy[i]=buffer[i];
        int retVal=it->setUserData(bufferCopy);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simReleaseScriptRawBuffer_internal(simInt scriptHandle,simInt bufferHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (scriptHandle==sim_handle_all)
        {
            for (int i=0;i<int(App::ct->luaScriptContainer->allScripts.size());i++)
            {
                CLuaScriptObject* it=App::ct->luaScriptContainer->allScripts[i];
                it->clearAllUserData();
            }
            return(1);
        }
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (scriptHandle==sim_handle_main_script)
            it=App::ct->luaScriptContainer->getMainScript();
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        if (bufferHandle==sim_handle_all)
        {
            it->clearAllUserData();
            return(1);
        }
        char* buffer=(char*)it->getUserData(bufferHandle);
        if (buffer==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_BUFFER_INEXISTANT);
            return(-1);
        }
        it->releaseUserData(bufferHandle);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCopyPasteObjects_internal(simInt* objectHandles,simInt objectCount,simInt options)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        // memorize current selection:
        std::vector<int> initSel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            initSel.push_back(App::ct->objCont->getSelID(i));
        // adjust the selection to copy:
        std::vector<int> selT;
        for (int i=0;i<objectCount;i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandles[i]);
            if (it!=nullptr)
            {
                if (((options&1)==0)||it->getModelBase())
                    selT.push_back(objectHandles[i]);
            }
        }
        // if we just wanna handle models, make sure no model has a parent that will also be copied:
        std::vector<int> sel;
        if (options&1)
        {
            for (size_t i=0;i<selT.size();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(selT[i]);
                bool ok=true;
                if (it->getParentObject()!=nullptr)
                {
                    for (size_t j=0;j<selT.size();j++)
                    {
                        C3DObject* it2=App::ct->objCont->getObjectFromHandle(selT[j]);
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
        App::ct->copyBuffer->memorizeBuffer();
        App::ct->copyBuffer->copyCurrentSelection(&sel,App::ct->environment->getSceneLocked());
        App::ct->objCont->deselectObjects();
        App::ct->copyBuffer->pasteBuffer(App::ct->environment->getSceneLocked());
        if (options&1)
            App::ct->objCont->removeFromSelectionAllExceptModelBase(true);
        int retVal=App::ct->objCont->getSelSize();
        for (int i=0;i<retVal;i++)
            objectHandles[i]=App::ct->objCont->getSelID(i);
        App::ct->copyBuffer->restoreBuffer();
        App::ct->copyBuffer->clearMemorizedBuffer();

        // Restore the initial selection:
        App::ct->objCont->deselectObjects();
        for (size_t i=0;i<initSel.size();i++)
            App::ct->objCont->addObjectToSelection(initSel[i]);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simScaleSelectedObjects_internal(simFloat scalingFactor,simBool scalePositionsToo)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
        {
            return(-1);
        }
        std::vector<int> sel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            sel.push_back(App::ct->objCont->getSelID(i));
        CSceneObjectOperations::scaleObjects(sel,scalingFactor,scalePositionsToo!=0);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simScaleObjects_internal(const simInt* objectHandles,simInt objectCount,simFloat scalingFactor,simBool scalePositionsToo)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simDeleteSelectedObjects_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
            return(-1);
        std::vector<int> sel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            sel.push_back(App::ct->objCont->getSelID(i));
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
        App::ct->objCont->deselectObjects();
        App::ct->objCont->eraseSeveralObjects(sel,true);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectUniqueIdentifier_internal(simInt objectHandle,simInt* uniqueIdentifier)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (objectHandle!=sim_handle_all)&&(!doesObjectExist(__func__,objectHandle)) )
        {
            return(-1);
        }
        int p=0;
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            if ( (it->getObjectHandle()==objectHandle)||(objectHandle==sim_handle_all) )
                uniqueIdentifier[p++]=it->getUniqueID();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetNameSuffix_internal(const simChar* name)
{
    C_API_FUNCTION_DEBUG;

    int suffixNumber;
    if (name!=nullptr)
        suffixNumber=tt::getNameSuffixNumber(name,true);
    else
        suffixNumber=getCNameSuffixNumber();
    return(suffixNumber);
}

simInt simSendData_internal(simInt targetID,simInt dataHeader,const simChar* dataName,const simChar* data,simInt dataLength,simInt antennaHandle,simFloat actionRadius,simFloat emissionAngle1,simFloat emissionAngle2,simFloat persistence)
{
    C_API_FUNCTION_DEBUG;

    if (App::ct->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (targetID!=0)&&(targetID!=sim_handle_all) )
        {
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(targetID);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_TARGET_HANDLE);
                return(-1);
            }
        }
        if (dataHeader<0)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA_HEADER);
            return(-1);
        }
        if (strlen(dataName)<1)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA_NAME);
            return(-1);
        }
        if (dataLength<1)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        if (antennaHandle!=sim_handle_default)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
                return(-1);
            }
        }
        actionRadius=tt::getLimitedFloat(0.0f,SIM_MAX_FLOAT,actionRadius);
        emissionAngle1=tt::getLimitedFloat(0.0f,piValue_f,emissionAngle1);
        emissionAngle2=tt::getLimitedFloat(0.0f,piValTimes2_f,emissionAngle2);
        persistence=tt::getLimitedFloat(0.0f,99999999999999.9f,persistence);
        if (persistence==0.0f)
            persistence=float(App::ct->simulation->getSimulationTimeStep_speedModified_ns())*1.5f/1000000.0f;
        std::string datN(dataName);
        App::ct->luaScriptContainer->broadcastDataContainer.broadcastData(0,targetID,dataHeader,datN,
                        float(App::ct->simulation->getSimulationTime_ns())/1000000.0f+persistence,actionRadius,antennaHandle,
                        emissionAngle1,emissionAngle2,data,dataLength);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReceiveData_internal(simInt dataHeader,const simChar* dataName,simInt antennaHandle,simInt index,simInt* dataLength,simInt* senderID,simInt* dataHeaderR,simChar** dataNameR)
{
    C_API_FUNCTION_DEBUG;

    if (App::ct->simulation->getSimulationState()==sim_simulation_stopped)
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA_NAME);
                return(nullptr);
            }
        }
        if (antennaHandle!=sim_handle_default)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(antennaHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ANTENNA_HANDLE);
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
        char* data0=App::ct->luaScriptContainer->broadcastDataContainer.receiveData(0,float(App::ct->simulation->getSimulationTime_ns())/1000000.0f,
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetGraphUserData_internal(simInt graphHandle,const simChar* dataStreamName,simFloat data)
{
    C_API_FUNCTION_DEBUG;

    if (!App::ct->simulation->isSimulationRunning())
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isGraph(__func__,graphHandle))
        {
            return(-1);
        }
        CGraph* it=App::ct->objCont->getGraph(graphHandle);
        CGraphData* stream=it->getGraphData(dataStreamName);
        if (stream==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA_STREAM);
            return(-1);
        }
        if (stream->getDataType()!=GRAPH_NOOBJECT_USER_DEFINED)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_DATA_STREAM_NOT_USER_DEFINED);
            return(-1);
        }
        stream->setUserData(data);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetNameSuffix_internal(simInt nameSuffixNumber)
{
    C_API_FUNCTION_DEBUG;

    if (nameSuffixNumber<-1)
        nameSuffixNumber=-1;
    setCNameSuffixNumber(nameSuffixNumber);
    return(1);
}

simInt simAddDrawingObject_internal(simInt objectType,simFloat size,simFloat duplicateTolerance,simInt parentObjectHandle,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (parentObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,parentObjectHandle))
            {
                return(-1);
            }
        }
        CDrawingObject* it=new CDrawingObject(objectType,size,duplicateTolerance,parentObjectHandle,maxItemCount,false);
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
        int retVal=App::ct->drawingCont->addObject(it);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveDrawingObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle==sim_handle_all)
            App::ct->drawingCont->removeAllObjects(false,true);
        else
        {
            CDrawingObject* it=App::ct->drawingCont->getObject(objectHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
                return(-1);
            }
            App::ct->drawingCont->removeObject(objectHandle);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddDrawingObjectItem_internal(simInt objectHandle,const simFloat* itemData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    { // protected with an additional mutex in CDrawingObject
        CDrawingObject* it=App::ct->drawingCont->getObject(objectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
            return(-1);
        }
        if (it->addItem(itemData))
            return(1);
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}
simInt simAddParticleObject_internal(simInt objectType,simFloat size,simFloat massOverVolume,const simVoid* params,simFloat lifeTime,simInt maxItemCount,const simFloat* ambient_diffuse,const simFloat* setToNULL,const simFloat* specular,const simFloat* emission)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::ct->simulation->isSimulationRunning())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        int retVal=CPluginContainer::dyn_addParticleObject(objectType,size,massOverVolume,params,lifeTime,maxItemCount,ambient_diffuse,nullptr,specular,emission);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveParticleObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!App::ct->simulation->isSimulationRunning())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        int retVal=-1;
        if (CPluginContainer::dyn_removeParticleObject(objectHandle)!=0)
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddParticleObjectItem_internal(simInt objectHandle,const simFloat* itemData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    if (!App::ct->simulation->isSimulationRunning())
    {
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1;
        if (CPluginContainer::dyn_addParticleObjectItem(objectHandle,itemData,float(App::ct->simulation->getSimulationTime_ns())/1000000.0f)!=0)
            retVal=1;
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat simGetObjectSizeFactor_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1.0f);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1.0f);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);

        float retVal=it->getSizeFactor();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAnnounceSceneContentChange_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::ct->undoBufferContainer->announceChange())
            return(-1);
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetMilling_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            CShape* shape=nullptr;
            if (it->getObjectType()==sim_object_shape_type)
                shape=(CShape*)it;
            if (shape!=nullptr)
                shape->resetMilling();
            return(1);
        }
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            CShape* shape=nullptr;
            if (it->getObjectType()==sim_object_shape_type)
                shape=(CShape*)it;
            if (shape!=nullptr)
                shape->resetMilling();
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simApplyMilling_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (objectHandle!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            CShape* shape=App::ct->objCont->getShape(objectHandle);
            if (shape!=nullptr)
            {
                if (shape->applyMilling())
                {
                    App::ct->objCont->eraseObject(shape,true);
                    return(0);
                }
            }
            return(1);
        }
        std::vector<int> toErase;
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            CShape* shape=nullptr;
            if (it->getObjectType()==sim_object_shape_type)
                shape=(CShape*)it;
            if (shape!=nullptr)
            {
                if (shape->applyMilling())
                    toErase.push_back(shape->getObjectHandle());
            }
        }
        // Now erase what needs to be erased:
        App::ct->objCont->eraseSeveralObjects(toErase,true);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetIntegerSignal_internal(const simChar* signalName,simInt signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->signalContainer->setIntegerSignal(signalName,signalValue,false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetIntegerSignal_internal(const simChar* signalName,simInt* signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::ct->signalContainer->getIntegerSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearIntegerSignal_internal(const simChar* signalName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::ct->signalContainer->clearAllIntegerSignals(false);
        else
            retVal=App::ct->signalContainer->clearIntegerSignal(signalName);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetFloatSignal_internal(const simChar* signalName,simFloat signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->signalContainer->setFloatSignal(signalName,signalValue,false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetFloatSignal_internal(const simChar* signalName,simFloat* signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::ct->signalContainer->getFloatSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearFloatSignal_internal(const simChar* signalName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::ct->signalContainer->clearAllFloatSignals(false);
        else
            retVal=App::ct->signalContainer->clearFloatSignal(signalName);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetDoubleSignal_internal(const simChar* signalName,simDouble signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->signalContainer->setDoubleSignal(signalName,signalValue,false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDoubleSignal_internal(const simChar* signalName,simDouble* signalValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;

        if (App::ct->signalContainer->getDoubleSignal(signalName,signalValue[0]))
            retVal=1;

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearDoubleSignal_internal(const simChar* signalName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::ct->signalContainer->clearAllDoubleSignals(false);
        else
            retVal=App::ct->signalContainer->clearDoubleSignal(signalName);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetStringSignal_internal(const simChar* signalName,const simChar* signalValue,simInt stringLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->signalContainer->setStringSignal(signalName,std::string(signalValue,stringLength),false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStringSignal_internal(const simChar* signalName,simInt* stringLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::string sigVal;

        if (App::ct->signalContainer->getStringSignal(signalName,sigVal))
        {
            char* retVal=new char[sigVal.length()];
            for (unsigned int i=0;i<sigVal.length();i++)
                retVal[i]=sigVal[i];
            stringLength[0]=(int)sigVal.length();
            return(retVal);
        }

        return(nullptr); // signal does not exist
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simClearStringSignal_internal(const simChar* signalName)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;

        if (signalName==nullptr)
            retVal=App::ct->signalContainer->clearAllStringSignals(false);
        else
            retVal=App::ct->signalContainer->clearStringSignal(signalName);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetSignalName_internal(simInt signalIndex,simInt signalType)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (signalType!=0)&&(signalType!=1)&&(signalType!=2) )
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(nullptr);
        }
        std::string sigName;
        bool res=false;

        if (signalType==0)
            res=App::ct->signalContainer->getIntegerSignalNameAtIndex(signalIndex,sigName);
        if (signalType==1)
            res=App::ct->signalContainer->getFloatSignalNameAtIndex(signalIndex,sigName);
        if (signalType==2)
            res=App::ct->signalContainer->getStringSignalNameAtIndex(signalIndex,sigName);

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectProperty_internal(simInt objectHandle,simInt prop)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        it->setLocalObjectProperty(prop);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectProperty_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=it->getLocalObjectProperty();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectSpecialProperty_internal(simInt objectHandle,simInt prop)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        it->setLocalObjectSpecialProperty(prop);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectSpecialProperty_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=it->getLocalObjectSpecialProperty();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetModelProperty_internal(simInt objectHandle,simInt modelProperty)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if ((modelProperty&sim_modelproperty_not_model)!=0)
            it->setModelBase(false);
        else
        {
            if (!it->getModelBase())
                it->setModelBase(true);
            it->setLocalModelProperty(modelProperty);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetModelProperty_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal;
        if (it->getModelBase())
            retVal=it->getLocalModelProperty();
        else
            retVal=sim_modelproperty_not_model;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDataOnPath_internal(simInt pathHandle,simFloat relativeDistance,simInt dataType,simInt* intData,simFloat* floatData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(pathHandle);
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
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_EMPTY);
                    return(-1);
                }
            }
            else
            { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
                CSimplePathPoint* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
                if (ctrlPt!=nullptr)
                {
                    intData[0]=ctrlPt->getAuxFlags();
                    ctrlPt->getAuxChannels(floatData);
                    return(1);
                }
                else
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_CTRL_PT);
                    return(-1);
                }
            }
        }
        else
            return(-1);

    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetPositionOnPath_internal(simInt pathHandle,simFloat relativeDistance,simFloat* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(pathHandle);
        C7Vector tr;

        if (relativeDistance>-0.5f)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformationPart1()*tr;
                tr.X.copyTo(position);
                return(1);
            }
            else
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformationPart1()*tr;
                tr.X.copyTo(position);
                return(1);
            }
            else
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetOrientationOnPath_internal(simInt pathHandle,simFloat relativeDistance,simFloat* eulerAngles)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(pathHandle);
        C7Vector tr;
        if (relativeDistance>-0.5f)
        { // regular use of the function
            if (it->pathContainer->getTransformationOnBezierCurveAtNormalizedVirtualDistance(relativeDistance,tr))
            {
                tr=it->getCumulativeTransformationPart1()*tr;
                C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_EMPTY);
                return(-1);
            }
        }
        else
        { // We are working with indices pointing on ctrl points. index=-(relativeDistance+1.0)
            CSimplePathPoint* ctrlPt=it->pathContainer->getSimplePathPoint(int(-relativeDistance-0.5f));
            if (ctrlPt!=nullptr)
            {
                tr=ctrlPt->getTransformation();
                tr=it->getCumulativeTransformationPart1()*tr;
                C3Vector(tr.Q.getEulerAngles()).copyTo(eulerAngles);
                return(1);
            }
            else
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_CTRL_PT);
                return(-1);
            }
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetClosestPositionOnPath_internal(simInt pathHandle,simFloat* absolutePosition,simFloat* pathPosition)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,pathHandle))
            return(-1);
        if (!isPath(__func__,pathHandle))
            return(-1);
        CPath* it=App::ct->objCont->getPath(pathHandle);
        C3Vector p(absolutePosition);
        if (it->pathContainer->getPositionOnPathClosestTo(p,*pathPosition))
        {
            float pl=it->pathContainer->getBezierVirtualPathLength();
            if (pl!=0.0f)
                *pathPosition/=pl;
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_EMPTY);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadForceSensor_internal(simInt objectHandle,simFloat* forceVector,simFloat* torqueVector)
{
    C_API_FUNCTION_DEBUG;

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
        if (App::ct->simulation->isSimulationStopped())
        {
//            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(0);
        }
        CForceSensor* it=App::ct->objCont->getForceSensor(handle);
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

        if (it->getForceSensorIsBroken())
            retVal|=2;

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simBreakForceSensor_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        if (!isForceSensor(__func__,objectHandle))
        {
            return(-1);
        }
        if (App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_RUNNING);
            return(-1);
        }
        CForceSensor* it=App::ct->objCont->getForceSensor(objectHandle);
        it->setForceSensorIsBroken();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeVertex_internal(simInt shapeHandle,simInt groupElementIndex,simInt vertexIndex,simFloat* relativePosition)
{
    C_API_FUNCTION_DEBUG;

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
        if (App::ct->environment->getSceneLocked())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        CGeometric* cc=it->geomData->geomInfo->getShapeComponentAtIndex(groupElementIndex);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeTriangle_internal(simInt shapeHandle,simInt groupElementIndex,simInt triangleIndex,simInt* vertexIndices,simFloat* triangleNormals)
{
    C_API_FUNCTION_DEBUG;

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
        if (App::ct->environment->getSceneLocked())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        CGeometric* cc=it->geomData->geomInfo->getShapeComponentAtIndex(groupElementIndex);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetLightParameters_internal(simInt objectHandle,simFloat* setToNULL,simFloat* diffusePart,simFloat* specularPart)
{
    C_API_FUNCTION_DEBUG;

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
        if (!isLight(__func__,objectHandle))
        {
            return(-1);
        }
        CLight* it=App::ct->objCont->getLight(objectHandle);
        int retVal=0;
        if (it->getLightActive())
            retVal|=1;
        for (int i=0;i<3;i++)
        {
            if (setToNULL!=nullptr)
                setToNULL[0+i]=0.0f;
            if (diffusePart!=nullptr)
                diffusePart[0+i]=it->getColor(true)->colors[3+i];
            if (specularPart!=nullptr)
                specularPart[0+i]=it->getColor(true)->colors[6+i];
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetLightParameters_internal(simInt objectHandle,simInt state,const simFloat* setToNULL,const simFloat* diffusePart,const simFloat* specularPart)
{
    C_API_FUNCTION_DEBUG;

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
        if (!isLight(__func__,objectHandle))
        {
            return(-1);
        }
        CLight* it=App::ct->objCont->getLight(objectHandle);
        it->setLightActive(state&1);
        for (int i=0;i<3;i++)
        {
            if (diffusePart!=nullptr)
                it->getColor(true)->colors[3+i]=diffusePart[0+i];
            if (specularPart!=nullptr)
                it->getColor(true)->colors[6+i]=specularPart[0+i];
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetVelocity_internal(simInt shapeHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,shapeHandle))
            return(-1);
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        C3Vector lv(it->getDynamicLinearVelocity());
        C3Vector av(it->getDynamicAngularVelocity());
        if (linearVelocity!=nullptr)
            lv.copyTo(linearVelocity);
        if (angularVelocity!=nullptr)
            av.copyTo(angularVelocity);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectVelocity_internal(simInt objectHandle,simFloat* linearVelocity,simFloat* angularVelocity)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(handle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddForceAndTorque_internal(simInt shapeHandle,const simFloat* force,const simFloat* torque)
{
    C_API_FUNCTION_DEBUG;

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
        CShape* it=App::ct->objCont->getShape(handle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAddForce_internal(simInt shapeHandle,const simFloat* position,const simFloat* force)
{
    C_API_FUNCTION_DEBUG;

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
        CShape* it=App::ct->objCont->getShape(handle);
        C3Vector r(position);
        C3Vector f(force);
        C3Vector t(r^f);
        // f & t are relative to the shape's frame now. We have to make them absolute:
        C4Vector q(it->getCumulativeTransformationPart1().Q);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simSetExplicitHandling_internal(simInt generalObjectHandle,int explicitFlags)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (generalObjectHandle>=SIM_IDSTART_3DOBJECT)&&(generalObjectHandle<SIM_IDEND_3DOBJECT) )
        { // scene objects
            if (!doesObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            C3DObject* it=App::ct->objCont->getObjectFromHandle(generalObjectHandle);
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
                ((CPath*)it)->setExplicitHandling(explicitFlags&1);
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
        if ( (generalObjectHandle>=SIM_IDSTART_COLLISION)&&(generalObjectHandle<SIM_IDEND_COLLISION) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CRegCollision* it=App::ct->collisions->getObject(generalObjectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_DISTANCE)&&(generalObjectHandle<SIM_IDEND_DISTANCE) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CRegDist* it=App::ct->distances->getObject(generalObjectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_IKGROUP)&&(generalObjectHandle<SIM_IDEND_IKGROUP) )
        { // IK objects
            if (!doesIKGroupExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CikGroup* it=App::ct->ikGroups->getIkGroup(generalObjectHandle);
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_GCSOBJECT)&&(generalObjectHandle<SIM_IDEND_GCSOBJECT) )
        { // GCS objects
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(generalObjectHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MECHANISM_INEXISTANT);
                return(-1);
            }
            it->setExplicitHandling(explicitFlags&1);
            return(1);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_COLLECTION)&&(generalObjectHandle<SIM_IDEND_COLLECTION) )
        { // collection objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_2DELEMENT)&&(generalObjectHandle<SIM_IDEND_2DELEMENT) )
        { // UI objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_LUASCRIPT)&&(generalObjectHandle<SIM_IDEND_LUASCRIPT) )
        { // Script objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_PATHPLANNINGTASK)&&(generalObjectHandle<SIM_IDEND_PATHPLANNINGTASK) )
        { // Path planning objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_MOTIONPLANNINGTASK)&&(generalObjectHandle<SIM_IDEND_MOTIONPLANNINGTASK) )
        { // Motion planning objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_TEXTURE)&&(generalObjectHandle<SIM_IDEND_TEXTURE) )
        { // Texture objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_DYNMATERIAL_OLD)&&(generalObjectHandle<SIM_IDEND_DYNMATERIAL_OLD) )
        { // Dyn material objects

        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetExplicitHandling_internal(simInt generalObjectHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (generalObjectHandle>=SIM_IDSTART_3DOBJECT)&&(generalObjectHandle<SIM_IDEND_3DOBJECT) )
        { // scene objects
            if (!doesObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            C3DObject* it=App::ct->objCont->getObjectFromHandle(generalObjectHandle);
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
                bool exp=((CPath*)it)->getExplicitHandling();
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
        if ( (generalObjectHandle>=SIM_IDSTART_COLLISION)&&(generalObjectHandle<SIM_IDEND_COLLISION) )
        { // collision objects
            if (!doesCollisionObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CRegCollision* it=App::ct->collisions->getObject(generalObjectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_DISTANCE)&&(generalObjectHandle<SIM_IDEND_DISTANCE) )
        { // distance objects
            if (!doesDistanceObjectExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CRegDist* it=App::ct->distances->getObject(generalObjectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_IKGROUP)&&(generalObjectHandle<SIM_IDEND_IKGROUP) )
        { // IK objects
            if (!doesIKGroupExist(__func__,generalObjectHandle))
            {
                return(-1);
            }
            CikGroup* it=App::ct->ikGroups->getIkGroup(generalObjectHandle);
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_GCSOBJECT)&&(generalObjectHandle<SIM_IDEND_GCSOBJECT) )
        { // GCS objects
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(generalObjectHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MECHANISM_INEXISTANT);
                return(-1);
            }
            bool exp=it->getExplicitHandling();
            return(exp);
        }
        if ( (generalObjectHandle>=SIM_IDSTART_COLLECTION)&&(generalObjectHandle<SIM_IDEND_COLLECTION) )
        { // collection objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_2DELEMENT)&&(generalObjectHandle<SIM_IDEND_2DELEMENT) )
        { // UI objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_LUASCRIPT)&&(generalObjectHandle<SIM_IDEND_LUASCRIPT) )
        { // Script objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_PATHPLANNINGTASK)&&(generalObjectHandle<SIM_IDEND_PATHPLANNINGTASK) )
        { // Path planning objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_MOTIONPLANNINGTASK)&&(generalObjectHandle<SIM_IDEND_MOTIONPLANNINGTASK) )
        { // Motion planning objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_TEXTURE)&&(generalObjectHandle<SIM_IDEND_TEXTURE) )
        { // Texture objects

        }
        if ( (generalObjectHandle>=SIM_IDSTART_DYNMATERIAL_OLD)&&(generalObjectHandle<SIM_IDEND_DYNMATERIAL_OLD) )
        { // Dyn material objects

        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetLinkDummy_internal(simInt dummyHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CDummy* it=App::ct->objCont->getDummy(dummyHandle);
        int retVal=it->getLinkedDummyID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetLinkDummy_internal(simInt dummyHandle,simInt linkedDummyHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CDummy* it=App::ct->objCont->getDummy(dummyHandle);
        it->setLinkedDummyID(linkedDummyHandle,false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetShapeColor_internal(simInt shapeHandle,const simChar* colorName,simInt colorComponent,const simFloat* rgbData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (shapeHandle!=sim_handle_all)&&(!isShape(__func__,shapeHandle)) )
        {
            return(-1);
        }
        for (int i=0;i<int(App::ct->objCont->shapeList.size());i++)
        {
            CShape* it=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
            if ( (it->getObjectHandle()==shapeHandle)||(shapeHandle==sim_handle_all) )
                it->setColor(colorName,colorComponent,rgbData);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeColor_internal(simInt shapeHandle,const simChar* colorName,simInt colorComponent,simFloat* rgbData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        int retVal=0;
        if (it->getColor(colorName,colorComponent,rgbData))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetDynamicObject_internal(simInt objectHandle)
{
    C_API_FUNCTION_DEBUG;

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
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            if ( (it->getObjectHandle()==objectHandle)||(objectHandle==sim_handle_all) )
            {
                if (it->getObjectHandle()==objectHandle)
                {
                    if (handleFlags&sim_handleflag_model)
                    {
                        std::vector<C3DObject*> toExplore;
                        toExplore.push_back(it);
                        while (toExplore.size()!=0)
                        {
                            C3DObject* obj=toExplore[0];
                            toExplore.erase(toExplore.begin(),toExplore.begin()+1);
                            for (int i=0;i<int(obj->childList.size());i++)
                                toExplore.push_back(obj->childList[i]);
                            obj->setDynamicsFullRefreshFlag(true);
                        }
                    }
                    else
                        it->setDynamicsFullRefreshFlag(true);
                }
                else
                    it->setDynamicsFullRefreshFlag(true);
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetJointMode_internal(simInt jointHandle,simInt jointMode,simInt options)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(jointHandle);
        it->setJointMode(jointMode);
        it->setHybridFunctionality(options&1);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetJointMode_internal(simInt jointHandle,simInt* options)
{
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(jointHandle);
        int retVal=it->getJointMode();
        options[0]=0;
        if (it->getHybridFunctionality())
            options[0]|=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialOpen_internal(const simChar* portString,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handle=-1;
#ifdef SIM_WITH_SERIAL
        handle=App::ct->serialPortContainer->serialPortOpen(false,portString,baudRate);
#endif
        return(handle);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialClose_internal(simInt portHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
#ifdef SIM_WITH_SERIAL
        if (App::ct->serialPortContainer->serialPortClose(portHandle))
            retVal=1;
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSerialSend_internal(simInt portHandle,const simChar* data,simInt dataLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    std::string dat(data,data+dataLength);
    retVal=App::ct->serialPortContainer->serialPortSend(portHandle,dat);
    if (retVal==-1)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simSerialRead_internal(simInt portHandle,simChar* buffer,simInt dataLengthToRead)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    std::string data;
    data.resize(dataLengthToRead);
    retVal=App::ct->serialPortContainer->serialPortReceive(portHandle,data,dataLengthToRead);
    if (retVal>0)
    {
        for (int i=0;i<retVal;i++)
            buffer[i]=data[i];
    }
    if (retVal==-1)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simSerialCheck_internal(simInt portHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::ct->serialPortContainer->serialPortCheck(portHandle);
    if (retVal==-1)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PORT_HANDLE);
#endif
    return(retVal);
}

simInt simGetContactInfo_internal(simInt dynamicPass,simInt objectHandle,simInt index,simInt* objectHandles,simFloat* contactInfo)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::ct->dynamicsContainer->getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo)!=0)
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetThreadIsFree_internal(simBool freeMode)
{
    C_API_FUNCTION_DEBUG;

    if (VThread::isCurrentThreadTheMainSimulationThread())
    {

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CANNOT_BE_CALLED_FROM_MAIN_THREAD);
        return(-1);
    }
    if (CThreadPool::setThreadFreeMode(freeMode!=0))
        return(1);
    return(0);
}


simInt simTubeOpen_internal(simInt dataHeader,const simChar* dataName,simInt readBufferSize,simBool notUsedButKeepFalse)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal;
        retVal=App::ct->commTubeContainer->openTube(dataHeader,dataName,false,readBufferSize);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simTubeClose_internal(simInt tubeHandle)
{
    C_API_FUNCTION_DEBUG;

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
            if (App::ct->commTubeContainer->closeTube(tubeHandle))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simTubeWrite_internal(simInt tubeHandle,const simChar* data,simInt dataLength)
{
    C_API_FUNCTION_DEBUG;

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
            if (App::ct->commTubeContainer->writeToTube_copyBuffer(tubeHandle,data,dataLength))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simTubeRead_internal(simInt tubeHandle,simInt* dataLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal;
        retVal=App::ct->commTubeContainer->readFromTube_bufferNotCopied(tubeHandle,dataLength[0]);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simTubeStatus_internal(simInt tubeHandle,simInt* readPacketsCount,simInt* writePacketsCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int readP=0;
        int writeP=0;
        int retVal;
        retVal=App::ct->commTubeContainer->getTubeStatus(tubeHandle,readP,writeP);
        if (readPacketsCount!=nullptr)
            readPacketsCount[0]=readP;
        if (writePacketsCount!=nullptr)
            writePacketsCount[0]=writeP;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAuxiliaryConsoleOpen_internal(const simChar* title,simInt maxLines,simInt mode,const simInt* position,const simInt* size,const simFloat* textColor,const simFloat* backgroundColor)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAuxiliaryConsoleClose_internal(simInt consoleHandle)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAuxiliaryConsoleShow_internal(simInt consoleHandle,simBool showState)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simAuxiliaryConsolePrint_internal(simInt consoleHandle,const simChar* text)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simImportShape_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return(-1);
        }
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FILE_NOT_FOUND);
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
        App::ct->objCont->deselectObjects();
        return(h);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}


simInt simImportMesh_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat identicalVerticeTolerance,simFloat scalingFactor,simFloat*** vertices,simInt** verticesSizes,simInt*** indices,simInt** indicesSizes,simFloat*** reserved,simChar*** names)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!VFile::doesFileExist(pathAndFilename))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FILE_NOT_FOUND);
            return(-1);
        }
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simExportMesh_internal(simInt fileformat,const simChar* pathAndFilename,simInt options,simFloat scalingFactor,simInt elementCount,const simFloat** vertices,const simInt* verticesSizes,const simInt** indices,const simInt* indicesSizes,simFloat** reserved,const simChar** names)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (App::ct->environment->getSceneLocked())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCENE_LOCKED);
            return(-1);
        }
        if (!CPluginContainer::isAssimpPluginAvailable())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ASSIMP_PLUGIN_NOT_FOUND);
            return(-1);
        }
        if (handleVerSpec_simExportMesh())
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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_FILE_FORMAT);
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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        int op=0;
        CPluginContainer::assimp_exportMeshes(elementCount,vertices,verticesSizes,indices,indicesSizes,pathAndFilename,format.c_str(),scalingFactor,1,op);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simCreateMeshShape_internal(simInt options,simFloat shadingAngle,const simFloat* vertices,simInt verticesSize,const simInt* indices,simInt indicesSize,simFloat* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<float> vert(vertices,vertices+verticesSize);
        std::vector<int> ind(indices,indices+indicesSize);
        CGeomProxy* geom=new CGeomProxy(nullptr,vert,ind,nullptr,nullptr);
        CShape* shape=new CShape();
        shape->setLocalTransformation(geom->getCreationTransformation());
        geom->setCreationTransformation(C7Vector::identityTransformation);
        ((CGeometric*)geom->geomInfo)->setGouraudShadingAngle(shadingAngle);
        ((CGeometric*)geom->geomInfo)->setEdgeThresholdAngle(shadingAngle);
        geom->geomInfo->setLocalInertiaFrame(C7Vector::identityTransformation);
        shape->geomData=geom;
        shape->setCulling((options&1)!=0);
        shape->setVisibleEdges((options&2)!=0);
        App::ct->objCont->addObjectToScene(shape,false,true);
        return(shape->getObjectHandle());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetShapeMesh_internal(simInt shapeHandle,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize,simFloat** normals)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        std::vector<float> wvert;
        std::vector<int> wind;
        std::vector<float> wnorm;
        it->geomData->geomInfo->getCumulativeMeshes(wvert,&wind,&wnorm);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreatePureShape_internal(simInt primitiveType,simInt options,const simFloat* sizes,simFloat mass,const simInt* precision)
{ // options: bit: 0=culling, 1=edges, 2=smooth, 3=respondable, 4=static, 5=open
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int pType=-1;
        bool cone=false;
        C3Vector s(tt::getLimitedFloat(0.00001f,100000.0f,sizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[1]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[2]));
        //bool smooth=(options&4)!=0;
        int openEnds=0;
        if ((options&32)!=0)
            openEnds=3;
        int faces=0;
        int sides=32;
        if (precision!=nullptr)
        {
            faces=tt::getLimitedInt(3,100,precision[0]);
            if (pType==2)
                sides=tt::getLimitedInt(3,50,precision[1]); // sphere
            else
                sides=tt::getLimitedInt(0,50,precision[1]);
        }

        if (primitiveType==0) // cuboid
            pType=1;
        if (primitiveType==1) // sphere
        {
            pType=2;
            faces=16;
            s(1)=s(0);
            s(2)=s(0);
        }
        if (primitiveType==2) // cylinder
        {
            pType=3;
            s(1)=s(0);
        }
        if (primitiveType==3) // cone
        {
            pType=3;
            s(1)=s(0);
            cone=true;
        }
        if (pType==-1)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_TYPE);
            return(-1);
        }
        CShape* shape=CAddOperations::addPrimitiveShape(pType,s,nullptr,faces,sides,0,true,openEnds,true,true,cone,1000.0f,false,0.5f);

        C7Vector identity;
        identity.setIdentity();
        shape->setLocalTransformation(identity);
        shape->setCulling((options&1)!=0);
        shape->setVisibleEdges((options&2)!=0);
        shape->setRespondable((options&8)!=0);
        shape->setShapeIsDynamicallyStatic((options&16)!=0);
        shape->geomData->geomInfo->setMass(tt::getLimitedFloat(0.000001f,10000.0f,mass));
        return(shape->getObjectHandle());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddBanner_internal(const simChar* label,simFloat size,simInt options,const simFloat* positionAndEulerAngles,simInt parentObjectHandle,const simFloat* labelColors,const simFloat* backgroundColors)
{ // from positionAndEulerAngles on, all can be nullptr for default behaviour
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int retVal=-1; // means error

        CBannerObject* it=new CBannerObject(label,options,parentObjectHandle,positionAndEulerAngles,labelColors,backgroundColors,size);
        retVal=App::ct->bannerCont->addObject(it);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveBanner_internal(simInt bannerID)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (bannerID==sim_handle_all)
            App::ct->bannerCont->removeAllObjects(false);
        else
        {
            int handleFlags=0;
            if (bannerID>=0)
            {
                handleFlags=bannerID&0xff00000;
                bannerID=bannerID&0xfffff;
            }
            CBannerObject* it=App::ct->bannerCont->getObject(bannerID);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_INEXISTANT);
                return(-1);
            }
            if (handleFlags&sim_handleflag_togglevisibility)
            {
                if (it->toggleVisibility())
                    return(1);
                return(0);
            }
            else
                App::ct->bannerCont->removeObject(bannerID);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateDummy_internal(simFloat size,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CDummy* it=new CDummy();
        it->setSize(size);
        if (color!=nullptr)
        {
            it->getColor()->setColor(color+0,sim_colorcomponent_ambient_diffuse);
            it->getColor()->setColor(color+6,sim_colorcomponent_specular);
            it->getColor()->setColor(color+9,sim_colorcomponent_emission);
        }
        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreatePath_internal(simInt attributes,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath* newObject=new CPath();
        App::ct->objCont->addObjectToScene(newObject,false,true);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertPathCtrlPoints_internal(simInt pathHandle,simInt options,simInt startIndex,simInt ptCnt,const simVoid* ptData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath* path=App::ct->objCont->getPath(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_INEXISTANT);
            return(-1);
        }

        path->pathContainer->enableActualization(false);
        int fiCnt=11;
        if (options&2)
            fiCnt=16;

        for (int i=0;i<ptCnt;i++)
        {
            CSimplePathPoint* pt=new CSimplePathPoint();
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCutPathCtrlPoints_internal(simInt pathHandle,simInt startIndex,simInt ptCnt)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPath* path=App::ct->objCont->getPath(pathHandle);
        if (path==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_INEXISTANT);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat* simGetIkGroupMatrix_internal(simInt ikGroupHandle,simInt options,simInt* matrixSize)
{ 
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(nullptr);
        }
        float* retData=nullptr;
        if (options==0)
            retData=it->getLastJacobianData(matrixSize);
        if (options==1)
            retData=it->getLastManipulabilityValue(matrixSize);
        return(retData);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}


simInt simCreateProximitySensor_internal(simInt sensorType,simInt subType,simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CProxSensor* it=new CProxSensor(sensorType);
        it->setSensableType(subType);
        it->setExplicitHandling((options&1)!=0);
        it->setShowVolumeWhenDetecting((options&2)==0);
        it->setShowVolumeWhenNotDetecting((options&4)==0);
        it->setFrontFaceDetection((options&8)==0);
        it->setBackFaceDetection((options&16)==0);
        it->setClosestObjectMode((options&32)==0);
        it->setNormalCheck((options&64)!=0);
//        it->setCheckOcclusions((options&128)!=0);
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

        if ( (sensorType==sim_proximitysensor_cylinder_subtype)||(sensorType==sim_proximitysensor_cone_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setRadius(floatParams[7]);
        if (sensorType==sim_proximitysensor_cylinder_subtype)
            it->convexVolume->setRadiusFar(floatParams[8]);
        if ( ((sensorType==sim_proximitysensor_ray_subtype)&&it->getRandomizedDetection())||(sensorType==sim_proximitysensor_cone_subtype)||(sensorType==sim_proximitysensor_disc_subtype) )
            it->convexVolume->setAngle(floatParams[9]);

        it->setAllowedNormal(floatParams[10]);
        it->convexVolume->setSmallestDistanceAllowed(floatParams[11]);
        it->setSize(floatParams[12]);

        if (color!=nullptr)
        {
            it->getColor(0)->setColor(color+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(0)->setColor(color+6,sim_colorcomponent_specular);
            it->getColor(0)->setColor(color+9,sim_colorcomponent_emission);

            it->getColor(1)->setColor(color+12,sim_colorcomponent_ambient_diffuse);
            it->getColor(1)->setColor(color+18,sim_colorcomponent_specular);
            it->getColor(1)->setColor(color+21,sim_colorcomponent_emission);

            it->getColor(2)->setColor(color+24,sim_colorcomponent_ambient_diffuse);
            it->getColor(2)->setColor(color+30,sim_colorcomponent_specular);
            it->getColor(2)->setColor(color+33,sim_colorcomponent_emission);

            it->getColor(3)->setColor(color+36,sim_colorcomponent_ambient_diffuse);
            it->getColor(3)->setColor(color+42,sim_colorcomponent_specular);
            it->getColor(3)->setColor(color+45,sim_colorcomponent_emission);
        }


        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateForceSensor_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CForceSensor* it=new CForceSensor();

        it->setEnableForceThreshold((options&1)!=0);
        it->setEnableTorqueThreshold((options&2)!=0);

        it->setFilterType(intParams[0]);
        it->setValueCountForFilter(intParams[1]);
        it->setConsecutiveThresholdViolationsForBreaking(intParams[2]);

        it->setSize(floatParams[0]);
        it->setForceThreshold(floatParams[1]);
        it->setTorqueThreshold(floatParams[2]);

        if (color!=nullptr)
        {
            it->getColor(false)->setColor(color+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(false)->setColor(color+6,sim_colorcomponent_specular);
            it->getColor(false)->setColor(color+9,sim_colorcomponent_emission);

            it->getColor(true)->setColor(color+12,sim_colorcomponent_ambient_diffuse);
            it->getColor(true)->setColor(color+18,sim_colorcomponent_specular);
            it->getColor(true)->setColor(color+21,sim_colorcomponent_emission);
        }

        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateVisionSensor_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CVisionSensor* it=new CVisionSensor();

        it->setExplicitHandling((options&1)!=0);
        it->setPerspectiveOperation((options&2)!=0);
        it->setShowVolumeWhenNotDetecting((options&4)==0);
        it->setShowVolumeWhenDetecting((options&8)==0);
        it->setUseExternalImage((options&16)!=0);
        it->setUseLocalLights((options&32)!=0);
        it->setShowFogIfAvailable((options&64)==0);
        it->setUseEnvironmentBackgroundColor((options&128)==0);

        int res[2]={intParams[0],intParams[1]};
        it->setDesiredResolution(res);

        it->setNearClippingPlane(floatParams[0]);
        it->setFarClippingPlane(floatParams[1]);
        if (it->getPerspectiveOperation())
            it->setViewAngle(floatParams[2]);
        else
            it->setOrthoViewSize(floatParams[2]);
        it->setSize(C3Vector(floatParams+3));
        it->setDefaultBufferValues(floatParams+6);

        if (color!=nullptr)
        {
            it->getColor(false)->setColor(color+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(false)->setColor(color+6,sim_colorcomponent_specular);
            it->getColor(false)->setColor(color+9,sim_colorcomponent_emission);

            it->getColor(true)->setColor(color+12,sim_colorcomponent_ambient_diffuse);
            it->getColor(true)->setColor(color+18,sim_colorcomponent_specular);
            it->getColor(true)->setColor(color+21,sim_colorcomponent_emission);
        }

        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}


simInt simCreateJoint_internal(simInt jointType,simInt jointMode,simInt options,const simFloat* sizes,const simFloat* colorA,const simFloat* colorB)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CJoint* it=new CJoint(jointType);
        it->setJointMode(jointMode);
        it->setHybridFunctionality(options&1);
        if (sizes!=nullptr)
        {
            it->setLength(sizes[0]);
            it->setDiameter(sizes[1]);
        }
        if (colorA!=nullptr)
        {
            it->getColor(false)->setColor(colorA+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(false)->setColor(colorA+6,sim_colorcomponent_specular);
            it->getColor(false)->setColor(colorA+9,sim_colorcomponent_emission);
        }
        if (colorB!=nullptr)
        {
            it->getColor(true)->setColor(colorB+0,sim_colorcomponent_ambient_diffuse);
            it->getColor(true)->setColor(colorB+6,sim_colorcomponent_specular);
            it->getColor(true)->setColor(colorB+9,sim_colorcomponent_emission);
        }
        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simFloatingViewAdd_internal(simFloat posX,simFloat posY,simFloat sizeX,simFloat sizeY,simInt options)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
#ifdef SIM_WITH_GUI
        CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
        if (page==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PAGE_INEXISTANT);
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
        sizeX=SIM_MIN(sizeX,2.0f*SIM_MIN(posX,1.0f-posX));
        sizeY=SIM_MIN(sizeY,2.0f*SIM_MIN(posY,1.0f-posY));
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simFloatingViewRemove_internal(simInt floatingViewHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        for (int i=0;i<8;i++)
        {
            CSPage* page=App::ct->pageContainer->getPage(i);
            if (page!=nullptr)
            {
                int viewIndex=page->getViewIndexFromViewUniqueID(floatingViewHandle);
                if (viewIndex!=-1)
                {
                    if (viewIndex>=page->getRegularViewCount())
                    {
                        page->removeFloatingView(viewIndex);
                        return(1);
                    }
                    break; // We can't remove the view because it is not floating (anymore?)
                }
            }
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCameraFitToView_internal(simInt viewHandleOrIndex,simInt objectCount,const simInt* objectHandles,simInt options,simFloat scaling)
{
    C_API_FUNCTION_DEBUG;

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
                    CSPage* page=App::ct->pageContainer->getPage(i);
                    int index=page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                    if (index!=-1)
                    {
                        view=page->getView(index);
                        break;
                    }
                }
            }
            else
            {
#ifdef SIM_WITH_GUI
                CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
#else
                CSPage* page=App::ct->pageContainer->getPage(0);
#endif
                if (page==nullptr)
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PAGE_INEXISTANT);
                    return(-1);
                }
                else
                    view=page->getView(viewHandleOrIndex);
            }
            if (view==nullptr)
            { // silent error
                return(0);
            }
            camera=App::ct->objCont->getCamera(view->getLinkedObjectID());
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
            camera=App::ct->objCont->getCamera(viewHandleOrIndex);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simAdjustView_internal(simInt viewHandleOrIndex,simInt associatedViewableObjectHandle,simInt options,const simChar* viewLabel)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CSView* view=nullptr;
        if (viewHandleOrIndex>=10000)
        {
            for (int i=0;i<8;i++)
            {
                CSPage* page=App::ct->pageContainer->getPage(i);
                int index=page->getViewIndexFromViewUniqueID(viewHandleOrIndex);
                if (index!=-1)
                {
                    view=page->getView(index);
                    break;
                }
            }
        }
        else
        {
#ifdef SIM_WITH_GUI
            CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
#else
            CSPage* page=App::ct->pageContainer->getPage(0);
#endif
            if (page==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PAGE_INEXISTANT);
                return(-1);
            }
            else
                view=page->getView(viewHandleOrIndex);
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
            C3DObject* it=App::ct->objCont->getObjectFromHandle(associatedViewableObjectHandle);
            int objType=it->getObjectType();
            if ( (objType!=sim_object_camera_type)&&(objType!=sim_object_graph_type)&&(objType!=sim_object_visionsensor_type) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_IS_NOT_VIEWABLE);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateHeightfieldShape_internal(simInt options,simFloat shadingAngle,simInt xPointCount,simInt yPointCount,simFloat xSize,const simFloat* heights)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if ( (xPointCount<2)||(xPointCount>2048)||(yPointCount<2)||(yPointCount>2048)||(xSize<0.00001f) )
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectInt32Parameter_internal(simInt objectHandle,simInt parameterID,simInt* parameter)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not retrieved
        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CCamera* camera=App::ct->objCont->getCamera(objectHandle);
        CJoint* joint=App::ct->objCont->getJoint(objectHandle);
        CDummy* dummy=App::ct->objCont->getDummy(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
        CMirror* mirror=App::ct->objCont->getMirror(objectHandle);
        CProxSensor* proximitySensor=App::ct->objCont->getProximitySensor(objectHandle);
        CMill* mill=App::ct->objCont->getMill(objectHandle);
        CMotionPlanningTask* motionPlanningObject=App::ct->motionPlanning->getObject(objectHandle);
        CLight* light=App::ct->objCont->getLight(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (parameterID==sim_objintparam_visibility_layer)
                {
                    parameter[0]=it->layer;
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
                    parameter[0]=0;
                    if (!it->getObjectTranslationDisabledDuringNonSimulation())
                        parameter[0]|=1;
                    if (!it->getObjectTranslationDisabledDuringSimulation())
                        parameter[0]|=2;
                    if (!it->getObjectRotationDisabledDuringNonSimulation())
                        parameter[0]|=4;
                    if (!it->getObjectRotationDisabledDuringSimulation())
                        parameter[0]|=8;
                    if (it->getObjectTranslationSettingsLocked())
                        parameter[0]|=16;
                    if (it->getObjectRotationSettingsLocked())
                        parameter[0]|=32;
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
                rendSens->getRealResolution(r);
                parameter[0]=r[0];
                retVal=1;
            }
            if (parameterID==sim_visionintparam_resolution_y)
            {
                int r[2];
                rendSens->getRealResolution(r);
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
                parameter[0]=rendSens->getDetectableEntityID();
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
                    if (tt::getValidInt(val,samples))
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
                if (rendSens->getPerspectiveOperation())
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
                    if (tt::getValidInt(val,samples))
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
        if (joint!=nullptr)
        {
            if (parameterID==sim_jointintparam_motor_enabled)
            {
                parameter[0]=0;
                if (joint->getEnableDynamicMotor())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_jointintparam_ctrl_enabled)
            {
                parameter[0]=0;
                if (joint->getEnableDynamicMotorControlLoop())
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
                if (joint->getDynamicMotorLockModeWhenInVelocityControl())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_jointintparam_vortex_dep_handle)
            {
                parameter[0]=joint->getVortexDependentJointId();
                retVal=1;
            }
        }
        if (shape!=nullptr)
        {
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
                if (shape->getShapeWireframe())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_compound)
            {
                parameter[0]=0;
                if (!shape->geomData->geomInfo->isGeometric())
                    parameter[0]=1;
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex)
            {
                parameter[0]=0;
                if (shape->geomData->geomInfo->isConvex())
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
                if (shape->getHideEdgeBorders())
                    parameter[0]=1;
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
        if (motionPlanningObject!=nullptr)
        {
            if (parameterID==sim_mplanintparam_nodes_computed)
            {
                if (motionPlanningObject->getPhase1NodeAreReady())
                    parameter[0]=1;
                else
                    parameter[0]=0;
                retVal=1;
            }
        }

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectInt32Parameter_internal(simInt objectHandle,simInt parameterID,simInt parameter)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not set
        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CJoint* joint=App::ct->objCont->getJoint(objectHandle);
        CDummy* dummy=App::ct->objCont->getDummy(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
        CMirror* mirror=App::ct->objCont->getMirror(objectHandle);
        CCamera* camera=App::ct->objCont->getCamera(objectHandle);
        CLight* light=App::ct->objCont->getLight(objectHandle);
        CProxSensor* proximitySensor=App::ct->objCont->getProximitySensor(objectHandle);
        CMotionPlanningTask* motionPlanningObject=App::ct->motionPlanning->getObject(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (parameterID==sim_objintparam_visibility_layer)
                {
                    it->layer=tt::getLimitedInt(0,65535,parameter);
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
                    it->setObjectTranslationDisabledDuringNonSimulation((parameter&1)==0);
                    it->setObjectTranslationDisabledDuringSimulation((parameter&2)==0);
                    it->setObjectRotationDisabledDuringNonSimulation((parameter&4)==0);
                    it->setObjectRotationDisabledDuringSimulation((parameter&8)==0);
                    it->setObjectTranslationSettingsLocked((parameter&16)!=0);
                    it->setObjectRotationSettingsLocked((parameter&32)!=0);
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
                rendSens->getRealResolution(r);
                if (parameterID==sim_visionintparam_resolution_x)
                    r[0]=parameter;
                else
                    r[1]=parameter;
                rendSens->setDesiredResolution(r);
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
                rendSens->setDetectableEntityID(parameter);
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
                rendSens->setPerspectiveOperation(parameter!=0);
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
                dummy->setLinkType(parameter,false);
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
            {
                joint->setEnableDynamicMotor(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_jointintparam_ctrl_enabled)
            {
                joint->setEnableDynamicMotorControlLoop(parameter!=0);
                retVal=1;
            }
            if (parameterID==2020)
            { // deprecated command
                retVal=1;
            }
            if (parameterID==sim_jointintparam_velocity_lock)
            {
                joint->setDynamicMotorLockModeWhenInVelocityControl(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_jointintparam_vortex_dep_handle)
            {
                if (joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,parameter))
                    retVal=1;
            }
        }
        if (shape!=nullptr)
        {
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
                shape->setShapeWireframe(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex)
            { // very careful when setting this!
                shape->geomData->geomInfo->setConvex(parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_shapeintparam_convex_check)
            {
                shape->geomData->geomInfo->checkIfConvex();
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
                shape->setHideEdgeBorders(parameter!=0);
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
        if (motionPlanningObject!=nullptr)
        {
            if (parameterID==sim_mplanintparam_prepare_nodes)
            {
                motionPlanningObject->calculateDataStructureIfNeeded(nullptr,nullptr,parameter!=0);
                retVal=1;
            }
            if (parameterID==sim_mplanintparam_clear_nodes)
            {
                motionPlanningObject->clearPhase2NodeVisualizationAndPathVisualization();
                retVal=1;
            }
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectFloatParameter_internal(simInt objectHandle,simInt parameterID,simFloat* parameter)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not retrieved
        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CJoint* joint=App::ct->objCont->getJoint(objectHandle);
        CForceSensor* forceSensor=App::ct->objCont->getForceSensor(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
        CLight* light=App::ct->objCont->getLight(objectHandle);
        CMirror* mirror=App::ct->objCont->getMirror(objectHandle);
        CPathPlanningTask* pathPlanningObject=App::ct->pathPlanning->getObject(objectHandle);
        CCamera* camera=App::ct->objCont->getCamera(objectHandle);
        CDummy* dummy=App::ct->objCont->getDummy(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                    it->getMarkingBoundingBox(minV,maxV);
                    if (parameterID<=sim_objfloatparam_objbbox_min_z)
                        parameter[0]=minV(parameterID-sim_objfloatparam_objbbox_min_x);
                    else
                        parameter[0]=maxV(parameterID-sim_objfloatparam_objbbox_max_x);
                    retVal=1;
                }
                if ((parameterID>=sim_objfloatparam_modelbbox_min_x)&&(parameterID<=sim_objfloatparam_modelbbox_max_z))
                {
                    C3Vector minV,maxV;
                    C7Vector ctmi(it->getCumulativeTransformationPart1().getInverse());
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
                    if (tt::getValidFloat(val,dist))
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
                    if (tt::getValidFloat(val,ap))
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
                joint->getDynamicMotorPositionControlParameters(pp,ip,dp);
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
                joint->getDynamicMotorSpringControlParameters(kp,cp);
                if (parameterID==sim_jointfloatparam_kc_k)
                    parameter[0]=kp;
                if (parameterID==sim_jointfloatparam_kc_c)
                    parameter[0]=cp;
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_ik_weight)
            {
                parameter[0]=joint->getIKWeight();
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
            if ((parameterID>=sim_jointfloatparam_intrinsic_x)&&(parameterID<=sim_jointfloatparam_intrinsic_qw))
            {
                C7Vector trFull(joint->getLocalTransformation());
                C7Vector trPart1(joint->getLocalTransformationPart1());
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
                parameter[0]=joint->getDynamicMotorUpperLimitVelocity();
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
                parameter[0]=shape->geomData->geomInfo->getMass();
                retVal=1;
            }
            if ((parameterID>=sim_shapefloatparam_texture_x)&&(parameterID<=sim_shapefloatparam_texture_scaling_y))
            {
                if (shape->geomData->geomInfo->isGeometric())
                {
                    CTextureProperty* tp=((CGeometric*)shape->geomData->geomInfo)->getTextureProperty();
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
                if (shape->geomData->geomInfo->isGeometric())
                {
                    parameter[0]=((CGeometric*)shape->geomData->geomInfo)->getGouraudShadingAngle();
                    retVal=1;
                }
                else
                    retVal=0;
            }
            if (parameterID==sim_shapefloatparam_edge_angle)
            {
                if (shape->geomData->geomInfo->isGeometric())
                {
                    parameter[0]=((CGeometric*)shape->geomData->geomInfo)->getEdgeThresholdAngle();
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
        }
        if (camera!=nullptr)
        {
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
                    if (tt::getValidFloat(val,dist))
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
                    if (tt::getValidFloat(val,ap))
                        parameter[0]=ap;
                }
                retVal=1;
            }
        }

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectFloatParameter_internal(simInt objectHandle,simInt parameterID,simFloat parameter)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0; // Means the parameter was not set
        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CJoint* joint=App::ct->objCont->getJoint(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
        CLight* light=App::ct->objCont->getLight(objectHandle);
        CMirror* mirror=App::ct->objCont->getMirror(objectHandle);
        CPathPlanningTask* pathPlanningObject=App::ct->pathPlanning->getObject(objectHandle);
        CCamera* camera=App::ct->objCont->getCamera(objectHandle);
        CDummy* dummy=App::ct->objCont->getDummy(objectHandle);
        if (parameterID<sim_objparam_end)
        { // for all scene objects
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                joint->getDynamicMotorPositionControlParameters(pp,ip,dp);
                if (parameterID==sim_jointfloatparam_pid_p)
                    pp=parameter;
                if (parameterID==sim_jointfloatparam_pid_i)
                    ip=parameter;
                if (parameterID==sim_jointfloatparam_pid_d)
                    dp=parameter;
                joint->setDynamicMotorPositionControlParameters(pp,ip,dp);
                retVal=1;
            }
            if ((parameterID==sim_jointfloatparam_kc_k)||(parameterID==sim_jointfloatparam_kc_c))
            {
                float kp,cp;
                joint->getDynamicMotorSpringControlParameters(kp,cp);
                if (parameterID==sim_jointfloatparam_kc_k)
                    kp=parameter;
                if (parameterID==sim_jointfloatparam_kc_c)
                    cp=parameter;
                joint->setDynamicMotorSpringControlParameters(kp,cp);
                retVal=1;
            }
            if (parameterID==sim_jointfloatparam_ik_weight)
            {
                joint->setIKWeight(parameter);
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
                joint->setDynamicMotorUpperLimitVelocity(parameter);
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
                shape->geomData->geomInfo->setMass(parameter);
                retVal=1;
            }
            if ((parameterID>=sim_shapefloatparam_texture_x)&&(parameterID<=sim_shapefloatparam_texture_scaling_y))
            {
                if (shape->geomData->geomInfo->isGeometric())
                {
                    CTextureProperty* tp=((CGeometric*)shape->geomData->geomInfo)->getTextureProperty();
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
                if (shape->geomData->geomInfo->isGeometric())
                {
                    if (!VThread::isCurrentThreadTheUiThread())
                    { // we are NOT in the UI thread. We execute the command now:
                        ((CGeometric*)shape->geomData->geomInfo)->setGouraudShadingAngle(parameter);
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
                if (shape->geomData->geomInfo->isGeometric())
                {
                    if (!VThread::isCurrentThreadTheUiThread())
                    { // we are NOT in the UI thread. We execute the command now:
                        ((CGeometric*)shape->geomData->geomInfo)->setEdgeThresholdAngle(parameter);
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
        if (camera!=nullptr)
        {
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
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetObjectStringParameter_internal(simInt objectHandle,simInt parameterID,simInt* parameterLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(nullptr);
        simChar* retVal=nullptr; // Means the parameter was not retrieved
        parameterLength[0]=0;
        C3DObject* object=App::ct->objCont->getObjectFromHandle(objectHandle);
        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
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
                shape->geomData->geomInfo->getColorStrings(colorNames);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectStringParameter_internal(simInt objectHandle,simInt parameterID,simChar* parameter,simInt parameterLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        int retVal=0; // Means the parameter was not set


        CVisionSensor* rendSens=App::ct->objCont->getVisionSensor(objectHandle);
        CShape* shape=App::ct->objCont->getShape(objectHandle);
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
                        ((CGeometric*)shape->geomData->geomInfo)->color.colorName.clear();
                    else
                        ((CGeometric*)shape->geomData->geomInfo)->color.colorName.assign(parameter,parameter+parameterLength);
                    retVal=1;
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetRotationAxis_internal(const simFloat* matrixStart,const simFloat* matrixGoal,simFloat* axis,simFloat* angle)
{
    C_API_FUNCTION_DEBUG;

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
    C_API_FUNCTION_DEBUG;

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
    C_API_FUNCTION_DEBUG;

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
        CJoint* it=App::ct->objCont->getJoint(handle);
        if (it->getJointType()==sim_joint_spherical_subtype)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPersistentDataWrite_internal(const simChar* dataTag,const simChar* dataValue,simInt dataLength,simInt options)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::ct->persistentDataContainer->writeData(dataTag,std::string(dataValue,dataLength),(options&1)!=0);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simChar* simPersistentDataRead_internal(const simChar* dataTag,simInt* dataLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::string sigVal;
        if (App::ct->persistentDataContainer->readData(dataTag,sigVal))
        {
            char* retVal=new char[sigVal.length()];
            for (unsigned int i=0;i<sigVal.length();i++)
                retVal[i]=sigVal[i];
            dataLength[0]=(int)sigVal.length();
            return(retVal);
        }
        return(nullptr); // data does not exist
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simIsHandleValid_internal(simInt generalObjectHandle,simInt generalObjectType)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_object_type))&&(App::ct->objCont->getObjectFromHandle(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_ui_type))&&(App::ct->buttonBlockContainer->getBlockWithID(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_collection_type))&&(App::ct->collections->getCollection(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_collision_type))&&(App::ct->collisions->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_distance_type))&&(App::ct->distances->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_constraintsolver_type))&&(App::ct->constraintSolver->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_ik_type))&&(App::ct->ikGroups->getIkGroup(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_script_type))&&(App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_pathplanning_type))&&(App::ct->pathPlanning->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_motionplanning_type))&&(App::ct->motionPlanning->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        if (((generalObjectType==-1)||(generalObjectType==sim_appobj_texture_type))&&(App::ct->textureCont->getObject(generalObjectHandle)!=nullptr))
        {
            return(1);
        }
        return(0); // handle is not valid!
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleVisionSensor_internal(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    C_API_FUNCTION_DEBUG;

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
        for (int i=0;i<int(App::ct->objCont->visionSensorList.size());i++)
        {
            CVisionSensor* it=(CVisionSensor*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->visionSensorList[i]);
            if (visionSensorHandle>=0)
            { // explicit handling
                it=(CVisionSensor*)App::ct->objCont->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                    return(-1);
                }
                retVal=it->handleSensor();
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReadVisionSensor_internal(simInt visionSensorHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    C_API_FUNCTION_DEBUG;

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
        CVisionSensor* it=(CVisionSensor*)App::ct->objCont->getObjectFromHandle(visionSensorHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetVisionSensor_internal(simInt visionSensorHandle)
{
    C_API_FUNCTION_DEBUG;

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
        for (int i=0;i<int(App::ct->objCont->visionSensorList.size());i++)
        {
            CVisionSensor* it=(CVisionSensor*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->visionSensorList[i]);
            if (visionSensorHandle>=0)
            { // Explicit handling
                it=(CVisionSensor*)App::ct->objCont->getObjectFromHandle(visionSensorHandle);
                if (!it->getExplicitHandling())
                {
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simCheckVisionSensor_internal(simInt sensorHandle,simInt entityHandle,simFloat** auxValues,simInt** auxValuesCount)
{
    C_API_FUNCTION_DEBUG;

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

        if (!App::ct->mainSettings->visionSensorsEnabled)
            return(0);

        if (auxValues!=nullptr)
            auxValues[0]=nullptr;
        if (auxValuesCount!=nullptr)
            auxValuesCount[0]=nullptr;
        int retVal=0;

        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simCheckVisionSensorEx_internal(simInt sensorHandle,simInt entityHandle,simBool returnImage)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(nullptr);
        }
        if (!isVisionSensor(__func__,sensorHandle))
        {
            return(nullptr);
        }
        if ( (entityHandle!=sim_handle_all)&&(!doesEntityExist(__func__,entityHandle)) )
        {
            return(nullptr);
        }
        if (entityHandle==sim_handle_all)
            entityHandle=-1;

        if (!App::ct->mainSettings->visionSensorsEnabled)
        {
            return(nullptr);
        }

        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
        float* retBuffer=it->checkSensorEx(entityHandle,returnImage!=0,false,false,true);
        return(retBuffer);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simGetVisionSensorResolution_internal(simInt sensorHandle,simInt* resolution)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(-1);
        }
        if (!isVisionSensor(__func__,sensorHandle))
        {
            return(-1);
        }
        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
        it->getRealResolution(resolution);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simGetVisionSensorImage_internal(simInt sensorHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(nullptr);
        }
        if (!isVisionSensor(__func__,sensorHandle))
        {
            return(nullptr);
        }
        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
        int res[2];
        it->getRealResolution(res);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simUChar* simGetVisionSensorCharImage_internal(simInt sensorHandle,simInt* resolutionX,simInt* resolutionY)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        sensorHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,sensorHandle))
        {
            return(nullptr);
        }
        if (!isVisionSensor(__func__,sensorHandle))
        {
            return(nullptr);
        }
        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
        int res[2];
        it->getRealResolution(res);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetVisionSensorImage_internal(simInt sensorHandle,const simFloat* image)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        if (!isVisionSensor(__func__,objectHandle))
        {
            return(-1);
        }
        CVisionSensor* it=App::ct->objCont->getVisionSensor(objectHandle);
        int retVal=0;
        if (it->setExternalImage(image,(handleFlags&sim_handleflag_greyscale)!=0))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetVisionSensorCharImage_internal(simInt sensorHandle,const simUChar* image)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=sensorHandle&0xff00000;
        int objectHandle=sensorHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        if (!isVisionSensor(__func__,objectHandle))
        {
            return(-1);
        }
        CVisionSensor* it=App::ct->objCont->getVisionSensor(objectHandle);
        int retVal=0;
        if (it->setExternalCharImage(image,(handleFlags&sim_handleflag_greyscale)!=0))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simFloat* simGetVisionSensorDepthBuffer_internal(simInt sensorHandle)
{
    C_API_FUNCTION_DEBUG;

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
        CVisionSensor* it=App::ct->objCont->getVisionSensor(sensorHandle);
        int res[2];
        it->getRealResolution(res);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simRMLPosition_internal(simInt dofs,simDouble timeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxVelAccelJerk,const simBool* selection,const simDouble* targetPosVel,simDouble* newPosVelAccel,simVoid* auxData)
{
    C_API_FUNCTION_DEBUG;

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
    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlposition,auxVals,data,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlposition,auxVals,data,replyData);
    }
    // void* returnData=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlposition,auxVals,data,replyData);
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
{
    C_API_FUNCTION_DEBUG;

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
    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlvelocity,auxVals,data,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlvelocity,auxVals,data,replyData);
    }

//    void* returnData=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlvelocity,auxVals,data,replyData);
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
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

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

    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlpos,auxVals,data,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlpos,auxVals,data,replyData);
    }
//    CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlpos,auxVals,data,replyData);

    delete[] data;
    return(replyData[1]);
}

simInt simRMLVel_internal(simInt dofs,simDouble smallestTimeStep,simInt flags,const simDouble* currentPosVelAccel,const simDouble* maxAccelJerk,const simBool* selection,const simDouble* targetVel,simVoid* auxData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

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

    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlvel,auxVals,data,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlvel,auxVals,data,replyData);
    }
//    CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlvel,auxVals,data,replyData);

    delete[] data;
    return(replyData[1]);
}

simInt simRMLStep_internal(simInt handle,simDouble timeStep,simDouble* newPosVelAccel,simVoid* auxData,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

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
    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlstep,auxVals,nullptr,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            returnData=CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlstep,auxVals,nullptr,replyData);
    }
//    void* returnData=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlstep,auxVals,nullptr,replyData);
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
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    int auxVals[4]={0,0,0,0};
    auxVals[1]=handle;
    int replyData[4]={-1,-1,-1,-1};

    if (CPluginContainer::getPluginFromName("ReflexxesTypeIV")!=nullptr)
        CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeIV",sim_message_eventcallback_rmlremove,auxVals,nullptr,replyData);
    else
    {
        if (CPluginContainer::getPluginFromName("ReflexxesTypeII")!=nullptr)
            CPluginContainer::sendEventCallbackMessageToOnePlugin("ReflexxesTypeII",sim_message_eventcallback_rmlremove,auxVals,nullptr,replyData);
    }
//    CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_rmlremove,auxVals,nullptr,replyData);
    return(replyData[1]);
}

simInt simGetObjectQuaternion_internal(simInt objectHandle,simInt relativeToObjectHandle,simFloat* quaternion)
{ 
    C_API_FUNCTION_DEBUG;
    // V-REP quaternion, internally: w x y z
    // V-REP quaternion, at interfaces: x y z w

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        C7Vector tr;
        if (relativeToObjectHandle==-1)
            tr=it->getCumulativeTransformationPart1();
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            if (it->getParentObject()==relObj)
            { // special here, in order to not lose precision in a series of get/set
                tr=it->getLocalTransformationPart1();
            }
            else
            {
                C7Vector relTr(relObj->getCumulativeTransformationPart1());
                tr=relTr.getInverse()*it->getCumulativeTransformationPart1();
            }
        }
        quaternion[0]=tr.Q(1);
        quaternion[1]=tr.Q(2);
        quaternion[2]=tr.Q(3);
        quaternion[3]=tr.Q(0);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetObjectQuaternion_internal(simInt objectHandle,simInt relativeToObjectHandle,const simFloat* quaternion)
{
    C_API_FUNCTION_DEBUG;
    // V-REP quaternion, internally: w x y z
    // V-REP quaternion, at interfaces: x y z w

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if (relativeToObjectHandle==sim_handle_parent)
        {
            relativeToObjectHandle=-1;
            C3DObject* parent=it->getParentObject();
            if (parent!=nullptr)
                relativeToObjectHandle=parent->getObjectHandle();
        }
        if (relativeToObjectHandle!=-1)
        {
            if (!doesObjectExist(__func__,relativeToObjectHandle))
                return(-1);
        }
        if (it->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)it;
            if (!shape->getShapeIsDynamicallyStatic()) // condition new since 5/5/2013
                shape->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        }
        else
            it->setDynamicsFullRefreshFlag(true); // dynamically enabled objects have to be reset first!
        if (relativeToObjectHandle==-1)
        {
            C4Vector q;
            q(0)=quaternion[3];
            q(1)=quaternion[0];
            q(2)=quaternion[1];
            q(3)=quaternion[2];
            App::ct->objCont->setAbsoluteAngles(it->getObjectHandle(),q.getEulerAngles());
        }
        else
        {
            C3DObject* relObj=App::ct->objCont->getObjectFromHandle(relativeToObjectHandle);
            if (it->getParentObject()==relObj)
            { // special here, in order to not lose precision in a series of get/set
                C7Vector tr(it->getLocalTransformationPart1());
                tr.Q(0)=quaternion[3];
                tr.Q(1)=quaternion[0];
                tr.Q(2)=quaternion[1];
                tr.Q(3)=quaternion[2];
                it->setLocalTransformation(tr);
            }
            else
            {
                C7Vector absTr(it->getCumulativeTransformationPart1());
                C7Vector relTr(relObj->getCumulativeTransformationPart1());
                C7Vector x(relTr.getInverse()*absTr);
                x.Q(0)=quaternion[3];
                x.Q(1)=quaternion[0];
                x.Q(2)=quaternion[1];
                x.Q(3)=quaternion[2];
                absTr=relTr*x;
                App::ct->objCont->setAbsoluteAngles(it->getObjectHandle(),absTr.Q.getEulerAngles());
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simFileDialog_internal(simInt mode,const simChar* title,const simChar* startPath,const simChar* initName,const simChar* extName,const simChar* ext)
{
    C_API_FUNCTION_DEBUG;
#ifdef SIM_WITH_GUI
    if (!isSimulatorInitialized(__func__))
        return(nullptr);
    char* retVal=nullptr;

    std::string nameAndPath;
    bool native=1;
    #ifndef WIN_VREP // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
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
{
    C_API_FUNCTION_DEBUG;
#ifdef SIM_WITH_GUI
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=CPluginContainer::customUi_msgBox(dlgType,buttons,title,message);
//    int retVal=App::uiThread->messageBox_api(dlgType,buttons,title,message);
    return(retVal);
#else
    return(-1);
#endif
}

simInt simSetShapeMassAndInertia_internal(simInt shapeHandle,simFloat mass,const simFloat* inertiaMatrix,const simFloat* centerOfMass,const simFloat* transformation)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        /*
        if (!App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            return(-1);
        }
        */
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        CShape* it=App::ct->objCont->getShape(shapeHandle);

        if (mass<0.0000001f)
            mass=0.0000001f;
        C3X3Matrix m;
        m.copyFromInterface(inertiaMatrix);
        m.axis[0](1)=m.axis[1](0);
        m.axis[0](2)=m.axis[2](0);
        m.axis[1](2)=m.axis[2](1);
        m/=mass; // in V-REP we work with the "massless inertia"
        it->geomData->geomInfo->setMass(mass);
        C3Vector com(centerOfMass);
        C4X4Matrix tr;
        if (transformation==nullptr)
            tr.setIdentity();
        else
            tr.copyFromInterface(transformation);

        C4Vector rot;
        C3Vector pmoment;
        CGeomWrap::findPrincipalMomentOfInertia(m,rot,pmoment);
        if (pmoment(0)<0.0000001f)
            pmoment(0)=0.0000001f;
        if (pmoment(1)<0.0000001f)
            pmoment(1)=0.0000001f;
        if (pmoment(2)<0.0000001f)
            pmoment(0)=0.0000001f;
        it->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoment);
        it->geomData->geomInfo->setLocalInertiaFrame(it->getCumulativeTransformation().getInverse()*tr.getTransformation()*C7Vector(rot,com));
        it->setDynamicsFullRefreshFlag(true);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeMassAndInertia_internal(simInt shapeHandle,simFloat* mass,simFloat* inertiaMatrix,simFloat* centerOfMass,const simFloat* transformation)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        CShape* it=App::ct->objCont->getShape(shapeHandle);

        mass[0]=it->geomData->geomInfo->getMass();
        //float mmm=it->geomData->geomInfo->getMass();

        C7Vector tr(it->getCumulativeTransformation()*it->geomData->geomInfo->getLocalInertiaFrame());
        C4X4Matrix ref;
        if (transformation==nullptr)
            ref.setIdentity();
        else
            ref.copyFromInterface(transformation);
        C3X3Matrix m(CGeomWrap::getNewTensor(it->geomData->geomInfo->getPrincipalMomentsOfInertia(),ref.getTransformation().getInverse()*tr));
        m*=mass[0]; // in V-REP we work with the "massless inertia"
        m.copyToInterface(inertiaMatrix);
        (ref.getTransformation().getInverse()*tr).X.copyTo(centerOfMass);

        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGroupShapes_internal(const simInt* shapeHandles,simInt shapeCount)
{
    C_API_FUNCTION_DEBUG;

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
            CShape* it=App::ct->objCont->getShape(shapeHandles[i]);
            if (it!=nullptr)
                shapes.push_back(it->getObjectHandle());
        }
        if (shapes.size()<2)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_NOT_ENOUGH_SHAPES);
            return(-1);
        }
        std::vector<int> initSelection;
        App::ct->objCont->getSelectedObjects(initSelection);
        int retVal;
        if (merging)
            retVal=CSceneObjectOperations::mergeSelection(&shapes,false);
        else
            retVal=CSceneObjectOperations::groupSelection(&shapes,false);
        App::ct->objCont->setSelectedObjects(initSelection);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt* simUngroupShape_internal(simInt shapeHandle,simInt* shapeCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        shapeCount[0]=0;
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        // Commented following because animation export won't work otherwise
        /*
        if (!App::ct->simulation->isSimulationStopped())
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SIMULATION_NOT_STOPPED);
            shapeCount[0]=0;
            return(nullptr);
        }
        */
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
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        if (it->geomData->geomInfo->isGeometric())
        {
            if (dividing)
            {
                std::vector<int> finalSel;
                std::vector<int> previousSel;
                std::vector<int> sel;
                previousSel.push_back(shapeHandle);
                sel.push_back(shapeHandle);
                CSceneObjectOperations::divideSelection(&sel,false);
                for (int j=0;j<int(sel.size());j++)
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CANNOT_DIVIDE_COMPOUND_SHAPE);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simConvexDecompose_internal(simInt shapeHandle,simInt options,const simInt* intParams,const simFloat* floatParams)
{ // one shape at a time!
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    int retVal=CSceneObjectOperations::convexDecompose_apiVersion(shapeHandle,options,intParams,floatParams);
    return(retVal);
}

simInt simAddGhost_internal(simInt ghostGroup,simInt objectHandle,simInt options,simFloat startTime,simFloat endTime,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

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
        int retVal=App::ct->ghostObjectCont->addGhost(ghostGroup,objectHandle,options,startTime,endTime,color);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simModifyGhost_internal(simInt ghostGroup,simInt ghostId,simInt operation,simFloat floatValue,simInt options,simInt optionsMask,const simFloat* colorOrTransformation)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::ct->ghostObjectCont->modifyGhost(ghostGroup,ghostId,operation,floatValue,options,optionsMask,colorOrTransformation);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simVoid simQuitSimulator_internal(simBool ignoredArgument)
{
    C_API_FUNCTION_DEBUG;
    SSimulationThreadCommand cmd;
    cmd.cmdId=FINAL_EXIT_REQUEST_CMD;
    App::appendSimulationThreadCommand(cmd);
}

simInt simGetThreadId_internal()
{
    C_API_FUNCTION_DEBUG;

    int retVal=VThread::getThreadId_apiQueried();
    return(retVal); // 0=GUI thread, 1=main sim thread, 2-n=aux. sim threads
}

simInt simLockResources_internal(simInt lockType,simInt reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

#ifndef SIM_WITHOUT_QT_AT_ALL
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
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

#ifndef SIM_WITHOUT_QT_AT_ALL
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
#endif // SIM_WITHOUT_QT_AT_ALL
}

simInt simEnableEventCallback_internal(simInt eventCallbackType,const simChar* plugin,simInt reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (CPluginContainer::enableOrDisableSpecificEventCallback(eventCallbackType,plugin))
            return(1);
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetShapeMaterial_internal(simInt shapeHandle,simInt materialIdOrShapeHandle)
{
    C_API_FUNCTION_DEBUG;

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
                    CShape* shape=App::ct->objCont->getShape(shapeHandle);
                    shape->getDynMaterial()->generateDefaultMaterial(materialIdOrShapeHandle);
                }
                else
                { // set the same material as another shape
                    CShape* matShape=App::ct->objCont->getShape(materialIdOrShapeHandle);
                    if (matShape!=nullptr)
                    {
                        CShape* shape=App::ct->objCont->getShape(shapeHandle);
                        shape->setDynMaterial(matShape->getDynMaterial()->copyYourself());
                        retVal=1;
                    }
                }
            }
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetTextureId_internal(const simChar* textureName,simInt* resolution)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        CTextureObject* to=App::ct->textureCont->getObject(textureName);
        if (to!=nullptr)
        {
            retVal=to->getObjectID();
            if (resolution!=nullptr)
                to->getTextureSize(resolution[0],resolution[1]);
        }
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReadTexture_internal(simInt textureId,simInt options,simInt posX,simInt posY,simInt sizeX,simInt sizeY)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to=App::ct->textureCont->getObject(textureId);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simWriteTexture_internal(simInt textureId,simInt options,const simChar* data,simInt posX,simInt posY,simInt sizeX,simInt sizeY,simFloat interpol)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CTextureObject* to=App::ct->textureCont->getObject(textureId);
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
                int retVal=-1;
                if (to->writePortionOfTexture((unsigned char*)data,posX,posY,sizeX,sizeY,(options&4)!=0,interpol))
                    retVal=1;
                return(retVal);
            }
            else
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateTexture_internal(const simChar* fileName,simInt options,const simFloat* planeSizes,const simFloat* scalingUV,const simFloat* xy_g,simInt fixedResolution,simInt* textureId,simInt* resolution,const simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

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
                    CShape* shape=CAddOperations::addPrimitiveShape(0,s,nullptr,0,32,0,false,0,false,false,false,1000.0f,false,0.5f);

                    C7Vector identity;
                    identity.setIdentity();
                    shape->setLocalTransformation(identity);
                    shape->setCulling(false);
                    shape->setVisibleEdges(true);
                    shape->setRespondable(false);
                    shape->setShapeIsDynamicallyStatic(true);
                    shape->geomData->geomInfo->setMass(1.0f);

                    if (resolution!=nullptr)
                    {
                        resolution[0]=resX;
                        resolution[1]=resY;
                    }

                    CTextureObject* textureObj=new CTextureObject(resX,resY);
                    textureObj->setImage(rgba,false,false,data); // keep false,false
                    textureObj->setObjectName(App::directories->getNameFromFull(fileName).c_str());
                    delete[] data;
                    textureObj->addDependentObject(shape->getObjectHandle(),((CGeometric*)shape->geomData->geomInfo)->getUniqueID());
                    int texID=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                    CTextureProperty* tp=new CTextureProperty(texID);
                    ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(tp);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FILE_NOT_FOUND);
        }
        else
        { // just creating a texture (not loading it)
            if (resolution!=nullptr)
            {
                C3Vector s(0.1f,0.1f,0.00001f);
                if (planeSizes!=nullptr)
                    s=C3Vector(tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,planeSizes[1]),0.00001f);
                CShape* shape=CAddOperations::addPrimitiveShape(0,s,nullptr,0,32,0,false,0,false,false,false,1000.0f,false,0.5f);
                C7Vector identity;
                identity.setIdentity();
                shape->setLocalTransformation(identity);
                shape->setCulling(false);
                shape->setVisibleEdges(true);
                shape->setRespondable(false);
                shape->setShapeIsDynamicallyStatic(true);
                shape->geomData->geomInfo->setMass(1.0f);

                CTextureObject* textureObj=new CTextureObject(resolution[0],resolution[1]);
                textureObj->setRandomContent();
                textureObj->setObjectName(App::directories->getNameFromFull(fileName).c_str());
                textureObj->addDependentObject(shape->getObjectHandle(),((CGeometric*)shape->geomData->geomInfo)->getUniqueID());
                int texID=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                CTextureProperty* tp=new CTextureProperty(texID);
                ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(tp);
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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        }
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simWriteCustomDataBlock_internal(simInt objectHandle,const simChar* tagName,const simChar* data,simInt dataSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        std::vector<char> buffer;
        int extractedBufSize;
        bool useTempBuffer=false;
        if (strlen(tagName)>=4)
            useTempBuffer=((tagName[0]=='@')&&(tagName[1]=='t')&&(tagName[2]=='m')&&(tagName[3]=='p'));
        if (data==nullptr)
            dataSize=0;
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(objectHandle);
            if (script!=nullptr)
            { // here we have a script
                if (useTempBuffer)
                {
                    if (strlen(tagName)!=0)
                    {
                        int l=script->getObjectCustomDataLength_tempData(356248756);
                        if (l>0)
                        {
                            buffer.resize(l,' ');
                            script->getObjectCustomData_tempData(356248756,&buffer[0]);
                        }
                        delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                        _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                        if (buffer.size()>0)
                            script->setObjectCustomData_tempData(356248756,&buffer[0],(int)buffer.size());
                        else
                            script->setObjectCustomData_tempData(356248756,nullptr,0);
                    }
                    else
                        script->setObjectCustomData_tempData(356248756,nullptr,0);
                }
                else
                {
                    if (strlen(tagName)!=0)
                    {
                        int l=script->getObjectCustomDataLength(356248756);
                        if (l>0)
                        {
                            buffer.resize(l,' ');
                            script->getObjectCustomData(356248756,&buffer[0]);
                        }
                        delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                        _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                        if (buffer.size()>0)
                            script->setObjectCustomData(356248756,&buffer[0],(int)buffer.size());
                        else
                            script->setObjectCustomData(356248756,nullptr,0);
                    }
                    else
                        script->setObjectCustomData(356248756,nullptr,0);
                }
            }
        }
        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(-1);
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (useTempBuffer)
            {
                if (strlen(tagName)!=0)
                {
                    int l=it->getObjectCustomDataLength_tempData(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        it->getObjectCustomData_tempData(356248756,&buffer[0]);
                    }
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        it->setObjectCustomData_tempData(356248756,&buffer[0],(int)buffer.size());
                    else
                        it->setObjectCustomData_tempData(356248756,nullptr,0);
                }
                else
                    it->setObjectCustomData(356248756,nullptr,0);
            }
            else
            {
                if (strlen(tagName)!=0)
                {
                    int l=it->getObjectCustomDataLength(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        it->getObjectCustomData(356248756,&buffer[0]);
                    }
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        it->setObjectCustomData(356248756,&buffer[0],(int)buffer.size());
                    else
                        it->setObjectCustomData(356248756,nullptr,0);
                }
                else
                    it->setObjectCustomData(356248756,nullptr,0);
            }
        }

        if (objectHandle==sim_handle_scene)
        {
            if (useTempBuffer)
            {
                if (strlen(tagName)!=0)
                {
                    int l=App::ct->customSceneData_tempData->getDataLength(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        App::ct->customSceneData_tempData->getData(356248756,&buffer[0]);
                    }
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        App::ct->customSceneData_tempData->setData(356248756,&buffer[0],(int)buffer.size());
                    else
                        App::ct->customSceneData_tempData->setData(356248756,nullptr,0);
                }
                else
                    App::ct->customSceneData_tempData->setData(356248756,nullptr,0);
            }
            else
            {
                if (strlen(tagName)!=0)
                {
                    int l=App::ct->customSceneData->getDataLength(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        App::ct->customSceneData->getData(356248756,&buffer[0]);
                    }
                    delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                    _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                    if (buffer.size()>0)
                        App::ct->customSceneData->setData(356248756,&buffer[0],(int)buffer.size());
                    else
                        App::ct->customSceneData->setData(356248756,nullptr,0);
                }
                else
                    App::ct->customSceneData->setData(356248756,nullptr,0);
            }
        }

        if (objectHandle==sim_handle_app)
        { // here we have the app
            if (strlen(tagName)!=0)
            {
                int l=App::ct->customAppData->getDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    App::ct->customAppData->getData(356248756,&buffer[0]);
                }
                delete[] _extractCustomDataFromBuffer(buffer,tagName,&extractedBufSize);
                _appendCustomDataToBuffer(buffer,tagName,data,dataSize);
                if (buffer.size()>0)
                    App::ct->customAppData->setData(356248756,&buffer[0],(int)buffer.size());
                else
                    App::ct->customAppData->setData(356248756,nullptr,0);
            }
            else
                App::ct->customAppData->setData(356248756,nullptr,0);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simReadCustomDataBlock_internal(simInt objectHandle,const simChar* tagName,simInt* dataSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retBuffer=nullptr;
        dataSize[0]=0;
        std::vector<char> buffer;
        bool useTempBuffer=false;
        if (strlen(tagName)>=4)
            useTempBuffer=((tagName[0]=='@')&&(tagName[1]=='t')&&(tagName[2]=='m')&&(tagName[3]=='p'));
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(objectHandle);
            if (script!=nullptr)
            {
                if (useTempBuffer)
                {
                    int l=script->getObjectCustomDataLength_tempData(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        script->getObjectCustomData_tempData(356248756,&buffer[0]);
                    }
                }
                else
                {
                    int l=script->getObjectCustomDataLength(356248756);
                    if (l>0)
                    {
                        buffer.resize(l,' ');
                        script->getObjectCustomData(356248756,&buffer[0]);
                    }
                }
                retBuffer=_extractCustomDataFromBuffer(buffer,tagName,dataSize);
            }
        }
        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // Here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (useTempBuffer)
            {
                int l=it->getObjectCustomDataLength_tempData(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    it->getObjectCustomData_tempData(356248756,&buffer[0]);
                }
            }
            else
            {
                int l=it->getObjectCustomDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    it->getObjectCustomData(356248756,&buffer[0]);
                }
            }
            retBuffer=_extractCustomDataFromBuffer(buffer,tagName,dataSize);
        }

        if (objectHandle==sim_handle_scene)
        { // here we have a scene
            if (useTempBuffer)
            {
                int l=App::ct->customSceneData_tempData->getDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    App::ct->customSceneData_tempData->getData(356248756,&buffer[0]);
                }
            }
            else
            {
                int l=App::ct->customSceneData->getDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    App::ct->customSceneData->getData(356248756,&buffer[0]);
                }
            }
            retBuffer=_extractCustomDataFromBuffer(buffer,tagName,dataSize);
        }

        if (objectHandle==sim_handle_app)
        { // here we have the app
            int l=App::ct->customAppData->getDataLength(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                App::ct->customAppData->getData(356248756,&buffer[0]);
            }
            retBuffer=_extractCustomDataFromBuffer(buffer,tagName,dataSize);
        }

        return(retBuffer);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simReadCustomDataBlockTags_internal(simInt objectHandle,simInt* tagCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retBuffer=nullptr;
        tagCount[0]=0;
        std::vector<char> buffer;
        std::vector<std::string> allTags;
        if (objectHandle>=SIM_IDSTART_LUASCRIPT)
        { // here we have a script
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(objectHandle);
            if (script!=nullptr)
            {
                int l=script->getObjectCustomDataLength_tempData(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    script->getObjectCustomData_tempData(356248756,&buffer[0]);
                    _extractCustomDataTagsFromBuffer(buffer,allTags);
                }

                l=script->getObjectCustomDataLength(356248756);
                if (l>0)
                {
                    buffer.resize(l,' ');
                    script->getObjectCustomData(356248756,&buffer[0]);
                    _extractCustomDataTagsFromBuffer(buffer,allTags);
                }
            }
        }
        if ( (objectHandle>=0)&&(objectHandle<SIM_IDSTART_LUASCRIPT) )
        { // here we have an object
            if (!doesObjectExist(__func__,objectHandle))
                return(nullptr);
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
            int l=it->getObjectCustomDataLength_tempData(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                it->getObjectCustomData_tempData(356248756,&buffer[0]);
                _extractCustomDataTagsFromBuffer(buffer,allTags);
            }
            l=it->getObjectCustomDataLength(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                it->getObjectCustomData(356248756,&buffer[0]);
                _extractCustomDataTagsFromBuffer(buffer,allTags);
            }
        }

        if (objectHandle==sim_handle_scene)
        { // here we have a scene
            int l=App::ct->customSceneData_tempData->getDataLength(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                App::ct->customSceneData_tempData->getData(356248756,&buffer[0]);
                _extractCustomDataTagsFromBuffer(buffer,allTags);
            }
            l=App::ct->customSceneData->getDataLength(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                App::ct->customSceneData->getData(356248756,&buffer[0]);
                _extractCustomDataTagsFromBuffer(buffer,allTags);
            }
        }

        if (objectHandle==sim_handle_app)
        { // here we have the application
            int l=App::ct->customAppData->getDataLength(356248756);
            if (l>0)
            {
                buffer.resize(l,' ');
                App::ct->customAppData->getData(356248756,&buffer[0]);
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
        return(retBuffer);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simAddPointCloud_internal(simInt pageMask,simInt layerMask,simInt objectHandle,simInt options,simFloat pointSize,simInt ptCnt,const simFloat* pointCoordinates,const simChar* defaultColors,const simChar* pointColors,const simFloat* pointNormals)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        CPtCloud_old* ptCloud=new CPtCloud_old(pageMask,layerMask,objectHandle,options,pointSize,ptCnt,pointCoordinates,(unsigned char*)pointColors,pointNormals,(unsigned char*)defaultColors);
        retVal=App::ct->pointCloudCont->addObject(ptCloud);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simModifyPointCloud_internal(simInt pointCloudHandle,simInt operation,const simInt* intParam,const simFloat* floatParam)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (operation==0)
        {
            if (App::ct->pointCloudCont->removeObject(pointCloudHandle))
                return(1);
        }
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeGeomInfo_internal(simInt shapeHandle,simInt* intData,simFloat* floatData,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1; // means error
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::ct->objCont->getShape(shapeHandle);
            retVal=0;
            if (shape->geomData->geomInfo->isGeometric())
            {
                CGeometric* geom=(CGeometric*)shape->geomData->geomInfo;
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
                    floatData[3]=geom->getPurePrimitiveInsideScaling();
                }
                if (geom->isPure())
                    retVal|=2;
                if (geom->isConvex())
                    retVal|=4;
            }
            else
            { // we have a compound...
                retVal|=1;
                if (shape->geomData->geomInfo->isPure())
                    retVal|=2;
            }
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt* simGetObjectsInTree_internal(simInt treeBaseHandle,simInt objectType,simInt options,simInt* objectCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int* retVal=nullptr;
        if ( (treeBaseHandle==sim_handle_scene)||doesObjectExist(__func__,treeBaseHandle) )
        {
            std::vector<C3DObject*> toExplore;
            if (treeBaseHandle!=sim_handle_scene)
            {
                C3DObject* baseObj=App::ct->objCont->getObjectFromHandle(treeBaseHandle);
                toExplore.push_back(baseObj);
            }
            else
            {
                for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
                {
                    C3DObject* obj=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    toExplore.push_back(obj);
                }
            }
            std::vector<int> outHandles;
            bool firstChildrenDone=false;
            while (toExplore.size()!=0)
            {
                C3DObject* obj=toExplore[0];
                toExplore.erase(toExplore.begin(),toExplore.begin()+1);
                if (treeBaseHandle!=sim_handle_scene)
                {
                    if ( (!firstChildrenDone)||((options&2)==0) )
                    {
                        for (int i=0;i<int(obj->childList.size());i++)
                            toExplore.push_back(obj->childList[i]);
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
                            if (obj->getParentObject()==nullptr)
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetObjectSizeValues_internal(simInt objectHandle,const simFloat* sizeValues)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(objectHandle);
            obj->setSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectSizeValues_internal(simInt objectHandle,simFloat* sizeValues)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(objectHandle);
            obj->getSizeValues(sizeValues);
            return(1);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simScaleObject_internal(simInt objectHandle,simFloat xScale,simFloat yScale,simFloat zScale,simInt options)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (doesObjectExist(__func__,objectHandle))
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(objectHandle);
            obj->scaleObjectNonIsometrically(xScale,yScale,zScale);
            return(1);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetShapeTextureId_internal(simInt shapeHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::ct->objCont->getShape(shapeHandle);
            if (shape->geomData->geomInfo->isGeometric())
            {
                CTextureProperty* tp=((CGeometric*)shape->geomData->geomInfo)->getTextureProperty();
                if (tp!=nullptr)
                {
                    CTextureObject* to=App::ct->textureCont->getObject(tp->getTextureObjectID());
                    return(to->getObjectID());
                }
                return(-1);
            }
            return(-1);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simSetShapeTexture_internal(simInt shapeHandle,simInt textureId,simInt mappingMode,simInt options,const simFloat* uvScaling,const simFloat* position,const simFloat* orientation)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::ct->objCont->getShape(shapeHandle);
            if (shape->geomData->geomInfo->isGeometric())
            {
                CTextureProperty* tp=((CGeometric*)shape->geomData->geomInfo)->getTextureProperty();
                if (tp!=nullptr)
                { // first remove any existing texture:
                    App::ct->textureCont->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                    delete tp;
                    ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(nullptr);
                }
                if (textureId==-1)
                    return(1);
                CTextureObject* to=App::ct->textureCont->getObject(textureId);
                if (to!=nullptr)
                {
                    to->addDependentObject(shape->getObjectHandle(),((CGeometric*)shape->geomData->geomInfo)->getUniqueID());
                    tp=new CTextureProperty(textureId);
                    ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(tp);
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
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TEXTURE_INEXISTANT);
            }
        }
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt* simGetCollectionObjects_internal(simInt collectionHandle,simInt* objectCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (doesCollectionExist(__func__,collectionHandle))
        {
            CRegCollection* coll=App::ct->collections->getCollection(collectionHandle);
            objectCount[0]=int(coll->collectionObjects.size());
            int* retVal=new int[objectCount[0]];
            for (int i=0;i<objectCount[0];i++)
                retVal[i]=coll->collectionObjects[i];
            return(retVal);
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetScriptAttribute_internal(simInt scriptHandle,simInt attributeID,simFloat floatVal,simInt intOrBoolVal)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustScriptDisabledDSim_compatibilityMode(intOrBoolVal==0);
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            it->setCustomizationScriptCleanupBeforeSave(intOrBoolVal!=0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            it->setExecutionOrder(intOrBoolVal);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            it->setNumberOfPasses(intOrBoolVal);
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution()) )
        {
            it->setAutomaticCascadingCallsDisabled_OLD(intOrBoolVal==0);
            retVal=1;
        }
        if ( (attributeID==sim_scriptattribute_enabled) )//&&(it->getScriptType()==sim_scripttype_childscript) )
        {
            it->setScriptIsDisabled(intOrBoolVal==0);
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
        {
            it->setDebugLevel(intOrBoolVal);
            retVal=1;
        }


        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetScriptAttribute_internal(simInt scriptHandle,simInt attributeID,simFloat* floatVal,simInt* intOrBoolVal)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
            return(-1);
        }
        int retVal=-1;
        if ( (attributeID==sim_customizationscriptattribute_activeduringsimulation)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustScriptDisabledDSim_compatibilityMode())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if ( (attributeID==sim_customizationscriptattribute_cleanupbeforesave)&&(it->getScriptType()==sim_scripttype_customizationscript) )
        {
            if (it->getCustomizationScriptCleanupBeforeSave())
                intOrBoolVal[0]=1;
            else
                intOrBoolVal[0]=0;
            retVal=1;
        }
        if ( (attributeID==sim_childscriptattribute_automaticcascadingcalls)&&(it->getScriptType()==sim_scripttype_childscript)&&(!it->getThreadedExecution()) )
        {
            if (it->getAutomaticCascadingCallsDisabled_OLD())
                intOrBoolVal[0]=0;
            else
                intOrBoolVal[0]=1;
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executionorder)
        {
            intOrBoolVal[0]=it->getExecutionOrder();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_executioncount)
        {
            intOrBoolVal[0]=it->getNumberOfPasses();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_debuglevel)
        {
            intOrBoolVal[0]=it->getDebugLevel();
            retVal=1;
        }
        if (attributeID==sim_scriptattribute_scripttype)
        {
            intOrBoolVal[0]=it->getScriptType();
            if (it->getThreadedExecution())
                intOrBoolVal[0]|=sim_scripttype_threaded;
            retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReorientShapeBoundingBox_internal(simInt shapeHandle,simInt relativeToHandle,simInt reservedSetToZero)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isShape(__func__,shapeHandle))
        {
            return(-1);
        }
        if ( (relativeToHandle!=-1)&&(relativeToHandle!=sim_handle_self) )
        {
            if (!doesObjectExist(__func__,relativeToHandle))
            {
                return(-1);
            }
        }

        CShape* theShape=App::ct->objCont->getShape(shapeHandle);
        C3DObject* theObjectRelativeTo=App::ct->objCont->getObjectFromHandle(relativeToHandle);
        if ( (!theShape->geomData->geomInfo->isPure())||(theShape->isCompound()) )
        { // We can reorient all shapes, except for pure simple shapes (i.e. pure non-compound shapes)
            if (relativeToHandle==-1)
                theShape->alignBoundingBoxWithWorld();
            else if (relativeToHandle==sim_handle_self)
                theShape->alignBoundingBoxWithMainAxis();
            else
            {
                C7Vector oldAbsTr(theShape->getCumulativeTransformationPart1());
                C7Vector oldAbsTr2(theObjectRelativeTo->getCumulativeTransformationPart1().getInverse()*oldAbsTr);
                C7Vector x(oldAbsTr2*oldAbsTr.getInverse());
                theShape->setLocalTransformation(theShape->getParentCumulativeTransformation().getInverse()*oldAbsTr2);
                theShape->alignBoundingBoxWithWorld();
                C7Vector newAbsTr2(theShape->getCumulativeTransformationPart1());
                C7Vector newAbsTr(x.getInverse()*newAbsTr2);
                theShape->setLocalTransformation(theShape->getParentCumulativeTransformation().getInverse()*newAbsTr);
            }
        }
        else
            return(0);
        return(1); // success
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSwitchThread_internal()
{
    C_API_FUNCTION_DEBUG;
    if (CThreadPool::getThreadAutomaticSwitch())
    { // Important: when a script forbids thread switching, we don't want that a plugin switches anyways
        if (CThreadPool::switchBackToPreviousThread())
            return(1);
    }
    return(0);
}

simInt simCreateIkGroup_internal(simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CikGroup* ikGroup=new CikGroup();
        ikGroup->setObjectName("IK_Group");
        App::ct->ikGroups->addIkGroup(ikGroup,false);
        ikGroup->setActive((options&1)==0);
        ikGroup->setCorrectJointLimits((options&2)!=0);
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
        {
            ikGroup->setDlsFactor(floatParams[0]);
            ikGroup->setJointLimitWeight(floatParams[1]);
            ikGroup->setJointTreshholdLinear(floatParams[2]);
            ikGroup->setJointTreshholdAngular(floatParams[3]);
        }
        return(ikGroup->getObjectID());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveIkGroup_internal(simInt ikGroupHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        App::ct->ikGroups->removeIkGroup(it->getObjectID());
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateIkElement_internal(simInt ikGroupHandle,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        int tip=intParams[0];
        int base=intParams[1];
        int constrBase=intParams[2];
        int constraints=intParams[3];
        if (!isDummy(__func__,tip))
        {
            return(-1);
        }
        if (App::ct->objCont->getObjectFromHandle(base)==nullptr)
            base=-1;
        if (App::ct->objCont->getObjectFromHandle(constrBase)==nullptr)
            constrBase=-1;
        CikEl* ikEl=new CikEl(tip);
        ikEl->setActive((options&1)==0);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateCollection_internal(const simChar* collectionName,simInt options)
{
    C_API_FUNCTION_DEBUG;

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
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            std::string text(collectionName);
            tt::removeIllegalCharacters(text,true);
            if (originalText!=text)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
            if (App::ct->collections->getCollection(text)!=nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ILLEGAL_COLLECTION_NAME);
                return(-1);
            }
        }
        else
            originalText="collection"; // default name

        CRegCollection* it=new CRegCollection(originalText.c_str());
        App::ct->collections->addCollection(it,false);
        it->setOverridesObjectMainProperties((options&1)!=0);
        return(it->getCollectionID());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simAddObjectToCollection_internal(simInt collectionHandle,simInt objectHandle,simInt what,simInt options)
{
    C_API_FUNCTION_DEBUG;

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
        CRegCollection* it=App::ct->collections->getCollection(collectionHandle);
        if (what!=sim_handle_all)
        {
            if (!doesObjectExist(__func__,objectHandle))
            {
                return(-1);
            }
        }
        CRegCollectionEl* el=nullptr;
        if (what==sim_handle_all)
        {
            el=new CRegCollectionEl(-1,GROUP_EVERYTHING,true);
        }
        if (what==sim_handle_single)
        {
            el=new CRegCollectionEl(objectHandle,GROUP_LOOSE,(options&1)==0);
        }
        if (what==sim_handle_tree)
        {
            int what=GROUP_FROM_BASE_INCLUDED;
            if ((options&2)!=0)
                what=GROUP_FROM_BASE_EXCLUDED;
            el=new CRegCollectionEl(objectHandle,what,(options&1)==0);
        }
        if (what==sim_handle_chain)
        {
            int what=GROUP_FROM_TIP_INCLUDED;
            if ((options&2)!=0)
                what=GROUP_FROM_TIP_EXCLUDED;
            el=new CRegCollectionEl(objectHandle,what,(options&1)==0);
        }
        if (el==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        it->addSubCollection(el);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveImage_internal(const simUChar* image,const simInt* resolution,simInt options,const simChar* filename,simInt quality,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=-1;
        if (CImageLoaderSaver::save(image,resolution,options,filename,quality,nullptr))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simUChar* simLoadImage_internal(simInt* resolution,simInt options,const simChar* filename,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal=CImageLoaderSaver::load(resolution,options,filename,reserved);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simUChar* simGetScaledImage_internal(const simUChar* imageIn,const simInt* resolutionIn,simInt* resolutionOut,simInt options,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        unsigned char* retVal=CImageLoaderSaver::getScaledImage(imageIn,resolutionIn,resolutionOut,options);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simTransformImage_internal(simUChar* image,const simInt* resolution,simInt options,const simFloat* floatParams,const simInt* intParams,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (CImageLoaderSaver::transformImage(image,resolution[0],resolution[1],options))
            return(1);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetQHull_internal(const simFloat* inVertices,simInt inVerticesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simInt reserved1,const simFloat* reserved2)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetDecimatedMesh_internal(const simFloat* inVertices,simInt inVerticesL,const simInt* inIndices,simInt inIndicesL,simFloat** verticesOut,simInt* verticesOutL,simInt** indicesOut,simInt* indicesOutL,simFloat decimationPercent,simInt reserved1,const simFloat* reserved2)
{
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simExportIk_internal(const simChar* pathAndFilename,simInt reserved1,simVoid* reserved2)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::ct->simulation->isSimulationStopped())
        {
            if (CFileOperations::apiExportIkContent(pathAndFilename,false))
                retVal=1;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simCallScriptFunctionEx_internal(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,simInt stackId)
{
    C_API_FUNCTION_DEBUG;
    CLuaScriptObject* script=nullptr;
    std::string funcName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        else
            funcName=funcNameAtScriptName;
        script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
            script=App::ct->luaScriptContainer->getMainScript();
        if (scriptHandleOrType==sim_scripttype_sandboxscript)
            script=App::ct->sandboxScript;
        if (scriptHandleOrType==sim_scripttype_addonscript)
            script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
        if (scriptHandleOrType==sim_scripttype_childscript)
        {
            int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
            script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
        }
        if (scriptHandleOrType==sim_scripttype_customizationscript)
        {
            int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
            script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
        }
    }

    if (script!=nullptr)
    {
        int retVal=-1; // error
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackId);
        if (stack!=nullptr)
        {
            if (script->getThreadedExecutionIsUnderWay())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId(),VThread::getCurrentThreadId()))
                    retVal=script->callScriptFunctionEx(funcName.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=1;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)funcName.c_str();
                    d[3]=stack;

                    retVal=CThreadPool::callRoutineViaSpecificThread(script->getThreadedScriptThreadId(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->callScriptFunctionEx(funcName.c_str(),stack);
                }
            }
            if (retVal==-3)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
            if (retVal==-2)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_FUNCTION_INEXISTANT);
            if (retVal==-1)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_ERROR_IN_SCRIPT_FUNCTION);
            if (retVal<-1)
                retVal=-1; // to stay backward compatible
        }
        else
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

simInt simComputeJacobian_internal(simInt ikGroupHandle,simInt options,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        int returnValue=-1;
        CikGroup* it=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        if (it->computeOnlyJacobian(options))
            returnValue=0;
        return(returnValue);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetConfigForTipPose_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simFloat thresholdDist,simInt maxTimeInMs,simFloat* retConfig,const simFloat* metric,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,const simFloat* lowLimits,const simFloat* ranges,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(-1);
        const float _defaultMetric[4]={1.0,1.0,1.0,0.1f};
        const float* theMetric=_defaultMetric;
        if (metric!=nullptr)
            theMetric=metric;
        std::vector<CJoint*> joints;
        std::vector<float> minVals;
        std::vector<float> rangeVals;
        CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        bool err=false;
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* aJoint=App::ct->objCont->getJoint(jointHandles[i]);
            if (aJoint==nullptr)
                err=true;
            else
            {
                joints.push_back(aJoint);
                if ( (lowLimits!=nullptr)&&(ranges!=nullptr) )
                {
                    minVals.push_back(lowLimits[i]);
                    rangeVals.push_back(ranges[i]);
                }
                else
                {
                    if (aJoint->getPositionIsCyclic())
                    {
                        minVals.push_back(-piValue_f);
                        rangeVals.push_back(piValTimes2_f);
                    }
                    else
                    {
                        minVals.push_back(aJoint->getPositionIntervalMin());
                        rangeVals.push_back(aJoint->getPositionIntervalRange());
                    }
                }
            }
        }
        if (err)
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLES);
        std::vector<CDummy*> tips;
        std::vector<CDummy*> targets;
        std::vector<C3DObject*> bases;
        if (!err)
        {
            if (ikGroup->ikElements.size()>0)
            {
                for (size_t i=0;i<ikGroup->ikElements.size();i++)
                {
                    CDummy* tip=App::ct->objCont->getDummy(ikGroup->ikElements[i]->getTooltip());
                    CDummy* target=App::ct->objCont->getDummy(ikGroup->ikElements[i]->getTarget());
                    C3DObject* base=nullptr;
                    if (ikGroup->ikElements[i]->getAlternativeBaseForConstraints()!=-1)
                        base=App::ct->objCont->getObjectFromHandle(ikGroup->ikElements[i]->getAlternativeBaseForConstraints());
                    else
                        base=App::ct->objCont->getObjectFromHandle(ikGroup->ikElements[i]->getBase());
                    if ((tip==nullptr)||(target==nullptr))
                        err=true;
                    tips.push_back(tip);
                    targets.push_back(target);
                    bases.push_back(base);
                }
            }
            else
            {
                err=true;
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            }
        }
        if ( (!err)&&(collisionPairCnt>0)&&(collisionPairs!=nullptr) )
        {
            bool e=false;
            for (int i=0;i<collisionPairCnt;i++)
            {
                C3DObject* eo1=App::ct->objCont->getObjectFromHandle(collisionPairs[2*i+0]);
                CRegCollection* ec1=App::ct->collections->getCollection(collisionPairs[2*i+0]);
                C3DObject* eo2=App::ct->objCont->getObjectFromHandle(collisionPairs[2*i+1]);
                CRegCollection* ec2=App::ct->collections->getCollection(collisionPairs[2*i+1]);
                e|=( ((eo1==nullptr)&&(ec1==nullptr)) || ((eo2==nullptr)&&(ec2==nullptr)&&(collisionPairs[2*i+1]!=sim_handle_all)) );
            }
            if (e)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_COLLISION_PAIRS);
                err=e;
            }
        }
        if (!err)
        {
            // Save joint positions/modes (all of them, just in case)
            std::vector<CJoint*> sceneJoints;
            std::vector<float> initSceneJointValues;
            std::vector<int> initSceneJointModes;
            for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
            {
                CJoint* aj=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
                sceneJoints.push_back(aj);
                initSceneJointValues.push_back(aj->getPosition());
                initSceneJointModes.push_back(aj->getJointMode());
            }

            ikGroup->setAllInvolvedJointsToPassiveMode();

            bool ikGroupWasActive=ikGroup->getActive();
            if (!ikGroupWasActive)
                ikGroup->setActive(true);

            // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
            std::vector<bool> enabledElements;
            for (int i=0;i<int(ikGroup->ikElements.size());i++)
                enabledElements.push_back(ikGroup->ikElements[i]->getActive());

            // Set the correct mode for the joints involved:
            for (int i=0;i<jointCnt;i++)
            {
                if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                    joints[i]->setJointMode(sim_jointmode_ik,false);
                else
                    joints[i]->setJointMode(sim_jointmode_dependent,false);
            }

            // do the calculation:
            int startTime=VDateTime::getTimeInMs();
            bool foundSomething=false;
            while (VDateTime::getTimeDiffInMs(startTime)<maxTimeInMs)
            {
                // 1. Pick a random state:
                for (int i=0;i<jointCnt;i++)
                    joints[i]->setPosition(minVals[i]+SIM_RAND_FLOAT*rangeVals[i]);

                // 2. Check distances between tip and target pairs (there might be several pairs!):
                float cumulatedDist=0.0;
                for (size_t el=0;el<ikGroup->ikElements.size();el++)
                {
                    C7Vector tipTr(tips[el]->getCumulativeTransformation());
                    C7Vector targetTr(targets[el]->getCumulativeTransformation());
                    C7Vector relTrInv(C7Vector::identityTransformation);
                    if (bases[el]!=nullptr)
                        relTrInv=bases[el]->getCumulativeTransformationPart1().getInverse();
                    tipTr=relTrInv*tipTr;
                    targetTr=relTrInv*targetTr;
                    C3Vector dx(tipTr.X-targetTr.X);
                    dx(0)*=theMetric[0];
                    dx(1)*=theMetric[1];
                    dx(2)*=theMetric[2];
                    float angle=tipTr.Q.getAngleBetweenQuaternions(targetTr.Q)*theMetric[3];
                    cumulatedDist+=sqrtf(dx(0)*dx(0)+dx(1)*dx(1)+dx(2)*dx(2)+angle*angle);
                }

                // 3. If distance<=threshold, try to perform IK:
                if (cumulatedDist<=thresholdDist)
                {
                    if (sim_ikresult_success==ikGroup->computeGroupIk(true))
                    { // 3.1 We found a configuration that works!
                        foundSomething=true;
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
                                        foundSomething=false;
                                        break;
                                    }
                                }
                            }
                        }
                        if (foundSomething)
                        { // last check: did we respect the limits?
                            if ( (lowLimits!=nullptr)&&(ranges!=nullptr) )
                            { // we had special limits!
                                for (int i=0;i<jointCnt;i++)
                                {
                                    if ( (joints[i]->getPosition()<minVals[i])||(joints[i]->getPosition()>minVals[i]+rangeVals[i]) )
                                        foundSomething=false;
                                }
                            }
                            if (foundSomething)
                            {
                                for (int i=0;i<jointCnt;i++)
                                    retConfig[i]=joints[i]->getPosition();
                                break;
                            }
                        }
                    }
                }
            }

            if (!ikGroupWasActive)
                ikGroup->setActive(false);

            // Restore the IK element activation state:
            for (size_t i=0;i<ikGroup->ikElements.size();i++)
                ikGroup->ikElements[i]->setActive(enabledElements[i]);

            // Restore joint positions/modes:
            for (size_t i=0;i<sceneJoints.size();i++)
            {
                if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                    sceneJoints[i]->setPosition(initSceneJointValues[i]);
                if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                    sceneJoints[i]->setJointMode(initSceneJointModes[i],false);
            }

            if (foundSomething)
                return(1);
            return(0);
        }
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simGenerateIkPath_internal(simInt ikGroupHandle,simInt jointCnt,const simInt* jointHandles,simInt ptCnt,simInt collisionPairCnt,const simInt* collisionPairs,const simInt* jointOptions,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesIKGroupExist(__func__,ikGroupHandle))
            return(nullptr);
        std::vector<CJoint*> joints;
        CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(ikGroupHandle);
        bool err=false;
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* aJoint=App::ct->objCont->getJoint(jointHandles[i]);
            if (aJoint==nullptr)
                err=true;
            else
                joints.push_back(aJoint);
        }
        if (err)
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLES);
        std::vector<CDummy*> tips;
        std::vector<CDummy*> targets;
        std::vector<C7Vector> startTrs;
        std::vector<C7Vector> goalTrs;
        if (!err)
        {
            if (ikGroup->ikElements.size()>0)
            {
                for (size_t i=0;i<ikGroup->ikElements.size();i++)
                {
                    CDummy* tip=App::ct->objCont->getDummy(ikGroup->ikElements[i]->getTooltip());
                    CDummy* target=App::ct->objCont->getDummy(ikGroup->ikElements[i]->getTarget());
                    if ((tip==nullptr)||(target==nullptr))
                        err=true;
                    tips.push_back(tip);
                    targets.push_back(target);
                    startTrs.push_back(tip->getCumulativeTransformation());
                    goalTrs.push_back(target->getCumulativeTransformation());
                }
            }
            else
            {
                err=true;
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_ELEMENT_INEXISTANT);
            }
        }
        if (!err)
        {
            if (ptCnt<2)
            {
                err=true;
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
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
            for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
            {
                CJoint* aj=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
                sceneJoints.push_back(aj);
                initSceneJointValues.push_back(aj->getPosition());
                initSceneJointModes.push_back(aj->getJointMode());
            }

            ikGroup->setAllInvolvedJointsToPassiveMode();

            bool ikGroupWasActive=ikGroup->getActive();
            if (!ikGroupWasActive)
                ikGroup->setActive(true);

            // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
            std::vector<bool> enabledElements;
            for (int i=0;i<int(ikGroup->ikElements.size());i++)
                enabledElements.push_back(ikGroup->ikElements[i]->getActive());

            // Set the correct mode for the joints involved:
            for (int i=0;i<jointCnt;i++)
            {
                if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                    joints[i]->setJointMode(sim_jointmode_ik,false);
                else
                    joints[i]->setJointMode(sim_jointmode_dependent,false);
            }

            // do the calculation:
            float t=0.0;
            float dt=1.0/(ptCnt-1);
            bool failed=false;
            std::vector<float> thePath;
            for (int iterCnt=0;iterCnt<ptCnt;iterCnt++)
            {
                for (size_t el=0;el<ikGroup->ikElements.size();el++)
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
                ikGroup->setActive(false);

            // Restore the IK element activation state:
            for (size_t i=0;i<ikGroup->ikElements.size();i++)
                ikGroup->ikElements[i]->setActive(enabledElements[i]);

            // Restore joint positions/modes:
            for (size_t i=0;i<sceneJoints.size();i++)
            {
                if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                    sceneJoints[i]->setPosition(initSceneJointValues[i]);
                if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                    sceneJoints[i]->setJointMode(initSceneJointModes[i],false);
            }

            // Restore target dummies:
            for (size_t el=0;el<ikGroup->ikElements.size();el++)
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simGetExtensionString_internal(simInt objectHandle,simInt index,const char* key)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        char* retVal=nullptr;
        std::string extensionString;
        if (objectHandle==-1)
            extensionString=App::ct->environment->getExtensionString();
        else
        {
            if (doesObjectExist(__func__,objectHandle))
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
                if ( (it->getObjectType()==sim_object_shape_type)&&(index>=0) )
                {
                    CGeometric* geom=((CShape*)it)->geomData->geomInfo->getShapeComponentAtIndex(index);
                    if (geom!=nullptr)
                        extensionString=geom->color.extensionString;
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simComputeMassAndInertia_internal(simInt shapeHandle,simFloat density)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=(CShape*)App::ct->objCont->getShape(shapeHandle);
            if (shape->geomData->geomInfo->isConvex())
            {
                std::vector<float> vert;
                std::vector<int> ind;
                shape->geomData->geomInfo->getCumulativeMeshes(vert,&ind,nullptr);
                C3Vector com;
                C3X3Matrix tensor;
                float mass=CVolInt::getMassCenterOfMassAndInertiaTensor(&vert[0],(int)vert.size()/3,&ind[0],(int)ind.size()/3,density,com,tensor);
                C4Vector rot;
                C3Vector pmoment;
                CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoment);
                shape->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoment);
                shape->geomData->geomInfo->setLocalInertiaFrame(C7Vector(rot,com));
                shape->geomData->geomInfo->setMass(mass);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
                return(1);
            }
            return(0);
        }
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCreateStack_internal()
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=new CInterfaceStack();
        int id=App::ct->interfaceStackContainer->addStack(stack);
        return(id);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simReleaseStack_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (App::ct->interfaceStackContainer->destroyStack(stackHandle))
            return(1);
        return(0);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCopyStack_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            int id=App::ct->interfaceStackContainer->addStack(stack->copyYourself());
            return(id);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushNullOntoStack_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushNullOntoStack();
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushBoolOntoStack_internal(simInt stackHandle,simBool value)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushBoolOntoStack(value);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt32OntoStack_internal(simInt stackHandle,simInt value)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushNumberOntoStack((double)value);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushFloatOntoStack_internal(simInt stackHandle,simFloat value)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushNumberOntoStack((double)value);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushDoubleOntoStack_internal(simInt stackHandle,simDouble value)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushNumberOntoStack(value);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushStringOntoStack_internal(simInt stackHandle,const simChar* value,simInt stringSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushStringOntoStack(value,stringSize);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushUInt8TableOntoStack_internal(simInt stackHandle,const simUChar* values,simInt valueCnt)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushUCharArrayTableOntoStack(values,valueCnt);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushInt32TableOntoStack_internal(simInt stackHandle,const simInt* values,simInt valueCnt)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushIntArrayTableOntoStack(values,valueCnt);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushFloatTableOntoStack_internal(simInt stackHandle,const simFloat* values,simInt valueCnt)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushFloatArrayTableOntoStack(values,valueCnt);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushDoubleTableOntoStack_internal(simInt stackHandle,const simDouble* values,simInt valueCnt)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushDoubleArrayTableOntoStack(values,valueCnt);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPushTableOntoStack_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->pushTableOntoStack();
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simInsertDataIntoStackTable_internal(simInt stackHandle)
{ // stack should have at least: table,key,value (where value is on top of stack)
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->insertDataIntoStackTable())
                return(1);
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackSize_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
            return(stack->getStackSize());
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPopStackItem_internal(simInt stackHandle,simInt count)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->popStackValue(count);
            return(stack->getStackSize());
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simMoveStackItemToTop_internal(simInt stackHandle,simInt cIndex)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->moveStackItemToTop(cIndex))
                return(1);
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_INDEX);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simIsStackValueNull_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->isStackValueNull())
                    return(1);
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackBoolValue_internal(simInt stackHandle,simBool* boolValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt32Value_internal(simInt stackHandle,simInt* numberValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                double v;
                if (stack->getStackNumberValue(v))
                {
                    numberValue[0]=(int)v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackFloatValue_internal(simInt stackHandle,simFloat* numberValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                double v;
                if (stack->getStackNumberValue(v))
                {
                    numberValue[0]=(float)v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackDoubleValue_internal(simInt stackHandle,simDouble* numberValue)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                double v;
                if (stack->getStackNumberValue(v))
                {
                    numberValue[0]=v;
                    return(1);
                }
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetStackStringValue_internal(simInt stackHandle,simInt* stringSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        if (stringSize!=nullptr)
            stringSize[0]=-1;
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(nullptr);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        if (stringSize!=nullptr)
            stringSize[0]=-1;
        return(nullptr);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    if (stringSize!=nullptr)
        stringSize[0]=-1;
    return(nullptr);
}

simInt simGetStackTableInfo_internal(simInt stackHandle,simInt infoType)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                int retVal=stack->getStackTableInfo(infoType);
                return(retVal);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackUInt8Table_internal(simInt stackHandle,simUChar* array,simInt count)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackUCharArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackInt32Table_internal(simInt stackHandle,simInt* array,simInt count)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackIntArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackFloatTable_internal(simInt stackHandle,simFloat* array,simInt count)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackFloatArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetStackDoubleTable_internal(simInt stackHandle,simDouble* array,simInt count)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->getStackDoubleArray(array,count))
                    return(1);
                return(0);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simUnfoldStackTable_internal(simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->getStackSize()>0)
            {
                if (stack->unfoldStackTable())
                    return(1);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDebugStack_internal(simInt stackHandle,simInt cIndex)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            stack->printContent(cIndex);
            return(1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetScriptVariable_internal(simInt scriptHandleOrType,const simChar* variableNameAtScriptName,simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CLuaScriptObject* script=nullptr;
        std::string variableName;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string varNameAtScriptName(variableNameAtScriptName);
            size_t p=varNameAtScriptName.find('@');
            if (p!=std::string::npos)
                variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
            else
                variableName=varNameAtScriptName;
            script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::ct->sandboxScript;
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                if (scriptName.size()>0)
                {
                    int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
                }
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            {
                int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
        }

        if (script!=nullptr)
        {
            bool doAClear=(stackHandle==0);
            CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
            if ( (stack==nullptr)&&(!doAClear) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
                return(-1);
            }
            if ( (stack!=nullptr)&&(stack->getStackSize()==0) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
                return(-1);
            }
            int retVal=-1; // error
            if (script->getThreadedExecutionIsUnderWay())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId(),VThread::getCurrentThreadId()))
                    retVal=script->setScriptVariable(variableName.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=2;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)variableName.c_str();
                    d[3]=stack;

                    retVal=CThreadPool::callRoutineViaSpecificThread(script->getThreadedScriptThreadId(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->setScriptVariable(variableName.c_str(),stack);
                }
            }

            if (retVal==-1)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat simGetEngineFloatParameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    C3DObject* it=(C3DObject*)object;
    bool success=true;
    float retVal=0.0f;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                retVal=App::ct->dynamicsContainer->getEngineFloatParam(paramId,&success);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simInt simGetEngineInt32Parameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    C3DObject* it=(C3DObject*)object;
    bool success=true;
    int retVal=0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                retVal=App::ct->dynamicsContainer->getEngineIntParam(paramId,&success);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simBool simGetEngineBoolParameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool* ok)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    C3DObject* it=(C3DObject*)object;
    bool success=true;
    simBool retVal=0;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                retVal=App::ct->dynamicsContainer->getEngineBoolParam(paramId,&success);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simInt simSetEngineFloatParameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simFloat val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        C3DObject* it=(C3DObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                success=App::ct->dynamicsContainer->setEngineFloatParam(paramId,val,false);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetEngineInt32Parameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simInt val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        C3DObject* it=(C3DObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                success=App::ct->dynamicsContainer->setEngineIntParam(paramId,val,false);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetEngineBoolParameter_internal(simInt paramId,simInt objectHandle,const simVoid* object,simBool val)
{   // if object is not nullptr, we use the object, otherwise the objectHandle.
    // if object is nullptr and objectHandle is -1, we retrieve a global parameter, otherwise a joint or shape parameter
    // this function doesn't generate any error messages
    C_API_FUNCTION_DEBUG;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        C3DObject* it=(C3DObject*)object;
        bool success=true;
        if (it==nullptr)
        {
            if (objectHandle!=-1)
            {
                it=App::ct->objCont->getObjectFromHandle(objectHandle);
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
                success=App::ct->dynamicsContainer->setEngineBoolParam(paramId,val,false);
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

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateOctree_internal(simFloat voxelSize,simInt options,simFloat pointSize,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        COctree* it=new COctree();
        it->setCellSize(voxelSize);
        it->setPointSize(int(pointSize+0.5));
        it->setUseRandomColors(options&1);
        it->setShowOctree(options&2);
        it->setUsePointsInsteadOfCubes(options&4);
        it->setColorIsEmissive(options&16);
        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreatePointCloud_internal(simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

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
        App::ct->objCont->addObjectToScene(it,false,true);
        int retVal=it->getObjectHandle();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetPointCloudOptions_internal(simInt pointCloudHandle,simFloat maxVoxelSize,simInt maxPtCntPerVoxel,simInt options,simFloat pointSize,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
        {
            return(-1);
        }
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        it->setCellSize(maxVoxelSize);
        it->setMaxPointCountPerCell(maxPtCntPerVoxel);
        it->setPointSize(int(pointSize+0.5));
        it->setUseRandomColors(options&1);
        it->setShowOctree(options&2);
        it->setDoNotUseCalculationStructure(options&8);
        it->setColorIsEmissive(options&16);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetPointCloudOptions_internal(simInt pointCloudHandle,simFloat* maxVoxelSize,simInt* maxPtCntPerVoxel,simInt* options,simFloat* pointSize,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
        {
            return(-1);
        }
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simInsertVoxelsIntoOctree_internal(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,const simUInt* tag,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        COctree* it=App::ct->objCont->getOctree(octreeHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveVoxelsFromOctree_internal(simInt octreeHandle,simInt options,const simFloat* pts,simInt ptCnt,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
        {
            return(-1);
        }
        COctree* it=App::ct->objCont->getOctree(octreeHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->subtractPoints(pts,ptCnt,options&1);
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertPointsIntoPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,const simUChar* color,simVoid* optionalValues)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemovePointsFromPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
        {
            return(-1);
        }
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->removePoints(pts,ptCnt,options&1,tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simIntersectPointsWithPointCloud_internal(simInt pointCloudHandle,simInt options,const simFloat* pts,simInt ptCnt,simFloat tolerance,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        if (pts==nullptr)
            it->clear();
        else
            it->intersectPoints(pts,ptCnt,options&1,tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

const float* simGetOctreeVoxels_internal(simInt octreeHandle,simInt* ptCnt,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__,octreeHandle))
        {
            return(nullptr);
        }
        COctree* it=App::ct->objCont->getOctree(octreeHandle);
        const std::vector<float>* p=it->getCubePositions();
        if (p->size()==0)
        {
            ptCnt[0]=0;
            return(nullptr);
        }
        ptCnt[0]=int(p->size())/3;
        return(&(p[0])[0]);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

const float* simGetPointCloudPoints_internal(simInt pointCloudHandle,simInt* ptCnt,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(nullptr);
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        const std::vector<float>* p=it->getPoints();
        if (p->size()==0)
        {
            ptCnt[0]=0;
            return(nullptr);
        }
        ptCnt[0]=int(p->size())/3;
        return(&(p[0])[0]);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simInsertObjectIntoOctree_internal(simInt octreeHandle,simInt objectHandle,simInt options,const simUChar* color,simUInt tag,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        COctree* it=App::ct->objCont->getOctree(octreeHandle);

        float savedCols[3]={it->getColor()->colors[0],it->getColor()->colors[1],it->getColor()->colors[2]};
        if (color!=nullptr)
        {
            it->getColor()->colors[0]=float(color[0])/255.1f;
            it->getColor()->colors[1]=float(color[1])/255.1f;
            it->getColor()->colors[2]=float(color[2])/255.1f;
        }
        it->insertObject(App::ct->objCont->getObjectFromHandle(objectHandle),tag);
        it->getColor()->colors[0]=savedCols[0];
        it->getColor()->colors[1]=savedCols[1];
        it->getColor()->colors[2]=savedCols[2];
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSubtractObjectFromOctree_internal(simInt octreeHandle,simInt objectHandle,simInt options,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        COctree* it=App::ct->objCont->getOctree(octreeHandle);
        it->subtractObject(App::ct->objCont->getObjectFromHandle(objectHandle));
        int retVal=int(it->getCubePositions()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simInsertObjectIntoPointCloud_internal(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat gridSize,const simUChar* color,simVoid* optionalValues)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isPointCloud(__func__,pointCloudHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        float savedGridSize=it->getBuildResolution();
        it->setBuildResolution(gridSize);
        float savedCols[3]={it->getColor()->colors[0],it->getColor()->colors[1],it->getColor()->colors[2]};
        if (color!=nullptr)
        {
            it->getColor()->colors[0]=float(color[0])/255.1f;
            it->getColor()->colors[1]=float(color[1])/255.1f;
            it->getColor()->colors[2]=float(color[2])/255.1f;
        }
        float insertionToleranceSaved=it->getInsertionDistanceTolerance();
        int optionalValuesBits=0;
        if (optionalValues!=nullptr)
            optionalValuesBits=((int*)optionalValues)[0];
        if (optionalValuesBits&1)
            it->setInsertionDistanceTolerance(((float*)optionalValues)[1]);
        it->insertObject(App::ct->objCont->getObjectFromHandle(objectHandle));
        it->setInsertionDistanceTolerance(insertionToleranceSaved);
        it->setBuildResolution(savedGridSize);
        it->getColor()->colors[0]=savedCols[0];
        it->getColor()->colors[1]=savedCols[1];
        it->getColor()->colors[2]=savedCols[2];
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSubtractObjectFromPointCloud_internal(simInt pointCloudHandle,simInt objectHandle,simInt options,simFloat tolerance,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!isOctree(__func__,pointCloudHandle))
            return(-1);
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        CPointCloud* it=App::ct->objCont->getPointCloud(pointCloudHandle);
        it->subtractObject(App::ct->objCont->getObjectFromHandle(objectHandle),tolerance);
        int retVal=int(it->getPoints()->size())/3;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCheckOctreePointOccupancy_internal(simInt octreeHandle,simInt options,const simFloat* points,simInt ptCnt,simUInt* tag,simUInt64* location,simVoid* reserved)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isOctree(__func__,octreeHandle))
            return(-1);
        if (ptCnt<=0)
            return(-1);
        COctree* it=App::ct->objCont->getOctree(octreeHandle);
        if (it->getOctreeInfo()==nullptr)
            return(0);
        const float* _pts=points;
        std::vector<float> __pts;
        if (options&1)
        {
            C7Vector tr(it->getCumulativeTransformation());
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
            if (CPluginContainer::mesh_checkOctreeCollisionWithSinglePoint(it->getOctreeInfo(),it->getCumulativeTransformation().getMatrix(),C3Vector(_pts),tag,location))
                return(1);
        }
        else
        {
            if (CPluginContainer::mesh_checkOctreeCollisionWithSeveralPoints(it->getOctreeInfo(),it->getCumulativeTransformation().getMatrix(),_pts,ptCnt))
                return(1);
        }
        return(0);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simOpenTextEditor_internal(const simChar* initText,const simChar* xml,simInt* various)
{
    C_API_FUNCTION_DEBUG;

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
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_STACK_CONTENT);
            return(nullptr);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(nullptr);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simUnpackTable_internal(simInt stackHandle,const simChar* buffer,simInt bufferSize)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
        if (stack!=nullptr)
        {
            if (stack->pushTableFromBuffer(buffer,bufferSize))
                return(0);
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DATA);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
        return(-1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetVisionSensorFilter_internal(simInt visionSensorHandle,simInt filterIndex,simInt options,const simInt* pSizes,const simUChar* bytes,const simInt* ints,const simFloat* floats,const simUChar* custom)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isVisionSensor(__func__,visionSensorHandle))
            return(-1);

        CVisionSensor* it=App::ct->objCont->getVisionSensor(visionSensorHandle);
        CComposedFilter* cf=it->getComposedFilter();
        if (cf!=nullptr)
        {
            if ( (filterIndex>=0)&&(filterIndex<cf->getSimpleFilterCount()) )
            {
                CSimpleFilter* sf=cf->getSimpleFilter(filterIndex);
                sf->setEnabled((options&1)!=0);
                std::vector<unsigned char> byteP;
                std::vector<unsigned char> custP;
                std::vector<int> intP;
                std::vector<float> floatP;
                int fVersion;
                sf->getParameters(byteP,intP,floatP,fVersion);
                sf->getCustomFilterParameters(custP);
                int bs=int(byteP.size());
                int is=int(intP.size());
                int fs=int(floatP.size());
                int cs=int(custP.size());
                if (bs>pSizes[0])
                    bs=pSizes[0];
                if (is>pSizes[1])
                    is=pSizes[1];
                if (fs>pSizes[2])
                    fs=pSizes[2];
                if (cs>pSizes[3])
                    cs=pSizes[3];
                for (int i=0;i<bs;i++)
                    byteP[i]=bytes[i];
                for (int i=0;i<is;i++)
                    intP[i]=ints[i];
                for (int i=0;i<fs;i++)
                    floatP[i]=floats[i];
                for (int i=0;i<cs;i++)
                    custP[i]=custom[i];
                sf->setParameters(byteP,intP,floatP,fVersion);
                sf->setCustomFilterParameters(custP);
                return(sf->getFilterType());
            }
        }
        return(0); // kind of silent error
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetVisionSensorFilter_internal(simInt visionSensorHandle,simInt filterIndex,simInt* options,simInt* pSizes,simUChar** bytes,simInt** ints,simFloat** floats,simUChar** custom)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isVisionSensor(__func__,visionSensorHandle))
            return(-1);

        CVisionSensor* it=App::ct->objCont->getVisionSensor(visionSensorHandle);
        CComposedFilter* cf=it->getComposedFilter();
        if (cf!=nullptr)
        {
            if ( (filterIndex>=0)&&(filterIndex<cf->getSimpleFilterCount()) )
            {
                CSimpleFilter* sf=cf->getSimpleFilter(filterIndex);
                options[0]=0;
                if (sf->getEnabled())
                    options[0]|=1;
                std::vector<unsigned char> byteP;
                std::vector<unsigned char> custP;
                std::vector<int> intP;
                std::vector<float> floatP;
                int fVersion;
                sf->getParameters(byteP,intP,floatP,fVersion);
                sf->getCustomFilterParameters(custP);
                pSizes[0]=int(byteP.size());
                pSizes[1]=int(intP.size());
                pSizes[2]=int(floatP.size());
                pSizes[3]=int(custP.size());
                unsigned char* _b=new unsigned char[pSizes[0]];
                int* _i=new int[pSizes[1]];
                float* _f=new float[pSizes[2]];
                unsigned char* _c=new unsigned char[pSizes[3]];
                for (int i=0;i<pSizes[0];i++)
                    _b[i]=byteP[i];
                for (int i=0;i<pSizes[1];i++)
                    _i[i]=intP[i];
                for (int i=0;i<pSizes[2];i++)
                    _f[i]=floatP[i];
                for (int i=0;i<pSizes[3];i++)
                    _c[i]=custP[i];
                bytes[0]=_b;
                ints[0]=_i;
                floats[0]=_f;
                custom[0]=_c;
                return(sf->getFilterType());
            }
        }
        return(0); // kind of silent error
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetReferencedHandles_internal(simInt objectHandle,simInt count,const simInt* referencedHandles,const simInt* reserved1,const simInt* reserved2)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        if ((handleFlags&sim_handleflag_keeporiginal)==0)
            it->setReferencedHandles(count,referencedHandles);
        else
            it->setReferencedOriginalHandles(count,referencedHandles);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetReferencedHandles_internal(simInt objectHandle,simInt** referencedHandles,simInt** reserved1,simInt** reserved2)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int handleFlags=objectHandle&0xff00000;
        objectHandle=objectHandle&0xfffff;
        if (!doesObjectExist(__func__,objectHandle))
            return(-1);
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int* handles=nullptr;
        int cnt;
        if ((handleFlags&sim_handleflag_keeporiginal)==0)
        {
            cnt=it->getReferencedHandlesCount();
            if (cnt>0)
            {
                handles=new int[cnt];
                it->getReferencedHandles(handles);
            }
        }
        else
        {
            cnt=it->getReferencedOriginalHandlesCount();
            if (cnt>0)
            {
                handles=new int[cnt];
                it->getReferencedOriginalHandles(handles);
            }
        }
        referencedHandles[0]=handles;
        return(cnt);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetShapeViz_internal(simInt shapeHandle,simInt index,struct SShapeVizInfo* info)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!isShape(__func__,shapeHandle))
            return(-1);
        int retVal=0;
        CShape* it=App::ct->objCont->getShape(shapeHandle);
        std::vector<CGeometric*> all;
        it->geomData->geomInfo->getAllShapeComponentsCumulative(all);
        if ( (index>=0)&&(index<int(all.size())) )
        {
            CGeometric* geom=all[index];
            C7Vector tr(geom->getVerticeLocalFrame());
            const std::vector<float>* wvert=geom->getVertices();
            const std::vector<int>* wind=geom->getIndices();
            const std::vector<float>* wnorm=geom->getNormals();
            info->verticesSize=int(wvert->size());
            info->vertices=new float[wvert->size()];
            for (size_t i=0;i<wvert->size()/3;i++)
            {
                C3Vector v(&(wvert[0])[0]+i*3);
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
            for (size_t i=0;i<3;i++)
            {
                info->colors[0+i]=geom->color.colors[0+i]; // ambient-diffuse
                info->colors[3+i]=geom->color.colors[6+i]; // specular
                info->colors[6+i]=geom->color.colors[9+i]; // emission
            }
            info->shadingAngle=geom->getGouraudShadingAngle();
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
                info->textureId=tp->getTextureObjectID();
            }
            else
            {
                retVal=1;
                info->texture=nullptr;
                info->textureCoords=nullptr;
                info->textureId=-1;
            }
            return(retVal);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simExecuteScriptString_internal(simInt scriptHandleOrType,const simChar* stringAtScriptName,simInt stackHandle)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CLuaScriptObject* script=nullptr;
        std::string stringToExecute;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            std::string strAtScriptName(stringAtScriptName);
            size_t p=strAtScriptName.rfind('@');
            if (p!=std::string::npos)
                stringToExecute.assign(strAtScriptName.begin(),strAtScriptName.begin()+p);
            else
                stringToExecute=strAtScriptName;
            script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::ct->sandboxScript;
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                if (scriptName.size()>0)
                {
                    int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
                }
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            {
                int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
        }

        if (script!=nullptr)
        {
            bool noReturnDesired=(stackHandle==0);
            CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
            if ( (stack==nullptr)&&(!noReturnDesired) )
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_HANDLE);
                return(-1);
            }
            int retVal=-1; // error
            if (script->getThreadedExecutionIsUnderWay())
            { // very special handling here!
                if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId(),VThread::getCurrentThreadId()))
                    retVal=script->executeScriptString(stringToExecute.c_str(),stack);
                else
                { // we have to execute that function via another thread!
                    void* d[4];
                    int callType=3;
                    d[0]=&callType;
                    d[1]=script;
                    d[2]=(void*)stringToExecute.c_str();
                    d[3]=stack;

                    retVal=CThreadPool::callRoutineViaSpecificThread(script->getThreadedScriptThreadId(),d);
                }
            }
            else
            {
                if (VThread::isCurrentThreadTheMainSimulationThread())
                { // For now we don't allow non-main threads to call non-threaded scripts!
                    retVal=script->executeScriptString(stringToExecute.c_str(),stack);
                }
            }

            if (retVal==-1)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetApiFunc_internal(simInt scriptHandleOrType,const simChar* apiWord)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int scriptType=-1;
        bool threaded=false;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
            if (script!=nullptr)
            {
                scriptType=script->getScriptType();
                threaded=script->getThreadedExecution();
            }
        }
        else
            scriptType=scriptHandleOrType;
        if ( (scriptType>=0) )//&&(strlen(apiWord)>0) )
        {
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
            std::map<std::string,bool> map;
            if (funcs)
            {
                pushAllVrepFunctionNamesThatStartSame_autoCompletionList(apiW,t,map,scriptType,threaded);
                App::ct->luaCustomFuncAndVarContainer->pushAllFunctionNamesThatStartSame_autoCompletionList(apiW,t,map);
            }
            if (vars)
            {
                pushAllVrepVariableNamesThatStartSame_autoCompletionList(apiW,t,map);
                App::ct->luaCustomFuncAndVarContainer->pushAllVariableNamesThatStartSame_autoCompletionList(apiW,t,map);
            }
            std::sort(t.begin(),t.end());
            std::string theWords;
            for (int i=0;i<int(t.size());i++)
            {
                theWords+=t[i];
                if (i!=int(t.size()-1))
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
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        return(nullptr);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simChar* simGetApiInfo_internal(simInt scriptHandleOrType,const simChar* apiWord)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int scriptType=-1;
        bool threaded=false;
        if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
        { // script is identified by its ID
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
            if (script!=nullptr)
            {
                scriptType=script->getScriptType();
                threaded=script->getThreadedExecution();
            }
        }
        else
            scriptType=scriptHandleOrType;
        if ( (scriptType>=0)&&(strlen(apiWord)>0) )
        {
            std::string tip(getVrepFunctionCalltip(apiWord,scriptType,threaded,true));
            if (tip.size()==0)
            {
                for (size_t i=0;i<App::ct->luaCustomFuncAndVarContainer->allCustomFunctions.size();i++)
                {
                    std::string n=App::ct->luaCustomFuncAndVarContainer->allCustomFunctions[i]->getFunctionName();
                    if (n.compare(apiWord)==0)
                    {
                        tip=App::ct->luaCustomFuncAndVarContainer->allCustomFunctions[i]->getCallTips();
                        break;
                    }
                }
            }
            char* buff=nullptr;
            if (tip.size()>0)
            {
                buff=new char[tip.size()+1];
                strcpy(buff,tip.c_str());
            }
            return(buff);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENTS);
        return(nullptr);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetModuleInfo_internal(const simChar* moduleName,simInt infoType,const simChar* stringInfo,simInt intInfo)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromName(moduleName);
        if (plug!=nullptr)
        {
            if (infoType==0)
            {
                plug->extendedVersionString=stringInfo;
                return(1);
            }
            if (infoType==1)
            {
                plug->buildDateString=stringInfo;
                return(1);
            }
            if (infoType==2)
            {
                plug->extendedVersionInt=intInfo;
                return(1);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PLUGIN_NAME);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetModuleInfo_internal(const simChar* moduleName,simInt infoType,simChar** stringInfo,simInt* intInfo)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPlugin* plug=CPluginContainer::getPluginFromName(moduleName);
        if (plug!=nullptr)
        {
            if (infoType==0)
            {
                char* txt=new char[plug->extendedVersionString.length()+1];
                strcpy(txt,plug->extendedVersionString.c_str());
                if (stringInfo!=nullptr)
                    stringInfo[0]=txt;
                else
                    delete[] txt;
                return(1);
            }
            if (infoType==1)
            {
                char* txt=new char[plug->buildDateString.length()+1];
                strcpy(txt,plug->buildDateString.c_str());
                if (stringInfo!=nullptr)
                    stringInfo[0]=txt;
                else
                    delete[] txt;
                return(1);
            }
            if (infoType==2)
            {
                intInfo[0]=plug->extendedVersionInt;
                return(1);
            }
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(-1);
        }
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_PLUGIN_NAME);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simIsDeprecated_internal(const simChar* funcOrConst)
{
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=isFuncOrConstDeprecated(funcOrConst);
        if (retVal<0)
            retVal=App::ct->luaCustomFuncAndVarContainer->isFuncOrConstDeprecated(funcOrConst);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}
simChar* simGetPersistentDataTags_internal(simInt* tagCount)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(nullptr);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        std::vector<std::string> allTags;
        tagCount[0]=App::ct->persistentDataContainer->getAllDataNames(allTags);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(nullptr);
}

simInt simEventNotification_internal(const simChar* event)
{
    C_API_FUNCTION_DEBUG;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        tinyxml2::XMLDocument xmldoc;
        tinyxml2::XMLError error=xmldoc.Parse(event);
        if(error==tinyxml2::XML_NO_ERROR)
        {
            tinyxml2::XMLElement* rootElement=xmldoc.FirstChildElement();
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
                                    CInterfaceStack* stack=new CInterfaceStack();
                                    int posAndSize[4];
                                    std::string txt=App::mainWindow->codeEditorContainer->getText(h,posAndSize);
                                    stack->pushStringOntoStack(txt.c_str(),0);
                                    stack->pushIntArrayTableOntoStack(posAndSize+0,2);
                                    stack->pushIntArrayTableOntoStack(posAndSize+2,2);
                                    int stackId=App::ct->interfaceStackContainer->addStack(stack);
                                    simCallScriptFunctionEx_internal(callingScript,data,stackId);
                                    App::ct->interfaceStackContainer->destroyStack(stackId);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simApplyTexture_internal(simInt shapeHandle,const simFloat* textureCoordinates,simInt textCoordSize,const simUChar* texture,const simInt* textureResolution,simInt options)
{
    C_API_FUNCTION_DEBUG;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isShape(__func__,shapeHandle))
        {
            CShape* shape=App::ct->objCont->getShape(shapeHandle);
            if (shape->geomData->geomInfo->isGeometric())
            {
                // first remove any existing texture:
                CTextureProperty* tp=((CGeometric*)shape->geomData->geomInfo)->getTextureProperty();
                if (tp!=nullptr)
                {
                    App::ct->textureCont->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                    delete tp;
                    ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(nullptr);
                }
                if (((CGeometric*)shape->geomData->geomInfo)->getIndices()->size()*2==textCoordSize)
                {
                    // Now create and attach the texture:
                    CTextureObject* textureObj=new CTextureObject(textureResolution[0],textureResolution[1]);
                    textureObj->setImage(options&16,options&32,(options&64)==0,texture);
                    textureObj->setObjectName("importedTexture");
                    textureObj->addDependentObject(shape->getObjectHandle(),((CGeometric*)shape->geomData->geomInfo)->getUniqueID());
                    retVal=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                    tp=new CTextureProperty(retVal);
                    ((CGeometric*)shape->geomData->geomInfo)->setTextureProperty(tp);
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
                    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_BAD_TEXTURE_COORD_SIZE);
            }
            else
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CANNOT_BE_COMPOUND_SHAPE);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simSetJointDependency_internal(simInt jointHandle,simInt masterJointHandle,simFloat offset,simFloat coeff)
{
    C_API_FUNCTION_DEBUG;
    int retVal=-1;

    if (!isSimulatorInitialized(__func__))
        return(retVal);

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (isJoint(__func__,jointHandle))
        {
            if ( (masterJointHandle==-1)||isJoint(__func__,masterJointHandle) )
            {
                CJoint* joint=App::ct->objCont->getJoint(jointHandle);
                if (joint->setDependencyJointID(masterJointHandle))
                {
                    joint->setDependencyJointOffset(offset);
                    joint->setDependencyJointCoeff(coeff);
                    retVal=0;
                    return(retVal);
                }
            }
        }
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(retVal);
}

simInt simSetStringNamedParam_internal(const simChar* paramName,const simChar* stringParam,simInt paramLength)
{
    C_API_FUNCTION_DEBUG;
    int retVal=-1;
    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        retVal=App::setApplicationNamedParam(paramName,stringParam,paramLength);
        if (retVal>=0)
            return(retVal);
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

simChar* simGetStringNamedParam_internal(const simChar* paramName,simInt* paramLength)
{
    C_API_FUNCTION_DEBUG;
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
                paramLength[0]=val.size();
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(retVal);
}

//************************************************************************************************************
//************************************************************************************************************
// FOLLOWING FUNCTIONS ARE VERY FAST, BUT NO SPECIFIC CHECKING IS DONE. ALSO, MANY OPERATE ON OBJECT POINTERS!
//************************************************************************************************************
//************************************************************************************************************


const simVoid* _simGetGeomWrapFromGeomProxy_internal(const simVoid* geomData)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomProxy*)geomData)->geomInfo);
}

simVoid _simGetLocalInertiaFrame_internal(const simVoid* geomInfo,simFloat* pos,simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr(((CGeomWrap*)geomInfo)->getLocalInertiaFrame());
    tr.Q.getInternalData(quat);
    tr.X.getInternalData(pos);
}

simVoid _simGetPrincipalMomentOfInertia_internal(const simVoid* geomInfo,simFloat* inertia)
{
    C_API_FUNCTION_DEBUG;
    ((CGeomWrap*)geomInfo)->getPrincipalMomentsOfInertia().getInternalData(inertia);
}


simInt _simGetPurePrimitiveType_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomWrap*)geomInfo)->getPurePrimitiveType());
}

simVoid _simGetPurePrimitiveSizes_internal(const simVoid* geometric,simFloat* sizes)
{
    C_API_FUNCTION_DEBUG;
    C3Vector s;
    ((CGeometric*)geometric)->getPurePrimitiveSizes(s);
    s.getInternalData(sizes);
}

simBool _simIsGeomWrapGeometric_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomWrap*)geomInfo)->isGeometric());
}

simBool _simIsGeomWrapConvex_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomWrap*)geomInfo)->isConvex());
}

simInt _simGetGeometricCount_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    std::vector<CGeometric*> all;
    ((CGeomWrap*)geomInfo)->getAllShapeComponentsCumulative(all);
    return((int)all.size());
}

simVoid _simGetAllGeometrics_internal(const simVoid* geomInfo,simVoid** allGeometrics)
{
    C_API_FUNCTION_DEBUG;
    std::vector<CGeometric*> all;
    ((CGeomWrap*)geomInfo)->getAllShapeComponentsCumulative(all);
    for (size_t i=0;i<all.size();i++)
        allGeometrics[i]=all[i];
}

simVoid _simMakeDynamicAnnouncement_internal(int announceType)
{
    C_API_FUNCTION_DEBUG;
    if (announceType==sim_announce_pureconenotsupported)
        App::ct->dynamicsContainer->markForWarningDisplay_pureConeNotSupported();
    if (announceType==sim_announce_purespheroidnotsupported)
        App::ct->dynamicsContainer->markForWarningDisplay_pureSpheroidNotSupported();
    if (announceType==sim_announce_newtondynamicrandommeshnotsupported)
        App::ct->dynamicsContainer->markForWarningDisplay_newtonDynamicRandomMeshNotSupported();
    if (announceType==sim_announce_containsnonpurenonconvexshapes)
        App::ct->dynamicsContainer->markForWarningDisplay_containsNonPureNonConvexShapes();
    if (announceType==sim_announce_containsstaticshapesondynamicconstruction)
        App::ct->dynamicsContainer->markForWarningDisplay_containsStaticShapesOnDynamicConstruction();
    if (announceType==sim_announce_purehollowshapenotsupported)
        App::ct->dynamicsContainer->markForWarningDisplay_pureHollowShapeNotSupported();
    if (announceType==sim_announce_vortexpluginisdemo)
        App::ct->dynamicsContainer->markForWarningDisplay_vortexPluginIsDemo();
}

simVoid _simGetVerticesLocalFrame_internal(const simVoid* geometric,simFloat* pos,simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr(((CGeometric*)geometric)->getVerticeLocalFrame());
    tr.Q.getInternalData(quat);
    tr.X.getInternalData(pos);
}

const simFloat* _simGetHeightfieldData_internal(const simVoid* geometric,simInt* xCount,simInt* yCount,simFloat* minHeight,simFloat* maxHeight)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeometric*)geometric)->getHeightfieldData(xCount[0],yCount[0],minHeight[0],maxHeight[0]));
}

simVoid _simGetCumulativeMeshes_internal(const simVoid* geomInfo,simFloat** vertices,simInt* verticesSize,simInt** indices,simInt* indicesSize)
{
    C_API_FUNCTION_DEBUG;
    std::vector<float> vert;
    std::vector<int> ind;
    ((CGeomWrap*)geomInfo)->getCumulativeMeshes(vert,&ind,nullptr);

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
    C_API_FUNCTION_DEBUG;
    return(((C3DObject*)object)->getObjectHandle());
}

simVoid _simGetObjectLocalTransformation_internal(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    if (excludeFirstJointTransformation)
        tr=((C3DObject*)object)->getLocalTransformationPart1();
    else
        tr=((C3DObject*)object)->getLocalTransformation();
    tr.X.getInternalData(pos);
    tr.Q.getInternalData(quat);
}

simVoid _simSetObjectLocalTransformation_internal(simVoid* object,const simFloat* pos,const simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    ((C3DObject*)object)->setLocalTransformation(tr);
}

simVoid _simGetObjectCumulativeTransformation_internal(const simVoid* object,simFloat* pos,simFloat* quat,simBool excludeFirstJointTransformation)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    if (excludeFirstJointTransformation!=0)
        tr=((C3DObject*)object)->getCumulativeTransformationPart1();
    else
        tr=((C3DObject*)object)->getCumulativeTransformation();
    if (pos!=nullptr)
        tr.X.getInternalData(pos);
    if (quat!=nullptr)
        tr.Q.getInternalData(quat);
}

const simVoid* _simGetGeomProxyFromShape_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->geomData);
}

simFloat _simGetMass_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomWrap*)geomInfo)->getMass());
}

simBool _simIsShapeDynamicallyStatic_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getShapeIsDynamicallyStatic());
}

simVoid _simGetInitialDynamicVelocity_internal(const simVoid* shape,simFloat* vel)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->getInitialDynamicLinearVelocity().getInternalData(vel);
}

simVoid _simSetInitialDynamicVelocity_internal(simVoid* shape,const simFloat* vel)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->setInitialDynamicLinearVelocity(C3Vector(vel));
}

simVoid _simGetInitialDynamicAngVelocity_internal(const simVoid* shape,simFloat* angularVel)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->getInitialDynamicAngularVelocity().getInternalData(angularVel);
}

simVoid _simSetInitialDynamicAngVelocity_internal(simVoid* shape,const simFloat* angularVel)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->setInitialDynamicAngularVelocity(C3Vector(angularVel));
}

simBool _simGetStartSleeping_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getStartInDynamicSleeping());
}

simBool _simGetWasPutToSleepOnce_internal(const simVoid* shape)
{ // flag is set to true whenever called!!!
    C_API_FUNCTION_DEBUG;
    bool a=((CShape*)shape)->getRigidBodyWasAlreadyPutToSleepOnce();
    ((CShape*)shape)->setRigidBodyWasAlreadyPutToSleepOnce(true);
    return(a);
}

simBool _simIsShapeDynamicallyRespondable_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getRespondable());
}

simInt _simGetDynamicCollisionMask_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getDynamicCollisionMask());
}

const simVoid* _simGetLastParentForLocalGlobalCollidable_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getLastParentForLocalGlobalCollidable());
}

simVoid _simSetGeomProxyDynamicsFullRefreshFlag_internal(simVoid* geomData,simBool flag)
{
    C_API_FUNCTION_DEBUG;
    ((CGeomProxy*)geomData)->setDynamicsFullRefreshFlag(flag!=0);
}

simBool _simGetGeomProxyDynamicsFullRefreshFlag_internal(const simVoid* geomData)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeomProxy*)geomData)->getDynamicsFullRefreshFlag());
}

simBool _simGetDynamicsFullRefreshFlag_internal(const simVoid* object)
{
    C_API_FUNCTION_DEBUG;
    return(((C3DObject*)object)->getDynamicsFullRefreshFlag());
}

simVoid _simSetDynamicsFullRefreshFlag_internal(const simVoid* object,simBool flag)
{
    C_API_FUNCTION_DEBUG;
    ((C3DObject*)object)->setDynamicsFullRefreshFlag(flag!=0);
}


simBool _simGetParentFollowsDynamic_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getParentFollowsDynamic());
}

const simVoid* _simGetParentObject_internal(const simVoid* object)
{
    C_API_FUNCTION_DEBUG;
    return(((C3DObject*)object)->getParentObject());
}

simVoid _simSetObjectCumulativeTransformation_internal(simVoid* object,const simFloat* pos,const simFloat* quat,simBool keepChildrenInPlace)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    App::ct->objCont->setAbsoluteConfiguration(((C3DObject*)object)->getObjectHandle(),tr,keepChildrenInPlace!=0);
}

simVoid _simSetShapeDynamicVelocity_internal(simVoid* shape,const simFloat* linear,const simFloat* angular)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->setDynamicVelocity(linear,angular);
}

simVoid _simGetAdditionalForceAndTorque_internal(const simVoid* shape,simFloat* force,simFloat* torque)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->getAdditionalForce().getInternalData(force);
    ((CShape*)shape)->getAdditionalTorque().getInternalData(torque);
}

simVoid _simClearAdditionalForceAndTorque_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->clearAdditionalForceAndTorque();
}

simBool _simGetJointPositionInterval_internal(const simVoid* joint,simFloat* minValue,simFloat* rangeValue)
{
    C_API_FUNCTION_DEBUG;
    if (minValue!=nullptr)
        minValue[0]=((CJoint*)joint)->getPositionIntervalMin();
    if (rangeValue!=nullptr)
        rangeValue[0]=((CJoint*)joint)->getPositionIntervalRange();
    return(!((CJoint*)joint)->getPositionIsCyclic());
}

const simVoid* _simGetObject_internal(int objID)
{
    C_API_FUNCTION_DEBUG;
    return(App::ct->objCont->getObjectFromHandle(objID));
}

const simVoid* _simGetIkGroupObject_internal(int ikGroupID)
{
    C_API_FUNCTION_DEBUG;
    return(App::ct->ikGroups->getIkGroup(ikGroupID));
}

simInt _simMpHandleIkGroupObject_internal(const simVoid* ikGroup)
{
    C_API_FUNCTION_DEBUG;
    return(((CikGroup*)ikGroup)->computeGroupIk(true));
}

simInt _simGetJointType_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getJointType());
}

simBool _simIsForceSensorBroken_internal(const simVoid* forceSensor)
{
    C_API_FUNCTION_DEBUG;
    return(((CForceSensor*)forceSensor)->getForceSensorIsBroken());
}

simVoid _simGetDynamicForceSensorLocalTransformationPart2_internal(const simVoid* forceSensor,simFloat* pos,simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr(((CForceSensor*)forceSensor)->getDynamicSecondPartLocalTransform());
    tr.X.getInternalData(pos);
    tr.Q.getInternalData(quat);
}

simBool _simIsDynamicMotorEnabled_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getEnableDynamicMotor());
}

simBool _simIsDynamicMotorPositionCtrlEnabled_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getEnableDynamicMotorControlLoop());
}

simVoid _simGetMotorPid_internal(const simVoid* joint,simFloat* pParam,simFloat* iParam,simFloat* dParam)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->getDynamicMotorPositionControlParameters(pParam[0],iParam[0],dParam[0]);
}

simFloat _simGetDynamicMotorTargetPosition_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getDynamicMotorPositionControlTargetPosition());
}

simFloat _simGetDynamicMotorTargetVelocity_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getDynamicMotorTargetVelocity());
}

simBool _simIsDynamicMotorTorqueModulationEnabled_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getEnableTorqueModulation());
}

simFloat _simGetDynamicMotorMaxForce_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getDynamicMotorMaximumForce());
}

simFloat _simGetDynamicMotorUpperLimitVelocity_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getDynamicMotorUpperLimitVelocity());
}

simVoid _simSetDynamicMotorReflectedPositionFromDynamicEngine_internal(simVoid* joint,simFloat pos)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(pos);
}

simVoid _simSetJointSphericalTransformation_internal(simVoid* joint,const simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->setSphericalTransformation(quat);
}

simVoid _simSetDynamicJointLocalTransformationPart2_internal(simVoid* joint,const simFloat* pos,const simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    ((CJoint*)joint)->setDynamicSecondPartLocalTransform(tr);
}

simVoid _simSetDynamicForceSensorLocalTransformationPart2_internal(simVoid* forceSensor,const simFloat* pos,const simFloat* quat)
{
    C_API_FUNCTION_DEBUG;
    C7Vector tr;
    tr.X.setInternalData(pos);
    tr.Q.setInternalData(quat);
    ((CForceSensor*)forceSensor)->setDynamicSecondPartLocalTransform(tr);
}

simVoid _simSetDynamicJointLocalTransformationPart2IsValid_internal(simVoid* joint,simBool valid)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->setDynamicSecondPartIsValid(valid!=0);
}

simVoid _simSetDynamicForceSensorLocalTransformationPart2IsValid_internal(simVoid* forceSensor,simBool valid)
{
    C_API_FUNCTION_DEBUG;
    ((CForceSensor*)forceSensor)->setDynamicSecondPartIsValid(valid!=0);
}

simVoid _simAddForceSensorCumulativeForcesAndTorques_internal(simVoid* forceSensor,const simFloat* force,const simFloat* torque,int totalPassesCount)
{
    C_API_FUNCTION_DEBUG;
    ((CForceSensor*)forceSensor)->addCumulativeForcesAndTorques(force,torque,totalPassesCount);
}

simVoid _simAddJointCumulativeForcesOrTorques_internal(simVoid* joint,simFloat forceOrTorque,int totalPassesCount)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->addCumulativeForceOrTorque(forceOrTorque,totalPassesCount);
}

simInt _simGetObjectListSize_internal(simInt objType)
{
    C_API_FUNCTION_DEBUG;
    if (objType==sim_object_shape_type)
        return(int(App::ct->objCont->shapeList.size()));
    if (objType==sim_object_joint_type)
        return(int(App::ct->objCont->jointList.size()));
    if (objType==sim_handle_all)
        return(int(App::ct->objCont->objectList.size())); // we put it also here for faster access!
    if (objType==sim_object_dummy_type)
        return(int(App::ct->objCont->dummyList.size()));
    if (objType==sim_object_octree_type)
        return(int(App::ct->objCont->octreeList.size()));
    if (objType==sim_object_pointcloud_type)
        return(int(App::ct->objCont->pointCloudList.size()));
    if (objType==sim_object_graph_type)
        return(int(App::ct->objCont->graphList.size()));
    if (objType==sim_object_camera_type)
        return(int(App::ct->objCont->cameraList.size()));
    if (objType==sim_object_proximitysensor_type)
        return(int(App::ct->objCont->proximitySensorList.size()));
    if (objType==sim_object_path_type)
        return(int(App::ct->objCont->pathList.size()));
    if (objType==sim_object_visionsensor_type)
        return(int(App::ct->objCont->visionSensorList.size()));
    if (objType==sim_object_mill_type)
        return(int(App::ct->objCont->millList.size()));
    if (objType==sim_object_forcesensor_type)
        return(int(App::ct->objCont->forceSensorList.size()));
    if (objType==sim_object_light_type)
        return(int(App::ct->objCont->lightList.size()));
    if (objType==sim_object_mirror_type)
        return(int(App::ct->objCont->mirrorList.size()));
    if (objType==-1)
        return(int(App::ct->objCont->orphanList.size()));
    return(int(App::ct->objCont->objectList.size()));
}

const simVoid* _simGetObjectFromIndex_internal(simInt objType,simInt index)
{
    C_API_FUNCTION_DEBUG;
    if (objType==sim_object_shape_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->shapeList[index]));
    if (objType==sim_object_joint_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->jointList[index]));
    if (objType==sim_handle_all)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[index])); // we put it also here for faster access!
    if (objType==sim_object_dummy_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->dummyList[index]));
    if (objType==sim_object_octree_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->octreeList[index]));
    if (objType==sim_object_pointcloud_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->pointCloudList[index]));
    if (objType==sim_object_graph_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->graphList[index]));
    if (objType==sim_object_camera_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->cameraList[index]));
    if (objType==sim_object_proximitysensor_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->proximitySensorList[index]));
    if (objType==sim_object_path_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->pathList[index]));
    if (objType==sim_object_visionsensor_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->visionSensorList[index]));
    if (objType==sim_object_mill_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->millList[index]));
    if (objType==sim_object_forcesensor_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->forceSensorList[index]));
    if (objType==sim_object_light_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->lightList[index]));
    if (objType==sim_object_mirror_type)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->mirrorList[index]));
    if (objType==-1)
        return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->orphanList[index]));
    return(App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[index]));
}

simInt _simGetContactCallbackCount_internal()
{
    C_API_FUNCTION_DEBUG;
    return((int)allContactCallbacks.size());
}

const void* _simGetContactCallback_internal(simInt index)
{
    C_API_FUNCTION_DEBUG;
    return((const void*)allContactCallbacks[index]);
}

simVoid _simSetDynamicSimulationIconCode_internal(simVoid* object,simInt code)
{
    C_API_FUNCTION_DEBUG;
    ((C3DObject*)object)->setDynamicSimulationIconCode(code);
}

simVoid _simSetDynamicObjectFlagForVisualization_internal(simVoid* object,simInt flag)
{
    C_API_FUNCTION_DEBUG;
    ((C3DObject*)object)->setDynamicObjectFlag_forVisualization(flag);
}

simInt _simGetTreeDynamicProperty_internal(const simVoid* object)
{
    C_API_FUNCTION_DEBUG;
    return(((C3DObject*)object)->getTreeDynamicProperty());
}

simInt _simGetObjectType_internal(const simVoid* object)
{
    C_API_FUNCTION_DEBUG;
    return(((C3DObject*)object)->getObjectType());
}

simVoid _simSetShapeIsStaticAndNotRespondableButDynamicTag_internal(const simVoid* shape,simBool tag)
{
    C_API_FUNCTION_DEBUG;
    ((CShape*)shape)->setShapeIsStaticAndNotRespondableButDynamicTag(tag!=0);
}

simBool _simGetShapeIsStaticAndNotRespondableButDynamicTag_internal(const simVoid* shape)
{
    C_API_FUNCTION_DEBUG;
    return(((CShape*)shape)->getShapeIsStaticAndNotRespondableButDynamicTag());
}

const simVoid** _simGetObjectChildren_internal(const simVoid* object,simInt* count)
{
    C_API_FUNCTION_DEBUG;
    C3DObject* it=(C3DObject*)object;
    count[0]=int(it->childList.size());
    if (count[0]!=0)
        return((const void**)&it->childList[0]);
    return(nullptr);
}

simInt _simGetDummyLinkType_internal(const simVoid* dummy,simInt* linkedDummyID)
{
    C_API_FUNCTION_DEBUG;
    int dType=((CDummy*)dummy)->getLinkType();
    if (linkedDummyID!=nullptr)
        linkedDummyID[0]=((CDummy*)dummy)->getLinkedDummyID();
    return(dType);
}

simInt _simGetJointMode_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getJointMode());
}

simBool _simIsJointInHybridOperation_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getHybridFunctionality());
}

simVoid _simDisableDynamicTreeForManipulation_internal(const simVoid* object,simBool disableFlag)
{
    C_API_FUNCTION_DEBUG;
    ((C3DObject*)object)->disableDynamicTreeForManipulation(disableFlag!=0);
}

simVoid _simSetJointPosition_internal(const simVoid* joint,simFloat pos)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->setPosition(pos,false);
}

simFloat _simGetJointPosition_internal(const simVoid* joint)
{
    C_API_FUNCTION_DEBUG;
    return(((CJoint*)joint)->getPosition());
}

simVoid _simSetDynamicMotorPositionControlTargetPosition_internal(const simVoid* joint,simFloat pos)
{
    C_API_FUNCTION_DEBUG;
    ((CJoint*)joint)->setDynamicMotorPositionControlTargetPosition(pos);
}

simVoid _simGetGravity_internal(simFloat* gravity)
{
    C_API_FUNCTION_DEBUG;
    App::ct->dynamicsContainer->getGravity().getInternalData(gravity);
}

simInt _simGetTimeDiffInMs_internal(simInt previousTime)
{
    C_API_FUNCTION_DEBUG;
    return(VDateTime::getTimeDiffInMs(previousTime));
}

simBool _simDoEntitiesCollide_internal(simInt entity1ID,simInt entity2ID,simInt* cacheBuffer,simBool overrideCollidableFlagIfShape1,simBool overrideCollidableFlagIfShape2,simBool pathOrMotionPlanningRoutineCalling)
{
    C_API_FUNCTION_DEBUG;
    return(CCollisionRoutine::doEntitiesCollide(entity1ID,entity2ID,nullptr,overrideCollidableFlagIfShape1!=0,overrideCollidableFlagIfShape2!=0,nullptr));
}

simBool _simGetDistanceBetweenEntitiesIfSmaller_internal(simInt entity1ID,simInt entity2ID,simFloat* distance,simFloat* ray,simInt* cacheBuffer,simBool overrideMeasurableFlagIfNonCollection1,simBool overrideMeasurableFlagIfNonCollection2,simBool pathPlanningRoutineCalling)
{
    C_API_FUNCTION_DEBUG;
    return(CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(entity1ID,entity2ID,distance[0],ray,cacheBuffer,cacheBuffer+2,overrideMeasurableFlagIfNonCollection1!=0,overrideMeasurableFlagIfNonCollection2!=0));
}

simInt _simHandleJointControl_internal(const simVoid* joint,simInt auxV,const simInt* inputValuesInt,const simFloat* inputValuesFloat,simFloat* outputValues)
{
    C_API_FUNCTION_DEBUG;
    float outVelocity=0.0f;
    float outForce=0.0f;
    ((CJoint*)joint)->handleDynJointControl((auxV&1)!=0,inputValuesInt[0],inputValuesInt[1],inputValuesFloat[0],inputValuesFloat[1],inputValuesFloat[2],inputValuesFloat[3],outVelocity,outForce);
    outputValues[0]=outVelocity;
    outputValues[1]=outForce;
    return(2);
}

simInt _simHandleCustomContact_internal(simInt objHandle1,simInt objHandle2,simInt engine,simInt* dataInt,simFloat* dataFloat)
{ // Careful with this function: it can also be called from any other thread (e.g. generated by the physics engine)
    C_API_FUNCTION_DEBUG;

    // 1. We handle the new calling method:
    if ( ((engine&1024)==0)&&App::ct->luaScriptContainer->isContactCallbackFunctionAvailable() ) // the engine flag 1024 means: the calling thread is not the simulation thread. We would have problems with the scripts
    {
        CInterfaceStack inStack;
        inStack.pushTableOntoStack();
        inStack.pushStringOntoStack("handle1",0);
        inStack.pushNumberOntoStack(double(objHandle1));
        inStack.insertDataIntoStackTable();
        inStack.pushStringOntoStack("handle2",0);
        inStack.pushNumberOntoStack(double(objHandle2));
        inStack.insertDataIntoStackTable();
        inStack.pushStringOntoStack("engine",0);
        inStack.pushNumberOntoStack(double(engine));
        inStack.insertDataIntoStackTable();
        CInterfaceStack outStack;
        int retInfo=0;
        App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_childscript,sim_syscb_contactcallback,&inStack,&outStack,&retInfo);
        if (retInfo>0)
            App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_contactcallback,&inStack,&outStack,&retInfo);

        bool ignoreContact;
        if (outStack.getStackMapBoolValue("ignoreContact",ignoreContact))
        {
            dataInt[0]=0;
            if (!ignoreContact)
            {
                bool collisionResponse=false;
                outStack.getStackMapBoolValue("collisionResponse",collisionResponse);
                if (collisionResponse)
                {
                    if (engine==sim_physics_ode)
                    {
                        outStack.getStackMapIntValue("ode.maxContacts",dataInt[1]);
                        outStack.getStackMapIntValue("ode.contactMode",dataInt[2]);
                    }
                    if (engine==sim_physics_bullet)
                    {
                        outStack.getStackMapFloatValue("bullet.friction",dataFloat[0]);
                        outStack.getStackMapFloatValue("bullet.restitution",dataFloat[1]);
                    }
                    if (engine==sim_physics_ode)
                    {
                        outStack.getStackMapFloatValue("ode.mu",dataFloat[0]);
                        outStack.getStackMapFloatValue("ode.mu2",dataFloat[1]);
                        outStack.getStackMapFloatValue("ode.bounce",dataFloat[2]);
                        outStack.getStackMapFloatValue("ode.bounceVel",dataFloat[3]);
                        outStack.getStackMapFloatValue("ode.softCfm",dataFloat[4]);
                        outStack.getStackMapFloatValue("ode.softErp",dataFloat[5]);
                        outStack.getStackMapFloatValue("ode.motion1",dataFloat[6]);
                        outStack.getStackMapFloatValue("ode.motion2",dataFloat[7]);
                        outStack.getStackMapFloatValue("ode.motionN",dataFloat[8]);
                        outStack.getStackMapFloatValue("ode.slip1",dataFloat[9]);
                        outStack.getStackMapFloatValue("ode.slip2",dataFloat[10]);
                        outStack.getStackMapFloatArray("ode.fDir1",dataFloat+11,3);
                    }
                    if (engine==sim_physics_vortex)
                    {
                        //outStack.getStackMapFloatValue("vortex.xxxx",dataFloat[0]);
                    }
                    if (engine==sim_physics_newton)
                    {
                        outStack.getStackMapFloatValue("newton.staticFriction",dataFloat[0]);
                        outStack.getStackMapFloatValue("newton.kineticFriction",dataFloat[1]);
                        outStack.getStackMapFloatValue("newton.restitution",dataFloat[2]);
                    }
                    return(1); // collision
                }
                else
                    return(0); // no collision
            }
        }
    }

    // 2. We check if a plugin wants to handle the contact:
    size_t callbackCount=allContactCallbacks.size();
    if (callbackCount!=0)
    {
        for (size_t i=0;i<callbackCount;i++)
        {
            int res=((contactCallback)allContactCallbacks[i])(objHandle1,objHandle2,engine,dataInt,dataFloat);
            if (res==0)
                return(0); // override... we don't wanna collide
            if (res>0)
                return(1); // override... we want the custom values
        }
    }
    return(-1); // we let V-REP handle the contact
}

simFloat _simGetPureHollowScaling_internal(const simVoid* geometric)
{
    C_API_FUNCTION_DEBUG;
    return(((CGeometric*)geometric)->getPurePrimitiveInsideScaling());
}

simVoid _simDynCallback_internal(const simInt* intData,const simFloat* floatData)
{
    C_API_FUNCTION_DEBUG;

    CInterfaceStack inStack;
    if (App::ct->luaScriptContainer->isDynCallbackFunctionAvailable())
    { // to make it a bit faster than blindly parsing the whole object hierarchy
        inStack.pushTableOntoStack();
        inStack.pushStringOntoStack("passCnt",0);
        inStack.pushNumberOntoStack(double(intData[1]));
        inStack.insertDataIntoStackTable();

        inStack.pushStringOntoStack("totalPasses",0);
        inStack.pushNumberOntoStack(double(intData[2]));
        inStack.insertDataIntoStackTable();

        inStack.pushStringOntoStack("dynStepSize",0);
        inStack.pushNumberOntoStack(double(floatData[0]));
        inStack.insertDataIntoStackTable();

        inStack.pushStringOntoStack("afterStep",0);
        inStack.pushBoolOntoStack(intData[3]!=0);
        inStack.insertDataIntoStackTable();

        App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_childscript,sim_syscb_dyncallback,&inStack,nullptr,nullptr);
        App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_dyncallback,&inStack,nullptr,nullptr);
    }
}


//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************

simInt simGetMaterialId_internal(const simChar* materialName)
{ // DEPRECATED since 29/10/2016.
    C_API_FUNCTION_DEBUG;
    // For backward compatibility (28/10/2016)
    // We now do not share materials anymore: each shape has its own material, so
    // the material of a shape is identified by the shape handle itself
    return(-1);
}

simInt simGetShapeMaterial_internal(simInt shapeHandle)
{ // DEPRECATED since 29/10/2016.
    C_API_FUNCTION_DEBUG;

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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleVarious_internal()
{ // DEPRECATED since 29/10/2016. use simHandleSimulationStart and simHandleSensingStart instead!
    // HandleVarious should be the last function call before the sensing phase
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        // Following is for camera tracking!
        for (size_t i=0;i<App::ct->objCont->cameraList.size();i++)
        {
            CCamera*  it=App::ct->objCont->getCamera(App::ct->objCont->cameraList[i]);
            it->handleTrackingAndHeadAlwaysUp();
        }

        // Following is for velocity measurement:
        float dt=float(App::ct->simulation->getSimulationTimeStep_speedModified_ns())/1000000.0f;
        for (size_t i=0;i<App::ct->objCont->jointList.size();i++)
            App::ct->objCont->getJoint(App::ct->objCont->jointList[i])->measureJointVelocity(dt);
        for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
            App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i])->measureVelocity(dt);

        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetMpConfigForTipPose_internal(simInt motionPlanningObjectHandle,simInt options,simFloat closeNodesDistance,simInt trialCount,const simFloat* tipPose,simInt maxTimeInMs,simFloat* outputJointPositions,const simFloat* referenceConfigs,simInt referenceConfigCount,const simFloat* jointWeights,const simInt* jointBehaviour,simInt correctionPasses)
{ // DEPRECATED since 21/1/2016   referenceConfigs can be nullptr, as well as jointWeights
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        // V-REP quaternion, internally: w x y z
        // V-REP quaternion, at interfaces: x y z w
        C7Vector tipTr;
        tipTr.X.set(tipPose);
        tipTr.Q(0)=tipPose[6];
        tipTr.Q(1)=tipPose[3];
        tipTr.Q(2)=tipPose[4];
        tipTr.Q(3)=tipPose[5];

        int retVal=it->getRobotConfigFromTipPose(tipTr,options,outputJointPositions,trialCount,closeNodesDistance,jointWeights,referenceConfigs,referenceConfigCount,jointBehaviour,correctionPasses,maxTimeInMs);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}




simInt simResetPath_internal(simInt pathHandle)
{ // DEPRECATED
    C_API_FUNCTION_DEBUG;

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
        { // Explicit handling
            CPath* it=App::ct->objCont->getPath(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetPath();
        }
        else
        {
            for (int i=0;i<int(App::ct->objCont->pathList.size());i++)
            {
                CPath* p=App::ct->objCont->getPath(App::ct->objCont->pathList[i]);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->resetPath();
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandlePath_internal(simInt pathHandle,simFloat deltaTime)
{ // DEPRECATED
    C_API_FUNCTION_DEBUG;

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
            CPath* it=App::ct->objCont->getPath(pathHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handlePath(deltaTime);
        }
        else
        {
            for (int i=0;i<int(App::ct->objCont->pathList.size());i++)
            {
                CPath* p=App::ct->objCont->getPath(App::ct->objCont->pathList[i]);
                if ( (pathHandle==sim_handle_all)||(!p->getExplicitHandling()) )
                    p->handlePath(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simResetJoint_internal(simInt jointHandle)
{ // DEPRECATED
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
            {
                return(-1);
            }
        }
        if (jointHandle>=0)
        { // Explicit handling
            CJoint* it=App::ct->objCont->getJoint(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->resetJoint_DEPRECATED();
        }
        else
        {
            for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
            {
                CJoint* p=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->resetJoint_DEPRECATED();
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleJoint_internal(simInt jointHandle,simFloat deltaTime)
{ // DEPRECATED
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (jointHandle!=sim_handle_all)&&(jointHandle!=sim_handle_all_except_explicit) )
        {
            if (!isJoint(__func__,jointHandle))
            {
                return(-1);
            }
        }
        if (jointHandle>=0)
        { // explicit handling
            CJoint* it=App::ct->objCont->getJoint(jointHandle);
            if (!it->getExplicitHandling_DEPRECATED())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            it->handleJoint_DEPRECATED(deltaTime);
        }
        else
        {
            for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
            {
                CJoint* p=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
                if ( (jointHandle==sim_handle_all)||(!p->getExplicitHandling_DEPRECATED()) )
                    p->handleJoint_DEPRECATED(deltaTime);
            }
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetPathPlanningHandle_internal(const simChar* pathPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    std::string pathPlanningObjectNameAdjusted=getCNameSuffixAdjustedName(pathPlanningObjectName);
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::ct->pathPlanning->getObject(pathPlanningObjectNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetMotionPlanningHandle_internal(const simChar* motionPlanningObjectName)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    std::string motionPlanningObjectNameAdjusted=getCNameSuffixAdjustedName(motionPlanningObjectName);
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simFloat* simFindMpPath_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(nullptr);
        }

        float* retVal=it->findPath(startConfig,goalConfig,options,stepSize,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simFloat* simSimplifyMpPath_internal(simInt motionPlanningObjectHandle,const simFloat* pathBuffer,simInt configCnt,simInt options,simFloat stepSize,simInt increment,simInt* outputConfigsCnt,simInt maxTimeInMs,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(nullptr);
        }
        if (configCnt<2)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_ARGUMENT);
            return(nullptr);
        }

        float* retVal=it->simplifyPath(pathBuffer,configCnt,options,stepSize,increment,outputConfigsCnt,maxTimeInMs,auxIntParams,auxFloatParams);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simFloat* simFindIkPath_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalPose,simInt options,simFloat stepSize,simInt* outputConfigsCnt,simFloat* reserved,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(nullptr);
        }

        // V-REP quaternion, internally: w x y z
        // V-REP quaternion, at interfaces: x y z w
        C7Vector gPose;
        gPose.X.set(goalPose);
        gPose.Q(0)=goalPose[6];
        gPose.Q(1)=goalPose[3];
        gPose.Q(2)=goalPose[4];
        gPose.Q(3)=goalPose[5];


        float* retVal=it->findIkPath(startConfig,gPose,options,stepSize,outputConfigsCnt,auxIntParams,auxFloatParams);

        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simFloat* simGetMpConfigTransition_internal(simInt motionPlanningObjectHandle,const simFloat* startConfig,const simFloat* goalConfig,simInt options,const simInt* select,simFloat calcStepSize,simFloat maxOutStepSize,simInt wayPointCnt,const simFloat* wayPoints,simInt* outputConfigsCnt,const simInt* auxIntParams,const simFloat* auxFloatParams)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(nullptr);
        }

        float* retVal=nullptr;
        retVal=it->getConfigTransition(startConfig,goalConfig,options,select,calcStepSize,maxOutStepSize,wayPointCnt,wayPoints,outputConfigsCnt,auxIntParams,auxFloatParams);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simCreateMotionPlanning_internal(simInt jointCnt,const simInt* jointHandles,const simInt* jointRangeSubdivisions,const simFloat* jointMetricWeights,simInt options,const simInt* intParams,const simFloat* floatParams,const simVoid* reserved)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (jointCnt<1)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_FIRST_ARGUMENT);
            return(-1);
        }
        std::vector<int> jh;
        for (int i=0;i<jointCnt;i++)
        {
            if (!isJoint(__func__,jointHandles[i]))
            {
                return(-1);
            }
            CJoint* j=App::ct->objCont->getJoint(jointHandles[i]);
            if (j->getJointType()==sim_joint_spherical_subtype)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_JOINT_SPHERICAL);
                return(-1);
            }
            jh.push_back(jointHandles[i]);
        }
        CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(intParams[0]);
        if (ikGroup==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_IK_GROUP_INEXISTANT);
            return(-1);
        }
        CMotionPlanningTask* it=new CMotionPlanningTask(jh);
        for (int i=0;i<jointCnt;i++)
        {
            if (jointRangeSubdivisions!=nullptr)
                it->setJointStepCount(jh[i],jointRangeSubdivisions[i]+1);
            if (jointMetricWeights!=nullptr)
                it->setRobotMetric(jh[i],jointMetricWeights[i]);
        }
        it->setIkGroup(ikGroup->getObjectID());
        it->setRobotSelfCollEntity1(intParams[1]);
        it->setRobotSelfCollEntity2(intParams[2]);
        it->setRobotEntity(intParams[3]);
        it->setObstacleEntity(intParams[4]);
        if (floatParams!=nullptr)
        {
            it->setSelfCollDistanceThreshold(floatParams[0]);
            it->setDistanceThreshold(floatParams[1]);
            it->setTipMetric(0,floatParams[2]);
            it->setTipMetric(1,floatParams[3]);
            it->setTipMetric(2,floatParams[4]);
            it->setTipMetric(3,floatParams[5]);
        }
        App::ct->motionPlanning->addObject(it,false);
        return(it->getObjectID());
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRemoveMotionPlanning_internal(simInt motionPlanningHandle)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->getObject(motionPlanningHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MOTION_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        App::ct->motionPlanning->removeObject(motionPlanningHandle);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSearchPath_internal(simInt pathPlanningObjectHandle,simFloat maximumSearchTime)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::ct->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        int retVal=0;
        if (it->performSearch(false,maximumSearchTime))
            retVal=1;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simInitializePathSearch_internal(simInt pathPlanningObjectHandle,simFloat maximumSearchTime,simFloat searchTimeStep)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::ct->pathPlanning->getObject(pathPlanningObjectHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_INEXISTANT);
            return(-1);
        }
        if (App::ct->pathPlanning->getTemporaryPathSearchObjectCount()>100)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TOO_MANY_TEMP_OBJECTS);
            return(-1);
        }

        maximumSearchTime=tt::getLimitedFloat(0.01f,36000.0f,maximumSearchTime);
        searchTimeStep=tt::getLimitedFloat(0.001f,SIM_MIN(1.0f,maximumSearchTime),searchTimeStep);
        CPathPlanningTask* oldIt=it;
        it=oldIt->copyYourself(); // we copy it because the original might be destroyed at any time
        it->setOriginalTask(oldIt);
        int retVal=-1; // error
        if (it->initiateSteppedSearch(false,maximumSearchTime,searchTimeStep))
            retVal=App::ct->pathPlanning->addTemporaryPathSearchObject(it);
        else
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PATH_PLANNING_OBJECT_NOT_CONSISTENT);
            delete it;
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simPerformPathSearchStep_internal(simInt temporaryPathSearchObject,simBool abortSearch)
{ // DEPRECATED since release 3.3.0
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::ct->pathPlanning->getTemporaryPathSearchObject(temporaryPathSearchObject);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_TEMP_PATH_SEARCH_OBJECT_INEXISTANT);
            return(-1);
        }
        if (abortSearch)
        {
            App::ct->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            delete it;
            return(0);
        }
        int retVal=it->performSteppedSearch();
        if (retVal!=-2)
        {
            App::ct->pathPlanning->removeTemporaryPathSearchObjectButDontDestroyIt(it);
            CPathPlanningTask* originalIt=it->getOriginalTask();
            int tree1Handle,tree2Handle;
            it->getAndDisconnectSearchTrees(tree1Handle,tree2Handle); // to keep trees visible!
            delete it;
            // Now we connect the trees only if the originalTask still exists:
            bool found=false;
            for (int ot=0;ot<int(App::ct->pathPlanning->allObjects.size());ot++)
            {
                if (App::ct->pathPlanning->allObjects[ot]==originalIt)
                {
                    found=true;
                    break;
                }
            }
            if (found)
                originalIt->connectExternalSearchTrees(tree1Handle,tree2Handle);
            else
            {
                App::ct->drawingCont->removeObject(tree1Handle);
                App::ct->drawingCont->removeObject(tree2Handle);
            }
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLockInterface_internal(simBool locked)
{ // DEPRECATED since release 3.1.0
    return(0);
}

simInt simCopyPasteSelectedObjects_internal()
{ // deprecated since 3.1.3
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (ifEditModeActiveGenerateErrorAndReturnTrue(__func__))
        {
            return(-1);
        }
        std::vector<int> sel;
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            sel.push_back(App::ct->objCont->getSelID(i));
        if (fullModelCopyFromApi)
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
        App::ct->copyBuffer->memorizeBuffer();
        App::ct->copyBuffer->copyCurrentSelection(&sel,App::ct->environment->getSceneLocked());
        App::ct->objCont->deselectObjects();
        App::ct->copyBuffer->pasteBuffer(App::ct->environment->getSceneLocked());
        App::ct->objCont->removeFromSelectionAllExceptModelBase(true);
        App::ct->copyBuffer->restoreBuffer();
        App::ct->copyBuffer->clearMemorizedBuffer();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSerialPortOpen_internal(simInt portNumber,simInt baudRate,simVoid* reserved1,simVoid* reserved2)
{ // deprecated (10/04/2012)
    C_API_FUNCTION_DEBUG;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::ct->serialPortContainer->serialPortOpen_old(false,portNumber,baudRate))
        return(1);
#endif
    return(-1);
}

simInt simSerialPortClose_internal(simInt portNumber)
{ // deprecated (10/04/2012)
    C_API_FUNCTION_DEBUG;
    if (!isSimulatorInitialized(__func__))
        return(-1);
#ifdef SIM_WITH_SERIAL
    if (App::ct->serialPortContainer->serialPortClose_old(portNumber))
        return(1);
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(-1);
}

simInt simSerialPortSend_internal(simInt portNumber,const simChar* data,simInt dataLength)
{ // deprecated (10/04/2012)
    C_API_FUNCTION_DEBUG;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::ct->serialPortContainer->serialPortSend_old(portNumber,data,dataLength);
    if (retVal==-1)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

simInt simSerialPortRead_internal(simInt portNumber,simChar* buffer,simInt dataLengthToRead)
{ // deprecated (10/04/2012)
    C_API_FUNCTION_DEBUG;
    if (!isSimulatorInitialized(__func__))
        return(-1);
    int retVal=-1;
#ifdef SIM_WITH_SERIAL
    retVal=App::ct->serialPortContainer->serialPortReceive_old(portNumber,buffer,dataLengthToRead);
    if (retVal==-1)
        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_PORT_NOT_OPEN);
#endif
    return(retVal);
}

simInt simAppendScriptArrayEntry_internal(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* arrayNameAtScriptName,const simChar* keyName,const simChar* data,const simInt* what)
{ // deprecated (23/02/2016)
    C_API_FUNCTION_DEBUG;
    CLuaScriptObject* script=nullptr;

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
            script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
                    script=App::ct->luaScriptContainer->getMainScript();
                if (scriptHandleOrType==sim_scripttype_addonscript)
                {
                    if (scriptName.size()>0)
                        script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
                }
                if (scriptHandleOrType==sim_scripttype_sandboxscript)
                    script=App::ct->sandboxScript;
                if (scriptHandleOrType==sim_scripttype_childscript)
                {
                    if (scriptName.size()>0)
                    { // new way
                        int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                        script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
                    }
                }
                if (scriptHandleOrType==sim_scripttype_customizationscript)
                { // new way only possible (6 was not available in the old way)
                    int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
                }
            }
            else
            { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
                arrayName=arrayNameAtScriptName;
                if (scriptHandleOrType==0) // main script
                    script=App::ct->luaScriptContainer->getMainScript();
                if (scriptHandleOrType==3) // child script
                {
                    int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
                }
                if (scriptHandleOrType==5) // customization
                {
                    int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
                }
            }
        }

        if (script!=nullptr)
        {
            int retVal=script->appendTableEntry(arrayName.c_str(),keyName,data,what);
            if (retVal==-1)
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
            return(retVal);
        }

        CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);
        return(-1);
    }

    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simClearScriptVariable_internal(const simChar* reservedSetToNull,simInt scriptHandleOrType,const simChar* variableNameAtScriptName)
{ // DEPRECATED (23/02/2016)
    C_API_FUNCTION_DEBUG;
    CLuaScriptObject* script=nullptr;

    std::string variableName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string varNameAtScriptName(variableNameAtScriptName);
        size_t p=varNameAtScriptName.find('@');
        if (p!=std::string::npos)
            variableName.assign(varNameAtScriptName.begin(),varNameAtScriptName.begin()+p);
        else
            variableName=varNameAtScriptName;
        script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_addonscript)
            {
                if (scriptName.size()>0)
                    script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
            }
            if (scriptHandleOrType==sim_scripttype_sandboxscript)
                script=App::ct->sandboxScript;
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                if (scriptName.size()>0)
                { // new way
                    int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                    script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
                }
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            { // new way only possible (6 was not available in the old way)
                int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            variableName=variableNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=script->clearScriptVariable(variableName.c_str());
        if (retVal==-1)
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

simVoid _simGetVortexParameters_internal(const simVoid* object,simInt version,simFloat* floatParams,simInt* intParams)
{ // if object is nullptr, we return general engine settings, if object is a shape, we return shape settings, otherwise joint settings
    // Version allows to adjust for future extensions.
    C_API_FUNCTION_DEBUG;
    std::vector<float> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::ct->dynamicsContainer->getVortexFloatParams(fparams);
        App::ct->dynamicsContainer->getVortexIntParams(iparams);
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
        { // when the dynamics plugin is the same version as V-REP, or newer!
            fcnt=10;
            icnt=1;
        }
    }
    else
    {
        C3DObject* obj=(C3DObject*)object;
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
            { // when the dynamics plugin is the same version as V-REP, or newer!
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
            { // when the dynamics plugin is the same version as V-REP, or newer!
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
    C_API_FUNCTION_DEBUG;
    std::vector<float> fparams;
    std::vector<int> iparams;
    int icnt=0;
    int fcnt=0;
    if (object==nullptr)
    {
        App::ct->dynamicsContainer->getNewtonFloatParams(fparams);
        App::ct->dynamicsContainer->getNewtonIntParams(iparams);
        if (version[0]>=0)
        { // when the dynamics plugin is the same version as V-REP, or newer!
            fcnt=2;
            icnt=2;
        }
        version[0]=0;
    }
    else
    {
        C3DObject* obj=(C3DObject*)object;
        if (obj->getObjectType()==sim_object_shape_type)
        {
            CShape* shape=(CShape*)object;
            CDynMaterialObject* mat=shape->getDynMaterial();
            mat->getNewtonFloatParams(fparams);
            mat->getNewtonIntParams(iparams);
            if (version[0]>=0)
            { // when the dynamics plugin is the same version as V-REP, or newer!
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
            { // when the dynamics plugin is the same version as V-REP, or newer!
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
    C_API_FUNCTION_DEBUG;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr);
    bounce[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_bounce,nullptr);
    fudge[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr);
}

simVoid _simGetJointBulletParameters_internal(const simVoid* joint,simFloat* stopERP,simFloat* stopCFM,simFloat* normalCFM)
{
    C_API_FUNCTION_DEBUG;
    stopERP[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr);
    stopCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr);
    normalCFM[0]=((CJoint*)joint)->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr);
}

CShape* __getShapeFromGeomInfo(const simVoid* geomInfo)
{
    for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
    {
        CShape* sh=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
        if (sh->geomData->geomInfo==(CGeomWrap*)geomInfo)
            return(sh);
    }
    return(nullptr);
}

simVoid _simGetOdeMaxContactFrictionCFMandERP_internal(const simVoid* geomInfo,simInt* maxContacts,simFloat* friction,simFloat* cfm,simFloat* erp)
{
    C_API_FUNCTION_DEBUG;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    maxContacts[0]=mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr);
    friction[0]=mat->getEngineFloatParam(sim_ode_body_friction,nullptr);
    cfm[0]=mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr);
    erp[0]=mat->getEngineFloatParam(sim_ode_body_softerp,nullptr);
}

simBool _simGetBulletCollisionMargin_internal(const simVoid* geomInfo,simFloat* margin,simInt* otherProp)
{
    C_API_FUNCTION_DEBUG;
    CGeomWrap* geomWrap=(CGeomWrap*)geomInfo;
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
    C_API_FUNCTION_DEBUG;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr));
}

simFloat _simGetBulletRestitution_internal(const simVoid* geomInfo)
{
    C_API_FUNCTION_DEBUG;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();
    return(mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr));
}

simVoid _simGetDamping_internal(const simVoid* geomInfo,simFloat* linDamping,simFloat* angDamping)
{
    C_API_FUNCTION_DEBUG;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::ct->dynamicsContainer->getDynamicEngineType(nullptr);
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
    C_API_FUNCTION_DEBUG;
    CShape* shape=__getShapeFromGeomInfo(geomInfo);
    CDynMaterialObject* mat=shape->getDynMaterial();

    int eng=App::ct->dynamicsContainer->getDynamicEngineType(nullptr);
    if (eng==sim_physics_bullet)
        return(mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr));
    if (eng==sim_physics_ode)
        return(mat->getEngineFloatParam(sim_ode_body_friction,nullptr));
    return(0.0f);
}

simInt simAddSceneCustomData_internal(simInt header,const simChar* data,simInt dataLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        App::ct->customSceneData->setData(header,data,dataLength);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetSceneCustomDataLength_internal(simInt header)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=App::ct->customSceneData->getDataLength(header);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetSceneCustomData_internal(simInt header,simChar* data)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        App::ct->customSceneData->getData(header,data);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}



simInt simAddObjectCustomData_internal(simInt objectHandle,simInt header,const simChar* data,simInt dataLength)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesObjectExist(__func__,objectHandle))
        {
            return(-1);
        }
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        it->setObjectCustomData(header,data,dataLength);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetObjectCustomDataLength_internal(simInt objectHandle,simInt header)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        int retVal=it->getObjectCustomDataLength(header);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetObjectCustomData_internal(simInt objectHandle,simInt header,simChar* data)
{
    C_API_FUNCTION_DEBUG;

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
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectHandle);
        it->getObjectCustomData(header,data);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simLoadUI_internal(const simChar* filename,int maxCount,int* uiHandles)
{
    C_API_FUNCTION_DEBUG;
    return(-1);
}

simInt simCreateUI_internal(const simChar* elementName,simInt menuAttributes,const simInt* clientSize,const simInt* cellSize,simInt* buttonHandles)
{
    C_API_FUNCTION_DEBUG;

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
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_INVALID_DIMENSIONS);
            return(-1);
        }
        CButtonBlock* it=new CButtonBlock(s[0],s[1],cellSize[0],cellSize[1],-1);
        it->setBlockName(elementName);
        if ((menuAttributes&sim_ui_menu_systemblock)!=0)
            it->setAttributes(it->getAttributes()|sim_ui_property_systemblock);
        App::ct->buttonBlockContainer->insertBlock(it,false);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateUIButton_internal(simInt elementHandle,const simInt* position,const simInt* size,simInt buttonProperty)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=new CSoftButton("",position[0],position[1],size[0],size[1]);
        if (!it->insertButton(but))
        {
            delete but;
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_WRONG_POS_SIZE_PARAMS);
            return(-1);
        }
        int retVal=but->buttonID;
        if (simSetUIButtonProperty_internal(elementHandle,retVal,buttonProperty)==-1)
            retVal=-1; // should not happen!
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetUIHandle_internal(const simChar* elementName)
{
    C_API_FUNCTION_DEBUG;

    std::string elementNameAdjusted=getCNameSuffixAdjustedName(elementName);
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithName(elementNameAdjusted);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        int retVal=it->getBlockID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIProperty_internal(simInt elementHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        int retVal=it->getAttributes();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIEventButton_internal(simInt elementHandle,simInt* auxiliaryValues)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
        {
            return(-1);
        }
        int retVal=-1;
#ifdef SIM_WITH_GUI
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        retVal=it->getLastEventButtonID(auxiliaryValues);
#endif
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIProperty_internal(simInt elementHandle,simInt elementProperty)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        // Following few new since 4/2/2013 (to bring newly made visible UI to the front)
        int attrib=it->getAttributes();
        it->setAttributes(elementProperty);
        int attribNew=it->getAttributes();
        if ( ((attrib&sim_ui_property_visible)==0)&&((attribNew&sim_ui_property_visible)!=0) )
            App::ct->buttonBlockContainer->sendBlockToFront(it->getBlockID());
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUIButtonSize_internal(simInt elementHandle,simInt buttonHandle,simInt* size)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        size[0]=but->getLength();
        size[1]=but->getHeight();
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}


simInt simGetUIButtonProperty_internal(simInt elementHandle,simInt buttonHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        int retVal=but->getAttributes();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonProperty_internal(simInt elementHandle,simInt buttonHandle,simInt buttonProperty)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->setAttributes(buttonProperty);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonLabel_internal(simInt elementHandle,simInt buttonHandle,const simChar* upStateLabel,const simChar* downStateLabel)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (upStateLabel!=nullptr)
            but->label=std::string(upStateLabel);
        if (downStateLabel!=nullptr)
            but->downLabel=std::string(downStateLabel);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simChar* simGetUIButtonLabel_internal(simInt elementHandle,simInt buttonHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(nullptr);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(nullptr);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        char* retVal=new char[but->label.length()+1];
        for (unsigned int i=0;i<but->label.length();i++)
            retVal[i]=but->label[i];
        retVal[but->label.length()]=0;
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(nullptr);
}

simInt simSetUISlider_internal(simInt elementHandle,simInt buttonHandle,simInt position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        but->setSliderPos((float(position)/500.0f)-1.0f);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simGetUISlider_internal(simInt elementHandle,simInt buttonHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (but->getButtonType()!=sim_buttonproperty_slider)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_BUTTON_NOT_SLIDER);
            return(-1);
        }
        int retVal=int((but->getSliderPos()+1.0f)*500.0f);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIButtonColor_internal(simInt elementHandle,simInt buttonHandle,const simFloat* upStateColor,const simFloat* downStateColor,const simFloat* labelColor)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
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
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRemoveUI_internal(simInt elementHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (elementHandle==sim_handle_all)
        {
            App::ct->buttonBlockContainer->removeAllBlocks(false);
            return(1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_UI_INEXISTANT);
            return(-1);
        }
        App::ct->buttonBlockContainer->removeBlockFromID(elementHandle);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simCreateUIButtonArray_internal(simInt elementHandle,simInt buttonHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(true);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetUIButtonArrayColor_internal(simInt elementHandle,simInt buttonHandle,const simInt* position,const simFloat* color)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        if (!but->setArrayColor(position[0],position[1],color))
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OPERATION_FAILED);
            return(-1);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simDeleteUIButtonArray_internal(simInt elementHandle,simInt buttonHandle)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        but->enableArray(false);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSetUIButtonTexture_internal(simInt elementHandle,simInt buttonHandle,const simInt* size,const simChar* textureData)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        if (!doesUIButtonExist(__func__,elementHandle,buttonHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        CSoftButton* but=it->getButtonWithID(buttonHandle);
        CTextureProperty* tp=but->getTextureProperty();
        if (tp!=nullptr)
        { // We already have a texture. Is it the same size/type? or do we wanna remove the texture anyway?
            int tob=tp->getTextureObjectID();
            bool remove=true;
            if ((tob>=SIM_IDSTART_TEXTURE)&&(tob<=SIM_IDSTART_TEXTURE)&&(size!=nullptr))
            { // we have the correct type (i.e. non-vision sensor)
                CTextureObject* to=App::ct->textureCont->getObject(tob);
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
                App::ct->textureCont->announceGeneralObjectWillBeErased(elementHandle,but->getUniqueID());
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
            int textureID=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
            tp=new CTextureProperty(textureID);
            but->setTextureProperty(tp);
        }
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simSaveUI_internal(int count,const int* uiHandles,const simChar* filename)
{
    C_API_FUNCTION_DEBUG;
    return(-1);
}

simInt simGetUIPosition_internal(simInt elementHandle,simInt* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
            return(-1);

        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        VPoint p;
        it->getBlockPositionAbsolute(p);
        position[0]=p.x;
        position[1]=p.y;
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simSetUIPosition_internal(simInt elementHandle,const simInt* position)
{
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if (!doesUIExist(__func__,elementHandle))
        {
            return(-1);
        }
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(elementHandle);
        it->setDesiredBlockPosition(position[0],position[1]);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleGeneralCallbackScript_internal(simInt callbackId,simInt callbackTag,simVoid* additionalData)
{ // Deprecated since release 3.4.1
    C_API_FUNCTION_DEBUG;
    return(-1);
}

simInt simRegisterCustomLuaFunction_internal(const simChar* funcName,const simChar* callTips,const simInt* inputArgumentTypes,simVoid(*callBack)(struct SLuaCallBack* p))
{ // deprecated
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
    {
        return(-1);
    }

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        bool retVal=1;
        if (App::ct->luaCustomFuncAndVarContainer->removeCustomFunction(funcName))
            retVal=0;// that function already existed. We remove it and replace it!
        std::vector<int> inputV;
        if (inputArgumentTypes!=nullptr)
        {
            for (int i=0;i<inputArgumentTypes[0];i++)
                inputV.push_back(inputArgumentTypes[i+1]);
        }
        CLuaCustomFunction* newFunction=new CLuaCustomFunction(funcName,callTips,inputV,callBack);
        if (!App::ct->luaCustomFuncAndVarContainer->insertCustomFunction(newFunction))
        {
            delete newFunction;
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_CUSTOM_LUA_FUNC_COULD_NOT_BE_REGISTERED);
            return(-1);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simRegisterContactCallback_internal(simInt(*callBack)(simInt,simInt,simInt,simInt*,simFloat*))
{ // Deprecated
    C_API_FUNCTION_DEBUG;

    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        for (int i=0;i<int(allContactCallbacks.size());i++)
        {
            if (allContactCallbacks[i]==callBack)
            { // We unregister that callback
                allContactCallbacks.erase(allContactCallbacks.begin()+i);
                return(0);
            }
        }
        // We register that callback:
        allContactCallbacks.push_back(callBack);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simRegisterJointCtrlCallback_internal(simInt(*callBack)(simInt,simInt,simInt,const simInt*,const simFloat*,simFloat*))
{ // deprecated
    C_API_FUNCTION_DEBUG;


    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        for (int i=0;i<int(allJointCtrlCallbacks.size());i++)
        {
            if (allJointCtrlCallbacks[i]==callBack)
            { // We unregister that callback
                allJointCtrlCallbacks.erase(allJointCtrlCallbacks.begin()+i);
                return(0);
            }
        }
        // We register that callback:
        allJointCtrlCallbacks.push_back(callBack);
        return(1);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_WRITE);
    return(-1);
}

simInt simGetMechanismHandle_internal(const simChar* mechanismName)
{ // deprecated
    C_API_FUNCTION_DEBUG;

    std::string mechanismNameAdjusted=getCNameSuffixAdjustedName(mechanismName);
    enableCNameSuffixAdjustment();
    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        CConstraintSolverObject* it=App::ct->constraintSolver->getObject(mechanismNameAdjusted.c_str());
        if (it==nullptr)
        {
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MECHANISM_INEXISTANT);
            return(-1);
        }
        int retVal=it->getObjectID();
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleMechanism_internal(simInt mechanismHandle)
{ // deprecated
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        if ( (mechanismHandle!=sim_handle_all)&&(mechanismHandle!=sim_handle_all_except_explicit) )
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(mechanismHandle);
            if (it==nullptr)
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_MECHANISM_INEXISTANT);
                return(-1);
            }
        }
        int calcCnt=0;
        if (mechanismHandle<0)
            calcCnt=App::ct->constraintSolver->computeAllMechanisms(mechanismHandle==sim_handle_all_except_explicit);
        else
        { // explicit handling
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(mechanismHandle);
            if (!it->getExplicitHandling())
            {
                CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_OBJECT_NOT_TAGGED_FOR_EXPLICIT_HANDLING);
                return(-1);
            }
            if (it->computeGcs())
                calcCnt++;
        }
        return(calcCnt);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simHandleCustomizationScripts_internal(simInt callType)
{ // deprecated
    C_API_FUNCTION_DEBUG;

    if (!isSimulatorInitialized(__func__))
        return(-1);

    IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
    {
        int retVal=0;
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            retVal=App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,callType,nullptr,nullptr,nullptr);
            App::ct->luaScriptContainer->removeDestroyedScripts(sim_scripttype_customizationscript);
        }
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_COULD_NOT_LOCK_RESOURCES_FOR_READ);
    return(-1);
}

simInt simCallScriptFunction_internal(simInt scriptHandleOrType,const simChar* functionNameAtScriptName,SLuaCallBack* data,const simChar* reservedSetToNull)
{ // DEPRECATED
    C_API_FUNCTION_DEBUG;
    CLuaScriptObject* script=nullptr;

    std::string funcName;
    if (scriptHandleOrType>=SIM_IDSTART_LUASCRIPT)
    { // script is identified by its ID
        std::string funcNameAtScriptName(functionNameAtScriptName);
        size_t p=funcNameAtScriptName.find('@');
        if (p!=std::string::npos)
            funcName.assign(funcNameAtScriptName.begin(),funcNameAtScriptName.begin()+p);
        else
            funcName=funcNameAtScriptName;
        script=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptHandleOrType);
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
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==sim_scripttype_childscript)
            {
                int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
            }
            if (scriptHandleOrType==sim_scripttype_customizationscript)
            {
                int objId=App::ct->objCont->getObjectHandleFromName(scriptName.c_str());
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
            if (scriptHandleOrType==sim_scripttype_addonscript)
                script=App::ct->addOnScriptContainer->getAddOnScriptFromName(scriptName.c_str());
        }
        else
        { // this is the old way of doing it. Deprecated. Was only 2 months active, not officially
            funcName=functionNameAtScriptName;
            if (scriptHandleOrType==0) // main script
                script=App::ct->luaScriptContainer->getMainScript();
            if (scriptHandleOrType==3) // child script
            {
                int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objId);
            }
            if (scriptHandleOrType==5) // customization
            {
                int objId=App::ct->objCont->getObjectHandleFromName(reservedSetToNull);
                script=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objId);
            }
        }
    }

    if (script!=nullptr)
    {
        int retVal=-1; // error
        if (script->getThreadedExecutionIsUnderWay())
        { // very special handling here!
            if (VThread::areThreadIDsSame(script->getThreadedScriptThreadId(),VThread::getCurrentThreadId()))
                retVal=script->callScriptFunction(funcName.c_str(),data);
            else
            { // we have to execute that function via another thread!
                void* d[4];
                int callType=0;
                d[0]=&callType;
                d[1]=script;
                d[2]=(void*)funcName.c_str();
                d[3]=data;

                retVal=CThreadPool::callRoutineViaSpecificThread(script->getThreadedScriptThreadId(),d);
            }
        }
        else
        {
            if (VThread::isCurrentThreadTheMainSimulationThread())
            { // For now we don't allow non-main threads to call non-threaded scripts!
                retVal=script->callScriptFunction(funcName.c_str(),data);
            }
        }
        if (retVal==-1)
            CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_FAILED_CALLING_SCRIPT_FUNCTION);
        return(retVal);
    }
    CApiErrors::setApiCallErrorMessage(__func__,SIM_ERROR_SCRIPT_INEXISTANT);

    return(-1);
}

