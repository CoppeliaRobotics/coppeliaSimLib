
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "dynamicsContainer.h"
#include "pluginContainer.h"
#include "app.h"
#include "v_repStringTable.h"
#include "tt.h"
#include "dynamicsRendering.h"
#ifdef SIM_WITH_GUI
#include "vMessageBox.h"
#endif
// index0=very precise, index1=precise (default), index2=fast, index3=very fast
const float DYNAMIC_BULLET_DEFAULT_STEP_SIZE[4]={0.005f,0.005f,0.005f,0.005f};
const int DYNAMIC_BULLET_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[4]={200,100,50,20};
const bool DYNAMIC_BULLET_DEFAULT_FULL_INTERNAL_SCALING[4]={true,true,true,true};
const float DYNAMIC_BULLET_DEFAULT_INTERNAL_SCALING_FACTOR[4]={10.0f,10.0f,10.0f,10.0f};
const float DYNAMIC_BULLET_DEFAULT_COLLISION_MARGIN_FACTOR[4]={0.1f,0.1f,0.1f,0.1f};

const float DYNAMIC_ODE_DEFAULT_STEP_SIZE[4]={0.005f,0.005f,0.005f,0.005f};
const int DYNAMIC_ODE_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[4]={200,100,50,20};
const bool DYNAMIC_ODE_DEFAULT_FULL_INTERNAL_SCALING[4]={true,true,true,true};
const float DYNAMIC_ODE_DEFAULT_INTERNAL_SCALING_FACTOR[4]={1.0f,1.0f,1.0f,1.0f};
const bool DYNAMIC_ODE_USE_QUICKSTEP[4]={true,true,true,true};
const float DYNAMIC_ODE_GLOBAL_CFM[4]={0.00001f,0.00001f,0.00001f,0.00001f};
const float DYNAMIC_ODE_GLOBAL_ERP[4]={0.6f,0.6f,0.6f,0.6f};

const float DYNAMIC_VORTEX_DEFAULT_STEP_SIZE[4]={0.0025f,0.005f,0.01f,0.025f};
const bool DYNAMIC_VORTEX_DEFAULT_AUTO_SLEEP[4]={true,true,true,true};
const float DYNAMIC_VORTEX_DEFAULT_INTERNAL_SCALING_FACTOR[4]={1.0f,1.0f,1.0f,1.0f};
const float DYNAMIC_VORTEX_DEFAULT_CONTACT_TOLERANCE[4]={0.001f,0.001f,0.001f,0.001f};
const bool DYNAMIC_VORTEX_DEFAULT_MULTITHREADING[4]={false,false,false,false}; // false since 8/3/2017: false is faster (for V-REP scenes) and more stable
// const bool DYNAMIC_VORTEX_DEFAULT_MULTITHREADING[4]={true,true,true,true};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_COMPLIANCE[4]={1.0e-7f,1.0e-7f,1.0e-7f,1.0e-7f};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_DAMPING[4]={8.0e+6,8.0e+6,8.0e+6,8.0e+6};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_KIN_LOSS[4]={6.0e-5f,6.0e-5f,6.0e-5f,6.0e-5f};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_COMPLIANCE[4]={1.0e-9f,1.0e-9f,1.0e-9f,1.0e-9f};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_DAMPING[4]={8.0e+8,8.0e+8,8.0e+8,8.0e+8};
const float DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_KIN_LOSS[4]={6.0e-7f,6.0e-7f,6.0e-7f,6.0e-7f};

const float DYNAMIC_NEWTON_DEFAULT_STEP_SIZE[4]={0.005f,0.005f,0.005f,0.005f};
const int DYNAMIC_NEWTON_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[4]={16,8,6,4};
const bool DYNAMIC_NEWTON_DEFAULT_MULTITHREADED[4]={true,true,true,true};
const bool DYNAMIC_NEWTON_DEFAULT_EXACT_SOLVER[4]={true,true,true,true};
const bool DYNAMIC_NEWTON_DEFAULT_HIGH_JOINT_ACCURACY[4]={true,true,true,true};
const float DYNAMIC_NEWTON_DEFAULT_CONTACT_MERGE_TOLERANCE[4]={0.01f,0.01f,0.01f,0.01f};


CDynamicsContainer::CDynamicsContainer()
{
    _dynamicsEnabled=true;
    _dynamicDefaultTypeCalculationParameters=1; // means precise

    getBulletDefaultFloatParams(_bulletFloatParams,_dynamicDefaultTypeCalculationParameters);
    getBulletDefaultIntParams(_bulletIntParams,_dynamicDefaultTypeCalculationParameters);

    getOdeDefaultFloatParams(_odeFloatParams,_dynamicDefaultTypeCalculationParameters);
    getOdeDefaultIntParams(_odeIntParams,_dynamicDefaultTypeCalculationParameters);

    getVortexDefaultFloatParams(_vortexFloatParams,_dynamicDefaultTypeCalculationParameters);
    getVortexDefaultIntParams(_vortexIntParams,_dynamicDefaultTypeCalculationParameters);

    getNewtonDefaultFloatParams(_newtonFloatParams,_dynamicDefaultTypeCalculationParameters);
    getNewtonDefaultIntParams(_newtonIntParams,_dynamicDefaultTypeCalculationParameters);

    _dynamicEngineToUse=sim_physics_bullet; // Bullet is default
    _dynamicEngineVersionToUse=0; // this is Bullet 2.78
    // _dynamicEngineVersionToUse=283; // this is Bullet 2.83

    contactPointColor.setColorsAllBlack();
    contactPointColor.setColor(1.0f,1.0f,0.0f,sim_colorcomponent_emission);
    _displayContactPoints=false;
    _tempDisabledWarnings=0;
    _currentlyInDynamicsCalculations=false;

    _gravity=C3Vector(0.0f,0.0f,-9.81f);
    _resetWarningFlags();
}

CDynamicsContainer::~CDynamicsContainer()
{
}

void CDynamicsContainer::simulationAboutToStart()
{
    _resetWarningFlags();
    _tempDisabledWarnings=0;

    removeWorld(); // not really needed

    // Following is just in case:
    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
        it->disableDynamicTreeForManipulation(false);
    }

    // Keep following (important that it is initialized BEFORE simHandleDynamics is called!!)
    if (getDynamicsEnabled())
        addWorldIfNotThere();
}

void CDynamicsContainer::simulationEnded()
{
    removeWorld();

    // Following is because some shapes might have been disabled because below z=-1000 meters:
    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
        it->disableDynamicTreeForManipulation(false);
    }
    
    _resetWarningFlags();
    _tempDisabledWarnings=0;
}

