#include "pathPointManipulation.h"
#include "app.h"

CPathPointManipulation_old::CPathPointManipulation_old()
{
    TRACE_INTERNAL;
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=true;
}

CPathPointManipulation_old::~CPathPointManipulation_old()
{
    TRACE_INTERNAL;
    // Following should already be erased:
}

void CPathPointManipulation_old::keyPress(int key)
{

}

std::vector<int>* CPathPointManipulation_old::getPointerToSelectedPathPointIndices_nonEditMode()
{
    return(&_selectedPathPointIndices_nonEditMode);
}

void CPathPointManipulation_old::clearPathPointIndices_nonEditMode()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
}

bool CPathPointManipulation_old::isPathPointIndexSelected_nonEditMode(int index,bool removeIfYes)
{
    TRACE_INTERNAL;
    for (int i=0;i<int(_selectedPathPointIndices_nonEditMode.size());i++)
    {
        if (index==_selectedPathPointIndices_nonEditMode[i])
        {
            if (!removeIfYes)
                return(true);
            _selectedPathPointIndices_nonEditMode.erase(_selectedPathPointIndices_nonEditMode.begin()+i);
            return(true);
        }
    }
    return(false);
}

void CPathPointManipulation_old::addPathPointToSelection_nonEditMode(int pathPointIdentifier)
{
    TRACE_INTERNAL;
    if (pathPointIdentifier>=NON_OBJECT_PICKING_ID_PATH_PTS_START)
    {
        if (_uniqueSelectedPathID_nonEditMode!=-1)
        { // Ok, we have one path selected
            if (!isPathPointIndexSelected_nonEditMode(pathPointIdentifier-NON_OBJECT_PICKING_ID_PATH_PTS_START,false))
                _selectedPathPointIndices_nonEditMode.push_back(pathPointIdentifier-NON_OBJECT_PICKING_ID_PATH_PTS_START);
        }
    }
}

void CPathPointManipulation_old::xorAddPathPointToSelection_nonEditMode(int pathPointIdentifier)
{
    TRACE_INTERNAL;
    if (pathPointIdentifier>=NON_OBJECT_PICKING_ID_PATH_PTS_START)
    {
        if (_uniqueSelectedPathID_nonEditMode!=-1)
        {
            if (!isPathPointIndexSelected_nonEditMode(pathPointIdentifier-NON_OBJECT_PICKING_ID_PATH_PTS_START,true))
                _selectedPathPointIndices_nonEditMode.push_back(pathPointIdentifier-NON_OBJECT_PICKING_ID_PATH_PTS_START);
        }
    }
}

void CPathPointManipulation_old::announceObjectSelectionChanged()
{
    TRACE_INTERNAL;
    if ((App::worldContainer==nullptr)||(App::currentWorld->sceneObjects==nullptr))
        return;
    if ( (App::currentWorld->sceneObjects->getSelectionCount()==1)&&_simulationStopped )
    {
        CPath_old* selPath=App::currentWorld->sceneObjects->getPathFromHandle(App::currentWorld->sceneObjects->getLastSelectionHandle());
        if (selPath!=nullptr)
        {
            if (selPath->getObjectHandle()!=_uniqueSelectedPathID_nonEditMode)
            {
                _selectedPathPointIndices_nonEditMode.clear();
                _uniqueSelectedPathID_nonEditMode=selPath->getObjectHandle();
            }
        }
        else
        {
            _selectedPathPointIndices_nonEditMode.clear();
            _uniqueSelectedPathID_nonEditMode=-1;
        }
    }
    else
    {
        _selectedPathPointIndices_nonEditMode.clear();
        _uniqueSelectedPathID_nonEditMode=-1;
    }
}

void CPathPointManipulation_old::announceSceneInstanceChanged()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
}

void CPathPointManipulation_old::simulationAboutToStart()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=false;
}

void CPathPointManipulation_old::simulationEnded()
{
    TRACE_INTERNAL;
    _simulationStopped=true;
}

int CPathPointManipulation_old::getUniqueSelectedPathId_nonEditMode()
{
    TRACE_INTERNAL;
    return(_uniqueSelectedPathID_nonEditMode);
}

int CPathPointManipulation_old::getSelectedPathPointIndicesSize_nonEditMode()
{
    TRACE_INTERNAL;
    return(int(_selectedPathPointIndices_nonEditMode.size()));
}
