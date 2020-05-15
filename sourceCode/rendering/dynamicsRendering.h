
#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _drawPoints(int displayAttrib,const C4X4Matrix& cameraRTM,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor);
void _drawRoughSphere(int displayAttrib,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor);
void _drawSphere(int displayAttrib,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor);
#endif

void displayParticles(void** particlesPointer,int particlesCount,int displayAttrib,const C4X4Matrix& cameraCTM,const float* cols,int objectType);
void displayContactPoints(int displayAttrib,const CColorObject& contactPointColor,const float* pts,int cnt);
