#include <simInternal.h>
#include <simStrings.h>
#include <utils.h>
#include <dummy.h>
#include <global.h>
#include <app.h>
#include <tt.h>
#ifdef SIM_WITH_GUI
#include <dummyRendering.h>
#include <guiApp.h>
#endif

CDummy::CDummy()
{
    _objectType = sim_sceneobject_dummy;
    _localObjectSpecialProperty = 0;

    _dummySize = 0.01;
    _assemblyTag = "*";
    _assignedToParentPath = false;
    _assignedToParentPathOrientation = false;
    _linkedDummyHandle = -1;
    _linkType = sim_dummytype_default;

    _visibilityLayer = DUMMY_LAYER;
    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _freeOnPathTrajectory = false;
    _virtualDistanceOffsetOnPath_OLD = 0.0;
    _virtualDistanceOffsetOnPath_variationWhenCopy_OLD = 0.0;

    _dummyColor.setDefaultValues();
    _dummyColor.setColor(1.0f, 0.8f, 0.55f, sim_colorcomponent_ambient_diffuse);

    // Mujoco parameters
    // ----------------------------------------------------
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_dummy_range1
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_dummy_range2
    _mujocoFloatParams.push_back(0.02);  // simi_mujoco_dummy_solreflimit1
    _mujocoFloatParams.push_back(1.0);   // simi_mujoco_dummy_solreflimit2
    _mujocoFloatParams.push_back(0.9);   // simi_mujoco_dummy_solimplimit1
    _mujocoFloatParams.push_back(0.95);  // simi_mujoco_dummy_solimplimit2
    _mujocoFloatParams.push_back(0.001); // simi_mujoco_dummy_solimplimit3
    _mujocoFloatParams.push_back(0.05);  // simi_mujoco_dummy_solimplimit4
    _mujocoFloatParams.push_back(2.0);   // simi_mujoco_dummy_solimplimit5
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_dummy_margin
    _mujocoFloatParams.push_back(-1.0);  // simi_mujoco_dummy_springlength
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_dummy_stiffness
    _mujocoFloatParams.push_back(0.0);   // simi_mujoco_dummy_damping

    _mujocoIntParams.push_back(0);  // simi_mujoco_dummy_bitcoded
    _mujocoIntParams.push_back(-1); // simi_mujoco_dummy_proxyjointid
    // ----------------------------------------------------

    computeBoundingBox();
}

CDummy::~CDummy()
{
}

void CDummy::copyEnginePropertiesTo(CDummy *target)
{
    std::vector<double> fp;
    std::vector<int> ip;

    // Mujoco:
    getMujocoFloatParams(fp);
    target->setMujocoFloatParams(fp);
    getMujocoIntParams(ip);
    target->setMujocoIntParams(ip);
}

double CDummy::getEngineFloatParam(int what, bool *ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_mujoco_dummy_float_start) && (what < sim_mujoco_dummy_float_end))
    {
        int w = what - sim_mujoco_dummy_range1 + simi_mujoco_dummy_range1;
        return (_mujocoFloatParams[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0.0);
}

int CDummy::getEngineIntParam(int what, bool *ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_mujoco_dummy_int_start) && (what < sim_mujoco_dummy_int_end))
    {
        int w = what - sim_mujoco_dummy_bitcoded + simi_mujoco_dummy_bitcoded;
        return (_mujocoIntParams[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
}

bool CDummy::getEngineBoolParam(int what, bool *ok) const
{
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_mujoco_dummy_bool_start) && (what < sim_mujoco_dummy_bool_end))
    {
        int b = 1;
        int w = (what - sim_mujoco_dummy_limited);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        return ((_mujocoIntParams[simi_mujoco_dummy_bitcoded] & b) != 0);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
}

void CDummy::getMujocoFloatParams(std::vector<double> &p) const
{
    p.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
}

void CDummy::getMujocoIntParams(std::vector<int> &p) const
{
    p.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());
}

bool CDummy::setEngineFloatParam(int what, double v)
{
    if ((what > sim_mujoco_dummy_float_start) && (what < sim_mujoco_dummy_float_end))
    {
        int w = what - sim_mujoco_dummy_range1 + simi_mujoco_dummy_range1;
        std::vector<double> fp;
        getMujocoFloatParams(fp);
        fp[w] = v;
        setMujocoFloatParams(fp);
        return (true);
    }
    return (false);
}

