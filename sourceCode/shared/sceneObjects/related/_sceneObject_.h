#pragma once

#include "syncObject.h"
#include "MyMath.h"

class CSceneObject;

enum {
    sim_syncobj_sceneobject_cmdstart=128,
    sim_syncobj_sceneobject_create,
    sim_syncobj_sceneobject_delete,
    sim_syncobj_sceneobject_setextensionstring,
    sim_syncobj_sceneobject_setvisibilitylayer,
    sim_syncobj_sceneobject_setname,
    sim_syncobj_sceneobject_setaltname,
    sim_syncobj_sceneobject_localtransf,
    sim_syncobj_sceneobject_setparent,
};

class _CSceneObject_ : public CSyncObject
{
public:
    _CSceneObject_();
    virtual ~_CSceneObject_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    CSceneObject* getParent() const;
    int getObjectHandle() const;
    bool getSelected() const;
    std::string getExtensionString() const;
    unsigned short getVisibilityLayer() const;
    std::string getObjectName() const;
    std::string getObjectAltName() const;

    C7Vector getLocalTransformation() const;
    virtual C7Vector getFullLocalTransformation() const;
    C7Vector getFullParentCumulativeTransformation() const;
    C7Vector getCumulativeTransformation() const;
    C7Vector getFullCumulativeTransformation() const;


    void setSelected(bool s); // doesn't generate a sync msg

    virtual bool setObjectHandle(int newObjectHandle);

    virtual bool setParent(CSceneObject* parent,bool keepObjectInPlace);
    virtual bool setExtensionString(const char* str);
    virtual bool setVisibilityLayer(unsigned short l);
    virtual bool setObjectName(const char* newName,bool check);
    virtual bool setObjectAltName(const char* newAltName,bool check);

    virtual bool setLocalTransformation(const C7Vector& tr);
    virtual bool setLocalTransformation(const C4Vector& q);
    virtual bool setLocalTransformation(const C3Vector& x);

protected:
    virtual void _setParent_send(int parentHandle,bool keepObjectInPlace) const;
    virtual void _setExtensionString_send(const char* str) const;
    virtual void _setVisibilityLayer_send(unsigned short l) const;
    virtual void _setObjectName_send(const char* newName) const;
    virtual void _setObjectAltName_send(const char* newAltName) const;
    virtual void _setLocalTransformation_send(const C7Vector& tr) const;


    int _objectHandle;
    std::string _extensionString;
    unsigned short _visibilityLayer;
    bool _selected;
    std::string _objectName;
    std::string _objectAltName;
    C7Vector _localTransformation;

    C7Vector _assemblingLocalTransformation; // When assembling this object
    bool _assemblingLocalTransformationIsUsed;
    std::vector<std::string> _assemblyMatchValuesChild;
    std::vector<std::string> _assemblyMatchValuesParent;
    // Keep parent stored as pointer. Very important!!! If we would use a simple
    // parent identifier, an object and its copy in the copy buffer would have
    // the exact same parent (which would be retrieved via 'CObjCont::getObject()')
    CSceneObject* _parentObject;
    int _objectType;
    int _localObjectProperty;
    bool _modelBase;
    bool _ignoredByViewFitting;
    int _hierarchyColorIndex;
    int _collectionSelfCollisionIndicator;
    int _localObjectSpecialProperty;
    int _localModelProperty;
    std::string _modelAcknowledgement;

};
