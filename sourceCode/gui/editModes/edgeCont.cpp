#include "edgeCont.h"
#include <algorithm>

const int eCnt=3;
const int eCnt2=2*eCnt;

CEdgeCont::CEdgeCont()
{
}

CEdgeCont::~CEdgeCont()
{
    clearEdges();
}

int CEdgeCont::getEdgeIndex(int ind1,int ind2)
{
    int indS=std::min<int>(ind1,ind2);
    int indB=std::max<int>(ind1,ind2);
    if (indS>=int(_edges.size()/eCnt2))
        return(-1); // that edge doesn't exist
    // 1. Special case?
    int firstSlotVal=_edges[eCnt2*indS+2*0+0];
    if (firstSlotVal<-1)
    { // yes, this vertex is shared among more than 3 edges
        for (int i=0;i<int(_auxEdges[-firstSlotVal-2].size()/2);i++)
        {
            if (_auxEdges[-firstSlotVal-2][2*i+0]==indB)
                return(_auxEdges[-firstSlotVal-2][2*i+1]);
        }
        return(-1);
    }
    else
    { // regular case
        for (int i=0;i<eCnt;i++)
        {
            if (_edges[eCnt2*indS+2*i+0]==indB)
                return(_edges[eCnt2*indS+2*i+1]);
        }
        return(-1);
    }
}

void CEdgeCont::clearEdges()
{
    allEdges.clear();
    allEdgesVisibilityState.clear();
    _edges.clear();
    _auxEdges.clear();
}

int CEdgeCont::addEdge(int ind1,int ind2)
{
    int t=getEdgeIndex(ind1,ind2);
    if (t!=-1)
        return(t); // edge already exists!
    int indS=std::min<int>(ind1,ind2);
    int indB=std::max<int>(ind1,ind2);
    while (indS>=int(_edges.size()/eCnt2))
    {
        for (int i=0;i<eCnt;i++)
            _edges.push_back(-1);
    }
    // 1. Special case?
    int firstSlotVal=_edges[eCnt2*indS+2*0+0];
    if (firstSlotVal<-1)
    { // yes!
        _auxEdges[-firstSlotVal-2].push_back(indB);
        int retVal=(int)allEdges.size()/2;
        _auxEdges[-firstSlotVal-2].push_back(retVal);
        allEdges.push_back(indS);
        allEdges.push_back(indB);
        allEdgesVisibilityState.push_back(2); // default visibility (2010/08/03)
        return(retVal);
    }
    for (int i=0;i<eCnt;i++)
    {
        if (_edges[eCnt2*indS+2*i+0]==-1)
        {
            _edges[eCnt2*indS+2*i+0]=indB;
            int retVal=(int)allEdges.size()/2;
            _edges[eCnt2*indS+2*i+1]=retVal;
            allEdges.push_back(indS);
            allEdges.push_back(indB);
            allEdgesVisibilityState.push_back(2); // default visibility (2010/08/03)
            return(retVal);
        }
    }
    // We have to prepare the special case:
    std::vector<int> dummy;
    _auxEdges.push_back(dummy);
    t=(int)_auxEdges.size()-1;
    for (int i=0;i<eCnt2;i++)
        _auxEdges[t].push_back(_edges[eCnt2*indS+i]);
    _edges[eCnt2*indS+2*0+0]=-(int(_auxEdges.size())+1);
    _auxEdges[t].push_back(indB);
    int retVal=(int)allEdges.size()/2;
    _auxEdges[t].push_back(retVal);
    allEdges.push_back(indS);
    allEdges.push_back(indB);
    allEdgesVisibilityState.push_back(2); // default visibility (2010/08/03)
    return(retVal);
}
