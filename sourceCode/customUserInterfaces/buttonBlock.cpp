
// This file requires some serious refactoring!!

#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "buttonBlock.h" 
#include "global.h"
#include "tt.h"
#include "buttonBlockContainer.h"
#include "v_repStrings.h"
#include "app.h"
#include "ttUtil.h"
#include <boost/lexical_cast.hpp>
#include "vDateTime.h"
#ifdef SIM_WITH_GUI
#include "oGL.h"
#endif

CButtonBlock::CButtonBlock(int theBlockWidth,int theBlockHeight,
                           int theButtonWidth,int theButtonHeight,
                           int theView)
{
    commonInit();
    // A value of -1 for the view means that it appears on every view
    blockWidth=1;
    blockHeight=1;
    rollupMin.x=0;
    rollupMin.y=0;
    rollupMax.x=0;
    rollupMax.y=0;
    _attributes|=sim_ui_property_visible;
    VPoint blkSize(theBlockWidth,theBlockHeight);
    setBlockSize(blkSize);
    VPoint butSize(theButtonWidth,theButtonHeight);
    setButtonSizeOriginal(butSize);
    viewToAppearOn=theView;
    blockName=IDSOGL_UI;
}

void CButtonBlock::commonInit()
{
    // A value of -1 for the view means that it appears on every view
    _attributes=sim_ui_property_moveable|sim_ui_property_relativetotopborder|sim_ui_property_settocenter;
    blockID=SIM_IDSTART_2DELEMENT;
    _caughtElements=0;
    objectIDAttachedTo=-1;
    _genericDialog=false;
    blockWidth=1;
    blockHeight=1;
    rollupMin.x=0;
    rollupMin.y=0;
    rollupMax.x=0;
    rollupMax.y=0;
    _initialValuesInitialized=false;
    VPoint blkSize(1,1);
    setBlockSize(blkSize);
    VPoint butSize(2,2);
    setButtonSizeOriginal(butSize);
    _lastEventButtonID=-1;
    blockPosition.x=0;
    blockPosition.y=0;
    desiredBlockPosition.x=0;
    desiredBlockPosition.y=0;
    viewToAppearOn=-1;
    _copyPositionOffset=0;
    _textureProperty=nullptr;
    blockName=IDSOGL_UI;
}

CButtonBlock::~CButtonBlock()
{
    for (int i=0;i<int(allButtons.size());i++)
        delete allButtons[i];
    delete _textureProperty;
    allButtons.clear();
    positionFastIndex.clear();
}

CTextureProperty* CButtonBlock::getTextureProperty()
{
    return(_textureProperty);
}

void CButtonBlock::setTextureProperty(CTextureProperty* tp)
{
    _textureProperty=tp;
}


void CButtonBlock::setGenericDialog(bool genericDialog)
{ 
    _genericDialog=genericDialog;
    if (_genericDialog)
        _attributes=(_attributes|sim_ui_property_systemblockcanmovetofront|sim_ui_property_systemblock); // changed on 15/10/2012
}

bool CButtonBlock::isGenericDialog()
{
    return(_genericDialog);
}

int CButtonBlock::getObjectIDAttachedTo()
{
    return(objectIDAttachedTo);
}

void CButtonBlock::setObjectIDAttachedTo(int objID)
{
    objectIDAttachedTo=objID;
}

void CButtonBlock::setDesiredBlockPosition(int x,int y)
{   
    tt::limitValue(-10000,10000,x);
    tt::limitValue(-10000,10000,y);
    desiredBlockPosition.x=x;
    desiredBlockPosition.y=y;
    blockPosition.x=x;
    blockPosition.y=y;
}
void CButtonBlock::getDesiredBlockPosition(VPoint& pos)
{
    pos.x=desiredBlockPosition.x;
    pos.y=desiredBlockPosition.y;
}

void CButtonBlock::getBlockPositionRelative(VPoint& pos)
{
    pos.x=blockPosition.x;
    pos.y=blockPosition.y;
}

void CButtonBlock::getBlockPositionAbsolute(VPoint& pos)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    pos.x=blockPosition.x;
    pos.y=blockPosition.y;
}

void CButtonBlock::setButtonSizeOriginal(VPoint theSize)
{
    _buttonWidth=theSize.x;
    _buttonHeight=theSize.y;
    tt::limitValue(BUTTON_MIN_X_SIZE,10000,_buttonWidth);
    _buttonWidth=(_buttonWidth/2)*2;        // has to be a multiple of 2
    _buttonHeight=(_buttonHeight/2)*2;  // has to be a multiple of 2
    tt::limitValue(BUTTON_MIN_Y_SIZE,10000,_buttonHeight);
}

CButtonBlock* CButtonBlock::copyYourself()
{
    CButtonBlock* newBlock=new CButtonBlock(blockWidth,blockHeight,getButtonWidthOriginal(),getButtonHeightOriginal(),viewToAppearOn);
    // We slightly shift the block's position so we can see the original:

    int copyPosOff=0;

    if (_attributes&sim_ui_property_moveable)
    {
        _copyPositionOffset+=20;
        copyPosOff=_copyPositionOffset;
    }

    newBlock->_attributes=(_attributes|sim_ui_property_systemblock)-sim_ui_property_systemblock;


    newBlock->desiredBlockPosition.x=desiredBlockPosition.x+copyPosOff;
    newBlock->desiredBlockPosition.y=desiredBlockPosition.y+copyPosOff;
    newBlock->blockPosition.x=blockPosition.x+copyPosOff;
    newBlock->blockPosition.y=blockPosition.y+copyPosOff;

    newBlock->blockID=blockID;
    newBlock->setBlockName(blockName);
    for (int i=0;i<int(allButtons.size());i++)
    {
        CSoftButton* newButton=allButtons[i]->copyYourself();
        int buttonHandle=newButton->buttonID;
        newBlock->insertButton(newButton);
        newButton->buttonID=buttonHandle;
    }
    newBlock->recomputePositionFastIndex();
    VPoint dummy;
    getRollupMin(dummy);
    newBlock->setRollupMin(dummy);
    getRollupMax(dummy);
    newBlock->setRollupMax(dummy);
    newBlock->setObjectIDAttachedTo(getObjectIDAttachedTo());
    if (_textureProperty!=nullptr)
        newBlock->_textureProperty=_textureProperty->copyYourself();

    return(newBlock);
}

