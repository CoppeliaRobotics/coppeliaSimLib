#include "simInternal.h"
#include "distanceRoutines.h"
#include "distanceRoutines.h"
#include "ikGroup_old.h"
#include "app.h"
#include "tt.h"
#include "ttUtil.h"
#include <algorithm>
#include "base64.h"
#include "pluginContainer.h"
#include "collisionRoutines.h"

CIkGroup_old::CIkGroup_old()
{
    _objectHandle=SIM_IDSTART_IKGROUP;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _ikPluginCounterpartHandle=-1;
    _calcTimeInMs=0;
    _initialValuesInitialized=false;
    _lastJacobian=nullptr;
    _calculationResult=sim_ikresult_not_performed;
}

CIkGroup_old::~CIkGroup_old()
{
    delete _lastJacobian;
}

void CIkGroup_old::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->initializeInitialValues(simulationAlreadyRunning);
    _initialValuesInitialized=true;
    _initialExplicitHandling=_explicitHandling;
}

void CIkGroup_old::simulationAboutToStart()
{
    initializeInitialValues(false);
}

void CIkGroup_old::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->simulationEnded();
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    {
        _CIkGroup_old::setExplicitHandling(_initialExplicitHandling);
    }
    _initialValuesInitialized=false;
}

CIkGroup_old* CIkGroup_old::copyYourself() const
{
    CIkGroup_old* newGroup=new CIkGroup_old();
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
        newGroup->_CIkGroup_old::_addIkElement(getIkElementFromIndex(i)->copyYourself());

    newGroup->_initialValuesInitialized=_initialValuesInitialized;
    newGroup->_initialExplicitHandling=_initialExplicitHandling;

    return(newGroup);
}

void CIkGroup_old::serialize(CSer &ar)
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
                        ar >> byteQuantity; 
                        CIkElement_old* it=new CIkElement_old();
                        it->serialize(ar);
                        _CIkGroup_old::_addIkElement(it);
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

            ar.xmlAddNode_comment(" 'calculationMethod' tag: can be 'pseudoInverse', 'dls', 'jacobianTranspose' or 'upi' ",exhaustiveXml);
            ar.xmlAddNode_enum("calculationMethod",_calculationMethod,sim_ik_pseudo_inverse_method,"pseudoInverse",sim_ik_damped_least_squares_method,"dls",sim_ik_jacobian_transpose_method,"jacobianTranspose",sim_ik_undamped_pseudo_inverse_method,"upi");
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

            ar.xmlGetNode_enum("calculationMethod",_calculationMethod,exhaustiveXml,"pseudoInverse",sim_ik_pseudo_inverse_method,"dls",sim_ik_damped_least_squares_method,"jacobianTranspose",sim_ik_jacobian_transpose_method,"upi",sim_ik_undamped_pseudo_inverse_method);
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
                    CIkElement_old* it=new CIkElement_old();
                    it->serialize(ar);
                    _CIkGroup_old::_addIkElement(it);
                    if (!ar.xmlPushSiblingNode("ikElement",false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CIkGroup_old::performObjectLoadingMapping(const std::map<int,int>* map)
{
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->performObjectLoadingMapping(map);
}

void CIkGroup_old::performIkGroupLoadingMapping(const std::map<int,int>* map)
{
    _doOnFailOrSuccessOf=CWorld::getLoadingMapping(map,_doOnFailOrSuccessOf);
}

bool CIkGroup_old::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    for (size_t i=0;i<getIkElementCount();i++)
    {
        if (getIkElementFromIndex(i)->announceObjectWillBeErased(objID,copyBuffer))
            return(true);
    }
    return(getIkElementCount()==0);
}

bool CIkGroup_old::announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer)
{ // Return value true means that this avoidance object should be destroyed
    if (_doOnFailOrSuccessOf==ikGroupID)
    {
        _doOnFailOrSuccessOf=-1;
        if (_doOnPerformed)
            _enabled=false;
    }
    return(false);
}

int CIkGroup_old::getIkPluginCounterpartHandle() const
{
    return(_ikPluginCounterpartHandle);
}

