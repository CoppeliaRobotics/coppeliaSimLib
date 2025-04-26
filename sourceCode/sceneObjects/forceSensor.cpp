#include <forceSensor.h>
#include <simInternal.h>
#include <tt.h>
#include <simStrings.h>
#include <algorithm>
#include <utils.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <forceSensorRendering.h>
#endif

CForceSensor::CForceSensor()
{
    commonInit();
}

void CForceSensor::commonInit()
{
    _objectType = sim_sceneobject_forcesensor;
    _forceThreshold = 100.0;
    _torqueThreshold = 10.0;
    _forceThresholdEnabled = false;
    _torqueThresholdEnabled = false;
    _consecutiveViolationsToTrigger = 10;
    _currentThresholdViolationCount = 0;
    _stillAutomaticallyBreaking = false;

    // Dynamic values:
    _intrinsicTransformationError.setIdentity();
    _filteredDynamicForces.clear();
    _filteredDynamicTorques.clear();
    _filteredValuesAreValid = false;

    _cumulativeForces_duringTimeStep.clear();
    _cumulativeTorques_duringTimeStep.clear();

    _lastForce_dynStep.clear();
    _lastTorque_dynStep.clear();
    _lastForceAndTorqueValid_dynStep = false;

    _forceSensorSize = 0.05;

    _filterSampleSize = 1;
    _filterType = 0; // average

    _color.setDefaultValues();
    _color.setColor(0.22f, 0.9f, 0.45f, sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.setDefaultValues();
    _color_removeSoon.setColor(0.22f, 0.22f, 0.22f, sim_colorcomponent_ambient_diffuse);
    _visibilityLayer = FORCE_SENSOR_LAYER;
    _localObjectSpecialProperty = 0;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
}

CForceSensor::~CForceSensor()
{
}

CColorObject* CForceSensor::getColor(bool part2)
{
    if (part2)
        return (&_color_removeSoon);
    return (&_color);
}

void CForceSensor::setFilterSampleSize(int c)
{
    c = tt::getLimitedInt(1, 1000, c);
    if (_filterSampleSize != c)
    {
        _filterSampleSize = c;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_filterSampleSize.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _filterSampleSize);
            App::worldContainer->pushEvent();
        }
    }
}

int CForceSensor::getFilterSampleSize() const
{
    return _filterSampleSize;
}

void CForceSensor::setFilterType(int t)
{
    if (t < 0)
        t = 0;
    if (t > 1)
        t = 1;
    if (_filterType != t)
    {
        _filterType = t;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_filterType.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _filterType);
            App::worldContainer->pushEvent();
        }
    }
}

int CForceSensor::getFilterType() const
{
    return _filterType;
}

bool CForceSensor::getStillAutomaticallyBreaking()
{
    bool retVal = _stillAutomaticallyBreaking;
    _stillAutomaticallyBreaking = false;
    return (retVal);
}

void CForceSensor::setForceThreshold(double t)
{
    t = tt::getLimitedFloat(0.0, 10000000000.0, t);
    if (_forceThreshold != t)
    {
        _forceThreshold = t;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_forceThreshold.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _forceThreshold);
            App::worldContainer->pushEvent();
        }
    }
}

double CForceSensor::getForceThreshold() const
{
    return _forceThreshold;
}

void CForceSensor::setTorqueThreshold(double t)
{
    t = tt::getLimitedFloat(0.0, 10000000000.0, t);
    if (_torqueThreshold != t)
    {
        _torqueThreshold = t;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_torqueThreshold.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _torqueThreshold);
            App::worldContainer->pushEvent();
        }
    }
}

double CForceSensor::getTorqueThreshold() const
{
    return _torqueThreshold;
}

void CForceSensor::setEnableForceThreshold(bool e)
{
    if (_forceThresholdEnabled != e)
    {
        _forceThresholdEnabled = e;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_forceThresholdEnabled.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _forceThresholdEnabled);
            App::worldContainer->pushEvent();
        }
    }
}

