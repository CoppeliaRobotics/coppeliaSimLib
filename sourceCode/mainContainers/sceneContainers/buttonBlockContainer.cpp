
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "buttonBlockContainer.h" 
#include "global.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"
#include "miscBase.h"

CButtonBlockContainer::CButtonBlockContainer(bool mainContainer)
{
    FUNCTION_DEBUG;
    newSceneProcedurePasses=0;
    infoBox=nullptr;
    copyOfBlock_forEditMode=nullptr;
    _lastBlockIdInFront=-1;
    for (int i=0;i<INFO_BOX_ROW_COUNT*2;i++)
        infoBoxButtonHandlers[i]=1+i;

    emptySceneProcedure(mainContainer);
}


CButtonBlockContainer::~CButtonBlockContainer()
{
    removeAllBlocks(true);
    if (copyOfBlock_forEditMode!=nullptr)
        delete copyOfBlock_forEditMode;
}

void CButtonBlockContainer::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CButtonBlockContainer::setTextureDependencies()
{
    for (int i=0;i<int(allBlocks.size());i++)
        allBlocks[i]->setTextureDependencies();
}

void CButtonBlockContainer::emptySceneProcedure(bool mainContainer)
{
    FUNCTION_DEBUG;
    removeAllBlocks(false);
    caughtBlock=-1;
    caughtButton=-1;
    caughtBlockForDownUpEvent=-1;
    caughtButtonForDownUpEvent=-1;
    editBoxInEditionBlock=-1;
    editBoxInEditionButton=-1;
    editBoxEditionPosition=-1;
    editBoxEditionText="";
    caughtButtonDown=false;
    mouseCaptured=false;
    editMode=false;
    _blockInEditMode=-1;
    shiftSelectionStart=-1;
    buttonsLocked=false;

#ifdef SIM_WITH_GUI
    if ( CMiscBase::handleVerSpec_hasInfoBar()&&(newSceneProcedurePasses==0) )
    {
        if (mainContainer)
        {
            if (App::operationalUIParts&sim_gui_infobar)
            {
                // System block creation:
                std::string fullPathAndFilename=App::directories->systemDirectory+VREP_SLASH;
                fullPathAndFilename.append("sysnfo.ttb");
                CButtonBlockContainer* cont=loadSystemButtonBlocks(fullPathAndFilename);
                if (cont!=nullptr)
                {
                    if (cont->allBlocks[0]!=nullptr)
                    {
                        infoBox=cont->allBlocks[0]->copyYourself();
                        infoBox->setBlockName("SYSTEM_BLOCK-INFO_OVERLAY");
                        infoBox->setAttributes(infoBox->getAttributes()|sim_ui_property_systemblock|sim_ui_property_pauseactive);
                        insertBlock(infoBox,false);
                        infoBox->setDesiredBlockPosition(0,0);
                    }
                    delete cont;
                }
            }
        }
    }
#endif
    newSceneProcedurePasses++;
}

void CButtonBlockContainer::removeAllBlocks(bool alsoSystemBlocks)
{
    if (alsoSystemBlocks)
    {
        while (allBlocks.size()!=0)
            removeBlockFromID(allBlocks[0]->getBlockID());
    }
    else
    {
        for (int i=0;i<int(allBlocks.size());i++)
        {
            CButtonBlock* it=allBlocks[i];
            if ( (it->getAttributes()&sim_ui_property_systemblock)==0 )
            {
                removeBlockFromID(it->getBlockID());
                i=0; // we have to re-check from the beginning, above function might have changed the order
            }
        }
    }
}

void CButtonBlockContainer::updateInfoWindowColorStyle(int colorStyle)
{
    if (infoBox==nullptr)
        return;
    bool background=(colorStyle==2);
    float txtCol[3];
    if ((colorStyle==0)||(colorStyle==2))
    {
        txtCol[0]=0.0f;
        txtCol[1]=0.0f;
        txtCol[2]=0.0f;
    }
    if (colorStyle==1)
    {
        txtCol[0]=1.0f;
        txtCol[1]=1.0f;
        txtCol[2]=1.0f;
    }
    for (int i=0;i<20;i++)
    {
        CSoftButton* b=infoBox->getButtonWithID(2*i+1);
        CSoftButton* b2=infoBox->getButtonWithID(2*i+2);
        CSoftButton* b3=infoBox->getButtonWithID(41+i-2);
        if ( (b!=nullptr)&&(b2!=nullptr)&&(b3!=nullptr) )
        {
            bool lineActive=b->label!="";
            if (background)
            {
                b->setAttributes(b->getAttributes()&(0xffff-sim_buttonproperty_transparent-sim_buttonproperty_nobackgroundcolor));
                b2->setAttributes(b2->getAttributes()&(0xffff-sim_buttonproperty_transparent-sim_buttonproperty_nobackgroundcolor));
                if (i>1)
                    b3->setAttributes(b3->getAttributes()&(0xffff-sim_buttonproperty_transparent-sim_buttonproperty_nobackgroundcolor));
                if (lineActive)
                {
                    b->setAttributes(b->getAttributes()|sim_buttonproperty_transparent);
                    b2->setAttributes(b2->getAttributes()|sim_buttonproperty_transparent);
                    if (i>1)
                        b3->setAttributes(b3->getAttributes()|sim_buttonproperty_transparent);
                }
                else
                {
                    b->setAttributes(b->getAttributes()|sim_buttonproperty_nobackgroundcolor);
                    b2->setAttributes(b2->getAttributes()|sim_buttonproperty_nobackgroundcolor);
                    if (i>1)
                        b3->setAttributes(b3->getAttributes()|sim_buttonproperty_nobackgroundcolor);
                }
            }
            else
            {
                b->setAttributes(b->getAttributes()|sim_buttonproperty_nobackgroundcolor);
                b2->setAttributes(b2->getAttributes()|sim_buttonproperty_nobackgroundcolor);
                if (i>1)
                    b3->setAttributes(b3->getAttributes()|sim_buttonproperty_nobackgroundcolor);
            }
            b->textColor[0]=txtCol[0];
            b->textColor[1]=txtCol[1];
            b->textColor[2]=txtCol[2];
            b2->textColor[0]=txtCol[0];
            b2->textColor[1]=txtCol[1];
            b2->textColor[2]=txtCol[2];
        }
    }
}

