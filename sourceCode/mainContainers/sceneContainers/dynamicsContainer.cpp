#include <simInternal.h>
#include <dynamicsContainer.h>
#include <app.h>
#include <simStringTable.h>
#include <tt.h>
#include <engineProperties.h>
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
                                         double *contactInfo) const
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

        App::worldContainer->pluginContainer->dyn_startSimulation(_dynamicEngineToUse, _dynamicEngineVersionToUse,
                                                                  floatParams, intParams);
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

    if ((_stringsNotSupportedWarning == 0))
    {
        if (_dynamicEngineToUse != sim_physics_mujoco)
        {
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_dummy); i++)
            {
                CDummy *it = App::currentWorld->sceneObjects->getDummyFromIndex(i);
                if ((it->getLinkedDummyHandle() != -1) && (it->getDummyType() == sim_dummytype_dyntendon))
                {
                    App::logMsg(sim_verbosity_warnings,
                                "Detected tendon constraints, which are only supported with the MuJoCo engine");
                    _stringsNotSupportedWarning++;
                    break;
                }
            }
        }
    }
}

void CDynamicsContainer::setDynamicEngineType(int t, int version)
{
    _dynamicEngineToUse = t;
    _dynamicEngineVersionToUse = version;
#ifdef SIM_WITH_GUI
    GuiApp::setLightDialogRefreshFlag(); // will trigger a refresh
#endif
}

int CDynamicsContainer::getDynamicEngineType(int *version) const
{
    if (version != nullptr)
        version[0] = _dynamicEngineVersionToUse;
    return (_dynamicEngineToUse);
}

void CDynamicsContainer::setDisplayContactPoints(bool d)
{
    _displayContactPoints = d;
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
        _stepSize = s;
        // Following for backward compatibility:
        setEngineFloatParam_old(sim_bullet_global_stepsize, s);
        setEngineFloatParam_old(sim_ode_global_stepsize, s);
        setEngineFloatParam_old(sim_vortex_global_stepsize, s);
        setEngineFloatParam_old(sim_newton_global_stepsize, s);
        setEngineFloatParam_old(sim_mujoco_global_stepsize, s);
        retVal = true;
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
        return (getEngineBoolParam_old(sim_bullet_global_computeinertias, nullptr));
    if (_dynamicEngineToUse == sim_physics_ode)
        return (getEngineBoolParam_old(sim_ode_global_computeinertias, nullptr));
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (getEngineBoolParam_old(sim_vortex_global_computeinertias, nullptr));
    if (_dynamicEngineToUse == sim_physics_newton)
        return (getEngineBoolParam_old(sim_newton_global_computeinertias, nullptr));
    if (_dynamicEngineToUse == sim_physics_mujoco)
        return (getEngineBoolParam_old(sim_mujoco_global_computeinertias, nullptr));
    return (false);
}

bool CDynamicsContainer::setIterationCount(int c)
{
    bool retVal = false;
    if (App::currentWorld->simulation->isSimulationStopped())
    {
        if (_dynamicEngineToUse == sim_physics_bullet)
            setEngineIntParam_old(sim_bullet_global_constraintsolvingiterations, c);
        if (_dynamicEngineToUse == sim_physics_ode)
            setEngineIntParam_old(sim_ode_global_constraintsolvingiterations, c);
        if (_dynamicEngineToUse == sim_physics_vortex)
            return (false); // not available
        if (_dynamicEngineToUse == sim_physics_newton)
            setEngineIntParam_old(sim_newton_global_constraintsolvingiterations, c);
        if (_dynamicEngineToUse == sim_physics_mujoco)
            setEngineIntParam_old(sim_mujoco_global_iterations, c);
        retVal = true;
    }
    return (retVal);
}

int CDynamicsContainer::getIterationCount() const
{
    if (_dynamicEngineToUse == sim_physics_bullet)
        return (getEngineIntParam_old(sim_bullet_global_constraintsolvingiterations, nullptr));
    if (_dynamicEngineToUse == sim_physics_ode)
        return (getEngineIntParam_old(sim_ode_global_constraintsolvingiterations, nullptr));
    if (_dynamicEngineToUse == sim_physics_vortex)
        return (0); // not available
    if (_dynamicEngineToUse == sim_physics_newton)
        return (getEngineIntParam_old(sim_newton_global_constraintsolvingiterations, nullptr));
    if (_dynamicEngineToUse == sim_physics_mujoco)
        return (getEngineIntParam_old(sim_mujoco_global_iterations, nullptr));
    return (0);
}

double CDynamicsContainer::getEngineFloatParam_old(int what, bool *ok, bool getDefault /*=false*/) const
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
    prop = _enumToProperty(what, sim_propertytype_vector, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> v;
        if (getVectorProperty(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector v;
        if (getVector3Property(prop.c_str(), v) > 0)
            return v(indexWithArrays);
    }
    // For backward compatibility:
    if ( (what == sim_bullet_global_stepsize) || (what == sim_ode_global_stepsize) || (what == sim_vortex_global_stepsize) || (what == sim_newton_global_stepsize) || (what == sim_mujoco_global_stepsize) )
        return (_stepSize);

    if (ok != nullptr)
        ok[0] = false;
    return 0.0;

/*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_global_float_start) && (what < sim_bullet_global_float_end))
    {
        std::vector<double> fp;
        getBulletFloatParams(fp, getDefault);
        int w = what - sim_bullet_global_stepsize + simi_bullet_global_stepsize;
        if (what == sim_bullet_global_stepsize) // for backw. compatibility
            return (_stepSize);
        return (fp[w]);
    }
    if ((what > sim_ode_global_float_start) && (what < sim_ode_global_float_end))
    {
        std::vector<double> fp;
        getOdeFloatParams(fp, getDefault);
        int w = what - sim_ode_global_stepsize + simi_ode_global_stepsize;
        if (what == sim_ode_global_stepsize) // for backw. compatibility
            return (_stepSize);
        return (fp[w]);
    }
    if ((what > sim_vortex_global_float_start) && (what < sim_vortex_global_float_end))
    {
        std::vector<double> fp;
        getVortexFloatParams(fp, getDefault);
        int w = what - sim_vortex_global_stepsize + simi_vortex_global_stepsize;
        if (what == sim_vortex_global_stepsize) // for backw. compatibility
            return (_stepSize);
        return (fp[w]);
    }
    if ((what > sim_newton_global_float_start) && (what < sim_newton_global_float_end))
    {
        std::vector<double> fp;
        getNewtonFloatParams(fp, getDefault);
        int w = what - sim_newton_global_stepsize + simi_newton_global_stepsize;
        if (what == sim_newton_global_stepsize) // for backw. compatibility
            return (_stepSize);
        return (fp[w]);
    }
    if ((what > sim_mujoco_global_float_start) && (what < sim_mujoco_global_float_end))
    {
        std::vector<double> fp;
        getMujocoFloatParams(fp, getDefault);
        int w = what - sim_mujoco_global_stepsize + simi_mujoco_global_stepsize;
        if (what == sim_mujoco_global_stepsize) // for backw. compatibility
            return (_stepSize);
        return (fp[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0.0);
    */
}

int CDynamicsContainer::getEngineIntParam_old(int what, bool *ok, bool getDefault /*=false*/) const
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

/*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_global_int_start) && (what < sim_bullet_global_int_end))
    {
        std::vector<int> ip;
        getBulletIntParams(ip, getDefault);
        int w = what - sim_bullet_global_constraintsolvingiterations + simi_bullet_global_constraintsolvingiterations;
        return (ip[w]);
    }
    if ((what > sim_ode_global_int_start) && (what < sim_ode_global_int_end))
    {
        std::vector<int> ip;
        getOdeIntParams(ip, getDefault);
        int w = what - sim_ode_global_constraintsolvingiterations + simi_ode_global_constraintsolvingiterations;
        return (ip[w]);
    }
    if ((what > sim_vortex_global_int_start) && (what < sim_vortex_global_int_end))
    {
        std::vector<int> ip;
        getVortexIntParams(ip, getDefault);
        int w = what - sim_vortex_global_bitcoded + simi_vortex_global_bitcoded;
        return (ip[w]);
    }
    if ((what > sim_newton_global_int_start) && (what < sim_newton_global_int_end))
    {
        std::vector<int> ip;
        getNewtonIntParams(ip, getDefault);
        int w = what - sim_newton_global_constraintsolvingiterations + simi_newton_global_constraintsolvingiterations;
        return (ip[w]);
    }
    if ((what > sim_mujoco_global_int_start) && (what < sim_mujoco_global_int_end))
    {
        std::vector<int> ip;
        getMujocoIntParams(ip, getDefault);
        int w = what - sim_mujoco_global_bitcoded + simi_mujoco_global_bitcoded;
        return (ip[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
    */
}