void CButtonBlock::setAttributes(int attr)
{
//  int previousAttrib=_attributes;
    _attributes=attr;
    // Following removed on 19/10/2012 (didn't understand why the system block tag cannot be set afterwards. Verified if there was no special side-effect, but couldn't find!)
    /*
    if (((previousAttrib&sim_ui_property_systemblock)==0)&&(_attributes&sim_ui_property_systemblock) )
    {
        if (App::ct->buttonBlockContainer!=nullptr)
        {
            // We cannot set the system block tag if the block was already added to the container! ...and why is that?!!!!
            bool present=false;
            for (int i=0;i<int(App::ct->buttonBlockContainer->allBlocks.size());i++)
            {
                if (App::ct->buttonBlockContainer->allBlocks[i]==this)
                    present=true;
            }
            if (present)
                _attributes-=sim_ui_property_systemblock; 
        }
    }
    */
}

int CButtonBlock::getAttributes()
{
    return(_attributes);
}

void CButtonBlock::getButtonSizeOriginal(VPoint& theSize)
{
    theSize.x=_buttonWidth;
    theSize.y=_buttonHeight;
}

void CButtonBlock::getButtonSizeRetina(VPoint& theSize)
{
    theSize.x=_buttonWidth*App::sc;
    theSize.y=_buttonHeight*App::sc;
}

int CButtonBlock::getButtonWidthOriginal()
{
    return(_buttonWidth);
}

int CButtonBlock::getButtonHeightOriginal()
{
    return(_buttonHeight);
}

int CButtonBlock::getButtonWidthRetina()
{
    return(_buttonWidth*App::sc);
}

int CButtonBlock::getButtonHeightRetina()
{
    return(_buttonHeight*App::sc);
}

CSoftButton* CButtonBlock::getButtonWithID(int id)
{
    for (int i=0;i<int(allButtons.size());i++)
    {
        if (allButtons[i]->buttonID==id)
            return(allButtons[i]);
    }
    return(nullptr);
}

CSoftButton* CButtonBlock::getButtonWithUniqueID(int id)
{
    for (int i=0;i<int(allButtons.size());i++)
    {
        if (allButtons[i]->getUniqueID()==id)
            return(allButtons[i]);
    }
    return(nullptr);
}

CSoftButton* CButtonBlock::getButtonAtPos(int x,int y)
{
    if ( (x<0)||(y<0)||(x>=blockWidth)||(y>=blockHeight) )
        return(nullptr);
    int pos=positionFastIndex[x+y*blockWidth];
    if (pos==-1)
        return(nullptr);
    return(allButtons[pos]);
}

CSoftButton* CButtonBlock::getButtonAtPosDontUseFastIndex(int x,int y)
{
    for (int i=0;i<int(allButtons.size());i++)
    {
        if (allButtons[i]->yPos==y)
        { // Same line
            if ( (allButtons[i]->xPos<=x)&&(allButtons[i]->xPos+allButtons[i]->getLength()>x) )
                return(allButtons[i]);
        }
    }
    return(nullptr);
}
bool CButtonBlock::removeButtonFromPos(int x,int y,bool updateFastIndex)
{   // updateFastIndex is true by default.
    // If updateFastIndex is true, it might be slow if many buttons have to be removed!
    // THIS WILL CALL THE ANNOUNCE BUTTON REMOVED FUNCTION!!!
    for (int i=0;i<int(allButtons.size());i++)
    {
        if (allButtons[i]->yPos==y)
        { // Same line
            if ( (allButtons[i]->xPos<=x)&&(allButtons[i]->xPos+allButtons[i]->getLength()>x) )
            {
                App::ct->objCont->announce2DElementButtonWillBeErased(blockID,allButtons[i]->getUniqueID());
                delete allButtons[i];
                allButtons.erase(allButtons.begin()+i);
                if (updateFastIndex)
                    recomputePositionFastIndex();
                return(true);
            }
        }
    }
    return(false);
}

bool CButtonBlock::insertButton(CSoftButton* theNewButton)
{
    // If return value is false, there is already a button at that position.
    // The new button has to be destroyed outside if it could not have been added!!
    for (int i=0;i<theNewButton->getLength();i++)
    {
        for (int j=0;j<theNewButton->getHeight();j++)
        {
            if (getButtonAtPos(theNewButton->xPos+i,theNewButton->yPos+j)!=nullptr)
                return(false);
        }
    }
    int buttonHandle=getFreeButtonHandle();
    theNewButton->buttonID=buttonHandle;
    return(insertButtonWithoutChecking(theNewButton));
}

int CButtonBlock::getFreeButtonHandle()
{ // Will return the smallest button handle which is not used yet
    std::vector<bool> usedHandles(MAX_X_BUTTONS*MAX_Y_BUTTONS+2,false);
    for (int i=0;i<int(allButtons.size());i++)
    {
        int handle=allButtons[i]->buttonID;
        if (handle<MAX_X_BUTTONS*MAX_Y_BUTTONS)
            usedHandles[handle]=true;
    }
    for (int i=0;i<int(usedHandles.size());i++)
    {
        if (!usedHandles[i])
            return(i);
    }
    return(-1);
}

