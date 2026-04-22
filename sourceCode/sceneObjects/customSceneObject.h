#pragma once

#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <sceneObject.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
CUSTOMSCENEOBJECT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_customSceneObject;
// ----------------------------------------------------------------------------------------------

class CCustomSceneObject : public CSceneObject
{
  public:
    CCustomSceneObject();
    virtual ~CCustomSceneObject();

    // Following functions are inherited from CSceneObject
    void addObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void setIsInScene(bool s) override;

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setStringProperty(const char* pName, const std::string& pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;

    double getObjectSize() const;

    CColorObject* getObjectColor();

    void setObjectSize(double s);

  protected:
    CColorObject _objectColor;
    double _objectSize;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