int CButtonBlockContainer::getNonSystemBlockCount()
{
    int retVal=0;
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if ( (allBlocks[i]->getAttributes()&sim_ui_property_systemblock)==0 )
            retVal++;
    }
    return(retVal);
}

int CButtonBlockContainer::getUnassociatedNonSystemBlockCount()
{
    int retVal=0;
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if ( (allBlocks[i]->getAttributes()&sim_ui_property_systemblock)==0 )
        {
            if (allBlocks[i]->getObjectIDAttachedTo()==-1)
                retVal++;
        }
    }
    return(retVal);
}


void CButtonBlockContainer::setViewSizeAndPosition(int sX,int sY,int pX,int pY)
{
    winSize[0]=sX;
    winSize[1]=sY;
    winPos[0]=pX;
    winPos[1]=pY;
}

void CButtonBlockContainer::simulationAboutToStart()
{
    for (int i=0;i<int(allBlocks.size());i++)
        allBlocks[i]->simulationAboutToStart();
}

void CButtonBlockContainer::simulationEnded()
{
    for (int i=0;i<int(allBlocks.size());i++)
        allBlocks[i]->simulationEnded();
//  if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
//  {
//  }
}

CButtonBlock* CButtonBlockContainer::getButtonBlockWithID(int id)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if (allBlocks[i]->blockID==id)
            return(allBlocks[i]);
    }
    return(nullptr);
}

void CButtonBlockContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(allBlocks.size());i++)
    {
        int s=tt::getNameSuffixNumber(allBlocks[i]->getBlockName().c_str(),true);
        if (i==0)
        {
            minSuffix=s;
            maxSuffix=s;
        }
        else
        {
            if (s<minSuffix)
                minSuffix=s;
            if (s>maxSuffix)
                maxSuffix=s;
        }
    }
}

bool CButtonBlockContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allBlocks[i]->getBlockName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allBlocks[i]->getBlockName().c_str(),true));
            for (int j=0;j<int(allBlocks.size());j++)
            {
                int s2=tt::getNameSuffixNumber(allBlocks[j]->getBlockName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(allBlocks[j]->getBlockName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CButtonBlockContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allBlocks[i]->getBlockName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allBlocks[i]->getBlockName().c_str(),true));
            allBlocks[i]->setBlockName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

void CButtonBlockContainer::insertBlock(CButtonBlock* theNewBlock,bool objectIsACopy)
{
    insertBlockWithSuffixOffset(theNewBlock,objectIsACopy,1);
}

void CButtonBlockContainer::insertBlockWithSuffixOffset(CButtonBlock* theNewBlock,bool objectIsACopy,int suffixOffset)
{
    // We find a free handle:
    int blockHandle=SIM_IDSTART_2DELEMENT;
    while (getButtonBlockWithID(blockHandle)!=nullptr)
        blockHandle++;
    theNewBlock->setBlockID(blockHandle);
    // We check the name doesn't exist:
    std::string name=theNewBlock->getBlockName();
    if (objectIsACopy)
        name=tt::generateNewName_dash(name,suffixOffset);
    else
    {
        while (getBlockWithName(name)!=nullptr)
            name=tt::generateNewName_noDash(name);
    }
    theNewBlock->setBlockName(name);
    // We finally add the block:
    allBlocks.push_back(theNewBlock);
    App::setFullDialogRefreshFlag();
}


CButtonBlock* CButtonBlockContainer::getBlockWithID(int id)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if (allBlocks[i]->blockID==id)
            return(allBlocks[i]);
    }
    return(nullptr);
}

bool CButtonBlockContainer::removeBlockFromID(int id)
{
    App::ct->objCont->announce2DElementWillBeErased(id);
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if (allBlocks[i]->blockID==id)
        {
            delete allBlocks[i];
            allBlocks.erase(allBlocks.begin()+i);
            App::setFullDialogRefreshFlag();
            return(true);
        }
    }
    return(false);
}

void CButtonBlockContainer::deselectButtons()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    selectedButtons.clear();
    if (App::getEditModeType()&BUTTON_EDIT_MODE)
        App::setLightDialogRefreshFlag();
}
void CButtonBlockContainer::addToSelection(int pos)
{
    if (!isInSelection(pos))
        selectedButtons.push_back(pos);
    App::setLightDialogRefreshFlag();
}
void CButtonBlockContainer::removeFromSelection(int pos)
{
    for (int i=0;i<int(selectedButtons.size());i++)
    {
        if (selectedButtons[i]==pos)
        {
            selectedButtons.erase(selectedButtons.begin()+i);
            return;
        }
    }
    App::setLightDialogRefreshFlag();
}
void CButtonBlockContainer::xorAddToSelection(int pos)
{
    if (!isInSelection(pos))
        selectedButtons.push_back(pos);
    else
        removeFromSelection(pos);
    App::setLightDialogRefreshFlag();
}
bool CButtonBlockContainer::isInSelection(int pos)
{
    for (int i=0;i<int(selectedButtons.size());i++)
    {
        if (selectedButtons[i]==pos)
            return(true);
    }
    return(false);
}

bool CButtonBlockContainer::doesBlockIDExist(int id)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if (allBlocks[i]->blockID==id)
            return(true);
    }
    return(false);
}

CButtonBlock* CButtonBlockContainer::getBlockWithName(std::string name)
{
    for (int i=0;i<int(allBlocks.size());i++)
    {
        if (allBlocks[i]->getBlockName().compare(name)==0)
            return(allBlocks[i]);
    }
    return(nullptr);
}

void CButtonBlockContainer::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(allBlocks.size()))
    {
        if ( (allBlocks[i]->getAttributes()&sim_ui_property_systemblock)==0 )
        {
            int blockID=allBlocks[i]->getBlockID();
            if (allBlocks[i]->announce3DObjectWillBeErased(objID,false))
            { // We have to remove this button block
                removeBlockFromID(blockID);
                i=0; // Ordering may have changed
            }
            else
                i++;
        }
        else
            i++;
    }
}