void CDynamicsContainer::_resetWarningFlags()
{
    _containsNonPureNonConvexShapes=0;
    _containsStaticShapesOnDynamicConstruction=0;
    _pureSpheroidNotSupportedMark=0;
    _newtonDynamicRandomMeshNotSupportedMark=0;
    _pureConeNotSupportedMark=0;
    _pureHollowShapeNotSupportedMark=0;
    _physicsEngineNotSupportedWarning=0;
    _nonDefaultEngineSettingsWarning=0;
    _vortexPluginIsDemoWarning=0;
}

void CDynamicsContainer::setTempDisabledWarnings(int mask)
{
    _tempDisabledWarnings=mask;
}

int CDynamicsContainer::getTempDisabledWarnings()
{
    return(_tempDisabledWarnings);
}

bool CDynamicsContainer::getCurrentlyInDynamicsCalculations()
{
    return(_currentlyInDynamicsCalculations);
}

void CDynamicsContainer::handleDynamics(float dt)
{
    App::ct->calcInfo->dynamicsStart();

    for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
        if (it!=nullptr)
            it->setDynamicObjectFlag_forVisualization(0);
    }
    addWorldIfNotThere();

    if (getDynamicsEnabled())
    {
        _currentlyInDynamicsCalculations=true;
        CPluginContainer::dyn_step(dt,float(App::ct->simulation->getSimulationTime_ns())/1000000.0f);
        _currentlyInDynamicsCalculations=false;
    }

    if (CPluginContainer::dyn_isDynamicContentAvailable())
        App::ct->calcInfo->dynamicsEnd(CPluginContainer::dyn_getDynamicStepDivider(),true);
    else
        App::ct->calcInfo->dynamicsEnd(0,false);
}

bool CDynamicsContainer::getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],float contactInfo[6])
{
    if (getDynamicsEnabled())
        return(CPluginContainer::dyn_getContactForce(dynamicPass,objectHandle,index,objectHandles,contactInfo)!=0);
    return(false);
}

void CDynamicsContainer::reportDynamicWorldConfiguration()
{
    if (getDynamicsEnabled())
        CPluginContainer::dyn_reportDynamicWorldConfiguration(-1,1,0.0f);
}

void CDynamicsContainer::addWorldIfNotThere()
{
    if (getDynamicsEnabled()&&(!isWorldThere()))
    {
        float floatParams[20];
        int intParams[20];
        int floatIndex=0;
        int intIndex=0;

        floatParams[floatIndex++]=getPositionScalingFactorDyn();
        floatParams[floatIndex++]=getLinearVelocityScalingFactorDyn();
        floatParams[floatIndex++]=getMassScalingFactorDyn();
        floatParams[floatIndex++]=getMasslessInertiaScalingFactorDyn();
        floatParams[floatIndex++]=getForceScalingFactorDyn();
        floatParams[floatIndex++]=getTorqueScalingFactorDyn();
        floatParams[floatIndex++]=getGravityScalingFactorDyn();
        floatParams[floatIndex++]=App::userSettings->dynamicActivityRange;

        intParams[intIndex++]=SIM_IDEND_3DOBJECT+1;
        intParams[intIndex++]=SIM_IDSTART_3DOBJECT;
        intParams[intIndex++]=SIM_IDEND_3DOBJECT;

        CPluginContainer::dyn_startSimulation(_dynamicEngineToUse,_dynamicEngineVersionToUse,floatParams,intParams);
    }
}

void CDynamicsContainer::removeWorld()
{
    if (isWorldThere())
        CPluginContainer::dyn_endSimulation();
}

bool CDynamicsContainer::isWorldThere()
{
    return(CPluginContainer::dyn_isInitialized());
}

void CDynamicsContainer::markForWarningDisplay_pureSpheroidNotSupported()
{
    if (_pureSpheroidNotSupportedMark==0)
        _pureSpheroidNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_newtonDynamicRandomMeshNotSupported()
{
    if (_newtonDynamicRandomMeshNotSupportedMark==0)
        _newtonDynamicRandomMeshNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_pureConeNotSupported()
{
    if (_pureConeNotSupportedMark==0)
        _pureConeNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_pureHollowShapeNotSupported()
{
    if (_pureHollowShapeNotSupportedMark==0)
        _pureHollowShapeNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_physicsEngineNotSupported()
{
    if (_physicsEngineNotSupportedWarning==0)
        _physicsEngineNotSupportedWarning++;
}

void CDynamicsContainer::markForWarningDisplay_vortexPluginIsDemo()
{
    if (_vortexPluginIsDemoWarning==0)
        _vortexPluginIsDemoWarning++;
}

bool CDynamicsContainer::displayVortexPluginIsDemoRequired()
{
    if ( (_vortexPluginIsDemoWarning==1)&&((_tempDisabledWarnings&128)==0) )
    {
        _vortexPluginIsDemoWarning++;
        return(true);
    }
    return(false);
}

void CDynamicsContainer::markForWarningDisplay_containsNonPureNonConvexShapes()
{
    if (_containsNonPureNonConvexShapes==0)
        _containsNonPureNonConvexShapes++;
}

void CDynamicsContainer::markForWarningDisplay_containsStaticShapesOnDynamicConstruction()
{
    if (_containsStaticShapesOnDynamicConstruction==0)
        _containsStaticShapesOnDynamicConstruction++;
}

bool CDynamicsContainer::displayNonPureNonConvexShapeWarningRequired()
{
    if ( (_containsNonPureNonConvexShapes==1)&&((_tempDisabledWarnings&16)==0) )
    {
        _containsNonPureNonConvexShapes++;
        return(true);
    }
    return(false);
}

bool CDynamicsContainer::displayStaticShapeOnDynamicConstructionWarningRequired()
{
    if ( (_containsStaticShapesOnDynamicConstruction==1)&&((_tempDisabledWarnings&32)==0) )
    {
        _containsStaticShapesOnDynamicConstruction++;
        return(true);
    }
    return(false);
}

bool CDynamicsContainer::displayNonDefaultParameterWarningRequired()
{
    if (getUseDynamicDefaultCalculationParameters()!=1)
    {
        if ( (_nonDefaultEngineSettingsWarning==0)&&((_tempDisabledWarnings&64)==0) )
        {
            _nonDefaultEngineSettingsWarning++;
            return(true);
        }
    }
    return(false);
}

void CDynamicsContainer::displayWarningsIfNeeded()
{
    if ( (_pureSpheroidNotSupportedMark==1)&&((_tempDisabledWarnings&1)==0) )
    {
#ifdef SIM_WITH_GUI
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PURE_SPHEROID),strTranslate(IDS_WARNING_WHEN_PURE_SPHEROID_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
#else
        printf("%s\n",IDS_WARNING_WHEN_PURE_SPHEROID_NOT_SUPPORTED);
#endif
        _pureSpheroidNotSupportedMark++;
    }
    if ( (_pureConeNotSupportedMark==1)&&((_tempDisabledWarnings&2)==0) )
    {
#ifdef SIM_WITH_GUI
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PURE_CONE),strTranslate(IDS_WARNING_WHEN_PURE_CONE_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
#else
        printf("%s\n",IDS_WARNING_WHEN_PURE_CONE_NOT_SUPPORTED);
#endif
        _pureConeNotSupportedMark++;
    }
    if ( (_pureHollowShapeNotSupportedMark==1)&&((_tempDisabledWarnings&4)==0) )
    {
#ifdef SIM_WITH_GUI
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PURE_HOLLOW_SHAPES),strTranslate(IDS_WARNING_WHEN_PURE_HOLLOW_SHAPE_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
#else
        printf("%s\n",IDS_WARNING_WHEN_PURE_HOLLOW_SHAPE_NOT_SUPPORTED);
#endif
        _pureHollowShapeNotSupportedMark++;
    }
    if ( (_physicsEngineNotSupportedWarning==1)&&((_tempDisabledWarnings&8)==0) )
    {
        if (_dynamicEngineToUse==sim_physics_vortex)
        {
#ifdef WIN_VREP
    #ifdef SIM_WITH_GUI
            App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
    #else
            printf("%s\n",IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED);
    #endif
#endif
#ifdef LIN_VREP
    #ifdef SIM_WITH_GUI
            App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
    #else
            printf("%s\n",IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED);
    #endif
#endif
#ifdef MAC_VREP
    #ifdef SIM_WITH_GUI
            App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WHEN_VORTEX_NOT_YET_SUPPORTED),VMESSAGEBOX_OKELI);
    #else
            printf("%s\n",IDS_WARNING_WHEN_VORTEX_NOT_YET_SUPPORTED);
    #endif
#endif
        }
        else
#ifdef SIM_WITH_GUI
            App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
#else
            printf("%s\n",IDS_WARNING_WHEN_PHYSICS_ENGINE_NOT_SUPPORTED);
#endif

        _physicsEngineNotSupportedWarning++;
    }
    if ( (_newtonDynamicRandomMeshNotSupportedMark==1)&&((_tempDisabledWarnings&256)==0) )
    {
#ifdef SIM_WITH_GUI
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_NEWTON_NON_CONVEX_MESH),strTranslate(IDS_WARNING_WITH_NEWTON_NON_CONVEX_DYNAMIC_MESH),VMESSAGEBOX_OKELI);
#else
        printf("%s\n",IDS_WARNING_WITH_NEWTON_NON_CONVEX_DYNAMIC_MESH);
#endif
        _newtonDynamicRandomMeshNotSupportedMark++;
    }
}

