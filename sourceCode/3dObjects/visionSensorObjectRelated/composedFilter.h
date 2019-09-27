
#pragma once

#include "vrepMainHeader.h"
#include "simpleFilter.h"
#include "drawingContainer2D.h"
#include "ser.h"

class CComposedFilter
{
public:

    CComposedFilter();
    virtual ~CComposedFilter();

    void displayOverlay(int c0[2],int c1[2]);
    CComposedFilter* copyYourself();
    void serialize(CSer& ar);
    int getSimpleFilterCount();
    void insertSimpleFilter(CSimpleFilter* it);
    CSimpleFilter* getSimpleFilter(int index);
    CSimpleFilter* getSimpleFilterFromUniqueID(int uniqueID);
    void removeAllSimpleFilters();
    void removeSimpleFilter(int index);
    bool moveSimpleFilter(int index,bool up);
    bool processAndTrigger(CVisionSensor* sensor,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* outputDepthBuffer,std::vector<std::vector<float> >& returnData);
    bool includesDepthBufferModification();
    void initializeInitialValues(bool simulationIsRunning);
    void simulationEnded();
    void removeBuffers();

    CDrawingContainer2D drawingContainer;
protected:

    // Variables which need to be serialized & copied 
    std::vector<CSimpleFilter*> _allSimpleFilters;
    float* buffer1;
    float* buffer2;
    float* workImage;
};