bool CDummy::setEngineIntParam(int what, int v)
{
    if ((what > sim_mujoco_dummy_int_start) && (what < sim_mujoco_dummy_int_end))
    {
        int w = what - sim_mujoco_dummy_bitcoded + simi_mujoco_dummy_bitcoded;
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[w] = v;
        setMujocoIntParams(ip);
        return (true);
    }
    return (false);
}

bool CDummy::setEngineBoolParam(int what, bool v)
{
    if ((what > sim_mujoco_dummy_bool_start) && (what < sim_mujoco_dummy_bool_end))
    {
        int b = 1;
        int w = (what - sim_mujoco_dummy_limited);
        while (w > 0)
        {
            b *= 2;
            w--;
        }

        int bitCoded = getEngineIntParam(sim_mujoco_dummy_bitcoded, nullptr);
        bitCoded |= b;
        if (!v)
            bitCoded -= b;
        setEngineIntParam(sim_mujoco_dummy_bitcoded, bitCoded);
        return (true);
    }
    return (false);
}

void CDummy::setMujocoFloatParams(const std::vector<double> &pp, bool reflectToLinkedDummy /*=true*/)
{
    std::vector<double> p(pp);
    bool diff = (_mujocoFloatParams.size() != p.size());
    if (!diff)
    {
        for (size_t i = 0; i < p.size(); i++)
        {
            if (_mujocoFloatParams[i] != p[i])
            {
                diff = true;
                break;
            }
        }
    }
    if (diff)
    {
        _mujocoFloatParams.assign(p.begin(), p.end());
        if (reflectToLinkedDummy)
            _reflectPropToLinkedDummy();
    }
}

void CDummy::setMujocoIntParams(const std::vector<int> &p, bool reflectToLinkedDummy /*=true*/)
{
    bool diff = (_mujocoIntParams.size() != p.size());
    if (!diff)
    {
        for (size_t i = 0; i < p.size(); i++)
        {
            if (_mujocoIntParams[i] != p[i])
            {
                diff = true;
                break;
            }
        }
    }
    if (diff)
    {
        _mujocoIntParams.assign(p.begin(), p.end());
        if (reflectToLinkedDummy)
            _reflectPropToLinkedDummy();
    }
}

void CDummy::_reflectPropToLinkedDummy() const
{ // will not infinitely recurse since once identical, it stops
    if ((_linkedDummyHandle != -1) &&
        ((_linkType == sim_dummytype_dynloopclosure) || (_linkType == sim_dummytype_dyntendon)))
    {
        CDummy *l = App::currentWorld->sceneObjects->getDummyFromHandle(_linkedDummyHandle);
        l->setMujocoFloatParams(_mujocoFloatParams, false);
        l->setMujocoIntParams(_mujocoIntParams, false);
    }
}

std::string CDummy::getObjectTypeInfo() const
{
    return "dummy";
}

std::string CDummy::getObjectTypeInfoExtended() const
{
    return getObjectTypeInfo();
}

bool CDummy::isPotentiallyCollidable() const
{
    return (true);
}

bool CDummy::isPotentiallyMeasurable() const
{
    return (true);
}

bool CDummy::isPotentiallyDetectable() const
{
    return (true);
}

void CDummy::computeBoundingBox()
{
    _setBB(C7Vector::identityTransformation, C3Vector(1.0, 1.0, 1.0) * _dummySize * 0.5);
}

void CDummy::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        _dummyColor.setEventParams(_objectHandle);
    else
        _dummyColor.setEventParams(-1);
}

void CDummy::scaleObject(double scalingFactor)
{
    setDummySize(_dummySize * scalingFactor);
    _virtualDistanceOffsetOnPath_OLD *= scalingFactor;
    _virtualDistanceOffsetOnPath_variationWhenCopy_OLD *= scalingFactor;

    CSceneObject::scaleObject(scalingFactor);
}

void CDummy::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    setLinkedDummyHandle(-1, false);
}