void CDynamicsContainer::setDynamicEngineType(int t,int version)
{
    _dynamicEngineToUse=t;
    _dynamicEngineVersionToUse=version;
    App::setLightDialogRefreshFlag(); // will trigger a refresh
}

int CDynamicsContainer::getDynamicEngineType(int* version)
{
    if (version!=nullptr)
        version[0]=_dynamicEngineVersionToUse;
    return(_dynamicEngineToUse);
}

void CDynamicsContainer::setDisplayContactPoints(bool d)
{
    _displayContactPoints=d;
}

bool CDynamicsContainer::getDisplayContactPoints()
{
    return(_displayContactPoints);
}


void CDynamicsContainer::setUseDynamicDefaultCalculationParameters(int def)
{
    def=tt::getLimitedInt(0,4,def);
    _dynamicDefaultTypeCalculationParameters=def;
}

int CDynamicsContainer::getUseDynamicDefaultCalculationParameters()
{
    return(_dynamicDefaultTypeCalculationParameters);
}

bool CDynamicsContainer::setCurrentDynamicStepSize(float s)
{ // will modify the current engine's step size if setting is custom
    if (App::ct->simulation->isSimulationStopped())
    {
        if (_dynamicEngineToUse==sim_physics_bullet)
            setEngineFloatParam(sim_bullet_global_stepsize,s,false);
        if (_dynamicEngineToUse==sim_physics_ode)
            setEngineFloatParam(sim_ode_global_stepsize,s,false);
        if (_dynamicEngineToUse==sim_physics_vortex)
            setEngineFloatParam(sim_vortex_global_stepsize,s,false);
        if (_dynamicEngineToUse==sim_physics_newton)
            setEngineFloatParam(sim_newton_global_stepsize,s,false);
        return(_dynamicDefaultTypeCalculationParameters==4);
    }
    return(false);
}

float CDynamicsContainer::getCurrentDynamicStepSize()
{
    if (_dynamicEngineToUse==sim_physics_bullet)
        return(getEngineFloatParam(sim_bullet_global_stepsize,nullptr));
    if (_dynamicEngineToUse==sim_physics_ode)
        return(getEngineFloatParam(sim_ode_global_stepsize,nullptr));
    if (_dynamicEngineToUse==sim_physics_vortex)
        return(getEngineFloatParam(sim_vortex_global_stepsize,nullptr));
    if (_dynamicEngineToUse==sim_physics_newton)
        return(getEngineFloatParam(sim_newton_global_stepsize,nullptr));
    return(0.0f);
}

bool CDynamicsContainer::setCurrentIterationCount(int c)
{ // will modify the current engine's it. count if setting is custom
    if (App::ct->simulation->isSimulationStopped())
    {
        if (_dynamicEngineToUse==sim_physics_bullet)
            setEngineIntParam(sim_bullet_global_constraintsolvingiterations,c,false);
        if (_dynamicEngineToUse==sim_physics_ode)
            setEngineIntParam(sim_ode_global_constraintsolvingiterations,c,false);
        if (_dynamicEngineToUse==sim_physics_vortex)
            return(false); // not available
        if (_dynamicEngineToUse==sim_physics_newton)
            setEngineIntParam(sim_newton_global_constraintsolvingiterations,c,false);
        return(_dynamicDefaultTypeCalculationParameters==4);
    }
    return(false);
}

int CDynamicsContainer::getCurrentIterationCount()
{
    if (_dynamicEngineToUse==sim_physics_bullet)
        return(getEngineIntParam(sim_bullet_global_constraintsolvingiterations,nullptr));
    if (_dynamicEngineToUse==sim_physics_ode)
        return(getEngineIntParam(sim_ode_global_constraintsolvingiterations,nullptr));
    if (_dynamicEngineToUse==sim_physics_vortex)
        return(0); // not available
    if (_dynamicEngineToUse==sim_physics_newton)
        return(getEngineIntParam(sim_newton_global_constraintsolvingiterations,nullptr));
    return(0.0f);
}

