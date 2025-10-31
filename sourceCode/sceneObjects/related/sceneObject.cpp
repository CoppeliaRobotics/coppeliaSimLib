#include <simInternal.h>
#include <tt.h>
#include <shape.h>
#include <camera.h>
#include <graph.h>
#include <path_old.h>
#include <customData_old.h>
#include <visionSensor.h>
#include <mill.h>
#include <light.h>
#include <vDateTime.h>
#include <utils.h>
#include <boost/lexical_cast.hpp>
#include <app.h>
#include <jointObject.h>
#include <sceneObject.h>
#include <dummy.h>
#include <global.h>
#include <simFlavor.h>
#include <base64.h>
#include <boost/algorithm/string.hpp>
#ifdef SIM_WITH_GUI
#include <rendering.h>
#include <oGL.h>
#include <oglSurface.h>
#include <guiApp.h>
#endif

CSceneObject::CSceneObject()
{
    customObjectData_volatile.setItemsAreVolatile();
    _selected = false;
    _isInScene = false;
    _modelInvisible = false;
    _parentObject = nullptr;
    _childOrder = -1;
    _scriptExecPriority = sim_scriptexecorder_normal;
    _localTransformation.setIdentity();
    _parentObjectHandle_forSerializationOnly = -1;
    _objectHandle = -1;
    _beforeDeleteCallbackSent = false;
    _ikPluginCounterpartHandle = -1;
    _dnaString = utils::generateUniqueString();
    _assemblingLocalTransformation.setIdentity();
    _assemblingLocalTransformationIsUsed = false;
    _userScriptParameters = nullptr;
    _dynamicsTemporarilyDisabled = false;
    _calculatedModelProperty = 0;

    _authorizedViewableObjects = -1; // all
    _assemblyMatchValuesChild.push_back("default");
    _assemblyMatchValuesParent.push_back("default");
    _forceAlwaysVisible_tmp = false;

    _objectProperty = sim_objectproperty_selectable;
    _hierarchyColorIndex = -1;
    _collectionSelfCollisionIndicator = 0;
    _ignorePosAndCameraOrthoviewSize_forUndoRedo = false;

    _modelBase = false;
    _dynamicFlag = 0;

    _transparentObjectDistanceOffset = 0;
    _objectManipulationMode_flaggedForGridOverlay = 0;

    _measuredAngularVelocity_velocityMeasurement = 0.0;
    _measuredLinearVelocity_velocityMeasurement.clear();
    _measuredAngularVelocity3_velocityMeasurement.clear();
    _measuredAngularVelocityAxis_velocityMeasurement.clear();
    _previousPositionOrientationIsValid = false;

    _dynamicsTemporarilyDisabled = false;
    _initialValuesInitialized = false;
    _initialConfigurationMemorized = false;
    _objectMovementPreferredAxes = 0x023; // about Z and in the X-Y plane!       0x03f; // full
    _objectManipulationModeEventId = -1;

    _objectMovementOptions = 0;
    _objectMovementRelativity[0] = 0; // relative to world by default
    _objectMovementRelativity[1] = 2; // relative to own frame by default
    _objectMovementStepSize[0] = 0.0; // i.e. use default
    _objectMovementStepSize[1] = 0.0; // i.e. use default

    _customObjectData_old = nullptr;
    _localObjectSpecialProperty = 0;
    _modelProperty = sim_modelproperty_not_model;

    _dynamicSimulationIconCode = sim_dynamicsimicon_none;

    _uniquePersistentIdString = utils::generateUniqueAlphaNumericString(); // persistent
    _modelAcknowledgement = "";
    _objectTempAlias = "_*_object_*_";
    _objectTempName_old = "_*_object_*_";

    _specificLight = -1; // default, i.e. all lights

    _bbFrame.setIdentity();
    _bbHalfSize.clear();
    _sizeFactor = 1.0;
    _sizeValues[0] = 1.0;
    _sizeValues[1] = 1.0;
    _sizeValues[2] = 1.0;
    _dynamicsResetFlag = false;
}

CSceneObject::~CSceneObject()
{
    delete _customObjectData_old;
    delete _userScriptParameters;
}

void CSceneObject::setIgnorePosAndCameraOrthoviewSize_forUndoRedo(bool s)
{
    _ignorePosAndCameraOrthoviewSize_forUndoRedo = s;
}

void CSceneObject::setForceAlwaysVisible_tmp(bool force)
{
    _forceAlwaysVisible_tmp = force;
}

void CSceneObject::measureVelocity(double dt)
{
    C7Vector abs(getCumulativeTransformation());
    if (_previousPositionOrientationIsValid)
    {
        C3Vector lin, rot, rotAxis;
        lin = (abs.X - _previousAbsTransf_velocityMeasurement.X) * (1.0 / dt);
        double angle = abs.Q.getAngleBetweenQuaternions(_previousAbsTransf_velocityMeasurement.Q) / dt;
        rot = (_previousAbsTransf_velocityMeasurement.getInverse() * abs).Q.getEulerAngles() * (1.0 / dt);

        C4Vector AA(_previousAbsTransf_velocityMeasurement.Q);
        C4Vector BB(abs.Q);
        if (AA(0) * BB(0) + AA(1) * BB(1) + AA(2) * BB(2) + AA(3) * BB(3) < 0.0)
            AA = AA * -1.0;
        C4Vector r((AA.getInverse() * BB).getAngleAndAxis());
        rotAxis.setData(r(1), r(2), r(3));
        rotAxis = AA * rotAxis;
        rotAxis.normalize();
        rotAxis *= r(0) / dt;

        _setMeasuredVelocity(lin, rot, rotAxis, angle);
    }
    _previousAbsTransf_velocityMeasurement = abs;
    _previousPositionOrientationIsValid = true;
}

void CSceneObject::_setMeasuredVelocity(const C3Vector& lin, const C3Vector& ang, const C3Vector& rotAxis, double angle)
{
    bool diff = (_measuredLinearVelocity_velocityMeasurement != lin);
    diff = diff || (_measuredAngularVelocity3_velocityMeasurement != ang);
    diff = diff || (_measuredAngularVelocityAxis_velocityMeasurement != rotAxis);
    diff = diff || (_measuredAngularVelocity_velocityMeasurement != angle);
    if (diff)
    {
        _measuredLinearVelocity_velocityMeasurement = lin;
        _measuredAngularVelocity3_velocityMeasurement = ang;
        _measuredAngularVelocityAxis_velocityMeasurement = rotAxis;
        _measuredAngularVelocity_velocityMeasurement = angle;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_calcLinearVelocity.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDoubleArray(cmd, _measuredLinearVelocity_velocityMeasurement.data, 3);
            ev->appendKeyDoubleArray(propObject_calcRotationAxis.name, _measuredAngularVelocityAxis_velocityMeasurement.data, 3);
            ev->appendKeyDouble(propObject_calcRotationVelocity.name, _measuredAngularVelocity_velocityMeasurement);
            App::worldContainer->pushEvent();
        }
    }
}

void CSceneObject::setAuthorizedViewableObjects(int objOrCollHandle)
{
    _authorizedViewableObjects = objOrCollHandle;
}

int CSceneObject::getAuthorizedViewableObjects() const
{
    return (_authorizedViewableObjects);
}

void CSceneObject::setTransparentObjectDistanceOffset(double d)
{
    _transparentObjectDistanceOffset = d;
}

double CSceneObject::getTransparentObjectDistanceOffset() const
{
    return (_transparentObjectDistanceOffset);
}

double CSceneObject::getMeasuredAngularVelocity() const
{
    return _measuredAngularVelocity_velocityMeasurement;
}

C3Vector CSceneObject::getMeasuredLinearVelocity() const
{
    return _measuredLinearVelocity_velocityMeasurement;
}

C3Vector CSceneObject::getMeasuredAngularVelocity3() const
{
    return _measuredAngularVelocity3_velocityMeasurement;
}

C3Vector CSceneObject::getMeasuredAngularVelocityAxis() const
{
    return _measuredAngularVelocityAxis_velocityMeasurement;
}

void CSceneObject::setHierarchyColorIndex(int c)
{
    bool diff = (_hierarchyColorIndex != c);
    if (diff)
    {
        _hierarchyColorIndex = c;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_hierarchyColor.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _hierarchyColorIndex);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getHierarchyColorIndex() const
{
    return (_hierarchyColorIndex);
}

void CSceneObject::setCollectionSelfCollisionIndicator(int c)
{
    bool diff = (_collectionSelfCollisionIndicator != c);
    if (diff)
    {
        _collectionSelfCollisionIndicator = c;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_collectionSelfCollInd.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _collectionSelfCollisionIndicator);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getCollectionSelfCollisionIndicator() const
{
    return (_collectionSelfCollisionIndicator);
}

void CSceneObject::setDynamicFlag(int flag)
{
    bool diff = (_dynamicFlag != flag);
    if (diff)
    {
        _dynamicFlag = flag;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_dynamicFlag.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyInt(cmd, _dynamicFlag);
            App::worldContainer->pushEvent();
        }
    }
}

CSceneObject* CSceneObject::getParent() const
{
    return (_parentObject);
}

int CSceneObject::getObjectType() const
{
    return (_objectType);
}

int CSceneObject::getObjectHandle() const
{
    return (_objectHandle);
}

long long int CSceneObject::getObjectUid() const
{
    return (_objectUid);
}

bool CSceneObject::getSelected() const
{
    return (_selected);
}

bool CSceneObject::getIsInScene() const
{
    return _isInScene;
}

bool CSceneObject::getModelBase() const
{
    return (_modelBase);
}

std::string CSceneObject::getExtensionString() const
{
    return (_extensionString);
}

int CSceneObject::getVisibilityLayer() const
{
    return _visibilityLayer;
}

int CSceneObject::getChildOrder() const
{
    return (_childOrder);
}

int CSceneObject::getHierarchyTreeObjects(std::vector<CSceneObject*>& allObjects)
{
    int retVal = 1;
    allObjects.push_back((CSceneObject*)this);
    for (size_t i = 0; i < _childList.size(); i++)
        retVal += _childList[i]->getHierarchyTreeObjects(allObjects);
    return (retVal);
}

std::string CSceneObject::getObjectName_old() const
{
    return (_objectName_old);
}

std::string CSceneObject::getObjectAltName_old() const
{
    return (_objectAltName_old);
}

std::string CSceneObject::getObjectAlias() const
{
    return (_objectAlias);
}

std::string CSceneObject::getObjectAlias_fullPath() const
{
    std::string retVal;
    if (_parentObject == nullptr)
        retVal = "/" + getObjectAliasAndOrderIfRequired();
    else
        retVal = _parentObject->getObjectAlias_fullPath() + "/" + getObjectAliasAndOrderIfRequired();
    return (retVal);
}

std::string CSceneObject::getObjectAlias_shortPath() const
{
    std::string previousAlias = getObjectAliasAndOrderIfRequired();
    size_t cnt = 1;
    std::string retVal("/" + previousAlias);
    if (_parentObject != nullptr)
    {
        CSceneObject* it = _parentObject;

        CSceneObject* itBeforeSkipping = nullptr;
        std::string retValBeforeSkipping;
        bool previouslySkipped = false;
        bool doNotSkip = false;
        while (it != nullptr)
        {
            if (cnt > 8)
                return (getObjectAlias_fullPath());
            std::string itAlias = it->getObjectAliasAndOrderIfRequired();
            if ((itAlias == previousAlias) && previouslySkipped)
            {
                it = itBeforeSkipping;
                retVal = retValBeforeSkipping;
                doNotSkip = true;
                previouslySkipped = false;
            }
            else
            {
                if (it->getParent() == nullptr)
                {
                    if (it->getModelBase() || doNotSkip ||
                        (App::currentWorld->sceneObjects->getObjectFromPath(nullptr, retVal.c_str(), 0) != this))
                    {
                        previousAlias = itAlias;
                        retVal = "/" + previousAlias + retVal;
                        cnt++;
                    }
                    else
                    {
                        if (!previouslySkipped)
                        {
                            retValBeforeSkipping = retVal;
                            itBeforeSkipping = it;
                        }
                        previouslySkipped = true;
                    }
                }
                else
                {
                    std::string tmp(".");
                    tmp += retVal;
                    if (it->getModelBase() || doNotSkip ||
                        (App::currentWorld->sceneObjects->getObjectFromPath(it->getParent(), tmp.c_str(), 0) != this))
                    {
                        previousAlias = itAlias;
                        retVal = "/" + previousAlias + retVal;
                        cnt++;
                    }
                    else
                    {
                        if (!previouslySkipped)
                        {
                            retValBeforeSkipping = retVal;
                            itBeforeSkipping = it;
                        }
                        previouslySkipped = true;
                    }
                }
                doNotSkip = false;
                it = it->getParent();
            }
        }
    }
    return (retVal);
}

std::string CSceneObject::getObjectAlias_printPath() const
{
    std::string retVal = getObjectAlias_shortPath();
    if (retVal.size() > 40)
    {
        size_t cnt = 0;
        size_t p2;
        for (size_t i = 0; i < retVal.size(); i++)
        {
            if (retVal[i] == '/')
            {
                cnt++;
                if (cnt == 2)
                    p2 = i;
                if (cnt >= 3)
                    break;
            }
        }
        if (cnt >= 3)
        {
            retVal = retVal.substr(0, p2 + 1) + " ... /";
            retVal += getObjectAliasAndOrderIfRequired();
        }
    }
    return (retVal);
}

std::string CSceneObject::getObjectAliasAndOrderIfRequired() const
{
    std::string retVal(_objectAlias);
    if (_childOrder >= 0)
    {
        retVal += "[";
        retVal += std::to_string(_childOrder);
        retVal += "]";
    }
    return (retVal);
}

std::string CSceneObject::getObjectAliasAndHandle() const
{
    std::string retVal(_objectAlias);
    retVal += "-";
    retVal += std::to_string(_objectHandle);
    return (retVal);
}

std::string CSceneObject::getObjectPathAndIndex(size_t modelCnt) const
{
    std::vector<const CSceneObject*> objects;
    objects.push_back(this);
    CSceneObject* p = this->getParent();
    for (size_t i = 0; i < modelCnt; i++)
    {
        while ((p != nullptr) && (!p->getModelBase()))
            p = p->getParent();
        if (p != nullptr)
        {
            objects.push_back(p);
            p = p->getParent();
        }
        else
            break;
    }
    std::string retVal;
    for (int i = int(objects.size()) - 1; i >= 0; i--)
    {
        const CSceneObject* it = objects[i];
        retVal += "/" + it->getObjectAlias();
        int index = 0;
        while (true)
        {
            if (it == App::currentWorld->sceneObjects->getObjectFromPath(nullptr, retVal.c_str(), index))
                break;
            index++;
        }
        // Following: we try to omit {0} if {1} doesn't exist
        if ((index != 0) || (App::currentWorld->sceneObjects->getObjectFromPath(nullptr, retVal.c_str(), 1) != nullptr))
            retVal += "{" + std::to_string(index) + "}";
    }
    return (retVal);
}

C7Vector CSceneObject::getIntrinsicTransformation(bool includeDynErrorComponent, bool* available /*=nullptr*/) const
{
    if (available != nullptr)
        available[0] = false;
    return (C7Vector::identityTransformation);
}

C7Vector CSceneObject::getLocalTransformation() const
{
    return (_localTransformation);
}

C7Vector CSceneObject::getFullLocalTransformation() const
{
    return (_localTransformation);
}

C7Vector CSceneObject::getFullParentCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject == nullptr)
        retVal.setIdentity();
    else
        retVal = _parentObject->getFullCumulativeTransformation();
    return (retVal);
}

C7Vector CSceneObject::getCumulativeTransformation() const
{
    C7Vector retVal;
    if (_parentObject == nullptr)
        retVal = getLocalTransformation();
    else
        retVal = getFullParentCumulativeTransformation() * getLocalTransformation();
    return (retVal);
}

C7Vector CSceneObject::getFullCumulativeTransformation() const
{
    return (getFullParentCumulativeTransformation() * getFullLocalTransformation());
}

void CSceneObject::recomputeModelInfluencedValues(int overrideFlags /*=-1*/)
{
    if (overrideFlags == -1)
    {
        if (_parentObject == nullptr)
        {
            if (_modelBase)
                overrideFlags = (_modelProperty | sim_modelproperty_not_model) - sim_modelproperty_not_model;
            else
                overrideFlags = 0;
        }
        else
        {
            _parentObject->recomputeModelInfluencedValues(-2);
            return;
        }
    }
    if (overrideFlags != -2)
    {
        if (_modelBase)
            overrideFlags |= ((_modelProperty | sim_modelproperty_not_model) - sim_modelproperty_not_model);
        _calculatedModelProperty = overrideFlags;
        _setModelInvisible((_calculatedModelProperty & sim_modelproperty_not_visible) != 0);

        _calculatedObjectProperty = _objectProperty;
        if ((_calculatedModelProperty & sim_modelproperty_not_showasinsidemodel) != 0)
            _calculatedObjectProperty |= sim_objectproperty_dontshowasinsidemodel;
    }

    for (size_t i = 0; i < _childList.size(); i++)
        _childList[i]->recomputeModelInfluencedValues(_calculatedModelProperty);
}

void CSceneObject::setObjectUniqueId()
{
    _objectUid = App::getFreshUniqueId(-1);
}

void CSceneObject::setSelected(bool s)
{ // to be only called from object container!
    bool diff = (_selected != s);
    if (diff)
    {
        _selected = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_selected.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _selected);
            App::worldContainer->pushEvent();
        }
    }
}

void CSceneObject::setIsInScene(bool s)
{
    _isInScene = s;
}

void CSceneObject::setParentPtr(CSceneObject* parent)
{
    _parentObject = parent;
}

void CSceneObject::_setModelInvisible(bool inv)
{
    bool diff = (_modelInvisible != inv);
    if (diff)
    {
        _modelInvisible = inv;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_modelInvisible.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyBool(cmd, inv);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getDynamicFlag() const
{
    return (_dynamicFlag);
}

bool CSceneObject::getShouldObjectBeDisplayed(int viewableHandle, int displayAttrib)
{
    if (_forceAlwaysVisible_tmp)
        return (true);

    bool display = false;
    if (((displayAttrib & sim_displayattribute_pickpass) == 0) ||
        ((_objectProperty & sim_objectproperty_selectinvisible) == 0))
    { // ok, no pickpass and select invisible
        if (displayAttrib & sim_displayattribute_dynamiccontentonly)
            display = (_dynamicFlag != 0);
        else
        {
            display = ((!isObjectPartOfInvisibleModel()) &&
                       ((App::currentWorld->environment->getActiveLayers() & getVisibilityLayer()) ||
                        (displayAttrib & sim_displayattribute_ignorelayer)));

            if (display)
            {
                if (_authorizedViewableObjects == -2)
                    display = false;
                else
                {
                    if ((_authorizedViewableObjects >= 0) && (viewableHandle != -1))
                    {
                        if (_authorizedViewableObjects <= SIM_IDEND_SCENEOBJECT)
                            display = (_authorizedViewableObjects == viewableHandle);
                        else
                        {
                            CCollection* gr =
                                App::currentWorld->collections->getObjectFromHandle(_authorizedViewableObjects);
                            if (gr != nullptr)
                                display = gr->isObjectInCollection(viewableHandle);
                            else
                                display = false; // should normally never happen
                        }
                    }
                }
            }
        }
    }

    return (display);
}

void CSceneObject::setModelAcknowledgement(const char* a)
{
    std::string ma(a);
    if (ma.length() > 3000)
        ma.erase(ma.begin() + 2999, ma.end());
    bool diff = (_modelAcknowledgement != ma);
    if (diff)
    {
        _modelAcknowledgement = ma;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_modelAcknowledgment.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyText(cmd, _modelAcknowledgement.c_str());
            App::worldContainer->pushEvent();
        }
    }
}

std::string CSceneObject::getModelAcknowledgement() const
{
    return _modelAcknowledgement;
}

void CSceneObject::setDynamicsResetFlag(bool reset, bool fullHierarchyTree)
{
    _dynamicsResetFlag = reset;
    if (reset)
    {
        _dynamicFlag = 0;
        if (_objectType == sim_sceneobject_joint)
        {
            CJoint* joint = (CJoint*)this;
            joint->setForceOrTorqueNotValid();
            joint->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
        if (_objectType == sim_sceneobject_forcesensor)
        {
            CForceSensor* sensor = (CForceSensor*)this;
            sensor->setForceAndTorqueNotValid();
            sensor->setIntrinsicTransformationError(C7Vector::identityTransformation);
        }
    }
    if (fullHierarchyTree)
    {
        for (size_t i = 0; i < getChildCount(); i++)
            getChildFromIndex(i)->setDynamicsResetFlag(reset, fullHierarchyTree);
    }
}

bool CSceneObject::getDynamicsResetFlag() const
{
    return (_dynamicsResetFlag);
}

void CSceneObject::setSizeFactor(double f)
{
    f = tt::getLimitedFloat(0.000001, 1000000.0, f);
    _sizeFactor = f;
}

double CSceneObject::getSizeFactor() const
{
    return (_sizeFactor);
}

void CSceneObject::setSizeValues(const double s[3])
{
    _sizeValues[0] = s[0];
    _sizeValues[1] = s[1];
    _sizeValues[2] = s[2];
}

void CSceneObject::getSizeValues(double s[3]) const
{
    s[0] = _sizeValues[0];
    s[1] = _sizeValues[1];
    s[2] = _sizeValues[2];
}

void CSceneObject::setScriptExecPriority_raw(int p)
{
    _scriptExecPriority = p;
}

int CSceneObject::getScriptExecPriority() const
{
    return (_scriptExecPriority);
}

int CSceneObject::getParentCount() const
{
    if (getParent() == nullptr)
        return (0);
    return (1 + getParent()->getParentCount());
}

int CSceneObject::_getAllowedObjectSpecialProperties() const
{
    int retVal = 0;
    if (isPotentiallyCollidable())
        retVal |= sim_objectspecialproperty_collidable;
    if (isPotentiallyMeasurable())
        retVal |= sim_objectspecialproperty_measurable;
    if (isPotentiallyDetectable())
        retVal |= sim_objectspecialproperty_detectable;
    if (isPotentiallyRenderable())
        retVal |= sim_objectspecialproperty_renderable;
    return (retVal);
}

std::string CSceneObject::getObjectTypeInfo() const
{
    return ("");
}
std::string CSceneObject::getObjectTypeInfoExtended() const
{
    return ("");
}

bool CSceneObject::isPotentiallyCollidable() const
{
    return (false);
}

bool CSceneObject::isPotentiallyMeasurable() const
{
    return (false);
}

bool CSceneObject::isPotentiallyDetectable() const
{
    return (false);
}

bool CSceneObject::isPotentiallyRenderable() const
{
    return (false);
}

void CSceneObject::setModelBase(bool m)
{ // is also called from the ungroup/divide shape routines!!
    bool diff = (_modelBase != m);
    if (diff)
    {
        _modelBase = m;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_modelBase.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyBool(cmd, m);
            App::worldContainer->pushEvent();
        }
        bool diff2 = false;
        if (_modelBase)
            diff2 = setModelProperty(0);
        else
            diff2 = setModelProperty(sim_modelproperty_not_model);
        setModelAcknowledgement("");
        if (!diff2)
            recomputeModelInfluencedValues();
    }
}

void CSceneObject::setObjectProperty(int p)
{
    bool diff = (_objectProperty != p);
    if (diff)
    {
        int cb = _objectProperty ^ p;
        _objectProperty = p;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_objectProperty.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->appendKeyInt("objectProperty", _objectProperty); // deprecated
#endif
            ev->appendKeyInt(propObject_objectProperty.name, _objectProperty);
            if (cb & sim_objectproperty_ignoreviewfitting)
                ev->appendKeyBool(propObject_ignoreViewFitting.name, _objectProperty & sim_objectproperty_ignoreviewfitting);
            if (cb & sim_objectproperty_collapsed)
                ev->appendKeyBool(propObject_collapsed.name, _objectProperty & sim_objectproperty_collapsed);
            if (cb & sim_objectproperty_selectable)
                ev->appendKeyBool(propObject_selectable.name, _objectProperty & sim_objectproperty_selectable);
            if (cb & sim_objectproperty_selectmodelbaseinstead)
                ev->appendKeyBool(propObject_selectModel.name, _objectProperty & sim_objectproperty_selectmodelbaseinstead);
            if (cb & sim_objectproperty_dontshowasinsidemodel)
                ev->appendKeyBool(propObject_hideFromModelBB.name, _objectProperty & sim_objectproperty_dontshowasinsidemodel);
            if (cb & sim_objectproperty_selectinvisible)
                ev->appendKeyBool(propObject_selectInvisible.name, _objectProperty & sim_objectproperty_selectinvisible);
            if (cb & sim_objectproperty_depthinvisible)
                ev->appendKeyBool(propObject_depthInvisible.name, _objectProperty & sim_objectproperty_depthinvisible);
            if (cb & sim_objectproperty_cannotdelete)
                ev->appendKeyBool(propObject_cannotDelete.name, _objectProperty & sim_objectproperty_cannotdelete);
            if (cb & sim_objectproperty_cannotdeleteduringsim)
                ev->appendKeyBool(propObject_cannotDeleteSim.name, _objectProperty & sim_objectproperty_cannotdeleteduringsim);
            App::worldContainer->pushEvent();
        }
        recomputeModelInfluencedValues();
    }
}

int CSceneObject::getObjectProperty() const
{
    return (_objectProperty);
}

int CSceneObject::getCumulativeObjectProperty()
{
    return (_calculatedObjectProperty);
    /*
    int retVal=_objectProperty;
    int o=getCumulativeModelProperty();
    if (o&sim_modelproperty_not_showasinsidemodel!=0)
        retVal|=sim_objectproperty_dontshowasinsidemodel;
    return(retVal);
    */
}

