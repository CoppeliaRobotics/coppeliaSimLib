#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "distanceRoutine.h"
#include "distanceRoutine.h"
#include "ikGroup.h"
#include "ikRoutine.h"
#include "app.h"
#include "tt.h"
#include "ttUtil.h"
#include <algorithm>

CikGroup::CikGroup()
{
    objectID=SIM_IDSTART_IKGROUP;
    maxIterations=3;
    active=true;
    ignoreMaxStepSizes=true;
    _avoidanceRobotEntity=-1;
    _avoidanceObstacleEntity=-1;
    avoidanceThreshold=0.01f;
    avoidanceWeight=1.0f;
    _calcTimeInMs=0;
    _initialValuesInitialized=false;
    _lastJacobian=nullptr;
    _explicitHandling=false;
    dlsFactor=0.1f;
    calculationMethod=sim_ik_pseudo_inverse_method;
    restoreIfPositionNotReached=false;
    restoreIfOrientationNotReached=false;
    doOnFailOrSuccessOf=-1;
    doOnFail=true;
    doOnPerformed=true;
    constraints=0;
    jointLimitWeight=1.0f;
    jointTreshholdAngular=2.0f*degToRad_f;
    jointTreshholdLinear=0.001f;
    _calculationResult=sim_ikresult_not_performed;
    _correctJointLimits=false;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
}

std::string CikGroup::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

CikGroup::~CikGroup()
{
    while (ikElements.size()!=0)
        removeIkElement(ikElements[0]->getObjectID());

    delete _lastJacobian;
}

void CikGroup::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->initializeInitialValues(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
}

void CikGroup::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CikGroup::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->simulationEnded();
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        _explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
}

float CikGroup::getCalculationTime()
{
    return(float(_calcTimeInMs)*0.001f);
}

void CikGroup::setAvoidanceWeight(float weight)
{
    tt::limitValue(0.001f,1.0f,weight);
    avoidanceWeight=weight;
}
float CikGroup::getAvoidanceWeight()
{
    return(avoidanceWeight);
}

void CikGroup::setJointTreshholdAngular(float treshhold)
{
    tt::limitValue(0.1f*degToRad_f,10.0f*degToRad_f,treshhold);
    jointTreshholdAngular=treshhold;
}

void CikGroup::setJointTreshholdLinear(float treshhold)
{
    tt::limitValue(0.0001f,1.0,treshhold);
    jointTreshholdLinear=treshhold;
}

void CikGroup::setRestoreIfPositionNotReached(bool active)
{
    restoreIfPositionNotReached=active;
}
void CikGroup::setRestoreIfOrientationNotReached(bool active)
{
    restoreIfOrientationNotReached=active;
}
void CikGroup::setDoOnFailOrSuccessOf(int groupID)
{ // set groupID -1 if you wanna disable this functionality
    doOnFailOrSuccessOf=groupID;
}
void CikGroup::setDoOnFail(bool onFail)
{
    doOnFail=onFail;
}
void CikGroup::setDoOnPerformed(bool turnOn)
{
    doOnPerformed=turnOn;
}

bool CikGroup::addIkElement(CikEl* anElement)
{ // If return value if false, the calling function has to destroy anElement (invalid)
    // We check if anElement is valid:
    if (App::ct->objCont->getDummy(anElement->getTooltip())==nullptr)
        return(false); // invalid
    
    // SINCE 14/12/2011 we are authorized to add several times an IK element with the same tip dummy!!

    // We look for a free id:
    int newID=0;
    while (getIkElement(newID)!=nullptr)
        newID++;
    anElement->setObjectID(newID);
    ikElements.push_back(anElement);
    return(true);
}

void CikGroup::setObjectID(int newID)
{
    objectID=newID;
}

void CikGroup::performIkGroupLoadingMapping(std::vector<int>* map)
{
    doOnFailOrSuccessOf=App::ct->objCont->getLoadingMapping(map,doOnFailOrSuccessOf);
}

void CikGroup::performCollectionLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->performCollectionLoadingMapping(map);
    if (_avoidanceRobotEntity>=SIM_IDSTART_COLLECTION)
        _avoidanceRobotEntity=App::ct->objCont->getLoadingMapping(map,_avoidanceRobotEntity);
    if (_avoidanceObstacleEntity>=SIM_IDSTART_COLLECTION)
        _avoidanceObstacleEntity=App::ct->objCont->getLoadingMapping(map,_avoidanceObstacleEntity);
}

bool CikGroup::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if ( (_avoidanceRobotEntity>=SIM_IDSTART_COLLECTION)&&(_avoidanceRobotEntity==groupID) )
        _avoidanceRobotEntity=-1;
    if ( (_avoidanceObstacleEntity>=SIM_IDSTART_COLLECTION)&&(_avoidanceObstacleEntity==groupID) )
        _avoidanceObstacleEntity=-1;
    int i=0;
    while (i<int(ikElements.size()))
    {
        if (ikElements[i]->announceCollectionWillBeErased(groupID))
        {
            removeIkElement(ikElements[i]->getObjectID());
            i=0; // ordering may have changed
        }
        else
            i++;
    }
    return(false);
}

void CikGroup::setAvoidanceRobot(int universalObject)
{
    _avoidanceRobotEntity=universalObject;
}
int CikGroup::getAvoidanceRobot()
{
    return(_avoidanceRobotEntity);
}
void CikGroup::setAvoidanceObstacle(int universalObject)
{
    _avoidanceObstacleEntity=universalObject;
}
int CikGroup::getAvoidanceObstacle()
{
    return(_avoidanceObstacleEntity);
}
void CikGroup::setAvoidanceThreshold(float t)
{
    tt::limitValue(0.001f,1.0,t);
    avoidanceThreshold=t;
}
float CikGroup::getAvoidanceThreshold()
{
    return(avoidanceThreshold);
}
void CikGroup::setIgnoreMaxStepSizes(bool ignore)
{
    ignoreMaxStepSizes=ignore;
}