bool CIkGroup_old::setObjectName(const char* newName,bool check)
{ // Overridden from _CIkGroup_old
    bool diff=false;
    CIkGroup_old* it=nullptr;
    if (check)
        it=App::currentWorld->ikGroups->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CIkGroup_old::setObjectName(newName,check); // no checking or object not yet in world
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->ikGroups->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm.c_str(),!tt::isHashFree(nm.c_str()));
                diff=_CIkGroup_old::setObjectName(nm.c_str(),check);
            }
        }
    }
    return(diff);
}

bool CIkGroup_old::setDoOnFailOrSuccessOf(int groupID,bool check)
{ // Overridden from _CIkGroup_old
    bool diff=false;
    CIkGroup_old* it=nullptr;
    if (check)
        it=App::currentWorld->ikGroups->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CIkGroup_old::setDoOnFailOrSuccessOf(groupID,false); // no checking or object not yet in world
    else
    { // object is in world
        for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
        {
            CIkGroup_old* prev=App::currentWorld->ikGroups->getObjectFromIndex(i);
            if (prev->getObjectHandle()==groupID)
            {
                diff=_CIkGroup_old::setDoOnFailOrSuccessOf(groupID,false);
                break;
            }
            if (prev==this)
                break;
        }
    }
    return(diff);

}

bool CIkGroup_old::setMaxIterations(int maxIter)
{ // Overridden from _CIkGroup_old
    tt::limitValue(1,400,maxIter);
    return(_CIkGroup_old::setMaxIterations(maxIter));
}

bool CIkGroup_old::setCalculationMethod(int theMethod)
{ // Overridden from _CIkGroup_old
    bool diff=false;
    if ( (theMethod==sim_ik_pseudo_inverse_method)||(theMethod==sim_ik_damped_least_squares_method)||(theMethod==sim_ik_jacobian_transpose_method)||(theMethod==sim_ik_undamped_pseudo_inverse_method) )
        diff=_CIkGroup_old::setCalculationMethod(theMethod);
    return(diff);
}

bool CIkGroup_old::setDampingFactor(float theFactor)
{ // Overridden from _CIkGroup_old
    tt::limitValue(0.000001f,10.0f,theFactor);
    return(_CIkGroup_old::setDampingFactor(theFactor));
}

bool CIkGroup_old::addIkElement(CIkElement_old* anElement)
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

void CIkGroup_old::_addIkElement(CIkElement_old* anElement)
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_addIkElement(anElement);

    anElement->setIkGroupPluginCounterpartHandle(_ikPluginCounterpartHandle);
    if (getObjectCanSync())
        anElement->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
}

void CIkGroup_old::_removeIkElement(int ikElementHandle)
{ // Overridden from _CIkGroup_old
    if (getObjectCanSync())
    {
        CIkElement_old* el=getIkElementFromHandle(ikElementHandle);
        if (el!=nullptr)
            el->removeSynchronizationObject(false);
    }

    _CIkGroup_old::_removeIkElement(ikElementHandle);
}



void CIkGroup_old::removeIkElement(int ikElementHandle)
{
    _removeIkElement(ikElementHandle);
}

void CIkGroup_old::setAllInvolvedJointsToNewJointMode(int jointMode) const
{
    for (size_t i=0;i<getIkElementCount();i++)
        getIkElementFromIndex(i)->setAllInvolvedJointsToNewJointMode(jointMode);
}

bool CIkGroup_old::computeOnlyJacobian(int options)
{
    bool retVal=false;
    retVal=CPluginContainer::ikPlugin_computeJacobian(_ikPluginCounterpartHandle,options);
    _setLastJacobian(CPluginContainer::ikPlugin_getJacobian(_ikPluginCounterpartHandle));
    return(retVal);
}

