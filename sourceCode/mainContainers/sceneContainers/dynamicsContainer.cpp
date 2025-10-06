#include <simInternal.h>
#include <dynamicsContainer.h>
#include <app.h>
#include <simStringTable.h>
#include <tt.h>
#include <engineProperties.h>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <dynamicsRendering.h>
#include <vMessageBox.h>
#include <guiApp.h>
#endif

CDynamicsContainer::CDynamicsContainer()
{
    _dynamicsEnabled = true;

    getBulletDefaultFloatParams(_bulletFloatParams);
    getBulletDefaultIntParams(_bulletIntParams);

    getOdeDefaultFloatParams(_odeFloatParams);
    getOdeDefaultIntParams(_odeIntParams);

    getVortexDefaultFloatParams(_vortexFloatParams);
    getVortexDefaultIntParams(_vortexIntParams);

    getNewtonDefaultFloatParams(_newtonFloatParams);
    getNewtonDefaultIntParams(_newtonIntParams);

    getMujocoDefaultFloatParams(_mujocoFloatParams);
    getMujocoDefaultIntParams(_mujocoIntParams);

    _dynamicEngineToUse = sim_physics_bullet; // Bullet is default
    _dynamicEngineVersionToUse = 0;           // this is Bullet 2.78
    _engineSettingsAreDefault = true;

    contactPointColor.setColorsAllBlack();
    contactPointColor.setColor(1.0, 1.0, 0.0, sim_colorcomponent_emission);
    _displayContactPoints = false;
    _tempDisabledWarnings = 0;
    _currentlyInDynamicsCalculations = false;

    _gravity = C3Vector(0.0, 0.0, -9.81);
    _resetWarningFlags();
}

CDynamicsContainer::~CDynamicsContainer()
{ // beware, the current world could be nullptr
}

bool CDynamicsContainer::getSettingsAreDefault() const
{
    return (_engineSettingsAreDefault);
}

void CDynamicsContainer::simulationAboutToStart()
{
    _resetWarningFlags();
    _tempDisabledWarnings = 0;

    removeWorld(); // not really needed

    // Keep following (important that it is initialized BEFORE simHandleDynamics is called!!)
    if (getDynamicsEnabled())
        addWorldIfNotThere();
}

void CDynamicsContainer::simulationEnded()
{
    removeWorld();
    _resetWarningFlags();
    _tempDisabledWarnings = 0;
}

void CDynamicsContainer::_resetWarningFlags()
{
    _containsNonPureNonConvexShapes = 0;
    _containsStaticShapesOnDynamicConstruction = 0;
    _pureSpheroidNotSupportedMark = 0;
    _pureConeNotSupportedMark = 0;
    _pureHollowShapeNotSupportedMark = 0;
    _physicsEngineNotSupportedWarning = 0;
    _vortexPluginIsDemoWarning = 0;
    _stringsNotSupportedWarning = 0;
}

void CDynamicsContainer::setTempDisabledWarnings(int mask)
{
    _tempDisabledWarnings = mask;
}

int CDynamicsContainer::getTempDisabledWarnings() const
{
    return (_tempDisabledWarnings);
}

bool CDynamicsContainer::getCurrentlyInDynamicsCalculations() const
{
    return (_currentlyInDynamicsCalculations);
}

void CDynamicsContainer::handleDynamics(double dt)
{
    App::worldContainer->calcInfo->dynamicsStart();
    addWorldIfNotThere();

    if (getDynamicsEnabled())
    {
        _currentlyInDynamicsCalculations = true;
        App::worldContainer->pluginContainer->dyn_step(dt, App::currentWorld->simulation->getSimulationTime());
        _currentlyInDynamicsCalculations = false;
    }

    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
        App::currentWorld->sceneObjects->getShapeFromIndex(i)->decrementRespondableSuspendCount();

    if (App::worldContainer->pluginContainer->dyn_isDynamicContentAvailable())
        App::worldContainer->calcInfo->dynamicsEnd(App::worldContainer->pluginContainer->dyn_getDynamicStepDivider(),
                                                   true);
    else
        App::worldContainer->calcInfo->dynamicsEnd(0, false);
}

bool CDynamicsContainer::getContactForce(int dynamicPass, int objectHandle, int index, int objectHandles[2],
                                         double* contactInfo) const
{
    if (getDynamicsEnabled())
        return (App::worldContainer->pluginContainer->dyn_getContactForce(dynamicPass, objectHandle, index,
                                                                          objectHandles, contactInfo) != 0);
    return (false);
}

void CDynamicsContainer::addWorldIfNotThere()
{
    if (getDynamicsEnabled() && (!isWorldThere()))
    {
        double floatParams[20];
        int intParams[20];
        int floatIndex = 0;
        int intIndex = 0;

        floatParams[floatIndex++] = getPositionScalingFactorDyn();
        floatParams[floatIndex++] = getLinearVelocityScalingFactorDyn();
        floatParams[floatIndex++] = getMassScalingFactorDyn();
        floatParams[floatIndex++] = getMasslessInertiaScalingFactorDyn();
        floatParams[floatIndex++] = getForceScalingFactorDyn();
        floatParams[floatIndex++] = getTorqueScalingFactorDyn();
        floatParams[floatIndex++] = getGravityScalingFactorDyn();
        floatParams[floatIndex++] = App::userSettings->dynamicActivityRange;

        intParams[intIndex++] = SIM_IDEND_SCENEOBJECT + 1;
        intParams[intIndex++] = SIM_IDSTART_SCENEOBJECT;
        intParams[intIndex++] = SIM_IDEND_SCENEOBJECT;

        App::worldContainer->pluginContainer->dyn_startSimulation(_dynamicEngineToUse, _dynamicEngineVersionToUse, floatParams, intParams);
    }
}

void CDynamicsContainer::removeWorld()
{
    if (isWorldThere())
        App::worldContainer->pluginContainer->dyn_endSimulation();
}

bool CDynamicsContainer::isWorldThere() const
{
    return (App::worldContainer->pluginContainer->dyn_isInitialized());
}

void CDynamicsContainer::markForWarningDisplay_pureSpheroidNotSupported()
{
    if (_pureSpheroidNotSupportedMark == 0)
        _pureSpheroidNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_pureConeNotSupported()
{
    if (_pureConeNotSupportedMark == 0)
        _pureConeNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_pureHollowShapeNotSupported()
{
    if (_pureHollowShapeNotSupportedMark == 0)
        _pureHollowShapeNotSupportedMark++;
}

void CDynamicsContainer::markForWarningDisplay_physicsEngineNotSupported()
{
    if (_physicsEngineNotSupportedWarning == 0)
        _physicsEngineNotSupportedWarning++;
}

void CDynamicsContainer::markForWarningDisplay_vortexPluginIsDemo()
{
    if (_vortexPluginIsDemoWarning == 0)
        _vortexPluginIsDemoWarning++;
}

void CDynamicsContainer::markForWarningDisplay_containsNonPureNonConvexShapes()
{
    if (_containsNonPureNonConvexShapes == 0)
        _containsNonPureNonConvexShapes++;
}

void CDynamicsContainer::markForWarningDisplay_containsStaticShapesOnDynamicConstruction()
{
    if (_containsStaticShapesOnDynamicConstruction == 0)
        _containsStaticShapesOnDynamicConstruction++;
}

void CDynamicsContainer::displayWarningsIfNeeded()
{
    if (App::getConsoleVerbosity() >= sim_verbosity_warnings)
    {
        if ((_pureSpheroidNotSupportedMark == 1) && ((_tempDisabledWarnings & 1) == 0))
        {
            App::logMsg(sim_verbosity_warnings,
                        "Detected a dynamically enabled, primitive non-spherical spheroid shape, that cannot be "
                        "simulated with currently selected engine. The shape will be handled as a primitive sphere.");
            _pureSpheroidNotSupportedMark++;
        }
        if ((_pureConeNotSupportedMark == 1) && ((_tempDisabledWarnings & 2) == 0))
        {
            App::logMsg(sim_verbosity_warnings,
                        "Detected a dynamically enabled, primitive cone shape, that cannot be simulated with currently "
                        "selected engine. The shape will be handled as a primitive cylinder.");
            _pureConeNotSupportedMark++;
        }
        if ((_pureHollowShapeNotSupportedMark == 1) && ((_tempDisabledWarnings & 4) == 0))
        {
            App::logMsg(sim_verbosity_warnings,
                        "Detected a dynamically enabled, primitive hollow shape, that cannot be simulated with "
                        "currently selected engine. The shape will be handled as a convex shape.");
            _pureHollowShapeNotSupportedMark++;
        }
        if ((_physicsEngineNotSupportedWarning == 1) && ((_tempDisabledWarnings & 8) == 0))
        {
            App::logMsg(sim_verbosity_warnings, "The physics engine currently selected is not supported (you might be "
                                                "missing a required plugin). Simulation will not run correctly.");
            _physicsEngineNotSupportedWarning++;
        }
    }

    if ((_containsNonPureNonConvexShapes == 1) && ((_tempDisabledWarnings & 16) == 0))
    {
        _containsNonPureNonConvexShapes++;
        if ((_dynamicEngineToUse == sim_physics_newton) || (_dynamicEngineToUse == sim_physics_mujoco))
            App::logMsg(sim_verbosity_warnings, "Detected dynamically enabled, non-convex shapes. Those will be "
                                                "handled by the physics engine as convex shapes.");
        else
            App::logMsg(sim_verbosity_warnings, "Detected dynamically enabled, non-convex shapes. Those might "
                                                "drastically slow down simulation, and introduce unstable behaviour.");
    }

    if ((_containsStaticShapesOnDynamicConstruction == 1) && ((_tempDisabledWarnings & 32) == 0))
    {
        _containsStaticShapesOnDynamicConstruction++;
        App::logMsg(sim_verbosity_warnings, "Detected a static, respondable shape, built on top of a non-static tree. "
                                            "This might lead to strange and unrealistic behaviour.");
    }

#ifndef MAC_SIM
    if ((_vortexPluginIsDemoWarning == 1) && ((_tempDisabledWarnings & 128) == 0))
    {
        _vortexPluginIsDemoWarning++;
        App::logMsg(sim_verbosity_warnings,
                    "Detected an unregistered version of the Vortex engine. You may obtain a free license for the "
                    "Vortex engine by downloading Vortex Studio Academic (2020a) here:\nwww.cm-labs.com");
    }
#endif

    if (_stringsNotSupportedWarning == 0)
    {
        if (_dynamicEngineToUse != sim_physics_mujoco)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_dummy); i++)
            {
                CDummy* it = App::currentWorld->sceneObjects->getDummyFromIndex(i);
                if ((it->getLinkedDummyHandle() != -1) && (it->getDummyType() == sim_dummytype_dyntendon))
                {
                    App::logMsg(sim_verbosity_warnings, "Detected tendon constraints, which are only supported with the MuJoCo engine");
                    _stringsNotSupportedWarning++;
                    break;
                }
            }
        }
    }
}

void CDynamicsContainer::setDynamicEngineType(int t, int version)
{
    bool diff = (_dynamicEngineToUse != t) || (_dynamicEngineVersionToUse != version);
    if (diff)
    {
        _dynamicEngineToUse = t;
        _dynamicEngineVersionToUse = version;
        if (App::worldContainer->getEventsEnabled())
        {
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_dynamicsEngine.name, true);
            int ar[2] = {_dynamicEngineToUse, _dynamicEngineVersionToUse};
            ev->appendKeyIntArray(propDyn_dynamicsEngine.name, ar, 2);
            App::worldContainer->pushEvent();
        }
        checkIfEngineSettingsAreDefault();
#ifdef SIM_WITH_GUI
        GuiApp::setLightDialogRefreshFlag();
#endif
    }
}

int CDynamicsContainer::getDynamicEngineType(int* version) const
{
    if (version != nullptr)
        version[0] = _dynamicEngineVersionToUse;
    return (_dynamicEngineToUse);
}

void CDynamicsContainer::setDisplayContactPoints(bool d)
{
    bool diff = (_displayContactPoints != d);
    if (diff)
    {
        _displayContactPoints = d;
        if (App::worldContainer->getEventsEnabled())
        {
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_showContactPoints.name, true);
            ev->appendKeyBool(propDyn_showContactPoints.name, _displayContactPoints);
            App::worldContainer->pushEvent();
        }
    }
}

bool CDynamicsContainer::getDisplayContactPoints() const
{
    return (_displayContactPoints);
}

bool CDynamicsContainer::setDesiredStepSize(double s)
{
    bool retVal = false;
    if (App::currentWorld->simulation->isSimulationStopped())
    {
        s = tt::getLimitedFloat(0.00001, 1.0, s);
        bool diff = (_stepSize != s);
        if (diff)
        {
            _stepSize = s;
            // Following for backward compatibility:
            _bulletFloatParams[simi_bullet_global_stepsize] = s;
            _odeFloatParams[simi_ode_global_stepsize] = s;
            _vortexFloatParams[simi_vortex_global_stepsize] = s;
            _newtonFloatParams[simi_newton_global_stepsize] = s;
            _mujocoFloatParams[simi_mujoco_global_stepsize] = s;
            if (App::worldContainer->getEventsEnabled())
            {
                CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_dynamicsStepSize.name, true);
                ev->appendKeyDouble(propDyn_dynamicsStepSize.name, _stepSize);
                App::worldContainer->pushEvent();
            }
            retVal = true;
        }
    }
    return (retVal);
}

double CDynamicsContainer::getDesiredStepSize() const
{
    return (_stepSize);
}

double CDynamicsContainer::getEffectiveStepSize() const
{
    double retVal = _stepSize;
    double sim = App::currentWorld->simulation->getTimeStep();
    int dynPasses = int((sim / retVal) + 0.5);
    if (dynPasses < 1)
        dynPasses = 1;
    retVal = sim / double(dynPasses);
    return (retVal);
}

bool CDynamicsContainer::getComputeInertias() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return getBoolPropertyValue(propDyn_bulletComputeInertias.name);
    if (_dynamicEngineToUse == sim_physics_ode)
        return getBoolPropertyValue(propDyn_odeComputeInertias.name);
    if (_dynamicEngineToUse == sim_physics_vortex)
        return getBoolPropertyValue(propDyn_vortexComputeInertias.name);
    if (_dynamicEngineToUse == sim_physics_newton)
        return getBoolPropertyValue(propDyn_newtonComputeInertias.name);
    if (_dynamicEngineToUse == sim_physics_mujoco)
        return getBoolPropertyValue(propDyn_mujocoComputeInertias.name);
    return (false);
}

