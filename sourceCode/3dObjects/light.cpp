
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "light.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "easyLock.h"
#include "app.h"
#include "lightRendering.h"

int CLight::_maximumOpenGlLights=8;

CLight::CLight()
{
    _commonInit();
}

CLight::CLight(int theType)
{
    _lightType=theType;
    _commonInit();
}

std::string CLight::getObjectTypeInfo() const
{
    return("Light");
}
std::string CLight::getObjectTypeInfoExtended() const
{
    if (_lightType==sim_light_omnidirectional_subtype)
        return("Light (omnidirectional)");
    if (_lightType==sim_light_spot_subtype)
        return("Light (spot light)");
    if (_lightType==sim_light_directional_subtype)
        return("Light (directional)");
    return("ERROR");
}
bool CLight::isPotentiallyCollidable() const
{
    return(false);
}
bool CLight::isPotentiallyMeasurable() const
{
    return(false);
}
bool CLight::isPotentiallyDetectable() const
{
    return(false);
}
bool CLight::isPotentiallyRenderable() const
{
    return(false);
}
bool CLight::isPotentiallyCuttable() const
{
    return(false);
}

void CLight::_commonInit()
{
    _initialValuesInitialized=false;
    setObjectType(sim_object_light_type);
    _lightSize=0.10f;
    _spotExponent=5;
    _spotCutoffAngle=90.0f*degToRad_f;
    layer=CAMERA_LIGHT_LAYER;
    _localObjectSpecialProperty=0;
    _setDefaultColors();    
    constantAttenuation=0.25f;
    linearAttenuation=0.0f;
    quadraticAttenuation=0.1f;

    lightActive=true;
    _lightIsLocal=false;
    if (_extensionString.size()!=0)
        _extensionString+=" ";
    if (_lightType==sim_light_omnidirectional_subtype)
        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} normalBias {0.012} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";
    if (_lightType==sim_light_spot_subtype)
        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.0} normalBias {0.00008} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";
    if (_lightType==sim_light_directional_subtype)
        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} normalBias {0.005} shadowTextureSize {2048}}} povray {shadow {true} fadeXDist {0.00}}";

    _objectManipulationModePermissions=0x013;

    if (_lightType==sim_light_omnidirectional_subtype)
        _objectName=IDSOGL_OMNIDIRECTIONAL_LIGHT;
    if (_lightType==sim_light_spot_subtype)
        _objectName=IDSOGL_SPOTLIGHT;
    if (_lightType==sim_light_directional_subtype)
        _objectName=IDSOGL_DIRECTIONAL_LIGHT;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

bool CLight::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    if (_lightType==sim_light_omnidirectional_subtype)
    {
        minV(0)=-0.5f*_lightSize;
        maxV(0)=0.5f*_lightSize;
        minV(1)=-0.5f*_lightSize;
        maxV(1)=0.5f*_lightSize;
        minV(2)=-0.5f*_lightSize;
        maxV(2)=0.5f*_lightSize;
    }
    if (_lightType==sim_light_spot_subtype)
    {
        minV(0)=-0.8f*_lightSize;
        maxV(0)=0.8f*_lightSize;
        minV(1)=-0.8f*_lightSize;
        maxV(1)=0.8f*_lightSize;
        minV(2)=-1.5f*_lightSize;
        maxV(2)=0.5f*_lightSize;
    }
    if (_lightType==sim_light_directional_subtype)
    {
        minV(0)=-_lightSize*0.5f;
        maxV(0)=_lightSize*0.5f;
        minV(1)=-_lightSize*0.5f;
        maxV(1)=_lightSize*0.5f;
        minV(2)=-0.5f*_lightSize;
        maxV(2)=0.5f*_lightSize;
    }
    return(true);
}

bool CLight::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