void CDummy::addSpecializedObjectEventData(CCbor *ev) const
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    float c[9];
    _dummyColor.getColor(c, sim_colorcomponent_ambient_diffuse);
    _dummyColor.getColor(c + 3, sim_colorcomponent_specular);
    _dummyColor.getColor(c + 6, sim_colorcomponent_emission);
    ev->openKeyArray("colors");
    ev->appendFloatArray(c, 9);
    ev->closeArrayOrMap(); // colors
#else
    _dummyColor.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propDummy_size.name, _dummySize);

#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // dummy
#endif
}

CSceneObject *CDummy::copyYourself()
{
    CDummy *newDummy = (CDummy *)CSceneObject::copyYourself();
    newDummy->_linkedDummyHandle = _linkedDummyHandle; // important for copy operations connections

    _dummyColor.copyYourselfInto(&newDummy->_dummyColor);
    newDummy->_dummySize = _dummySize;
    newDummy->_linkType = _linkType;
    newDummy->_assemblyTag = _assemblyTag;
    newDummy->_assignedToParentPath = _assignedToParentPath;
    newDummy->_assignedToParentPathOrientation = _assignedToParentPathOrientation;
    newDummy->_freeOnPathTrajectory = _freeOnPathTrajectory;

    if (App::worldContainer->copyBuffer->isCopyForPasting())
    { // here the original object is not reset (the variation) because it is located in the copy buffer!
        _virtualDistanceOffsetOnPath_OLD += _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
        newDummy->_virtualDistanceOffsetOnPath_OLD = _virtualDistanceOffsetOnPath_OLD;
        newDummy->_virtualDistanceOffsetOnPath_variationWhenCopy_OLD = 0.0; // the new new object's variation is reset!
    }
    else
    {
        newDummy->_virtualDistanceOffsetOnPath_OLD = _virtualDistanceOffsetOnPath_OLD;
        newDummy->_virtualDistanceOffsetOnPath_variationWhenCopy_OLD =
            _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
        _virtualDistanceOffsetOnPath_variationWhenCopy_OLD = 0.0; // we reset the original object!!
    }

    newDummy->_mujocoFloatParams.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
    newDummy->_mujocoIntParams.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());

    return (newDummy);
}

bool CDummy::setAssignedToParentPath(bool assigned)
{
    bool diff = (_assignedToParentPath != assigned);
    if (diff)
    {
        _assignedToParentPath = assigned;
        if (_assignedToParentPath)
            setAssignedToParentPathOrientation(false);
    }
    return (diff);
}

bool CDummy::setAssignedToParentPathOrientation(bool assigned)
{
    bool diff = (_assignedToParentPathOrientation != assigned);
    if (diff)
    {
        _assignedToParentPathOrientation = assigned;
        if (_assignedToParentPathOrientation)
            setAssignedToParentPath(false);
    }
    return (diff);
}

void CDummy::setFreeOnPathTrajectory(bool isFree)
{
    _freeOnPathTrajectory = isFree;
}

void CDummy::setVirtualDistanceOffsetOnPath(double off)
{
    _virtualDistanceOffsetOnPath_OLD = off;
}

void CDummy::setVirtualDistanceOffsetOnPath_variationWhenCopy(double off)
{
    _virtualDistanceOffsetOnPath_variationWhenCopy_OLD = off;
}

void CDummy::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}

void CDummy::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}

void CDummy::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}

void CDummy::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CDummy::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
}

void CDummy::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}

void CDummy::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}

void CDummy::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CDummy::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CDummy::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
}

void CDummy::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CDummy::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() &&
            ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
        {
        }
    }
    CSceneObject::simulationEnded();
}