bool CForceSensor::getEnableForceThreshold() const
{
    return _forceThresholdEnabled;
}

void CForceSensor::setEnableTorqueThreshold(bool e)
{
    if (_torqueThresholdEnabled != e)
    {
        _torqueThresholdEnabled = e;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_torqueThresholdEnabled.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _torqueThresholdEnabled);
            App::worldContainer->pushEvent();
        }
    }
}

bool CForceSensor::getEnableTorqueThreshold() const
{
    return _torqueThresholdEnabled;
}

void CForceSensor::setConsecutiveViolationsToTrigger(int count)
{
    count = tt::getLimitedInt(1, 10000, count);
    if (_consecutiveViolationsToTrigger != count)
    {
        _consecutiveViolationsToTrigger = count;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_consecutiveViolationsToTrigger.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _consecutiveViolationsToTrigger);
            App::worldContainer->pushEvent();
        }
    }
}

int CForceSensor::getConsecutiveViolationsToTrigger() const
{
    return _consecutiveViolationsToTrigger;
}

void CForceSensor::addCumulativeForcesAndTorques(const C3Vector& f, const C3Vector& t, int countForAverage)
{   // the countForAverage mechanism is needed because we need to average all values in a simulation time step (but this
    // is called every dynamic simulation time step!!)
    _setForceAndTorque(true, &f, &t);
    if (countForAverage > 0)
    {
        _cumulatedForces_forFilter.push_back(_cumulativeForces_duringTimeStep / double(countForAverage));
        _cumulatedTorques_forFilter.push_back(_cumulativeTorques_duringTimeStep / double(countForAverage));
        _cumulativeForces_duringTimeStep.clear();
        _cumulativeTorques_duringTimeStep.clear();
        if (int(_cumulatedForces_forFilter.size()) > _filterSampleSize)
        { // we have too many values for the filter. Remove the first entry:
            _cumulatedForces_forFilter.erase(_cumulatedForces_forFilter.begin(), _cumulatedForces_forFilter.begin() + 1);
            _cumulatedTorques_forFilter.erase(_cumulatedTorques_forFilter.begin(), _cumulatedTorques_forFilter.begin() + 1);
        }
        _computeFilteredValues();
        _handleSensorTriggering();
    }
}

void CForceSensor::setForceAndTorqueNotValid()
{
    _setFilteredForceAndTorque(false);
    _setForceAndTorque(false);
}