bool CDynamicsContainer::getEngineBoolParam_old(int what, bool *ok, bool getDefault /*=false*/) const
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

/*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_global_bool_start) && (what < sim_bullet_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_bullet_global_fullinternalscaling);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getBulletIntParams(ip, getDefault);
        return ((ip[simi_bullet_global_bitcoded] & b) != 0);
    }
    if ((what > sim_ode_global_bool_start) && (what < sim_ode_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_ode_global_fullinternalscaling);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getOdeIntParams(ip, getDefault);
        return ((ip[simi_ode_global_bitcoded] & b) != 0);
    }
    if ((what > sim_vortex_global_bool_start) && (what < sim_vortex_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_vortex_global_autosleep);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getVortexIntParams(ip, getDefault);
        return ((ip[simi_vortex_global_bitcoded] & b) != 0);
    }
    if ((what > sim_newton_global_bool_start) && (what < sim_newton_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_newton_global_multithreading);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getNewtonIntParams(ip, getDefault);
        return ((ip[simi_newton_global_bitcoded] & b) != 0);
    }
    if ((what > sim_mujoco_global_bool_start) && (what < sim_mujoco_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_mujoco_global_computeinertias);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getMujocoIntParams(ip, getDefault);
        return ((ip[simi_mujoco_global_bitcoded] & b) != 0);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
    */
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
        prop = _enumToProperty(what, sim_propertytype_vector, indexWithArrays);
        if (prop.size() > 0)
        {
            std::vector<double> w;
            if (getVectorProperty(prop.c_str(), w) > 0)
            {
                w[indexWithArrays] = v;
                if (setVectorProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
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
        if ( (what == sim_bullet_global_stepsize) || (what == sim_ode_global_stepsize) || (what == sim_vortex_global_stepsize) || (what == sim_newton_global_stepsize) || (what == sim_mujoco_global_stepsize) )
        {
            _stepSize = v;
            retVal = true;
        }
    }

/*
    if ((what > sim_bullet_global_float_start) && (what < sim_bullet_global_float_end))
    {
        int w = what - sim_bullet_global_stepsize + simi_bullet_global_stepsize;
        std::vector<double> fp;
        getBulletFloatParams(fp);
        fp[w] = v;
        setBulletFloatParams(fp);
        if (what == sim_bullet_global_stepsize) // for backw. compatibility
            _stepSize = v;
        retVal = true;
    }
    if ((what > sim_ode_global_float_start) && (what < sim_ode_global_float_end))
    {
        int w = what - sim_ode_global_stepsize + simi_ode_global_stepsize;
        std::vector<double> fp;
        getOdeFloatParams(fp);
        fp[w] = v;
        setOdeFloatParams(fp);
        if (what == sim_ode_global_stepsize) // for backw. compatibility
            _stepSize = v;
        retVal = true;
    }
    if ((what > sim_vortex_global_float_start) && (what < sim_vortex_global_float_end))
    {
        int w = what - sim_vortex_global_stepsize + simi_vortex_global_stepsize;
        std::vector<double> fp;
        getVortexFloatParams(fp);
        fp[w] = v;
        setVortexFloatParams(fp);
        if (what == sim_vortex_global_stepsize) // for backw. compatibility
            _stepSize = v;
        retVal = true;
    }
    if ((what > sim_newton_global_float_start) && (what < sim_newton_global_float_end))
    {
        int w = what - sim_newton_global_stepsize + simi_newton_global_stepsize;
        std::vector<double> fp;
        getNewtonFloatParams(fp);
        fp[w] = v;
        setNewtonFloatParams(fp);
        if (what == sim_newton_global_stepsize) // for backw. compatibility
            _stepSize = v;
        retVal = true;
    }
    if ((what > sim_mujoco_global_float_start) && (what < sim_mujoco_global_float_end))
    {
        int w = what - sim_mujoco_global_stepsize + simi_mujoco_global_stepsize;
        std::vector<double> fp;
        getMujocoFloatParams(fp);
        fp[w] = v;
        setMujocoFloatParams(fp);
        if (what == sim_mujoco_global_stepsize) // for backw. compatibility
            _stepSize = v;
        retVal = true;
    }
    */
    if (retVal)
        checkIfEngineSettingsAreDefault();
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

    /*
    if ((what > sim_bullet_global_int_start) && (what < sim_bullet_global_int_end))
    {
        int w = what - sim_bullet_global_constraintsolvingiterations + simi_bullet_global_constraintsolvingiterations;
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[w] = v;
        setBulletIntParams(ip);
        retVal = true;
    }
    if ((what > sim_ode_global_int_start) && (what < sim_ode_global_int_end))
    {
        int w = what - sim_ode_global_constraintsolvingiterations + simi_ode_global_constraintsolvingiterations;
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[w] = v;
        setOdeIntParams(ip);
        retVal = true;
    }
    if ((what > sim_vortex_global_int_start) && (what < sim_vortex_global_int_end))
    {
        int w = what - sim_vortex_global_bitcoded + simi_vortex_global_bitcoded;
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[w] = v;
        setVortexIntParams(ip);
        retVal = true;
    }
    if ((what > sim_newton_global_int_start) && (what < sim_newton_global_int_end))
    {
        int w = what - sim_newton_global_constraintsolvingiterations + simi_newton_global_constraintsolvingiterations;
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[w] = v;
        setNewtonIntParams(ip);
        retVal = true;
    }
    if ((what > sim_mujoco_global_int_start) && (what < sim_mujoco_global_int_end))
    {
        int w = what - sim_mujoco_global_bitcoded + simi_mujoco_global_bitcoded;
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[w] = v;
        setMujocoIntParams(ip);
        retVal = true;
    }
    */
    if (retVal)
        checkIfEngineSettingsAreDefault();
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

/*
    if ((what > sim_bullet_global_bool_start) && (what < sim_bullet_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_bullet_global_fullinternalscaling);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[simi_bullet_global_bitcoded] |= b;
        if (!v)
            ip[simi_bullet_global_bitcoded] -= b;
        setBulletIntParams(ip);
        retVal = true;
    }
    if ((what > sim_ode_global_bool_start) && (what < sim_ode_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_ode_global_fullinternalscaling);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[simi_ode_global_bitcoded] |= b;
        if (!v)
            ip[simi_ode_global_bitcoded] -= b;
        setOdeIntParams(ip);
        retVal = true;
    }
    if ((what > sim_vortex_global_bool_start) && (what < sim_vortex_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_vortex_global_autosleep);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[simi_vortex_global_bitcoded] |= b;
        if (!v)
            ip[simi_vortex_global_bitcoded] -= b;
        setVortexIntParams(ip);
        retVal = true;
    }
    if ((what > sim_newton_global_bool_start) && (what < sim_newton_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_newton_global_multithreading);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[simi_newton_global_bitcoded] |= b;
        if (!v)
            ip[simi_newton_global_bitcoded] -= b;
        setNewtonIntParams(ip);
        retVal = true;
    }
    if ((what > sim_mujoco_global_bool_start) && (what < sim_mujoco_global_bool_end))
    {
        int b = 1;
        int w = (what - sim_mujoco_global_computeinertias);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[simi_mujoco_global_bitcoded] |= b;
        if (!v)
            ip[simi_mujoco_global_bitcoded] -= b;
        setMujocoIntParams(ip);
        retVal = true;
    }
    */

    if (retVal)
        checkIfEngineSettingsAreDefault();
    return (retVal);
}

void CDynamicsContainer::getBulletFloatParams(std::vector<double> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getBulletDefaultFloatParams(p);
    else
        p.assign(_bulletFloatParams.begin(), _bulletFloatParams.end());
}

void CDynamicsContainer::setBulletFloatParams(const std::vector<double> &p)
{
    _bulletFloatParams.assign(p.begin(), p.end());
    _bulletFloatParams[simi_bullet_global_stepsize] =
        tt::getLimitedFloat(0.00001, 1.0, _bulletFloatParams[simi_bullet_global_stepsize]); // step size
    _bulletFloatParams[simi_bullet_global_internalscalingfactor] = tt::getLimitedFloat(
        0.0001, 10000.0, _bulletFloatParams[simi_bullet_global_internalscalingfactor]); // internal scaling factor
    _bulletFloatParams[simi_bullet_global_collisionmarginfactor] = tt::getLimitedFloat(
        0.001, 100.0, _bulletFloatParams[simi_bullet_global_collisionmarginfactor]); // collision margin factor
}

void CDynamicsContainer::getBulletIntParams(std::vector<int> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getBulletDefaultIntParams(p);
    else
        p.assign(_bulletIntParams.begin(), _bulletIntParams.end());
}

void CDynamicsContainer::setBulletIntParams(const std::vector<int> &p)
{
    _bulletIntParams.assign(p.begin(), p.end());
    _bulletIntParams[simi_bullet_global_constraintsolvingiterations] = tt::getLimitedFloat(
        1, 10000, _bulletIntParams[simi_bullet_global_constraintsolvingiterations]); // constr. solv. iterations
}

void CDynamicsContainer::getOdeFloatParams(std::vector<double> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getOdeDefaultFloatParams(p);
    else
        p.assign(_odeFloatParams.begin(), _odeFloatParams.end());
}

void CDynamicsContainer::setOdeFloatParams(const std::vector<double> &p)
{
    _odeFloatParams.assign(p.begin(), p.end());
    _odeFloatParams[simi_ode_global_stepsize] =
        tt::getLimitedFloat(0.00001, 1.0, _odeFloatParams[simi_ode_global_stepsize]); // step size
    _odeFloatParams[simi_ode_global_internalscalingfactor] = tt::getLimitedFloat(
        0.0001, 10000.0, _odeFloatParams[simi_ode_global_internalscalingfactor]); // internal scaling factor
    _odeFloatParams[simi_ode_global_cfm] =
        tt::getLimitedFloat(0.0, 1.0, _odeFloatParams[simi_ode_global_cfm]); // global CFM
    _odeFloatParams[simi_ode_global_erp] =
        tt::getLimitedFloat(0.0, 1.0, _odeFloatParams[simi_ode_global_erp]); // global ERP
}

void CDynamicsContainer::getOdeIntParams(std::vector<int> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getOdeDefaultIntParams(p);
    else
        p.assign(_odeIntParams.begin(), _odeIntParams.end());
}

void CDynamicsContainer::setOdeIntParams(const std::vector<int> &p)
{
    _odeIntParams.assign(p.begin(), p.end());
    _odeIntParams[simi_ode_global_constraintsolvingiterations] = tt::getLimitedFloat(
        1, 10000, _odeIntParams[simi_ode_global_constraintsolvingiterations]); // constr. solv. iterations
}

void CDynamicsContainer::getVortexFloatParams(std::vector<double> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getVortexDefaultFloatParams(p);
    else
        p.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
}

void CDynamicsContainer::setVortexFloatParams(const std::vector<double> &p)
{
    _vortexFloatParams.assign(p.begin(), p.end());
    _vortexFloatParams[simi_vortex_global_stepsize] =
        tt::getLimitedFloat(0.00001, 1.0, _vortexFloatParams[simi_vortex_global_stepsize]); // step size
    _vortexFloatParams[simi_vortex_global_internalscalingfactor] = tt::getLimitedFloat(
        0.0001, 10000.0, _vortexFloatParams[simi_vortex_global_internalscalingfactor]); // internal scaling factor
    _vortexFloatParams[simi_vortex_global_contacttolerance] =
        tt::getLimitedFloat(0.0, 10.0, _vortexFloatParams[simi_vortex_global_contacttolerance]); // contact tolerance
}

void CDynamicsContainer::getVortexIntParams(std::vector<int> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getVortexDefaultIntParams(p);
    else
        p.assign(_vortexIntParams.begin(), _vortexIntParams.end());
}

void CDynamicsContainer::setVortexIntParams(const std::vector<int> &p)
{
    _vortexIntParams.assign(p.begin(), p.end());
}

void CDynamicsContainer::getNewtonFloatParams(std::vector<double> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getNewtonDefaultFloatParams(p);
    else
        p.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
}

void CDynamicsContainer::setNewtonFloatParams(const std::vector<double> &p)
{
    _newtonFloatParams.assign(p.begin(), p.end());
    _newtonFloatParams[simi_newton_global_stepsize] =
        tt::getLimitedFloat(0.00001, 1.0, _newtonFloatParams[simi_newton_global_stepsize]); // step size
    _newtonFloatParams[simi_newton_global_contactmergetolerance] = tt::getLimitedFloat(
        0.0001, 1.0, _newtonFloatParams[simi_newton_global_contactmergetolerance]); // contact merge tolerance
}

void CDynamicsContainer::getNewtonIntParams(std::vector<int> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getNewtonDefaultIntParams(p);
    else
        p.assign(_newtonIntParams.begin(), _newtonIntParams.end());
}

void CDynamicsContainer::setNewtonIntParams(const std::vector<int> &p)
{
    _newtonIntParams.assign(p.begin(), p.end());
    _newtonIntParams[simi_newton_global_constraintsolvingiterations] = tt::getLimitedInt(
        1, 128, _newtonIntParams[simi_newton_global_constraintsolvingiterations]); // solving iterations
}

void CDynamicsContainer::getMujocoFloatParams(std::vector<double> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getMujocoDefaultFloatParams(p);
    else
        p.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
}

void CDynamicsContainer::setMujocoFloatParams(const std::vector<double> &p)
{
    _mujocoFloatParams.assign(p.begin(), p.end());
    _mujocoFloatParams[simi_mujoco_global_stepsize] =
        tt::getLimitedFloat(0.00001, 1.0, _mujocoFloatParams[simi_mujoco_global_stepsize]);
}

void CDynamicsContainer::getMujocoIntParams(std::vector<int> &p, bool getDefault /*=false*/) const
{
    if (getDefault)
        getMujocoDefaultIntParams(p);
    else
        p.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());
}