void CDummy::serialize(CSer &ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_y2");
            ar << _dummySize;
            ar.flush();

            ar.storeDataName("Atg");
            ar << _assemblyTag;
            ar.flush();

            ar.storeDataName("Cl0");
            ar.setCountingMode();
            _dummyColor.serialize(ar, 0);
            if (ar.setWritingMode())
                _dummyColor.serialize(ar, 0);

            ar.storeDataName("Lli");
            ar << _linkedDummyHandle;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            // removed on 2010/01/26        SIM_SET_CLEAR_BIT(dummy,0,_targetDummy);
            SIM_SET_CLEAR_BIT(dummy, 1, _assignedToParentPath);
            SIM_SET_CLEAR_BIT(dummy, 2, _freeOnPathTrajectory);
            SIM_SET_CLEAR_BIT(dummy, 3, _assignedToParentPathOrientation);
            // removed on 17/6/2011.. probably always 0     SIM_SET_CLEAR_BIT(dummy,4,_removeAfterAssembly);
            ar << dummy;
            ar.flush();

            ar.storeDataName("_o5");
            ar << _virtualDistanceOffsetOnPath_OLD << _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
            ar.flush();

            ar.storeDataName("Dl2");
            ar << _linkType;
            ar.flush();

            ar.storeDataName("_j1"); // mujoco params:
            ar << int(_mujocoFloatParams.size()) << int(_mujocoIntParams.size());
            for (size_t i = 0; i < _mujocoFloatParams.size(); i++)
                ar << _mujocoFloatParams[i];
            for (size_t i = 0; i < _mujocoIntParams.size(); i++)
                ar << _mujocoIntParams[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool before2009_12_16 = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Dy2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _dummySize = (double)bla;
                    }

                    if (theName.compare("_y2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _dummySize;
                    }

                    if (theName.compare("Atg") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _assemblyTag;
                    }

                    if (theName.compare("Lli") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _linkedDummyHandle;
                    }
                    if (theName.compare("Cl0") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        _dummyColor.serialize(ar, 0);
                    }
                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _assignedToParentPath = SIM_IS_BIT_SET(dummy, 1);
                        _freeOnPathTrajectory = SIM_IS_BIT_SET(dummy, 2);
                        _assignedToParentPathOrientation = SIM_IS_BIT_SET(dummy, 3);
                    }
                    if (theName.compare("Po4") == 0)
                    { // for backward compatibility (23/02/2011)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _virtualDistanceOffsetOnPath_OLD = (double)bla;
                    }
                    if (theName.compare("Po5") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        _virtualDistanceOffsetOnPath_OLD = (double)bla;
                        _virtualDistanceOffsetOnPath_variationWhenCopy_OLD = (double)bli;
                    }

                    if (theName.compare("_o5") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _virtualDistanceOffsetOnPath_OLD >> _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
                    }

                    if (theName.compare("Dl2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _linkType;
                    }
                    if (theName.compare("Ack") == 0)
                    { // for backward compatibility (17/6/2011)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _modelAcknowledgement;
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
                    }

                    if (theName.compare("_j1") == 0)
                    { // Mujoco params:
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
                    }

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (before2009_12_16)
            {
                if (_linkedDummyHandle == -1)
                    _linkType = sim_dummytype_default;
            }
            if (ar.getSerializationVersionThatWroteThisFile() < 17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                utils::scaleColorUp_(_dummyColor.getColorsPtr());
            }
            computeBoundingBox();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("size", _dummySize);
            ar.xmlAddNode_string("assemblyTag", _assemblyTag.c_str());

            if (exhaustiveXml)
                ar.xmlAddNode_int("linkedDummyHandle", _linkedDummyHandle);
            else
            {
                std::string str;
                CDummy *it = App::currentWorld->sceneObjects->getDummyFromHandle(_linkedDummyHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                ar.xmlAddNode_comment(
                    " 'linkedDummy' tag only provided for backward compatibility, use instead 'linkedDummyAlias' tag",
                    exhaustiveXml);
                ar.xmlAddNode_string("linkedDummy", str.c_str());
                if (it != nullptr)
                {
                    str = it->getObjectAlias() + "*";
                    str += std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("linkedDummyAlias", str.c_str());
            }

            ar.xmlAddNode_comment(" 'linkType' tag: can be 'default', 'dynamics_loopClosure', 'dynamics_tendon', "
                                  "'assembly', 'parentAssembly', 'childAssembly'",
                                  exhaustiveXml);
            ar.xmlAddNode_enum("linkType", _linkType, sim_dummytype_dynloopclosure, "dynamics_loopClosure",
                               sim_dummy_linktype_dynamics_force_constraint, "dynamics_forceConstraint",
                               sim_dummy_linktype_ik_tip_target, "ik_tipTarget", sim_dummytype_dyntendon,
                               "dynamics_tendon", sim_dummytype_default, "default", sim_dummytype_assembly, "assembly");

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("switches");
                ar.xmlAddNode_bool("assignedToParentPath", _assignedToParentPath);
                ar.xmlAddNode_bool("freeOnPathTrajectory", _freeOnPathTrajectory);
                ar.xmlAddNode_bool("assignedToParentPathOrientation", _assignedToParentPathOrientation);
                ar.xmlPopNode();

                ar.xmlAddNode_float("virtualDistanceOffsetOnPath", _virtualDistanceOffsetOnPath_OLD);
                ar.xmlAddNode_float("virtualDistanceOffsetOnPath_whenCopy",
                                    _virtualDistanceOffsetOnPath_variationWhenCopy_OLD);
            }

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
                _dummyColor.serialize(ar, 0);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_dummyColor.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("object", rgb, 3);
            }
            ar.xmlPopNode();
            ar.xmlPushNewNode("dynamics");
            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("mujoco");
            double v[5];
            for (size_t i = 0; i < 2; i++)
                v[i] = getEngineFloatParam(sim_mujoco_dummy_range1 + int(i), nullptr);
            ar.xmlAddNode_floats("range", v, 2);
            for (size_t i = 0; i < 2; i++)
                v[i] = getEngineFloatParam(sim_mujoco_dummy_solreflimit1 + int(i), nullptr);
            ar.xmlAddNode_floats("solreflimit", v, 2);
            for (size_t i = 0; i < 5; i++)
                v[i] = getEngineFloatParam(sim_mujoco_dummy_solimplimit1 + int(i), nullptr);
            ar.xmlAddNode_floats("solimplimit", v, 5);
            ar.xmlAddNode_float("margin", getEngineFloatParam(sim_mujoco_dummy_margin, nullptr));
            ar.xmlAddNode_float("springlength", getEngineFloatParam(sim_mujoco_dummy_springlength, nullptr));
            ar.xmlAddNode_float("stiffness", getEngineFloatParam(sim_mujoco_dummy_stiffness, nullptr));
            ar.xmlAddNode_float("damping", getEngineFloatParam(sim_mujoco_dummy_damping, nullptr));
            ar.xmlAddNode_bool("limited", getEngineBoolParam(sim_mujoco_dummy_limited, nullptr));
            ar.xmlPopNode(); // mujoco
            ar.xmlPopNode(); // engines
            ar.xmlPopNode(); // dynamics
        }
        else
        {
            ar.xmlGetNode_float("size", _dummySize, exhaustiveXml);
            ar.xmlGetNode_string("assemblyTag", _assemblyTag, false);

            if (exhaustiveXml)
                ar.xmlGetNode_int("linkedDummyHandle", _linkedDummyHandle);
            else
            {
                ar.xmlGetNode_string("linkedDummyAlias", _linkedDummyLoadAlias, exhaustiveXml);
                ar.xmlGetNode_string("linkedDummy", _linkedDummyLoadName_old, exhaustiveXml);
            }
            ar.xmlGetNode_enum("linkType", _linkType, exhaustiveXml, "dynamics_loopClosure",
                               sim_dummytype_dynloopclosure, "dynamics_forceConstraint",
                               sim_dummy_linktype_dynamics_force_constraint, "ik_tipTarget",
                               sim_dummy_linktype_ik_tip_target, "dynamics_tendon", sim_dummytype_dyntendon, "default",
                               sim_dummytype_default, "assembly", sim_dummytype_assembly);

            if (exhaustiveXml && ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("assignedToParentPath", _assignedToParentPath);
                ar.xmlGetNode_bool("freeOnPathTrajectory", _freeOnPathTrajectory);
                ar.xmlGetNode_bool("assignedToParentPathOrientation", _assignedToParentPathOrientation);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                ar.xmlGetNode_float("virtualDistanceOffsetOnPath", _virtualDistanceOffsetOnPath_OLD, exhaustiveXml);
                ar.xmlGetNode_float("virtualDistanceOffsetOnPath_whenCopy",
                                    _virtualDistanceOffsetOnPath_variationWhenCopy_OLD, exhaustiveXml);
            }

            if (ar.xmlPushChildNode("color", exhaustiveXml))
            {
                if (exhaustiveXml)
                    _dummyColor.serialize(ar, 0);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object", rgb, 3, exhaustiveXml))
                        _dummyColor.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                             sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("dynamics", exhaustiveXml))
            {
                if (ar.xmlPushChildNode("engines", exhaustiveXml))
                {
                    if (ar.xmlPushChildNode("mujoco", exhaustiveXml))
                    {
                        double w[5];
                        if (ar.xmlGetNode_floats("range", w, 2, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 2; j++)
                                setEngineFloatParam(sim_mujoco_dummy_range1 + int(j), w[j]);
                        }
                        if (ar.xmlGetNode_floats("solreflimit", w, 2, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 2; j++)
                                setEngineFloatParam(sim_mujoco_dummy_solreflimit1 + int(j), w[j]);
                        }
                        if (ar.xmlGetNode_floats("solimplimit", w, 5, exhaustiveXml))
                        {
                            for (size_t j = 0; j < 5; j++)
                                setEngineFloatParam(sim_mujoco_dummy_solimplimit1 + int(j), w[j]);
                        }
                        double v;
                        if (ar.xmlGetNode_float("margin", v, exhaustiveXml))
                            setEngineFloatParam(sim_mujoco_dummy_margin, v);
                        if (ar.xmlGetNode_float("springlength", v, exhaustiveXml))
                            setEngineFloatParam(sim_mujoco_dummy_springlength, v);
                        if (ar.xmlGetNode_float("stiffness", v, exhaustiveXml))
                            setEngineFloatParam(sim_mujoco_dummy_stiffness, v);
                        if (ar.xmlGetNode_float("damping", v, exhaustiveXml))
                            setEngineFloatParam(sim_mujoco_dummy_damping, v);
                        bool bv;
                        if (ar.xmlGetNode_bool("limited", bv, exhaustiveXml))
                            setEngineBoolParam(sim_mujoco_dummy_limited, bv);
                        ar.xmlPopNode(); // mujoco
                    }
                    ar.xmlPopNode(); // engines
                }
                ar.xmlPopNode(); // dynamics
            }
            computeBoundingBox();
        }
    }
}

