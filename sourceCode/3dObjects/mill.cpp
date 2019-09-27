
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "mill.h"
#include "tt.h"
#include "cuttingRoutine.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "pluginContainer.h"
#include "millRendering.h"

CMill::CMill(int theType)
{
    commonInit();
    setMillType(theType);
}

CMill::CMill()
{ // needed by the serialization routine only!
    commonInit();
}

CMill::~CMill()
{
    delete convexVolume;
}

std::string CMill::getObjectTypeInfo() const
{
    return("Mill");
}

std::string CMill::getObjectTypeInfoExtended() const
{
    if (_millType==sim_mill_pyramid_subtype)
        return("Mill (pyramid)");
    if (_millType==sim_mill_cylinder_subtype)
        return("Mill (cylinder)");
    if (_millType==sim_mill_disc_subtype)
        return("Mill (disc)");
    if (_millType==sim_mill_cone_subtype)
        return("Mill (cone)");
    return("ERROR");
}
bool CMill::isPotentiallyCollidable() const
{
    return(false);
}
bool CMill::isPotentiallyMeasurable() const
{
    return(false);
}
bool CMill::isPotentiallyDetectable() const
{
    return(false);
}
bool CMill::isPotentiallyRenderable() const
{
    return(false);
}
bool CMill::isPotentiallyCuttable() const
{
    return(false);
}

void CMill::commonInit()
{
    convexVolume=new CConvexVolume();
    _explicitHandling=false;
    _objectType=sim_object_mill_type;
    _size=0.01f;
    _localObjectSpecialProperty=0;
    _millableObject=-1;
    _millDataValid=false;
    _calcTimeInMs=0;
    _milledSurface=0.0f;
    _milledVolume=0.0f;
    _milledObjectCount=0;

    _initialValuesInitialized=false;

    _objectManipulationModePermissions=0x013;

    passiveVolumeColor.setDefaultValues();
    passiveVolumeColor.setColor(0.0f,0.5f,0.9f,sim_colorcomponent_ambient_diffuse);
    activeVolumeColor.setDefaultValues();
    activeVolumeColor.setColor(1.0f,0.1f,0.1f,sim_colorcomponent_ambient_diffuse);
    layer=MILL_LAYER;
    _objectName="Mill";
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

void CMill::setExplicitHandling(bool setExplicit)
{
    _explicitHandling=setExplicit;
}

bool CMill::getExplicitHandling() const
{
    return(_explicitHandling);
}

int CMill::getMillableObject() const
{
    return(_millableObject);
}

void CMill::setMillableObject(int objectID)
{
    _millableObject=objectID;
}

bool CMill::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CMill::removeSceneDependencies()
{
    removeSceneDependenciesMain();
    _millableObject=-1;
}

C3DObject* CMill::copyYourself()
{   
    CMill* newMill=(CMill*)copyYourselfMain();

    newMill->_millableObject=_millableObject;
    newMill->_milledSurface=_milledSurface;
    newMill->_milledVolume=_milledVolume;
    newMill->_milledObjectCount=_milledObjectCount;
    newMill->_millDataValid=false;
    newMill->_size=_size;
    newMill->_explicitHandling=_explicitHandling;
    newMill->_millType=_millType;

    delete newMill->convexVolume;
    newMill->convexVolume=convexVolume->copyYourself();

    // Colors:
    passiveVolumeColor.copyYourselfInto(&newMill->passiveVolumeColor);
    activeVolumeColor.copyYourselfInto(&newMill->activeVolumeColor);

    newMill->_initialValuesInitialized=_initialValuesInitialized;
    newMill->_initialExplicitHandling=_initialExplicitHandling;

    return(newMill);
}

bool CMill::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    if (_millableObject==objectHandle)
        _millableObject=-1;
    return(retVal);
}

void CMill::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
    if (_millableObject==groupID)
        _millableObject=-1;
}
void CMill::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CMill::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CMill::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CMill::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CMill::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performObjectLoadingMappingMain(map,loadingAmodel);
    if (_millableObject<SIM_IDSTART_COLLECTION)
        _millableObject=App::ct->objCont->getLoadingMapping(map,_millableObject);
}
void CMill::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
    if (_millableObject>=SIM_IDSTART_COLLECTION)
        _millableObject=App::ct->objCont->getLoadingMapping(map,_millableObject);
}
void CMill::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CMill::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CMill::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CMill::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CMill::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CMill::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CMill::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CMill::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CMill::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
}

void CMill::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CMill::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

