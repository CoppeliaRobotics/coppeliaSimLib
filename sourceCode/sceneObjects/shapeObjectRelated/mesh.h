#pragma once

#include <meshWrapper.h>
#include <textureProperty.h>
#include <simLib/simConst.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
MESH_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_mesh;
// ----------------------------------------------------------------------------------------------

class CMesh : public CMeshWrapper
{
  public:
    CMesh();
    CMesh(const CPose& meshFrame, const std::vector<double>& vertices, const std::vector<int>& indices,
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
    CMesh* getMeshFromUid(long long int meshUid, const CPose& parentCumulTr, CPose& shapeRelTr) override;
    void appendMeshes(std::vector<CMesh*>& meshes) override;
    void pushObjectCreationEvent(int shapeHandle, int shapeUid, const CPose& shapeRelTr);
    void pushObjectRemoveEvent();

    int countTriangles() const override;
    void getCumulativeMeshes(const CPose& parentCumulTr, std::vector<double>& vertices, std::vector<int>* indices, std::vector<double>* normals) override;
    void getCumulativeMeshes(const CPose& parentCumulTr, const CMeshWrapper* wrapper, std::vector<double>& vertices, std::vector<int>* indices, std::vector<double>* normals) override;
    void setColor(int colorComponent, const float* rgbData) override;
    void setColor(const CShape* shape, int& elementIndex, const char* colorName, int colorComponent, const float* rgbData, int& rgbDataOffset) override;
    bool getColor(const char* colorName, int colorComponent, float* rgbData, int& rgbDataOffset) const override;
    void getAllMeshComponentsCumulative(const CPose& parentCumulTr, std::vector<CMesh*>& shapeComponentList, std::vector<CPose>* OptParentCumulTrList = nullptr) override;
    CMesh* getMeshComponentAtIndex(const CPose& parentCumulTr, int& index, CPose* optParentCumulTrOut = nullptr) override;
    int getComponentCount() const override;
    bool serialize(CSer& ar, const char* shapeName, const CPose& parentCumulIFrame, bool rootLevel) override;
    void flipFaces() override;
    void setHideEdgeBorders_OLD(bool v) override;
    bool getHideEdgeBorders_OLD() const override;
    int getTextureCount() const override;
    bool hasTextureThatUsesFixedTextureCoordinates() const override;
    void removeAllTextures() override;
    void getColorStrings(std::string& colorStrings, bool onlyNamed) const override;
    void setHeightfieldDiamonds(bool d);

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
    void setWireframe(bool w);
    bool getWireframe() const;

    std::vector<double>* getVertices();
    std::vector<int>* getIndices();
    std::vector<double>* getNormals();
    std::vector<unsigned char>* getEdges();

    std::vector<float>* getVerticesForDisplayAndDisk();
    std::vector<float>* getNormalsForDisplayAndDisk();

    void copyVisualAttributesTo(CMeshWrapper* target);
    void takeVisualAttributesFrom(CMesh* origin) override;

    bool reorientBB(const CQuaternion* rot) override;
    void setBBFrame(const CPose& bbFrame) override;

    int setBoolProperty_mesh(const char* pName, bool pState, const CPose& shapeRelTr);
    int getBoolProperty_mesh(const char* pName, bool& pState, const CPose& shapeRelTr) const;
    int setIntProperty_mesh(const char* pName, int pState, const CPose& shapeRelTr);
    int getIntProperty_mesh(const char* pName, int& pState, const CPose& shapeRelTr) const;
    int getLongProperty_mesh(const char* pName, long long int& pState, const CPose& shapeRelTr) const;
    int getHandleProperty_mesh(const char* pName, long long int& pState, const CPose& shapeRelTr) const;
    int setFloatProperty_mesh(const char* pName, double pState, const CPose& shapeRelTr);
    int getFloatProperty_mesh(const char* pName, double& pState, const CPose& shapeRelTr) const;
    int setStringProperty_mesh(const char* pName, const std::string& pState, const CPose& shapeRelTr);
    int getStringProperty_mesh(const char* pName, std::string& pState, const CPose& shapeRelTr) const;
    int setBufferProperty_mesh(const char* pName, const std::string& pState, const CPose& shapeRelTr);
    int getBufferProperty_mesh(const char* pName, std::string& pState, const CPose& shapeRelTr) const;
    int setIntArray2Property_mesh(const char* pName, const int* pState, const CPose& shapeRelTr);
    int getIntArray2Property_mesh(const char* pName, int* pState, const CPose& shapeRelTr) const;
    int setVector3Property_mesh(const char* pName, const C3Vector& pState, const CPose& shapeRelTr);
    int getVector3Property_mesh(const char* pName, C3Vector& pState, const CPose& shapeRelTr) const;
    int getMatrixProperty_mesh(const char* pName, CMatrix& pState, const CPose& shapeRelTr) const;
    int setQuaternionProperty_mesh(const char* pName, const CQuaternion& pState, const CPose& shapeRelTr);
    int getQuaternionProperty_mesh(const char* pName, CQuaternion& pState, const CPose& shapeRelTr) const;
    int setPoseProperty_mesh(const char* pName, const CPose& pState, const CPose& shapeRelTr);
    int getPoseProperty_mesh(const char* pName, CPose& pState, const CPose& shapeRelTr) const;
    int setColorProperty_mesh(const char* pName, const float* pState, const CPose& shapeRelTr);
    int getColorProperty_mesh(const char* pName, float* pState, const CPose& shapeRelTr) const;
    int setFloatArrayProperty_mesh(const char* pName, const std::vector<double>& pState, const CPose& shapeRelTr);
    int getFloatArrayProperty_mesh(const char* pName, std::vector<double>& pState, const CPose& shapeRelTr) const;
    int setIntArrayProperty_mesh(const char* pName, const std::vector<int>& pState, const CPose& shapeRelTr);
    int getIntArrayProperty_mesh(const char* pName, std::vector<int>& pState, const CPose& shapeRelTr) const;
    int getStringArrayProperty_mesh(const char* pName, std::vector<std::string>& pState, const CPose& shapeRelTr) const;
    int removeProperty(const char* pName) override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;

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
    void _transformMesh(const CPose& tr);
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
    bool _wireframe;
    int _edgeWidth_DEPRERCATED;
    double _shadingAngle;
    double _edgeThresholdAngle;
    bool _convex;
    int _isInSceneShapeHandle;
    int _isInSceneShapeUid;

    CTextureProperty* _textureProperty;

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
    void display(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib, CColorObject* collisionColor, int dynObjFlag_forVisualization, int transparencyHandling, bool multishapeEditSelected) override;
    void display_colorCoded(const CPose& cumulIFrameTr, CShape* geomData, int objectId, int displayAttrib) override;
    void displayGhost(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib, bool originalColors, bool backfaceCulling, double transparency, const float* newColors) override;

    void display_extRenderer(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib, const CPose& tr, int shapeHandle, int& componentIndex) override;
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
