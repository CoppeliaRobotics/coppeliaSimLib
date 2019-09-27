
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "mainSettings.h"
#include "global.h"
#include "tt.h"
#include "app.h"
#include "vVarious.h"

CMainSettings::CMainSettings()
{
    setUpDefaultValues();
}

CMainSettings::~CMainSettings()
{

}

void CMainSettings::simulationAboutToStart()
{

}

void CMainSettings::simulationEnded()
{

}

void CMainSettings::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CMainSettings::setUpDefaultValues()
{
    _scenePathAndName="";
    _activeLayers=0x00ff;

    infoWindowColorStyle=0; // default
    statusBoxOpenState=false;
    infoWindowOpenState=true;
    
    collisionColor.setDefaultValues();
    collisionColor.setColor(0.0f,0.0f,0.0f,sim_colorcomponent_ambient_diffuse);
    collisionColor.setColor(1.0f,0.0f,0.0f,sim_colorcomponent_emission);

    proximitySensorsEnabled=true;
    visionSensorsEnabled=true;
    ikCalculationEnabled=true;
    gcsCalculationEnabled=true;
    collisionDetectionEnabled=true;
    distanceCalculationEnabled=true;
    jointMotionHandlingEnabled_DEPRECATED=true;
    pathMotionHandlingEnabled_DEPRECATED=true;
    millsEnabled=true;
    mirrorsDisabled=false;
    clippingPlanesDisabled=false;

    forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers=false;
}

void CMainSettings::setActiveLayers(unsigned short l)
{
    _activeLayers=l;
    App::setRefreshHierarchyViewFlag();
    App::setLightDialogRefreshFlag();
}

unsigned short CMainSettings::getActiveLayers()
{
    return(_activeLayers);
}

void CMainSettings::setScenePathAndName(const char* pathAndName)
{
    _scenePathAndName=pathAndName;
#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=NEW_SCENE_NAME_UITHREADCMD;
    cmdIn.stringParams.push_back(getSceneNameForUi());
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

}

std::string CMainSettings::getScenePathAndName() const
{
    return(_scenePathAndName);
}

std::string CMainSettings::getScenePath() const
{
    return(VVarious::splitPath_path(_scenePathAndName));
}

std::string CMainSettings::getSceneName() const
{
    return(VVarious::splitPath_fileBase(_scenePathAndName));
}

std::string CMainSettings::getSceneNameForUi() const
{
    if (_scenePathAndName=="")
        return("new scene");
    return(getSceneName());
}

std::string CMainSettings::getSceneNameWithExt() const
{
    return(VVarious::splitPath_fileBaseAndExtension(_scenePathAndName));
}

