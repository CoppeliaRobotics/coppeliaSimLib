
#pragma once

#include "vrepMainHeader.h"
#include "drawingObject2D.h"

class CDrawingContainer2D
{
public:
    CDrawingContainer2D();
    virtual ~CDrawingContainer2D();

    void render(int minViewPos[2],int maxViewPos[2]);
    void addObject(CDrawingObject2D* it);
    void removeAllObjects();
private:
    std::vector<CDrawingObject2D*> _allObjects;
};