float CDynamicsContainer::getEngineFloatParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_global_float_start)&&(what<sim_bullet_global_float_end))
    {
        std::vector<float> fp;
        getBulletFloatParams(fp); // use this routine, since default params are not in _xxxFloatParams
        int w=what-sim_bullet_global_stepsize+simi_bullet_global_stepsize;
        return(fp[w]);
    }
    if ((what>sim_ode_global_float_start)&&(what<sim_ode_global_float_end))
    {
        std::vector<float> fp;
        getOdeFloatParams(fp); // use this routine, since default params are not in _xxxFloatParams
        int w=what-sim_ode_global_stepsize+simi_ode_global_stepsize;
        return(fp[w]);
    }
    if ((what>sim_vortex_global_float_start)&&(what<sim_vortex_global_float_end))
    {
        std::vector<float> fp;
        getVortexFloatParams(fp); // use this routine, since default params are not in _xxxFloatParams
        int w=what-sim_vortex_global_stepsize+simi_vortex_global_stepsize;
        return(fp[w]);
    }
    if ((what>sim_newton_global_float_start)&&(what<sim_newton_global_float_end))
    {
        std::vector<float> fp;
        getNewtonFloatParams(fp); // use this routine, since default params are not in _xxxFloatParams
        int w=what-sim_newton_global_stepsize+simi_newton_global_stepsize;
        return(fp[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0.0);
}

int CDynamicsContainer::getEngineIntParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_global_int_start)&&(what<sim_bullet_global_int_end))
    {
        std::vector<int> ip;
        getBulletIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        int w=what-sim_bullet_global_constraintsolvingiterations+simi_bullet_global_constraintsolvingiterations;
        return(ip[w]);
    }
    if ((what>sim_ode_global_int_start)&&(what<sim_ode_global_int_end))
    {
        std::vector<int> ip;
        getOdeIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        int w=what-sim_ode_global_constraintsolvingiterations+simi_ode_global_constraintsolvingiterations;
        return(ip[w]);
    }
    if ((what>sim_vortex_global_int_start)&&(what<sim_vortex_global_int_end))
    {
        std::vector<int> ip;
        getVortexIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        int w=what-sim_vortex_global_bitcoded+simi_vortex_global_bitcoded;
        return(ip[w]);
    }
    if ((what>sim_newton_global_int_start)&&(what<sim_newton_global_int_end))
    {
        std::vector<int> ip;
        getNewtonIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        int w=what-sim_newton_global_constraintsolvingiterations+simi_newton_global_constraintsolvingiterations;
        return(ip[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

bool CDynamicsContainer::getEngineBoolParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_global_bool_start)&&(what<sim_bullet_global_bool_end))
    {
        int b=1;
        int w=(what-sim_bullet_global_fullinternalscaling);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getBulletIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        return((ip[simi_bullet_global_bitcoded]&b)!=0);
    }
    if ((what>sim_ode_global_bool_start)&&(what<sim_ode_global_bool_end))
    {
        int b=1;
        int w=(what-sim_ode_global_fullinternalscaling);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getOdeIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        return((ip[simi_ode_global_bitcoded]&b)!=0);
    }
    if ((what>sim_vortex_global_bool_start)&&(what<sim_vortex_global_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_global_autosleep);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getVortexIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        return((ip[simi_vortex_global_bitcoded]&b)!=0);
    }
    if ((what>sim_newton_global_bool_start)&&(what<sim_newton_global_bool_end))
    {
        int b=1;
        int w=(what-sim_newton_global_multithreading);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getNewtonIntParams(ip); // use this routine, since default params are not in _xxxIntParams
        return((ip[simi_newton_global_bitcoded]&b)!=0);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

bool CDynamicsContainer::setEngineFloatParam(int what,float v,bool setDirect)
{
    if ((what>sim_bullet_global_float_start)&&(what<sim_bullet_global_float_end))
    {
        int w=what-sim_bullet_global_stepsize+simi_bullet_global_stepsize;
        std::vector<float> fp;
        getBulletFloatParams(fp);
        fp[w]=v;
        setBulletFloatParams(fp,setDirect);
        return(true);
    }
    if ((what>sim_ode_global_float_start)&&(what<sim_ode_global_float_end))
    {
        int w=what-sim_ode_global_stepsize+simi_ode_global_stepsize;
        std::vector<float> fp;
        getOdeFloatParams(fp);
        fp[w]=v;
        setOdeFloatParams(fp,setDirect);
        return(true);
    }
    if ((what>sim_vortex_global_float_start)&&(what<sim_vortex_global_float_end))
    {
        int w=what-sim_vortex_global_stepsize+simi_vortex_global_stepsize;
        std::vector<float> fp;
        getVortexFloatParams(fp);
        fp[w]=v;
        setVortexFloatParams(fp,setDirect);
        return(true);
    }
    if ((what>sim_newton_global_float_start)&&(what<sim_newton_global_float_end))
    {
        int w=what-sim_newton_global_stepsize+simi_newton_global_stepsize;
        std::vector<float> fp;
        getNewtonFloatParams(fp);
        fp[w]=v;
        setNewtonFloatParams(fp,setDirect);
        return(true);
    }
    return(false);
}

bool CDynamicsContainer::setEngineIntParam(int what,int v,bool setDirect)
{
    if ((what>sim_bullet_global_int_start)&&(what<sim_bullet_global_int_end))
    {
        int w=what-sim_bullet_global_constraintsolvingiterations+simi_bullet_global_constraintsolvingiterations;
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[w]=v;
        setBulletIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_ode_global_int_start)&&(what<sim_ode_global_int_end))
    {
        int w=what-sim_ode_global_constraintsolvingiterations+simi_ode_global_constraintsolvingiterations;
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[w]=v;
        setOdeIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_vortex_global_int_start)&&(what<sim_vortex_global_int_end))
    {
        int w=what-sim_vortex_global_bitcoded+simi_vortex_global_bitcoded;
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[w]=v;
        setVortexIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_newton_global_int_start)&&(what<sim_newton_global_int_end))
    {
        int w=what-sim_newton_global_constraintsolvingiterations+simi_newton_global_constraintsolvingiterations;
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[w]=v;
        setNewtonIntParams(ip,setDirect);
        return(true);
    }
    return(false);
}