bool CButtonBlock::insertButtonWithoutChecking(CSoftButton* theNewButton)
{   // Same as insert button, with the difference that we don't check if
    // the button handle or position are valid (has to be checked outside)
    // We check that the button lies inside of the block:
    if ( (theNewButton->xPos>=blockWidth)||(theNewButton->yPos>=blockHeight) )
        return(false);
    // Now we check that the x- and y-sizes are not too big:
    if (theNewButton->xPos+theNewButton->getLength()>blockWidth)
        theNewButton->adjustLength(blockWidth-theNewButton->xPos);
    if (theNewButton->yPos+theNewButton->getHeight()>blockHeight)
        theNewButton->adjustHeight(blockHeight-theNewButton->yPos);
    allButtons.push_back(theNewButton);
    // Now we actualize the position fast index:
    for (int i=0;i<theNewButton->getLength();i++)
    {
        for (int j=0;j<theNewButton->getHeight();j++)
            positionFastIndex[theNewButton->xPos+i+(theNewButton->yPos+j)*blockWidth]=(int)allButtons.size()-1;
    }
    return(true); // Successful
}

void CButtonBlock::setBlockSize(VPoint size)
{
    // We check max/min values:
    if (size.x<1)
        size.x=1;
    if (size.x>MAX_X_BUTTONS)
        size.x=MAX_X_BUTTONS;
    if (size.y<1)
        size.y=1;
    if (size.y>MAX_Y_BUTTONS)
        size.y=MAX_Y_BUTTONS;
    // We remove all buttons which lies outside of the new dimensions:
    int i=0;
    while (i<int(allButtons.size()))
    {
        if ( (allButtons[i]->xPos<size.x)&&(allButtons[i]->yPos<size.y) )
        { // This button starts inside of the new dimensions
            // Now we perform a possible resize of this button:
            if (allButtons[i]->xPos+allButtons[i]->getLength()>size.x)
                allButtons[i]->adjustLength(size.x-allButtons[i]->xPos);
            if (allButtons[i]->yPos+allButtons[i]->getHeight()>size.y)
                allButtons[i]->adjustHeight(size.y-allButtons[i]->yPos);
            i++;
        }
        else
        { // We have to remove this button:
            delete allButtons[i];
            allButtons.erase(allButtons.begin()+i);
        }
    }
    // Now we can set the new size:
    blockWidth=size.x;
    blockHeight=size.y;
    // We empty the selection buffer:
    if (App::ct->buttonBlockContainer!=nullptr)
        App::ct->buttonBlockContainer->deselectButtons();
    // We recompute the fast index:
    recomputePositionFastIndex();
    // We set the rollup-values:
    VPoint dummy(0,0);
    setRollupMin(dummy);
    dummy.x=blockWidth-1;
    setRollupMax(dummy);
}

void CButtonBlock::recomputePositionFastIndex()
{
    positionFastIndex.reserve(blockWidth*blockHeight);
    positionFastIndex.clear();
    for (int i=0;i<blockWidth*blockHeight;i++)
        positionFastIndex.push_back(-1);
    for (int i=0;i<int(allButtons.size());i++)
    {
        for (int j=0;j<allButtons[i]->getLength();j++)
        {
            for (int k=0;k<allButtons[i]->getHeight();k++)
                positionFastIndex[allButtons[i]->xPos+j+(allButtons[i]->yPos+k)*blockWidth]=i;
        }
    }
}

void CButtonBlock::getBlockSize(VPoint& size)
{
    size.x=blockWidth;
    size.y=blockHeight;
}

bool CButtonBlock::doesButtonIDExist(int id)
{
    for (int i=0;i<int(allButtons.size());i++)
    {
        if (allButtons[i]->buttonID==id)
            return(true);
    }
    return(false);
}

void CButtonBlock::setRollupMin(VPoint rollupMinVal)
{
    if (rollupMinVal.x<0)
        rollupMinVal.x=0;
    if (rollupMinVal.y<0)
        rollupMinVal.y=0;
    if (rollupMinVal.x>=blockWidth)
        rollupMinVal.x=blockWidth-1;
    if (rollupMinVal.y>=blockHeight)
        rollupMinVal.y=blockHeight-1;
    if (rollupMinVal.x>rollupMax.x)
        rollupMax.x=rollupMinVal.x;
    if (rollupMinVal.y>rollupMax.y)
        rollupMax.y=rollupMinVal.y;
    rollupMin=rollupMinVal;
}
void CButtonBlock::setRollupMax(VPoint rollupMaxVal)
{
    if (rollupMaxVal.x<0)
        rollupMaxVal.x=0;
    if (rollupMaxVal.y<0)
        rollupMaxVal.y=0;
    if (rollupMaxVal.x>=blockWidth)
        rollupMaxVal.x=blockWidth-1;
    if (rollupMaxVal.y>=blockHeight)
        rollupMaxVal.y=blockHeight-1;
    if (rollupMaxVal.x<rollupMin.x)
        rollupMin.x=rollupMaxVal.x;
    if (rollupMaxVal.y<rollupMin.y)
        rollupMin.y=rollupMaxVal.y;
    rollupMax=rollupMaxVal;
}
void CButtonBlock::getRollupMin(VPoint& rollupMinVal)
{
    rollupMinVal=rollupMin;
}
void CButtonBlock::getRollupMax(VPoint& rollupMaxVal)
{
    rollupMaxVal=rollupMax;
}