CikGroup* CikGroup::copyYourself()
{
    CikGroup* newGroup=new CikGroup();
    newGroup->_avoidanceRobotEntity=_avoidanceRobotEntity;
    newGroup->_avoidanceObstacleEntity=_avoidanceObstacleEntity;
    newGroup->avoidanceThreshold=avoidanceThreshold;
    newGroup->ignoreMaxStepSizes=ignoreMaxStepSizes;
    newGroup->objectID=objectID;
    newGroup->objectName=objectName;
    newGroup->maxIterations=maxIterations;
    newGroup->active=active;
    newGroup->dlsFactor=dlsFactor;
    newGroup->calculationMethod=calculationMethod;
    newGroup->restoreIfPositionNotReached=restoreIfPositionNotReached;
    newGroup->restoreIfOrientationNotReached=restoreIfOrientationNotReached;
    newGroup->doOnFailOrSuccessOf=doOnFailOrSuccessOf;
    newGroup->doOnFail=doOnFail;
    newGroup->doOnPerformed=doOnPerformed;
    newGroup->_correctJointLimits=_correctJointLimits;
    newGroup->constraints=constraints;
    newGroup->jointLimitWeight=jointLimitWeight;
    newGroup->avoidanceWeight=avoidanceWeight;
    newGroup->jointTreshholdAngular=jointTreshholdAngular;
    newGroup->jointTreshholdLinear=jointTreshholdLinear;
    newGroup->_explicitHandling=_explicitHandling;
    newGroup->_calculationResult=_calculationResult;
    newGroup->_calcTimeInMs=0;
    for (int i=0;i<int(ikElements.size());i++)
        newGroup->ikElements.push_back(ikElements[i]->copyYourself());

    newGroup->_initialValuesInitialized=_initialValuesInitialized;
    newGroup->_initialExplicitHandling=_initialExplicitHandling;

    return(newGroup);
}

