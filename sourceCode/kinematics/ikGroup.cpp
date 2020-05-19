#include "simInternal.h"
#include "distanceRoutines.h"
#include "distanceRoutines.h"
#include "ikGroup.h"
#include "ikRoutines.h"
#include "app.h"
#include "tt.h"
#include "ttUtil.h"
#include <algorithm>
#include "base64.h"
#include "pluginContainer.h"

CIkGroup::CIkGroup()
{
    _objectHandle=SIM_IDSTART_IKGROUP;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _ikPluginCounterpartHandle=-1;
    _calcTimeInMs=0;
    _initialValuesInitialized=false;
    _lastJacobian=nullptr;
    _calculationResult=sim_ikresult_not_performed;
}

CIkGroup::~CIkGroup()
{
    delete _lastJacobian;
}

void CIkGroup::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->initializeInitialValues(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
}

void CIkGroup::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CIkGroup::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->simulationEnded();
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    {
        _CIkGroup_::setExplicitHandling(_initialExplicitHandling);
    }
    _initialValuesInitialized=false;
}

CIkGroup* CIkGroup::copyYourself() const
{
    CIkGroup* newGroup=new CIkGroup();
    newGroup->_objectHandle=_objectHandle; // important for copy operations connections
    newGroup->_doOnFailOrSuccessOf=_doOnFailOrSuccessOf; // important for copy operations connections
    newGroup->_ignoreMaxStepSizes=_ignoreMaxStepSizes;
    newGroup->_objectName=_objectName;
    newGroup->_maxIterations=_maxIterations;
    newGroup->_enabled=_enabled;
    newGroup->_dampingFactor=_dampingFactor;
    newGroup->_calculationMethod=_calculationMethod;
    newGroup->_restoreIfPositionNotReached=_restoreIfPositionNotReached;
    newGroup->_restoreIfOrientationNotReached=_restoreIfOrientationNotReached;
    newGroup->_doOnFail=_doOnFail;
    newGroup->_doOnPerformed=_doOnPerformed;
    newGroup->_explicitHandling=_explicitHandling;
    newGroup->_calculationResult=_calculationResult;
    newGroup->_calcTimeInMs=0;
    for (size_t i=0;i<getIkElementCount();i++)
        newGroup->_CIkGroup_::_addIkElement(getIkElementFromIndex(i)->copyYourself());

    newGroup->_initialValuesInitialized=_initialValuesInitialized;
    newGroup->_initialExplicitHandling=_initialExplicitHandling;

    return(newGroup);
}

