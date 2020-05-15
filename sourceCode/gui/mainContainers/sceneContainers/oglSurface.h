#pragma once

#include "viewSelector.h"
#include "pageSelector.h"
#include "buttonBlock.h"
#include "hierarchy.h"

class COglSurface
{
public:
    COglSurface();
    virtual ~COglSurface();

    void adjustBrowserAndHierarchySizesToDefault();
    void setSurfaceSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    bool leftMouseButtonDown(int x,int y,int selectionStatus);
    bool getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective);
    void leftMouseButtonUp(int x,int y);
    void mouseMove(int x,int y,bool passiveAndFocused);
    int modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition);
    int getCursor(int x,int y);
    bool _hierarchyResizingMousePosition(int x,int y);
    void mouseWheel(int deltaZ,int x,int y);
    bool rightMouseButtonDown(int x,int y);
    void rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    bool middleMouseButtonDown(int x,int y);
    void middleMouseButtonUp(int x,int y);
    bool leftMouseButtonDoubleClick(int x,int y,int selectionStatus);
    unsigned char* render(int currentCursor,int mouseButtonState,int mousePos[2],int* frameResol);
    void setViewSelectionActive(bool isActive);
    void setPageSelectionActive(bool isActive);
    void setHierarchySurfaceSizeAndPosition();
    void setViewSurfaceSizeAndPosition();
    void actualizeAllSurfacesSizeAndPosition();
    void setUpDefaultValues();
    void startViewSelection(int objectType,int subViewIndex);
    void keyPress(int key,QWidget* mainWindow);
    void setHierarchyEnabled(bool isEnabled);
    bool isHierarchyEnabled();
    int getHierarchyWidth();
    void setHierarchyWidth(int w);
    void setFocusObject(int obj);
    int getFocusObject();
    int getCaughtElements();
    void clearCaughtElements(int keepMask);
    bool isViewSelectionActive();
    bool isPageSelectionActive();
    bool isScenePageOrViewSelectionActive();

    CViewSelector* viewSelector;
    CPageSelector* pageSelector;

    CHierarchy* hierarchy;

    static bool _hierarchyEnabled;

private:

    bool viewSelectionActive;
    bool pageSelectionActive;
    bool _hierarchyClosingButtonDown;
    bool _hierarchyResizingAction;

    int _caughtElements; // This refers to the hierarchy closing button and the hierarchy/browser resizing bar only!

    static int _hierarchyWidth;
    static int _hierarchyMinWidth;

    bool _readyToRender;
    int surfaceSize[2];
    int surfacePosition[2];

    int mouseRelativePosition[2];
    int mousePreviousRelativePosition[2];

    int focusObject;
};
