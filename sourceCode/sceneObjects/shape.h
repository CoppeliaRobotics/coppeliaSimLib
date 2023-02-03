#pragma once

#include <sceneObject.h>
#include <mesh.h>
#include <dummy.h>

class CShape : public CSceneObject  
{
public:

    CShape();
    CShape(const std::vector<double>& allHeights,int xSize,int ySize,double dx,double zSize); // heightfield
    CShape(const C7Vector* transformation,const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* normals,const std::vector<double>* textCoord); // mesh
    CShape(const C7Vector& transformation,CMeshWrapper* newGeomInfo);
    virtual ~CShape();

    C7Vector reinitMesh(const C7Vector* transformation,const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* normals,const std::vector<double>* textCoord);
    C7Vector reinitMesh2(const C7Vector& transformation,CMeshWrapper* newGeomInfo);
    void setNewMesh(CMeshWrapper* newGeomInfo);
    void invertFrontBack();
    C3Vector getBoundingBoxHalfSizes() const;
    void scaleMesh(double xVal,double yVal,double zVal);
    void scaleMesh(double x,double y,double z,double& xp,double& yp,double& zp);

    int getMeshModificationCounter();
    CMeshWrapper* getMeshWrapper() const;
    CMesh* getSingleMesh() const;
    void disconnectMesh();

    void* _meshCalculationStructure;
    C3Vector _meshBoundingBoxHalfSizes;
    int _meshModificationCounter;
    CMeshWrapper* _mesh;


    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void scaleObjectNonIsometrically(double x,double y,double z);
    void serialize(CSer& ar);
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    // Various
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

    void display_extRenderer(CViewableBase* renderingObject,int displayAttrib);

    // Overridden from CSceneObject:
    bool setParent(CSceneObject* newParent);


    void prepareVerticesIndicesNormalsAndEdgesForSerialization();

    bool getStartInDynamicSleeping() const;
    void setStartInDynamicSleeping(bool sleeping);
    bool getShapeIsDynamicallyStatic() const;
    void setShapeIsDynamicallyStatic(bool sta);
    bool getShapeIsDynamicallyKinematic() const;
    void setShapeIsDynamicallyKinematic(bool kin);

    C3Vector getInitialDynamicLinearVelocity();
    void setInitialDynamicLinearVelocity(const C3Vector& vel);
    C3Vector getInitialDynamicAngularVelocity();
    void setInitialDynamicAngularVelocity(const C3Vector& vel);
    bool isCompound() const;
    void setColor(const char* colorName,int colorComponent,float r,float g,float b);
    void setColor(const char* colorName,int colorComponent,const float* rgbData);
    bool getColor(const char* colorName,int colorComponent,float* rgbData);
    void setRespondable(bool r);
    bool getRespondable();
    void setDynamicCollisionMask(unsigned short m);
    unsigned short getDynamicCollisionMask();

    bool getSetAutomaticallyToNonStaticIfGetsParent();
    void setSetAutomaticallyToNonStaticIfGetsParent(bool autoNonStatic);



    void setDynamicVelocity(const C3Vector& linearV,const C3Vector& angularV);
    C3Vector getDynamicLinearVelocity();
    C3Vector getDynamicAngularVelocity();
    void addAdditionalForceAndTorque(const C3Vector& f,const C3Vector& t);
    void clearAdditionalForce();
    void clearAdditionalTorque();
    void clearAdditionalForceAndTorque();
    C3Vector getAdditionalForce();
    C3Vector getAdditionalTorque();

    CSceneObject* getLastParentForLocalGlobalRespondable();
    void clearLastParentForLocalGlobalRespondable();

    // Distance measurement functions
    bool getShapeShapeDistance_IfSmaller(CShape* it,double &dist,double ray[7],int buffer[2]);
    bool getDistanceToDummy_IfSmaller(CDummy* it,double &dist,double ray[7],int& buffer);

    // Collision detection functions
    bool isMeshCalculationStructureInitialized();
    void initializeMeshCalculationStructureIfNeeded();
    void removeMeshCalculationStructure();
    bool doesShapeCollideWithShape(CShape* collidee,std::vector<double>* intersections);

    // Bounding box functions
    void alignBoundingBoxWithMainAxis();
    void alignBoundingBoxWithWorld();
    bool alignTubeBoundingBoxWithMainAxis();
    bool alignCuboidBoundingBoxWithMainAxis();

    void setInsideAndOutsideFacesSameColor_DEPRECATED(bool s);
    bool getInsideAndOutsideFacesSameColor_DEPRECATED();

    void actualizeContainsTransparentComponent();
    bool getContainsTransparentComponent();

    void setRigidBodyWasAlreadyPutToSleepOnce(bool s);
    bool getRigidBodyWasAlreadyPutToSleepOnce();

    static bool getDebugObbStructures();
    static void setDebugObbStructures(bool d);

protected:
    void _serializeBackCompatibility(CSer& ar);
    C7Vector _acceptNewGeometry(const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* textCoord,const std::vector<double>* norm);
    C7Vector _recomputeOrientation(C7Vector& m,bool alignWithMainAxis);
    C7Vector _recomputeTubeOrCuboidOrientation(C7Vector& m,bool tube,bool& error);
    static bool _getTubeReferenceFrame(const std::vector<double>& v,C7Vector& tr);
    static bool _getCuboidReferenceFrame(const std::vector<double>& v,const std::vector<int>& ind,C7Vector& tr);
    void _computeMeshBoundingBox();

    bool _reorientGeometry(int type); // 0=main axis, 1=world, 2=tube, 3=cuboid

    unsigned short _dynamicCollisionMask;
    CSceneObject* _lastParentForLocalGlobalRespondable;

    // Variables which need to be serialized
    bool _startInDynamicSleeping;
    bool _shapeIsDynamicallyStatic;
    bool _shapeIsDynamicallyKinematic; // for static shapes that move (used by e.g. Mujoco)
    bool _shapeIsDynamicallyRespondable;
    bool _parentFollowsDynamic;
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

    bool _rigidBodyWasAlreadyPutToSleepOnce;

    C3Vector _initialInitialDynamicLinearVelocity;
    C3Vector _initialInitialDynamicAngularVelocity;
    static bool _visualizeObbStructures;

};