void CIkGroup::serialize(CSer &ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Oid");
            ar << _objectHandle;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName;
            ar.flush();

            ar.storeDataName("Mit");
            ar << _maxIterations;
            ar.flush();

            ar.storeDataName("Dpg");
            ar << _dampingFactor;
            ar.flush();

            ar.storeDataName("Cmt");
            ar << _calculationMethod;
            ar.flush();

            ar.storeDataName("Doo");
            ar << _doOnFailOrSuccessOf;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_enabled);
            SIM_SET_CLEAR_BIT(nothing,1,_restoreIfPositionNotReached);
            SIM_SET_CLEAR_BIT(nothing,2,_restoreIfOrientationNotReached);
            SIM_SET_CLEAR_BIT(nothing,3,_doOnFail);
            SIM_SET_CLEAR_BIT(nothing,4,_doOnPerformed);
            SIM_SET_CLEAR_BIT(nothing,5,!_ignoreMaxStepSizes);
            SIM_SET_CLEAR_BIT(nothing,6,_explicitHandling);
            // 14/12/2011       SIM_SET_CLEAR_BIT(nothing,7,_avoidanceCheckAgainstAll); DO NOT USE FOR BACKWARD COMPATIBILITY'S SAKE!
            ar << nothing;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            for (size_t i=0;i<getIkElementCount();i++)
            {
                ar.storeDataName("Ike");
                ar.setCountingMode();
                getIkElementFromIndex(i)->serialize(ar);
                if (ar.setWritingMode())
                    getIkElementFromIndex(i)->serialize(ar);
            }

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
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectHandle;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectName;
                    }
                    if (theName.compare("Mit")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _maxIterations;
                    }
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Ctr")==0)
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Wgt")==0)
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Jts")==0)
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Va2")==0)
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Avx")==0)
                    // DEPRECATED SINCE 1.4.2020  if (theName.compare("Avt")==0)
                    if (theName.compare("Dpg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dampingFactor;
                    }
                    if (theName.compare("Cmt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _calculationMethod;
                    }
                    if (theName.compare("Doo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _doOnFailOrSuccessOf;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _enabled=SIM_IS_BIT_SET(nothing,0);
                        _restoreIfPositionNotReached=SIM_IS_BIT_SET(nothing,1);
                        _restoreIfOrientationNotReached=SIM_IS_BIT_SET(nothing,2);
                        _doOnFail=SIM_IS_BIT_SET(nothing,3);
                        _doOnPerformed=SIM_IS_BIT_SET(nothing,4);
                        _ignoreMaxStepSizes=!SIM_IS_BIT_SET(nothing,5);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,6);
                    }
                    if (theName.compare("Uis")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (theName.compare("Ike")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        CIkElement* it=new CIkElement();
                        it->serialize(ar);
                        _CIkGroup_::_addIkElement(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_int("handle",_objectHandle);

            if (exhaustiveXml)
                ar.xmlAddNode_string("name",_objectName.c_str());
            else
                ar.xmlAddNode_string("name",("@ik@"+_objectName).c_str());

            ar.xmlAddNode_int("maxIterations",_maxIterations);

            ar.xmlAddNode_comment(" 'calculationMethod' tag: can be 'pseudoInverse', 'dls' or 'jacobianTranspose' ",exhaustiveXml);
            ar.xmlAddNode_enum("calculationMethod",_calculationMethod,sim_ik_pseudo_inverse_method,"pseudoInverse",sim_ik_damped_least_squares_method,"dls",sim_ik_jacobian_transpose_method,"jacobianTranspose");
            ar.xmlAddNode_float("dlsFactor",_dampingFactor);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("enabled",_enabled);
            ar.xmlAddNode_bool("restoreIfPositionNotReached",_restoreIfPositionNotReached);
            ar.xmlAddNode_bool("restoreIfOrientationNotReached",_restoreIfOrientationNotReached);
            if (exhaustiveXml)
            {
                ar.xmlAddNode_bool("doOnFail",_doOnFail);
                ar.xmlAddNode_bool("doOnSuccess",_doOnPerformed);
            }
            ar.xmlAddNode_bool("ignoreMaxStepSizes",_ignoreMaxStepSizes);
            ar.xmlAddNode_bool("explicitHandling",_explicitHandling);
            ar.xmlPopNode();

            if (exhaustiveXml)
                ar.xmlAddNode_int("ikGroupHandleToConditionallyExecute",_doOnFailOrSuccessOf);

            if (exhaustiveXml)
            {
                std::string str(base64_encode((unsigned char*)_uniquePersistentIdString.c_str(),_uniquePersistentIdString.size()));
                ar.xmlAddNode_string("uniquePersistentIdString_base64Coded",str.c_str());
            }

            for (size_t i=0;i<getIkElementCount();i++)
            {
                ar.xmlAddNode_comment(" 'ikElement' tag: at least one of such tag is required ",exhaustiveXml);
                ar.xmlPushNewNode("ikElement");
                getIkElementFromIndex(i)->serialize(ar);
                ar.xmlPopNode();
            }
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_int("handle",_objectHandle);

            if ( ar.xmlGetNode_string("name",_objectName,exhaustiveXml)&&(!exhaustiveXml) )
            {
                if (_objectName.find("@ik@")==0)
                    _objectName.erase(_objectName.begin(),_objectName.begin()+strlen("@ik@"));
                tt::removeIllegalCharacters(_objectName,true);
            }

            ar.xmlGetNode_int("maxIterations",_maxIterations,exhaustiveXml);

            ar.xmlGetNode_enum("calculationMethod",_calculationMethod,exhaustiveXml,"pseudoInverse",sim_ik_pseudo_inverse_method,"dls",sim_ik_damped_least_squares_method,"jacobianTranspose",sim_ik_jacobian_transpose_method);
            ar.xmlGetNode_float("dlsFactor",_dampingFactor,exhaustiveXml);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled",_enabled,exhaustiveXml);
                ar.xmlGetNode_bool("restoreIfPositionNotReached",_restoreIfPositionNotReached,exhaustiveXml);
                ar.xmlGetNode_bool("restoreIfOrientationNotReached",_restoreIfOrientationNotReached,exhaustiveXml);
                if (exhaustiveXml)
                {
                    ar.xmlGetNode_bool("doOnFail",_doOnFail);
                    ar.xmlGetNode_bool("doOnSuccess",_doOnPerformed);
                }
                ar.xmlGetNode_bool("ignoreMaxStepSizes",_ignoreMaxStepSizes,exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling",_explicitHandling,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
                ar.xmlGetNode_int("ikGroupHandleToConditionallyExecute",_doOnFailOrSuccessOf);

            if (exhaustiveXml&&ar.xmlGetNode_string("uniquePersistentIdString_base64Coded",_uniquePersistentIdString))
                _uniquePersistentIdString=base64_decode(_uniquePersistentIdString);

            if (ar.xmlPushChildNode("ikElement",true))
            {
                while (true)
                {
                    CIkElement* it=new CIkElement();
                    it->serialize(ar);
                    _CIkGroup_::_addIkElement(it);
                    if (!ar.xmlPushSiblingNode("ikElement",false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CIkGroup::serializeWExtIk(CExtIkSer &ar) const
{
    ar.writeInt(_objectHandle);

    ar.writeString(_objectName.c_str());

    ar.writeInt(_maxIterations);

    ar.writeInt(0); // DEPRECATED SINCE 1.4.2020

    ar.writeFloat(1.0f); // DEPRECATED SINCE 1.4.2020

    ar.writeFloat(0.0f); // DEPRECATED SINCE 1.4.2020
    ar.writeFloat(0.0f); // DEPRECATED SINCE 1.4.2020

    ar.writeFloat(_dampingFactor);

    ar.writeInt(_calculationMethod);

    ar.writeInt(_doOnFailOrSuccessOf);

    unsigned char nothing=0;
    SIM_SET_CLEAR_BIT(nothing,0,_enabled);
    SIM_SET_CLEAR_BIT(nothing,1,_restoreIfPositionNotReached);
    SIM_SET_CLEAR_BIT(nothing,2,_restoreIfOrientationNotReached);
    SIM_SET_CLEAR_BIT(nothing,3,_doOnFail);
    SIM_SET_CLEAR_BIT(nothing,4,_doOnPerformed);
    SIM_SET_CLEAR_BIT(nothing,5,!_ignoreMaxStepSizes);
    SIM_SET_CLEAR_BIT(nothing,6,_explicitHandling);
    ar.writeByte(nothing);

    nothing=0;
    SIM_SET_CLEAR_BIT(nothing,0,false); // DEPRECATED SINCE 1.4.2020
    ar.writeByte(nothing);

    ar.writeInt(int(getIkElementCount()));
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->serializeWExtIk(ar);
}

void CIkGroup::performObjectLoadingMapping(const std::vector<int>* map)
{
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->performObjectLoadingMapping(map);
}

void CIkGroup::performIkGroupLoadingMapping(const std::vector<int>* map)
{
    _doOnFailOrSuccessOf=CWorld::getLoadingMapping(map,_doOnFailOrSuccessOf);
}

bool CIkGroup::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    for (size_t i=0;i<getIkElementCount();i++)
    {
        if (getIkElementFromIndex(i)->announceObjectWillBeErased(objID,copyBuffer))
            return(true);
    }
    return(getIkElementCount()==0);
}

bool CIkGroup::announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer)
{ // Return value true means that this avoidance object should be destroyed
    if (_doOnFailOrSuccessOf==ikGroupID)
    {
        _doOnFailOrSuccessOf=-1;
        if (_doOnPerformed)
            _enabled=false;
    }
    return(false);
}

int CIkGroup::getIkPluginCounterpartHandle() const
{
    return(_ikPluginCounterpartHandle);
}

bool CIkGroup::setObjectName(const char* newName,bool check)
{ // Overridden from _CIkGroup_
    bool diff=false;
    CIkGroup* it=nullptr;
    if (check)
        it=App::currentWorld->ikGroups->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CIkGroup_::setObjectName(newName,check); // no checking or object not yet in world
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->ikGroups->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm,!tt::isHashFree(nm.c_str()));
                diff=_CIkGroup_::setObjectName(nm.c_str(),check);
            }
        }
    }
    return(diff);
}

bool CIkGroup::setDoOnFailOrSuccessOf(int groupID,bool check)
{ // Overridden from _CIkGroup_
    bool diff=false;
    CIkGroup* it=nullptr;
    if (check)
        it=App::currentWorld->ikGroups->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CIkGroup_::setDoOnFailOrSuccessOf(groupID,false); // no checking or object not yet in world
    else
    { // object is in world
        for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
        {
            CIkGroup* prev=App::currentWorld->ikGroups->getObjectFromIndex(i);
            if (prev->getObjectHandle()==groupID)
            {
                diff=_CIkGroup_::setDoOnFailOrSuccessOf(groupID,false);
                break;
            }
            if (prev==this)
                break;
        }
    }
    return(diff);

}

bool CIkGroup::setMaxIterations(int maxIter)
{ // Overridden from _CIkGroup_
    tt::limitValue(1,400,maxIter);
    return(_CIkGroup_::setMaxIterations(maxIter));
}

bool CIkGroup::setCalculationMethod(int theMethod)
{ // Overridden from _CIkGroup_
    bool diff=false;
    if ( (theMethod==sim_ik_pseudo_inverse_method)||(theMethod==sim_ik_damped_least_squares_method)||(theMethod==sim_ik_jacobian_transpose_method) )
        diff=_CIkGroup_::setCalculationMethod(theMethod);
    return(diff);
}

bool CIkGroup::setDampingFactor(float theFactor)
{ // Overridden from _CIkGroup_
    tt::limitValue(0.000001f,10.0f,theFactor);
    return(_CIkGroup_::setDampingFactor(theFactor));
}

bool CIkGroup::addIkElement(CIkElement* anElement)
{ // If return value if false, the calling function has to destroy anElement (invalid)
    // We check if anElement is valid:
    if (App::currentWorld->sceneObjects->getDummyFromHandle(anElement->getTipHandle())==nullptr)
        return(false); // invalid

    int newHandle=0;
    while (getIkElementFromHandle(newHandle)!=nullptr)
        newHandle++;
    anElement->setObjectHandle(newHandle);

    _addIkElement(anElement);

    return(true);
}

void CIkGroup::_addIkElement(CIkElement* anElement)
{ // Overridden from _CIkGroup_
    _CIkGroup_::_addIkElement(anElement);

    anElement->setIkGroupPluginCounterpartHandle(_ikPluginCounterpartHandle);
    if (getObjectCanSync())
        anElement->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
}

void CIkGroup::_removeIkElement(int ikElementHandle)
{ // Overridden from _CIkGroup_
    if (getObjectCanSync())
    {
        CIkElement* el=getIkElementFromHandle(ikElementHandle);
        if (el!=nullptr)
            el->removeSynchronizationObject(false);
    }

    _CIkGroup_::_removeIkElement(ikElementHandle);
}



void CIkGroup::removeIkElement(int ikElementHandle)
{
    _removeIkElement(ikElementHandle);
}

void CIkGroup::setAllInvolvedJointsToNewJointMode(int jointMode) const
{
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->setAllInvolvedJointsToNewJointMode(jointMode);
}

// OLD_IK_FUNC:
bool CIkGroup::computeOnlyJacobian(int options)
{
    // Now we prepare a vector with all valid and active elements:
    std::vector<CIkElement*> validElements;
    validElements.reserve(getIkElementCount());
    validElements.clear();

    for (int elementNumber=0;elementNumber<int(getIkElementCount());elementNumber++)
    {
        CIkElement* element=getIkElementFromIndex(elementNumber);
        CDummy* tooltip=App::currentWorld->sceneObjects->getDummyFromHandle(element->getTipHandle());
        CSceneObject* base=App::currentWorld->sceneObjects->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getEnabled())
            valid=false;
        if (tooltip==nullptr)
            valid=false; // should normally never happen!
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            CSceneObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParent();
                if (iterat==base)
                {
                    baseOk=true;
                    if (jointPresent)
                        valid=true;
                }
                if ( (iterat!=base)&&(iterat!=nullptr)&&(iterat->getObjectType()==sim_object_joint_type) )
                {
                    if ( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) )
                        jointPresent=true;
                }
            }
        }
        if (valid)
            validElements.push_back(element);
    }

    // Now validElements contains all valid elements we have to use in the following computation!
    if (validElements.size()==0)
        return(false); // error

    _resetTemporaryParameters();

    // Here we prepare all element equations:
    for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
    {
        CIkElement* element=validElements[elementNumber];
        element->prepareIkEquations(1.0);
    }
    return(performOnePass_jacobianOnly(&validElements,options));
}