void CButtonBlock::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    if (objectIDAttachedTo!=-1)
    {
        _initialValuesInitialized=simulationIsRunning;
        for (int i=0;i<int(allButtons.size());i++)
            allButtons[i]->initializeInitialValues(simulationIsRunning);
        if (simulationIsRunning)
        {
            _initialAttributes=_attributes;
            _initialPos[0]=desiredBlockPosition.x;
            _initialPos[1]=desiredBlockPosition.y;
        }
    }
}

void CButtonBlock::simulationAboutToStart()
{ 
    if (objectIDAttachedTo!=-1)
        initializeInitialValues(true);
}

void CButtonBlock::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (objectIDAttachedTo!=-1)
    {
        for (int i=0;i<int(allButtons.size());i++)
            allButtons[i]->simulationEnded();
        if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
        {
            _attributes=_initialAttributes;
            desiredBlockPosition.x=_initialPos[0];
            desiredBlockPosition.y=_initialPos[1];
        }
        _initialValuesInitialized=false;
    }
}
void CButtonBlock::setBlockID(int newBlockID)
{
    blockID=newBlockID;
}
int CButtonBlock::getBlockID()
{
    return(blockID);
}
void CButtonBlock::setViewToAppearOn(int theView)
{
    viewToAppearOn=theView;
}
int CButtonBlock::getViewToAppearOn()
{
    return(viewToAppearOn);
}

void CButtonBlock::removeAllVisionSensorTextures()
{
    if (_textureProperty!=nullptr)
    {
        if ((_textureProperty->getTextureObjectID()<SIM_IDSTART_TEXTURE)||(_textureProperty->getTextureObjectID()>SIM_IDEND_TEXTURE))
        {
            delete _textureProperty;
            _textureProperty=nullptr;
        }
    }
    for (int i=0;i<int(allButtons.size());i++)
        allButtons[i]->removeVisionSensorTexture();
}


void CButtonBlock::removeAllObjectAttachements()
{
    objectIDAttachedTo=-1;
}

void CButtonBlock::perform3DObjectLoadingMapping(std::vector<int>* map)
{
    if (App::ct->objCont!=nullptr)
    {
        objectIDAttachedTo=App::ct->objCont->getLoadingMapping(map,objectIDAttachedTo);
        if (_textureProperty!=nullptr)
            _textureProperty->performObjectLoadingMapping(map);
    }
    for (int i=0;i<int(allButtons.size());i++)
        allButtons[i]->perform3DObjectLoadingMapping(map);
}

void CButtonBlock::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    if (App::ct->objCont!=nullptr)
    {
        if (_textureProperty!=nullptr)
            _textureProperty->performTextureObjectLoadingMapping(map);
    }
    for (int i=0;i<int(allButtons.size());i++)
        allButtons[i]->performTextureObjectLoadingMapping(map);
}

bool CButtonBlock::announce3DObjectWillBeErased(int objectID,bool copyBuffer)
{
    if (objectIDAttachedTo==objectID)
        return(true);
    if (_textureProperty!=nullptr)
    {
        if (_textureProperty->announceObjectWillBeErased(objectID))
        {
            delete _textureProperty;
            _textureProperty=nullptr;
        }
    }
    for (int i=0;i<int(allButtons.size());i++)
        allButtons[i]->announce3DObjectWillBeErased(objectID);
    return(false);
}

void CButtonBlock::setTextureDependencies()
{
    if (_textureProperty!=nullptr)
        _textureProperty->addTextureDependencies(blockID,0);
    for (int i=0;i<int(allButtons.size());i++)
        allButtons[i]->setTextureDependencies(blockID);
}

void CButtonBlock::setBlockName(std::string name)
{
    blockName=name;
}
std::string CButtonBlock::getBlockName()
{
    return(blockName);
}

VPoint CButtonBlock::_getBlockSizeAndOtherButtonSizeAndPos(VPoint& blockSize,VPoint& blockPos,VPoint& buttonSize,VPoint& buttonPos,VPoint& otherButtonSize,CSoftButton* button)
{
    VPoint pos;
    blockSize.x=(getButtonWidthRetina()+2)*blockWidth-2;
    blockSize.y=(getButtonHeightRetina()+2)*blockHeight-2;
    otherButtonSize.x=(getButtonWidthRetina()+2)*button->getLength()-2;
    otherButtonSize.y=(getButtonHeightRetina()+2)*button->getHeight()-2;
    if (_attributes&sim_ui_property_relativetotopborder)
        pos.y=App::ct->buttonBlockContainer->winSize[1]-1-(blockPos.y+buttonPos.y*(buttonSize.y+2)+otherButtonSize.y/2);
    else
        pos.y=blockPos.y+1+blockSize.y-(buttonPos.y*(buttonSize.y+2)+otherButtonSize.y/2);
    if (_attributes&sim_ui_property_relativetoleftborder)
        pos.x=blockPos.x+1+buttonPos.x*(buttonSize.x+2)+otherButtonSize.x/2;
    else
        pos.x=App::ct->buttonBlockContainer->winSize[0]-1-blockPos.x-blockSize.x+buttonPos.x*(buttonSize.x+2)+otherButtonSize.x/2;
    return(pos);
}

void CButtonBlock::getAllAttachedTextureProperties(std::vector<CTextureProperty*>& textPropVect)
{
    if (_textureProperty!=nullptr)
        textPropVect.push_back(_textureProperty);   
    for (int i=0;i<int(allButtons.size());i++)
    {
        CTextureProperty* tp=allButtons[i]->getTextureProperty();
        if (tp!=nullptr)
            textPropVect.push_back(tp); 
    }
}

