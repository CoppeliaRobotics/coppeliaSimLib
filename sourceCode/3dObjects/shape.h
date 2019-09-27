
#pragma once

#include "3DObject.h"
#include "dummy.h"
#include "geomProxy.h"

class CShape : public C3DObject  
{
public:

    CShape();
    virtual ~CShape();

    // Following functions are inherited from 3DObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    C3DObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer);
    void performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performGcsLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(std::vector<int>* map);
    void bufferMainDisplayStateVariables();
    void bufferedMainDisplayStateVariablesToDisplay();
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationIsRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;

    // Various
    void commonInit();
    bool getCulling();
    void setCulling(bool culState);
    bool getVisibleEdges();
    void setVisibleEdges(bool v);
    bool getHideEdgeBorders();
    void setHideEdgeBorders(bool v);
    int getEdgeWidth_DEPRECATED();
    void setEdgeWidth_DEPRECATED(int w);
    bool getShapeWireframe();
    void setShapeWireframe(bool w);
    CDynMaterialObject* getDynMaterial();
    void setDynMaterial(CDynMaterialObject* mat);

    void display_extRenderer(CViewableBase* renderingObject,int displayAttrib);

    void resetMilling();
    bool applyMilling();

    void prepareVerticesIndicesNormalsAndEdgesForSerialization();

    bool getStartInDynamicSleeping();
    void setStartInDynamicSleeping(bool sleeping);
    bool getShapeIsDynamicallyStatic();
    void setShapeIsDynamicallyStatic(bool sta);

    C3Vector getInitialDynamicLinearVelocity();
    void setInitialDynamicLinearVelocity(const C3Vector& vel);
    C3Vector getInitialDynamicAngularVelocity();
    void setInitialDynamicAngularVelocity(const C3Vector& vel);
    bool isCompound() const;
    void setColor(const char* colorName,int colorComponent,const float* rgbData);
    bool getColor(const char* colorName,int colorComponent,float* rgbData);
    void setRespondable(bool r);
    bool getRespondable();
    void setDynamicCollisionMask(unsigned short m);
    unsigned short getDynamicCollisionMask();
    void setParentFollowsDynamic(bool f);
    bool getParentFollowsDynamic();
    void setShapeIsStaticAndNotRespondableButDynamicTag(bool f);
    bool getShapeIsStaticAndNotRespondableButDynamicTag();

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

    C3DObject* getLastParentForLocalGlobalCollidable();
    void clearLastParentForLocalGlobalCollidable();

    // Distance measurement functions
    bool getShapeShapeDistance_IfSmaller(CShape* it,float &dist,float ray[7],int buffer[2]);
    bool getDistanceToDummy_IfSmaller(CDummy* it,float &dist,float ray[7],int& buffer);

    // Collision detection functions
    bool isCollisionInformationInitialized();
//  void initializeCollisionDetection();
    void initializeCalculationStructureIfNeeded();
    void removeCollisionInformation();
    bool doesShapeCollideWithShape(CShape* collidee,std::vector<float>* intersections);

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

    CGeomProxy* geomData;
protected:

    bool reorientGeometry(int type); // 0=main axis, 1=world, 2=tube, 3=cuboid

    unsigned short _dynamicCollisionMask;
    C3DObject* _lastParentForLocalGlobalCollidable;

    // Variables which need to be serialized
    bool _startInDynamicSleeping;
    bool _shapeIsDynamicallyStatic;
    bool _shapeIsDynamicallyRespondable;
    bool _shapeIsStaticAndNotRespondableButDynamicTag; // needed when a dynamic joint connects to a static non respondable shape
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

    bool _rigidBodyWasAlreadyPutToSleepOnce;

    bool _initialValuesInitialized;
    C3Vector _initialInitialDynamicLinearVelocity;
    C3Vector _initialInitialDynamicAngularVelocity;
    static bool _visualizeObbStructures;
};
