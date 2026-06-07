#pragma once

#include <colorObject.h>
#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <dynMaterialObject.h>
#include <obj.h>

class CViewableBase;
class CShape;
class CMesh;
class CSceneObject;

class CMeshWrapper : public Obj
{
  public:
    CMeshWrapper();
    virtual ~CMeshWrapper();

    void addItem(CMeshWrapper* m);
    void detachItems();

    virtual void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    virtual void performSceneObjectLoadingMapping(const std::map<int, int>* map);
    virtual void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map);
    virtual void announceSceneObjectWillBeErased(const CSceneObject* object);
    virtual void setTextureDependencies(int shapeID);
    virtual bool getContainsTransparentComponents() const;
    virtual void displayGhost(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib, bool originalColors,
                              bool backfaceCulling, double transparency, const float* newColors);
    virtual void display(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib,
                         CColorObject* collisionColor, int dynObjFlag_forVisualization, int transparencyHandling,
                         bool multishapeEditSelected);
    virtual void display_extRenderer(const CPose& cumulIFrameTr, CShape* geomData, int displayAttrib,
                                     const CPose& tr, int shapeHandle, int& componentIndex);
    virtual void display_colorCoded(const CPose& cumulIFrameTr, CShape* geomData, int objectId, int displayAttrib);
    virtual CMeshWrapper* copyYourself();
    virtual void scale(double isoVal);
    virtual void setPurePrimitiveType(int theType, double xOrDiameter, double y, double zOrHeight);
    virtual int getPurePrimitiveType() const;
    virtual bool isMesh() const;
    virtual bool isPure() const;
    virtual bool isConvex() const;
    virtual void takeVisualAttributesFrom(CMesh* origin);
    virtual int countTriangles() const;
    virtual void getCumulativeMeshes(const CPose& parentCumulTr, std::vector<double>& vertices,
                                     std::vector<int>* indices, std::vector<double>* normals);
    virtual void getCumulativeMeshes(const CPose& parentCumulTr, const CMeshWrapper* wrapper,
                                     std::vector<double>& vertices, std::vector<int>* indices,
                                     std::vector<double>* normals);
    virtual void setColor(int colorComponent, const float* rgbData); // cumulative
    virtual void setColor(const CShape* shape, int& elementIndex, const char* colorName, int colorComponent,
                          const float* rgbData, int& rgbDataOffset);
    virtual bool getColor(const char* colorName, int colorComponent, float* rgbData, int& rgbDataOffset) const;
    virtual void getAllMeshComponentsCumulative(const CPose& parentCumulTr, std::vector<CMesh*>& shapeComponentList,
                                                std::vector<CPose>* OptParentCumulTrList = nullptr);
    virtual CMesh* getMeshComponentAtIndex(const CPose& parentCumulTr, int& index,
                                           CPose* optParentCumulTrOut = nullptr);
    virtual int getComponentCount() const;
    virtual bool serialize(CSer& ar, const char* shapeName, const CPose& parentCumulIFrame, bool rootLevel);
    virtual void flipFaces();
    virtual double getShadingAngle() const;
    virtual void setCulling(bool c);
    virtual void setVisibleEdges(bool v);
    virtual void setShadingAngle(double angle);
    virtual double getEdgeThresholdAngle() const;
    virtual void setEdgeThresholdAngle(double angle);
    virtual void setHideEdgeBorders_OLD(bool v);
    virtual bool getHideEdgeBorders_OLD() const;
    virtual int getTextureCount() const;
    virtual bool hasTextureThatUsesFixedTextureCoordinates() const;
    virtual void removeAllTextures();
    virtual void getColorStrings(std::string& colorStrings, bool onlyNamed) const;
    virtual CMesh* getFirstMesh();
    virtual CMesh* getMeshFromUid(int64_t meshUid, const CPose& parentCumulTr, CPose& shapeRelTr);
    virtual void appendMeshes(std::vector<CMesh*>& meshes);

    void addObjectEventData(int parentObjectHandle, CCbor* ev);
    int setFloatProperty_wrapper(const char* pName, double pState);
    int getFloatProperty_wrapper(const char* pName, double& pState) const;
    int setVector3Property_wrapper(const char* pName, const C3Vector& pState);
    int getVector3Property_wrapper(const char* pName, C3Vector& pState) const;
    int setMatrixProperty_wrapper(const char* pName, const CMatrix& pState);
    int getMatrixProperty_wrapper(const char* pName, CMatrix& pState) const;
    int setQuaternionProperty_wrapper(const char* pName, const CQuaternion& pState);
    int getQuaternionProperty_wrapper(const char* pName, CQuaternion& pState) const;
    int setFloatArrayProperty_wrapper(const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty_wrapper(const char* pName, std::vector<double>& pState) const;
    int getPropertyName_wrapper(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_wrapper(const char* pName, int& info, std::string& infoTxt) const;

    void copyAttributesTo(CMeshWrapper* target);
    void copyWrapperData(CMeshWrapper* target);
    void scaleMassAndInertia(double s);
    void setMass(double m);
    double getMass() const;
    void setName(std::string newName);
    std::string getName() const;

    int getDynMaterialId_old() const;
    void setDynMaterialId_old(int id);
    // ---------------------

    CPose getDiagonalInertiaInfo(C3Vector& diagMasslessI) const;
    CPose getBB(C3Vector* optBBSize) const;
    virtual void setBBFrame(const CPose& bbFrame);
    virtual bool reorientBB(const CQuaternion* rot);
    bool getShapeRelIFrame(const CPose& parentCumulTr, const CMeshWrapper* wrapper, CPose& shapeRelIFrame) const;
    bool getShapeRelBB(const CPose& parentCumulTr, const CMeshWrapper* wrapper, CPose& shapeRelBB,
                       C3Vector* optBBSize) const;
    C3Vector getCOM() const;
    CPose getIFrame() const;
    void setIFrame(const CPose& iframe);
    void setCOM(const C3Vector& com);
    C3X3Matrix getInertia() const;
    void setInertia(const C3X3Matrix& im, int modifItemRow = -1, int modifItemCol = -1);
    C3Vector getPMI() const;
    void setPMI(const C3Vector& pmi);
    std::string getInertiaErrorString() const;
    void setInertiaAndComputePMI(const C3X3Matrix& inertia, bool forcePMICalc = false);

    static bool getPMIFromInertia(const C3X3Matrix& tensor, CQuaternion& rotation, C3Vector& principalMoments);
    static C3X3Matrix getInertiaFromPMI(const C3Vector& principalMoments, const CPose& newFrame);
    static C3X3Matrix getInertiaInNewFrame(const CQuaternion& oldFrame, const C3X3Matrix& oldMatrix,
                                           const CQuaternion& newFrame);
    static std::string getInertiaErrorString(const C3X3Matrix& matrix);

    std::vector<CMeshWrapper*> childList;

  protected:
    void _commonInit();
    void _computeInertiaFromComposingInertias();
    std::string _name;
    double _mass;

    CPose _iFrame;      // Inertia ref. frame, relative to parent _iFrame. Identity if root
    C3Vector _com;         // Center of mass, relative to _iFrame
    C3X3Matrix _iMatrix;   // Mass-less inertia matrix, expressed in the _iFrame
    C3Vector _pmi;         // Principal moment of inertia (calculated from _iMatrix), expressed in the _iFrame
    CQuaternion _pmiRotFrame; // Frame of the principal moment of inertia (calculated from _iMatrix), expressed in the _iFrame
    CPose _bbFrame;     // Ref. frame of the bounding box and vertices, relative to _iFrame
    C3Vector _bbSize;      // Size of the bounding box, relative to _iFrame

    int _parentObjectHandle;

    int _dynMaterialId_old;
    bool _convex_OLD;
};
