#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_OPENGL
void _displayInertia(CMeshWrapper* geomWrap,float bboxDiagonal,const float normalVectorForPointsAndLines[3]);
void _displayTriangles(CMesh* geometric,int geomModifCounter,CTextureProperty* tp);
#endif

void displayShape(CShape* shape,CViewableBase* renderingObject,int displayAttrib);
void displayGeometric(CMesh* geometric,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
void displayGeometric_colorCoded(CMesh* geometric,CShape* geomData,int objectId,int displayAttrib);
void displayGeometricGhost(CMesh* geometric,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors);
