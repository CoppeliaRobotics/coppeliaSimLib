#include <colorObject.h>
#include <app.h>
#include <vDateTime.h>
#include <tt.h>
#include <rendering.h>

CColorObject::CColorObject()
{
    setDefaultValues();
}

CColorObject::~CColorObject()
{
}

void CColorObject::setFlash(bool flashIsOn)
{
    _flash=flashIsOn;
}

bool CColorObject::getFlash() const
{
    return(_flash);
}

void CColorObject::setUseSimulationTime(bool sim)
{
    _useSimulationTime=sim;
}

bool CColorObject::getUseSimulationTime() const
{
    return(_useSimulationTime);
}

void CColorObject::setFlashFrequency(float f)
{
    _flashFrequency=f;
}

float CColorObject::getFlashFrequency() const
{
    return(_flashFrequency);
}

void CColorObject::setFlashRatio(float f)
{
    _flashRatio=f;
}

float CColorObject::getFlashRatio() const
{
    return(_flashRatio);
}

void CColorObject::setFlashPhase(float f)
{
    _flashPhase=f;
}

float CColorObject::getFlashPhase() const
{
    return(_flashPhase);
}

void CColorObject::setDefaultValues()
{
    float col[15];
    for (size_t i=0;i<15;i++)
        col[i]=0.0;
    for (size_t i=6;i<9;i++)
        col[i]=0.25;
    col[12]=0.5; // temperature channel
    setColors(col);
    setTranslucid(false);
    setOpacity(0.5);
    setShininess(48);
    setColorName("");
    setExtensionString("povray { pattern {default}}");
    _flash=false;
    _useSimulationTime=false;
    _flashFrequency=2.0;
    _flashRatio=0.5;
    _flashPhase=0.0;
}

void CColorObject::setColorsAllBlack()
{
    float col[15];
    for (size_t i=0;i<15;i++)
        col[i]=0.0;
    setColors(col);
}

void CColorObject::getColor(float col[3],unsigned char colorMode) const
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
    const float* ptr=getColorsPtr();
    for (size_t i=0;i<3;i++)
        col[i]=ptr[offset+i];
}

void CColorObject::getNewColors(float cols[9]) const
{
    for (size_t i=0;i<3;i++)
    {
        cols[0+i]=_colors[0+i];
        cols[3+i]=_colors[6+i];
        cols[6+i]=_colors[9+i];
    }
}

void CColorObject::setColor(const float theColor[3],unsigned char colorMode)
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
    float col[15];
    getColors(col);
    for (size_t i=0;i<3;i++)
        col[offset+i]=theColor[i];
    setColors(col);
}

void CColorObject::pushShapeColorChangeEvent(int objectHandle,int colorIndex)
{
    if ( (objectHandle!=-1)&&App::worldContainer->getEventsEnabled() )
    {
        const char* cmd="color";
        auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(objectHandle,false,cmd,false);
        CInterfaceStackTable* sdata=new CInterfaceStackTable();
        data->appendMapObject_stringObject(cmd,sdata);
        float c[9];
        int w=sim_colorcomponent_ambient_diffuse;
        getColor(c+0,w);
        getColor(c+3,sim_colorcomponent_specular);
        getColor(c+6,sim_colorcomponent_emission);
        sdata->appendMapObject_stringFloatArray("color",c,9);
        float transp=0.0;
        if (_translucid)
            transp=1.0-_opacity;
        sdata->appendMapObject_stringFloat("transparency",transp);
        sdata->appendMapObject_stringInt32("index",colorIndex);
        App::worldContainer->pushEvent(event);
    }
}

void CColorObject::pushColorChangeEvent(int objectHandle,float col1[9],float col2[9],float col3[9],float col4[9])
{
    if ( (objectHandle!=-1)&&App::worldContainer->getEventsEnabled() )
    {
        const char* cmd="colors";
        auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(objectHandle,false,cmd,false);
        CInterfaceStackTable* sdata=new CInterfaceStackTable();
        data->appendMapObject_stringObject(cmd,sdata);
        sdata->appendArrayObject_floatArray(col1,9);
        if (col2!=nullptr)
            sdata->appendArrayObject_floatArray(col2,9);
        if (col3!=nullptr)
            sdata->appendArrayObject_floatArray(col3,9);
        if (col4!=nullptr)
            sdata->appendArrayObject_floatArray(col4,9);
        App::worldContainer->pushEvent(event);
    }
}

void CColorObject::setColor(float r,float g,float b,unsigned char colorMode)
{
    float col[3]={r,g,b};
    setColor(col,colorMode);
}

void CColorObject::setConvexColors()
{
    setDefaultValues();
    float col[15];
    getColors(col);
    col[0]=0.72f;
    col[1]=0.88f;
    col[2]=0.35f;
    setColors(col);
}

