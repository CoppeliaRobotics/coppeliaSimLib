#pragma once

#include <edgeElement.h>
#include <vector>
#include <simMath/3Vector.h>

class CMeshManip
{
  public:
    CMeshManip(double* vertices, int verticesNb, int* indices, int indicesNb);
    virtual ~CMeshManip();

    std::vector<CEdgeElement*> edges;
    std::vector<C3Vector> faceNormals;

    // Various static functions (some of them are redundant or very old!!):
    static bool extractOneShape(std::vector<double>* vertices, std::vector<int>* indices,
                                std::vector<double>* sVertices, std::vector<int>* sIndices);
    static void useOnlyReferencedVertices(std::vector<double>* vertices, std::vector<double>* sVertices,
                                          std::vector<int>* sIndices);

    static double getMaxEdgeLength(const std::vector<double>& vertices, const std::vector<int>& indices);
    static bool reduceTriangleSize(std::vector<double>& vertices, std::vector<int>& indices,
                                   std::vector<double>* normals, std::vector<float>* texCoords, double maxEdgeSize);
    static bool correctTriangleWinding(std::vector<double>* vertices, std::vector<int>* indices);
    static bool isInsideOut(std::vector<double>* vertices, std::vector<int>* indices);
    static void setInsideOut(std::vector<int>* indices);
    static void getProjectionOfPointOnLine(double x, double y, double z, double vx, double vy, double vz, double& px,
                                           double& py, double& pz);
    static bool mergeWith(std::vector<double>* tVertices, std::vector<int>* tIndices, std::vector<double>* tNormals,
                          std::vector<double>* sVertices, std::vector<int>* sIndices, std::vector<double>* sNormals);
    template<typename T> static void reduceToUnit(T vector[3]);
    template<typename T> static void calcNormal(T v[3][3], T out[3]);
    static void getSize(std::vector<double>* vertices, double x[2], double y[2], double z[2], bool& start);
    static void centerAndScale(std::vector<double>* vertices, double x, double y, double z, double sf);
    template<typename T> static void getNormals(const std::vector<T>* vertices, const std::vector<int>* indices, std::vector<T>* normals);
    template<typename T> static double getShadingAngleFromPerTriangleCornerNormals(const std::vector<T>* inVertices, const std::vector<int>* inIndices, const std::vector<T>* inNormals, double minAngle = 1.0 * piValue / 180.0);
    template<typename T> static void generatePerVertexNormalsAndTextCoords(const std::vector<T>* inVertices, const std::vector<int>* inIndices, const std::vector<T>* inNormals, const std::vector<float>* inTexCoords, std::vector<T>* outVertices, std::vector<int>* outIndices, std::vector<T>* outNormals, std::vector<float>* outTexCoords = nullptr);
    template<typename T> static void generatePerTriangleCornerNormalsAndTextCoords(const std::vector<T>* inVertices, const std::vector<int>* inIndices, const std::vector<T>* inNormals, const std::vector<float>* inTexCoords, std::vector<T>* outVertices, std::vector<int>* outIndices, std::vector<T>* outNormals, std::vector<float>* outTexCoords = nullptr);
    static void getTrianglesFromPolygons(const std::vector<std::vector<int>>& polygons, std::vector<int>& indices);

  private:
    static void _setExtractionExploration(std::vector<unsigned char>* exploration, int index, unsigned char bit,
                                          int& allBits, int& twoBits);
    static int _reduceTriangleSizePass(std::vector<double>& vertices, std::vector<int>& indices,
                                       std::vector<double>* normals, std::vector<float>* texCoords, double maxEdgeSize);
    static int _getNeighbour(int actualTriangle, std::vector<int>* indices, int actualEdge[2],
                             std::vector<std::vector<int>*>* edges, std::vector<unsigned char>* exploredState);
};
