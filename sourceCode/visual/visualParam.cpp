
#include "vrepMainHeader.h"
#include "visualParam.h"
#include "app.h"
#include "vDateTime.h"
#include "tt.h"
#include "rendering.h"

CVisualParam::CVisualParam()
{
    setDefaultValues();
}

CVisualParam::~CVisualParam()
{

}

void CVisualParam::setFlash(bool flashIsOn)
{
    flash=flashIsOn;
}

bool CVisualParam::getFlash() const
{
    return(flash);
}

void CVisualParam::setDefaultValues()
{
    setColorsAllBlack();
    for (int i=6;i<9;i++)
        colors[i]=0.25f;
    colors[12]=0.5f; // this is the temperature channel (ambient temperature by default, i.e. not cold, not warm)
    translucid=false;
    flash=false;
    useSimulationTime=false;
    flashFrequency=2.0f;
    flashRatio=0.5f;
    flashPhase=0.0f;
    transparencyFactor=0.5f;
    shininess=48;
    colorName="";
    extensionString="povray { pattern {default}}";
}

void CVisualParam::setColorsAllBlack()
{
    for (int i=0;i<15;i++)
        colors[i]=0.0f;
}

void CVisualParam::setColor(const float theColor[3],unsigned char colorMode)
{
    int offset=0;
    if (colorMode==sim_colorcomponent_ambient_diffuse)
        offset=0;
    else if (colorMode==sim_colorcomponent_diffuse)
        offset=3;
    else if (colorMode==sim_colorcomponent_specular)
        offset=6;
    else if (colorMode==sim_colorcomponent_emission)
        offset=9;
    else if (colorMode==sim_colorcomponent_auxiliary)
        offset=12;
    for (int i=0;i<3;i++)
        colors[offset+i]=theColor[i];
}
void CVisualParam::setColor(float r,float g,float b,unsigned char colorMode)
{
    float col[3]={r,g,b};
    setColor(col,colorMode);
}

void CVisualParam::setConvexColors()
{
    setDefaultValues();
    colors[0]=0.72f;
    colors[1]=0.88f;
    colors[2]=0.35f;
}

void CVisualParam::copyYourselfInto(CVisualParam* it) const
{ // all is true by default
    for (int i=0;i<15;i++)
        it->colors[i]=colors[i];
    it->flash=flash;
    it->useSimulationTime=useSimulationTime;
    it->flashFrequency=flashFrequency;
    it->flashRatio=flashRatio;
    it->flashPhase=flashPhase;
    it->shininess=shininess;
    it->transparencyFactor=transparencyFactor;
    it->translucid=translucid;
    it->colorName=colorName;
    it->extensionString=extensionString;
}