void CForceSensor::_setForceAndTorque(bool valid, const C3Vector* f /*= nullptr*/, const C3Vector* t /*= nullptr*/)
{
    C3Vector vf(C3Vector::zeroVector);
    C3Vector vt(C3Vector::zeroVector);
    if (valid)
    {
        if (f != nullptr)
            vf = f[0];
        if (t != nullptr)
            vt = t[0];
        _cumulativeForces_duringTimeStep += f[0];
        _cumulativeTorques_duringTimeStep += t[0];
    }
    else
    {
        _cumulativeForces_duringTimeStep.clear();
        _cumulativeTorques_duringTimeStep.clear();
    }
    _lastForceAndTorqueValid_dynStep = valid;
    bool diff = ((_lastForce_dynStep != vf) || (_lastTorque_dynStep != vt));
    if (diff)
    {
        _lastForce_dynStep = vf;
        _lastTorque_dynStep = vt;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_sensorForce.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _lastForce_dynStep.data, 3);
            ev->appendKeyDoubleArray(propFSensor_sensorTorque.name, _lastTorque_dynStep.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CForceSensor::_setFilteredForceAndTorque(bool valid, const C3Vector* f /*= nullptr*/, const C3Vector* t /*= nullptr*/)
{
    C3Vector vf(C3Vector::zeroVector);
    C3Vector vt(C3Vector::zeroVector);
    if (valid)
    {
        if (f != nullptr)
            vf = f[0];
        if (t != nullptr)
            vt = t[0];
    }
    _filteredValuesAreValid = valid;
    bool diff = ((_filteredDynamicForces != vf) || (_filteredDynamicTorques != vt));
    if (diff)
    {
        _filteredDynamicForces = vf;
        _filteredDynamicTorques = vt;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_sensorAverageForce.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _filteredDynamicForces.data, 3);
            ev->appendKeyDoubleArray(propFSensor_sensorAverageTorque.name, _filteredDynamicTorques.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CForceSensor::_computeFilteredValues()
{
    if (int(_cumulatedForces_forFilter.size()) >= _filterSampleSize)
    {
        C3Vector f;
        C3Vector t;
        if (_filterType == 0)
        { // average filter
            C3Vector fo;
            C3Vector to;
            fo.clear();
            to.clear();
            for (int i = 0; i < _filterSampleSize; i++)
            {
                fo += _cumulatedForces_forFilter[i];
                to += _cumulatedTorques_forFilter[i];
            }
            f = fo / double(_filterSampleSize);
            t = to / double(_filterSampleSize);
        }
        if (_filterType == 1)
        {
            std::vector<double> fx;
            std::vector<double> fy;
            std::vector<double> fz;
            std::vector<double> tx;
            std::vector<double> ty;
            std::vector<double> tz;
            for (int i = 0; i < _filterSampleSize; i++)
            {
                fx.push_back(_cumulatedForces_forFilter[i](0));
                fy.push_back(_cumulatedForces_forFilter[i](1));
                fz.push_back(_cumulatedForces_forFilter[i](2));
                tx.push_back(_cumulatedTorques_forFilter[i](0));
                ty.push_back(_cumulatedTorques_forFilter[i](1));
                tz.push_back(_cumulatedTorques_forFilter[i](2));
            }
            std::sort(fx.begin(), fx.end());
            std::sort(fy.begin(), fy.end());
            std::sort(fz.begin(), fz.end());
            std::sort(tx.begin(), tx.end());
            std::sort(ty.begin(), ty.end());
            std::sort(tz.begin(), tz.end());
            int ind = _filterSampleSize / 2;
            f(0) = fx[ind];
            f(1) = fy[ind];
            f(2) = fz[ind];
            t(0) = tx[ind];
            t(1) = ty[ind];
            t(2) = tz[ind];
        }
        _setFilteredForceAndTorque(true, &f, &t);
    }
    else
        _setFilteredForceAndTorque(false);
}

bool CForceSensor::getDynamicForces(C3Vector& f, bool dynamicStepValue) const
{
    if (dynamicStepValue)
    {
        if (App::currentWorld->dynamicsContainer->getCurrentlyInDynamicsCalculations())
        {
            if (!_lastForceAndTorqueValid_dynStep)
                return (false);
            f = _lastForce_dynStep;
            return (true);
        }
        return (false);
    }
    else
    {
        if ((!_filteredValuesAreValid)) //||(!_dynamicSecondPartIsValid) )
            return (false);
        f = _filteredDynamicForces;
        return (true);
    }
}

bool CForceSensor::getDynamicTorques(C3Vector& t, bool dynamicStepValue) const
{
    if (dynamicStepValue)
    {
        if (App::currentWorld->dynamicsContainer->getCurrentlyInDynamicsCalculations())
        {
            if (!_lastForceAndTorqueValid_dynStep)
                return (false);
            t = _lastTorque_dynStep;
            return (true);
        }
        return (false);
    }
    else
    {
        if ((!_filteredValuesAreValid)) //||(!_dynamicSecondPartIsValid) )
            return (false);
        t = _filteredDynamicTorques;
        return (true);
    }
}

void CForceSensor::_handleSensorTriggering()
{
    if (_filteredValuesAreValid)
    {
        bool trigger = false;
        if (_forceThresholdEnabled && (_filteredDynamicForces.getLength() >= _forceThreshold))
            trigger = true;
        if (_torqueThresholdEnabled && (_filteredDynamicTorques.getLength() >= _torqueThreshold))
            trigger = true;
        if (trigger)
            _currentThresholdViolationCount++;
        else
            _currentThresholdViolationCount = 0;
        if (_currentThresholdViolationCount >= _consecutiveViolationsToTrigger)
        { // we need to break something!
            std::vector<CScriptObject*> scripts;
            getAttachedScripts(scripts, -1, true);
            getAttachedScripts(scripts, -1, false);
            if (scripts.size() > 0)
            {
                CInterfaceStack* inStack = App::worldContainer->interfaceStackContainer->createStack();
                inStack->pushTableOntoStack();
                inStack->insertKeyInt32IntoStackTable("handle", getObjectHandle());
                inStack->insertKeyDoubleArrayIntoStackTable("force", _lastForce_dynStep.data, 3);
                inStack->insertKeyDoubleArrayIntoStackTable("torque", _lastTorque_dynStep.data, 3);
                inStack->insertKeyDoubleArrayIntoStackTable("filteredForce", _filteredDynamicForces.data, 3);
                inStack->insertKeyDoubleArrayIntoStackTable("filteredTorque", _filteredDynamicTorques.data, 3);
                for (size_t i = 0; i < scripts.size(); i++)
                {
                    CScriptObject* script = scripts[i];
                    if (script->hasSystemFunctionOrHook(sim_syscb_trigger))
                    {
                        if (!script->getThreadedExecution_oldThreads())
                            script->systemCallScript(sim_syscb_trigger, inStack, nullptr);
                    }
                }
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
            _currentThresholdViolationCount = 0;
        }
    }
}

C7Vector CForceSensor::getIntrinsicTransformation(bool includeDynErrorComponent, bool* available /*=nullptr*/) const
{ // Overridden from CSceneObject
    C7Vector retVal;
    retVal.setIdentity();
    if (includeDynErrorComponent)
        retVal = _intrinsicTransformationError;
    if (available != nullptr)
        available[0] = true;
    return (retVal);
}

C7Vector CForceSensor::getFullLocalTransformation() const
{ // Overridden from CSceneObject
    return (_localTransformation * getIntrinsicTransformation(true));
}

void CForceSensor::setIntrinsicTransformationError(const C7Vector& tr)
{
    bool diff = (_intrinsicTransformationError != tr);
    if (diff)
    {
        _intrinsicTransformationError = tr;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_intrinsicError.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            double p[7];
            _intrinsicTransformationError.getData(p, true);
            ev->appendKeyDoubleArray(cmd, p, 7);
            App::worldContainer->pushEvent();
        }
    }
}

void CForceSensor::getDynamicErrorsFull(C3Vector& linear, C3Vector& angular) const
{
    linear = _intrinsicTransformationError.X;
    angular = _intrinsicTransformationError.Q.getEulerAngles();
}

double CForceSensor::getDynamicPositionError() const
{
    return _intrinsicTransformationError.X.getLength();
}

double CForceSensor::getDynamicOrientationError() const
{
    return _intrinsicTransformationError.Q.getAngleBetweenQuaternions(C4Vector::identityRotation);
}

void CForceSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _currentThresholdViolationCount = 0;
    _setFilteredForceAndTorque(false);
    _setForceAndTorque(false);
    _cumulatedForces_forFilter.clear();
    _cumulatedTorques_forFilter.clear();
    setIntrinsicTransformationError(C7Vector::identityTransformation);
}

void CForceSensor::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CForceSensor::simulationEnded()
{   // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
        }
    }
    _setForceAndTorque(false);
    _setFilteredForceAndTorque(false);
    _cumulatedForces_forFilter.clear();
    _cumulatedTorques_forFilter.clear();
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    CSceneObject::simulationEnded();
}

std::string CForceSensor::getObjectTypeInfo() const
{
    return "forceSensor";
}
std::string CForceSensor::getObjectTypeInfoExtended() const
{
    std::string retVal(getObjectTypeInfo());

    double lin = getDynamicPositionError();
    double ang = getDynamicOrientationError();
    if ((lin != 0.0) || (ang != 0.0))
    {
        retVal += "(dyn. err.: " + utils::getDoubleEString(false, lin);
        retVal += "/" + utils::getDoubleEString(false, ang * radToDeg);
        retVal += ")";
    }

    return retVal;
}

bool CForceSensor::isPotentiallyCollidable() const
{
    return (false);
}
bool CForceSensor::isPotentiallyMeasurable() const
{
    return (false);
}
bool CForceSensor::isPotentiallyDetectable() const
{
    return (false);
}
bool CForceSensor::isPotentiallyRenderable() const
{
    return (false);
}

void CForceSensor::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _forceSensorSize * 0.5);
}