bool CDynamicsContainer::setEngineBoolParam(int what,bool v,bool setDirect)
{
    if ((what>sim_bullet_global_bool_start)&&(what<sim_bullet_global_bool_end))
    {
        int b=1;
        int w=(what-sim_bullet_global_fullinternalscaling);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[simi_bullet_global_bitcoded]|=b;
        if (!v)
            ip[simi_bullet_global_bitcoded]-=b;
        setBulletIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_ode_global_bool_start)&&(what<sim_ode_global_bool_end))
    {
        int b=1;
        int w=(what-sim_ode_global_fullinternalscaling);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[simi_ode_global_bitcoded]|=b;
        if (!v)
            ip[simi_ode_global_bitcoded]-=b;
        setOdeIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_vortex_global_bool_start)&&(what<sim_vortex_global_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_global_autosleep);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[simi_vortex_global_bitcoded]|=b;
        if (!v)
            ip[simi_vortex_global_bitcoded]-=b;
        setVortexIntParams(ip,setDirect);
        return(true);
    }
    if ((what>sim_newton_global_bool_start)&&(what<sim_newton_global_bool_end))
    {
        int b=1;
        int w=(what-sim_newton_global_multithreading);
        while (w>0) {b*=2; w--;}
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[simi_newton_global_bitcoded]|=b;
        if (!v)
            ip[simi_newton_global_bitcoded]-=b;
        setNewtonIntParams(ip,setDirect);
        return(true);
    }
    return(false);
}

void CDynamicsContainer::getBulletFloatParams(std::vector<float>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
    else
        getBulletDefaultFloatParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getBulletDefaultFloatParams(std::vector<float>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_BULLET_DEFAULT_STEP_SIZE[defType]); // simi_bullet_global_stepsize
    p.push_back(DYNAMIC_BULLET_DEFAULT_INTERNAL_SCALING_FACTOR[defType]); // simi_bullet_global_internalscalingfactor
    p.push_back(DYNAMIC_BULLET_DEFAULT_COLLISION_MARGIN_FACTOR[defType]); // simi_bullet_global_collisionmarginfactor
    p.push_back(0.0); // free
    p.push_back(0.0); // free
    // BULLET_FLOAT_PARAM_CNT_CURRENT=5
}
void CDynamicsContainer::setBulletFloatParams(const std::vector<float>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _bulletFloatParams.assign(p.begin(),p.end());
    _bulletFloatParams[simi_bullet_global_stepsize]=tt::getLimitedFloat(0.00001f,1.0f,_bulletFloatParams[simi_bullet_global_stepsize]); // step size
    _bulletFloatParams[simi_bullet_global_internalscalingfactor]=tt::getLimitedFloat(0.0001f,10000.0f,_bulletFloatParams[simi_bullet_global_internalscalingfactor]); // internal scaling factor
    _bulletFloatParams[simi_bullet_global_collisionmarginfactor]=tt::getLimitedFloat(0.001f,100.0f,_bulletFloatParams[simi_bullet_global_collisionmarginfactor]); // collision margin factor
}
void CDynamicsContainer::getBulletIntParams(std::vector<int>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_bulletIntParams.begin(),_bulletIntParams.end());
    else
        getBulletDefaultIntParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getBulletDefaultIntParams(std::vector<int>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_BULLET_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]); // simi_bullet_global_constraintsolvingiterations
    int v=0;
    if (DYNAMIC_BULLET_DEFAULT_FULL_INTERNAL_SCALING[defType])
        v|=simi_bullet_global_fullinternalscaling;
    p.push_back(v); // simi_bullet_global_bitcoded
    p.push_back(sim_bullet_constraintsolvertype_sequentialimpulse); // simi_bullet_global_constraintsolvertype
    // BULLET_INT_PARAM_CNT_CURRENT=3
}
void CDynamicsContainer::setBulletIntParams(const std::vector<int>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _bulletIntParams.assign(p.begin(),p.end());
    _bulletIntParams[simi_bullet_global_constraintsolvingiterations]=tt::getLimitedFloat(1,10000,_bulletIntParams[simi_bullet_global_constraintsolvingiterations]); // constr. solv. iterations
}


void CDynamicsContainer::getOdeFloatParams(std::vector<float>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_odeFloatParams.begin(),_odeFloatParams.end());
    else
        getOdeDefaultFloatParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getOdeDefaultFloatParams(std::vector<float>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_ODE_DEFAULT_STEP_SIZE[defType]); // simi_bullet_global_stepsize
    p.push_back(DYNAMIC_ODE_DEFAULT_INTERNAL_SCALING_FACTOR[defType]); // simi_bullet_global_internalscalingfactor
    p.push_back(DYNAMIC_ODE_GLOBAL_CFM[defType]); // simi_bullet_global_cfm
    p.push_back(DYNAMIC_ODE_GLOBAL_ERP[defType]); // simi_bullet_global_erp
    p.push_back(0.0); // free
    // ODE_FLOAT_PARAM_CNT_CURRENT=5
}
void CDynamicsContainer::setOdeFloatParams(const std::vector<float>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _odeFloatParams.assign(p.begin(),p.end());
    _odeFloatParams[simi_ode_global_stepsize]=tt::getLimitedFloat(0.00001f,1.0f,_odeFloatParams[simi_ode_global_stepsize]); // step size
    _odeFloatParams[simi_ode_global_internalscalingfactor]=tt::getLimitedFloat(0.0001f,10000.0f,_odeFloatParams[simi_ode_global_internalscalingfactor]); // internal scaling factor
    _odeFloatParams[simi_ode_global_cfm]=tt::getLimitedFloat(0.0f,1.0f,_odeFloatParams[simi_ode_global_cfm]); // global CFM
    _odeFloatParams[simi_ode_global_erp]=tt::getLimitedFloat(0.0f,1.0f,_odeFloatParams[simi_ode_global_erp]); // global ERP
}
void CDynamicsContainer::getOdeIntParams(std::vector<int>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_odeIntParams.begin(),_odeIntParams.end());
    else
        getOdeDefaultIntParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getOdeDefaultIntParams(std::vector<int>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_ODE_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]); // simi_ode_global_constraintsolvingiterations
    int v=0;
    if (DYNAMIC_ODE_DEFAULT_FULL_INTERNAL_SCALING[defType])
        v|=simi_ode_global_fullinternalscaling;
    if (DYNAMIC_ODE_USE_QUICKSTEP[defType])
        v|=simi_ode_global_quickstep;
    p.push_back(v); // simi_ode_global_bitcoded
    p.push_back(-1); // simi_ode_global_randomseed
    // ODE_INT_PARAM_CNT_CURRENT=3
}
void CDynamicsContainer::setOdeIntParams(const std::vector<int>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _odeIntParams.assign(p.begin(),p.end());
    _odeIntParams[simi_ode_global_constraintsolvingiterations]=tt::getLimitedFloat(1,10000,_odeIntParams[simi_ode_global_constraintsolvingiterations]); // constr. solv. iterations
}