void CButtonBlock::serialize(CSer &ar)
{
    if (ar.isStoring())
    {       // Storing
        ar.storeDataName("Bkh");
        ar << blockID;
        ar.flush();

        ar.storeDataName("Att");
        ar << _attributes;
        ar.flush();

        ar.storeDataName("Bkn");
        ar << blockName;
        ar.flush();

        ar.storeDataName("Bks");
        ar << blockWidth << blockHeight;
        ar.flush();

        ar.storeDataName("Bkp");
        ar << desiredBlockPosition.x << desiredBlockPosition.y;
        ar.flush();

        ar.storeDataName("Bns");
        ar << _buttonWidth << _buttonHeight;
        ar.flush();

        ar.storeDataName("Vew");
        ar << viewToAppearOn;
        ar.flush();

        ar.storeDataName("Aoi");
        ar << objectIDAttachedTo;
        ar.flush();

        ar.storeDataName("Rus");
        ar << rollupMin.x << rollupMin.y << rollupMax.x << rollupMax.y;
        ar.flush();

        for (int i=0;i<int(allButtons.size());i++)
        {
            ar.storeDataName("Btn");
            ar.setCountingMode();
            allButtons[i]->serialize(ar);
            if (ar.setWritingMode())
                allButtons[i]->serialize(ar);
        }

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
                if (theName.compare("Rus")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> rollupMin.x >> rollupMin.y >> rollupMax.x >> rollupMax.y;
                }

                if (theName.compare("Bkn")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> blockName;
                }

                if (theName.compare("Att")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _attributes;
                }

                if (theName.compare("Bkh")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> blockID;
                }
    
                if (theName.compare("Bks")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    VPoint blkSize;
                    ar >> blkSize.x >> blkSize.y;
                    setBlockSize(blkSize);
                }

                if (theName.compare("Bkp")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> desiredBlockPosition.x >> desiredBlockPosition.y;
                    blockPosition=desiredBlockPosition;
                }

                if (theName.compare("Bns")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> _buttonWidth >> _buttonHeight;
                }

                if (theName.compare("Vew")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> viewToAppearOn;
                }

                if (theName.compare("Aoi")==0)
                {
                    noHit=false;
                    ar >> byteQuantity;
                    ar >> objectIDAttachedTo;
                }
                if (theName.compare("Btn")==0)
                {
                    noHit=false;
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CSoftButton* newButton=new CSoftButton("",0,0,1,1);
                    newButton->serialize(ar);
                    int buttonHandle=newButton->buttonID;
                    insertButtonWithoutChecking(newButton);
                    newButton->buttonID=buttonHandle;
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

#ifdef SIM_WITH_GUI
int CButtonBlock::getCaughtElements()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(_caughtElements);
}

void CButtonBlock::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _caughtElements&=keepMask;
}

void CButtonBlock::setLastEventButtonID(int id,int auxVals[2])
{
    _lastEventButtonID=id;
    for (int i=0;i<2;i++)
        _lastEventButtonAuxVals[i]=auxVals[i];
}

int CButtonBlock::getLastEventButtonID(int auxVals[2])
{ // auxVals can be nullptr
    int retVal=_lastEventButtonID;
    if (auxVals!=nullptr)
    {
        for (int i=0;i<2;i++)
            auxVals[i]=_lastEventButtonAuxVals[i];
    }
    _lastEventButtonID=-1; // we clear the event
    return(retVal);
}

void CButtonBlock::displayBlock(int winSize[2],bool justCameToFront)
{
    if (!isDisplayedNow())
        return;
    if (CEnvironment::getCustomUisTemporarilyDisabled())
    {
        if ((_attributes&sim_ui_property_systemblock)==0)
            return;
    }
    if (_attributes&sim_ui_property_fixedwidthfont)
        ogl::selectBitmapFont(1);
    int psClosed[2]={blockWidth*(getButtonWidthRetina()+2),blockHeight*(getButtonHeightRetina()+2)};
    int psOpen[2]={blockWidth*(getButtonWidthRetina()+2),blockHeight*(getButtonHeightRetina()+2)};
    int vs[2]={0,psOpen[1]};

    if (_attributes&sim_ui_property_settocenter)
    { // the first time the element is displayed:
        desiredBlockPosition.y+=winSize[1]/2-psOpen[1]/2; // changed the = into += on 2009/07/16 (if set to center is set pos could not be changed before)
        desiredBlockPosition.x+=winSize[0]/2-psOpen[0]/2; // changed the = into += on 2009/07/16 (if set to center is set pos could not be changed before)
        _attributes-=sim_ui_property_settocenter;
    }

    if (_attributes&sim_ui_property_rolledup)
    {
        psClosed[0]=((rollupMax.x-rollupMin.x)+1)*(getButtonWidthRetina()+2);
        psClosed[1]=((rollupMax.y-rollupMin.y)+1)*(getButtonHeightRetina()+2);
        vs[0]=rollupMin.x*(getButtonWidthRetina()+2);
        vs[1]=(blockHeight-rollupMin.y)*(getButtonHeightRetina()+2);
    }
    int aa;
    int correction[2]={0,0};
    if (_attributes&sim_ui_property_relativetotopborder)
    {
        aa=desiredBlockPosition.y+psOpen[1]-vs[1]+psClosed[1];
        if (aa>winSize[1])
            correction[1]=winSize[1]-aa;
        aa=desiredBlockPosition.y+correction[1]+psOpen[1]-vs[1];
        if (aa<0)
            correction[1]+=-aa;
    }
    else
    {
        aa=desiredBlockPosition.y+vs[1];
        if (aa>winSize[1])
            correction[1]=winSize[1]-aa;
        aa=desiredBlockPosition.y+correction[1]+vs[1]-psClosed[1];
        if (aa<0)
            correction[1]+=-aa;
    }
    if (_attributes&sim_ui_property_relativetoleftborder)
    {
        aa=desiredBlockPosition.x+vs[0]+psClosed[0];
        if (aa>winSize[0])
            correction[0]=winSize[0]-aa;
        aa=desiredBlockPosition.x+correction[0]+vs[0];
        if (aa<0)
            correction[0]+=-aa;
    }
    else
    {
        aa=desiredBlockPosition.x+psOpen[0]-vs[0];
        if (aa>winSize[0])
            correction[0]=winSize[0]-aa;
        aa=desiredBlockPosition.x+correction[0]+psOpen[0]-vs[0]-psClosed[0];
        if (aa<0)
            correction[0]+=-aa;
    }
    blockPosition.x=desiredBlockPosition.x+correction[0];
    blockPosition.y=desiredBlockPosition.y+correction[1];

    // Following is used in edit mode to display the overlay (not selected, selected, first sel.)
    std::vector<unsigned char> usedSpaces;
    if ( (App::ct->buttonBlockContainer->getBlockInEdition()==blockID)&&(App::ct->buttonBlockContainer->editMode) )
    {
        usedSpaces.reserve(blockWidth*blockHeight);
        for (int i=0;i<blockWidth*blockHeight;i++)
            usedSpaces.push_back(0);
        for (int i=0;i<(int)(App::ct->buttonBlockContainer->selectedButtons.size()-1);i++)
            usedSpaces[App::ct->buttonBlockContainer->selectedButtons[i]]=1;
        if (App::ct->buttonBlockContainer->selectedButtons.size()!=0)
            usedSpaces[App::ct->buttonBlockContainer->selectedButtons[App::ct->buttonBlockContainer->selectedButtons.size()-1]]=2;
    }
    VPoint buttonSize(getButtonWidthRetina(),getButtonHeightRetina());
    VPoint pos;
    VPoint blockPos;
    getBlockPositionAbsolute(blockPos);

    // Following is needed to see the shift-selection:
    VPoint startP(0,0);
    VPoint endP(0,0);
    if ( (App::ct->buttonBlockContainer->getBlockInEdition()==blockID)&&(App::ct->buttonBlockContainer->editMode) )
    {
        if (App::ct->buttonBlockContainer->shiftSelectionStart!=-1)
        {
            startP.x=App::ct->buttonBlockContainer->shiftSelectionStart%(blockWidth);
            startP.y=App::ct->buttonBlockContainer->shiftSelectionStart/(blockWidth);
            int combP=mouseDownCatchInexistant(App::ct->buttonBlockContainer->mousePos.x,App::ct->buttonBlockContainer->mousePos.y,true);
            endP.x=combP%(blockWidth);
            endP.y=combP/(blockWidth);
            if (combP==-1)
            {
                startP.x=-1;
                startP.y=-1;
                endP.x=-1;
                endP.y=-1;
            }
            int tmp;
            if (startP.x>endP.x)
            {
                tmp=startP.x;
                startP.x=endP.x;
                endP.x=tmp;
            }
            if (startP.y>endP.y)
            {
                tmp=startP.y;
                startP.y=endP.y;
                endP.y=tmp;
            }
        }
    }

    for (int i=0;i<int(allButtons.size());i++)
    {
        CSoftButton* it=allButtons[i];
        if (isButtonInVisibleZone(it))
        {
        bool buttonDown=((blockID==App::ct->buttonBlockContainer->caughtBlock)&&(it->buttonID==App::ct->buttonBlockContainer->caughtButton)&&App::ct->buttonBlockContainer->caughtButtonDown);
        std::string txt=it->label;
        float txtCol[3]={it->textColor[0],it->textColor[1],it->textColor[2]};
        VPoint buttPos(it->xPos,it->yPos);

        VPoint blockSize,otherButtonSize;
        pos=_getBlockSizeAndOtherButtonSizeAndPos(blockSize,blockPos,buttonSize,buttPos,otherButtonSize,it);

        VPoint blockPosAbs;
        if (_attributes&sim_ui_property_relativetotopborder)
            blockPosAbs.y=App::ct->buttonBlockContainer->winSize[1]-blockPos.y;
        else
            blockPosAbs.y=blockPos.y+blockSize.y+2;
        if (_attributes&sim_ui_property_relativetoleftborder)
            blockPosAbs.x=blockPos.x;
        else
            blockPosAbs.x=App::ct->buttonBlockContainer->winSize[0]-blockPos.x-blockSize.x-2;

        it->setVertical(otherButtonSize.y>otherButtonSize.x);
        if ((it->getAttributes()&sim_buttonproperty_isdown)&&(it->getButtonType()==sim_buttonproperty_button))
            txt=it->downLabel;
        float sliderVal=it->getSliderPos();
        bool editing=false;
        if ( (it->getButtonType()==sim_buttonproperty_editbox)&&(blockID==App::ct->buttonBlockContainer->editBoxInEditionBlock)&&(it->buttonID==App::ct->buttonBlockContainer->editBoxInEditionButton) )
        {
            txt=App::ct->buttonBlockContainer->editBoxEditionText;
            editing=true;
        }
        if (it->isArrayEnabled())
            it->drawArray(pos,otherButtonSize);
        else
        {
            if ( (App::ct->buttonBlockContainer->caughtBlock==blockID)&&(App::ct->buttonBlockContainer->caughtButton==it->buttonID) )
            {
                if ( (App::ct->buttonBlockContainer->caughtBlock==App::ct->buttonBlockContainer->caughtBlockForDownUpEvent)&&(App::ct->buttonBlockContainer->caughtButton==App::ct->buttonBlockContainer->caughtButtonForDownUpEvent) )
                {
                    if ( (buttonDown!=App::ct->buttonBlockContainer->caughtButtonDownForDownUpEvent)&&(!editing) )
                    { // We have to generate an up or down event
                        App::ct->buttonBlockContainer->caughtButtonDownForDownUpEvent=buttonDown;
                        App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,App::ct->buttonBlockContainer->caughtBlock,App::ct->buttonBlockContainer->caughtButton,it->getAttributes(),buttonDown,nullptr,0);
                        int auxVals[2]={it->getAttributes(),buttonDown};
                        setLastEventButtonID(App::ct->buttonBlockContainer->caughtButton,auxVals);
                    }
                }
            }

            int atr=it->getAttributes();
            if (buttonDown)
                atr|=sim_buttonproperty_isdown;
            float* secondTextColor=nullptr; // For now (2009/07/24)

            if (App::ct->environment->get2DElementTexturesEnabled())
                ogl::drawButton(pos,otherButtonSize,txtCol,it->backgroundColor,it->downBackgroundColor,txt,atr,
                    editing,App::ct->buttonBlockContainer->editBoxEditionPosition,sliderVal,it->getVertical(),VDateTime::getTimeInMs(),secondTextColor,_textureProperty,&blockPosAbs,&blockSize,it->getTextureProperty());
            else
                ogl::drawButton(pos,otherButtonSize,txtCol,it->backgroundColor,it->downBackgroundColor,txt,atr,
                    editing,App::ct->buttonBlockContainer->editBoxEditionPosition,sliderVal,it->getVertical(),VDateTime::getTimeInMs(),secondTextColor,nullptr,&blockPosAbs,&blockSize,nullptr);

        }
        if ( (App::ct->buttonBlockContainer->getBlockInEdition()==blockID)&&(App::ct->buttonBlockContainer->editMode) )
        {
            bool mainSel=false;
            bool showSelected=false;
            for (int k=0;k<it->getHeight();k++)
            {
                for (int m=0;m<it->getLength();m++)
                {
                    mainSel=mainSel||(usedSpaces[blockWidth*(it->yPos+k)+it->xPos+m]==2);
                    showSelected=showSelected||(usedSpaces[blockWidth*(it->yPos+k)+it->xPos+m]==1)||mainSel;
                }
            }
            if ( (!showSelected)&&(App::ct->buttonBlockContainer->shiftSelectionStart!=-1) )
            {
                if ( (buttPos.y+it->getHeight()>startP.y)&&(buttPos.y<=endP.y) )
                {
                    if ( (buttPos.x+it->getLength()>startP.x)&&(buttPos.x<=endP.x) )
                        showSelected=true;
                }
            }

            ogl::drawButtonEdit(pos,otherButtonSize,showSelected,mainSel);
            for (int k=0;k<it->getHeight();k++) // These positions were processed
            {
                for (int m=0;m<it->getLength();m++)
                    usedSpaces[blockWidth*(it->yPos+k)+it->xPos+m]=255;
            }
        }
        }
    }
    if ( (App::ct->buttonBlockContainer->getBlockInEdition()==blockID)&&(App::ct->buttonBlockContainer->editMode) )
    {
        VPoint blockSize((getButtonWidthRetina()+2)*blockWidth-2,(getButtonHeightRetina()+2)*blockHeight-2);
        VPoint otherButtonSize(getButtonWidthRetina(),getButtonHeightRetina());
        for (int k=0;k<blockWidth;k++)
        {
            for (int l=0;l<blockHeight;l++)
            {
                if (usedSpaces[blockWidth*l+k]!=255)
                {
                    if (_attributes&sim_ui_property_relativetotopborder)
                        pos.y=App::ct->buttonBlockContainer->winSize[1]-1-(blockPos.y+l*(buttonSize.y+2)+buttonSize.y/2);
                    else
                        pos.y=blockPos.y+1+blockSize.y-(l*(buttonSize.y+2)+buttonSize.y/2);
                    if (_attributes&sim_ui_property_relativetoleftborder)
                        pos.x=blockPos.x+1+k*(buttonSize.x+2)+(otherButtonSize.x)/2;
                    else
                        pos.x=App::ct->buttonBlockContainer->winSize[0]-1-blockPos.x-blockSize.x+k*(buttonSize.x+2)+(otherButtonSize.x)/2;
                    bool mainSel=(usedSpaces[blockWidth*l+k]==2);
                    bool showSelected=(usedSpaces[blockWidth*l+k]==1)||mainSel;
                    if ( (!showSelected)&&(App::ct->buttonBlockContainer->shiftSelectionStart!=-1) )
                    {
                        if ( (k>=startP.x)&&(k<=endP.x)&&(l>=startP.y)&&(l<=endP.y) )
                            showSelected=true;
                    }
                    ogl::drawButtonEdit(pos,otherButtonSize,showSelected,mainSel);
                }
            }
        }
    }
    if (_attributes&sim_ui_property_fixedwidthfont) // We need to reset to default font
        ogl::selectBitmapFont(0);
}

