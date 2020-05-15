#pragma once

#include "path.h"
#include "sceneObjectContainer.h"
#include "vMenubar.h"

class CPathEditMode
{
public:
    CPathEditMode(CPath* path,CSceneObjectContainer* objCont);
    virtual ~CPathEditMode();

    void endEditMode(bool cancelChanges);
    CPathCont* getEditModePathContainer();
    CPath* getEditModePath();

    int getEditModeBufferSize();
    int getLastEditModeBufferValue();
    int getEditModeBufferValue(int index);
    std::vector<int>* getEditModeBuffer();
    void deselectEditModeBuffer();
    void removeItemFromEditModeBuffer(int item);
    bool isEditModeItemAValidItem(int item);
    void xorAddItemToEditModeBuffer(int item);
    void addItemToEditModeBuffer(int item);
    bool alreadyInEditModeBuffer(int item);
    int getBezierPathPointCount();
    CSimplePathPoint* getSimplePathPoint(int editModeBufferIndex);

    void makeDummies();

    void addMenu(VMenu* menu,CSceneObject* viewableObject);
    bool processCommand(int commandID,CSceneObject* viewableObject);

private:
    void _copySelection(std::vector<int>* selection);
    void _paste(int insertPosition);
    void _insertNewPoint(int insertPosition);
    void _cutSelection(std::vector<int>* selection);
    void _deleteSelection(std::vector<int>* selection);
    void _keepXAxisAndAlignZAxis(std::vector<int>* selection);
    void _generatePath();

    std::vector<CSimplePathPoint*> editBufferPathPointsCopy;
    CPath* _path;
    CPathCont* _editionPathCont;
    CSceneObjectContainer* _objCont;
    std::vector<int> editModeBuffer;
};
