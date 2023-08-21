#include <app.h>
#include <pluginContainer.h>
#include <simLib/simConst.h>
#include <pathPlanningInterface.h>
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

#define SIMIK0_DEFAULT "simIK0"
#define SIMUI_DEFAULT "simUI"
#define SIMBULLET278_DEFAULT "simBullet-2-78"
#define SIMBULLET283_DEFAULT "simBullet-2-83"
#define SIMODE_DEFAULT "simODE"
#define SIMVORTEX_DEFAULT "simVortex"
#define SIMNEWTON_DEFAULT "simNewton"
#define SIMMUJOCO_DEFAULT "simMujoco"
#define SIMGEOM_DEFAULT "simGeom"
#define SIMCODEEDITOR_DEFAULT "simCodeEditor"
#define SIMRUCKIG_DEFAULT "simRuckig"
#define SIMASSIMP_DEFAULT "simAssimp"
#define SIMQHULL_DEFAULT "simQHull"
#define SIMCONVEXDECOMPOSE_DEFAULT "simConvexDecompose"
#define SIMOPENMESH_DEFAULT "simOpenMesh"
#define SIMPOVRAY_DEFAULT "simPovRay"
#define SIMOPENGL3_DEFAULT "simOpenGL3"

CPluginContainer::CPluginContainer()
{
    _nextHandle=0;

    currentDynEngine=nullptr;
    bullet278Engine=nullptr;
    bullet283Engine=nullptr;
    odeEngine=nullptr;
    vortexEngine=nullptr;
    newtonEngine=nullptr;
    mujocoEngine=nullptr;
    currentGeomPlugin=nullptr;
    currentIKPlugin=nullptr;
    currentCodeEditorPlugin=nullptr;
    currentRuckigPlugin=nullptr;
    currentUIPlugin=nullptr;
    currentAssimpPlugin=nullptr;
    currentExternalRendererPlugin=nullptr;
    currentPovRayPlugin=nullptr;
    currentOpenGl3Plugin=nullptr;
    currentQHullPlugin=nullptr;
    currentConvexDecomposePlugin=nullptr;
    currentMeshDecimationPlugin=nullptr;

    ikEnvironment=-1;
}

CPluginContainer::~CPluginContainer()
{
}

CPlugin* CPluginContainer::getCurrentPlugin()
{
    if (currentPluginStack.size()==0)
        return(nullptr);
    return(currentPluginStack[currentPluginStack.size()-1]);
}

CPlugin* CPluginContainer::_tryToLoadPluginOnce(const char* namespaceAndVersion)
{
    CPlugin* retVal=nullptr;
    if (_autoLoadPluginsTrials.find(namespaceAndVersion)==_autoLoadPluginsTrials.end())
    {
        retVal=loadAndInitPlugin(namespaceAndVersion,-1);
        if (retVal==nullptr)
            _autoLoadPluginsTrials.insert(namespaceAndVersion); // memorize that we failed
    }
    return(retVal);
}

CPlugin* CPluginContainer::loadAndInitPlugin(const char* namespaceAndVersion,int loadOrigin)
{ // namespaceAndVersion: e.g. simAssimp, simAssimp-2-78, etc.
    // loadOrigin: -1: c++, otherwise script handle
    TRACE_INTERNAL;
    CPlugin* plug=getPluginFromName(namespaceAndVersion);
    if (plug==nullptr)
    {
        std::string fileAndPath(App::folders->getExecutablePath()+"/");
#ifndef WIN_SIM
        fileAndPath+="lib";
#endif
        fileAndPath+=namespaceAndVersion;
#ifdef WIN_SIM
        fileAndPath+=".dll";
#endif
#ifdef LIN_SIM
        fileAndPath+=".so";
#endif
#ifdef MAC_SIM
        fileAndPath+=".dylib";
#endif
        std::string msgB("plugin ");
        msgB+=std::string(namespaceAndVersion)+": ";
        std::string msg(msgB+"loading...");
        CScriptObject* scr=App::worldContainer->getScriptFromHandle(loadOrigin);
        if (scr!=nullptr)
            App::logScriptMsg(scr,sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
        else
            App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
        plug=new CPlugin(fileAndPath.c_str(),namespaceAndVersion,loadOrigin);
        plug->setHandle(_nextHandle);
        _allPlugins.push_back(plug);
        std::string errMsg;

        int loadRes=plug->load(&errMsg);
        if (loadRes==1)
        {
            if (plug->init(&errMsg))
            {
                msg=msgB+"done.";
                if (scr!=nullptr)
                    App::logScriptMsg(scr,sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
                else
                    App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
                loadRes=2;
                _nextHandle++;
            }
            else
            {
                msg=msgB+"init failed.";
                if (scr!=nullptr)
                    App::logScriptMsg(scr,sim_verbosity_errors,msg.c_str());
                else
                    App::logMsg(sim_verbosity_errors,msg.c_str());
            }
        }
        else
        {
            msg=msgB+errMsg;
            if (scr!=nullptr)
                App::logScriptMsg(scr,sim_verbosity_errors,msg.c_str());
            else
                App::logMsg(sim_verbosity_errors,msg.c_str());
        }
        if (loadRes<2)
        { // failed
            _allPlugins.pop_back();
            delete plug;
            plug=nullptr;
        }
    }
    else
        plug->addDependency(loadOrigin);

    if (plug!=nullptr)
        CSimFlavor::getIntVal_str(3,plug->getName().c_str());

    return(plug);
}

void CPluginContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    int pluginData[4]={scriptHandle,0,0,0};
    sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scriptstatedestroyed,pluginData);

    for (size_t i=0;i<_allPlugins.size();i++)
        _allPlugins[i]->removeDependency(scriptHandle);
}

int CPluginContainer::addAndInitPlugin_old(const char* filename,const char* pluginName)
{ // pluginName: e.g. Assimp, IK, etc.
    TRACE_INTERNAL;
    App::logMsg(sim_verbosity_debug,(std::string("addPlugin: ")+pluginName).c_str());

    CPlugin* plug=getPluginFromName_old(pluginName,true);
    if (plug!=nullptr)
        return(plug->getHandle());
    plug=new CPlugin(filename,pluginName,-2);
    plug->setHandle(_nextHandle);
    _allPlugins.push_back(plug);
    std::string errStr;
    int loadRes=plug->loadAndInit_old(&errStr);
    if (loadRes<=0)
    { // failed
        if (errStr.size()>0)
            App::logMsg(sim_verbosity_errors,errStr.c_str());
        _allPlugins.pop_back();
        delete plug;
        return(loadRes-1);
    }
    _nextHandle++;
    return(plug->getHandle());
}
CPlugin* CPluginContainer::getPluginFromName(const char* pluginNamespaceAndVersion)
{
    CPlugin* retVal=nullptr;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->getName()==pluginNamespaceAndVersion)
        {
            retVal=_allPlugins[i];
            break;
        }
    }
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromName_old(const char* pluginName,bool caseSensitive)
{
    CPlugin* retVal=nullptr;
    std::string thatPl(pluginName);
    if (!caseSensitive)
        std::transform(thatPl.begin(),thatPl.end(),thatPl.begin(),::tolower);
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        std::string thisPl(_allPlugins[i]->getName());
        if (!caseSensitive)
            std::transform(thisPl.begin(),thisPl.end(),thisPl.begin(),::tolower);
        if (thisPl.compare(thatPl)==0)
        {
            retVal=_allPlugins[i];
            break;
        }
    }
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromIndex(size_t index)
{
    CPlugin* retVal=nullptr;
    if (index<_allPlugins.size())
        retVal=_allPlugins[index];
    return(retVal);
}

CPlugin* CPluginContainer::getPluginFromHandle(int handle)
{
    CPlugin* retVal=nullptr;
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->getHandle()==handle)
        {
            retVal=_allPlugins[i];
            break;
        }
    }
    return(retVal);
}

void CPluginContainer::lockInterface()
{
    _pluginInterfaceMutex.lock();
}

void CPluginContainer::unlockInterface()
{
    _pluginInterfaceMutex.unlock();
}

bool CPluginContainer::deinitAndUnloadPlugin(int handle,int unloadOrigin,bool force/*=false*/)
{ // not for legacy plugins
    TRACE_INTERNAL;
    bool retVal=false;
    CPlugin* it=getPluginFromHandle(handle);
    if ( (it!=nullptr)&&(!it->isLegacyPlugin()) )
    {
        it->removeDependency(unloadOrigin);
        if ( force||(!it->hasAnyDependency()) )
        {
            std::string msgB("plugin ");
            msgB+=it->getName()+": ";
            std::string msg(msgB+"cleanup...");
            CScriptObject* scr=App::worldContainer->getScriptFromHandle(unloadOrigin);
            if (scr!=nullptr)
                App::logScriptMsg(scr,sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
            else
                App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
            lockInterface();
            it->cleanup();
            retVal=true;
            for (size_t i=0;i<_allPlugins.size();i++)
            {
                if (_allPlugins[i]==it)
                {
                    delete _allPlugins[i];
                    _allPlugins.erase(_allPlugins.begin()+i);
                    break;
                }
            }
            msg=msgB+"done.";
            if (scr!=nullptr)
                App::logScriptMsg(scr,sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
            else
                App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,msg.c_str());
            unlockInterface();
        }
    }
    return(retVal);
}

void CPluginContainer::unloadNewPlugins()
{
    for (int i=int(_allPlugins.size())-1;i>=0;i--)
    {
        if (!_allPlugins[i]->isLegacyPlugin())
            deinitAndUnloadPlugin(_allPlugins[i]->getHandle(),-1);
    }
}

void CPluginContainer::unloadLegacyPlugins()
{
    for (int i=int(_allPlugins.size())-1;i>=0;i--)
    {
        if (_allPlugins[i]->isLegacyPlugin())
            simUnloadModule_internal(_allPlugins[i]->getHandle());
    }
}

void CPluginContainer::_removePlugin_old(int handle)
{
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        if (_allPlugins[i]->getHandle()==handle)
        {
            delete _allPlugins[i];
            _allPlugins.erase(_allPlugins.begin()+i);
            break;
        }
    }
}

