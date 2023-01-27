/*
Source code based on the V-REP library source code from Coppelia
Robotics AG on September 2019

Copyright (C) 2006-2023 Coppelia Robotics AG
Copyright (C) 2019 Robot Nordic ApS

All rights reserved.

GNU GPL license:
================
The code in this file is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

See the GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*/

#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _drawPoints(int displayAttrib,const C4X4Matrix& cameraRTM,const C3Vector& _currentPosition,double _size,int _objectType,const float* _additionalColor);
void _drawRoughSphere(int displayAttrib,const C3Vector& _currentPosition,double _size,int _objectType,const float* _additionalColor);
void _drawSphere(int displayAttrib,const C3Vector& _currentPosition,double _size,int _objectType,const float* _additionalColor);
#endif

void displayParticles(void** particlesPointer,int particlesCount,int displayAttrib,const C4X4Matrix& cameraCTM,const float* cols,int objectType);
void displayContactPoints(int displayAttrib,const CColorObject& contactPointColor,const double* pts,int cnt);