void CSceneObject::setLocalObjectSpecialProperty(int prop)
{ // sets the local value
    prop &= _getAllowedObjectSpecialProperties();
    bool diff = (_localObjectSpecialProperty != prop);
    if (diff)
    {
        int cb = _localObjectSpecialProperty ^ prop;
        _localObjectSpecialProperty = prop;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            if (cb & (sim_objectspecialproperty_collidable | sim_objectspecialproperty_measurable | sim_objectspecialproperty_detectable))
            {
                const char* cmd = propObject_collidable.name;
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
                if (cb & sim_objectspecialproperty_collidable)
                    ev->appendKeyBool(propObject_collidable.name, _localObjectSpecialProperty & sim_objectspecialproperty_collidable);
                if (cb & sim_objectspecialproperty_measurable)
                    ev->appendKeyBool(propObject_measurable.name, _localObjectSpecialProperty & sim_objectspecialproperty_measurable);
                if (cb & sim_objectspecialproperty_detectable)
                    ev->appendKeyBool(propObject_detectable.name, _localObjectSpecialProperty & sim_objectspecialproperty_detectable);
                App::worldContainer->pushEvent();
            }
        }
    }
}

int CSceneObject::getLocalObjectSpecialProperty() const
{ // returns the local value
    return _localObjectSpecialProperty;
}

int CSceneObject::getCumulativeObjectSpecialProperty()
{ // returns the cumulative value
    int o = getCumulativeModelProperty();
    if (o == 0)
        return (_localObjectSpecialProperty); // nothing is overridden!
    int p = _localObjectSpecialProperty;

    if (o & sim_modelproperty_not_collidable)
        p = (p | sim_objectspecialproperty_collidable) - sim_objectspecialproperty_collidable;
    if (o & sim_modelproperty_not_measurable)
        p = (p | sim_objectspecialproperty_measurable) - sim_objectspecialproperty_measurable;
    if (o & sim_modelproperty_not_renderable)
        p = (p | sim_objectspecialproperty_renderable) - sim_objectspecialproperty_renderable;
    if (o & sim_modelproperty_not_detectable)
        p = (p | sim_objectspecialproperty_detectable) - sim_objectspecialproperty_detectable;

    return (p);
}

bool CSceneObject::setModelProperty(int prop)
{ // model properties are actually override properties
    bool wm = true;
    if (prop & sim_modelproperty_not_model)
    {
        prop = sim_modelproperty_not_model;
        wm = false;
    }
    if (wm != _modelBase)
        setModelBase(wm);
    bool diff = (_modelProperty != prop);
    if (diff)
    {
        int cb = _modelProperty ^ prop;
        _modelProperty = prop;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_modelProperty.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->appendKeyInt("modelProperty", _modelProperty); // Deprecated
#endif
            ev->appendKeyInt(propObject_modelProperty.name, _modelProperty);
            if (cb & sim_modelproperty_not_collidable)
                ev->appendKeyBool(propObject_modelNotCollidable.name, _modelProperty & sim_modelproperty_not_collidable);
            if (cb & sim_modelproperty_not_measurable)
                ev->appendKeyBool(propObject_modelNotMeasurable.name, _modelProperty & sim_modelproperty_not_measurable);
            if (cb & sim_modelproperty_not_detectable)
                ev->appendKeyBool(propObject_modelNotDetectable.name, _modelProperty & sim_modelproperty_not_detectable);
            if (cb & sim_modelproperty_not_dynamic)
                ev->appendKeyBool(propObject_modelNotDynamic.name, _modelProperty & sim_modelproperty_not_dynamic);
            if (cb & sim_modelproperty_not_respondable)
                ev->appendKeyBool(propObject_modelNotRespondable.name, _modelProperty & sim_modelproperty_not_respondable);
            if (cb & sim_modelproperty_not_visible)
                ev->appendKeyBool(propObject_modelNotVisible.name, _modelProperty & sim_modelproperty_not_visible);
            if (cb & sim_modelproperty_scripts_inactive)
                ev->appendKeyBool(propObject_modelScriptsNotActive.name, _modelProperty & sim_modelproperty_scripts_inactive);
            if (cb & sim_modelproperty_not_showasinsidemodel)
                ev->appendKeyBool(propObject_modelNotInParentBB.name, _modelProperty & sim_modelproperty_not_showasinsidemodel);
            App::worldContainer->pushEvent();
        }
        recomputeModelInfluencedValues();
    }
    return diff;
}

int CSceneObject::getModelProperty() const
{ // model properties are actually override properties. This func. returns the local value
    return (_modelProperty);
}

int CSceneObject::getCumulativeModelProperty() const
{ // model properties are actually override properties. This func. returns the cumulative value
    return _calculatedModelProperty;
}

bool CSceneObject::isObjectVisible() const
{
    return (isObjectInVisibleLayer() && (!isObjectPartOfInvisibleModel()));
}

bool CSceneObject::isObjectInVisibleLayer() const
{
    return ((App::currentWorld->environment->getActiveLayers() & _visibilityLayer) != 0);
}

bool CSceneObject::isObjectPartOfInvisibleModel() const
{
    return ((getCumulativeModelProperty() & sim_modelproperty_not_visible) != 0);
}

int CSceneObject::getTreeDynamicProperty() // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable
{
    int retVal = 0;
    if (!_dynamicsTemporarilyDisabled)
    {
        int o = getCumulativeModelProperty();
        if ((o & sim_modelproperty_not_dynamic) == 0)
            retVal |= sim_objdynprop_dynamic;
        if ((o & sim_modelproperty_not_respondable) == 0)
            retVal |= sim_objdynprop_respondable;
    }
    return (retVal);
}

int CSceneObject::getModelSelectionHandle(bool firstObject)
{ // firstObject is true by default
#ifdef SIM_WITH_GUI
    if (CSimFlavor::getBoolVal(9))
    {
        if ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->getKeyDownState() & 1) &&
            (GuiApp::mainWindow->getKeyDownState() & 2))
            return (getObjectHandle());
    }
#endif

    if (_modelBase)
    {
        if (((_objectProperty & sim_objectproperty_selectmodelbaseinstead) == 0))
            return (getObjectHandle());
        if (getParent() == nullptr)
            return (getObjectHandle());
        int retV = getParent()->getModelSelectionHandle(false);
        if (retV == -1)
            retV = getObjectHandle();
        return (retV);
    }
    else
    {
        if (getParent() == nullptr)
        {
            if (firstObject)
                return (getObjectHandle());
            return (-1);
        }
        int retV = getParent()->getModelSelectionHandle(false);
        if (retV != -1)
            return (retV);
        if (firstObject)
            return (getObjectHandle());
        return (-1);
    }
}

void CSceneObject::clearManipulationModeOverlayGridFlag()
{
    _objectManipulationMode_flaggedForGridOverlay = 0;
}

void CSceneObject::scaleObject(double scalingFactor)
{
    _sizeFactor *= scalingFactor;
    _sizeValues[0] *= scalingFactor;
    _sizeValues[1] *= scalingFactor;
    _sizeValues[2] *= scalingFactor;
    App::worldContainer->setModificationFlag(256); // object scaled
    computeBoundingBox();
    pushObjectRefreshEvent();
}

bool CSceneObject::scaleObjectNonIsometrically(double x, double y, double z)
{ // arriving here only for objects that only supports iso scaling (all, except for shapes)
    bool retVal = false;
    if ((x > 0.00001) && (y > 0.00001) && (z > 0.00001))
    {
        scaleObject(cbrt(x * y * z)); // most objects only scale isometrically
        retVal = true;
    }
    return (retVal);
}

void CSceneObject::scalePosition(double scalingFactor)
{ // This routine will scale an object's position. The object itself keeps the same size.
    C7Vector local(getLocalTransformation());
    setLocalTransformation(local.X * scalingFactor);
    _assemblingLocalTransformation.X = _assemblingLocalTransformation.X * scalingFactor;
    _dynamicsResetFlag = true;
}

void CSceneObject::setSpecificLight(int h)
{
    _specificLight = h;
}

int CSceneObject::getSpecificLight() const
{
    return (_specificLight);
}

bool CSceneObject::setBeforeDeleteCallbackSent()
{
    bool retVal = !_beforeDeleteCallbackSent;
    _beforeDeleteCallbackSent = true;
    return (retVal);
}

bool CSceneObject::getModelBB(const C7Vector& baseCoordInv, C3Vector& minV, C3Vector& maxV, bool first) const
{ // For model selection display! Return value false means there is no model BB
    bool retVal = false;
    int objProp = getObjectProperty();
    int modProp = getModelProperty();

    bool exploreChildren = ((modProp & sim_modelproperty_not_showasinsidemodel) == 0) || first;
    bool includeThisBox = (objProp & sim_objectproperty_dontshowasinsidemodel) == 0;
    first = false;

    if (includeThisBox && exploreChildren)
    {
        retVal = true;
        C3Vector bbs;
        C7Vector bbf(getBB(&bbs));
        bbs *= 2.0;
        C7Vector tr(baseCoordInv * getCumulativeTransformation() * bbf);
        C3Vector v;
        for (double i = -1.0; i < 2.0; i = i + 2.0)
        {
            v(0) = bbs(0) * i * 0.5;
            for (double j = -1.0; j < 2.0; j = j + 2.0)
            {
                v(1) = bbs(1) * j * 0.5;
                for (double k = -1.0; k < 2.0; k = k + 2.0)
                {
                    v(2) = bbs(2) * k * 0.5;
                    C3Vector w(tr * v);
                    maxV.keepMax(w);
                    minV.keepMin(w);
                }
            }
        }
    }

    if (exploreChildren)
    {
        for (size_t i = 0; i < getChildCount(); i++)
        {
            if (getChildFromIndex(i)->getModelBB(baseCoordInv, minV, maxV, false))
                retVal = true;
        }
    }
    return (retVal);
}

void CSceneObject::performGcsLoadingMapping(const std::map<int, int>* map)
{
}

void CSceneObject::announceCollectionWillBeErased(int collectionID, bool copyBuffer)
{
    if (_authorizedViewableObjects == collectionID)
        _authorizedViewableObjects = -2; // not visible anymore!
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::sceneObjects->getObject(id)'-call or similar

    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_objecttype_collection)
            {
                if (it->second[i].generalObjectHandle == collectionID)
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
    if (!copyBuffer)
    {
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_objecttype_collection)
                {
                    if (it->second[i].generalObjectHandle == collectionID)
                        it->second[i].generalObjectHandle = -1;
                }
            }
        }
    }
}

void CSceneObject::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_collision_type)
            {
                if (it->second[i].generalObjectHandle == collisionID)
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
    if (!copyBuffer)
    {
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_collision_type)
                {
                    if (it->second[i].generalObjectHandle == collisionID)
                        it->second[i].generalObjectHandle = -1;
                }
            }
        }
    }
}
void CSceneObject::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_distance_type)
            {
                if (it->second[i].generalObjectHandle == distanceID)
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
    if (!copyBuffer)
    {
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_distance_type)
                {
                    if (it->second[i].generalObjectHandle == distanceID)
                        it->second[i].generalObjectHandle = -1;
                }
            }
        }
    }
}
void CSceneObject::performIkLoadingMapping(const std::map<int, int>* map, int opType)
{
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_ik_type)
                it->second[i].generalObjectHandle = CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
        }
    }
    if (opType == 0)
    { // scene load only
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_ik_type)
                    it->second[i].generalObjectHandle =
                        CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
            }
        }
    }
}

void CSceneObject::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    if ((_authorizedViewableObjects >= 0) && (_authorizedViewableObjects > SIM_IDEND_SCENEOBJECT))
        _authorizedViewableObjects = CWorld::getLoadingMapping(map, _authorizedViewableObjects);
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_objecttype_collection)
                it->second[i].generalObjectHandle = CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
        }
    }
    if (opType == 0)
    { // scene load only
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_objecttype_collection)
                    it->second[i].generalObjectHandle =
                        CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
            }
        }
    }
}

void CSceneObject::performCollisionLoadingMapping(const std::map<int, int>* map, int opType)
{
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_collision_type)
                it->second[i].generalObjectHandle = CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
        }
    }
    if (opType == 0)
    { // scene load only
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_collision_type)
                    it->second[i].generalObjectHandle =
                        CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
            }
        }
    }
}

void CSceneObject::performDistanceLoadingMapping(const std::map<int, int>* map, int opType)
{
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_distance_type)
                it->second[i].generalObjectHandle = CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
        }
    }
    if (opType == 0)
    { // scene load only
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_distance_type)
                    it->second[i].generalObjectHandle =
                        CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
            }
        }
    }
}

void CSceneObject::performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
}

void CSceneObject::performDynMaterialObjectLoadingMapping(const std::map<int, int>* map)
{
}

void CSceneObject::getAllObjectsRecursive(std::vector<CSceneObject*>* objectList, bool baseIncluded, bool start) const
{ // baseIncluded and start are true by default.
    // Returns all objects build on this (including children of children of...)
    if (baseIncluded || (!start))
        objectList->push_back((CSceneObject*)this);
    for (size_t i = 0; i < getChildCount(); i++)
        getChildFromIndex(i)->getAllObjectsRecursive(objectList, true, false);
}

void CSceneObject::getChain(std::vector<CSceneObject*>& objectList, bool tipIncluded, bool start) const
{ // tipIncluded and start are true by default.
    // Returns the chain with this object as tip
    if (tipIncluded || (!start))
        objectList.push_back((CSceneObject*)this);
    if (getParent() != nullptr)
        getParent()->getChain(objectList, true, false);
}

CSceneObject* CSceneObject::getFirstParentInSelection(const std::vector<CSceneObject*>* sel) const
{
    CSceneObject* it = getParent();
    if (it == nullptr)
        return (nullptr);
    for (size_t i = 0; i < sel->size(); i++)
    {
        if (sel->at(i) == it)
            return (it);
    }
    return (it->getFirstParentInSelection(sel));
}

CSceneObject* CSceneObject::getLastParentInSelection(const std::vector<CSceneObject*>* sel) const
{
    CSceneObject* it = getParent();
    CSceneObject* retVal = nullptr;
    while (it != nullptr)
    {
        for (size_t i = 0; i < sel->size(); i++)
        {
            if (sel->at(i) == it)
            {
                retVal = it;
                break;
            }
        }
        it = it->getParent();
    }
    return (retVal);
}

void CSceneObject::removeSceneDependencies()
{
    _customReferencedHandles.clear();
    _customReferencedOriginalHandles.clear();
}

void CSceneObject::addSpecializedObjectEventData(CCbor* ev)
{
}

void CSceneObject::pushObjectCreationEvent()
{
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = App::worldContainer->createSceneObjectAddEvent(this);
        CSceneObject::_addCommonObjectEventData(ev);
        addSpecializedObjectEventData(ev);
        App::worldContainer->pushEvent();

        if (_objectType == sim_sceneobject_shape)
        {
            std::vector<CMesh*> all;
            std::vector<C7Vector> allTr;
            ((CShape*)this)->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, all, &allTr);
            for (size_t i = 0; i < all.size(); i++)
                all[i]->pushObjectCreationEvent(_objectUid, allTr[i]);
        }
    }
}

void CSceneObject::pushObjectRefreshEvent()
{
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, nullptr, false);
        CSceneObject::_addCommonObjectEventData(ev);
        addSpecializedObjectEventData(ev);
        App::worldContainer->pushEvent();
    }
}

void CSceneObject::_addCommonObjectEventData(CCbor* ev) const
{
    ev->appendKeyText(propObject_objectType.name, getObjectTypeInfo().c_str());
    ev->appendKeyInt(propObject_layer.name, _visibilityLayer);
    ev->appendKeyInt(propObject_childOrder.name, _childOrder);
    std::vector<int> ch;
    for (size_t i = 0; i < _childList.size(); i++)
        ch.push_back(_childList[i]->getObjectHandle());
    ev->appendKeyIntArray(propObject_children.name, ch.data(), ch.size());
    double p[7] = {_localTransformation.X(0), _localTransformation.X(1), _localTransformation.X(2),
                   _localTransformation.Q(1), _localTransformation.Q(2), _localTransformation.Q(3),
                   _localTransformation.Q(0)};
    ev->appendKeyDoubleArray(propObject_pose.name, p, 7);
    ev->appendKeyText(propObject_alias.name, _objectAlias.c_str());
    ev->appendKeyBool(propObject_modelInvisible.name, _modelInvisible);
    ev->appendKeyBool(propObject_modelBase.name, _modelBase);

#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->appendKeyInt("objectProperty", _objectProperty); // deprecated
    ev->appendKeyInt("dynamicFlag", _dynamicFlag);
    ev->appendKeyText("oldName", _objectName_old.c_str());
#endif
    ev->appendKeyInt(propObject_objectProperty.name, _objectProperty);
    ev->appendKeyBool(propObject_ignoreViewFitting.name, _objectProperty & sim_objectproperty_ignoreviewfitting);
    ev->appendKeyBool(propObject_collapsed.name, _objectProperty & sim_objectproperty_collapsed);
    ev->appendKeyBool(propObject_selectable.name, _objectProperty & sim_objectproperty_selectable);
    ev->appendKeyBool(propObject_selectModel.name, _objectProperty & sim_objectproperty_selectmodelbaseinstead);
    ev->appendKeyBool(propObject_hideFromModelBB.name, _objectProperty & sim_objectproperty_dontshowasinsidemodel);
    ev->appendKeyBool(propObject_selectInvisible.name, _objectProperty & sim_objectproperty_selectinvisible);
    ev->appendKeyBool(propObject_depthInvisible.name, _objectProperty & sim_objectproperty_depthinvisible);
    ev->appendKeyBool(propObject_cannotDelete.name, _objectProperty & sim_objectproperty_cannotdelete);
    ev->appendKeyBool(propObject_cannotDeleteSim.name, _objectProperty & sim_objectproperty_cannotdeleteduringsim);

#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->appendKeyInt("modelProperty", _modelProperty); // deprecated
#endif
    ev->appendKeyInt(propObject_modelProperty.name, _modelProperty);
    ev->appendKeyBool(propObject_modelNotCollidable.name, _modelProperty & sim_modelproperty_not_collidable);
    ev->appendKeyBool(propObject_modelNotMeasurable.name, _modelProperty & sim_modelproperty_not_measurable);
    ev->appendKeyBool(propObject_modelNotDetectable.name, _modelProperty & sim_modelproperty_not_detectable);
    ev->appendKeyBool(propObject_modelNotDynamic.name, _modelProperty & sim_modelproperty_not_dynamic);
    ev->appendKeyBool(propObject_modelNotRespondable.name, _modelProperty & sim_modelproperty_not_respondable);
    ev->appendKeyBool(propObject_modelNotVisible.name, _modelProperty & sim_modelproperty_not_visible);
    ev->appendKeyBool(propObject_modelScriptsNotActive.name, _modelProperty & sim_modelproperty_scripts_inactive);
    ev->appendKeyBool(propObject_modelNotInParentBB.name, _modelProperty & sim_modelproperty_not_showasinsidemodel);

    long long int pUid = -1;
    int pH = -1;
    if (_parentObject != nullptr)
    {
        pUid = _parentObject->getObjectUid();
        pH = _parentObject->getObjectHandle();
    }
    ev->appendKeyInt(propObject_parentUid.name, pUid);
    ev->appendKeyInt(propObject_parentHandle.name, pH);
    ev->appendKeyBool(propObject_selected.name, _selected);
    ev->appendKeyInt(propObject_hierarchyColor.name, _hierarchyColorIndex);
    ev->appendKeyInt(propObject_collectionSelfCollInd.name, _collectionSelfCollisionIndicator);
    ev->appendKeyBool(propObject_collidable.name, _localObjectSpecialProperty & sim_objectspecialproperty_collidable);
    ev->appendKeyBool(propObject_measurable.name, _localObjectSpecialProperty & sim_objectspecialproperty_measurable);
    ev->appendKeyBool(propObject_detectable.name, _localObjectSpecialProperty & sim_objectspecialproperty_detectable);
    ev->appendKeyText(propObject_modelAcknowledgment.name, _modelAcknowledgement.c_str());
    ev->appendKeyBuff(propObject_dna.name, (unsigned char*)_dnaString.data(), _dnaString.size());
    ev->appendKeyText(propObject_persistentUid.name, _uniquePersistentIdString.c_str());
    ev->appendKeyDoubleArray(propObject_calcLinearVelocity.name, _measuredLinearVelocity_velocityMeasurement.data, 3);
    ev->appendKeyDoubleArray(propObject_calcRotationAxis.name, _measuredAngularVelocityAxis_velocityMeasurement.data, 3);
    ev->appendKeyDouble(propObject_calcRotationVelocity.name, _measuredAngularVelocity_velocityMeasurement);
    ev->appendKeyInt(propObject_dynamicIcon.name, _dynamicSimulationIconCode);
    ev->appendKeyInt(propObject_dynamicFlag.name, _dynamicFlag);

    _bbFrame.getData(p, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    // deprecated
    ev->openKeyMap("boundingBox");
    ev->appendKeyDoubleArray("pose", p, 7);
    ev->appendKeyDoubleArray("hsize", _bbHalfSize.data, 3);
    ev->closeArrayOrMap();
#endif
    ev->appendKeyDoubleArray(propObject_bbPose.name, p, 7);
    ev->appendKeyDoubleArray(propObject_bbHsize.name, _bbHalfSize.data, 3);

    customObjectData.appendEventData(nullptr, ev);
    customObjectData_volatile.appendEventData(nullptr, ev);
    //    ev->openKeyMap("customData");
    //    customObjectData.appendEventData(ev);
    //    customObjectData_volatile.appendEventData(ev);
    //    ev->closeArrayOrMap(); // customData

#if SIM_EVENT_PROTOCOL_VERSION == 2
    // deprecated
    ev->appendKeyInt("movementOptions", _objectMovementOptions);
    ev->appendKeyInt("movementPreferredAxes", _objectMovementPreferredAxes);
#else
    ev->appendKeyInt(propObject_movementOptions.name, _objectMovementOptions);
    ev->appendKeyInt(propObject_movementPreferredAxes.name, _objectMovementPreferredAxes);
#endif
    ev->appendKeyBool(propObject_movTranslNoSim.name, (_objectMovementOptions & 1) == 0);
    ev->appendKeyBool(propObject_movTranslInSim.name, (_objectMovementOptions & 2) == 0);
    ev->appendKeyBool(propObject_movRotNoSim.name, (_objectMovementOptions & 4) == 0);
    ev->appendKeyBool(propObject_movRotInSim.name, (_objectMovementOptions & 8) == 0);
    ev->appendKeyBool(propObject_movAltTransl.name, (_objectMovementOptions & 16) == 0);
    ev->appendKeyBool(propObject_movAltRot.name, (_objectMovementOptions & 32) == 0);
    /*
    ev->appendKeyBool(propObject_movTranslX.name, (_objectMovementOptions & 64) == 0);
    ev->appendKeyBool(propObject_movTranslY.name, (_objectMovementOptions & 128) == 0);
    ev->appendKeyBool(propObject_movTranslZ.name, (_objectMovementOptions & 256) == 0);
    ev->appendKeyBool(propObject_movRotX.name, (_objectMovementOptions & 512) == 0);
    ev->appendKeyBool(propObject_movRotY.name, (_objectMovementOptions & 1024) == 0);
    ev->appendKeyBool(propObject_movRotZ.name, (_objectMovementOptions & 2048) == 0);
    */

    ev->appendKeyBool(propObject_movPrefTranslX.name, _objectMovementPreferredAxes & 1);
    ev->appendKeyBool(propObject_movPrefTranslY.name, _objectMovementPreferredAxes & 2);
    ev->appendKeyBool(propObject_movPrefTranslZ.name, _objectMovementPreferredAxes & 4);
    ev->appendKeyBool(propObject_movPrefRotX.name, _objectMovementPreferredAxes & 8);
    ev->appendKeyBool(propObject_movPrefRotY.name, _objectMovementPreferredAxes & 16);
    ev->appendKeyBool(propObject_movPrefRotZ.name, _objectMovementPreferredAxes & 32);

    ev->appendKeyDoubleArray(propObject_movementStepSize.name, _objectMovementStepSize, 2);
    ev->appendKeyIntArray(propObject_movementRelativity.name, _objectMovementRelativity, 2);
}

CSceneObject* CSceneObject::copyYourself()
{ // This routine should be called in the very first line of function
    // "copyYourself" in every joint, camera, etc. !

    CSceneObject* theNewObject = nullptr;
    if (getObjectType() == sim_sceneobject_shape)
        theNewObject = new CShape();
    if (getObjectType() == sim_sceneobject_octree)
        theNewObject = new COcTree();
    if (getObjectType() == sim_sceneobject_pointcloud)
        theNewObject = new CPointCloud();
    if (getObjectType() == sim_sceneobject_joint)
        theNewObject = new CJoint();
    if (getObjectType() == sim_sceneobject_graph)
        theNewObject = new CGraph();
    if (getObjectType() == sim_sceneobject_dummy)
        theNewObject = new CDummy();
    if (getObjectType() == sim_sceneobject_script)
        theNewObject = new CScript();
    if (getObjectType() == sim_sceneobject_proximitysensor)
        theNewObject = new CProxSensor();
    if (getObjectType() == sim_sceneobject_camera)
        theNewObject = new CCamera();
    if (getObjectType() == sim_sceneobject_light)
        theNewObject = new CLight(((CLight*)this)->getLightType());
    if (getObjectType() == sim_sceneobject_path)
        theNewObject = new CPath_old();
    if (getObjectType() == sim_sceneobject_mirror)
        theNewObject = new CMirror();
    if (getObjectType() == sim_sceneobject_visionsensor)
        theNewObject = new CVisionSensor();
    if (getObjectType() == sim_sceneobject_mill)
        theNewObject = new CMill();
    if (getObjectType() == sim_sceneobject_forcesensor)
        theNewObject = new CForceSensor();

    theNewObject->_objectHandle = _objectHandle; // important for copy operations connections
    theNewObject->_authorizedViewableObjects = _authorizedViewableObjects;
    theNewObject->_initialVisibilityLayer = _initialVisibilityLayer; // for cases object copied during simulation
    if (_initialValuesInitialized)
        theNewObject->_visibilityLayer = _initialVisibilityLayer; // for cases object copied during simulation
    else
        theNewObject->_visibilityLayer = _visibilityLayer;
    theNewObject->_childOrder = _childOrder;
    theNewObject->_localTransformation = _localTransformation;
    theNewObject->_objectAlias = _objectAlias;
    theNewObject->_objectName_old = _objectName_old;
    theNewObject->_objectAltName_old = _objectAltName_old;
    theNewObject->_objectProperty = _objectProperty;
    theNewObject->_hierarchyColorIndex = _hierarchyColorIndex;
    theNewObject->_collectionSelfCollisionIndicator = _collectionSelfCollisionIndicator;
    theNewObject->_modelBase = _modelBase;
    theNewObject->_objectType = _objectType;
    theNewObject->_localObjectSpecialProperty = _localObjectSpecialProperty;
    theNewObject->_modelProperty = _modelProperty;
    theNewObject->_extensionString = _extensionString;
    theNewObject->_scriptExecPriority = _scriptExecPriority;

    theNewObject->_dnaString = _dnaString;
    theNewObject->_copyString = _copyString;

    theNewObject->_assemblingLocalTransformation = _assemblingLocalTransformation;
    theNewObject->_assemblingLocalTransformationIsUsed = _assemblingLocalTransformationIsUsed;

    theNewObject->_assemblyMatchValuesChild.assign(_assemblyMatchValuesChild.begin(), _assemblyMatchValuesChild.end());
    theNewObject->_assemblyMatchValuesParent.assign(_assemblyMatchValuesParent.begin(),
                                                    _assemblyMatchValuesParent.end());
    theNewObject->_objectMovementPreferredAxes = _objectMovementPreferredAxes;
    theNewObject->_objectMovementOptions = _objectMovementOptions;
    theNewObject->_objectMovementRelativity[0] = _objectMovementRelativity[0];
    theNewObject->_objectMovementRelativity[1] = _objectMovementRelativity[1];
    theNewObject->_objectMovementStepSize[0] = _objectMovementStepSize[0];
    theNewObject->_objectMovementStepSize[1] = _objectMovementStepSize[1];
    theNewObject->_bbFrame = _bbFrame;
    theNewObject->_bbHalfSize = _bbHalfSize;
    theNewObject->_sizeFactor = _sizeFactor;
    theNewObject->_sizeValues[0] = _sizeValues[0];
    theNewObject->_sizeValues[1] = _sizeValues[1];
    theNewObject->_sizeValues[2] = _sizeValues[2];
    theNewObject->_modelAcknowledgement = _modelAcknowledgement;
    theNewObject->_transparentObjectDistanceOffset = _transparentObjectDistanceOffset;
    customObjectData.copyYourselfInto(theNewObject->customObjectData);
    customObjectData_volatile.copyYourselfInto(theNewObject->customObjectData_volatile);

    delete theNewObject->_customObjectData_old;
    theNewObject->_customObjectData_old = nullptr;
    if (_customObjectData_old != nullptr)
        theNewObject->_customObjectData_old = _customObjectData_old->copyYourself();

    delete theNewObject->_userScriptParameters;
    theNewObject->_userScriptParameters = nullptr;
    if (_userScriptParameters != nullptr)
        theNewObject->_userScriptParameters = _userScriptParameters->copyYourself();

    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
        theNewObject->_customReferencedHandles[it->first].assign(it->second.begin(), it->second.end());
    for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        theNewObject->_customReferencedOriginalHandles[it->first].assign(it->second.begin(), it->second.end());

    // Important:
    theNewObject->_parentObjectHandle_forSerializationOnly =
        _parentObjectHandle_forSerializationOnly; // and not -1!!! (objects can also be copied in the Copy buffer)
    if (_parentObject != nullptr)
        theNewObject->_parentObjectHandle_forSerializationOnly = _parentObject->getObjectHandle();

    return (theNewObject);
}

void CSceneObject::clearObjectCustomData_old()
{
    delete _customObjectData_old;
    _customObjectData_old = nullptr;
}

bool CSceneObject::getCustomDataEvents(std::map<std::string, bool>& customDataEvents, std::map<std::string, bool>& signalEvents)
{
    customDataEvents.clear();
    customObjectData.getDataEvents(customDataEvents);
    signalEvents.clear();
    customObjectData_volatile.getDataEvents(signalEvents);
    return customDataEvents.size() + signalEvents.size() > 0;
}

void CSceneObject::clearCustomDataEvents()
{
    customObjectData.clearDataEvents();
    customObjectData_volatile.clearDataEvents();
}

void CSceneObject::writeCustomDataBlock(bool tmpData, const char* dataName, const char* data, size_t dataLength)
{
    bool diff = false;
    if (tmpData)
        diff = customObjectData_volatile.setData(dataName, data, dataLength, false);
    else
        diff = customObjectData.setData(dataName, data, dataLength, false);

    if (diff && (!tmpData) && _isInScene && App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, nullptr, false);
        customObjectData.appendEventData(dataName, ev);
        //ev->appendKeyBuffer(cmd.c_str(), data, dataLength);
        App::worldContainer->pushEvent();
    }
}