bool CPluginContainer::unloadPlugin_old(int handle)
{
    TRACE_INTERNAL;
    bool retVal=false;
    CPlugin* it=getPluginFromHandle(handle);
    if (it!=nullptr)
    {
        App::logMsg(sim_verbosity_debug,(std::string("unloadPlugin_old: ")+it->getName()).c_str());
#ifdef SIM_WITH_GUI
        if ( (GuiApp::mainWindow!=nullptr)&&(it->getName().compare("CodeEditor")==0) )
            GuiApp::mainWindow->codeEditorContainer->closeAll();
#endif
        it->cleanup();
        std::string nm(it->getName());
        _removePlugin_old(handle);
        App::worldContainer->scriptCustomFuncAndVarContainer->announcePluginWasKilled(nm.c_str());
        retVal=true;
    }
    return(retVal);
}

int CPluginContainer::getPluginCount()
{
    return(int(_allPlugins.size()));
}

void CPluginContainer::uiCallAllPlugins(int msg,int* auxData/*=nullptr*/,void* auxPointer/*=nullptr*/)
{
    lockInterface();
    size_t index=0;
    while (index<_allPlugins.size())
    {
        CPlugin* plug=_allPlugins[index];
        plug->msg_ui(msg,auxData,auxPointer);
        index++;
    }
    unlockInterface();
}

void CPluginContainer::sendEventCallbackMessageToAllPlugins(int msg,int* auxData/*=nullptr*/,void* auxPointer/*=nullptr*/)
{
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        CPlugin* plug=_allPlugins[size_t(i)];
        plug->msg(msg,auxData,auxPointer);
    }
}

void CPluginContainer::sendEventCallbackMessageToAllPlugins_old(int msg,int* auxVals/*=nullptr*/,void* data/*=nullptr*/,int retVals[4]/*=nullptr*/)
{
    bool special=false;
    int memorized[4]={0,0,0,0};
    for (size_t i=0;i<_allPlugins.size();i++)
    {
        CPlugin* plug=_allPlugins[i];
        if (plug->isLegacyPlugin())
        { // only old plugins
            if (retVals!=nullptr)
            {
                retVals[0]=-1;
                retVals[1]=-1;
                retVals[2]=-1;
                retVals[3]=-1;
            }
            bool retData=plug->msg(msg,auxVals,data,retVals);
            if ( retData||((retVals!=nullptr)&&((retVals[0]!=-1)||(retVals[1]!=-1)||(retVals[2]!=-1)||(retVals[3]!=-1))) )
            {
                if (msg!=sim_message_eventcallback_mainscriptabouttobecalled) // this message is handled in a special fashion, because the remoteApi and ROS might interfere otherwise!
                    return; // We interrupt the callback forwarding!
                memorized[0]=retVals[0];
                memorized[1]=retVals[1];
                memorized[2]=retVals[2];
                memorized[3]=retVals[3];
                special=true;
            }
        }
    }
    if (special)
    {
        retVals[0]=memorized[0];
        retVals[1]=memorized[1];
        retVals[2]=memorized[2];
        retVals[3]=memorized[3];
    }
}

bool CPluginContainer::selectExtRenderer(int index)
{
    currentExternalRendererPlugin=nullptr;
    if (index==sim_rendermode_povray-sim_rendermode_povray)
    {
        if (currentPovRayPlugin==nullptr)
        {
            #ifdef SIM_WITH_GUI
                if (VThread::isSimThread())
                    currentPovRayPlugin=_tryToLoadPluginOnce(SIMPOVRAY_DEFAULT);
                else
                    GuiApp::appendSimulationThreadCommand(SELECT_RENDERER_CMD,index);
            #else
                currentPovRayPlugin=_tryToLoadPluginOnce(SIMPOVRAY_DEFAULT);
            #endif
        }
        currentExternalRendererPlugin=currentPovRayPlugin;
    }
    if (index==sim_rendermode_opengl3-sim_rendermode_povray)
    {
        if (currentOpenGl3Plugin==nullptr)
        {
            #ifdef SIM_WITH_GUI
                if (VThread::isSimThread())
                    currentOpenGl3Plugin=_tryToLoadPluginOnce(SIMOPENGL3_DEFAULT);
                else
                    GuiApp::appendSimulationThreadCommand(SELECT_RENDERER_CMD,index);
            #else
                currentOpenGl3Plugin=_tryToLoadPluginOnce(SIMOPENGL3_DEFAULT);
            #endif
        }
        currentExternalRendererPlugin=currentOpenGl3Plugin;
    }
    return(currentExternalRendererPlugin!=nullptr);
}

bool CPluginContainer::extRenderer(int msg,void* data)
{
    bool retVal=false;
    if (currentExternalRendererPlugin!=nullptr)
    {
        currentExternalRendererPlugin->pushCurrentPlugin();
        if (currentExternalRendererPlugin==currentPovRayPlugin)
            currentPovRayPlugin->povRayAddr(msg,data);
        if (currentExternalRendererPlugin==currentOpenGl3Plugin)
            currentOpenGl3Plugin->openGl3Addr(msg,data);
        currentExternalRendererPlugin->popCurrentPlugin();
        retVal=true;
    }
    return(retVal);
}

bool CPluginContainer::qhull(void* data)
{
    if (currentQHullPlugin==nullptr)
        currentQHullPlugin=_tryToLoadPluginOnce(SIMQHULL_DEFAULT);

    bool retVal=false;
    if (currentQHullPlugin!=nullptr)
    {
        currentQHullPlugin->pushCurrentPlugin();
        currentQHullPlugin->qhullAddr(data);
        currentQHullPlugin->popCurrentPlugin();
        retVal=true;
    }
    else
        App::logMsg(sim_verbosity_errors,"QHull plugin was not found.");
    return(retVal);
}

bool CPluginContainer::hacd(void* data)
{
    if (currentConvexDecomposePlugin==nullptr)
        currentConvexDecomposePlugin=_tryToLoadPluginOnce(SIMCONVEXDECOMPOSE_DEFAULT);

    bool retVal=false;
    if (currentConvexDecomposePlugin!=nullptr)
    {
        currentConvexDecomposePlugin->pushCurrentPlugin();
        currentConvexDecomposePlugin->hacdAddr(data);
        currentConvexDecomposePlugin->popCurrentPlugin();
        retVal=true;
    }
    return(retVal);
}

bool CPluginContainer::vhacd(void* data)
{
    if (currentConvexDecomposePlugin==nullptr)
        currentConvexDecomposePlugin=_tryToLoadPluginOnce(SIMCONVEXDECOMPOSE_DEFAULT);

    bool retVal=false;
    if (currentConvexDecomposePlugin!=nullptr)
    {
        currentConvexDecomposePlugin->pushCurrentPlugin();
        currentConvexDecomposePlugin->vhacdAddr(data);
        currentConvexDecomposePlugin->popCurrentPlugin();
        retVal=true;
    }
    return(retVal);
}

bool CPluginContainer::meshDecimator(void* data)
{
    if (currentMeshDecimationPlugin==nullptr)
        currentMeshDecimationPlugin=_tryToLoadPluginOnce(SIMOPENMESH_DEFAULT);

    bool retVal=false;
    if (currentMeshDecimationPlugin!=nullptr)
    {
        currentMeshDecimationPlugin->pushCurrentPlugin();
        currentMeshDecimationPlugin->decimatorAddr(data);
        currentMeshDecimationPlugin->popCurrentPlugin();
        retVal=true;
    }
    return(retVal);
}

bool CPluginContainer::dyn_startSimulation(int engine,int version,const double floatParams[20],const int intParams[20])
{
    currentDynEngine=nullptr;
    if (engine==sim_physics_bullet)
    {
        if (version!=283)
        {
            if (bullet278Engine==nullptr)
                _tryToLoadPluginOnce(SIMBULLET278_DEFAULT);
            currentDynEngine=bullet278Engine;
        }
        else
        {
            if (bullet283Engine==nullptr)
                _tryToLoadPluginOnce(SIMBULLET283_DEFAULT);
            currentDynEngine=bullet283Engine;
        }
    }
    if (engine==sim_physics_ode)
    {
        if (odeEngine==nullptr)
            _tryToLoadPluginOnce(SIMODE_DEFAULT);
        currentDynEngine=odeEngine;
    }
    if (engine==sim_physics_vortex)
    {
        if (vortexEngine==nullptr)
            _tryToLoadPluginOnce(SIMVORTEX_DEFAULT);
        currentDynEngine=vortexEngine;
    }
    if (engine==sim_physics_newton)
    {
        if (newtonEngine==nullptr)
            _tryToLoadPluginOnce(SIMNEWTON_DEFAULT);
        currentDynEngine=newtonEngine;
    }
    if (engine==sim_physics_mujoco)
    {
        if (mujocoEngine==nullptr)
            _tryToLoadPluginOnce(SIMMUJOCO_DEFAULT);
        currentDynEngine=mujocoEngine;
    }

    bool retVal=false;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        int res=currentDynEngine->dynPlugin_startSimulation(floatParams,intParams);
        currentDynEngine->popCurrentPlugin();
        retVal=(res!=0);
    }
    return(retVal);
}

void CPluginContainer::dyn_endSimulation()
{
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        currentDynEngine->dynPlugin_endSimulation();
        currentDynEngine->popCurrentPlugin();
    }
    currentDynEngine=nullptr;
}

