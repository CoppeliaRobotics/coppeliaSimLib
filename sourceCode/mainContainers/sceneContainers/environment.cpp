#include <simInternal.h>
#include <environment.h>
#include <tt.h>
#include <global.h>
#include <fileOperations.h>
#include <sceneObjectOperations.h>
#include <vDateTime.h>
#include <utils.h>
#include <app.h>
#include <base64.h>
#include <boost/algorithm/string.hpp>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <environmentRendering.h>
#include <guiApp.h>
#endif

int CEnvironment::_nextSceneUniqueID = 0;
bool CEnvironment::_shapeTexturesTemporarilyDisabled = false;
bool CEnvironment::_shapeEdgesTemporarilyDisabled = false;
bool CEnvironment::_customUisTemporarilyDisabled = false;

CEnvironment::CEnvironment()
{
    _sceneIsClosingFlag = false;
    setUpDefaultValues();
    _sceneUniqueID = _nextSceneUniqueID++;
    generateNewUniquePersistentIdString();
}

CEnvironment::~CEnvironment()
{ // beware, the current world could be nullptr
}

void CEnvironment::generateNewUniquePersistentIdString()
{
    _sceneUniquePersistentIdString = utils::generateUniqueAlphaNumericString();
}

std::string CEnvironment::getUniquePersistentIdString() const
{
    return (_sceneUniquePersistentIdString);
}

int CEnvironment::getNextSceneUniqueId()
{
    return (_nextSceneUniqueID);
}

bool CEnvironment::getSceneIsClosingFlag() const
{
    return (_sceneIsClosingFlag);
}

void CEnvironment::setSceneIsClosingFlag(bool closing)
{
    _sceneIsClosingFlag = closing;
}

void CEnvironment::setShapeTexturesTemporarilyDisabled(bool d)
{
    _shapeTexturesTemporarilyDisabled = d;
}

bool CEnvironment::getShapeTexturesTemporarilyDisabled()
{
    return (_shapeTexturesTemporarilyDisabled);
}

void CEnvironment::setShapeEdgesTemporarilyDisabled(bool d)
{
    _shapeEdgesTemporarilyDisabled = d;
}

bool CEnvironment::getShapeEdgesTemporarilyDisabled()
{
    return (_shapeEdgesTemporarilyDisabled);
}

void CEnvironment::setCustomUisTemporarilyDisabled(bool d)
{
    _customUisTemporarilyDisabled = d;
}

bool CEnvironment::getCustomUisTemporarilyDisabled()
{
    return (_customUisTemporarilyDisabled);
}

void CEnvironment::setSceneCanBeDiscardedWhenNewSceneOpened(bool canBeDiscarded)
{
    _sceneCanBeDiscardedWhenNewSceneOpened = canBeDiscarded;
}

bool CEnvironment::getSceneCanBeDiscardedWhenNewSceneOpened() const
{
    return (_sceneCanBeDiscardedWhenNewSceneOpened);
}

void CEnvironment::setUpDefaultValues()
{
    _scenePathAndName = "";
    _activeLayers = 0x00ff;
    _nonAmbientLightsAreActive = false;
    fogEnabled = false;
    fogDensity = 0.5;
    fogStart = 0.0;
    fogEnd = 5.0;
    fogType = 0; // 0=linear, 1=exp, 2=exp2

    _sceneCanBeDiscardedWhenNewSceneOpened = false;
    autoSaveLastSaveTimeInSecondsSince1970 = VDateTime::getSecondsSince1970();

    backGroundColor[0] = 0.72f;
    backGroundColor[1] = 0.81f;
    backGroundColor[2] = 0.88f;
    backGroundColorDown[0] = 0.05f;
    backGroundColorDown[1] = 0.05f;
    backGroundColorDown[2] = 0.1f;

    fogBackgroundColor[0] = 0.0f;
    fogBackgroundColor[1] = 0.0f;
    fogBackgroundColor[2] = 0.0f;

    ambientLightColor[0] = 0.2f;
    ambientLightColor[1] = 0.2f;
    ambientLightColor[2] = 0.2f;

    _extensionString = "povray {fogDist {4.00} fogTransp {0.50}}";

    wirelessEmissionVolumeColor.setColorsAllBlack();
    wirelessEmissionVolumeColor.setColor(1.0f, 1.0f, 0.0f, sim_colorcomponent_emission);
    wirelessEmissionVolumeColor.setTranslucid(true);
    wirelessEmissionVolumeColor.setOpacity(0.2f);
    wirelessReceptionVolumeColor.setColorsAllBlack();
    wirelessReceptionVolumeColor.setColor(1.0f, 1.0f, 0.0f, sim_colorcomponent_emission);

    _acknowledgement = "";
    _visualizeWirelessEmitters = false;
    _visualizeWirelessReceivers = false;
    _requestFinalSave = false;
    _sceneIsLocked = false;
    _shapeTexturesEnabled = true;
    _2DElementTexturesEnabled = true;
    _calculationMaxTriangleSize = 0.3; // from 0.8 to 0.3 on 2010/07/07
    _calculationMinRelTriangleSize = 0.02;
    _saveExistingCalculationStructures = false;
}