std::string CSceneObject::readCustomDataBlock(bool tmpData, const char* dataName) const
{
    std::string retVal;
    if (tmpData)
        retVal = customObjectData_volatile.getData(dataName);
    else
        retVal = customObjectData.getData(dataName);
    return (retVal);
}

std::string CSceneObject::getAllCustomDataBlockTags(bool tmpData, size_t* cnt) const
{
    std::string retVal;
    if (tmpData)
        retVal = customObjectData_volatile.getAllTags(cnt);
    else
        retVal = customObjectData.getAllTags(cnt);
    return (retVal);
}

void CSceneObject::setObjectCustomData_old(int header, const char* data, int dataLength)
{
    if (_customObjectData_old == nullptr)
        _customObjectData_old = new CCustomData_old();
    _customObjectData_old->setData(header, data, dataLength);
}

int CSceneObject::getObjectCustomDataLength_old(int header) const
{
    if (_customObjectData_old == nullptr)
        return (0);
    return (_customObjectData_old->getDataLength(header));
}

void CSceneObject::getObjectCustomData_old(int header, char* data) const
{
    if (_customObjectData_old == nullptr)
        return;
    _customObjectData_old->getData(header, data);
}

void CSceneObject::setObjectMovementPreferredAxes(int p)
{ // bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    p &= 0x3f;
    bool diff = (_objectMovementPreferredAxes != p);
    if (diff)
    {
        int cb = _objectMovementPreferredAxes ^ p;
        _objectMovementPreferredAxes = p;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_movementPreferredAxes.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->appendKeyInt("movementPreferredAxes", _objectMovementPreferredAxes); // deprecated
#endif
            if (cb & 1)
                ev->appendKeyBool(propObject_movPrefTranslX.name, _objectMovementPreferredAxes & 1);
            if (cb & 2)
                ev->appendKeyBool(propObject_movPrefTranslY.name, _objectMovementPreferredAxes & 2);
            if (cb & 4)
                ev->appendKeyBool(propObject_movPrefTranslZ.name, _objectMovementPreferredAxes & 4);
            if (cb & 8)
                ev->appendKeyBool(propObject_movPrefRotX.name, _objectMovementPreferredAxes & 8);
            if (cb & 16)
                ev->appendKeyBool(propObject_movPrefRotY.name, _objectMovementPreferredAxes & 16);
            if (cb & 32)
                ev->appendKeyBool(propObject_movPrefRotZ.name, _objectMovementPreferredAxes & 32);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getObjectMovementPreferredAxes() const
{ // bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    return (_objectMovementPreferredAxes);
}

void CSceneObject::setObjectMovementOptions(int p)
{
    bool diff = (_objectMovementOptions != p);
    if (diff)
    {
        int cb = _objectMovementOptions ^ p;
        _objectMovementOptions = p;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_movementOptions.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->appendKeyInt("movementOptions", _objectMovementOptions); // deprecated
#endif
            if (cb & 1)
                ev->appendKeyBool(propObject_movTranslNoSim.name, (_objectMovementOptions & 1) == 0);
            if (cb & 2)
                ev->appendKeyBool(propObject_movTranslInSim.name, (_objectMovementOptions & 2) == 0);
            if (cb & 4)
                ev->appendKeyBool(propObject_movRotNoSim.name, (_objectMovementOptions & 4) == 0);
            if (cb & 8)
                ev->appendKeyBool(propObject_movRotInSim.name, (_objectMovementOptions & 8) == 0);
            if (cb & 16)
                ev->appendKeyBool(propObject_movAltTransl.name, (_objectMovementOptions & 16) == 0);
            if (cb & 32)
                ev->appendKeyBool(propObject_movAltRot.name, (_objectMovementOptions & 32) == 0);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getObjectMovementOptions() const
{
    return (_objectMovementOptions);
}

void CSceneObject::setObjectMovementRelativity(int index, int p)
{
    bool diff = (_objectMovementRelativity[index] != p);
    if (diff)
    {
        _objectMovementRelativity[index] = p;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_movementRelativity.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyIntArray(cmd, _objectMovementRelativity, 2);
            App::worldContainer->pushEvent();
        }
    }
}

int CSceneObject::getObjectMovementRelativity(int index) const
{
    return (_objectMovementRelativity[index]);
}

void CSceneObject::setObjectMovementStepSize(int index, double s)
{
    if (index == 0)
    {
        if (s < 0.0005)
            s = 0.0; // default
        else
        {
            double sc = 1.0;
            if ((s >= 0.0075) && (s < 0.075))
                sc = 10.0;
            if (s >= 0.075)
                sc = 100.0;
            if (s < 0.0015 * sc)
                s = 0.001 * sc;
            else if (s < 0.00375 * sc)
            {
                if (sc < 2.0)
                    s = 0.002 * sc;
                else
                    s = 0.0025 * sc;
            }
            else
                s = 0.005 * sc;
        }
    }
    else
    {
        if (s < 0.05 * degToRad)
            s = 0.0; // default
        else if (s < 1.5 * degToRad)
            s = 1.0 * degToRad;
        else if (s < 3.5 * degToRad)
            s = 2.0 * degToRad;
        else if (s < 7.5 * degToRad)
            s = 5.0 * degToRad;
        else if (s < 12.5 * degToRad)
            s = 10.0 * degToRad;
        else if (s < 22.5 * degToRad)
            s = 15.0 * degToRad;
        else if (s < 37.5 * degToRad)
            s = 30.0 * degToRad;
        else
            s = 45.0 * degToRad;
    }
    bool diff = (_objectMovementStepSize[index] != s);
    if (diff)
    {
        _objectMovementStepSize[index] = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_movementStepSize.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyDoubleArray(cmd, _objectMovementStepSize, 2);
            App::worldContainer->pushEvent();
        }
    }
}

double CSceneObject::getObjectMovementStepSize(int index) const
{
    return (_objectMovementStepSize[index]);
}

void CSceneObject::setMechanismID(int id)
{
    _mechanismID = id;
}

int CSceneObject::getMechanismID() const
{ // Make sure setMechanismID was called just before!!
    return (_mechanismID);
}

void CSceneObject::simulationAboutToStart()
{
}

void CSceneObject::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    setDynamicSimulationIconCode(sim_dynamicsimicon_none);
    _initialValuesInitialized = true;
    _modelProperty = (_modelProperty | sim_modelproperty_not_reset) - sim_modelproperty_not_reset;
    setDynamicFlag(0);
    _dynamicsResetFlag = false;

    _setMeasuredVelocity(C3Vector::zeroVector, C3Vector::zeroVector, C3Vector::zeroVector, 0.0);
    _previousPositionOrientationIsValid = false;
    if (_userScriptParameters != nullptr)
        _userScriptParameters->initializeInitialValues(simulationAlreadyRunning);

    // this section is special and reserved to local configuration restoration!
    //********************************
    _initialConfigurationMemorized = true;
    _initialParentUniqueId = -1; // -1 means there was no parent at begin
    CSceneObject* p = getParent();
    if (p != nullptr)
        _initialParentUniqueId = p->getObjectUid();
    _initialLocalPose = _localTransformation;
    _initialAbsPose = getCumulativeTransformation();
    //********************************

    _dynamicsTemporarilyDisabled = false;

    _initialMainPropertyOverride = _modelProperty;

    _initialVisibilityLayer = _visibilityLayer;
    if ((_objectProperty & sim_objectproperty_hiddenforsimulation) != 0)
        setVisibilityLayer(0);
}

void CSceneObject::simulationEnded_restoreHierarchy()
{ // called before simulationEnded
    if (_initialValuesInitialized && _initialConfigurationMemorized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
            long long int puid = -1;
            CSceneObject* p = getParent();
            if (p != nullptr)
                puid = p->getObjectUid();
            if (puid != _initialParentUniqueId)
            {
                CSceneObject* oldParent = App::currentWorld->sceneObjects->getObjectFromUid(_initialParentUniqueId);
                App::currentWorld->sceneObjects->setObjectParent(this, oldParent, true);
            }
        }
    }
}

void CSceneObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    // called after simulationEnded_restoreHierarchy
    setDynamicSimulationIconCode(sim_dynamicsimicon_none);
    setDynamicFlag(0);
    if (_userScriptParameters != nullptr)
        _userScriptParameters->simulationEnded();
    if (_initialValuesInitialized)
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0)
        {
            if (_initialConfigurationMemorized)
            { // this section is special and reserved to local configuration restoration!
                long long int puid = -1;
                CSceneObject* p = getParent();
                if (p != nullptr)
                    puid = p->getObjectUid();
                if (puid != _initialParentUniqueId)
                    setAbsoluteTransformation(_initialAbsPose);
                else
                    setLocalTransformation(_initialLocalPose);
                _modelProperty = _initialMainPropertyOverride;
                _initialConfigurationMemorized = false;
            }
        }

        if ((_objectProperty & sim_objectproperty_hiddenforsimulation) != 0)
            setVisibilityLayer(_initialVisibilityLayer);
    }
    _initialValuesInitialized = false;
}

void CSceneObject::computeBoundingBox()
{ // overridden
}

void CSceneObject::_setBB(const C7Vector& bbFrame, const C3Vector& bbHalfSize)
{
    if (((bbHalfSize - _bbHalfSize).getLength() > 0.0001) || ((bbFrame.X - _bbFrame.X).getLength() > 0.0001) ||
        (bbFrame.Q.getAngleBetweenQuaternions(_bbFrame.Q) > 0.001))
    {
        _bbFrame = bbFrame;
        _bbHalfSize = bbHalfSize;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = "boundingBox";
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            double p[7] = {_bbFrame.X(0), _bbFrame.X(1), _bbFrame.X(2), _bbFrame.Q(1),
                           _bbFrame.Q(2), _bbFrame.Q(3), _bbFrame.Q(0)};
#if SIM_EVENT_PROTOCOL_VERSION == 2
            ev->openKeyMap(cmd);
            ev->appendKeyDoubleArray("pose", p, 7);
            ev->appendKeyDoubleArray("hsize", _bbHalfSize.data, 3);
            ev->closeArrayOrMap();
#endif
            ev->appendKeyDoubleArray(propObject_bbPose.name, p, 7);
            ev->appendKeyDoubleArray(propObject_bbHsize.name, _bbHalfSize.data, 3);
            App::worldContainer->pushEvent();
        }
    }
}

C7Vector CSceneObject::getBB(C3Vector* bbHalfSize) const
{
    if (bbHalfSize != nullptr)
        bbHalfSize[0] = _bbHalfSize;
    return (_bbFrame);
}

C3Vector CSceneObject::getBBHSize() const
{
    return (_bbHalfSize);
}

void CSceneObject::temporarilyDisableDynamicTree()
{
    _dynamicsTemporarilyDisabled = true;
    for (size_t i = 0; i < _childList.size(); i++)
        _childList[i]->temporarilyDisableDynamicTree();
}

void CSceneObject::setDynamicSimulationIconCode(int c)
{
    bool diff = (_dynamicSimulationIconCode != c);
    if (diff)
    {
        _dynamicSimulationIconCode = c;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_dynamicIcon.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _dynamicSimulationIconCode);
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
#endif
    }
}

int CSceneObject::getDynamicSimulationIconCode() const
{
    return _dynamicSimulationIconCode;
}

void CSceneObject::setAssemblingLocalTransformation(const C7Vector& tr)
{
    _assemblingLocalTransformation = tr;
}
C7Vector CSceneObject::getAssemblingLocalTransformation() const
{
    return (_assemblingLocalTransformation);
}
void CSceneObject::setAssemblingLocalTransformationIsUsed(bool u)
{
    _assemblingLocalTransformationIsUsed = u;
}

bool CSceneObject::getAssemblingLocalTransformationIsUsed()
{
    return (_assemblingLocalTransformationIsUsed);
}

void CSceneObject::setAssemblyMatchValues(bool asChild, const char* str)
{
    std::vector<std::string>* v;
    if (asChild)
        v = &_assemblyMatchValuesChild;
    else
        v = &_assemblyMatchValuesParent;
    v->clear();
    std::vector<std::string> words;
    tt::separateWords(str, ',', words);
    std::map<std::string, bool> hist;
    for (size_t i = 0; i < words.size(); i++)
    {
        std::string strr(words[i]);
        tt::removeSpacesAtBeginningAndEnd(strr);
        tt::removeIllegalCharacters(strr, false);
        if (strr.size() > 0)
        {
            std::map<std::string, bool>::iterator it = hist.find(strr);
            if (it == hist.end())
            { // avoid doubles
                hist[strr] = true;
                v->push_back(strr);
            }
        }
    }
}

std::string CSceneObject::getAssemblyMatchValues(bool asChild) const
{
    const std::vector<std::string>* v;
    if (asChild)
        v = &_assemblyMatchValuesChild;
    else
        v = &_assemblyMatchValuesParent;
    std::string retVal;
    for (size_t i = 0; i < v->size(); i++)
    {
        if (retVal.size() > 0)
            retVal += ",";
        retVal += v->at(i);
    }
    return (retVal);
}

const std::vector<std::string>* CSceneObject::getChildAssemblyMatchValuesPointer() const
{
    return (&_assemblyMatchValuesChild);
}

bool CSceneObject::doesParentAssemblingMatchValuesMatchWithChild(
    const std::vector<std::string>* assemblingChildMatchValues, bool ignoreDefaultNames /*=false*/) const
{
    for (size_t i = 0; i < _assemblyMatchValuesParent.size(); i++)
    {
        for (size_t j = 0; j < assemblingChildMatchValues->size(); j++)
        {
            if ((!ignoreDefaultNames) || (_assemblyMatchValuesParent[i].compare("default") != 0))
            {
                if (assemblingChildMatchValues->at(j).compare(_assemblyMatchValuesParent[i]) == 0)
                    return (true);
            }
        }
    }
    return (false);
}

int CSceneObject::getAllChildrenThatMayBecomeAssemblyParent(const std::vector<std::string>* assemblingChildMatchValues,
                                                            std::vector<CSceneObject*>& objects) const
{
    for (size_t i = 0; i < getChildCount(); i++)
    {
        CSceneObject* child = getChildFromIndex(i);
        if (child->doesParentAssemblingMatchValuesMatchWithChild(assemblingChildMatchValues, true))
            objects.push_back(child);
        child->getAllChildrenThatMayBecomeAssemblyParent(assemblingChildMatchValues, objects);
    }
    return (int(objects.size()));
}

void CSceneObject::generateDnaString()
{
    _dnaString = utils::generateUniqueString();
    if (_isInScene && App::worldContainer->getEventsEnabled())
    {
        const char* cmd = propObject_dna.name;
        CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->appendKeyBuff(cmd, (unsigned char*)_dnaString.data(), _dnaString.size());
        App::worldContainer->pushEvent();
    }
}

std::string CSceneObject::getDnaString() const
{
    return (_dnaString);
}

void CSceneObject::setCopyString(const char* str)
{ // multipurpose. Will be copied during copy op.
    _copyString = str;
}

std::string CSceneObject::getCopyString() const
{
    return _copyString;
}

std::string CSceneObject::getUniquePersistentIdString() const
{
    return _uniquePersistentIdString;
}

int CSceneObject::getScriptsInTree(std::vector<SScriptInfo>& scripts, int scriptType, bool legacyEmbeddedScripts, int depth /* = 0 */)
{ // For a given depth level, scripts are sorted from high to low priority. Will append to "scripts"
    int maxDepth = -1;
    if ((getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) == 0)
    {
        if (scriptType == sim_scripttype_customization)
        {
            CScriptObject* it = nullptr;
            if (legacyEmbeddedScripts)
                it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customization, _objectHandle);
            else
            {
                if ((_objectType == sim_sceneobject_script) && (((CScript*)this)->scriptObject != nullptr) && (((CScript*)this)->scriptObject->getScriptType() == sim_scripttype_customization))
                    it = ((CScript*)this)->scriptObject;
            }
            if ((it != nullptr) && (!it->getScriptIsDisabled()))
            {
                SScriptInfo s;
                s.scriptHandle = it->getScriptHandle();
                s.depth = depth;
                scripts.push_back(s);
                maxDepth = depth;
            }
        }

        if (((scriptType & 0x0f) == sim_scripttype_simulation) && (!App::currentWorld->simulation->isSimulationStopped()))
        {
            CScriptObject* it = nullptr;
            if (legacyEmbeddedScripts)
                it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_simulation, _objectHandle);
            else
            {
                if ((_objectType == sim_sceneobject_script) && (((CScript*)this)->scriptObject != nullptr) && (((CScript*)this)->scriptObject->getScriptType() == sim_scripttype_simulation))
                    it = ((CScript*)this)->scriptObject;
            }
            if ((it != nullptr) && (!it->getScriptIsDisabled()))
            {
                SScriptInfo s;
                s.scriptHandle = it->getScriptHandle();
                s.depth = depth;
                scripts.push_back(s);
                maxDepth = depth;
            }
        }

        // Now the children themselves:
        std::vector<CSceneObject*> children;
        std::vector<CSceneObject*> childrenNormalPriority;
        std::vector<CSceneObject*> childrenLastPriority;
        for (size_t i = 0; i < getChildCount(); i++)
        {
            CSceneObject* it = getChildFromIndex(i);
            int p = it->getScriptExecPriority();
            if (p == sim_scriptexecorder_first)
                children.push_back(it);
            if (p == sim_scriptexecorder_normal)
                childrenNormalPriority.push_back(it);
            if (p == sim_scriptexecorder_last)
                childrenLastPriority.push_back(it);
        }
        children.insert(children.end(), childrenNormalPriority.begin(), childrenNormalPriority.end());
        children.insert(children.end(), childrenLastPriority.begin(), childrenLastPriority.end());
        for (size_t i = 0; i < children.size(); i++)
            maxDepth = std::max<int>(maxDepth, children[i]->getScriptsInTree(scripts, scriptType, legacyEmbeddedScripts, depth + 1));
    }
    return maxDepth;
}

size_t CSceneObject::getAttachedScripts(std::vector<CScriptObject*>& scripts, int scriptType, bool legacyEmbeddedScripts)
{ // will append to "scripts"!
    if (scriptType == -1)
    {
        getAttachedScripts(scripts, sim_scripttype_simulation, legacyEmbeddedScripts);
        getAttachedScripts(scripts, sim_scripttype_customization, legacyEmbeddedScripts);
    }
    else
    {
        if ((getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) == 0)
        {
            if (legacyEmbeddedScripts)
            {
                CScriptObject* it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(scriptType, _objectHandle);
                if ( (it != nullptr) && (!it->getScriptIsDisabled()) )
                {
                    if (scriptType == sim_scripttype_customization)
                        scripts.push_back(it);
                    if ((scriptType == sim_scripttype_simulation) && (!App::currentWorld->simulation->isSimulationStopped()))
                        scripts.push_back(it);
                }
            }
            else
            { // all scripts attached to this object:
                std::vector<CScriptObject*> childrenNormalPriority;
                std::vector<CScriptObject*> childrenLastPriority;
                for (size_t i = 0; i < getChildCount(); i++)
                {
                    CSceneObject* c = getChildFromIndex(i);
                    if (c->getObjectType() == sim_sceneobject_script)
                    {
                        CScript* it = (CScript*)c;
                        if ((it->scriptObject != nullptr) && (it->scriptObject->getScriptType() == scriptType) && (!it->scriptObject->getScriptIsDisabled()))
                        {
                            int p = it->getScriptExecPriority();
                            if (p == sim_scriptexecorder_first)
                                scripts.push_back(it->scriptObject);
                            if (p == sim_scriptexecorder_normal)
                                childrenNormalPriority.push_back(it->scriptObject);
                            if (p == sim_scriptexecorder_last)
                                childrenLastPriority.push_back(it->scriptObject);
                        }
                    }
                }
                scripts.insert(scripts.end(), childrenNormalPriority.begin(), childrenNormalPriority.end());
                scripts.insert(scripts.end(), childrenLastPriority.begin(), childrenLastPriority.end());
            }
        }
    }
    return scripts.size();
}

void CSceneObject::getScriptsInChain(std::vector<int>& scripts, int scriptType, bool legacyEmbeddedScripts)
{ // We go from leaf to root, child, then customization scripts. For a given depth level, scripts are sorted from high to low priority
    // will append to "scripts"
    if (scriptType == -1)
    {
        getScriptsInChain(scripts, sim_scripttype_simulation, legacyEmbeddedScripts);
        getScriptsInChain(scripts, sim_scripttype_customization, legacyEmbeddedScripts);
    }
    else
    {
        scriptType = scriptType & 0x0f;
        if ((getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) == 0)
        {
            if (legacyEmbeddedScripts)
            {
                CScriptObject* it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(scriptType, _objectHandle);
                if ( (it != nullptr) && (!it->getScriptIsDisabled()) )
                {
                    if (scriptType == sim_scripttype_customization)
                        scripts.push_back(it->getScriptHandle());
                    if ((scriptType == sim_scripttype_simulation) && (!App::currentWorld->simulation->isSimulationStopped()))
                        scripts.push_back(it->getScriptHandle());
                }
            }
            else
            { // all scripts attached to this object:
                std::vector<int> childrenNormalPriority;
                std::vector<int> childrenLastPriority;
                for (size_t i = 0; i < getChildCount(); i++)
                {
                    CSceneObject* c = getChildFromIndex(i);
                    if (c->getObjectType() == sim_sceneobject_script)
                    {
                        CScript* it = (CScript*)c;
                        if ((it->scriptObject != nullptr) && (it->scriptObject->getScriptType() == scriptType) && (!it->scriptObject->getScriptIsDisabled()))
                        {
                            int p = it->getScriptExecPriority();
                            if (p == sim_scriptexecorder_first)
                                scripts.push_back(it->getObjectHandle());
                            if (p == sim_scriptexecorder_normal)
                                childrenNormalPriority.push_back(it->getObjectHandle());
                            if (p == sim_scriptexecorder_last)
                                childrenLastPriority.push_back(it->getObjectHandle());
                        }
                    }
                }
                scripts.insert(scripts.end(), childrenNormalPriority.begin(), childrenNormalPriority.end());
                scripts.insert(scripts.end(), childrenLastPriority.begin(), childrenLastPriority.end());
            }
            CSceneObject* parent = getParent();
            if (parent != nullptr)
                parent->getScriptsInChain(scripts, scriptType, legacyEmbeddedScripts);
            else
            {
                if (!legacyEmbeddedScripts)
                {
                    if (_objectType == sim_sceneobject_script)
                    {
                        CScript* it = (CScript*)this;
                        if ((it->scriptObject != nullptr) && (it->scriptObject->getScriptType() == scriptType) && (!it->scriptObject->getScriptIsDisabled()))
                            scripts.push_back(it->scriptObject->getScriptHandle());
                    }
                }
            }
        }
    }
}

