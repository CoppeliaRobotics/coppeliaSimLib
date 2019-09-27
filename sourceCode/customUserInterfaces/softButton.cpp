
// This file requires some serious refactoring!!

#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "softButton.h"
#include "tt.h"
#include "global.h"
#include "app.h"
#ifdef SIM_WITH_GUI
#include "oGL.h"
#endif

CSoftButton::CSoftButton(std::string theLabel,int w,int h,int theLength,int theHeight)
{
    backgroundColor[0]=0.7f;
    backgroundColor[1]=0.7f;
    backgroundColor[2]=0.7f;
    downBackgroundColor[0]=0.7f; 
    downBackgroundColor[1]=0.7f;
    downBackgroundColor[2]=0.7f;
    textColor[0]=0.0f;
    textColor[1]=0.0f;
    textColor[2]=0.0f;
    label=theLabel;
    downLabel=theLabel;
    _textureProperty=nullptr;
    arrayColors=nullptr;
    _buttonAttributes=sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered;
    sliderPos=0.0f;
    if (theLength<1)
        theLength=1;
    length=theLength;
    height=theHeight;
    xPos=w;
    yPos=h;
    _initialValuesInitialized=false;
    _buttonUniqueID=_nextButtonUniqueID++;
}

void CSoftButton::setAttributes(int attr)
{
    int beforeType=_buttonAttributes&0x0007;
    int beforeProperty=_buttonAttributes&(0xffff-0x0007);
    _buttonAttributes=attr;
    int afterType=attr&0x0007;
    int afterProperty=attr&(0xffff-0x0007);
    if ( (afterType==sim_buttonproperty_editbox)&&(beforeType!=sim_buttonproperty_editbox) )
    {
        downBackgroundColor[0]=1.0f;
        downBackgroundColor[1]=1.0f;
        downBackgroundColor[2]=1.0f;
    }
    if ( (afterType!=sim_buttonproperty_editbox)&&(beforeType==sim_buttonproperty_editbox) )
    {
        downBackgroundColor[0]=0.7f;
        downBackgroundColor[1]=0.7f;
        downBackgroundColor[2]=0.7f;
    }
    if (afterType!=sim_buttonproperty_button)
        _buttonAttributes&=(0xffff-sim_buttonproperty_staydown-sim_buttonproperty_isdown-sim_buttonproperty_rollupaction-sim_buttonproperty_closeaction-sim_buttonproperty_downupevent);
    if ((_buttonAttributes&sim_buttonproperty_staydown)==0)
        _buttonAttributes&=(0xffff-sim_buttonproperty_rollupaction);
    else
        _buttonAttributes&=(0xffff-sim_buttonproperty_downupevent);
    if ( (beforeProperty&sim_buttonproperty_rollupaction)&&(afterProperty&sim_buttonproperty_closeaction) )
        _buttonAttributes&=(0xffff-sim_buttonproperty_rollupaction);
    if ( (beforeProperty&sim_buttonproperty_closeaction)&&(afterProperty&sim_buttonproperty_rollupaction) )
        _buttonAttributes&=(0xffff-sim_buttonproperty_closeaction);
}

int CSoftButton::getAttributes()
{
    return(_buttonAttributes);
}

void CSoftButton::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialSimulation_label=label;
        _initialSimulation_buttonAttributes=_buttonAttributes;
        _initialSimulation_sliderPos=sliderPos;
    }
}

void CSoftButton::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        _buttonAttributes=_initialSimulation_buttonAttributes;
        label=_initialSimulation_label;
        sliderPos=_initialSimulation_sliderPos;
    }
    _initialValuesInitialized=false;
}


CSoftButton::~CSoftButton()
{
    delete _textureProperty;
    delete[] arrayColors;
}

void CSoftButton::announce3DObjectWillBeErased(int objID)
{
    if (_textureProperty!=nullptr)
    {
        if (_textureProperty->announceObjectWillBeErased(objID))
        {
            delete _textureProperty;
            _textureProperty=nullptr;
        }
    }
}

void CSoftButton::perform3DObjectLoadingMapping(std::vector<int>* map)
{
    if (App::ct->objCont!=nullptr)
    {
        if (_textureProperty!=nullptr)
            _textureProperty->performObjectLoadingMapping(map);
    }
}

void CSoftButton::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    if (App::ct->objCont!=nullptr)
    {
        if (_textureProperty!=nullptr)
            _textureProperty->performTextureObjectLoadingMapping(map);
    }
}