void CMainSettings::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing

            ar.storeDataName("Va5");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,proximitySensorsEnabled);
            SIM_SET_CLEAR_BIT(dummy,1,statusBoxOpenState);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va2");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,!infoWindowOpenState);
            SIM_SET_CLEAR_BIT(dummy,1,mirrorsDisabled);
            SIM_SET_CLEAR_BIT(dummy,2,clippingPlanesDisabled);
            SIM_SET_CLEAR_BIT(dummy,3,ikCalculationEnabled);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va3");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,gcsCalculationEnabled);
            SIM_SET_CLEAR_BIT(dummy,1,collisionDetectionEnabled);
            SIM_SET_CLEAR_BIT(dummy,2,distanceCalculationEnabled);
            SIM_SET_CLEAR_BIT(dummy,3,!jointMotionHandlingEnabled_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,4,!pathMotionHandlingEnabled_DEPRECATED);
            SIM_SET_CLEAR_BIT(dummy,5,!visionSensorsEnabled);
            SIM_SET_CLEAR_BIT(dummy,6,!millsEnabled);
    // 27/11/2012       SIM_SET_CLEAR_BIT(dummy,7,!_dynamicsEnabled);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Crc");
            ar.setCountingMode();
            collisionColor.serialize(ar,0);
            if (ar.setWritingMode())
                collisionColor.serialize(ar,0);

            ar.storeDataName("Al2");
            ar << _activeLayers;
            ar.flush();

            ar.storeDataName("Iwc");
            ar << infoWindowColorStyle;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Va1")==0)
                    { // Keep for backward compatibility (3/1/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;

                        bool bulletUseDefault=!SIM_IS_BIT_SET(dummy,0);
                        bool _displayContactPoints=SIM_IS_BIT_SET(dummy,2);
                        App::ct->dynamicsContainer->setDisplayContactPoints(_displayContactPoints);
                        bool odeUseDefault=!SIM_IS_BIT_SET(dummy,3);
                        proximitySensorsEnabled=SIM_IS_BIT_SET(dummy,5);
                        bool _dynamicODEUseQuickStep=!SIM_IS_BIT_SET(dummy,6);
                        statusBoxOpenState=SIM_IS_BIT_SET(dummy,7);

                        App::ct->dynamicsContainer->setEngineBoolParam(sim_ode_global_quickstep,_dynamicODEUseQuickStep,true);
                        int _dynamicDefaultTypeCalculationParameters=1; // precise.
                        if ((!bulletUseDefault)||(!odeUseDefault))
                            _dynamicDefaultTypeCalculationParameters=4; // custom
                        App::ct->dynamicsContainer->setUseDynamicDefaultCalculationParameters(_dynamicDefaultTypeCalculationParameters);

                        forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers=true;
                    }

                    if (theName.compare("Ss2")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicsBULLETStepSize,_dynamicsODEStepSize;
                        ar >> _dynamicsBULLETStepSize >> _dynamicsODEStepSize;
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_stepsize,_dynamicsBULLETStepSize,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_stepsize,_dynamicsODEStepSize,true);
                    }

                    if (theName.compare("Ddi")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        int _dynamicDefaultTypeCalculationParameters;
                        ar >> _dynamicDefaultTypeCalculationParameters;
                        App::ct->dynamicsContainer->setUseDynamicDefaultCalculationParameters(_dynamicDefaultTypeCalculationParameters);

                    }

                    if (theName.compare("Va4")==0)
                    { // for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        bool _displayContactPoints=SIM_IS_BIT_SET(dummy,2);
                        App::ct->dynamicsContainer->setDisplayContactPoints(_displayContactPoints);
                        proximitySensorsEnabled=SIM_IS_BIT_SET(dummy,5);
                        bool _dynamicODEUseQuickStep=!SIM_IS_BIT_SET(dummy,6);
                        App::ct->dynamicsContainer->setEngineBoolParam(sim_ode_global_quickstep,_dynamicODEUseQuickStep,true);
                        statusBoxOpenState=SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va5")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        proximitySensorsEnabled=SIM_IS_BIT_SET(dummy,0);
                        statusBoxOpenState=SIM_IS_BIT_SET(dummy,1);
                    }

                    if (theName.compare("Va2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        infoWindowOpenState=!SIM_IS_BIT_SET(dummy,0);
                        mirrorsDisabled=SIM_IS_BIT_SET(dummy,1);
                        clippingPlanesDisabled=SIM_IS_BIT_SET(dummy,2);
                        ikCalculationEnabled=SIM_IS_BIT_SET(dummy,3);
                    }

                    if (theName.compare("Va3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        gcsCalculationEnabled=SIM_IS_BIT_SET(dummy,0);
                        collisionDetectionEnabled=SIM_IS_BIT_SET(dummy,1);
                        distanceCalculationEnabled=SIM_IS_BIT_SET(dummy,2);
                        jointMotionHandlingEnabled_DEPRECATED=!SIM_IS_BIT_SET(dummy,3);
                        pathMotionHandlingEnabled_DEPRECATED=!SIM_IS_BIT_SET(dummy,4);
                        visionSensorsEnabled=!SIM_IS_BIT_SET(dummy,5);
                        millsEnabled=!SIM_IS_BIT_SET(dummy,6);
                        bool _dynamicsEnabled=!SIM_IS_BIT_SET(dummy,7);
                        App::ct->dynamicsContainer->setDynamicsEnabled(_dynamicsEnabled);
                    }
                    if (theName.compare("Crc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        collisionColor.serialize(ar,0);
                    }
                    if (theName.compare("Al2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _activeLayers;
                    }
                    if (theName.compare("Iwc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> infoWindowColorStyle;
                    }
                    if (theName.compare("Dsd")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012;
                    }
                    if (theName.compare("Dis")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicBULLETInternalScalingFactor;
                        ar >> _dynamicBULLETInternalScalingFactor;
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_internalscalingfactor,_dynamicBULLETInternalScalingFactor,true);
                    }
                    if (theName.compare("Dcs")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        int _dynamicBULLETConstraintSolvingIterations;
                        ar >> _dynamicBULLETConstraintSolvingIterations;
                        App::ct->dynamicsContainer->setEngineIntParam(sim_bullet_global_constraintsolvingiterations,_dynamicBULLETConstraintSolvingIterations,true);
                    }
                    if (theName.compare("Gvy")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        C3Vector _gravity;
                        ar >> _gravity(0) >> _gravity(1) >> _gravity(2);
                        App::ct->dynamicsContainer->setGravity(_gravity);
                    }
                    if (theName.compare("Od1")==0)
                    { // for backward compatibility (3/1/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicODEInternalScalingFactor,_dynamicODEGlobalERP,_dynamicODEGlobalCFM;
                        int _dynamicODEConstraintSolvingIterations;
                        ar >> dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012 >> _dynamicODEInternalScalingFactor >> _dynamicODEConstraintSolvingIterations >> _dynamicODEGlobalERP >> _dynamicODEGlobalCFM;
                        App::ct->dynamicsContainer->setEngineIntParam(sim_ode_global_constraintsolvingiterations,_dynamicODEConstraintSolvingIterations,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_internalscalingfactor,_dynamicODEInternalScalingFactor,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_erp,_dynamicODEGlobalERP,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_cfm,_dynamicODEGlobalCFM,true);
                    }
                    if (theName.compare("Od2")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicODEInternalScalingFactor,_dynamicODEGlobalERP,_dynamicODEGlobalCFM;
                        int _dynamicODEConstraintSolvingIterations;
                        ar >> _dynamicODEInternalScalingFactor >> _dynamicODEConstraintSolvingIterations >> _dynamicODEGlobalERP >> _dynamicODEGlobalCFM;
                        App::ct->dynamicsContainer->setEngineIntParam(sim_ode_global_constraintsolvingiterations,_dynamicODEConstraintSolvingIterations,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_internalscalingfactor,_dynamicODEInternalScalingFactor,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_erp,_dynamicODEGlobalERP,true);
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_cfm,_dynamicODEGlobalCFM,true);
                    }
                    if (theName.compare("Deu")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        int _dynamicEngineToUse;
                        ar >> _dynamicEngineToUse;
                        App::ct->dynamicsContainer->setDynamicEngineType(_dynamicEngineToUse,0);
                    }
                    if (theName.compare("Bcm")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicBULLETCollisionMarginFactor;
                        ar >> _dynamicBULLETCollisionMarginFactor;
                        App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_collisionmarginfactor,_dynamicBULLETCollisionMarginFactor,true);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

