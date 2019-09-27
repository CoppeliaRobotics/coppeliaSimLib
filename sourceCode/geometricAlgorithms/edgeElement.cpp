
#include "vrepMainHeader.h"
#include "edgeElement.h"

CEdgeElement::CEdgeElement(int v0,int v1,int tri,CEdgeElement* nextEl,int position,float vertices[])
{
    vertex0=v0;
    vertex1=v1;
    triangle=tri;
    next=nextEl;
    pos=position;
    C3Vector p0(vertices[3*vertex0+0],vertices[3*vertex0+1],vertices[3*vertex0+2]);
    C3Vector p1(vertices[3*vertex1+0],vertices[3*vertex1+1],vertices[3*vertex1+2]);
    n=(p1-p0).getNormalized();
}

CEdgeElement::~CEdgeElement()
{
    delete next;
    next=nullptr;
}