int CButtonBlock::mouseDownCatch(int xCoord,int yCoord,bool& cursorCatch,bool test)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (!test)
        _caughtElements&=0xffff-sim_left_button;
    if (!isDisplayedNow())
        return(-1);
    VPoint buttonSize(getButtonWidthRetina(),getButtonHeightRetina());
    for (int i=(int)allButtons.size()-1;i>=0;i--)
    {
        CSoftButton* itButton=allButtons[i];
        if ( isButtonInVisibleZone(itButton)&&
            ( ((itButton->getAttributes()&sim_buttonproperty_ignoremouse)==0)||
            (App::ct->buttonBlockContainer->getButtonEditMode_editMode()&&((_attributes&sim_ui_property_systemblock)==0)) ) ) // The block might be rolled-up!
        {
        VPoint buttPos(itButton->xPos,itButton->yPos);
        VPoint pos;
        VPoint blockPos;
        getBlockPositionAbsolute(blockPos);

        VPoint blockSize,otherButtonSize;
        pos=_getBlockSizeAndOtherButtonSizeAndPos(blockSize,blockPos,buttonSize,buttPos,otherButtonSize,itButton);

        if ((xCoord+1>=pos.x-otherButtonSize.x/2)&&(xCoord-1<=pos.x+otherButtonSize.x/2))
        {
            if ((yCoord+1>=pos.y-otherButtonSize.y/2)&&(yCoord-1<=pos.y+otherButtonSize.y/2))
            { // We clicked onto the zone. We check now if we clicked the cursor
                if (!itButton->getVertical())
                { // Horizontal slider
                    int p=(int)(itButton->getSliderPos()*0.5f*(1.0f-BUTTON_SLIDER_X_SIZE)*(float)otherButtonSize.x);
                    int sx=(int)(0.5f*BUTTON_SLIDER_X_SIZE*(float)otherButtonSize.x);
                    int sy=(int)(0.5f*BUTTON_SLIDER_Y_SIZE*(float)otherButtonSize.y);
                    if ((xCoord+1>=pos.x+p-sx)&&(xCoord-1<=pos.x+p+sx))
                    {
                        if ((yCoord+1>=pos.y-sy)&&(yCoord-1<=pos.y+sy))
                            cursorCatch=true;
                    }
                }
                else
                { // Vertical slider
                    int p=(int)(itButton->getSliderPos()*0.5f*(1.0f-BUTTON_SLIDER_X_SIZE)*(float)otherButtonSize.y);
                    int sx=(int)(0.5f*BUTTON_SLIDER_Y_SIZE*(float)otherButtonSize.x);
                    int sy=(int)(0.5f*BUTTON_SLIDER_X_SIZE*(float)otherButtonSize.y);
                    if ((yCoord+1>=pos.y-p-sy)&&(yCoord-1<=pos.y-p+sy))
                    {
                        if ((xCoord+1>=pos.x-sx)&&(xCoord-1<=pos.x+sx))
                            cursorCatch=true;
                    }
                }
                if (!test)
                    _caughtElements|=sim_left_button;
                return(itButton->buttonID);
            }
        }
        }
    }
    return(-1);
}