void CEnvironment::appendGenesisData(CCbor *ev) const
{
    ev->appendKeyBool(propScene_finalSaveRequest.name, _requestFinalSave);
    ev->appendKeyBool(propScene_sceneIsLocked.name, _sceneIsLocked);
    ev->appendKeyBool(propScene_saveCalculationStructs.name, _saveExistingCalculationStructures);
    ev->appendKeyInt(propScene_sceneUid.name, _sceneUniqueID);
    ev->appendKeyInt(propScene_visibilityLayers.name, _activeLayers);
    ev->appendKeyText(propScene_scenePath.name, _scenePathAndName.c_str());
    ev->appendKeyText(propScene_acknowledgment.name, _acknowledgement.c_str());
    ev->appendKeyText(propScene_sceneUidString.name, _sceneUniquePersistentIdString.c_str());
    ev->appendKeyFloatArray(propScene_ambientLight.name, ambientLightColor, 3);
}

void CEnvironment::setAmbientLight(const float c[3])
{
    bool diff = (ambientLightColor[0] != c[0]) || (ambientLightColor[1] != c[1]) || (ambientLightColor[2] != c[2]);
    if (diff)
    {
        for (size_t i = 0; i < 3; i++)
            ambientLightColor[i] = c[i];
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_ambientLight.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyFloatArray(cmd, ambientLightColor, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CEnvironment::setActiveLayers(int l)
{
    bool diff = (_activeLayers != l);
    if (diff)
    {
        _activeLayers = l;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_visibilityLayers.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyInt(cmd, _activeLayers);
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        GuiApp::setRefreshHierarchyViewFlag();
        GuiApp::setLightDialogRefreshFlag();
#endif
    }
}

int CEnvironment::getActiveLayers() const
{
    return (_activeLayers);
}

bool CEnvironment::areNonAmbientLightsActive() const
{
    return (_nonAmbientLightsAreActive);
}

void CEnvironment::setNonAmbientLightsActive(bool a)
{
    _nonAmbientLightsAreActive = a;
}

int CEnvironment::getSceneUniqueID() const
{
    return (_sceneUniqueID);
}

void CEnvironment::setSaveExistingCalculationStructures(bool s)
{
    bool diff = (_saveExistingCalculationStructures != s);
    if (diff)
    {
        _saveExistingCalculationStructures = s;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_saveCalculationStructs.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyBool(cmd, _saveExistingCalculationStructures);
            App::worldContainer->pushEvent();
        }
    }
}

bool CEnvironment::getSaveExistingCalculationStructures() const
{
    return (_saveExistingCalculationStructures);
}

void CEnvironment::setSaveExistingCalculationStructuresTemp(bool s)
{
    _saveExistingCalculationStructuresTemp = s;
}

bool CEnvironment::getSaveExistingCalculationStructuresTemp() const
{
    return (_saveExistingCalculationStructuresTemp);
}

void CEnvironment::setCalculationMaxTriangleSize(double s)
{
    _calculationMaxTriangleSize = tt::getLimitedFloat(0.01, 100.0, s);
}

double CEnvironment::getCalculationMaxTriangleSize() const
{
    return (_calculationMaxTriangleSize);
}

void CEnvironment::setCalculationMinRelTriangleSize(double s)
{
    _calculationMinRelTriangleSize = tt::getLimitedFloat(0.001, 1.0, s);
}

double CEnvironment::getCalculationMinRelTriangleSize() const
{
    return (_calculationMinRelTriangleSize);
}

void CEnvironment::setShapeTexturesEnabled(bool e)
{
    _shapeTexturesEnabled = e;
}
bool CEnvironment::getShapeTexturesEnabled() const
{
    return (_shapeTexturesEnabled);
}

std::string CEnvironment::getExtensionString() const
{
    return (_extensionString);
}

void CEnvironment::setExtensionString(const char *str)
{
    _extensionString = str;
}

void CEnvironment::set2DElementTexturesEnabled(bool e)
{
    _2DElementTexturesEnabled = e;
}
bool CEnvironment::get2DElementTexturesEnabled() const
{
    return (_2DElementTexturesEnabled);
}

void CEnvironment::setRequestFinalSave(bool finalSaveActivated)
{
    bool diff = (_requestFinalSave != finalSaveActivated);
    if (diff)
    {
        _requestFinalSave = finalSaveActivated;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_finalSaveRequest.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyBool(cmd, _requestFinalSave);
            App::worldContainer->pushEvent();
        }
    }
}

bool CEnvironment::getRequestFinalSave() const
{
    return (_requestFinalSave);
}

void CEnvironment::setSceneLocked()
{
    if (!_sceneIsLocked)
    {
        setRequestFinalSave(false);
        _sceneIsLocked = true;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_sceneIsLocked.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyBool(cmd, _sceneIsLocked);
            App::worldContainer->pushEvent();
        }
    }
}