void CLight::_setDefaultColors()
{
    if (_lightType==sim_light_omnidirectional_subtype)
    {
        objectColor.setDefaultValues();
        lightColor.setDefaultValues();
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_diffuse);
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_specular);
    }
    if (_lightType==sim_light_spot_subtype)
    {
        objectColor.setDefaultValues();
        objectColor.setColor(1.0f,0.375f,0.25f,sim_colorcomponent_ambient_diffuse);
        lightColor.setDefaultValues();
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_diffuse);
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_specular);
    }
    if (_lightType==sim_light_directional_subtype)
    {
        objectColor.setDefaultValues();
        objectColor.setColor(0.45f,0.45f,0.75f,sim_colorcomponent_ambient_diffuse);
        lightColor.setDefaultValues();
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_diffuse);
        lightColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_specular);
    }
}

CLight::~CLight()
{
}

bool CLight::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CLight::scaleObject(float scalingFactor)
{
    _lightSize*=scalingFactor;
    linearAttenuation/=scalingFactor;
    quadraticAttenuation/=scalingFactor*scalingFactor;

    std::string val;
    if (tt::getValueOfKey("fadeXDist@povray",_extensionString.c_str(),val))
    {
        float f;
        if (tt::getValidFloat(val,f))
        {
            f*=scalingFactor;
            tt::insertKeyAndValue("fadeXDist@povray",tt::FNb(0,f,3,false).c_str(),_extensionString);
        }
    }

    scaleObjectMain(scalingFactor);
}

void CLight::scaleObjectNonIsometrically(float x,float y,float z)
{
    scaleObject(cbrt(x*y*z));
}

void CLight::setLightSize(float size)
{
    tt::limitValue(0.001f,100.0f,size);
    _lightSize=size;
}

float CLight::getLightSize()
{
    return(_lightSize);
}

float CLight::getAttenuationFactor(short type)
{
    if (type==CONSTANT_ATTENUATION) return(constantAttenuation);
    if (type==LINEAR_ATTENUATION) return(linearAttenuation);
    if (type==QUADRATIC_ATTENUATION) return(quadraticAttenuation);
    return(0);
}

void CLight::setAttenuationFactor(float value,short type)
{
    tt::limitValue(0.0,1000.0,value);
    if (type==CONSTANT_ATTENUATION) constantAttenuation=value;
    if (type==LINEAR_ATTENUATION) linearAttenuation=value;
    if (type==QUADRATIC_ATTENUATION) quadraticAttenuation=value;
}

void CLight::setLightActive(bool active)
{
    if (active!=lightActive)
        App::setRefreshHierarchyViewFlag();
    lightActive=active;
}


bool CLight::getLightActive()
{
    return(lightActive);
}

void CLight::setSpotExponent(int e)
{
    _spotExponent=tt::getLimitedInt(0,128,e);
}

int CLight::getSpotExponent()
{
    return(_spotExponent);
}

void CLight::setSpotCutoffAngle(float co)
{
    _spotCutoffAngle=tt::getLimitedFloat(5.0f*degToRad_f,90.0f*degToRad_f,co);
}

float CLight::getSpotCutoffAngle()
{
    return(_spotCutoffAngle);
}


int  CLight::getLightType()
{
    return(_lightType);
}

void CLight::removeSceneDependencies()
{
    removeSceneDependenciesMain();
}

C3DObject* CLight::copyYourself()
{   
    CLight* newLight=(CLight*)copyYourselfMain();

    // Various
    newLight->_lightSize=_lightSize;
    newLight->_lightType=_lightType;
    newLight->_spotExponent=_spotExponent;
    newLight->_spotCutoffAngle=_spotCutoffAngle;
    newLight->lightActive=lightActive;
    newLight->constantAttenuation=constantAttenuation;
    newLight->linearAttenuation=linearAttenuation;
    newLight->quadraticAttenuation=quadraticAttenuation;
    newLight->_lightIsLocal=_lightIsLocal;

    // Colors:
    objectColor.copyYourselfInto(&newLight->objectColor);
    lightColor.copyYourselfInto(&newLight->lightColor);

    return(newLight);
}

bool CLight::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void CLight::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CLight::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CLight::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CLight::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CLight::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CLight::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
}
void CLight::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
}
void CLight::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CLight::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CLight::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CLight::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}

