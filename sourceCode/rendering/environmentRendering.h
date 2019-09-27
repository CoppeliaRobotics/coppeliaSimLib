
#include "app.h"
#include "rendering.h"

void displayBackground(const int* viewSize,bool fogEnabled,const float* fogBackgroundColor,const float* backGroundColorDown,const float* backGroundColor);
void enableAmbientLight(bool on,const float* ambientLightColor);
void activateFog(const float* fogBackgroundColor,int fogType,float dd,float farClipp,float fogStart,float fogEnd,float fogDensity);
void enableFog(bool on);
