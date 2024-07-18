#include <simInternal.h>
#include <ikElement_old.h>
#include <app.h>
#include <tt.h>

CIkElement_old::CIkElement_old()
{
    _commonInit();
}

CIkElement_old::CIkElement_old(int theTooltip)
{
    _commonInit();
    _tipHandle = theTooltip;
}

void CIkElement_old::_commonInit()
{
    _baseHandle = -1;
    _constraintBaseHandle = -1;
    _minAngularPrecision = 0.1 * degToRad;
    _minLinearPrecision = 0.0005;
    _enabled = true;
    _constraints = sim_ik_x_constraint | sim_ik_y_constraint | sim_ik_z_constraint;
    _positionWeight = 1.0;
    _orientationWeight = 1.0;
    _tipHandle = -1;
    _ikElementPluginCounterpartHandle = -1;
    _ikGroupPluginCounterpartHandle = -1;
}

CIkElement_old::~CIkElement_old()
{
}

void CIkElement_old::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
}

void CIkElement_old::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
}

CIkElement_old *CIkElement_old::copyYourself() const
{
    CIkElement_old *newEl = new CIkElement_old();
    newEl->_objectHandle = _objectHandle; // important for copy operations connections
    newEl->_constraintBaseHandle = _constraintBaseHandle;
    newEl->_tipHandle = _tipHandle;
    newEl->_baseHandle = _baseHandle;
    newEl->_minAngularPrecision = _minAngularPrecision;
    newEl->_minLinearPrecision = _minLinearPrecision;
    newEl->_enabled = _enabled;
    newEl->_constraints = _constraints;
    newEl->_positionWeight = _positionWeight;
    newEl->_orientationWeight = _orientationWeight;
    return (newEl);
}