void CForceSensor::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _color.setEventParams(true, _objectHandle);
    else
        _color.setEventParams(true, -1);
}

void CForceSensor::setForceSensorSize(double s)
{
    tt::limitValue(0.001, 10.0, s);
    if (_forceSensorSize != s)
    {
        _forceSensorSize = s;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propFSensor_size.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _forceSensorSize);
            App::worldContainer->pushEvent();
        }
    }
}

double CForceSensor::getForceSensorSize() const
{
    return (_forceSensorSize);
}

void CForceSensor::scaleObject(double scalingFactor)
{
    setForceSensorSize(_forceSensorSize * scalingFactor);
    setForceThreshold(_forceThreshold * scalingFactor * scalingFactor * scalingFactor);                   //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
    setTorqueThreshold(_torqueThreshold * scalingFactor * scalingFactor * scalingFactor * scalingFactor); //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity which doesn't change
    _setFilteredForceAndTorque(false);
    _setForceAndTorque(false);
    _cumulatedForces_forFilter.clear();
    _cumulatedTorques_forFilter.clear();
    CSceneObject::scaleObject(scalingFactor);
}

void CForceSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CForceSensor::addSpecializedObjectEventData(CCbor* ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->openKeyArray("colors");
    float c[9];
    _color.getColor(c, sim_colorcomponent_ambient_diffuse);
    _color.getColor(c + 3, sim_colorcomponent_specular);
    _color.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    _color_removeSoon.getColor(c, sim_colorcomponent_ambient_diffuse);
    _color_removeSoon.getColor(c + 3, sim_colorcomponent_specular);
    _color_removeSoon.getColor(c + 6, sim_colorcomponent_emission);
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
#else
    _color.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propFSensor_size.name, _forceSensorSize);
    double p[7];
    _intrinsicTransformationError.getData(p, true);
    ev->appendKeyDoubleArray(propFSensor_intrinsicError.name, p, 7);
    ev->appendKeyDoubleArray(propFSensor_sensorForce.name, _lastForce_dynStep.data, 3);
    ev->appendKeyDoubleArray(propFSensor_sensorTorque.name, _lastTorque_dynStep.data, 3);
    ev->appendKeyDoubleArray(propFSensor_sensorAverageForce.name, _filteredDynamicForces.data, 3);
    ev->appendKeyDoubleArray(propFSensor_sensorAverageTorque.name, _filteredDynamicTorques.data, 3);
    ev->appendKeyBool(propFSensor_forceThresholdEnabled.name, _forceThresholdEnabled);
    ev->appendKeyBool(propFSensor_torqueThresholdEnabled.name, _torqueThresholdEnabled);
    ev->appendKeyInt(propFSensor_filterType.name, _filterType);
    ev->appendKeyInt(propFSensor_filterSampleSize.name, _filterSampleSize);
    ev->appendKeyInt(propFSensor_consecutiveViolationsToTrigger.name, _consecutiveViolationsToTrigger);
    ev->appendKeyDouble(propFSensor_forceThreshold.name, _forceThreshold);
    ev->appendKeyDouble(propFSensor_torqueThreshold.name, _torqueThreshold);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // forceSensor