int CIkGroup::computeGroupIk(bool forMotionPlanning)
{ // Return value is one of following: sim_ikresult_not_performed, sim_ikresult_success, sim_ikresult_fail
    int retVal=sim_ikresult_not_performed;
    if (_enabled)
    {
        bool doIt=true;
        if (!forMotionPlanning)
        {
            if (_doOnFailOrSuccessOf!=-1)
            { // Conditional execution part:
                CIkGroup* it=App::currentWorld->ikGroups->getObjectFromHandle(_doOnFailOrSuccessOf);
                if (it!=nullptr)
                {
                    if (_doOnPerformed)
                    {
                        if (it->getCalculationResult()==sim_ikresult_not_performed)
                            doIt=false;
                        if (it->getCalculationResult()==sim_ikresult_success)
                        {
                            if (_doOnFail)
                                doIt=false;
                        }
                        else
                        {
                            if (!_doOnFail)
                                doIt=false;
                        }
                    }
                    else
                    {
                        if (it->getCalculationResult()!=sim_ikresult_not_performed)
                            doIt=false;
                    }
                }
            }
        }

        if (doIt)
        {
            if (!forMotionPlanning)
                App::worldContainer->calcInfo->inverseKinematicsStart();

            if (App::userSettings->useOldIk||(!CPluginContainer::isIkPluginAvailable()))
            {
                bool applyNewValues=false;
                retVal=_computeGroupIk(forMotionPlanning,applyNewValues);
                if (applyNewValues)
                    _applyTemporaryParameters();
            }
            else
            {
                retVal=CPluginContainer::ikPlugin_handleIkGroup(_ikPluginCounterpartHandle);
                // do not check for success to apply values. Always apply them (the IK lib decides for that)
                for (size_t i=0;i<getIkElementCount();i++)
                {
                    CIkElement* element=getIkElementFromIndex(i);
                    element->setAllInvolvedJointsToIkPluginPositions();
                }
                if (!forMotionPlanning)
                    _setLastJacobian(CPluginContainer::ikPlugin_getJacobian(_ikPluginCounterpartHandle));
            }
            /*
            if (_lastJacobian!=nullptr)
            {
                printf("Last Jacobian: size: %i, %i\n",_lastJacobian->rows,_lastJacobian->cols);
                for (size_t i=0;i<_lastJacobian->rows*_lastJacobian->cols;i++)
                    printf("%i: %f\n",i,_lastJacobian->data[i]);
                printf("-------------------\n");
            }
            */
            if (!forMotionPlanning)
                App::worldContainer->calcInfo->inverseKinematicsEnd();
        }
    }
    return(retVal);
}