void CDynamicsContainer::setMujocoIntParams(const std::vector<int> &p)
{
    _mujocoIntParams.assign(p.begin(), p.end());
    _mujocoIntParams[simi_mujoco_global_iterations] =
        tt::getLimitedInt(1, 1000, _mujocoIntParams[simi_mujoco_global_iterations]);
    _mujocoIntParams[simi_mujoco_global_integrator] =
        tt::getLimitedInt(0, 2, _mujocoIntParams[simi_mujoco_global_integrator]);
    _mujocoIntParams[simi_mujoco_global_solver] = tt::getLimitedInt(0, 2, _mujocoIntParams[simi_mujoco_global_solver]);
    _mujocoIntParams[simi_mujoco_global_njmax] =
        tt::getLimitedInt(100, 50000, _mujocoIntParams[simi_mujoco_global_njmax]);
    _mujocoIntParams[simi_mujoco_global_nconmax] =
        tt::getLimitedInt(100, 50000, _mujocoIntParams[simi_mujoco_global_nconmax]);
    _mujocoIntParams[simi_mujoco_global_cone] = tt::getLimitedInt(0, 1, _mujocoIntParams[simi_mujoco_global_cone]);
    _mujocoIntParams[simi_mujoco_global_overridekin] =
        tt::getLimitedInt(0, 2, _mujocoIntParams[simi_mujoco_global_overridekin]);
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
    _dynamicsEnabled = e;
    if (!e)
        App::currentWorld->dynamicsContainer->removeWorld();
    else
    {
        if (App::currentWorld->simulation->isSimulationRunning())
            App::currentWorld->dynamicsContainer->addWorldIfNotThere();
    }
}

