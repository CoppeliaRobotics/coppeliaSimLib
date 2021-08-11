#include "simInternal.h"
#include "environment.h"
#include "tt.h"
#include "global.h"
#include "fileOperations.h"
#include "sceneObjectOperations.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "app.h"
#include "environmentRendering.h"
#include "base64.h"
#include <boost/algorithm/string.hpp>
#include "pluginContainer.h"

int CEnvironment::_nextSceneUniqueID=0;
bool CEnvironment::_shapeTexturesTemporarilyDisabled=false;
bool CEnvironment::_shapeEdgesTemporarilyDisabled=false;
bool CEnvironment::_customUisTemporarilyDisabled=false;

CEnvironment::CEnvironment()
{
    _sceneIsClosingFlag=false;
    setUpDefaultValues();
    _sceneUniqueID=_nextSceneUniqueID++;
    generateNewUniquePersistentIdString();
}

CEnvironment::~CEnvironment()
{ // beware, the current world could be nullptr
}

void CEnvironment::generateNewUniquePersistentIdString()
{
    _sceneUniquePersistentIdString=CTTUtil::generateUniqueReadableString();
}

std::string CEnvironment::getUniquePersistentIdString() const
{
    return(_sceneUniquePersistentIdString);
}

int CEnvironment::getNextSceneUniqueId()
{
    return(_nextSceneUniqueID);
}

bool CEnvironment::getSceneIsClosingFlag() const
{
    return(_sceneIsClosingFlag);
}

void CEnvironment::setSceneIsClosingFlag(bool closing)
{
    _sceneIsClosingFlag=closing;
}

void CEnvironment::setShapeTexturesTemporarilyDisabled(bool d)
{
    _shapeTexturesTemporarilyDisabled=d;
}

bool CEnvironment::getShapeTexturesTemporarilyDisabled()
{
    return(_shapeTexturesTemporarilyDisabled);
}

void CEnvironment::setShapeEdgesTemporarilyDisabled(bool d)
{
    _shapeEdgesTemporarilyDisabled=d;
}

bool CEnvironment::getShapeEdgesTemporarilyDisabled()
{
    return(_shapeEdgesTemporarilyDisabled);
}

void CEnvironment::setCustomUisTemporarilyDisabled(bool d)
{
    _customUisTemporarilyDisabled=d;
}

bool CEnvironment::getCustomUisTemporarilyDisabled()
{
    return(_customUisTemporarilyDisabled);
}

void CEnvironment::setSceneCanBeDiscardedWhenNewSceneOpened(bool canBeDiscarded)
{
    _sceneCanBeDiscardedWhenNewSceneOpened=canBeDiscarded;
}

bool CEnvironment::getSceneCanBeDiscardedWhenNewSceneOpened() const
{
    return(_sceneCanBeDiscardedWhenNewSceneOpened);
}

void CEnvironment::setUpDefaultValues()
{
    _nonAmbientLightsAreActive=false;
    fogEnabled=false;
    fogDensity=0.5f;
    fogStart=0.0f;
    fogEnd=5.0f;
    fogType=0; // 0=linear, 1=exp, 2=exp2

    _sceneCanBeDiscardedWhenNewSceneOpened=false;
    autoSaveLastSaveTimeInSecondsSince1970=VDateTime::getSecondsSince1970();

    backGroundColor[0]=0.72f;
    backGroundColor[1]=0.81f;
    backGroundColor[2]=0.88f;
    backGroundColorDown[0]=0.05f;
    backGroundColorDown[1]=0.05f;
    backGroundColorDown[2]=0.1f;

    fogBackgroundColor[0]=0.0f;
    fogBackgroundColor[1]=0.0f;
    fogBackgroundColor[2]=0.0f;

    ambientLightColor[0]=0.2f;
    ambientLightColor[1]=0.2f;
    ambientLightColor[2]=0.2f;

    _extensionString="povray {fogDist {4.00} fogTransp {0.50}}";

    wirelessEmissionVolumeColor.setColorsAllBlack();
    wirelessEmissionVolumeColor.setColor(1.0f,1.0f,0.0f,sim_colorcomponent_emission);
    wirelessEmissionVolumeColor.setTranslucid(true);
    wirelessEmissionVolumeColor.setTransparencyFactor(0.2f);
    wirelessReceptionVolumeColor.setColorsAllBlack();
    wirelessReceptionVolumeColor.setColor(1.0f,1.0f,0.0f,sim_colorcomponent_emission);

    _acknowledgement="";
    _visualizeWirelessEmitters=false;
    _visualizeWirelessReceivers=false;
    _requestFinalSave=false;
    _sceneIsLocked=false;
    _shapeTexturesEnabled=true;
    _2DElementTexturesEnabled=true;
    _calculationMaxTriangleSize=0.3f; // from 0.8 to 0.3 on 2010/07/07
    _calculationMinRelTriangleSize=0.02f;
    _saveExistingCalculationStructures=false;
}

