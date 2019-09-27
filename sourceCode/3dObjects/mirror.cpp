
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "mirror.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "mirrorRendering.h"

int CMirror::currentMirrorContentBeingRendered=-1;

CMirror::CMirror()
{
    _commonInit();
}

std::string CMirror::getObjectTypeInfo() const
{
    return("Mirror");
}
std::string CMirror::getObjectTypeInfoExtended() const
{
    return("Mirror");
}
bool CMirror::isPotentiallyCollidable() const
{
    return(false);
}
bool CMirror::isPotentiallyMeasurable() const
{
    return(false);
}
bool CMirror::isPotentiallyDetectable() const
{
    return(false);
}
bool CMirror::isPotentiallyRenderable() const
{
    return(true);
}
bool CMirror::isPotentiallyCuttable() const
{
    return(false);
}

void CMirror::_commonInit()
{
    _initialValuesInitialized=false;
    setObjectType(sim_object_mirror_type);
    _mirrorWidth=0.5f;
    _mirrorHeight=1.0f;
    _mirrorReflectance=0.75f;
    mirrorColor[0]=0.0f;
    mirrorColor[1]=0.0f;
    mirrorColor[2]=0.0f;

    clipPlaneColor.setDefaultValues();
    clipPlaneColor.setColor(0.0f,0.525f,0.6f,sim_colorcomponent_ambient_diffuse);
    clipPlaneColor.translucid=true;
    clipPlaneColor.transparencyFactor=0.4f;

    _active=true;
    _isMirror=true;
    _clippingObjectOrCollection=-1; // clipping all
    layer=CAMERA_LIGHT_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_renderable;

    _objectManipulationModePermissions=0x013;

    _objectName=IDSOGL_MIRROR;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

bool CMirror::getContainsTransparentComponent() const
{
    if (_isMirror)
        return(false);
    return(clipPlaneColor.translucid);
}

CVisualParam* CMirror::getClipPlaneColor()
{
    return(&clipPlaneColor);
}


bool CMirror::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    minV(0)=-0.5f*_mirrorWidth;
    maxV(0)=0.5f*_mirrorWidth;
    minV(1)=-0.5f*_mirrorHeight;
    maxV(1)=0.5f*_mirrorHeight;
    minV(2)=0.0f;
    maxV(2)=0.0f;
    return(true);
}

bool CMirror::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

CMirror::~CMirror()
{
}

bool CMirror::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CMirror::scaleObject(float scalingFactor)
{
    _mirrorWidth*=scalingFactor;
    _mirrorHeight*=scalingFactor;
    scaleObjectMain(scalingFactor);
}

void CMirror::scaleObjectNonIsometrically(float x,float y,float z)
{
    _mirrorWidth*=x;
    _mirrorHeight*=y;
    scaleObjectNonIsometricallyMain(x,y,z);
}

void CMirror::setMirrorWidth(float w)
{
    _mirrorWidth=tt::getLimitedFloat(0.001f,100.0f,w);
}

float CMirror::getMirrorWidth()
{
    return(_mirrorWidth);
}

void CMirror::setMirrorHeight(float h)
{
    _mirrorHeight=tt::getLimitedFloat(0.001f,100.0f,h);
}

float CMirror::getMirrorHeight()
{
    return(_mirrorHeight);
}

void CMirror::setReflectance(float r)
{
    _mirrorReflectance=tt::getLimitedFloat(0.0f,1.0f,r);
}

float CMirror::getReflectance()
{
    return(_mirrorReflectance);
}

void CMirror::setActive(bool a)
{
    _active=a;
}

bool CMirror::getActive()
{
    return(_active);
}

void CMirror::setIsMirror(bool m)
{
    if (m!=_isMirror)
        App::setRefreshHierarchyViewFlag();
    _isMirror=m;
}

bool CMirror::getIsMirror()
{
    return(_isMirror);
}

void CMirror::setClippingObjectOrCollection(int co)
{
    _clippingObjectOrCollection=co;
}

int CMirror::getClippingObjectOrCollection()
{
    return(_clippingObjectOrCollection);
}

void CMirror::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CMirror::copyYourself()
{   
    CMirror* newMirror=(CMirror*)copyYourselfMain();

    // Various
    newMirror->_mirrorHeight=_mirrorHeight;
    newMirror->_mirrorWidth=_mirrorWidth;
    newMirror->_active=_active;
    newMirror->_isMirror=_isMirror;
    newMirror->_mirrorReflectance=_mirrorReflectance;
    newMirror->_clippingObjectOrCollection=_clippingObjectOrCollection;

    newMirror->mirrorColor[0]=mirrorColor[0];
    newMirror->mirrorColor[1]=mirrorColor[1];
    newMirror->mirrorColor[2]=mirrorColor[2];

    clipPlaneColor.copyYourselfInto(&newMirror->clipPlaneColor);

    newMirror->_initialMirrorActive=_initialMirrorActive;

    return(newMirror);
}

bool CMirror::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(id)'-call or similar
    // Return value true means 'this' has to be erased too!
    if (_clippingObjectOrCollection==objectHandle)
    {
        _clippingObjectOrCollection=-1;
        if (!_isMirror)
            _active=false;
    }
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void CMirror::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_clippingObjectOrCollection==groupID)
    {
        _clippingObjectOrCollection=-1;
        if (!_isMirror)
            _active=false;
    }
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CMirror::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CMirror::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CMirror::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CMirror::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CMirror::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    if (_clippingObjectOrCollection<SIM_IDSTART_COLLECTION)
        _clippingObjectOrCollection=App::ct->objCont->getLoadingMapping(map,_clippingObjectOrCollection);
}
void CMirror::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
    if (_clippingObjectOrCollection>=SIM_IDSTART_COLLECTION)
        _clippingObjectOrCollection=App::ct->objCont->getLoadingMapping(map,_clippingObjectOrCollection);
}
void CMirror::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CMirror::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CMirror::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CMirror::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CMirror::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CMirror::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CMirror::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CMirror::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CMirror::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialMirrorActive=_active;
    }
}


void CMirror::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CMirror::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _active=_initialMirrorActive;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

void CMirror::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Msz");
            ar << _mirrorWidth << _mirrorHeight;
            ar.flush();

            ar.storeDataName("Cas");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_active);
            SIM_SET_CLEAR_BIT(nothing,1,!_isMirror);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Clp");
            ar << _clippingObjectOrCollection;
            ar.flush();

            ar.storeDataName("Mcr");
            ar << _mirrorReflectance << mirrorColor[0] << mirrorColor[1] << mirrorColor[2];
            ar.flush();

            ar.storeDataName("Cpc");
            ar.setCountingMode();
            clipPlaneColor.serialize(ar,0);
            if (ar.setWritingMode())
                clipPlaneColor.serialize(ar,0);

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
                    if (theName.compare("Msz")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _mirrorWidth >> _mirrorHeight;
                    }
                    if (theName=="Cas")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _active=SIM_IS_BIT_SET(nothing,0);
                        _isMirror=!SIM_IS_BIT_SET(nothing,1);
                    }
                    if (theName.compare("Clp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _clippingObjectOrCollection;
                    }
                    if (theName.compare("Mcr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _mirrorReflectance >> mirrorColor[0] >> mirrorColor[1] >> mirrorColor[2];
                    }
                    if (theName.compare("Cpc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        clipPlaneColor.serialize(ar,0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(mirrorColor);
            }
        }
    }
}

void CMirror::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CMirror::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CMirror::display");
    EASYLOCK(_objectMutex);
    displayMirror(this,renderingObject,displayAttrib);
}