void CColorObject::copyYourselfInto(CColorObject* it) const
{
    it->setColors(_colors);
    it->setShininess(_shininess);
    it->setOpacity(_opacity);
    it->setTranslucid(_translucid);
    it->setColorName(_colorName.c_str());
    it->setExtensionString(_extensionString.c_str());

    it->_flash=_flash;
    it->_useSimulationTime=_useSimulationTime;
    it->_flashFrequency=_flashFrequency;
    it->_flashRatio=_flashRatio;
    it->_flashPhase=_flashPhase;
}

void CColorObject::serialize(CSer& ar,int objType)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            // Keep following for backward compatibility (5/10/2014):
            ar.storeDataName("Cl2");
            float c[15];
            for (int i=0;i<15;i++)
                c[i]=_colors[i];
            if (objType<3)
            {
                c[0]=(c[0]*0.85)-0.125;
                c[1]=(c[1]*0.85)-0.125;
                c[2]=(c[2]*0.85)-0.125;
                c[3]=0.25;
                c[4]=0.25;
                c[5]=0.25;
            }
            if (objType==3)
            {
                c[0]=c[3]/2.0;
                c[1]=c[4]/2.0;
                c[2]=c[5]/2.0;
            }
            for (int i=0;i<15;i++)
                ar << c[i];
            ar.flush();

            ar.storeDataName("Cl3");
            for (int i=0;i<15;i++)
                ar << _colors[i];
            ar.flush();

            ar.storeDataName("Sh2");
            ar << _shininess;
            ar.flush();

            ar.storeDataName("Trf");
            ar << _opacity;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char dummy=0;
    //      FREE
    //      FREE
            SIM_SET_CLEAR_BIT(dummy,2,!_translucid);
    //      FREE
            SIM_SET_CLEAR_BIT(dummy,4,_flash);
            SIM_SET_CLEAR_BIT(dummy,5,_useSimulationTime);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Fi3");
            ar << _flashFrequency << _flashRatio << _flashPhase;
            ar.flush();

            ar.storeDataName("Cnm");
            ar << _colorName;
            ar.flush();

            ar.storeDataName("Rst");
            ar << _extensionString;
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
                            ar >> _colors[i];
                    }

                    if (theName.compare("Cl2")==0)
                    { // for backward compatibility 5/10/2014
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<15;i++)
                            ar >> _colors[i];
                     }

                    if ( (theName.compare("Cls")==0)||(theName.compare("Cl2")==0) )
                    { // for backward compatibility (5/10/2014)
                        if (objType<3)
                        {
                            float avgDiff=(_colors[3]+_colors[4]+_colors[5])/3.0;
                            _colors[0]=(_colors[0]+avgDiff*0.5)/0.85;
                            _colors[1]=(_colors[1]+avgDiff*0.5)/0.85;
                            _colors[2]=(_colors[2]+avgDiff*0.5)/0.85;

                            _colors[0]*=App::userSettings->colorAdjust_backCompatibility;
                            _colors[1]*=App::userSettings->colorAdjust_backCompatibility;
                            _colors[2]*=App::userSettings->colorAdjust_backCompatibility;

                            float mx=std::max<float>(std::max<float>(_colors[0],_colors[1]),_colors[2]);
                            if (mx>1.0)
                            {
                                _colors[0]/=mx;
                                _colors[1]/=mx;
                                _colors[2]/=mx;
                            }
                            _colors[3]=0.0;
                            _colors[4]=0.0;
                            _colors[5]=0.0;
                        }
                        if (objType==3)
                        {
                            _colors[0]=0.0;
                            _colors[1]=0.0;
                            _colors[2]=0.0;
                            float avgDiff=(_colors[3]+_colors[4]+_colors[5])/3.0;
                            _colors[3]=avgDiff;
                            _colors[4]=avgDiff;
                            _colors[5]=avgDiff;
                        }
                    }

                    if (theName.compare("Cl3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        for (int i=0;i<15;i++)
                            ar >> _colors[i];
                    }

                    if (theName.compare("Shi")==0)
                    { // for backward compatibility (around January-March 2012?)
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _shininess;
                        if (_shininess<48)
                            _shininess=48;
                    }
                    if (theName.compare("Sh2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _shininess;
                    }
                    if (theName.compare("Trf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _opacity;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _translucid=!SIM_IS_BIT_SET(dummy,2);
                        _flash=SIM_IS_BIT_SET(dummy,4);
                        _useSimulationTime=SIM_IS_BIT_SET(dummy,5);
                    }
                    if (theName.compare("Fi3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _flashFrequency >> _flashRatio >> _flashPhase;
                    }
                    if (theName.compare("Cnm")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _colorName;
                    }
                    if (theName.compare("Rst")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _extensionString;
                    }
                    if (theName.compare("Pov")==0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        int povPatternType;
                        ar >> povPatternType;
                        _extensionString="povray {pattern {";
                        _extensionString+=_getPatternStringFromPatternId_backwardCompatibility_3_2_2016(povPatternType);
                        _extensionString+="}}";
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_string("name",_colorName.c_str());

            ar.xmlAddNode_string("extensionString",_extensionString.c_str());

            ar.xmlAddNode_floats("ambient",_colors+0,3);
            ar.xmlAddNode_floats("diffuse",_colors+3,3);
            ar.xmlAddNode_floats("specular",_colors+6,3);
            ar.xmlAddNode_floats("emission",_colors+9,3);
            ar.xmlAddNode_floats("temperature",_colors+12,3);

            ar.xmlAddNode_int("shininess",_shininess);

            ar.xmlAddNode_float("transparency",_opacity);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("transparent",_translucid);
            ar.xmlPopNode();

            ar.xmlPushNewNode("flashing");
            ar.xmlAddNode_bool("enabled",_flash);
            ar.xmlAddNode_bool("inSimulationTime",_useSimulationTime);
            ar.xmlAddNode_float("frequency",_flashFrequency);
            ar.xmlAddNode_float("ratio",_flashRatio);
            ar.xmlAddNode_float("phase",_flashPhase);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_string("name",_colorName);

            ar.xmlGetNode_string("extensionString",_extensionString);

            ar.xmlGetNode_floats("ambient",_colors+0,3);
            ar.xmlGetNode_floats("diffuse",_colors+3,3);
            ar.xmlGetNode_floats("specular",_colors+6,3);
            ar.xmlGetNode_floats("emission",_colors+9,3);
            ar.xmlGetNode_floats("temperature",_colors+12,3);

            ar.xmlGetNode_int("shininess",_shininess);

            ar.xmlGetNode_float("transparency",_opacity);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("transparent",_translucid);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("flashing"))
            {
                ar.xmlGetNode_bool("enabled",_flash);
                ar.xmlGetNode_bool("inSimulationTime",_useSimulationTime);
                ar.xmlGetNode_float("frequency",_flashFrequency);
                ar.xmlGetNode_float("ratio",_flashRatio);
                ar.xmlGetNode_float("phase",_flashPhase);
                ar.xmlPopNode();
            }

        }
    }
}

std::string CColorObject::_getPatternStringFromPatternId_backwardCompatibility_3_2_2016(int id)
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

void CColorObject::makeCurrentColor(bool useAuxiliaryComponent) const
{
    makeCurrentColor2(false,useAuxiliaryComponent);
}

void CColorObject::makeCurrentColor2(bool forceNonTransparent,bool useAuxiliaryComponent) const
{
    makeColorCurrent(this,forceNonTransparent,useAuxiliaryComponent);
}

bool CColorObject::getTranslucid() const
{
    return(_translucid);
}

float CColorObject::getOpacity() const
{
    return(_opacity);
}

int CColorObject::getShininess() const
{
    return(_shininess);
}

std::string CColorObject::getColorName() const
{
    return(_colorName);
}

std::string CColorObject::getExtensionString() const
{
    return(_extensionString);
}

void CColorObject::getColors(float col[15]) const
{
    for (size_t i=0;i<15;i++)
        col[i]=_colors[i];
}

const float* CColorObject::getColorsPtr() const
{
    return(_colors);
}

float* CColorObject::getColorsPtr()
{
    return(_colors);
}

bool CColorObject::_isSame(const CColorObject* it) const
{
    bool retVal=true;
    for (size_t i=0;i<15;i++)
    {
        if (it->_colors[i]!=_colors[i])
        {
            retVal=false;
            break;
        }
    }
    retVal=retVal&&(_shininess==it->_shininess);
    retVal=retVal&&(_opacity==it->_opacity);
    retVal=retVal&&(_translucid==it->_translucid);
    retVal=retVal&&(_colorName==it->_colorName);
    retVal=retVal&&(_extensionString==it->_extensionString);
    return(retVal);
}

void CColorObject::setColors(const float col[15])
{
    bool diff=false;
    for (size_t i=0;i<15;i++)
    {
        if (_colors[i]!=col[i])
        {
            diff=true;
            break;
        }
    }
    if (diff)
    {
        for (size_t i=0;i<15;i++)
            _colors[i]=col[i];
    }
}

void CColorObject::setTranslucid(bool e)
{
    bool diff=(_translucid!=e);
    if (diff)
        _translucid=e;
}

void CColorObject::setOpacity(float e)
{
    bool diff=(_opacity!=e);
    if (diff)
        _opacity=e;
}

void CColorObject::setShininess(int e)
{
    bool diff=(_shininess!=e);
    if (diff)
        _shininess=e;
}

void CColorObject::setColorName(const char* nm)
{
    bool diff=(_colorName!=nm);
    if (diff)
        _colorName=nm;
}

void CColorObject::setExtensionString(const char* nm)
{
    bool diff=(_extensionString!=nm);
    if (diff)
        _extensionString=nm;
}
