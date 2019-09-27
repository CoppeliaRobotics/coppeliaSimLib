#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "environment.h"
#include "tt.h"
#include "global.h"
#include "fileOperations.h"
#include "sceneObjectOperations.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "app.h"
#include "environmentRendering.h"

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
{
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
    wirelessEmissionVolumeColor.translucid=true;
    wirelessEmissionVolumeColor.transparencyFactor=0.2f;
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
    _showPartRepository=false;
    _showPalletRepository=false;
    _jobFuncEnabled=false;
    _currentJob="default";
    _jobs.clear();
    _jobs.push_back(_currentJob);
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
//  App::ct->objCont->resetDialogRefreshFlags();
}

bool CEnvironment::getSceneLocked() const
{
    return(_sceneIsLocked);
}

void CEnvironment::simulationAboutToStart()
{

}

void CEnvironment::simulationEnded()
{
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }

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

void CEnvironment::setAcknowledgement(const std::string& a)
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

            ar.storeDataName("Job");
            ar << _currentJob;
            ar << int(_jobs.size());
            for (size_t i=0;i<_jobs.size();i++)
                ar << _jobs[i];
            ar.flush();

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
                        float mx=SIM_MAX(SIM_MAX(ambientLightColor[0],ambientLightColor[1]),ambientLightColor[2]);
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
                    if (theName.compare("Job")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        ar >> _currentJob;
                        int cnt;
                        ar >> cnt;
                        _jobs.clear();
                        for (int i=0;i<cnt;i++)
                        {
                            std::string job;
                            ar >> job;
                            _jobs.push_back(job);
                        }
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
}

void CEnvironment::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
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

std::string CEnvironment::getCurrentJob() const
{
    return(_currentJob);
}

bool CEnvironment::setCurrentJob(const char* jobName)
{
    for (size_t i=0;i<_jobs.size();i++)
    {
        if (_jobs[i].compare(jobName)==0)
        {
            _currentJob=jobName;
            return(true);
        }
    }
    return(false);
}

int CEnvironment::getJobCount()
{
    return(int(_jobs.size()));
}

int CEnvironment::getJobIndex(const std::string& name)
{
    for (int i=0;i<int(_jobs.size());i++)
    {
        if (_jobs[i].compare(name)==0)
            return(i);
    }
    return(-1);
}

std::string CEnvironment::getJobAtIndex(int index)
{
    if ( (index>=0)&&(index<int(_jobs.size())) )
        return(_jobs[index]);
    return("");
}

bool CEnvironment::createNewJob(std::string newName)
{
    if (getJobIndex(newName)==-1)
    {
        _currentJob=newName;
        _jobs.push_back(_currentJob);
        return(true);
    }
    return(false);
}

bool CEnvironment::deleteCurrentJob()
{
    if (_jobs.size()>1)
    {
        int ind=getJobIndex(_currentJob);
        if (ind>=0)
        {
            _jobs.erase(_jobs.begin()+ind);
            if (int(_jobs.size())<=ind)
                ind--;
            _currentJob=_jobs[ind];
            return(true);
        }
    }
    return(false);
}

bool CEnvironment::renameCurrentJob(std::string newName)
{
    tt::removeIllegalCharacters(newName,false);
    if (getJobIndex(newName)==-1)
    {
        int ind=getJobIndex(_currentJob);
        _jobs[ind]=newName;
        _currentJob=newName;
        return(true);
    }
    return(false);
}

bool CEnvironment::switchJob(int index)
{
    if ( (index>=0)&&(index<int(_jobs.size())) )
    {
        if (_jobs[index].compare(_currentJob)!=0)
        {
            _currentJob=_jobs[index];
            return(true);
        }
    }
    return(false);
}

bool CEnvironment::getJobFunctionalityEnabled()
{
    return(_jobFuncEnabled);    
}

void CEnvironment::setJobFunctionalityEnabled(bool en)
{
    _jobFuncEnabled=en;
}

void CEnvironment::setShowPartRepository(bool en)
{
    _showPartRepository=en;
}

bool CEnvironment::getShowPartRepository()
{
    return(_showPartRepository);
}

void CEnvironment::setShowPalletRepository(bool en)
{
    _showPalletRepository=en;
}

bool CEnvironment::getShowPalletRepository()
{
    return(_showPalletRepository);
}

#ifdef SIM_WITH_GUI
void CEnvironment::addLayoutMenu(VMenu* menu)
{ // GUI THREAD only
    bool simStopped=App::ct->simulation->isSimulationStopped();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    menu->appendMenuItem(noEditMode,false,BR_COMMAND_1_SCCMD+5,"General properties");
    menu->appendMenuItem(simStopped&&noEditMode,false,BR_COMMAND_1_SCCMD+0,"Actions");

    menu->appendMenuItem(noEditMode,_showPartRepository,BR_COMMAND_1_SCCMD+3,"Part repository",true);
    menu->appendMenuItem(noEditMode,_showPalletRepository,BR_COMMAND_1_SCCMD+4,"Pallet repository",true);
    // BR_COMMAND_1_SCCMD+11 is for the verify layout toolbar button
}

void CEnvironment::addJobsMenu(VMenu* menu)
{ // GUI THREAD only
    bool enabled=App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE)&&_jobFuncEnabled;
    menu->appendMenuItem((_jobs.size()<99)&&enabled,false,BR_COMMAND_1_SCCMD+297,"Create new job");
    menu->appendMenuItem((_jobs.size()>1)&&enabled,false,BR_COMMAND_1_SCCMD+298,"Delete current job");
    menu->appendMenuItem(enabled,false,BR_COMMAND_1_SCCMD+299,"Rename current job");
    menu->appendMenuSeparator();
    for (size_t i=0;i<_jobs.size();i++)
    {
        std::string tmp("Job '");
        tmp+=_jobs[i];
        tmp+="'";
        menu->appendMenuItem(enabled,_currentJob.compare(_jobs[i])==0,BR_COMMAND_1_SCCMD+300+int(i),tmp.c_str(),true);
    }
}

bool CEnvironment::processGuiCommand(int commandID)
{ // GUI THREAD only. Return value is true if the command belonged to object edition menu and was executed
    if ( (commandID>=BR_COMMAND_1_SCCMD)&&(commandID<BR_COMMANDS_END_SCCMD) )
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=commandID;
        App::appendSimulationThreadCommand(cmd);
        return(true);
    }
    return(false);
}
#endif