bool CEnvironment::getSceneLocked() const
{
    return (_sceneIsLocked);
}

void CEnvironment::setVisualizeWirelessEmitters(bool v)
{
    _visualizeWirelessEmitters = v;
}

bool CEnvironment::getVisualizeWirelessEmitters() const
{
    return (_visualizeWirelessEmitters);
}

void CEnvironment::setVisualizeWirelessReceivers(bool v)
{
    _visualizeWirelessReceivers = v;
}

bool CEnvironment::getVisualizeWirelessReceivers() const
{
    return (_visualizeWirelessReceivers);
}

void CEnvironment::setFogEnabled(bool e)
{
    fogEnabled = e;
}

bool CEnvironment::getFogEnabled() const
{
    return (fogEnabled);
}

void CEnvironment::setFogDensity(double d)
{
    tt::limitValue(0.0, 1000.0, d);
    fogDensity = d;
}

double CEnvironment::getFogDensity() const
{
    return (fogDensity);
}

void CEnvironment::setFogStart(double s)
{
    tt::limitValue(0.0, 1000.0, s); // "inverted" is allowed in opengl!
    fogStart = s;
}

double CEnvironment::getFogStart() const
{
    return (fogStart);
}

void CEnvironment::setFogEnd(double e)
{
    tt::limitValue(0.01, 1000.0, e); // "inverted" is allowed in opengl!
    fogEnd = e;
}

double CEnvironment::getFogEnd() const
{
    return (fogEnd);
}

void CEnvironment::setFogType(double t)
{
    tt::limitValue(0, 2, t);
    fogType = t;
}

int CEnvironment::getFogType() const
{
    return (fogType);
}

void CEnvironment::setAcknowledgement(const char *a)
{
    std::string ack(a);
    if (ack.length() > 3000)
        ack.erase(ack.begin() + 2999, ack.end());
    bool diff = (_acknowledgement != ack);
    if (diff)
    {
        _acknowledgement = ack;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_acknowledgment.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyText(cmd, _acknowledgement.c_str());
            App::worldContainer->pushEvent();
        }
    }
}

std::string CEnvironment::getAcknowledgement() const
{
    return (_acknowledgement);
}

