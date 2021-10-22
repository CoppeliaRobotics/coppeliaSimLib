#pragma once

#include "syncObject.h"
#include "MyMath.h"
#include <string>
#include "interfaceStack.h"

class CSceneObject;

enum {
    sim_syncobj_sceneobject_cmdstart=128,
    sim_syncobj_sceneobject_create,
    sim_syncobj_sceneobject_delete,
    sim_syncobj_sceneobject_setextensionstring,
    sim_syncobj_sceneobject_setvisibilitylayer,
    sim_syncobj_sceneobject_setalias,
    sim_syncobj_sceneobject_setname,
    sim_syncobj_sceneobject_setaltname,
    sim_syncobj_sceneobject_localtransf,
    sim_syncobj_sceneobject_setparent,
    sim_syncobj_sceneobject_setchildorder,
};

#define EVENTTYPE_OBJECTADDED "objectAdded"
#define EVENTTYPE_OBJECTCHANGED "objectChanged"
#define EVENTTYPE_OBJECTREMOVED "objectRemoved"

class _CSceneObject_ : public CSyncObject
{
public:
    _CSceneObject_();
    virtual ~_CSceneObject_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getObjectType() const;
    CSceneObject* getParent() const;
    int getObjectHandle() const;
    int getObjectUniqueId() const;
    bool getSelected() const;
    bool getIsInScene() const;
    bool getModelBase() const;
    bool getModelInvisible() const;
    std::string getExtensionString() const;
    unsigned short getVisibilityLayer() const;
    int getChildOrder() const;
    int getHierarchyTreeObjects(std::vector<CSceneObject*>& allObjects);
    std::string getObjectAlias() const;
    std::string getObjectAliasAndOrderIfRequired() const;
    std::string getObjectAliasAndHandle() const;
    std::string getObjectAlias_fullPath() const;
    std::string getObjectAlias_shortPath() const;
    std::string getObjectAlias_printPath() const;
    std::string getObjectName_old() const;
    std::string getObjectAltName_old() const;

    C7Vector getLocalTransformation() const;
    virtual C7Vector getFullLocalTransformation() const;
    C7Vector getFullParentCumulativeTransformation() const;
    C7Vector getCumulativeTransformation() const;
    C7Vector getFullCumulativeTransformation() const;

    void recomputeModelInfluencedValues(int flags=-1);


    void setObjectUniqueId();
    void setSelected(bool s); // doesn't generate a sync msg
    void setIsInScene(bool s);

    virtual bool setObjectHandle(int newObjectHandle);

    void setParentPtr(CSceneObject* parent);
    virtual bool setParent(CSceneObject* parent);
    virtual bool setChildOrder(int order);
    virtual bool setExtensionString(const char* str);
    virtual bool setVisibilityLayer(unsigned short l);
    virtual bool setObjectAlias_direct(const char* newAlias);
    virtual bool setObjectName_direct_old(const char* newName);
    virtual bool setObjectAltName_direct_old(const char* newAltName);

    virtual bool setLocalTransformation(const C7Vector& tr);
    virtual bool setLocalTransformation(const C4Vector& q);
    virtual bool setLocalTransformation(const C3Vector& x);

protected:
    void _setModelInvisible(bool inv);

    virtual void _setChildOrder_send(int order) const;
    virtual void _setParent_send(int parentHandle) const;
    virtual void _setExtensionString_send(const char* str) const;
    virtual void _setVisibilityLayer_send(unsigned short l) const;
    virtual void _setObjectAlias_send(const char* newName) const;
    virtual void _setObjectName_send(const char* newName) const;
    virtual void _setObjectAltName_send(const char* newAltName) const;
    virtual void _setLocalTransformation_send(const C7Vector& tr) const;


    int _objectHandle;
    int _objectUniqueId; // valid for a given session (non-persistent)
    std::string _extensionString;
    unsigned short _visibilityLayer;
    bool _selected;
    bool _isInScene;
    bool _modelInvisible;
    int _childOrder;
    std::string _objectAlias;
    C7Vector _localTransformation;

    std::vector<CSceneObject*> _childList;
    C7Vector _assemblingLocalTransformation; // When assembling this object
    bool _assemblingLocalTransformationIsUsed;
    std::vector<std::string> _assemblyMatchValuesChild;
    std::vector<std::string> _assemblyMatchValuesParent;
    // Keep parent stored as pointer. Very important!!! If we would use a simple
    // parent identifier, an object and its copy in the copy buffer would have
    // the exact same parent (which would be retrieved via 'CObjCont::getObject()')
    CSceneObject* _parentObject;
    int _objectType;
    int _objectProperty;
    bool _modelBase;
    bool _ignoredByViewFitting;
    int _hierarchyColorIndex;
    int _collectionSelfCollisionIndicator;
    int _localObjectSpecialProperty;
    int _modelProperty;
    int _calculatedModelProperty;
    int _calculatedObjectProperty;
    std::string _modelAcknowledgement;

    static int _objectUniqueIDCounter;

    // Old:
    std::string _objectName_old;
    std::string _objectAltName_old;
};