bool CEnvironment::areNonAmbientLightsActive() const
{
    return(_nonAmbientLightsAreActive);
}

void CEnvironment::setNonAmbientLightsActive(bool a)
{
    _nonAmbientLightsAreActive=a;
}

int CEnvironment::getSceneUniqueID() const
{
    return(_sceneUniqueID);
}

void CEnvironment::setSaveExistingCalculationStructures(bool s)
{
    _saveExistingCalculationStructures=s;
}

bool CEnvironment::getSaveExistingCalculationStructures() const
{
    return(_saveExistingCalculationStructures);
}

void CEnvironment::setSaveExistingCalculationStructuresTemp(bool s)
{
    _saveExistingCalculationStructuresTemp=s;
}

bool CEnvironment::getSaveExistingCalculationStructuresTemp() const
{
    return(_saveExistingCalculationStructuresTemp);
}

void CEnvironment::setCalculationMaxTriangleSize(float s)
{
    _calculationMaxTriangleSize=tt::getLimitedFloat(0.01f,100.0f,s);
}

float CEnvironment::getCalculationMaxTriangleSize() const
{
    return(_calculationMaxTriangleSize);
}

void CEnvironment::setCalculationMinRelTriangleSize(float s)
{
    _calculationMinRelTriangleSize=tt::getLimitedFloat(0.001f,1.0f,s);
}

float CEnvironment::getCalculationMinRelTriangleSize() const
{
    return(_calculationMinRelTriangleSize);
}

void CEnvironment::setShapeTexturesEnabled(bool e)
{
    _shapeTexturesEnabled=e;
}
bool CEnvironment::getShapeTexturesEnabled() const
{
    return(_shapeTexturesEnabled);
}

std::string CEnvironment::getExtensionString() const
{
    return(_extensionString);
}

void CEnvironment::setExtensionString(const char* str)
{
    _extensionString=str;
}

void CEnvironment::set2DElementTexturesEnabled(bool e)
{
    _2DElementTexturesEnabled=e;
}
bool CEnvironment::get2DElementTexturesEnabled() const
{
    return(_2DElementTexturesEnabled);
}

void CEnvironment::setRequestFinalSave(bool finalSaveActivated)
{
    _requestFinalSave=finalSaveActivated;
}

bool CEnvironment::getRequestFinalSave() const
{
    return(_requestFinalSave);
}

void CEnvironment::setSceneLocked()
{
    _sceneIsLocked=true;
    _requestFinalSave=false;
}

bool CEnvironment::getSceneLocked() const
{
    return(_sceneIsLocked);
}

void CEnvironment::setVisualizeWirelessEmitters(bool v)
{
    _visualizeWirelessEmitters=v;
}

bool CEnvironment::getVisualizeWirelessEmitters() const
{
    return(_visualizeWirelessEmitters);
}

void CEnvironment::setVisualizeWirelessReceivers(bool v)
{
    _visualizeWirelessReceivers=v;
}

bool CEnvironment::getVisualizeWirelessReceivers() const
{
    return(_visualizeWirelessReceivers);
}

void CEnvironment::setFogEnabled(bool e)
{
    fogEnabled=e;
}

bool CEnvironment::getFogEnabled() const
{
    return(fogEnabled);
}

void CEnvironment::setFogDensity(float d)
{
    tt::limitValue(0.0f,1000.0f,d);
    fogDensity=d;
}

