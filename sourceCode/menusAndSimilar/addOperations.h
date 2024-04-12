
#pragma once

#include <sView.h>
#include <shape.h>

class CAddOperations
{
  public:
    CAddOperations();
    virtual ~CAddOperations();

    static CShape *addPrimitiveShape(int type, const C3Vector &psizes, int options = 0, const int subdiv[3] = nullptr,
                                     int faceSubdiv = 0, int sides = 0, int discSubdiv = 0, bool dynamic = false,
                                     int pure = 1, double density = 500.0);

#ifdef SIM_WITH_GUI
    static bool processCommand(int commandID, CSView *subView);
    static void addMenu(VMenu *menu, CSView *subView, bool onlyCamera);
    static CShape *addPrimitive_withDialog(int command, const C3Vector *optSizes);
#endif
};