void CMill::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Snt");
            ar << _millType;
            ar.flush();

            ar.storeDataName("Vod");
            ar.setCountingMode();
            convexVolume->serialize(ar);
            if (ar.setWritingMode())
                convexVolume->serialize(ar);

            ar.storeDataName("Sns");
            ar << _size;
            ar.flush();

            ar.storeDataName("Pra");
            unsigned char nothing=0;
            // 12/12/2011 SIM_SET_CLEAR_BIT(nothing,0,_millAllCuttable);
            // free since 13/7/2016 SIM_SET_CLEAR_BIT(nothing,1,_showMillBase_DEPRECATED);
            SIM_SET_CLEAR_BIT(nothing,2,_explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            passiveVolumeColor.serialize(ar,0);
            if (ar.setWritingMode())
                passiveVolumeColor.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            activeVolumeColor.serialize(ar,0);
            if (ar.setWritingMode())
                activeVolumeColor.serialize(ar,0);

            ar.storeDataName("Sox");
            ar << _millableObject;
            ar.flush();

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
                    if (theName.compare("Snt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _millType;
                    }
                    if (theName.compare("Vod")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        if (convexVolume!=nullptr)
                            delete convexVolume;
                        convexVolume=new CConvexVolume();
                        convexVolume->serialize(ar);
                    }
                    if (theName.compare("Sns")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _size;
                    }
                    if (theName.compare("Sox")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _millableObject;
                    }
                    if (theName=="Pra")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        // free since 13/7/2016 _showMillBase_DEPRECATED=SIM_IS_BIT_SET(nothing,1);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        passiveVolumeColor.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        activeVolumeColor.serialize(ar,0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(passiveVolumeColor.colors);
                CTTUtil::scaleColorUp_(activeVolumeColor.colors);
            }
        }
    }
}

void CMill::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

bool CMill::getMillingVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(convexVolume->getVolumeBoundingBox(minV,maxV));
}

bool CMill::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    getMillingVolumeBoundingBox(minV,maxV);
    C3Vector m(_size*0.25f,_size*0.25f,_size*0.25f); // mill base
    C3Vector n(-_size*0.25f,-_size*0.25f,-_size*0.25f);
    minV.keepMin(n);
    maxV.keepMax(m);
    return(true);
}

bool CMill::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return (getFullBoundingBox(minV,maxV));
}

void CMill::resetMill(bool exceptExplicitHandling)
{
    if ( (!exceptExplicitHandling)||(!getExplicitHandling()) )
    {
        _millDataValid=false;
        _calcTimeInMs=0;
    }
}

int CMill::handleMill(bool exceptExplicitHandling,float& milledSurface,float& milledVolume,bool justForInitialization)
{
    if (exceptExplicitHandling&&getExplicitHandling())
        return(0); // We don't want to handle those
    _millDataValid=false;
    _calcTimeInMs=0;
    if (!App::ct->mainSettings->millsEnabled)
        return(0);
    if (!CPluginContainer::isMeshPluginAvailable())
        return(0);

    int stTime=VDateTime::getTimeInMs();

    // For now:
    int cutObjectHandle=-1;
    milledSurface=0.0f;
    milledVolume=0.0f;
    _milledObjectCount=CCuttingRoutine::cutEntity(getObjectHandle(),_millableObject,cutObjectHandle,milledSurface,milledVolume,justForInitialization,false);
    _milledSurface=milledSurface;
    _milledVolume=milledVolume;
    _calcTimeInMs=VDateTime::getTimeDiffInMs(stTime);
    _millDataValid=true;
    return(_milledObjectCount);
}

float CMill::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

bool CMill::getMillDataIsValid() const
{
    return(_millDataValid);
}

CVisualParam* CMill::getColor(bool getActiveColor)
{
    if (getActiveColor)
        return(&activeVolumeColor);
    return(&passiveVolumeColor);
}

void CMill::setSize(float newSize)
{
    tt::limitValue(0.0001f,10.0f,newSize);
    _size=newSize;
}
float CMill::getSize() const
{
    return(_size);
}

bool CMill::getMilledSurface(float& surf) const
{
    if (!_millDataValid)
        return(false);
    surf=_milledSurface;
    return(true);
}

bool CMill::getMilledVolume(float& vol) const
{
    if (!_millDataValid)
        return(false);
    vol=_milledVolume;
    return(true);
}

bool CMill::getMilledCount(int& milledCount) const
{
    if (!_millDataValid)
        return(false);
    milledCount=_milledObjectCount;
    return(true);
}

void CMill::scaleObject(float scalingFactor)
{
    _size*=scalingFactor;
    convexVolume->scaleVolume(scalingFactor);
    scaleObjectMain(scalingFactor);
}

void CMill::scaleObjectNonIsometrically(float x,float y,float z)
{
    float xp,yp,zp;
    convexVolume->scaleVolumeNonIsometrically(x,y,z,xp,yp,zp);
    _size*=cbrt(xp*yp*zp);
    scaleObjectNonIsometricallyMain(xp,yp,zp);
}

void CMill::setMillType(int theType)
{
    _millType=theType;
    if (theType==sim_mill_cylinder_subtype)
        convexVolume->setVolumeType(CYLINDER_TYPE_CONVEX_VOLUME,_objectType,_size);
    if (theType==sim_mill_disc_subtype)
        convexVolume->setVolumeType(DISC_TYPE_CONVEX_VOLUME,_objectType,_size);
    if (theType==sim_mill_pyramid_subtype)
        convexVolume->setVolumeType(PYRAMID_TYPE_CONVEX_VOLUME,_objectType,_size);
    if (theType==sim_mill_cone_subtype)
        convexVolume->setVolumeType(CONE_TYPE_CONVEX_VOLUME,_objectType,_size);
}
int CMill::getMillType() const
{
    return(_millType);
}

void CMill::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CMill::display");
    EASYLOCK(_objectMutex);
    displayMill(this,renderingObject,displayAttrib);
}