int CIkGroup::_computeGroupIk(bool forMotionPlanning,bool& applyNewValues)
{ // Return value is one of following: sim_ikresult_not_performed, sim_ikresult_success, sim_ikresult_fail
    applyNewValues=false;
    // Now we prepare a vector with all valid and active elements:
    std::vector<CIkElement*> validElements;
    validElements.reserve(getIkElementCount());
    validElements.clear();

    for (int elementNumber=0;elementNumber<int(getIkElementCount());elementNumber++)
    {
        CIkElement* element=getIkElementFromIndex(elementNumber);
        CDummy* tooltip=App::currentWorld->sceneObjects->getDummyFromHandle(element->getTipHandle());
        CSceneObject* base=App::currentWorld->sceneObjects->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getEnabled())
            valid=false;
        if (tooltip==nullptr)
            valid=false; // should normally never happen!
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            CSceneObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParent();
                if (iterat==base)
                {
                    baseOk=true;
                    if (jointPresent)
                        valid=true;
                }
                if ( (iterat!=base)&&(iterat!=nullptr)&&(iterat->getObjectType()==sim_object_joint_type) )
                {
                    if ( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) )
                        jointPresent=true;
                }
            }
            if (!valid)
            {
                // 28/4/2016 element->setActive(false); // This element has an error
                if (!baseOk)
                    element->setBase(-1); // The base was illegal!
            }
        }
        if (valid)
            validElements.push_back(element);
    }
    // Now validElements contains all valid elements we have to use in the following computation!
    if (validElements.size()==0)
        return(sim_ikresult_fail); // Error!

    _resetTemporaryParameters();

    // Here we have the main iteration loop:
    float interpolFact=1.0f; // We first try to solve in one step
    int successNumber=0;
    bool limitOrAvoidanceNeedMoreCalculation;
    bool leaveNow=false;
    bool errorOccured=false;
    for (int iterationNb=0;iterationNb<_maxIterations;iterationNb++)
    {
        // Here we prepare all element equations:
        for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
        {
            CIkElement* element=validElements[elementNumber];
            element->prepareIkEquations(interpolFact);
        }

        int res=performOnePass(&validElements,limitOrAvoidanceNeedMoreCalculation,interpolFact,forMotionPlanning);
        if (res==-1)
        {
            errorOccured=true;
            break;
        }
        if (res==1)
        { // Joint variations within tolerance
            successNumber++;

            // We check if all IK elements are under the required precision and
            // that there are not active joint limitation or avoidance equations
            bool posAndOrAreOk=true;
            for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
            {
                CIkElement* element=validElements[elementNumber];
                bool posit,orient;
                element->checkIfWithinTolerance(posit,orient,true);
                if (!(posit&&orient))
                {
                    posAndOrAreOk=false;
                    break;
                }
            }
            if (posAndOrAreOk&&(!limitOrAvoidanceNeedMoreCalculation))
                leaveNow=true; // Everything is fine, we can leave here
        }
        else
        { // Joint variations not within tolerance
            successNumber=0;
            interpolFact=interpolFact/2.0f;
            _resetTemporaryParameters();
        }

        // Here we remove all element equations (free memory)
        for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
        {
            CIkElement* element=validElements[elementNumber];
            element->removeIkEquations();
        }
        if (leaveNow)
            break;
    }
    int returnValue=sim_ikresult_success;
    if (errorOccured)
        returnValue=sim_ikresult_fail;
    applyNewValues=(!errorOccured);
    for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
    {
        CIkElement* element=validElements[elementNumber];
        bool posit,orient;
        element->checkIfWithinTolerance(posit,orient,true);
        if ( (!posit)||(!orient) )
        {
            returnValue=sim_ikresult_fail;
            if ( (_restoreIfPositionNotReached&&(!posit))||
                (_restoreIfOrientationNotReached&&(!orient)) )
                applyNewValues=false;
        }
    }
    return(returnValue);
}

void CIkGroup::_resetTemporaryParameters()
{
    // We prepare all joint temporary parameters:
    for (size_t jc=0;jc<App::currentWorld->sceneObjects->getJointCount();jc++)
    {
        CJoint* it=App::currentWorld->sceneObjects->getJointFromIndex(jc);

        it->setPosition_useTempValues(it->getPosition());
        it->initializeParametersForIK();
    }
    // We prepare all dummy temporary parameters (needed for rail-type mechanisms):
    for (size_t jc=0;jc<App::currentWorld->sceneObjects->getDummyCount();jc++)
    { // It doesn't matter if the dummy is free or not or if its parent is a path. All parameters must be set
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromIndex(jc);
        it->setTempLocalTransformation(it->getFullLocalTransformation());
    }
}

