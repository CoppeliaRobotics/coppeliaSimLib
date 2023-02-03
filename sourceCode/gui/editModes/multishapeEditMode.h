
#pragma once

#include <shape.h>

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
    CMesh* getCurrentMultishapeGeometricComponent();
    CMesh* getMultishapeGeometricComponentAtIndex(int index);
    void displayAllGeometricComponents(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling);

    bool processCommand(int commandID);

private:
    CShape* _shape;
    std::vector<CMesh*> _multishapeGeometricComponents;
    int _multishapeGeometricComponentIndex;
};