void CDynamicsContainer::getVortexFloatParams(std::vector<float>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
    else
        getVortexDefaultFloatParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getVortexDefaultFloatParams(std::vector<float>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_VORTEX_DEFAULT_STEP_SIZE[defType]); // simi_vortex_global_stepsize
    p.push_back(DYNAMIC_VORTEX_DEFAULT_INTERNAL_SCALING_FACTOR[defType]); // simi_vortex_global_internalscalingfactor
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONTACT_TOLERANCE[defType]); // simi_vortex_global_contacttolerance
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_COMPLIANCE[defType]); // simi_vortex_global_constraintlinearcompliance
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_DAMPING[defType]); // simi_vortex_global_constraintlineardamping
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_LIN_KIN_LOSS[defType]); // simi_vortex_global_constraintlinearkineticloss
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_COMPLIANCE[defType]); // simi_vortex_global_constraintangularcompliance
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_DAMPING[defType]); // simi_vortex_global_constraintangulardamping
    p.push_back(DYNAMIC_VORTEX_DEFAULT_CONSTRAINT_ANG_KIN_LOSS[defType]); // simi_vortex_global_constraintangularkineticloss
    p.push_back(0.01f); // RESERVED. used to be auto angular damping tension ratio, not used anymore
    // VORTEX_FLOAT_PARAM_CNT_CURRENT=10
}
void CDynamicsContainer::setVortexFloatParams(const std::vector<float>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _vortexFloatParams.assign(p.begin(),p.end());
    _vortexFloatParams[simi_vortex_global_stepsize]=tt::getLimitedFloat(0.00001f,1.0f,_vortexFloatParams[simi_vortex_global_stepsize]); // step size
    _vortexFloatParams[simi_vortex_global_internalscalingfactor]=tt::getLimitedFloat(0.0001f,10000.0f,_vortexFloatParams[simi_vortex_global_internalscalingfactor]); // internal scaling factor
    _vortexFloatParams[simi_vortex_global_contacttolerance]=tt::getLimitedFloat(0.0f,10.0f,_vortexFloatParams[simi_vortex_global_contacttolerance]); // contact tolerance
    // _vortexFloatParams[9] is RESERVED (used to be the auto angular damping tension ratio)
}
void CDynamicsContainer::getVortexIntParams(std::vector<int>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_vortexIntParams.begin(),_vortexIntParams.end());
    else
        getVortexDefaultIntParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getVortexDefaultIntParams(std::vector<int>& p,int defType)
{
    p.clear();
    int v=0;
    if (DYNAMIC_VORTEX_DEFAULT_AUTO_SLEEP[defType])
        v|=simi_vortex_global_autosleep;
    if (DYNAMIC_VORTEX_DEFAULT_MULTITHREADING[defType])
        v|=simi_vortex_global_multithreading;
    v|=4; // always on by default (full internal scaling)
    // bit4 (8) is RESERVED!! (was auto-angular damping)
    p.push_back(v); // simi_vortex_global_bitcoded
    // VORTEX_INT_PARAM_CNT_CURRENT=1
}
void CDynamicsContainer::setVortexIntParams(const std::vector<int>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _vortexIntParams.assign(p.begin(),p.end());
}


void CDynamicsContainer::getNewtonFloatParams(std::vector<float>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
    else
        getNewtonDefaultFloatParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getNewtonDefaultFloatParams(std::vector<float>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_NEWTON_DEFAULT_STEP_SIZE[defType]); // simi_newton_global_stepsize
    p.push_back(DYNAMIC_NEWTON_DEFAULT_CONTACT_MERGE_TOLERANCE[defType]); // simi_newton_global_contactmergetolerance
    // NEWTON_FLOAT_PARAM_CNT_CURRENT=2
}
void CDynamicsContainer::setNewtonFloatParams(const std::vector<float>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _newtonFloatParams.assign(p.begin(),p.end());
    _newtonFloatParams[simi_newton_global_stepsize]=tt::getLimitedFloat(0.00001f,1.0f,_newtonFloatParams[simi_newton_global_stepsize]); // step size
    _newtonFloatParams[simi_newton_global_contactmergetolerance]=tt::getLimitedFloat(0.0001f,1.0f,_newtonFloatParams[simi_newton_global_contactmergetolerance]); // contact merge tolerance
}
void CDynamicsContainer::getNewtonIntParams(std::vector<int>& p)
{
    if (_dynamicDefaultTypeCalculationParameters==4)
        p.assign(_newtonIntParams.begin(),_newtonIntParams.end());
    else
        getNewtonDefaultIntParams(p,_dynamicDefaultTypeCalculationParameters);
}
void CDynamicsContainer::getNewtonDefaultIntParams(std::vector<int>& p,int defType)
{
    p.clear();
    p.push_back(DYNAMIC_NEWTON_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]); // simi_newton_global_constraintsolvingiterations
    int options;
    if (DYNAMIC_NEWTON_DEFAULT_MULTITHREADED[defType])
        options|=simi_newton_global_multithreading;
    if (DYNAMIC_NEWTON_DEFAULT_EXACT_SOLVER[defType])
        options|=simi_newton_global_exactsolver;
    if (DYNAMIC_NEWTON_DEFAULT_HIGH_JOINT_ACCURACY[defType])
        options|=simi_newton_global_highjointaccuracy;
    p.push_back(options); // simi_newton_global_bitcoded
    // NEWTON_INT_PARAM_CNT_CURRENT=2
}

void CDynamicsContainer::setNewtonIntParams(const std::vector<int>& p,bool setDirect)
{
    if ((_dynamicDefaultTypeCalculationParameters==4)||setDirect)
        _newtonIntParams.assign(p.begin(),p.end());
    _newtonIntParams[simi_newton_global_constraintsolvingiterations]=tt::getLimitedInt(1,128,_newtonIntParams[simi_newton_global_constraintsolvingiterations]); // solving iterations
}