#endif
}

CSceneObject* CForceSensor::copyYourself()
{
    CForceSensor* newForceSensor = (CForceSensor*)CSceneObject::copyYourself();

    newForceSensor->_forceSensorSize = _forceSensorSize;
    newForceSensor->_forceThreshold = _forceThreshold;
    newForceSensor->_torqueThreshold = _torqueThreshold;
    newForceSensor->_forceThresholdEnabled = _forceThresholdEnabled;
    newForceSensor->_torqueThresholdEnabled = _torqueThresholdEnabled;
    newForceSensor->_consecutiveViolationsToTrigger = _consecutiveViolationsToTrigger;
    newForceSensor->_currentThresholdViolationCount = _currentThresholdViolationCount;

    newForceSensor->_filterSampleSize = _filterSampleSize;
    newForceSensor->_filterType = _filterType;

    _color.copyYourselfInto(&newForceSensor->_color);

    return (newForceSensor);
}

void CForceSensor::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CForceSensor::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void CForceSensor::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CForceSensor::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CForceSensor::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}
void CForceSensor::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performObjectLoadingMapping(map, opType);
}
void CForceSensor::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollectionLoadingMapping(map, opType);
}
void CForceSensor::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performCollisionLoadingMapping(map, opType);
}
void CForceSensor::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performDistanceLoadingMapping(map, opType);
}
void CForceSensor::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performIkLoadingMapping(map, opType);
}

