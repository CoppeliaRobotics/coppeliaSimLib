#pragma once

#include "shapeEditMode.h"
#include "multishapeEditMode.h"
#include "pathEditMode.h"
#include "pathPointManipulation.h"

class CEditModeContainer
{
public:
    CEditModeContainer();
    virtual ~CEditModeContainer();

    bool enterEditMode(int objID,int modeType);
    void endEditMode(bool cancelChanges);
    int getEditModeObjectID();
    CShape* getEditModeShape();
    CPath* getEditModePath();
    CSceneObject* getEditModeObject();
    CPathCont* getEditModePathContainer();
    int getEditModeType();
    void swapShapeEditModeType(int theType);

    int getEditModeBufferSize();
    int getLastEditModeBufferValue();
    int getEditModeBufferValue(int index);
    std::vector<int>* getEditModeBuffer();
    void deselectEditModeBuffer();
    void removeItemFromEditModeBuffer(int item);
    void xorAddItemToEditModeBuffer(int item,bool disableEdgeFollowing);
    void addItemToEditModeBuffer(int item,bool disableEdgeFollowing);

    CShapeEditMode* getShapeEditMode();
    CMultishapeEditMode* getMultishapeEditMode();
    CPathEditMode* getPathEditMode();

    void addMenu(VMenu* menu,CSceneObject* viewableObject);
    bool processCommand(int commandID,CSceneObject* viewableObject);
    bool keyPress(int key);


    void announceObjectSelectionChanged();
    void announceSceneInstanceChanged();
    void simulationAboutToStart();
    void simulationEnded();

    CPathPointManipulation* pathPointManipulation;

private:
    bool _processShapeEditModeCommand(int commandID);
    bool _processMultishapeEditModeCommand(int commandID);
    bool _processPathEditModeCommand(int commandID,CSceneObject* viewableObject);

    CShapeEditMode* _shapeEditMode;
    CMultishapeEditMode* _multishapeEditMode;
    CPathEditMode* _pathEditMode;

    int _editModeObject;
    bool _editMode_hierarchyWasEnabledBeforeEditMode;
    bool _modelBrowserVisibilityBeforeEditMode;
    bool _simulationStopped;
};
