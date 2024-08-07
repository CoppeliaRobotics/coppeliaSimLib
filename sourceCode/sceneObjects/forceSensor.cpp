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
    _consecutiveThresholdViolationsForBreaking = 10;
    _currentThresholdViolationCount = 0;
    _stillAutomaticallyBreaking = false;

    // Dynamic values:
    _intrinsicTransformationError.setIdentity();
    _filteredDynamicForces.clear();
    _filteredDynamicTorques.clear();

    _cumulativeForcesTmp.clear();
    _cumulativeTorquesTmp.clear();

    _lastForce_dynStep.clear();
    _lastTorque_dynStep.clear();
    _lastForceAndTorqueValid_dynStep = false;

    _forceSensorSize = 0.05;

    _valueCountForFilter = 1;
    _filterType = 0; // average
    _filteredValuesAreValid = false;

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

CColorObject *CForceSensor::getColor(bool part2)
{
    if (part2)
        return (&_color_removeSoon);
    return (&_color);
}

void CForceSensor::setValueCountForFilter(int c)
{
    _valueCountForFilter = tt::getLimitedInt(1, 1000, c);
}

int CForceSensor::getValueCountForFilter() const
{
    return (_valueCountForFilter);
}

void CForceSensor::setFilterType(int t)
{
    _filterType = t;
}

int CForceSensor::getFilterType() const
{
    return (_filterType);
}
bool CForceSensor::getStillAutomaticallyBreaking()
{
    bool retVal = _stillAutomaticallyBreaking;
    _stillAutomaticallyBreaking = false;
    return (retVal);
}
void CForceSensor::setForceThreshold(double t)
{
    _forceThreshold = tt::getLimitedFloat(0.0, 10000000000.0, t);
}
double CForceSensor::getForceThreshold() const
{
    return (_forceThreshold);
}
void CForceSensor::setTorqueThreshold(double t)
{
    _torqueThreshold = tt::getLimitedFloat(0.0, 10000000000.0, t);
}
double CForceSensor::getTorqueThreshold() const
{
    return (_torqueThreshold);
}
void CForceSensor::setEnableForceThreshold(bool e)
{
    _forceThresholdEnabled = e;
}
bool CForceSensor::getEnableForceThreshold() const
{
    return (_forceThresholdEnabled);
}
void CForceSensor::setEnableTorqueThreshold(bool e)
{
    _torqueThresholdEnabled = e;
}
bool CForceSensor::getEnableTorqueThreshold() const
{
    return (_torqueThresholdEnabled);
}
void CForceSensor::setConsecutiveThresholdViolationsForBreaking(int count)
{
    _consecutiveThresholdViolationsForBreaking = tt::getLimitedInt(1, 10000, count);
}
int CForceSensor::getConsecutiveThresholdViolationsForBreaking() const
{
    return (_consecutiveThresholdViolationsForBreaking);
}

void CForceSensor::addCumulativeForcesAndTorques(const C3Vector &f, const C3Vector &t, int countForAverage)
{ // the countForAverage mechanism is needed because we need to average all values in a simulation time step (but this
  // is called every dynamic simulation time step!!)
    _lastForce_dynStep = f;
    _lastTorque_dynStep = t;
    _lastForceAndTorqueValid_dynStep = true;
    _cumulativeForcesTmp += f;
    _cumulativeTorquesTmp += t;
    if (countForAverage > 0)
    {
        _cumulatedForces.push_back(_cumulativeForcesTmp / double(countForAverage));
        _cumulatedTorques.push_back(_cumulativeTorquesTmp / double(countForAverage));
        _cumulativeForcesTmp.clear();
        _cumulativeTorquesTmp.clear();
        if (int(_cumulatedForces.size()) > _valueCountForFilter)
        { // we have too many values for the filter. Remove the first entry:
            _cumulatedForces.erase(_cumulatedForces.begin(), _cumulatedForces.begin() + 1);
            _cumulatedTorques.erase(_cumulatedTorques.begin(), _cumulatedTorques.begin() + 1);
        }
        _computeFilteredValues();
        _handleSensorBreaking();
    }
}

void CForceSensor::setForceAndTorqueNotValid()
{
    _filteredValuesAreValid = false;
    _lastForceAndTorqueValid_dynStep = false;
}

