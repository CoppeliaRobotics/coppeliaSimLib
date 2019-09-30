
#pragma once

#include "buttonBlock.h"
#include "mainCont.h"
#include "VPoint.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif


const int INFO_BOX_ROW_COUNT=20;

class CButtonBlockContainer : public CMainCont
{
public:

    CButtonBlockContainer(bool mainContainer);
    virtual ~CButtonBlockContainer();

    void emptySceneProcedure(bool mainContainer);
    void updateInfoWindowColorStyle(int colorStyle);
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    CButtonBlock* getButtonBlockWithID(int id);
    void removeAllBlocks(bool alsoSystemBlocks);
    void announceObjectWillBeErased(int objID);
    void setTextureDependencies();

    void insertBlock(CButtonBlock* theNewBlock,bool objectIsACopy);
    void insertBlockWithSuffixOffset(CButtonBlock* theNewBlock,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);

    CButtonBlock* getBlockWithID(int id);
    CButtonBlock* getBlockWithName(std::string name);
    bool removeBlockFromID(int id);

    bool doesBlockIDExist(int id);
    void deselectButtons();
    void addToSelection(int pos);
    void removeFromSelection(int pos);
    void xorAddToSelection(int pos);
    bool isInSelection(int pos);
    void setViewSizeAndPosition(int sX,int sY,int pX,int pY);

    void a3DObjectWasSelected(int objID);
    void sendBlockToFront(int blockID);
    int getNonSystemBlockCount();
    int getUnassociatedNonSystemBlockCount();

    int winSize[2];
    int winPos[2];
    int caughtBlock;
    int caughtButton;

    int caughtBlockForDownUpEvent;
    int caughtButtonForDownUpEvent;
    bool caughtButtonDownForDownUpEvent;

    int editBoxInEditionBlock;
    int editBoxInEditionButton;
    int editBoxEditionPosition;
    std::string editBoxEditionText;
    bool caughtButtonDown;
    bool mouseCaptured;
    VPoint mousePos;
    bool editMode;
    bool buttonsLocked;
    std::vector<int> selectedButtons;
    std::vector<CButtonBlock*> allBlocks;
    int shiftSelectionStart;

#ifdef SIM_WITH_GUI
    void displayAllBlocks(int currentView,bool dialogsHaveFocus);
    bool mouseDown(int xCoord,int yCoord,int currentView,int selectionStatus);
    bool mouseDownTest(int xCoord,int yCoord,int currentView);
    bool leftMouseButtonDoubleClick(int xCoord,int yCoord,int currentView);
    bool mouseUp(int xCoord,int yCoord,int currentView);
    bool mouseMove(int xCoord,int yCoord);
    void onKeyDown(unsigned int key);
    void looseFocus();
    void setEditBoxEdition(int block,int button,bool applyChangesIfLeavingEditMode);
    bool getEditBoxEdition();
    CSoftButton* getInfoBoxButton(int index,int subIndex);
    CButtonBlockContainer* loadSystemButtonBlocks(std::string fullPathAndFilename);
    void setButtonEditMode_editMode(bool isOn);
    bool getButtonEditMode_editMode();
    void copyBlockInEdition_editMode();
    void pasteCopiedBlock_editMode();
    void deleteBlockInEdition_editMode();
    bool deleteSelectedButtons_editMode();
    void setBlockInEdition(int blockID);
    int getBlockInEdition();
    int getCaughtElements();
    void clearCaughtElements(int keepMask);
    CButtonBlock* getCopyOfBlock_forEditMode();
#endif

private:
    int _blockInEditMode;
    CButtonBlock* copyOfBlock_forEditMode;
    CButtonBlock* infoBox;
    int infoBoxButtonHandlers[INFO_BOX_ROW_COUNT*2];

    int newSceneProcedurePasses;
    int _lastBlockIdInFront;
};
