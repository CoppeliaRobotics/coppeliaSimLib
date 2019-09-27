
#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _drawPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const float normalVectorForLinesAndPoints[3]);
void _drawTrianglePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawQuadPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawDiscPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawCubePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM);
void _drawSpherePoints(CDrawingObject* drawingObject,int displayAttrib);
void _drawLines(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const float normalVectorForLinesAndPoints[3]);
void _drawTriangles(CDrawingObject* drawingObject,int displayAttrib);
#endif

void displayDrawingObject(CDrawingObject* drawingObject,C7Vector& tr,bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