void CPluginContainer::dyn_step(double timeStep,double simulationTime)
{
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        currentDynEngine->dynPlugin_step(timeStep,simulationTime);
        currentDynEngine->popCurrentPlugin();
    }
}

bool CPluginContainer::dyn_isInitialized()
{
    return(currentDynEngine!=nullptr);
}

bool CPluginContainer::dyn_isDynamicContentAvailable()
{
    int res=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        res=currentDynEngine->dynPlugin_isDynamicContentAvailable();
        currentDynEngine->popCurrentPlugin();
    }
    return(res!=0);
}

void CPluginContainer::dyn_serializeDynamicContent(const char* filenameAndPath,int bulletSerializationBuffer)
{
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        currentDynEngine->dynPlugin_serializeDynamicContent(filenameAndPath,bulletSerializationBuffer);
        currentDynEngine->popCurrentPlugin();
    }
}

int CPluginContainer::dyn_addParticleObject(int objectType,double size,double massOverVolume,const void* params,double lifeTime,int maxItemCount,const float* ambient,const float* diffuse,const float* specular,const float* emission)
{
    int retVal=-1;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        retVal=currentDynEngine->dynPlugin_addParticleObject(objectType,size,massOverVolume,params,lifeTime,maxItemCount,ambient,diffuse,specular,emission);
        currentDynEngine->popCurrentPlugin();
    }
    return(retVal);
}

bool CPluginContainer::dyn_removeParticleObject(int objectHandle)
{
    int res=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        res=currentDynEngine->dynPlugin_removeParticleObject(objectHandle);
        currentDynEngine->popCurrentPlugin();
    }
    return(res!=0);
}

bool CPluginContainer::dyn_addParticleObjectItem(int objectHandle,const double* itemData,double simulationTime)
{
    int res=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        res=currentDynEngine->dynPlugin_addParticleObjectItem(objectHandle,itemData,simulationTime);
        currentDynEngine->popCurrentPlugin();
    }
    return(res!=0);
}

int CPluginContainer::dyn_getParticleObjectOtherFloatsPerItem(int objectHandle)
{
    int retVal=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        retVal=currentDynEngine->dynPlugin_getParticleObjectOtherFloatsPerItem(objectHandle);
        currentDynEngine->popCurrentPlugin();
    }
    return(retVal);
}

double* CPluginContainer::dyn_getContactPoints(int* count)
{
    double* retVal=nullptr;
    count[0]=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        retVal=currentDynEngine->dynPlugin_getContactPoints(count);
        currentDynEngine->popCurrentPlugin();
    }
    return(retVal);
}

void** CPluginContainer::dyn_getParticles(int index,int* particlesCount,int* objectType,float** cols)
{
    void** retVal=nullptr;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        retVal=currentDynEngine->dynPlugin_getParticles(index,particlesCount,objectType,cols);
        currentDynEngine->popCurrentPlugin();
    }
    return(retVal);
}

bool CPluginContainer::dyn_getParticleData(const void* particle,double* pos,double* size,int* objectType,float** additionalColor)
{
    int res=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        res=currentDynEngine->dynPlugin_getParticleData(particle,pos,size,objectType,additionalColor);
        currentDynEngine->popCurrentPlugin();
    }
    return(res!=0);
}

bool CPluginContainer::dyn_getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],double* contactInfo)
{
    int res=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        res=currentDynEngine->dynPlugin_getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo);
        currentDynEngine->popCurrentPlugin();
    }
    return(res!=0);
}

int CPluginContainer::dyn_getDynamicStepDivider()
{
    int retVal=0;
    if (currentDynEngine!=nullptr)
    {
        currentDynEngine->pushCurrentPlugin();
        retVal=currentDynEngine->dynPlugin_getDynamicStepDivider();
        currentDynEngine->popCurrentPlugin();
    }
    return(retVal);
}

double CPluginContainer::dyn_computePMI(const std::vector<double>& vertices,const std::vector<int>& indices,C7Vector& tr,C3Vector& diagI)
{ // returns the mass-less diagonal inertia. Returned mass is for a density of 1000
    if (mujocoEngine==nullptr)
        mujocoEngine=_tryToLoadPluginOnce(SIMMUJOCO_DEFAULT);
    double mass=0.0;
    if (mujocoEngine!=nullptr)
    {
        mujocoEngine->pushCurrentPlugin();
        mass=mujocoEngine->mujocoPlugin_computePMI(vertices.data(),int(vertices.size()),indices.data(),int(indices.size()),tr.X.data,tr.Q.data,diagI.data);
        mujocoEngine->popCurrentPlugin();
    }
    return(mass);
}

double CPluginContainer::dyn_computeInertia(int shapeHandle,C7Vector& tr,C3Vector& diagI)
{ // returns the mass-less diagonal inertia, relative to the shape's ref frame. Returned mass is for a density of 1000
    if (mujocoEngine==nullptr)
        mujocoEngine=_tryToLoadPluginOnce(SIMMUJOCO_DEFAULT);
    double mass=0.0;
    if (mujocoEngine!=nullptr)
    {
        mujocoEngine->pushCurrentPlugin();
        mass=mujocoEngine->mujocoPlugin_computeInertia(shapeHandle,tr.X.data,tr.Q.data,diagI.data);
        mujocoEngine->popCurrentPlugin();
    }
    if (mass==0.0)
    { // fallback algo
        CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
        if (it->getMesh()->isConvex())
        {
            std::vector<double> vert;
            std::vector<int> ind;
            it->getMesh()->getCumulativeMeshes(C7Vector::identityTransformation,vert,&ind,nullptr);
            C3Vector com;
            C3X3Matrix tensor;
            mass=CVolInt::getMassCenterOfMassAndInertiaTensor(&vert[0],(int)vert.size()/3,&ind[0],(int)ind.size()/3,1000.0,com,tensor);
            C4Vector rot;
            CMeshWrapper::getPMIFromInertia(tensor,rot,diagI);
            tr=C7Vector(rot,com);
        }
    }

    return(mass);
}

bool CPluginContainer::isGeomPluginAvailable()
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    return(currentGeomPlugin!=nullptr);
}

bool CPluginContainer::isCodeEditorPluginAvailable()
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    return(currentCodeEditorPlugin!=nullptr);
}

bool CPluginContainer::isAssimpPluginAvailable()
{
    if (currentAssimpPlugin==nullptr)
        currentAssimpPlugin=_tryToLoadPluginOnce(SIMASSIMP_DEFAULT);

    return(currentAssimpPlugin!=nullptr);
}

void CPluginContainer::geomPlugin_releaseBuffer(void* buffer)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_releaseBuffer(buffer);
        currentGeomPlugin->popCurrentPlugin();
    }
}