void CForceSensor::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    CSceneObject::performTextureObjectLoadingMapping(map, opType);
}

void CForceSensor::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CForceSensor::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing.
            ar.storeDataName("_iz");
            ar << _forceSensorSize;
            ar.flush();

            ar.storeDataName("Vab");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _forceThresholdEnabled);
            SIM_SET_CLEAR_BIT(dummy, 1, _torqueThresholdEnabled);
            //      SIM_SET_CLEAR_BIT(dummy,2,_breaksPositionConstraints); // removed on 2010/02/06
            //      SIM_SET_CLEAR_BIT(dummy,3,_breaksOrientationConstraints); // removed on 2010/02/06
            ar << dummy;
            ar.flush();

            ar.storeDataName("_ri");
            ar << _forceThreshold << _torqueThreshold;
            ar << _consecutiveViolationsToTrigger;
            ar.flush();

            ar.storeDataName("Fil");
            ar << _filterSampleSize << _filterType;
            ar.flush();

            // "Bus" is reserved keyword since 2010/10/09       ar.storeDataName("Bus");

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            _color.serialize(ar, 0);
            if (ar.setWritingMode())
                _color.serialize(ar, 0);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Siz") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a;
                        ar >> a;
                        _forceSensorSize = (double)a;
                        ;
                    }

                    if (theName.compare("_iz") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _forceSensorSize;
                    }

                    if (theName.compare("Cl1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _color.serialize(ar, 0);
                    }
                    if (theName.compare("Vab") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _forceThresholdEnabled = SIM_IS_BIT_SET(dummy, 0);
                        _torqueThresholdEnabled = SIM_IS_BIT_SET(dummy, 1);
                    }
                    if (theName.compare("Tri") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        ar >> _consecutiveViolationsToTrigger;
                        _forceThreshold = (double)bla;
                        _torqueThreshold = (double)bli;
                    }

                    if (theName.compare("_ri") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _forceThreshold >> _torqueThreshold;
                        ar >> _consecutiveViolationsToTrigger;
                    }

                    if (theName.compare("Tre") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        ar >> _consecutiveViolationsToTrigger;
                        _forceThreshold = (double)bla;
                        _torqueThreshold = (double)bli;
                        _stillAutomaticallyBreaking = true;
                    }
                    if (theName.compare("Fil") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _filterSampleSize >> _filterType;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile() < 17)
                utils::scaleColorUp_(_color.getColorsPtr());
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("size", _forceSensorSize);

            ar.xmlPushNewNode("filter");
            ar.xmlAddNode_int("sampleSize", _filterSampleSize);
            ar.xmlAddNode_bool("averageFilter", _filterType == 0); // 0=average, 1=median
            ar.xmlPopNode();

            ar.xmlPushNewNode("force");
            ar.xmlAddNode_bool("thresholdEnabled", _forceThresholdEnabled);
            ar.xmlAddNode_float("threshold", _forceThreshold);
            ar.xmlPopNode();

            ar.xmlPushNewNode("torque");
            ar.xmlAddNode_bool("threasholdEnabled", _torqueThresholdEnabled);
            ar.xmlAddNode_float("threshold", _torqueThreshold);
            ar.xmlPopNode();

            ar.xmlAddNode_int("consecutiveThresholdViolationsForTrigger", _consecutiveViolationsToTrigger);

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("objectColor");
                _color.serialize(ar, 0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_color.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("objectColor", rgb, 3);
            }
        }
        else
        {
            if (ar.xmlGetNode_float("size", _forceSensorSize, exhaustiveXml))
                setForceSensorSize(_forceSensorSize);

            if (ar.xmlPushChildNode("filter", exhaustiveXml))
            {
                if (ar.xmlGetNode_int("sampleSize", _filterSampleSize, exhaustiveXml))
                    setFilterSampleSize(_filterSampleSize);
                bool avg;
                if (ar.xmlGetNode_bool("averageFilter", avg, exhaustiveXml))
                {
                    _filterType = 0;
                    if (!avg)
                        _filterType = 1;
                }
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("force", exhaustiveXml))
            {
                ar.xmlGetNode_bool("thresholdEnabled", _forceThresholdEnabled, exhaustiveXml);
                if (ar.xmlGetNode_float("threshold", _forceThreshold, exhaustiveXml))
                    setForceThreshold(_forceThreshold);
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("torque", exhaustiveXml))
            {
                ar.xmlGetNode_bool("threasholdEnabled", _torqueThresholdEnabled);
                if (ar.xmlGetNode_float("threshold", _torqueThreshold, exhaustiveXml))
                    setTorqueThreshold(_torqueThreshold);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_int("consecutiveThresholdViolationsForTrigger", _consecutiveViolationsToTrigger,
                              exhaustiveXml);

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("objectColor", false))
                {
                    _color.serialize(ar, 0);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("objectColor", rgb, 3, false))
                    _color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                    sim_colorcomponent_ambient_diffuse);
            }

            if (ar.xmlPushChildNode("color", false))
            { // for backward compatibility
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("part1"))
                    {
                        _color.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("part2"))
                    {
                        _color_removeSoon.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("part1", rgb, 3, exhaustiveXml))
                        _color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                        sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlGetNode_ints("part2", rgb, 3, exhaustiveXml))
                        _color_removeSoon.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                                   sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }
            computeBoundingBox();
        }
    }
}

