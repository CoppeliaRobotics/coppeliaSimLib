#include <app.h>
#include <pluginContainer.h>
#include <simLib/simConst.h>
#include <simInternal.h>
#include <utils.h>
#include <apiErrors.h>
#include <collisionRoutines.h>
#include <volInt.h>
#include <algorithm>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
    #include <guiApp.h>
#endif

CPlugin::CPlugin(const char* filename,const char* pluginnamespaceAndVersion,int loadOrigin)
{
    _initAddress=nullptr;
    _cleanupAddress=nullptr;
    _msgAddress=nullptr;
    _initAddress_ui=nullptr;
    _cleanupAddress_ui=nullptr;
    _msgAddress_ui=nullptr;

    _filename=filename;
    _stage=stage_none;
    _name=pluginnamespaceAndVersion;
    if (loadOrigin>=-1)
    { // new plugins
        size_t p=_name.find('-');
        if (p==std::string::npos)
            _namespace=_name;
        else
            _namespace.assign(_name.begin(),_name.begin()+p);
    }
    else
        _namespace.clear(); // old plugins
    if (loadOrigin!=-1) // plugins auto loaded by the system have no direct dependency: they simply can be loaded again on demand
        _dependencies.insert(loadOrigin);
    extendedVersionInt=-1;
    _consoleVerbosity=sim_verbosity_useglobal;
    _statusbarVerbosity=sim_verbosity_useglobal;

    // Following used to detect appartenance:
    instance=nullptr;
    geomPlugin_createMesh=nullptr;
    oldIkPlugin_createEnv=nullptr;
    _codeEditor_openModal=nullptr;
    _customUi_msgBox=nullptr;
    _assimp_importShapes=nullptr;
    ruckigPlugin_pos=nullptr;
    povRayAddr=nullptr;
    openGl3Addr=nullptr;
    qhullAddr=nullptr;
    hacdAddr=nullptr;
    vhacdAddr=nullptr;
    decimatorAddr=nullptr;
    bullet278_engine=nullptr;
    bullet283_engine=nullptr;
    ode_engine=nullptr;
    vortex_engine=nullptr;
    newton_engine=nullptr;
    mujoco_engine=nullptr;
}