bool CDynamicsContainer::setIterationCount(int c)
{
    bool retVal = false;
    if (App::currentWorld->simulation->isSimulationStopped())
    {
        if (_dynamicEngineToUse == sim_physics_bullet)
            setIntProperty(propDyn_bulletIterations.name, c);
        if (_dynamicEngineToUse == sim_physics_ode)
            setIntProperty(propDyn_odeQuickStepIterations.name, c);
        if (_dynamicEngineToUse == sim_physics_vortex)
            return (false); // not available
        if (_dynamicEngineToUse == sim_physics_newton)
            setIntProperty(propDyn_newtonIterations.name, c);
        if (_dynamicEngineToUse == sim_physics_mujoco)
            setIntProperty(propDyn_mujocoIterations.name, c);
        retVal = true;
    }
    return (retVal);
}

int CDynamicsContainer::getIterationCount() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return (_bulletIntParams[simi_bullet_global_constraintsolvingiterations]);
    if (_dynamicEngineToUse == sim_physics_ode)
        return (_odeIntParams[simi_ode_global_constraintsolvingiterations]);
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (0); // not available
    if (_dynamicEngineToUse == sim_physics_newton)
        return (_newtonIntParams[simi_newton_global_constraintsolvingiterations]);
    if (_dynamicEngineToUse == sim_physics_mujoco)
        return (_mujocoIntParams[simi_mujoco_global_iterations]);
    return (0);
}

double CDynamicsContainer::getEngineFloatParam_old(int what, bool* ok, bool getDefault /*=false*/) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        double v;
        if (getFloatProperty(prop.c_str(), v) > 0)
            return v;
    }
    prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> v;
        if (getFloatArrayProperty(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector v;
        if (getVector3Property(prop.c_str(), v) > 0)
            return v(indexWithArrays);
    }
    if ((what == sim_bullet_global_stepsize) || (what == sim_ode_global_stepsize) || (what == sim_vortex_global_stepsize) || (what == sim_newton_global_stepsize) || (what == sim_mujoco_global_stepsize))
        return (_stepSize);

    if (ok != nullptr)
        ok[0] = false;
    return 0.0;
}

int CDynamicsContainer::getEngineIntParam_old(int what, bool* ok, bool getDefault /*=false*/) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        int v;
        if (getIntProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return 0;
}

bool CDynamicsContainer::getEngineBoolParam_old(int what, bool* ok, bool getDefault /*=false*/) const
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        bool v;
        if (getBoolProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return false;
}

bool CDynamicsContainer::setEngineFloatParam_old(int what, double v)
{
    bool retVal = false;

    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setFloatProperty(prop.c_str(), v) > 0)
            retVal = true;
    }
    if (!retVal)
    {
        prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
        if (prop.size() > 0)
        {
            std::vector<double> w;
            if (getFloatArrayProperty(prop.c_str(), w) > 0)
            {
                w[indexWithArrays] = v;
                if (setFloatArrayProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
                    retVal = true;
            }
        }
    }
    if (!retVal)
    {
        prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
        if (prop.size() > 0)
        {
            C3Vector w;
            if (getVector3Property(prop.c_str(), w) > 0)
            {
                w(indexWithArrays) = v;
                if (setVector3Property(prop.c_str(), &w) > 0)
                    retVal = true;
            }
        }
    }
    if (!retVal)
    { // for backw. compatibility
        if ((what == sim_bullet_global_stepsize) || (what == sim_ode_global_stepsize) || (what == sim_vortex_global_stepsize) || (what == sim_newton_global_stepsize) || (what == sim_mujoco_global_stepsize))
        {
            _stepSize = v;
            retVal = true;
            checkIfEngineSettingsAreDefault();
        }
    }

    return (retVal);
}

bool CDynamicsContainer::setEngineIntParam_old(int what, int v)
{
    bool retVal = false;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setIntProperty(prop.c_str(), v) > 0)
            retVal = true;
    }
    return (retVal);
}

bool CDynamicsContainer::setEngineBoolParam_old(int what, bool v)
{
    bool retVal = false;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setBoolProperty(prop.c_str(), v) > 0)
            retVal = true;
    }
    return (retVal);
}

void CDynamicsContainer::getVortexFloatParams(std::vector<double>& p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getVortexDefaultFloatParams(p);
    else
        p.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
}

void CDynamicsContainer::getVortexIntParams(std::vector<int>& p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getVortexDefaultIntParams(p);
    else
        p.assign(_vortexIntParams.begin(), _vortexIntParams.end());
}

void CDynamicsContainer::getNewtonFloatParams(std::vector<double>& p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getNewtonDefaultFloatParams(p);
    else
        p.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
}

void CDynamicsContainer::getNewtonIntParams(std::vector<int>& p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getNewtonDefaultIntParams(p);
    else
        p.assign(_newtonIntParams.begin(), _newtonIntParams.end());
}

