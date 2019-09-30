
#pragma once

#include <QWidget>

class C3DObject;

class CPageSelector  
{
public:
    CPageSelector();
    virtual ~CPageSelector();

    void newSceneProcedure();
    void setUpDefaultValues();
    void render();
    void setViewSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    int getPageIndexInViewSelection(int mousePos[2]);

    bool leftMouseButtonDown(int x,int y,int selectionStatus);
    void leftMouseButtonUp(int x,int y);
    void mouseMove(int x,int y,bool passiveAndFocused);
    int getCursor(int x,int y);
    int getPageIndex(int x,int y);
    bool rightMouseButtonDown(int x,int y);
    void rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    bool leftMouseButtonDoubleClick(int x,int y,int selectionStatus);
    void setViewSelectionInfo(int objType,int viewInd,int subViewInd);
    void keyPress(int key);
    int getCaughtElements();
    void clearCaughtElements(int keepMask);

private:
    int viewSize[2];
    int viewPosition[2];
    int mouseDownRelativePosition[2];
    int mouseRelativePosition[2];

    int _caughtElements;

    int viewSelectionSize[2];
    int tns[2];     // Thumnail size
    int objectType; // Type of objects to display
    int viewIndex;
    int subViewIndex;
};
