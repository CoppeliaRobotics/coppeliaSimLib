
// This file requires some serious refactoring!!

#pragma once

#include "vrepMainHeader.h"
#include "hierarchyElement.h"
#include "vMenubar.h"

class CHierarchy  
{
public:
    CHierarchy();
    virtual ~CHierarchy();

    void newSceneProcedure();
    void setRenderingSizeAndPosition(int xSize,int ySize,int xPos,int yPos);
    bool render();
    void removeAll();
    void setUpDefaultValues();
    void rebuildHierarchy();
    bool leftMouseDown(int x,int y,int selectionStatus);
    void leftMouseUp(int x,int y);
    bool rightMouseDown(int x,int y);
    void rightMouseUp(int x,int y,int absX,int absY,QWidget* mainWindow);
    void mouseMove(int x,int y,bool passiveAndFocused);
    bool mouseWheel(int deltaZ,int x,int y);
    bool leftMouseDblClick(int x,int y,int selectionStatus);
    void validateViewPosition();

    void setRebuildHierarchyFlag();
    void setRefreshViewFlag();
    void setResetViewFlag();

    void addMenu(VMenu* menu);
    bool processCommand(int commandID);
    void looseFocus();
    void keyPress(int key);
    int getSliderPositions(int vSliderTopLeft[2],int vSliderBottomRight[2],int hSliderTopLeft[2],int hSliderBottomRight[2],float prop[2]);

    std::string getEditedLabel();
    void setEditionLabel(std::string txt);
    int getInflateActionObjectID(int mousePositionX,int mousePositionY);
    int getScriptActionObjectID(int mousePositionX,int mousePositionY);
    int getScriptParameterActionObjectID(int mousePositionX,int mousePositionY);
    int getTextActionObjectID(int mousePositionX,int mousePositionY);
    int getLineObjectID(int mousePositionY,int textPosStart[2]);
    int getSimulationActionObjectID(int mousePositionX,int mousePositionY);

    int getActionObjectID(int mousePositionY,int tolerance);
    int getActionObjectID_icon(int mousePositionX,int mousePositionY,bool ignoreXPositionAndCheckLineInstead=false);
    int getActionModelID_icon(int mousePositionX,int mousePositionY);
    void getActionObjectIDs(int mouseDownPositionY,int mouseUpPositionY,int tolerance,std::vector<int>* toBeSelected);
    void drawEditionLabel(int textPosX,int textPosY);

    int getCaughtElements();
    void clearCaughtElements(int keepMask);
    void _drawLinesLinkingDummies(int maxRenderedPos[2]);

    std::vector<int> inflateIconPosition;
    std::vector<int> objectPosition;
    std::vector<int> objectIconPosition;
    std::vector<int> modelIconPosition;
    std::vector<int> scriptIconPosition;
    std::vector<int> scriptParametersIconPosition;
    std::vector<int> textPosition;
    std::vector<int> simulationIconPosition;
    std::vector<int> lineLastPosition;

    std::string editionText;
    int editionTextEditPos;

private:
    int _caughtElements;
    int renderingSize[2];
    int renderingPosition[2];
    std::vector<CHierarchyElement*> rootElements;
    int mouseDownRelativePosition[2];
    int mouseRelativePosition[2];
    int previousMouseRelativePosition[2];
    int viewPosition[2];
    bool shiftingAllowed;
    int minRenderedPosition[2];
    int maxRenderedPosition[2];
    bool shiftSelectionStarted;
    int refreshViewFlag;
    bool rebuildHierarchyFlag;
    bool resetViewFlag;
    int labelEditObjectID;
    int verticalScrollbarWidth;
    int horizontalScrollbarHeight;
    int sliderMoveMode;
    int viewPosWhenMouseOnSliderDown[2];
    int _mouseDownDragObjectID;
    int _mouseDownDragOffset[2];
    int objectIDWhereTheMouseCurrentlyIs_minus9999ForNone;
    int _worldSelectID_down;
    int _worldSelectID_moving;
};