void CSceneObject::_setLocalTransformation_send(const C7Vector& tr) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setObjectLocalTransformation(_ikPluginCounterpartHandle,
                                                                                       _localTransformation);
}

void CSceneObject::_setParent_send(int parentHandle) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int p = -1;
        if (getParent() != nullptr)
            p = getParent()->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setObjectParent(_ikPluginCounterpartHandle, p);
    }
}

bool CSceneObject::setParent(CSceneObject* parent)
{
    bool diff = (_parentObject != parent);
    if (diff)
    {
        _parentObject = parent;
        long long int pUid = -1;
        int pH = -1;
        if (_parentObject != nullptr)
        {
            pUid = _parentObject->getObjectUid();
            pH = _parentObject->getObjectHandle();
        }
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_parentUid.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyInt(cmd, pUid);
            ev->appendKeyInt(propObject_parentHandle.name, pH);
            App::worldContainer->pushEvent();
        }
        _setParent_send(pH);
    }
    return (diff);
}

void CSceneObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {                            // Storing
            ar.storeDataName("3do"); // Scene object identifier. Needed for forward compatibility when
                                     // trying to load an object type that doesn't yet exist!
            ar << ((unsigned char)57) << ((unsigned char)58) << ((unsigned char)59);
            ar.flush();

            C7Vector tr = getLocalTransformation();
            ar.storeDataName("_fq");
            if (_ignorePosAndCameraOrthoviewSize_forUndoRedo)
                tr.setIdentity();
            ar << tr.Q(0) << tr.Q(1) << tr.Q(2) << tr.Q(3) << tr.X(0) << tr.X(1) << tr.X(2);
            ar.flush();

            ar.storeDataName("_lt");
            ar << _assemblingLocalTransformation.Q(0) << _assemblingLocalTransformation.Q(1)
               << _assemblingLocalTransformation.Q(2) << _assemblingLocalTransformation.Q(3)
               << _assemblingLocalTransformation.X(0) << _assemblingLocalTransformation.X(1)
               << _assemblingLocalTransformation.X(2);
            ar.flush();

            ar.storeDataName("Am2");
            ar << int(_assemblyMatchValuesChild.size());
            for (size_t i = 0; i < _assemblyMatchValuesChild.size(); i++)
                ar << _assemblyMatchValuesChild[i];
            ar << int(_assemblyMatchValuesParent.size());
            for (size_t i = 0; i < _assemblyMatchValuesParent.size(); i++)
                ar << _assemblyMatchValuesParent[i];
            ar.flush();

            ar.storeDataName("Ids");
            int parentID = -1;
            if (getParent() != nullptr)
                parentID = getParent()->getObjectHandle();
            ar << _objectHandle << parentID;
            ar.flush();

            ar.storeDataName("Ali"); // keep this before "Nme"
            ar << _objectAlias;
            ar.flush();

            ar.storeDataName("Anm"); // keep this before "Nme"
            ar << _objectAltName_old;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName_old;
            ar.flush();

            ar.storeDataName("Hci");
            ar << _hierarchyColorIndex;
            ar.flush();

            ar.storeDataName("Sci");
            ar << _collectionSelfCollisionIndicator;
            ar.flush();

            ar.storeDataName("Op2");
            int objProp =
                _objectProperty | sim_objectproperty_reserved5; // Needed for backward compatibility (still in serialization version 15)
            ar << objProp;
            ar.flush();

            // Keep a while for backward compatibility (19/4/2017) (in case people want to return to a previous
            // CoppeliaSim version):
            ar.storeDataName("Va2");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _modelBase);
            SIM_SET_CLEAR_BIT(dummy, 1, (_objectMovementOptions & 2) != 0);
            SIM_SET_CLEAR_BIT(dummy, 2, (_objectMovementOptions & 1) != 0);
            SIM_SET_CLEAR_BIT(dummy, 7, _assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va3");
            dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _modelBase);
            SIM_SET_CLEAR_BIT(dummy, 1, (_objectMovementOptions & 2) != 0);
            SIM_SET_CLEAR_BIT(dummy, 2, (_objectMovementOptions & 1) != 0);
            // 3 is reserved and should remain at false
            SIM_SET_CLEAR_BIT(dummy, 4, (_objectMovementOptions & 8) != 0);
            SIM_SET_CLEAR_BIT(dummy, 5, (_objectMovementOptions & 4) != 0);
            SIM_SET_CLEAR_BIT(dummy, 7, _assemblingLocalTransformationIsUsed);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va4");
            dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, (_objectMovementOptions & 16) != 0);
            SIM_SET_CLEAR_BIT(dummy, 1, (_objectMovementOptions & 32) != 0);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Omp");
            ar << _localObjectSpecialProperty;
            ar.flush();

            ar.storeDataName("Mpo");
            ar << _modelProperty;
            ar.flush();

            ar.storeDataName("Lar");
            ar << (unsigned short)_visibilityLayer;
            ar.flush();

            ar.storeDataName("Sep");
            ar << _scriptExecPriority;
            ar.flush();

            ar.storeDataName("Cor");
            ar << _childOrder;
            ar.flush();

            ar.storeDataName("_m5");
            ar << _objectMovementPreferredAxes << _objectMovementRelativity[0];
            ar << _objectMovementStepSize[0];
            ar.flush();

            ar.storeDataName("_mr");
            ar << _objectMovementRelativity[1];
            ar << _objectMovementStepSize[1];
            ar.flush();

            ar.storeDataName("_fa");
            ar << _sizeFactor;
            ar.flush();

            ar.storeDataName("_fb");
            ar << _sizeValues[0] << _sizeValues[1] << _sizeValues[2];
            ar.flush();

            if (customObjectData.getDataCount() != 0)
            {
                ar.storeDataName("Cda");
                ar.setCountingMode();
                customObjectData.serializeData(ar, nullptr);
                if (ar.setWritingMode())
                    customObjectData.serializeData(ar, nullptr);
            }

            if (_customObjectData_old != nullptr)
            { // keep for backward compatibility (e.g. until V4.4.0)
                ar.storeDataName("Cod");
                ar.setCountingMode();
                _customObjectData_old->serializeData(ar, nullptr, -1);
                if (ar.setWritingMode())
                    _customObjectData_old->serializeData(ar, nullptr, -1);
            }

            if (_userScriptParameters != nullptr)
            {
                ar.storeDataName("Lsp");
                ar.setCountingMode();
                _userScriptParameters->serialize(ar);
                if (ar.setWritingMode())
                    _userScriptParameters->serialize(ar);
            }

            // _customReferencedHandles with tag "" handled separately, for backw. comp.
            auto it = _customReferencedHandles.find("");
            if (it != _customReferencedHandles.end())
            {
                if (it->second.size() > 0)
                {
                    ar.storeDataName("Crh");
                    ar << int(it->second.size());
                    for (size_t i = 0; i < it->second.size(); i++)
                    {
                        ar << it->second[i].generalObjectType;
                        ar << it->second[i].generalObjectHandle;
                        ar << int(0);
                    }
                    ar.flush();
                }
            }

            // _customReferencedOriginalHandles with tag "" handled separately, for backw. comp.
            auto itt = _customReferencedOriginalHandles.find("");
            if (itt != _customReferencedOriginalHandles.end())
            {
                if (itt->second.size() > 0)
                {
                    ar.storeDataName("Orh");
                    ar << int(itt->second.size());
                    for (size_t i = 0; i < itt->second.size(); i++)
                    {
                        ar << itt->second[i].generalObjectType;
                        ar << itt->second[i].generalObjectHandle;
                        if (itt->second[i].generalObjectHandle >= 0)
                            ar << itt->second[i].uniquePersistentIdString;
                    }
                    ar.flush();
                }
            }

            for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
            {
                if (it->first != "")
                {
                    if (it->second.size() > 0)
                    {
                        ar.storeDataName("Cr2");
                        ar << it->first;
                        ar << int(it->second.size());
                        for (size_t i = 0; i < it->second.size(); i++)
                            ar << it->second[i].generalObjectHandle;
                        ar.flush();
                    }
                }
            }

            for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
            {
                if (it->first != "")
                {
                    if (it->second.size() > 0)
                    {
                        ar.storeDataName("Or2");
                        ar << it->first;
                        ar << int(it->second.size());
                        for (size_t i = 0; i < it->second.size(); i++)
                        {
                            ar << it->second[i].generalObjectHandle;
                            ar << it->second[i].uniquePersistentIdString;
                        }
                        ar.flush();
                    }
                }
            }

            ar.storeDataName("Ack");
            ar << _modelAcknowledgement;
            ar.flush();

            ar.storeDataName("Ups");
            ar << _dnaString;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            ar.storeDataName("_do");
            ar << _transparentObjectDistanceOffset;
            ar.flush();

            ar.storeDataName("Avo");
            ar << _authorizedViewableObjects;
            ar.flush();

            ar.storeDataName("Rst");
            ar << _extensionString;
            ar.flush();

            ar.storeDataName(SER_NEXT_STEP);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool hasAltName = false;
            bool hasAlias = false;
            bool _assemblingLocalTransformationIsUsed_compatibility = false;
            while (theName.compare(SER_NEXT_STEP) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_NEXT_STEP) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("3do") == 0)
                    { // 3D object identifier. Needed for forward compatibility when trying to load an object type that
                        // doesn't yet exist!
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy >> dummy >> dummy;
                    }
                    if (theName.compare("Cfq") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        C7Vector tr;
                        float a[7];
                        for (size_t ai = 0; ai < 7; ai++)
                            ar >> a[ai];
                        tr.Q(0) = (double)a[0];
                        tr.Q(1) = (double)a[1];
                        tr.Q(2) = (double)a[2];
                        tr.Q(3) = (double)a[3];
                        tr.X(0) = (double)a[4];
                        tr.X(1) = (double)a[5];
                        tr.X(2) = (double)a[6];
                        tr.Q.normalize(); // we read from float. Make sure we are perfectly normalized!
                        setLocalTransformation(tr);
                    }

                    if (theName.compare("_fq") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        C7Vector tr;
                        ar >> tr.Q(0) >> tr.Q(1) >> tr.Q(2) >> tr.Q(3) >> tr.X(0) >> tr.X(1) >> tr.X(2);
                        tr.Q.normalize();
                        setLocalTransformation(tr);
                    }

                    if (theName.compare("Hci") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _hierarchyColorIndex;
                    }
                    if (theName.compare("Sci") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _collectionSelfCollisionIndicator;
                    }
                    if (theName.compare("Alt") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a[7];
                        for (size_t ai = 0; ai < 7; ai++)
                            ar >> a[ai];
                        _assemblingLocalTransformation.Q(0) = (double)a[0];
                        _assemblingLocalTransformation.Q(1) = (double)a[1];
                        _assemblingLocalTransformation.Q(2) = (double)a[2];
                        _assemblingLocalTransformation.Q(3) = (double)a[3];
                        _assemblingLocalTransformation.X(0) = (double)a[4];
                        _assemblingLocalTransformation.X(1) = (double)a[5];
                        _assemblingLocalTransformation.X(2) = (double)a[6];
                        _assemblingLocalTransformation.Q
                            .normalize(); // we read from float. Make sure we are perfectly normalized!

                        if (ar.getSerializationVersionThatWroteThisFile() < 20)
                        {
                            C3Vector v(_assemblingLocalTransformation.Q(1), _assemblingLocalTransformation.Q(2),
                                       _assemblingLocalTransformation.Q(3));
                            if ((_assemblingLocalTransformation.X.getLength() > 0.0) || (v.getLength() > 0.0))
                                _assemblingLocalTransformationIsUsed_compatibility = true;
                        }
                    }

                    if (theName.compare("_lt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _assemblingLocalTransformation.Q(0) >> _assemblingLocalTransformation.Q(1) >>
                            _assemblingLocalTransformation.Q(2) >> _assemblingLocalTransformation.Q(3) >>
                            _assemblingLocalTransformation.X(0) >> _assemblingLocalTransformation.X(1) >>
                            _assemblingLocalTransformation.X(2);
                        _assemblingLocalTransformation.Q.normalize();
                        if (ar.getSerializationVersionThatWroteThisFile() < 20)
                        {
                            C3Vector v(_assemblingLocalTransformation.Q(1), _assemblingLocalTransformation.Q(2),
                                       _assemblingLocalTransformation.Q(3));
                            if ((_assemblingLocalTransformation.X.getLength() > 0.0) || (v.getLength() > 0.0))
                                _assemblingLocalTransformationIsUsed_compatibility = true;
                        }
                    }

                    if (theName.compare("Amv") == 0)
                    { // Keep for backward compatibility (31/3/2017)
                        noHit = false;
                        ar >> byteQuantity;
                        int child, parent;
                        ar >> child >> parent;
                        _assemblyMatchValuesChild.clear();
                        if (child == 0)
                            _assemblyMatchValuesChild.push_back("default");
                        else
                            _assemblyMatchValuesChild.push_back(std::to_string(child));
                        _assemblyMatchValuesParent.clear();
                        if (parent == 0)
                            _assemblyMatchValuesParent.push_back("default");
                        else
                            _assemblyMatchValuesParent.push_back(std::to_string(parent));
                    }
                    if (theName.compare("Am2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        int childCnt, parentCnt;
                        std::string word;
                        ar >> childCnt;
                        _assemblyMatchValuesChild.clear();
                        for (int i = 0; i < childCnt; i++)
                        {
                            ar >> word;
                            _assemblyMatchValuesChild.push_back(word);
                        }
                        ar >> parentCnt;
                        _assemblyMatchValuesParent.clear();
                        for (int i = 0; i < parentCnt; i++)
                        {
                            ar >> word;
                            _assemblyMatchValuesParent.push_back(word);
                        }
                    }
                    if (theName.compare("Ids") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectHandle >> _parentObjectHandle_forSerializationOnly;
                    }
                    if (theName.compare("Ali") == 0)
                    {
                        hasAltName = true;
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectAlias;
                        _objectTempAlias = _objectAlias;
                        hasAlias = true;
                    }
                    if (theName.compare("Anm") == 0)
                    {
                        hasAltName = true;
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectAltName_old;
                    }
                    if (theName.compare("Nme") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectName_old;
                        if (!hasAltName)
                            _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
                        _objectTempName_old = _objectName_old;
                        if (!hasAlias)
                            _objectAlias = _objectName_old.substr(0, _objectName_old.find('#'));
                    }
                    if (theName.compare("Op2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectProperty;
                    }
                    if (theName.compare("Var") == 0)
                    { // Keep for backward compatibility (31/3/2017)
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase = SIM_IS_BIT_SET(dummy, 0);
                        if (SIM_IS_BIT_SET(dummy, 1))
                            _objectMovementOptions = _objectMovementOptions | 10;
                        if (SIM_IS_BIT_SET(dummy, 2))
                            _objectMovementOptions = _objectMovementOptions | 5;
                        if (SIM_IS_BIT_SET(dummy, 3))
                            _objectProperty |= sim_objectproperty_ignoreviewfitting;
                        // reserved since 9/6/2013 _useSpecialLocalTransformationWhenAssembling=SIM_IS_BIT_SET(dummy,4);
                        bool assemblyCanHaveChildRole = !SIM_IS_BIT_SET(dummy, 5);
                        bool assemblyCanHaveParentRole = !SIM_IS_BIT_SET(dummy, 6);
                        _assemblingLocalTransformationIsUsed = SIM_IS_BIT_SET(dummy, 7);

                        if (ar.getSerializationVersionThatWroteThisFile() < 20)
                            _assemblingLocalTransformationIsUsed = _assemblingLocalTransformationIsUsed_compatibility;

                        if (!assemblyCanHaveChildRole)
                            _assemblyMatchValuesChild.clear();
                        if (!assemblyCanHaveParentRole)
                            _assemblyMatchValuesParent.clear();
                    }
                    if (theName.compare("Va2") == 0)
                    { // Keep for backward compatibility (19/4/2017)
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase = SIM_IS_BIT_SET(dummy, 0);
                        if (SIM_IS_BIT_SET(dummy, 1))
                            _objectMovementOptions = _objectMovementOptions | 10;
                        if (SIM_IS_BIT_SET(dummy, 2))
                            _objectMovementOptions = _objectMovementOptions | 5;
                        if (SIM_IS_BIT_SET(dummy, 3))
                            _objectProperty |= sim_objectproperty_ignoreviewfitting;
                        _assemblingLocalTransformationIsUsed = SIM_IS_BIT_SET(dummy, 7);
                    }
                    if (theName.compare("Va3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _modelBase = SIM_IS_BIT_SET(dummy, 0);
                        if (SIM_IS_BIT_SET(dummy, 1))
                            _objectMovementOptions = _objectMovementOptions | 2;
                        if (SIM_IS_BIT_SET(dummy, 2))
                            _objectMovementOptions = _objectMovementOptions | 1;
                        if (SIM_IS_BIT_SET(dummy, 3))
                            _objectProperty |= sim_objectproperty_ignoreviewfitting;
                        if (SIM_IS_BIT_SET(dummy, 4))
                            _objectMovementOptions = _objectMovementOptions | 8;
                        if (SIM_IS_BIT_SET(dummy, 5))
                            _objectMovementOptions = _objectMovementOptions | 4;
                        _assemblingLocalTransformationIsUsed = SIM_IS_BIT_SET(dummy, 7);
                    }
                    if (theName.compare("Va4") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        if (SIM_IS_BIT_SET(dummy, 0))
                            _objectMovementOptions = _objectMovementOptions | 16;
                        if (SIM_IS_BIT_SET(dummy, 1))
                            _objectMovementOptions = _objectMovementOptions | 32;
                    }

                    if (theName.compare("Omp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _localObjectSpecialProperty;
                    }
                    if (theName.compare("Mpo") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _modelProperty;
                        if (!_modelBase)
                            _modelProperty = sim_modelproperty_not_model;
                    }
                    if (theName.compare("Lar") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned short vl;
                        ar >> vl;
                        _visibilityLayer = vl;
                    }
                    if (theName.compare("Sep") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _scriptExecPriority;
                    }
                    if (theName.compare("Cor") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _childOrder;
                    }
                    if (theName.compare("Om5") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectMovementPreferredAxes >> _objectMovementRelativity[0];
                        float bla;
                        ar >> bla;
                        _objectMovementStepSize[0] = (double)bla;
                    }

                    if (theName.compare("_m5") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectMovementPreferredAxes >> _objectMovementRelativity[0];
                        ar >> _objectMovementStepSize[0];
                    }

                    if (theName.compare("Omr") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectMovementRelativity[1];
                        float bla;
                        ar >> bla;
                        _objectMovementStepSize[1] = (double)bla;
                    }

                    if (theName.compare("_mr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectMovementRelativity[1];
                        ar >> _objectMovementStepSize[1];
                    }

                    if (theName.compare("Cda") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        customObjectData.serializeData(ar, nullptr);
                    }
                    if (theName.compare("Cod") == 0)
                    { // keep for backward compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        _customObjectData_old = new CCustomData_old();
                        _customObjectData_old->serializeData(ar, nullptr, -1);
                        if (customObjectData.getDataCount() == 0)
                            _customObjectData_old->initNewFormat(customObjectData, true);
                    }
                    if (theName.compare("Lsp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _userScriptParameters = new CUserParameters();
                        _userScriptParameters->serialize(ar);
                        if (_userScriptParameters->userParamEntries.size() == 0)
                        {
                            delete _userScriptParameters;
                            _userScriptParameters = nullptr;
                        }
                        else
                        {
                            if (CSimFlavor::getBoolVal(18))
                                App::logMsg(sim_verbosity_errors, "Contains script simulation parameters...");
                        }
                    }
                    if (theName.compare("Crh") == 0)
                    { // treat tag "" separately, for backw. compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt, dummy;
                        ar >> cnt;
                        _customReferencedHandles.insert({"", {}});
                        auto it = _customReferencedHandles.find("");
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomRefs r;
                            ar >> r.generalObjectType;
                            ar >> r.generalObjectHandle;
                            ar >> dummy;
                            it->second.push_back(r);
                        }
                    }
                    if (theName.compare("Orh") == 0)
                    { // treat tag "" separately, for backw. compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt;
                        ar >> cnt;
                        _customReferencedOriginalHandles.insert({"", {}});
                        auto it = _customReferencedOriginalHandles.find("");
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomOriginalRefs r;
                            ar >> r.generalObjectType;
                            ar >> r.generalObjectHandle;
                            if (r.generalObjectHandle >= 0)
                                ar >> r.uniquePersistentIdString;
                            it->second.push_back(r);
                        }
                    }
                    if (theName.compare("Cr2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        std::string key;
                        ar >> key;
                        int cnt;
                        ar >> cnt;
                        _customReferencedHandles.insert({key.c_str(), {}});
                        auto it = _customReferencedHandles.find(key.c_str());
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomRefs r;
                            r.generalObjectType = sim_objecttype_sceneobject;
                            ar >> r.generalObjectHandle;
                            it->second.push_back(r);
                        }
                    }
                    if (theName.compare("Or2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        std::string key;
                        ar >> key;
                        int cnt;
                        ar >> cnt;
                        _customReferencedOriginalHandles.insert({key.c_str(), {}});
                        auto it = _customReferencedOriginalHandles.find(key.c_str());
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomOriginalRefs r;
                            r.generalObjectType = sim_objecttype_sceneobject;
                            ar >> r.generalObjectHandle;
                            ar >> r.uniquePersistentIdString;
                            it->second.push_back(r);
                        }
                    }

                    if (theName.compare("Sfa") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _sizeFactor = (double)bla;
                    }

                    if (theName.compare("_fa") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sizeFactor;
                    }

                    if (theName.compare("Sfb") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        _sizeValues[0] = (double)bla;
                        _sizeValues[1] = (double)bli;
                        _sizeValues[2] = (double)blo;
                    }

                    if (theName.compare("_fb") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _sizeValues[0] >> _sizeValues[1] >> _sizeValues[2];
                    }

                    if (theName.compare("Ack") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _modelAcknowledgement;
                    }
                    if (theName.compare("Ups") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dnaString;
                        if (_dnaString.size() == 0)
                            generateDnaString();
                    }
                    if (theName.compare("Uis") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (theName.compare("Tdo") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _transparentObjectDistanceOffset = (double)bla;
                    }

                    if (theName.compare("_do") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _transparentObjectDistanceOffset;
                    }

                    if (theName.compare("Avo") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _authorizedViewableObjects;
                    }
                    if (theName.compare("Rst") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _extensionString;
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            //*************************************************************
            // For backward compatibility 13/09/2011
            if ((_objectProperty & sim_objectproperty_reserved5) == 0)
            { // this used to be the sim_objectproperty_visible property. If it wasn't set in the past, we now try to
                // hide it in a hidden layer:
                if (_visibilityLayer < 256)
                    _visibilityLayer = _visibilityLayer * 256;
            }
            else
                _objectProperty -= sim_objectproperty_reserved5;
            //*************************************************************

            //*************************************************************
            // For old models to support the DNA-thing by default:
            if ((ar.getCoppeliaSimVersionThatWroteThisFile() < 30003) && getModelBase())
            {
                _dnaString = "1234567890123456";
                std::string a(utils::generateUniqueAlphaNumericString());
                while (a.length() < 16)
                    a = a + "*";
                std::string b("1234567890123456");
                int fbp = ar.getFileBufferReadPointer();
                b[2] = ((unsigned char*)&fbp)[0];
                b[3] = ((unsigned char*)&fbp)[1];
                b[4] = ((unsigned char*)&fbp)[2];
                b[5] = ((unsigned char*)&fbp)[3];
                for (int i = 0; i < 16; i++)
                {
                    _dnaString[i] += a[i];
                    _dnaString[i] += b[i];
                }
            }
            //*************************************************************
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlPushNewNode("common");

            if (exhaustiveXml)
                ar.xmlAddNode_string(propObject_alias.name, _objectAlias.c_str());
            else
                ar.xmlAddNode_string(propObject_alias.name, (_objectAlias + "*" + std::to_string(_objectHandle)).c_str());
            ar.xmlAddNode_comment(" 'name' and 'altName' tags only used for backward compatibility:", exhaustiveXml);
            ar.xmlAddNode_string("name", _objectName_old.c_str());
            ar.xmlAddNode_string("altName", _objectAltName_old.c_str());

            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("handle", _objectHandle);
                int parentID = -1;
                if (getParent() != nullptr)
                    parentID = getParent()->getObjectHandle();
                ar.xmlAddNode_int("parentHandle", parentID);
            }

            ar.xmlPushNewNode("localFrame");
            C7Vector tr = getLocalTransformation();
            if (getObjectType() == sim_sceneobject_shape)
            {
                ar.xmlAddNode_comment(" 'position' tag (in case of a shape): the value of this tag will be used to "
                                      "correctly build the shape, relative to its parent (or children), ",
                                      exhaustiveXml);
                ar.xmlAddNode_comment(" however, when load operation is finished, the local position of the shape will "
                                      "very probably be different (because the position of the shape ",
                                      exhaustiveXml);
                ar.xmlAddNode_comment(" is automatically selected to be at the geometric center of the shape) ",
                                      exhaustiveXml);
            }
            ar.xmlAddNode_3float("position", tr.X(0), tr.X(1), tr.X(2));
            if (exhaustiveXml)
                ar.xmlAddNode_4float("quaternion", tr.Q(0), tr.Q(1), tr.Q(2), tr.Q(3));
            else
            {
                if (getObjectType() == sim_sceneobject_shape)
                {
                    ar.xmlAddNode_comment(" 'euler' tag (in case of a shape): the value of this tag will be used to "
                                          "correctly build the shape, relative to its parent (or children), ",
                                          exhaustiveXml);
                    ar.xmlAddNode_comment(" however, when load operation is finished, the local orientation of the "
                                          "shape might be different (primitive shapes have a fixed orientation) ",
                                          exhaustiveXml);
                }
                C3Vector euler(tr.Q.getEulerAngles());
                euler *= 180.0 / piValue;
                ar.xmlAddNode_floats("euler", euler.data, 3);
            }
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("assembling");
                ar.xmlPushNewNode("localFrame");
                tr = _assemblingLocalTransformation;
                ar.xmlAddNode_3float("position", tr.X(0), tr.X(1), tr.X(2));
                ar.xmlAddNode_4float("quaternion", tr.Q(0), tr.Q(1), tr.Q(2), tr.Q(3));
                ar.xmlPopNode();
                ar.xmlPushNewNode("matchValues");
                ar.xmlAddNode_strings("child", _assemblyMatchValuesChild);
                ar.xmlAddNode_strings("parent", _assemblyMatchValuesParent);
                ar.xmlPopNode();
                ar.xmlAddNode_bool("localFrameIsUsed", _assemblingLocalTransformationIsUsed);
                ar.xmlPopNode();
            }

            ar.xmlAddNode_int("hierarchyColorIndex", _hierarchyColorIndex);

            ar.xmlAddNode_int("collectionSelfCollisionIndicator", _collectionSelfCollisionIndicator);

            ar.xmlPushNewNode("localObjectProperty");
            ar.xmlAddNode_bool("hierarchyCollapsed", _objectProperty & sim_objectproperty_collapsed);
            ar.xmlAddNode_bool("selectable", _objectProperty & sim_objectproperty_selectable);
            ar.xmlAddNode_bool("selectModelBaseInstead", _objectProperty & sim_objectproperty_selectmodelbaseinstead);
            ar.xmlAddNode_bool("dontShowAsInsideModel", _objectProperty & sim_objectproperty_dontshowasinsidemodel);
            ar.xmlAddNode_bool("selectInvisible", _objectProperty & sim_objectproperty_selectinvisible);
            ar.xmlAddNode_bool("depthInvisible", _objectProperty & sim_objectproperty_depthinvisible);
            ar.xmlAddNode_bool("cannotDelete", _objectProperty & sim_objectproperty_cannotdelete);
            ar.xmlAddNode_bool("cannotDeleteDuringSimulation",
                               _objectProperty & sim_objectproperty_cannotdeleteduringsim);
            ar.xmlAddNode_bool("ignoreViewFitting", _objectProperty & sim_objectproperty_ignoreviewfitting);
            ar.xmlAddNode_bool("hiddenForSimulation", _objectProperty & sim_objectproperty_hiddenforsimulation);
            ar.xmlPopNode();

            ar.xmlPushNewNode("localObjectSpecialProperty");
            ar.xmlAddNode_bool("collidable", _localObjectSpecialProperty & sim_objectspecialproperty_collidable);
            ar.xmlAddNode_bool("measurable", _localObjectSpecialProperty & sim_objectspecialproperty_measurable);
            if ((_localObjectSpecialProperty & sim_objectspecialproperty_detectable) ==
                sim_objectspecialproperty_detectable)
                ar.xmlAddNode_bool("detectable", true);
            if ((_localObjectSpecialProperty & sim_objectspecialproperty_detectable) == 0)
                ar.xmlAddNode_bool("detectable", false);
            // OLD:
            ar.xmlAddNode_comment(" 'renderable' tag for backward compatibility, set to 'true':", exhaustiveXml);
            ar.xmlAddNode_bool("renderable", _localObjectSpecialProperty &
                                                 sim_objectspecialproperty_renderable); // for backward compatibility
            ar.xmlAddNode_comment(" following 5 for backward compatibility:", exhaustiveXml);
            ar.xmlAddNode_bool("ultrasonicDetectable",
                               _localObjectSpecialProperty & sim_objectspecialproperty_detectable_ultrasonic);
            ar.xmlAddNode_bool("infraredDetectable",
                               _localObjectSpecialProperty & sim_objectspecialproperty_detectable_infrared);
            ar.xmlAddNode_bool("laserDetectable",
                               _localObjectSpecialProperty & sim_objectspecialproperty_detectable_laser);
            ar.xmlAddNode_bool("inductiveDetectable",
                               _localObjectSpecialProperty & sim_objectspecialproperty_detectable_inductive);
            ar.xmlAddNode_bool("capacitiveDetectable",
                               _localObjectSpecialProperty & sim_objectspecialproperty_detectable_capacitive);

            ar.xmlPopNode();

            ar.xmlPushNewNode("localModelProperty");
            ar.xmlAddNode_bool("notCollidable", _modelProperty & sim_modelproperty_not_collidable);
            ar.xmlAddNode_bool("notMeasurable", _modelProperty & sim_modelproperty_not_measurable);
            ar.xmlAddNode_bool("notDetectable", _modelProperty & sim_modelproperty_not_detectable);
            ar.xmlAddNode_bool("notDynamic", _modelProperty & sim_modelproperty_not_dynamic);
            ar.xmlAddNode_bool("notRespondable", _modelProperty & sim_modelproperty_not_respondable);
            ar.xmlAddNode_bool("notReset", _modelProperty & sim_modelproperty_not_reset);
            ar.xmlAddNode_bool("notVisible", _modelProperty & sim_modelproperty_not_visible);
            ar.xmlAddNode_bool("scriptsInactive", _modelProperty & sim_modelproperty_scripts_inactive);
            ar.xmlAddNode_bool("notShowAsInsideModel", _modelProperty & sim_modelproperty_not_showasinsidemodel);

            // For backward compatibility:
            ar.xmlAddNode_comment(" 'notRenderable' tag for backward compatibility, set to 'false':", exhaustiveXml);
            ar.xmlAddNode_bool("notRenderable", _modelProperty & sim_modelproperty_not_renderable);

            ar.xmlPopNode();

            ar.xmlAddNode_int("layer", _visibilityLayer);
            ar.xmlAddNode_int("childOrder", _childOrder);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("modelBase", _modelBase);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("manipulation");
                ar.xmlAddNode_int("permissions", _objectMovementPreferredAxes);
                ar.xmlPushNewNode("translation");
                ar.xmlAddNode_bool("disabledDuringSimulation", (_objectMovementOptions & 2) != 0);
                ar.xmlAddNode_bool("disabledDuringNonSimulation", (_objectMovementOptions & 1) != 0);
                ar.xmlAddNode_bool("settingsLocked", (_objectMovementOptions & 16) != 0);
                ar.xmlAddNode_int("relativeTo", _objectMovementRelativity[0]);
                ar.xmlAddNode_float("nonDefaultStepSize", _objectMovementStepSize[0]);
                ar.xmlPopNode();
                ar.xmlPushNewNode("rotation");
                ar.xmlAddNode_bool("disabledDuringSimulation", (_objectMovementOptions & 8) != 0);
                ar.xmlAddNode_bool("disabledDuringNonSimulation", (_objectMovementOptions & 4) != 0);
                ar.xmlAddNode_bool("settingsLocked", (_objectMovementOptions & 32) != 0);
                ar.xmlAddNode_int("relativeTo", _objectMovementRelativity[1]);
                ar.xmlAddNode_float("nonDefaultStepSize", _objectMovementStepSize[1]);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                ar.xmlAddNode_float("sizeFactor", _sizeFactor);

                ar.xmlAddNode_int("scriptExecPriority", _scriptExecPriority);

                ar.xmlAddNode_floats("sizeValues", _sizeValues, 3);

                std::string str(base64_encode((unsigned char*)_dnaString.c_str(), _dnaString.size()));
                ar.xmlAddNode_string("dnaString_base64Coded", str.c_str());

                str =
                    base64_encode((unsigned char*)_uniquePersistentIdString.c_str(), _uniquePersistentIdString.size());
                ar.xmlAddNode_string("uniquePersistentString_base64Coded", str.c_str());

                ar.xmlAddNode_float("transparentObjectDistanceOffset", _transparentObjectDistanceOffset);

                ar.xmlAddNode_int("authorizedViewableObjects", _authorizedViewableObjects);
            }

            ar.xmlAddNode_string("extensionString", _extensionString.c_str());

            ar.xmlAddNode_string("modelAcknowledgement", _modelAcknowledgement.c_str());

            if (exhaustiveXml)
            {
                if (customObjectData.getDataCount() != 0)
                {
                    ar.xmlPushNewNode("customObjectData");
                    customObjectData.serializeData(ar, getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }
                if (_customObjectData_old != nullptr)
                { // keep a while for backward compatibility (e.g. until V4.4.0)
                    ar.xmlPushNewNode("customData");
                    _customObjectData_old->serializeData(ar, getObjectAliasAndHandle().c_str(), -1);
                    ar.xmlPopNode();
                }
                if (_userScriptParameters != nullptr)
                {
                    ar.xmlPushNewNode("userParameters");
                    _userScriptParameters->serialize(ar);
                    ar.xmlPopNode();
                }

                // _customReferencedHandles with tag "" handled separately, for backw. comp.
                auto it = _customReferencedHandles.find("");
                if (it != _customReferencedHandles.end())
                {
                    if (it->second.size() > 0)
                    {
                        ar.xmlPushNewNode("customReferencedHandles");
                        ar.xmlAddNode_int("count", int(it->second.size()));
                        std::vector<int> tmp;
                        for (size_t i = 0; i < it->second.size(); i++)
                            tmp.push_back(it->second[i].generalObjectType);
                        ar.xmlAddNode_ints("generalObjectTypes", &tmp[0], tmp.size());
                        tmp.clear();
                        for (size_t i = 0; i < it->second.size(); i++)
                            tmp.push_back(it->second[i].generalObjectHandle);
                        ar.xmlAddNode_ints("generalObjectHandles", &tmp[0], tmp.size());
                        ar.xmlPopNode();
                    }
                }

                for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
                {
                    if (it->first != "")
                    {
                        if (it->second.size() > 0)
                        {
                            ar.xmlPushNewNode("customTaggedReferencedHandles");
                            ar.xmlAddNode_string("tag", it->first.c_str());
                            ar.xmlAddNode_int("count", int(it->second.size()));
                            std::vector<int> tmp;
                            for (size_t i = 0; i < it->second.size(); i++)
                                tmp.push_back(it->second[i].generalObjectHandle);
                            ar.xmlAddNode_ints("objectHandles", &tmp[0], tmp.size());
                            ar.xmlPopNode();
                        }
                    }
                }

                // _customReferencedOriginalHandles with tag "" handled separately, for backw. comp.
                auto itt = _customReferencedOriginalHandles.find("");
                if (itt != _customReferencedOriginalHandles.end())
                {
                    if (itt->second.size() > 0)
                    {
                        ar.xmlPushNewNode("customReferencedOriginalHandles");
                        ar.xmlAddNode_int("count", int(itt->second.size()));
                        std::vector<int> tmp;
                        for (size_t i = 0; i < itt->second.size(); i++)
                            tmp.push_back(itt->second[i].generalObjectType);
                        ar.xmlAddNode_ints("generalObjectTypes", &tmp[0], tmp.size());
                        tmp.clear();
                        for (size_t i = 0; i < itt->second.size(); i++)
                            tmp.push_back(itt->second[i].generalObjectHandle);
                        ar.xmlAddNode_ints("generalObjectHandles", &tmp[0], tmp.size());
                        std::vector<std::string> sTmp;
                        for (size_t i = 0; i < itt->second.size(); i++)
                            sTmp.push_back(base64_encode(
                                (unsigned char*)itt->second[i].uniquePersistentIdString.c_str(),
                                itt->second[i].uniquePersistentIdString.size()));
                        ar.xmlAddNode_strings("uniquePersistentIdString_base64Coded", sTmp);

                        ar.xmlPopNode();
                    }
                }

                for (auto itt = _customReferencedOriginalHandles.begin(); itt != _customReferencedOriginalHandles.end(); ++itt)
                {
                    if (itt->first != "")
                    {
                        if (itt->second.size() > 0)
                        {
                            ar.xmlPushNewNode("customTaggedReferencedOriginalHandles");
                            ar.xmlAddNode_string("tag", itt->first.c_str());
                            ar.xmlAddNode_int("count", int(itt->second.size()));
                            std::vector<int> tmp;
                            for (size_t i = 0; i < itt->second.size(); i++)
                                tmp.push_back(itt->second[i].generalObjectHandle);
                            ar.xmlAddNode_ints("objectHandles", &tmp[0], tmp.size());
                            std::vector<std::string> sTmp;
                            for (size_t i = 0; i < itt->second.size(); i++)
                                sTmp.push_back(base64_encode(
                                    (unsigned char*)itt->second[i].uniquePersistentIdString.c_str(),
                                    itt->second[i].uniquePersistentIdString.size()));
                            ar.xmlAddNode_strings("uniquePersistentIdString_base64Coded", sTmp);

                            ar.xmlPopNode();
                        }
                    }
                }
            }

            ar.xmlPopNode();
        }
        else
        {
            bool aliasFound = false;
            bool _ignoredByViewFitting_backCompat = false;
            if (ar.xmlPushChildNode("common", exhaustiveXml))
            {
                aliasFound = ar.xmlGetNode_string(
                    propObject_alias.name, _objectAlias, false); // keep false for compatibility with older versions! exhaustiveXml);
                if (aliasFound)
                {
                    _objectTempAlias = _objectAlias;
                    _objectAlias = _objectAlias.substr(0, _objectAlias.find('*'));
                    _objectAlias = tt::getValidAlias(_objectAlias.c_str());
                }
                if (ar.xmlGetNode_string("name", _objectName_old, exhaustiveXml) && (!exhaustiveXml))
                {
                    tt::removeIllegalCharacters(_objectName_old, true);
                    _objectTempName_old = _objectName_old;
                    _objectName_old = "XYZ___" + _objectName_old + "___XYZ";
                }
                if (ar.xmlGetNode_string("altName", _objectAltName_old, exhaustiveXml) && (!exhaustiveXml))
                {
                    tt::removeAltNameIllegalCharacters(_objectAltName_old);
                    _objectTempAltName_old = _objectAltName_old;
                    _objectAltName_old = "XYZ___" + _objectAltName_old + "___XYZ";
                }

                if (exhaustiveXml)
                {
                    ar.xmlGetNode_int("handle", _objectHandle);
                    ar.xmlGetNode_int("parentHandle", _parentObjectHandle_forSerializationOnly);
                }

                if (ar.xmlPushChildNode("localFrame", exhaustiveXml))
                {
                    C7Vector tr;
                    tr.setIdentity();
                    ar.xmlGetNode_floats("position", tr.X.data, 3, exhaustiveXml);
                    if (exhaustiveXml)
                    {
                        ar.xmlGetNode_floats("quaternion", tr.Q.data, 4);
                        tr.Q.normalize(); // just in case
                    }
                    else
                    {
                        C3Vector euler;
                        if (ar.xmlGetNode_floats("euler", euler.data, 3, exhaustiveXml))
                        {
                            euler(0) *= piValue / 180.0;
                            euler(1) *= piValue / 180.0;
                            euler(2) *= piValue / 180.0;
                            tr.Q.setEulerAngles(euler);
                        }
                    }
                    setLocalTransformation(tr);
                    ar.xmlPopNode();
                }

                if (exhaustiveXml && ar.xmlPushChildNode("assembling"))
                {
                    if (ar.xmlPushChildNode("localFrame"))
                    {
                        ar.xmlGetNode_floats("position", _assemblingLocalTransformation.X.data, 3);
                        ar.xmlGetNode_floats("quaternion", _assemblingLocalTransformation.Q.data, 4);
                        _assemblingLocalTransformation.Q.normalize(); // just in case
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("matchValues"))
                    {
                        _assemblyMatchValuesChild.clear();
                        ar.xmlGetNode_strings("child", _assemblyMatchValuesChild);
                        _assemblyMatchValuesParent.clear();
                        ar.xmlGetNode_strings("parent", _assemblyMatchValuesParent);
                        ar.xmlPopNode();
                    }
                    ar.xmlGetNode_bool("localFrameIsUsed", _assemblingLocalTransformationIsUsed);
                    ar.xmlPopNode();
                }

                ar.xmlGetNode_int("hierarchyColorIndex", _hierarchyColorIndex, exhaustiveXml);

                ar.xmlGetNode_int("collectionSelfCollisionIndicator", _collectionSelfCollisionIndicator, exhaustiveXml);

                if (ar.xmlPushChildNode("localObjectProperty", exhaustiveXml))
                {
                    _objectProperty = 0;
                    ar.xmlGetNode_flags("hierarchyCollapsed", _objectProperty, sim_objectproperty_collapsed,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("selectable", _objectProperty, sim_objectproperty_selectable, exhaustiveXml);
                    ar.xmlGetNode_flags("selectModelBaseInstead", _objectProperty,
                                        sim_objectproperty_selectmodelbaseinstead, exhaustiveXml);
                    ar.xmlGetNode_flags("dontShowAsInsideModel", _objectProperty,
                                        sim_objectproperty_dontshowasinsidemodel, exhaustiveXml);
                    ar.xmlGetNode_flags("selectInvisible", _objectProperty, sim_objectproperty_selectinvisible,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("depthInvisible", _objectProperty, sim_objectproperty_depthinvisible,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDelete", _objectProperty, sim_objectproperty_cannotdelete,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("cannotDeleteDuringSimulation", _objectProperty,
                                        sim_objectproperty_cannotdeleteduringsim, exhaustiveXml);
                    ar.xmlGetNode_flags("ignoreViewFitting", _objectProperty, sim_objectproperty_ignoreviewfitting,
                                        false);
                    ar.xmlGetNode_flags("hiddenForSimulation", _objectProperty, sim_objectproperty_hiddenforsimulation,
                                        false);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("localObjectSpecialProperty", exhaustiveXml))
                {
                    _localObjectSpecialProperty = 0;
                    ar.xmlGetNode_flags("collidable", _localObjectSpecialProperty, sim_objectspecialproperty_collidable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("measurable", _localObjectSpecialProperty, sim_objectspecialproperty_measurable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("renderable", _localObjectSpecialProperty, sim_objectspecialproperty_renderable,
                                        false); // for backward compatibility
                    // Following 5 for backward compatibility:
                    ar.xmlGetNode_flags("ultrasonicDetectable", _localObjectSpecialProperty,
                                        sim_objectspecialproperty_detectable_ultrasonic, false);
                    ar.xmlGetNode_flags("infraredDetectable", _localObjectSpecialProperty,
                                        sim_objectspecialproperty_detectable_infrared, false);
                    ar.xmlGetNode_flags("laserDetectable", _localObjectSpecialProperty,
                                        sim_objectspecialproperty_detectable_laser, false);
                    ar.xmlGetNode_flags("inductiveDetectable", _localObjectSpecialProperty,
                                        sim_objectspecialproperty_detectable_inductive, false);
                    ar.xmlGetNode_flags("capacitiveDetectable", _localObjectSpecialProperty,
                                        sim_objectspecialproperty_detectable_capacitive, false);
                    bool dummyDet;
                    if (ar.xmlGetNode_bool("detectable", dummyDet, false))
                    {
                        _localObjectSpecialProperty |= sim_objectspecialproperty_detectable;
                        if (!dummyDet)
                            _localObjectSpecialProperty -= sim_objectspecialproperty_detectable;
                    }

                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("switches", exhaustiveXml))
                {
                    ar.xmlGetNode_bool("modelBase", _modelBase, exhaustiveXml);
                    ar.xmlGetNode_bool("ignoredByViewFitting", _ignoredByViewFitting_backCompat, false);
                    ar.xmlPopNode();

                    if (_modelBase)
                        _modelProperty = 0;
                    else
                        _modelProperty = sim_modelproperty_not_model;
                }

                if (ar.xmlPushChildNode("localModelProperty", exhaustiveXml))
                {
                    ar.xmlGetNode_flags("notCollidable", _modelProperty, sim_modelproperty_not_collidable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("notMeasurable", _modelProperty, sim_modelproperty_not_measurable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("notRenderable", _modelProperty, sim_modelproperty_not_renderable,
                                        false); // for backward compatibility
                    ar.xmlGetNode_flags("notDetectable", _modelProperty, sim_modelproperty_not_detectable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("notDynamic", _modelProperty, sim_modelproperty_not_dynamic, exhaustiveXml);
                    ar.xmlGetNode_flags("notRespondable", _modelProperty, sim_modelproperty_not_respondable,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("notReset", _modelProperty, sim_modelproperty_not_reset, exhaustiveXml);
                    ar.xmlGetNode_flags("notVisible", _modelProperty, sim_modelproperty_not_visible, exhaustiveXml);
                    ar.xmlGetNode_flags("scriptsInactive", _modelProperty, sim_modelproperty_scripts_inactive,
                                        exhaustiveXml);
                    ar.xmlGetNode_flags("notShowAsInsideModel", _modelProperty, sim_modelproperty_not_showasinsidemodel,
                                        exhaustiveXml);
                    ar.xmlPopNode();
                }

                int l;
                if (ar.xmlGetNode_int("layer", l, exhaustiveXml))
                    _visibilityLayer = l;

                if (ar.xmlGetNode_int("childOrder", l, false)) // Keep false for compatibility with older versions! exhaustiveXml))
                    _childOrder = l;

                if (exhaustiveXml && ar.xmlPushChildNode("manipulation"))
                {
                    ar.xmlGetNode_int("permissions", _objectMovementPreferredAxes);
                    if (ar.xmlPushChildNode("translation"))
                    {
                        bool tmp;
                        if (ar.xmlGetNode_bool("disabledDuringSimulation", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 2;
                        }
                        if (ar.xmlGetNode_bool("disabledDuringNonSimulation", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 1;
                        }
                        if (ar.xmlGetNode_bool("settingsLocked", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 16;
                        }
                        ar.xmlGetNode_int("relativeTo", _objectMovementRelativity[0]);
                        ar.xmlGetNode_float("nonDefaultStepSize", _objectMovementStepSize[0]);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("rotation"))
                    {
                        bool tmp;
                        if (ar.xmlGetNode_bool("disabledDuringSimulation", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 8;
                        }
                        if (ar.xmlGetNode_bool("disabledDuringNonSimulation", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 4;
                        }
                        if (ar.xmlGetNode_bool("settingsLocked", tmp, false))
                        {
                            if (tmp)
                                _objectMovementOptions = _objectMovementOptions | 32;
                        }
                        ar.xmlGetNode_int("relativeTo", _objectMovementRelativity[1]);
                        ar.xmlGetNode_float("nonDefaultStepSize", _objectMovementStepSize[1]);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }

                if (exhaustiveXml)
                {
                    ar.xmlGetNode_float("sizeFactor", _sizeFactor, exhaustiveXml);
                    ar.xmlGetNode_int("scriptExecPriority", _scriptExecPriority, exhaustiveXml);
                    ar.xmlGetNode_floats("sizeValues", _sizeValues, 3, exhaustiveXml);
                    ar.xmlGetNode_float("transparentObjectDistanceOffset", _transparentObjectDistanceOffset,
                                        exhaustiveXml);
                    ar.xmlGetNode_int("authorizedViewableObjects", _authorizedViewableObjects, exhaustiveXml);
                }

                if (exhaustiveXml && ar.xmlGetNode_string("dnaString_base64Coded", _dnaString))
                    _dnaString = base64_decode(_dnaString);

                if (exhaustiveXml &&
                    ar.xmlGetNode_string("uniquePersistentString_base64Coded", _uniquePersistentIdString))
                    _uniquePersistentIdString = base64_decode(_uniquePersistentIdString);

                ar.xmlGetNode_string("extensionString", _extensionString, exhaustiveXml);

                ar.xmlGetNode_string("modelAcknowledgement", _modelAcknowledgement, exhaustiveXml);

                if (exhaustiveXml && ar.xmlPushChildNode("customObjectData", false))
                {
                    customObjectData.serializeData(ar, getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }
                if (exhaustiveXml && ar.xmlPushChildNode("customData", false))
                {
                    _customObjectData_old = new CCustomData_old();
                    _customObjectData_old->serializeData(ar, getObjectAliasAndHandle().c_str(), -1);
                    if (customObjectData.getDataCount() == 0)
                        _customObjectData_old->initNewFormat(customObjectData, true);
                    ar.xmlPopNode();
                }
                if (exhaustiveXml && ar.xmlPushChildNode("userParameters", false))
                {
                    if (_userScriptParameters != nullptr)
                        delete _userScriptParameters;
                    _userScriptParameters = new CUserParameters();
                    _userScriptParameters->serialize(ar);
                    ar.xmlPopNode();
                }

                if (exhaustiveXml && ar.xmlPushChildNode("customReferencedHandles", false))
                {
                    int cnt;
                    ar.xmlGetNode_int("count", cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    _customReferencedHandles.insert({"", {}});
                    auto it = _customReferencedHandles.find("");
                    if (cnt > 0)
                    {
                        ot.resize(cnt, -1);
                        oh.resize(cnt, -1);
                        ar.xmlGetNode_ints("generalObjectTypes", &ot[0], cnt);
                        ar.xmlGetNode_ints("generalObjectHandles", &oh[0], cnt);
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomRefs r;
                            r.generalObjectType = ot[i];
                            r.generalObjectHandle = oh[i];
                            it->second.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }

                if (exhaustiveXml && ar.xmlPushChildNode("customTaggedReferencedHandles", false))
                {
                    std::string key;
                    ar.xmlGetNode_string("tag", key);
                    int cnt;
                    ar.xmlGetNode_int("count", cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    _customReferencedHandles.insert({key.c_str(), {}});
                    auto it = _customReferencedHandles.find(key.c_str());
                    if (cnt > 0)
                    {
                        ot.resize(cnt, sim_objecttype_sceneobject);
                        oh.resize(cnt, -1);
                        ar.xmlGetNode_ints("objectHandles", &oh[0], cnt);
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomRefs r;
                            r.generalObjectType = ot[i];
                            r.generalObjectHandle = oh[i];
                            it->second.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }

                if (exhaustiveXml && ar.xmlPushChildNode("customReferencedOriginalHandles", false))
                {
                    int cnt;
                    ar.xmlGetNode_int("count", cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    std::vector<std::string> oi;
                    _customReferencedOriginalHandles.insert({"", {}});
                    auto it = _customReferencedOriginalHandles.find("");
                    if (cnt > 0)
                    {
                        ot.resize(cnt, -1);
                        oh.resize(cnt, -1);
                        ar.xmlGetNode_ints("generalObjectTypes", &ot[0], cnt);
                        ar.xmlGetNode_ints("generalObjectHandles", &oh[0], cnt);
                        ar.xmlGetNode_strings("uniquePersistentIdString_base64Coded", oi);
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomOriginalRefs r;
                            r.generalObjectType = ot[i];
                            r.generalObjectHandle = oh[i];
                            if (r.generalObjectHandle >= 0)
                                r.uniquePersistentIdString = base64_decode(oi[i]);
                            it->second.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }
                if (exhaustiveXml && ar.xmlPushChildNode("customTaggedReferencedOriginalHandles", false))
                {
                    std::string key;
                    ar.xmlGetNode_string("tag", key);
                    int cnt;
                    ar.xmlGetNode_int("count", cnt);
                    std::vector<int> ot;
                    std::vector<int> oh;
                    std::vector<std::string> oi;
                    _customReferencedOriginalHandles.insert({key.c_str(), {}});
                    auto it = _customReferencedOriginalHandles.find(key.c_str());
                    if (cnt > 0)
                    {
                        ot.resize(cnt, sim_objecttype_sceneobject);
                        oh.resize(cnt, -1);
                        ar.xmlGetNode_ints("objectHandles", &oh[0], cnt);
                        ar.xmlGetNode_strings("uniquePersistentIdString_base64Coded", oi);
                        for (int i = 0; i < cnt; i++)
                        {
                            SCustomOriginalRefs r;
                            r.generalObjectType = ot[i];
                            r.generalObjectHandle = oh[i];
                            if (r.generalObjectHandle >= 0)
                                r.uniquePersistentIdString = base64_decode(oi[i]);
                            it->second.push_back(r);
                        }
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }
            if (_ignoredByViewFitting_backCompat)
                _objectProperty |= sim_objectproperty_ignoreviewfitting; // for backward compatibility
            if (!aliasFound)
            {
                if (exhaustiveXml)
                    _objectAlias = _objectName_old.substr(0, _objectName_old.find('#'));
                else
                    _objectAlias = _objectTempName_old.substr(0, _objectTempName_old.find('#'));
            }
        }
    }
}

void CSceneObject::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    if (opType == 3)
        _objectHandle = CWorld::getLoadingMapping(map, _objectHandle);
    _parentObjectHandle_forSerializationOnly = CWorld::getLoadingMapping(map, _parentObjectHandle_forSerializationOnly);

    if (opType <= 1) // scene or model loading only
        App::currentWorld->sceneObjects->setObjectParent(this, App::currentWorld->sceneObjects->getObjectFromHandle(_parentObjectHandle_forSerializationOnly), false);

    if ((_authorizedViewableObjects >= 0) && (_authorizedViewableObjects <= SIM_IDEND_SCENEOBJECT))
        _authorizedViewableObjects = CWorld::getLoadingMapping(map, _authorizedViewableObjects);

    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_objecttype_sceneobject)
                it->second[i].generalObjectHandle = CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
        }
    }
    if (opType == 0)
    { // scene loading only
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_objecttype_sceneobject)
                    it->second[i].generalObjectHandle =
                        CWorld::getLoadingMapping(map, it->second[i].generalObjectHandle);
            }
        }
    }
}

void CSceneObject::performScriptLoadingMapping(const std::map<int, int>* map, int opType)
{
}

std::string CSceneObject::getDisplayName() const
{
    return (getObjectAlias_printPath());
}

void CSceneObject::announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!!
#ifdef SIM_WITH_GUI
    // if we are in edit mode, we leave edit mode:
    if ((GuiApp::getEditModeType() != NO_EDIT_MODE) && (!copyBuffer))
    {
        if (GuiApp::mainWindow->editModeContainer->getEditModeObjectID() == object->getObjectHandle())
            GuiApp::mainWindow->editModeContainer->processCommand(
                ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,
                nullptr); // This is if we destroy the object being edited (shouldn't normally happen!)
    }
#endif

    if (_authorizedViewableObjects == object->getObjectHandle())
        _authorizedViewableObjects = -2; // not visible anymore!

    // If the object's parent will be erased, make the object child of its grand-parents
    if (!copyBuffer)
    {
        CSceneObject* parent = getParent();
        if (parent != nullptr)
        {
            if (parent == object)
                App::currentWorld->sceneObjects->setObjectParent(this, parent->getParent(), true);
        }
        removeChild(object);
    }
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_objecttype_sceneobject)
            {
                if (it->second[i].generalObjectHandle == object->getObjectHandle())
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
    if (!copyBuffer)
    {
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_objecttype_sceneobject)
                {
                    if (it->second[i].generalObjectHandle == object->getObjectHandle())
                        it->second[i].generalObjectHandle = -1;
                }
            }
        }
    }
}

void CSceneObject::announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript,
                                              bool copyBuffer)
{
}

void CSceneObject::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{
    // This routine can be called for sceneObjects-objects, but also for objects
    // in the copy-buffer!! So never make use of any
    // 'ct::sceneObjects->getObject(id)'-call or similar
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectType == sim_appobj_ik_type)
            {
                if (it->second[i].generalObjectHandle == ikGroupID)
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
    if (!copyBuffer)
    {
        for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
        {
            for (size_t i = 0; i < it->second.size(); i++)
            {
                if (it->second[i].generalObjectType == sim_appobj_ik_type)
                {
                    if (it->second[i].generalObjectHandle == ikGroupID)
                        it->second[i].generalObjectHandle = -1;
                }
            }
        }
    }
}

void CSceneObject::setReferencedHandles(size_t cnt, const int* handles, const char* tag)
{
    if (tag == nullptr)
        _customReferencedHandles.clear();
    else
    {
        auto it = _customReferencedHandles.find(tag);
        if (it != _customReferencedHandles.end())
            it->second.clear();
        else
        {
            if (cnt != 0)
            {
                _customReferencedHandles.insert({tag, {}});
                it = _customReferencedHandles.find(tag);
            }
        }
        if (cnt != 0)
        {
            for (int i = 0; i < cnt; i++)
            {
                SCustomRefs r;
                r.generalObjectType = sim_objecttype_sceneobject;
                r.generalObjectHandle = -1;
                if (handles[i] >= 0)
                {
                    if (App::currentWorld->sceneObjects->getObjectFromHandle(handles[i]) != nullptr)
                        r.generalObjectHandle = handles[i];
                    else
                    { // backw. compatibility:
                        if (App::currentWorld->collisions_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_collision_type;
                            r.generalObjectHandle = handles[i];
                        }
                        if (App::currentWorld->distances_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_distance_type;
                            r.generalObjectHandle = handles[i];
                        }
                        if (App::currentWorld->ikGroups_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_ik_type;
                            r.generalObjectHandle = handles[i];
                        }
                        if (App::currentWorld->collections->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_objecttype_collection;
                            r.generalObjectHandle = handles[i];
                        }
                    }
                }
                it->second.push_back(r);
            }
        }
    }
}

size_t CSceneObject::getReferencedHandlesCount(const char* tag) const
{
    size_t retVal = 0;
    auto it = _customReferencedHandles.find(tag);
    if (it != _customReferencedHandles.end())
        retVal = it->second.size();
    return retVal;
}

size_t CSceneObject::getReferencedHandles(int* handles, const char* tag) const
{
    size_t retVal = 0;
    auto it = _customReferencedHandles.find(tag);
    if (it != _customReferencedHandles.end())
    {
        retVal = it->second.size();
        for (size_t i = 0; i < retVal; i++)
            handles[i] = it->second[i].generalObjectHandle;
    }
    return retVal;
}

void CSceneObject::getReferencedHandlesTags(std::vector<std::string>& tags) const
{
    for (auto it = _customReferencedHandles.begin(); it != _customReferencedHandles.end(); ++it)
    {
        if (it->second.size() > 0)
            tags.push_back(it->first);
    }
}

void CSceneObject::setReferencedOriginalHandles(int cnt, const int* handles, const char* tag)
{
    if (tag == nullptr)
        _customReferencedOriginalHandles.clear();
    else
    {
        auto it = _customReferencedOriginalHandles.find(tag);
        if (it != _customReferencedOriginalHandles.end())
            it->second.clear();
        else
        {
            if (cnt != 0)
            {
                _customReferencedOriginalHandles.insert({tag, {}});
                it = _customReferencedOriginalHandles.find(tag);
            }
        }
        if (cnt != 0)
        {
            for (int i = 0; i < cnt; i++)
            {
                SCustomOriginalRefs r;
                r.generalObjectType = sim_objecttype_sceneobject;
                r.generalObjectHandle = -1;
                if (handles[i] >= 0)
                {
                    if (App::currentWorld->sceneObjects->getObjectFromHandle(handles[i]) != nullptr)
                    {
                        r.generalObjectHandle = handles[i];
                        r.uniquePersistentIdString =
                            App::currentWorld->sceneObjects->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                    }
                    else
                    { // backw. compatibility
                        if (App::currentWorld->collisions_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_collision_type;
                            r.generalObjectHandle = handles[i];
                            r.uniquePersistentIdString =
                                App::currentWorld->collisions_old->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                        }
                        if (App::currentWorld->distances_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_distance_type;
                            r.generalObjectHandle = handles[i];
                            r.uniquePersistentIdString =
                                App::currentWorld->distances_old->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                        }
                        if (App::currentWorld->ikGroups_old->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_appobj_ik_type;
                            r.generalObjectHandle = handles[i];
                            r.uniquePersistentIdString =
                                App::currentWorld->ikGroups_old->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                        }
                        if (App::currentWorld->collections->getObjectFromHandle(handles[i]) != nullptr)
                        {
                            r.generalObjectType = sim_objecttype_collection;
                            r.generalObjectHandle = handles[i];
                            r.uniquePersistentIdString =
                                App::currentWorld->collections->getObjectFromHandle(handles[i])->getUniquePersistentIdString();
                        }
                    }
                }
                it->second.push_back(r);
            }
        }
    }
}

size_t CSceneObject::getReferencedOriginalHandlesCount(const char* tag) const
{
    size_t retVal = 0;
    auto it = _customReferencedOriginalHandles.find(tag);
    if (it != _customReferencedOriginalHandles.end())
        retVal = it->second.size();
    return retVal;
}

size_t CSceneObject::getReferencedOriginalHandles(int* handles, const char* tag) const
{
    size_t retVal = 0;
    auto it = _customReferencedOriginalHandles.find(tag);
    if (it != _customReferencedOriginalHandles.end())
    {
        retVal = it->second.size();
        for (size_t i = 0; i < retVal; i++)
            handles[i] = it->second[i].generalObjectHandle;
    }
    return retVal;
}

void CSceneObject::getReferencedOriginalHandlesTags(std::vector<std::string>& tags) const
{
    for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
    {
        if (it->second.size() > 0)
            tags.push_back(it->first);
    }
}

void CSceneObject::checkReferencesToOriginal(const std::map<std::string, int>& allUniquePersistentIdStrings)
{
    for (auto it = _customReferencedOriginalHandles.begin(); it != _customReferencedOriginalHandles.end(); ++it)
    {
        for (size_t i = 0; i < it->second.size(); i++)
        {
            if (it->second[i].generalObjectHandle >= 0)
            {
                auto itt = allUniquePersistentIdStrings.find(it->second[i].uniquePersistentIdString);
                if (itt != allUniquePersistentIdStrings.end())
                    it->second[i].generalObjectHandle = itt->second;
                else
                    it->second[i].generalObjectHandle = -1;
            }
        }
    }
}

void CSceneObject::setAbsoluteTransformation(const C7Vector& v)
{
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse() * v);
}

void CSceneObject::setAbsoluteTransformation(const C4Vector& q)
{
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse().Q * q);
}

void CSceneObject::setAbsoluteTransformation(const C3Vector& x)
{
    C7Vector tr(getLocalTransformation());
    tr.X = getFullParentCumulativeTransformation().getInverse() * x;
    setLocalTransformation(tr);
}

int CSceneObject::getIkPluginCounterpartHandle() const
{
    return (_ikPluginCounterpartHandle);
}

bool CSceneObject::hasAncestor(const CSceneObject* potentialAncestor) const
{
    bool retVal = false;
    if (getParent() != nullptr)
    {
        if (getParent() == potentialAncestor)
            retVal = true;
        else
            retVal = getParent()->hasAncestor(potentialAncestor);
    }
    return (retVal);
}

void CSceneObject::setParentHandle_forSerializationOnly(int pHandle)
{
    _parentObjectHandle_forSerializationOnly = pHandle;
}

void CSceneObject::getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives,
                                          bool visibleModelsOnly) const
{
    for (size_t i = 0; i < getChildCount(); i++)
    {
        CSceneObject* child = getChildFromIndex(i);
        if (child->getModelBase())
        {
            if (!child->isObjectPartOfInvisibleModel())
                firstModelRelatives.push_back(child);
        }
        else
            child->getFirstModelRelatives(firstModelRelatives, visibleModelsOnly);
    }
}

int CSceneObject::countFirstModelRelatives(bool visibleModelsOnly) const
{
    int cnt = 0;
    for (size_t i = 0; i < getChildCount(); i++)
    {
        CSceneObject* child = getChildFromIndex(i);
        if (child->getModelBase())
        {
            if (!child->isObjectPartOfInvisibleModel())
                cnt++;
        }
        else
            cnt += child->countFirstModelRelatives(visibleModelsOnly);
    }
    return (cnt);
}

std::string CSceneObject::getObjectTempAlias() const
{
    return (_objectTempAlias);
}

std::string CSceneObject::getObjectTempName_old() const
{
    return (_objectTempName_old);
}

std::string CSceneObject::getObjectTempAltName_old() const
{
    return (_objectTempAltName_old);
}

CUserParameters* CSceneObject::getUserScriptParameterObject()
{
    return (_userScriptParameters);
}

void CSceneObject::setUserScriptParameterObject(CUserParameters* obj)
{
    if (_userScriptParameters != nullptr)
        delete _userScriptParameters;
    _userScriptParameters = obj;
}

void CSceneObject::acquireCommonPropertiesFromObject_simpleXMLLoading(const CSceneObject* obj)
{ // names can't be changed here, probably same with aliases!
    //    _objectName=obj->_objectName;
    //    _objectAlias=obj->_objectAlias;
    //    _objectAltName=obj->_objectAltName;
    _objectTempAlias = obj->_objectTempAlias;
    _objectTempName_old = obj->_objectTempName_old;
    _localTransformation = obj->_localTransformation;
    _hierarchyColorIndex = obj->_hierarchyColorIndex;
    _collectionSelfCollisionIndicator = obj->_collectionSelfCollisionIndicator;
    _objectProperty = obj->_objectProperty;
    _localObjectSpecialProperty = obj->_localObjectSpecialProperty;
    _modelProperty = obj->_modelProperty;
    _modelBase = obj->_modelBase;
    setVisibilityLayer(obj->getVisibilityLayer());
    _extensionString = obj->_extensionString;
    _modelAcknowledgement = obj->_modelAcknowledgement;
}

void CSceneObject::setRestoreToDefaultLights(bool s)
{
    _restoreToDefaultLights = s;
}

bool CSceneObject::getRestoreToDefaultLights() const
{
    return (_restoreToDefaultLights);
}

#ifdef SIM_WITH_GUI
void CSceneObject::display(CViewableBase* renderingObject, int displayAttrib)
{
}

void CSceneObject::displayFrames(CViewableBase* renderingObject, double size, bool persp)
{
    if (persp)
    {
        C7Vector x(renderingObject->getCumulativeTransformation().getInverse() * getCumulativeTransformation());
        size *= x(2);
    }
    C7Vector tr(getCumulativeTransformation());
    _displayFrame(tr, size * 0.0125);
    bool available = false;
    C7Vector localFrame(getIntrinsicTransformation(true, &available));
    if (available)
        _displayFrame(tr * localFrame, size * 0.0125);
}

void CSceneObject::displayBoundingBox(CViewableBase* renderingObject, bool mainSelection)
{
    _displayBoundingBox(this, renderingObject, mainSelection);
}

void CSceneObject::displayManipulationModeOverlayGrid(CViewableBase* renderingObject, double size, bool persp)
{
    if (_objectManipulationMode_flaggedForGridOverlay == 0)
        return;
    if (persp)
    {
        C3Vector x(renderingObject->getCumulativeTransformation().getInverse() * _objectManipulationModeMouseDownPos);
        size *= x(2);
    }
    size *= 0.05;
    ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack);
    ogl::setAlpha(0.5);
    //    GuiApp::setLightDialogRefreshFlag(); // to actualize the position and orientation dialogs!
    bool isPathPoints = false;
    C3Vector localPositionOnPath;
    localPositionOnPath.clear();
    if (_objectType == sim_sceneobject_path)
    {
        std::vector<int> pathPointsToTakeIntoAccount;
        CPathCont_old* pc;
        if (((GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD) ||
             (GuiApp::mainWindow->editModeContainer->pathPointManipulation
                  ->getSelectedPathPointIndicesSize_nonEditMode() != 0)) &&
            ((_objectManipulationMode_flaggedForGridOverlay & 8) == 0))

        { // (path is in edition or path points are selected) and no rotation
            isPathPoints = true;
            if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
            { // Path is in edition
                pc = GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old();
                pathPointsToTakeIntoAccount.assign(GuiApp::mainWindow->editModeContainer->getEditModeBuffer()->begin(),
                                                   GuiApp::mainWindow->editModeContainer->getEditModeBuffer()->end());
            }
            else
            { // Path points are selected (but not in path edit mode)
                pc = ((CPath_old*)this)->pathContainer;
                pathPointsToTakeIntoAccount.assign(GuiApp::mainWindow->editModeContainer->pathPointManipulation
                                                       ->getPointerToSelectedPathPointIndices_nonEditMode()
                                                       ->begin(),
                                                   GuiApp::mainWindow->editModeContainer->pathPointManipulation
                                                       ->getPointerToSelectedPathPointIndices_nonEditMode()
                                                       ->end());
            }
        }
        else
        { // Path is not in edition and no path points are selected
            pc = ((CPath_old*)this)->pathContainer;
            int cnt = pc->getSimplePathPointCount();
            for (int i = 0; i < cnt; i++)
                pathPointsToTakeIntoAccount.push_back(i);
        }
        C3Vector minCoord, maxCoord;
        for (int i = 0; i < int(pathPointsToTakeIntoAccount.size()); i++)
        {
            CSimplePathPoint_old* aPt = pc->getSimplePathPoint(pathPointsToTakeIntoAccount[i]);
            C3Vector c(aPt->getTransformation().X);
            if (i == 0)
            {
                minCoord = c;
                maxCoord = c;
            }
            else
            {
                minCoord.keepMin(c);
                maxCoord.keepMax(c);
            }
            localPositionOnPath += c;
        }
        if (pathPointsToTakeIntoAccount.size() != 0)
            localPositionOnPath /= double(pathPointsToTakeIntoAccount.size());
        else
            return; // Should normally never happen
    }

    C4X4Matrix tr(getCumulativeTransformation().getMatrix());
    int axisInfo;
    if (_objectManipulationMode_flaggedForGridOverlay & 8)
    { // rotation.
        axisInfo = _objectManipulationMode_flaggedForGridOverlay - 8;

        if (getObjectMovementRelativity(1) == 0) // world
            tr.M.setIdentity();
        if (getObjectMovementRelativity(1) == 1) // parent frame
            tr.M = getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectMovementRelativity(1) == 2) || isPathPoints) // own frame
            tr.M = getCumulativeTransformation().getMatrix().M;
    }
    else
    { // translation
        axisInfo = _objectManipulationMode_flaggedForGridOverlay - 16;
        if (getObjectMovementRelativity(0) == 0) // world
            tr.M.setIdentity();
        if (getObjectMovementRelativity(0) == 1) // parent frame
            tr.M = getFullParentCumulativeTransformation().getMatrix().M;
        if ((getObjectMovementRelativity(0) == 2) || isPathPoints) // own frame
            tr.M = getCumulativeTransformation().getMatrix().M;
        if (isPathPoints)
            tr.X = tr * localPositionOnPath;
    }

    if (_objectManipulationMode_flaggedForGridOverlay & 8)
    { // rotation
        // set the orientation according to the rotation axis:
        C3X3Matrix rot;
        if (axisInfo == 0)
            rot.buildYRotation(piValD2); // rot around x
        if (axisInfo == 1)
            rot.buildXRotation(-piValD2); // rot around y
        if (axisInfo == 2)
            rot.setIdentity(); // rot around z
        tr.M *= rot;
    }

    glPushMatrix();

    glDisable(GL_DEPTH_TEST);
    ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (_objectManipulationMode_flaggedForGridOverlay & 8)
    { // rotation
        glTranslated(tr.X(0), tr.X(1), tr.X(2));
        C3X3Matrix rrot;
        if (getObjectMovementRelativity(1) == 2) // own frame
            rrot = tr.M;
        else
        {
            rrot.buildZRotation(_objectManipulationModeTotalRotation);
            rrot = tr.M * rrot;
        }
        C4Vector axis = rrot.getQuaternion().getAngleAndAxis();
        glRotated(axis(0) * radToDeg, axis(1), axis(2), axis(3));

        double a = 5.0 * piValue / 180.0 - _objectManipulationModeTotalRotation;
        double oldX = cos(-_objectManipulationModeTotalRotation);
        double oldY = sin(-_objectManipulationModeTotalRotation);
        double h = size * 1.4;
        double ha = size * 1.4;
        int cnt = 1;

        // First the flat green circle:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::MANIPULATION_MODE_OVERLAY_COLOR);
        glPolygonOffset(
            0.5, 0.0); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);
        glBegin(GL_QUADS);
        for (int i = 0; i < 72; i++)
        {
            glVertex3d(oldX * h, oldY * h, 0.0);
            glVertex3d(oldX * size, oldY * size, 0.0);
            oldX = cos(a);
            oldY = sin(a);
            a += 5.0 * piValue / 180.0;
            glVertex3d(oldX * size, oldY * size, 0.0);
            glVertex3d(oldX * h, oldY * h, 0.0);
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Now the graduation:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::MANIPULATION_MODE_OVERLAY_GRID_COLOR);
        a = 10.0 * piValue / 180.0 - _objectManipulationModeTotalRotation;
        oldX = cos(-_objectManipulationModeTotalRotation);
        oldY = sin(-_objectManipulationModeTotalRotation);
        for (int i = 0; i < 36; i++)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3d(oldX * size, oldY * size, 0.0);
            oldX = cos(a);
            oldY = sin(a);
            a += 10.0 * piValue / 180.0;
            glVertex3d(oldX * size, oldY * size, 0.0);
            if (cnt == 0)
                glVertex3d(oldX * h, oldY * h, 0.0);
            else
                glVertex3d(oldX * ha, oldY * ha, 0.0);
            cnt++;
            if (cnt == 3)
                cnt = 0;
            glEnd();
        }
        // Now the moving part:
        glLineWidth(3.0);
        double h2 = size * 0.8;
        ogl::drawSingle3dLine(-h, 0.0, 0.0, h, 0.0, 0.0, nullptr);
        ogl::drawSingle3dLine(h, 0.0, 0.0, cos(0.1) * h2, sin(0.1) * h2, 0.0, nullptr);
        ogl::drawSingle3dLine(h, 0.0, 0.0, cos(-0.1) * h2, sin(-0.1) * h2, 0.0, nullptr);
        std::string s(utils::getAngleString(true, _objectManipulationModeTotalRotation));
        double h3 = size * 1.1;

        ogl::setBlending(false);
        ogl::drawBitmapTextTo3dPosition(h3, 0.0, size * 0.05, s.c_str(), nullptr);
        ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ogl::drawSingle3dLine(0.0, -h, 0.0, 0.0, h, 0.0, nullptr);
        glLineWidth(1.0);
    }
    else
    { // translation
        C3X3Matrix rot;
        bool xAxisOnly = false;
        if (axisInfo == 0)
        { // y-z plane
            C3X3Matrix r1;
            r1.buildZRotation(piValD2);
            C3X3Matrix r2;
            r2.buildXRotation(piValD2);
            rot = r1 * r2;
        }
        if (axisInfo == 1)
            rot.buildXRotation(piValD2); // z-x plane
        if (axisInfo == 2)
            rot.setIdentity(); // x-y plane
        if (axisInfo == 3)
        { // x-axis
            rot.setIdentity();
            xAxisOnly = true;
        }
        if (axisInfo == 4)
        { // y-axis
            rot.buildZRotation(-piValD2);
            xAxisOnly = true;
        }
        if (axisInfo == 5)
        { // y-axis
            rot.buildYRotation(piValD2);
            xAxisOnly = true;
        }
        tr.M *= rot;
        C3Vector totTransl(rot.getTranspose() * _objectManipulationModeTotalTranslation);
        glTranslated(tr.X(0), tr.X(1), tr.X(2));
        C4Vector axis = tr.M.getQuaternion().getAngleAndAxis();
        glRotated(axis(0) * radToDeg, axis(1), axis(2), axis(3));

        glTranslated(-totTransl(0), -totTransl(1), -totTransl(2));
        C3Vector dir[2] = {C3Vector::unitXVector, C3Vector::unitYVector};
        C3Vector perp[2] = {C3Vector::unitYVector, C3Vector::unitXVector};
        double unt = size * 0.25;
        int grdCnt = 5;
        C3Vector v;

        // First the green bands:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::MANIPULATION_MODE_OVERLAY_COLOR);
        glPolygonOffset(
            0.5, 0.0); // Second argument set to 0.0 on 2009.01.05 (otherwise strange effects on some graphic cards)
        glEnable(GL_POLYGON_OFFSET_FILL);
        glBegin(GL_QUADS);
        for (int axis = 0; axis < 2; axis++)
        {
            v = dir[axis] * -unt * double(grdCnt + 1);
            C3Vector w(perp[axis] * -unt * 0.8);
            glVertex3d(v(0) + w(0), v(1) + w(1), v(2) + w(2));
            glVertex3d(v(0) - w(0), v(1) - w(1), v(2) - w(2));
            glVertex3d(-v(0) - w(0), -v(1) - w(1), -v(2) - w(2));
            glVertex3d(-v(0) + w(0), -v(1) + w(1), -v(2) + w(2));
            if (xAxisOnly)
                break;
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Now the graduation:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::MANIPULATION_MODE_OVERLAY_GRID_COLOR);
        ogl::buffer.clear();
        for (int axis = 0; axis < 2; axis++)
        {
            v = dir[axis] * -unt * double(grdCnt + 1);
            ogl::addBuffer3DPoints(v(0), v(1), v(2));
            v *= -1.0;
            ogl::addBuffer3DPoints(v(0), v(1), v(2));
            for (int i = -grdCnt; i <= grdCnt; i++)
            {
                C3Vector w(dir[axis] * -unt * double(i));
                v = perp[axis] * -unt * 0.8;
                ogl::addBuffer3DPoints(v(0) + w(0), v(1) + w(1), v(2) + w(2));
                v *= -1.0;
                ogl::addBuffer3DPoints(v(0) + w(0), v(1) + w(1), v(2) + w(2));
            }
            if (xAxisOnly)
                break;
        }
        if (ogl::buffer.size() != 0)
            ogl::drawRandom3dLines(&ogl::buffer[0], (int)ogl::buffer.size() / 3, false, nullptr);
        ogl::buffer.clear();

        // Now the moving part:
        glTranslated(totTransl(0), totTransl(1), totTransl(2));

        ogl::setBlending(false);
        ogl::buffer.clear();
        for (int axis = 0; axis < 2; axis++)
        {
            C3Vector w, s;
            w = perp[axis] * -unt * 0.8;
            s = dir[axis] * unt * 0.8;
            v = dir[axis] * -unt * double(grdCnt + 1);
            ogl::addBuffer3DPoints(v(0), v(1), v(2));
            v *= -1.0;
            ogl::addBuffer3DPoints(v(0) + s(0), v(1) + s(1), v(2) + s(2));
            ogl::addBuffer3DPoints(v(0) + s(0), v(1) + s(1), v(2) + s(2));
            ogl::addBuffer3DPoints(v(0) + w(0), v(1) + w(1), v(2) + w(2));
            ogl::addBuffer3DPoints(v(0) + s(0), v(1) + s(1), v(2) + s(2));
            ogl::addBuffer3DPoints(v(0) - w(0), v(1) - w(1), v(2) - w(2));
            std::string st(utils::getSizeString(true, totTransl(axis)));
            ogl::drawBitmapTextTo3dPosition(v(0) + s(0) * 2.0 + w(0) * 2.0, v(1) + s(1) * 2.0 + w(1) * 2.0,
                                            v(2) + s(2) * 2.0 + w(2) * 2.0, st.c_str(), nullptr);
            if (xAxisOnly)
                break;
        }
        ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(3.0);
        if (ogl::buffer.size() != 0)
            ogl::drawRandom3dLines(&ogl::buffer[0], (int)ogl::buffer.size() / 3, false, nullptr);
        ogl::buffer.clear();
        glLineWidth(1.0);
    }
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    ogl::setBlending(false);
}

bool CSceneObject::setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf, double rotationAmount,
                                                                bool perspective, int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)
    if ((!App::currentWorld->simulation->isSimulationStopped()) && (getObjectMovementOptions() & 8))
    {
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectManipulationModeEventId = -1;
        return (false);
    }
    if (App::currentWorld->simulation->isSimulationStopped() && (getObjectMovementOptions() & 4))
    {
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectManipulationModeEventId = -1;
        return (false);
    }
    static int otherAxisMemorized = 0;
    bool ctrlKeyDown = ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->getKeyDownState() & 1)) &&
                       ((_objectMovementOptions & 32) == 0);
    if ((!ctrlKeyDown) && ((getObjectMovementPreferredAxes() & 56) == 0))
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green
        // manip disc doesn't appear
        _objectManipulationModeAxisIndex = -1;
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectMovementPreferredAxesPreviousCtrlKeyDown = ctrlKeyDown;
        return (false);
    }
    if ((eventID != _objectManipulationModeEventId) || (ctrlKeyDown != _objectMovementPreferredAxesPreviousCtrlKeyDown))
    {
        if ((otherAxisMemorized > 1) && ((getObjectMovementPreferredAxes() & 56) != 0))
            otherAxisMemorized = 0;
        if (otherAxisMemorized > 2)
            otherAxisMemorized = 0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation = 0.0;
        _objectManipulationModeEventId = eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation = 0.0;
        _objectManipulationModeMouseDownPos = getCumulativeTransformation().X;
        // Let's first see around which axis we wanna rotate:
        int _objectMovementPreferredAxesTEMP = getObjectMovementPreferredAxes();
        bool specialMode = false;
        if (ctrlKeyDown)
            specialMode = true;
        C4X4Matrix objAbs(getCumulativeTransformation().getMatrix());
        C3X3Matrix rotAxes;
        if (getObjectMovementRelativity(1) == 2)
            rotAxes = objAbs.M; // own frame
        if (getObjectMovementRelativity(1) == 1)
            rotAxes = getFullParentCumulativeTransformation().getMatrix().M; // parent frame
        if (getObjectMovementRelativity(1) == 0)
            rotAxes.setIdentity(); // absolute frame

        double ml = 0.0;
        _objectManipulationModeAxisIndex = -1;
        for (int i = 0; i < 3; i++)
        {
            double l;
            if (perspective)
                l = (cameraAbsConf.X - objAbs.X) * rotAxes.axis[i];
            else
                l = cameraAbsConf.M.axis[2] * rotAxes.axis[i];
            if ((fabs(l) >= ml) && (_objectMovementPreferredAxesTEMP & (8 << i)))
            {
                ml = fabs(l);
                _objectManipulationModeAxisIndex = i;
            }
        }
        if (specialMode)
        {
            int ax = _objectManipulationModeAxisIndex;
            if (_objectManipulationModeAxisIndex == -1)
                ax = otherAxisMemorized;
            if (_objectManipulationModeAxisIndex == 0)
                ax += 1 + otherAxisMemorized;
            if (_objectManipulationModeAxisIndex == 1)
            {
                if (otherAxisMemorized == 0)
                    ax = 0;
                else
                    ax = 2;
            }
            if (_objectManipulationModeAxisIndex == 2)
                ax -= (1 + otherAxisMemorized);
            _objectManipulationModeAxisIndex = ax;
            otherAxisMemorized++;
        }
    }

    _objectMovementPreferredAxesPreviousCtrlKeyDown = ctrlKeyDown;

    if (_objectManipulationModeAxisIndex == -1)
        return (false); // rotation not allowed

    double ss = getObjectMovementStepSize(1);
    if (ss == 0.0)
        ss = App::userSettings->getRotationStepSize();
    if ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->getKeyDownState() & 2))
    {
        ss = 0.1 * degToRad;
        rotationAmount /= 5.0;
    }
    double axisEffectiveRotationAmount = 0.0;
    _objectManipulationModeSubRotation += rotationAmount;
    double w = fmod(_objectManipulationModeSubRotation, ss);
    axisEffectiveRotationAmount = _objectManipulationModeSubRotation - w;
    _objectManipulationModeTotalRotation += axisEffectiveRotationAmount;
    _objectManipulationModeSubRotation = w;

    C3Vector euler;
    euler.clear();
    euler(_objectManipulationModeAxisIndex) = axisEffectiveRotationAmount;
    C4Vector rot(euler);
    C7Vector tr(_localTransformation);
    if (getObjectMovementRelativity(1) == 2)
        tr.Q *= rot; // relative to own frame
    if (getObjectMovementRelativity(1) == 1)
        tr.Q = rot * tr.Q; // relative to parent frame
    if (getObjectMovementRelativity(1) == 0)
    { // relative to world frame
        C4Vector trq(getCumulativeTransformation().Q);
        trq = rot * trq;
        C4Vector pinv(getFullParentCumulativeTransformation().Q.getInverse());
        tr.Q = pinv * trq;
    }
    setLocalTransformation(tr);
    if (getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
        setDynamicsResetFlag(true, true);
    _objectManipulationMode_flaggedForGridOverlay = _objectManipulationModeAxisIndex + 8;
    return (true);
}

