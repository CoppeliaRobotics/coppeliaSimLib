#include "pathPointManipulation.h"
#include "app.h"

CPathPointManipulation::CPathPointManipulation()
{
    TRACE_INTERNAL;
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=true;
}

CPathPointManipulation::~CPathPointManipulation()
{
    TRACE_INTERNAL;
    // Following should already be erased:
}

void CPathPointManipulation::keyPress(int key)
{

}

std::vector<int>* CPathPointManipulation::getPointerToSelectedPathPointIndices_nonEditMode()
{
    return(&_selectedPathPointIndices_nonEditMode);
}

void CPathPointManipulation::clearPathPointIndices_nonEditMode()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
}

bool CPathPointManipulation::isPathPointIndexSelected_nonEditMode(int index,bool removeIfYes)
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

void CPathPointManipulation::addPathPointToSelection_nonEditMode(int pathPointIdentifier)
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

void CPathPointManipulation::xorAddPathPointToSelection_nonEditMode(int pathPointIdentifier)
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

void CPathPointManipulation::announceObjectSelectionChanged()
{
    TRACE_INTERNAL;
    if ((App::worldContainer==nullptr)||(App::currentWorld->sceneObjects==nullptr))
        return;
    if ( (App::currentWorld->sceneObjects->getSelectionCount()==1)&&_simulationStopped )
    {
        CPath* selPath=App::currentWorld->sceneObjects->getPathFromHandle(App::currentWorld->sceneObjects->getLastSelectionHandle());
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

void CPathPointManipulation::announceSceneInstanceChanged()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
}

void CPathPointManipulation::simulationAboutToStart()
{
    TRACE_INTERNAL;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=false;
}

void CPathPointManipulation::simulationEnded()
{
    TRACE_INTERNAL;
    _simulationStopped=true;
}

int CPathPointManipulation::getUniqueSelectedPathId_nonEditMode()
{
    TRACE_INTERNAL;
    return(_uniqueSelectedPathID_nonEditMode);
}

int CPathPointManipulation::getSelectedPathPointIndicesSize_nonEditMode()
{
    TRACE_INTERNAL;
    return(int(_selectedPathPointIndices_nonEditMode.size()));
}

bool CPathPointManipulation::deleteSelectedPathPoints_nonEditMode()
{
    TRACE_INTERNAL;
    SSimulationThreadCommand cmd;
    cmd.cmdId=DELETE_SELECTED_PATH_POINTS_NON_EDIT_FROMUI_TOSIM_CMD;
    cmd.intParams.push_back(_uniqueSelectedPathID_nonEditMode);
    cmd.intVectorParams.push_back(_selectedPathPointIndices_nonEditMode);
    App::simThread->appendSimulationThreadCommand(cmd);
    _selectedPathPointIndices_nonEditMode.clear();
    return(true);
}
