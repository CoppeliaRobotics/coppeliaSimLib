#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _displayForceSensor(CForceSensor* forceSensor,int displayAttrib,bool partOne,float sizeParam);
#endif

void displayForceSensor(CForceSensor* forceSensor,CViewableBase* renderingObject,int displayAttrib);
