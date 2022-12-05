
#include "app.h"
#include "rendering.h"

void displayBackground(const int* viewSize,bool fogEnabled,const float* fogBackgroundColor,const float* backGroundColorDown,const float* backGroundColor);
void enableAmbientLight(bool on,const float* ambientLightColor);
void activateFog(const float* fogBackgroundColor,int fogType,double dd,double farClipp,double fogStart,double fogEnd,double fogDensity);
void enableFog(bool on);