// SPECIAL FUNCTION TO GUARANTEE FORWARD COMPATIBILITY WHEN LOADING OBJECT TYPES THAT DON'T EXIST YET!
void CDummy::loadUnknownObjectType(CSer &ar)
{
    CSceneObject::serialize(ar);
    // Now finish reading the object! (but we throw its content away!)
    std::string theName = "";
    while (theName.compare(SER_END_OF_OBJECT) != 0)
    {
        theName = ar.readDataName();
        if (theName.compare(SER_END_OF_OBJECT) != 0)
            ar.loadUnknownData();
    }
}

void CDummy::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
    _linkedDummyHandle = CWorld::getLoadingMapping(map, _linkedDummyHandle);
    std::vector<int> ip;
    getMujocoIntParams(ip);
    ip[1] = CWorld::getLoadingMapping(map, ip[1]); // Mujoco proxy joint
    setMujocoIntParams(ip);
}

void CDummy::setLinkedDummyHandle(int handle, bool check)
{
    int _linkedDummyHandleOld = _linkedDummyHandle;
    CDummy *thisObject = nullptr;
    if (check)
        thisObject = App::currentWorld->sceneObjects->getDummyFromHandle(_objectHandle);
    if (thisObject != this)
        _linkedDummyHandle = handle;
    else
    {
        CDummy *linkedDummy = App::currentWorld->sceneObjects->getDummyFromHandle(_linkedDummyHandle);
        if (handle == -1)
        { // we unlink this dummy and its partner:
            if (linkedDummy != nullptr)
                linkedDummy->setLinkedDummyHandle(-1, false);
            _linkedDummyHandle = -1;
        }
        else if (_linkedDummyHandle != handle)
        { // We link this dummy to another dummy
            CDummy *newLinkedDummy = App::currentWorld->sceneObjects->getDummyFromHandle(handle);
            if (linkedDummy != nullptr)
                linkedDummy->setLinkedDummyHandle(-1, false); // we first detach it from its old partner
            if (newLinkedDummy != nullptr)
            { // we detach the new dummy from its original linking:
                newLinkedDummy->setLinkedDummyHandle(-1, true);
                _linkedDummyHandle = handle;
                newLinkedDummy->setLinkedDummyHandle(getObjectHandle(), false);

                if (_linkType == sim_dummy_linktype_gcs_tip)
                    newLinkedDummy->setDummyType(sim_dummy_linktype_gcs_target, false);
                if (_linkType == sim_dummy_linktype_gcs_target)
                    newLinkedDummy->setDummyType(sim_dummy_linktype_gcs_tip, false);
                if ((_linkType == sim_dummy_linktype_ik_tip_target) ||
                    (_linkType == sim_dummy_linktype_gcs_loop_closure) || (_linkType == sim_dummytype_dynloopclosure) ||
                    (_linkType == sim_dummytype_dyntendon) ||
                    (_linkType == sim_dummy_linktype_dynamics_force_constraint))
                    newLinkedDummy->setDummyType(_linkType, false);
            }
            else
                _linkedDummyHandle = -1;
        }
    }
    if (_linkedDummyHandleOld != _linkedDummyHandle)
    {
        _reflectPropToLinkedDummy();
        _setLinkedDummyHandle_sendOldIk(_linkedDummyHandle);
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
        GuiApp::setFullDialogRefreshFlag();
#endif
    }
}

