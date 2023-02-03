
#pragma once

#include <simMath/3Vector.h>

class CEdgeElement  
{
public:
    CEdgeElement(int v0,int v1,int tri,CEdgeElement* nextEl,int position,double vertices[]);
    virtual ~CEdgeElement();
    int vertex0;
    int vertex1;
    int triangle;
    int pos;
    C3Vector n;
    CEdgeElement* next;
};
