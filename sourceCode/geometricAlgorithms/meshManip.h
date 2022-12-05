#pragma once

#include "edgeElement.h"
#include <vector>
#include "3Vector.h"

class CMeshManip  
{
public:
    CMeshManip(double* vertices,int verticesNb,int* indices,int indicesNb);
    virtual ~CMeshManip();

    std::vector<CEdgeElement*> edges;
    std::vector<C3Vector> faceNormals;

    // Various static functions (some of them are redundant or very old!!):
    static bool extractOneShape(std::vector<double>* vertices,std::vector<int>* indices,std::vector<double>* sVertices,std::vector<int>* sIndices);
    static void useOnlyReferencedVertices(std::vector<double>* vertices,std::vector<double>* sVertices,std::vector<int>* sIndices);
    static void removeNonReferencedVertices(std::vector<double>& vertices,std::vector<int>& indices);
    static bool checkVerticesIndicesNormalsTexCoords(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<double>* texCoords,bool checkDoubles,double tolerance,bool checkSameTriangles);

    static int removeColinearTriangles(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<double>* texCoords,double tolerance);
    static void removeDoubleVertices(std::vector<double>& vertices,std::vector<int>& mapping,double tolerance);
    static void removeDoubleIndices(std::vector<double>& vertices,std::vector<int>& indices,bool checkSameWinding);
    static bool reduceTriangleSize(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<double>* texCoords,double maxEdgeSize,double verticeMergeTolerance);
    static bool correctTriangleWinding(std::vector<double>* vertices,std::vector<int>* indices);
    static bool isInsideOut(std::vector<double>* vertices,std::vector<int>* indices);
    static void setInsideOut(std::vector<int>* indices);
    static void getProjectionOfPointOnLine(double x,double y,double z,double vx,double vy,double vz,double &px,double &py,double &pz);
    static bool mergeWith(std::vector<double>* tVertices,std::vector<int>* tIndices,std::vector<double>* tNormals,
                    std::vector<double>* sVertices,std::vector<int>* sIndices,std::vector<double>* sNormals);
    static void reduceToUnit(double vector[3]);
    static void calcNormal(double v[3][3],double out[3]);
    static void getSize(std::vector<double>* vertices,double x[2],double y[2],double z[2],bool &start);
    static void centerAndScale(std::vector<double>* vertices,double x,double y,double z,double sf);
    static void getNormals(const std::vector <double>* vertices,const std::vector<int>* indices,std::vector<double>* normals);
    static void getTrianglesFromPolygons(const std::vector<std::vector<int> >& polygons,std::vector<int>& indices);

private:
    static void _setExtractionExploration(std::vector<unsigned char>* exploration,int index,unsigned char bit,int &allBits,int &twoBits);
    static int _reduceTriangleSizePass(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<double>* texCoords,double maxEdgeSize);
    static int _getNeighbour(int actualTriangle,std::vector<int>* indices,int actualEdge[2],std::vector<std::vector<int>*>* edges,std::vector<unsigned char>* exploredState);
};