void CButtonBlockContainer::a3DObjectWasSelected(int objID)
{
    for (int i=int(allBlocks.size())-1;i>=0;i--) // search direction inverted on 2009/04/10
    {   
        CButtonBlock* it=allBlocks[i];
        if ( (it->getAttributes()&sim_ui_property_systemblock)==0 )
        {
            if (it->getObjectIDAttachedTo()==objID)
                sendBlockToFront(it->getBlockID());
        }
    }
}

void CButtonBlockContainer::sendBlockToFront(int blockID)
{
    CButtonBlock* itBlock=getBlockWithID(blockID);
    if ( ( (itBlock->getAttributes()&sim_ui_property_systemblock)==0 )||(itBlock->getAttributes()&sim_ui_property_systemblockcanmovetofront) ) // second part since 15/10/2012
    {
        std::vector<CButtonBlock*> toFront;
        for (int i=0;i<int(allBlocks.size());i++)
        {
            CButtonBlock* aBlock=allBlocks[i];
            if (aBlock==itBlock)
            { // We add this one
                toFront.push_back(itBlock);
            }
        }
        // We remove the listed blocks and the clicked block:
        for (int i=0;i<int(toFront.size());i++)
        {
            for (int j=0;j<int(allBlocks.size());j++)
            {
                if (allBlocks[j]==toFront[i])
                {
                    allBlocks.erase(allBlocks.begin()+j);
                    break;
                }
            }
        }

        // We add everything at the end:
        for (int i=0;i<int(toFront.size());i++)
        {               
            if (toFront[i]!=itBlock)
                allBlocks.push_back(toFront[i]);
        }                                   
        allBlocks.push_back(itBlock);
    }
}

#ifdef SIM_WITH_GUI
void CButtonBlockContainer::displayAllBlocks(int currentView,bool dialogsHaveFocus)
{
    {
        if (infoBox!=nullptr)
        {
            static bool previousOpenState=true;
            int butt=infoBox->getLastEventButtonID(nullptr);
            if (butt==0)
                App::ct->mainSettings->infoWindowOpenState=!App::ct->mainSettings->infoWindowOpenState;
            if (butt==200)
            {
                App::ct->mainSettings->infoWindowColorStyle++;
                if (App::ct->mainSettings->infoWindowColorStyle>2)
                    App::ct->mainSettings->infoWindowColorStyle=0;
            }
            // Following needs to be run at every frame, since the amount of displayed info changes (when sim runs. for example). Tested!
            updateInfoWindowColorStyle(App::ct->mainSettings->infoWindowColorStyle);

            if (previousOpenState!=App::ct->mainSettings->infoWindowOpenState)
            {
                CSoftButton* it=infoBox->getButtonWithID(0);
                if (App::ct->mainSettings->infoWindowOpenState)
                {
                    infoBox->setAttributes((infoBox->getAttributes()|sim_ui_property_rolledup)-sim_ui_property_rolledup);
                    if (it!=nullptr)
                        it->setAttributes(it->getAttributes()|sim_buttonproperty_isdown);
                }
                else
                {
                    infoBox->setAttributes(infoBox->getAttributes()|sim_ui_property_rolledup);
                    if (it!=nullptr)
                        it->setAttributes((it->getAttributes()|sim_buttonproperty_isdown)-sim_buttonproperty_isdown);
                }
                previousOpenState=App::ct->mainSettings->infoWindowOpenState;
            }
        }
    }
    bool showInfoAndStatusBox= ((App::mainWindow==nullptr)||(!App::mainWindow->simulationRecorder->getIsRecording())||(!App::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar()) );
    bool editModeButNotButtonEditMode=((App::getEditModeType()!=NO_EDIT_MODE)&&(App::getEditModeType()!=BUTTON_EDIT_MODE));


    for (int i=0;i<int(allBlocks.size());i++)
    {
        CButtonBlock* it=allBlocks[i];
        if ( ((it->viewToAppearOn==-1)||(it->viewToAppearOn==currentView)) && (showInfoAndStatusBox||(it!=infoBox)) && ((!editModeButNotButtonEditMode)||(it==infoBox)) )
            it->displayBlock(winSize,(i==int(allBlocks.size())-1)&&(_lastBlockIdInFront!=allBlocks[i]->getBlockID()));
    }
    if (allBlocks.size()!=0)
        _lastBlockIdInFront=allBlocks[allBlocks.size()-1]->getBlockID();
}