void CIkGroup::_applyTemporaryParameters()
{
    // Joints:
    for (size_t jc=0;jc<App::currentWorld->sceneObjects->getJointCount();jc++)
    {
        CJoint* it=App::currentWorld->sceneObjects->getJointFromIndex(jc);

        it->setPosition(it->getPosition_useTempValues());
        it->applyTempParametersEx();
    }
    // Dummies: (for rail-type mechanisms)
    for (size_t jc=0;jc<App::currentWorld->sceneObjects->getDummyCount();jc++)
    {
        CDummy* it=App::currentWorld->sceneObjects->getDummyFromIndex(jc);
        it->setLocalTransformation(it->getTempLocalTransformation());
    }
}

int CIkGroup::performOnePass(std::vector<CIkElement*>* validElements,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,bool forMotionPlanning)
{   // Return value -1 means that an error occured --> keep old configuration
    // Return value 0 means that the max. angular or linear variation were overpassed.
    // Return value 1 means everything went ok
    // In that case the joints temp. values are not actualized. Another pass is needed
    // Here we have the multi-ik solving algorithm:
    //********************************************************************************
    limitOrAvoidanceNeedMoreCalculation=false;
    // We prepare a vector of all used joints and a counter for the number of rows:
    std::vector<CJoint*> allJoints;
    std::vector<int> allJointStages;
    int numberOfRows=0;
    for (int elementNumber=0;elementNumber<int(validElements->size());elementNumber++)
    {
        CIkElement* element=validElements->at(elementNumber);
        numberOfRows=numberOfRows+element->matrix->rows;
        for (int i=0;i<int(element->rowJointHandles->size());i++)
        {
            int current=element->rowJointHandles->at(i);
            int currentStage=element->rowJointStages->at(i);
            // We check if that joint is already present:
            bool present=false;
            for (int j=0;j<int(allJoints.size());j++)
            {
                if ( (allJoints[j]->getObjectHandle()==current)&&(allJointStages[j]==currentStage) )
                {
                    present=true;
                    break;
                }
            }
            if (!present)
            {
                allJoints.push_back(App::currentWorld->sceneObjects->getJointFromHandle(current));
                allJointStages.push_back(currentStage);
            }
        }
    }


    // Now we prepare the individual joint constraints part:
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        if (allJoints[i]->getJointType()!=sim_joint_spherical_subtype)
        {
            if ( (allJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(allJoints[i]->getJointMode()==sim_jointmode_dependent) )
                numberOfRows++;
        }
    }
    //---------------------------------------------------------------------------

    // We prepare the main matrix and the main error vector.
    CMatrix mainMatrix(numberOfRows,int(allJoints.size()));
    CMatrix mainMatrix_correctJacobian(numberOfRows,int(allJoints.size()));
    // We have to zero it first:
    mainMatrix.clear();
    mainMatrix_correctJacobian.clear();
    CMatrix mainErrorVector(numberOfRows,1);

    // Now we fill in the main matrix and the main error vector:
    int currentRow=0;
    for (int elementNumber=0;elementNumber<int(validElements->size());elementNumber++)
    {
        CIkElement* element=validElements->at(elementNumber);
        for (int i=0;i<element->errorVector->rows;i++)
        { // We go through the rows:
            // We first set the error part:
            mainErrorVector(currentRow,0)=(*element->errorVector)(i,0);
            // Now we set the delta-parts:
            for (int j=0;j<element->matrix->cols;j++)
            { // We go through the columns:
                // We search for the right entry
                int jointID=element->rowJointHandles->at(j);
                int stage=element->rowJointStages->at(j);
                int index=0;
                while ( (allJoints[index]->getObjectHandle()!=jointID)||(allJointStages[index]!=stage) )
                    index++;
                mainMatrix(currentRow,index)=(*element->matrix)(i,j);
                mainMatrix_correctJacobian(currentRow,index)=(*element->matrix_correctJacobian)(i,j);
            }
            currentRow++;
        }
    }

    // Now we prepare the individual joint constraints part:
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        if ( ((allJoints[i]->getJointMode()==sim_jointmode_dependent)||(allJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent))&&(allJoints[i]->getJointType()!=sim_joint_spherical_subtype) )
        {
            int dependenceID=allJoints[i]->getDependencyJointHandle();
            if (dependenceID!=-1)
            {
                bool found=false;
                int j;
                for (j=0;j<int(allJoints.size());j++)
                {
                    if (allJoints[j]->getObjectHandle()==dependenceID)
                    {
                        found=true;
                        break;
                    }
                }
                if (found)
                {
                    dependenceID=j;
                    float coeff=allJoints[i]->getDependencyJointMult();
                    float fact=allJoints[i]->getDependencyJointOffset();
                    mainErrorVector(currentRow,0)=((allJoints[i]->getPosition_useTempValues()-fact)-
                                    coeff*allJoints[dependenceID]->getPosition_useTempValues())*interpolFact;
                    mainMatrix(currentRow,i)=-1.0f;
                    mainMatrix(currentRow,dependenceID)=coeff;
                    mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    mainMatrix_correctJacobian(currentRow,dependenceID)=coeff;
                }
                else
                {   // joint of dependenceID is not part of this group calculation:
                    // therefore we take its current value --> WRONG! Since all temp params are initialized!
                    CJoint* dependentJoint=App::currentWorld->sceneObjects->getJointFromHandle(dependenceID);
                    if (dependentJoint!=nullptr)
                    {
                        float coeff=allJoints[i]->getDependencyJointMult();
                        float fact=allJoints[i]->getDependencyJointOffset();
                        mainErrorVector(currentRow,0)=((allJoints[i]->getPosition_useTempValues()-fact)-
                                        coeff*dependentJoint->getPosition_useTempValues())*interpolFact;
                        mainMatrix(currentRow,i)=-1.0f;
                        mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    }
                }
            }
            else
            {
                mainErrorVector(currentRow,0)=interpolFact*(allJoints[i]->getPosition_useTempValues()-allJoints[i]->getDependencyJointOffset());
                mainMatrix(currentRow,i)=-1.0f;
                mainMatrix_correctJacobian(currentRow,i)=-1.0f;
            }
            currentRow++;
        }
    }
    //---------------------------------------------------------------------------

    // We take the joint weights into account here (part1):
    for (int i=0;i<mainMatrix.rows;i++)
    {
        for (int j=0;j<int(allJoints.size());j++)
        {
            float coeff=allJoints[j]->getIKWeight();
            if (coeff>=0.0f)
                coeff=sqrtf(coeff);
            else
                coeff=-sqrtf(-coeff);
            mainMatrix(i,j)=mainMatrix(i,j)*coeff;
            mainMatrix_correctJacobian(i,j)=mainMatrix_correctJacobian(i,j)*coeff;
        }
    }
    // Now we just have to solve:
    int doF=mainMatrix.cols;
    int eqNumb=mainMatrix.rows;
    CMatrix solution(doF,1);

    if (!forMotionPlanning)
    {
        delete _lastJacobian;
        _lastJacobian=new CMatrix(mainMatrix_correctJacobian);
    }

    if (_calculationMethod==sim_ik_pseudo_inverse_method)
    {
        CMatrix JT(mainMatrix);
        JT.transpose();
        CMatrix pseudoJ(doF,eqNumb);
        CMatrix JJTInv(mainMatrix*JT);
        if (!JJTInv.inverse())
            return(-1);
        pseudoJ=JT*JJTInv;
        solution=pseudoJ*mainErrorVector;
    }
    if (_calculationMethod==sim_ik_damped_least_squares_method)
    {
        CMatrix JT(mainMatrix);
        JT.transpose();
        CMatrix DLSJ(doF,eqNumb);
        CMatrix JJTInv(mainMatrix*JT);
        CMatrix ID(mainMatrix.rows,mainMatrix.rows);
        ID.setIdentity();
        ID/=1.0f/(_dampingFactor*_dampingFactor);
        JJTInv+=ID;
        if (!JJTInv.inverse())
            return(-1);
        DLSJ=JT*JJTInv;
        solution=DLSJ*mainErrorVector;
    }
    if (_calculationMethod==sim_ik_jacobian_transpose_method)
    {
        CMatrix JT(mainMatrix);
        JT.transpose();
        solution=JT*mainErrorVector;
    }

    // We take the joint weights into account here (part2):
    for (int i=0;i<doF;i++)
    {
        CJoint* it=allJoints[i];
        float coeff=sqrtf(fabs(it->getIKWeight()));
        solution(i,0)=solution(i,0)*coeff;
    }

    // We check if some variations are too big:
    if (!_ignoreMaxStepSizes)
    {
        for (int i=0;i<doF;i++)
        {
            CJoint* it=allJoints[i];
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                solution(i,0)=tt::getNormalizedAngle(solution(i,0));
            if (fabs(solution(i,0))>it->getMaxStepSize())
                return(0);
        }
    }
    // Now we set the computed values
    for (int i=0;i<doF;i++)
    {
        CJoint* it=allJoints[i];
        int stage=allJointStages[i];
        if (it->getJointType()!=sim_joint_spherical_subtype)
            it->setPosition_useTempValues(it->getPosition_useTempValues()+solution(i,0));
        else
            it->setTempParameterEx(it->getTempParameterEx(stage)+solution(i,0),stage);
    }
    return(1);
}