int CIkGroup_old::computeGroupIk(bool independentComputation)
{ // Return value is one of following: sim_ikresult_not_performed, sim_ikresult_success, sim_ikresult_fail
    int retVal=sim_ikresult_not_performed;
    if (_enabled)
    {
        bool doIt=true;
        if (!independentComputation)
        {
            if (_doOnFailOrSuccessOf!=-1)
            { // Conditional execution part:
                CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(_doOnFailOrSuccessOf);
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
            retVal=CPluginContainer::ikPlugin_handleIkGroup(_ikPluginCounterpartHandle);
            // do not check for success to apply values. Always apply them (the IK lib decides for that)
            for (size_t i=0;i<getIkElementCount();i++)
            {
                CIkElement_old* element=getIkElementFromIndex(i);
                element->setAllInvolvedJointsToIkPluginPositions();
            }
            if (!independentComputation)
                _setLastJacobian(CPluginContainer::ikPlugin_getJacobian(_ikPluginCounterpartHandle));
            /*
            if (_lastJacobian!=nullptr)
            {
                printf("Last Jacobian: size: %i, %i\n",_lastJacobian->rows,_lastJacobian->cols);
                for (size_t i=0;i<_lastJacobian->rows*_lastJacobian->cols;i++)
                    printf("%i: %f\n",i,_lastJacobian->data[i]);
                printf("-------------------\n");
            }
            */
        }
    }
    return(retVal);
}

const float*  CIkGroup_old::getLastJacobianData(int matrixSize[2])
{
    const CMatrix* m=getLastJacobian();
    if ( (m==nullptr)||(m->data.size()==0) )
        return(nullptr);
    matrixSize[0]=int(m->cols);
    matrixSize[1]=int(m->rows);
    return(m->data.data());
}


float*  CIkGroup_old::getLastManipulabilityValue(int matrixSize[2])
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

float CIkGroup_old::_getDeterminant(const CMatrix& m,const std::vector<int>* activeRows,const std::vector<int>* activeColumns) const
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

void CIkGroup_old::_setEnabled_send(bool e) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setEnabled_send(e);

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

void CIkGroup_old::_setMaxIterations_send(int it) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setMaxIterations_send(it);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,_calculationMethod,_dampingFactor,it);
}

void CIkGroup_old::_setCalculationMethod_send(int m) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setCalculationMethod_send(m);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,m,_dampingFactor,_maxIterations);
}

void CIkGroup_old::_setDampingFactor_send(float f) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setDampingFactor_send(f);

    // Synchronize with IK plugin:
    if (_ikPluginCounterpartHandle!=-1)
        CPluginContainer::ikPlugin_setIkGroupCalculation(_ikPluginCounterpartHandle,_calculationMethod,f,_maxIterations);
}

void CIkGroup_old::_setIgnoreMaxStepSizes_send(bool e) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setIgnoreMaxStepSizes_send(e);

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

void CIkGroup_old::_setRestoreIfPositionNotReached_send(bool e) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setRestoreIfPositionNotReached_send(e);

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

void CIkGroup_old::_setRestoreIfOrientationNotReached_send(bool e) const
{ // Overridden from _CIkGroup_old
    _CIkGroup_old::_setRestoreIfOrientationNotReached_send(e);

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

float CIkGroup_old::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

void CIkGroup_old::resetCalculationResult()
{
    _calculationResult=sim_ikresult_not_performed;
    _calcTimeInMs=0;
}

void CIkGroup_old::setCalculationResult(int res,int calcTimeInMs)
{
    _calculationResult=res;
    _calcTimeInMs=calcTimeInMs;
}

int CIkGroup_old::getCalculationResult() const
{
    return(_calculationResult);
}

const CMatrix* CIkGroup_old::getLastJacobian() const
{
    return(_lastJacobian);
}

void CIkGroup_old::_setLastJacobian(CMatrix* j)
{
    if (_lastJacobian!=nullptr)
        delete _lastJacobian;
    _lastJacobian=j;
}

std::string CIkGroup_old::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CIkGroup_old::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_ikgroup;
        setSyncMsgRouting(parentRouting,r);

        // Build IK group in IK plugin:
        _ikPluginCounterpartHandle=CPluginContainer::ikPlugin_createIkGroup();

        // Prepare the IK elements for building their plugin counterparts:
        for (size_t i=0;i<getIkElementCount();i++)
        {
            CIkElement_old* it=getIkElementFromIndex(i);
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
            CIkElement_old* it=getIkElementFromIndex(i);
            it->buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
        }
    }
}

void CIkGroup_old::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        _setDoOnFailOrSuccessOf_send(_doOnFailOrSuccessOf);
    }
}

void CIkGroup_old::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Synchronize with IK plugin:
            if (_ikPluginCounterpartHandle!=-1)
                CPluginContainer::ikPlugin_eraseIkGroup(_ikPluginCounterpartHandle);
        }
    }
    // IK plugin part:
    _ikPluginCounterpartHandle=-1;
}