bool CButtonBlockContainer::mouseDown(int xCoord,int yCoord,int currentView,int selectionStatus)
{
    bool showInfoAndStatusBox= ((App::mainWindow==nullptr)||(!App::mainWindow->simulationRecorder->getIsRecording())||(!App::mainWindow->simulationRecorder->getHideInfoTextAndStatusBar()) );
    bool editModeButNotButtonEditMode=((App::getEditModeType()!=NO_EDIT_MODE)&&(App::getEditModeType()!=BUTTON_EDIT_MODE));

    mousePos.x=xCoord;
    mousePos.y=yCoord;
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
        allBlocks[i]->clearCaughtElements(0xffff-sim_left_button);

    if (editMode)
    {
        setEditBoxEdition(-1,-1,true);
        bool dummyVal;
        shiftSelectionStart=-1;
        CButtonBlock* itBlock=nullptr;
        CSoftButton* itButton=nullptr;
        int buttID=-1;
        for (int i=(int)allBlocks.size()-1;i>=0;i--)
        {
            itBlock=allBlocks[i];
            if ( (itBlock->getAttributes()&sim_ui_property_systemblock)==0 )
            {
                if ((itBlock->viewToAppearOn==-1)||(itBlock->viewToAppearOn==currentView))
                {
                    setBlockInEdition(itBlock->blockID);
                    if (selectionStatus!=SHIFTSELECTION)
                    {
                        buttID=itBlock->mouseDownCatch(xCoord,yCoord,dummyVal,false);
                        if (buttID!=-1)
                        {
                            itButton=itBlock->getButtonWithID(buttID);
                            buttID=itButton->xPos+itButton->yPos*itBlock->blockWidth;
                            break;
                        }
                        else
                        {
                            buttID=itBlock->mouseDownCatchInexistant(xCoord,yCoord,false);
                            if (buttID!=-1)
                                break;
                        }
                    }
                    else
                    {
                        buttID=itBlock->mouseDownCatchInexistant(xCoord,yCoord,false);
                        if (buttID!=-1)
                            break;
                    }
                }
            }
        }
        if (buttID!=-1)
        { // We know that we don't have any system block here!
            setBlockInEdition(itBlock->blockID);
            // we put that block to the front:
            int found=-1;
            for (int i=0;i<int(allBlocks.size());i++)
            {
                if (allBlocks[i]->blockID==itBlock->blockID)
                {
                    found=i;
                    break;
                }
            }
            allBlocks.erase(allBlocks.begin()+found);
            allBlocks.push_back(itBlock);

            App::setLightDialogRefreshFlag();
            if (selectionStatus==CTRLSELECTION)
                xorAddToSelection(buttID);
            else if (selectionStatus==SHIFTSELECTION)
                shiftSelectionStart=buttID;
            else
            {
                deselectButtons();
                addToSelection(buttID);
            }
            return(true);
        }
        else
            return(false);
    }
    bool caught=false;
    CButtonBlock* itBlock=nullptr;
    CSoftButton* itButton=nullptr;
    bool cursorCatch;
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
    {
        itBlock=allBlocks[i];
        if ((itBlock->viewToAppearOn==-1)||(itBlock->viewToAppearOn==currentView))
        {
            if ( (showInfoAndStatusBox||(itBlock!=infoBox)) && ((!editModeButNotButtonEditMode)||(itBlock==infoBox)) )
            {
                cursorCatch=false;
                int buttID=itBlock->mouseDownCatch(xCoord,yCoord,cursorCatch,false);
                if (buttID!=-1)
                {
                    if ( (itBlock->getAttributes()&sim_ui_property_selectassociatedobject)&&(App::getEditModeType()==NO_EDIT_MODE) )
                    {
                        C3DObject* obj=App::ct->objCont->getObjectFromHandle(itBlock->getObjectIDAttachedTo());
                        if (obj!=nullptr)
                        {
                            // Modified on 2/6/2013 (was annoying when several objects were selected and an UI was clicked)
                            App::ct->objCont->removeObjectFromSelection(obj->getObjectHandle());
                            App::ct->objCont->addObjectToSelection(obj->getObjectHandle());
                        }
                    }
                    caught=true;
                    itButton=itBlock->getButtonWithID(buttID);
                    break;
                }
            }
        }
    }

    if (caught)
    {
        bool simNotPausedOrActiveAtPause=(!App::ct->simulation->isSimulationPaused())||(itBlock->getAttributes()&sim_ui_property_pauseactive);//getActiveDuringPause(); // New since 2010/10/29: 2DElements now stay visible (but inactive) during pause:
        if ((itButton->getAttributes()&sim_buttonproperty_enabled)&&(itButton->getButtonType()==sim_buttonproperty_button)&&simNotPausedOrActiveAtPause )
        {
            setEditBoxEdition(-1,-1,true);
            caughtBlock=itBlock->blockID;
            caughtButton=itButton->buttonID;
            caughtButtonDown=true;

            caughtBlockForDownUpEvent=-1;
            caughtButtonForDownUpEvent=-1;
            int p=sim_buttonproperty_staydown|sim_buttonproperty_isdown;
            if ( (itButton->getAttributes()&sim_buttonproperty_downupevent)&&((itButton->getAttributes()&p)==0) )
            { // prepare to generate a down event
                caughtBlockForDownUpEvent=itBlock->blockID;
                caughtButtonForDownUpEvent=itButton->buttonID;
                caughtButtonDownForDownUpEvent=false;
            }
        }
        else if ((itButton->getAttributes()&sim_buttonproperty_enabled)&&(itButton->getButtonType()==sim_buttonproperty_slider)&&simNotPausedOrActiveAtPause )
        {
            setEditBoxEdition(-1,-1,true);
            // We have to check whether we clicked the little cursor...
            if (cursorCatch)
            {
                caughtBlock=itBlock->blockID;
                caughtButton=itButton->buttonID;
                caughtBlockForDownUpEvent=-1;
                caughtButtonForDownUpEvent=-1;
                caughtButtonDown=true;
            }
            else
            {
                caughtBlock=-1;
                caughtButton=-1;
                caughtBlockForDownUpEvent=-1;
                caughtButtonForDownUpEvent=-1;
                caughtButtonDown=false;
            }
        }
        else if ((itButton->getAttributes()&sim_buttonproperty_enabled)&&(itButton->getButtonType()==sim_buttonproperty_editbox)&&simNotPausedOrActiveAtPause )
        {   // We just clicked an edit-box button
            if ( (!getEditBoxEdition())||(editBoxEditionPosition!=-1) )
                setEditBoxEdition(itBlock->blockID,itButton->buttonID,true);
            else
            {
                if ( (itBlock->blockID==editBoxInEditionBlock)&&(itButton->buttonID==editBoxInEditionButton) )
                    editBoxEditionPosition=(int)editBoxEditionText.length();
                else
                    setEditBoxEdition(itBlock->blockID,itButton->buttonID,true);
            }
            caughtBlock=-1;
            caughtButton=-1;
            caughtBlockForDownUpEvent=-1;
            caughtButtonForDownUpEvent=-1;
            caughtButtonDown=false;
        }
        else
        {
            setEditBoxEdition(-1,-1,true);
            caughtBlock=-1;
            caughtBlockForDownUpEvent=-1;
            caughtButtonForDownUpEvent=-1;
            if ( ( (itButton->getButtonType()==sim_buttonproperty_label)&&(itBlock->getAttributes()&sim_ui_property_moveable) )||(!simNotPausedOrActiveAtPause) )
                caughtBlock=itBlock->blockID; // So that it can be shifted
            caughtButton=-1;
            caughtButtonDown=false;
        }
        mouseCaptured=true;
        // Now we put that block to the front
        if (itBlock->getObjectIDAttachedTo()!=-1)
        { // If that block is attached to an object, then also put all other blocks attached to the same object to the front!
            for (int i=(int)allBlocks.size()-1;i>=0;i--)
            {
                CButtonBlock* itBlock2=allBlocks[i];
                if (itBlock2->getObjectIDAttachedTo()==itBlock->getObjectIDAttachedTo())
                    sendBlockToFront(itBlock2->getBlockID());
            }
            sendBlockToFront(itBlock->getBlockID()); // We send the clicked one again to the front
        }
        else
            sendBlockToFront(itBlock->getBlockID()); // We just send this one to the front
        return(true);
    }
    else
    {
        caughtBlock=-1;
        caughtButton=-1;
        caughtBlockForDownUpEvent=-1;
        caughtButtonForDownUpEvent=-1;
        caughtButtonDown=false;
        mouseCaptured=false;
        return(false);
    }
}

