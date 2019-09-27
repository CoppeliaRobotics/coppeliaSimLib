
#include "pathPlanning.h"
#include "pathPlanningInterface.h"
#include "v_rep_internal.h"

CPathPlanning::CPathPlanning()
{
}

CPathPlanning::~CPathPlanning()
{
}

int CPathPlanning::searchPath(int maxTimePerPass)
{ // maxTimePerPass is in miliseconds
    return(false);
}
bool CPathPlanning::setPartialPath()
{
    return(false);
}
int CPathPlanning::smoothFoundPath(int steps,int maxTimePerPass)
{ // step specifies the smoothing factor
    return(0);
}

void CPathPlanning::getPathData(std::vector<float>& data)
{
}

void CPathPlanning::getSearchTreeData(std::vector<float>& data,bool fromStart)
{
}

bool CPathPlanning::doCollide(float* dist)
{ // dist can be nullptr
    return(false);
}