bool CSceneObject::setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,
                                                                   const C3Vector& clicked3DPoint, double prevPos[2],
                                                                   double pos[2], double screenHalfSizes[2],
                                                                   double halfSizes[2], bool perspective, int eventID)
{ // bits 0-2: position x,y,z (relative to parent frame), bits 3-5: Euler e9,e1,e2 (relative to own frame)

    if ((!App::currentWorld->simulation->isSimulationStopped()) && (getObjectMovementOptions() & 2))
    {
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectManipulationModeEventId = -1;
        return (false);
    }
    if (App::currentWorld->simulation->isSimulationStopped() && (getObjectMovementOptions() & 1))
    {
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectManipulationModeEventId = -1;
        return (false);
    }

    C4X4Matrix objAbs;
    objAbs.X = getCumulativeTransformation().X;
    if (getObjectMovementRelativity(0) == 0)
        objAbs.M.setIdentity();
    if (getObjectMovementRelativity(0) == 1)
        objAbs.M = getFullParentCumulativeTransformation().getMatrix().M;
    if (getObjectMovementRelativity(0) == 2)
        objAbs.M = getCumulativeTransformation().getMatrix().M;
    static int otherAxisMemorized = 0;
    bool ctrlKeyDown = ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->getKeyDownState() & 1)) &&
                       ((_objectMovementOptions & 16) == 0);
    if ((!ctrlKeyDown) && ((getObjectMovementPreferredAxes() & 7) == 0))
    { // This is special so that, when no manip is allowed but we held down the ctrl key and released it, the green
        // manip bars don't appear
        _objectManipulationModeAxisIndex = -1;
        _objectManipulationMode_flaggedForGridOverlay = 0;
        _objectMovementPreferredAxesPreviousCtrlKeyDown = ctrlKeyDown;
        return (false);
    }
    if (eventID != _objectManipulationModeEventId)
        _objectManipulationModeRelativePositionOfClickedPoint = clicked3DPoint - objAbs.X; // Added on 2010/07/29
    if ((eventID != _objectManipulationModeEventId) || (ctrlKeyDown != _objectMovementPreferredAxesPreviousCtrlKeyDown))
    {
        if (otherAxisMemorized > 1)
            otherAxisMemorized = 0;
        _objectManipulationModeSubTranslation.clear();
        _objectManipulationModeSubRotation = 0.0;
        _objectManipulationModeEventId = eventID;
        _objectManipulationModeTotalTranslation.clear();
        _objectManipulationModeTotalRotation = 0.0;
        _objectManipulationModeMouseDownPos = getCumulativeTransformation().X;
        // Let's first see on which plane we wanna translate:
        int _objectMovementPreferredAxesTEMP = getObjectMovementPreferredAxes();
        bool specialMode = false;
        bool specialMode2 = false;
        if (ctrlKeyDown)
        {
            if ((_objectMovementPreferredAxesTEMP & 7) != 7)
            {
                _objectMovementPreferredAxesTEMP ^= 7;
                specialMode2 = ((_objectMovementPreferredAxesTEMP & 7) == 7);
            }
            else
                specialMode = true;
        }
        double ml = 0.0;
        _objectManipulationModeAxisIndex = -1;
        unsigned char planeComb[3] = {6, 5, 3};
        for (int i = 0; i < 3; i++)
        {
            double l;
            if (perspective)
                l = (cameraAbsConf.X - objAbs.X) * objAbs.M.axis[i];
            else
                l = cameraAbsConf.M.axis[2] * objAbs.M.axis[i];
            if ((fabs(l) >= ml) && ((_objectMovementPreferredAxesTEMP & (planeComb[i])) == planeComb[i]))
            {
                ml = fabs(l);
                _objectManipulationModeAxisIndex = i;
            }
        }
        if (_objectManipulationModeAxisIndex == -1)
        { // maybe we are constrained to stay on a line?
            for (int i = 0; i < 3; i++)
            {
                if (_objectMovementPreferredAxesTEMP & (1 << i))
                    _objectManipulationModeAxisIndex = 3 + i;
            }
        }
        else
        {
            if (specialMode)
                _objectManipulationModeAxisIndex += 3;
        }
        if (specialMode2)
        {
            if (otherAxisMemorized != 0)
                _objectManipulationModeAxisIndex += 3;
            otherAxisMemorized++;
        }
    }

    _objectMovementPreferredAxesPreviousCtrlKeyDown = ctrlKeyDown;

    if (_objectManipulationModeAxisIndex == -1)
        return (false);                                                       // rotation not allowed
    C4X4Matrix originalPlane(objAbs);                                         // x-y plane
    originalPlane.X += _objectManipulationModeRelativePositionOfClickedPoint; // Added on 2010/07/29
    if (_objectManipulationModeAxisIndex == 0)
    { // y-z plane
        C3X3Matrix rot;
        rot.buildYRotation(piValD2);
        originalPlane.M *= rot;
    }
    if (_objectManipulationModeAxisIndex == 1)
    { // z-x plane
        C3X3Matrix rot;
        rot.buildXRotation(piValD2);
        originalPlane.M *= rot;
    }
    bool projectOntoXAxis = false;
    if (_objectManipulationModeAxisIndex == 3)
    { // x axis
        projectOntoXAxis = true;
    }
    if (_objectManipulationModeAxisIndex == 4)
    { // y axis
        projectOntoXAxis = true;
        C3X3Matrix rot;
        rot.buildZRotation(piValD2);
        originalPlane.M *= rot;
    }
    if (_objectManipulationModeAxisIndex == 5)
    { // z axis
        projectOntoXAxis = true;
        C3X3Matrix rot;
        rot.buildYRotation(piValD2);
        originalPlane.M *= rot;
    }

    C4X4Matrix plane(originalPlane);
    C3Vector p[2]; // previous and current point on the plane
    double d = -(plane.X * plane.M.axis[2]);
    double screenP[2] = {prevPos[0], prevPos[1]};
    C4X4Matrix cam(cameraAbsConf);
    bool singularityProblem = false;

    for (int pass = 0; pass < 2; pass++)
    {
        double tt[2];
        for (int i = 0; i < 2; i++)
        {
            if (i == 1)
            {
                screenP[0] = pos[0];
                screenP[1] = pos[1];
            }
            C3Vector pp(cam.X);
            if (!perspective)
            {
                if (fabs(cam.M.axis[2] * plane.M.axis[2]) < 0.05)
                {
                    singularityProblem = true;
                    break;
                }
                pp -= cam.M.axis[0] * halfSizes[0] * (screenP[0] / screenHalfSizes[0]);
                pp += cam.M.axis[1] * halfSizes[1] * (screenP[1] / screenHalfSizes[1]);
                double t = (-d - (plane.M.axis[2] * pp)) / (cam.M.axis[2] * plane.M.axis[2]);
                p[i] = pp + cam.M.axis[2] * t;
            }
            else
            {
                C3Vector v(cam.M.axis[2] + cam.M.axis[0] * tan(-screenP[0]) + cam.M.axis[1] * tan(screenP[1]));
                v.normalize();
                pp += v;
                if (fabs(v * plane.M.axis[2]) < 0.05)
                {
                    singularityProblem = true;
                    break;
                }
                double t = (-d - (plane.M.axis[2] * pp)) / (v * plane.M.axis[2]);
                tt[i] = t;
                p[i] = pp + v * t;
            }
        }
        if (!singularityProblem)
        {
            if ((!perspective) || (tt[0] * tt[1] > 0.0))
                break;
            singularityProblem = true;
        }
        plane.M = cam.M;
    }
    if (projectOntoXAxis)
    {
        C4X4Matrix inv(originalPlane.getInverse());
        p[0] *= inv;
        p[1] *= inv;
        p[0](1) = 0.0;
        p[0](2) = 0.0;
        p[1](1) = 0.0;
        p[1](2) = 0.0;
        p[0] *= originalPlane;
        p[1] *= originalPlane;
    }
    else
    {
        if (singularityProblem)
        { // we have to project the coordinates onto the original plane:
            C4X4Matrix inv(originalPlane.getInverse());
            p[0] *= inv;
            p[1] *= inv;
            p[0](2) = 0.0;
            p[1](2) = 0.0;
            p[0] *= originalPlane;
            p[1] *= originalPlane;
        }
    }

    // We snap the translation!
    C3Vector v(p[1] - p[0]);
    v = objAbs.getInverse().M * v;
    _objectManipulationModeSubTranslation += v;
    for (int i = 0; i < 3; i++)
    {
        double ss = getObjectMovementStepSize(0);
        if (ss == 0.0)
            ss = App::userSettings->getTranslationStepSize();
        if ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->getKeyDownState() & 2))
            ss = 0.001;
        double w = fmod(_objectManipulationModeSubTranslation(i), ss);
        v(i) = _objectManipulationModeSubTranslation(i) - w;
        _objectManipulationModeTotalTranslation(i) += _objectManipulationModeSubTranslation(i) - w;
        _objectManipulationModeSubTranslation(i) = w;
    }
    v = objAbs.M * v;
    C4X4Matrix m(getCumulativeTransformation());
    m.X += v;
    setLocalTransformation(getFullParentCumulativeTransformation().getInverse().getMatrix() * m);
    if (getDynamicFlag() > 1) // for non-static shapes, and other objects that are in the dyn. world
        setDynamicsResetFlag(true, true);
    _objectManipulationMode_flaggedForGridOverlay = _objectManipulationModeAxisIndex + 16;
    return (true);
}
#endif

