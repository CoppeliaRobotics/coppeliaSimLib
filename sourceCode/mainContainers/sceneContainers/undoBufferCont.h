
#pragma once

#include "mainCont.h"
#include "undoBuffer.h"
#include "undoBufferArrays.h"

class CUndoBufferCont : public CMainCont
{
public:
    CUndoBufferCont();
    virtual ~CUndoBufferCont();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void emptySceneProcedure();
    void memorizeStateIfNeeded();

    void emptyRedoBuffer(); 

    bool announceChange();          // Will directly save the changes
    void announceChangeStart();     // Will only save changes if announceChangeEnd is called after
    void announceChangeEnd();       // Will only save changes if announceChangeStart was called previously
    void announceChangeGradual();   // Will save grouped changes (with a delay)

    bool memorizeState();
    void undo();
    void redo();
    bool canUndo();
    bool canRedo();
    bool isSceneSaveMaybeNeededFlagSet();
    void clearSceneSaveMaybeNeededFlag();

    bool isUndoSavingOrRestoringUnderWay();
    int getNextBufferId();

    CUndoBufferArrays undoBufferArrays;
private:

    CUndoBufferCameras* _getFullBuffer(int index,std::vector<char>& fullBuff);
    int _getUsedMemory();
    bool _isGoodToMemorizeUndoOrRedo();
    void _commonInit();

    void _rememberSelectionState();
    void _restoreSelectionState();

    int _currentStateIndex;
    std::vector<CUndoBuffer*> _buffers;
    bool _announceChangeStartCalled;
    int _announceChangeGradualCalledTime;
    bool _sceneSaveMightBeNeeded;
    bool _undoPointSavingOrRestoringUnderWay;
    bool _inUndoRoutineNow;
    int _nextBufferId;

    std::vector<std::string> _selectionState;
};