bool CDynamicsContainer::getDynamicsEnabled() const
{
    return (_dynamicsEnabled);
}

void CDynamicsContainer::setGravity(const C3Vector &gr)
{
    _gravity = gr;
    _gravity(0) = tt::getLimitedFloat(-1000.0, +1000.0, _gravity(0));
    _gravity(1) = tt::getLimitedFloat(-1000.0, +1000.0, _gravity(1));
    _gravity(2) = tt::getLimitedFloat(-1000.0, +1000.0, _gravity(2));
}

C3Vector CDynamicsContainer::getGravity() const
{
    return (_gravity);
}

bool CDynamicsContainer::_engineFloatsAreSimilar(const std::vector<double> &arr1, const std::vector<double> &arr2) const
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

void CDynamicsContainer::serialize(CSer &ar)
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
                                getEngineFloatParam_old(sim_bullet_global_internalscalingfactor, nullptr));
            ar.xmlAddNode_float("collisionmarginfactor",
                                getEngineFloatParam_old(sim_bullet_global_collisionmarginfactor, nullptr));

            ar.xmlAddNode_int("constraintsolvingiterations",
                              getEngineIntParam_old(sim_bullet_global_constraintsolvingiterations, nullptr));
            ar.xmlAddNode_int("constraintsolvertype",
                              getEngineIntParam_old(sim_bullet_global_constraintsolvertype, nullptr));

            ar.xmlAddNode_bool("fullinternalscaling",
                               getEngineBoolParam_old(sim_bullet_global_fullinternalscaling, nullptr));
            ar.xmlAddNode_bool("computeinertias", getEngineBoolParam_old(sim_bullet_global_computeinertias, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("internalscalingfactor",
                                getEngineFloatParam_old(sim_ode_global_internalscalingfactor, nullptr));
            ar.xmlAddNode_float("cfm", getEngineFloatParam_old(sim_ode_global_cfm, nullptr));
            ar.xmlAddNode_float("erp", getEngineFloatParam_old(sim_ode_global_erp, nullptr));

            ar.xmlAddNode_int("constraintsolvingiterations",
                              getEngineIntParam_old(sim_ode_global_constraintsolvingiterations, nullptr));
            ar.xmlAddNode_int("randomseed", getEngineIntParam_old(sim_ode_global_randomseed, nullptr));

            ar.xmlAddNode_bool("fullinternalscaling", getEngineBoolParam_old(sim_ode_global_fullinternalscaling, nullptr));
            ar.xmlAddNode_bool("quickstep", getEngineBoolParam_old(sim_ode_global_quickstep, nullptr));
            ar.xmlAddNode_bool("computeinertias", getEngineBoolParam_old(sim_ode_global_computeinertias, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("internalscalingfactor",
                                getEngineFloatParam_old(sim_vortex_global_internalscalingfactor, nullptr));
            ar.xmlAddNode_float("contacttolerance", getEngineFloatParam_old(sim_vortex_global_contacttolerance, nullptr));
            ar.xmlAddNode_float("constraintlinearcompliance",
                                getEngineFloatParam_old(sim_vortex_global_constraintlinearcompliance, nullptr));
            ar.xmlAddNode_float("constraintlineardamping",
                                getEngineFloatParam_old(sim_vortex_global_constraintlineardamping, nullptr));
            ar.xmlAddNode_float("constraintlinearkineticloss",
                                getEngineFloatParam_old(sim_vortex_global_constraintlinearkineticloss, nullptr));
            ar.xmlAddNode_float("constraintangularcompliance",
                                getEngineFloatParam_old(sim_vortex_global_constraintangularcompliance, nullptr));
            ar.xmlAddNode_float("constraintangulardamping",
                                getEngineFloatParam_old(sim_vortex_global_constraintangulardamping, nullptr));
            ar.xmlAddNode_float("constraintangularkineticloss",
                                getEngineFloatParam_old(sim_vortex_global_constraintangularkineticloss, nullptr));

            ar.xmlAddNode_bool("autosleep", getEngineBoolParam_old(sim_vortex_global_autosleep, nullptr));
            ar.xmlAddNode_bool("multithreading", getEngineBoolParam_old(sim_vortex_global_multithreading, nullptr));
            ar.xmlAddNode_bool("computeinertias", getEngineBoolParam_old(sim_vortex_global_computeinertias, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_comment(" 'stepsize' tag: used for backward compatibility", exhaustiveXml);
            ar.xmlAddNode_float("stepsize", _stepSize);
            ar.xmlAddNode_float("contactmergetolerance",
                                getEngineFloatParam_old(sim_newton_global_contactmergetolerance, nullptr));
            ar.xmlAddNode_int("constraintsolvingiterations",
                              getEngineIntParam_old(sim_newton_global_constraintsolvingiterations, nullptr));
            ar.xmlAddNode_bool("multithreading", getEngineBoolParam_old(sim_newton_global_multithreading, nullptr));
            ar.xmlAddNode_bool("exactsolver", getEngineBoolParam_old(sim_newton_global_exactsolver, nullptr));
            ar.xmlAddNode_bool("highjointaccuracy", getEngineBoolParam_old(sim_newton_global_highjointaccuracy, nullptr));
            ar.xmlAddNode_bool("computeinertias", getEngineBoolParam_old(sim_newton_global_computeinertias, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            ar.xmlAddNode_float("impratio", getEngineFloatParam_old(sim_mujoco_global_impratio, nullptr));
            double w[5];
            for (size_t j = 0; j < 3; j++)
                w[j] = getEngineFloatParam_old(sim_mujoco_global_wind1 + int(j), nullptr);
            ar.xmlAddNode_floats("wind", w, 3);
            ar.xmlAddNode_float("density", getEngineFloatParam_old(sim_mujoco_global_density, nullptr));
            ar.xmlAddNode_float("viscosity", getEngineFloatParam_old(sim_mujoco_global_viscosity, nullptr));
            ar.xmlAddNode_float("boundmass", getEngineFloatParam_old(sim_mujoco_global_boundmass, nullptr));
            ar.xmlAddNode_float("boundinertia", getEngineFloatParam_old(sim_mujoco_global_boundinertia, nullptr));
            ar.xmlAddNode_float("overridemargin", getEngineFloatParam_old(sim_mujoco_global_overridemargin, nullptr));
            for (size_t j = 0; j < 2; j++)
                w[j] = getEngineFloatParam_old(sim_mujoco_global_overridesolref1 + int(j), nullptr);
            ar.xmlAddNode_floats("overridesolref", w, 2);
            for (size_t j = 0; j < 5; j++)
                w[j] = getEngineFloatParam_old(sim_mujoco_global_overridesolimp1 + int(j), nullptr);
            ar.xmlAddNode_floats("overridesolimp", w, 5);
            ar.xmlAddNode_float("kinmass", getEngineFloatParam_old(sim_mujoco_global_kinmass, nullptr));
            ar.xmlAddNode_float("kininertia", getEngineFloatParam_old(sim_mujoco_global_kininertia, nullptr));
            ar.xmlAddNode_int("iterations", getEngineIntParam_old(sim_mujoco_global_iterations, nullptr));
            ar.xmlAddNode_int("integrator", getEngineIntParam_old(sim_mujoco_global_integrator, nullptr));
            ar.xmlAddNode_int("solver", getEngineIntParam_old(sim_mujoco_global_solver, nullptr));
            ar.xmlAddNode_int("njmax", getEngineIntParam_old(sim_mujoco_global_njmax, nullptr));
            ar.xmlAddNode_int("nconmax", getEngineIntParam_old(sim_mujoco_global_nconmax, nullptr));
            ar.xmlAddNode_int("nstack", getEngineIntParam_old(sim_mujoco_global_nstack, nullptr));
            ar.xmlAddNode_int("cone", getEngineIntParam_old(sim_mujoco_global_cone, nullptr));
            ar.xmlAddNode_int("overridekin", getEngineIntParam_old(sim_mujoco_global_overridekin, nullptr));
            ar.xmlAddNode_int("rebuildtrigger", getEngineIntParam_old(sim_mujoco_global_rebuildtrigger, nullptr));
            ar.xmlAddNode_bool("computeinertias", getEngineBoolParam_old(sim_mujoco_global_computeinertias, nullptr));
            ar.xmlAddNode_bool("multithreaded", getEngineBoolParam_old(sim_mujoco_global_multithreaded, nullptr));
            ar.xmlAddNode_bool("multiccd", getEngineBoolParam_old(sim_mujoco_global_multiccd, nullptr));
            ar.xmlAddNode_bool("balanceinertias", getEngineBoolParam_old(sim_mujoco_global_balanceinertias, nullptr));
            ar.xmlAddNode_bool("overridecontacts", getEngineBoolParam_old(sim_mujoco_global_overridecontacts, nullptr));
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
                        setEngineFloatParam_old(sim_bullet_global_internalscalingfactor, v);
                    if (ar.xmlGetNode_float("collisionmarginfactor", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_bullet_global_collisionmarginfactor, v);

                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_bullet_global_constraintsolvingiterations, vi);
                    if (ar.xmlGetNode_int("constraintsolvertype", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_bullet_global_constraintsolvertype, vi);

                    if (ar.xmlGetNode_bool("fullinternalscaling", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_bullet_global_fullinternalscaling, vb);
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_bullet_global_computeinertias, vb);
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
                        setEngineFloatParam_old(sim_ode_global_internalscalingfactor, v);
                    if (ar.xmlGetNode_float("cfm", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_ode_global_cfm, v);
                    if (ar.xmlGetNode_float("erp", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_ode_global_erp, v);

                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_ode_global_constraintsolvingiterations, vi);
                    if (ar.xmlGetNode_int("randomseed", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_ode_global_randomseed, vi);

                    if (ar.xmlGetNode_bool("fullinternalscaling", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_ode_global_fullinternalscaling, vb);
                    if (ar.xmlGetNode_bool("quickstep", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_ode_global_quickstep, vb);
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_ode_global_computeinertias, vb);
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
                        setEngineFloatParam_old(sim_vortex_global_internalscalingfactor, v);
                    if (ar.xmlGetNode_float("contacttolerance", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_contacttolerance, v);
                    if (ar.xmlGetNode_float("constraintlinearcompliance", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintlinearcompliance, v);
                    if (ar.xmlGetNode_float("constraintlineardamping", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintlineardamping, v);
                    if (ar.xmlGetNode_float("constraintlinearkineticloss", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintlinearkineticloss, v);
                    if (ar.xmlGetNode_float("constraintangularcompliance", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintangularcompliance, v);
                    if (ar.xmlGetNode_float("constraintangulardamping", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintangulardamping, v);
                    if (ar.xmlGetNode_float("constraintangularkineticloss", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_vortex_global_constraintangularkineticloss, v);

                    if (ar.xmlGetNode_bool("autosleep", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_vortex_global_autosleep, vb);
                    if (ar.xmlGetNode_bool("multithreading", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_vortex_global_multithreading, vb);
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_vortex_global_computeinertias, vb);
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
                        setEngineFloatParam_old(sim_newton_global_contactmergetolerance, v);
                    if (ar.xmlGetNode_int("constraintsolvingiterations", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_newton_global_constraintsolvingiterations, vi);
                    if (ar.xmlGetNode_bool("multithreading", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_newton_global_multithreading, vb);
                    if (ar.xmlGetNode_bool("exactsolver", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_newton_global_exactsolver, vb);
                    if (ar.xmlGetNode_bool("highjointaccuracy", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_newton_global_highjointaccuracy, vb);
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_newton_global_computeinertias, vb);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("mujoco"))
                {
                    if (ar.xmlGetNode_float("impratio", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_impratio, v);
                    double w[5];
                    if (ar.xmlGetNode_floats("wind", w, 3, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 3; j++)
                            setEngineFloatParam_old(sim_mujoco_global_wind1 + int(j), w[j]);
                    }
                    if (ar.xmlGetNode_float("density", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_density, v);
                    if (ar.xmlGetNode_float("viscosity", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_viscosity, v);
                    if (ar.xmlGetNode_float("boundmass", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_boundmass, v);
                    if (ar.xmlGetNode_float("boundinertia", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_boundinertia, v);
                    if (ar.xmlGetNode_float("overridemargin", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_overridemargin, v);
                    if (ar.xmlGetNode_floats("overridesolref", w, 2, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 2; j++)
                            setEngineFloatParam_old(sim_mujoco_global_overridesolref1 + int(j), w[j]);
                    }
                    if (ar.xmlGetNode_floats("overridesolimp", w, 5, exhaustiveXml))
                    {
                        for (size_t j = 0; j < 5; j++)
                            setEngineFloatParam_old(sim_mujoco_global_overridesolimp1 + int(j), w[j]);
                    }
                    if (ar.xmlGetNode_float("kinmass", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_kinmass, v);
                    if (ar.xmlGetNode_float("kininertia", v, exhaustiveXml))
                        setEngineFloatParam_old(sim_mujoco_global_kininertia, v);
                    if (ar.xmlGetNode_int("iterations", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_iterations, vi);
                    if (ar.xmlGetNode_int("integrator", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_integrator, vi);
                    if (ar.xmlGetNode_int("solver", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_solver, vi);
                    if (ar.xmlGetNode_int("njmax", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_njmax, vi);
                    if (ar.xmlGetNode_int("nconmax", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_nconmax, vi);
                    if (ar.xmlGetNode_int("nstack", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_nstack, vi);
                    if (ar.xmlGetNode_int("cone", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_cone, vi);
                    if (ar.xmlGetNode_int("overridekin", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_overridekin, vi);
                    if (ar.xmlGetNode_int("rebuildtrigger", vi, exhaustiveXml))
                        setEngineIntParam_old(sim_mujoco_global_rebuildtrigger, vi);
                    if (ar.xmlGetNode_bool("computeinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_mujoco_global_computeinertias, vb);
                    if (ar.xmlGetNode_bool("multithreaded", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_mujoco_global_multithreaded, vb);
                    if (ar.xmlGetNode_bool("multiccd", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_mujoco_global_multiccd, vb);
                    if (ar.xmlGetNode_bool("balanceinertias", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_mujoco_global_balanceinertias, vb);
                    if (ar.xmlGetNode_bool("overridecontacts", vb, exhaustiveXml))
                        setEngineBoolParam_old(sim_mujoco_global_overridecontacts, vb);
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

void CDynamicsContainer::getBulletDefaultFloatParams(std::vector<double> &p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005); // simi_bullet_global_stepsize
    p.push_back(10.0);  // simi_bullet_global_internalscalingfactor
    p.push_back(0.1);   // simi_bullet_global_collisionmarginfactor
    p.push_back(0.0);   // free
    p.push_back(0.0);   // free
}

void CDynamicsContainer::getBulletDefaultIntParams(std::vector<int> &p, int defType /*=-1*/) const
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

void CDynamicsContainer::getOdeDefaultFloatParams(std::vector<double> &p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005);   // simi_bullet_global_stepsize
    p.push_back(1.0);     // simi_bullet_global_internalscalingfactor
    p.push_back(0.00001); // simi_bullet_global_cfm
    p.push_back(0.6);     // simi_bullet_global_erp
    p.push_back(0.0);     // free
}

void CDynamicsContainer::getOdeDefaultIntParams(std::vector<int> &p, int defType /*=-1*/) const
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

void CDynamicsContainer::getVortexDefaultFloatParams(std::vector<double> &p, int defType /*=-1*/) const
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

void CDynamicsContainer::getVortexDefaultIntParams(std::vector<int> &p, int defType /*=-1*/) const
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

void CDynamicsContainer::getNewtonDefaultFloatParams(std::vector<double> &p, int defType /*=-1*/) const
{
    p.clear();
    p.push_back(0.005); // simi_newton_global_stepsize
    p.push_back(0.01);  // simi_newton_global_contactmergetolerance
}

void CDynamicsContainer::getNewtonDefaultIntParams(std::vector<int> &p, int defType /*=-1*/) const
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

void CDynamicsContainer::getMujocoDefaultFloatParams(std::vector<double> &p, int defType /*=-1*/) const
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
}

void CDynamicsContainer::getMujocoDefaultIntParams(std::vector<int> &p, int defType /*=-1*/) const
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
    p.push_back(0);                       // simi_mujoco_global_integrator, Euler
    p.push_back(2);                       // simi_mujoco_global_solver, Newton
    p.push_back(5000);                    // simi_mujoco_global_njmax
    p.push_back(2000);                    // simi_mujoco_global_nconmax
    p.push_back(0);                       // simi_mujoco_global_cone, pyramidal
    p.push_back(0);                       // simi_mujoco_global_overridekin, do not override
    p.push_back(-1);                      // simi_mujoco_global_nstack
    p.push_back(1 + 2 + 4 + 8 + 16 + 32); // simi_mujoco_global_rebuildtrigger
}

#ifdef SIM_WITH_GUI
void CDynamicsContainer::renderYour3DStuff(CViewableBase *renderingObject, int displayAttrib)
{ // Has to be displayed as overlay!
    if (isWorldThere())
    {
        if ((displayAttrib & sim_displayattribute_noparticles) == 0)
        {
            int index = 0;
            float *cols;
            int objectType;
            int particlesCount;
            C4X4Matrix m(renderingObject->getFullCumulativeTransformation().getMatrix());
            void **particlesPointer =
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

void CDynamicsContainer::renderYour3DStuff_overlay(CViewableBase *renderingObject, int displayAttrib)
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
                    double *pts = App::worldContainer->pluginContainer->dyn_getContactPoints(&cnt);

                    displayContactPoints(displayAttrib, contactPointColor, pts, cnt);
                }
            }
        }
    }
}
#endif

void CDynamicsContainer::pushGenesisEvents()
{
    CCbor *ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, nullptr, false);

    // Engine properties:
    setBoolProperty(nullptr, false, ev);
    setIntProperty(nullptr, 0, ev);
    setFloatProperty(nullptr, 0.0, ev);
    setVector3Property(nullptr, nullptr, ev);
    setVectorProperty(nullptr, nullptr, 0, ev);
    _sendEngineString(ev);

    App::worldContainer->pushEvent();
}

int CDynamicsContainer::setBoolProperty(const char* pName, bool pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ( (eev == nullptr) && (pName != nullptr) )
    { // regular properties (i.e. non-engine properties)
    }

    // Following only for engine properties:
    // -------------------------------------
    auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndexBitCoded, int simiIndex)
    {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            int nv = (arr[simiIndexBitCoded] | simiIndex) - (1 - pState) * simiIndex;
            if ( (nv != arr[simiIndexBitCoded]) ||(pName == nullptr) )
            {
                if (pName != nullptr)
                    arr[simiIndexBitCoded] = nv;
                if (App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propertyName.c_str(), true);
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

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    // -------------------------------------

    return retVal;
}

int CDynamicsContainer::getBoolProperty(const char* pName, bool& pState) const
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
    if (strcmp(pName, propDyn_bulletComputeInertias.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_global_computeinertias];
    }
    else if (strcmp(pName, propDyn_bulletInternalScalingFull.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_global_fullinternalscaling];
    }
    else if (strcmp(pName, propDyn_odeQuickStepEnabled.name) == 0)
    {
        retVal = 1;
        pState = _odeIntParams[simi_ode_global_quickstep];
    }
    else if (strcmp(pName, propDyn_odeComputeInertias.name) == 0)
    {
        retVal = 1;
        pState = _odeIntParams[simi_ode_global_computeinertias];
    }
    else if (strcmp(pName, propDyn_odeInternalScalingFull.name) == 0)
    {
        retVal = 1;
        pState = _odeIntParams[simi_ode_global_fullinternalscaling];
    }
    else if (strcmp(pName, propDyn_vortexComputeInertias.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_global_computeinertias];
    }
    else if (strcmp(pName, propDyn_vortexAutoSleep.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_global_autosleep];
    }
    else if (strcmp(pName, propDyn_vortexMultithreading.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_global_multithreading];
    }
    else if (strcmp(pName, propDyn_newtonComputeInertias.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_global_computeinertias];
    }
    else if (strcmp(pName, propDyn_newtonMultithreading.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_global_multithreading];
    }
    else if (strcmp(pName, propDyn_newtonExactSolver.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_global_exactsolver];
    }
    else if (strcmp(pName, propDyn_newtonHighJointAccuracy.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_global_highjointaccuracy];
    }
    else if (strcmp(pName, propDyn_mujocoComputeInertias.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_computeinertias];
    }
    else if (strcmp(pName, propDyn_mujocoBalanceInertias.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_balanceinertias];
    }
    else if (strcmp(pName, propDyn_mujocoMultithreaded.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_multithreaded];
    }
    else if (strcmp(pName, propDyn_mujocoMulticcd.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_multiccd];
    }
    else if (strcmp(pName, propDyn_mujocoContactParamsOverride.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_overridecontacts];
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setIntProperty(const char* pName, int pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ( (eev == nullptr) && (pName != nullptr) )
    { // regular properties (i.e. non-engine properties)
    }

    // Following only for engine properties:
    // -------------------------------------
    auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndex)
    {
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
                        ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propertyName.c_str(), true);
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
    handleProp(propDyn_mujocoNjMax.name, _mujocoIntParams, simi_mujoco_global_njmax);
    handleProp(propDyn_mujocoNconMax.name, _mujocoIntParams, simi_mujoco_global_nconmax);
    handleProp(propDyn_mujocoNstack.name, _mujocoIntParams, simi_mujoco_global_nstack);
    handleProp(propDyn_mujocoCone.name, _mujocoIntParams, simi_mujoco_global_cone);
    handleProp(propDyn_mujocoKinematicBodiesOverrideFlags.name, _mujocoIntParams, simi_mujoco_global_overridekin);

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    // -------------------------------------

    return retVal;
}

int CDynamicsContainer::getIntProperty(const char* pName, int& pState) const
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
    if (strcmp(pName, propDyn_bulletSolver.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_global_constraintsolvertype];
    }
    else if (strcmp(pName, propDyn_bulletIterations.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_global_constraintsolvingiterations];
    }
    else if (strcmp(pName, propDyn_odeQuickStepIterations.name) == 0)
    {
        retVal = 1;
        pState = _odeIntParams[simi_ode_global_constraintsolvingiterations];
    }
    else if (strcmp(pName, propDyn_newtonIterations.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_global_constraintsolvingiterations];
    }
    else if (strcmp(pName, propDyn_mujocoIntegrator.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_integrator];
    }
    else if (strcmp(pName, propDyn_mujocoSolver.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_solver];
    }
    else if (strcmp(pName, propDyn_mujocoIterations.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_iterations];
    }
    else if (strcmp(pName, propDyn_mujocoRebuildTrigger.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_rebuildtrigger];
    }
    else if (strcmp(pName, propDyn_mujocoNjMax.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_njmax];
    }
    else if (strcmp(pName, propDyn_mujocoNconMax.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_nconmax];
    }
    else if (strcmp(pName, propDyn_mujocoNstack.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_nstack];
    }
    else if (strcmp(pName, propDyn_mujocoCone.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_cone];
    }
    else if (strcmp(pName, propDyn_mujocoKinematicBodiesOverrideFlags.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_global_overridekin];
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setFloatProperty(const char* pName, double pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ( (eev == nullptr) && (pName != nullptr) )
    { // regular properties (i.e. non-engine properties)
    }

    // Following only for engine properties:
    // -------------------------------------
    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex)
    {
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
                        ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propertyName.c_str(), true);
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

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    // -------------------------------------

    return retVal;
}

int CDynamicsContainer::getFloatProperty(const char* pName, double& pState) const
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
    if (strcmp(pName, propDyn_bulletInternalScalingScaling.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_global_internalscalingfactor];
    }
    else if (strcmp(pName, propDyn_bulletCollMarginScaling.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_global_collisionmarginfactor];
    }
    else if (strcmp(pName, propDyn_odeInternalScalingScaling.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_global_internalscalingfactor];
    }
    else if (strcmp(pName, propDyn_odeGlobalErp.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_global_erp];
    }
    else if (strcmp(pName, propDyn_odeGlobalCfm.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_global_cfm];
    }
    else if (strcmp(pName, propDyn_vortexContactTolerance.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_contacttolerance];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsLinearCompliance.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintlinearcompliance];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsLinearDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintlineardamping];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsLinearKineticLoss.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintlinearkineticloss];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsAngularCompliance.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintangularcompliance];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsAngularDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintangulardamping];
    }
    else if (strcmp(pName, propDyn_vortexConstraintsAngularKineticLoss.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_global_constraintangularkineticloss];
    }
    else if (strcmp(pName, propDyn_newtonContactMergeTolerance.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_global_contactmergetolerance];
    }
    else if (strcmp(pName, propDyn_mujocoKinematicBodiesMass.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_kinmass];
    }
    else if (strcmp(pName, propDyn_mujocoKinematicBodiesInertia.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_kininertia];
    }
    else if (strcmp(pName, propDyn_mujocoBoundMass.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_boundmass];
    }
    else if (strcmp(pName, propDyn_mujocoBoundInertia.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_boundinertia];
    }
    else if (strcmp(pName, propDyn_mujocoContactParamsMargin.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_overridemargin];
    }
    else if (strcmp(pName, propDyn_mujocoImpRatio.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_impratio];
    }
    else if (strcmp(pName, propDyn_mujocoDensity.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_density];
    }
    else if (strcmp(pName, propDyn_mujocoViscosity.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_global_viscosity];
    }
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setStringProperty(const char* pName, const char* pState)
{
    int retVal = -1;
    if (strcmp(pName, propGlobal_engineProperties.name) == 0)
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
    return retVal;
}

int CDynamicsContainer::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = -1;
    if (strcmp(pName, propGlobal_engineProperties.name) == 0)
    {
        retVal = 1;
        CEngineProperties prop;
        pState = prop.getObjectProperties(-1);
    }

    return retVal;
}

int CDynamicsContainer::setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ( (eev == nullptr) && (pName != nullptr) )
    { // regular properties (i.e. non-engine properties)
    }

    // Following only for engine properties:
    // -------------------------------------
    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1)
    {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            bool pa = false;
            for (size_t i = 0; i < 3; i++)
                pa = pa || ((pState != nullptr) && (arr[simiIndex1 + i] != pState->data[i]));
            if ( (pName == nullptr) || pa )
            {
                if (pName != nullptr)
                {
                    for (size_t i = 0; i < 3; i++)
                        arr[simiIndex1 + i] = pState->data[i];
                }
                if (App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propertyName.c_str(), true);
                    ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 3);
                    if (pName != nullptr)
                        _sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propDyn_mujocoWind.name, _mujocoFloatParams, simi_mujoco_global_wind1);

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    // -------------------------------------

    return retVal;
}

int CDynamicsContainer::getVector3Property(const char* pName, C3Vector& pState) const
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
    auto handleProp = [&](const std::vector<double>& arr, int simiIndex1)
    {
        retVal = 1;
        for (size_t i = 0; i < 3; i++)
            pState(i) = arr[simiIndex1 + i];
    };

    if (strcmp(pName, propDyn_mujocoWind.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_global_wind1);
    // ------------------------

    return retVal;
}

int CDynamicsContainer::setVectorProperty(const char* pName, const double* v, int vL, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    if ( (eev == nullptr) && (pName != nullptr) )
    { // regular properties (i.e. non-engine properties)
    }

    // Following only for engine properties:
    // -------------------------------------
    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1, size_t n)
    {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            bool pa = false;
            for (size_t i = 0; i < n; i++)
                pa = pa || ((vL > i) && (arr[simiIndex1 + i] != v[i]));
            if ( (pName == nullptr) || pa )
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
                        ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propertyName.c_str(), true);
                    ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, n);
                    if (pName != nullptr)
                        _sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propDyn_mujocoContactParamsSolref.name, _mujocoFloatParams, simi_mujoco_global_overridesolref1, 2);
    handleProp(propDyn_mujocoContactParamsSolimp.name, _mujocoFloatParams, simi_mujoco_global_overridesolimp1, 5);

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    // -------------------------------------

    return retVal;
}

int CDynamicsContainer::getVectorProperty(const char* pName, std::vector<double>& pState) const
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
    auto handleProp = [&](const std::vector<double>& arr, int simiIndex1, size_t n)
    {
        retVal = 1;
        for (size_t i = 0; i < n; i++)
            pState.push_back(arr[simiIndex1 + i]);
    };

    if (strcmp(pName, propDyn_mujocoContactParamsSolref.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_global_overridesolref1, 2);
    if (strcmp(pName, propDyn_mujocoContactParamsSolimp.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_global_overridesolimp1, 5);
    // ------------------------

    return retVal;
}

int CDynamicsContainer::getPropertyName(int& index, std::string& pName) const
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_dyn.size(); i++)
    {
        index--;
        if (index == -1)
        {
            pName = allProps_dyn[i].name;
            retVal = 1;
            break;
        }
    }
    return retVal;
}

int CDynamicsContainer::getPropertyInfo(const char* pName, int& info, int& size) const
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_dyn.size(); i++)
    {
        if (strcmp(allProps_dyn[i].name, pName) == 0)
        {
            retVal = allProps_dyn[i].type;
            info = allProps_dyn[i].flags;
            size = 0;
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
            ev = App::worldContainer->createEvent(EVENTTYPE_DYNSETTINGSCHANGED, -1, propGlobal_engineProperties.name, true);
        ev->appendKeyString(propGlobal_engineProperties.name, current.c_str());
        if ( (ev != nullptr) && (eev == nullptr) )
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
                    if ( (j > 0) || (allProps_dyn[i].oldEnums[j + 1] != -1) )
                        indexWithArrays = int(j);
                    else
                        indexWithArrays = -1;
                    retVal =  allProps_dyn[i].name;
                }
                break;
            }
        }
        if (retVal.size() > 0)
            break;
    }
    return retVal;
}

