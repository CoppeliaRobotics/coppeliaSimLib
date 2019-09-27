
#pragma once

#include "geomWrap.h"
#include "3Vector.h"
#include "7Vector.h"

class CGeomProxy   
{
public:
    CGeomProxy();
    virtual ~CGeomProxy();
    CGeomProxy(const std::vector<float>& allHeights,int xSize,int ySize,float dx,float zSize); // For heightfield creation
    CGeomProxy(const C7Vector* transformation,const std::vector<float>& vert,const std::vector<int>& ind,const std::vector<float>* normals,const std::vector<float>* textCoord);
    CGeomProxy(const C7Vector& transformation,CGeomWrap* newGeomInfo);

    void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    void perform3DObjectLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void announce3DObjectWillBeErased(int objectID);
    void removeCollisionInformation();
    C3Vector getBoundingBoxHalfSizes();
    void invertFrontBack();
    void initializeCalculationStructureIfNeeded();
    bool isCollisionInformationInitialized();
    void computeBoundingBox();
    void scale(float xVal,float yVal,float zVal);
    void scale(float x,float y,float z,float& xp,float& yp,float& zp);
    void setDynamicsFullRefreshFlag(bool refresh);
    bool getDynamicsFullRefreshFlag();
    int getGeomDataModificationCounter();
    void setTextureDependencies(int shapeID);
    C7Vector recomputeOrientation(C7Vector& m,bool alignWithMainAxis);
    C7Vector recomputeTubeOrCuboidOrientation(C7Vector& m,bool tube,bool& error);
    CGeomProxy* copyYourself();
    void serialize(CSer& ar,const char* shapeName);
    C7Vector getCreationTransformation();
    void setCreationTransformation(const C7Vector& tr);
    bool applyCuttingChanges(const C7Vector& shapeCTM);
    void acceptNewGeometry(const std::vector<float>& vert,const std::vector<int>& ind,const std::vector<float>* textCoord,const std::vector<float>* norm);

    CGeomWrap* geomInfo;

    void* collInfo;

protected:
    void _commonInit();
    static bool _getTubeReferenceFrame(const std::vector<float>& v,C7Vector& tr);
    static bool _getCuboidReferenceFrame(const std::vector<float>& v,const std::vector<int>& ind,C7Vector& tr);

    C7Vector _creationTransf;
    C3Vector _boundingBoxHalfSizes;

    bool _dynamicsFullRefreshFlag;
    int _geomDataModificationCounter;
};
