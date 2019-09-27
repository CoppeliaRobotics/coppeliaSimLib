
#pragma once

#include "vrepMainHeader.h"
#include <QWidget>

class C3DObject;

class CSceneSelector  
{
public:
    CSceneSelector();
    virtual ~CSceneSelector();

    void newSceneProcedure();
    void setUpDefaultValues();
    void render();
    void setViewSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    int getSceneIndexInViewSelection(int mousePos[2]);

    bool leftMouseButtonDown(int x,int y,int selectionStatus);
    void leftMouseButtonUp(int x,int y);
    void mouseMove(int x,int y,bool passiveAndFocused);
    int getCursor(int x,int y);
    int getSceneIndex(int x,int y);
    bool rightMouseButtonDown(int x,int y);
    void rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    bool leftMouseButtonDoubleClick(int x,int y,int selectionStatus);
    void keyPress(int key);
    int getCaughtElements();
    void clearCaughtElements(int keepMask);
    void markAsFirstRender();

private:
    int viewSize[2];
    int viewPosition[2];
    int mouseDownRelativePosition[2];
    int mouseRelativePosition[2];

    int _caughtElements;

    int viewSelectionSize[2];
    int tns[2];     // Thumnail size
    int viewIndex;
    int subViewIndex;
    bool _firstRender;
};
