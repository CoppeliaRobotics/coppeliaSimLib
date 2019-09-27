#pragma once

#include "vrepMainHeader.h"
#include "mainCont.h"
#include "sPage.h"
#include "global.h"
#include "buttonBlock.h"

#define PAGES_COUNT 8

class CPageContainer : public CMainCont
{
public:
    CPageContainer();
    virtual ~CPageContainer();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void emptySceneProcedure();
    void initializeInitialValues(bool simulationIsRunning,int initializeOnlyForThisNewObject);
    void removeAllPages();
    void setUpDefaultPages(bool createASingleView=false);
    void announceObjectWillBeErased(int objectID);
    void serialize(CSer& ar);
    void performObjectLoadingMapping(std::vector<int>* map);
    CSPage* getPage(int pageIndex) const;

    void setPageSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    void removePage(int pageIndex);
    int getPageCount() const;
    bool processCommand(int commandID,int viewIndex);

private:
    CSPage* _allPages[PAGES_COUNT];
    int _activePageIndex;

    int _initialValuesInitialized;
    int _initialActivePageIndex;
    int _pageSize[2];
    int _pagePosition[2];

    int mouseRelativePosition[2];

    int _caughtElements;


    int rightMouseCaughtBy;
    int leftMouseCaughtBy;
    bool rightMouseCaughtSoftDialog;
    bool leftMouseCaughtSoftDialog;

    int prepareForPopupMenu;
    int focusObject;

#ifdef SIM_WITH_GUI
public:
    void renderCurrentPage(bool hideWatermark);
    void setActivePage(int pageIndex);
    int getActivePageIndex() const;
    void addPageMenu(VMenu* menu);
    void clearAllLastMouseDownViewIndex();

    bool leftMouseButtonDown(int x,int y,int selectionStatus);
    bool getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const;
    void leftMouseButtonUp(int x,int y);
    void mouseMove(int x,int y,bool passiveAndFocused);
    int modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition);
    int getCursor(int x,int y) const;
    void mouseWheel(int deltaZ,int x,int y);
    bool middleMouseButtonDown(int x,int y);
    void middleMouseButtonUp(int x,int y);
    bool rightMouseButtonDown(int x,int y);
    void rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    bool leftMouseButtonDoubleClick(int x,int y,int selectionStatus);
    void looseFocus();
    int getFocusObject() const;
    void setFocusObject(int obj);
    void keyPress(int key,QWidget* mainWindow);
    int getCaughtElements() const;
    void clearCaughtElements(int keepMask);
#endif
};
