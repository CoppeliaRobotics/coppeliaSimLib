#pragma once

#include <colorObject.h>
#include <simMath/4X4Matrix.h>
#include <vMutex.h>

class CSceneObject;

class CDrawingObject  
{
public:
    CDrawingObject(int theObjectType,double size,double duplicateTolerance,int sceneObjId,int maxItemCount,int creatorHandle);
    virtual ~CDrawingObject();

    void draw(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    void setObjectId(int newId);
    void setObjectUniqueId();
    long long int getObjectUid() const;
    int getObjectId() const;
    bool addItem(const double* itemData);
    void addItems(const double* itemData,size_t itemCnt);
    void setItems(const double* itemData,size_t itemCnt);
    int getObjectType() const;
    bool announceObjectWillBeErased(const CSceneObject* object);
    bool announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void adjustForFrameChange(const C7Vector& preCorrection);
    void adjustForScaling(double xScale,double yScale,double zScale);

    void pushAddEvent();
    void pushAppendNewPointEvent();

    int getSceneObjectId() const;

    double getSize() const;
    int getMaxItemCount() const;
    int getStartItem() const;
    int getExpectedFloatsPerItem() const;

    std::vector<double>* getDataPtr();

    CColorObject color;

    int verticesPerItem;
    int quaternionsPerItem;
    int colorsPerItem;
    int otherFloatsPerItem; // sizes and/or transparency
    int floatsPerItem;

protected:
    void _initBufferedEventData();
    void _getEventData(std::vector<float>& vertices,std::vector<float>& quaternions,std::vector<float>& colors) const;

    void _setItemSizes();

    int _objectId;
    long long int _objectUid;
    int _sceneObjectId;
    long long int _sceneObjectUid;
    int _objectType;
    double _size;
    int _maxItemCount;
    int _startItem;
    double _duplicateTolerance;
    int _creatorHandle;
    bool _rebuildRemoteItems;

    VMutex _objectMutex;

    std::vector<double> _data;
    std::vector<double> _bufferedEventData;
};