bool CButtonBlockContainer::leftMouseButtonDoubleClick(int xCoord,int yCoord,int currentView)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    bool editModeButNotButtonEditMode=((App::getEditModeType()!=NO_EDIT_MODE)&&(App::getEditModeType()!=BUTTON_EDIT_MODE));
    if (editModeButNotButtonEditMode)
        return(false);

    if (mouseDownTest(xCoord,yCoord,currentView))
    { // not handled for now
//      mouseDown(xCoord,yCoord,currentView,NOSELECTION);
//      mouseUp(xCoord,yCoord,currentView);
        return(true);
    }
    return(false);
}

bool CButtonBlockContainer::mouseDownTest(int xCoord,int yCoord,int currentView)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    // This routine is just to check whether the mouse is on a button or not. No action!!
    CButtonBlock* itBlock=nullptr;
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
    {
        itBlock=allBlocks[i];
        if ((itBlock->viewToAppearOn==-1)||(itBlock->viewToAppearOn==currentView))
        {
            bool dummyVal;
            int buttID=itBlock->mouseDownCatch(xCoord,yCoord,dummyVal,true);
            if (buttID!=-1)
                return(true);
            if ( (itBlock->blockID==getBlockInEdition())&&editMode)
            {
                buttID=itBlock->mouseDownCatchInexistant(xCoord,yCoord,true);
                if (buttID!=-1)
                    return(true);
            }
        }
    }
    return(false);
}

void CButtonBlockContainer::onKeyDown(unsigned int key)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        if ( (key==ENTER_KEY)||(key==TAB_KEY) ) // Enter or tab-key pressed
        {
            CButtonBlock* itBlock=getBlockWithID(editBoxInEditionBlock);
            if (itBlock!=nullptr)
            {
                CSoftButton* itButton=itBlock->getButtonWithID(editBoxInEditionButton);
                if (itButton!=nullptr)
                {
                    if (itButton->label!=editBoxEditionText)
                        setEditBoxEdition(-1,-1,true); // We leave the edition mode and apply changes
                }
            }
            setEditBoxEdition(-1,-1,false); // We leave the edition mode (changes were already applied above)
            return;
        }

        if ( (key==LEFT_KEY)||(key==RIGHT_KEY) )
        {
            if (editBoxEditionPosition==-1)
                editBoxEditionPosition=(int)editBoxEditionText.length();
            if ( (key==LEFT_KEY)&&(editBoxEditionPosition>0) )
                editBoxEditionPosition--;
            if ( (key==RIGHT_KEY)&&(editBoxEditionPosition<int(editBoxEditionText.length())) )
                editBoxEditionPosition++;
            return;
        }
        if (key==BACKSPACE_KEY) // Backspace
        {
            if (editBoxEditionPosition==-1)
            {
                editBoxEditionText="";
                editBoxEditionPosition=0;
            }
            else
            {
                if (editBoxEditionText.length()!=0)
                {
                    if (editBoxEditionPosition>0)
                    {
                        editBoxEditionText.erase(editBoxEditionText.begin()+editBoxEditionPosition-1);
                        editBoxEditionPosition--;
                    }
                }
            }
        }
        if (key==DELETE_KEY) // Delete-key
        {
            if (editBoxEditionPosition==-1)
            {
                editBoxEditionText="";
                editBoxEditionPosition=0;
            }
            else
            {
                if (editBoxEditionText.length()!=0)
                {
                    if (editBoxEditionPosition<int(editBoxEditionText.length()))
                        editBoxEditionText.erase(editBoxEditionText.begin()+editBoxEditionPosition);
                }
            }
        }
        if ( (key>=' ')&&(key<127) )
        {
            if (editBoxEditionPosition==-1)
            {
                editBoxEditionPosition=0;
                editBoxEditionText="";
            }
            editBoxEditionText.insert(editBoxEditionText.begin()+editBoxEditionPosition,(char)key);
            editBoxEditionPosition++;
        }
    }
    if (App::getEditModeType()==BUTTON_EDIT_MODE)
    {
        CSoftButton* singleTextableButton=nullptr;
        CSoftButton* singleButtonWithText=nullptr;
        CButtonBlock* itBlock=getBlockWithID(getBlockInEdition());
        if ((itBlock!=nullptr)&&(selectedButtons.size()==1))
        {
            VPoint size;
            itBlock->getBlockSize(size);
            int val=selectedButtons[0];
            singleTextableButton=itBlock->getButtonAtPos(val%size.x,val/size.x);
            if (singleTextableButton!=nullptr)
            {
                if (singleTextableButton->getButtonType()!=sim_buttonproperty_slider)
                {
                    singleButtonWithText=singleTextableButton;
                    if (singleButtonWithText->getAttributes()&sim_buttonproperty_isdown)
                    {
                        if (singleButtonWithText->downLabel.size()==0)
                            singleButtonWithText=nullptr;
                    }
                    else
                    {
                        if (singleButtonWithText->label.size()==0)
                            singleButtonWithText=nullptr;
                    }
                }
                else
                    singleTextableButton=nullptr;
            }
        }

        if (key==DELETE_KEY)
            App::mainWindow->editModeContainer->processCommand(UI_EDIT_MODE_DELETE_SELECTED_BUTTONS_EMCMD,nullptr);
        if (key==BACKSPACE_KEY)
        {
            if (singleButtonWithText==nullptr)
                App::mainWindow->editModeContainer->processCommand(UI_EDIT_MODE_DELETE_SELECTED_BUTTONS_EMCMD,nullptr);
            else
            {
                if (singleButtonWithText->getAttributes()&sim_buttonproperty_isdown)
                    singleButtonWithText->downLabel.resize(singleButtonWithText->downLabel.size()-1);
                else
                    singleButtonWithText->label.resize(singleButtonWithText->label.size()-1);
                App::setFullDialogRefreshFlag();
            }
        }
        if ( (key>=' ')&&(key<127)&&(singleTextableButton!=nullptr) )
        {
            if (singleTextableButton->getAttributes()&sim_buttonproperty_isdown)
                singleTextableButton->downLabel+=char(key);
            else
                singleTextableButton->label+=char(key);
            App::setFullDialogRefreshFlag();
        }
    }
}

