#pragma once

#include <drawingObject.h>
#include <simMath/7Vector.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                               \
FUNCX(propDrawCont_drawingObjects, "drawingObjects", sim_propertytype_handlearray, sim_propertyinfo_notwritable, R"({"label": "Drawing objects", "description": "Handles of all drawing objects"})", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    const std::vector<SProperty> allProps_drawCont = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CViewableBase;

class CDrawingContainer
{
  public:
    CDrawingContainer();
    virtual ~CDrawingContainer();

    void simulationEnded();
    void eraseAllObjects();
    int addObject(CDrawingObject* it);
    CDrawingObject* getObject(int objectId) const;
    void announceObjectWillBeErased(const CSceneObject* object);
    void announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void removeObject(int objectId);

    int getHandleProperty(long long int target, const char* pName, long long int& pState) const;
    int getStringProperty(long long int target, const char* pName, std::string& pState) const;
    int getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState) const;
    int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt) const;

    void pushGenesisEvents();
    void pushAppendNewPointEvents();

  private:
    void _publishAllDrawingObjectHandlesEvent() const;
    std::vector<CDrawingObject*> _allObjects;

#ifdef SIM_WITH_GUI
  public:
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject, int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject, int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject, int displayAttrib);
    void drawAll(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix& cameraCTM);
    void drawObjectsParentedWith(bool overlay, bool transparentObject, int parentObjectId, int displayAttrib, const C4X4Matrix& cameraCTM);
#endif
};
