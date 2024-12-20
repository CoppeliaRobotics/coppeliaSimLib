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
#pragma once
#include <app.h>
#include <oGL.h>

void _activateNonAmbientLights(int lightHandle, CViewableBase* viewable);
void _prepareAuxClippingPlanes();
void _enableAuxClippingPlanes(int objID);
void _prepareOrEnableAuxClippingPlanes(bool prepare, int objID);
void _disableAuxClippingPlanes();
void _selectLights(CSceneObject* object, CViewableBase* viewable);
void _restoreDefaultLights(CSceneObject* object, CViewableBase* viewable);
void _commonStart(CSceneObject* object, CViewableBase* viewable);
void _commonFinish(CSceneObject* object, CViewableBase* viewable);
void _displayBoundingBox(CSceneObject* object, CViewableBase* viewable, bool mainSelection);
void _displayFrame(const C7Vector& tr, double frameSize, int color = 0);
void _displayBoundingBox(const C3Vector* objectFrame, const C7Vector& absBBFrame, const C3Vector& bbSize,
                         CSceneObject* object, bool mainSelection);

void _drawTriangles(const float* vertices, int verticesCnt, const int* indices, int indicesCnt, const float* normals,
                    const float* textureCoords, int* vertexBufferId, int* normalBufferId, int* texCoordBufferId);
bool _drawEdges(const float* vertices, int verticesCnt, const int* indices, int indicesCnt, const unsigned char* edges,
                int* edgeBufferId);
void _drawColorCodedTriangles(const float* vertices, int verticesCnt, const int* indices, int indicesCnt,
                              const float* normals, int* vertexBufferId, int* normalBufferId);

bool _start3DTextureDisplay(CTextureProperty* tp);
void _end3DTextureDisplay(CTextureProperty* tp);
bool _start2DTextureDisplay(CTextureProperty* tp);
void _end2DTextureDisplay(CTextureProperty* tp);

void _startTextureDisplay(CTextureObject* to, bool interpolateColor, int applyMode, bool repeatX, bool repeatY);
void _endTextureDisplay();

void initializeRendering();
void deinitializeRendering();
#ifdef USES_QGLWIDGET
void initGl_ifNeeded();
#else
void initGl_openGLWidget();
#endif
void deinitGl_ifNeeded();
void increaseVertexBufferRefCnt(int vertexBufferId);
void decreaseVertexBufferRefCnt(int vertexBufferId);
void increaseNormalBufferRefCnt(int normalBufferId);
void decreaseNormalBufferRefCnt(int normalBufferId);
void increaseEdgeBufferRefCnt(int edgeBufferId);
void decreaseEdgeBufferRefCnt(int edgeBufferId);
void decreaseTexCoordBufferRefCnt(int texCoordBufferId);
void destroyGlTexture(unsigned int texName);
void makeColorCurrent(const CColorObject* visParam, bool forceNonTransparent, bool useAuxiliaryComponent);
