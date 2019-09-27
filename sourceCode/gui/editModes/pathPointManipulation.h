
#pragma once

#include "vrepMainHeader.h"

class CPathPointManipulation
{
public:
    CPathPointManipulation();
    virtual ~CPathPointManipulation();

    void announceObjectSelectionChanged();
    void announceSceneInstanceChanged();
    void simulationAboutToStart();
    void simulationEnded();

    void keyPress(int key);

    void addPathPointToSelection_nonEditMode(int pathPointIdentifier);
    void xorAddPathPointToSelection_nonEditMode(int pathPointIdentifier);

    std::vector<int>* getPointerToSelectedPathPointIndices_nonEditMode();
    void clearPathPointIndices_nonEditMode();
    bool isPathPointIndexSelected_nonEditMode(int index,bool removeIfYes);
    int getUniqueSelectedPathId_nonEditMode();
    int getSelectedPathPointIndicesSize_nonEditMode();
    bool deleteSelectedPathPoints_nonEditMode();

private:
    bool _simulationStopped;
    std::vector<int> _selectedPathPointIndices_nonEditMode;
    int _uniqueSelectedPathID_nonEditMode;
};