void CSceneObject::buildOrUpdate_oldIk()
{
    // Build IK plugin counterpart, if not a joint:
    if (_ikPluginCounterpartHandle == -1)
        _ikPluginCounterpartHandle = App::worldContainer->pluginContainer->oldIkPlugin_createDummy();
    // Update the remote object:
    _setLocalTransformation_send(_localTransformation);
}

void CSceneObject::connect_oldIk()
{
    int h = -1;
    if (getParent() != nullptr)
        h = getParent()->getObjectHandle();
    _setParent_send(h);
}

void CSceneObject::remove_oldIk()
{
    if (_ikPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_eraseObject(_ikPluginCounterpartHandle);
    _ikPluginCounterpartHandle = -1;
}

void CSceneObject::setExtensionString(const char* str)
{
    _extensionString = str;
}

void CSceneObject::setVisibilityLayer(int l)
{
    bool diff = (_visibilityLayer != l);
    if (diff)
    {
        _visibilityLayer = l;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_layer.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyInt(cmd, l);
            App::worldContainer->pushEvent();
        }
    }
}

void CSceneObject::setChildOrder(int order)
{
    bool diff = (_childOrder != order);
    if (diff)
    {
        _childOrder = order;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_childOrder.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyInt(cmd, order);
            App::worldContainer->pushEvent();
        }
    }
}

