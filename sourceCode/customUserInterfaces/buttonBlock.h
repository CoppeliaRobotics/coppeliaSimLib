#pragma once

#include <softButton.h>

class CButtonBlockContainer;

class CButtonBlock  
{
public:
    
    CButtonBlock(int theBlockWidth,int theBlockHeight,
                           int theButtonWidth,int theButtonHeight,
                           int theView);
    virtual ~CButtonBlock();
    void commonInit();
    CSoftButton* getButtonWithID(int id);
    CSoftButton* getButtonWithUniqueID(int id);
    CSoftButton* getButtonAtPos(int x,int y);
    CSoftButton* getButtonAtPosDontUseFastIndex(int x,int y);
    void getAllAttachedTextureProperties(std::vector<CTextureProperty*>& textPropVect);
    bool removeButtonFromPos(int x,int y,bool updateFastIndex=true);
    bool insertButton(CSoftButton* theNewButton);
    bool insertButtonWithoutChecking(CSoftButton* theNewButton);
    void setDesiredBlockPosition(int x,int y);
    void setBlockSize(VPoint size);
    void getBlockSize(VPoint& size);
    void getDesiredBlockPosition(VPoint& pos);
    void getBlockPositionRelative(VPoint& pos);
    void getBlockPositionAbsolute(VPoint& pos);
    bool doesButtonIDExist(int id);
    void serialize(CSer &ar);
    void setButtonSizeOriginal(VPoint theSize);
    void getButtonSizeOriginal(VPoint& theSize);
    void getButtonSizeRetina(VPoint& theSize);
    int getButtonWidthOriginal();
    int getButtonHeightOriginal();
    int getButtonWidthRetina();
    int getButtonHeightRetina();


    CButtonBlock* copyYourself();
    void recomputePositionFastIndex();
    void setRollupMin(VPoint rollupMinVal);
    void getRollupMin(VPoint& rollupMinVal);
    void setRollupMax(VPoint rollupMaxVal);
    void getRollupMax(VPoint& rollupMaxVal);
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();
    void setBlockID(int newBlockID);
    int getBlockID();
    void setViewToAppearOn(int theView);
    int getViewToAppearOn();
    int getFreeButtonHandle();
    void performSceneObjectLoadingMapping(const std::map<int,int>* map);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    bool announceSceneObjectWillBeErased(int objectID,bool copyBuffer);
    void setTextureDependencies();
    void removeAllVisionSensorTextures();
    void removeAllObjectAttachements();

    void setBlockName(std::string name);
    void setGenericDialog(bool genericDialog);
    bool isGenericDialog();
    std::string getBlockName();
    int getObjectIDAttachedTo();
    void setObjectIDAttachedTo(int objID);

    void setAttributes(int attr);
    int getAttributes();
    CTextureProperty* getTextureProperty();
    void setTextureProperty(CTextureProperty* tp);

    // Variables which need to be serialized & copied
    int blockID;
    int viewToAppearOn;
    int blockWidth;
    int blockHeight;


    std::vector<CSoftButton*> allButtons;

protected:
    VPoint _getBlockSizeAndOtherButtonSizeAndPos(VPoint& blockSize,VPoint& blockPos,VPoint& buttonSize,VPoint& buttonPos,VPoint& otherButtonSize,CSoftButton* button);

    // Variables which need to be serialized & copied
    int _attributes;
    VPoint blockPosition;
    VPoint desiredBlockPosition;
    VPoint rollupMin;
    VPoint rollupMax;
    std::string blockName;
    int objectIDAttachedTo;
    CTextureProperty* _textureProperty;
    int _buttonWidth;
    int _buttonHeight;


    // Other:
    std::vector<int> positionFastIndex;
    bool _genericDialog;

    // Don't serialize:
    int _copyPositionOffset;
    int _caughtElements;
    int _lastEventButtonID;
    int _lastEventButtonAuxVals[2];

    bool _initialValuesInitialized;
    int _initialAttributes;
    int _initialPos[2];

#ifdef SIM_WITH_GUI
public:
    void displayBlock(int winSize[2],bool justCameToFront);
    bool isButtonInVisibleZone(CSoftButton* it);
    bool isDisplayedNow();
    int getCaughtElements();
    void clearCaughtElements(int keepMask);
    void setLastEventButtonID(int id,int auxVals[2]);
    int getLastEventButtonID(int auxVals[2]);
    int mouseDownCatch(int xCoord,int yCoord,bool& cursorCatch,bool test);
    int mouseDownCatchInexistant(int xCoord,int yCoord,bool test);
    void mouseMoveCatch(int xCoord,int yCoord);
#endif
};
