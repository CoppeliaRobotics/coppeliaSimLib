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
void _displayInertia(CMeshWrapper* geomWrap,double bboxDiagonal,const double normalVectorForPointsAndLines[3]);
void _displayTriangles(CMesh* geometric,int geomModifCounter,CTextureProperty* tp);
#endif

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib);
void displayGeometric(CMesh* geometric,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
void displayGeometric_colorCoded(CMesh* geometric,CShape* geomData,int objectId,int displayAttrib);
void displayGeometricGhost(CMesh* geometric,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors);