int CIkGroup_old::getConfigForTipPose(int jointCnt,const int* jointHandles,float thresholdDist,int maxTimeInMs,float* retConfig,const float* metric,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,const float* lowLimits,const float* ranges,std::string& errorMsg)
{ // ret: -1: error, 0: nothing found, 1: solution found
    int retVal=-1;
    retVal=CPluginContainer::ikPlugin_getConfigForTipPose(getIkPluginCounterpartHandle(),jointCnt,jointHandles,thresholdDist,-maxTimeInMs,retConfig,metric,collisionPairCnt,collisionPairs,jointOptions,lowLimits,ranges,errorMsg);
    return(retVal);
}

int CIkGroup_old::checkIkGroup(int jointCnt,const int* jointHandles,float* jointValues,const int* jointOptions)
{ // ret: -1: object not tagger for explicit handling, -2: invalid handles, otherwise an Ik calculation result
    int retVal=sim_ikresult_not_performed;
    if (!getExplicitHandling())
        retVal=-1;

    // Check if the handles are valid:
    if (retVal>=sim_ikresult_not_performed)
    {
        for (int i=0;i<jointCnt;i++)
        {
            if (App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i])==nullptr)
                retVal=-2;
        }
    }

    if (retVal>=sim_ikresult_not_performed)
    {
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
        {
            CJoint* aj=App::currentWorld->sceneObjects->getJointFromIndex(i);
            sceneJoints.push_back(aj);
            initSceneJointValues.push_back(aj->getPosition());
            initSceneJointModes.push_back(aj->getJointMode());
        }

        setAllInvolvedJointsToNewJointMode(sim_jointmode_kinematic);
        // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
        std::vector<bool> enabledElements;
        for (size_t i=0;i<getIkElementCount();i++)
            enabledElements.push_back(getIkElementFromIndex(i)->getEnabled());

        // Set the correct mode for the joints involved:
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
            if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                joint->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_ik_deprecated);
            else
                joint->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_dependent);
        }

        // do the calculation:
        retVal=computeGroupIk(false);

        // Prepare the return array with the computed joint values:
        for (int i=0;i<jointCnt;i++)
        {
            CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
            jointValues[i]=joint->getPosition();
        }

        // Restore the IK element activation state:
        for (size_t i=0;i<getIkElementCount();i++)
            getIkElementFromIndex(i)->setEnabled(enabledElements[i]);

        // Restore joint positions/modes:
        for (size_t i=0;i<sceneJoints.size();i++)
        {
            if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                sceneJoints[i]->setPosition(initSceneJointValues[i],false);
            if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                sceneJoints[i]->setJointMode_noDynMotorTargetPosCorrection(initSceneJointModes[i]);
        }
    }
    return(retVal);
}

