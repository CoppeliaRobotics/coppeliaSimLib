
#pragma once

#include "sView.h"
#include "shape.h"

class CAddOperations  
{
public:
    CAddOperations();
    virtual ~CAddOperations();

    static bool processCommand(int commandID,CSView* subView);
    static CShape* addPrimitiveShape(int type,const C3Vector& sizes,const int subdiv[3],int faces,int sides,int discSubdiv,bool smooth,int openEnds,bool dynamic,bool pure,bool cone,float density,bool negVolume,float negVolumeScaling);

#ifdef SIM_WITH_GUI
    static void addMenu(VMenu* menu,CSView* subView,bool onlyCamera);
    static CShape* addPrimitive_withDialog(int command,const C3Vector* optSizes);
#endif
};