void CVisualParam::serialize(CSer& ar,int objType)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            // Keep following for backward compatibility (5/10/2014):
            ar.storeDataName("Cl2");
            float c[15];
            for (int i=0;i<15;i++)
                c[i]=colors[i];
            if (objType<3)
            {
                c[0]=(c[0]*0.85f)-0.125f;
                c[1]=(c[1]*0.85f)-0.125f;
                c[2]=(c[2]*0.85f)-0.125f;
                c[3]=0.25f;
                c[4]=0.25f;
                c[5]=0.25f;
            }
            if (objType==3)
            {
                c[0]=c[3]/2.0f;
                c[1]=c[4]/2.0f;
                c[2]=c[5]/2.0f;
            }
            for (int i=0;i<15;i++)
                ar << c[i];
            ar.flush();

            ar.storeDataName("Cl3");
            for (int i=0;i<15;i++)
                ar << colors[i];
            ar.flush();

            ar.storeDataName("Sh2");
            ar << shininess;
            ar.flush();

            ar.storeDataName("Trf");
            ar << transparencyFactor;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
    //      FREE
    //      FREE
            SIM_SET_CLEAR_BIT(dummy,2,!translucid);
    //      FREE
            SIM_SET_CLEAR_BIT(dummy,4,flash);
            SIM_SET_CLEAR_BIT(dummy,5,useSimulationTime);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Fi3");
            ar << flashFrequency << flashRatio << flashPhase;
            ar.flush();

            ar.storeDataName("Cnm");
            ar << colorName;
            ar.flush();

            ar.storeDataName("Rst");
            ar << extensionString;
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
                    if (theName.compare("Cls")==0)
                    { // for backward compatibility 5/9/2014 (before 3.1.3)
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<12;i++)
                            ar >> colors[i];
                    }

                    if (theName.compare("Cl2")==0)
                    { // for backward compatibility 5/10/2014
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<15;i++)
                            ar >> colors[i];
                     }

                    if ( (theName.compare("Cls")==0)||(theName.compare("Cl2")==0) )
                    { // for backward compatibility (5/10/2014)
                        if (objType<3)
                        {
                            float avgDiff=(colors[3]+colors[4]+colors[5])/3.0f;
                            colors[0]=(colors[0]+avgDiff*0.5f)/0.85f;
                            colors[1]=(colors[1]+avgDiff*0.5f)/0.85f;
                            colors[2]=(colors[2]+avgDiff*0.5f)/0.85f;

                            colors[0]*=App::userSettings->colorAdjust_backCompatibility;
                            colors[1]*=App::userSettings->colorAdjust_backCompatibility;
                            colors[2]*=App::userSettings->colorAdjust_backCompatibility;

                            float mx=SIM_MAX(SIM_MAX(colors[0],colors[1]),colors[2]);
                            if (mx>1.0f)
                            {
                                colors[0]/=mx;
                                colors[1]/=mx;
                                colors[2]/=mx;
                            }
                            colors[3]=0.0f;
                            colors[4]=0.0f;
                            colors[5]=0.0f;
                        }
                        if (objType==3)
                        {
                            colors[0]=0.0f;
                            colors[1]=0.0f;
                            colors[2]=0.0f;
                            float avgDiff=(colors[3]+colors[4]+colors[5])/3.0f;
                            colors[3]=avgDiff;
                            colors[4]=avgDiff;
                            colors[5]=avgDiff;
                        }
                    }

                    if (theName.compare("Cl3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<15;i++)
                            ar >> colors[i];
                    }

                    if (theName.compare("Shi")==0)
                    { // for backward compatibility (around January-March 2012?)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> shininess;
                        if (shininess<48)
                            shininess=48;
                    }
                    if (theName.compare("Sh2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> shininess;
                    }
                    if (theName.compare("Trf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> transparencyFactor;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        translucid=!SIM_IS_BIT_SET(dummy,2);
                        flash=SIM_IS_BIT_SET(dummy,4);
                        useSimulationTime=SIM_IS_BIT_SET(dummy,5);
                    }
                    if (theName.compare("Fi3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> flashFrequency >> flashRatio >> flashPhase;
                    }
                    if (theName.compare("Cnm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> colorName;
                    }
                    if (theName.compare("Rst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> extensionString;
                    }
                    if (theName.compare("Pov")==0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        int povPatternType;
                        ar >> povPatternType;
                        extensionString="povray {pattern {";
                        extensionString+=_getPatternStringFromPatternId_backwardCompatibility_3_2_2016(povPatternType);
                        extensionString+="}}";
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

std::string CVisualParam::_getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id)
{
    if (id==sim_pov_pattern_null)
        return("default");
    if (id==sim_pov_white_marble)
        return("whiteMarble");
    if (id==sim_pov_blood_marble)
        return("bloodMarble");
    if (id==sim_pov_blue_agate)
        return("blueAgate");
    if (id==sim_pov_sapphire_agate)
        return("sapphireAgate");
    if (id==sim_pov_brown_agate)
        return("brownAgate");
    if (id==sim_pov_pink_granite)
        return("pinkGranite");
    if (id==sim_pov_pink_alabaster)
        return("pinkAlabaster");
    if (id==sim_pov_cherry_wood)
        return("cherryWood");
    if (id==sim_pov_pine_wood)
        return("pineWood");
    if (id==sim_pov_dark_wood)
        return("darkWood");
    if (id==sim_pov_tan_wood)
        return("tanWood");
    if (id==sim_pov_white_wood)
        return("whiteWood");
    if (id==sim_pov_tom_wood)
        return("tomWood");
    if (id==sim_pov_dmf_wood1)
        return("dmfWood1");
    if (id==sim_pov_dmf_wood2)
        return("dmfWood2");
    if (id==sim_pov_dmf_wood3)
        return("dmfWood3");
    if (id==sim_pov_dmf_wood4)
        return("dmfWood4");
    if (id==sim_pov_dmf_wood5)
        return("dmfWood5");
    if (id==sim_pov_dmf_wood6)
        return("dmfWood6");
    if (id==sim_pov_dmf_light_oak)
        return("dmfLightOak");
    if (id==sim_pov_dmf_dark_oak)
        return("dmfDarkOak");
    if (id==sim_pov_emb_wood1)
        return("embWood1");
    if (id==sim_pov_yellow_pine)
        return("yellowPine");
    if (id==sim_pov_rose_wood)
        return("roseWood");
    if (id==sim_pov_sandal_wood)
        return("sandalWood");
    if (id==sim_pov_glass)
        return("glass");
    if (id==sim_pov_glass2)
        return("glass2");
    if (id==sim_pov_glass3)
        return("glass3");
    if (id==sim_pov_green_glass)
        return("greenGlass");
    if (id==sim_pov_light_glass)
        return("lightGlass");
    if (id==sim_pov_bold_glass)
        return("boldGlass");
    if (id==sim_pov_wine_bottle)
        return("wineBottle");
    if (id==sim_pov_beer_bottle)
        return("beerBottle");
    if (id==sim_pov_ruby_glass)
        return("rubyGlass");
    if (id==sim_pov_blue_glass)
        return("blueGlass");
    if (id==sim_pov_yellow_glass)
        return("yellowGlass");
    if (id==sim_pov_orange_glass)
        return("orangeGlass");
    if (id==sim_pov_vicks_bottle_glass)
        return("vicksBottleGlass");
    if (id==sim_pov_chrome_metal)
        return("chromeMetal");
    if (id==sim_pov_brass_metal)
        return("brassMetal");
    if (id==sim_pov_copper_metal)
        return("copperMetal");
    if (id==sim_pov_bronze_metal)
        return("bronzeMetal");
    if (id==sim_pov_silver_metal)
        return("silverMetal");
    if (id==sim_pov_gold_metal)
        return("goldMetal");
    if (id==sim_pov_polished_chrome)
        return("polishedChrome");
    if (id==sim_pov_polished_brass)
        return("polishedBrass");
    if (id==sim_pov_new_brass)
        return("newBrass");
    if (id==sim_pov_spun_brass)
        return("spunBrass");
    if (id==sim_pov_brushed_aluminum)
        return("brushedAluminium");
    if (id==sim_pov_silver1)
        return("silver1");
    if (id==sim_pov_silver2)
        return("silver2");
    if (id==sim_pov_silver3)
        return("silver3");
    if (id==sim_pov_brass_valley)
        return("brassValley");
    if (id==sim_pov_rust)
        return("rust");
    if (id==sim_pov_rusty_iron)
        return("rustyIron");
    if (id==sim_pov_soft_silver)
        return("softSilver");
    if (id==sim_pov_new_penny)
        return("newPenny");
    if (id==sim_pov_tinny_brass)
        return("tinnyBrass");
    if (id==sim_pov_gold_nugget)
        return("goldNugget");
    if (id==sim_pov_aluminum)
        return("aluminium");
    if (id==sim_pov_bright_bronze)
        return("brightBronze");
    if (id==sim_pov_water)
        return("water");
    if (id==sim_pov_cork)
        return("cork");
    if (id==sim_pov_lightning)
        return("lightning");
    if (id==sim_pov_mirror)
        return("mirror");
    return("default");
}

void CVisualParam::makeCurrentColor(bool useAuxiliaryComponent) const
{
    makeCurrentColor2(false,useAuxiliaryComponent);
}

void CVisualParam::makeCurrentColor2(bool forceNonTransparent,bool useAuxiliaryComponent) const
{
    makeColorCurrent(this,forceNonTransparent,useAuxiliaryComponent);
}
