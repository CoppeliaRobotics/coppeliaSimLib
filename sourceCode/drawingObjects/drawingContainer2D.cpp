
#include "vrepMainHeader.h"
#include "drawingContainer2D.h"

CDrawingContainer2D::CDrawingContainer2D()
{
}

CDrawingContainer2D::~CDrawingContainer2D()
{
    removeAllObjects();
}

void CDrawingContainer2D::addObject(CDrawingObject2D* it)
{
    _allObjects.push_back(it);
}

void CDrawingContainer2D::removeAllObjects()
{
    for (int i=0;i<int(_allObjects.size());i++)
        delete _allObjects[i];
    _allObjects.clear();
}

void CDrawingContainer2D::render(int minViewPos[2],int maxViewPos[2])
{
    for (size_t i=0;i<_allObjects.size();i++)
        _allObjects[i]->draw(minViewPos,maxViewPos);
}