bool CIkGroup::performOnePass_jacobianOnly(std::vector<CIkElement*>* validElements,int options)
{
    // We prepare a vector of all used joints and a counter for the number of rows:
    std::vector<CJoint*> allJoints;
    std::vector<int> allJointStages;
    int numberOfRows=0;
    for (int elementNumber=0;elementNumber<int(validElements->size());elementNumber++)
    {
        CIkElement* element=validElements->at(elementNumber);
        numberOfRows=numberOfRows+element->matrix->rows;
        for (int i=0;i<int(element->rowJointHandles->size());i++)
        {
            int current=element->rowJointHandles->at(i);
            int currentStage=element->rowJointStages->at(i);
            // We check if that joint is already present:
            bool present=false;
            for (int j=0;j<int(allJoints.size());j++)
            {
                if ( (allJoints[j]->getObjectHandle()==current)&&(allJointStages[j]==currentStage) )
                {
                    present=true;
                    break;
                }
            }
            if (!present)
            {
                allJoints.push_back(App::currentWorld->sceneObjects->getJointFromHandle(current));
                allJointStages.push_back(currentStage);
            }
        }
    }

    // Now we prepare the individual joint constraints part:
    for (int i=0;i<int(allJoints.size());i++)
    {
        if (allJoints[i]->getJointType()!=sim_joint_spherical_subtype)
        {
            if ( (allJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(allJoints[i]->getJointMode()==sim_jointmode_dependent) )
                numberOfRows++;
        }
    }

    // We prepare the main matrix and the main error vector.
    CMatrix mainMatrix(numberOfRows,int(allJoints.size()));
    CMatrix mainMatrix_correctJacobian(numberOfRows,int(allJoints.size()));
    // We have to zero it first:
    mainMatrix.clear();
    mainMatrix_correctJacobian.clear();
    CMatrix mainErrorVector(numberOfRows,1);

    // Now we fill in the main matrix and the main error vector:
    int currentRow=0;
    for (int elementNumber=0;elementNumber<int(validElements->size());elementNumber++)
    {
        CIkElement* element=validElements->at(elementNumber);
        for (int i=0;i<element->errorVector->rows;i++)
        { // We go through the rows:
            // We first set the error part:
            mainErrorVector(currentRow,0)=(*element->errorVector)(i,0);
            // Now we set the delta-parts:
            for (int j=0;j<element->matrix->cols;j++)
            { // We go through the columns:
                // We search for the right entry
                int jointID=element->rowJointHandles->at(j);
                int stage=element->rowJointStages->at(j);
                int index=0;
                while ( (allJoints[index]->getObjectHandle()!=jointID)||(allJointStages[index]!=stage) )
                    index++;
                mainMatrix(currentRow,index)=(*element->matrix)(i,j);
                mainMatrix_correctJacobian(currentRow,index)=(*element->matrix_correctJacobian)(i,j);
            }
            currentRow++;
        }
    }

    // Now we prepare the individual joint constraints part:
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        if ( ((allJoints[i]->getJointMode()==sim_jointmode_dependent)||(allJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent))&&(allJoints[i]->getJointType()!=sim_joint_spherical_subtype) )
        {
            int dependenceID=allJoints[i]->getDependencyJointHandle();
            if (dependenceID!=-1)
            {
                bool found=false;
                int j;
                for (j=0;j<int(allJoints.size());j++)
                {
                    if (allJoints[j]->getObjectHandle()==dependenceID)
                    {
                        found=true;
                        break;
                    }
                }
                if (found)
                {
                    dependenceID=j;
                    float coeff=allJoints[i]->getDependencyJointMult();
                    float fact=allJoints[i]->getDependencyJointOffset();
                    mainErrorVector(currentRow,0)=((allJoints[i]->getPosition_useTempValues()-fact)-
                                    coeff*allJoints[dependenceID]->getPosition_useTempValues());
                    mainMatrix(currentRow,i)=-1.0f;
                    mainMatrix(currentRow,dependenceID)=coeff;
                    mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    mainMatrix_correctJacobian(currentRow,dependenceID)=coeff;
                }
                else
                {   // joint of dependenceID is not part of this group calculation:
                    // therefore we take its current value --> WRONG! Since all temp params are initialized!
                    CJoint* dependentJoint=App::currentWorld->sceneObjects->getJointFromHandle(dependenceID);
                    if (dependentJoint!=nullptr)
                    {
                        float coeff=allJoints[i]->getDependencyJointMult();
                        float fact=allJoints[i]->getDependencyJointOffset();
                        mainErrorVector(currentRow,0)=((allJoints[i]->getPosition_useTempValues()-fact)-
                                        coeff*dependentJoint->getPosition_useTempValues());
                        mainMatrix(currentRow,i)=-1.0f;
                        mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    }
                }
            }
            else
            {
                mainErrorVector(currentRow,0)=(allJoints[i]->getPosition_useTempValues()-allJoints[i]->getDependencyJointOffset());
                mainMatrix(currentRow,i)=-1.0f;
                mainMatrix_correctJacobian(currentRow,i)=-1.0f;
            }
            currentRow++;
        }
    }

    if ((options&1)!=0)
    { // We take the joint weights into account here (part1):
        for (int i=0;i<mainMatrix.rows;i++)
        {
            for (int j=0;j<int(allJoints.size());j++)
            {
                float coeff=allJoints[j]->getIKWeight();
                if (coeff>=0.0f)
                    coeff=sqrtf(coeff);
                else
                    coeff=-sqrtf(-coeff);
                mainMatrix(i,j)=mainMatrix(i,j)*coeff;
                mainMatrix_correctJacobian(i,j)=mainMatrix_correctJacobian(i,j)*coeff;
            }
        }
    }

    delete _lastJacobian;
    _lastJacobian=new CMatrix(mainMatrix_correctJacobian);

    return(true);
}