bool CSceneObject::canDestroyNow()
{ // can be overridden
    bool retVal = true;

    if ((App::currentWorld != nullptr) && (App::currentWorld->sceneObjects != nullptr))
    { // For old scripts
        CScriptObject* it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_simulation, _objectHandle);
        if ((it != nullptr) && (it->getExecutionDepth() != 0))
            retVal = false;
        it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customization, _objectHandle);
        if ((it != nullptr) && (it->getExecutionDepth() != 0))
            retVal = false;
    }

    return retVal;
}

void CSceneObject::setObjectHandle(int newObjectHandle)
{
    _objectHandle = newObjectHandle;
}

void CSceneObject::setObjectAlias_direct(const char* newName)
{
    bool diff = (_objectAlias != newName);
    if (diff)
    {
        _objectAlias = newName;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_alias.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            ev->appendKeyText(cmd, newName);
            App::worldContainer->pushEvent();
        }
    }
}

void CSceneObject::setObjectName_direct_old(const char* newName)
{
    _objectName_old = newName;
}

void CSceneObject::setObjectAltName_direct_old(const char* newAltName)
{
    _objectAltName_old = newAltName;
}

void CSceneObject::setLocalTransformation(const C7Vector& tr)
{
    bool diff = (_localTransformation != tr);
    if (diff)
    {
        _localTransformation = tr;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_pose.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            double p[7] = {tr.X(0), tr.X(1), tr.X(2), tr.Q(1), tr.Q(2), tr.Q(3), tr.Q(0)};
            ev->appendKeyDoubleArray(cmd, p, 7);
            App::worldContainer->pushEvent();
        }
        _setLocalTransformation_send(_localTransformation);
    }
}

void CSceneObject::setLocalTransformation(const C4Vector& q)
{
    bool diff = (_localTransformation.Q != q);
    if (diff)
    {
        _localTransformation.Q = q;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_pose.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            double p[7] = {_localTransformation.X(0), _localTransformation.X(1), _localTransformation.X(2),
                           _localTransformation.Q(1), _localTransformation.Q(2), _localTransformation.Q(3),
                           _localTransformation.Q(0)};
            ev->appendKeyDoubleArray(cmd, p, 7);
            App::worldContainer->pushEvent();
        }
        C7Vector tr(_localTransformation);
        tr.Q = q;
        _setLocalTransformation_send(tr);
    }
}

void CSceneObject::setLocalTransformation(const C3Vector& x)
{
    bool diff = (_localTransformation.X != x);
    if (diff)
    {
        _localTransformation.X = x;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_pose.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, cmd, true);
            double p[7] = {_localTransformation.X(0), _localTransformation.X(1), _localTransformation.X(2),
                           _localTransformation.Q(1), _localTransformation.Q(2), _localTransformation.Q(3),
                           _localTransformation.Q(0)};
            ev->appendKeyDoubleArray(cmd, p, 7);
            App::worldContainer->pushEvent();
        }
        C7Vector tr(_localTransformation);
        tr.X = x;
        _setLocalTransformation_send(tr);
    }
}

size_t CSceneObject::getChildCount() const
{
    return _childList.size();
}

CSceneObject* CSceneObject::getChildFromIndex(size_t index) const
{
    CSceneObject* retVal = nullptr;
    if (index < _childList.size())
        retVal = _childList[index];
    return (retVal);
}

const std::vector<CSceneObject*>* CSceneObject::getChildren() const
{
    return &_childList;
}

void CSceneObject::addChild(CSceneObject* child)
{
    if (child == nullptr)
        _setChildren(nullptr); // clear all children
    else
    {
        std::vector<CSceneObject*> c(_childList.begin(), _childList.end());
        c.push_back(child);
        _setChildren(&c);
    }
}

bool CSceneObject::removeChild(const CSceneObject* child)
{
    bool retVal = false;
    std::vector<CSceneObject*> c(_childList.begin(), _childList.end());
    for (size_t i = 0; i < c.size(); i++)
    {
        if (c[i] == child)
        {
            c.erase(c.begin() + i);
            retVal = true;
            break;
        }
    }
    if (retVal)
        _setChildren(&c);
    return retVal;
}

bool CSceneObject::_setChildren(std::vector<CSceneObject*>* children)
{
    bool diff = false;
    if (children != nullptr)
    {
        diff = (children->size() != _childList.size());
        if (!diff)
        {
            for (size_t i = 0; i < children->size(); i++)
            {
                if (children->at(i) != _childList[i])
                {
                    diff = true;
                    break;
                }
            }
        }
    }
    else
        diff = (_childList.size() == 0);
    if (diff)
    {
        if (children != nullptr)
            _childList.assign(children->begin(), children->end());
        else
            _childList.clear();
        handleOrderIndexOfChildren();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propObject_children.name;
            CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            std::vector<int> ch;
            for (size_t i = 0; i < _childList.size(); i++)
                ch.push_back(_childList[i]->getObjectHandle());
            ev->appendKeyIntArray(cmd, ch.data(), ch.size());
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
#endif
    }
    return diff;
}

void CSceneObject::handleOrderIndexOfChildren()
{
    std::map<std::string, int> nameMap;
    std::vector<int> co(_childList.size());
    for (size_t i = 0; i < _childList.size(); i++)
    {
        CSceneObject* child = _childList[i];
        std::string hn(child->getObjectAlias());
        std::map<std::string, int>::iterator it = nameMap.find(hn);
        if (it == nameMap.end())
            nameMap[hn] = 0;
        else
            nameMap[hn]++;
        co[i] = nameMap[hn];
    }
    for (size_t i = 0; i < _childList.size(); i++)
    {
        CSceneObject* child = _childList[i];
        std::string hn(child->getObjectAlias());
    //    std::map<std::string, int>::iterator it = nameMap.find(hn);
        if (nameMap[hn] == 0)
            co[i] = -1; // means unique with that name, with that parent
        child->setChildOrder(co[i]);
    }
}

int CSceneObject::getChildSequence(const CSceneObject* child, int* totalSiblings /*= nullptr*/) const
{
    for (size_t i = 0; i < _childList.size(); i++)
    {
        if (_childList[i] == child)
        {
            if (totalSiblings != nullptr)
                totalSiblings[0] = int(_childList.size());
            return (int(i));
        }
    }
    if (totalSiblings != nullptr)
        totalSiblings[0] = 0;
    return (-1);
}

bool CSceneObject::setChildSequence(CSceneObject* child, int order)
{
    std::vector<CSceneObject*> c(_childList.begin(), _childList.end());
    if ((order < int(c.size())) && (order >= -int(c.size())))
    {
        if (order < 0)
            order += int(c.size()); // neg. value: from back
        for (size_t i = 0; i < c.size(); i++)
        {
            if (c[i] == child)
            {
                if (order != i)
                {
                    c.erase(c.begin() + i);
                    c.insert(c.begin() + order, child);
                    _setChildren(&c);
                }
                return true;
            }
        }
    }
    return false;
}

