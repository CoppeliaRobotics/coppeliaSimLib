#pragma once

#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <sceneObject.h>

enum
{ /* Mujoco dummy double params */
    simi_mujoco_dummy_range1 = 0,
    simi_mujoco_dummy_range2,
    simi_mujoco_dummy_solreflimit1,
    simi_mujoco_dummy_solreflimit2,
    simi_mujoco_dummy_solimplimit1,
    simi_mujoco_dummy_solimplimit2,
    simi_mujoco_dummy_solimplimit3,
    simi_mujoco_dummy_solimplimit4,
    simi_mujoco_dummy_solimplimit5,
    simi_mujoco_dummy_margin,
    simi_mujoco_dummy_springlength,
    simi_mujoco_dummy_stiffness,
    simi_mujoco_dummy_damping,
};

enum
{ /* Mujoco dummy int params */
    simi_mujoco_dummy_bitcoded = 0,
    simi_mujoco_dummy_proxyjointid,
};

enum
{ /* Mujoco dummy bool params */
    simi_mujoco_dummy_limited = 1,
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propDummy_size,                    "size",                                     sim_propertytype_float,     0) \
    FUNCX(propDummy_colDiffuse,              "diffuseColor",                             sim_propertytype_color,     0) \
    FUNCX(propDummy_colSpecular,             "specularColor",                            sim_propertytype_color,     0) \
    FUNCX(propDummy_colEmission,             "emissionColor",                            sim_propertytype_color,     0) \

#define FUNCX(name, str, v1, v2) const CProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<CProperty> allProps_dummy = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
#undef CONCAT_PROP
// ----------------------------------------------------------------------------------------------


class CDummy : public CSceneObject
{
  public:
    CDummy();
    virtual ~CDummy();

    void buildOrUpdate_oldIk();
    void connect_oldIk();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor *ev) const;
    CSceneObject *copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void serialize(CSer &ar);
    void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    void performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int, int> *map);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int> *map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    void announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState);
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, int& size);

    bool getFreeOnPathTrajectory() const;
    double getVirtualDistanceOffsetOnPath() const;
    double getVirtualDistanceOffsetOnPath_variationWhenCopy() const;
    std::string getLinkedDummyLoadAlias() const;
    std::string getLinkedDummyLoadName_old() const;
    double getDummySize() const;
    bool getAssignedToParentPath() const;
    bool getAssignedToParentPathOrientation() const;
    int getLinkedDummyHandle() const;
    int getDummyType() const;
    std::string getAssemblyTag() const;

    CColorObject *getDummyColor();
    void setDummyColor(const float* col, int colComp);
    void loadUnknownObjectType(CSer &ar);

    bool setAssignedToParentPath(bool assigned);
    bool setAssignedToParentPathOrientation(bool assigned);
    void setLinkedDummyHandle(int handle, bool check);
    bool setDummyType(int lt, bool check);
    void setDummySize(double s);
    void setAssemblyTag(const char *tag);
    void setFreeOnPathTrajectory(bool isFree);
    void setVirtualDistanceOffsetOnPath(double off);
    void setVirtualDistanceOffsetOnPath_variationWhenCopy(double off);

    double getEngineFloatParam(int what, bool *ok) const;
    int getEngineIntParam(int what, bool *ok) const;
    bool getEngineBoolParam(int what, bool *ok) const;
    bool setEngineFloatParam(int what, double v);
    bool setEngineIntParam(int what, int v);
    bool setEngineBoolParam(int what, bool v);

    void copyEnginePropertiesTo(CDummy *target);

  protected:
    void getMujocoFloatParams(std::vector<double> &p) const;
    void getMujocoIntParams(std::vector<int> &p) const;
    void setMujocoFloatParams(const std::vector<double> &p, bool reflectToLinkedDummy = true);
    void setMujocoIntParams(const std::vector<int> &p, bool reflectToLinkedDummy = true);

    void _reflectPropToLinkedDummy() const;
    void _setLinkedDummyHandle_sendOldIk(int h) const;
    void _setLinkType_sendOldIk(int t) const;

    bool _freeOnPathTrajectory;
    double _virtualDistanceOffsetOnPath_OLD;
    double _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
    std::string _linkedDummyLoadAlias;
    std::string _linkedDummyLoadName_old;

    CColorObject _dummyColor;
    double _dummySize;
    int _linkedDummyHandle;
    int _linkType;
    std::string _assemblyTag;
    bool _assignedToParentPath;
    bool _assignedToParentPathOrientation;

    std::vector<double> _mujocoFloatParams;
    std::vector<int> _mujocoIntParams;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
#endif
};