void CikGroup::serialize(CSer &ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Oid");
            ar << objectID;
            ar.flush();

            ar.storeDataName("Nme");
            ar << objectName;
            ar.flush();

            ar.storeDataName("Mit");
            ar << maxIterations;
            ar.flush();

            ar.storeDataName("Ctr");
            ar << constraints;
            ar.flush();

            ar.storeDataName("Wgt");
            ar << jointLimitWeight << avoidanceWeight;
            ar.flush();

            ar.storeDataName("Jts");
            ar << jointTreshholdAngular << jointTreshholdLinear;
            ar.flush();

            ar.storeDataName("Dpg");
            ar << dlsFactor;
            ar.flush();

            ar.storeDataName("Cmt");
            ar << calculationMethod;
            ar.flush();

            ar.storeDataName("Doo");
            ar << doOnFailOrSuccessOf;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,active);
            SIM_SET_CLEAR_BIT(nothing,1,restoreIfPositionNotReached);
            SIM_SET_CLEAR_BIT(nothing,2,restoreIfOrientationNotReached);
            SIM_SET_CLEAR_BIT(nothing,3,doOnFail);
            SIM_SET_CLEAR_BIT(nothing,4,doOnPerformed);
            SIM_SET_CLEAR_BIT(nothing,5,!ignoreMaxStepSizes);
            SIM_SET_CLEAR_BIT(nothing,6,_explicitHandling);
            // 14/12/2011       SIM_SET_CLEAR_BIT(nothing,7,_avoidanceCheckAgainstAll); DO NOT USE FOR BACKWARD COMPATIBILITY'S SAKE!
            ar << nothing;
            ar.flush();

            ar.storeDataName("Va2");
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_correctJointLimits);
            ar << nothing;
            ar.flush();


            ar.storeDataName("Avx");
            ar << _avoidanceRobotEntity << _avoidanceObstacleEntity;
            ar.flush();

            ar.storeDataName("Avt");
            ar << avoidanceThreshold;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            for (int i=0;i<int(ikElements.size());i++)
            {
                ar.storeDataName("Ike");
                ar.setCountingMode();
                ikElements[i]->serialize(ar);
                if (ar.setWritingMode())
                    ikElements[i]->serialize(ar);
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
                        ar >> objectID;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> objectName;
                    }
                    if (theName.compare("Mit")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> maxIterations;
                    }
                    if (theName.compare("Ctr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> constraints;
                    }
                    if (theName.compare("Wgt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> jointLimitWeight >> avoidanceWeight;
                    }
                    if (theName.compare("Jts")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> jointTreshholdAngular >> jointTreshholdLinear;
                    }
                    if (theName.compare("Dpg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> dlsFactor;
                    }
                    if (theName.compare("Cmt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> calculationMethod;
                    }
                    if (theName.compare("Doo")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> doOnFailOrSuccessOf;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        active=SIM_IS_BIT_SET(nothing,0);
                        restoreIfPositionNotReached=SIM_IS_BIT_SET(nothing,1);
                        restoreIfOrientationNotReached=SIM_IS_BIT_SET(nothing,2);
                        doOnFail=SIM_IS_BIT_SET(nothing,3);
                        doOnPerformed=SIM_IS_BIT_SET(nothing,4);
                        ignoreMaxStepSizes=!SIM_IS_BIT_SET(nothing,5);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,6);
                    }
                    if (theName.compare("Va2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _correctJointLimits=SIM_IS_BIT_SET(nothing,0);
                    }
                    if (theName.compare("Avx")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _avoidanceRobotEntity >> _avoidanceObstacleEntity;
                        // Following for backw. compatibility (probably around 2012):
                        if ((_avoidanceRobotEntity==-1)&&(constraints&sim_ik_avoidance_constraint))
                            constraints-=sim_ik_avoidance_constraint;
                    }
                    if (theName.compare("Avt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> avoidanceThreshold;
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
                        CikEl* it=new CikEl();
                        it->serialize(ar);
                        ikElements.push_back(it);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CikGroup::serializeWExtIk(CExtIkSer &ar)
{
    ar.writeInt(objectID);

    ar.writeString(objectName.c_str());

    ar.writeInt(maxIterations);

    ar.writeInt(constraints);
    
    ar.writeFloat(jointLimitWeight);

    ar.writeFloat(jointTreshholdAngular);
    ar.writeFloat(jointTreshholdLinear);

    ar.writeFloat(dlsFactor);

    ar.writeInt(calculationMethod);

    ar.writeInt(doOnFailOrSuccessOf);

    unsigned char nothing=0;
    SIM_SET_CLEAR_BIT(nothing,0,active);
    SIM_SET_CLEAR_BIT(nothing,1,restoreIfPositionNotReached);
    SIM_SET_CLEAR_BIT(nothing,2,restoreIfOrientationNotReached);
    SIM_SET_CLEAR_BIT(nothing,3,doOnFail);
    SIM_SET_CLEAR_BIT(nothing,4,doOnPerformed);
    SIM_SET_CLEAR_BIT(nothing,5,!ignoreMaxStepSizes);
    SIM_SET_CLEAR_BIT(nothing,6,_explicitHandling);
    ar.writeByte(nothing);

    nothing=0;
    SIM_SET_CLEAR_BIT(nothing,0,_correctJointLimits);
    ar.writeByte(nothing);

    ar.writeInt(int(ikElements.size()));
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->serializeWExtIk(ar);
}

void CikGroup::getCollisionPartners(std::vector<C3DObject*>* partners,
                                    std::vector<float>* treshholds,
                                    CJoint* bottomJoint,
                                    std::vector<CJoint*>* allGroupJoints)
{ // Will return registered avoidance partners. Partners are stored in the returned
    // vector in the following way: partner1A,partner1B,partner2A,partner2B,partner3A, etc.
    // A partners are built on top of bottomJoint and bottomJoint is their next joint parent listed
    // in allGroupJoints.
    // B partners are never built on top of bottomJoint.
    // The returned partners have their measurable flag set (or it was overridden!!)

    std::vector<C3DObject*> gr1;
    gr1.reserve(16);
    gr1.clear();
    std::vector<C3DObject*> gr2;
    gr2.reserve(16);
    gr2.clear();
    if (_avoidanceRobotEntity<SIM_IDSTART_COLLECTION)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(_avoidanceRobotEntity);
        if ( (it!=nullptr)&&(it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable) ) // we check the measurable property here (special!!)
            gr1.push_back(it);
    }
    else
        App::ct->collections->getShapesAndDummiesFromCollection(_avoidanceRobotEntity,&gr1,sim_objectspecialproperty_measurable,false);
    if (_avoidanceObstacleEntity<SIM_IDSTART_COLLECTION)
    {
        if (_avoidanceObstacleEntity!=-1)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(_avoidanceObstacleEntity);
            if (it!=nullptr)
                gr2.push_back(it);
        }
        else
        { // Here we need to get all objects that are measurable but not contained in gr1!
            App::ct->objCont->getAllShapesAndDummiesFromSceneExcept(gr1,gr2,sim_objectspecialproperty_measurable);
        }
    }
    else
        App::ct->collections->getShapesAndDummiesFromCollection(_avoidanceObstacleEntity,&gr2,sim_objectspecialproperty_measurable,false);


    // Now we prepare 2 vectors
    std::vector<C3DObject*> aux1;
    aux1.reserve(SIM_MAX(gr1.size(),gr2.size()));
    aux1.clear();
    std::vector<C3DObject*> aux2;
    aux2.reserve(SIM_MAX(gr1.size(),gr2.size()));
    aux2.clear();
    for (int pass=0;pass<2;pass++)
    {
        for (int i=0;i<int(gr1.size());i++)
        {
            C3DObject* grObj=gr1[i];
            // Now we check that this object has only bottomJoint as listed joint parent:
            C3DObject* iterat=grObj->getParentObject();
            while (iterat!=nullptr)
            {
                if (iterat==bottomJoint)
                { // We can add this element (dummy or shape)
                    aux1.push_back(grObj);
                    break;
                }
                if (iterat->getObjectType()==sim_object_joint_type)
                {   // If iterat is listed in allGroupJoints, we have to break:
                    bool listed=false;
                    for (int j=0;j<int(allGroupJoints->size());j++)
                    {
                        if (allGroupJoints->at(j)==iterat)
                        {
                            listed=true;
                            break;
                        }
                    }
                    if (listed)
                        break;
                }
                iterat=iterat->getParentObject();
            }
        }
        for (int i=0;i<int(gr2.size());i++)
        {
            C3DObject* grObj=gr2[i];
            // Now we check that this object is not parented with bottomJoint:
            if (!grObj->isObjectParentedWith(bottomJoint))
                aux2.push_back(grObj);
        }
        for (int i=0;i<int(aux1.size());i++)
        {
            C3DObject* object=aux1[i];
            for (int j=0;j<int(aux2.size());j++)
            {
                C3DObject* obstacle=aux2[j];
                if (object!=obstacle)
                {
                    // Now we have to add the partners object-obstacle only if not
                    // already present:
                    bool present=false;
                    int k;
                    for (k=0;k<int(partners->size())/2;k++)
                    {
                        if ( (partners->at(2*k+0)==object)&&(partners->at(2*k+1)==obstacle) )
                        {
                            present=true;
                            break;
                        }
                    }
                    if (!present)
                    {
                        partners->push_back(object);
                        partners->push_back(obstacle);
                        treshholds->push_back(avoidanceThreshold);
                    }
                    else
                    { // Already present! We have to set the treshhold to the minimum:
                        if (treshholds->at(k)>avoidanceThreshold)
                            treshholds->at(k)=avoidanceThreshold;
                    }
                }
            }
        }
        aux1.clear();
        aux2.clear();
        std::swap(gr1,gr2);
    }
}

void CikGroup::performObjectLoadingMapping(std::vector<int>* map)
{
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->performObjectLoadingMapping(map);

    if (_avoidanceRobotEntity<SIM_IDSTART_COLLECTION)
        _avoidanceRobotEntity=App::ct->objCont->getLoadingMapping(map,_avoidanceRobotEntity);
    if (_avoidanceObstacleEntity<SIM_IDSTART_COLLECTION)
        _avoidanceObstacleEntity=App::ct->objCont->getLoadingMapping(map,_avoidanceObstacleEntity);
}

void CikGroup::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CikGroup::getExplicitHandling()
{
    return(_explicitHandling);
}

void CikGroup::setAllInvolvedJointsToPassiveMode()
{
    for (int i=0;i<int(ikElements.size());i++)
        ikElements[i]->setAllInvolvedJointsToPassiveMode();
}

void CikGroup::setObjectName(std::string newName)
{
    objectName=newName;
}

void CikGroup::setActive(bool isActive)
{
    active=isActive;
}

void CikGroup::resetCalculationResult()
{
    _calculationResult=sim_ikresult_not_performed;
    _calcTimeInMs=0;
}

void CikGroup::setCorrectJointLimits(bool c)
{
    _correctJointLimits=c;
}

bool CikGroup::getCorrectJointLimits()
{
    return(_correctJointLimits);
}

void CikGroup::setCalculationResult(int res,int calcTimeInMs)
{
    _calculationResult=res;
    _calcTimeInMs=calcTimeInMs;
}

int CikGroup::getCalculationResult()
{
    return(_calculationResult);
}

void CikGroup::setJointLimitWeight(float weight)
{
    tt::limitValue(0.001f,1.0f,weight);
    jointLimitWeight=weight;
}
float CikGroup::getJointLimitWeight()
{
    return(jointLimitWeight);
}

float CikGroup::getJointTreshholdAngular()
{
    return(jointTreshholdAngular);
}
float CikGroup::getJointTreshholdLinear()
{
    return(jointTreshholdLinear);
}

void CikGroup::setConstraints(int constr)
{
    constraints=constr;
}

int CikGroup::getConstraints()
{
    return(constraints);
}

void CikGroup::setMaxIterations(int maxIter)
{
    tt::limitValue(1,400,maxIter);
    maxIterations=maxIter;
}
int CikGroup::getMaxIterations()
{
    return(maxIterations);
}

bool CikGroup::getActive()
{
    return(active);
}
void CikGroup::setDlsFactor(float theFactor)
{
    tt::limitValue(0.000001f,10.0f,theFactor);
    dlsFactor=theFactor;
}
float CikGroup::getDlsFactor()
{
    return(dlsFactor);
}
void CikGroup::setCalculationMethod(int theMethod)
{
    if ( (theMethod==sim_ik_pseudo_inverse_method)||(theMethod==sim_ik_damped_least_squares_method)||
        (theMethod==sim_ik_jacobian_transpose_method) )
    {
        calculationMethod=theMethod;
    }
}
int CikGroup::getCalculationMethod()
{
    return(calculationMethod);
}

bool CikGroup::getRestoreIfPositionNotReached()
{
    return(restoreIfPositionNotReached);
}
bool CikGroup::getRestoreIfOrientationNotReached()
{
    return(restoreIfOrientationNotReached);
}
int CikGroup::getDoOnFailOrSuccessOf()
{
    return(doOnFailOrSuccessOf);
}
bool CikGroup::getDoOnFail()
{
    return(doOnFail);
}
bool CikGroup::getDoOnPerformed()
{
    return(doOnPerformed);
}
void CikGroup::removeIkElement(int elementID)
{
    for (int i=0;i<int(ikElements.size());i++)
    {
        if (ikElements[i]->getObjectID()==elementID)
        {

            delete ikElements[i];
            ikElements.erase(ikElements.begin()+i);
            break;
        }
    }
}

CikEl* CikGroup::getIkElement(int ikElementID)
{
    for (int i=0;i<int(ikElements.size());i++)
    {
        if (ikElements[i]->getObjectID()==ikElementID)
            return(ikElements[i]);
    }
    return(nullptr);
}

CikEl* CikGroup::getIkElementWithTooltipID(int tooltipID)
{ 
    if (tooltipID==-1)
        return(nullptr);
    for (int i=0;i<int(ikElements.size());i++)
    {
        if (ikElements[i]->getTooltip()==tooltipID)
            return(ikElements[i]);
    }
    return(nullptr);
}

int CikGroup::getObjectID()
{
    return(objectID);
}

std::string CikGroup::getObjectName()
{
    return(objectName);
}

bool CikGroup::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this object should be destroyed
    if ( (_avoidanceRobotEntity<SIM_IDSTART_COLLECTION)&&(_avoidanceRobotEntity==objID) )
        _avoidanceRobotEntity=-1;
    if ( (_avoidanceObstacleEntity<SIM_IDSTART_COLLECTION)&&(_avoidanceObstacleEntity==objID) )
        _avoidanceObstacleEntity=-1;

    for (int i=0;i<int(ikElements.size());i++)
    {
        if (ikElements[i]->announceObjectWillBeErased(objID,copyBuffer))
            return(true);
    }
    return(ikElements.size()==0);
}

bool CikGroup::announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer)
{ // Return value true means that this avoidance object should be destroyed
    if (doOnFailOrSuccessOf==ikGroupID)
    {
        doOnFailOrSuccessOf=-1;
        active=false;
    }
    return(false);
}

bool CikGroup::getIgnoreMaxStepSizes()
{
    return(ignoreMaxStepSizes);
}

void CikGroup::getAllActiveJoints(std::vector<CJoint*>& jointList)
{ // Retrieves all active joints in this group. Ordering is random!
    for (int elementNumber=0;elementNumber<int(ikElements.size());elementNumber++)
    {
        CikEl* element=ikElements[elementNumber];
        CDummy* tooltip=App::ct->objCont->getDummy(element->getTooltip());
        C3DObject* base=App::ct->objCont->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getActive())
            valid=false;
        if (tooltip==nullptr)
            valid=false;
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            C3DObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParentObject();
                if (iterat==base)
                {
                    baseOk=true;
                    if (jointPresent)
                        valid=true;
                }
                if ( (iterat!=base)&&(iterat!=nullptr)&&(iterat->getObjectType()==sim_object_joint_type) )
                    jointPresent=true;
            }
            if (!valid)
            {
                // 28/4/2016 element->setActive(false); // This element has an error
                if (!baseOk)
                    element->setBase(-1); // The base was illegal!
            }
        }
        if (valid)
        { // We add all joint between tooltip and base which are not yet present:
            C3DObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                if (iterat->getObjectType()==sim_object_joint_type)
                {
                    if (std::find(jointList.begin(),jointList.end(),iterat)==jointList.end())
                        jointList.push_back((CJoint*)iterat);
                }
                iterat=iterat->getParentObject();
            }
        }
    }
}

