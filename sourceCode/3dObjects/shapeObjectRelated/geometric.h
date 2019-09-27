
#pragma once

#include "geomWrap.h"
#include "textureProperty.h"

class CGeometric : public CGeomWrap
{
public:
    CGeometric();
    virtual ~CGeometric();

    void display(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
    void display_colorCoded(CGeomProxy* geomData,int objectId,int displayAttrib);
    void displayForCutting(CGeomProxy* geomData,int displayAttrib,CVisualParam* collisionColor,const float normalVectorForPointsAndLines[3]);
    void displayGhost(CGeomProxy* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,float transparency,const float* newColors);

    void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    void display_extRenderer(CGeomProxy* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex);
    void perform3DObjectLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void announce3DObjectWillBeErased(int objectID);
    void setTextureDependencies(int shapeID);
    bool getContainsTransparentComponents();
    CGeometric* copyYourself();
    void scale(float xVal,float yVal,float zVal);
    int getPurePrimitiveType();
    void setPurePrimitiveType(int theType,float xOrDiameter,float y,float zOrHeight);
    bool isGeometric();
    bool isPure();
    bool isConvex();
    bool checkIfConvex();
    void setConvex(bool convex);
    bool containsOnlyPureConvexShapes();
    void getCumulativeMeshes(std::vector<float>& vertices,std::vector<int>* indices,std::vector<float>* normals);
    void setColor(const char* colorName,int colorComponent,const float* rgbData);
    bool getColor(const char* colorName,int colorComponent,float* rgbData);
    void getAllShapeComponentsCumulative(std::vector<CGeometric*>& shapeComponentList); // needed by the dynamics routine
    CGeometric* getShapeComponentAtIndex(int& index);
    void serialize(CSer& ar,const char* shapeName);
    void preMultiplyAllVerticeLocalFrames(const C7Vector& preTr);
    void flipFaces();
    float getGouraudShadingAngle();
    void setGouraudShadingAngle(float angle);
    float getEdgeThresholdAngle();
    void setEdgeThresholdAngle(float angle);
    void setHideEdgeBorders(bool v);
    bool getHideEdgeBorders();
    int getTextureCount();
    bool hasTextureThatUsesFixedTextureCoordinates();
    void removeAllTextures();
    void getColorStrings(std::string& colorStrings);
    void setHeightfieldDiamonds(int d); //0 for Bullet, 1 for ODE

    int getUniqueID();
    void setMesh(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>* normals,const C7Vector& transformation);
    void setMeshDataDirect(const std::vector<float>& vertices,const std::vector<int>& indices,const std::vector<float>& normals,const std::vector<unsigned char>& edges);



    void setHeightfieldData(const std::vector<float>& heights,int xCount,int yCount);
    float* getHeightfieldData(int& xCount,int& yCount,float& minHeight,float& maxHeight);
    void getPurePrimitiveSizes(C3Vector& s);
    void setPurePrimitiveInsideScaling(float s);
    float getPurePrimitiveInsideScaling();

    void setConvexVisualAttributes();

    C7Vector getVerticeLocalFrame();
    void setVerticeLocalFrame(const C7Vector& tr);

    CTextureProperty* getTextureProperty();
    void setTextureProperty(CTextureProperty* tp);

    void setVisibleEdges(bool v);
    bool getVisibleEdges();
    void setEdgeWidth_DEPRECATED(int w);
    int getEdgeWidth_DEPRECATED();
    void setCulling(bool c);
    bool getCulling();
    bool getDisplayInverted_DEPRECATED();
    void setDisplayInverted_DEPRECATED(bool di);

    void actualizeGouraudShadingAndVisibleEdges();

    void setInsideAndOutsideFacesSameColor_DEPRECATED(bool s);
    bool getInsideAndOutsideFacesSameColor_DEPRECATED();
    void setWireframe(bool w);
    bool getWireframe();

    std::vector<float>* getVertices();
    std::vector<int>* getIndices();
    std::vector<float>* getNormals();
    std::vector<unsigned char>* getEdges();
    int* getVertexBufferIdPtr();
    int* getNormalBufferIdPtr();
    int* getEdgeBufferIdPtr();


    void copyVisualAttributesTo(CGeometric* target);

    // Following few routines in order not to save duplicate data:
    static void clearTempVerticesIndicesNormalsAndEdges();
    static void serializeTempVerticesIndicesNormalsAndEdges(CSer& ar);
    static int getBufferIndexOfVertices(const std::vector<float>& vert);
    static int addVerticesToBufferAndReturnIndex(const std::vector<float>& vert);
    static void getVerticesFromBufferBasedOnIndex(int index,std::vector<float>& vert);
    static int getBufferIndexOfIndices(const std::vector<int>& ind);
    static int addIndicesToBufferAndReturnIndex(const std::vector<int>& ind);
    static void getIndicesFromBufferBasedOnIndex(int index,std::vector<int>& ind);
    static int getBufferIndexOfNormals(const std::vector<float>& norm);
    static int addNormalsToBufferAndReturnIndex(const std::vector<float>& norm);
    static void getNormalsFromBufferBasedOnIndex(int index,std::vector<float>& norm);
    static int getBufferIndexOfEdges(const std::vector<unsigned char>& edges);
    static int addEdgesToBufferAndReturnIndex(const std::vector<unsigned char>& edges);
    static void getEdgesFromBufferBasedOnIndex(int index,std::vector<unsigned char>& edges);


    CVisualParam color;
    CVisualParam insideColor_DEPRECATED;
    CVisualParam edgeColor_DEPRECATED;

    // Do not serialize nor copy following 3:
    std::vector<float> textureCoords_notCopiedNorSerialized; // 2 values per vertex

    std::vector<float> _heightfieldHeights;
    int _heightfieldXCount;
    int _heightfieldYCount;

protected:
    void _recomputeNormals();
    void _computeVisibleEdges();

    static void _savePackedIntegers(CSer& ar,const std::vector<int>& data);
    static void _loadPackedIntegers(CSer& ar,std::vector<int>& data);

    std::vector<float> _vertices;
    std::vector<int> _indices;
    std::vector<float> _normals;
    std::vector<unsigned char> _edges;
    
    bool _visibleEdges;
    bool _hideEdgeBorders;
    bool _culling;
    bool _displayInverted_DEPRECATED;
    bool _insideAndOutsideFacesSameColor_DEPRECATED;
    bool _wireframe;
    int _edgeWidth_DEPRERCATED;
    float _gouraudShadingAngle;
    float _edgeThresholdAngle;

    C7Vector _verticeLocalFrame; // frame relative to the shape. All vertices are transformed by it. This frame also represents the configuration of the origin frame of pure shapes!!!!
    CTextureProperty* _textureProperty;
    int _uniqueID;

    int _purePrimitive;
    float _purePrimitiveXSizeOrDiameter;
    float _purePrimitiveYSize;
    float _purePrimitiveZSizeOrHeight;
    float _purePrimitiveInsideScaling;

    int _tempVerticesIndexForSerialization;
    int _tempIndicesIndexForSerialization;
    int _tempNormalsIndexForSerialization;
    int _tempEdgesIndexForSerialization;

    int _vertexBufferId;
    int _normalBufferId;
    int _edgeBufferId;

    unsigned int _extRendererObjectId;
    unsigned int _extRendererObject_lastMeshId;
    unsigned int _extRendererObject_lastTextureId;

    unsigned int _extRendererMeshId;
    int _extRendererMesh_lastVertexBufferId;

    unsigned int _extRendererTextureId;
    unsigned int _extRendererTexture_lastTextureId;

    static unsigned int _extRendererUniqueObjectID;
    static unsigned int _extRendererUniqueMeshID;
    static unsigned int _extRendererUniqueTextureID;

    static int _nextUniqueID;


    static std::vector<std::vector<float>*> _tempVertices;
    static std::vector<std::vector<int>*> _tempIndices;
    static std::vector<std::vector<float>*> _tempNormals;
    static std::vector<std::vector<unsigned char>*> _tempEdges;

#ifdef SIM_WITH_GUI
public:
    bool getNonCalculatedTextureCoordinates(std::vector<float>& texCoords);

#endif
};
