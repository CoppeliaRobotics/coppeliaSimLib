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

#include <app.h>
#include <rendering.h>

#ifdef SIM_WITH_OPENGL
void _drawPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3]);
void _drawTrianglePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawQuadPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawDiscPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawCubePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawSpherePoints(CDrawingObject* drawingObject,int displayAttrib);
void _drawLines(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3]);
void _drawLineStrip(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3]);
void _drawTriangles(CDrawingObject* drawingObject,int displayAttrib);
#endif

void displayDrawingObject(CDrawingObject* drawingObject,C7Vector& tr,bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