void CIkElement_old::serialize(CSer &ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Oid");
            ar << _objectHandle;
            ar.flush();

            ar.storeDataName("Ik2");
            ar << _tipHandle << _baseHandle;
            ar.flush();

            ar.storeDataName("Abc");
            ar << _constraintBaseHandle;
            ar.flush();

            ar.storeDataName("_r2");
            ar << _minAngularPrecision << _minLinearPrecision;
            ar.flush();

            ar.storeDataName("Ctr");
            ar << _constraints;
            ar.flush();

            ar.storeDataName("_gt");
            ar << _positionWeight << _orientationWeight;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing = 0;
            nothing = nothing + 1 * _enabled;
            ar << nothing;
            ar.flush();

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
                    if (theName.compare("Oid") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectHandle;
                    }
                    if (theName.compare("Ik2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _tipHandle >> _baseHandle;
                    }
                    if (theName.compare("Abc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _constraintBaseHandle;
                    }
                    if (theName.compare("Prc") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b;
                        ar >> a >> b;
                        _minAngularPrecision = (double)a;
                        _minLinearPrecision = (double)b;
                        _minAngularPrecision *= degToRad;
                    }
                    if (theName.compare("Pr2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b;
                        ar >> a >> b;
                        _minAngularPrecision = (double)a;
                        _minLinearPrecision = (double)b;
                    }

                    if (theName.compare("_r2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _minAngularPrecision >> _minLinearPrecision;
                    }

                    if (theName.compare("Ctr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _constraints;
                    }
                    if (theName.compare("Wgt") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float a, b;
                        ar >> a >> b;
                        _positionWeight = (double)a;
                        _orientationWeight = (double)b;
                    }

                    if (theName.compare("_gt") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _positionWeight >> _orientationWeight;
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _enabled = ((nothing & 1) != 0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_int("id", _objectHandle);

            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("tipHandle", _tipHandle);
                ar.xmlAddNode_int("baseHandle", _baseHandle);
                ar.xmlAddNode_int("alternateBaseHandle", _constraintBaseHandle);
            }
            else
            {
                std::string str;
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(_tipHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                ar.xmlAddNode_comment(" 'tip' tag: is required and has to be the name of an existing scene object ",
                                      exhaustiveXml);
                ar.xmlAddNode_string("tip", str.c_str());
                str.clear();
                it = App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                ar.xmlAddNode_string("base", str.c_str());
                str.clear();
                it = App::currentWorld->sceneObjects->getObjectFromHandle(_constraintBaseHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                ar.xmlAddNode_string("alternateBase", str.c_str());
            }

            ar.xmlPushNewNode("precision");
            ar.xmlAddNode_float("linear", _minLinearPrecision);
            ar.xmlAddNode_float("angular", _minAngularPrecision * 180.0 / piValue);
            ar.xmlPopNode();

            ar.xmlPushNewNode("weight");
            ar.xmlAddNode_float("position", _positionWeight);
            ar.xmlAddNode_float("orientation", _orientationWeight);
            ar.xmlPopNode();

            ar.xmlPushNewNode("constraints");
            ar.xmlAddNode_bool("x", _constraints & sim_ik_x_constraint);
            ar.xmlAddNode_bool("y", _constraints & sim_ik_y_constraint);
            ar.xmlAddNode_bool("z", _constraints & sim_ik_z_constraint);
            ar.xmlAddNode_bool("alpha_beta", _constraints & sim_ik_alpha_beta_constraint);
            ar.xmlAddNode_bool("gamma", _constraints & sim_ik_gamma_constraint);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("enabled", _enabled);
            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_int("id", _objectHandle);

            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("tipHandle", _tipHandle);
                ar.xmlGetNode_int("baseHandle", _baseHandle);
                ar.xmlGetNode_int("alternateBaseHandle", _constraintBaseHandle);
            }
            else
            {
                ar.xmlGetNode_string("tip", _tipLoadName, exhaustiveXml);
                ar.xmlGetNode_string("base", _baseLoadName, exhaustiveXml);
                ar.xmlGetNode_string("alternateBase", _altBaseLoadName, exhaustiveXml);
            }

            if (ar.xmlPushChildNode("precision", exhaustiveXml))
            {
                ar.xmlGetNode_float("linear", _minLinearPrecision, exhaustiveXml);
                if (ar.xmlGetNode_float("angular", _minAngularPrecision, exhaustiveXml))
                    _minAngularPrecision *= piValue / 180.0;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("weight", exhaustiveXml))
            {
                ar.xmlGetNode_float("position", _positionWeight, exhaustiveXml);
                ar.xmlGetNode_float("orientation", _orientationWeight, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("constraints", exhaustiveXml))
            {
                _constraints = 0;
                bool tmp;
                if (ar.xmlGetNode_bool("x", tmp, exhaustiveXml) && tmp)
                    _constraints |= sim_ik_x_constraint;
                if (ar.xmlGetNode_bool("y", tmp, exhaustiveXml) && tmp)
                    _constraints |= sim_ik_y_constraint;
                if (ar.xmlGetNode_bool("z", tmp, exhaustiveXml) && tmp)
                    _constraints |= sim_ik_z_constraint;
                if (ar.xmlGetNode_bool("alpha_beta", tmp, exhaustiveXml) && tmp)
                    _constraints |= sim_ik_alpha_beta_constraint;
                if (ar.xmlGetNode_bool("gamma", tmp, exhaustiveXml) && tmp)
                    _constraints |= sim_ik_gamma_constraint;
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled", _enabled, exhaustiveXml);
                ar.xmlPopNode();
            }
        }
    }
}

bool CIkElement_old::announceObjectWillBeErased(int objID, bool copyBuffer)
{ // Return value true means that this IK el object should be destroyed
    if ((_baseHandle == objID) || (_constraintBaseHandle == objID) || (_tipHandle == objID))
        return (true); // This element has to be erased!
    return (false);
}

void CIkElement_old::performObjectLoadingMapping(const std::map<int, int> *map)
{
    _tipHandle = CWorld::getLoadingMapping(map, _tipHandle);
    _baseHandle = CWorld::getLoadingMapping(map, _baseHandle);
    _constraintBaseHandle = CWorld::getLoadingMapping(map, _constraintBaseHandle);
}

int CIkElement_old::getTargetHandle() const
{
    CDummy *tip = App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
    if (tip == nullptr)
        return (-1);
    int linkedDummyHandle = tip->getLinkedDummyHandle();
    if (linkedDummyHandle == -1)
        return (-1);
    if (tip->getDummyType() != sim_dummy_linktype_ik_tip_target)
        return (-1);
    return (linkedDummyHandle); // this should be the target!
}

bool CIkElement_old::setMinLinearPrecision(double prec)
{
    tt::limitValue(0.00001, 1.0, prec);
    bool diff = (_minLinearPrecision != prec);
    if (diff)
    {
        _minLinearPrecision = prec;
        _setMinLinearPrecision_send(prec);
    }
    return (diff);
}

bool CIkElement_old::setMinAngularPrecision(double prec)
{
    tt::limitValue(0.001 * degToRad, 180.0 * degToRad, prec);
    bool diff = (_minAngularPrecision != prec);
    if (diff)
    {
        _minAngularPrecision = prec;
        _setMinAngularPrecision_send(prec);
    }
    return (diff);
}

bool CIkElement_old::setPositionWeight(double weight)
{
    tt::limitValue(0.001, 1.0, weight);
    bool diff = (_positionWeight != weight);
    if (diff)
    {
        _positionWeight = weight;
        _setPositionWeight_send(weight);
    }
    return (diff);
}

bool CIkElement_old::setOrientationWeight(double weight)
{
    tt::limitValue(0.001, 1.0, weight);
    bool diff = (_orientationWeight != weight);
    if (diff)
    {
        _orientationWeight = weight;
        _setOrientationWeight_send(weight);
    }
    return (diff);
}

void CIkElement_old::setAllInvolvedJointsToIkPluginPositions() const
{
    if (_enabled)
    {
        CSceneObject *it = App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
        CSceneObject *baseObj = App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
        while ((it != baseObj) && (it != nullptr))
        {
            it = it->getParent();
            if ((it != nullptr) && (it != baseObj))
            {
                if (it->getObjectType() == sim_sceneobject_joint)
                {
                    CJoint *joint = (CJoint *)it;
                    if ((joint->getJointMode() == sim_jointmode_ik_deprecated) ||
                        (joint->getJointMode() == sim_jointmode_reserved_previously_ikdependent))
                    {
                        int h = joint->getIkPluginCounterpartHandle();
                        if (joint->getJointType() == sim_joint_spherical)
                            joint->setSphericalTransformation(
                                App::worldContainer->pluginContainer->oldIkPlugin_getSphericalJointQuaternion(h));
                        else
                            joint->setPosition(App::worldContainer->pluginContainer->oldIkPlugin_getJointPosition(h));
                    }
                }
            }
        }
    }
}

void CIkElement_old::setAllInvolvedJointsToNewJointMode(int jointMode) const
{
    CSceneObject *iterat = App::currentWorld->sceneObjects->getDummyFromHandle(_tipHandle);
    CSceneObject *baseObj = App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
    while ((iterat != baseObj) && (iterat != nullptr))
    {
        iterat = iterat->getParent();
        if ((iterat != nullptr) && (iterat != baseObj))
        {
            if (iterat->getObjectType() == sim_sceneobject_joint)
                ((CJoint *)iterat)->setJointMode_noDynMotorTargetPosCorrection(jointMode);
        }
    }
}

void CIkElement_old::_setEnabled_send(bool e) const
{
    if (_ikElementPluginCounterpartHandle != -1)
    {
        int flags = 0;
        if (e)
            flags = flags | 1;
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementFlags(_ikGroupPluginCounterpartHandle,
                                                                            _ikElementPluginCounterpartHandle, flags);
    }
}

void CIkElement_old::_setBase_send(int h) const
{
    if (_ikElementPluginCounterpartHandle != -1)
    {
        int bh = -1;
        CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(h);
        if (obj != nullptr)
            bh = obj->getIkPluginCounterpartHandle();
        int abh = -1;
        obj = App::currentWorld->sceneObjects->getObjectFromHandle(_constraintBaseHandle);
        if (obj != nullptr)
            abh = obj->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementBase(_ikGroupPluginCounterpartHandle,
                                                                           _ikElementPluginCounterpartHandle, bh, abh);
    }
}

void CIkElement_old::_setAlternativeBaseForConstraints_send(int h) const
{
    if (_ikElementPluginCounterpartHandle != -1)
    {
        int bh = -1;
        CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(_baseHandle);
        if (obj != nullptr)
            bh = obj->getIkPluginCounterpartHandle();
        int abh = -1;
        obj = App::currentWorld->sceneObjects->getObjectFromHandle(h);
        if (obj != nullptr)
            abh = obj->getIkPluginCounterpartHandle();
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementBase(_ikGroupPluginCounterpartHandle,
                                                                           _ikElementPluginCounterpartHandle, bh, abh);
    }
}

void CIkElement_old::_setMinLinearPrecision_send(double f) const
{
    if (_ikElementPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementPrecision(
            _ikGroupPluginCounterpartHandle, _ikElementPluginCounterpartHandle, f, _minAngularPrecision);
}

void CIkElement_old::_setMinAngularPrecision_send(double f) const
{
    if (_ikElementPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementPrecision(
            _ikGroupPluginCounterpartHandle, _ikElementPluginCounterpartHandle, _minLinearPrecision, f);
}

void CIkElement_old::_setPositionWeight_send(double f) const
{
    if (_ikElementPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementWeights(
            _ikGroupPluginCounterpartHandle, _ikElementPluginCounterpartHandle, f, _orientationWeight);
}

void CIkElement_old::_setOrientationWeight_send(double f) const
{
    if (_ikElementPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementWeights(
            _ikGroupPluginCounterpartHandle, _ikElementPluginCounterpartHandle, _positionWeight, f);
}

void CIkElement_old::_setConstraints_send(int c) const
{
    if (_ikElementPluginCounterpartHandle != -1)
        App::worldContainer->pluginContainer->oldIkPlugin_setIkElementConstraints(_ikGroupPluginCounterpartHandle,
                                                                                  _ikElementPluginCounterpartHandle, c);
}

std::string CIkElement_old::getTipLoadName() const
{
    return (_tipLoadName);
}

std::string CIkElement_old::getBaseLoadName() const
{
    return (_baseLoadName);
}

std::string CIkElement_old::getAltBaseLoadName() const
{
    return (_altBaseLoadName);
}

void CIkElement_old::setIkGroupPluginCounterpartHandle(int h)
{
    _ikGroupPluginCounterpartHandle = h;
}

int CIkElement_old::getIkPluginCounterpartHandle() const
{
    return (_ikElementPluginCounterpartHandle);
}

void CIkElement_old::buildOrUpdate_oldIk()
{
    // Build remote IK element in IK plugin:
    int counterpartHandle = -1;
    CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(_tipHandle);
    if (it != nullptr)
        counterpartHandle = it->getIkPluginCounterpartHandle();
    _ikElementPluginCounterpartHandle = App::worldContainer->pluginContainer->oldIkPlugin_addIkElement(
        _ikGroupPluginCounterpartHandle, counterpartHandle);

    // Update remote IK element:
    _setEnabled_send(_enabled);
    _setBase_send(_baseHandle);
    _setAlternativeBaseForConstraints_send(_constraintBaseHandle);
    _setMinLinearPrecision_send(_minLinearPrecision);
    _setMinAngularPrecision_send(_minAngularPrecision);
    _setPositionWeight_send(_positionWeight);
    _setOrientationWeight_send(_orientationWeight);
    _setConstraints_send(_constraints);
}

void CIkElement_old::connect_oldIk()
{
}

void CIkElement_old::remove_oldIk()
{
    if ((_ikGroupPluginCounterpartHandle != -1) && (_ikElementPluginCounterpartHandle != -1))
        App::worldContainer->pluginContainer->oldIkPlugin_eraseIkElement(_ikGroupPluginCounterpartHandle,
                                                                         _ikElementPluginCounterpartHandle);
    _ikGroupPluginCounterpartHandle = -1;
    _ikElementPluginCounterpartHandle = -1;
}

double CIkElement_old::getOrientationWeight() const
{
    return (_orientationWeight);
}

int CIkElement_old::getObjectHandle() const
{
    return (_objectHandle);
}

bool CIkElement_old::getEnabled() const
{
    return (_enabled);
}

int CIkElement_old::getTipHandle() const
{
    return (_tipHandle);
}

int CIkElement_old::getBase() const
{
    return (_baseHandle);
}

int CIkElement_old::getAlternativeBaseForConstraints() const
{
    return (_constraintBaseHandle);
}

double CIkElement_old::getMinLinearPrecision() const
{
    return (_minLinearPrecision);
}

double CIkElement_old::getMinAngularPrecision() const
{
    return (_minAngularPrecision);
}

double CIkElement_old::getPositionWeight() const
{
    return (_positionWeight);
}

int CIkElement_old::getConstraints() const
{
    return (_constraints);
}

bool CIkElement_old::setObjectHandle(int newHandle)
{
    bool diff = (_objectHandle != newHandle);
    _objectHandle = newHandle;
    return (diff);
}

bool CIkElement_old::setEnabled(bool isEnabled)
{
    bool diff = (_enabled != isEnabled);
    if (diff)
    {
        _enabled = isEnabled;
        _setEnabled_send(isEnabled);
    }
    return (diff);
}

bool CIkElement_old::setTipHandle(int newTipHandle)
{
    bool diff = (_tipHandle != newTipHandle);
    _tipHandle = newTipHandle;
    return (diff);
}

bool CIkElement_old::setBase(int newBase)
{
    bool diff = (_baseHandle != newBase);
    if (diff)
    {
        _baseHandle = newBase;
        _setBase_send(newBase);
    }
    return (diff);
}

bool CIkElement_old::setAlternativeBaseForConstraints(int b)
{
    bool diff = (_constraintBaseHandle != b);
    if (diff)
    {
        _constraintBaseHandle = b;
        _setAlternativeBaseForConstraints_send(b);
    }
    return (diff);
}

bool CIkElement_old::setConstraints(int constr)
{
    bool diff = (_constraints != constr);
    if (diff)
    {
        _constraints = constr;
        _setConstraints_send(constr);
    }
    return (diff);
}