bool CDummy::setDummyType(int lt, bool check)
{
    bool diff = (_linkType != lt);
    if (diff)
    {
        _linkType = lt;
        _setLinkType_sendOldIk(lt);
        _dummyColor.setDefaultValues();
        if (lt == sim_dummytype_default)
            _dummyColor.setColor(1.0f, 0.8f, 0.55f, sim_colorcomponent_ambient_diffuse);
        if (lt == sim_dummytype_dynloopclosure)
            _dummyColor.setColor(0.0f, 1.0f, 1.0f, sim_colorcomponent_ambient_diffuse);
        if (lt == sim_dummytype_dyntendon)
            _dummyColor.setColor(0.0f, 0.5f, 1.0f, sim_colorcomponent_ambient_diffuse);
        if (lt == sim_dummytype_assembly)
            _dummyColor.setColor(1.0f, 0.0f, 0.0f, sim_colorcomponent_ambient_diffuse);
    }
    if ((_linkedDummyHandle != -1) && check)
    {
        CDummy *it = App::currentWorld->sceneObjects->getDummyFromHandle(_linkedDummyHandle);
        CDummy *thisObject = App::currentWorld->sceneObjects->getDummyFromHandle(_objectHandle);
        if ((thisObject == this) && (it != nullptr))
        { // dummy is in the scene
            if (lt == sim_dummy_linktype_gcs_tip)
                it->setDummyType(sim_dummy_linktype_gcs_target, false);
            if (lt == sim_dummy_linktype_gcs_target)
                it->setDummyType(sim_dummy_linktype_gcs_tip, false);
            if ((lt == sim_dummy_linktype_ik_tip_target) || (lt == sim_dummy_linktype_gcs_loop_closure) ||
                (lt == sim_dummytype_dynloopclosure) || (lt == sim_dummytype_dyntendon) ||
                (lt == sim_dummy_linktype_dynamics_force_constraint))
                it->setDummyType(lt, false);
        }
        _reflectPropToLinkedDummy();
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
        GuiApp::setFullDialogRefreshFlag();
#endif
    }
    return (diff);
}