bool CButtonBlockContainer::mouseUp(int xCoord,int yCoord,int currentView)
{
    if ( editMode&&(shiftSelectionStart!=-1) )
    {
        CButtonBlock* itBlock=nullptr;
        int buttID=-1;
        for (int i=(int)allBlocks.size()-1;i>=0;i--)
        {
            itBlock=allBlocks[i];
            if ((itBlock->viewToAppearOn==-1)||(itBlock->viewToAppearOn==currentView))
            {
                if (itBlock->blockID==getBlockInEdition())
                {
                    buttID=itBlock->mouseDownCatchInexistant(xCoord,yCoord,false);
                    if (buttID!=-1)
                        break;
                }
            }
        }
        if (buttID!=-1)
        {
            if (itBlock->blockID==getBlockInEdition())
            {
                VPoint start(shiftSelectionStart%itBlock->blockWidth,shiftSelectionStart/itBlock->blockWidth);
                VPoint end(buttID%itBlock->blockWidth,buttID/itBlock->blockWidth);
                int dx=1;
                if (end.x-start.x!=0)
                    dx=(end.x-start.x)/abs(end.x-start.x);
                int dy=1;
                if (end.y-start.y!=0)
                    dy=(end.y-start.y)/abs(end.y-start.y);
                for (int j=start.y;j!=end.y+dy;j+=dy)
                {
                    for (int i=start.x;i!=end.x+dx;i+=dx)
                    {
                        CSoftButton* aButton=itBlock->getButtonAtPos(i,j);
                        if (aButton==nullptr)
                            addToSelection(i+itBlock->blockWidth*j);
                        else
                            addToSelection(aButton->xPos+itBlock->blockWidth*aButton->yPos);
                    }
                }

            }
            shiftSelectionStart=-1;
            return(true);
        }
        else
        {
            shiftSelectionStart=-1;
            return(false);
        }
    }

    if (!mouseCaptured)
        return(false);
    // Here we check if a slider was clicked
    CButtonBlock* aa=getBlockWithID(caughtBlock);
    if (aa!=nullptr)
    {
        CSoftButton* bb=aa->getButtonWithID(caughtButton);
        if (bb!=nullptr)
        {
            if (bb->getButtonType()==sim_buttonproperty_slider)
            {
                App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,caughtBlock,caughtButton,bb->getAttributes(),int(500.0f*(bb->getSliderPos()+1.0f)),nullptr,0);
                int auxVals[2]={bb->getAttributes(),int(500.0f*(bb->getSliderPos()+1.0f))};
                aa->setLastEventButtonID(caughtButton,auxVals);
                caughtBlock=-1;
                caughtButton=-1;
                caughtBlockForDownUpEvent=-1;
                caughtButtonForDownUpEvent=-1;
                mouseCaptured=false;
                caughtButtonDown=false;
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                return(true);
            }
        }
    }

    bool caught=false;
    CButtonBlock* itBlock=nullptr;
    CSoftButton* itButton=nullptr;
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
    {
        bool dummyVal;
        itBlock=allBlocks[i];
        int buttID=itBlock->mouseDownCatch(xCoord,yCoord,dummyVal,false);
        if (buttID!=-1)
        {
            caught=true;
            itButton=itBlock->getButtonWithID(buttID);
            break;
        }
    }
    if (caught)
    {
        if ((itBlock->blockID==caughtBlock)&&(itButton->buttonID==caughtButton))
        {   // Here a button was pressed
            if (itButton->getAttributes()&sim_buttonproperty_staydown)
                itButton->setAttributes(itButton->getAttributes()^sim_buttonproperty_isdown);

            if ( (itButton->getAttributes()&sim_buttonproperty_downupevent)==0 )
            { // buttons with up/down events are handled elsewhere!
                App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,caughtBlock,caughtButton,itButton->getAttributes(),itButton->getAttributes()^sim_buttonproperty_isdown,nullptr,0);
                int auxVals[2]={itButton->getAttributes(),itButton->getAttributes()^sim_buttonproperty_isdown};
                itBlock->setLastEventButtonID(caughtButton,auxVals);
            }
            // Now we check if the button is a rollup/rolldown button:
            if ( (itButton->getAttributes()&sim_buttonproperty_rollupaction)&&(itButton->getAttributes()&sim_buttonproperty_staydown))
            {
                if ((itButton->getAttributes()&sim_buttonproperty_isdown)==0)
                    itBlock->setAttributes(itBlock->getAttributes()|sim_ui_property_rolledup);
                else
                    itBlock->setAttributes((itBlock->getAttributes()|sim_ui_property_rolledup)-sim_ui_property_rolledup);
            }
            // Now we check if the button is a close-up button:
            if (itButton->getAttributes()&sim_buttonproperty_closeaction)
                itBlock->setAttributes((itBlock->getAttributes()|sim_ui_property_visible)-sim_ui_property_visible);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        caughtBlock=-1;
        caughtButton=-1;

        // Now make sure you post the up event for down/up event buttons (if not already posted!)
        if ( (App::ct->buttonBlockContainer->caughtBlockForDownUpEvent!=-1)&&(App::ct->buttonBlockContainer->caughtButtonForDownUpEvent!=-1) )
        {
            CButtonBlock* udBlock=getBlockWithID(App::ct->buttonBlockContainer->caughtBlockForDownUpEvent);
            CSoftButton* udButton=nullptr;
            if (udBlock!=nullptr)
                udButton=udBlock->getButtonWithID(App::ct->buttonBlockContainer->caughtButtonForDownUpEvent);
            if ( (udButton!=nullptr)&&App::ct->buttonBlockContainer->caughtButtonDownForDownUpEvent)
            { // We have to generate an up event:
                App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,App::ct->buttonBlockContainer->caughtBlockForDownUpEvent,App::ct->buttonBlockContainer->caughtButtonForDownUpEvent,udButton->getAttributes(),0,nullptr,0);
                int auxVals[2]={udButton->getAttributes(),0};
                udBlock->setLastEventButtonID(App::ct->buttonBlockContainer->caughtButtonForDownUpEvent,auxVals);
            }
        }
        caughtBlockForDownUpEvent=-1;
        caughtButtonForDownUpEvent=-1;

        mouseCaptured=false;
        caughtButtonDown=false;
        return(true);
    }
    else
    {
        caughtBlock=-1;
        caughtButton=-1;
        caughtBlockForDownUpEvent=-1;
        caughtButtonForDownUpEvent=-1;
        mouseCaptured=false;
        caughtButtonDown=false;
        return(false);
    }
}

