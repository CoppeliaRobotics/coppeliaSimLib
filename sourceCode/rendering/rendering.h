
#include "app.h"

#ifdef SIM_WITH_OPENGL
#include "oGL.h"

void _activateNonAmbientLights(int lightHandle,CViewableBase* viewable);
void _prepareAuxClippingPlanes();
void _enableAuxClippingPlanes(int objID);
void _prepareOrEnableAuxClippingPlanes(bool prepare,int objID);
void _disableAuxClippingPlanes();
void _drawReference(C3DObject* object,float refSize);
void _displayBoundingBox(C3DObject* object,int displayAttrib,bool displRef,float refSize);
void _selectLights(C3DObject* object,CViewableBase* viewable);
void _restoreDefaultLights(C3DObject* object,CViewableBase* viewable);
void _commonStart(C3DObject* object,CViewableBase* viewable,int displayAttrib);
void _commonFinish(C3DObject* object,CViewableBase* viewable);

void _drawTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,const float* textureCoords,int* vertexBufferId,int* normalBufferId,int* texCoordBufferId);
bool _drawEdges(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const unsigned char* edges,int* edgeBufferId);
void _drawColorCodedTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,int* vertexBufferId,int* normalBufferId);

bool _start3DTextureDisplay(CTextureProperty* tp);
void _end3DTextureDisplay(CTextureProperty* tp);
bool _start2DTextureDisplay(CTextureProperty* tp);
void _end2DTextureDisplay(CTextureProperty* tp);

void _startTextureDisplay(CTextureObject* to,bool interpolateColor,int applyMode,bool repeatX,bool repeatY);
void _endTextureDisplay();

#endif

void initializeRendering();
void deinitializeRendering();
void initGl_ifNeeded();
void deinitGl_ifNeeded();
void increaseVertexBufferRefCnt(int vertexBufferId);
void decreaseVertexBufferRefCnt(int vertexBufferId);
void increaseNormalBufferRefCnt(int normalBufferId);
void decreaseNormalBufferRefCnt(int normalBufferId);
void increaseEdgeBufferRefCnt(int edgeBufferId);
void decreaseEdgeBufferRefCnt(int edgeBufferId);
void decreaseTexCoordBufferRefCnt(int texCoordBufferId);
void destroyGlTexture(unsigned int texName);
void makeColorCurrent(const CVisualParam* visParam,bool forceNonTransparent,bool useAuxiliaryComponent);
