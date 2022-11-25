#pragma once

#include "colorObject.h"
#include "4X4Matrix.h"
#include "vMutex.h"

class CSceneObject;

class CDrawingObject  
{
public:
    CDrawingObject(int theObjectType,floatDouble size,floatDouble duplicateTolerance,int sceneObjId,int maxItemCount,int creatorHandle);
    virtual ~CDrawingObject();

    void draw(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    void setObjectId(int newId);
    void setObjectUniqueId();
    long long int getObjectUid() const;
    int getObjectId() const;
    bool addItem(const floatDouble* itemData);
    void addItems(const floatDouble* itemData,size_t itemCnt);
    void setItems(const floatDouble* itemData,size_t itemCnt);
    int getObjectType() const;
    bool announceObjectWillBeErased(const CSceneObject* object);
    bool announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void adjustForFrameChange(const C7Vector& preCorrection);
    void adjustForScaling(floatDouble xScale,floatDouble yScale,floatDouble zScale);

    void pushAddEvent();
    void pushAppendNewPointEvent();

    int getSceneObjectId() const;

    floatDouble getSize() const;
    int getMaxItemCount() const;
    int getStartItem() const;
    int getExpectedFloatsPerItem() const;

    std::vector<floatDouble>* getDataPtr();

    CColorObject color;

    int verticesPerItem;
    int quaternionsPerItem;
    int colorsPerItem;
    int otherFloatsPerItem; // sizes and/or transparency
    int floatsPerItem;

protected:
    void _initBufferedEventData();
    void _getEventData(std::vector<floatDouble>& vertices,std::vector<floatDouble>& quaternions,std::vector<floatDouble>& colors) const;

    void _setItemSizes();

    int _objectId;
    long long int _objectUid;
    int _sceneObjectId;
    long long int _sceneObjectUid;
    int _objectType;
    floatDouble _size;
    int _maxItemCount;
    int _startItem;
    floatDouble _duplicateTolerance;
    int _creatorHandle;
    bool _rebuildRemoteItems;

    VMutex _objectMutex;

    std::vector<floatDouble> _data;
    std::vector<floatDouble> _bufferedEventData;
};