#ifdef SIM_WITH_GUI
void CForceSensor::display(CViewableBase* renderingObject, int displayAttrib)
{
    displayForceSensor(this, renderingObject, displayAttrib);
}
#endif

int CForceSensor::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_forceThresholdEnabled.name)
        {
            retVal = 1;
            setEnableForceThreshold(pState);
        }
        else if (_pName == propFSensor_torqueThresholdEnabled.name)
        {
            retVal = 1;
            setEnableTorqueThreshold(pState);
        }
    }

    return retVal;
}

int CForceSensor::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_forceThresholdEnabled.name)
        {
            retVal = 1;
            pState = _forceThresholdEnabled;
        }
        else if (_pName == propFSensor_torqueThresholdEnabled.name)
        {
            retVal = 1;
            pState = _torqueThresholdEnabled;
        }
    }

    return retVal;
}

int CForceSensor::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_filterType.name)
        {
            retVal = 1;
            setFilterType(pState);
        }
        else if (_pName == propFSensor_filterSampleSize.name)
        {
            retVal = 1;
            setFilterSampleSize(pState);
        }
        else if (_pName == propFSensor_consecutiveViolationsToTrigger.name)
        {
            retVal = 1;
            setConsecutiveViolationsToTrigger(pState);
        }
    }

    return retVal;
}