bool CButtonBlockContainer::mouseMove(int xCoord,int yCoord)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (!mouseCaptured)
    {
        mousePos.x=xCoord; // Needed to see the shift-selection
        mousePos.y=yCoord;
        return(false);
    }
    if ( (caughtBlock!=-1)&&(caughtButton==-1) )
    { // Here we can shift the block
        CButtonBlock* itBlock=getBlockWithID(caughtBlock);
        if (itBlock!=nullptr)
        { // Important (a block can be destroyed during shifting (e.g. through a script!)
            VPoint blockPos;
            itBlock->getBlockPositionRelative(blockPos);
            if (itBlock->getAttributes()&sim_ui_property_relativetotopborder)
                blockPos.y=blockPos.y-(yCoord-mousePos.y);
            else
                blockPos.y=blockPos.y+(yCoord-mousePos.y);

            if (itBlock->getAttributes()&sim_ui_property_relativetoleftborder)
                blockPos.x=blockPos.x+(xCoord-mousePos.x);
            else
                blockPos.x=blockPos.x-(xCoord-mousePos.x);
            itBlock->setDesiredBlockPosition(blockPos.x,blockPos.y);
        }
        mousePos.x=xCoord;
        mousePos.y=yCoord;
        return(true);
    }
    // Here we handle sliders:
    CButtonBlock* aa=getBlockWithID(caughtBlock);
    if (aa!=nullptr)
    {
        CSoftButton* bb=aa->getButtonWithID(caughtButton);
        if (bb!=nullptr)
        {
            if (bb->getButtonType()==sim_buttonproperty_slider)
            { // We have to move the slider
                if (!bb->getVertical())
                { // Horizontal slider
                    float buttXSize=((float)((aa->getButtonWidthRetina()+2)*bb->getLength()-2))*(1.0f-(float)BUTTON_SLIDER_X_SIZE);
                    float dx=(xCoord-mousePos.x)*2.0f/buttXSize;
                    bb->setSliderPos(bb->getSliderPos()+dx);
                }
                else
                { // Vertical slider
                    float buttYSize=((float)((aa->getButtonHeightRetina()+2)*bb->getHeight()-2))*(1.0f-(float)BUTTON_SLIDER_X_SIZE);
                    float dy=(yCoord-mousePos.y)*2.0f/buttYSize;
                    bb->setSliderPos(bb->getSliderPos()-dy);
                }
                mousePos.x=xCoord;
                mousePos.y=yCoord;
                App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,caughtBlock,caughtButton,bb->getAttributes(),int(500.0f*(bb->getSliderPos()+1.0f)),nullptr,0);
                int auxVals[2]={bb->getAttributes(),int(500.0f*(bb->getSliderPos()+1.0f))};
                aa->setLastEventButtonID(caughtButton,auxVals);
                return(true);
            }
        }
    }


    for (int i=0;i<int(allBlocks.size());i++)
    {
        CButtonBlock* itBlock=allBlocks[i];
        if (itBlock->blockID==caughtBlock)
        {
            itBlock->mouseMoveCatch(xCoord,yCoord);
            break;
        }
    }
    return(true);
}

void CButtonBlockContainer::looseFocus()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    setEditBoxEdition(-1,-1,true);
    deselectButtons();
    if ( (App::ct->objCont!=nullptr)&&((App::getEditModeType()&BUTTON_EDIT_MODE)==0) )
    { // In the button edit mode, we don't want this to happen
        setBlockInEdition(-1);
    }
}

void CButtonBlockContainer::setEditBoxEdition(int block,int button,bool applyChangesIfLeavingEditMode)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (applyChangesIfLeavingEditMode)
    { // New since 25/02/2011 (so that we only cancel input if we pressed esc!)
        CButtonBlock* itBlock=getBlockWithID(editBoxInEditionBlock);
        if (itBlock!=nullptr)
        {
            CSoftButton* itButton=itBlock->getButtonWithID(editBoxInEditionButton);
            if (itButton!=nullptr)
            {
                if (itButton->label!=editBoxEditionText)
                {
                    itButton->label=editBoxEditionText;
                    App::ct->outsideCommandQueue->addCommand(sim_message_ui_button_state_change,editBoxInEditionBlock,editBoxInEditionButton,itButton->getAttributes(),0,nullptr,0);
                    int auxVals[2]={itButton->getAttributes(),0};
                    itBlock->setLastEventButtonID(editBoxInEditionButton,auxVals);
                }
            }
        }
    }
    editBoxEditionPosition=-1;
    editBoxInEditionBlock=block;
    editBoxInEditionButton=button;
    CButtonBlock* itBlock=getBlockWithID(editBoxInEditionBlock);
    if (itBlock!=nullptr)
    {
        CSoftButton* itButton=itBlock->getButtonWithID(editBoxInEditionButton);
        if (itButton!=nullptr)
            editBoxEditionText=itButton->label;
    }
}

