
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "dummy.h"
#include "global.h"
#include "easyLock.h"
#include "app.h"
#include "tt.h"
#include "dummyRendering.h"

CDummy::CDummy()
{
    _objectType=sim_object_dummy_type;

    layer=DUMMY_LAYER;
    _localObjectSpecialProperty=0;

    _objectName=IDSOGL_DUMMY;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);

    size=0.01f;
    color.setDefaultValues();
    color.setColor(1.0f,0.8f,0.55f,sim_colorcomponent_ambient_diffuse);
    _assignedToParentPath=false;
    _assignedToParentPathOrientation=false;
    _freeOnPathTrajectory=false;
    _virtualDistanceOffsetOnPath=0.0f;
    _virtualDistanceOffsetOnPath_variationWhenCopy=0.0f;
    _linkedDummyID=-1;
    _linkType=sim_dummy_linktype_ik_tip_target;
}

CDummy::~CDummy()
{
}

std::string CDummy::getObjectTypeInfo() const
{
    return(IDSOGL_DUMMY);
}

std::string CDummy::getObjectTypeInfoExtended() const
{
    return(IDSOGL_DUMMY);
}
bool CDummy::isPotentiallyCollidable() const
{
    return(true);
}
bool CDummy::isPotentiallyMeasurable() const
{
    return(true);
}
bool CDummy::isPotentiallyDetectable() const
{
    return(true);
}
bool CDummy::isPotentiallyRenderable() const
{
    return(false);
}
bool CDummy::isPotentiallyCuttable() const
{
    return(false);
}

bool CDummy::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    maxV(0)=maxV(1)=maxV(2)=size/2.0f;
    minV=maxV*-1.0f;
    return(true);
}

bool CDummy::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

bool CDummy::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CDummy::scaleObject(float scalingFactor)
{
    size*=scalingFactor;
    _virtualDistanceOffsetOnPath*=scalingFactor;
    _virtualDistanceOffsetOnPath_variationWhenCopy*=scalingFactor;
    scaleObjectMain(scalingFactor);

    // We have to reconstruct a part of the dynamics world:
    _dynamicsFullRefreshFlag=true; // yes, because we might have a position scaling too!
}

void CDummy::scaleObjectNonIsometrically(float x,float y,float z)
{
    scaleObject(cbrt(x*y*z));
}

void CDummy::removeSceneDependencies()
{
    removeSceneDependenciesMain();
    _linkedDummyID=-1;
}

C3DObject* CDummy::copyYourself()
{   
    CDummy* newDummy=(CDummy*)copyYourselfMain();

    color.copyYourselfInto(&newDummy->color);
    newDummy->size=size;
    newDummy->_linkedDummyID=_linkedDummyID;
    newDummy->_linkType=_linkType;
    newDummy->_assignedToParentPath=_assignedToParentPath;
    newDummy->_assignedToParentPathOrientation=_assignedToParentPathOrientation;
    newDummy->_freeOnPathTrajectory=_freeOnPathTrajectory;

    if (App::ct->copyBuffer->isCopyForPasting())
    { // here the original object is not reset (the variation) because it is located in the copy buffer!
        _virtualDistanceOffsetOnPath+=_virtualDistanceOffsetOnPath_variationWhenCopy;
        newDummy->_virtualDistanceOffsetOnPath=_virtualDistanceOffsetOnPath;
        newDummy->_virtualDistanceOffsetOnPath_variationWhenCopy=0.0f; // the new new object's variation is reset!
    }
    else
    {
        newDummy->_virtualDistanceOffsetOnPath=_virtualDistanceOffsetOnPath;
        newDummy->_virtualDistanceOffsetOnPath_variationWhenCopy=_virtualDistanceOffsetOnPath_variationWhenCopy;
        _virtualDistanceOffsetOnPath_variationWhenCopy=0.0f; // we reset the original object!!
    }

    return(newDummy);
}

void CDummy::setAssignedToParentPath(bool assigned)
{
    _assignedToParentPath=assigned;
    if (assigned)
        _assignedToParentPathOrientation=false;
}

bool CDummy::getAssignedToParentPath() const
{
    return(_assignedToParentPath);
}

void CDummy::setAssignedToParentPathOrientation(bool assigned)
{
    _assignedToParentPathOrientation=assigned;
    if (assigned)
        _assignedToParentPath=false;
}

