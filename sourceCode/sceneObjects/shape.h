#pragma once

#include <sceneObject.h>
#include <mesh.h>
#include <dummy.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
SHAPE_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_shape;
// ----------------------------------------------------------------------------------------------

class CShape : public CSceneObject
{
  public:
    CShape();
    CShape(const std::vector<double>& allHeights, int xSize, int ySize, double dx, double minH, double maxH); // heightfield
    CShape(const C7Vector& transformation, const std::vector<double>& vertices, const std::vector<int>& indices,
           const std::vector<double>* optNormals, const std::vector<float>* optTexCoords, int options); // mesh
    virtual ~CShape();

    void replaceMesh(CMeshWrapper* newMesh, bool keepMeshAttributes);
    CMeshWrapper* detachMesh();
    bool computeMassAndInertia(double density);

    void invertFrontBack();

    int getMeshModificationCounter();
    CMeshWrapper* getMesh() const;
    CMesh* getSingleMesh() const;
    CMesh* getMeshFromUid(long long int meshUid, const C7Vector& parentCumulTr, C7Vector& shapeRelTr) const;
    void appendMeshes(std::vector<CMesh*>& meshes) const;

    void* _meshCalculationStructure;
    int _meshModificationCounter;

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    bool scaleObjectNonIsometrically(double x, double y, double z) override;
    void serialize(CSer& ar) override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;
    void setIsInScene(bool s) override;

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setStringProperty(const char* pName, const char* pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setVector2Property(const char* pName, const double* pState) override;
    int getVector2Property(const char* pName, double* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setQuaternionProperty(const char* pName, const C4Vector& pState) override;
    int getQuaternionProperty(const char* pName, C4Vector& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int setIntArrayProperty(const char* pName, const int* v, int vL) override;
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    // Various
    void copyAttributesTo(CShape* target);
    void commonInit();
    int getComponentCount() const;
    bool getCulling() const;
    void setCulling(bool culState);
    bool getVisibleEdges() const;
    void setVisibleEdges(bool v);
    double getShadingAngle() const;
    void setShadingAngle(double a);
    void setRespondableSuspendCount(int cnt);
    void decrementRespondableSuspendCount();

    bool getHideEdgeBorders_OLD() const;
    void setHideEdgeBorders_OLD(bool v);
    int getEdgeWidth_DEPRECATED() const;
    void setEdgeWidth_DEPRECATED(int w);
    bool getShapeWireframe_OLD() const;
    void setShapeWireframe_OLD(bool w);
    CDynMaterialObject* getDynMaterial();
    void setDynMaterial(CDynMaterialObject* mat);

    void display_extRenderer(CViewableBase* renderingObject, int displayAttrib);

    // Overridden from CSceneObject:
    bool setParent(CSceneObject* newParent) override;

    void prepareVerticesIndicesNormalsAndEdgesForSerialization();

    bool getStartInDynamicSleeping() const;
    void setStartInDynamicSleeping(bool sleeping);
    bool getStatic() const;
    void setStatic(bool sta);
    bool getDynKinematic() const;
    void setDynKinematic(bool kin);

    C3Vector getInitialDynamicLinearVelocity();
    void setInitialDynamicLinearVelocity(const C3Vector& vel);
    C3Vector getInitialDynamicAngularVelocity();
    void setInitialDynamicAngularVelocity(const C3Vector& vel);
    bool isCompound() const;
    void setColor(const char* colorName, int colorComponent, float r, float g, float b);
    void setColor(const char* colorName, int colorComponent, const float* rgbData);
    bool getColor(const char* colorName, int colorComponent, float* rgbData);
    void setRespondable(bool r);
    bool getRespondable();
    void setRespondableMask(int m);
    int getRespondableMask();

    bool getSetAutomaticallyToNonStaticIfGetsParent();
    void setSetAutomaticallyToNonStaticIfGetsParent(bool autoNonStatic);

    void setDynamicVelocity(const C3Vector& linearV, const C3Vector& angularV);
    C3Vector getDynamicLinearVelocity();
    C3Vector getDynamicAngularVelocity();
    void addAdditionalForceAndTorque(const C3Vector& f, const C3Vector& t);
    void clearAdditionalForce();
    void clearAdditionalTorque();
    void clearAdditionalForceAndTorque();
    C3Vector getAdditionalForce();
    C3Vector getAdditionalTorque();

    CSceneObject* getLastParentForLocalGlobalRespondable();
    void clearLastParentForLocalGlobalRespondable();

    // Distance measurement functions
    bool getShapeShapeDistance_IfSmaller(CShape* it, double& dist, double ray[7], int buffer[2]);
    bool getDistanceToDummy_IfSmaller(CDummy* it, double& dist, double ray[7], int& buffer);

    // Collision detection functions
    bool isMeshCalculationStructureInitialized();
    void initializeMeshCalculationStructureIfNeeded();
    void removeMeshCalculationStructure();
    bool doesShapeCollideWithShape(CShape* collidee, std::vector<double>* intersections);

    bool relocateFrame(const char* mode, const C7Vector* tr = nullptr);
    bool alignBB(const char* mode, const C7Vector* tr = nullptr);
    C7Vector getCumulCenteredMeshFrame() const;

    void setInsideAndOutsideFacesSameColor_DEPRECATED(bool s);
    bool getInsideAndOutsideFacesSameColor_DEPRECATED();

    void actualizeContainsTransparentComponent();
    bool getContainsTransparentComponent();

    void setRigidBodyWasAlreadyPutToSleepOnce(bool s);
    bool getRigidBodyWasAlreadyPutToSleepOnce();

  protected:
    CMeshWrapper* _mesh;
    void _serializeMesh(CSer& ar);

    int _respondableMask;
    CSceneObject* _lastParentForLocalGlobalRespondable;

    // Variables which need to be serialized
    bool _startInDynamicSleeping;
    bool _shapeIsDynamicallyStatic;
    bool _shapeIsDynamicallyKinematic; // for static shapes that move (used by e.g. Mujoco)
    bool _shapeIsDynamicallyRespondable;
    bool _containsTransparentComponents; // to be able to order shapes according to transparency
    bool _setAutomaticallyToNonStaticIfGetsParent;
    C3Vector _initialDynamicLinearVelocity;
    C3Vector _initialDynamicAngularVelocity;
    CDynMaterialObject* _dynMaterial;

    // other variables:
    C3Vector _dynamicLinearVelocity;
    C3Vector _dynamicAngularVelocity;
    C3Vector _additionalForce;
    C3Vector _additionalTorque;
    int _respondableSuspendCount;
    std::vector<int> _meshIds; // for mesh delete events

    bool _rigidBodyWasAlreadyPutToSleepOnce;

    C3Vector _initialInitialDynamicLinearVelocity;
    C3Vector _initialInitialDynamicAngularVelocity;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
    void displayInertia(CViewableBase* renderingObject, double size, bool persp);
    void displayFrames(CViewableBase* renderingObject, double size, bool persp) override;
#endif
};