int CIkGroup_old::generateIkPath(int jointCnt,const int* jointHandles,int ptCnt,int collisionPairCnt,const int* collisionPairs,const int* jointOptions,std::vector<float>& path)
{ // ret: -1: invalidHandles, -2: ik Element inexistent, -3: invalid arg, -4: ik tip not linked to target, -5: invalid coll pairs, 0: failed, 1: succeeded
    int retVal=0;

    std::vector<CJoint*> joints;
    for (int i=0;i<jointCnt;i++)
    {
        CJoint* aJoint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandles[i]);
        if (aJoint==nullptr)
            retVal=-1;
        else
            joints.push_back(aJoint);
    }
    std::vector<CDummy*> tips;
    std::vector<CDummy*> targets;
    std::vector<C7Vector> startTrs;
    std::vector<C7Vector> goalTrs;
    if (retVal==0)
    {
        if (getIkElementCount()>0)
        {
            for (size_t i=0;i<getIkElementCount();i++)
            {
                CIkElement_old* ikElement=getIkElementFromIndex(i);
                CDummy* tip=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTipHandle());
                CDummy* target=App::currentWorld->sceneObjects->getDummyFromHandle(ikElement->getTargetHandle());
                if ((tip==nullptr)||(target==nullptr))
                    retVal=-4;
                tips.push_back(tip);
                targets.push_back(target);
                startTrs.push_back(tip->getFullCumulativeTransformation());
                goalTrs.push_back(target->getFullCumulativeTransformation());
            }
        }
        else
            retVal=-2;
    }
    if (retVal==0)
    {
        if (ptCnt<2)
            retVal=-3;
    }
    if ( (retVal==0)&&(collisionPairCnt>0)&&(collisionPairs!=nullptr) )
    {
        for (int i=0;i<collisionPairCnt;i++)
        {
            if (collisionPairs[2*i+0]!=-1)
            {
                if (App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+0])==nullptr)
                    retVal=-5;
                else
                {
                    if (collisionPairs[2*i+1]!=sim_handle_all)
                    {
                        if (App::currentWorld->collections->getObjectFromHandle(collisionPairs[2*i+1])==nullptr)
                            retVal=-5;
                    }
                }
            }
        }
    }
    if (retVal==0)
    {
        // Save joint positions/modes (all of them, just in case)
        std::vector<CJoint*> sceneJoints;
        std::vector<float> initSceneJointValues;
        std::vector<int> initSceneJointModes;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
        {
            CJoint* aj=App::currentWorld->sceneObjects->getJointFromIndex(i);
            sceneJoints.push_back(aj);
            initSceneJointValues.push_back(aj->getPosition());
            initSceneJointModes.push_back(aj->getJointMode());
        }

        setAllInvolvedJointsToNewJointMode(sim_jointmode_kinematic);

        bool ikGroupWasActive=getEnabled();
        if (!ikGroupWasActive)
            setEnabled(true);

        // It can happen that some IK elements get deactivated when the user provided wrong handles, so save the activation state:
        std::vector<bool> enabledElements;
        for (size_t i=0;i<getIkElementCount();i++)
        {
            CIkElement_old* ikElement=getIkElementFromIndex(i);
            enabledElements.push_back(ikElement->getEnabled());
        }

        // Set the correct mode for the joints involved:
        for (int i=0;i<jointCnt;i++)
        {
            if ( (jointOptions==nullptr)||((jointOptions[i]&1)==0) )
                joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_ik_deprecated);
            else
                joints[i]->setJointMode_noDynMotorTargetPosCorrection(sim_jointmode_dependent);
        }

        // do the calculation:
        float t=0.0;
        float dt=1.0/(ptCnt-1);
        bool failed=false;
        std::vector<float> thePath;
        for (int iterCnt=0;iterCnt<ptCnt;iterCnt++)
        {
            for (size_t el=0;el<getIkElementCount();el++)
            { // set all targets to an interpolated pose
                C7Vector tr;
                tr.buildInterpolation(startTrs[el],goalTrs[el],t);
                targets[el]->setAbsoluteTransformation(tr);
            }

            // Try to perform IK:
            if (sim_ikresult_success==computeGroupIk(true))
            {
                bool colliding=false;
                if ( (collisionPairCnt>0)&&(collisionPairs!=nullptr) )
                { // we need to check if this state collides:
                    for (int i=0;i<collisionPairCnt;i++)
                    {
                        if (collisionPairs[2*i+0]>=0)
                        {
                            int env=collisionPairs[2*i+1];
                            if (env==sim_handle_all)
                                env=-1;
                            if (CCollisionRoutine::doEntitiesCollide(collisionPairs[2*i+0],env,nullptr,false,false,nullptr))
                            {
                                colliding=true;
                                break;
                            }
                        }
                    }
                }
                if (!colliding)
                { // we save this path point
                    for (int i=0;i<jointCnt;i++)
                        thePath.push_back(joints[i]->getPosition());
                }
                else
                    failed=true;
            }
            else
                failed=true;
            if (failed)
                break;
            t+=dt;
        }

        if (!ikGroupWasActive)
            setEnabled(false);

        // Restore the IK element activation state:
        for (size_t i=0;i<getIkElementCount();i++)
        {
            CIkElement_old* ikElement=getIkElementFromIndex(i);
            ikElement->setEnabled(enabledElements[i]);
        }

        // Restore joint positions/modes:
        for (size_t i=0;i<sceneJoints.size();i++)
        {
            if (sceneJoints[i]->getPosition()!=initSceneJointValues[i])
                sceneJoints[i]->setPosition(initSceneJointValues[i],false);
            if (sceneJoints[i]->getJointMode()!=initSceneJointModes[i])
                sceneJoints[i]->setJointMode_noDynMotorTargetPosCorrection(initSceneJointModes[i]);
        }

        // Restore target dummies:
        for (size_t el=0;el<getIkElementCount();el++)
            targets[el]->setAbsoluteTransformation(goalTrs[el]);

        if (!failed)
        {
            retVal=1;
            path.assign(thePath.begin(),thePath.end());
        }
        else
            retVal=0;
    }
    return(retVal);
}