bool CDummy::getAssignedToParentPathOrientation() const
{
    return(_assignedToParentPathOrientation);
}

void CDummy::setFreeOnPathTrajectory(bool isFree)
{
    _freeOnPathTrajectory=isFree;
}

bool CDummy::getFreeOnPathTrajectory() const
{
    return(_freeOnPathTrajectory);
}

void CDummy::setVirtualDistanceOffsetOnPath(float off)
{
    _virtualDistanceOffsetOnPath=off;
}

float CDummy::getVirtualDistanceOffsetOnPath() const
{
    return(_virtualDistanceOffsetOnPath);
}

void CDummy::setVirtualDistanceOffsetOnPath_variationWhenCopy(float off)
{
    _virtualDistanceOffsetOnPath_variationWhenCopy=off;
}

float CDummy::getVirtualDistanceOffsetOnPath_variationWhenCopy() const
{
    return(_virtualDistanceOffsetOnPath_variationWhenCopy);
}

void CDummy::setSize(float theNewSize) 
{ 
    size=theNewSize;
}

float CDummy::getSize() const
{ 
    return(size);
}

void CDummy::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CDummy::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CDummy::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}

void CDummy::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CDummy::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CDummy::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CDummy::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CDummy::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CDummy::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CDummy::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CDummy::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CDummy::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CDummy::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CDummy::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
}

void CDummy::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CDummy::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
//  {
//  }
    simulationEndedMain();
}

void CDummy::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Dy2");
            ar << size;
            ar.flush();

            ar.storeDataName("Cl0");
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            ar.storeDataName("Lli");
            ar << _linkedDummyID;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
    // removed on 2010/01/26        SIM_SET_CLEAR_BIT(dummy,0,_targetDummy);
            SIM_SET_CLEAR_BIT(dummy,1,_assignedToParentPath);
            SIM_SET_CLEAR_BIT(dummy,2,_freeOnPathTrajectory);
            SIM_SET_CLEAR_BIT(dummy,3,_assignedToParentPathOrientation);
    // removed on 17/6/2011.. probably always 0     SIM_SET_CLEAR_BIT(dummy,4,_removeAfterAssembly);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Po5");
            ar << _virtualDistanceOffsetOnPath << _virtualDistanceOffsetOnPath_variationWhenCopy;
            ar.flush();

            ar.storeDataName("Dl2");
            ar << _linkType;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool before2009_12_16=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Dy2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> size;
                    }
                    if (theName.compare("Lli")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _linkedDummyID;
                    }
                    if (theName.compare("Cl0")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _assignedToParentPath=SIM_IS_BIT_SET(dummy,1);
                        _freeOnPathTrajectory=SIM_IS_BIT_SET(dummy,2);
                        _assignedToParentPathOrientation=SIM_IS_BIT_SET(dummy,3);
                    }
    //************* for backward compatibility (23/02/2011) ****************
                    if (theName.compare("Po4")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _virtualDistanceOffsetOnPath;
                    }
    //************************************************************************
                    if (theName.compare("Po5")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _virtualDistanceOffsetOnPath >> _virtualDistanceOffsetOnPath_variationWhenCopy;
                    }

                    if (theName.compare("Dl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _linkType;
                    }
    //************** for backward compatibility (17/6/2011) **********************
                    if (theName.compare("Ack")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _modelAcknowledgement; // this is now the 3DObject's variable!!! (was Dummy's variable before)
                    }
    //****************************************************************************
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (before2009_12_16)
            {
                if (_linkedDummyID==-1)
                    _linkType=sim_dummy_linktype_dynamics_loop_closure;
            }
            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(color.colors);
            }
        }
    }
}

void CDummy::serializeWExtIk(CExtIkSer& ar)
{ // make sure to do similar in the serializeWExtIkStatic routine
    serializeWExtIkMain(ar);

    ar.writeInt(_linkedDummyID);

    ar.writeInt(_linkType);
}

void CDummy::serializeWExtIkStatic(CExtIkSer& ar)
{ // make sure to do similar in the serializeWExtIk routine
//  serializeWExtIkMain(ar); (called in the other obejct's routines!!)

    ar.writeInt(-1); // linked dummy ID

    ar.writeInt(0); // value doesn't matter (link type)
}