void CikGroup::getTipAndTargetLists(std::vector<CDummy*>& tipList,std::vector<CDummy*>& targetList)
{
    for (int elementNumber=0;elementNumber<int(ikElements.size());elementNumber++)
    {
        CikEl* element=ikElements[elementNumber];
        CDummy* tooltip=App::ct->objCont->getDummy(element->getTooltip());
        CDummy* target=App::ct->objCont->getDummy(element->getTarget());
        C3DObject* base=App::ct->objCont->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getActive())
            valid=false;
        if (tooltip==nullptr)
            valid=false;
        if (target==nullptr)
            valid=false;
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            C3DObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParentObject();
                if (iterat==base)
                {
                    baseOk=true;
                    if (jointPresent)
                        valid=true;
                }
                if ( (iterat!=base)&&(iterat!=nullptr)&&(iterat->getObjectType()==sim_object_joint_type) )
                    jointPresent=true;
            }
            if (!valid)
            {
                // 28/4/2016 element->setActive(false); // This element has an error
                if (!baseOk)
                    element->setBase(-1); // The base was illegal!
            }
        }
        if (valid)
        { 
            tipList.push_back(tooltip);
            targetList.push_back(target);
        }
    }
}

bool CikGroup::computeOnlyJacobian(int options)
{
    // Now we prepare a vector with all valid and active elements:
    std::vector<CikEl*> validElements;
    validElements.reserve(ikElements.size());
    validElements.clear();

    for (int elementNumber=0;elementNumber<int(ikElements.size());elementNumber++)
    {
        CikEl* element=ikElements[elementNumber];
        CDummy* tooltip=App::ct->objCont->getDummy(element->getTooltip());
        C3DObject* base=App::ct->objCont->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getActive())
            valid=false;
        if (tooltip==nullptr)
            valid=false; // should normally never happen!
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            C3DObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParentObject();
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
        CikEl* element=validElements[elementNumber];
        element->prepareIkEquations(1.0);
    }
    return(performOnePass_jacobianOnly(&validElements,options));
}