int CForceSensor::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_filterType.name)
        {
            retVal = 1;
            pState = _filterType;
        }
        else if (_pName == propFSensor_filterSampleSize.name)
        {
            retVal = 1;
            pState = _filterSampleSize;
        }
        else if (_pName == propFSensor_consecutiveViolationsToTrigger.name)
        {
            retVal = 1;
            pState = _consecutiveViolationsToTrigger;
        }
    }

    return retVal;
}

int CForceSensor::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_size.name)
        {
            setForceSensorSize(pState);
            retVal = 1;
        }
        else if (_pName == propFSensor_forceThreshold.name)
        {
            setForceThreshold(pState);
            retVal = 1;
        }
        else if (_pName == propFSensor_torqueThreshold.name)
        {
            setTorqueThreshold(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CForceSensor::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_size.name)
        {
            pState = _forceSensorSize;
            retVal = 1;
        }
        else if (_pName == propFSensor_forceThreshold.name)
        {
            pState = _forceThreshold;
            retVal = 1;
        }
        else if (_pName == propFSensor_torqueThreshold.name)
        {
            pState = _torqueThreshold;
            retVal = 1;
        }
    }

    return retVal;
}

int CForceSensor::setVector3Property(const char* ppName, const C3Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setVector3Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CForceSensor::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getVector3Property(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_sensorAverageForce.name)
        {
            pState = _filteredDynamicForces;
            retVal = 1;
        }
        else if (_pName == propFSensor_sensorAverageTorque.name)
        {
            pState = _filteredDynamicTorques;
            retVal = 1;
        }
        else if (_pName == propFSensor_sensorForce.name)
        {
            pState = _lastForce_dynStep;
            retVal = 1;
        }
        else if (_pName == propFSensor_sensorTorque.name)
        {
            pState = _lastTorque_dynStep;
            retVal = 1;
        }
    }

    return retVal;
}

int CForceSensor::setPoseProperty(const char* ppName, const C7Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setPoseProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CForceSensor::getPoseProperty(const char* ppName, C7Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPoseProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propFSensor_intrinsicError.name)
        {
            retVal = 1;
            pState = _intrinsicTransformationError;
        }
    }

    return retVal;
}

int CForceSensor::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.setColorProperty(pName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CForceSensor::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.getColorProperty(pName, pState);
    if (retVal != -1)
    {
    }
    return retVal;
}

int CForceSensor::getPropertyName(int& index, std::string& pName, std::string& appartenance) const
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".forceSensor";
        retVal = _color.getPropertyName(index, pName);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_forceSensor[i].name, pName.c_str()))
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_forceSensor[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CForceSensor::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".forceSensor";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_forceSensor[i].name, pName.c_str()))
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_forceSensor[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CForceSensor::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, infoTxt);
    if (retVal == -1)
        retVal = _color.getPropertyInfo(pName, info, infoTxt);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if (strcmp(allProps_forceSensor[i].name, pName) == 0)
            {
                retVal = allProps_forceSensor[i].type;
                info = allProps_forceSensor[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_forceSensor[i].infoTxt, "") != 0))
                    infoTxt = allProps_forceSensor[i].infoTxt;
                else
                    infoTxt = allProps_forceSensor[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}

int CForceSensor::getPropertyInfo_static(const char* ppName, int& info, std::string& infoTxt)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, infoTxt);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, infoTxt, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if (strcmp(allProps_forceSensor[i].name, pName) == 0)
            {
                retVal = allProps_forceSensor[i].type;
                info = allProps_forceSensor[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_forceSensor[i].infoTxt, "") != 0))
                    infoTxt = allProps_forceSensor[i].infoTxt;
                else
                    infoTxt = allProps_forceSensor[i].shortInfoTxt;
                break;
            }
        }
    }
    return retVal;
}
