
#pragma once

#include "colorObject.h"
#include "4X4Matrix.h"
#include "vMutex.h"

class CDrawingObject  
{
public:
    CDrawingObject(int theObjectType,float size,float duplicateTolerance,int sceneObjID,int maxItemCount,int creatorHandle);
    virtual ~CDrawingObject();

    void draw(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    void setObjectID(int newID);
    int getObjectID() const;
    bool addItem(const float* itemData);
    int getObjectType() const;
    bool announceObjectWillBeErased(int objID);
    bool announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void adjustForFrameChange(const C7Vector& preCorrection);
    void adjustForScaling(float xScale,float yScale,float zScale);

    bool canMeshBeExported() const;
    void getExportableMesh(std::vector<float>& vertices,std::vector<int>& indices) const;

    int getSceneObjectID() const;

    float getSize() const;
    int getMaxItemCount() const;
    int getStartItem() const;

    std::vector<float>* getDataPtr();

    CColorObject color;

    int verticesPerItem;
    int normalsPerItem;
    int otherFloatsPerItem;
    int floatsPerItem;

protected:
    void _exportTrianglePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportQuadPoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportDiscPoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportCubePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportSpherePoints(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportTriangles(C7Vector& tr,std::vector<float>& vertices,std::vector<int>& indices) const;
    void _exportTriOrQuad(C7Vector& tr,C3Vector* v0,C3Vector* v1,C3Vector* v2,C3Vector* v3,std::vector<float>& vertices,std::vector<int>& indices,int& nextIndex) const;

    void _setItemSizes();

    int _objectID;
    int _sceneObjectID;
    int _objectType;
    float _size;
    int _maxItemCount;
    int _startItem;
    float _duplicateTolerance;
    int _creatorHandle;

    VMutex _objectMutex;

    std::vector<float> _data;
};
