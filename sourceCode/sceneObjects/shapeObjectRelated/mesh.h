#pragma once

#include <meshWrapper.h>
#include <textureProperty.h>
#include <simLib/simConst.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
MESH_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_mesh = {MESH_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

class CMesh : public CMeshWrapper
{
  public:
    CMesh();
    CMesh(const C7Vector& meshFrame, const std::vector<double>& vertices, const std::vector<int>& indices,
          const std::vector<double>* optNormals, const std::vector<float>* optTexCoords, int options);
    virtual ~CMesh();

    void prepareVerticesIndicesNormalsAndEdgesForSerialization() override;
    void performSceneObjectLoadingMapping(const std::map<int, int>* map) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void announceSceneObjectWillBeErased(const CSceneObject* object) override;
    void setTextureDependencies(int shapeID) override;
    bool getContainsTransparentComponents() const override;
    CMesh* copyYourself() override;
    void scale(double isoVal) override;
    void scale(double xVal, double yVal, double zVal);
    int getPurePrimitiveType() const override;
    void setPurePrimitiveType(int theType, double xOrDiameter, double y, double zOrHeight) override;
    bool isMesh() const override;
    bool isPure() const override;
    bool isConvex() const override;
    void setConvex_raw(bool c);
    bool checkIfConvex();
    CMesh* getFirstMesh() override;
    CMesh* getMeshFromUid(long long int meshUid, const C7Vector& parentCumulTr, C7Vector& shapeRelTr) override;
    void pushObjectCreationEvent(int shapeHandle, int shapeUid, const C7Vector& shapeRelTr);
    void pushObjectRemoveEvent();

    int countTriangles() const override;
    void getCumulativeMeshes(const C7Vector& parentCumulTr, std::vector<double>& vertices, std::vector<int>* indices, std::vector<double>* normals) override;
    void getCumulativeMeshes(const C7Vector& parentCumulTr, const CMeshWrapper* wrapper, std::vector<double>& vertices, std::vector<int>* indices, std::vector<double>* normals) override;
    void setColor(int colorComponent, const float* rgbData) override;
    void setColor(const CShape* shape, int& elementIndex, const char* colorName, int colorComponent, const float* rgbData, int& rgbDataOffset) override;
    bool getColor(const char* colorName, int colorComponent, float* rgbData, int& rgbDataOffset) const override;
    void getAllMeshComponentsCumulative(const C7Vector& parentCumulTr, std::vector<CMesh*>& shapeComponentList, std::vector<C7Vector>* OptParentCumulTrList = nullptr) override;
    CMesh* getMeshComponentAtIndex(const C7Vector& parentCumulTr, int& index, C7Vector* optParentCumulTrOut = nullptr) override;
    int getComponentCount() const override;
    bool serialize(CSer& ar, const char* shapeName, const C7Vector& parentCumulIFrame, bool rootLevel) override;
    void flipFaces() override;
    void setHideEdgeBorders_OLD(bool v) override;
    bool getHideEdgeBorders_OLD() const override;
    int getTextureCount() const override;
    bool hasTextureThatUsesFixedTextureCoordinates() const override;
    void removeAllTextures() override;
    void getColorStrings(std::string& colorStrings, bool onlyNamed) const override;
    void setHeightfieldDiamonds(bool d);

    long long int getUniqueID() const;

    void setHeightfieldData(const std::vector<double>& heights, int xCount, int yCount);
    double* getHeightfieldData(int& xCount, int& yCount, double& minHeight, double& maxHeight);
    void getPurePrimitiveSizes(C3Vector& s) const;
    void setPurePrimitiveInsideScaling_OLD(double s);
    double getPurePrimitiveInsideScaling_OLD() const;

    CTextureProperty* getTextureProperty();
    void setTextureProperty(CTextureProperty* tp);

    void setTextureRepeatU(bool r);
    bool getTextureRepeatU() const;
    void setTextureRepeatV(bool r);
    bool getTextureRepeatV() const;
    void setTextureInterpolate(bool r);
    bool getTextureInterpolate() const;
    void setTextureApplyMode(int m);
    int getTextureApplyMode() const;

    double getShadingAngle() const override;
    void setShadingAngle(double angle) override;
    double getEdgeThresholdAngle() const override;
    void setEdgeThresholdAngle(double angle) override;
    void setVisibleEdges(bool v) override;
    bool getVisibleEdges() const;
    void setEdgeWidth_DEPRECATED(int w);
    int getEdgeWidth_DEPRECATED() const;
    void setCulling(bool c) override;
    bool getCulling() const;
    void setColor(const float* c, unsigned char colorMode);

    bool getDisplayInverted_DEPRECATED() const;
    void setDisplayInverted_DEPRECATED(bool di);

    void actualizeGouraudShadingAndVisibleEdges();

    void setInsideAndOutsideFacesSameColor_DEPRECATED(bool s);
    bool getInsideAndOutsideFacesSameColor_DEPRECATED() const;
    void setWireframe_OLD(bool w);
    bool getWireframe_OLD() const;

    std::vector<double>* getVertices();
    std::vector<int>* getIndices();
    std::vector<double>* getNormals();
    std::vector<unsigned char>* getEdges();

    std::vector<float>* getVerticesForDisplayAndDisk();
    std::vector<float>* getNormalsForDisplayAndDisk();

    void copyVisualAttributesTo(CMeshWrapper* target);
    void takeVisualAttributesFrom(CMesh* origin) override;

    bool reorientBB(const C4Vector* rot) override;
    void setBBFrame(const C7Vector& bbFrame) override;

    int setBoolProperty(const char* pName, bool pState, const C7Vector& shapeRelTr);
    int getBoolProperty(const char* pName, bool& pState, const C7Vector& shapeRelTr) const;
    int setIntProperty(const char* pName, int pState, const C7Vector& shapeRelTr);
    int getIntProperty(const char* pName, int& pState, const C7Vector& shapeRelTr) const;
    int getLongProperty(const char* pName, long long int& pState, const C7Vector& shapeRelTr) const;
    int getHandleProperty(const char* pName, long long int& pState, const C7Vector& shapeRelTr) const;
    int setFloatProperty(const char* pName, double pState, const C7Vector& shapeRelTr);
    int getFloatProperty(const char* pName, double& pState, const C7Vector& shapeRelTr) const;
    int setStringProperty(const char* pName, const char* pState, const C7Vector& shapeRelTr);
    int getStringProperty(const char* pName, std::string& pState, const C7Vector& shapeRelTr) const;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL, const C7Vector& shapeRelTr);
    int getBufferProperty(const char* pName, std::string& pState, const C7Vector& shapeRelTr) const;
    int setIntArray2Property(const char* pName, const int* pState, const C7Vector& shapeRelTr);
    int getIntArray2Property(const char* pName, int* pState, const C7Vector& shapeRelTr) const;
    int setVector2Property(const char* pName, const double* pState, const C7Vector& shapeRelTr);
    int getVector2Property(const char* pName, double* pState, const C7Vector& shapeRelTr) const;
    int setVector3Property(const char* pName, const C3Vector& pState, const C7Vector& shapeRelTr);
    int getVector3Property(const char* pName, C3Vector& pState, const C7Vector& shapeRelTr) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState, const C7Vector& shapeRelTr);
    int getQuaternionProperty(const char* pName, C4Vector& pState, const C7Vector& shapeRelTr) const;
    int setPoseProperty(const char* pName, const C7Vector& pState, const C7Vector& shapeRelTr);
    int getPoseProperty(const char* pName, C7Vector& pState, const C7Vector& shapeRelTr) const;
    int setColorProperty(const char* pName, const float* pState, const C7Vector& shapeRelTr);
    int getColorProperty(const char* pName, float* pState, const C7Vector& shapeRelTr) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL, const C7Vector& shapeRelTr);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState, const C7Vector& shapeRelTr) const;
    int setIntArrayProperty(const char* pName, const int* v, int vL, const C7Vector& shapeRelTr);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState, const C7Vector& shapeRelTr) const;
    int removeProperty(const char* pName);
    static int getPropertyName(int& index, std::string& pName, CMesh* targetObject, int excludeFlags);
    static int getPropertyInfo(const char* pName, int& info, std::string& infoTxt, CMesh* targetObject);

    std::string getMeshState() const;

    // Following few routines in order not to save duplicate data:
    static void clearTempVerticesIndicesNormalsAndEdges();
    static void serializeTempVerticesIndicesNormalsAndEdges(CSer& ar);
    static int getBufferIndexOfVertices(const std::vector<float>& vert);
    static int addVerticesToBufferAndReturnIndex(const std::vector<float>& vert);
    static void getVerticesFromBufferBasedOnIndex(int index, std::vector<float>& vert);
    static int getBufferIndexOfIndices(const std::vector<int>& ind);
    static int addIndicesToBufferAndReturnIndex(const std::vector<int>& ind);
    static void getIndicesFromBufferBasedOnIndex(int index, std::vector<int>& ind);
    static int getBufferIndexOfNormals(const std::vector<float>& norm);
    static int addNormalsToBufferAndReturnIndex(const std::vector<float>& norm);
    static void getNormalsFromBufferBasedOnIndex(int index, std::vector<float>& norm);
    static int getBufferIndexOfEdges(const std::vector<unsigned char>& edges);
    static int addEdgesToBufferAndReturnIndex(const std::vector<unsigned char>& edges);
    static void getEdgesFromBufferBasedOnIndex(int index, std::vector<unsigned char>& edges);

    CColorObject color;
    CColorObject insideColor_DEPRECATED;
    CColorObject edgeColor_DEPRECATED;

    std::vector<double> _heightfieldHeights;
    int _heightfieldXCount;
    int _heightfieldYCount;

  protected:
    void _updateNonDisplayAndNonDiskValues();
    void _updateDisplayAndDiskValues();
    void _transformMesh(const C7Vector& tr);
    void _commonInit();
    void _recomputeNormals();
    void _computeVisibleEdges();
    C3Vector _computeBBSize(C3Vector* optBBCenter = nullptr);

    static void _loadPackedIntegers_OLD(CSer& ar, std::vector<int>& data);

    std::vector<double> _vertices;
    std::vector<int> _indices;
    std::vector<double> _normals;
    std::vector<unsigned char> _edges;

    std::vector<float> _verticesForDisplayAndDisk;
    std::vector<float> _normalsForDisplayAndDisk;

    bool _visibleEdges;
    bool _hideEdgeBorders_OLD;
    bool _culling;
    bool _displayInverted_DEPRECATED;
    bool _insideAndOutsideFacesSameColor_DEPRECATED;
    bool _wireframe_OLD;
    int _edgeWidth_DEPRERCATED;
    double _shadingAngle;
    double _edgeThresholdAngle;
    bool _convex;
    int _isInSceneShapeHandle;
    int _isInSceneShapeUid;

    CTextureProperty* _textureProperty;
    long long int _uniqueID;

    int _purePrimitive;
    double _purePrimitiveXSizeOrDiameter;
    double _purePrimitiveYSize;
    double _purePrimitiveZSizeOrHeight;
    double _purePrimitiveInsideScaling;

    int _tempVerticesIndexForSerialization;
    int _tempIndicesIndexForSerialization;
    int _tempNormalsIndexForSerialization;
    int _tempEdgesIndexForSerialization;

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

    // temp, for serialization purpose:
    static std::vector<std::vector<float>*> _tempVerticesForDisk;
    static std::vector<std::vector<int>*> _tempIndicesForDisk;
    static std::vector<std::vector<float>*> _tempNormalsForDisk;
    static std::vector<std::vector<unsigned char>*> _tempEdgesForDisk;

#ifdef SIM_WITH_GUI
  public:
    void display(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, CColorObject* collisionColor, int dynObjFlag_forVisualization, int transparencyHandling, bool multishapeEditSelected) override;
    void display_colorCoded(const C7Vector& cumulIFrameTr, CShape* geomData, int objectId, int displayAttrib) override;
    void displayGhost(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, bool originalColors, bool backfaceCulling, double transparency, const float* newColors) override;

    void display_extRenderer(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, const C7Vector& tr, int shapeHandle, int& componentIndex) override;
    bool getNonCalculatedTextureCoordinates(std::vector<float>& texCoords);
    int* getVertexBufferIdPtr();
    int* getNormalBufferIdPtr();
    int* getEdgeBufferIdPtr();

  protected:
    int _vertexBufferId;
    int _normalBufferId;
    int _edgeBufferId;
#endif
};