double CDynamicsContainer::getPositionScalingFactorDyn() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return (_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_ode)
        return (_odeFloatParams[simi_ode_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
    return (1.0);
}

double CDynamicsContainer::getGravityScalingFactorDyn() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return (_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_ode)
        return (_odeFloatParams[simi_ode_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
    return (1.0);
}

double CDynamicsContainer::getLinearVelocityScalingFactorDyn() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return (_bulletFloatParams[simi_bullet_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_ode)
        return (_odeFloatParams[simi_ode_global_internalscalingfactor]);
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (_vortexFloatParams[simi_vortex_global_internalscalingfactor]);
    return (1.0);
}

double CDynamicsContainer::getMassScalingFactorDyn() const
{
    bool full = false;
    if (_dynamicEngineToUse == sim_physics_bullet)
        full = (_bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_ode)
        full = (_odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_vortex)
        full = true;
    if (full)
    {
        double f = getPositionScalingFactorDyn();
        return (f * f * f);
    }
    return (1.0);
}

double CDynamicsContainer::getMasslessInertiaScalingFactorDyn() const
{
    double f = getPositionScalingFactorDyn();
    return (f * f);
}

double CDynamicsContainer::getForceScalingFactorDyn() const
{
    bool full = false;
    if (_dynamicEngineToUse == sim_physics_bullet)
        full = (_bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_ode)
        full = (_odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_vortex)
        full = true;

    double f = getPositionScalingFactorDyn();
    if (full)
        return (f * f * f * f);
    return (f);
}

double CDynamicsContainer::getTorqueScalingFactorDyn() const
{
    bool full = false;
    if (_dynamicEngineToUse == sim_physics_bullet)
        full = (_bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_ode)
        full = (_odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_fullinternalscaling);
    if (_dynamicEngineToUse == sim_physics_vortex)
        full = true;

    double f = getPositionScalingFactorDyn();
    if (full)
        return (f * f * f * f * f);
    return (f * f);
}

void CDynamicsContainer::setDynamicsEnabled(bool e)
{
    bool diff = (_dynamicsEnabled != e);
    if (diff)
    {
        _dynamicsEnabled = e;
        if (App::worldContainer->getEventsEnabled())
        {
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_dynamicsEnabled.name, true);
            ev->appendKeyBool(propDyn_dynamicsEnabled.name, _dynamicsEnabled);
            App::worldContainer->pushEvent();
        }
        if (!e)
            App::currentWorld->dynamicsContainer->removeWorld();
        else
        {
            if (App::currentWorld->simulation->isSimulationRunning())
                App::currentWorld->dynamicsContainer->addWorldIfNotThere();
        }
    }
}

bool CDynamicsContainer::getDynamicsEnabled() const
{
    return (_dynamicsEnabled);
}

void CDynamicsContainer::setGravity(C3Vector gr)
{
    gr(0) = tt::getLimitedFloat(-1000.0, +1000.0, gr(0));
    gr(1) = tt::getLimitedFloat(-1000.0, +1000.0, gr(1));
    gr(2) = tt::getLimitedFloat(-1000.0, +1000.0, gr(2));
    bool diff = (_gravity != gr);
    if (diff)
    {
        _gravity = gr;
        if (App::worldContainer->getEventsEnabled())
        {
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_gravity.name, true);
            ev->appendKeyDoubleArray(propDyn_gravity.name, _gravity.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

C3Vector CDynamicsContainer::getGravity() const
{
    return (_gravity);
}

bool CDynamicsContainer::_engineFloatsAreSimilar(const std::vector<double>& arr1, const std::vector<double>& arr2) const
{ // allow for a 1% deviation max
    if (arr1.size() != arr2.size())
        return (false);
    for (size_t i = 0; i < arr1.size(); i++)
    {
        if ((arr1[i] == 0.0) || (arr2[i] == 0.0))
        {
            if (arr1[i] != arr2[i])
                return (false);
        }
        else
        {
            if (fabs((arr1[i] - arr2[i]) / arr1[i]) > 0.01)
                return (false);
        }
    }
    return (true);
}

void CDynamicsContainer::checkIfEngineSettingsAreDefault()
{
    _engineSettingsAreDefault = true;

    if (((_stepSize - 0.005) / 0.005) > 0.01)
    {
        _engineSettingsAreDefault = false;
        return;
    }

    std::vector<double> fVals;
    std::vector<int> iVals;

    if (_dynamicEngineToUse == sim_physics_bullet)
    {
        getBulletDefaultFloatParams(fVals);
        getBulletDefaultIntParams(iVals);
        if (!_engineFloatsAreSimilar(fVals, _bulletFloatParams))
        {
            _engineSettingsAreDefault = false;
            return;
        }
        for (size_t i = 0; i < iVals.size(); i++)
        {
            if (iVals[i] != _bulletIntParams[i])
            {
                _engineSettingsAreDefault = false;
                return;
            }
        }
    }

    if (_dynamicEngineToUse == sim_physics_ode)
    {
        getOdeDefaultFloatParams(fVals);
        getOdeDefaultIntParams(iVals);
        if (!_engineFloatsAreSimilar(fVals, _odeFloatParams))
        {
            _engineSettingsAreDefault = false;
            return;
        }
        for (size_t i = 0; i < iVals.size(); i++)
        {
            if (iVals[i] != _odeIntParams[i])
            {
                _engineSettingsAreDefault = false;
                return;
            }
        }
    }

    if (_dynamicEngineToUse == sim_physics_vortex)
    {
        getVortexDefaultFloatParams(fVals);
        getVortexDefaultIntParams(iVals);
        if (!_engineFloatsAreSimilar(fVals, _vortexFloatParams))
        {
            _engineSettingsAreDefault = false;
            return;
        }
        for (size_t i = 0; i < iVals.size(); i++)
        {
            if (iVals[i] != _vortexIntParams[i])
            {
                _engineSettingsAreDefault = false;
                return;
            }
        }
    }

    if (_dynamicEngineToUse == sim_physics_newton)
    {
        getNewtonDefaultFloatParams(fVals);
        getNewtonDefaultIntParams(iVals);
        if (!_engineFloatsAreSimilar(fVals, _newtonFloatParams))
        {
            _engineSettingsAreDefault = false;
            return;
        }
        for (size_t i = 0; i < iVals.size(); i++)
        {
            if (iVals[i] != _newtonIntParams[i])
            {
                _engineSettingsAreDefault = false;
                return;
            }
        }
    }

    if (_dynamicEngineToUse == sim_physics_mujoco)
    {
        getMujocoDefaultFloatParams(fVals);
        getMujocoDefaultIntParams(iVals);
        if (!_engineFloatsAreSimilar(fVals, _mujocoFloatParams))
        {
            _engineSettingsAreDefault = false;
            return;
        }
        for (size_t i = 0; i < iVals.size(); i++)
        {
            if (iVals[i] != _mujocoIntParams[i])
            {
                _engineSettingsAreDefault = false;
                return;
            }
        }
    }
}

void CDynamicsContainer::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_n3");
            ar << _dynamicEngineToUse;
            ar << _gravity(0) << _gravity(1) << _gravity(2);
            ar << int(5); // 5 is for backw. compat. (dyn. settings mode=custom)
            ar.flush();

            ar.storeDataName("Ver");
            ar << _dynamicEngineVersionToUse;
            ar.flush();

            ar.storeDataName("_tp"); // since 17.08.2022, step size is not individual to engine anymore
            ar << _stepSize;
            ar.flush();

            ar.storeDataName("_o5"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (int i = 0; i < int(_vortexFloatParams.size()); i++)
                ar << _vortexFloatParams[i];
            for (int i = 0; i < int(_vortexIntParams.size()); i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("_w1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (int i = 0; i < int(_newtonFloatParams.size()); i++)
                ar << _newtonFloatParams[i];
            for (int i = 0; i < int(_newtonIntParams.size()); i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("_j1"); // mujoco params:
            ar << int(_mujocoFloatParams.size()) << int(_mujocoIntParams.size());
            for (int i = 0; i < int(_mujocoFloatParams.size()); i++)
                ar << _mujocoFloatParams[i];
            for (int i = 0; i < int(_mujocoIntParams.size()); i++)
                ar << _mujocoIntParams[i];
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _dynamicsEnabled);
            SIM_SET_CLEAR_BIT(dummy, 1, _displayContactPoints);
            SIM_SET_CLEAR_BIT(
                dummy, 2,
                (_bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling) !=
                    0); // _dynamicBULLETFullInternalScaling, keep a while for file write backw. compatibility
                        // (09/03/2016)
            SIM_SET_CLEAR_BIT(
                dummy, 3,
                (_odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_fullinternalscaling) !=
                    0); // _dynamicODEFullInternalScaling, keep a while for file write backw. compatibility (09/03/2016)
            SIM_SET_CLEAR_BIT(
                dummy, 4,
                (_odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_quickstep) !=
                    0); // _dynamicODEUseQuickStep, keep a while for file write backw. compatibility (09/03/2016)
            // reserved SIM_SET_CLEAR_BIT(dummy,5,_dynamicVORTEXFullInternalScaling);
            ar << dummy;
            ar.flush();

            ar.storeDataName("_uN"); // Bullet params (keep this after "Bul" and "Var"):
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (int i = 0; i < int(_bulletFloatParams.size()); i++)
                ar << _bulletFloatParams[i];
            for (int i = 0; i < int(_bulletIntParams.size()); i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("_dN"); // ODE params (keep this after "Ode" and "Var"):
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (int i = 0; i < int(_odeFloatParams.size()); i++)
                ar << _odeFloatParams[i];
            for (int i = 0; i < int(_odeIntParams.size()); i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            int oldDynamicsSettingsMode = 5; // i.e. custom
            bool hasStepSizeTag = false;
            _stepSize = 0.005; // just in case
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;

                    if (theName.compare("Eng") == 0)
                    { // keep for backward compatibility (23/09/2013)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse;
                        float bla;
                        for (size_t i = 0; i < 3; i++)
                        {
                            ar >> bla;
                            _gravity(i) = (double)bla;
                        }
                        ar >> oldDynamicsSettingsMode;
                        oldDynamicsSettingsMode++;
                    }

                    if (theName.compare("En2") == 0)
                    { // keep for backward compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse;
                        float bla;
                        for (size_t i = 0; i < 3; i++)
                        {
                            ar >> bla;
                            _gravity(i) = (double)bla;
                        }
                        ar >> oldDynamicsSettingsMode;
                        oldDynamicsSettingsMode++;
                    }
                    if (theName.compare("En3") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse;
                        float bla;
                        for (size_t i = 0; i < 3; i++)
                        {
                            ar >> bla;
                            _gravity(i) = (double)bla;
                        }
                        ar >> oldDynamicsSettingsMode;
                    }

                    if (theName.compare("_n3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineToUse;
                        ar >> _gravity(0) >> _gravity(1) >> _gravity(2);
                        ar >> oldDynamicsSettingsMode;
                    }

                    if (theName.compare("Ver") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dynamicEngineVersionToUse;
                    }

                    if (theName.compare("Stp") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _stepSize = (double)bla;
                        hasStepSizeTag = true;
                    }

                    if (theName.compare("_tp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _stepSize;
                        hasStepSizeTag = true;
                    }

                    if (theName.compare("Bul") == 0)
                    { // Keep for backward compatibility (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _bulletFloatParams[simi_bullet_global_stepsize] = (double)bla;
                        _bulletFloatParams[simi_bullet_global_internalscalingfactor] = (double)bli;
                        ar >> _bulletIntParams[simi_bullet_global_constraintsolvingiterations];
                        ar >> bla;
                        _bulletFloatParams[simi_bullet_global_collisionmarginfactor] = (double)bla;
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_bullet))
                            _stepSize = _bulletFloatParams[simi_bullet_global_stepsize];
                    }

                    if (theName.compare("Ode") == 0)
                    { // Keep for backward compatibility (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _odeFloatParams[simi_ode_global_stepsize] = (double)bla;
                        _odeFloatParams[simi_ode_global_internalscalingfactor] = (double)bli;
                        ar >> _odeIntParams[simi_ode_global_constraintsolvingiterations];
                        ar >> bla >> bli;
                        _odeFloatParams[simi_ode_global_cfm] = (double)bla;
                        _odeFloatParams[simi_ode_global_erp] = (double)bli;
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_ode))
                            _stepSize = _odeFloatParams[simi_ode_global_stepsize];
                    }

                    if (theName.compare("Vo5") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if (ar.getCoppeliaSimVersionThatWroteThisFile() < 30400)
                        { // In Vortex Studio we have some crashes and instability due to multithreading. At the same
                            // time, if multithreading is off, it is faster for CoppeliaSim scenes
                            _vortexIntParams[0] |= simi_vortex_global_multithreading;
                            _vortexIntParams[0] -= simi_vortex_global_multithreading;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_vortex))
                            _stepSize = _vortexFloatParams[simi_vortex_global_stepsize];
                    }

                    if (theName.compare("_o5") == 0)
                    { // vortex params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if (ar.getCoppeliaSimVersionThatWroteThisFile() < 30400)
                        { // In Vortex Studio we have some crashes and instability due to multithreading. At the same
                            // time, if multithreading is off, it is faster for CoppeliaSim scenes
                            _vortexIntParams[0] |= simi_vortex_global_multithreading;
                            _vortexIntParams[0] -= simi_vortex_global_multithreading;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_vortex))
                            _stepSize = _vortexFloatParams[simi_vortex_global_stepsize];
                    }

                    if (theName.compare("Nw1") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_newton))
                            _stepSize = _newtonFloatParams[simi_newton_global_stepsize];
                    }

                    if (theName.compare("_w1") == 0)
                    { // Newton params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_newton))
                            _stepSize = _newtonFloatParams[simi_newton_global_stepsize];
                    }

                    if (theName.compare("Mj1") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_mujoco))
                            _stepSize = _mujocoFloatParams[simi_mujoco_global_stepsize];
                    }

                    if (theName.compare("_j1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_mujoco))
                            _stepSize = _mujocoFloatParams[simi_mujoco_global_stepsize];
                    }

                    if (theName.compare("BuN") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_bullet))
                            _stepSize = _bulletFloatParams[simi_bullet_global_stepsize];
                    }

                    if (theName.compare("_uN") == 0)
                    { // Bullet params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_bullet))
                            _stepSize = _bulletFloatParams[simi_bullet_global_stepsize];
                    }

                    if (theName.compare("OdN") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_ode))
                            _stepSize = _odeFloatParams[simi_ode_global_stepsize];
                    }

                    if (theName.compare("_dN") == 0)
                    { // ODE params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_ode))
                            _stepSize = _odeFloatParams[simi_ode_global_stepsize];
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;

                        _dynamicsEnabled = SIM_IS_BIT_SET(dummy, 0);
                        _displayContactPoints = SIM_IS_BIT_SET(dummy, 1);
                        bool dynBULLETFullInternalScaling =
                            SIM_IS_BIT_SET(dummy, 2); // keep for backw. compatibility (09/03/2016)
                        bool dynODEFullInternalScaling =
                            SIM_IS_BIT_SET(dummy, 3); // keep for backw. compatibility (09/03/2016)
                        bool dynODEUseQuickStep =
                            SIM_IS_BIT_SET(dummy, 4); // keep for backw. compatibility (09/03/2016)
                        // reserved _dynamicVORTEXFullInternalScaling=SIM_IS_BIT_SET(dummy,5);

                        // Following for backw. compatibility (09/03/2016)
                        if (dynBULLETFullInternalScaling)
                            _bulletIntParams[simi_bullet_global_bitcoded] |= simi_bullet_global_fullinternalscaling;
                        else
                            _bulletIntParams[simi_bullet_global_bitcoded] =
                                (_bulletIntParams[simi_bullet_global_bitcoded] |
                                 simi_bullet_global_fullinternalscaling) -
                                simi_bullet_global_fullinternalscaling;
                        if (dynODEFullInternalScaling)
                            _odeIntParams[simi_ode_global_bitcoded] |= simi_ode_global_fullinternalscaling;
                        else
                            _odeIntParams[simi_ode_global_bitcoded] =
                                (_odeIntParams[simi_ode_global_bitcoded] | simi_ode_global_fullinternalscaling) -
                                simi_ode_global_fullinternalscaling;
                        if (dynODEUseQuickStep)
                            _odeIntParams[simi_ode_global_bitcoded] |= simi_ode_global_quickstep;
                        else
                            _odeIntParams[simi_ode_global_bitcoded] =
                                (_odeIntParams[simi_ode_global_bitcoded] | simi_ode_global_quickstep) -
                                simi_ode_global_quickstep;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _fixVortexInfVals();
            if (!hasStepSizeTag)
            {
                if (oldDynamicsSettingsMode < 5)
                { // for backward compatibility. Previously this was one of the default settings (precise, balanced,
                    // fast, etc.)
                    getBulletDefaultFloatParams(_bulletFloatParams, oldDynamicsSettingsMode);
                    getBulletDefaultIntParams(_bulletIntParams, oldDynamicsSettingsMode);
                    getOdeDefaultFloatParams(_odeFloatParams, oldDynamicsSettingsMode);
                    getOdeDefaultIntParams(_odeIntParams, oldDynamicsSettingsMode);
                    getVortexDefaultFloatParams(_vortexFloatParams, oldDynamicsSettingsMode);
                    getVortexDefaultIntParams(_vortexIntParams, oldDynamicsSettingsMode);
                    getNewtonDefaultFloatParams(_newtonFloatParams, oldDynamicsSettingsMode);
                    getNewtonDefaultIntParams(_newtonIntParams, oldDynamicsSettingsMode);
                    getMujocoDefaultFloatParams(_mujocoFloatParams, oldDynamicsSettingsMode);
                    getMujocoDefaultIntParams(_mujocoIntParams, oldDynamicsSettingsMode);
                }
                _bulletFloatParams[simi_bullet_global_stepsize] = _stepSize;
                _odeFloatParams[simi_ode_global_stepsize] = _stepSize;
                _vortexFloatParams[simi_vortex_global_stepsize] = _stepSize;
                _newtonFloatParams[simi_newton_global_stepsize] = _stepSize;
                _mujocoFloatParams[simi_mujoco_global_stepsize] = _stepSize;
            }
            checkIfEngineSettingsAreDefault();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'engine' tag: can be 'bullet', 'ode', 'vortex' or 'newton' ", exhaustiveXml);
            ar.xmlAddNode_enum("engine", _dynamicEngineToUse, sim_physics_bullet, "bullet", sim_physics_ode, "ode",
                               sim_physics_vortex, "vortex", sim_physics_newton, "newton", sim_physics_mujoco,
                               "mujoco");

            ar.xmlAddNode_int("engineVersion", _dynamicEngineVersionToUse);

            ar.xmlAddNode_float("stepsize", _stepSize);

            ar.xmlAddNode_comment(" 'settingsMode' tag: keep at 5", exhaustiveXml);
            ar.xmlAddNode_int("settingsMode", 5);

            ar.xmlAddNode_floats("gravity", _gravity.data, 3);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("dynamicsEnabled", _dynamicsEnabled);
            ar.xmlAddNode_bool("showContactPoints", _displayContactPoints);
            ar.xmlPopNode();

            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("bullet");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("internalscalingfactor",
                                _bulletFloatParams[simi_bullet_global_internalscalingfactor]);
            ar.xmlAddNode_float("collisionmarginfactor",
                                _bulletFloatParams[simi_bullet_global_collisionmarginfactor]);

            ar.xmlAddNode_int("constraintsolvingiterations",
                              _bulletIntParams[simi_bullet_global_constraintsolvingiterations]);
            ar.xmlAddNode_int("constraintsolvertype",
                              _bulletIntParams[simi_bullet_global_constraintsolvertype]);

            ar.xmlAddNode_bool("fullinternalscaling", _bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling);
            ar.xmlAddNode_bool("computeinertias", _bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_computeinertias);
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("internalscalingfactor",
                                _odeFloatParams[simi_ode_global_internalscalingfactor]);
            ar.xmlAddNode_float("cfm", _odeFloatParams[simi_ode_global_cfm]);
            ar.xmlAddNode_float("erp", _odeFloatParams[simi_ode_global_erp]);

            ar.xmlAddNode_int("constraintsolvingiterations",
                              _odeIntParams[simi_ode_global_constraintsolvingiterations]);
            ar.xmlAddNode_int("randomseed", _odeIntParams[simi_ode_global_randomseed]);

            ar.xmlAddNode_bool("fullinternalscaling", getBoolPropertyValue(propDyn_odeInternalScalingFull.name));
            ar.xmlAddNode_bool("quickstep", getBoolPropertyValue(propDyn_odeQuickStepEnabled.name));
            ar.xmlAddNode_bool("computeinertias", getBoolPropertyValue(propDyn_odeComputeInertias.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("internalscalingfactor",
                                _vortexFloatParams[simi_vortex_global_internalscalingfactor]);
            ar.xmlAddNode_float("contacttolerance", _vortexFloatParams[simi_vortex_global_contacttolerance]);
            ar.xmlAddNode_float("constraintlinearcompliance",
                                _vortexFloatParams[simi_vortex_global_constraintlinearcompliance]);
            ar.xmlAddNode_float("constraintlineardamping",
                                _vortexFloatParams[simi_vortex_global_constraintlineardamping]);
            ar.xmlAddNode_float("constraintlinearkineticloss",
                                _vortexFloatParams[simi_vortex_global_constraintlinearkineticloss]);
            ar.xmlAddNode_float("constraintangularcompliance",
                                _vortexFloatParams[simi_vortex_global_constraintangularcompliance]);
            ar.xmlAddNode_float("constraintangulardamping",
                                _vortexFloatParams[simi_vortex_global_constraintangulardamping]);
            ar.xmlAddNode_float("constraintangularkineticloss",
                                _vortexFloatParams[simi_vortex_global_constraintangularkineticloss]);

            ar.xmlAddNode_bool("autosleep", getBoolPropertyValue(propDyn_vortexAutoSleep.name));
            ar.xmlAddNode_bool("multithreading", getBoolPropertyValue(propDyn_vortexMultithreading.name));
            ar.xmlAddNode_bool("computeinertias", getBoolPropertyValue(propDyn_vortexComputeInertias.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("contactmergetolerance", _newtonFloatParams[simi_newton_global_contactmergetolerance]);
            ar.xmlAddNode_int("constraintsolvingiterations", _newtonFloatParams[simi_newton_global_constraintsolvingiterations]);
            ar.xmlAddNode_bool("multithreading", getBoolPropertyValue(propDyn_newtonMultithreading.name));
            ar.xmlAddNode_bool("exactsolver", getBoolPropertyValue(propDyn_newtonExactSolver.name));
            ar.xmlAddNode_bool("highjointaccuracy", getBoolPropertyValue(propDyn_newtonHighJointAccuracy.name));
            ar.xmlAddNode_bool("computeinertias", getBoolPropertyValue(propDyn_newtonComputeInertias.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            ar.xmlAddNode_float("impratio", _mujocoFloatParams[simi_mujoco_global_impratio]);
            double w[5];
            for (size_t j = 0; j < 3; j++)
                w[j] = _mujocoFloatParams[simi_mujoco_global_wind1 + int(j)];
            ar.xmlAddNode_floats("wind", w, 3);
            ar.xmlAddNode_float("density", _mujocoFloatParams[simi_mujoco_global_density]);
            ar.xmlAddNode_float("viscosity", _mujocoFloatParams[simi_mujoco_global_viscosity]);
            ar.xmlAddNode_float("boundmass", _mujocoFloatParams[simi_mujoco_global_boundmass]);
            ar.xmlAddNode_float("boundinertia", _mujocoFloatParams[simi_mujoco_global_boundinertia]);
            ar.xmlAddNode_float("overridemargin", _mujocoFloatParams[simi_mujoco_global_overridemargin]);
            for (size_t j = 0; j < 2; j++)
                w[j] = _mujocoFloatParams[simi_mujoco_global_overridesolref1 + int(j)];
            ar.xmlAddNode_floats("overridesolref", w, 2);
            for (size_t j = 0; j < 5; j++)
                w[j] = _mujocoFloatParams[simi_mujoco_global_overridesolimp1 + int(j)];
            ar.xmlAddNode_floats("overridesolimp", w, 5);
            ar.xmlAddNode_float("kinematicweldtorquescale", _mujocoFloatParams[simi_mujoco_global_kinematicweldtorquescale]);
            for (size_t j = 0; j < 2; j++)
                w[j] = _mujocoFloatParams[simi_mujoco_global_kinematicweldsolref1 + int(j)];
            ar.xmlAddNode_floats("kinematicweldsolref", w, 2);
            for (size_t j = 0; j < 5; j++)
                w[j] = _mujocoFloatParams[simi_mujoco_global_kinematicweldsolimp1 + int(j)];
            ar.xmlAddNode_floats("kinematicweldsolimp", w, 5);
            ar.xmlAddNode_float("kinmass", _mujocoFloatParams[simi_mujoco_global_kinmass]);
            ar.xmlAddNode_float("kininertia", _mujocoFloatParams[simi_mujoco_global_kininertia]);
            ar.xmlAddNode_float("tolerance", _mujocoFloatParams[simi_mujoco_global_tolerance]);
            ar.xmlAddNode_float("lsTolerance", _mujocoFloatParams[simi_mujoco_global_ls_tolerance]);
            ar.xmlAddNode_float("noslipTolerance", _mujocoFloatParams[simi_mujoco_global_noslip_tolerance]);
            ar.xmlAddNode_float("ccdTolerance", _mujocoFloatParams[simi_mujoco_global_ccd_tolerance]);
            ar.xmlAddNode_int("iterations", _mujocoIntParams[simi_mujoco_global_iterations]);
            ar.xmlAddNode_int("integrator", _mujocoIntParams[simi_mujoco_global_integrator]);
            ar.xmlAddNode_int("solver", _mujocoIntParams[simi_mujoco_global_solver]);
            ar.xmlAddNode_int("mbMemory", _mujocoIntParams[simi_mujoco_global_mbmemory]);
            // Following 3 deprecated:
            //ar.xmlAddNode_int("njmax", _mujocoIntParams[simi_mujoco_global_njmax]);
            //ar.xmlAddNode_int("nconmax", _mujocoIntParams[simi_mujoco_global_nconmax]);
            //ar.xmlAddNode_int("nstack", _mujocoIntParams[simi_mujoco_global_nstack]);
            ar.xmlAddNode_int("cone", _mujocoIntParams[simi_mujoco_global_cone]);
            ar.xmlAddNode_int("overridekin", _mujocoIntParams[simi_mujoco_global_overridekin]);
            ar.xmlAddNode_int("rebuildtrigger", _mujocoIntParams[simi_mujoco_global_rebuildtrigger]);
            ar.xmlAddNode_int("jacobian", _mujocoIntParams[simi_mujoco_global_jacobian]);
            ar.xmlAddNode_int("lsIterations", _mujocoIntParams[simi_mujoco_global_ls_iterations]);
            ar.xmlAddNode_int("noslipIterations", _mujocoIntParams[simi_mujoco_global_noslip_iterations]);
            ar.xmlAddNode_int("ccdIterations", _mujocoIntParams[simi_mujoco_global_ccd_iterations]);
            ar.xmlAddNode_int("sdfIterations", _mujocoIntParams[simi_mujoco_global_sdf_iterations]);
            ar.xmlAddNode_int("sdfInitpoints", _mujocoIntParams[simi_mujoco_global_sdf_initpoints]);
            ar.xmlAddNode_bool("computeinertias", getBoolPropertyValue(propDyn_mujocoComputeInertias.name));
            ar.xmlAddNode_bool("multithreaded", getBoolPropertyValue(propDyn_mujocoMultithreaded.name));
            ar.xmlAddNode_bool("multiccd", getBoolPropertyValue(propDyn_mujocoMulticcd.name));
            ar.xmlAddNode_bool("balanceinertias", getBoolPropertyValue(propDyn_mujocoBalanceInertias.name));
            ar.xmlAddNode_bool("overridecontacts", getBoolPropertyValue(propDyn_mujocoContactParamsOverride.name));
            ar.xmlAddNode_bool("equalityEnable", getBoolPropertyValue(propDyn_mujocoEqualityEnable.name));
            ar.xmlAddNode_bool("frictionlossEnable", getBoolPropertyValue(propDyn_mujocoFrictionlossEnable.name));
            ar.xmlAddNode_bool("limitEnable", getBoolPropertyValue(propDyn_mujocoLimitEnable.name));
            ar.xmlAddNode_bool("contactEnable", getBoolPropertyValue(propDyn_mujocoContactEnable.name));
            ar.xmlAddNode_bool("passiveEnable", getBoolPropertyValue(propDyn_mujocoPassiveEnable.name));
            ar.xmlAddNode_bool("gravityEnable", getBoolPropertyValue(propDyn_mujocoGravityEnable.name));
            ar.xmlAddNode_bool("warmstartEnable", getBoolPropertyValue(propDyn_mujocoWarmstartEnable.name));
            ar.xmlAddNode_bool("actuationEnable", getBoolPropertyValue(propDyn_mujocoActuationEnable.name));
            ar.xmlAddNode_bool("refsafeEnable", getBoolPropertyValue(propDyn_mujocoRefsafeEnable.name));
            ar.xmlAddNode_bool("sensorEnable", getBoolPropertyValue(propDyn_mujocoSensorEnable.name));
            ar.xmlAddNode_bool("midphaseEnable", getBoolPropertyValue(propDyn_mujocoMidphaseEnable.name));
            ar.xmlAddNode_bool("eulerdampEnable", getBoolPropertyValue(propDyn_mujocoEulerdampEnable.name));
            ar.xmlAddNode_bool("autoresetEnable", getBoolPropertyValue(propDyn_mujocoAutoresetEnable.name));
            ar.xmlAddNode_bool("energyEnable", getBoolPropertyValue(propDyn_mujocoEnergyEnable.name));
            ar.xmlAddNode_bool("invdiscreteEnable", getBoolPropertyValue(propDyn_mujocoInvdiscreteEnable.name));
            ar.xmlAddNode_bool("nativeccdEnable", getBoolPropertyValue(propDyn_mujocoNativeccdEnable.name));
            ar.xmlAddNode_bool("alignfree", getBoolPropertyValue(propDyn_mujocoAlignfree.name));
            ar.xmlPopNode();

            ar.xmlPopNode();
        }
        else
        {
            int oldDynamicsSettingsMode = 5; // i.e. custom
            _stepSize = 0.005;               // just in case

            ar.xmlGetNode_enum("engine", _dynamicEngineToUse, exhaustiveXml, "bullet", sim_physics_bullet, "ode",
                               sim_physics_ode, "vortex", sim_physics_vortex, "newton", sim_physics_newton, "mujoco",
                               sim_physics_mujoco);

            if (ar.xmlGetNode_int("engineVersion", _dynamicEngineVersionToUse, exhaustiveXml))
            {
                if ((_dynamicEngineVersionToUse != 0) && (_dynamicEngineVersionToUse != 283))
                    _dynamicEngineVersionToUse = 0;
            }

            bool hasStepSizeTag = ar.xmlGetNode_float("stepsize", _stepSize, exhaustiveXml);

            bool engMod = ar.xmlGetNode_enum("engineMode", oldDynamicsSettingsMode, false, "veryAccurate", 0,
                                             "accurate", 1, "fast", 2, "veryFast", 3, "customized", 4);
            if (engMod)
                oldDynamicsSettingsMode++;

            ar.xmlGetNode_enum("settingsMode", oldDynamicsSettingsMode, exhaustiveXml && (!engMod), "veryAccurate", 0,
                               "accurate", 1, "balanced", 2, "fast", 3, "veryFast", 4, "custom", 5);

            ar.xmlGetNode_floats("gravity", _gravity.data, 3, exhaustiveXml);

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("dynamicsEnabled", _dynamicsEnabled, exhaustiveXml);
                ar.xmlGetNode_bool("showContactPoints", _displayContactPoints, exhaustiveXml);
                ar.xmlPopNode();
            }

            double v;
            int vi;
            bool vb;
            if (ar.xmlPushChildNode("engines", exhaustiveXml))
            {
                if (ar.xmlPushChildNode("bullet", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("stepsize", v, exhaustiveXml))
                    {
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_bullet))
                            _stepSize = v;
                    }
                    if (ar.xmlGetNode_float("internalscalingfactor", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_global_internalscalingfactor] = v;
                    if (ar.xmlGetNode_float("collisionmarginfactor", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_global_collisionmarginfactor] = v;

                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        _bulletIntParams[simi_bullet_global_constraintsolvingiterations] = vi;
                    if (ar.xmlGetNode_int("constraintsolvertype", vi, exhaustiveXml))
                        _bulletIntParams[simi_bullet_global_constraintsolvertype] = vi;

                    if (ar.xmlGetNode_bool("fullinternalscaling", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_global_bitcoded] |= simi_bullet_global_fullinternalscaling;
                        if (!vb)
                            _bulletIntParams[simi_bullet_global_bitcoded] -= simi_bullet_global_fullinternalscaling;
                    }
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_global_bitcoded] |= simi_bullet_global_computeinertias;
                        if (!vb)
                            _bulletIntParams[simi_bullet_global_bitcoded] -= simi_bullet_global_computeinertias;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("ode"))
                {
                    if (ar.xmlGetNode_float("stepsize", v, exhaustiveXml))
                    {
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_ode))
                            _stepSize = v;
                    }
                    if (ar.xmlGetNode_float("internalscalingfactor", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_global_internalscalingfactor] = v;
                    if (ar.xmlGetNode_float("cfm", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_global_cfm] = v;
                    if (ar.xmlGetNode_float("erp", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_global_erp] = v;

                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        _odeIntParams[simi_ode_global_constraintsolvingiterations] = vi;
                    if (ar.xmlGetNode_int("randomseed", vi, exhaustiveXml))
                        _odeIntParams[simi_ode_global_randomseed] = vi;

                    if (ar.xmlGetNode_bool("fullinternalscaling", vb, exhaustiveXml))
                    {
                        _odeIntParams[simi_ode_global_bitcoded] |= simi_ode_global_fullinternalscaling;
                        if (!vb)
                            _odeIntParams[simi_ode_global_bitcoded] -= simi_ode_global_fullinternalscaling;
                    }
                    if (ar.xmlGetNode_bool("quickstep", vb, exhaustiveXml))
                    {
                        _odeIntParams[simi_ode_global_bitcoded] |= simi_ode_global_quickstep;
                        if (!vb)
                            _odeIntParams[simi_ode_global_bitcoded] -= simi_ode_global_quickstep;
                    }
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                    {
                        _odeIntParams[simi_ode_global_bitcoded] |= simi_ode_global_computeinertias;
                        if (!vb)
                            _odeIntParams[simi_ode_global_bitcoded] -= simi_ode_global_computeinertias;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("vortex"))
                {
                    if (ar.xmlGetNode_float("stepsize", v, exhaustiveXml))
                    {
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_vortex))
                            _stepSize = v;
                    }
                    if (ar.xmlGetNode_float("internalscalingfactor", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_internalscalingfactor] = v;
                    if (ar.xmlGetNode_float("contacttolerance", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_contacttolerance] = v;
                    if (ar.xmlGetNode_float("constraintlinearcompliance", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintlinearcompliance] = v;
                    if (ar.xmlGetNode_float("constraintlineardamping", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintlineardamping] = v;
                    if (ar.xmlGetNode_float("constraintlinearkineticloss", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintlinearkineticloss] = v;
                    if (ar.xmlGetNode_float("constraintangularcompliance", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintangularcompliance] = v;
                    if (ar.xmlGetNode_float("constraintangulardamping", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintangulardamping] = v;
                    if (ar.xmlGetNode_float("constraintangularkineticloss", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_global_constraintangularkineticloss] = v;

                    if (ar.xmlGetNode_bool("autosleep", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_global_bitcoded] |= simi_vortex_global_autosleep;
                        if (!vb)
                            _vortexIntParams[simi_vortex_global_bitcoded] -= simi_vortex_global_autosleep;
                    }
                    if (ar.xmlGetNode_bool("multithreading", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_global_bitcoded] |= simi_vortex_global_multithreading;
                        if (!vb)
                            _vortexIntParams[simi_vortex_global_bitcoded] -= simi_vortex_global_multithreading;
                    }
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_global_bitcoded] |= simi_vortex_global_computeinertias;
                        if (!vb)
                            _vortexIntParams[simi_vortex_global_bitcoded] -= simi_vortex_global_computeinertias;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("newton"))
                {
                    if (ar.xmlGetNode_float("stepsize", v, exhaustiveXml))
                    {
                        if ((!hasStepSizeTag) && (_dynamicEngineToUse == sim_physics_newton))
                            _stepSize = v;
                    }
                    if (ar.xmlGetNode_float("contactmergetolerance", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_global_contactmergetolerance] = v;
                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        _newtonIntParams[simi_newton_global_constraintsolvingiterations] = vi;
                    if (ar.xmlGetNode_bool("multithreading", vb, exhaustiveXml))
                    {
                        _newtonIntParams[simi_newton_global_bitcoded] |= simi_newton_global_multithreading;
                        if (!vb)
                            _newtonIntParams[simi_newton_global_bitcoded] -= simi_newton_global_multithreading;
                    }
                    if (ar.xmlGetNode_bool("exactsolver", vb, exhaustiveXml))
                    {
                        _newtonIntParams[simi_newton_global_bitcoded] |= simi_newton_global_exactsolver;
                        if (!vb)
                            _newtonIntParams[simi_newton_global_bitcoded] -= simi_newton_global_exactsolver;
                    }
                    if (ar.xmlGetNode_bool("highjointaccuracy", vb, exhaustiveXml))
                    {
                        _newtonIntParams[simi_newton_global_bitcoded] |= simi_newton_global_highjointaccuracy;
                        if (!vb)
                            _newtonIntParams[simi_newton_global_bitcoded] -= simi_newton_global_highjointaccuracy;
                    }
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                    {
                        _newtonIntParams[simi_newton_global_bitcoded] |= simi_newton_global_computeinertias;
                        if (!vb)
                            _newtonIntParams[simi_newton_global_bitcoded] -= simi_newton_global_computeinertias;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("mujoco"))
                {
                    if (ar.xmlGetNode_float("impratio", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_impratio] = v;
                    double w[5];
                    if (ar.xmlGetNode_floats("wind", w, 3, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 3; j++)
                            _mujocoFloatParams[simi_mujoco_global_wind1 + int(j)] = w[j];
                    }
                    if (ar.xmlGetNode_float("density", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_density] = v;
                    if (ar.xmlGetNode_float("viscosity", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_viscosity] = v;
                    if (ar.xmlGetNode_float("boundmass", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_boundmass] = v;
                    if (ar.xmlGetNode_float("boundinertia", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_boundinertia] = v;
                    if (ar.xmlGetNode_float("overridemargin", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_overridemargin] = v;
                    if (ar.xmlGetNode_floats("overridesolref", w, 2, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 2; j++)
                            _mujocoFloatParams[simi_mujoco_global_overridesolref1 + int(j)] = w[j];
                    }
                    if (ar.xmlGetNode_floats("overridesolimp", w, 5, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 5; j++)
                            _mujocoFloatParams[simi_mujoco_global_overridesolimp1 + int(j)] = w[j];
                    }
                    if (ar.xmlGetNode_float("kinematicweldtorquescale", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_kinematicweldtorquescale] = v;
                    if (ar.xmlGetNode_floats("kinematicweldsolref", w, 2, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 2; j++)
                            _mujocoFloatParams[simi_mujoco_global_kinematicweldsolref1 + int(j)] = w[j];
                    }
                    if (ar.xmlGetNode_floats("kinematicweldsolimp", w, 5, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 5; j++)
                            _mujocoFloatParams[simi_mujoco_global_kinematicweldsolimp1 + int(j)] = w[j];
                    }
                    if (ar.xmlGetNode_float("kinmass", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_kinmass] = v;
                    if (ar.xmlGetNode_float("kininertia", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_kininertia] = v;
                    if (ar.xmlGetNode_float("tolerance", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_tolerance] = v;
                    if (ar.xmlGetNode_float("lsTolerance", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_ls_tolerance] = v;
                    if (ar.xmlGetNode_float("noslipTolerance", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_noslip_tolerance] = v;
                    if (ar.xmlGetNode_float("ccdTolerance", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_global_ccd_tolerance] = v;
                    if (ar.xmlGetNode_int("iterations", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_iterations] = vi;
                    if (ar.xmlGetNode_int("integrator", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_integrator] = vi;
                    if (ar.xmlGetNode_int("solver", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_solver] = vi;
                    if (ar.xmlGetNode_int("mbMemory", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_mbmemory] = vi;
                    // Following 3 deprecated:
                    //if (ar.xmlGetNode_int("njmax", vi, exhaustiveXml))
                    //    _mujocoIntParams[simi_mujoco_global_njmax] = vi;
                    //if (ar.xmlGetNode_int("nconmax", vi, exhaustiveXml))
                    //    _mujocoIntParams[simi_mujoco_global_nconmax] = vi;
                    //if (ar.xmlGetNode_int("nstack", vi, exhaustiveXml))
                    //    _mujocoIntParams[simi_mujoco_global_nstack] = vi;
                    if (ar.xmlGetNode_int("cone", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_cone] = vi;
                    if (ar.xmlGetNode_int("overridekin", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_overridekin] = vi;
                    if (ar.xmlGetNode_int("rebuildtrigger", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_rebuildtrigger] = vi;
                    if (ar.xmlGetNode_int("jacobian", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_jacobian] = vi;
                    if (ar.xmlGetNode_int("lsIterations", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_ls_iterations] = vi;
                    if (ar.xmlGetNode_int("noslipIterations", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_noslip_iterations] = vi;
                    if (ar.xmlGetNode_int("ccdIterations", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_ccd_iterations] = vi;
                    if (ar.xmlGetNode_int("sdfIterations", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_sdf_iterations] = vi;
                    if (ar.xmlGetNode_int("sdfInitpoints", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_global_sdf_initpoints] = vi;
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_global_bitcoded] |= simi_mujoco_global_computeinertias;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_global_bitcoded] -= simi_mujoco_global_computeinertias;
                    }
                    if (ar.xmlGetNode_bool("multithreaded", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_global_bitcoded] |= simi_mujoco_global_multithreaded;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_global_bitcoded] -= simi_mujoco_global_multithreaded;
                    }
                    if (ar.xmlGetNode_bool("multiccd", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_global_bitcoded] |= simi_mujoco_global_multiccd;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_global_bitcoded] -= simi_mujoco_global_multiccd;
                    }
                    if (ar.xmlGetNode_bool("balanceinertias", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_global_bitcoded] |= simi_mujoco_global_balanceinertias;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_global_bitcoded] -= simi_mujoco_global_balanceinertias;
                    }
                    if (ar.xmlGetNode_bool("overridecontacts", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_global_bitcoded] |= simi_mujoco_global_overridecontacts;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_global_bitcoded] -= simi_mujoco_global_overridecontacts;
                    }
                    if (ar.xmlGetNode_bool("equalityEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoEqualityEnable.name, vb);
                    if (ar.xmlGetNode_bool("frictionlossEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoFrictionlossEnable.name, vb);
                    if (ar.xmlGetNode_bool("limitEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoLimitEnable.name, vb);
                    if (ar.xmlGetNode_bool("contactEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoContactEnable.name, vb);
                    if (ar.xmlGetNode_bool("passiveEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoPassiveEnable.name, vb);
                    if (ar.xmlGetNode_bool("gravityEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoGravityEnable.name, vb);
                    if (ar.xmlGetNode_bool("warmstartEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoWarmstartEnable.name, vb);
                    if (ar.xmlGetNode_bool("actuationEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoActuationEnable.name, vb);
                    if (ar.xmlGetNode_bool("refsafeEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoRefsafeEnable.name, vb);
                    if (ar.xmlGetNode_bool("sensorEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoSensorEnable.name, vb);
                    if (ar.xmlGetNode_bool("midphaseEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoMidphaseEnable.name, vb);
                    if (ar.xmlGetNode_bool("eulerdampEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoEulerdampEnable.name, vb);
                    if (ar.xmlGetNode_bool("autoresetEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoAutoresetEnable.name, vb);
                    if (ar.xmlGetNode_bool("energyEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoEnergyEnable.name, vb);
                    if (ar.xmlGetNode_bool("invdiscreteEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoInvdiscreteEnable.name, vb);
                    if (ar.xmlGetNode_bool("nativeccdEnable", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoNativeccdEnable.name, vb);
                    if (ar.xmlGetNode_bool("alignfree", vb, exhaustiveXml))
                        setBoolProperty(propDyn_mujocoAlignfree.name, vb);

                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            _fixVortexInfVals();
            if (!hasStepSizeTag)
            {
                if (oldDynamicsSettingsMode < 5)
                { // for backward compatibility. Previously this was one of the default settings (precise, balanced,
                    // fast, etc.)
                    getBulletDefaultFloatParams(_bulletFloatParams, oldDynamicsSettingsMode);
                    getBulletDefaultIntParams(_bulletIntParams, oldDynamicsSettingsMode);
                    getOdeDefaultFloatParams(_odeFloatParams, oldDynamicsSettingsMode);
                    getOdeDefaultIntParams(_odeIntParams, oldDynamicsSettingsMode);
                    getVortexDefaultFloatParams(_vortexFloatParams, oldDynamicsSettingsMode);
                    getVortexDefaultIntParams(_vortexIntParams, oldDynamicsSettingsMode);
                    getNewtonDefaultFloatParams(_newtonFloatParams, oldDynamicsSettingsMode);
                    getNewtonDefaultIntParams(_newtonIntParams, oldDynamicsSettingsMode);
                    getMujocoDefaultFloatParams(_mujocoFloatParams, oldDynamicsSettingsMode);
                    getMujocoDefaultIntParams(_mujocoIntParams, oldDynamicsSettingsMode);
                }
                _bulletFloatParams[simi_bullet_global_stepsize] = _stepSize;
                _odeFloatParams[simi_ode_global_stepsize] = _stepSize;
                _vortexFloatParams[simi_vortex_global_stepsize] = _stepSize;
                _newtonFloatParams[simi_newton_global_stepsize] = _stepSize;
                _mujocoFloatParams[simi_mujoco_global_stepsize] = _stepSize;
            }
            checkIfEngineSettingsAreDefault();
        }
    }
}

void CDynamicsContainer::_fixVortexInfVals()
{ // to fix a past complication (i.e. neg. val. of unsigned would be inf)
    for (size_t i = 3; i < 9; i++)
    { // only in this range unsigned or had the inf. probl.
        if (_vortexFloatParams[i] < 0.0)
            _vortexFloatParams[i] = DBL_MAX;
    }
}

void CDynamicsContainer::getBulletDefaultFloatParams(std::vector<double>& p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005); // simi_bullet_global_stepsize
    p.push_back(10.0);  // simi_bullet_global_internalscalingfactor
    p.push_back(0.1);   // simi_bullet_global_collisionmarginfactor
    p.push_back(0.0);   // free
    p.push_back(0.0);   // free
}

void CDynamicsContainer::getBulletDefaultIntParams(std::vector<int>& p, int defType /*=-1*/) const
{
    p.clear();
    if (defType == -1)
        p.push_back(100); // simi_bullet_global_constraintsolvingiterations
    else
    { // back compatibility
        int DYNAMIC_BULLET_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[5] = {500, 200, 100, 50, 20};
        p.push_back(DYNAMIC_BULLET_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]);
    }
    int v = 0;
    v |= simi_bullet_global_fullinternalscaling;
    // v|=simi_bullet_global_computeinertias; // off by default
    p.push_back(v);                                                 // simi_bullet_global_bitcoded
    p.push_back(sim_bullet_constraintsolvertype_sequentialimpulse); // simi_bullet_global_constraintsolvertype
}

void CDynamicsContainer::getOdeDefaultFloatParams(std::vector<double>& p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005);   // simi_bullet_global_stepsize
    p.push_back(1.0);     // simi_bullet_global_internalscalingfactor
    p.push_back(0.00001); // simi_bullet_global_cfm
    p.push_back(0.6);     // simi_bullet_global_erp
    p.push_back(0.0);     // free
}

void CDynamicsContainer::getOdeDefaultIntParams(std::vector<int>& p, int defType /*=-1*/) const
{
    p.clear();
    if (defType == -1)
        p.push_back(100); // simi_ode_global_constraintsolvingiterations
    else
    { // back compatibility
        int DYNAMIC_ODE_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[5] = {500, 200, 100, 50, 20};
        p.push_back(DYNAMIC_ODE_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]);
    }
    int v = 0;
    v |= simi_ode_global_fullinternalscaling;
    v |= simi_ode_global_quickstep;
    // v|=simi_ode_global_computeinertias; // off by default
    p.push_back(v);  // simi_ode_global_bitcoded
    p.push_back(-1); // simi_ode_global_randomseed
}

void CDynamicsContainer::getVortexDefaultFloatParams(std::vector<double>& p, int defType /*=-1*/) const
{
    p.clear();
    if (defType == -1)
        p.push_back(0.005); // simi_vortex_global_stepsize
    else
    { // back compatibility
        double DYNAMIC_VORTEX_DEFAULT_STEP_SIZE[5] = {0.001, 0.0025, 0.005, 0.01, 0.025};
        p.push_back(DYNAMIC_VORTEX_DEFAULT_STEP_SIZE[defType]);
    }
    p.push_back(1.0);     // simi_vortex_global_internalscalingfactor
    p.push_back(0.001);   // simi_vortex_global_contacttolerance
    p.push_back(1.0e-7f); // simi_vortex_global_constraintlinearcompliance
    p.push_back(8.0e+6);  // simi_vortex_global_constraintlineardamping
    p.push_back(6.0e-5f); // simi_vortex_global_constraintlinearkineticloss
    p.push_back(1.0e-9f); // simi_vortex_global_constraintangularcompliance
    p.push_back(8.0e+8);  // simi_vortex_global_constraintangulardamping
    p.push_back(6.0e-7f); // simi_vortex_global_constraintangularkineticloss
    p.push_back(0.01);    // RESERVED. used to be auto angular damping tension ratio, not used anymore
}

void CDynamicsContainer::getVortexDefaultIntParams(std::vector<int>& p, int defType /*=-1*/) const
{
    p.clear();
    int v = 0;
    v |= simi_vortex_global_autosleep;
    // v|=simi_vortex_global_multithreading; false by default
    v |= 4; // always on by default (full internal scaling)
    // bit4 (8) is RESERVED!! (was auto-angular damping)
    // v|=simi_vortex_global_computeinertias; // off by default
    p.push_back(v); // simi_vortex_global_bitcoded
}

void CDynamicsContainer::getNewtonDefaultFloatParams(std::vector<double>& p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005); // simi_newton_global_stepsize
    p.push_back(0.01);  // simi_newton_global_contactmergetolerance
}

void CDynamicsContainer::getNewtonDefaultIntParams(std::vector<int>& p, int defType /*=-1*/) const
{
    p.clear();
    if (defType == -1)
        p.push_back(8); // simi_newton_global_constraintsolvingiterations
    else
    { // back compatibility
        int DYNAMIC_NEWTON_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[5] = {24, 16, 8, 6, 4};
        p.push_back(DYNAMIC_NEWTON_DEFAULT_CONSTRAINT_SOLVING_ITERATIONS[defType]);
    }
    int options = 0;
    options |= simi_newton_global_multithreading;
    options |= simi_newton_global_exactsolver;
    options |= simi_newton_global_highjointaccuracy;
    // options|=simi_newton_global_computeinertias; // off by default
    p.push_back(options); // simi_newton_global_bitcoded
}

void CDynamicsContainer::getMujocoDefaultFloatParams(std::vector<double>& p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005);    // simi_mujoco_global_stepsize
    p.push_back(1.0);      // simi_mujoco_global_impratio
    p.push_back(0.0);      // simi_mujoco_global_wind1
    p.push_back(0.0);      // simi_mujoco_global_wind2
    p.push_back(0.0);      // simi_mujoco_global_wind3
    p.push_back(0.0);      // simi_mujoco_global_density
    p.push_back(0.0);      // simi_mujoco_global_viscosity
    p.push_back(0.0);      // simi_mujoco_global_boundmass
    p.push_back(0.000001); // simi_mujoco_global_boundinertia
    p.push_back(0.0);      // simi_mujoco_global_overridemargin
    p.push_back(0.02);     // simi_mujoco_global_overridesolref1
    p.push_back(1.0);      // simi_mujoco_global_overridesolref2
    p.push_back(0.9);      // simi_mujoco_global_overridesolimp1
    p.push_back(0.95);     // simi_mujoco_global_overridesolimp2
    p.push_back(0.001);    // simi_mujoco_global_overridesolimp3
    p.push_back(0.5);      // simi_mujoco_global_overridesolimp4
    p.push_back(2.0);      // simi_mujoco_global_overridesolimp5
    p.push_back(1000.0);   // simi_mujoco_global_kinmass
    p.push_back(1.0);      // simi_mujoco_global_kininertia
    p.push_back(1e-8);     // simi_mujoco_global_tolerance
    p.push_back(0.01);     // simi_mujoco_global_ls_tolerance
    p.push_back(1e-8);     // simi_mujoco_global_noslip_tolerance
    p.push_back(1e-6);     // simi_mujoco_global_ccd_tolerance
    p.push_back(0.02);     // simi_mujoco_global_kinematicweldsolref1
    p.push_back(1.0);      // simi_mujoco_global_kinematicweldsolref2
    p.push_back(0.9);      // simi_mujoco_global_kinematicweldsolimp1
    p.push_back(0.95);     // simi_mujoco_global_kinematicweldsolimp2
    p.push_back(0.001);    // simi_mujoco_global_kinematicweldsolimp3
    p.push_back(0.5);      // simi_mujoco_global_kinematicweldsolimp4
    p.push_back(2.0);      // simi_mujoco_global_kinematicweldsolimp5
    p.push_back(1.0);      // simi_mujoco_global_kinematicweldtorquescale
}

void CDynamicsContainer::getMujocoDefaultIntParams(std::vector<int>& p, int defType /*=-1*/) const
{
    p.clear();
    int options = 0;
    // options|=simi_mujoco_global_computeinertias; // false by default
    options |= simi_mujoco_global_multithreaded;
    // options|=simi_mujoco_global_multiccd; // false by default
    options |= simi_mujoco_global_balanceinertias;
    // options|=simi_mujoco_global_overridecontacts; // false by default
    p.push_back(options);                 // sim_mujoco_global_bitcoded
    p.push_back(100);                     // simi_mujoco_global_iterations
    p.push_back(3);                       // simi_mujoco_global_integrator, implicitfast (was Euler until 18.11.2024)
    p.push_back(2);                       // simi_mujoco_global_solver, Newton
    p.push_back(5000);                    // simi_mujoco_global_njmax (deprecated and not used anymore)
    p.push_back(2000);                    // simi_mujoco_global_nconmax (deprecated and not used anymore)
    p.push_back(0);                       // simi_mujoco_global_cone, pyramidal
    p.push_back(0);                       // simi_mujoco_global_overridekin, do not override
    p.push_back(-1);                      // simi_mujoco_global_nstack (deprecated and not used anymore)
    p.push_back(1 + 2 + 4 + 8 + 16 + 32); // simi_mujoco_global_rebuildtrigger
    p.push_back(-1);                      // simi_mujoco_global_mbmemory
    p.push_back(-1);                      // simi_mujoco_global_jacobian (auto)
    p.push_back(50);                      // simi_mujoco_global_ls_iterations
    p.push_back(5);                       // simi_mujoco_global_noslip_iterations
    p.push_back(50);                      // simi_mujoco_global_ccd_iterations
    p.push_back(10);                      // simi_mujoco_global_sdf_iterations
    p.push_back(40);                      // simi_mujoco_global_sdf_initpoints
    options = 0;
    options |= simi_mujoco_global_equality | simi_mujoco_global_frictionloss | simi_mujoco_global_limit | simi_mujoco_global_contact;
    options |= simi_mujoco_global_passive | simi_mujoco_global_gravity | simi_mujoco_global_warmstart | simi_mujoco_global_actuation;
    options |= simi_mujoco_global_refsafe | simi_mujoco_global_sensor | simi_mujoco_global_midphase | simi_mujoco_global_eulerdamp;
    // simi_mujoco_global_autoreset, simi_mujoco_global_energy, simi_mujoco_global_invdiscrete, simi_mujoco_global_nativeccd, simi_mujoco_global_alignfree are off by default
    p.push_back(options); // sim_mujoco_global_bitcoded2
}

#ifdef SIM_WITH_GUI
void CDynamicsContainer::renderYour3DStuff(CViewableBase* renderingObject, int displayAttrib)
{ // Has to be displayed as overlay!
    if (isWorldThere())
    {
        if ((displayAttrib & sim_displayattribute_noparticles) == 0)
        {
            int index = 0;
            float* cols;
            int objectType;
            int particlesCount;
            C4X4Matrix m(renderingObject->getFullCumulativeTransformation().getMatrix());
            void** particlesPointer =
                App::worldContainer->pluginContainer->dyn_getParticles(index++, &particlesCount, &objectType, &cols);
            while (particlesCount != -1)
            {
                if ((particlesPointer != nullptr) && (particlesCount > 0) &&
                    ((objectType & sim_particle_invisible) == 0))
                {
                    if (((displayAttrib & sim_displayattribute_forvisionsensor) == 0) ||
                        (objectType & sim_particle_painttag))
                        displayParticles(particlesPointer, particlesCount, displayAttrib, m, cols, objectType);
                }
                particlesPointer = App::worldContainer->pluginContainer->dyn_getParticles(index++, &particlesCount,
                                                                                          &objectType, &cols);
            }
        }
    }
}

void CDynamicsContainer::renderYour3DStuff_overlay(CViewableBase* renderingObject, int displayAttrib)
{ // Has to be displayed as overlay!
    if (isWorldThere())
    {
        if ((displayAttrib & sim_displayattribute_noparticles) == 0)
        {
            if ((displayAttrib & sim_displayattribute_renderpass) &&
                ((displayAttrib & sim_displayattribute_forvisionsensor) == 0))
            {
                if (getDisplayContactPoints())
                {
                    int cnt = 0;
                    double* pts = App::worldContainer->pluginContainer->dyn_getContactPoints(&cnt);

                    displayContactPoints(displayAttrib, contactPointColor, pts, cnt);
                }
            }
        }
    }
}
#endif

void CDynamicsContainer::appendGenesisData(CCbor* ev)
{
    ev->appendKeyBool(propDyn_dynamicsEnabled.name, _dynamicsEnabled);
    ev->appendKeyBool(propDyn_showContactPoints.name, _displayContactPoints);
    int ar[2] = {_dynamicEngineToUse, _dynamicEngineVersionToUse};
    ev->appendKeyIntArray(propDyn_dynamicsEngine.name, ar, 2);
    ev->appendKeyDouble(propDyn_dynamicsStepSize.name, _stepSize);
    ev->appendKeyDoubleArray(propDyn_gravity.name, _gravity.data, 3);

    // Engine properties:
    setBoolProperty(nullptr, false, ev);
    setIntProperty(nullptr, 0, ev);
    setFloatProperty(nullptr, 0.0, ev);
    setIntArray2Property(nullptr, nullptr, ev);
    setVector2Property(nullptr, nullptr, ev);
    setVector3Property(nullptr, nullptr, ev);
    setFloatArrayProperty(nullptr, nullptr, 0, ev);
    _sendEngineString(ev);
}

int CDynamicsContainer::setBoolProperty(const char* pName, bool pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        if (strcmp(pName, propDyn_dynamicsEnabled.name) == 0)
        {
            retVal = 1;
            setDynamicsEnabled(pState);
        }
        else if (strcmp(pName, propDyn_showContactPoints.name) == 0)
        {
            retVal = 1;
            setDisplayContactPoints(pState);
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndexBitCoded, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                int nv = (arr[simiIndexBitCoded] | simiIndex) - (1 - pState) * simiIndex;
                if ((nv != arr[simiIndexBitCoded]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndexBitCoded] = nv;
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyBool(propertyName.c_str(), arr[simiIndexBitCoded] & simiIndex);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propDyn_bulletComputeInertias.name, _bulletIntParams, simi_bullet_global_bitcoded, simi_bullet_global_computeinertias);
        handleProp(propDyn_bulletInternalScalingFull.name, _bulletIntParams, simi_bullet_global_bitcoded, simi_bullet_global_fullinternalscaling);
        handleProp(propDyn_odeQuickStepEnabled.name, _odeIntParams, simi_ode_global_bitcoded, simi_ode_global_quickstep);
        handleProp(propDyn_odeComputeInertias.name, _odeIntParams, simi_ode_global_bitcoded, simi_ode_global_computeinertias);
        handleProp(propDyn_odeInternalScalingFull.name, _odeIntParams, simi_ode_global_bitcoded, simi_ode_global_fullinternalscaling);
        handleProp(propDyn_vortexComputeInertias.name, _vortexIntParams, simi_vortex_global_bitcoded, simi_vortex_global_computeinertias);
        handleProp(propDyn_vortexAutoSleep.name, _vortexIntParams, simi_vortex_global_bitcoded, simi_vortex_global_autosleep);
        handleProp(propDyn_vortexMultithreading.name, _vortexIntParams, simi_vortex_global_bitcoded, simi_vortex_global_multithreading);
        handleProp(propDyn_newtonComputeInertias.name, _newtonIntParams, simi_newton_global_bitcoded, simi_newton_global_computeinertias);
        handleProp(propDyn_newtonMultithreading.name, _newtonIntParams, simi_newton_global_bitcoded, simi_newton_global_multithreading);
        handleProp(propDyn_newtonExactSolver.name, _newtonIntParams, simi_newton_global_bitcoded, simi_newton_global_exactsolver);
        handleProp(propDyn_newtonHighJointAccuracy.name, _newtonIntParams, simi_newton_global_bitcoded, simi_newton_global_highjointaccuracy);
        handleProp(propDyn_mujocoComputeInertias.name, _mujocoIntParams, simi_mujoco_global_bitcoded, simi_mujoco_global_computeinertias);
        handleProp(propDyn_mujocoBalanceInertias.name, _mujocoIntParams, simi_mujoco_global_bitcoded, simi_mujoco_global_balanceinertias);
        handleProp(propDyn_mujocoMultithreaded.name, _mujocoIntParams, simi_mujoco_global_bitcoded, simi_mujoco_global_multithreaded);
        handleProp(propDyn_mujocoMulticcd.name, _mujocoIntParams, simi_mujoco_global_bitcoded, simi_mujoco_global_multiccd);
        handleProp(propDyn_mujocoContactParamsOverride.name, _mujocoIntParams, simi_mujoco_global_bitcoded, simi_mujoco_global_overridecontacts);
        handleProp(propDyn_mujocoEqualityEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_equality);
        handleProp(propDyn_mujocoFrictionlossEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_frictionloss);
        handleProp(propDyn_mujocoLimitEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_limit);
        handleProp(propDyn_mujocoContactEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_contact);
        handleProp(propDyn_mujocoPassiveEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_passive);
        handleProp(propDyn_mujocoGravityEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_gravity);
        handleProp(propDyn_mujocoWarmstartEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_warmstart);
        handleProp(propDyn_mujocoActuationEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_actuation);
        handleProp(propDyn_mujocoRefsafeEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_refsafe);
        handleProp(propDyn_mujocoSensorEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_sensor);
        handleProp(propDyn_mujocoMidphaseEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_midphase);
        handleProp(propDyn_mujocoEulerdampEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_eulerdamp);
        handleProp(propDyn_mujocoAutoresetEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_autoreset);
        handleProp(propDyn_mujocoEnergyEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_energy);
        handleProp(propDyn_mujocoInvdiscreteEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_invdiscrete);
        handleProp(propDyn_mujocoNativeccdEnable.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_nativeccd);
        handleProp(propDyn_mujocoAlignfree.name, _mujocoIntParams, simi_mujoco_global_bitcoded2, simi_mujoco_global_alignfree);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getBoolProperty(const char* pName, bool& pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    // First non-engine properties:
    if (strcmp(pName, propDyn_dynamicsEnabled.name) == 0)
    {
        pState = _dynamicsEnabled;
        retVal = 1;
    }
    else if (strcmp(pName, propDyn_showContactPoints.name) == 0)
    {
        pState = _displayContactPoints;
        retVal = 1;
    }

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const int* bulletIntParams = _bulletIntParams.data();
        const int* odeIntParams = _odeIntParams.data();
        const int* vortexIntParams = _vortexIntParams.data();
        const int* newtonIntParams = _newtonIntParams.data();
        const int* mujocoIntParams = _mujocoIntParams.data();
        std::vector<int> __bulletIntParams;
        std::vector<int> __odeIntParams;
        std::vector<int> __vortexIntParams;
        std::vector<int> __newtonIntParams;
        std::vector<int> __mujocoIntParams;
        if (getDefaultValue)
        {
            getBulletDefaultIntParams(__bulletIntParams);
            getOdeDefaultIntParams(__odeIntParams);
            getVortexDefaultIntParams(__vortexIntParams);
            getNewtonDefaultIntParams(__newtonIntParams);
            getMujocoDefaultIntParams(__mujocoIntParams);
            bulletIntParams = __bulletIntParams.data();
            odeIntParams = __odeIntParams.data();
            vortexIntParams = __vortexIntParams.data();
            newtonIntParams = __newtonIntParams.data();
            mujocoIntParams = __mujocoIntParams.data();
        }
        if (strcmp(pName, propDyn_bulletComputeInertias.name) == 0)
        {
            retVal = 1;
            pState = bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_computeinertias;
        }
        else if (strcmp(pName, propDyn_bulletInternalScalingFull.name) == 0)
        {
            retVal = 1;
            pState = bulletIntParams[simi_bullet_global_bitcoded] & simi_bullet_global_fullinternalscaling;
        }
        else if (strcmp(pName, propDyn_odeQuickStepEnabled.name) == 0)
        {
            retVal = 1;
            pState = odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_quickstep;
        }
        else if (strcmp(pName, propDyn_odeComputeInertias.name) == 0)
        {
            retVal = 1;
            pState = odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_computeinertias;
        }
        else if (strcmp(pName, propDyn_odeInternalScalingFull.name) == 0)
        {
            retVal = 1;
            pState = odeIntParams[simi_ode_global_bitcoded] & simi_ode_global_fullinternalscaling;
        }
        else if (strcmp(pName, propDyn_vortexComputeInertias.name) == 0)
        {
            retVal = 1;
            pState = vortexIntParams[simi_vortex_global_bitcoded] & simi_vortex_global_computeinertias;
        }
        else if (strcmp(pName, propDyn_vortexAutoSleep.name) == 0)
        {
            retVal = 1;
            pState = vortexIntParams[simi_vortex_global_bitcoded] & simi_vortex_global_autosleep;
        }
        else if (strcmp(pName, propDyn_vortexMultithreading.name) == 0)
        {
            retVal = 1;
            pState = vortexIntParams[simi_vortex_global_bitcoded] & simi_vortex_global_multithreading;
        }
        else if (strcmp(pName, propDyn_newtonComputeInertias.name) == 0)
        {
            retVal = 1;
            pState = newtonIntParams[simi_newton_global_bitcoded] & simi_newton_global_computeinertias;
        }
        else if (strcmp(pName, propDyn_newtonMultithreading.name) == 0)
        {
            retVal = 1;
            pState = newtonIntParams[simi_newton_global_bitcoded] & simi_newton_global_multithreading;
        }
        else if (strcmp(pName, propDyn_newtonExactSolver.name) == 0)
        {
            retVal = 1;
            pState = newtonIntParams[simi_newton_global_bitcoded] & simi_newton_global_exactsolver;
        }
        else if (strcmp(pName, propDyn_newtonHighJointAccuracy.name) == 0)
        {
            retVal = 1;
            pState = newtonIntParams[simi_newton_global_bitcoded] & simi_newton_global_highjointaccuracy;
        }
        else if (strcmp(pName, propDyn_mujocoComputeInertias.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded] & simi_mujoco_global_computeinertias;
        }
        else if (strcmp(pName, propDyn_mujocoBalanceInertias.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded] & simi_mujoco_global_balanceinertias;
        }
        else if (strcmp(pName, propDyn_mujocoMultithreaded.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded] & simi_mujoco_global_multithreaded;
        }
        else if (strcmp(pName, propDyn_mujocoMulticcd.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded] & simi_mujoco_global_multiccd;
        }
        else if (strcmp(pName, propDyn_mujocoContactParamsOverride.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded] & simi_mujoco_global_overridecontacts;
        }
        else if (strcmp(pName, propDyn_mujocoEqualityEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_equality;
        }
        else if (strcmp(pName, propDyn_mujocoFrictionlossEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_frictionloss;
        }
        else if (strcmp(pName, propDyn_mujocoLimitEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_limit;
        }
        else if (strcmp(pName, propDyn_mujocoContactEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_contact;
        }
        else if (strcmp(pName, propDyn_mujocoPassiveEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_passive;
        }
        else if (strcmp(pName, propDyn_mujocoGravityEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_gravity;
        }
        else if (strcmp(pName, propDyn_mujocoWarmstartEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_warmstart;
        }
        else if (strcmp(pName, propDyn_mujocoActuationEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_actuation;
        }
        else if (strcmp(pName, propDyn_mujocoRefsafeEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_refsafe;
        }
        else if (strcmp(pName, propDyn_mujocoSensorEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_sensor;
        }
        else if (strcmp(pName, propDyn_mujocoMidphaseEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_midphase;
        }
        else if (strcmp(pName, propDyn_mujocoEulerdampEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_eulerdamp;
        }
        else if (strcmp(pName, propDyn_mujocoAutoresetEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_autoreset;
        }
        else if (strcmp(pName, propDyn_mujocoEnergyEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_energy;
        }
        else if (strcmp(pName, propDyn_mujocoInvdiscreteEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_invdiscrete;
        }
        else if (strcmp(pName, propDyn_mujocoNativeccdEnable.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_nativeccd;
        }
        else if (strcmp(pName, propDyn_mujocoAlignfree.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_bitcoded2] & simi_mujoco_global_alignfree;
        }
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setIntProperty(const char* pName, int pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                if ((pState != arr[simiIndex]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndex] = pState;
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyInt(propertyName.c_str(), arr[simiIndex]);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propDyn_bulletSolver.name, _bulletIntParams, simi_bullet_global_constraintsolvertype);
        handleProp(propDyn_bulletIterations.name, _bulletIntParams, simi_bullet_global_constraintsolvingiterations);
        handleProp(propDyn_odeQuickStepIterations.name, _odeIntParams, simi_ode_global_constraintsolvingiterations);
        handleProp(propDyn_newtonIterations.name, _newtonIntParams, simi_newton_global_constraintsolvingiterations);
        handleProp(propDyn_mujocoIntegrator.name, _mujocoIntParams, simi_mujoco_global_integrator);
        handleProp(propDyn_mujocoSolver.name, _mujocoIntParams, simi_mujoco_global_solver);
        handleProp(propDyn_mujocoIterations.name, _mujocoIntParams, simi_mujoco_global_iterations);
        handleProp(propDyn_mujocoRebuildTrigger.name, _mujocoIntParams, simi_mujoco_global_rebuildtrigger);
        handleProp(propDyn_mujocoNjMax.name, _mujocoIntParams, simi_mujoco_global_njmax);     // deprecated
        handleProp(propDyn_mujocoNconMax.name, _mujocoIntParams, simi_mujoco_global_nconmax); // deprecated
        handleProp(propDyn_mujocoNstack.name, _mujocoIntParams, simi_mujoco_global_nstack);   // deprecated
        handleProp(propDyn_mujocoCone.name, _mujocoIntParams, simi_mujoco_global_cone);
        handleProp(propDyn_mujocoKinematicBodiesOverrideFlags.name, _mujocoIntParams, simi_mujoco_global_overridekin);
        handleProp(propDyn_mujocoMbMemory.name, _mujocoIntParams, simi_mujoco_global_mbmemory);
        handleProp(propDyn_mujocoJacobian.name, _mujocoIntParams, simi_mujoco_global_jacobian);
        handleProp(propDyn_mujocoLs_iterations.name, _mujocoIntParams, simi_mujoco_global_ls_iterations);
        handleProp(propDyn_mujocoNoslip_iterations.name, _mujocoIntParams, simi_mujoco_global_noslip_iterations);
        handleProp(propDyn_mujocoCcd_iterations.name, _mujocoIntParams, simi_mujoco_global_ccd_iterations);
        handleProp(propDyn_mujocoSdf_iterations.name, _mujocoIntParams, simi_mujoco_global_sdf_iterations);
        handleProp(propDyn_mujocoSdf_initpoints.name, _mujocoIntParams, simi_mujoco_global_sdf_initpoints);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getIntProperty(const char* pName, int& pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    // First non-engine properties:
    /*
    if (strcmp(pName, propJoint_length.name) == 0)
    {
        pState = _length;
        retVal = 1;
    }
    */

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const int* bulletIntParams = _bulletIntParams.data();
        const int* odeIntParams = _odeIntParams.data();
        const int* vortexIntParams = _vortexIntParams.data();
        const int* newtonIntParams = _newtonIntParams.data();
        const int* mujocoIntParams = _mujocoIntParams.data();
        std::vector<int> __bulletIntParams;
        std::vector<int> __odeIntParams;
        std::vector<int> __vortexIntParams;
        std::vector<int> __newtonIntParams;
        std::vector<int> __mujocoIntParams;
        if (getDefaultValue)
        {
            getBulletDefaultIntParams(__bulletIntParams);
            getOdeDefaultIntParams(__odeIntParams);
            getVortexDefaultIntParams(__vortexIntParams);
            getNewtonDefaultIntParams(__newtonIntParams);
            getMujocoDefaultIntParams(__mujocoIntParams);
            bulletIntParams = __bulletIntParams.data();
            odeIntParams = __odeIntParams.data();
            vortexIntParams = __vortexIntParams.data();
            newtonIntParams = __newtonIntParams.data();
            mujocoIntParams = __mujocoIntParams.data();
        }
        if (strcmp(pName, propDyn_bulletSolver.name) == 0)
        {
            retVal = 1;
            pState = bulletIntParams[simi_bullet_global_constraintsolvertype];
        }
        else if (strcmp(pName, propDyn_bulletIterations.name) == 0)
        {
            retVal = 1;
            pState = bulletIntParams[simi_bullet_global_constraintsolvingiterations];
        }
        else if (strcmp(pName, propDyn_odeQuickStepIterations.name) == 0)
        {
            retVal = 1;
            pState = odeIntParams[simi_ode_global_constraintsolvingiterations];
        }
        else if (strcmp(pName, propDyn_newtonIterations.name) == 0)
        {
            retVal = 1;
            pState = newtonIntParams[simi_newton_global_constraintsolvingiterations];
        }
        else if (strcmp(pName, propDyn_mujocoIntegrator.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_integrator];
        }
        else if (strcmp(pName, propDyn_mujocoSolver.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_solver];
        }
        else if (strcmp(pName, propDyn_mujocoIterations.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_iterations];
        }
        else if (strcmp(pName, propDyn_mujocoRebuildTrigger.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_rebuildtrigger];
        }
        else if (strcmp(pName, propDyn_mujocoNjMax.name) == 0)
        {
            retVal = 1;
            pState = -1; // deprecated
        }
        else if (strcmp(pName, propDyn_mujocoNconMax.name) == 0)
        {
            retVal = 1;
            pState = -1; // deprecated
        }
        else if (strcmp(pName, propDyn_mujocoNstack.name) == 0)
        {
            retVal = 1;
            pState = -1; // deprecated
        }
        else if (strcmp(pName, propDyn_mujocoCone.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_cone];
        }
        else if (strcmp(pName, propDyn_mujocoKinematicBodiesOverrideFlags.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_overridekin];
        }
        else if (strcmp(pName, propDyn_mujocoMbMemory.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_mbmemory];
        }
        else if (strcmp(pName, propDyn_mujocoJacobian.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_jacobian];
        }
        else if (strcmp(pName, propDyn_mujocoLs_iterations.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_ls_iterations];
        }
        else if (strcmp(pName, propDyn_mujocoNoslip_iterations.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_noslip_iterations];
        }
        else if (strcmp(pName, propDyn_mujocoCcd_iterations.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_ccd_iterations];
        }
        else if (strcmp(pName, propDyn_mujocoSdf_iterations.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_sdf_iterations];
        }
        else if (strcmp(pName, propDyn_mujocoSdf_initpoints.name) == 0)
        {
            retVal = 1;
            pState = mujocoIntParams[simi_mujoco_global_sdf_initpoints];
        }
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setFloatProperty(const char* pName, double pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        if (strcmp(pName, propDyn_dynamicsStepSize.name) == 0)
        {
            setDesiredStepSize(pState);
            retVal = 1;
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                if ((pState != arr[simiIndex]) || (pName == nullptr))
                {
                    if (pName != nullptr)
                        arr[simiIndex] = pState;
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyDouble(propertyName.c_str(), arr[simiIndex]);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propDyn_bulletInternalScalingScaling.name, _bulletFloatParams, simi_bullet_global_internalscalingfactor);
        handleProp(propDyn_bulletCollMarginScaling.name, _bulletFloatParams, simi_bullet_global_collisionmarginfactor);
        handleProp(propDyn_odeInternalScalingScaling.name, _odeFloatParams, simi_ode_global_internalscalingfactor);
        handleProp(propDyn_odeGlobalErp.name, _odeFloatParams, simi_ode_global_erp);
        handleProp(propDyn_odeGlobalCfm.name, _odeFloatParams, simi_ode_global_cfm);
        handleProp(propDyn_vortexContactTolerance.name, _vortexFloatParams, simi_vortex_global_contacttolerance);
        handleProp(propDyn_vortexConstraintsLinearCompliance.name, _vortexFloatParams, simi_vortex_global_constraintlinearcompliance);
        handleProp(propDyn_vortexConstraintsLinearDamping.name, _vortexFloatParams, simi_vortex_global_constraintlineardamping);
        handleProp(propDyn_vortexConstraintsLinearKineticLoss.name, _vortexFloatParams, simi_vortex_global_constraintlinearkineticloss);
        handleProp(propDyn_vortexConstraintsAngularCompliance.name, _vortexFloatParams, simi_vortex_global_constraintangularcompliance);
        handleProp(propDyn_vortexConstraintsAngularDamping.name, _vortexFloatParams, simi_vortex_global_constraintangulardamping);
        handleProp(propDyn_vortexConstraintsAngularKineticLoss.name, _vortexFloatParams, simi_vortex_global_constraintangularkineticloss);
        handleProp(propDyn_newtonContactMergeTolerance.name, _newtonFloatParams, simi_newton_global_contactmergetolerance);
        handleProp(propDyn_mujocoKinematicBodiesMass.name, _mujocoFloatParams, simi_mujoco_global_kinmass);
        handleProp(propDyn_mujocoKinematicBodiesInertia.name, _mujocoFloatParams, simi_mujoco_global_kininertia);
        handleProp(propDyn_mujocoBoundMass.name, _mujocoFloatParams, simi_mujoco_global_boundmass);
        handleProp(propDyn_mujocoBoundInertia.name, _mujocoFloatParams, simi_mujoco_global_boundinertia);
        handleProp(propDyn_mujocoContactParamsMargin.name, _mujocoFloatParams, simi_mujoco_global_overridemargin);
        handleProp(propDyn_mujocoImpRatio.name, _mujocoFloatParams, simi_mujoco_global_impratio);
        handleProp(propDyn_mujocoDensity.name, _mujocoFloatParams, simi_mujoco_global_density);
        handleProp(propDyn_mujocoViscosity.name, _mujocoFloatParams, simi_mujoco_global_viscosity);
        handleProp(propDyn_mujocoTolerance.name, _mujocoFloatParams, simi_mujoco_global_tolerance);
        handleProp(propDyn_mujocoLs_tolerance.name, _mujocoFloatParams, simi_mujoco_global_ls_tolerance);
        handleProp(propDyn_mujocoNoslip_tolerance.name, _mujocoFloatParams, simi_mujoco_global_noslip_tolerance);
        handleProp(propDyn_mujocoCcd_tolerance.name, _mujocoFloatParams, simi_mujoco_global_ccd_tolerance);
        handleProp(propDyn_mujocoKinematicWeldTorqueScale.name, _mujocoFloatParams, simi_mujoco_global_kinematicweldtorquescale);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getFloatProperty(const char* pName, double& pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;

    // First non-engine properties:
    if (strcmp(pName, propDyn_dynamicsStepSize.name) == 0)
    {
        pState = _stepSize;
        retVal = 1;
    }

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const double* bulletFloatParams = _bulletFloatParams.data();
        const double* odeFloatParams = _odeFloatParams.data();
        const double* vortexFloatParams = _vortexFloatParams.data();
        const double* newtonFloatParams = _newtonFloatParams.data();
        const double* mujocoFloatParams = _mujocoFloatParams.data();
        std::vector<double> __bulletFloatParams;
        std::vector<double> __odeFloatParams;
        std::vector<double> __vortexFloatParams;
        std::vector<double> __newtonFloatParams;
        std::vector<double> __mujocoFloatParams;
        if (getDefaultValue)
        {
            getBulletDefaultFloatParams(__bulletFloatParams);
            getOdeDefaultFloatParams(__odeFloatParams);
            getVortexDefaultFloatParams(__vortexFloatParams);
            getNewtonDefaultFloatParams(__newtonFloatParams);
            getMujocoDefaultFloatParams(__mujocoFloatParams);
            bulletFloatParams = __bulletFloatParams.data();
            odeFloatParams = __odeFloatParams.data();
            vortexFloatParams = __vortexFloatParams.data();
            newtonFloatParams = __newtonFloatParams.data();
            mujocoFloatParams = __mujocoFloatParams.data();
        }
        if (strcmp(pName, propDyn_bulletInternalScalingScaling.name) == 0)
        {
            retVal = 1;
            pState = bulletFloatParams[simi_bullet_global_internalscalingfactor];
        }
        else if (strcmp(pName, propDyn_bulletCollMarginScaling.name) == 0)
        {
            retVal = 1;
            pState = bulletFloatParams[simi_bullet_global_collisionmarginfactor];
        }
        else if (strcmp(pName, propDyn_odeInternalScalingScaling.name) == 0)
        {
            retVal = 1;
            pState = odeFloatParams[simi_ode_global_internalscalingfactor];
        }
        else if (strcmp(pName, propDyn_odeGlobalErp.name) == 0)
        {
            retVal = 1;
            pState = odeFloatParams[simi_ode_global_erp];
        }
        else if (strcmp(pName, propDyn_odeGlobalCfm.name) == 0)
        {
            retVal = 1;
            pState = odeFloatParams[simi_ode_global_cfm];
        }
        else if (strcmp(pName, propDyn_vortexContactTolerance.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_contacttolerance];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsLinearCompliance.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintlinearcompliance];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsLinearDamping.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintlineardamping];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsLinearKineticLoss.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintlinearkineticloss];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsAngularCompliance.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintangularcompliance];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsAngularDamping.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintangulardamping];
        }
        else if (strcmp(pName, propDyn_vortexConstraintsAngularKineticLoss.name) == 0)
        {
            retVal = 1;
            pState = vortexFloatParams[simi_vortex_global_constraintangularkineticloss];
        }
        else if (strcmp(pName, propDyn_newtonContactMergeTolerance.name) == 0)
        {
            retVal = 1;
            pState = newtonFloatParams[simi_newton_global_contactmergetolerance];
        }
        else if (strcmp(pName, propDyn_mujocoKinematicBodiesMass.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_kinmass];
        }
        else if (strcmp(pName, propDyn_mujocoKinematicBodiesInertia.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_kininertia];
        }
        else if (strcmp(pName, propDyn_mujocoBoundMass.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_boundmass];
        }
        else if (strcmp(pName, propDyn_mujocoBoundInertia.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_boundinertia];
        }
        else if (strcmp(pName, propDyn_mujocoContactParamsMargin.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_overridemargin];
        }
        else if (strcmp(pName, propDyn_mujocoImpRatio.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_impratio];
        }
        else if (strcmp(pName, propDyn_mujocoDensity.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_density];
        }
        else if (strcmp(pName, propDyn_mujocoViscosity.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_viscosity];
        }
        else if (strcmp(pName, propDyn_mujocoTolerance.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_tolerance];
        }
        else if (strcmp(pName, propDyn_mujocoLs_tolerance.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_ls_tolerance];
        }
        else if (strcmp(pName, propDyn_mujocoNoslip_tolerance.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_noslip_tolerance];
        }
        else if (strcmp(pName, propDyn_mujocoCcd_tolerance.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_ccd_tolerance];
        }
        else if (strcmp(pName, propDyn_mujocoKinematicWeldTorqueScale.name) == 0)
        {
            retVal = 1;
            pState = mujocoFloatParams[simi_mujoco_global_kinematicweldtorquescale];
        }
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setStringProperty(const char* pName, const char* pState)
{
    int retVal = -1;
    if (strcmp(pName, propDyn_engineProperties.name) == 0)
    {
        retVal = 0;
        CEngineProperties prop;
        std::string current(prop.getObjectProperties(-1));
        if (prop.setObjectProperties(-1, pState))
        {
            retVal = 1;
            std::string current2(prop.getObjectProperties(-1));
            if (current != current2)
                _sendEngineString();
        }
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = -1;
    if (strcmp(pName, propDyn_engineProperties.name) == 0)
    {
        retVal = 1;
        CEngineProperties prop;
        pState = prop.getObjectProperties(-1);
    }

    return retVal;
}

int CDynamicsContainer::setIntArray2Property(const char* pName, const int* pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getIntArray2Property(const char* pName, int* pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    // First non-engine properties:

    // Engine-only properties:
    // ------------------------
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setVector2Property(const char* pName, const double* pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                if (pState != nullptr)
                {
                    for (size_t i = 0; i < 2; i++)
                        pa = pa || ((pState != nullptr) && (arr[simiIndex1 + i] != pState[i]));
                }
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < 2; i++)
                            arr[simiIndex1 + i] = pState[i];
                    }
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 2);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        //        handleProp(propDyn_mujocoContactParamsSolref.name, _mujocoFloatParams, simi_mujoco_global_overridesolref1);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getVector2Property(const char* pName, double* pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    // First non-engine properties:

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const double* bulletFloatParams = _bulletFloatParams.data();
        const double* odeFloatParams = _odeFloatParams.data();
        const double* vortexFloatParams = _vortexFloatParams.data();
        const double* newtonFloatParams = _newtonFloatParams.data();
        const double* mujocoFloatParams = _mujocoFloatParams.data();
        std::vector<double> __bulletFloatParams;
        std::vector<double> __odeFloatParams;
        std::vector<double> __vortexFloatParams;
        std::vector<double> __newtonFloatParams;
        std::vector<double> __mujocoFloatParams;
        if (getDefaultValue)
        {
            getBulletDefaultFloatParams(__bulletFloatParams);
            getOdeDefaultFloatParams(__odeFloatParams);
            getVortexDefaultFloatParams(__vortexFloatParams);
            getNewtonDefaultFloatParams(__newtonFloatParams);
            getMujocoDefaultFloatParams(__mujocoFloatParams);
            bulletFloatParams = __bulletFloatParams.data();
            odeFloatParams = __odeFloatParams.data();
            vortexFloatParams = __vortexFloatParams.data();
            newtonFloatParams = __newtonFloatParams.data();
            mujocoFloatParams = __mujocoFloatParams.data();
        }
        auto handleProp = [&](const double* arr, int simiIndex1) {
            retVal = 1;
            for (size_t i = 0; i < 2; i++)
                pState[i] = arr[simiIndex1 + i];
        };

        //        if (strcmp(pName, propDyn_mujocoContactParamsSolref.name) == 0)
        //            handleProp(mujocoFloatParams, simi_mujoco_global_overridesolref1);
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
        if (strcmp(pName, propDyn_gravity.name) == 0)
        {
            setGravity(pState[0]);
            retVal = 1;
        }
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                for (size_t i = 0; i < 3; i++)
                    pa = pa || ((pState != nullptr) && (arr[simiIndex1 + i] != pState->data[i]));
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < 3; i++)
                            arr[simiIndex1 + i] = pState->data[i];
                    }
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 3);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propDyn_mujocoWind.name, _mujocoFloatParams, simi_mujoco_global_wind1);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getVector3Property(const char* pName, C3Vector& pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    // First non-engine properties:
    if (strcmp(pName, propDyn_gravity.name) == 0)
    {
        pState = _gravity;
        retVal = 1;
    }

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const double* bulletFloatParams = _bulletFloatParams.data();
        const double* odeFloatParams = _odeFloatParams.data();
        const double* vortexFloatParams = _vortexFloatParams.data();
        const double* newtonFloatParams = _newtonFloatParams.data();
        const double* mujocoFloatParams = _mujocoFloatParams.data();
        std::vector<double> __bulletFloatParams;
        std::vector<double> __odeFloatParams;
        std::vector<double> __vortexFloatParams;
        std::vector<double> __newtonFloatParams;
        std::vector<double> __mujocoFloatParams;
        if (getDefaultValue)
        {
            getBulletDefaultFloatParams(__bulletFloatParams);
            getOdeDefaultFloatParams(__odeFloatParams);
            getVortexDefaultFloatParams(__vortexFloatParams);
            getNewtonDefaultFloatParams(__newtonFloatParams);
            getMujocoDefaultFloatParams(__mujocoFloatParams);
            bulletFloatParams = __bulletFloatParams.data();
            odeFloatParams = __odeFloatParams.data();
            vortexFloatParams = __vortexFloatParams.data();
            newtonFloatParams = __newtonFloatParams.data();
            mujocoFloatParams = __mujocoFloatParams.data();
        }
        auto handleProp = [&](const double* arr, int simiIndex1) {
            retVal = 1;
            for (size_t i = 0; i < 3; i++)
                pState(i) = arr[simiIndex1 + i];
        };

        if (strcmp(pName, propDyn_mujocoWind.name) == 0)
            handleProp(mujocoFloatParams, simi_mujoco_global_wind1);
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setFloatArrayProperty(const char* pName, const double* v, int vL, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ((eev == nullptr) && (pName != nullptr))
    { // regular properties (i.e. non-engine properties)
    }

    if (retVal == -1)
    {
        // Following only for engine properties:
        // -------------------------------------
        auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1, size_t n) {
            if ((pName == nullptr) || (propertyName == pName))
            {
                retVal = 1;
                bool pa = false;
                for (size_t i = 0; i < n; i++)
                    pa = pa || ((vL > i) && (arr[simiIndex1 + i] != v[i]));
                if ((pName == nullptr) || pa)
                {
                    if (pName != nullptr)
                    {
                        for (size_t i = 0; i < n; i++)
                        {
                            if (vL > i)
                                arr[simiIndex1 + i] = v[i];
                        }
                    }
                    if (App::worldContainer->getEventsEnabled())
                    {
                        if (ev == nullptr)
                            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propertyName.c_str(), true);
                        ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, n);
                        if (pName != nullptr)
                            _sendEngineString(ev);
                    }
                }
            }
        };

        handleProp(propDyn_mujocoContactParamsSolref.name, _mujocoFloatParams, simi_mujoco_global_overridesolref1, 2);
        handleProp(propDyn_mujocoContactParamsSolimp.name, _mujocoFloatParams, simi_mujoco_global_overridesolimp1, 5);
        handleProp(propDyn_mujocoKinematicWeldSolref.name, _mujocoFloatParams, simi_mujoco_global_kinematicweldsolref1, 2);
        handleProp(propDyn_mujocoKinematicWeldSolimp.name, _mujocoFloatParams, simi_mujoco_global_kinematicweldsolimp1, 5);

        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
        // -------------------------------------
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getFloatArrayProperty(const char* pName, std::vector<double>& pState, bool getDefaultValue /*= false*/) const
{
    int retVal = -1;
    pState.clear();
    // First non-engine properties:
    /*
    if (strcmp(pName, propJoint_length.name) == 0)
    {
        pState = _length;
        retVal = 1;
    }
    */

    // Engine-only properties:
    // ------------------------
    if (retVal == -1)
    {
        const double* bulletFloatParams = _bulletFloatParams.data();
        const double* odeFloatParams = _odeFloatParams.data();
        const double* vortexFloatParams = _vortexFloatParams.data();
        const double* newtonFloatParams = _newtonFloatParams.data();
        const double* mujocoFloatParams = _mujocoFloatParams.data();
        std::vector<double> __bulletFloatParams;
        std::vector<double> __odeFloatParams;
        std::vector<double> __vortexFloatParams;
        std::vector<double> __newtonFloatParams;
        std::vector<double> __mujocoFloatParams;
        if (getDefaultValue)
        {
            getBulletDefaultFloatParams(__bulletFloatParams);
            getOdeDefaultFloatParams(__odeFloatParams);
            getVortexDefaultFloatParams(__vortexFloatParams);
            getNewtonDefaultFloatParams(__newtonFloatParams);
            getMujocoDefaultFloatParams(__mujocoFloatParams);
            bulletFloatParams = __bulletFloatParams.data();
            odeFloatParams = __odeFloatParams.data();
            vortexFloatParams = __vortexFloatParams.data();
            newtonFloatParams = __newtonFloatParams.data();
            mujocoFloatParams = __mujocoFloatParams.data();
        }
        auto handleProp = [&](const double* arr, int simiIndex1, size_t n) {
            retVal = 1;
            for (size_t i = 0; i < n; i++)
                pState.push_back(arr[simiIndex1 + i]);
        };

        if (strcmp(pName, propDyn_mujocoContactParamsSolref.name) == 0)
            handleProp(mujocoFloatParams, simi_mujoco_global_overridesolref1, 2);
        if (strcmp(pName, propDyn_mujocoContactParamsSolimp.name) == 0)
            handleProp(mujocoFloatParams, simi_mujoco_global_overridesolimp1, 5);
        if (strcmp(pName, propDyn_mujocoKinematicWeldSolref.name) == 0)
            handleProp(mujocoFloatParams, simi_mujoco_global_kinematicweldsolref1, 2);
        if (strcmp(pName, propDyn_mujocoKinematicWeldSolimp.name) == 0)
            handleProp(mujocoFloatParams, simi_mujoco_global_kinematicweldsolimp1, 5);
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setIntArrayProperty(const char* pName, const int* v, int vL)
{
    int retVal = -1;

    if (strcmp(pName, propDyn_dynamicsEngine.name) == 0)
    {
        if (vL >= 2)
        {
            setDynamicEngineType(v[0], v[1]);
            retVal = 1;
        }
        else
            retVal = 0;
    }

    if (retVal == 1)
        checkIfEngineSettingsAreDefault();

    return retVal;
}

int CDynamicsContainer::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    int retVal = -1;
    pState.clear();

    if (strcmp(pName, propDyn_dynamicsEngine.name) == 0)
    {
        pState.push_back(_dynamicEngineToUse);
        pState.push_back(_dynamicEngineVersionToUse);
        retVal = 1;
    }

    return retVal;
}

int CDynamicsContainer::getPropertyName(int& index, std::string& pName) const
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_dyn.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_dyn[i].name, pName.c_str()))
        {
            if ((allProps_dyn[i].flags & sim_propertyinfo_deprecated) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_dyn[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CDynamicsContainer::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_dyn.size(); i++)
    {
        if (strcmp(allProps_dyn[i].name, pName) == 0)
        {
            retVal = allProps_dyn[i].type;
            info = allProps_dyn[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_dyn[i].infoTxt, "") != 0))
                infoTxt = allProps_dyn[i].infoTxt;
            else
                infoTxt = allProps_dyn[i].shortInfoTxt;
            break;
        }
    }
    return retVal;
}

void CDynamicsContainer::_sendEngineString(CCbor* eev /*= nullptr*/)
{
    if (App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = nullptr;
        if (eev != nullptr)
            ev = eev;
        CEngineProperties prop;
        std::string current(prop.getObjectProperties(-1));
        if (ev == nullptr)
            ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, propDyn_engineProperties.name, true);
        ev->appendKeyText(propDyn_engineProperties.name, current.c_str());
        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
    }
}

std::string CDynamicsContainer::_enumToProperty(int oldEnum, int type, int& indexWithArrays) const
{
    std::string retVal;
    for (size_t i = 0; i < allProps_dyn.size(); i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            int en = allProps_dyn[i].oldEnums[j];
            if (en == -1)
                break;
            else if (en == oldEnum)
            {
                if (type == allProps_dyn[i].type)
                {
                    if ((j > 0) || (allProps_dyn[i].oldEnums[j + 1] != -1))
                        indexWithArrays = int(j);
                    else
                        indexWithArrays = -1;
                    retVal = allProps_dyn[i].name;
                }
                break;
            }
        }
        if (retVal.size() > 0)
            break;
    }
    return retVal;
}

// Some helpers:
bool CDynamicsContainer::getBoolPropertyValue(const char* pName, bool getDefaultValue /*= false*/) const
{
    bool retVal = false;
    getBoolProperty(pName, retVal, getDefaultValue);
    return retVal;
}

int CDynamicsContainer::getIntPropertyValue(const char* pName, bool getDefaultValue /*= false*/) const
{
    int retVal = 0;
    getIntProperty(pName, retVal, getDefaultValue);
    return retVal;
}

double CDynamicsContainer::getFloatPropertyValue(const char* pName, bool getDefaultValue /*= false*/) const
{
    double retVal = 0.0;
    getFloatProperty(pName, retVal, getDefaultValue);
    return retVal;
}
