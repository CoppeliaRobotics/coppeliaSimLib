
#pragma once

#include "vrepMainHeader.h"

class CEdgeCont 
{
public:
    CEdgeCont();
    virtual ~CEdgeCont();
    
    int getEdgeIndex(int ind1,int ind2);
    void clearEdges();
    int addEdge(int ind1,int ind2);

    std::vector<int> allEdges;
    std::vector<unsigned char> allEdgesVisibilityState; // 0=invisible, 1=visible, 2=default (2010/08/03)

protected:

    std::vector<int> _edges;
    std::vector<std::vector<int> > _auxEdges;
};