float CDynamicsContainer::getPositionScalingFactorDyn()
{
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
        return(DYNAMIC_BULLET_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_odeFloatParams[simi_ode_global_internalscalingfactor]);
        return(DYNAMIC_ODE_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
        return(DYNAMIC_VORTEX_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        return(1.0f);
        //if (_dynamicDefaultTypeCalculationParameters==4)
        //  return(_newtonFloatParams[1]);
        //return(DYNAMIC_NEWTON_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    return(1.0f);
}

float CDynamicsContainer::getGravityScalingFactorDyn()
{
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
        return(DYNAMIC_BULLET_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_odeFloatParams[simi_ode_global_internalscalingfactor]);
        return(DYNAMIC_ODE_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
        return(DYNAMIC_VORTEX_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        return(1.0f);
        //if (_dynamicDefaultTypeCalculationParameters==4)
        //  return(_newtonFloatParams[1]);
        //return(DYNAMIC_NEWTON_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    return(1.0f);
}

float CDynamicsContainer::getLinearVelocityScalingFactorDyn()
{
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
        return(DYNAMIC_BULLET_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_odeFloatParams[simi_ode_global_internalscalingfactor]);
        return(DYNAMIC_ODE_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            return(_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
        return(DYNAMIC_VORTEX_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        return(1.0f);
        //if (_dynamicDefaultTypeCalculationParameters==4)
        //  return(_newtonFloatParams[1]);
        //return(DYNAMIC_NEWTON_DEFAULT_INTERNAL_SCALING_FACTOR[_dynamicDefaultTypeCalculationParameters]);
    }
    return(1.0f);
}

float CDynamicsContainer::getMassScalingFactorDyn()
{
    bool full=false;
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_bulletIntParams[simi_bullet_global_bitcoded]&simi_bullet_global_fullinternalscaling);
        else
            full=DYNAMIC_BULLET_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_odeIntParams[simi_ode_global_bitcoded]&simi_ode_global_fullinternalscaling);
        else
            full=DYNAMIC_ODE_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        full=true;
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        full=true;
    }
    if (full)
    {
        float f=getPositionScalingFactorDyn();
        return(f*f*f);
    }
    return(1.0f);
}

float CDynamicsContainer::getMasslessInertiaScalingFactorDyn()
{
    float f=getPositionScalingFactorDyn();
    return(f*f);
}

float CDynamicsContainer::getForceScalingFactorDyn()
{
    bool full=false;
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_bulletIntParams[simi_bullet_global_bitcoded]&simi_bullet_global_fullinternalscaling);
        else
            full=DYNAMIC_BULLET_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_odeIntParams[simi_ode_global_bitcoded]&simi_ode_global_fullinternalscaling);
        else
            full=DYNAMIC_ODE_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        full=true;
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        full=true;
    }

    float f=getPositionScalingFactorDyn();
    if (full)
        return(f*f*f*f);
    return(f);
}

float CDynamicsContainer::getTorqueScalingFactorDyn()
{
    bool full=false;
    if (_dynamicEngineToUse==sim_physics_bullet)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_bulletIntParams[simi_bullet_global_bitcoded]&simi_bullet_global_fullinternalscaling);
        else
            full=DYNAMIC_BULLET_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_ode)
    {
        if (_dynamicDefaultTypeCalculationParameters==4)
            full=(_odeIntParams[simi_ode_global_bitcoded]&simi_ode_global_fullinternalscaling);
        else
            full=DYNAMIC_ODE_DEFAULT_FULL_INTERNAL_SCALING[_dynamicDefaultTypeCalculationParameters];
    }
    if (_dynamicEngineToUse==sim_physics_vortex)
    {
        full=true;
    }
    if (_dynamicEngineToUse==sim_physics_newton)
    {
        full=true;
    }

    float f=getPositionScalingFactorDyn();
    if (full)
        return(f*f*f*f*f);
    return(f*f);
}

void CDynamicsContainer::setDynamicsEnabled(bool e)
{
    _dynamicsEnabled=e;
    if (!e)
        App::ct->dynamicsContainer->removeWorld();
    else
    {
        if (App::ct->simulation->isSimulationRunning())
            App::ct->dynamicsContainer->addWorldIfNotThere();
    }
}

bool CDynamicsContainer::getDynamicsEnabled()
{
    return(_dynamicsEnabled);
}

void CDynamicsContainer::setGravity(const C3Vector& gr)
{
    _gravity=gr;
    _gravity(0)=tt::getLimitedFloat(-1000.0f,+1000.0f,_gravity(0));
    _gravity(1)=tt::getLimitedFloat(-1000.0f,+1000.0f,_gravity(1));
    _gravity(2)=tt::getLimitedFloat(-1000.0f,+1000.0f,_gravity(2));
}

C3Vector CDynamicsContainer::getGravity()
{
    return(_gravity);
}