float CEnvironment::getFogDensity() const
{
    return(fogDensity);
}

void CEnvironment::setFogStart(float s)
{
    tt::limitValue(0.0f,1000.0f,s); // "inverted" is allowed in opengl!
    fogStart=s;
}

float CEnvironment::getFogStart() const
{
    return(fogStart);
}

void CEnvironment::setFogEnd(float e)
{
    tt::limitValue(0.01f,1000.0f,e); // "inverted" is allowed in opengl!
    fogEnd=e;
}

float CEnvironment::getFogEnd() const
{
    return(fogEnd);
}

void CEnvironment::setFogType(float t)
{
    tt::limitValue(0,2,t);
    fogType=t;
}

int CEnvironment::getFogType() const
{
    return(fogType);
}

void CEnvironment::setAcknowledgement(const char* a)
{
    _acknowledgement=a;
    if (_acknowledgement.length()>3000)
        _acknowledgement.erase(_acknowledgement.begin()+2999,_acknowledgement.end());
}

std::string CEnvironment::getAcknowledgement() const
{
    return(_acknowledgement);
}
    
void CEnvironment::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Fdn");
            ar << fogEnd;
            ar.flush();

            ar.storeDataName("Fd2");
            ar << fogType << fogStart << fogDensity;
            ar.flush();

            ar.storeDataName("Clc");
            ar << backGroundColor[0] << backGroundColor[1] << backGroundColor[2];
            ar.flush();

            ar.storeDataName("Cld");
            ar << backGroundColorDown[0] << backGroundColorDown[1] << backGroundColorDown[2];
            ar.flush();

            ar.storeDataName("Fbg");
            ar << fogBackgroundColor[0] << fogBackgroundColor[1] << fogBackgroundColor[2];
            ar.flush();

            // Keep for backward/forward compatibility (5/10/2014):
            ar.storeDataName("Alc");
            ar << ambientLightColor[0]*0.5f << ambientLightColor[1]*0.5f << ambientLightColor[2]*0.5f;
            ar.flush();

            ar.storeDataName("Al2");
            ar << ambientLightColor[0] << ambientLightColor[1] << ambientLightColor[2];
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,!_2DElementTexturesEnabled);
            SIM_SET_CLEAR_BIT(dummy,1,_saveExistingCalculationStructures);
            SIM_SET_CLEAR_BIT(dummy,2,_visualizeWirelessEmitters);
            SIM_SET_CLEAR_BIT(dummy,3,!_visualizeWirelessReceivers);
            SIM_SET_CLEAR_BIT(dummy,4,fogEnabled);
            SIM_SET_CLEAR_BIT(dummy,5,_sceneIsLocked);
            SIM_SET_CLEAR_BIT(dummy,6,_requestFinalSave); // needed so that undo/redo works on that item too!
            SIM_SET_CLEAR_BIT(dummy,7,!_shapeTexturesEnabled);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va3");
            dummy=0;
            // free        SIM_SET_CLEAR_BIT(dummy,0,_showPartRepository);
            // free       SIM_SET_CLEAR_BIT(dummy,1,_showPalletRepository);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Ack");
            ar << _acknowledgement;
            ar.flush();

            ar.storeDataName("Mt2");
            ar << _calculationMaxTriangleSize;
            ar.flush();

            ar.storeDataName("Mrs");
            ar << _calculationMinRelTriangleSize;
            ar.flush();

            ar.storeDataName("Rst");
            ar << _extensionString;
            ar.flush();

            ar.storeDataName("Evc");
            ar.setCountingMode();
            wirelessEmissionVolumeColor.serialize(ar,1);
            if (ar.setWritingMode())
                wirelessEmissionVolumeColor.serialize(ar,1);

            ar.storeDataName("Wtc");
            ar.setCountingMode();
            wirelessReceptionVolumeColor.serialize(ar,1);
            if (ar.setWritingMode())
                wirelessReceptionVolumeColor.serialize(ar,1);

            ar.storeDataName("Ups");
            ar << _sceneUniquePersistentIdString;
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
                    if (theName.compare("Fdn")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> fogEnd;
                    }
                    if (theName.compare("Fd2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> fogType >> fogStart >> fogDensity;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _2DElementTexturesEnabled=!SIM_IS_BIT_SET(dummy,0);
                        _saveExistingCalculationStructures=SIM_IS_BIT_SET(dummy,1);
                        _visualizeWirelessEmitters=SIM_IS_BIT_SET(dummy,2);
                        _visualizeWirelessReceivers=!SIM_IS_BIT_SET(dummy,3);
                        fogEnabled=SIM_IS_BIT_SET(dummy,4);
                        _sceneIsLocked=SIM_IS_BIT_SET(dummy,5);
                        _requestFinalSave=SIM_IS_BIT_SET(dummy,6); // needed so that undo/redo works on that item too!
                        _shapeTexturesEnabled=!SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                    }
                    if (theName.compare("Va3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
    // free                    _showPartRepository=SIM_IS_BIT_SET(dummy,0);
    // free                    _showPalletRepository=SIM_IS_BIT_SET(dummy,1);
                    }
                    if (theName.compare("Clc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> backGroundColor[0] >> backGroundColor[1] >> backGroundColor[2];
                        backGroundColorDown[0]=backGroundColor[0];
                        backGroundColorDown[1]=backGroundColor[1];
                        backGroundColorDown[2]=backGroundColor[2];
                    }
                    if (theName.compare("Cld")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> backGroundColorDown[0] >> backGroundColorDown[1] >> backGroundColorDown[2];
                    }
                    if (theName.compare("Fbg")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> fogBackgroundColor[0] >> fogBackgroundColor[1] >> fogBackgroundColor[2];
                    }
                    if (theName.compare("Alc")==0)
                    { // keep for backward/forward compatibility (5/10/2014)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> ambientLightColor[0] >> ambientLightColor[1] >> ambientLightColor[2];
                        ambientLightColor[0]*=2.0f;
                        ambientLightColor[1]*=2.0f;
                        ambientLightColor[2]*=2.0f;
                        float mx=std::max<float>(std::max<float>(ambientLightColor[0],ambientLightColor[1]),ambientLightColor[2]);
                        if (mx>0.4f)
                        {
                            ambientLightColor[0]=ambientLightColor[0]*0.4f/mx;
                            ambientLightColor[1]=ambientLightColor[1]*0.4f/mx;
                            ambientLightColor[2]=ambientLightColor[2]*0.4f/mx;
                        }
                    }
                    if (theName.compare("Al2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> ambientLightColor[0] >> ambientLightColor[1] >> ambientLightColor[2];
                    }
                    if (theName.compare("Ack")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _acknowledgement;
                        if (App::userSettings->xrTest==123456789)
                        {
                        }
                    }
                    if (theName.compare("Mts")==0)
                    { // keep for backward compatibility (2010/07/07)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _calculationMaxTriangleSize;
                        _calculationMaxTriangleSize=0.3f;
                    }
                    if (theName.compare("Mt2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _calculationMaxTriangleSize;
                    }
                    if (theName.compare("Mrs")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _calculationMinRelTriangleSize;
                    }
                    if (theName.compare("Rst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _extensionString;
                    }
                    if (theName.compare("Pof")==0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float povDist,povTransp;
                        ar >> povDist >> povTransp;
                        _extensionString="povray {fogDist {";
                        _extensionString+=tt::FNb(0,povDist,3,false);
                        _extensionString+="} fogTransp {";
                        _extensionString+=tt::FNb(0,povTransp,3,false);
                        _extensionString+="}}";
                    }
                    if (theName.compare("Evc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        wirelessEmissionVolumeColor.serialize(ar,1);
                    }
                    if (theName.compare("Wtc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        wirelessReceptionVolumeColor.serialize(ar,1);
                    }
                    if (theName.compare("Ups")==0)
                    {
                        noHit=false;
                        std::string s;
                        ar >> byteQuantity;
                        ar >> s;
                        if (s.size()!=0)
                            _sceneUniquePersistentIdString=s;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                float avg=(ambientLightColor[0]+ambientLightColor[1]+ambientLightColor[2])/3.0f;
                if (avg>0.21f)
                    CTTUtil::scaleLightDown_(ambientLightColor);
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {       // Storing
            if (exhaustiveXml)
            {
                ar.xmlAddNode_floats("ambientLight",ambientLightColor,3);

                ar.xmlPushNewNode("backgroundColor");
                ar.xmlAddNode_floats("down",backGroundColorDown,3);
                ar.xmlAddNode_floats("up",backGroundColor,3);
                ar.xmlPopNode();
            }
            else
            {
                ar.xmlAddNode_3int("ambientLight",int(255.1f*ambientLightColor[0]),int(255.1f*ambientLightColor[1]),int(255.1f*ambientLightColor[2]));

                ar.xmlPushNewNode("backgroundColor");
                ar.xmlAddNode_3int("down",int(255.1f*backGroundColorDown[0]),int(255.1f*backGroundColorDown[1]),int(255.1f*backGroundColorDown[2]));
                ar.xmlAddNode_3int("up",int(255.1f*backGroundColor[0]),int(255.1f*backGroundColor[1]),int(255.1f*backGroundColor[2]));
                ar.xmlPopNode();
            }

            ar.xmlPushNewNode("fog");
            ar.xmlAddNode_bool("enabled",fogEnabled);
            ar.xmlAddNode_comment(" 'type' tag: can be 'linear', 'exp' or 'exp2' ",exhaustiveXml);
            ar.xmlAddNode_enum("type",fogType,0,"linear",1,"exp",2,"exp2");
            ar.xmlAddNode_2float("startEnd",fogStart,fogEnd);
            ar.xmlAddNode_float("density",fogDensity);
            if (exhaustiveXml)
                ar.xmlAddNode_floats("color",fogBackgroundColor,3);
            else
                ar.xmlAddNode_3int("color",int(255.1f*fogBackgroundColor[0]),int(255.1f*fogBackgroundColor[1]),int(255.1f*fogBackgroundColor[2]));
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("shapeTexturesEnabled",_shapeTexturesEnabled);
            ar.xmlPopNode();

            ar.xmlAddNode_string("acknowledgment",_acknowledgement.c_str());

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("calcStruct");
                ar.xmlAddNode_bool("saveExisting",_saveExistingCalculationStructures);
                ar.xmlAddNode_float("maxTriSize",_calculationMaxTriangleSize);
                ar.xmlAddNode_float("minRelativeTriSize",_calculationMinRelTriangleSize);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("wireless");
                ar.xmlPushNewNode("emitters");
                ar.xmlAddNode_bool("visualize",_visualizeWirelessEmitters);
                ar.xmlPushNewNode("color");
                wirelessEmissionVolumeColor.serialize(ar,1);
                ar.xmlPopNode();
                ar.xmlPopNode();
                ar.xmlPushNewNode("receivers");
                ar.xmlAddNode_bool("visualize",_visualizeWirelessReceivers);
                ar.xmlPushNewNode("color");
                wirelessReceptionVolumeColor.serialize(ar,1);
                ar.xmlPopNode();
                ar.xmlPopNode();
                ar.xmlPopNode();
            }

            ar.xmlAddNode_string("extensionString",_extensionString.c_str());

            if (exhaustiveXml)
            {
                std::string str(base64_encode((unsigned char*)_sceneUniquePersistentIdString.c_str(),_sceneUniquePersistentIdString.size()));
                ar.xmlAddNode_string("sceneUniquePersistentIdString_base64Coded",str.c_str());
            }
        }
        else
        {       // Loading
            if (exhaustiveXml)
            {
                ar.xmlGetNode_floats("ambientLight",ambientLightColor,3);

                if (ar.xmlPushChildNode("backgroundColor"))
                {
                    ar.xmlGetNode_floats("down",backGroundColorDown,3);
                    ar.xmlGetNode_floats("up",backGroundColor,3);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("ambientLight",rgb,3,exhaustiveXml))
                {
                    ambientLightColor[0]=float(rgb[0])/255.0f;
                    ambientLightColor[1]=float(rgb[1])/255.0f;
                    ambientLightColor[2]=float(rgb[2])/255.0f;
                }
                if (ar.xmlPushChildNode("backgroundColor",exhaustiveXml))
                {
                    if (ar.xmlGetNode_ints("down",rgb,3,exhaustiveXml))
                    {
                        backGroundColorDown[0]=float(rgb[0])/255.0f;
                        backGroundColorDown[1]=float(rgb[1])/255.0f;
                        backGroundColorDown[2]=float(rgb[2])/255.0f;
                    }
                    if (ar.xmlGetNode_ints("up",rgb,3,exhaustiveXml))
                    {
                        backGroundColor[0]=float(rgb[0])/255.0f;
                        backGroundColor[1]=float(rgb[1])/255.0f;
                        backGroundColor[2]=float(rgb[2])/255.0f;
                    }
                    ar.xmlPopNode();
                }
            }

            if (ar.xmlPushChildNode("fog",exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled",fogEnabled,exhaustiveXml);
                if (ar.xmlGetNode_enum("type",fogType,exhaustiveXml,"linear",0,"exp",1,"exp2",2))
                    setFogType(fogType);
                ar.xmlGetNode_2float("startEnd",fogStart,fogEnd,exhaustiveXml);
                ar.xmlGetNode_float("density",fogDensity,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_floats("color",fogBackgroundColor,3);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("color",rgb,3,exhaustiveXml))
                    {
                        fogBackgroundColor[0]=float(rgb[0])/255.0f;
                        fogBackgroundColor[1]=float(rgb[1])/255.0f;
                        fogBackgroundColor[2]=float(rgb[2])/255.0f;
                    }
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("shapeTexturesEnabled",_shapeTexturesEnabled,exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_string("acknowledgment",_acknowledgement,exhaustiveXml);

            if (exhaustiveXml&&ar.xmlPushChildNode("calcStruct"))
            {
                ar.xmlGetNode_bool("saveExisting",_saveExistingCalculationStructures);
                ar.xmlGetNode_float("maxTriSize",_calculationMaxTriangleSize);
                ar.xmlGetNode_float("minRelativeTriSize",_calculationMinRelTriangleSize);
                ar.xmlPopNode();
            }

            if (exhaustiveXml&&ar.xmlPushChildNode("wireless"))
            {
                if (ar.xmlPushChildNode("emitters"))
                {
                    ar.xmlGetNode_bool("visualize",_visualizeWirelessEmitters);
                    if (ar.xmlPushChildNode("color"))
                    {
                        wirelessEmissionVolumeColor.serialize(ar,1);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("receivers"))
                {
                    ar.xmlGetNode_bool("visualize",_visualizeWirelessReceivers);
                    if (ar.xmlPushChildNode("color"))
                    {
                        wirelessReceptionVolumeColor.serialize(ar,1);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }

            ar.xmlGetNode_string("extensionString",_extensionString,exhaustiveXml);

            if ( exhaustiveXml&&ar.xmlGetNode_string("sceneUniquePersistentIdString_base64Coded",_sceneUniquePersistentIdString))
                _sceneUniquePersistentIdString=base64_decode(_sceneUniquePersistentIdString);
        }
    }
}

void CEnvironment::setBackgroundColor(int viewSize[2])
{
    displayBackground(viewSize,fogEnabled,fogBackgroundColor,backGroundColorDown,backGroundColor);
}

void CEnvironment::activateAmbientLight(bool a)
{
    enableAmbientLight(a,ambientLightColor);
}

void CEnvironment::activateFogIfEnabled(CViewableBase* viewable,bool forDynamicContentOnly)
{
    if (fogEnabled&&viewable->getShowFogIfAvailable()&&(!forDynamicContentOnly)&&(App::getEditModeType()==NO_EDIT_MODE) )
    {
        activateFog(fogBackgroundColor,fogType,viewable->getFogStrength(),viewable->getFarClippingPlane(),fogStart,fogEnd,fogDensity);
        CViewableBase::fogWasActivated=true;
    }
    else
        deactivateFog();
}

void CEnvironment::deactivateFog()
{
    enableFog(false);
    CViewableBase::fogWasActivated=false;
}

void CEnvironment::temporarilyDeactivateFog()
{
    if (CViewableBase::fogWasActivated)
        enableFog(false);
}

void CEnvironment::reactivateFogThatWasTemporarilyDisabled()
{
    if (CViewableBase::fogWasActivated)
        enableFog(true);
}