void CDummy::setAssemblyTag(const char *tag)
{
    _assemblyTag = tag;
}

void CDummy::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
    if (_linkedDummyHandle == object->getObjectHandle())
        setLinkedDummyHandle(-1, !copyBuffer);
    if (_mujocoIntParams[1] == object->getObjectHandle()) // that's the Mujoco proxy joint
    {
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[1] = -1;
        setMujocoIntParams(ip);
    }
}

void CDummy::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CDummy::_setLinkedDummyHandle_sendOldIk(int h) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int hh = -1;
        if (h != -1)
        {
            if (_linkType == sim_dummy_linktype_ik_tip_target)
                hh = App::currentWorld->sceneObjects->getObjectFromHandle(h)->getIkPluginCounterpartHandle();
        }
        App::worldContainer->pluginContainer->oldIkPlugin_setLinkedDummy(_ikPluginCounterpartHandle, hh);
    }
}

void CDummy::_setLinkType_sendOldIk(int t) const
{
    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle != -1)
    {
        int hh = -1;
        if (_linkedDummyHandle != -1)
        {
            if (t == sim_dummy_linktype_ik_tip_target)
                hh = App::currentWorld->sceneObjects->getObjectFromHandle(_linkedDummyHandle)
                         ->getIkPluginCounterpartHandle();
        }
        App::worldContainer->pluginContainer->oldIkPlugin_setLinkedDummy(_ikPluginCounterpartHandle, hh);
    }
}

