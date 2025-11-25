#pragma once

#include <colorObject.h>
#include <simMath/4X4Matrix.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                       \
FUNCX(propDrawingObj_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object type", "description": ""})", "")                \
FUNCX(propDrawingObj_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Object meta information", "description": ""})", "") \
FUNCX(propDrawingObj_handle, "handle", sim_propertytype_handle, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"({"label": "Handle", "description": "", "handleType": ""})", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
    DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    const std::vector<SProperty> allProps_drawingObj = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CSceneObject;

class CDrawingObject
{
  public:
    CDrawingObject(int theObjectType, double size, double duplicateTolerance, int sceneObjId, int maxItemCount, int creatorHandle);
    virtual ~CDrawingObject();

    void setObjectId(int newId);
    int getObjectId() const;
    bool addItem(const double* itemData);
    void addItems(const double* itemData, size_t itemCnt);
    void setItems(const double* itemData, size_t itemCnt);
    int getObjectType() const;
    bool announceObjectWillBeErased(const CSceneObject* object);
    bool announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);

    int getHandleProperty(const char* pName, long long int& pState) const;
    int getStringProperty(const char* pName, std::string& pState) const;
    static int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    static int getPropertyInfo(const char* pName, int& info, std::string& infoTxt);

    void pushAddEvent();
    void pushAppendNewPointEvent();

    int getSceneObjectId() const;

    double getSize() const;
    int getMaxItemCount() const;
    int getStartItem() const;
    int getExpectedFloatsPerItem() const;

    std::vector<double>* getDataPtr();

#ifdef SIM_WITH_GUI
    void draw(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix& cameraCTM);
#endif

    CColorObject color;

    int verticesPerItem;
    int quaternionsPerItem;
    int colorsPerItem;
    int otherFloatsPerItem; // sizes and/or transparency
    int floatsPerItem;

  protected:
    void _initBufferedEventData();
    void _getEventData(std::vector<float>& vertices, std::vector<float>& quaternions, std::vector<float>& colors) const;

    void _setItemSizes();

    int _objectId;
    int _sceneObjectId;
    long long int _sceneObjectUid;
    int _objectType;
    double _size;
    int _maxItemCount;
    int _startItem;
    double _duplicateTolerance;
    int _creatorHandle;
    bool _rebuildRemoteItems;

    std::vector<double> _data;
    std::vector<double> _bufferedEventData;
};