CPlugin::~CPlugin()
{
    if (instance!=nullptr)
    {
        /* We let the app unload all libraries when the app ends. Otherwise we get tricky crashes with Federico's plugins
        std::string errStr;
        VVarious::closeLibrary(instance,&errStr);
        if (errStr.size()>0)
            App::logMsg(sim_verbosity_errors,errStr.c_str());
            */
    }
    if (geomPlugin_createMesh!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentGeomPlugin=nullptr;
    if (bullet278_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->bullet278Engine=nullptr;
    if (bullet283_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->bullet283Engine=nullptr;
    if (ode_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->odeEngine=nullptr;
    if (vortex_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->vortexEngine=nullptr;
    if (newton_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->newtonEngine=nullptr;
    if (mujoco_engine!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->mujocoEngine=nullptr;
    if (oldIkPlugin_createEnv!=nullptr) // also check constructor above
    {
        App::worldContainer->pluginContainer->currentIKPlugin=nullptr;
        App::worldContainer->pluginContainer->ikEnvironment=-1;
    }
    if (ruckigPlugin_pos!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentRuckigPlugin=nullptr;
    if (_codeEditor_openModal!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentCodeEditorPlugin=nullptr;
    if (_customUi_msgBox!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentUIPlugin=nullptr;
    if (_assimp_importShapes!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentAssimpPlugin=nullptr;
    if (povRayAddr!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentPovRayPlugin=nullptr;
    if (openGl3Addr!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentOpenGl3Plugin=nullptr;
    if (qhullAddr!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentQHullPlugin=nullptr;
    if (hacdAddr!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentConvexDecomposePlugin=nullptr;
    if (decimatorAddr!=nullptr) // also check constructor above
        App::worldContainer->pluginContainer->currentMeshDecimationPlugin=nullptr;
    _pluginCallbackContainer.clear();
    _pluginVariableContainer.clear();
}

void CPlugin::setConsoleVerbosity(int level)
{
    _consoleVerbosity=level;
}

int CPlugin::getConsoleVerbosity() const
{
    return(_consoleVerbosity);
}

void CPlugin::setStatusbarVerbosity(int level)
{
    _statusbarVerbosity=level;
}

int CPlugin::getStatusbarVerbosity() const
{
    return(_statusbarVerbosity);
}

std::string CPlugin::getName() const
{
    return(_name);
}

std::string CPlugin::getNamespace() const
{
    return(_namespace);
}

int CPlugin::getPluginVersion() const
{
    return(pluginVersion);
}

std::string CPlugin::getExtendedVersionString() const
{
    return(extendedVersionString);
}

void CPlugin::setExtendedVersionString(const char* str)
{
    extendedVersionString=str;
}

std::string CPlugin::getBuildDateString() const
{
    return(buildDateString);
}

void CPlugin::setBuildDateString(const char* str)
{
    buildDateString=str;
}

int CPlugin::getExtendedVersionInt() const
{
    return(extendedVersionInt);
}

void CPlugin::setExtendedVersionInt(int v)
{
    extendedVersionInt=v;
}

int CPlugin::getHandle() const
{
    return(handle);
}

bool CPlugin::isLegacyPlugin() const
{
    return(_initAddress==nullptr);
}

bool CPlugin::isUiPlugin() const
{
    return(_initAddress_ui!=nullptr);
}

void CPlugin::setHandle(int h)
{
    handle=h;
}

void CPlugin::pushCurrentPlugin()
{
    App::worldContainer->pluginContainer->currentPluginStack.push_back(this);
}

void CPlugin::popCurrentPlugin()
{
    App::worldContainer->pluginContainer->currentPluginStack.pop_back();
}

CPluginCallbackContainer* CPlugin::getPluginCallbackContainer()
{
    return(&_pluginCallbackContainer);
}

CPluginVariableContainer* CPlugin::getPluginVariableContainer()
{
    return(&_pluginVariableContainer);
}

void CPlugin::addDependency(int loadOrigin)
{
    _dependencies.insert(loadOrigin);
}

void CPlugin::removeDependency(int loadOrigin)
{
    _dependencies.erase(loadOrigin);
}

bool CPlugin::hasDependency(int loadOrigin) const
{
    return(_dependencies.find(loadOrigin)!=_dependencies.end());
}

std::string CPlugin::getDependencies() const
{
    std::string retVal;
    for (auto it=_dependencies.begin();it!=_dependencies.end();it++)
        retVal+=std::to_string(*it)+" ";
    if (retVal.size()>0)
        retVal.pop_back();
    return(retVal);
}

bool CPlugin::hasAnyDependency() const
{
    return(!_dependencies.empty());
}

int CPlugin::load(std::string* errStr)
{ // retVal: -2 (could not open library), -1 (missing init entry point), 1=ok
    int retVal=-2; // could not open library
    WLibrary lib=VVarious::openLibrary(_filename.c_str(),errStr); // here we have the extension in the filename (.dll, .so or .dylib)
    if (lib!=nullptr)
    {
        instance=lib;
        _initAddress=(ptrInit)(VVarious::resolveLibraryFuncName(lib,"simInit"));
        _cleanupAddress=(ptrCleanup)(VVarious::resolveLibraryFuncName(lib,"simCleanup"));
        _msgAddress=(ptrMsg)(VVarious::resolveLibraryFuncName(lib,"simMsg"));

        _initAddress_ui=(ptrInit_ui)(VVarious::resolveLibraryFuncName(lib,"simInit_ui"));
        _cleanupAddress_ui=(ptrCleanup_ui)(VVarious::resolveLibraryFuncName(lib,"simCleanup_ui"));
        _msgAddress_ui=(ptrMsg_ui)(VVarious::resolveLibraryFuncName(lib,"simMsg_ui"));

        if (_initAddress!=nullptr)
            retVal=1;
        else
        {
            VVarious::closeLibrary(instance,nullptr);
            if (errStr!=nullptr)
                errStr[0]="missing init entry point in plugin";
            instance=nullptr;
            retVal=-1; // missing init entry point
        }
    }
    return(retVal);
}

bool CPlugin::init(std::string* errStr)
{
    bool retVal=false;
    if (_initAddress!=nullptr)
    {
        if ( (!VThread::isSimThread())&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
            App::logMsg(sim_verbosity_errors,"wrong thread in CPlugin::init");

        pushCurrentPlugin();
        SSimInit d;
        d.pluginName=_name.c_str();
        std::string libPath(App::folders->getExecutablePath()+"/");
#ifndef WIN_SIM
        libPath+="lib";
#endif
#ifdef SIM_WITH_GUI
        libPath+="coppeliaSim.";
#else
        libPath+="coppeliaSimHeadless.";
#endif
#ifdef WIN_SIM
        libPath+="dll";
#endif
#ifdef LIN_SIM
        libPath+="so";
#endif
#ifdef MAC_SIM
        libPath+="dylib";
#endif
        d.coppeliaSimLibPath=libPath.data();
        pluginVersion=_initAddress(&d);
        popCurrentPlugin();
        if (pluginVersion!=0)
        {
            retVal=true;
            _loadAuxEntryPoints();
            _stage=stage_siminitdone;
#ifdef SIM_WITH_GUI
            if ( (GuiApp::mainWindow!=nullptr)&&(_initAddress_ui!=nullptr) ) // will have to be adjusted with the new framework eventually
            {
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=CALL_PLUGIN_INITUI_FROM_UITHREAD_UITHREADCMD;
                cmdIn.intParams.push_back(handle);
                GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                while (_stage!=stage_uiinitdone)
                    VThread::sleep(1);
            }
#endif
        }
        else
        { // could not properly initialize
            VVarious::closeLibrary(instance,nullptr);
            if (errStr!=nullptr)
                errStr[0]="could not properly initialize plugin";
            instance=nullptr;
        }
    }
    else
    {
        if (errStr!=nullptr)
            errStr[0]="plugin does not support on demand load";
    }
    return(retVal);
}

bool CPlugin::msg(int msgId,int* auxData/*=nullptr*/,void* auxPointer/*=nullptr*/,int* reserved_legacy/*=nullptr*/)
{
    bool retVal=false; // only used by legacy plugins
    pushCurrentPlugin();
    if (_initAddress!=nullptr)
    {
        if ( (!VThread::isSimThread())&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
        {
            std::string m("wrong thread in CPlugin::msg (");
            m+=std::to_string(msgId)+")";
            App::logMsg(sim_verbosity_errors,m.c_str());
        }
        if (_msgAddress!=nullptr)
        {
            SSimMsg p;
            p.msgId=msgId;
            p.auxData=auxData;
            p.auxPointer=auxPointer;
            _msgAddress(&p); // new plugin
        }
    }
    else
    { // legacy plugin
        if (_messageAddress_legacy!=nullptr)
        {
            void* returnData=_messageAddress_legacy(msgId,auxData,auxPointer,reserved_legacy);
            retVal=(returnData!=nullptr);
            if (returnData!=nullptr)
                delete[] (char*)returnData;
        }
    }
    popCurrentPlugin();
    return(retVal);
}

void CPlugin::init_ui()
{
    if ( (_initAddress_ui!=nullptr)&&(_stage==stage_siminitdone) )
    {
        if ( VThread::isSimThread()&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
            App::logMsg(sim_verbosity_errors,"wrong thread in CPlugin::init_ui");

        _initAddress_ui();
        _stage=stage_uiinitdone;
    }
}

void CPlugin::cleanup_ui()
{
    if ( (_cleanupAddress_ui!=nullptr)&&(_stage==stage_docleanup) )
    {
        if ( VThread::isSimThread()&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
            App::logMsg(sim_verbosity_errors,"wrong thread in CPlugin::cleanup_ui");

        _cleanupAddress_ui();
        _stage=stage_uicleanupdone;
    }
}

void CPlugin::msg_ui(int msgId,int* auxData/*=nullptr*/,void* auxPointer/*=nullptr*/)
{
    if ( (_msgAddress_ui!=nullptr)&&(_stage==stage_uiinitdone) )
    {
        if ( VThread::isSimThread()&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
        {
            std::string m("wrong thread in CPlugin::msg_ui (");
            m+=std::to_string(msgId)+")";
            App::logMsg(sim_verbosity_errors,m.c_str());
        }

        SSimMsg_ui p;
        p.msgId=msgId;
        p.auxData=auxData;
        p.auxPointer=auxPointer;
        _msgAddress_ui(&p);
    }
}

void CPlugin::cleanup()
{
    if (_initAddress!=nullptr)
    {
        if ( (!VThread::isSimThread())&&CSimFlavor::getBoolVal(18) ) // old scenes might still trigger wrong thread usage, in some cases
            App::logMsg(sim_verbosity_errors,"wrong thread in CPlugin::cleanup");

        if (_cleanupAddress!=nullptr)
        {
            if (_stage==stage_uiinitdone)
            {
                _stage=stage_docleanup;
#ifdef SIM_WITH_GUI
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=CALL_PLUGIN_CLEANUPUI_FROM_UITHREAD_UITHREADCMD;
                cmdIn.intParams.push_back(handle);
                GuiApp::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif
                while (_stage!=stage_uicleanupdone)
                    VThread::sleep(1);
            }
            pushCurrentPlugin();
            _cleanupAddress();
            popCurrentPlugin();
        }
    }
    else
    { // old plugin
        if (_endAddress_legacy!=nullptr)
        {
            pushCurrentPlugin();
            _endAddress_legacy();
            popCurrentPlugin();
        }
    }
}

int CPlugin::loadAndInit_old(std::string* errStr)
{ // retVal: -2 (could not open library), -1 (missing init entry point), 0 (could not properly initialize), otherwise plugin version
    int retVal=-2; // could not open library
    QString curr(QDir::currentPath());
    QDir::setCurrent(App::getApplicationDir().c_str()); // needed for old plugins, also Linux and macOS!
    WLibrary lib=VVarious::openLibrary(_filename.c_str(),errStr); // here we have the extension in the filename (.dll, .so or .dylib)
    if (lib!=nullptr)
    {
        instance=lib;
        _startAddress_legacy=(ptrStart)(VVarious::resolveLibraryFuncName(lib,"simStart"));
        if (_startAddress_legacy==nullptr)
            _startAddress_legacy=(ptrStart)(VVarious::resolveLibraryFuncName(lib,"v_repStart")); // for backward compatibility

        _endAddress_legacy=(ptrEnd)(VVarious::resolveLibraryFuncName(lib,"simEnd"));
        if (_endAddress_legacy==nullptr)
            _endAddress_legacy=(ptrEnd)(VVarious::resolveLibraryFuncName(lib,"v_repEnd")); // for backward compatibility
        _messageAddress_legacy=(ptrMessage)(VVarious::resolveLibraryFuncName(lib,"simMessage"));
        if (_messageAddress_legacy==nullptr)
            _messageAddress_legacy=(ptrMessage)(VVarious::resolveLibraryFuncName(lib,"v_repMessage")); // for backward compatibility
        if ( (_startAddress_legacy!=nullptr)&&(_endAddress_legacy!=nullptr)&&(_messageAddress_legacy!=nullptr) )
        {
            pushCurrentPlugin();
            pluginVersion=(int)_startAddress_legacy(nullptr,0);
            popCurrentPlugin();

            if (pluginVersion!=0)
                _loadAuxEntryPoints();
            else
            { // could not properly initialize
                VVarious::closeLibrary(instance,nullptr);
                if (errStr!=nullptr)
                    errStr[0]="could not properly initialize plugin";
                instance=nullptr;
            }
            retVal=pluginVersion;
        }
        else
            retVal=-1; // missing entry points
    }
    QDir::setCurrent(curr);
    return(retVal);
}

void CPlugin::_loadAuxEntryPoints()
{
    povRayAddr=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(instance,"simPovRay"));
    if (povRayAddr!=nullptr)
        App::worldContainer->pluginContainer->currentPovRayPlugin=this;

    openGl3Addr=(ptrExtRenderer)(VVarious::resolveLibraryFuncName(instance,"simOpenGL3Renderer"));
    if (openGl3Addr!=nullptr)
        App::worldContainer->pluginContainer->currentOpenGl3Plugin=this;

    qhullAddr=(ptrQhull)(VVarious::resolveLibraryFuncName(instance,"simQhull"));
    if (qhullAddr!=nullptr)
        App::worldContainer->pluginContainer->currentQHullPlugin=this;

    hacdAddr=(ptrHACD)(VVarious::resolveLibraryFuncName(instance,"simHACD"));
    vhacdAddr=(ptrVHACD)(VVarious::resolveLibraryFuncName(instance,"simVHACD"));
    if ( (hacdAddr!=nullptr)&&(vhacdAddr!=nullptr) )
        App::worldContainer->pluginContainer->currentConvexDecomposePlugin=this;

    decimatorAddr=(ptrMeshDecimator)(VVarious::resolveLibraryFuncName(instance,"simDecimateMesh"));
    if (decimatorAddr!=nullptr)
        App::worldContainer->pluginContainer->currentMeshDecimationPlugin=this;

    // For the dynamic plugins:
    dynPlugin_startSimulation=(ptr_dynPlugin_startSimulation_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_startSimulation_D"));
    dynPlugin_endSimulation=(ptr_dynPlugin_endSimulation)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_endSimulation"));
    dynPlugin_step=(ptr_dynPlugin_step_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_step_D"));
    dynPlugin_isDynamicContentAvailable=(ptr_dynPlugin_isDynamicContentAvailable)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_isDynamicContentAvailable"));
    dynPlugin_serializeDynamicContent=(ptr_dynPlugin_serializeDynamicContent)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_serializeDynamicContent"));
    dynPlugin_addParticleObject=(ptr_dynPlugin_addParticleObject_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_addParticleObject_D"));
    dynPlugin_removeParticleObject=(ptr_dynPlugin_removeParticleObject)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_removeParticleObject"));
    dynPlugin_addParticleObjectItem=(ptr_dynPlugin_addParticleObjectItem_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_addParticleObjectItem_D"));
    dynPlugin_getParticleObjectOtherFloatsPerItem=(ptr_dynPlugin_getParticleObjectOtherFloatsPerItem)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getParticleObjectOtherFloatsPerItem"));
    dynPlugin_getContactPoints=(ptr_dynPlugin_getContactPoints_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getContactPoints_D"));
    dynPlugin_getParticles=(ptr_dynPlugin_getParticles)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getParticles"));
    dynPlugin_getParticleData=(ptr_dynPlugin_getParticleData_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getParticleData_D"));
    dynPlugin_getContactForce=(ptr_dynPlugin_getContactForce_D)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getContactForce_D"));
    dynPlugin_getDynamicStepDivider=(ptr_dynPlugin_getDynamicStepDivider)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_getDynamicStepDivider"));
    mujocoPlugin_computeInertia=(ptr_mujocoPlugin_computeInertia)(VVarious::resolveLibraryFuncName(instance,"mujocoPlugin_computeInertia"));
    mujocoPlugin_computePMI=(ptr_mujocoPlugin_computePMI)(VVarious::resolveLibraryFuncName(instance,"mujocoPlugin_computePMI"));
    bullet278_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_bullet278"));
    bullet283_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_bullet283"));
    ode_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_ode"));
    vortex_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_vortex"));
    newton_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_newton"));
    mujoco_engine=(ptr_dynPlugin_engine)(VVarious::resolveLibraryFuncName(instance,"dynPlugin_mujoco"));
    if (bullet278_engine!=nullptr)
        App::worldContainer->pluginContainer->bullet278Engine=this;
    if (bullet283_engine!=nullptr)
        App::worldContainer->pluginContainer->bullet283Engine=this;
    if (ode_engine!=nullptr)
        App::worldContainer->pluginContainer->odeEngine=this;
    if (vortex_engine!=nullptr)
        App::worldContainer->pluginContainer->vortexEngine=this;
    if (newton_engine!=nullptr)
        App::worldContainer->pluginContainer->newtonEngine=this;
    if (mujoco_engine!=nullptr)
        App::worldContainer->pluginContainer->mujocoEngine=this;

    // For the geom plugin:
    geomPlugin_releaseBuffer=(ptr_geomPlugin_releaseBuffer)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_releaseBuffer"));
    geomPlugin_createMesh=(ptr_geomPlugin_createMesh)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createMesh"));
    geomPlugin_copyMesh=(ptr_geomPlugin_copyMesh)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_copyMesh"));
    geomPlugin_getMeshFromSerializationData=(ptr_geomPlugin_getMeshFromSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshFromSerializationData"));
    geomPlugin_getMeshSerializationData=(ptr_geomPlugin_getMeshSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshSerializationData"));
    geomPlugin_scaleMesh=(ptr_geomPlugin_scaleMesh)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_scaleMesh"));
    geomPlugin_destroyMesh=(ptr_geomPlugin_destroyMesh)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_destroyMesh"));
    geomPlugin_getMeshRootObbVolume=(ptr_geomPlugin_getMeshRootObbVolume)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshRootObbVolume"));
    geomPlugin_createOctreeFromPoints=(ptr_geomPlugin_createOctreeFromPoints)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createOctreeFromPoints"));
    geomPlugin_createOctreeFromColorPoints=(ptr_geomPlugin_createOctreeFromColorPoints)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createOctreeFromColorPoints"));
    geomPlugin_createOctreeFromMesh=(ptr_geomPlugin_createOctreeFromMesh)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createOctreeFromMesh"));
    geomPlugin_createOctreeFromOctree=(ptr_geomPlugin_createOctreeFromOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createOctreeFromOctree"));
    geomPlugin_copyOctree=(ptr_geomPlugin_copyOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_copyOctree"));
    geomPlugin_getOctreeFromSerializationData=(ptr_geomPlugin_getOctreeFromSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeFromSerializationData"));
    geomPlugin_getOctreeSerializationData=(ptr_geomPlugin_getOctreeSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeSerializationData"));
    geomPlugin_getOctreeFromSerializationData_float=(ptr_geomPlugin_getOctreeFromSerializationData_float)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeFromSerializationData_float"));
    geomPlugin_getOctreeSerializationData_float=(ptr_geomPlugin_getOctreeSerializationData_float)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeSerializationData_float"));
    geomPlugin_scaleOctree=(ptr_geomPlugin_scaleOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_scaleOctree"));
    geomPlugin_destroyOctree=(ptr_geomPlugin_destroyOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_destroyOctree"));
    geomPlugin_getOctreeVoxelData=(ptr_geomPlugin_getOctreeVoxelData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeVoxelData"));
    geomPlugin_getOctreeUserData=(ptr_geomPlugin_getOctreeUserData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeUserData"));
    geomPlugin_getOctreeCornersFromOctree=(ptr_geomPlugin_getOctreeCornersFromOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeCornersFromOctree"));
    geomPlugin_insertPointsIntoOctree=(ptr_geomPlugin_insertPointsIntoOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertPointsIntoOctree"));
    geomPlugin_insertColorPointsIntoOctree=(ptr_geomPlugin_insertColorPointsIntoOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertColorPointsIntoOctree"));
    geomPlugin_insertMeshIntoOctree=(ptr_geomPlugin_insertMeshIntoOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertMeshIntoOctree"));
    geomPlugin_insertOctreeIntoOctree=(ptr_geomPlugin_insertOctreeIntoOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertOctreeIntoOctree"));
    geomPlugin_removePointsFromOctree=(ptr_geomPlugin_removePointsFromOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_removePointsFromOctree"));
    geomPlugin_removeMeshFromOctree=(ptr_geomPlugin_removeMeshFromOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_removeMeshFromOctree"));
    geomPlugin_removeOctreeFromOctree=(ptr_geomPlugin_removeOctreeFromOctree)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_removeOctreeFromOctree"));
    geomPlugin_createPtcloudFromPoints=(ptr_geomPlugin_createPtcloudFromPoints)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createPtcloudFromPoints"));
    geomPlugin_createPtcloudFromColorPoints=(ptr_geomPlugin_createPtcloudFromColorPoints)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_createPtcloudFromColorPoints"));
    geomPlugin_copyPtcloud=(ptr_geomPlugin_copyPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_copyPtcloud"));
    geomPlugin_getPtcloudFromSerializationData=(ptr_geomPlugin_getPtcloudFromSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudFromSerializationData"));
    geomPlugin_getPtcloudSerializationData=(ptr_geomPlugin_getPtcloudSerializationData)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudSerializationData"));
    geomPlugin_getPtcloudFromSerializationData_float=(ptr_geomPlugin_getPtcloudFromSerializationData_float)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudFromSerializationData_float"));
    geomPlugin_getPtcloudSerializationData_float=(ptr_geomPlugin_getPtcloudSerializationData_float)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudSerializationData_float"));
    geomPlugin_scalePtcloud=(ptr_geomPlugin_scalePtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_scalePtcloud"));
    geomPlugin_destroyPtcloud=(ptr_geomPlugin_destroyPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_destroyPtcloud"));
    geomPlugin_getPtcloudPoints=(ptr_geomPlugin_getPtcloudPoints)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudPoints"));
    geomPlugin_getPtcloudOctreeCorners=(ptr_geomPlugin_getPtcloudOctreeCorners)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudOctreeCorners"));
    geomPlugin_getPtcloudNonEmptyCellCount=(ptr_geomPlugin_getPtcloudNonEmptyCellCount)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudNonEmptyCellCount"));
    geomPlugin_insertPointsIntoPtcloud=(ptr_geomPlugin_insertPointsIntoPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertPointsIntoPtcloud"));
    geomPlugin_insertColorPointsIntoPtcloud=(ptr_geomPlugin_insertColorPointsIntoPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_insertColorPointsIntoPtcloud"));
    geomPlugin_removePointsFromPtcloud=(ptr_geomPlugin_removePointsFromPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_removePointsFromPtcloud"));
    geomPlugin_removeOctreeFromPtcloud=(ptr_geomPlugin_removeOctreeFromPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_removeOctreeFromPtcloud"));
    geomPlugin_intersectPointsWithPtcloud=(ptr_geomPlugin_intersectPointsWithPtcloud)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_intersectPointsWithPtcloud"));
    geomPlugin_getMeshMeshCollision=(ptr_geomPlugin_getMeshMeshCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshMeshCollision"));
    geomPlugin_getMeshOctreeCollision=(ptr_geomPlugin_getMeshOctreeCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshOctreeCollision"));
    geomPlugin_getMeshTriangleCollision=(ptr_geomPlugin_getMeshTriangleCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshTriangleCollision"));
    geomPlugin_getMeshSegmentCollision=(ptr_geomPlugin_getMeshSegmentCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshSegmentCollision"));
    geomPlugin_getOctreeOctreeCollision=(ptr_geomPlugin_getOctreeOctreeCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeOctreeCollision"));
    geomPlugin_getOctreePtcloudCollision=(ptr_geomPlugin_getOctreePtcloudCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreePtcloudCollision"));
    geomPlugin_getOctreeTriangleCollision=(ptr_geomPlugin_getOctreeTriangleCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeTriangleCollision"));
    geomPlugin_getOctreeSegmentCollision=(ptr_geomPlugin_getOctreeSegmentCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeSegmentCollision"));
    geomPlugin_getOctreePointsCollision=(ptr_geomPlugin_getOctreePointsCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreePointsCollision"));
    geomPlugin_getOctreePointCollision=(ptr_geomPlugin_getOctreePointCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreePointCollision"));
    geomPlugin_getBoxBoxCollision=(ptr_geomPlugin_getBoxBoxCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxBoxCollision"));
    geomPlugin_getBoxTriangleCollision=(ptr_geomPlugin_getBoxTriangleCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxTriangleCollision"));
    geomPlugin_getBoxSegmentCollision=(ptr_geomPlugin_getBoxSegmentCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxSegmentCollision"));
    geomPlugin_getBoxPointCollision=(ptr_geomPlugin_getBoxPointCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxPointCollision"));
    geomPlugin_getTriangleTriangleCollision=(ptr_geomPlugin_getTriangleTriangleCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getTriangleTriangleCollision"));
    geomPlugin_getTriangleSegmentCollision=(ptr_geomPlugin_getTriangleSegmentCollision)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getTriangleSegmentCollision"));
    geomPlugin_getMeshMeshDistanceIfSmaller=(ptr_geomPlugin_getMeshMeshDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshMeshDistanceIfSmaller"));
    geomPlugin_getMeshOctreeDistanceIfSmaller=(ptr_geomPlugin_getMeshOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshOctreeDistanceIfSmaller"));
    geomPlugin_getMeshPtcloudDistanceIfSmaller=(ptr_geomPlugin_getMeshPtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshPtcloudDistanceIfSmaller"));
    geomPlugin_getMeshTriangleDistanceIfSmaller=(ptr_geomPlugin_getMeshTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshTriangleDistanceIfSmaller"));
    geomPlugin_getMeshSegmentDistanceIfSmaller=(ptr_geomPlugin_getMeshSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshSegmentDistanceIfSmaller"));
    geomPlugin_getMeshPointDistanceIfSmaller=(ptr_geomPlugin_getMeshPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getMeshPointDistanceIfSmaller"));
    geomPlugin_getOctreeOctreeDistanceIfSmaller=(ptr_geomPlugin_getOctreeOctreeDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeOctreeDistanceIfSmaller"));
    geomPlugin_getOctreePtcloudDistanceIfSmaller=(ptr_geomPlugin_getOctreePtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreePtcloudDistanceIfSmaller"));
    geomPlugin_getOctreeTriangleDistanceIfSmaller=(ptr_geomPlugin_getOctreeTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeTriangleDistanceIfSmaller"));
    geomPlugin_getOctreeSegmentDistanceIfSmaller=(ptr_geomPlugin_getOctreeSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreeSegmentDistanceIfSmaller"));
    geomPlugin_getOctreePointDistanceIfSmaller=(ptr_geomPlugin_getOctreePointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getOctreePointDistanceIfSmaller"));
    geomPlugin_getPtcloudPtcloudDistanceIfSmaller=(ptr_geomPlugin_getPtcloudPtcloudDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudPtcloudDistanceIfSmaller"));
    geomPlugin_getPtcloudTriangleDistanceIfSmaller=(ptr_geomPlugin_getPtcloudTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudTriangleDistanceIfSmaller"));
    geomPlugin_getPtcloudSegmentDistanceIfSmaller=(ptr_geomPlugin_getPtcloudSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudSegmentDistanceIfSmaller"));
    geomPlugin_getPtcloudPointDistanceIfSmaller=(ptr_geomPlugin_getPtcloudPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getPtcloudPointDistanceIfSmaller"));
    geomPlugin_getApproxBoxBoxDistance=(ptr_geomPlugin_getApproxBoxBoxDistance)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getApproxBoxBoxDistance"));
    geomPlugin_getBoxBoxDistanceIfSmaller=(ptr_geomPlugin_getBoxBoxDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxBoxDistanceIfSmaller"));
    geomPlugin_getBoxTriangleDistanceIfSmaller=(ptr_geomPlugin_getBoxTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxTriangleDistanceIfSmaller"));
    geomPlugin_getBoxSegmentDistanceIfSmaller=(ptr_geomPlugin_getBoxSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxSegmentDistanceIfSmaller"));
    geomPlugin_getBoxPointDistanceIfSmaller=(ptr_geomPlugin_getBoxPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getBoxPointDistanceIfSmaller"));
    geomPlugin_getTriangleTriangleDistanceIfSmaller=(ptr_geomPlugin_getTriangleTriangleDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getTriangleTriangleDistanceIfSmaller"));
    geomPlugin_getTriangleSegmentDistanceIfSmaller=(ptr_geomPlugin_getTriangleSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getTriangleSegmentDistanceIfSmaller"));
    geomPlugin_getTrianglePointDistanceIfSmaller=(ptr_geomPlugin_getTrianglePointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getTrianglePointDistanceIfSmaller"));
    geomPlugin_getSegmentSegmentDistanceIfSmaller=(ptr_geomPlugin_getSegmentSegmentDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getSegmentSegmentDistanceIfSmaller"));
    geomPlugin_getSegmentPointDistanceIfSmaller=(ptr_geomPlugin_getSegmentPointDistanceIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_getSegmentPointDistanceIfSmaller"));
    geomPlugin_volumeSensorDetectMeshIfSmaller=(ptr_geomPlugin_volumeSensorDetectMeshIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_volumeSensorDetectMeshIfSmaller"));
    geomPlugin_volumeSensorDetectOctreeIfSmaller=(ptr_geomPlugin_volumeSensorDetectOctreeIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_volumeSensorDetectOctreeIfSmaller"));
    geomPlugin_volumeSensorDetectPtcloudIfSmaller=(ptr_geomPlugin_volumeSensorDetectPtcloudIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_volumeSensorDetectPtcloudIfSmaller"));
    geomPlugin_volumeSensorDetectTriangleIfSmaller=(ptr_geomPlugin_volumeSensorDetectTriangleIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_volumeSensorDetectTriangleIfSmaller"));
    geomPlugin_volumeSensorDetectSegmentIfSmaller=(ptr_geomPlugin_volumeSensorDetectSegmentIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_volumeSensorDetectSegmentIfSmaller"));
    geomPlugin_raySensorDetectMeshIfSmaller=(ptr_geomPlugin_raySensorDetectMeshIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_raySensorDetectMeshIfSmaller"));
    geomPlugin_raySensorDetectOctreeIfSmaller=(ptr_geomPlugin_raySensorDetectOctreeIfSmaller)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_raySensorDetectOctreeIfSmaller"));
    geomPlugin_isPointInVolume=(ptr_geomPlugin_isPointInVolume)(VVarious::resolveLibraryFuncName(instance,"geomPlugin_isPointInVolume"));
    if (geomPlugin_createMesh!=nullptr)
        App::worldContainer->pluginContainer->currentGeomPlugin=this;

    // For the IK plugin:
    oldIkPlugin_createEnv=(ptr_oldIkPlugin_createEnv)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_createEnv"));
    oldIkPlugin_eraseEnvironment=(ptr_oldIkPlugin_eraseEnvironment)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_eraseEnvironment"));
    oldIkPlugin_eraseObject=(ptr_oldIkPlugin_eraseObject)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_eraseObject"));
    oldIkPlugin_setObjectParent=(ptr_oldIkPlugin_setObjectParent)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setObjectParent"));
    oldIkPlugin_createDummy=(ptr_oldIkPlugin_createDummy)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_createDummy"));
    oldIkPlugin_setLinkedDummy=(ptr_oldIkPlugin_setLinkedDummy)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setLinkedDummy"));
    oldIkPlugin_createJoint=(ptr_oldIkPlugin_createJoint)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_createJoint"));
    oldIkPlugin_setJointMode=(ptr_oldIkPlugin_setJointMode)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointMode"));
    oldIkPlugin_setJointInterval=(ptr_oldIkPlugin_setJointInterval)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointInterval"));
    oldIkPlugin_setJointScrewPitch=(ptr_oldIkPlugin_setJointScrewPitch)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointScrewPitch"));
    oldIkPlugin_setJointIkWeight=(ptr_oldIkPlugin_setJointIkWeight)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointIkWeight"));
    oldIkPlugin_setJointMaxStepSize=(ptr_oldIkPlugin_setJointMaxStepSize)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointMaxStepSize"));
    oldIkPlugin_setJointDependency=(ptr_oldIkPlugin_setJointDependency)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointDependency"));
    oldIkPlugin_getJointPosition=(ptr_oldIkPlugin_getJointPosition)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getJointPosition"));
    oldIkPlugin_setJointPosition=(ptr_oldIkPlugin_setJointPosition)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setJointPosition"));
    oldIkPlugin_getSphericalJointQuaternion=(ptr_oldIkPlugin_getSphericalJointQuaternion)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getSphericalJointQuaternion"));
    oldIkPlugin_setSphericalJointQuaternion=(ptr_oldIkPlugin_setSphericalJointQuaternion)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setSphericalJointQuaternion"));
    oldIkPlugin_createIkGroup=(ptr_oldIkPlugin_createIkGroup)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_createIkGroup"));
    oldIkPlugin_eraseIkGroup=(ptr_oldIkPlugin_eraseIkGroup)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_eraseIkGroup"));
    oldIkPlugin_setIkGroupFlags=(ptr_oldIkPlugin_setIkGroupFlags)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkGroupFlags"));
    oldIkPlugin_setIkGroupCalculation=(ptr_oldIkPlugin_setIkGroupCalculation)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkGroupCalculation"));
    oldIkPlugin_addIkElement=(ptr_oldIkPlugin_addIkElement)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_addIkElement"));
    oldIkPlugin_eraseIkElement=(ptr_oldIkPlugin_eraseIkElement)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_eraseIkElement"));
    oldIkPlugin_setIkElementFlags=(ptr_oldIkPlugin_setIkElementFlags)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkElementFlags"));
    oldIkPlugin_setIkElementBase=(ptr_oldIkPlugin_setIkElementBase)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkElementBase"));
    oldIkPlugin_setIkElementConstraints=(ptr_oldIkPlugin_setIkElementConstraints)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkElementConstraints"));
    oldIkPlugin_setIkElementPrecision=(ptr_oldIkPlugin_setIkElementPrecision)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkElementPrecision"));
    oldIkPlugin_setIkElementWeights=(ptr_oldIkPlugin_setIkElementWeights)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setIkElementWeights"));
    oldIkPlugin_handleIkGroup=(ptr_oldIkPlugin_handleIkGroup)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_handleIkGroup"));
    oldIkPlugin_computeJacobian=(ptr_oldIkPlugin_computeJacobian)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_computeJacobian"));
    oldIkPlugin_getJacobian=(ptr_oldIkPlugin_getJacobian)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getJacobian"));
    oldIkPlugin_getManipulability=(ptr_oldIkPlugin_getManipulability)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getManipulability"));
    oldIkPlugin_getConfigForTipPose=(ptr_oldIkPlugin_getConfigForTipPose)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getConfigForTipPose"));
    oldIkPlugin_getObjectLocalTransformation=(ptr_oldIkPlugin_getObjectLocalTransformation)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_getObjectLocalTransformation"));
    oldIkPlugin_setObjectLocalTransformation=(ptr_oldIkPlugin_setObjectLocalTransformation)(VVarious::resolveLibraryFuncName(instance,"ikPlugin_setObjectLocalTransformation"));
    if (oldIkPlugin_createEnv!=nullptr)
    {
        App::worldContainer->pluginContainer->currentIKPlugin=this;
        pushCurrentPlugin();
        App::worldContainer->pluginContainer->ikEnvironment=App::worldContainer->pluginContainer->currentIKPlugin->oldIkPlugin_createEnv();
        popCurrentPlugin();
    }

    // For the Ruckig plugin:
    ruckigPlugin_pos=(ptrRuckigPlugin_pos)(VVarious::resolveLibraryFuncName(instance,"ruckigPlugin_pos"));
    ruckigPlugin_vel=(ptrRuckigPlugin_vel)(VVarious::resolveLibraryFuncName(instance,"ruckigPlugin_vel"));
    ruckigPlugin_step=(ptrRuckigPlugin_step)(VVarious::resolveLibraryFuncName(instance,"ruckigPlugin_step"));
    ruckigPlugin_remove=(ptrRuckigPlugin_remove)(VVarious::resolveLibraryFuncName(instance,"ruckigPlugin_remove"));
    ruckigPlugin_dofs=(ptrRuckigPlugin_dofs)(VVarious::resolveLibraryFuncName(instance,"ruckigPlugin_dofs"));
    if (ruckigPlugin_pos!=nullptr)
        App::worldContainer->pluginContainer->currentRuckigPlugin=this;

    // For the code editor:
    _codeEditor_openModal=(ptrCodeEditor_openModal)(VVarious::resolveLibraryFuncName(instance,"codeEditor_openModal"));
    _codeEditor_open=(ptrCodeEditor_open)(VVarious::resolveLibraryFuncName(instance,"codeEditor_open"));
    _codeEditor_setText=(ptrCodeEditor_setText)(VVarious::resolveLibraryFuncName(instance,"codeEditor_setText"));
    _codeEditor_getText=(ptrCodeEditor_getText)(VVarious::resolveLibraryFuncName(instance,"codeEditor_getText"));
    _codeEditor_show=(ptrCodeEditor_show)(VVarious::resolveLibraryFuncName(instance,"codeEditor_show"));
    _codeEditor_close=(ptrCodeEditor_close)(VVarious::resolveLibraryFuncName(instance,"codeEditor_close"));
    if (_codeEditor_openModal!=nullptr)
        App::worldContainer->pluginContainer->currentCodeEditorPlugin=this;

    // Following 2 only used with deprecated API functions:
    _customUi_msgBox=(ptrCustomUi_msgBox)(VVarious::resolveLibraryFuncName(instance,"customUi_msgBox"));
    _customUi_fileDialog=(ptrCustomUi_fileDialog)(VVarious::resolveLibraryFuncName(instance,"customUi_fileDialog"));
    if (_customUi_msgBox!=nullptr)
        App::worldContainer->pluginContainer->currentUIPlugin=this;

    _assimp_importShapes=(ptrassimp_importShapes)(VVarious::resolveLibraryFuncName(instance,"assimp_importShapes"));
    _assimp_exportShapes=(ptrassimp_exportShapes)(VVarious::resolveLibraryFuncName(instance,"assimp_exportShapes"));
    _assimp_importMeshes=(ptrassimp_importMeshes)(VVarious::resolveLibraryFuncName(instance,"assimp_importMeshes"));
    _assimp_exportMeshes=(ptrassimp_exportMeshes)(VVarious::resolveLibraryFuncName(instance,"assimp_exportMeshes"));
    if (_assimp_importShapes!=nullptr)
        App::worldContainer->pluginContainer->currentAssimpPlugin=this;
}