int CSceneObject::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_modelBase.name) == 0)
    {
        retVal = 1;
        setModelBase(pState);
    }
    else if (strcmp(pName, propObject_ignoreViewFitting.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_ignoreviewfitting) - (1 - pState) * sim_objectproperty_ignoreviewfitting);
    }
    else if (strcmp(pName, propObject_collapsed.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_collapsed) - (1 - pState) * sim_objectproperty_collapsed);
    }
    else if (strcmp(pName, propObject_selectable.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_selectable) - (1 - pState) * sim_objectproperty_selectable);
    }
    else if (strcmp(pName, propObject_selectModel.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_selectmodelbaseinstead) - (1 - pState) * sim_objectproperty_selectmodelbaseinstead);
    }
    else if (strcmp(pName, propObject_hideFromModelBB.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_dontshowasinsidemodel) - (1 - pState) * sim_objectproperty_dontshowasinsidemodel);
    }
    else if (strcmp(pName, propObject_selectInvisible.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_selectinvisible) - (1 - pState) * sim_objectproperty_selectinvisible);
    }
    else if (strcmp(pName, propObject_depthInvisible.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_depthinvisible) - (1 - pState) * sim_objectproperty_depthinvisible);
    }
    else if (strcmp(pName, propObject_cannotDelete.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_cannotdeleteduringsim) - (1 - pState) * sim_objectproperty_cannotdeleteduringsim);
    }
    else if (strcmp(pName, propObject_cannotDeleteSim.name) == 0)
    {
        retVal = 1;
        setObjectProperty((_objectProperty | sim_objectproperty_cannotdeleteduringsim) - (1 - pState) * sim_objectproperty_cannotdeleteduringsim);
    }
    else if ((strcmp(pName, propObject_modelNotCollidable.name) == 0) || (strcmp(pName, propObject_modelNotCollidableDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_collidable) - (1 - pState) * sim_modelproperty_not_collidable);
    }
    else if ((strcmp(pName, propObject_modelNotMeasurable.name) == 0) || (strcmp(pName, propObject_modelNotMeasurableDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_measurable) - (1 - pState) * sim_modelproperty_not_measurable);
    }
    else if ((strcmp(pName, propObject_modelNotDetectable.name) == 0) || (strcmp(pName, propObject_modelNotDetectableDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_detectable) - (1 - pState) * sim_modelproperty_not_detectable);
    }
    else if ((strcmp(pName, propObject_modelNotDynamic.name) == 0) || (strcmp(pName, propObject_modelNotDynamicDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_dynamic) - (1 - pState) * sim_modelproperty_not_dynamic);
    }
    else if ((strcmp(pName, propObject_modelNotRespondable.name) == 0) || (strcmp(pName, propObject_modelNotRespondableDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_respondable) - (1 - pState) * sim_modelproperty_not_respondable);
    }
    else if ((strcmp(pName, propObject_modelNotVisible.name) == 0) || (strcmp(pName, propObject_modelNotVisibleDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_visible) - (1 - pState) * sim_modelproperty_not_visible);
    }
    else if ((strcmp(pName, propObject_modelScriptsNotActive.name) == 0) || (strcmp(pName, propObject_modelScriptsNotActiveDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_scripts_inactive) - (1 - pState) * sim_modelproperty_scripts_inactive);
    }
    else if ((strcmp(pName, propObject_modelNotInParentBB.name) == 0) || (strcmp(pName, propObject_modelNotInParentBBDEPRECATED.name) == 0))
    {
        retVal = 1;
        setModelProperty((_modelProperty | sim_modelproperty_not_showasinsidemodel) - (1 - pState) * sim_modelproperty_not_showasinsidemodel);
    }
    else if ((strcmp(pName, propObject_movTranslNoSim.name) == 0) || (strcmp(pName, propObject_movTranslNoSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 1) - pState * 1);
    }
    else if ((strcmp(pName, propObject_movTranslInSim.name) == 0) || (strcmp(pName, propObject_movTranslInSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 2) - pState * 2);
    }
    else if ((strcmp(pName, propObject_movRotNoSim.name) == 0) || (strcmp(pName, propObject_movRotNoSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 4) - pState * 4);
    }
    else if ((strcmp(pName, propObject_movRotInSim.name) == 0) || (strcmp(pName, propObject_movRotInSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 8) - pState * 8);
    }
    else if ((strcmp(pName, propObject_movAltTransl.name) == 0) || (strcmp(pName, propObject_movAltTranslDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 16) - pState * 16);
    }
    else if ((strcmp(pName, propObject_movAltRot.name) == 0) || (strcmp(pName, propObject_movAltRotDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions((_objectMovementOptions | 32) - pState * 32);
    }
    else if ((strcmp(pName, propObject_movPrefTranslX.name) == 0) || (strcmp(pName, propObject_movPrefTranslXDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 1) - (1 - pState) * 1);
    }
    else if ((strcmp(pName, propObject_movPrefTranslY.name) == 0) || (strcmp(pName, propObject_movPrefTranslYDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 2) - (1 - pState) * 2);
    }
    else if ((strcmp(pName, propObject_movPrefTranslZ.name) == 0) || (strcmp(pName, propObject_movPrefTranslZDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 4) - (1 - pState) * 4);
    }
    else if ((strcmp(pName, propObject_movPrefRotX.name) == 0) || (strcmp(pName, propObject_movPrefRotXDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 8) - (1 - pState) * 8);
    }
    else if ((strcmp(pName, propObject_movPrefRotY.name) == 0) || (strcmp(pName, propObject_movPrefRotYDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 16) - (1 - pState) * 16);
    }
    else if ((strcmp(pName, propObject_movPrefRotZ.name) == 0) || (strcmp(pName, propObject_movPrefRotZDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes((_objectMovementPreferredAxes | 32) - (1 - pState) * 32);
    }
    else if (_pName == propObject_selected.name)
    {
        retVal = 1;
        if (pState)
            App::currentWorld->sceneObjects->addObjectToSelection(_objectHandle);
        else
            App::currentWorld->sceneObjects->removeObjectFromSelection(_objectHandle);
    }
    else if (_pName == propObject_collidable.name)
    {
        retVal = 1;
        setLocalObjectSpecialProperty((_localObjectSpecialProperty | sim_objectspecialproperty_collidable) - (1 - pState) * sim_objectspecialproperty_collidable);
    }
    else if (_pName == propObject_measurable.name)
    {
        retVal = 1;
        setLocalObjectSpecialProperty((_localObjectSpecialProperty | sim_objectspecialproperty_measurable) - (1 - pState) * sim_objectspecialproperty_measurable);
    }
    else if (_pName == propObject_detectable.name)
    {
        retVal = 1;
        setLocalObjectSpecialProperty((_localObjectSpecialProperty | sim_objectspecialproperty_detectable) - (1 - pState) * sim_objectspecialproperty_detectable);
    }

    return retVal;
}

int CSceneObject::getBoolProperty(const char* ppName, bool& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_modelInvisible.name) == 0)
    {
        retVal = 1;
        pState = _modelInvisible;
    }
    else if (strcmp(pName, propObject_modelBase.name) == 0)
    {
        retVal = 1;
        pState = _modelBase;
    }
    else if (strcmp(pName, propObject_ignoreViewFitting.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_ignoreviewfitting) != 0;
    }
    else if (strcmp(pName, propObject_collapsed.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_collapsed) != 0;
    }
    else if (strcmp(pName, propObject_selectable.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_selectable) != 0;
    }
    else if (strcmp(pName, propObject_selectModel.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_selectmodelbaseinstead) != 0;
    }
    else if (strcmp(pName, propObject_hideFromModelBB.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_dontshowasinsidemodel) != 0;
    }
    else if (strcmp(pName, propObject_selectInvisible.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_selectinvisible) != 0;
    }
    else if (strcmp(pName, propObject_depthInvisible.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_depthinvisible) != 0;
    }
    else if (strcmp(pName, propObject_cannotDelete.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_cannotdelete) != 0;
    }
    else if (strcmp(pName, propObject_cannotDeleteSim.name) == 0)
    {
        retVal = 1;
        pState = (_objectProperty & sim_objectproperty_cannotdeleteduringsim) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotCollidable.name) == 0) || (strcmp(pName, propObject_modelNotCollidableDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_collidable) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotMeasurable.name) == 0) || (strcmp(pName, propObject_modelNotMeasurableDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_measurable) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotDetectable.name) == 0) || (strcmp(pName, propObject_modelNotDetectableDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_detectable) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotDynamic.name) == 0) || (strcmp(pName, propObject_modelNotDynamicDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_dynamic) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotRespondable.name) == 0) || (strcmp(pName, propObject_modelNotRespondableDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_respondable) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotVisible.name) == 0) || (strcmp(pName, propObject_modelNotVisibleDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_visible) != 0;
    }
    else if ((strcmp(pName, propObject_modelScriptsNotActive.name) == 0) || (strcmp(pName, propObject_modelScriptsNotActiveDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_scripts_inactive) != 0;
    }
    else if ((strcmp(pName, propObject_modelNotInParentBB.name) == 0) || (strcmp(pName, propObject_modelNotInParentBBDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_modelProperty & sim_modelproperty_not_showasinsidemodel) != 0;
    }
    else if ((strcmp(pName, propObject_movTranslNoSim.name) == 0) || (strcmp(pName, propObject_movTranslNoSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 1) == 0;
    }
    else if ((strcmp(pName, propObject_movTranslInSim.name) == 0) || (strcmp(pName, propObject_movTranslInSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 2) == 0;
    }
    else if ((strcmp(pName, propObject_movRotNoSim.name) == 0) || (strcmp(pName, propObject_movRotNoSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 4) == 0;
    }
    else if ((strcmp(pName, propObject_movRotInSim.name) == 0) || (strcmp(pName, propObject_movRotInSimDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 8) == 0;
    }
    else if ((strcmp(pName, propObject_movAltTransl.name) == 0) || (strcmp(pName, propObject_movAltTranslDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 16) == 0;
    }
    else if ((strcmp(pName, propObject_movAltRot.name) == 0) || (strcmp(pName, propObject_movAltRotDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementOptions & 32) == 0;
    }
    else if ((strcmp(pName, propObject_movPrefTranslX.name) == 0) || (strcmp(pName, propObject_movPrefTranslXDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 1) != 0;
    }
    else if ((strcmp(pName, propObject_movPrefTranslY.name) == 0) || (strcmp(pName, propObject_movPrefTranslYDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 2) != 0;
    }
    else if ((strcmp(pName, propObject_movPrefTranslZ.name) == 0) || (strcmp(pName, propObject_movPrefTranslZDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 4) != 0;
    }
    else if ((strcmp(pName, propObject_movPrefRotX.name) == 0) || (strcmp(pName, propObject_movPrefRotXDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 8) != 0;
    }
    else if ((strcmp(pName, propObject_movPrefRotY.name) == 0) || (strcmp(pName, propObject_movPrefRotYDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 16) != 0;
    }
    else if ((strcmp(pName, propObject_movPrefRotZ.name) == 0) || (strcmp(pName, propObject_movPrefRotZDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = (_objectMovementPreferredAxes & 32) != 0;
    }
    else if (_pName == propObject_selected.name)
    {
        retVal = 1;
        pState = _selected;
    }
    else if (_pName == propObject_collidable.name)
    {
        retVal = 1;
        pState = (_localObjectSpecialProperty & sim_objectspecialproperty_collidable) != 0;
    }
    else if (_pName == propObject_measurable.name)
    {
        retVal = 1;
        pState = (_localObjectSpecialProperty & sim_objectspecialproperty_measurable) != 0;
    }
    else if (_pName == propObject_detectable.name)
    {
        retVal = 1;
        pState = (_localObjectSpecialProperty & sim_objectspecialproperty_detectable) != 0;
    }
    else if (_pName == propObject_visible.name)
    {
        retVal = 1;
        pState = isObjectVisible();
    }

    return retVal;
}

int CSceneObject::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_layer.name) == 0)
    {
        retVal = 1;
        setVisibilityLayer(pState);
    }
    else if (strcmp(pName, propObject_parentHandle.name) == 0)
    {
        retVal = 0;
        CSceneObject* newParent = App::currentWorld->sceneObjects->getObjectFromHandle(pState);
        if ( (newParent != nullptr) || (pState == -1) )
        {
            if (App::currentWorld->sceneObjects->setObjectParent(this, newParent, false))
                retVal = 1;
        }
    }
    else if (strcmp(pName, propObject_objectProperty.name) == 0)
    {
        retVal = 1;
        setObjectProperty(pState);
    }
    else if ( (strcmp(pName, propObject_modelProperty.name) == 0) || (strcmp(pName, propObject_modelPropertyDEPRECATED.name) == 0) )
    {
        retVal = 1;
        setModelProperty(pState);
    }
    else if ((strcmp(pName, propObject_movementOptions.name) == 0) || (strcmp(pName, propObject_movementOptionsDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementOptions(pState);
    }
    else if ((strcmp(pName, propObject_movementPreferredAxes.name) == 0) || (strcmp(pName, propObject_movementPreferredAxesDEPRECATED.name) == 0))
    {
        retVal = 1;
        setObjectMovementPreferredAxes(pState);
    }
    else if (_pName == propObject_hierarchyColor.name)
    {
        retVal = 1;
        setHierarchyColorIndex(pState);
    }
    else if (_pName == propObject_collectionSelfCollInd.name)
    {
        retVal = 1;
        setCollectionSelfCollisionIndicator(pState);
    }

    return retVal;
}

int CSceneObject::getIntProperty(const char* ppName, int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_layer.name) == 0)
    {
        retVal = 1;
        pState = _visibilityLayer;
    }
    else if (strcmp(pName, propObject_childOrder.name) == 0)
    {
        retVal = 1;
        pState = _childOrder;
    }
    else if (strcmp(pName, propObject_parentHandle.name) == 0)
    {
        retVal = 1;
        pState = -1;
        if (_parentObject != nullptr)
            pState = _parentObject->getObjectHandle();
    }
    else if (strcmp(pName, propObject_objectProperty.name) == 0)
    {
        retVal = 1;
        pState = _objectProperty;
    }
    else if ((strcmp(pName, propObject_modelProperty.name) == 0) || (strcmp(pName, propObject_modelPropertyDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = _modelProperty;
    }
    else if ((strcmp(pName, propObject_movementOptions.name) == 0) || (strcmp(pName, propObject_movementOptionsDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = _objectMovementOptions;
    }
    else if ((strcmp(pName, propObject_movementPreferredAxes.name) == 0) || (strcmp(pName, propObject_movementPreferredAxesDEPRECATED.name) == 0))
    {
        retVal = 1;
        pState = _objectMovementPreferredAxes;
    }
    else if (_pName == propObject_hierarchyColor.name)
    {
        retVal = 1;
        pState = _hierarchyColorIndex;
    }
    else if (_pName == propObject_collectionSelfCollInd.name)
    {
        retVal = 1;
        pState = _collectionSelfCollisionIndicator;
    }
    else if (_pName == propObject_dynamicIcon.name)
    {
        retVal = 1;
        pState = _dynamicSimulationIconCode;
    }
    else if (_pName == propObject_dynamicFlag.name)
    {
        retVal = 1;
        pState = _dynamicFlag;
    }

    return retVal;
}

int CSceneObject::setLongProperty(const char* ppName, long long int pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getLongProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_parentUid.name) == 0)
    {
        retVal = 1;
        pState = -1;
        if (_parentObject != nullptr)
            pState = _parentObject->getObjectUid();
    }
    else if (strcmp(pName, propObject_objectUid.name) == 0)
    {
        retVal = 1;
        pState = _objectUid;
    }

    return retVal;
}

int CSceneObject::setHandleProperty(const char* ppName, long long int pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getHandleProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    /*
    if (strcmp(pName, propObject_parentUid.name) == 0)
    {
        retVal = 1;
        pState = -1;
        if (_parentObject != nullptr)
            pState = _parentObject->getObjectUid();
    }
    else if (strcmp(pName, propObject_objectUid.name) == 0)
    {
        retVal = 1;
        pState = _objectUid;
    }
    */

    return retVal;
}

int CSceneObject::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getFloatProperty(const char* ppName, double& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (_pName == propObject_calcRotationVelocity.name)
    {
        pState = _measuredAngularVelocity_velocityMeasurement;
        retVal = 1;
    }

    return retVal;
}

int CSceneObject::setStringProperty(const char* ppName, const char* pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (_pName == propObject_alias.name)
    {
        App::currentWorld->sceneObjects->setObjectAlias(this, pState, false);
        retVal = 1;
    }
    else if (_pName == propObject_deprecatedName.name)
    {
        setObjectName_direct_old(pState);
        retVal = 1;
    }
    else if (_pName == propObject_modelAcknowledgment.name)
    {
        setModelAcknowledgement(pState);
        retVal = 1;
    }

    return retVal;
}

int CSceneObject::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (_pName == propObject_alias.name)
    {
        retVal = 1;
        pState = _objectAlias;
    }
    else if (_pName == propObject_deprecatedName.name)
    {
        retVal = 1;
        pState = getObjectName_old();
    }
    else if (_pName == propObject_objectType.name)
    {
        retVal = 1;
        pState = getObjectTypeInfo();
    }
    else if (_pName == propObject_modelAcknowledgment.name)
    {
        retVal = 1;
        pState = _modelAcknowledgement;
    }
    else if (_pName == propObject_persistentUid.name)
    {
        retVal = 1;
        pState = _uniquePersistentIdString;
    }
    else if (_pName == propObject_modelHash.name)
    {
        retVal = 1;
        pState = App::currentWorld->sceneObjects->getModelState(_objectHandle);
        size_t hv = std::hash<std::string>{}(pState);
        std::stringstream ss;
        ss << std::hex << hv;
        pState = ss.str();
    }

    return retVal;
}

int CSceneObject::setBufferProperty(const char* ppName, const char* buffer, int bufferL)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    if (buffer == nullptr)
        bufferL = 0;
    std::string pN(pName);
    CCustomData* customDataPtr = nullptr;
    if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        customDataPtr = &customObjectData;
    else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        customDataPtr = &customObjectData_volatile;
    if (customDataPtr != nullptr)
    {
        if (pN.size() > 0)
        {
            bool diff = customDataPtr->setData(pN.c_str(), buffer, bufferL, true);
            if (diff && _isInScene && App::worldContainer->getEventsEnabled())
            {
                CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, nullptr, false);
                customDataPtr->appendEventData(pN.c_str(), ev);
                // ev->appendKeyBuffer(cmd.c_str(), buffer, bufferL);
                App::worldContainer->pushEvent();
            }
            retVal = 1;
        }
    }

    return retVal;
}

int CSceneObject::getBufferProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    std::string pN(pName);
    const CCustomData* customDataPtr = nullptr;
    if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        customDataPtr = &customObjectData;
    else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        customDataPtr = &customObjectData_volatile;
    if (customDataPtr != nullptr)
    {
        if (pN.size() > 0)
        {
            if (customDataPtr->hasData(pN.c_str(), false) >= 0)
            {
                pState = customDataPtr->getData(pN.c_str());
                retVal = 1;
            }
        }
    }

    if (retVal == -1)
    {
        if (_pName == propObject_dna.name)
        {
            retVal = 1;
            pState = _dnaString;
        }
    }

    return retVal;
}

int CSceneObject::setIntArray2Property(const char* ppName, const int* pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getIntArray2Property(const char* ppName, int* pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::setVector2Property(const char* ppName, const double* pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getVector2Property(const char* ppName, double* pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::setVector3Property(const char* ppName, const C3Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (_pName == propObject_calcLinearVelocity.name)
    {
        retVal = 1;
        pState = _measuredLinearVelocity_velocityMeasurement;
    }
    else if (_pName == propObject_calcRotationAxis.name)
    {
        retVal = 1;
        pState = _measuredAngularVelocityAxis_velocityMeasurement;
    }
    else if (_pName == propObject_bbHsize.name)
    {
        retVal = 1;
        pState = _bbHalfSize;
    }
    else if ( (_pName == propObject_modelBBSize.name) || (_pName == propObject_modelBBPos.name) )
    {
        retVal = 1;
        C3Vector minV(C3Vector::inf);
        C3Vector maxV(C3Vector::ninf);
        if (!getModelBB((getCumulativeTransformation() * getBB(nullptr)).getInverse(), minV, maxV, true))
            pState.clear();
        else
        {
            retVal = 1;
            if (_pName == propObject_modelBBSize.name)
                pState = maxV - minV;
            else
                pState = (maxV + minV) * 0.5;
        }
    }

    return retVal;
}

int CSceneObject::setQuaternionProperty(const char* ppName, const C4Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getQuaternionProperty(const char* ppName, C4Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::setPoseProperty(const char* ppName, const C7Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_pose.name) == 0)
    {
        retVal = 1;
        setLocalTransformation(pState);
    }

    return retVal;
}

int CSceneObject::getPoseProperty(const char* ppName, C7Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    if (strcmp(pName, propObject_pose.name) == 0)
    {
        retVal = 1;
        pState = _localTransformation;
    }
    if (strcmp(pName, propObject_bbPose.name) == 0)
    {
        retVal = 1;
        pState = _bbFrame;
    }

    return retVal;
}

int CSceneObject::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::getColorProperty(const char* ppName, float* pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    return retVal;
}

int CSceneObject::setFloatArrayProperty(const char* ppName, const double* v, int vL)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    if (v == nullptr)
        vL = 0;

    if ((strcmp(pName, propObject_movementStepSize.name) == 0) || (strcmp(pName, propObject_movementStepSizeDEPRECATED.name) == 0))
    {
        if (vL >= 2)
        {
            retVal = 1;
            setObjectMovementStepSize(0, v[0]);
            setObjectMovementStepSize(1, v[1]);
        }
        else
            retVal = 0;
    }

    return retVal;
}

int CSceneObject::getFloatArrayProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    pState.clear();

    if ((strcmp(pName, propObject_movementStepSize.name) == 0) || (strcmp(pName, propObject_movementStepSizeDEPRECATED.name) == 0))
    {
        pState.push_back(_objectMovementStepSize[0]);
        pState.push_back(_objectMovementStepSize[1]);
        retVal = 1;
    }

    return retVal;
}

int CSceneObject::setIntArrayProperty(const char* ppName, const int* v, int vL)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    if (v == nullptr)
        vL = 0;

    if ((strcmp(pName, propObject_movementRelativity.name) == 0) || (strcmp(pName, propObject_movementRelativityDEPRECATED.name) == 0))
    {
        if (vL >= 2)
        {
            retVal = 1;
            setObjectMovementRelativity(0, v[0]);
            setObjectMovementRelativity(1, v[1]);
        }
        else
            retVal = 0;
    }

    return retVal;
}

int CSceneObject::getIntArrayProperty(const char* ppName, std::vector<int>& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    pState.clear();

    if ((strcmp(pName, propObject_movementRelativity.name) == 0) || (strcmp(pName, propObject_movementRelativityDEPRECATED.name) == 0))
    {
        pState.push_back(_objectMovementRelativity[0]);
        pState.push_back(_objectMovementRelativity[1]);
        retVal = 1;
    }

    return retVal;
}

int CSceneObject::setHandleArrayProperty(const char* ppName, const long long int* v, int vL)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    if (v == nullptr)
        vL = 0;

    return retVal;
}

int CSceneObject::getHandleArrayProperty(const char* ppName, std::vector<long long int>& pState) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    pState.clear();

    if (strcmp(pName, propObject_children.name) == 0)
    {
        for (size_t i = 0; i < _childList.size(); i++)
            pState.push_back(_childList[i]->getObjectHandle());
        retVal = 1;
    }

    return retVal;
}

int CSceneObject::removeProperty(const char* ppName)
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;

    std::string pN(pName);
    CCustomData* customDataPtr = nullptr;
    if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        customDataPtr = &customObjectData;
    else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        customDataPtr = &customObjectData_volatile;
    if (customDataPtr != nullptr)
    {
        if (pN.size() > 0)
        {
            int tp = customDataPtr->hasData(pN.c_str(), true);
            if (tp >= 0)
            {
                bool diff = customDataPtr->clearData((propertyStrings[tp] + pN).c_str());
                if (diff && _isInScene && App::worldContainer->getEventsEnabled())
                {
                    CCbor* ev = App::worldContainer->createSceneObjectChangedEvent(this, true, nullptr, false);
                    customDataPtr->appendEventData(pN.c_str(), ev, true);
                    App::worldContainer->pushEvent();
                }
                retVal = 1;
            }
        }
    }

    return retVal;
}

int CSceneObject::getPropertyName(int& index, std::string& pName, std::string& appartenance) const
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_sceneObject.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_sceneObject[i].name, pName.c_str()))
        {
            if ((allProps_sceneObject[i].flags & sim_propertyinfo_deprecated) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_sceneObject[i].name;
                    //pName = "object." + pName;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    if (retVal == -1)
    {
        if (customObjectData.getPropertyName(index, pName))
        {
            pName = CUSTOMDATAPREFIX + pName;
            //pName = "object." + pName;
            retVal = 1;
        }
        if (customObjectData_volatile.getPropertyName(index, pName))
        {
            pName = SIGNALPREFIX + pName;
            //pName = "object." + pName;
            retVal = 1;
        }
    }
    return retVal;
}

int CSceneObject::getPropertyName_bstatic(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_sceneObject.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_sceneObject[i].name, pName.c_str()))
        {
            if ((allProps_sceneObject[i].flags & sim_propertyinfo_deprecated) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_sceneObject[i].name;
                    //pName = "object." + pName;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CSceneObject::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt) const
{
    std::string _pName(utils::getWithoutPrefix(ppName, "object."));
    const char* pName = _pName.c_str();
    int retVal = -1;
    for (size_t i = 0; i < allProps_sceneObject.size(); i++)
    {
        if (strcmp(allProps_sceneObject[i].name, pName) == 0)
        {
            retVal = allProps_sceneObject[i].type;
            info = allProps_sceneObject[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_sceneObject[i].infoTxt, "") != 0))
                infoTxt = allProps_sceneObject[i].infoTxt;
            else
                infoTxt = allProps_sceneObject[i].shortInfoTxt;
            break;
        }
    }
    if (retVal == -1)
    {
        std::string pN(pName);
        bool signal = false;
        const CCustomData* customDataPtr = nullptr;
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
            customDataPtr = &customObjectData;
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            signal = true;
            customDataPtr = &customObjectData_volatile;
        }
        if (customDataPtr != nullptr)
        {
            if (pN.size() > 0)
            {
                int s;
                retVal = customDataPtr->hasData(pN.c_str(), true, &s);
                if (retVal >= 0)
                {
                    info = sim_propertyinfo_removable;
                    if (signal)
                        info = info | sim_propertyinfo_modelhashexclude;
                    if (s > LARGE_PROPERTY_SIZE)
                        s = s | 0x100;
                    infoTxt = "";
                }
            }
        }
    }
    return retVal;
}

int CSceneObject::getPropertyInfo_bstatic(const char* pName, int& info, std::string& infoTxt)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_sceneObject.size(); i++)
    {
        if (strcmp(allProps_sceneObject[i].name, pName) == 0)
        {
            retVal = allProps_sceneObject[i].type;
            info = allProps_sceneObject[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_sceneObject[i].infoTxt, "") != 0))
                infoTxt = allProps_sceneObject[i].infoTxt;
            else
                infoTxt = allProps_sceneObject[i].shortInfoTxt;
            break;
        }
    }
    return retVal;
}