bool CDummy::getFreeOnPathTrajectory() const
{
    return (_freeOnPathTrajectory);
}

double CDummy::getVirtualDistanceOffsetOnPath() const
{
    return (_virtualDistanceOffsetOnPath_OLD);
}

double CDummy::getVirtualDistanceOffsetOnPath_variationWhenCopy() const
{
    return (_virtualDistanceOffsetOnPath_variationWhenCopy_OLD);
}

std::string CDummy::getLinkedDummyLoadName_old() const
{
    return (_linkedDummyLoadName_old);
}

std::string CDummy::getLinkedDummyLoadAlias() const
{
    return (_linkedDummyLoadAlias);
}

void CDummy::buildOrUpdate_oldIk()
{
    CSceneObject::buildOrUpdate_oldIk();
}

void CDummy::connect_oldIk()
{
    CSceneObject::connect_oldIk();

    _setLinkedDummyHandle_sendOldIk(_linkedDummyHandle);
    _setLinkType_sendOldIk(_linkType);
}

bool CDummy::getAssignedToParentPath() const
{
    return (_assignedToParentPath);
}

bool CDummy::getAssignedToParentPathOrientation() const
{
    return (_assignedToParentPathOrientation);
}

double CDummy::getDummySize() const
{
    return (_dummySize);
}

int CDummy::getDummyType() const
{
    return (_linkType);
}

std::string CDummy::getAssemblyTag() const
{
    return (_assemblyTag);
}

int CDummy::getLinkedDummyHandle() const
{
    return (_linkedDummyHandle);
}

CColorObject *CDummy::getDummyColor()
{
    return (&_dummyColor);
}

void CDummy::setDummySize(double s)
{
    bool diff = (_dummySize != s);
    if (diff)
    {
        _dummySize = s;
        computeBoundingBox();
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propDummy_size.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _dummySize);
            App::worldContainer->pushEvent();
        }
    }
}

#ifdef SIM_WITH_GUI
void CDummy::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayDummy(this, renderingObject, displayAttrib);
}
#endif

int CDummy::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _dummyColor.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(pName, propDummy_size.name) == 0)
        {
            setDummySize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CDummy::getFloatProperty(const char* ppName, double& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = _dummyColor.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (strcmp(pName, propDummy_size.name) == 0)
        {
            pState = _dummySize;
            retVal = 1;
        }
    }

    return retVal;
}

int CDummy::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _dummyColor.setColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CDummy::getColorProperty(const char* ppName, float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = _dummyColor.getColorProperty(pName, pState);
    if (retVal != -1)
    {

    }
    return retVal;
}

int CDummy::getPropertyName(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".dummy";
        retVal = _dummyColor.getPropertyName(index, pName);
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_dummy.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_dummy[i].name;
                //pName = "dummy." + pName;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CDummy::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".dummy";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_dummy.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_dummy[i].name;
                //pName = "dummy." + pName;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CDummy::getPropertyInfo(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info, size);
    if (retVal == -1)
        retVal = _dummyColor.getPropertyInfo(pName, info, size);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_dummy.size(); i++)
        {
            if (strcmp(allProps_dummy[i].name, pName) == 0)
            {
                retVal = allProps_dummy[i].type;
                info = allProps_dummy[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

int CDummy::getPropertyInfo_static(const char* ppName, int& info, int& size)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "dummy."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info, size);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, size, 1 + 4 + 8, "");
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_dummy.size(); i++)
        {
            if (strcmp(allProps_dummy[i].name, pName) == 0)
            {
                retVal = allProps_dummy[i].type;
                info = allProps_dummy[i].flags;
                size = 0;
                break;
            }
        }
    }
    return retVal;
}