void CSoftButton::setTextureDependencies(int buttonBlockID)
{
    if (_textureProperty!=nullptr)
        _textureProperty->addTextureDependencies(buttonBlockID,_buttonUniqueID);
}

int CSoftButton::getUniqueID()
{
    return(_buttonUniqueID);
}

void CSoftButton::setVertical(bool isVertical)
{
    vertical=isVertical;
}
bool CSoftButton::getVertical()
{
    return(vertical);
}

void CSoftButton::enableArray(bool enable)
{
    delete[] arrayColors;
    arrayColors=nullptr;
    if (enable)
    {
        arrayColors=new float[length*height*3];
        for (int i=0;i<length*height*3;i++)
            arrayColors[i]=0.0f;
    }
}

bool CSoftButton::setArrayColor(int x,int y,const float col[3])
{
    if (arrayColors==nullptr)
        return(false);
    if ( (x<0)||(x>=length)||(y<0)||(y>=height) )
        return(false);
    arrayColors[3*(x+y*length)+0]=col[0];
    arrayColors[3*(x+y*length)+1]=col[1];
    arrayColors[3*(x+y*length)+2]=col[2];
    return(true);
}

bool CSoftButton::isArrayEnabled()
{
    return(arrayColors!=nullptr);
}

int CSoftButton::getLength()
{
    return(length);
}

void CSoftButton::adjustLength(int newLength)
{
    length=newLength;
    enableArray(isArrayEnabled()); // adjust array size
}

int CSoftButton::getHeight()
{
    return(height);
}

void CSoftButton::adjustHeight(int newHeight)
{
    height=newHeight;
    enableArray(isArrayEnabled()); // adjust array size
}
void CSoftButton::setSliderPos(float pos)
{
    tt::limitValue(-1.0f,1.0f,pos);
    sliderPos=pos;
}
float CSoftButton::getSliderPos()
{
    return(sliderPos);
}

int CSoftButton::getButtonType()
{
    return(_buttonAttributes&0x0007);
}

void CSoftButton::setTextureProperty(CTextureProperty* tp)
{
    _textureProperty=tp;
}

CTextureProperty* CSoftButton::getTextureProperty()
{
    return(_textureProperty);
}

CSoftButton* CSoftButton::copyYourself()
{ // Everything is copied.
    CSoftButton* newButton=new CSoftButton(label,xPos,yPos,length,height);
    newButton->buttonID=buttonID;
    newButton->_buttonAttributes=_buttonAttributes;
    if (arrayColors!=nullptr)
    {
        newButton->arrayColors=new float[length*height*3];
        for (int i=0;i<length*height*3;i++)
            newButton->arrayColors[i]=arrayColors[i];
    }
    for (int i=0;i<3;i++)
    {
        newButton->backgroundColor[i]=backgroundColor[i];
        newButton->downBackgroundColor[i]=downBackgroundColor[i];
        newButton->textColor[i]=textColor[i];
    }
    newButton->downLabel=downLabel;
    newButton->sliderPos=sliderPos;
    if (_textureProperty!=nullptr)
        newButton->_textureProperty=_textureProperty->copyYourself();

    return(newButton);
}

void CSoftButton::removeVisionSensorTexture()
{
    if (_textureProperty!=nullptr)
    {
        if ((_textureProperty->getTextureObjectID()<SIM_IDSTART_TEXTURE)||(_textureProperty->getTextureObjectID()>SIM_IDEND_TEXTURE))
        {
            delete _textureProperty;
            _textureProperty=nullptr;
        }
    }
}


void CSoftButton::serialize(CSer& ar)
{
    if (ar.isStoring())
    {       // Storing

        ar.storeDataName("Bva");
        ar << _buttonAttributes << buttonID << length << height << xPos << yPos << label << downLabel << sliderPos;
        ar.flush();

        ar.storeDataName("Bnc");
        ar << backgroundColor[0] << backgroundColor[1] << backgroundColor[2];
        ar << downBackgroundColor[0] << downBackgroundColor[1] << downBackgroundColor[2];
        ar << textColor[0] << textColor[1] << textColor[2];
        ar.flush();

        if (_textureProperty!=nullptr)
        {
            ar.storeDataName("Toj");
            ar.setCountingMode();
            _textureProperty->serialize(ar);
            if (ar.setWritingMode())
                _textureProperty->serialize(ar);
        }

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
                if (theName=="Bva")
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _buttonAttributes >> buttonID >> length >> height >> xPos >> yPos >> label >> downLabel >> sliderPos;
                }
                if (theName=="Bnc")
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> backgroundColor[0] >> backgroundColor[1] >> backgroundColor[2];
                    ar >> downBackgroundColor[0] >> downBackgroundColor[1] >> downBackgroundColor[2];
                    ar >> textColor[0] >> textColor[1] >> textColor[2];
                }
                if (theName.compare("Toj")==0)
                {
                    noHit=false;
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    _textureProperty=new CTextureProperty();
                    _textureProperty->serialize(ar);
                }
                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
}