bool CButtonBlock::isDisplayedNow()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( ((_attributes&sim_ui_property_visible)==0)&&(!App::ct->buttonBlockContainer->editMode) )
        return(false);

    // 2DElements stay visible during pause:
    if ((App::ct->simulation->isSimulationStopped())&&(_attributes&sim_ui_property_visibleduringsimulationonly)&&(!App::ct->buttonBlockContainer->editMode))
        return(false);
    if ((_attributes&sim_ui_property_visiblewhenobjectselected)&&(objectIDAttachedTo!=-1)&&(!App::ct->buttonBlockContainer->editMode))
    {
        if (!App::ct->objCont->isObjectSelected(objectIDAttachedTo))
            return(false);
    }
    return(true);
}

int CButtonBlock::mouseDownCatchInexistant(int xCoord,int yCoord,bool test)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    // Same as mouseDownCatch, but we check if we clicked onto an empty space of that block
    if (!test)
        _caughtElements&=0xffff-sim_left_button;
    if (!isDisplayedNow())
        return(-1);
    VPoint buttonSize(getButtonWidthRetina(),getButtonHeightRetina());
    VPoint pos;
    VPoint blockSize((getButtonWidthRetina()+2)*blockWidth-2,(getButtonHeightRetina()+2)*blockHeight-2);
    VPoint blockPos;
    getBlockPositionAbsolute(blockPos);
    if (_attributes&sim_ui_property_relativetotopborder)
        pos.y=App::ct->buttonBlockContainer->winSize[1]-1-blockPos.y;
    else
        pos.y=blockPos.y+1+blockSize.y;
    if (_attributes&sim_ui_property_relativetoleftborder)
        pos.x=blockPos.x+1;
    else
        pos.x=App::ct->buttonBlockContainer->winSize[0]-1-blockPos.x-blockSize.x;
    int x=(xCoord-pos.x);
    if (x<0)
        return(-1);
    x=x/(getButtonWidthRetina()+2);
    int y=(pos.y-yCoord);
    if (y<0)
        return(-1);
    y=y/(getButtonHeightRetina()+2);
    if ( (x>=blockWidth)||(y>=blockHeight) )
        return(-1);
    if (!test)
        _caughtElements|=sim_left_button;
    return(y*blockWidth+x);
}

void CButtonBlock::mouseMoveCatch(int xCoord,int yCoord)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    bool dummyVal;
    int butt=mouseDownCatch(xCoord,yCoord,dummyVal,true);
    App::ct->buttonBlockContainer->caughtButtonDown=(butt==App::ct->buttonBlockContainer->caughtButton);
}

bool CButtonBlock::isButtonInVisibleZone(CSoftButton* it)
{
    if (((_attributes&sim_ui_property_rolledup)==0)||(App::ct->buttonBlockContainer->editMode&&((_attributes&sim_ui_property_systemblock)==0)))
        return(true);
    if ( (it->xPos<rollupMin.x)||(it->xPos+it->getLength()-1>rollupMax.x) )
        return(false);
    if ( (it->yPos<rollupMin.y)||(it->yPos+it->getHeight()-1>rollupMax.y) )
        return(false);
    return(true);
}

#endif
