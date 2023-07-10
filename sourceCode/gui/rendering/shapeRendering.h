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

void _displayTriangles(CMesh* geometric,int geomModifCounter,CTextureProperty* tp);
void _displayInertia(const C7Vector& tr,const C3Vector& pmi,double comFrameSize);
void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib);
void displayGeometric(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
void displayGeometric_colorCoded(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int objectId,int displayAttrib);
void displayGeometricGhost(const C7Vector& cumulIFrameTr,CMesh* geometric,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors);
