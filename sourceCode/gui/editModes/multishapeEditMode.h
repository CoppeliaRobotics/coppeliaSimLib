
#pragma once

#include "vrepMainHeader.h"
#include "shape.h"

class CMultishapeEditMode
{
public:
    CMultishapeEditMode(CShape* shape);
    virtual ~CMultishapeEditMode();

    CShape* getEditModeMultishape();

    int getMultishapeGeometricComponentsSize();
    int getMultishapeGeometricComponentIndex();
    void setMultishapeGeometricComponentIndex(int index);
    bool isCurrentMultishapeGeometricComponentValid();
    CGeometric* getCurrentMultishapeGeometricComponent();
    CGeometric* getMultishapeGeometricComponentAtIndex(int index);
    void displayAllGeometricComponents(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling);

    bool processCommand(int commandID);

private:
    CShape* _shape;
    std::vector<CGeometric*> _multishapeGeometricComponents;
    int _multishapeGeometricComponentIndex;
};