float*  CIkGroup::getLastJacobianData(int matrixSize[2])
{
    const CMatrix* m=getLastJacobian();
    if (m==nullptr)
        return(nullptr);
    matrixSize[0]=m->cols;
    matrixSize[1]=m->rows;
    return(m->data);
}


float*  CIkGroup::getLastManipulabilityValue(int matrixSize[2])
{
    static float v;
    const CMatrix* m=getLastJacobian();
    if (m==nullptr)
        return(nullptr);
    matrixSize[0]=1;
    matrixSize[1]=1;

    CMatrix JT(m[0]);

    JT.transpose();
    CMatrix JJT(m[0]*JT);

    v=sqrt(_getDeterminant(JJT,nullptr,nullptr));

    return(&v);
}

float CIkGroup::_getDeterminant(const CMatrix& m,const std::vector<int>* activeRows,const std::vector<int>* activeColumns) const
{ // activeRows and activeColumns are nullptr by default (--> all rows and columns are active)
    // Routine is recursive! (i.e. Laplace expansion, which is not efficient for large matrices!)
    if (activeRows==nullptr)
    { // First call goes here:
        std::vector<int> actR;
        std::vector<int> actC;
        for (int i=0;i<m.cols;i++)
        {
            actR.push_back(i);
            actC.push_back(i);
        }
        return(_getDeterminant(m,&actR,&actC));
    }

    // If we arrived here, we have to compute the determinant of the sub-matrix obtained
    // by removing all rows and columns not listed in activeRows, respectively activeColumns

    if (activeRows->size()==2)
    { // We compute this directly, we have a two-by-two matrix:
        float retVal=0.0f;
        retVal+=m(activeRows->at(0),activeColumns->at(0))*m(activeRows->at(1),activeColumns->at(1));
        retVal-=m(activeRows->at(0),activeColumns->at(1))*m(activeRows->at(1),activeColumns->at(0));
        return(retVal);
    }

    if (activeRows->size()==3)
    { // We compute this directly, we have a three-by-three matrix:
        float retVal=0.0f;
        retVal+=m(activeRows->at(0),activeColumns->at(0)) * ( (m(activeRows->at(1),activeColumns->at(1))*m(activeRows->at(2),activeColumns->at(2))) - (m(activeRows->at(1),activeColumns->at(2))*m(activeRows->at(2),activeColumns->at(1))) );
        retVal-=m(activeRows->at(0),activeColumns->at(1)) * ( (m(activeRows->at(1),activeColumns->at(0))*m(activeRows->at(2),activeColumns->at(2))) - (m(activeRows->at(1),activeColumns->at(2))*m(activeRows->at(2),activeColumns->at(0))) );
        retVal+=m(activeRows->at(0),activeColumns->at(2)) * ( (m(activeRows->at(1),activeColumns->at(0))*m(activeRows->at(2),activeColumns->at(1))) - (m(activeRows->at(1),activeColumns->at(1))*m(activeRows->at(2),activeColumns->at(0))) );
        return(retVal);
    }

    // The general routine
    std::vector<int> actR;
    std::vector<int> actC;
    float retVal=0.0f;

    for (int colInd=1;colInd<int(activeColumns->size());colInd++)
        actC.push_back(activeColumns->at(colInd));
    for (int rowInd=0;rowInd<int(activeRows->size());rowInd++)
    {
        actR.clear();
        int i=activeRows->at(rowInd);
        for (int rowInd2=0;rowInd2<int(activeRows->size());rowInd2++)
        {
            int j=activeRows->at(rowInd2);
            if (j!=i)
                actR.push_back(j);
        }
        retVal+=m(i,activeColumns->at(0))*_getDeterminant(m,&actR,&actC)*pow(float(-1.0f),float(rowInd+2)); // was rowInd+1 until 3.1.3 rev2.
    }
    return(retVal);
}