bool CButtonBlockContainer::getEditBoxEdition()
{
    return(editBoxInEditionBlock!=-1);
}

CSoftButton* CButtonBlockContainer::getInfoBoxButton(int index,int subIndex)
{
    if (infoBox==nullptr)
        return(nullptr);
    if ((index>=INFO_BOX_ROW_COUNT)||(index<0))
        return(nullptr);
    if ((subIndex<0)||(subIndex>=2))
        return(nullptr);
    if (infoBoxButtonHandlers[2*index+subIndex]<0)
        return(nullptr);
    return(infoBox->getButtonWithID(infoBoxButtonHandlers[2*index+subIndex]));
}

CButtonBlockContainer* CButtonBlockContainer::loadSystemButtonBlocks(std::string fullPathAndFilename)
{
    FUNCTION_DEBUG;
    if (VFile::doesFileExist(fullPathAndFilename))
    {
        try
        {
            CSer ar(fullPathAndFilename.c_str(),CSer::filetype_vrep_bin_ui_file);
            int serializationVersion;
            unsigned short dum0;
            int dum1;
            char dum2;
            int result=ar.readOpenBinary(serializationVersion,dum0,dum1,dum2,false);
            CButtonBlockContainer* cont=new CButtonBlockContainer(false);
            if (result==1)
            {
                if (infoBox!=nullptr)
                {
                    if (infoBox->getButtonWithID(0)!=nullptr)
                        infoBox->getButtonWithID(0)->setAttributes(infoBox->getButtonWithID(0)->getAttributes()|sim_buttonproperty_isdown);
                }
                std::vector<CButtonBlock*> loadedButtonBlockList;
                int byteQuantity;
                std::string theName="";
                while (theName.compare(SER_END_OF_FILE)!=0)
                {
                    theName=ar.readDataName();
                    if (theName.compare(SER_END_OF_FILE)!=0)
                    {
                        bool noHit=true;
                        if (theName.compare(SER_END_OF_OBJECT)==0) // Probably we can remove those two lines...
                            noHit=false;
                        if (theName.compare(SER_BUTTON_BLOCK)==0)
                        {
                            ar >> byteQuantity;
                            CButtonBlock* it=new CButtonBlock(1,1,10,10,0);
                            it->serialize(ar);
                            loadedButtonBlockList.push_back(it);
                            noHit=false;
                        }
                        if (noHit)
                            ar.loadUnknownData();
                    }
                }
                // We add all blocks and perform the blockMapping
                std::vector<int> blockMapping;
                for (int i=0;i<int(loadedButtonBlockList.size());i++)
                {
                    blockMapping.push_back(loadedButtonBlockList[i]->getBlockID()); // Old ID
                    cont->insertBlock(loadedButtonBlockList[i],true);
                    blockMapping.push_back(loadedButtonBlockList[i]->getBlockID()); // New ID
                }
                ar.readClose();
            }
            else
            {
                delete cont;
                cont=nullptr;
            }
            return(cont);
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
            return(nullptr);
        }
    }
    return(nullptr);
}

bool CButtonBlockContainer::getButtonEditMode_editMode()
{
    return(editMode);
}

void CButtonBlockContainer::setButtonEditMode_editMode(bool isOn)
{
    if (!App::ct->simulation->isSimulationRunning())
    {
        setEditBoxEdition(-1,-1,false);
        editMode=isOn;
        deselectButtons();
    }
    if (!editMode)
    {
        delete copyOfBlock_forEditMode;
        copyOfBlock_forEditMode=nullptr;
    }
}

void CButtonBlockContainer::copyBlockInEdition_editMode()
{
    if (editMode)
    {
        delete copyOfBlock_forEditMode;
        copyOfBlock_forEditMode=nullptr;
        CButtonBlock* it=getBlockWithID(getBlockInEdition());
        if (it!=nullptr)
            copyOfBlock_forEditMode=it->copyYourself();
        setBlockInEdition(-1);
    }
}

void CButtonBlockContainer::pasteCopiedBlock_editMode()
{
    if (editMode)
    {
        if (copyOfBlock_forEditMode!=nullptr)
        {
            CButtonBlock* it=copyOfBlock_forEditMode->copyYourself();
            insertBlock(it,false);
            setBlockInEdition(it->getBlockID());
        }
    }
}

void CButtonBlockContainer::deleteBlockInEdition_editMode()
{
    if (editMode)
    {
        CButtonBlock* it=getBlockWithID(getBlockInEdition());
        if (it!=nullptr)
            removeBlockFromID(it->getBlockID());
        setBlockInEdition(-1);
    }
}

bool CButtonBlockContainer::deleteSelectedButtons_editMode()
{
    if (editMode)
    {
        CButtonBlock* itBlock=getBlockWithID(getBlockInEdition());
        if (itBlock==nullptr)
            return(false);
        if (selectedButtons.size()<1)
            return(false);
        VPoint size;
        itBlock->getBlockSize(size);
        for (int i=0;i<int(selectedButtons.size());i++)
        {
            int val=selectedButtons[i];
            itBlock->removeButtonFromPos(val%size.x,val/size.x,false);
        }
        itBlock->recomputePositionFastIndex();
        return(true);
    }
    return(false);
}

void CButtonBlockContainer::setBlockInEdition(int blockID)
{
    _blockInEditMode=blockID;
}

int CButtonBlockContainer::getBlockInEdition()
{
    return(_blockInEditMode);
}

int CButtonBlockContainer::getCaughtElements()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int retVal=0;
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
        retVal|=allBlocks[i]->getCaughtElements();
    return(retVal);
}

void CButtonBlockContainer::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=(int)allBlocks.size()-1;i>=0;i--)
        allBlocks[i]->clearCaughtElements(keepMask);
}

CButtonBlock* CButtonBlockContainer::getCopyOfBlock_forEditMode()
{
    return(copyOfBlock_forEditMode);
}
#endif