int CikGroup::computeGroupIk(bool forMotionPlanning)
{ // Return value is one of following: sim_ikresult_not_performed, sim_ikresult_success, sim_ikresult_fail
    if (!active)
        return(sim_ikresult_not_performed); // That group is not active!
    if (!forMotionPlanning)
    {
        if (doOnFailOrSuccessOf!=-1)
        { // Conditional execution part:
            CikGroup* it=App::ct->ikGroups->getIkGroup(doOnFailOrSuccessOf);
            if (it!=nullptr)
            {
                if (doOnPerformed)
                {
                    if (it->getCalculationResult()==sim_ikresult_not_performed)
                        return(sim_ikresult_not_performed);
                    if (it->getCalculationResult()==sim_ikresult_success)
                    {
                        if (doOnFail)
                            return(sim_ikresult_not_performed);
                    }
                    else
                    {
                        if (!doOnFail)
                            return(sim_ikresult_not_performed);
                    }
                }
                else
                {
                    if (it->getCalculationResult()!=sim_ikresult_not_performed)
                        return(sim_ikresult_not_performed);
                }
            }
        }
    }

    if (!forMotionPlanning)
        App::ct->calcInfo->inverseKinematicsStart();
    // Now we prepare a vector with all valid and active elements:
    std::vector<CikEl*> validElements;
    validElements.reserve(ikElements.size());
    validElements.clear();

    for (int elementNumber=0;elementNumber<int(ikElements.size());elementNumber++)
    {
        CikEl* element=ikElements[elementNumber];
        CDummy* tooltip=App::ct->objCont->getDummy(element->getTooltip());
        C3DObject* base=App::ct->objCont->getObjectFromHandle(element->getBase());
        bool valid=true;
        if (!element->getActive())
            valid=false;
        if (tooltip==nullptr)
            valid=false; // should normally never happen!
        // We check that tooltip is parented with base and has at least one joint in-between:
        if (valid)
        {
            valid=false;
            bool jointPresent=false;
            bool baseOk=false;
            C3DObject* iterat=tooltip;
            while ( (iterat!=base)&&(iterat!=nullptr) )
            {
                iterat=iterat->getParentObject();
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
    {
        if (!forMotionPlanning)
            App::ct->calcInfo->inverseKinematicsEnd();
        return(sim_ikresult_fail); // Error!
    }

    _resetTemporaryParameters();

    // Here we have the main iteration loop:
    float interpolFact=1.0f; // We first try to solve in one step
    int successNumber=0;
    bool limitOrAvoidanceNeedMoreCalculation;
    bool leaveNow=false;
    bool errorOccured=false;
    for (int iterationNb=0;iterationNb<maxIterations;iterationNb++)
    {
        // Here we prepare all element equations:
        for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
        {
            CikEl* element=validElements[elementNumber];
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
                CikEl* element=validElements[elementNumber];
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
            CikEl* element=validElements[elementNumber];
            element->removeIkEquations();
        }
        if (leaveNow)
            break;
    }
    int returnValue=sim_ikresult_success;
    if (errorOccured)
        returnValue=sim_ikresult_fail;
    bool setNewValues=(!errorOccured);
    for (int elementNumber=0;elementNumber<int(validElements.size());elementNumber++)
    {
        CikEl* element=validElements[elementNumber];
        bool posit,orient;
        element->checkIfWithinTolerance(posit,orient,true);
        if ( (!posit)||(!orient) )
        {
            returnValue=sim_ikresult_fail;
            if ( (restoreIfPositionNotReached&&(!posit))||
                (restoreIfOrientationNotReached&&(!orient)) )
                setNewValues=false;
        }
    }

    // We set all joint parameters:
    if (setNewValues)
        _applyTemporaryParameters();
    if (!forMotionPlanning)
        App::ct->calcInfo->inverseKinematicsEnd();
    return(returnValue);
}

void CikGroup::_resetTemporaryParameters()
{
    // We prepare all joint temporary parameters:
    for (int jc=0;jc<int(App::ct->objCont->jointList.size());jc++)
    {
        CJoint* it=App::ct->objCont->getJoint(App::ct->objCont->jointList[jc]);

        it->setPosition(it->getPosition(),true);
        it->initializeParametersForIK(getJointTreshholdAngular());
    }
    // We prepare all dummy temporary parameters (needed for rail-type mechanisms):
    for (int jc=0;jc<int(App::ct->objCont->dummyList.size());jc++)
    { // It doesn't matter if the dummy is free or not or if its parent is a path. All parameters must be set
        CDummy* it=App::ct->objCont->getDummy(App::ct->objCont->dummyList[jc]);
        it->setTempLocalTransformation(it->getLocalTransformation());
    }
}

void CikGroup::_applyTemporaryParameters()
{
    // Joints:
    for (int jc=0;jc<int(App::ct->objCont->jointList.size());jc++)
    {
        CJoint* it=App::ct->objCont->getJoint(App::ct->objCont->jointList[jc]);

        it->setPosition(it->getPosition(true),false);
        it->applyTempParametersEx();
    }
    // Dummies: (for rail-type mechanisms)
    for (int jc=0;jc<int(App::ct->objCont->dummyList.size());jc++)
    {
        CDummy* it=App::ct->objCont->getDummy(App::ct->objCont->dummyList[jc]);
        it->setLocalTransformation(it->getTempLocalTransformation());
    }
}

int CikGroup::performOnePass(std::vector<CikEl*>* validElements,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,bool forMotionPlanning)
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
        CikEl* element=validElements->at(elementNumber);
        numberOfRows=numberOfRows+element->matrix->rows;
        for (int i=0;i<int(element->rowJointIDs->size());i++)
        {
            int current=element->rowJointIDs->at(i);
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
                allJoints.push_back(App::ct->objCont->getJoint(current));
                allJointStages.push_back(currentStage);
            }
        }
    }

    // Now we prepare the obstacle avoidance part:
    //---------------------------------------------------------------------------
    std::vector<float> avoidanceError;
    std::vector<CMatrix*> avoidanceMatrix;
    std::vector<std::vector<int>*> avoidanceRowIDs;

    if ( (constraints&sim_ik_avoidance_constraint)&&(_avoidanceRobotEntity!=-1) )
    {
        std::vector<std::vector<C3DObject*>*> collPartners;
        std::vector<std::vector<float>*> collTreshholds;
        for (int i=0;i<int(allJoints.size());i++)
        {
            CJoint* it=allJoints[i];
            std::vector<C3DObject*>* cp=new std::vector<C3DObject*>;
            cp->reserve(16);
            cp->clear();
            std::vector<float>* ct=new std::vector<float>;
            ct->reserve(8);
            ct->clear();
            // Following routine returns only partners that have the measurable flag set (or overridden)!!!!!
            getCollisionPartners(cp,ct,it,&allJoints);
            collPartners.push_back(cp);
            collTreshholds.push_back(ct);
        }
        for (int jointCounter=0;jointCounter<int(collPartners.size());jointCounter++)
        { // We perform this for every joint-stage:
            bool activateAvoidanceForThatJointStage=false;
            std::vector<C3DObject*>* coll=collPartners[jointCounter];
            std::vector<float>* tresh=collTreshholds[jointCounter];
            C3DObject* avoidingObject=nullptr;
            // Now we first search for the smallest distance among these partners:
            float importance=0;
            float ray[7];
            float dist=0.0f;
            float currentTreshhold=0.0f;
            for (int p=0;p<int(coll->size())/2;p++)
            {
                float distTmp=tresh->at(p);
                float rayTmp[7];
                C3DObject* object=coll->at(2*p+0);
                C3DObject* obstacle=coll->at(2*p+1);
                // *****************************************************************************
                // The measurable flags of object and obstacle have been checked beforehand!!!!!
                // *****************************************************************************
                int dummyCache[2]={-1,-1};
//              if (CDistanceRoutine::getObjectObstacleDistanceIfSmaller(object,obstacle,distTmp,rayTmp,true,true))
                if (CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(object->getObjectHandle(),obstacle->getObjectHandle(),distTmp,rayTmp,dummyCache,dummyCache,true,true))
                {   // The measured distance is under the treshhold!!
                    // Now we check if the importance is bigger than a previously
                    // detected treshhold violation (treshholds can vary from partner
                    // to partner!). Also if measured distanc is 0, we don't do anything.
                    if (distTmp>0.000000001f)
                    {
                        float importanceTmp=tresh->at(p)/distTmp;
                        if (importanceTmp>importance)
                        { // These avoidance partners are more important!! We take these:
                            dist=distTmp;
                            importance=tresh->at(p)/distTmp;
                            currentTreshhold=tresh->at(p);
                            for (int i=0;i<7;i++)
                                ray[i]=rayTmp[i];
                            avoidingObject=object;
                            activateAvoidanceForThatJointStage=true;
                        }
                    }
                }
            }
            if (activateAvoidanceForThatJointStage)
            {
                // If an obstacle is closer that 0.95*treshhold:
                // (important in case target and tooltip are within tolerance)
                if ((dist/currentTreshhold)<0.95f)
                    limitOrAvoidanceNeedMoreCalculation=true;
                // First we have to find a base. Base will be the first joint (from
                // avoidingObject) which is not listed in allJoints or nullptr.
                C3DObject* base=avoidingObject->getParentObject();
                while (base!=nullptr)
                {
                    if (base->getObjectType()==sim_object_joint_type)
                    { // We check if this joint is listed:
                        bool listed=false;
                        for (int i=0;i<int(allJoints.size());i++)
                        {
                            if (allJoints[i]==base)
                            {
                                listed=true;
                                break;
                            }
                        }
                        if (!listed)
                            break;
                    }
                    base=base->getParentObject();
                }
                // We have the base now!
                // We find the closestPoint on avoidingObject and compute its
                // relative position. Then we make a relative transformation
                // matrix with it:
                C7Vector absPos;
                absPos.setIdentity();
                absPos.X(0)=ray[0];
                absPos.X(1)=ray[1];
                absPos.X(2)=ray[2];
                C7Vector absObj;
                absObj=avoidingObject->getCumulativeTransformationPart1(true).getInverse();
                C7Vector relPos(absObj*absPos);
                // We get the jacobian:
                std::vector<int>* rowIDs=new std::vector<int>;
                rowIDs->reserve(16);
                rowIDs->clear();
                CMatrix* jacMatr=CIkRoutine::getAvoidanceJacobian(base,avoidingObject,relPos.getMatrix(),rowIDs);
                // We compute the error vector (relative to the chosen base):
                C3Vector pt1(ray+0);
                C3Vector pt2(ray+3);
                if (base!=nullptr)
                {
                    C7Vector baseAbs(base->getCumulativeTransformation(true).getInverse());
                    pt1*=baseAbs;
                    pt2*=baseAbs;
                }
                CMatrix d0(1,3);
                d0(0,0)=pt1(0)-pt2(0);
                d0(0,1)=pt1(1)-pt2(1);
                d0(0,2)=pt1(2)-pt2(2);
                float e0=0.5f*(currentTreshhold*currentTreshhold-d0(0,0)*d0(0,0)-
                                d0(0,1)*d0(0,1)-d0(0,2)*d0(0,2));
                CMatrix* line=new CMatrix(1,jacMatr->cols);
                (*line)=d0*(*jacMatr);
                delete jacMatr;

                // We add the matrix, the error vector and the jointIDs:
                avoidanceError.push_back(e0*avoidanceWeight);
                avoidanceMatrix.push_back(line);
                avoidanceRowIDs.push_back(rowIDs);
            }
        }
        for (int i=0;i<int(collPartners.size());i++)
        {
            collPartners[i]->clear();
            delete collPartners[i];
            collTreshholds[i]->clear();
            delete collTreshholds[i];
        }
        collPartners.clear();
        collTreshholds.clear();
    }
    numberOfRows=numberOfRows+int(avoidanceError.size());

    //---------------------------------------------------------------------------

    // Now we prepare the joint limitation part:
    //---------------------------------------------------------------------------
    std::vector<float> limitationError;
    std::vector<int> limitationIndex;
    std::vector<float> limitationValue;
    if (_correctJointLimits)
    {
        for (int jointCounter=0;jointCounter<int(allJoints.size());jointCounter++)
        {
            CJoint* it=allJoints[jointCounter];
            int stage=allJointStages[jointCounter];
            float minVal=it->getPositionIntervalMin();
            float range=it->getPositionIntervalRange();
            float value=it->getPosition(true);
            if (it->getJointType()==sim_joint_revolute_subtype)
            { // We have to handle a revolute joint
                if (!it->getPositionIsCyclic())
                { // Limitation applies only if joint is not cyclic!
                    float distFromMin=value-jointTreshholdAngular-minVal;
                    float distFromMax=value+jointTreshholdAngular-(minVal+range);
                    float eq=0.0f;
                    float activate=-10.0f;
                    if (distFromMin<0.0f)
                    {
                        activate=1.0f; // We correct in the positive direction
                        eq=-distFromMin;
                    }
                    if (distFromMax>0.0f)
                    {
                        activate=-1.0f; // We correct in the negative direction
                        eq=distFromMax;
                    }
                    if (activate>-5.0f)
                    { // We have to activate a joint limitation equation!
                        // If we are over the treshhold of more than 5%:
                        // (important in case target and tooltip are within tolerance)
                        if (eq>(jointTreshholdAngular*0.05f))
                            limitOrAvoidanceNeedMoreCalculation=true;
                        // First the error part:
                        limitationError.push_back(eq*jointLimitWeight);
                        // Now the matrix part:
                        limitationIndex.push_back(jointCounter);
                        limitationValue.push_back(activate);
                    }
                }
            }
            if (it->getJointType()==sim_joint_prismatic_subtype)
            { // We have to handle a prismatic joint:
                float distFromMin=value-(minVal+jointTreshholdLinear);
                float distFromMax=value-(minVal+range-jointTreshholdLinear);
                float eq=0.0f;
                float activate=-10.0f;
                if ( (distFromMin<0.0f)&&(fabs(distFromMin)<fabs(distFromMax)) )
                {
                    activate=1.0f; // We correct in the positive direction
                    eq=-distFromMin;
                }
                if ( (distFromMax>0.0f)&&(fabs(distFromMax)<fabs(distFromMin)) )
                {
                    activate=-1.0f; // We correct in the negative direction
                    eq=distFromMax;
                }
                if (activate>-5.0f)
                { // We have to activate a joint limitation equation!
                    // If we are over the treshhold of more than 5%:
                    // (important in case target and tooltip are within tolerance)
                    if (eq>(jointTreshholdLinear*0.05f))
                        limitOrAvoidanceNeedMoreCalculation=true;
                    // First the error part:
                    limitationError.push_back(eq*jointLimitWeight);
                    // Now the matrix part:
                    limitationIndex.push_back(jointCounter);
                    limitationValue.push_back(activate);
                }
            }
            if (it->getJointType()==sim_joint_spherical_subtype)
            { // We have to handle a spherical joint
                if ( (it->getTempSphericalJointLimitations()!=0)&&(stage==1) ) // Joint limitation configuration was activated!
                {
                    float v=it->getTempParameterEx(stage);
                    float distFromMax=v+jointTreshholdAngular-(minVal+range);
                    float eq;
                    float activate=-10.0f;
                    if (distFromMax>0.0f)
                    {
                        activate=-1.0f; // We correct in the negative direction
                        eq=distFromMax;
                    }
                    if (activate>-5.0f)
                    { // We have to activate a joint limitation equation!
                        // If we are over the treshhold of more than 5%:
                        // (important in case target and tooltip are within tolerance)
                        if (eq>(jointTreshholdAngular*0.05f))
                            limitOrAvoidanceNeedMoreCalculation=true;
                        // First the error part:
                        limitationError.push_back(eq*jointLimitWeight);
                        // Now the matrix part:
                        limitationIndex.push_back(jointCounter);
                        limitationValue.push_back(activate);
                    }
                }
            }
        }
    }
    numberOfRows=numberOfRows+int(limitationError.size());
    //---------------------------------------------------------------------------

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
        CikEl* element=validElements->at(elementNumber);
        for (int i=0;i<element->errorVector->rows;i++)
        { // We go through the rows:
            // We first set the error part:
            mainErrorVector(currentRow,0)=(*element->errorVector)(i,0);
            // Now we set the delta-parts:
            for (int j=0;j<element->matrix->cols;j++)
            { // We go through the columns:
                // We search for the right entry
                int jointID=element->rowJointIDs->at(j);
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
    // Now we add the joint limitation equations:
    for (int i=0;i<int(limitationError.size());i++)
    { // We go through the rows:
        mainErrorVector(currentRow,0)=limitationError[i];
        // Now we set the delta-part:
        mainMatrix(currentRow,limitationIndex[i])=limitationValue[i];
        mainMatrix_correctJacobian(currentRow,limitationIndex[i])=limitationValue[i];
        currentRow++;
    }

    // Now we add the avoidance equations:
    for (int avNumber=0;avNumber<int(avoidanceError.size());avNumber++)
    {
        CMatrix* avMatr=avoidanceMatrix[avNumber];
        float avErr=avoidanceError[avNumber];
        std::vector<int>* avIDs=avoidanceRowIDs[avNumber];

        mainErrorVector(currentRow,0)=avErr;
        // Now we set the delta-parts:
        for (int j=0;j<avMatr->cols;j++)
        { // We go through the columns:
            // We search for the right entry
            int jointID=avIDs->at(j);
            int index=0;
            while (allJoints[index]->getObjectHandle()!=jointID)
                index++;
            mainMatrix(currentRow,index)=(*avMatr)(0,j);
            mainMatrix_correctJacobian(currentRow,index)=(*avMatr)(0,j);
        }
        currentRow++;
    }
    // We free the avoidance equations:
    for (int i=0;i<int(avoidanceError.size());i++)
    {
        delete avoidanceMatrix[i];
        avoidanceRowIDs[i]->clear();
        delete avoidanceRowIDs[i];
    }
    avoidanceError.clear();
    avoidanceMatrix.clear();
    avoidanceRowIDs.clear();    

    // Now we prepare the individual joint constraints part:
    //---------------------------------------------------------------------------
    for (int i=0;i<int(allJoints.size());i++)
    {
        if ( ((allJoints[i]->getJointMode()==sim_jointmode_dependent)||(allJoints[i]->getJointMode()==sim_jointmode_reserved_previously_ikdependent))&&(allJoints[i]->getJointType()!=sim_joint_spherical_subtype) )
        {
            int dependenceID=allJoints[i]->getDependencyJointID();
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
                    float coeff=allJoints[i]->getDependencyJointCoeff();
                    float fact=allJoints[i]->getDependencyJointFact();
                    mainErrorVector(currentRow,0)=((allJoints[i]->getPosition(true)-fact)-
                                    coeff*allJoints[dependenceID]->getPosition(true))*interpolFact;
                    mainMatrix(currentRow,i)=-1.0f;
                    mainMatrix(currentRow,dependenceID)=coeff;
                    mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    mainMatrix_correctJacobian(currentRow,dependenceID)=coeff;
                }
                else
                {   // joint of dependenceID is not part of this group calculation:
                    // therefore we take its current value --> WRONG! Since all temp params are initialized!
                    CJoint* dependentJoint=App::ct->objCont->getJoint(dependenceID);
                    if (dependentJoint!=nullptr)
                    {
                        float coeff=allJoints[i]->getDependencyJointCoeff();
                        float fact=allJoints[i]->getDependencyJointFact();
                        mainErrorVector(currentRow,0)=((allJoints[i]->getPosition(true)-fact)-
                                        coeff*dependentJoint->getPosition(true))*interpolFact;
                        mainMatrix(currentRow,i)=-1.0f;
                        mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    }
                }
            }
            else
            {               
                mainErrorVector(currentRow,0)=interpolFact*(allJoints[i]->getPosition(true)-allJoints[i]->getDependencyJointFact());
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
        _lastJacobian=new CMatrix(mainMatrix_correctJacobian); //mainMatrix);
    }

    if (calculationMethod==sim_ik_pseudo_inverse_method)
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
    if (calculationMethod==sim_ik_damped_least_squares_method)
    {
        CMatrix JT(mainMatrix);
        JT.transpose();
        CMatrix DLSJ(doF,eqNumb);
        CMatrix JJTInv(mainMatrix*JT);
        CMatrix ID(mainMatrix.rows,mainMatrix.rows);
        ID.setIdentity();
        ID/=1.0f/(dlsFactor*dlsFactor);
        JJTInv+=ID;
        if (!JJTInv.inverse())
            return(-1);
        DLSJ=JT*JJTInv;
        solution=DLSJ*mainErrorVector;
    }
    if (calculationMethod==sim_ik_jacobian_transpose_method)
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
    if (!ignoreMaxStepSizes)
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
            it->setPosition(it->getPosition(true)+solution(i,0),true);
        else
            it->setTempParameterEx(it->getTempParameterEx(stage)+solution(i,0),stage);
    }
    return(1);
}

bool CikGroup::performOnePass_jacobianOnly(std::vector<CikEl*>* validElements,int options)
{
    // We prepare a vector of all used joints and a counter for the number of rows:
    std::vector<CJoint*> allJoints;
    std::vector<int> allJointStages;
    int numberOfRows=0;
    for (int elementNumber=0;elementNumber<int(validElements->size());elementNumber++)
    {
        CikEl* element=validElements->at(elementNumber);
        numberOfRows=numberOfRows+element->matrix->rows;
        for (int i=0;i<int(element->rowJointIDs->size());i++)
        {
            int current=element->rowJointIDs->at(i);
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
                allJoints.push_back(App::ct->objCont->getJoint(current));
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
        CikEl* element=validElements->at(elementNumber);
        for (int i=0;i<element->errorVector->rows;i++)
        { // We go through the rows:
            // We first set the error part:
            mainErrorVector(currentRow,0)=(*element->errorVector)(i,0);
            // Now we set the delta-parts:
            for (int j=0;j<element->matrix->cols;j++)
            { // We go through the columns:
                // We search for the right entry
                int jointID=element->rowJointIDs->at(j);
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
            int dependenceID=allJoints[i]->getDependencyJointID();
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
                    float coeff=allJoints[i]->getDependencyJointCoeff();
                    float fact=allJoints[i]->getDependencyJointFact();
                    mainErrorVector(currentRow,0)=((allJoints[i]->getPosition(true)-fact)-
                                    coeff*allJoints[dependenceID]->getPosition(true));
                    mainMatrix(currentRow,i)=-1.0f;
                    mainMatrix(currentRow,dependenceID)=coeff;
                    mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    mainMatrix_correctJacobian(currentRow,dependenceID)=coeff;
                }
                else
                {   // joint of dependenceID is not part of this group calculation:
                    // therefore we take its current value --> WRONG! Since all temp params are initialized!
                    CJoint* dependentJoint=App::ct->objCont->getJoint(dependenceID);
                    if (dependentJoint!=nullptr)
                    {
                        float coeff=allJoints[i]->getDependencyJointCoeff();
                        float fact=allJoints[i]->getDependencyJointFact();
                        mainErrorVector(currentRow,0)=((allJoints[i]->getPosition(true)-fact)-
                                        coeff*dependentJoint->getPosition(true));
                        mainMatrix(currentRow,i)=-1.0f;
                        mainMatrix_correctJacobian(currentRow,i)=-1.0f;
                    }
                }
            }
            else
            {
                mainErrorVector(currentRow,0)=(allJoints[i]->getPosition(true)-allJoints[i]->getDependencyJointFact());
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

float*  CikGroup::getLastJacobianData(int matrixSize[2])
{
    if (_lastJacobian==nullptr)
        return(nullptr);
    matrixSize[0]=_lastJacobian->cols;
    matrixSize[1]=_lastJacobian->rows;
    return(_lastJacobian->data);
}


float*  CikGroup::getLastManipulabilityValue(int matrixSize[2])
{
    static float v;
    if (_lastJacobian==nullptr)
        return(nullptr);
    matrixSize[0]=1;
    matrixSize[1]=1;

    CMatrix JT(_lastJacobian[0]);
//  printf("a %f, %f, %f\n",JT(0,0),JT(1,0),JT(2,0));
//  printf("a %f, %f, %f\n",JT(0,1),JT(1,1),JT(2,1));
//  printf("a %f, %f, %f\n",JT(0,2),JT(1,2),JT(2,2));

    JT.transpose();
    CMatrix JJT(_lastJacobian[0]*JT);

    v=sqrt(getDeterminant(JJT,nullptr,nullptr));

    return(&v);
}

float CikGroup::getDeterminant(const CMatrix& m,const std::vector<int>* activeRows,const std::vector<int>* activeColumns)
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
        return(getDeterminant(m,&actR,&actC));
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
        retVal+=m(i,activeColumns->at(0))*getDeterminant(m,&actR,&actC)*pow(float(-1.0f),float(rowInd+2)); // was rowInd+1 until 3.1.3 rev2.
    }
    return(retVal);
}
