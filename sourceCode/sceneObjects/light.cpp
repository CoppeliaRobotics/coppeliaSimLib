#include "light.h"
#include "simInternal.h"
#include "tt.h"
#include "simStrings.h"
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

void CLight::_commonInit()
{
    setObjectType(sim_object_light_type);
    _lightSize=0.10f;
    _spotExponent=5;
    _spotCutoffAngle=90.0f*degToRad_f;
    _visibilityLayer=CAMERA_LIGHT_LAYER;
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
        _objectAlias=IDSOGL_OMNIDIRECTIONAL_LIGHT;
    if (_lightType==sim_light_spot_subtype)
        _objectAlias=IDSOGL_SPOTLIGHT;
    if (_lightType==sim_light_directional_subtype)
        _objectAlias=IDSOGL_DIRECTIONAL_LIGHT;
    _objectName_old=_objectAlias;
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
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
        if (tt::getValidFloat(val.c_str(),f))
        {
            f*=scalingFactor;
            tt::insertKeyAndValue("fadeXDist@povray",tt::FNb(0,f,3,false).c_str(),_extensionString);
        }
    }

    CSceneObject::scaleObject(scalingFactor);
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
    CSceneObject::removeSceneDependencies();
}

CSceneObject* CLight::copyYourself()
{   
    CLight* newLight=(CLight*)CSceneObject::copyYourself();

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
    bool retVal=CSceneObject::announceObjectWillBeErased(objectHandle,copyBuffer);
    return(retVal);
}

void CLight::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CLight::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CLight::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}
void CLight::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CLight::performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
}
void CLight::performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CLight::performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CLight::performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}
void CLight::performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CLight::performTextureObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}

void CLight::performDynMaterialObjectLoadingMapping(const std::vector<int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CLight::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _initialLightActive=lightActive;
}

void CLight::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CLight::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            lightActive=_initialLightActive;
        }
    }
    CSceneObject::simulationEnded();
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

CColorObject* CLight::getColor(bool getLightColor)
{
    if (getLightColor)
        return(&lightColor);
    return(&objectColor);
}

void CLight::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
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
                CTTUtil::scaleColorUp_(objectColor.getColorsPtr());
                CTTUtil::scaleLightDown_(lightColor.getColorsPtr());
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

            if (ar.getCoppeliaSimVersionThatWroteThisFile()<=30601)
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
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'type' tag: can be 'omnidirectional', 'spotlight' or 'directional' ",exhaustiveXml);
            ar.xmlAddNode_enum("type",_lightType,sim_light_omnidirectional_subtype,"omnidirectional",sim_light_spot_subtype,"spotlight",sim_light_directional_subtype,"directional");

            ar.xmlAddNode_float("size",_lightSize);

            ar.xmlAddNode_int("spotExponent",_spotExponent);

            ar.xmlAddNode_float("cutoffAngle",_spotCutoffAngle*180.0f/piValue_f);

            ar.xmlPushNewNode("attenuationFactors");
            ar.xmlAddNode_float("constant",constantAttenuation);
            ar.xmlAddNode_float("linear",linearAttenuation);
            ar.xmlAddNode_float("quadratic",quadraticAttenuation);
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("lightIsActive",lightActive);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("lightIsLocal",_lightIsLocal);
            ar.xmlPopNode();

            ar.xmlPushNewNode("color");
            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("object");
                objectColor.serialize(ar,0);
                ar.xmlPopNode();
                ar.xmlPushNewNode("light");
                lightColor.serialize(ar,0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(objectColor.getColorsPtr()[l]*255.1f);
                ar.xmlAddNode_ints("object",rgb,3);
                ar.xmlPushNewNode("light");
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(lightColor.getColorsPtr()[3+l]*255.1f);
                ar.xmlAddNode_ints("ambientDiffuse",rgb,3);
                for (size_t l=0;l<3;l++)
                    rgb[l]=int(lightColor.getColorsPtr()[6+l]*255.1f);
                ar.xmlAddNode_ints("specular",rgb,3);
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_enum("type",_lightType,exhaustiveXml,"omnidirectional",sim_light_omnidirectional_subtype,"spotlight",sim_light_spot_subtype,"directional",sim_light_directional_subtype);

            if (ar.xmlGetNode_float("size",_lightSize,exhaustiveXml))
                setLightSize(_lightSize);

            if (ar.xmlGetNode_int("spotExponent",_spotExponent,exhaustiveXml))
                setSpotExponent(_spotExponent);

            if (ar.xmlGetNode_float("cutoffAngle",_spotCutoffAngle,exhaustiveXml))
                setSpotCutoffAngle(_spotCutoffAngle*piValue_f/180.0f);

            if (ar.xmlPushChildNode("attenuationFactors",exhaustiveXml))
            {
                if (ar.xmlGetNode_float("constant",constantAttenuation,exhaustiveXml))
                    setAttenuationFactor(constantAttenuation,CONSTANT_ATTENUATION);
                if (ar.xmlGetNode_float("linear",linearAttenuation,exhaustiveXml))
                    setAttenuationFactor(linearAttenuation,LINEAR_ATTENUATION);
                if (ar.xmlGetNode_float("quadratic",quadraticAttenuation,exhaustiveXml))
                    setAttenuationFactor(quadraticAttenuation,QUADRATIC_ATTENUATION);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("lightIsActive",lightActive,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("lightIsLocal",_lightIsLocal,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color",exhaustiveXml))
            {
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("object"))
                    {
                        objectColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("light"))
                    {
                        lightColor.serialize(ar,0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("object",rgb,3,exhaustiveXml))
                        objectColor.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_ambient_diffuse);
                    if (ar.xmlPushChildNode("light",exhaustiveXml))
                    {
                        if (ar.xmlGetNode_ints("ambientDiffuse",rgb,3,exhaustiveXml))
                            lightColor.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_diffuse);
                        if (ar.xmlGetNode_ints("specular",rgb,3,exhaustiveXml))
                            lightColor.setColor(float(rgb[0])/255.0f,float(rgb[1])/255.0f,float(rgb[2])/255.0f,sim_colorcomponent_specular);
                        ar.xmlPopNode();
                    }
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CLight::serializeWExtIk(CExtIkSer& ar)
{
    CSceneObject::serializeWExtIk(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CLight::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayLight(this,renderingObject,displayAttrib);
}
