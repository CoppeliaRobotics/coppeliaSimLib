#pragma once

#include <meshWrapper.h>
#include <textureProperty.h>
#include <simLib/simConst.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                          \
    FUNCX(propMesh_textureResolution, "textureResolution", sim_propertytype_intarray2, sim_propertyinfo_notwritable, "Texture resolution", "")     \
    FUNCX(propMesh_textureCoordinates, "textureCoordinates", sim_propertytype_floatarray, sim_propertyinfo_notwritable, "Texture coordinates", "") \
    FUNCX(propMesh_textureApplyMode, "textureApplyMode", sim_propertytype_int, 0, "Texture apply mode", "")                                        \
    FUNCX(propMesh_textureRepeatU, "textureRepeatU", sim_propertytype_bool, 0, "Texture repeat U", "")                                             \
    FUNCX(propMesh_textureRepeatV, "textureRepeatV", sim_propertytype_bool, 0, "Texture repeat V", "")                                             \
    FUNCX(propMesh_textureInterpolate, "textureInterpolate", sim_propertytype_bool, 0, "Interpolate texture", "")                                  \
    FUNCX(propMesh_texture, "rawTexture", sim_propertytype_buffer, sim_propertyinfo_notwritable, "Texture", "")                                    \
    FUNCX(propMesh_textureID, "textureID", sim_propertytype_int, sim_propertyinfo_notwritable, "Texture ID", "")                                   \
    FUNCX(propMesh_vertices, "vertices", sim_propertytype_floatarray, sim_propertyinfo_notwritable, "Vertices", "")                                \
    FUNCX(propMesh_indices, "indices", sim_propertytype_intarray, sim_propertyinfo_notwritable, "Indices", "Indices (3 values per triangle)")      \
    FUNCX(propMesh_normals, "normals", sim_propertytype_floatarray, sim_propertyinfo_notwritable, "Normals", "Normals (3*3 values per triangle)")  \
    FUNCX(propMesh_shadingAngle, "shadingAngle", sim_propertytype_float, 0, "Shading angle", "")                                                   \
    FUNCX(propMesh_showEdges, "showEdges", sim_propertytype_bool, 0, "Visible edges", "")                                                          \
    FUNCX(propMesh_culling, "culling", sim_propertytype_bool, 0, "Backface culling", "")                                                           \
    FUNCX(propMesh_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable, "Object type", "")                             \
    FUNCX(propMesh_shapeUid, "shapeUid", sim_propertytype_int, sim_propertyinfo_notwritable, "Shape UID", "Unique identifier of parent shape")     \
    FUNCX(propMesh_convex, "convex", sim_propertytype_bool, sim_propertyinfo_notwritable, "Convex", "Whether mesh is convex or not")               \
    FUNCX(propMesh_colorName, "colorName", sim_propertytype_string, 0, "Color name", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_mesh = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CMesh : public CMeshWrapper
{
  public:
    CMesh();
    CMesh(const C7Vector& meshFrame, const std::vector<double>& vertices, const std::vector<int>& indices,
          const std::vector<double>* optNormals, const std::vector<float>* optTexCoords, int options);
    virtual ~CMesh();

    void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    void performSceneObjectLoadingMapping(const std::map<int, int>* map);
    void performTextureObjectLoadingMapping(const std::map<int, int>* map);
    void announceSceneObjectWillBeErased(const CSceneObject* object);
    void setTextureDependencies(int shapeID);
    bool getContainsTransparentComponents() const;
    CMesh* copyYourself();
    void scale(double isoVal);
    void scale(double xVal, double yVal, double zVal);
    int getPurePrimitiveType() const;
    void setPurePrimitiveType(int theType, double xOrDiameter, double y, double zOrHeight);
    bool isMesh() const;
    bool isPure() const;
    bool isConvex() const;
    void setConvex_raw(bool c);
    bool checkIfConvex();
    CMesh* getFirstMesh();
    CMesh* getMeshFromUid(long long int meshUid, const C7Vector& parentCumulTr, C7Vector& shapeRelTr);
    void pushObjectCreationEvent(int shapeUid, const C7Vector& shapeRelTr);
    void pushObjectRemoveEvent();

    int countTriangles() const;
    void getCumulativeMeshes(const C7Vector& parentCumulTr, std::vector<double>& vertices, std::vector<int>* indices,
                             std::vector<double>* normals);
    void getCumulativeMeshes(const C7Vector& parentCumulTr, const CMeshWrapper* wrapper, std::vector<double>& vertices,
                             std::vector<int>* indices, std::vector<double>* normals);
    void setColor(const CShape* shape, int& elementIndex, const char* colorName, int colorComponent,
                  const float* rgbData, int& rgbDataOffset);
    bool getColor(const char* colorName, int colorComponent, float* rgbData, int& rgbDataOffset) const;
    void getAllMeshComponentsCumulative(const C7Vector& parentCumulTr, std::vector<CMesh*>& shapeComponentList,
                                        std::vector<C7Vector>* OptParentCumulTrList = nullptr);
    CMesh* getMeshComponentAtIndex(const C7Vector& parentCumulTr, int& index, C7Vector* optParentCumulTrOut = nullptr);
    int getComponentCount() const;
    bool serialize(CSer& ar, const char* shapeName, const C7Vector& parentCumulIFrame, bool rootLevel);
    void flipFaces();
    void setHideEdgeBorders_OLD(bool v);
    bool getHideEdgeBorders_OLD() const;
    int getTextureCount() const;
    bool hasTextureThatUsesFixedTextureCoordinates() const;
    void removeAllTextures();
    void getColorStrings(std::string& colorStrings, bool onlyNamed) const;
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

    double getShadingAngle() const;
    void setShadingAngle(double angle);
    double getEdgeThresholdAngle() const;
    void setEdgeThresholdAngle(double angle);
    void setVisibleEdges(bool v);
    bool getVisibleEdges() const;
    void setEdgeWidth_DEPRECATED(int w);
    int getEdgeWidth_DEPRECATED() const;
    void setCulling(bool c);
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
    void takeVisualAttributesFrom(CMesh* origin);

    bool reorientBB(const C4Vector* rot);
    void setBBFrame(const C7Vector& bbFrame);

    int setBoolProperty(const char* pName, bool pState, const C7Vector& shapeRelTr);
    int getBoolProperty(const char* pName, bool& pState, const C7Vector& shapeRelTr) const;
    int setIntProperty(const char* pName, int pState, const C7Vector& shapeRelTr);
    int getIntProperty(const char* pName, int& pState, const C7Vector& shapeRelTr) const;
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
    static int getPropertyName(int& index, std::string& pName, CMesh* targetObject);
    static int getPropertyInfo(const char* pName, int& info, std::string& infoTxt, CMesh* targetObject);

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
    void display(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, CColorObject* collisionColor,
                 int dynObjFlag_forVisualization, int transparencyHandling, bool multishapeEditSelected);
    void display_colorCoded(const C7Vector& cumulIFrameTr, CShape* geomData, int objectId, int displayAttrib);
    void displayGhost(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, bool originalColors,
                      bool backfaceCulling, double transparency, const float* newColors);

    void display_extRenderer(const C7Vector& cumulIFrameTr, CShape* geomData, int displayAttrib, const C7Vector& tr,
                             int shapeHandle, int& componentIndex);
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
