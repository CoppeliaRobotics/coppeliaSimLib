
#include "pathPointManipulation.h"
#include "funcDebug.h"
#include "app.h"

CPathPointManipulation::CPathPointManipulation()
{
    FUNCTION_DEBUG;
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=true;
}

CPathPointManipulation::~CPathPointManipulation()
{
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
    _selectedPathPointIndices_nonEditMode.clear();
}

bool CPathPointManipulation::isPathPointIndexSelected_nonEditMode(int index,bool removeIfYes)
{
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
    if ((App::ct==nullptr)||(App::ct->objCont==nullptr))
        return;
    if ( (App::ct->objCont->getSelSize()==1)&&_simulationStopped )
    {
        CPath* selPath=App::ct->objCont->getPath(App::ct->objCont->getLastSelectionID());
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
    FUNCTION_DEBUG;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
}

void CPathPointManipulation::simulationAboutToStart()
{
    FUNCTION_DEBUG;
    _selectedPathPointIndices_nonEditMode.clear();
    _uniqueSelectedPathID_nonEditMode=-1;
    _simulationStopped=false;
}

void CPathPointManipulation::simulationEnded()
{
    FUNCTION_DEBUG;
    _simulationStopped=true;
}

int CPathPointManipulation::getUniqueSelectedPathId_nonEditMode()
{
    FUNCTION_DEBUG;
    return(_uniqueSelectedPathID_nonEditMode);
}

int CPathPointManipulation::getSelectedPathPointIndicesSize_nonEditMode()
{
    FUNCTION_DEBUG;
    return(int(_selectedPathPointIndices_nonEditMode.size()));
}

bool CPathPointManipulation::deleteSelectedPathPoints_nonEditMode()
{
    FUNCTION_DEBUG;
    SSimulationThreadCommand cmd;
    cmd.cmdId=DELETE_SELECTED_PATH_POINTS_NON_EDIT_FROMUI_TOSIM_CMD;
    cmd.intParams.push_back(_uniqueSelectedPathID_nonEditMode);
    cmd.intVectorParams.push_back(_selectedPathPointIndices_nonEditMode);
    App::simThread->appendSimulationThreadCommand(cmd);
    _selectedPathPointIndices_nonEditMode.clear();
    return(true);
}