void CDynamicsContainer::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("En2"); // En2 since we have the Vortex option
            ar << _dynamicEngineToUse << _gravity(0) << _gravity(1) << _gravity(2) << _dynamicDefaultTypeCalculationParameters;
            ar.flush();

            ar.storeDataName("Ver");
            ar << _dynamicEngineVersionToUse;
            ar.flush();

            ar.storeDataName("Bul"); // keep a while for file write backw. compatibility (09/03/2016)
            // ar << _dynamicsBULLETStepSize << _dynamicBULLETInternalScalingFactor << _dynamicBULLETConstraintSolvingIterations << _dynamicBULLETCollisionMarginFactor;
            ar << _bulletFloatParams[simi_bullet_global_stepsize] << _bulletFloatParams[simi_bullet_global_internalscalingfactor] << _bulletIntParams[simi_bullet_global_constraintsolvingiterations] << _bulletFloatParams[simi_bullet_global_collisionmarginfactor];
            ar.flush();

            ar.storeDataName("Ode"); // keep a while for file write backw. compatibility (09/03/2016)
            // ar << _dynamicsODEStepSize << _dynamicODEInternalScalingFactor << _dynamicODEConstraintSolvingIterations << _dynamicODEGlobalCFM << _dynamicODEGlobalERP;
            ar << _odeFloatParams[simi_ode_global_stepsize] << _odeFloatParams[simi_ode_global_internalscalingfactor] << _odeIntParams[simi_ode_global_constraintsolvingiterations] << _odeFloatParams[simi_ode_global_cfm] << _odeFloatParams[simi_ode_global_erp];
            ar.flush();

            ar.storeDataName("Vo5"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (int i=0;i<int(_vortexFloatParams.size());i++)
                ar << _vortexFloatParams[i];
            for (int i=0;i<int(_vortexIntParams.size());i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("Nw1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (int i=0;i<int(_newtonFloatParams.size());i++)
                ar << _newtonFloatParams[i];
            for (int i=0;i<int(_newtonIntParams.size());i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_dynamicsEnabled);
            SIM_SET_CLEAR_BIT(dummy,1,_displayContactPoints);
            SIM_SET_CLEAR_BIT(dummy,2,(_bulletIntParams[simi_bullet_global_bitcoded]&simi_bullet_global_fullinternalscaling)!=0); // _dynamicBULLETFullInternalScaling, keep a while for file write backw. compatibility (09/03/2016)
            SIM_SET_CLEAR_BIT(dummy,3,(_odeIntParams[simi_ode_global_bitcoded]&simi_ode_global_fullinternalscaling)!=0); // _dynamicODEFullInternalScaling, keep a while for file write backw. compatibility (09/03/2016)
            SIM_SET_CLEAR_BIT(dummy,4,(_odeIntParams[simi_ode_global_bitcoded]&simi_ode_global_quickstep)!=0); // _dynamicODEUseQuickStep, keep a while for file write backw. compatibility (09/03/2016)
            // reserved SIM_SET_CLEAR_BIT(dummy,5,_dynamicVORTEXFullInternalScaling);
            ar << dummy;
            ar.flush();

            ar.storeDataName("BuN"); // Bullet params (keep this after "Bul" and "Var"):
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (int i=0;i<int(_bulletFloatParams.size());i++)
                ar << _bulletFloatParams[i];
            for (int i=0;i<int(_bulletIntParams.size());i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("OdN"); // ODE params (keep this after "Ode" and "Var"):
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (int i=0;i<int(_odeFloatParams.size());i++)
                ar << _odeFloatParams[i];
            for (int i=0;i<int(_odeIntParams.size());i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;

                    if (theName.compare("Eng")==0)
                    { // keep for backward compatibility (23/09/2013)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse >> _gravity(0) >> _gravity(1) >> _gravity(2) >>_dynamicDefaultTypeCalculationParameters;
                    }

                    if (theName.compare("En2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse >> _gravity(0) >> _gravity(1) >> _gravity(2) >>_dynamicDefaultTypeCalculationParameters;
                    }

                    if (theName.compare("Ver")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineVersionToUse;
                    }

                    if (theName.compare("Bul")==0)
                    { // Keep for backward compatibility (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _dynamicsBULLETStepSize >> _dynamicBULLETInternalScalingFactor >> _dynamicBULLETConstraintSolvingIterations >> _dynamicBULLETCollisionMarginFactor;
                        ar >> _bulletFloatParams[simi_bullet_global_stepsize] >> _bulletFloatParams[simi_bullet_global_internalscalingfactor] >> _bulletIntParams[simi_bullet_global_constraintsolvingiterations] >> _bulletFloatParams[simi_bullet_global_collisionmarginfactor];
                    }

                    if (theName.compare("Ode")==0)
                    { // Keep for backward compatibility (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _dynamicsODEStepSize >> _dynamicODEInternalScalingFactor >> _dynamicODEConstraintSolvingIterations >> _dynamicODEGlobalCFM >> _dynamicODEGlobalERP;
                        ar >> _odeFloatParams[simi_ode_global_stepsize] >> _odeFloatParams[simi_ode_global_internalscalingfactor] >> _odeIntParams[simi_ode_global_constraintsolvingiterations] >> _odeFloatParams[simi_ode_global_cfm] >> _odeFloatParams[simi_ode_global_erp];
                    }

                    if (theName.compare("Vo5")==0)
                    { // vortex params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_vortexFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_vortexIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if (ar.getVrepVersionThatWroteThisFile()<30400)
                        { // In Vortex Studio we have some crashes and instability due to multithreading. At the same time, if multithreading is off, it is faster for V-REP scenes
                            _vortexIntParams[0]|=simi_vortex_global_multithreading;
                            _vortexIntParams[0]-=simi_vortex_global_multithreading;
                        }
                    }
                    if (theName.compare("Nw1")==0)
                    { // Newton params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_newtonFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_newtonIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("BuN")==0)
                    { // Bullet params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_bulletFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_bulletIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("OdN")==0)
                    { // ODE params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_odeFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_odeIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;

                        _dynamicsEnabled=SIM_IS_BIT_SET(dummy,0);
                        _displayContactPoints=SIM_IS_BIT_SET(dummy,1);
                        bool dynBULLETFullInternalScaling=SIM_IS_BIT_SET(dummy,2); // keep for backw. compatibility (09/03/2016)
                        bool dynODEFullInternalScaling=SIM_IS_BIT_SET(dummy,3); // keep for backw. compatibility (09/03/2016)
                        bool dynODEUseQuickStep=SIM_IS_BIT_SET(dummy,4); // keep for backw. compatibility (09/03/2016)
                        // reserved _dynamicVORTEXFullInternalScaling=SIM_IS_BIT_SET(dummy,5);

                        // Following for backw. compatibility (09/03/2016)
                        if (dynBULLETFullInternalScaling)
                            _bulletIntParams[simi_bullet_global_bitcoded]|=simi_bullet_global_fullinternalscaling;
                        else
                            _bulletIntParams[simi_bullet_global_bitcoded]=(_bulletIntParams[simi_bullet_global_bitcoded]|simi_bullet_global_fullinternalscaling)-simi_bullet_global_fullinternalscaling;
                        if (dynODEFullInternalScaling)
                            _odeIntParams[simi_ode_global_bitcoded]|=simi_ode_global_fullinternalscaling;
                        else
                            _odeIntParams[simi_ode_global_bitcoded]=(_odeIntParams[simi_ode_global_bitcoded]|simi_ode_global_fullinternalscaling)-simi_ode_global_fullinternalscaling;
                        if (dynODEUseQuickStep)
                            _odeIntParams[simi_ode_global_bitcoded]|=simi_ode_global_quickstep;
                        else
                            _odeIntParams[simi_ode_global_bitcoded]=(_odeIntParams[simi_ode_global_bitcoded]|simi_ode_global_quickstep)-simi_ode_global_quickstep;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CDynamicsContainer::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{ // Has to be displayed as overlay!
    if (isWorldThere())
    {
        if ((displayAttrib&sim_displayattribute_noparticles)==0)
        {
            int index=0;
            float* cols;
            int objectType;
            int particlesCount;
            C4X4Matrix m(renderingObject->getCumulativeTransformation().getMatrix());
            void** particlesPointer=CPluginContainer::dyn_getParticles(index++,&particlesCount,&objectType,&cols);
            while (particlesCount!=-1)
            {
                if ((particlesPointer!=nullptr)&&(particlesCount>0)&&((objectType&sim_particle_invisible)==0))
                {
                    if ( ((displayAttrib&sim_displayattribute_forvisionsensor)==0)||(objectType&sim_particle_painttag) )
                        displayParticles(particlesPointer,particlesCount,displayAttrib,m,cols,objectType);
                }
                particlesPointer=CPluginContainer::dyn_getParticles(index++,&particlesCount,&objectType,&cols);
            }
        }
    }
}

void CDynamicsContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib)
{ // Has to be displayed as overlay!
    if (isWorldThere())
    {
        if ((displayAttrib&sim_displayattribute_noparticles)==0)
        {
            if ((displayAttrib&sim_displayattribute_renderpass)&&((displayAttrib&sim_displayattribute_forvisionsensor)==0) )
            {
                if (getDisplayContactPoints())
                {
                    int cnt=0;
                    float* pts=CPluginContainer::dyn_getContactPoints(&cnt);

                    displayContactPoints(displayAttrib,contactPointColor,pts,cnt);
                }
            }
        }
    }
}
