
#pragma once

#include "vrepMainHeader.h"
#include "shape.h"
#include "objCont.h"
#include "textureContainer.h"
#include "uiThread.h"
#include "textureProperty.h"
#include "edgeCont.h"
#include "vMenubar.h"

class CShapeEditMode
{
public:
    CShapeEditMode(CShape* shape,int editModeType,CObjCont* objCont,CTextureContainer* textureCont,CUiThread* uiThread,bool identicalVerticesCheck,bool identicalTrianglesCheck,float identicalVerticesTolerance);
    virtual ~CShapeEditMode();

    bool endEditMode(bool cancelChanges);
    int getEditModeType();
    CShape* getEditModeShape();
    void swapShapeEditModeType(int theType);

    void selectionFromTriangleToVertexEditMode(std::vector<int>* newVertexSel=nullptr);
    void selectionFromVertexToTriangleEditMode(std::vector<int>* newTriangleSel=nullptr);
    void selectionFromEdgeToTriangleEditMode(std::vector<int>* newTriangleSel=nullptr);
    void selectionFromEdgeToVertexEditMode(std::vector<int>* newTriangleSel=nullptr);
    void selectionFromTriangleToEdgeEditMode(std::vector<int>* newEdgeSel=nullptr);
    void selectionFromVertexToEdgeEditMode(std::vector<int>* newEdgeSel=nullptr);

    // Edit mode selection routines:
    int getEditModeBufferSize();
    int getLastEditModeBufferValue();
    int getEditModeBufferValue(int index);
    std::vector<int>* getEditModeBuffer();
    C3Vector getEditionVertex(int index);
    void setEditionVertex(int index,const C3Vector& v);
    void getEditionTriangle(int index,int tri[3]);
    void getEditionEdge(int index,int edge[2]);
    void deselectEditModeBuffer();
    void removeItemFromEditModeBuffer(int item);
    bool isEditModeItemAValidItem(int item);
    void xorAddItemToEditModeBuffer(int item,bool disableEdgeFollowing);
    void addItemToEditModeBuffer(int item,bool disableEdgeFollowing);
    bool alreadyInEditModeBuffer(int item);

    // Other edit mode routines:
    void actualizeEditModeEditionEdges();
    float getEdgeAngle(int edgeID);
    void getNextEdges(int rearVertex,int frontVertex,std::vector<int>& edgeList);

    void displayVertices(int displayAttrib);
    void displayFaceOrientation(int displayAttrib);
    void displayEdgeEditMode(int displayAttrib);
    void flipTriangle(int index);
    void addTriangle(int ind1,int ind2,int ind3);

    void copySelection(std::vector<int>* selection);
    void paste();
    void cutSelection(std::vector<int>* selection);
    void deleteSelection(std::vector<int>* selection);
    void addMenu(VMenu* menu);
    bool processCommand(int commandID);
    void copySelectedFaces(std::vector<int>* sel,std::vector<float>* vert,std::vector<int>* ind,std::vector<float>* norm,std::vector<float>* tex);

    bool getShowHiddenVerticeAndEdges();
    void setShowHiddenVerticeAndEdges(bool show);
    bool getAutomaticallyFollowEdges();
    void setAutomaticallyFollowEdges(bool follow);
    float getEdgeMaxAngle();
    void setEdgeMaxAngle(float a);
    float getEdgeDirectionChangeMaxAngle();
    void setEdgeDirectionChangeMaxAngle(float a);

    int getEditionVerticesSize();
    int getEditionIndicesSize();
    int getEditionEdgesSize();

    void makeShape();
    void makePrimitive(int what);
    void flipTriangles();
    void subdivideTriangles();

    void insertTriangles();
    void insertTriangleFan();
    void makeDummies();

    void makePath();

private:
    CShape* _shape;
    int _editModeType;
    CObjCont* _objCont;
    CTextureContainer* _textureCont;
    CUiThread* _uiThread;

    std::vector<int> editModeBuffer;
    std::vector<float> editBufferVerticesCopy;
    std::vector<int> editBufferIndicesCopy;
    std::vector<float> editBufferNormalsCopy;
    std::vector<float> editBufferTextureCoordsCopy;

    std::vector<float> _editionVertices;
    std::vector<int> _editionIndices;
    std::vector<float> _editionNormals;
    std::vector<float> _editionTextureCoords;
    CTextureProperty* _editionTextureProperty;
    std::vector<unsigned char> _editionTexture;
    CEdgeCont _edgeCont;
    std::vector<int> _editionIndicesToEdgesIndex;
    bool showHiddenVerticeAndEdges;
    bool automaticallyFollowEdges;
    float edgeMaxAngle;
    float edgeDirectionChangeMaxAngle;
    bool _identicalVerticesCheck;
    bool _identicalTrianglesCheck;
    float _identicalVerticesTolerance;
};