// SPECIAL FUNCTION TO GUARANTEE FORWARD COMPATIBILITY WHEN LOADING OBJECT TYPES THAT DON'T EXIST YET!
void CDummy::loadUnknownObjectType(CSer& ar)
{
    serializeMain(ar);
    // Now finish reading the object! (but we throw its content away!)
    std::string theName="";
    while (theName.compare(SER_END_OF_OBJECT)!=0)
    {
        theName=ar.readDataName();
        if (theName.compare(SER_END_OF_OBJECT)!=0)
            ar.loadUnknownData();
    }
}

void CDummy::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    _linkedDummyID=App::ct->objCont->getLoadingMapping(map,_linkedDummyID);
}

void CDummy::setLinkedDummyID(int id,bool setDirectly)
{
    int before=_linkedDummyID;
    if (setDirectly)
        _linkedDummyID=id;
    else
    {
        CDummy* linkedDummy=App::ct->objCont->getDummy(_linkedDummyID);
        if (id==-1)
        { // we unlink this dummy and its partner:
            if (linkedDummy!=nullptr)
                linkedDummy->setLinkedDummyID(-1,true);
            _linkedDummyID=-1;
        }
        else if (_linkedDummyID!=id)
        { // We link this dummy to another dummy
            CDummy* newLinkedDummy=App::ct->objCont->getDummy(id);
            if (linkedDummy!=nullptr)
                linkedDummy->setLinkedDummyID(-1,true); // we first detach it from its old partner
            if (newLinkedDummy!=nullptr)
            { // we detach the new dummy from its original linking:
                newLinkedDummy->setLinkedDummyID(-1,false);
                _linkedDummyID=id;
                newLinkedDummy->setLinkedDummyID(getObjectHandle(),true);

                if (_linkType==sim_dummy_linktype_gcs_tip)
                    newLinkedDummy->setLinkType(sim_dummy_linktype_gcs_target,true);
                if (_linkType==sim_dummy_linktype_gcs_target)
                    newLinkedDummy->setLinkType(sim_dummy_linktype_gcs_tip,true);
                if ( (_linkType==sim_dummy_linktype_ik_tip_target)||(_linkType==sim_dummy_linktype_gcs_loop_closure)||(_linkType==sim_dummy_linktype_dynamics_loop_closure)||(_linkType==sim_dummy_linktype_dynamics_force_constraint) )
                    newLinkedDummy->setLinkType(_linkType,true);
            }
            else
                _linkedDummyID=-1; // that dummy doesn't exist
        }
    }
    if (before!=_linkedDummyID)
    {
        App::setRefreshHierarchyViewFlag();
        App::setFullDialogRefreshFlag();
    }
}

CVisualParam* CDummy::getColor()
{
    return(&color);
}

void CDummy::setLinkType(int lt,bool setDirectly)
{
    _linkType=lt;
    if ((_linkedDummyID!=-1)&&(!setDirectly))
    {
        CDummy* it=App::ct->objCont->getDummy(_linkedDummyID);
        if (it!=nullptr)
        {
            if (lt==sim_dummy_linktype_gcs_tip)
                it->setLinkType(sim_dummy_linktype_gcs_target,true);
            if (lt==sim_dummy_linktype_gcs_target)
                it->setLinkType(sim_dummy_linktype_gcs_tip,true);
            if ( (lt==sim_dummy_linktype_ik_tip_target)||(lt==sim_dummy_linktype_gcs_loop_closure)||(lt==sim_dummy_linktype_dynamics_loop_closure)||(lt==sim_dummy_linktype_dynamics_force_constraint) )
                it->setLinkType(lt,true);
        }
        App::setRefreshHierarchyViewFlag();
        App::setFullDialogRefreshFlag();
    }
}

C7Vector CDummy::getTempLocalTransformation() const
{
    return(_localTransformation_temp);
}

void CDummy::setTempLocalTransformation(const C7Vector& tr)
{
    _localTransformation_temp=tr;
}

int CDummy::getLinkType() const
{
    return(_linkType);
}


int CDummy::getLinkedDummyID() const
{
    return(_linkedDummyID);
}

bool CDummy::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    if (_linkedDummyID==objectHandle)
        setLinkedDummyID(-1,copyBuffer);
    return(retVal);
}

void CDummy::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}

void CDummy::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CDummy::display");
    EASYLOCK(_objectMutex);
    displayDummy(this,renderingObject,displayAttrib);
}
