// This file requires some serious refactoring!

#pragma once

#include "sView.h"
#include "ser.h"

class CSPage  
{
public:
    CSPage(int thePageType);
    virtual ~CSPage();
    void render();
    void initializeInitialValues(bool simulationAlreadyRunning,int initializeOnlyForThisNewObject);
    void simulationEnded();
    void serialize(CSer& ar);
    bool setPageType(int newType);
    int getPageType() const;
    void announceObjectWillBeErased(int objectID);
    void performObjectLoadingMapping(const std::map<int,int>* map);
    void getPageSizeAndPosition(int& sizeX,int& sizeY,int& posX,int& posY) const;
    void setPageSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    CSView* getView(size_t viewIndex);
    void getViewSizeAndPosition(int sViewSize[2],int sViewPos[2],size_t subViewIndex) const;
    void getFloatingViewRelativeSizeAndPosition(floatDouble sViewSize[2],floatDouble sViewPos[2],size_t subViewIndex) const;
    void setViewSizesAndPositions();
    size_t getRegularViewCount() const;
    size_t getViewCount() const;
    void addFloatingView();
    void addFloatingView(CSView* theFloatingView,floatDouble relSize[2],floatDouble relPos[2]);
    void getBorderCorrectedFloatingViewPosition(int posX,int posY,int sizeX,int sizeY,int& newPosX,int& newPosY) const;
    bool removeFloatingView(size_t viewIndex);
    int getViewIndexFromViewUniqueID(int uniqueID) const;
    bool isViewValid(CSView* v) const;

private:
    // Variables which need to be serialized:
    int _pageType;
    int _lastMouseDownViewIndex;
    std::vector<CSView*> _allViews;
    // Positions and sizes are relative now (2009/05/22)
    std::vector<floatDouble> _allViewAuxSizesAndPos;

    std::vector<floatDouble> _initialAuxViewSizesAndPos;
    std::vector<int> _initialAuxViewUniqueIDs;
    bool _initialValuesInitialized;

    // Variables which don't need to be serialized:
    int _pagePosition[2];
    int _pageSize[2];
    int _caughtElements;
    int mouseRelativePosition[2];
    int previousMouseRelativePosition[2];

    int auxViewResizingAction;
    int viewIndexOfResizingAction;
    // Positions and sizes are relative now (2009/05/22)
    floatDouble floatingViewAddOffset;

#ifdef SIM_WITH_GUI
public:
    int getCaughtElements() const;
    void clearCaughtElements(int keepMask);
    bool viewIsPassive(size_t viewIndex) const;
    void swapViews(size_t index1,size_t index2,bool alsoSizeAndPosInfo);
    size_t bringViewToFrontIfPossible(size_t index);
    void getViewRelativeMousePosition(int mouseX,int mouseY,int& relMouseX,int& relMouseY,size_t index) const;
    int getViewIndexOfMousePosition(int mouseX,int mouseY) const;
    bool doubleClickActionForView(size_t viewIndex);
    void clearAllMouseJustWentDownAndUpFlags();
    int getMousePosRelativeToFloatingViewBorders(int mouseX,int mouseY,size_t index) const;
    void clearLastMouseDownViewIndex();
    int getLastMouseDownViewIndex() const;
    bool leftMouseButtonDown(int x,int y,int selectionStatus);
    bool getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const;
    void leftMouseButtonUp(int x,int y);
    int getCursor(int x,int y) const;
    void mouseMove(int x,int y,bool passiveAndFocused);
    int modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition);
    void mouseWheel(int deltaZ,int x,int y);
    bool middleMouseButtonDown(int x,int y);
    void middleMouseButtonUp(int x,int y);
    bool rightMouseButtonDown(int x,int y);
    bool rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    bool leftMouseButtonDoubleClick(int x,int y,int selectionStatus);
#endif
};
