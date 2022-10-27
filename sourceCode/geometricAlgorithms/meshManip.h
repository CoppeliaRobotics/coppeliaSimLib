#pragma once

#include "edgeElement.h"
#include "MyMath.h"

class CMeshManip  
{
public:
    CMeshManip(float* vertices,int verticesNb,int* indices,int indicesNb);
    virtual ~CMeshManip();

    std::vector<CEdgeElement*> edges;
    std::vector<C3Vector> faceNormals;

    // Various static functions (some of them are redundant or very old!!):
    static bool extractOneShape(std::vector<float>* vertices,std::vector<int>* indices,std::vector<float>* sVertices,std::vector<int>* sIndices);
    static void useOnlyReferencedVertices(std::vector<float>* vertices,std::vector<float>* sVertices,std::vector<int>* sIndices);
    static void removeNonReferencedVertices(std::vector<float>& vertices,std::vector<int>& indices);
    static bool checkVerticesIndicesNormalsTexCoords(std::vector<float>& vertices,std::vector<int>& indices,std::vector<float>* normals,std::vector<float>* texCoords,bool checkDoubles,float tolerance,bool checkSameTriangles);

    static int removeColinearTriangles(std::vector<float>& vertices,std::vector<int>& indices,std::vector<float>* normals,std::vector<float>* texCoords,float tolerance);
    static void removeDoubleVertices(std::vector<float>& vertices,std::vector<int>& mapping,float tolerance);
    static void removeDoubleIndices(std::vector<float>& vertices,std::vector<int>& indices,bool checkSameWinding);
    static bool reduceTriangleSize(std::vector<float>& vertices,std::vector<int>& indices,std::vector<float>* normals,std::vector<float>* texCoords,float maxEdgeSize,float verticeMergeTolerance);
    static bool correctTriangleWinding(std::vector<float>* vertices,std::vector<int>* indices);
    static bool isInsideOut(std::vector<float>* vertices,std::vector<int>* indices);
    static void setInsideOut(std::vector<int>* indices);
    static void getProjectionOfPointOnLine(float x,float y,float z,float vx,float vy,float vz,float &px,float &py,float &pz);
    static bool mergeWith(std::vector<float>* tVertices,std::vector<int>* tIndices,std::vector<float>* tNormals,
                    std::vector<float>* sVertices,std::vector<int>* sIndices,std::vector<float>* sNormals); static void reduceToUnit(float vector[3]);
    static void calcNormal(float v[3][3],float out[3]);
    static void getSize(std::vector<float>* vertices,float x[2],float y[2],float z[2],bool &start);
    static void centerAndScale(std::vector<float>* vertices,float x,float y,float z,float sf);
    static void getNormals(const std::vector <float>* vertices,const std::vector<int>* indices,std::vector<float>* normals);
    static void getTrianglesFromPolygons(const std::vector<std::vector<int> >& polygons,std::vector<int>& indices);

private:
    static void _setExtractionExploration(std::vector<unsigned char>* exploration,int index,unsigned char bit,int &allBits,int &twoBits);
    static int _reduceTriangleSizePass(std::vector<float>& vertices,std::vector<int>& indices,std::vector<float>* normals,std::vector<float>* texCoords,float maxEdgeSize);
    static int _getNeighbour(int actualTriangle,std::vector<int>* indices,int actualEdge[2],std::vector<std::vector<int>*>* edges,std::vector<unsigned char>* exploredState);
};