void CIkGroup::_setEnabled_send(bool e) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setEnabled_send(e);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int flags=0;
        if (e)
            flags|=1;
        if (_ignoreMaxStepSizes)
            flags|=2;
        if (_restoreIfPositionNotReached)
            flags|=4;
        if (_restoreIfOrientationNotReached)
            flags|=8;
        CPluginContainer::ikPlugin_setIkGroupFlags(_ikPluginCounterpartHandle,flags);
    }
}

void CIkGroup::_setMaxIterations_send(int it) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setMaxIterations_send(it);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,_calculationMethod,_dampingFactor,it);
}

void CIkGroup::_setCalculationMethod_send(int m) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setCalculationMethod_send(m);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,m,_dampingFactor,_maxIterations);
}

void CIkGroup::_setDampingFactor_send(float f) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setDampingFactor_send(f);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,_calculationMethod,f,_maxIterations);
}

void CIkGroup::_setIgnoreMaxStepSizes_send(bool e) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setIgnoreMaxStepSizes_send(e);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int flags=0;
        if (_enabled)
            flags|=1;
        if (e)
            flags|=2;
        if (_restoreIfPositionNotReached)
            flags|=4;
        if (_restoreIfOrientationNotReached)
            flags|=8;
        CPluginContainer::ikPlugin_setIkGroupFlags(_ikPluginCounterpartHandle,flags);
    }
}

void CIkGroup::_setRestoreIfPositionNotReached_send(bool e) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setRestoreIfPositionNotReached_send(e);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int flags=0;
        if (_enabled)
            flags|=1;
        if (_ignoreMaxStepSizes)
            flags|=2;
        if (e)
            flags|=4;
        if (_restoreIfOrientationNotReached)
            flags|=8;
        CPluginContainer::ikPlugin_setIkGroupFlags(_ikPluginCounterpartHandle,flags);
    }
}

void CIkGroup::_setRestoreIfOrientationNotReached_send(bool e) const
{ // Overridden from _CIkGroup_
    _CIkGroup_::_setRestoreIfOrientationNotReached_send(e);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
    {
        int flags=0;
        if (_enabled)
            flags|=1;
        if (_ignoreMaxStepSizes)
            flags|=2;
        if (_restoreIfPositionNotReached)
            flags|=4;
        if (e)
            flags|=8;
        CPluginContainer::ikPlugin_setIkGroupFlags(_ikPluginCounterpartHandle,flags);
    }
}

float CIkGroup::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

void CIkGroup::resetCalculationResult()
{
    _calculationResult=sim_ikresult_not_performed;
    _calcTimeInMs=0;
}

void CIkGroup::setCalculationResult(int res,int calcTimeInMs)
{
    _calculationResult=res;
    _calcTimeInMs=calcTimeInMs;
}

int CIkGroup::getCalculationResult() const
{
    return(_calculationResult);
}

const CMatrix* CIkGroup::getLastJacobian() const
{
    return(_lastJacobian);
}

void CIkGroup::_setLastJacobian(CMatrix* j)
{
    if (_lastJacobian!=nullptr)
        delete _lastJacobian;
    _lastJacobian=j;
}

std::string CIkGroup::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CIkGroup::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_ikgroup;
        setSyncMsgRouting(parentRouting,r);

        // Build remote IK Group:
        sendVoid(sim_syncobj_ikgroup_create);

        // Build IK group in IK plugin:
        _ikPluginCounterpartHandle=CPluginContainer::ikPlugin_createIkGroup();

        // Prepare the IK elements for building their plugin counterparts:
        for (size_t i=0;i<getIkElementCount();i++)
        {
            CIkElement* it=getIkElementFromIndex(i);
            it->setIkGroupPluginCounterpartHandle(_ikPluginCounterpartHandle);
        }

        // Update the remote object:
        _setExplicitHandling_send(_explicitHandling);
        _setEnabled_send(_enabled);
        _setMaxIterations_send(_maxIterations);
        _setCalculationMethod_send(_calculationMethod);
        _setDampingFactor_send(_dampingFactor);
        _setIgnoreMaxStepSizes_send(_ignoreMaxStepSizes);
        _setRestoreIfPositionNotReached_send(_restoreIfPositionNotReached);
        _setRestoreIfOrientationNotReached_send(_restoreIfOrientationNotReached);
        // _setDoOnFailOrSuccessOf_send(); done in connectSynchronizationObject()
        _setDoOnFail_send(_doOnFail);
        _setDoOnPerformed_send(_doOnPerformed);
        _setObjectName_send(_objectName.c_str());

        // Populate remote IK group with remote IK elements:
        for (size_t i=0;i<getIkElementCount();i++)
        {
            CIkElement* it=getIkElementFromIndex(i);
            it->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
        }
    }
}

void CIkGroup::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        _setDoOnFailOrSuccessOf_send(_doOnFailOrSuccessOf);
    }
}

void CIkGroup::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote IK Group:
            sendVoid(sim_syncobj_ikgroup_delete);

            // Synchronize with IK plugin:
            if (_ikPluginCounterpartHandle!=-1)
                CPluginContainer::ikPlugin_eraseIkGroup(_ikPluginCounterpartHandle);
        }
    }
    // IK plugin part:
    _ikPluginCounterpartHandle=-1;
}