void* CPluginContainer::geomPlugin_createMesh(const double* vertices,int verticesSize,const int* indices,int indicesSize,const C7Vector* meshOrigin/*=nullptr*/,double triangleEdgeMaxLength/*=0.3*/,int maxTrianglesInBoundingBox/*=8*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double tr[7];
        double* _tr=nullptr;
        if (meshOrigin!=nullptr)
        {
            meshOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createMesh(vertices,verticesSize,indices,indicesSize,_tr,triangleEdgeMaxLength,maxTrianglesInBoundingBox);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyMesh(const void* meshObbStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_copyMesh(meshObbStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_getMeshFromSerializationData(const unsigned char* serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshFromSerializationData(serializationData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_getMeshSerializationData(const void* meshObbStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned char* data=currentGeomPlugin->geomPlugin_getMeshSerializationData(meshObbStruct,&l);
        currentGeomPlugin->popCurrentPlugin();
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
    }
}
void CPluginContainer::geomPlugin_scaleMesh(void* meshObbStruct,double scalingFactor)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_scaleMesh(meshObbStruct,scalingFactor);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_destroyMesh(void* meshObbStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_destroyMesh(meshObbStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
}
double CPluginContainer::geomPlugin_getMeshRootObbVolume(const void* meshObbStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    double retVal=0.0;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshRootObbVolume(meshObbStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromPoints(const double* points,int pointCnt,const C7Vector* octreeOrigin/*=nullptr*/,double cellS/*=0.05*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double tr[7];
        double* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromPoints(points,pointCnt,_tr,cellS,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromColorPoints(const double* points,int pointCnt,const C7Vector* octreeOrigin/*=nullptr*/,double cellS/*=0.05*/,const unsigned char* rgbData/*=nullptr*/,const unsigned int* usrData/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double tr[7];
        double* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromColorPoints(points,pointCnt,_tr,cellS,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromMesh(const void* meshObbStruct,const C7Vector& meshTransformation,const C7Vector* octreeOrigin/*=nullptr*/,double cellS/*=0.05*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double _meshTr[7];
        meshTransformation.getData(_meshTr);
        double tr[7];
        double* _tr=nullptr;
        if (octreeOrigin!=nullptr)
        {
            octreeOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromMesh(meshObbStruct,_meshTr,_tr,cellS,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createOctreeFromOctree(const void* otherOctreeStruct,const C7Vector& otherOctreeTransformation,const C7Vector* newOctreeOrigin/*=nullptr*/,double newOctreeCellS/*=0.05*/,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double _otherOcTr[7];
        otherOctreeTransformation.getData(_otherOcTr);
        double tr[7];
        double* _tr=nullptr;
        if (newOctreeOrigin!=nullptr)
        {
            newOctreeOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createOctreeFromOctree(otherOctreeStruct,_otherOcTr,_tr,newOctreeCellS,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyOctree(const void* ocStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_copyOctree(ocStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_getOctreeFromSerializationData(const unsigned char* serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeFromSerializationData(serializationData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_getOctreeSerializationData(const void* ocStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned char* data=currentGeomPlugin->geomPlugin_getOctreeSerializationData(ocStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void* CPluginContainer::geomPlugin_getOctreeFromSerializationData_float(const unsigned char* serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeFromSerializationData_float(serializationData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_getOctreeSerializationData_float(const void* ocStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned char* data=currentGeomPlugin->geomPlugin_getOctreeSerializationData_float(ocStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_scaleOctree(void* ocStruct,double f)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_scaleOctree(ocStruct,f);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_destroyOctree(void* ocStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_destroyOctree(ocStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_getOctreeVoxelPositions(const void* ocStruct,std::vector<double>& voxelPositions)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        double* data=currentGeomPlugin->geomPlugin_getOctreeVoxelData(ocStruct,&l);
        if (data!=nullptr)
        {
            voxelPositions.resize(3*l);
            for (int i=0;i<l;i++)
            {
                voxelPositions[3*i+0]=data[6*i+0];
                voxelPositions[3*i+1]=data[6*i+1];
                voxelPositions[3*i+2]=data[6*i+2];
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_getOctreeVoxelColors(const void* ocStruct,std::vector<float>& voxelColors)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        double* data=currentGeomPlugin->geomPlugin_getOctreeVoxelData(ocStruct,&l);
        if (data!=nullptr)
        {
            voxelColors.resize(4*l);
            for (int i=0;i<l;i++)
            {
                voxelColors[4*i+0]=(float)data[6*i+3];
                voxelColors[4*i+1]=(float)data[6*i+4];
                voxelColors[4*i+2]=(float)data[6*i+5];
                voxelColors[4*i+3]=0.0;
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_getOctreeUserData(const void* ocStruct,std::vector<unsigned int>& userData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned int* data=currentGeomPlugin->geomPlugin_getOctreeUserData(ocStruct,&l);
        if (data!=nullptr)
        {
            userData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_getOctreeCornersFromOctree(const void* ocStruct,std::vector<double>& points)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        double* data=currentGeomPlugin->geomPlugin_getOctreeCornersFromOctree(ocStruct,&l);
        if (data!=nullptr)
        {
            points.assign(data,data+3*l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_insertPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const double* points,int pointCnt,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertPointsIntoOctree(ocStruct,_tr,points,pointCnt,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_insertColorPointsIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const double* points,int pointCnt,const unsigned char* rgbData/*=nullptr*/,const unsigned int* usrData/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertColorPointsIntoOctree(ocStruct,_tr,points,pointCnt,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_insertMeshIntoOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        double _tr2[7];
        meshTransformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertMeshIntoOctree(ocStruct,_tr1,obbStruct,_tr2,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_insertOctreeIntoOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,const unsigned char rgbData[3]/*=nullptr*/,unsigned int usrData/*=0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octree1Transformation.getData(_tr1);
        double _tr2[7];
        octree2Transformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertOctreeIntoOctree(oc1Struct,_tr1,oc2Struct,_tr2,rgbData,usrData);
        currentGeomPlugin->popCurrentPlugin();
    }
}
bool CPluginContainer::geomPlugin_removePointsFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const double* points,int pointCnt)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_removePointsFromOctree(ocStruct,_tr,points,pointCnt);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeMeshFromOctree(void* ocStruct,const C7Vector& octreeTransformation,const void* obbStruct,const C7Vector& meshTransformation)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        double _tr2[7];
        meshTransformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_removeMeshFromOctree(ocStruct,_tr1,obbStruct,_tr2);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeOctreeFromOctree(void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octree1Transformation.getData(_tr1);
        double _tr2[7];
        octree2Transformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_removeOctreeFromOctree(oc1Struct,_tr1,oc2Struct,_tr2);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createPtcloudFromPoints(const double* points,int pointCnt,const C7Vector* ptcloudOrigin/*=nullptr*/,double cellS/*=0.05*/,int maxPointCnt/*=20*/,const unsigned char rgbData[3]/*=nullptr*/,double proximityTol/*=0.005*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double tr[7];
        double* _tr=nullptr;
        if (ptcloudOrigin!=nullptr)
        {
            ptcloudOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createPtcloudFromPoints(points,pointCnt,_tr,cellS,maxPointCnt,rgbData,proximityTol);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_createPtcloudFromColorPoints(const double* points,int pointCnt,const C7Vector* ptcloudOrigin/*=nullptr*/,double cellS/*=0.05*/,int maxPointCnt/*=20*/,const unsigned char* rgbData/*=nullptr*/,double proximityTol/*=0.005*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        double tr[7];
        double* _tr=nullptr;
        if (ptcloudOrigin!=nullptr)
        {
            ptcloudOrigin->getData(tr);
            _tr=tr;
        }
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_createPtcloudFromColorPoints(points,pointCnt,_tr,cellS,maxPointCnt,rgbData,proximityTol);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_copyPtcloud(const void* pcStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_copyPtcloud(pcStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void* CPluginContainer::geomPlugin_getPtcloudFromSerializationData(const unsigned char* serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudFromSerializationData(serializationData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_getPtcloudSerializationData(const void* pcStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned char* data=currentGeomPlugin->geomPlugin_getPtcloudSerializationData(pcStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void* CPluginContainer::geomPlugin_getPtcloudFromSerializationData_float(const unsigned char* serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    void* retVal=nullptr;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudFromSerializationData_float(serializationData);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_getPtcloudSerializationData_float(const void* pcStruct,std::vector<unsigned char>& serializationData)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        unsigned char* data=currentGeomPlugin->geomPlugin_getPtcloudSerializationData_float(pcStruct,&l);
        if (data!=nullptr)
        {
            serializationData.assign(data,data+l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_scalePtcloud(void* pcStruct,double f)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_scalePtcloud(pcStruct,f);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_destroyPtcloud(void* pcStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_destroyPtcloud(pcStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_getPtcloudPoints(const void* pcStruct,std::vector<double>& pointData,std::vector<double>* colorData/*=nullptr*/,double prop/*=1.0*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    pointData.clear();
    if (colorData!=nullptr)
        colorData->clear();
    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        double* data=currentGeomPlugin->geomPlugin_getPtcloudPoints(pcStruct,&l,prop);
        if (data!=nullptr)
        {
            for (int i=0;i<l;i++)
            {
                pointData.push_back(data[6*i+0]);
                pointData.push_back(data[6*i+1]);
                pointData.push_back(data[6*i+2]);
                if (colorData!=nullptr)
                {
                    colorData->push_back(data[6*i+3]);
                    colorData->push_back(data[6*i+4]);
                    colorData->push_back(data[6*i+5]);
                    colorData->push_back(1.0);
                }
            }
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}

void CPluginContainer::geomPlugin_getPtcloudOctreeCorners(const void* pcStruct,std::vector<double>& points)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        int l;
        currentGeomPlugin->pushCurrentPlugin();
        double* data=currentGeomPlugin->geomPlugin_getPtcloudOctreeCorners(pcStruct,&l);
        if (data!=nullptr)
        {
            points.assign(data,data+3*l);
            currentGeomPlugin->geomPlugin_releaseBuffer(data);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::geomPlugin_getPtcloudNonEmptyCellCount(const void* pcStruct)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    int retVal=0;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudNonEmptyCellCount(pcStruct);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::geomPlugin_insertPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const double* points,int pointCnt,const unsigned char rgbData[3]/*=nullptr*/,double proximityTol/*=0.001*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertPointsIntoPtcloud(pcStruct,_tr,points,pointCnt,rgbData,proximityTol);
        currentGeomPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::geomPlugin_insertColorPointsIntoPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const double* points,int pointCnt,const unsigned char* rgbData/*=nullptr*/,double proximityTol/*=0.001*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        currentGeomPlugin->geomPlugin_insertColorPointsIntoPtcloud(pcStruct,_tr,points,pointCnt,rgbData,proximityTol);
        currentGeomPlugin->popCurrentPlugin();
    }
}
bool CPluginContainer::geomPlugin_removePointsFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const double* points,int pointCnt,double proximityTol,int* countRemoved/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_removePointsFromPtcloud(pcStruct,_tr,points,pointCnt,proximityTol,countRemoved);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_removeOctreeFromPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* countRemoved/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        double _tr2[7];
        octreeTransformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_removeOctreeFromPtcloud(pcStruct,_tr,ocStruct,_tr2,countRemoved);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_intersectPointsWithPtcloud(void* pcStruct,const C7Vector& ptcloudTransformation,const double* points,int pointCnt,double proximityTol/*=0.001*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_intersectPointsWithPtcloud(pcStruct,_tr,points,pointCnt,proximityTol);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshMeshCollision(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,std::vector<double>* intersections/*=nullptr*/,int* mesh1Caching/*=nullptr*/,int* mesh2Caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        mesh1Transformation.getData(_tr1);
        double _tr2[7];
        mesh2Transformation.getData(_tr2);
        double* _intersections;
        int _intersectionsSize;
        double** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshMeshCollision(mesh1ObbStruct,_tr1,mesh2ObbStruct,_tr2,_int,&_intersectionsSize,mesh1Caching,mesh2Caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshOctreeCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,int* meshCaching/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        meshTransformation.getData(_tr1);
        double _tr2[7];
        octreeTransformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshOctreeCollision(meshObbStruct,_tr1,ocStruct,_tr2,meshCaching,ocCaching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshTriangleCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,std::vector<double>* intersections/*=nullptr*/,int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double* _intersections;
        int _intersectionsSize;
        double** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshTriangleCollision(meshObbStruct,_tr,p.data,v.data,w.data,_int,&_intersectionsSize,caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshSegmentCollision(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,std::vector<double>* intersections/*=nullptr*/,int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double* _intersections;
        int _intersectionsSize;
        double** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshSegmentCollision(meshObbStruct,_tr,segmentExtremity.data,segmentVector.data,_int,&_intersectionsSize,caching);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeOctreeCollision(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,unsigned long long int* oc1Caching/*=nullptr*/,unsigned long long int* oc2Caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octree1Transformation.getData(_tr1);
        double _tr2[7];
        octree2Transformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeOctreeCollision(oc1Struct,_tr1,oc2Struct,_tr2,oc1Caching,oc2Caching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePtcloudCollision(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& ptcloudTransformation,unsigned long long int* ocCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        double _tr2[7];
        ptcloudTransformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreePtcloudCollision(ocStruct,_tr1,pcStruct,_tr2,ocCaching,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeTriangleCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,unsigned long long int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeTriangleCollision(ocStruct,_tr1,p.data,v.data,w.data,caching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeSegmentCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentExtremity,const C3Vector& segmentVector,unsigned long long int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeSegmentCollision(ocStruct,_tr1,segmentExtremity.data,segmentVector.data,caching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointsCollision(const void* ocStruct,const C7Vector& octreeTransformation,const double* points,int pointCount)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreePointsCollision(ocStruct,_tr1,points,pointCount);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointCollision(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,unsigned int* usrData/*=nullptr*/,unsigned long long int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreePointCollision(ocStruct,_tr1,point.data,usrData,caching);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxBoxCollision(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        box1Transformation.getData(_tr1);
        double _tr2[7];
        box2Transformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxBoxCollision(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data,boxesAreSolid);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxTriangleCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        boxTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxTriangleCollision(_tr1,boxHalfSize.data,boxIsSolid,p.data,v.data,w.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxSegmentCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        boxTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxSegmentCollision(_tr1,boxHalfSize.data,boxIsSolid,segmentEndPoint.data,segmentVector.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxPointCollision(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,const C3Vector& point)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        boxTransformation.getData(_tr1);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxPointCollision(_tr1,boxHalfSize.data,point.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleTriangleCollision(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,std::vector<double>* intersections/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double* _intersections;
        int _intersectionsSize;
        double** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getTriangleTriangleCollision(p1.data,v1.data,w1.data,p2.data,v2.data,w2.data,_int,&_intersectionsSize);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleSegmentCollision(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,std::vector<double>* intersections/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double* _intersections;
        int _intersectionsSize;
        double** _int=nullptr;
        if (intersections!=nullptr)
            _int=&_intersections;
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getTriangleSegmentCollision(p.data,v.data,w.data,segmentEndPoint.data,segmentVector.data,_int,&_intersectionsSize);
        if (retVal&&(intersections!=nullptr))
        {
            intersections->assign(_intersections,_intersections+_intersectionsSize);
            currentGeomPlugin->geomPlugin_releaseBuffer(_intersections);
        }
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(const void* mesh1ObbStruct,const C7Vector& mesh1Transformation,const void* mesh2ObbStruct,const C7Vector& mesh2Transformation,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* mesh1Caching/*=nullptr*/,int* mesh2Caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        mesh1Transformation.getData(_tr1);
        double _tr2[7];
        mesh2Transformation.getData(_tr2);
        double _minDistSegPt1[3];
        double _minDistSegPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshMeshDistanceIfSmaller(mesh1ObbStruct,_tr1,mesh2ObbStruct,_tr2,&dist,_minDistSegPt1,_minDistSegPt2,mesh1Caching,mesh2Caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistSegPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistSegPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshOctreeDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* ocStruct,const C7Vector& octreeTransformation,double& dist,C3Vector* meshMinDistPt/*=nullptr*/,C3Vector* ocMinDistPt/*=nullptr*/,int* meshCaching/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        meshTransformation.getData(_tr1);
        double _tr2[7];
        octreeTransformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshOctreeDistanceIfSmaller(meshObbStruct,_tr1,ocStruct,_tr2,&dist,_minDistPt1,_minDistPt2,meshCaching,ocCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (meshMinDistPt!=nullptr)
                meshMinDistPt->setData(_minDistPt1);
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshPtcloudDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const void* pcStruct,const C7Vector& pcTransformation,double& dist,C3Vector* meshMinDistPt/*=nullptr*/,C3Vector* pcMinDistPt/*=nullptr*/,int* meshCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        meshTransformation.getData(_tr1);
        double _tr2[7];
        pcTransformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshPtcloudDistanceIfSmaller(meshObbStruct,_tr1,pcStruct,_tr2,&dist,_minDistPt1,_minDistPt2,meshCaching,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (meshMinDistPt!=nullptr)
                meshMinDistPt->setData(_minDistPt1);
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshTriangleDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshTriangleDistanceIfSmaller(meshObbStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshSegmentDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/,int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshSegmentDistanceIfSmaller(meshObbStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getMeshPointDistanceIfSmaller(const void* meshObbStruct,const C7Vector& meshTransformation,const C3Vector& point,double& dist,C3Vector* minDistSegPt/*=nullptr*/,int* caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getMeshPointDistanceIfSmaller(meshObbStruct,_tr,point.data,&dist,_minDistPt1,caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeOctreeDistanceIfSmaller(const void* oc1Struct,const C7Vector& octree1Transformation,const void* oc2Struct,const C7Vector& octree2Transformation,double& dist,C3Vector* oc1MinDistPt/*=nullptr*/,C3Vector* oc2MinDistPt/*=nullptr*/,unsigned long long int* oc1Caching/*=nullptr*/,unsigned long long int* oc2Caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octree1Transformation.getData(_tr1);
        double _tr2[7];
        octree2Transformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeOctreeDistanceIfSmaller(oc1Struct,_tr1,oc2Struct,_tr2,&dist,_minDistPt1,_minDistPt2,oc1Caching,oc2Caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (oc1MinDistPt!=nullptr)
                oc1MinDistPt->setData(_minDistPt1);
            if (oc2MinDistPt!=nullptr)
                oc2MinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePtcloudDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const void* pcStruct,const C7Vector& pcTransformation,double& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* pcMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        octreeTransformation.getData(_tr1);
        double _tr2[7];
        pcTransformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreePtcloudDistanceIfSmaller(ocStruct,_tr1,pcStruct,_tr2,&dist,_minDistPt1,_minDistPt2,ocCaching,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setData(_minDistPt1);
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeTriangleDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,double& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* triMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeTriangleDistanceIfSmaller(ocStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,ocCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setData(_minDistPt1);
            if (triMinDistPt!=nullptr)
                triMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreeSegmentDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,C3Vector* ocMinDistPt/*=nullptr*/,C3Vector* segMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreeSegmentDistanceIfSmaller(ocStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,ocCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setData(_minDistPt1);
            if (segMinDistPt!=nullptr)
                segMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getOctreePointDistanceIfSmaller(const void* ocStruct,const C7Vector& octreeTransformation,const C3Vector& point,double& dist,C3Vector* ocMinDistPt/*=nullptr*/,unsigned long long int* ocCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getOctreePointDistanceIfSmaller(ocStruct,_tr,point.data,&dist,_minDistPt1,ocCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (ocMinDistPt!=nullptr)
                ocMinDistPt->setData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudPtcloudDistanceIfSmaller(const void* pc1Struct,const C7Vector& pc1Transformation,const void* pc2Struct,const C7Vector& pc2Transformation,double& dist,C3Vector* pc1MinDistPt/*=nullptr*/,C3Vector* pc2MinDistPt/*=nullptr*/,unsigned long long int* pc1Caching/*=nullptr*/,unsigned long long int* pc2Caching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        pc1Transformation.getData(_tr1);
        double _tr2[7];
        pc2Transformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudPtcloudDistanceIfSmaller(pc1Struct,_tr1,pc2Struct,_tr2,&dist,_minDistPt1,_minDistPt2,pc1Caching,pc2Caching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (pc1MinDistPt!=nullptr)
                pc1MinDistPt->setData(_minDistPt1);
            if (pc2MinDistPt!=nullptr)
                pc2MinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudTriangleDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& p,const C3Vector& v,const C3Vector& w,double& dist,C3Vector* pcMinDistPt/*=nullptr*/,C3Vector* triMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        pcTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudTriangleDistanceIfSmaller(pcStruct,_tr,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setData(_minDistPt1);
            if (triMinDistPt!=nullptr)
                triMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudSegmentDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,C3Vector* pcMinDistPt/*=nullptr*/,C3Vector* segMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        pcTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudSegmentDistanceIfSmaller(pcStruct,_tr,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setData(_minDistPt1);
            if (segMinDistPt!=nullptr)
                segMinDistPt->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getPtcloudPointDistanceIfSmaller(const void* pcStruct,const C7Vector& pcTransformation,const C3Vector& point,double& dist,C3Vector* pcMinDistPt/*=nullptr*/,unsigned long long int* pcCaching/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        pcTransformation.getData(_tr);
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getPtcloudPointDistanceIfSmaller(pcStruct,_tr,point.data,&dist,_minDistPt1,pcCaching);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (pcMinDistPt!=nullptr)
                pcMinDistPt->setData(_minDistPt1);
        }
    }
    return(retVal);
}
double CPluginContainer::geomPlugin_getApproxBoxBoxDistance(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    double retVal=0.0;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        box1Transformation.getData(_tr1);
        double _tr2[7];
        box2Transformation.getData(_tr2);
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getApproxBoxBoxDistance(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxBoxDistanceIfSmaller(const C7Vector& box1Transformation,const C3Vector& box1HalfSize,const C7Vector& box2Transformation,const C3Vector& box2HalfSize,bool boxesAreSolid,double& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr1[7];
        box1Transformation.getData(_tr1);
        double _tr2[7];
        box2Transformation.getData(_tr2);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxBoxDistanceIfSmaller(_tr1,box1HalfSize.data,_tr2,box2HalfSize.data,boxesAreSolid,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxTriangleDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& p,const C3Vector& v,const C3Vector& w,double& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        boxTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxTriangleDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,p.data,v.data,w.data,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxSegmentDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,C3Vector* distSegPt1/*=nullptr*/,C3Vector* distSegPt2/*=nullptr*/,bool altRoutine/*=false*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        boxTransformation.getData(_tr);
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxSegmentDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setData(_minDistPt1);
            if (distSegPt2!=nullptr)
                distSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getBoxPointDistanceIfSmaller(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,double& dist,C3Vector* distSegPt1/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        boxTransformation.getData(_tr);
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getBoxPointDistanceIfSmaller(_tr,boxHalfSize.data,boxIsSolid,point.data,&dist,_minDistPt1);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (distSegPt1!=nullptr)
                distSegPt1->setData(_minDistPt1);
        }
    }
    return(retVal);
}
double CPluginContainer::geomPlugin_getBoxPointDistance(const C7Vector& boxTransformation,const C3Vector& boxHalfSize,bool boxIsSolid,const C3Vector& point,C3Vector* distSegPt1/*=nullptr*/)
{
    double dist=DBL_MAX;
    geomPlugin_getBoxPointDistanceIfSmaller(boxTransformation,boxHalfSize,boxIsSolid,point,dist,distSegPt1);
    return(dist);
}
bool CPluginContainer::geomPlugin_getTriangleTriangleDistanceIfSmaller(const C3Vector& p1,const C3Vector& v1,const C3Vector& w1,const C3Vector& p2,const C3Vector& v2,const C3Vector& w2,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getTriangleTriangleDistanceIfSmaller(p1.data,v1.data,w1.data,p2.data,v2.data,w2.data,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTriangleSegmentDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getTriangleSegmentDistanceIfSmaller(p.data,v.data,w.data,segmentEndPoint.data,segmentVector.data,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getTrianglePointDistanceIfSmaller(const C3Vector& p,const C3Vector& v,const C3Vector& w,const C3Vector& point,double& dist,C3Vector* minDistSegPt/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getTrianglePointDistanceIfSmaller(p.data,v.data,w.data,point.data,&dist,_minDistPt1);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getSegmentSegmentDistanceIfSmaller(const C3Vector& segment1EndPoint,const C3Vector& segment1Vector,const C3Vector& segment2EndPoint,const C3Vector& segment2Vector,double& dist,C3Vector* minDistSegPt1/*=nullptr*/,C3Vector* minDistSegPt2/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _minDistPt1[3];
        double _minDistPt2[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getSegmentSegmentDistanceIfSmaller(segment1EndPoint.data,segment1Vector.data,segment2EndPoint.data,segment2Vector.data,&dist,_minDistPt1,_minDistPt2);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt1!=nullptr)
                minDistSegPt1->setData(_minDistPt1);
            if (minDistSegPt2!=nullptr)
                minDistSegPt2->setData(_minDistPt2);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_getSegmentPointDistanceIfSmaller(const C3Vector& segmentEndPoint,const C3Vector& segmentVector,const C3Vector& point,double& dist,C3Vector* minDistSegPt/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _minDistPt1[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_getSegmentPointDistanceIfSmaller(segmentEndPoint.data,segmentVector.data,point.data,&dist,_minDistPt1);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (minDistSegPt!=nullptr)
                minDistSegPt->setData(_minDistPt1);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectMeshIfSmaller(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const void* obbStruct,const C7Vector& meshTransformation,double& dist,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const double* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double _detectPt[3];
        double _triN[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectMeshIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),obbStruct,_tr,&dist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
            if (triN!=nullptr)
                triN->setData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectOctreeIfSmaller(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const void* ocStruct,const C7Vector& octreeTransformation,double& dist,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const double* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        double _detectPt[3];
        double _triN[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectOctreeIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),ocStruct,_tr,&dist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
            if (triN!=nullptr)
                triN->setData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectPtcloudIfSmaller(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const void* pcStruct,const C7Vector& ptcloudTransformation,double& dist,bool fast/*=false*/,C3Vector* detectPt/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const double* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        ptcloudTransformation.getData(_tr);
        double _detectPt[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectPtcloudIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),pcStruct,_tr,&dist,fast,_detectPt);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectTriangleIfSmaller(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const C3Vector& p,const C3Vector& v,const C3Vector& w,double& dist,bool frontDetection/*=true*/,bool backDetection/*=true*/,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const double* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        double _detectPt[3];
        double _triN[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectTriangleIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),p.data,v.data,w.data,&dist,frontDetection,backDetection,maxAngle,_detectPt,_triN);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
            if (triN!=nullptr)
                triN->setData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_volumeSensorDetectSegmentIfSmaller(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const C3Vector& segmentEndPoint,const C3Vector& segmentVector,double& dist,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    const double* _planesOut=nullptr;
    if (planesOut.size()>0)
        _planesOut=&planesOut[0];
    if (currentGeomPlugin!=nullptr)
    {
        double _detectPt[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_volumeSensorDetectSegmentIfSmaller(_planesIn,int(planesIn.size()),_planesOut,int(planesOut.size()),segmentEndPoint.data,segmentVector.data,&dist,maxAngle,_detectPt);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_raySensorDetectMeshIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* obbStruct,const C7Vector& meshTransformation,double& dist,double forbiddenDist/*=0.0*/,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/,bool* forbiddenDistTouched/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        meshTransformation.getData(_tr);
        double _detectPt[3];
        double _triN[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_raySensorDetectMeshIfSmaller(rayStart.data,rayVect.data,obbStruct,_tr,&dist,forbiddenDist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN,forbiddenDistTouched);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
            if (triN!=nullptr)
                triN->setData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_raySensorDetectOctreeIfSmaller(const C3Vector& rayStart,const C3Vector& rayVect,const void* ocStruct,const C7Vector& octreeTransformation,double& dist,double forbiddenDist/*=0.0*/,bool fast/*=false*/,bool frontDetection/*=true*/,bool backDetection/*=true*/,double maxAngle/*=0.0*/,C3Vector* detectPt/*=nullptr*/,C3Vector* triN/*=nullptr*/,bool* forbiddenDistTouched/*=nullptr*/)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        double _tr[7];
        octreeTransformation.getData(_tr);
        double _detectPt[3];
        double _triN[3];
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_raySensorDetectOctreeIfSmaller(rayStart.data,rayVect.data,ocStruct,_tr,&dist,forbiddenDist,fast,frontDetection,backDetection,maxAngle,_detectPt,_triN,forbiddenDistTouched);
        currentGeomPlugin->popCurrentPlugin();
        if (retVal)
        {
            if (detectPt!=nullptr)
                detectPt->setData(_detectPt);
            if (triN!=nullptr)
                triN->setData(_triN);
        }
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_isPointInVolume(const std::vector<double>& planesIn,const C3Vector& point)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    const double* _planesIn=nullptr;
    if (planesIn.size()>0)
        _planesIn=&planesIn[0];
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        retVal=currentGeomPlugin->geomPlugin_isPointInVolume(_planesIn,int(planesIn.size()),point.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::geomPlugin_isPointInVolume1AndOutVolume2(const std::vector<double>& planesIn,const std::vector<double>& planesOut,const C3Vector& point)
{
    if (currentGeomPlugin==nullptr)
        currentGeomPlugin=_tryToLoadPluginOnce(SIMGEOM_DEFAULT);

    bool retVal=false;
    if (currentGeomPlugin!=nullptr)
    {
        currentGeomPlugin->pushCurrentPlugin();
        if (planesIn.size()==0)
            retVal=true;
        else
            retVal=currentGeomPlugin->geomPlugin_isPointInVolume(&planesIn[0],int(planesIn.size()),point.data);
        if (retVal&&(planesOut.size()>0))
            retVal=!currentGeomPlugin->geomPlugin_isPointInVolume(&planesOut[0],int(planesOut.size()),point.data);
        currentGeomPlugin->popCurrentPlugin();
    }
    return(retVal);
}

void CPluginContainer::oldIkPlugin_emptyEnvironment()
{
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        //currentIkPlugin->oldIkPlugin_switchEnvironment(ikEnvironment);
        currentIKPlugin->oldIkPlugin_eraseEnvironment(ikEnvironment);
        ikEnvironment=currentIKPlugin->oldIkPlugin_createEnv();
        currentIKPlugin->popCurrentPlugin();
    }
}

void CPluginContainer::oldIkPlugin_eraseObject(int objectHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_eraseObject(ikEnvironment,objectHandle);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setObjectParent(int objectHandle,int parentObjectHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setObjectParent(ikEnvironment,objectHandle,parentObjectHandle);
        currentIKPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::oldIkPlugin_createDummy()
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_createDummy(ikEnvironment);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_setLinkedDummy(int dummyHandle,int linkedDummyHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setLinkedDummy(ikEnvironment,dummyHandle,linkedDummyHandle);
        currentIKPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::oldIkPlugin_createJoint(int jointType)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_createJoint(ikEnvironment,jointType);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_setJointMode(int jointHandle,int jointMode)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointMode(ikEnvironment,jointHandle,jointMode);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setJointInterval(int jointHandle,bool cyclic,double jMin,double jRange)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    double mr[2]={jMin,jRange};
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointInterval(ikEnvironment,jointHandle,cyclic,mr);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setJointScrewPitch(int jointHandle,double pitch)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointScrewPitch(ikEnvironment,jointHandle,pitch);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setJointIkWeight(int jointHandle,double ikWeight)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointIkWeight(ikEnvironment,jointHandle,ikWeight);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setJointMaxStepSize(int jointHandle,double maxStepSize)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointMaxStepSize(ikEnvironment,jointHandle,maxStepSize);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setJointDependency(int jointHandle,int dependencyJointHandle,double offset,double mult)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointDependency(ikEnvironment,jointHandle,dependencyJointHandle,offset,mult);
        currentIKPlugin->popCurrentPlugin();
    }
}
double CPluginContainer::oldIkPlugin_getJointPosition(int jointHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    double retVal=0.0;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_getJointPosition(ikEnvironment,jointHandle);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_setJointPosition(int jointHandle,double position)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setJointPosition(ikEnvironment,jointHandle,position);
        currentIKPlugin->popCurrentPlugin();
    }
}
C4Vector CPluginContainer::oldIkPlugin_getSphericalJointQuaternion(int jointHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    C4Vector retVal;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_getSphericalJointQuaternion(ikEnvironment,jointHandle,retVal.data);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_setSphericalJointQuaternion(int jointHandle,const C4Vector& quaternion)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setSphericalJointQuaternion(ikEnvironment,jointHandle,quaternion.data);
        currentIKPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::oldIkPlugin_createIkGroup()
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_createIkGroup(ikEnvironment);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_eraseIkGroup(int ikGroupHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_eraseIkGroup(ikEnvironment,ikGroupHandle);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkGroupFlags(int ikGroupHandle,int flags)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkGroupFlags(ikEnvironment,ikGroupHandle,flags);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkGroupCalculation(int ikGroupHandle,int method,double damping,int maxIterations)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkGroupCalculation(ikEnvironment,ikGroupHandle,method,damping,maxIterations);
        currentIKPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::oldIkPlugin_addIkElement(int ikGroupHandle,int tipHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_addIkElement(ikEnvironment,ikGroupHandle,tipHandle);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
void CPluginContainer::oldIkPlugin_eraseIkElement(int ikGroupHandle,int ikElementIndex)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_eraseIkElement(ikEnvironment,ikGroupHandle,ikElementIndex);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkElementFlags(int ikGroupHandle,int ikElementIndex,int flags)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkElementFlags(ikEnvironment,ikGroupHandle,ikElementIndex,flags);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkElementBase(int ikGroupHandle,int ikElementIndex,int baseHandle,int constraintsBaseHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkElementBase(ikEnvironment,ikGroupHandle,ikElementIndex,baseHandle,constraintsBaseHandle);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkElementConstraints(int ikGroupHandle,int ikElementIndex,int constraints)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkElementConstraints(ikEnvironment,ikGroupHandle,ikElementIndex,constraints);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkElementPrecision(int ikGroupHandle,int ikElementIndex,double linearPrecision,double angularPrecision)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkElementPrecision(ikEnvironment,ikGroupHandle,ikElementIndex,linearPrecision,angularPrecision);
        currentIKPlugin->popCurrentPlugin();
    }
}
void CPluginContainer::oldIkPlugin_setIkElementWeights(int ikGroupHandle,int ikElementIndex,double linearWeight,double angularWeight)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setIkElementWeights(ikEnvironment,ikGroupHandle,ikElementIndex,linearWeight,angularWeight);
        currentIKPlugin->popCurrentPlugin();
    }
}
int CPluginContainer::oldIkPlugin_handleIkGroup(int ikGroupHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=sim_ikresult_not_performed;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_handleIkGroup(ikEnvironment,ikGroupHandle);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
bool CPluginContainer::oldIkPlugin_computeJacobian(int ikGroupHandle,int options)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    bool retVal=false;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_computeJacobian(ikEnvironment,ikGroupHandle,options);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
CMatrix* CPluginContainer::oldIkPlugin_getJacobian(int ikGroupHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    CMatrix* retVal=nullptr;
    if (currentIKPlugin!=nullptr)
    {
        int matrixSize[2];
        currentIKPlugin->pushCurrentPlugin();
        double* jc=currentIKPlugin->oldIkPlugin_getJacobian(ikEnvironment,ikGroupHandle,matrixSize);
        currentIKPlugin->popCurrentPlugin();
        if (jc!=nullptr)
        {
            retVal=new CMatrix(matrixSize[1],matrixSize[0]);
            for (size_t r=0;r<size_t(matrixSize[1]);r++)
            {
                for (size_t c=0;c<size_t(matrixSize[0]);c++)
                    (retVal[0])(r,c)=jc[r*matrixSize[0]+c];
            }
            delete[] jc;
        }
    }
    return(retVal);
}
double CPluginContainer::oldIkPlugin_getManipulability(int ikGroupHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    double retVal=0.0;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        retVal=currentIKPlugin->oldIkPlugin_getManipulability(ikEnvironment,ikGroupHandle);
        currentIKPlugin->popCurrentPlugin();
    }
    return(retVal);
}
int CPluginContainer::oldIkPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,double thresholdDist,int maxIterationsOrTimeInMs,double* retConfig,const double* metric,bool(*validationCallback)(double*),const int* jointOptions,const double* lowLimits,const double* ranges,std::string& errString)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        char* errS=currentIKPlugin->oldIkPlugin_getConfigForTipPose(ikEnvironment,ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxIterationsOrTimeInMs,&retVal,retConfig,metric,validationCallback,jointOptions,lowLimits,ranges);
        currentIKPlugin->popCurrentPlugin();
        if ( (retVal<0)&&(errS!=nullptr) )
        {
            errString=errS;
            delete[] errS;
        }
    }
    else
        errString=SIM_ERROR_IK_PLUGIN_NOT_FOUND;
    return(retVal);
}

static std::vector<int> _ikValidationCb_collisionPairs;
static std::vector<int> _ikValidationCb_jointHandles;

bool _validationCallback(double* conf)
{
    bool collisionFree=true;
    std::vector<double> memorized;
    std::vector<CJoint*> joints;
    for (size_t i=0;i<_ikValidationCb_jointHandles.size();i++)
    {
        CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(_ikValidationCb_jointHandles[i]);
        joints.push_back(it);
        memorized.push_back(it->getPosition());
        it->setPosition(conf[i]);
    }
    for (size_t i=0;i<_ikValidationCb_collisionPairs.size()/2;i++)
    {
        int robot=_ikValidationCb_collisionPairs[2*i+0];
        if (robot>=0)
        {
            int env=_ikValidationCb_collisionPairs[2*i+1];
            if (env==sim_handle_all)
                env=-1;
            if (CCollisionRoutine::doEntitiesCollide(robot,env,nullptr,false,false,nullptr))
            {
                collisionFree=false;
                break;
            }
        }
    }
    for (size_t i=0;i<_ikValidationCb_jointHandles.size();i++)
    {
        CJoint* it=joints[i];
        it->setPosition(memorized[i]);
    }
    return(collisionFree);
}

int CPluginContainer::oldIkPlugin_getConfigForTipPose(int ikGroupHandle,int jointCnt,const int* jointHandles,double thresholdDist,int maxIterationsOrTimeInMs,double* retConfig,const double* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const double* lowLimits,const double* ranges,std::string& errString)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    int retVal=-1;
    if (currentIKPlugin!=nullptr)
    {
        bool(*_validationCB)(double*)=nullptr;
        bool err=false;
        if ( (collisionPairCnt>0)&&(collisionPairs!=nullptr) )
        {
            _ikValidationCb_jointHandles.assign(jointHandles,jointHandles+jointCnt);
            _ikValidationCb_collisionPairs.clear();
            for (size_t i=0;i<size_t(collisionPairCnt);i++)
            {
                CSceneObject* eo1=App::currentWorld->sceneObjects->getObjectFromHandle(collisionPairs[2*i+0]);
                CCollection* ec1=App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+0]);
                CSceneObject* eo2=App::currentWorld->sceneObjects->getObjectFromHandle(collisionPairs[2*i+1]);
                CCollection* ec2=App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+1]);
                err=err||( ((eo1==nullptr)&&(ec1==nullptr)) || ((eo2==nullptr)&&(ec2==nullptr)&&(collisionPairs[2*i+1]!=sim_handle_all)) );
                _ikValidationCb_collisionPairs.push_back(collisionPairs[2*i+0]);
                _ikValidationCb_collisionPairs.push_back(collisionPairs[2*i+1]);
            }
            _validationCB=_validationCallback;
            if (err)
                errString=SIM_ERROR_INVALID_COLLISION_PAIRS;
        }
        if (!err)
        {
            currentIKPlugin->pushCurrentPlugin();
            retVal=oldIkPlugin_getConfigForTipPose(ikGroupHandle,jointCnt,jointHandles,thresholdDist,maxIterationsOrTimeInMs,retConfig,metric,_validationCB,jointOptions,lowLimits,ranges,errString);
            currentIKPlugin->popCurrentPlugin();
        }
    }
    else
        errString=SIM_ERROR_IK_PLUGIN_NOT_FOUND;
    return(retVal);
}
C7Vector CPluginContainer::oldIkPlugin_getObjectLocalTransformation(int objectHandle)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    C7Vector tr;
    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_getObjectLocalTransformation(ikEnvironment,objectHandle,tr.X.data,tr.Q.data);
        currentIKPlugin->popCurrentPlugin();
    }
    return(tr);
}
void CPluginContainer::oldIkPlugin_setObjectLocalTransformation(int objectHandle,const C7Vector& tr)
{
    if (currentIKPlugin==nullptr)
        currentIKPlugin=_tryToLoadPluginOnce(SIMIK0_DEFAULT);

    if (currentIKPlugin!=nullptr)
    {
        currentIKPlugin->pushCurrentPlugin();
        currentIKPlugin->oldIkPlugin_setObjectLocalTransformation(ikEnvironment,objectHandle,tr.X.data,tr.Q.data);
        currentIKPlugin->popCurrentPlugin();
    }
}

bool CPluginContainer::codeEditor_openModal(const char* initText,const char* properties,std::string& modifiedText,int* positionAndSize)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    bool retVal=false;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        char* buffer=currentCodeEditorPlugin->_codeEditor_openModal(initText,properties,positionAndSize);
        currentCodeEditorPlugin->popCurrentPlugin();
        if (buffer!=nullptr)
        {
            modifiedText=buffer;
            delete[] buffer;
            retVal=true;
        }
    }
    return(retVal);
}

int CPluginContainer::codeEditor_open(const char* initText,const char* properties)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    int retVal=-1;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        retVal=currentCodeEditorPlugin->_codeEditor_open(initText,properties);
        currentCodeEditorPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::codeEditor_setText(int handle,const char* text,int insertMode)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    int retVal=-1;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        retVal=currentCodeEditorPlugin->_codeEditor_setText(handle,text,insertMode);
        currentCodeEditorPlugin->popCurrentPlugin();
    }
    return(retVal);
}

bool CPluginContainer::codeEditor_getText(int handle,std::string& text,int* positionAndSize)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    bool retVal=false;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        char* buffer=currentCodeEditorPlugin->_codeEditor_getText(handle,positionAndSize);
        currentCodeEditorPlugin->popCurrentPlugin();
        if (buffer!=nullptr)
        {
            text=buffer;
            delete[] buffer;
            retVal=true;
        }
    }
    return(retVal);
}

int CPluginContainer::codeEditor_show(int handle,int showState)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    int retVal=-1;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        retVal=currentCodeEditorPlugin->_codeEditor_show(handle,showState);
        currentCodeEditorPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::codeEditor_close(int handle,int* positionAndSize)
{
    if (currentCodeEditorPlugin==nullptr)
        currentCodeEditorPlugin=_tryToLoadPluginOnce(SIMCODEEDITOR_DEFAULT);

    int retVal=-1;
    if (currentCodeEditorPlugin!=nullptr)
    {
        currentCodeEditorPlugin->pushCurrentPlugin();
        retVal=currentCodeEditorPlugin->_codeEditor_close(handle,positionAndSize);
        currentCodeEditorPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_pos(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxVel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetPos,const double* targetVel)
{
    if (currentRuckigPlugin==nullptr)
        currentRuckigPlugin=_tryToLoadPluginOnce(SIMRUCKIG_DEFAULT);

    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
    {
        currentRuckigPlugin->pushCurrentPlugin();
        retVal=currentRuckigPlugin->ruckigPlugin_pos(scriptHandle,dofs,smallestTimeStep,flags,currentPos,currentVel,currentAccel,maxVel,maxAccel,maxJerk,selection,targetPos,targetVel);
        currentRuckigPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_vel(int scriptHandle,int dofs,double smallestTimeStep,int flags,const double* currentPos,const double* currentVel,const double* currentAccel,const double* maxAccel,const double* maxJerk,const bool* selection,const double* targetVel)
{
    if (currentRuckigPlugin==nullptr)
        currentRuckigPlugin=_tryToLoadPluginOnce(SIMRUCKIG_DEFAULT);

    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
    {
        currentRuckigPlugin->pushCurrentPlugin();
        retVal=currentRuckigPlugin->ruckigPlugin_vel(scriptHandle,dofs,smallestTimeStep,flags,currentPos,currentVel,currentAccel,maxAccel,maxJerk,selection,targetVel);
        currentRuckigPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_step(int objHandle,double timeStep,double* newPos,double* newVel,double* newAccel,double* syncTime)
{
    if (currentRuckigPlugin==nullptr)
        currentRuckigPlugin=_tryToLoadPluginOnce(SIMRUCKIG_DEFAULT);

    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
    {
        currentRuckigPlugin->pushCurrentPlugin();
        retVal=currentRuckigPlugin->ruckigPlugin_step(objHandle,timeStep,newPos,newVel,newAccel,syncTime);
        currentRuckigPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_remove(int objHandle)
{
    if (currentRuckigPlugin==nullptr)
        currentRuckigPlugin=_tryToLoadPluginOnce(SIMRUCKIG_DEFAULT);

    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
    {
        currentRuckigPlugin->pushCurrentPlugin();
        retVal=currentRuckigPlugin->ruckigPlugin_remove(objHandle);
        currentRuckigPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::ruckigPlugin_dofs(int objHandle)
{
    if (currentRuckigPlugin==nullptr)
        currentRuckigPlugin=_tryToLoadPluginOnce(SIMRUCKIG_DEFAULT);

    int retVal=-2;
    if (currentRuckigPlugin!=nullptr)
    {
        currentRuckigPlugin->pushCurrentPlugin();
        retVal=currentRuckigPlugin->ruckigPlugin_dofs(objHandle);
        currentRuckigPlugin->popCurrentPlugin();
    }
    return(retVal);
}

int CPluginContainer::customUi_msgBox(int type, int buttons, const char *title, const char *message,int defaultAnswer)
{ // only used with deprecated API function
    if (currentUIPlugin==nullptr)
        currentUIPlugin=_tryToLoadPluginOnce(SIMUI_DEFAULT);
    int retVal=-1;
    if (currentUIPlugin!=nullptr)
    {
        retVal=defaultAnswer;
        bool doIt=false;
        if (type==sim_msgbox_type_info)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_infos);
        if (type==sim_msgbox_type_question)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_questions);
        if (type==sim_msgbox_type_warning)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_warnings);
        if (type==sim_msgbox_type_critical)
            doIt=(App::getDlgVerbosity()>=sim_verbosity_errors);
        if (doIt)
        {
            currentUIPlugin->pushCurrentPlugin();
            retVal=currentUIPlugin->_customUi_msgBox(type,buttons,title,message);
            currentUIPlugin->popCurrentPlugin();
        }
    }
    return(retVal);
}

bool CPluginContainer::customUi_fileDialog(int type, const char *title, const char *startPath, const char *initName, const char *extName, const char *ext, int native,std::string& files)
{ // only used with deprecated API function
    if (currentUIPlugin==nullptr)
        currentUIPlugin=_tryToLoadPluginOnce(SIMUI_DEFAULT);
    bool retVal=false;
    if (currentUIPlugin!=nullptr)
    {
        currentUIPlugin->pushCurrentPlugin();
        char* res=currentUIPlugin->_customUi_fileDialog(type,title,startPath,initName,extName,ext,native);
        currentUIPlugin->popCurrentPlugin();
        if (res!=nullptr)
        {
            if (strlen(res)>0)
            {
                files.assign(res);
                retVal=true;
            }
            simReleaseBuffer_internal(res);
        }
    }
    return(retVal);
}

int* CPluginContainer::assimp_importShapes(const char* fileNames,int maxTextures,double scaling,int upVector,int options,int* shapeCount)
{
    if (currentAssimpPlugin==nullptr)
        currentAssimpPlugin=_tryToLoadPluginOnce(SIMASSIMP_DEFAULT);

    int* retVal=nullptr;
    if (currentAssimpPlugin!=nullptr)
    {
        currentAssimpPlugin->pushCurrentPlugin();
        retVal=currentAssimpPlugin->_assimp_importShapes(fileNames,maxTextures,scaling,upVector,options,shapeCount);
        currentAssimpPlugin->popCurrentPlugin();
    }
    else
        App::logMsg(sim_verbosity_errors,"simAssimp plugin was not found.");
    return(retVal);
}

void CPluginContainer::assimp_exportShapes(const int* shapeHandles,int shapeCount,const char* filename,const char* format,double scaling,int upVector,int options)
{
    if (currentAssimpPlugin==nullptr)
        currentAssimpPlugin=_tryToLoadPluginOnce(SIMASSIMP_DEFAULT);

    if (currentAssimpPlugin!=nullptr)
    {
        currentAssimpPlugin->pushCurrentPlugin();
        currentAssimpPlugin->_assimp_exportShapes(shapeHandles,shapeCount,filename,format,scaling,upVector,options);
        currentAssimpPlugin->popCurrentPlugin();
    }
    else
        App::logMsg(sim_verbosity_errors,"simAssimp plugin was not found.");
}

int CPluginContainer::assimp_importMeshes(const char* fileNames,double scaling,int upVector,int options,double*** allVertices,int** verticesSizes,int*** allIndices,int** indicesSizes)
{
    if (currentAssimpPlugin==nullptr)
        currentAssimpPlugin=_tryToLoadPluginOnce(SIMASSIMP_DEFAULT);

    int retVal=0;
    if (currentAssimpPlugin!=nullptr)
    {
        currentAssimpPlugin->pushCurrentPlugin();
        retVal=currentAssimpPlugin->_assimp_importMeshes(fileNames,scaling,upVector,options,allVertices,verticesSizes,allIndices,indicesSizes);
        currentAssimpPlugin->popCurrentPlugin();
    }
    else
        App::logMsg(sim_verbosity_errors,"simAssimp plugin was not found.");
    return(retVal);
}

void CPluginContainer::assimp_exportMeshes(int meshCnt,const double** allVertices,const int* verticesSizes,const int** allIndices,const int* indicesSizes,const char* filename,const char* format,double scaling,int upVector,int options)
{
    if (currentAssimpPlugin==nullptr)
        currentAssimpPlugin=_tryToLoadPluginOnce(SIMASSIMP_DEFAULT);

    if (currentAssimpPlugin!=nullptr)
    {
        currentAssimpPlugin->pushCurrentPlugin();
        currentAssimpPlugin->_assimp_exportMeshes(meshCnt,allVertices,verticesSizes,allIndices,indicesSizes,filename,format,scaling,upVector,options);
        currentAssimpPlugin->popCurrentPlugin();
    }
    else
        App::logMsg(sim_verbosity_errors,"simAssimp plugin was not found.");
}
