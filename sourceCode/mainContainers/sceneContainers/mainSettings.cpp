#include "simInternal.h"
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
{ // beware, the current world could be nullptr
}

void CMainSettings::setUpDefaultValues()
{
    _scenePathAndName="";

    infoWindowColorStyle=0; // default
    statusBoxOpenState=false;
    infoWindowOpenState=true;
    
    collisionColor.setDefaultValues();
    collisionColor.setColor(0.0,0.0,0.0,sim_colorcomponent_ambient_diffuse);
    collisionColor.setColor(1.0,0.0,0.0,sim_colorcomponent_emission);

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
    return(VVarious::splitPath_path(_scenePathAndName.c_str()));
}

std::string CMainSettings::getSceneName() const
{
    return(VVarious::splitPath_fileBase(_scenePathAndName.c_str()));
}

std::string CMainSettings::getSceneNameForUi() const
{
    if (_scenePathAndName=="")
        return("new scene");
    return(getSceneName());
}

std::string CMainSettings::getSceneNameWithExt() const
{
    return(VVarious::splitPath_fileBaseAndExtension(_scenePathAndName.c_str()));
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

            ar.storeDataName("Al2"); // Kept for backward compatibility
            ar << App::currentWorld->environment->getActiveLayers();
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
                        App::currentWorld->dynamicsContainer->setDisplayContactPoints(_displayContactPoints);
                        bool odeUseDefault=!SIM_IS_BIT_SET(dummy,3);
                        proximitySensorsEnabled=SIM_IS_BIT_SET(dummy,5);
                        bool _dynamicODEUseQuickStep=!SIM_IS_BIT_SET(dummy,6);
                        statusBoxOpenState=SIM_IS_BIT_SET(dummy,7);
                        if ((!bulletUseDefault)||(!odeUseDefault))
                            App::currentWorld->dynamicsContainer->setEngineBoolParam(sim_ode_global_quickstep,_dynamicODEUseQuickStep);
                        forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers=true;
                    }

                    if (theName.compare("Ss2")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicsBULLETStepSize,_dynamicsODEStepSize;
                        ar >> _dynamicsBULLETStepSize >> _dynamicsODEStepSize;
                        App::currentWorld->dynamicsContainer->setDesiredStepSize((double)_dynamicsBULLETStepSize);
                    }

                    if (theName.compare("Va4")==0)
                    { // for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        bool _displayContactPoints=SIM_IS_BIT_SET(dummy,2);
                        App::currentWorld->dynamicsContainer->setDisplayContactPoints(_displayContactPoints);
                        proximitySensorsEnabled=SIM_IS_BIT_SET(dummy,5);
                        bool _dynamicODEUseQuickStep=!SIM_IS_BIT_SET(dummy,6);
                        App::currentWorld->dynamicsContainer->setEngineBoolParam(sim_ode_global_quickstep,_dynamicODEUseQuickStep);
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
                        App::currentWorld->dynamicsContainer->setDynamicsEnabled(_dynamicsEnabled);
                    }
                    if (theName.compare("Crc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        collisionColor.serialize(ar,0);
                    }
                    if (theName.compare("Al2")==0)
                    { // For backward compatibility
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned short l;
                        ar >> l;
                        App::currentWorld->environment->setActiveLayers(l);
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
                        float bla;
                        ar >> bla;
                        dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012=(double)bla;
                    }
                    if (theName.compare("Dis")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicBULLETInternalScalingFactor;
                        ar >> _dynamicBULLETInternalScalingFactor;
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_bullet_global_internalscalingfactor,(double)_dynamicBULLETInternalScalingFactor);
                    }
                    if (theName.compare("Dcs")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        int _dynamicBULLETConstraintSolvingIterations;
                        ar >> _dynamicBULLETConstraintSolvingIterations;
                        App::currentWorld->dynamicsContainer->setEngineIntParam(sim_bullet_global_constraintsolvingiterations,_dynamicBULLETConstraintSolvingIterations);
                    }
                    if (theName.compare("Gvy")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        C3Vector _gravity;
                        for (size_t i=0;i<3;i++)
                        {
                            float bla;
                            ar >> bla;
                            _gravity(i)=(double)bla;
                        }
                        App::currentWorld->dynamicsContainer->setGravity(_gravity);
                    }
                    if (theName.compare("Od1")==0)
                    { // for backward compatibility (3/1/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicODEInternalScalingFactor,_dynamicODEGlobalERP,_dynamicODEGlobalCFM;
                        int _dynamicODEConstraintSolvingIterations;
                        ar >> dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012;
                        ar >> _dynamicODEInternalScalingFactor;
                        ar >> _dynamicODEConstraintSolvingIterations;
                        ar >> _dynamicODEGlobalERP >> _dynamicODEGlobalCFM;
                        App::currentWorld->dynamicsContainer->setEngineIntParam(sim_ode_global_constraintsolvingiterations,_dynamicODEConstraintSolvingIterations);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_internalscalingfactor,(double)_dynamicODEInternalScalingFactor);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_erp,(double)_dynamicODEGlobalERP);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_cfm,(double)_dynamicODEGlobalCFM);
                    }
                    if (theName.compare("Od2")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicODEInternalScalingFactor,_dynamicODEGlobalERP,_dynamicODEGlobalCFM;
                        int _dynamicODEConstraintSolvingIterations;
                        ar >> _dynamicODEInternalScalingFactor;
                        ar >> _dynamicODEConstraintSolvingIterations;
                        ar >> _dynamicODEGlobalERP >> _dynamicODEGlobalCFM;
                        App::currentWorld->dynamicsContainer->setEngineIntParam(sim_ode_global_constraintsolvingiterations,_dynamicODEConstraintSolvingIterations);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_internalscalingfactor,(double)_dynamicODEInternalScalingFactor);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_erp,(double)_dynamicODEGlobalERP);
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_ode_global_cfm,(double)_dynamicODEGlobalCFM);
                    }
                    if (theName.compare("Deu")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        int _dynamicEngineToUse;
                        ar >> _dynamicEngineToUse;
                        App::currentWorld->dynamicsContainer->setDynamicEngineType(_dynamicEngineToUse,0);
                    }
                    if (theName.compare("Bcm")==0)
                    { // Keep for backward compatibility (27/11/2012)
                        noHit=false;
                        ar >> byteQuantity;
                        float _dynamicBULLETCollisionMarginFactor;
                        ar >> _dynamicBULLETCollisionMarginFactor;
                        App::currentWorld->dynamicsContainer->setEngineFloatParam(sim_bullet_global_collisionmarginfactor,(double)_dynamicBULLETCollisionMarginFactor);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_comment(" 'visibleLayers' tag: deprecated. Use equivalent in 'environment' section ",exhaustiveXml);
            ar.xmlAddNode_int("visibleLayers",App::currentWorld->environment->getActiveLayers()); // Kept for backward compatibility

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("visionSensorsEnabled",visionSensorsEnabled);
            ar.xmlAddNode_bool("proximitySensorsEnabled",proximitySensorsEnabled);
            ar.xmlAddNode_bool("mirrorsEnabled",!mirrorsDisabled);
            ar.xmlAddNode_bool("clippingPlanesEnabled",!clippingPlanesDisabled);
            ar.xmlAddNode_bool("ikEnabled",ikCalculationEnabled);
            ar.xmlAddNode_bool("collisionDetectionsEnabled",collisionDetectionEnabled);
            ar.xmlAddNode_bool("distanceCalculationsEnabled",distanceCalculationEnabled);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("collisionColor");
                collisionColor.serialize(ar,0);
                ar.xmlPopNode();
            }
        }
        else
        {

            int l;
            if (ar.xmlGetNode_int("visibleLayers",l,exhaustiveXml))
            { // For backward compatibility
                tt::limitValue(0,65526,l);
                App::currentWorld->environment->setActiveLayers((unsigned short)l);
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("visionSensorsEnabled",visionSensorsEnabled,exhaustiveXml);
                ar.xmlGetNode_bool("proximitySensorsEnabled",proximitySensorsEnabled,exhaustiveXml);
                if (ar.xmlGetNode_bool("mirrorsEnabled",mirrorsDisabled,exhaustiveXml))
                    mirrorsDisabled=!mirrorsDisabled;
                if (ar.xmlGetNode_bool("clippingPlanesEnabled",clippingPlanesDisabled,exhaustiveXml))
                    clippingPlanesDisabled=!clippingPlanesDisabled;
                ar.xmlGetNode_bool("ikEnabled",ikCalculationEnabled,exhaustiveXml);
                ar.xmlGetNode_bool("collisionDetectionsEnabled",collisionDetectionEnabled,exhaustiveXml);
                ar.xmlGetNode_bool("distanceCalculationsEnabled",distanceCalculationEnabled,exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml&&ar.xmlPushChildNode("collisionColor"))
            {
                collisionColor.serialize(ar,0);
                ar.xmlPopNode();
            }
        }
    }
}