void CEnvironment::serialize(CSer &ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_dn");
            ar << fogEnd;
            ar.flush();

            ar.storeDataName("Vil");
            ar << (unsigned short) _activeLayers;
            ar.flush();

            ar.storeDataName("_d2");
            ar << fogType;
            ar << fogStart << fogDensity;
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
            ar << (float)(ambientLightColor[0] * 0.5) << (float)(ambientLightColor[1] * 0.5)
               << (float)(ambientLightColor[2] * 0.5);
            ar.flush();

            ar.storeDataName("Al2");
            ar << ambientLightColor[0] << ambientLightColor[1] << ambientLightColor[2];
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, !_2DElementTexturesEnabled);
            SIM_SET_CLEAR_BIT(dummy, 1, _saveExistingCalculationStructures);
            SIM_SET_CLEAR_BIT(dummy, 2, _visualizeWirelessEmitters);
            SIM_SET_CLEAR_BIT(dummy, 3, !_visualizeWirelessReceivers);
            SIM_SET_CLEAR_BIT(dummy, 4, fogEnabled);
            SIM_SET_CLEAR_BIT(dummy, 5, _sceneIsLocked);
            SIM_SET_CLEAR_BIT(dummy, 6, _requestFinalSave); // needed so that undo/redo works on that item too!
            SIM_SET_CLEAR_BIT(dummy, 7, !_shapeTexturesEnabled);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va3");
            dummy = 0;
            // free        SIM_SET_CLEAR_BIT(dummy,0,_showPartRepository);
            // free       SIM_SET_CLEAR_BIT(dummy,1,_showPalletRepository);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Ack");
            ar << _acknowledgement;
            ar.flush();

            ar.storeDataName("_t2");
            ar << _calculationMaxTriangleSize;
            ar.flush();

            ar.storeDataName("_rs");
            ar << _calculationMinRelTriangleSize;
            ar.flush();

            ar.storeDataName("Rst");
            ar << _extensionString;
            ar.flush();

            ar.storeDataName("Evc");
            ar.setCountingMode();
            wirelessEmissionVolumeColor.serialize(ar, 1);
            if (ar.setWritingMode())
                wirelessEmissionVolumeColor.serialize(ar, 1);

            ar.storeDataName("Wtc");
            ar.setCountingMode();
            wirelessReceptionVolumeColor.serialize(ar, 1);
            if (ar.setWritingMode())
                wirelessReceptionVolumeColor.serialize(ar, 1);

            ar.storeDataName("Ups");
            ar << _sceneUniquePersistentIdString;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Fdn") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        fogEnd = (double)bla;
                    }

                    if (theName.compare("_dn") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> fogEnd;
                    }

                    if (theName.compare("Vil") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned short al;
                        ar >> al;
                        _activeLayers = (int)al;
                    }
                    if (theName.compare("Fd2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> fogType;
                        float bla, bli;
                        ar >> bla >> bli;
                        fogStart = (double)bla;
                        fogDensity = (double)bli;
                    }

                    if (theName.compare("_d2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> fogType;
                        ar >> fogStart >> fogDensity;
                    }

                    if (theName.compare("Var") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _2DElementTexturesEnabled = !SIM_IS_BIT_SET(dummy, 0);
                        _saveExistingCalculationStructures = SIM_IS_BIT_SET(dummy, 1);
                        _visualizeWirelessEmitters = SIM_IS_BIT_SET(dummy, 2);
                        _visualizeWirelessReceivers = !SIM_IS_BIT_SET(dummy, 3);
                        fogEnabled = SIM_IS_BIT_SET(dummy, 4);
                        _sceneIsLocked = SIM_IS_BIT_SET(dummy, 5);
                        _requestFinalSave =
                            SIM_IS_BIT_SET(dummy, 6); // needed so that undo/redo works on that item too!
                        _shapeTexturesEnabled = !SIM_IS_BIT_SET(dummy, 7);
                    }
                    if (theName.compare("Va2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                    }
                    if (theName.compare("Va3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // free                    _showPartRepository=SIM_IS_BIT_SET(dummy,0);
                        // free                    _showPalletRepository=SIM_IS_BIT_SET(dummy,1);
                    }
                    if (theName.compare("Clc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> backGroundColor[0] >> backGroundColor[1] >> backGroundColor[2];
                        backGroundColorDown[0] = backGroundColor[0];
                        backGroundColorDown[1] = backGroundColor[1];
                        backGroundColorDown[2] = backGroundColor[2];
                    }
                    if (theName.compare("Cld") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> backGroundColorDown[0] >> backGroundColorDown[1] >> backGroundColorDown[2];
                    }
                    if (theName.compare("Fbg") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> fogBackgroundColor[0] >> fogBackgroundColor[1] >> fogBackgroundColor[2];
                    }
                    if (theName.compare("Alc") == 0)
                    { // keep for backward/forward compatibility (5/10/2014)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo;
                        ar >> bla >> bli >> blo;
                        ambientLightColor[0] = bla;
                        ambientLightColor[1] = bli;
                        ambientLightColor[2] = blo;
                        ambientLightColor[0] *= 2.0;
                        ambientLightColor[1] *= 2.0;
                        ambientLightColor[2] *= 2.0;
                        float mx = std::max<float>(std::max<float>(ambientLightColor[0], ambientLightColor[1]),
                                                   ambientLightColor[2]);
                        if (mx > 0.4)
                        {
                            ambientLightColor[0] = ambientLightColor[0] * 0.4 / mx;
                            ambientLightColor[1] = ambientLightColor[1] * 0.4 / mx;
                            ambientLightColor[2] = ambientLightColor[2] * 0.4 / mx;
                        }
                    }
                    if (theName.compare("Al2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> ambientLightColor[0] >> ambientLightColor[1] >> ambientLightColor[2];
                    }
                    if (theName.compare("Ack") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _acknowledgement;
                        if (CSimFlavor::getBoolVal(18))
                        {
                        }
                    }
                    if (theName.compare("Mts") == 0)
                    { // keep for backward compatibility (2010/07/07)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _calculationMaxTriangleSize = 0.3;
                    }
                    if (theName.compare("Mt2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _calculationMaxTriangleSize = (double)bla;
                    }

                    if (theName.compare("_t2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _calculationMaxTriangleSize;
                    }

                    if (theName.compare("Mrs") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _calculationMinRelTriangleSize = (double)bla;
                    }

                    if (theName.compare("_rs") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _calculationMinRelTriangleSize;
                    }

                    if (theName.compare("Rst") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _extensionString;
                    }
                    if (theName.compare("Pof") == 0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float povDist, povTransp;
                        ar >> povDist >> povTransp;
                        _extensionString = "povray {fogDist {";
                        _extensionString += utils::getSizeString(false, (double)povDist);
                        _extensionString += "} fogTransp {";
                        _extensionString += utils::getSizeString(false, (double)povTransp);
                        _extensionString += "}}";
                    }
                    if (theName.compare("Evc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        wirelessEmissionVolumeColor.serialize(ar, 1);
                    }
                    if (theName.compare("Wtc") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        wirelessReceptionVolumeColor.serialize(ar, 1);
                    }
                    if (theName.compare("Ups") == 0)
                    {
                        noHit = false;
                        std::string s;
                        ar >> byteQuantity;
                        ar >> s;
                        if (s.size() != 0)
                        {
                            _sceneUniquePersistentIdString = s;
                            if (!utils::isAlphaNumeric(_sceneUniquePersistentIdString))
                                generateNewUniquePersistentIdString();
                        }
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }

            if (ar.getSerializationVersionThatWroteThisFile() < 17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                double avg = (ambientLightColor[0] + ambientLightColor[1] + ambientLightColor[2]) / 3.0;
                if (avg > 0.21)
                    utils::scaleLightDown_(ambientLightColor);
            }
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        { // Storing
            if (exhaustiveXml)
            {
                ar.xmlAddNode_floats("ambientLight", ambientLightColor, 3);

                ar.xmlPushNewNode("backgroundColor");
                ar.xmlAddNode_floats("down", backGroundColorDown, 3);
                ar.xmlAddNode_floats("up", backGroundColor, 3);
                ar.xmlPopNode();
            }
            else
            {
                ar.xmlAddNode_3int("ambientLight", int(255.1 * ambientLightColor[0]), int(255.1 * ambientLightColor[1]),
                                   int(255.1 * ambientLightColor[2]));

                ar.xmlPushNewNode("backgroundColor");
                ar.xmlAddNode_3int("down", int(255.1 * backGroundColorDown[0]), int(255.1 * backGroundColorDown[1]),
                                   int(255.1 * backGroundColorDown[2]));
                ar.xmlAddNode_3int("up", int(255.1 * backGroundColor[0]), int(255.1 * backGroundColor[1]),
                                   int(255.1 * backGroundColor[2]));
                ar.xmlPopNode();
            }

            ar.xmlAddNode_int("visibleLayers", _activeLayers);

            ar.xmlPushNewNode("fog");
            ar.xmlAddNode_bool("enabled", fogEnabled);
            ar.xmlAddNode_comment(" 'type' tag: can be 'linear', 'exp' or 'exp2' ", exhaustiveXml);
            ar.xmlAddNode_enum("type", fogType, 0, "linear", 1, "exp", 2, "exp2");
            ar.xmlAddNode_2float("startEnd", fogStart, fogEnd);
            ar.xmlAddNode_float("density", fogDensity);
            if (exhaustiveXml)
                ar.xmlAddNode_floats("color", fogBackgroundColor, 3);
            else
                ar.xmlAddNode_3int("color", int(255.1 * fogBackgroundColor[0]), int(255.1 * fogBackgroundColor[1]),
                                   int(255.1 * fogBackgroundColor[2]));
            ar.xmlPopNode();

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("shapeTexturesEnabled", _shapeTexturesEnabled);
            ar.xmlPopNode();

            ar.xmlAddNode_string("acknowledgment", _acknowledgement.c_str());

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("calcStruct");
                ar.xmlAddNode_bool("saveExisting", _saveExistingCalculationStructures);
                ar.xmlAddNode_float("maxTriSize", _calculationMaxTriangleSize);
                ar.xmlAddNode_float("minRelativeTriSize", _calculationMinRelTriangleSize);
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("wireless");
                ar.xmlPushNewNode("emitters");
                ar.xmlAddNode_bool("visualize", _visualizeWirelessEmitters);
                ar.xmlPushNewNode("color");
                wirelessEmissionVolumeColor.serialize(ar, 1);
                ar.xmlPopNode();
                ar.xmlPopNode();
                ar.xmlPushNewNode("receivers");
                ar.xmlAddNode_bool("visualize", _visualizeWirelessReceivers);
                ar.xmlPushNewNode("color");
                wirelessReceptionVolumeColor.serialize(ar, 1);
                ar.xmlPopNode();
                ar.xmlPopNode();
                ar.xmlPopNode();
            }

            ar.xmlAddNode_string("extensionString", _extensionString.c_str());

            if (exhaustiveXml)
            {
                std::string str(base64_encode((unsigned char *)_sceneUniquePersistentIdString.c_str(),
                                              _sceneUniquePersistentIdString.size()));
                ar.xmlAddNode_string("sceneUniquePersistentIdString_base64Coded", str.c_str());
            }
        }
        else
        { // Loading
            if (exhaustiveXml)
            {
                ar.xmlGetNode_floats("ambientLight", ambientLightColor, 3);

                if (ar.xmlPushChildNode("backgroundColor"))
                {
                    ar.xmlGetNode_floats("down", backGroundColorDown, 3);
                    ar.xmlGetNode_floats("up", backGroundColor, 3);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("ambientLight", rgb, 3, exhaustiveXml))
                {
                    ambientLightColor[0] = double(rgb[0]) / 255.0;
                    ambientLightColor[1] = double(rgb[1]) / 255.0;
                    ambientLightColor[2] = double(rgb[2]) / 255.0;
                }
                if (ar.xmlPushChildNode("backgroundColor", exhaustiveXml))
                {
                    if (ar.xmlGetNode_ints("down", rgb, 3, exhaustiveXml))
                    {
                        backGroundColorDown[0] = double(rgb[0]) / 255.0;
                        backGroundColorDown[1] = double(rgb[1]) / 255.0;
                        backGroundColorDown[2] = double(rgb[2]) / 255.0;
                    }
                    if (ar.xmlGetNode_ints("up", rgb, 3, exhaustiveXml))
                    {
                        backGroundColor[0] = double(rgb[0]) / 255.0;
                        backGroundColor[1] = double(rgb[1]) / 255.0;
                        backGroundColor[2] = double(rgb[2]) / 255.0;
                    }
                    ar.xmlPopNode();
                }
            }

            int l;
            if (ar.xmlGetNode_int("visibleLayers", l, exhaustiveXml))
            {
                tt::limitValue(0, 65526, l);
                _activeLayers = l;
            }

            if (ar.xmlPushChildNode("fog", exhaustiveXml))
            {
                ar.xmlGetNode_bool("enabled", fogEnabled, exhaustiveXml);
                if (ar.xmlGetNode_enum("type", fogType, exhaustiveXml, "linear", 0, "exp", 1, "exp2", 2))
                    setFogType(fogType);
                ar.xmlGetNode_2float("startEnd", fogStart, fogEnd, exhaustiveXml);
                ar.xmlGetNode_float("density", fogDensity, exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_floats("color", fogBackgroundColor, 3);
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("color", rgb, 3, exhaustiveXml))
                    {
                        fogBackgroundColor[0] = double(rgb[0]) / 255.0;
                        fogBackgroundColor[1] = double(rgb[1]) / 255.0;
                        fogBackgroundColor[2] = double(rgb[2]) / 255.0;
                    }
                }
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("shapeTexturesEnabled", _shapeTexturesEnabled, exhaustiveXml);
                ar.xmlPopNode();
            }

            ar.xmlGetNode_string("acknowledgment", _acknowledgement, exhaustiveXml);

            if (exhaustiveXml && ar.xmlPushChildNode("calcStruct"))
            {
                ar.xmlGetNode_bool("saveExisting", _saveExistingCalculationStructures);
                ar.xmlGetNode_float("maxTriSize", _calculationMaxTriangleSize);
                ar.xmlGetNode_float("minRelativeTriSize", _calculationMinRelTriangleSize);
                ar.xmlPopNode();
            }

            if (exhaustiveXml && ar.xmlPushChildNode("wireless"))
            {
                if (ar.xmlPushChildNode("emitters"))
                {
                    ar.xmlGetNode_bool("visualize", _visualizeWirelessEmitters);
                    if (ar.xmlPushChildNode("color"))
                    {
                        wirelessEmissionVolumeColor.serialize(ar, 1);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("receivers"))
                {
                    ar.xmlGetNode_bool("visualize", _visualizeWirelessReceivers);
                    if (ar.xmlPushChildNode("color"))
                    {
                        wirelessReceptionVolumeColor.serialize(ar, 1);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }

            ar.xmlGetNode_string("extensionString", _extensionString, exhaustiveXml);

            if (exhaustiveXml &&
                ar.xmlGetNode_string("sceneUniquePersistentIdString_base64Coded", _sceneUniquePersistentIdString))
            {
                _sceneUniquePersistentIdString = base64_decode(_sceneUniquePersistentIdString);
                if (!utils::isAlphaNumeric(_sceneUniquePersistentIdString))
                    generateNewUniquePersistentIdString();
            }
        }
    }
}

#ifdef SIM_WITH_GUI
void CEnvironment::setBackgroundColor(int viewSize[2])
{
    displayBackground(viewSize, fogEnabled, fogBackgroundColor, backGroundColorDown, backGroundColor);
}

void CEnvironment::activateAmbientLight(bool a)
{
    enableAmbientLight(a, ambientLightColor);
}

void CEnvironment::activateFogIfEnabled(CViewableBase *viewable, bool forDynamicContentOnly)
{
    int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
    editMode = GuiApp::getEditModeType();
#endif
    if (fogEnabled && viewable->getShowFogIfAvailable() && (!forDynamicContentOnly) && (editMode == NO_EDIT_MODE))
    {
        double np, fp;
        viewable->getClippingPlanes(np, fp);
        activateFog(fogBackgroundColor, fogType, viewable->getFogStrength(), fp, fogStart, fogEnd, fogDensity);
        CViewableBase::fogWasActivated = true;
    }
    else
        deactivateFog();
}

void CEnvironment::deactivateFog()
{
    enableFog(false);
    CViewableBase::fogWasActivated = false;
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
#endif

void CEnvironment::setScenePathAndName(const char *pathAndName)
{
    bool diff = (_scenePathAndName != pathAndName);
    if (diff)
    {
        _scenePathAndName = pathAndName;
        if (App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propScene_scenePath.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
            ev->appendKeyText(cmd, _scenePathAndName.c_str());
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = NEW_SCENE_NAME_UITHREADCMD;
        cmdIn.stringParams.push_back(getSceneNameForUi());
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif
    }
}

std::string CEnvironment::getScenePathAndName() const
{
    return (_scenePathAndName);
}

std::string CEnvironment::getScenePath() const
{
    return (VVarious::splitPath_path(_scenePathAndName.c_str()));
}

std::string CEnvironment::getSceneName() const
{
    return (VVarious::splitPath_fileBase(_scenePathAndName.c_str()));
}

std::string CEnvironment::getSceneNameForUi() const
{
    if (_scenePathAndName == "")
        return ("new scene");
    return (getSceneName());
}

std::string CEnvironment::getSceneNameWithExt() const
{
    return (VVarious::splitPath_fileBaseAndExtension(_scenePathAndName.c_str()));
}

int CEnvironment::setBoolProperty(const char* pName, bool pState)
{
    int retVal = -1;

    if (strcmp(pName, propScene_finalSaveRequest.name) == 0)
    {
        retVal = 1;
        setRequestFinalSave(pState);
    }
    else if (strcmp(pName, propScene_saveCalculationStructs.name) == 0)
    {
        retVal = 1;
        setSaveExistingCalculationStructures(pState);
    }

    return retVal;
}

int CEnvironment::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propScene_finalSaveRequest.name) == 0)
    {
        retVal = 1;
        pState = _requestFinalSave;
    }
    else if (strcmp(pName, propScene_saveCalculationStructs.name) == 0)
    {
        retVal = 1;
        pState = _saveExistingCalculationStructures;
    }
    else if (strcmp(pName, propScene_sceneIsLocked.name) == 0)
    {
        retVal = 1;
        pState = _sceneIsLocked;
    }

    return retVal;
}

int CEnvironment::setIntProperty(const char* pName, int pState)
{
    int retVal = -1;

    if (strcmp(pName, propScene_visibilityLayers.name) == 0)
    {
        setActiveLayers(pState);
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::getIntProperty(const char* pName, int& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propScene_sceneUid.name) == 0)
    {
        pState = _sceneUniqueID;
        retVal = 1;
    }
    else if (strcmp(pName, propScene_visibilityLayers.name) == 0)
    {
        pState = _activeLayers;
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::setStringProperty(const char* pName, const char* pState)
{
    int retVal = -1;

    if (strcmp(pName, propScene_scenePath.name) == 0)
    {
        setScenePathAndName(pState);
        retVal = 1;
    }
    else if (strcmp(pName, propScene_acknowledgment.name) == 0)
    {
        setAcknowledgement(pState);
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propScene_scenePath.name) == 0)
    {
        pState = _scenePathAndName;
        retVal = 1;
    }
    else if (strcmp(pName, propScene_acknowledgment.name) == 0)
    {
        pState = _acknowledgement;
        retVal = 1;
    }
    else if (strcmp(pName, propScene_sceneUidString.name) == 0)
    {
        pState = _sceneUniquePersistentIdString;
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::setColorProperty(const char* pName, const float* pState)
{
    int retVal = -1;

    if (strcmp(pName, propScene_ambientLight.name) == 0)
    {
        setAmbientLight(pState);
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::getColorProperty(const char* pName, float* pState) const
{
    int retVal = -1;

    if (strcmp(pName, propScene_ambientLight.name) == 0)
    {
        for (size_t i = 0; i < 3; i++)
            pState[i] = ambientLightColor[i];
        retVal = 1;
    }

    return retVal;
}

int CEnvironment::getPropertyName(int& index, std::string& pName)
{
    int retVal = -1;

    for (size_t i = 0; i < allProps_scene.size(); i++)
    {
        index--;
        if (index == -1)
        {
            pName = allProps_scene[i].name;
            //pName = "scene." + pName;
            retVal = 1;
            break;
        }
    }

    return retVal;
}

int CEnvironment::getPropertyInfo(const char* pName, int& info)
{
    int retVal = -1;

    for (size_t i = 0; i < allProps_scene.size(); i++)
    {
        if (strcmp(allProps_scene[i].name, pName) == 0)
        {
            retVal = allProps_scene[i].type;
            info = allProps_scene[i].flags;
            break;
        }
    }

    return retVal;
}

