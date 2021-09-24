
// This file requires some serious refactoring!

#pragma once

#include "ser.h"
#include "3Vector.h"
#ifdef SIM_WITH_GUI
#include "vMenubar.h"
#endif

class CSView  
{
public:
    CSView(int theLinkedObjectID);
    virtual ~CSView();
    void render(int mainWindowXPos,bool clipWithMainWindowXPos,bool drawText,bool passiveSubView);
    void initializeInitialValues(bool simulationAlreadyRunning);
    bool simulationEnded();
    void setDefaultValues();
    int getLinkedObjectID() const;
    void setLinkedObjectID(int theNewLinkedObjectID,bool noDefaultValues);
    void setAlternativeViewName(const char* name);
    std::string getAlternativeViewName() const;
    void setPerspectiveDisplay(bool perspective);
    bool getPerspectiveDisplay() const;

    void setShowEdges(bool edges);
    bool getShowEdges() const;
    void setThickEdges(bool thickEdges);
    bool getThickEdges() const;
    void setVisualizeOnlyInertias(bool inertiasOnly);
    bool getVisualizeOnlyInertias() const;
    int getCursor(int x,int y) const;


    void setRenderingMode(int mode);
    int getRenderingMode() const;
    void setRemoveFloatingViewAtSimulationEnd(bool remove);
    bool getRemoveFloatingViewAtSimulationEnd() const;
    void setDoNotSaveFloatingView(bool doNotSave);
    bool getDoNotSaveFloatingView() const;


    void setXYGraphAutoModeDuringSimulation(bool autoMode);
    bool getXYGraphAutoModeDuringSimulation() const;
    void setTimeGraphXAutoModeDuringSimulation(bool autoMode);
    bool getTimeGraphXAutoModeDuringSimulation() const;
    void setTimeGraphYAutoModeDuringSimulation(bool autoMode);
    bool getTimeGraphYAutoModeDuringSimulation() const;
    void setXYGraphIsOneOneProportional(bool autoMode);
    bool getXYGraphIsOneOneProportional() const;
    void setTimeGraph(bool timeGraph);
    bool getTimeGraph() const;
    void setCanSwapViewWithMainView(bool canDoIt);
    bool getCanSwapViewWithMainView() const;

    void setCanBeClosed(bool canDoIt);
    bool getCanBeClosed() const;
    void setCanBeShifted(bool canDoIt);
    bool getCanBeShifted() const;
    void setCanBeResized(bool canDoIt);
    bool getCanBeResized() const;

    void setFitViewToScene(bool doIt);
    bool getFitViewToScene() const;
    void setFitViewToSelection(bool doIt);
    bool getFitViewToSelection() const;

    void setGraphPosition(float x,float y);
    void getGraphPosition(float position[2]) const;
    void setGraphSize(float x,float y);
    void getGraphSize(float size[2]) const;
    void serialize(CSer& ar);
    bool announceObjectWillBeErased(int objectID);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void setViewSizeAndPosition(int sizeX,int sizeY,int posX,int posY);
    void getViewSize(int size[2]) const;
    void getViewPosition(int pos[2]) const;
    int getSelectionStatus() const;
    bool processCommand(int commandID,int subViewIndex);
    void setCenterPosition(float pos[3]);
    void getCenterPosition(float pos[3]) const;
    void setViewIndex(size_t ind);
    size_t getViewIndex() const;
    void setTrackedGraphCurveIndex(int ind);
    int getTrackedGraphCurveIndex() const;

    int getUniqueID() const;

    CSView* copyYourself(); // special! Only used when copy and pasting viewable objects in the copy buffer that are associated with a floating view!
    // Following two lines are only used when copy and pasting viewable objects are associated with a floating view!
    float _relativeViewPosition[2];
    float _relativeViewPositionOffset;
    float _relativeViewSize[2];


private:
    // Variables which need to be serialized:
    int linkedObjectID;
    bool perspectiveDisplay;
    bool _showEdges;
    bool _thickEdges;
    bool _visualizeOnlyInertias;
    bool _removeFloatingViewAtSimulationEnd;
    int _renderingMode;
    bool _xyGraphInAutoModeDuringSimulation;
    bool _timeGraphXInAutoModeDuringSimulation;
    bool _timeGraphYInAutoModeDuringSimulation;
    bool _xyGraphIsOneOneProportional;
    bool graphIsTimeGraph;
    bool _canSwapViewWithMainView;
    bool _canBeClosed;
    bool _canBeShifted;
    bool _canBeResized;
    float graphPosition[2];
    float graphSize[2];
    bool _fitSceneToView;
    bool _fitSelectionToView;
    
    // Variables which don't need to be serialized:
    int _viewPosition[2];
    int _viewSize[2];
    std::string _alternativeViewName;

    int _caughtElements;
    bool _doNotSaveFloatingView;

    int mouseDownRelativePosition[2];
    int _rightMouseButtonDownCoord[2];

    int rightMouseDownRelativePosition[2];
    bool dontActivatePopup;
    int mouseRelativePosition[2];
    int mousePreviousRelativePosition[2];
    int selectionStatus;
    bool mouseIsDown;
    bool rightButtonIsCaught;
    bool mouseJustWentDownFlag;
    int mouseDownTimings;
    bool mouseJustWentDownWasProcessed;
    bool mouseJustWentUpFlag;
    bool _mouseMovedWhileDownFlag;

    float centerPosition[3];
    float mousePositionDepth;

    bool subviewIsPassive;
    int _trackedCurveIndex;
    int _uniqueID;

    bool _initialValuesInitialized;
    bool _initialPerspectiveDisplay;
    bool _initialShowEdges;
    bool _initialThickEdges;
    bool _initialVisualizeOnlyInertias;
    int _initialRenderingMode;
    bool _initialGraphIsTimeGraph;

    size_t _viewIndex; // set before each rendering

#ifdef SIM_WITH_GUI
public:
    void addMenu(VMenu* menu);
    void getMouseDownRelativePosition(int p[2]) const;
    void getMouseRelativePosition(int p[2]) const;
    void getPreviousMouseRelativePosition(int p[2]) const;
    bool isMouseDown() const;
    bool didMouseJustGoDown() const;
    bool wasMouseJustWentDownEventProcessed() const;
    bool didMouseJustGoUp() const;
    bool didMouseMoveWhileDown() const;
    bool leftMouseButtonDown(int x,int y,int selStatus);
    bool getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const;
    void leftMouseButtonUp(int x,int y);
    void mouseMove(int x,int y,bool passiveAndFocused);
    int modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition);
    bool mouseWheel(int deltaZ,int x,int y);
    bool middleMouseButtonDown(int x,int y,bool _subViewIsPassive);
    void middleMouseButtonUp(int x,int y);
    bool rightMouseButtonDown(int x,int y,bool _subViewIsPassive);
    bool rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow,int subViewIndex);
    bool leftMouseButtonDoubleClick(int x,int y,int selStatus);
    void setMousePositionDepth(float depth);
    float getMousePositionDepth() const;
    void clearMouseJustWentDownAndUpFlag();
    void handleCameraOrGraphMotion();
    int getCaughtElements() const;
    void clearCaughtElements(int keepMask);

private:
    void cameraAndObjectMotion();
    void graphMotion();
    void _handleClickRayIntersection(int x,int y,bool mouseDown);
#endif
};