void CLight::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}

void CLight::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}

void CLight::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}

void CLight::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CLight::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialLightActive=lightActive;
    }
}

void CLight::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CLight::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        lightActive=_initialLightActive;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

void  CLight::setLightIsLocal(bool l)
{
    _lightIsLocal=l;
}

bool  CLight::getLightIsLocal()
{
    return(_lightIsLocal);
}

void CLight::setMaxAvailableOglLights(int c)
{
    _maximumOpenGlLights=c;
}

int CLight::getMaxAvailableOglLights()
{
    return(_maximumOpenGlLights);
}

CVisualParam* CLight::getColor(bool getLightColor)
{
    if (getLightColor)
        return(&lightColor);
    return(&objectColor);
}

void CLight::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing

            ar.storeDataName("Cp2");
            ar << _lightType << _spotExponent << _lightSize;
            ar.flush();

            ar.storeDataName("Cp3");
            ar << _spotCutoffAngle;
            ar.flush();

            ar.storeDataName("Caf");
            ar << constantAttenuation << linearAttenuation << quadraticAttenuation;
            ar.flush();

            ar.storeDataName("Cas");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,lightActive);
            SIM_SET_CLEAR_BIT(nothing,1,_lightIsLocal);
            // RESERVED SIM_SET_CLEAR_BIT(nothing,2,!povShadow);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            objectColor.serialize(ar,0);
            if (ar.setWritingMode())
                objectColor.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            lightColor.serialize(ar,3);
            if (ar.setWritingMode())
                lightColor.serialize(ar,3);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool povShadow_backwardCompatibility_3_2_2016=true;
            float povFadeXDist_backwardCompatibility_3_2_2016=-1.0;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Cp2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _lightType >> _spotExponent >> _lightSize;
                    }
                    if (theName.compare("Cp3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _spotCutoffAngle;
                    }
                    if (theName.compare("Caf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> constantAttenuation >> linearAttenuation >> quadraticAttenuation;
                    }
                    if (theName.compare("Pfd")==0)
                    { // keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> povFadeXDist_backwardCompatibility_3_2_2016;
                    }
                    if (theName=="Cas")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        lightActive=SIM_IS_BIT_SET(nothing,0);
                        _lightIsLocal=SIM_IS_BIT_SET(nothing,1);
                        povShadow_backwardCompatibility_3_2_2016=!SIM_IS_BIT_SET(nothing,2);
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        objectColor.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        lightColor.serialize(ar,3);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(objectColor.colors);
                CTTUtil::scaleLightDown_(lightColor.colors);
            }

            if (povFadeXDist_backwardCompatibility_3_2_2016>=0.0)
            { // keep for backward compatibility (3/2/2016)
                _extensionString="povray {shadow {";
                if (povShadow_backwardCompatibility_3_2_2016)
                    _extensionString+="true} fadeXDist {";
                else
                    _extensionString+="false} fadeXDist {";
                _extensionString+=tt::FNb(0,povFadeXDist_backwardCompatibility_3_2_2016,3,false);
                _extensionString+="}}";
            }

            if (ar.getVrepVersionThatWroteThisFile()<=30601)
            {
                if (_extensionString.find("openGL3")==std::string::npos)
                {
                    if (_extensionString.size()!=0)
                        _extensionString+=" ";
                    if (_lightType==sim_light_omnidirectional_subtype)
                        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} normalBias {0.012} shadowTextureSize {2048}}}";
                    if (_lightType==sim_light_spot_subtype)
                        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.0} normalBias {0.00008} shadowTextureSize {2048}}}";
                    if (_lightType==sim_light_directional_subtype)
                        _extensionString+="openGL3 {lightProjection {nearPlane {0.1} farPlane {10} orthoSize {8} bias {0.001} normalBias {0.005} shadowTextureSize {2048}}}";
                }
            }
        }
    }
}

void CLight::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CLight::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CLight::display");
    EASYLOCK(_objectMutex);
    displayLight(this,renderingObject,displayAttrib);
}