int CSoftButton::_nextButtonUniqueID=1; // start at 1 because 0 is for the block (so that texture dependencies don't get confused!!)

#ifdef SIM_WITH_GUI
void CSoftButton::drawArray(VPoint p,VPoint s)
{
    if (arrayColors==nullptr)
        return;

    ogl::setAlpha(0.5f);
    if (_buttonAttributes&sim_buttonproperty_transparent)
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    int size[2]={s.x+2,s.y+2};
    int pos[2]={p.x-s.x/2-1,p.y+s.y/2+1};
    int dx[2]={size[0]/length,size[1]/height};
    // First the color pads:
    if ((_buttonAttributes&sim_buttonproperty_nobackgroundcolor)==0)
    {
        for (int i=0;i<length;i++)
        {
            int x=pos[0]+i*dx[0];
            for (int j=0;j<height;j++)
            {
                int y=pos[1]-j*dx[1];
                ogl::setMaterialColor(sim_colorcomponent_emission,arrayColors+3*(i+j*length));
                glBegin(GL_QUADS);
                glVertex3i(x,y,0);
                glVertex3i(x,y-dx[1],0);
                glVertex3i(x+dx[0],y-dx[1],0);
                glVertex3i(x+dx[0],y,0);
                glEnd();
            }
        }
    }
    // Now the button shading:
    if ( ((_buttonAttributes&0x0003)==sim_buttonproperty_button)&&((_buttonAttributes&sim_buttonproperty_borderless)==0)&&((_buttonAttributes&sim_buttonproperty_nobackgroundcolor)==0) )
    {
        for (int i=0;i<length;i++)
        {
            int x=pos[0]+i*dx[0];
            for (int j=0;j<height;j++)
            {
                int y=pos[1]-j*dx[1];

                ogl::setMaterialColor(sim_colorcomponent_emission,arrayColors[3*(i+j*length)+0]*1.5f,arrayColors[3*(i+j*length)+1]*1.5f,arrayColors[3*(i+j*length)+2]*1.5f);
                glBegin(GL_LINE_STRIP);
                glVertex3i(x+1,y-dx[1]+1,0);
                glVertex3i(x+1,y,0);
                glVertex3i(x+dx[0],y,0);
                glEnd();
                ogl::setMaterialColor(sim_colorcomponent_emission,arrayColors[3*(i+j*length)+0]*0.66f,arrayColors[3*(i+j*length)+1]*0.66f,arrayColors[3*(i+j*length)+2]*0.66f);
                glBegin(GL_LINE_STRIP);
                glVertex3i(x+dx[0],y,0);
                glVertex3i(x+dx[0],y-dx[1]+1,0);
                glVertex3i(x+1,y-dx[1]+1,0);
                glEnd();
            }
        }
    }

    if ( ((_buttonAttributes&0x0003)==sim_buttonproperty_editbox)&&((_buttonAttributes&sim_buttonproperty_borderless)==0)&&((_buttonAttributes&sim_buttonproperty_nobackgroundcolor)==0) )
    {
        ogl::setMaterialColor(sim_colorcomponent_emission,backgroundColor);
        for (int i=1;i<length;i++)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0]+i*dx[0]+1,pos[1]+1,0);
            glVertex3i(pos[0]+i*dx[0]+1,pos[1]-size[1]+1,0);
            glEnd();
        }
        for (int i=1;i<height;i++)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0],pos[1]-i*dx[1],0);
            glVertex3i(pos[0]+size[0],pos[1]-i*dx[1],0);
            glEnd();
        }
    }
    ogl::setBlending(false);
    glEnable(GL_DEPTH_TEST);
}
#endif