void CForceSensor::_computeFilteredValues()
{
    if (int(_cumulatedForces.size()) >= _valueCountForFilter)
    {
        _filteredValuesAreValid = true;
        if (_filterType == 0)
        { // average filter
            C3Vector fo;
            C3Vector to;
            fo.clear();
            to.clear();
            for (int i = 0; i < _valueCountForFilter; i++)
            {
                fo += _cumulatedForces[i];
                to += _cumulatedTorques[i];
            }
            _filteredDynamicForces = fo / double(_valueCountForFilter);
            _filteredDynamicTorques = to / double(_valueCountForFilter);
        }
        if (_filterType == 1)
        {
            std::vector<double> fx;
            std::vector<double> fy;
            std::vector<double> fz;
            std::vector<double> tx;
            std::vector<double> ty;
            std::vector<double> tz;
            for (int i = 0; i < _valueCountForFilter; i++)
            {
                fx.push_back(_cumulatedForces[i](0));
                fy.push_back(_cumulatedForces[i](1));
                fz.push_back(_cumulatedForces[i](2));
                tx.push_back(_cumulatedTorques[i](0));
                ty.push_back(_cumulatedTorques[i](1));
                tz.push_back(_cumulatedTorques[i](2));
            }
            std::sort(fx.begin(), fx.end());
            std::sort(fy.begin(), fy.end());
            std::sort(fz.begin(), fz.end());
            std::sort(tx.begin(), tx.end());
            std::sort(ty.begin(), ty.end());
            std::sort(tz.begin(), tz.end());
            int ind = _valueCountForFilter / 2;
            _filteredDynamicForces(0) = fx[ind];
            _filteredDynamicForces(1) = fy[ind];
            _filteredDynamicForces(2) = fz[ind];
            _filteredDynamicTorques(0) = tx[ind];
            _filteredDynamicTorques(1) = ty[ind];
            _filteredDynamicTorques(2) = tz[ind];
        }
    }
    else
        _filteredValuesAreValid = false;
}

bool CForceSensor::getDynamicForces(C3Vector &f, bool dynamicStepValue) const
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

bool CForceSensor::getDynamicTorques(C3Vector &t, bool dynamicStepValue) const
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

void CForceSensor::_handleSensorBreaking()
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
        if (_currentThresholdViolationCount >= _consecutiveThresholdViolationsForBreaking)
        { // we need to break something!
            std::vector<CScriptObject*> scripts;
            getAttachedScripts(scripts, -1, true);
            getAttachedScripts(scripts, -1, false);
            if (scripts.size() > 0)
            {
                CInterfaceStack *inStack = App::worldContainer->interfaceStackContainer->createStack();
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

C7Vector CForceSensor::getIntrinsicTransformation(bool includeDynErrorComponent, bool *available /*=nullptr*/) const
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

void CForceSensor::setIntrinsicTransformationError(const C7Vector &tr)
{
    bool diff = (_intrinsicTransformationError != tr);
    if (diff)
    {
        _intrinsicTransformationError = tr;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = "intrinsicPose";
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            double p[7] = {tr.X(0), tr.X(1), tr.X(2), tr.Q(1), tr.Q(2), tr.Q(3), tr.Q(0)};
            ev->appendKeyDoubleArray(cmd, p, 7);
            App::worldContainer->pushEvent();
        }
    }
}

void CForceSensor::getDynamicErrorsFull(C3Vector &linear, C3Vector &angular) const
{
    linear = _intrinsicTransformationError.X;
    angular = _intrinsicTransformationError.Q.getEulerAngles();
}

double CForceSensor::getDynamicPositionError() const
{
    return (_intrinsicTransformationError.X.getLength());
}

double CForceSensor::getDynamicOrientationError() const
{
    return (_intrinsicTransformationError.Q.getAngleBetweenQuaternions(C4Vector::identityRotation));
}

void CForceSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _filteredDynamicForces.clear();
    _filteredDynamicTorques.clear();
    _currentThresholdViolationCount = 0;
    _filteredValuesAreValid = false;
    _lastForceAndTorqueValid_dynStep = false;
    _cumulatedForces.clear();
    _cumulatedTorques.clear();
    _cumulativeForcesTmp.clear();
    _cumulativeTorquesTmp.clear();
    setIntrinsicTransformationError(C7Vector::identityTransformation);
}

void CForceSensor::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CForceSensor::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
        }
    }
    _filteredValuesAreValid = false;
    _lastForceAndTorqueValid_dynStep = false;
    _cumulatedForces.clear();
    _cumulatedTorques.clear();
    _cumulativeForcesTmp.clear();
    _cumulativeTorquesTmp.clear();
    setIntrinsicTransformationError(C7Vector::identityTransformation);
    CSceneObject::simulationEnded();
}

std::string CForceSensor::getObjectTypeInfo() const
{
    return "forceSensor";
}
std::string CForceSensor::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
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
        _color.setEventParams(_objectHandle);
    else
        _color.setEventParams(-1);
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
            const char *cmd = propForceSensor_size.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
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
    _forceThreshold *=
        scalingFactor * scalingFactor * scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working
                                                       // against gravity which doesn't change
    _torqueThreshold *= scalingFactor * scalingFactor * scalingFactor *
                        scalingFactor; //*scalingFactor; removed one on 2010/02/17 b/c often working against gravity
                                       // which doesn't change
    _filteredValuesAreValid = false;
    _lastForceAndTorqueValid_dynStep = false;

    CSceneObject::scaleObject(scalingFactor);
}

void CForceSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CForceSensor::addSpecializedObjectEventData(CCbor *ev) const
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
    ev->appendKeyDouble(propForceSensor_size.name, _forceSensorSize);
    C7Vector tr(getIntrinsicTransformation(true));
    double p[7] = {tr.X(0), tr.X(1), tr.X(2), tr.Q(1), tr.Q(2), tr.Q(3), tr.Q(0)};
    ev->appendKeyDoubleArray("intrinsicPose", p, 7);
    // todo
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // forceSensor
#endif
}

CSceneObject *CForceSensor::copyYourself()
{
    CForceSensor *newForceSensor = (CForceSensor *)CSceneObject::copyYourself();

    newForceSensor->_forceSensorSize = _forceSensorSize;
    newForceSensor->_forceThreshold = _forceThreshold;
    newForceSensor->_torqueThreshold = _torqueThreshold;
    newForceSensor->_forceThresholdEnabled = _forceThresholdEnabled;
    newForceSensor->_torqueThresholdEnabled = _torqueThresholdEnabled;
    newForceSensor->_consecutiveThresholdViolationsForBreaking = _consecutiveThresholdViolationsForBreaking;
    newForceSensor->_currentThresholdViolationCount = _currentThresholdViolationCount;

    newForceSensor->_valueCountForFilter = _valueCountForFilter;
    newForceSensor->_filterType = _filterType;

    _color.copyYourselfInto(&newForceSensor->_color);

    return (newForceSensor);
}

void CForceSensor::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
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
void CForceSensor::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
}
void CForceSensor::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}
void CForceSensor::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}
void CForceSensor::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}
void CForceSensor::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CForceSensor::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CForceSensor::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CForceSensor::serialize(CSer &ar)
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
            ar << _consecutiveThresholdViolationsForBreaking;
            ar.flush();

            ar.storeDataName("Fil");
            ar << _valueCountForFilter << _filterType;
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
                        ar >> _consecutiveThresholdViolationsForBreaking;
                        _forceThreshold = (double)bla;
                        _torqueThreshold = (double)bli;
                    }

                    if (theName.compare("_ri") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _forceThreshold >> _torqueThreshold;
                        ar >> _consecutiveThresholdViolationsForBreaking;
                    }

                    if (theName.compare("Tre") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        ar >> _consecutiveThresholdViolationsForBreaking;
                        _forceThreshold = (double)bla;
                        _torqueThreshold = (double)bli;
                        _stillAutomaticallyBreaking = true;
                    }
                    if (theName.compare("Fil") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _valueCountForFilter >> _filterType;
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
            ar.xmlAddNode_int("sampleSize", _valueCountForFilter);
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

            ar.xmlAddNode_int("consecutiveThresholdViolationsForTrigger", _consecutiveThresholdViolationsForBreaking);

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
                if (ar.xmlGetNode_int("sampleSize", _valueCountForFilter, exhaustiveXml))
                    setValueCountForFilter(_valueCountForFilter);
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

            ar.xmlGetNode_int("consecutiveThresholdViolationsForTrigger", _consecutiveThresholdViolationsForBreaking,
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
void CForceSensor::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayForceSensor(this, renderingObject, displayAttrib);
}
#endif

int CForceSensor::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propForceSensor_size.name)
        {
            setForceSensorSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CForceSensor::getFloatProperty(const char* ppName, double& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _color.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propForceSensor_size.name)
        {
            pState = _forceSensorSize;
            retVal = 1;
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

int CForceSensor::getColorProperty(const char* ppName, float* pState)
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

int CForceSensor::getPropertyName(int& index, std::string& pName, std::string& appartenance)
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
            index--;
            if (index == -1)
            {
                pName = allProps_forceSensor[i].name;
                retVal = 1;
                break;
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
            index--;
            if (index == -1)
            {
                pName = allProps_forceSensor[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CForceSensor::getPropertyInfo(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = _color.getPropertyInfo(pName, info, size);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if (strcmp(allProps_forceSensor[i].name, pName) == 0)
            {
                retVal = allProps_forceSensor[i].type;
                info = allProps_forceSensor[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

int CForceSensor::getPropertyInfo_static(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "forceSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, size);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_forceSensor.size(); i++)
        {
            if (strcmp(allProps_forceSensor[i].name, pName) == 0)
            {
                retVal = allProps_forceSensor[i].type;
                info = allProps_forceSensor[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

