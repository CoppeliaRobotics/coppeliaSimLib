
#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _displayInertia(CGeomWrap* geomWrap,float bboxDiagonal,const float normalVectorForPointsAndLines[3]);
void _displayTriangles(CGeometric* geometric,int geomModifCounter,CTextureProperty* tp);
#endif

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib);
void displayGeometric(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
void displayGeometric_colorCoded(CGeometric* geometric,CGeomProxy* geomData,int objectId,int displayAttrib);
void displayGeometricForCutting(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,const float normalVectorForPointsAndLines[3]);
void displayGeometricGhost(CGeometric* geometric,CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors);
