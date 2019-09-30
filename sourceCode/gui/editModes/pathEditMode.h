
#pragma once

#include "path.h"
#include "objCont.h"
#include "vMenubar.h"

class CPathEditMode
{
public:
    CPathEditMode(CPath* path,CObjCont* objCont);
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

    void addMenu(VMenu* menu,C3DObject* viewableObject);
    bool processCommand(int commandID,C3DObject* viewableObject);

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
    CObjCont* _objCont;
    std::vector<int> editModeBuffer;
};
