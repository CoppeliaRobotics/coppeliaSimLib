#include <simLib/simConst.h>
#include <meshRoutines.h>
#include <meshManip.h>
#include <tt.h>
#include <stdio.h>
#include <simInternal.h>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <app.h>

void CMeshRoutines::getEdgeFeatures(double* vertices, int verticesLength, int* indices, int indicesLength,
                                    std::vector<int>* theVertexIDs, std::vector<int>* theEdgeIDs,
                                    std::vector<int>* theFaceIDs, double angleTolerance, bool forDisplay,
                                    bool hideEdgeBorders)
{ // theVertexIDs, theEdgeIDs or theFaceIDs can be nullptr
    // For each vertex, edge and face, an identifier will be associated:
    // Same triangle ID --> triangles belong to the same face (with given tolerance between normals)
    // Same edge ID --> edges belong to the same edge (with given tolerance). -1 --> edge is embedded in a face
    // Vertex ID is -1 --> vertex is embedded in an edge or face
    // If for Display is true, we calculate edges for nice display, otherwise, we calculate edges that can be used for
    // the dynamic collision rersponse algorithms

    std::vector<int> vertexIDs(verticesLength / 3, 0);
    std::vector<int> edgeIDs(indicesLength, -2);
    std::vector<int> faceIDs(indicesLength / 3, -1);

    // We use a CMeshManip-object for faster access:
    CMeshManip mesh(vertices, verticesLength, indices, indicesLength);
    // First we group similar triangles according to a max. tolerance angle:
    const double cosToleranceAngle = cos(angleTolerance);
    int faceIDCounter = 0;
    for (int i = 0; i < indicesLength / 3; i++)
    {
        if (faceIDs[i] == -1)
        {                                              // this triangle was not yet processed:
            C3Vector normalVect = mesh.faceNormals[i]; // We get its normal vector
            // We prepare recursion with the triangle itself (beginning):
            std::vector<int> neighbours;
            neighbours.push_back(i);
            // Now we recursively go through all its neighbours:
            while (neighbours.size() != 0)
            {
                int el = neighbours[neighbours.size() - 1];
                neighbours.erase(neighbours.end() - 1);
                faceIDs[el] = faceIDCounter;
                int indOr[3] = {indices[3 * el + 0], indices[3 * el + 1], indices[3 * el + 2]};
                for (int ed = 0; ed < 3; ed++)
                { // We check the triangle's 3 edges:
                    int ind[3];
                    int ked = ed;
                    for (int led = 0; led < 3; led++)
                    {
                        ind[led] = indOr[ked];
                        ked++;
                        if (ked == 3)
                            ked = 0;
                    }
                    CEdgeElement* edgeIt = mesh.edges[ind[0]];
                    int neighbourCount = 0;
                    int neighbourWithNoEdgeShowCount = 0;
                    while (edgeIt != nullptr)
                    { // We have to check all triangles with same edges:
                        if (edgeIt->vertex1 == ind[1])
                        { // Same edge. We check if already processed:
                            if (i != edgeIt->triangle)
                                neighbourCount++;
                            if ((faceIDs[edgeIt->triangle] == -1) || (forDisplay && (i != edgeIt->triangle)))
                            { // Not yet processed! Is the angle between normals under tolerance?
                                C3Vector normalVect2 = mesh.faceNormals[edgeIt->triangle];
                                if (normalVect.getAngle(normalVect2) < angleTolerance)
                                { // This triangle belongs to the same plane!
                                    neighbourWithNoEdgeShowCount++;
                                    if (!forDisplay)
                                        neighbours.push_back(edgeIt->triangle);
                                    // Now we have to mark the edge which is in common as non-existing (-1):
                                    edgeIDs[3 * el + ed] = -1;                        // Edge from "From" triangle
                                    edgeIDs[3 * edgeIt->triangle + edgeIt->pos] = -1; // Edge from "To" triangle
                                }
                            }
                        }
                        edgeIt = edgeIt->next;
                    }

                    if (hideEdgeBorders && (neighbourCount == 0))
                    {
                        edgeIDs[3 * el + ed] = -1; // mark the edge as non-existing (-1)
                    }
                    //                  if ( (neighbourCount>1)&&(neighbourWithNoEdgeShowCount>0) )
                    //                  {
                    //                      edgeIDs[3*el+ed]=-1; // mark the edge as non-existing (-1)
                    //                  }
                }
            }
            faceIDCounter++;
        }
    }

    // Now we group similar edges according to a max. tolerance angle:
    int edgeIDCounter = 0;
    for (int i = 0; i < int(mesh.edges.size()); i++) // In fact we could go to the half only!
    {
        CEdgeElement* edgeIt = mesh.edges[i];
        while (edgeIt != nullptr)
        {
            if (edgeIDs[3 * edgeIt->triangle + edgeIt->pos] == -2)
            { // This edge exists and wasn't yet processed:
                C3Vector normalVect = edgeIt->n;
                // We prepare recursion with the edge itself (beginning):
                std::vector<int> neighbours;
                neighbours.push_back(3 * edgeIt->triangle + edgeIt->pos);
                // Now we recursively go through all its neighbours:
                while (neighbours.size() != 0)
                {
                    int el = neighbours[neighbours.size() - 1];
                    int thePos = el % 3;
                    neighbours.erase(neighbours.end() - 1);
                    edgeIDs[el] = edgeIDCounter;
                    // Now we check its neighbours on both sides:
                    int pointIDs[2];
                    pointIDs[0] = indices[el];
                    if (thePos == 2)
                        pointIDs[1] = indices[el - 2];
                    else
                        pointIDs[1] = indices[el + 1];
                    for (int side = 0; side < 2; side++)
                    {
                        int vertexID = pointIDs[side];
                        CEdgeElement* edgeIt2 = mesh.edges[vertexID];
                        while (edgeIt2 != nullptr)
                        {
                            if (edgeIDs[3 * edgeIt2->triangle + edgeIt2->pos] == -2)
                            { // Wasn't processed yet
                                C3Vector normalVect2 = edgeIt2->n;
                                double dd = normalVect * normalVect2; // Scalar product
                                if ((dd > cosToleranceAngle) || (dd < -cosToleranceAngle))
                                { // This segment belongs to the same edge!
                                    neighbours.push_back(3 * edgeIt2->triangle + edgeIt2->pos);
                                    // Now we have to disable the vertex (-1), but only if the two
                                    // edges have different coordinates:
                                    if (((pointIDs[0] == edgeIt2->vertex0) && (pointIDs[1] == edgeIt2->vertex1)) ||
                                        ((pointIDs[0] == edgeIt2->vertex1) && (pointIDs[1] == edgeIt2->vertex0)))
                                    {
                                    }
                                    else
                                        vertexIDs[vertexID] = -1;
                                }
                            }
                            edgeIt2 = edgeIt2->next;
                        }
                    }
                }
                edgeIDCounter++;
            }
            edgeIt = edgeIt->next;
        }
    }
    // Now we have to do a last thing: disable all vertices which have only disabled edges:
    for (int i = 0; i < verticesLength / 3; i++)
    {
        CEdgeElement* edgeIt = mesh.edges[i];
        bool hasActiveEdge = false;
        while (edgeIt != nullptr)
        {
            if (edgeIDs[3 * edgeIt->triangle + edgeIt->pos] != -1)
                hasActiveEdge = true;
            edgeIt = edgeIt->next;
        }
        if (!hasActiveEdge)
            vertexIDs[i] = -1; // We disable this point
    }

    if (theVertexIDs != nullptr)
    {
        theVertexIDs->clear();
        theVertexIDs->assign(vertexIDs.begin(), vertexIDs.end());
    }
    if (theEdgeIDs != nullptr)
    {
        theEdgeIDs->clear();
        theEdgeIDs->assign(edgeIDs.begin(), edgeIDs.end());
    }
    if (theFaceIDs != nullptr)
    {
        theFaceIDs->clear();
        theFaceIDs->assign(faceIDs.begin(), faceIDs.end());
    }
}

void CMeshRoutines::removeThinTriangles(std::vector<double>& vertices, std::vector<int>& indices,
                                        double percentageToKeep)
{
    std::vector<std::pair<double, size_t>> thinPairs;
    for (size_t i = 0; i < indices.size() / 3; i++)
    {
        int ind[3] = {indices[3 * i + 0], indices[3 * i + 1], indices[3 * i + 2]};
        size_t mmax1, mmax2, mmax3;
        double lng = 0.0;
        for (size_t j = 0; j < 3; j++)
        {
            size_t k = j + 1;
            if (k > 2)
                k = 0;
            size_t l = k + 1;
            if (l > 2)
                l = 0;
            double s = (C3Vector(vertices.data() + 3 * ind[j]) - C3Vector(vertices.data() + 3 * ind[k])).getLength();
            if (s > lng)
            {
                lng = s;
                mmax1 = j;
                mmax2 = k;
                mmax3 = l;
            }
        }
        C3Vector dir(C3Vector(vertices.data() + 3 * ind[mmax2]) - C3Vector(vertices.data() + 3 * ind[mmax1]));
        double L = dir.normalize();
        C3Vector v(C3Vector(vertices.data() + 3 * ind[mmax3]) - C3Vector(vertices.data() + 3 * ind[mmax1]));
        double s2 = dir * v;
        double thinness = (v - dir * s2).getLength() / L;
        thinPairs.push_back(std::make_pair(thinness, i));
    }
    std::sort(thinPairs.begin(), thinPairs.end(),
              [](const std::pair<double, int>& p1, const std::pair<double, int>& p2) { return p1.first > p2.first; });

    std::vector<int> nind;
    nind.swap(indices);
    for (size_t i = 0; i < int(percentageToKeep * double(nind.size() / 3)); i++)
    {
        size_t j = thinPairs[i].second;
        indices.push_back(nind[3 * j + 0]);
        indices.push_back(nind[3 * j + 1]);
        indices.push_back(nind[3 * j + 2]);
    }
    removeNonReferencedVertices(vertices, indices);
}

bool CMeshRoutines::getConvexHull(const std::vector<double>& verticesIn, std::vector<double>& verticesOut,
                                  std::vector<int>& indicesOut)
{ // If algo fails, verticesOut are the same as verticesIn, and indicesOut is not touched
    // Keep in mind that verticesIn and verticesOut could be the same buffer
    std::vector<double> initVerticesIn(verticesIn);
    std::vector<double> vert(initVerticesIn);
    int verticesInLength = int(vert.size());
    void* data[10];
    data[0] = (double*)vert.data();
    data[1] = &verticesInLength;
    bool generateIndices = true;
    data[2] = &generateIndices;
    bool success = false;
    data[3] = &success;
    double* outVert = nullptr;
    data[4] = &outVert;
    int outVertLength;
    data[5] = &outVertLength;
    int* outInd = nullptr;
    data[6] = &outInd;
    int outIndLength;
    data[7] = &outIndLength;
    for (size_t j = 0; j < 5; j++)
    { // we try 5 times, each time with the output of previous calculation:
        if (App::worldContainer->pluginContainer->qhull(data) && success)
        {
            std::vector<double> vertices(outVert, outVert + outVertLength);
            std::vector<int> indices(outInd, outInd + outIndLength);
            removeDuplicateVerticesAndTriangles(vertices, &indices, nullptr, nullptr, 0.0005);
            toDelaunayMesh(vertices, indices, nullptr, nullptr);
            bool tooFewTriangles = false;
            if (indices.size() < 12)
            { // revert vertex removal and DelaunayMesh from above
                vertices.assign(outVert, outVert + outVertLength);
                indices.assign(outInd, outInd + outIndLength);
                tooFewTriangles = true;
            }
            delete[] outVert;
            delete[] outInd;
            if (getConvexType(vertices, indices, 0.015) == 0)
            {
                verticesOut.assign(vertices.begin(), vertices.end());
                indicesOut.assign(indices.begin(), indices.end());
                return (true);
            }
            // QHull failed producing a correct convex hull. We try again, by removing problematic points, i.e. points
            // that lie very close to edges
            if (tooFewTriangles)
                break;
            removeThinTriangles(vertices, indices, 0.99);
            vert.swap(vertices);
            data[0] = (double*)vert.data();
            verticesInLength = int(vert.size());
        }
    }
    verticesOut.assign(initVerticesIn.begin(), initVerticesIn.end());
    return (false);
}

bool CMeshRoutines::getDecimatedMesh(const std::vector<double>& verticesIn, const std::vector<int>& indicesIn,
                                     double percentageToKeep, std::vector<double>& verticesOut,
                                     std::vector<int>& indicesOut, double distTolerance)
{
    bool retVal = false;
    if ((verticesIn.size() >= 9) && (indicesIn.size() >= 6))
    {
        std::vector<double> vert(verticesIn);
        std::vector<int> ind(indicesIn);
        removeDuplicateVerticesAndTriangles(vert, &ind, nullptr, nullptr, distTolerance);

        void* data[20];
        data[0] = vert.data();
        int vl = (int)vert.size();
        data[1] = &vl;
        data[2] = ind.data();
        int il = (int)ind.size();
        data[3] = &il;
        data[4] = &percentageToKeep;
        int version = 0;
        data[5] = &version;
        bool success = false;
        data[6] = &success;
        double* outVert = nullptr;
        data[7] = &outVert;
        int outVertLength;
        data[8] = &outVertLength;
        int* outInd = nullptr;
        data[9] = &outInd;
        int outIndLength;
        data[10] = &outIndLength;
        verticesOut.clear();
        indicesOut.clear();
        if (App::worldContainer->pluginContainer->meshDecimator(data))
        {
            if (success)
            {
                for (int i = 0; i < outVertLength; i++)
                    verticesOut.push_back(outVert[i]);
                delete[] outVert;
                for (int i = 0; i < outIndLength; i++)
                    indicesOut.push_back(outInd[i]);
                delete[] outInd;
                retVal = true;
            }
        }
        else
            App::logMsg(sim_verbosity_errors, "mesh decimation failed. Is the OpenMesh plugin loaded?");
    }
    return (retVal);
}

int CMeshRoutines::convexDecompose(const double* vertices, int verticesLength, const int* indices, int indicesLength,
                                   std::vector<std::vector<double>*>& verticesList,
                                   std::vector<std::vector<int>*>& indicesList, size_t nClusters, double concavity,
                                   bool addExtraDistPoints, bool addFacesPoints, double ccConnectDist,
                                   size_t targetNTrianglesDecimatedMesh, size_t maxHullVertices,
                                   double smallestClusterThreshold, bool useHACD, int resolution_VHACD,
                                   int depth_VHACD_old, double concavity_VHACD, int planeDownsampling_VHACD,
                                   int convexHullDownsampling_VHACD, double alpha_VHACD, double beta_VHACD,
                                   double gamma_VHACD_old, bool pca_VHACD, bool voxelBased_VHACD,
                                   int maxVerticesPerCH_VHACD, double minVolumePerCH_VHACD)
{ // 2 100 0 1 1 30 2000
    TRACE_INTERNAL;
    void* data[30];
    int el = 0;
    double** vertList = nullptr;
    int** indList = nullptr;
    int* vertCountList = nullptr;
    int* indCountList = nullptr;

    data[0] = (double*)vertices;
    data[1] = &verticesLength;
    data[2] = (int*)indices;
    data[3] = &indicesLength;
    if (useHACD)
    {
        data[4] = &nClusters;
        data[5] = &concavity;
        data[6] = &addExtraDistPoints;
        data[7] = &addFacesPoints;
        data[8] = &ccConnectDist;
        data[9] = &targetNTrianglesDecimatedMesh;
        data[10] = &maxHullVertices;
        data[11] = &smallestClusterThreshold;
        data[12] = &el;
        data[13] = &vertList;
        data[14] = &indList;
        data[15] = &vertCountList;
        data[16] = &indCountList;
        App::worldContainer->pluginContainer->hacd(data);
    }
    else
    {
        data[4] = &resolution_VHACD;
        data[5] = &depth_VHACD_old;
        data[6] = &concavity_VHACD;
        data[7] = &planeDownsampling_VHACD;
        data[8] = &convexHullDownsampling_VHACD;
        data[9] = &alpha_VHACD;
        data[10] = &beta_VHACD;
        data[11] = &gamma_VHACD_old;
        data[12] = &pca_VHACD;
        data[13] = &voxelBased_VHACD;
        data[14] = &maxVerticesPerCH_VHACD;
        data[15] = &minVolumePerCH_VHACD;
        data[16] = &el;
        data[17] = &vertList;
        data[18] = &indList;
        data[19] = &vertCountList;
        data[20] = &indCountList;
        App::worldContainer->pluginContainer->vhacd(data);
    }

    for (int mesh = 0; mesh < el; mesh++)
    {
        std::vector<double>* _vert = new std::vector<double>;
        std::vector<int>* _ind = new std::vector<int>;
        for (int i = 0; i < vertCountList[mesh]; i++)
            _vert->push_back(vertList[mesh][i]);
        for (int i = 0; i < indCountList[mesh]; i++)
            _ind->push_back(indList[mesh][i]);
        delete[] vertList[mesh];
        delete[] indList[mesh];

        getConvexHull(
            _vert[0], _vert[0],
            _ind[0]); // better results with that! (convex decomp. routine has large tolerance regarding convexivity)

        // We do some checkings on our own here, just in case:
        C3Vector mmin, mmax;
        for (int i = 0; i < int(_vert->size() / 3); i++)
        {
            C3Vector v(&_vert->at(3 * i + 0));
            if (i == 0)
            {
                mmin = v;
                mmax = v;
            }
            else
            {
                mmin.keepMin(v);
                mmax.keepMax(v);
            }
        }
        C3Vector extent(mmax - mmin);
        if ((_ind->size() >= 12) && (std::min<double>(std::min<double>(extent(0), extent(1)), extent(2)) > 0.0001))
        {
            verticesList.push_back(_vert);
            indicesList.push_back(_ind);
        }
        else
        {
            delete _vert;
            delete _ind;
        }
    }
    if (el > 0)
    {
        delete[] indCountList;
        delete[] vertCountList;
        delete[] indList;
        delete[] vertList;
    }
    return ((int)verticesList.size());
}

void CMeshRoutines::_insertEdge(std::vector<std::vector<int>*>& allEdges, int vertexIndex1, int vertexIndex2,
                                int triangleIndex)
{
    int minI = std::min<int>(vertexIndex1, vertexIndex2);
    int maxI = std::max<int>(vertexIndex1, vertexIndex2);
    if (allEdges[minI] == nullptr)
        allEdges[minI] = new std::vector<int>;
    allEdges[minI]->push_back(maxI);
    allEdges[minI]->push_back(triangleIndex);
}

int CMeshRoutines::_getTriangleIndexFromEdge(std::vector<std::vector<int>*>& allEdges, int vertexIndex1,
                                             int vertexIndex2, int triangleIndexToExclude)
{ // returns -1 if there is no such triangle, or more than 1
    int minI = std::min<int>(vertexIndex1, vertexIndex2);
    int maxI = std::max<int>(vertexIndex1, vertexIndex2);
    if (allEdges[minI] == nullptr)
        return (-1);
    int cnt = 0;
    int retVal = -1;
    for (int i = 0; i < int(allEdges[minI]->size()) / 2; i++)
    {
        if ((maxI == allEdges[minI]->at(2 * i + 0)) && (triangleIndexToExclude != allEdges[minI]->at(2 * i + 1)))
        {
            cnt++;
            retVal = allEdges[minI]->at(2 * i + 1);
        }
    }
    if (cnt == 1)
        return (retVal);
    return (-1);
}

int CMeshRoutines::getConvexType(const std::vector<double>& vertices, const std::vector<int>& indices,
                                 double distanceToleranceInPercent)
{ // retVal=0: convex, 1: not convex, 2: not Delaunay mesh
    // Since identical vertices are allowed, first merge them:
    std::vector<double> vertices_(vertices);
    std::vector<int> indices_(indices);
    removeDuplicateVerticesAndTriangles(vertices_, &indices_, nullptr, nullptr, 0.000001);

    // Check if all edges touch exactly 2 triangles, i.e. the mesh is water-tight and remains so, when randomly moving
    // vertices around:
    std::vector<std::map<int, int>> allEdges(vertices_.size() / 3);
    for (size_t i = 0; i < indices_.size() / 3; i++)
    {
        int tri[3] = {indices_[3 * i + 0], indices_[3 * i + 1], indices_[3 * i + 2]};
        if ((tri[0] != tri[1]) && (tri[0] != tri[2]) && (tri[1] != tri[2]))
        {
            for (size_t j = 0; j < 3; j++)
            {
                size_t k = j + 1;
                if (k > 2)
                    k = 0;
                int si = std::min<int>(tri[j], tri[k]);
                int bi = std::max<int>(tri[j], tri[k]);
                auto e = allEdges[si].find(bi);
                if (e == allEdges[si].end())
                    allEdges[si].insert(std::make_pair(bi, 1));
                else
                    allEdges[si][bi] += 1;
            }
        }
    }
    for (size_t i = 0; i < allEdges.size(); i++)
    {
        auto m = allEdges[i];
        for (auto it = m.begin(); it != m.end(); it++)
        {
            if (it->second != 2)
                return (2); // non-Delaunay mesh
        }
    }

    // Check if all face planes have all points on one side (the neg. side), i.e. check for convexity:
    C3Vector minV, maxV;
    for (size_t i = 0; i < vertices_.size() / 3; i++)
    {
        C3Vector v(&vertices_[3 * i]);
        if (i == 0)
        {
            minV = v;
            maxV = v;
        }
        else
        {
            minV.keepMin(v);
            maxV.keepMax(v);
        }
    }
    C3Vector boxDim(maxV - minV);
    double toleratedDist = distanceToleranceInPercent * (boxDim(0) + boxDim(1) + boxDim(2)) / 3.0;
    std::vector<double> planeDefinitions;
    for (size_t i = 0; i < indices_.size() / 3; i++)
    {
        int ind[3] = {indices_[3 * i + 0], indices_[3 * i + 1], indices_[3 * i + 2]};
        C3Vector p0(vertices_.data() + 3 * ind[0]);
        C3Vector p1(vertices_.data() + 3 * ind[1]);
        C3Vector p2(vertices_.data() + 3 * ind[2]);
        C3Vector v0(p1 - p0);
        C3Vector v1(p2 - p0);
        C3Vector n(v0 ^ v1);
        n.normalize();
        double d = n * ((p0 + p1 + p2) / 3.0);
        for (size_t j = 0; j < vertices_.size() / 3; j++)
        {
            C3Vector v(vertices_.data() + 3 * j);
            double dist = v(0) * n(0) + v(1) * n(1) + v(2) * n(2) - d;
            if (dist > toleratedDist)
                return (1); // not convex (above tolerance)
        }
    }
    return (0);
}

void CMeshRoutines::createCube(std::vector<double>& vertices, std::vector<int>& indices, const C3Vector& sizes,
                               const int subdivisions[3])
{
    vertices.clear();
    indices.clear();
    int divX = subdivisions[0];
    int divY = subdivisions[1];
    int divZ = subdivisions[2];
    double xhSize = sizes(0) / 2.0;
    double yhSize = sizes(1) / 2.0;
    double zhSize = sizes(2) / 2.0;
    double xs = sizes(0) / ((double)divX);
    double ys = sizes(1) / ((double)divY);
    double zs = sizes(2) / ((double)divZ);

    // We first create the vertices:
    //******************************
    // Plane1:
    for (int i = 0; i < (divY + 1); i++)
    { // along y
        for (int j = 0; j < (divX + 1); j++)
        { // along x
            tt::addToFloatArray(&vertices, -xhSize + j * xs, -yhSize + i * ys, -zhSize);
        }
    }
    // Plane2:
    for (int i = 1; i < (divZ + 1); i++)
    { // along z
        for (int j = 0; j < (divX + 1); j++)
        { // along x
            tt::addToFloatArray(&vertices, -xhSize + j * xs, -yhSize, -zhSize + i * zs);
        }
    }
    // Plane3:
    for (int i = 1; i < (divZ + 1); i++)
    { // along z
        for (int j = 0; j < (divX + 1); j++)
        { // along x
            tt::addToFloatArray(&vertices, -xhSize + j * xs, +yhSize, -zhSize + i * zs);
        }
    }
    // Plane4:
    for (int i = 1; i < divY; i++)
    { // along y
        for (int j = 0; j < (divX + 1); j++)
        { // along x
            tt::addToFloatArray(&vertices, -xhSize + j * xs, -yhSize + i * ys, +zhSize);
        }
    }
    // Plane5:
    for (int i = 1; i < divY; i++)
    { // along y
        for (int j = 1; j < divZ; j++)
        { // along z
            tt::addToFloatArray(&vertices, -xhSize, -yhSize + i * ys, -zhSize + j * zs);
        }
    }
    // Plane6:
    for (int i = 1; i < divY; i++)
    { // along y
        for (int j = 1; j < divZ; j++)
        { // along z
            tt::addToFloatArray(&vertices, +xhSize, -yhSize + i * ys, -zhSize + j * zs);
        }
    }

    // Now we create the indices:
    //***************************
    // Plane1:
    for (int i = 0; i < divY; i++)
    { // along y
        for (int j = 0; j < divX; j++)
        { // along x
            tt::addToIntArray(&indices, i * (divX + 1) + j + 1, i * (divX + 1) + j, (i + 1) * (divX + 1) + j + 1);
            tt::addToIntArray(&indices, (i + 1) * (divX + 1) + j + 1, i * (divX + 1) + j, (i + 1) * (divX + 1) + j);
        }
    }
    // Plane2:
    int off = divY * (divX + 1) + divX + 1;
    for (int i = 0; i < divX; i++)
    { // along x
        tt::addToIntArray(&indices, i, i + 1, off + i + 1);
        tt::addToIntArray(&indices, i, off + i + 1, off + i);
    }
    off = divY * (divX + 1);
    for (int i = 1; i < divZ; i++)
    { // along y
        for (int j = 0; j < divX; j++)
        { // along x
            tt::addToIntArray(&indices, off + i * (divX + 1) + j, off + i * (divX + 1) + j + 1,
                              off + (i + 1) * (divX + 1) + j + 1);
            tt::addToIntArray(&indices, off + i * (divX + 1) + j, off + (i + 1) * (divX + 1) + j + 1,
                              off + (i + 1) * (divX + 1) + j);
        }
    }
    // Plane3:
    int oldOff = divY * (divX + 1);
    off = (divY + 1) * (divX + 1) + divZ * (divX + 1);
    for (int i = 0; i < divX; i++)
    { // along x
        tt::addToIntArray(&indices, oldOff + i + 1, oldOff + i, off + i + 1);
        tt::addToIntArray(&indices, off + i + 1, oldOff + i, off + i);
    }
    off = (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1);
    for (int i = 1; i < divZ; i++)
    { // along y
        for (int j = 0; j < divX; j++)
        { // along x
            tt::addToIntArray(&indices, off + i * (divX + 1) + j + 1, off + i * (divX + 1) + j,
                              off + (i + 1) * (divX + 1) + j + 1);
            tt::addToIntArray(&indices, off + (i + 1) * (divX + 1) + j + 1, off + i * (divX + 1) + j,
                              off + (i + 1) * (divX + 1) + j);
        }
    }
    // Plane4:
    oldOff = (divY + 1) * (divX + 1) + divZ * (divX + 1) + (divZ - 1) * (divX + 1);
    if (divY >= 2)
        off = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + (divY - 2) * (divX + 1);
    else
        off = (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1);
    for (int i = 0; i < divX; i++)
    { // along x
        tt::addToIntArray(&indices, oldOff + i + 1, oldOff + i, off + i + 1);
        tt::addToIntArray(&indices, off + i + 1, oldOff + i, off + i);
    }
    if (divY >= 2)
    {
        oldOff = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1);
        off = (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1);
        for (int i = 0; i < divX; i++)
        { // along x
            tt::addToIntArray(&indices, oldOff + i + 1, oldOff + i, off + i + 1);
            tt::addToIntArray(&indices, off + i + 1, oldOff + i, off + i);
        }
    }
    off = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - (divX + 1);
    for (int i = 1; i < divY - 1; i++)
    { // along y
        for (int j = 0; j < divX; j++)
        { // along x
            tt::addToIntArray(&indices, off + i * (divX + 1) + j, off + i * (divX + 1) + j + 1,
                              off + (i + 1) * (divX + 1) + j + 1);
            tt::addToIntArray(&indices, off + i * (divX + 1) + j, off + (i + 1) * (divX + 1) + j + 1,
                              off + (i + 1) * (divX + 1) + j);
        }
    }
    // Plane5:
    if (divZ == 1)
    { // No vertical division
        if (divY == 1)
        { // No division in Y -> here we have only one face
            tt::addToIntArray(&indices, (divX + 1) * divY, 0,
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - divX - 1);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - divX - 1, 0,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1));
        }
        else
        {
            tt::addToIntArray(&indices, (divX + 1) * divY, (divX + 1) * (divY - 1),
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - divX - 1);
            tt::addToIntArray(&indices, (divX + 1) * (divY + 1) + 2 * divZ * (divX + 1) + (divY - 2) * (divX + 1),
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - divX - 1, (divX + 1) * (divY - 1));
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1), 0,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1));
            tt::addToIntArray(&indices, divX + 1, 0, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1));
        }
        int up = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1);
        int down = divX + 1;
        for (int i = 0; i < divY - 2; i++)
        {
            tt::addToIntArray(&indices, down + i * (divX + 1), up + i * (divX + 1), up + (i + 1) * (divX + 1));
            tt::addToIntArray(&indices, down + (i + 1) * (divX + 1), down + i * (divX + 1), up + (i + 1) * (divX + 1));
        }
    }
    else
    { // Vertical division
        if (divY == 1)
        { // No division in Y
            tt::addToIntArray(&indices, divY * (divX + 1), 0, (divY + 1) * (divX + 1));
            tt::addToIntArray(&indices, divY * (divX + 1), (divY + 1) * (divX + 1),
                              (divY + 1) * (divX + 1) + divZ * (divX + 1));
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 2 * (divX + 1),
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1),
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - (divX + 1));
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1),
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 2 * (divX + 1),
                              (divY + 1) * (divX + 1) + (divZ - 2) * (divX + 1));
            int left = (divY + 1) * (divX + 1);
            int right = (divY + 1) * (divX + 1) + divZ * (divX + 1);
            for (int i = 0; i < divZ - 2; i++)
            {
                tt::addToIntArray(&indices, right + i * (divX + 1), left + i * (divX + 1), left + (i + 1) * (divX + 1));
                tt::addToIntArray(&indices, right + (i + 1) * (divX + 1), right + i * (divX + 1),
                                  left + (i + 1) * (divX + 1));
            }
        }
        else
        { // Here we have the very general case
            // The corners first:
            int pos = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + (divY - 1) * (divX + 1);
            int posInt = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1);
            tt::addToIntArray(&indices, divX + 1, 0, (divY + 1) * (divX + 1));
            tt::addToIntArray(&indices, divX + 1, (divY + 1) * (divX + 1), pos);
            tt::addToIntArray(&indices, divY * (divX + 1), (divY - 1) * (divX + 1), pos + (divY - 2) * (divZ - 1));
            tt::addToIntArray(&indices, divY * (divX + 1), pos + (divY - 2) * (divZ - 1),
                              (divY + 1) * (divX + 1) + divZ * (divX + 1));
            tt::addToIntArray(&indices, pos + divZ - 2, (divY + 1) * (divX + 1) + (divZ - 2) * (divX + 1),
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1));
            tt::addToIntArray(&indices, pos + divZ - 2, (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1), posInt);
            tt::addToIntArray(&indices, posInt - 2 * (divX + 1), pos + (divZ - 1) * (divY - 1) - 1,
                              posInt + (divY - 2) * (divX + 1));
            tt::addToIntArray(&indices, posInt - 2 * (divX + 1), posInt + (divY - 2) * (divX + 1), posInt - (divX + 1));
            // The down and up part:
            int down1 = divX + 1;
            int down2 = pos;
            int up1 = pos + divZ - 2;
            int up2 = posInt;
            for (int i = 0; i < divY - 2; i++)
            {
                tt::addToIntArray(&indices, down1 + (i + 1) * (divX + 1), down1 + i * (divX + 1),
                                  down2 + i * (divZ - 1));
                tt::addToIntArray(&indices, down1 + (i + 1) * (divX + 1), down2 + i * (divZ - 1),
                                  down2 + (i + 1) * (divZ - 1));
                tt::addToIntArray(&indices, up1 + (i + 1) * (divZ - 1), up1 + i * (divZ - 1), up2 + i * (divX + 1));
                tt::addToIntArray(&indices, up1 + (i + 1) * (divZ - 1), up2 + i * (divX + 1),
                                  up2 + (i + 1) * (divX + 1));
            }
            // The left and right part:
            int left1 = (divY + 1) * (divX + 1);
            int left2 = pos;
            int right1 = pos + (divZ - 1) * (divY - 2);
            int right2 = (divY + 1) * (divX + 1) + divZ * (divX + 1);
            for (int i = 0; i < divZ - 2; i++)
            {
                tt::addToIntArray(&indices, left1 + i * (divX + 1), left1 + (i + 1) * (divX + 1), left2 + i);
                tt::addToIntArray(&indices, left2 + i, left1 + (i + 1) * (divX + 1), left2 + i + 1);
                tt::addToIntArray(&indices, right1 + i, right1 + i + 1, right2 + i * (divX + 1));
                tt::addToIntArray(&indices, right2 + i * (divX + 1), right1 + i + 1, right2 + (i + 1) * (divX + 1));
            }
        }
    }
    off = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + (divY - 1) * (divX + 1);
    for (int i = 0; i < divY - 2; i++)
    { // along y
        for (int j = 0; j < divZ - 2; j++)
        { // along z
            tt::addToIntArray(&indices, off + i * (divZ - 1) + j, off + i * (divZ - 1) + j + 1,
                              off + (i + 1) * (divZ - 1) + j);
            tt::addToIntArray(&indices, off + i * (divZ - 1) + j + 1, off + (i + 1) * (divZ - 1) + j + 1,
                              off + (i + 1) * (divZ - 1) + j);
        }
    }

    // Plane6:
    if (divZ == 1)
    { // No vertical division
        if (divY == 1)
        { // No division in Y -> here we have only one face
            tt::addToIntArray(&indices, divX, (divX + 1) * divY + divX,
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 1);
            tt::addToIntArray(&indices, divX, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 1,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX);
        }
        else
        {
            tt::addToIntArray(&indices, (divX + 1) * (divY - 1) + divX, (divX + 1) * divY + divX,
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 1);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 1,
                              (divX + 1) * (divY + 1) + 2 * divZ * (divX + 1) + (divY - 2) * (divX + 1) + divX,
                              (divX + 1) * (divY - 1) + divX);
            tt::addToIntArray(&indices, divX, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX);
            tt::addToIntArray(&indices, divX, 2 * divX + 1, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + divX);
        }
        int up = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + divX;
        int down = 2 * divX + 1;
        for (int i = 0; i < divY - 2; i++)
        {
            tt::addToIntArray(&indices, up + i * (divX + 1), down + i * (divX + 1), up + (i + 1) * (divX + 1));
            tt::addToIntArray(&indices, down + i * (divX + 1), down + (i + 1) * (divX + 1), up + (i + 1) * (divX + 1));
        }
    }
    else
    { // Vertical division
        if (divY == 1)
        { // No division in Y
            tt::addToIntArray(&indices, divX, divY * (divX + 1) + divX, (divY + 1) * (divX + 1) + divX);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + divX, divY * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + divZ * (divX + 1) + divX);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 2 * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 1);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) - 2 * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + (divZ - 2) * (divX + 1) + divX);
            int left = (divY + 1) * (divX + 1) + divX;
            int right = (divY + 1) * (divX + 1) + divZ * (divX + 1) + divX;
            for (int i = 0; i < divZ - 2; i++)
            {
                tt::addToIntArray(&indices, left + i * (divX + 1), right + i * (divX + 1), left + (i + 1) * (divX + 1));
                tt::addToIntArray(&indices, right + i * (divX + 1), right + (i + 1) * (divX + 1),
                                  left + (i + 1) * (divX + 1));
            }
        }
        else
        { // Here we have the very general case
            // The corners first:
            int pos =
                (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + (divY - 1) * (divX + 1) + (divY - 1) * (divZ - 1);
            int posInt = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + divX;
            tt::addToIntArray(&indices, divX, 2 * divX + 1, (divY + 1) * (divX + 1) + divX);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + divX, 2 * divX + 1, pos);
            tt::addToIntArray(&indices, (divY - 1) * (divX + 1) + divX, divY * (divX + 1) + divX,
                              pos + (divY - 2) * (divZ - 1));
            tt::addToIntArray(&indices, pos + (divY - 2) * (divZ - 1), divY * (divX + 1) + divX,
                              (divY + 1) * (divX + 1) + divZ * (divX + 1) + divX);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + (divZ - 2) * (divX + 1) + divX, pos + divZ - 2,
                              (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX);
            tt::addToIntArray(&indices, (divY + 1) * (divX + 1) + (divZ - 1) * (divX + 1) + divX, pos + divZ - 2,
                              posInt);
            tt::addToIntArray(&indices, pos + (divZ - 1) * (divY - 1) - 1, posInt - 2 * (divX + 1),
                              posInt + (divY - 2) * (divX + 1));
            tt::addToIntArray(&indices, posInt + (divY - 2) * (divX + 1), posInt - 2 * (divX + 1), posInt - (divX + 1));
            // The down and up part:
            int down1 = 2 * divX + 1;
            int down2 = pos;
            int up1 = pos + divZ - 2;
            int up2 = posInt;
            for (int i = 0; i < divY - 2; i++)
            {
                tt::addToIntArray(&indices, down1 + i * (divX + 1), down1 + (i + 1) * (divX + 1),
                                  down2 + i * (divZ - 1));
                tt::addToIntArray(&indices, down2 + i * (divZ - 1), down1 + (i + 1) * (divX + 1),
                                  down2 + (i + 1) * (divZ - 1));
                tt::addToIntArray(&indices, up1 + i * (divZ - 1), up1 + (i + 1) * (divZ - 1), up2 + i * (divX + 1));
                tt::addToIntArray(&indices, up2 + i * (divX + 1), up1 + (i + 1) * (divZ - 1),
                                  up2 + (i + 1) * (divX + 1));
            }
            // The left and right part:
            int left1 = (divY + 1) * (divX + 1) + divX;
            int left2 = pos;
            int right1 = pos + (divZ - 1) * (divY - 2);
            int right2 = (divY + 1) * (divX + 1) + divZ * (divX + 1) + divX;
            for (int i = 0; i < divZ - 2; i++)
            {
                tt::addToIntArray(&indices, left1 + (i + 1) * (divX + 1), left1 + i * (divX + 1), left2 + i);
                tt::addToIntArray(&indices, left1 + (i + 1) * (divX + 1), left2 + i, left2 + i + 1);
                tt::addToIntArray(&indices, right1 + i + 1, right1 + i, right2 + i * (divX + 1));
                tt::addToIntArray(&indices, right1 + i + 1, right2 + i * (divX + 1), right2 + (i + 1) * (divX + 1));
            }
        }
    }
    off = (divY + 1) * (divX + 1) + 2 * divZ * (divX + 1) + (divY - 1) * (divX + 1) + (divZ - 1) * (divY - 1);
    for (int i = 0; i < divY - 2; i++)
    { // along y
        for (int j = 0; j < divZ - 2; j++)
        { // along z
            tt::addToIntArray(&indices, off + i * (divZ - 1) + j + 1, off + i * (divZ - 1) + j,
                              off + (i + 1) * (divZ - 1) + j);
            tt::addToIntArray(&indices, off + (i + 1) * (divZ - 1) + j + 1, off + i * (divZ - 1) + j + 1,
                              off + (i + 1) * (divZ - 1) + j);
        }
    }
}

void CMeshRoutines::createCapsule(std::vector<double>& vertices, std::vector<int>& indices, const C3Vector& sizes,
                                  int sides, int faceSubdiv)
{ // sizes[0]&sizes[1]: diameters, sizes[2]: tube length
    vertices.clear();
    indices.clear();
    double xhSize = sizes(0) / 2.0;
    double yhSize = sizes(1) / 2.0;
    double zhSize = sizes(2) / 2.0;

    double sa = 2.0 * piValue / ((double)sides);
    int ff = sides / 2;
    double fa = piValD2 / ((double)ff);

    double rhSize = std::max<double>(xhSize, yhSize);

    // We set up the vertices:
    tt::addToFloatArray(&vertices, 0.0, 0.0, rhSize + zhSize);
    tt::addToFloatArray(&vertices, 0.0, 0.0, -rhSize - zhSize);
    for (int i = 0; i < sides; i++)
    {
        for (int j = 1; j <= ff; j++)
            tt::addToFloatArray(&vertices, xhSize * sin(fa * j) * cos(sa * i), yhSize * sin(fa * j) * sin(sa * i),
                                zhSize + rhSize * cos(fa * j));

        for (int j = 1; j <= faceSubdiv; j++)
            tt::addToFloatArray(&vertices, xhSize * cos(sa * i), yhSize * sin(sa * i),
                                zhSize - j * sizes(2) / (faceSubdiv + 1));

        for (int j = ff; j > 0; j--)
            tt::addToFloatArray(&vertices, xhSize * sin(fa * j) * cos(sa * i), yhSize * sin(fa * j) * sin(sa * i),
                                -zhSize - rhSize * cos(fa * j));
    }

    int off1 = 2;
    int off2 = 2 * ff + faceSubdiv;

    // We set up the indices:
    for (int i = 0; i < sides - 1; i++)
    {
        tt::addToIntArray(&indices, 0, i * off2 + off1, (i + 1) * off2 + off1);
        for (int j = 0; j < ff - 1; j++)
        {
            tt::addToIntArray(&indices, i * off2 + off1 + j, i * off2 + off1 + j + 1, (i + 1) * off2 + off1 + j);
            tt::addToIntArray(&indices, i * off2 + off1 + j + 1, (i + 1) * off2 + off1 + j + 1,
                              (i + 1) * off2 + off1 + j);
        }
        for (int j = 0; j < faceSubdiv + 1; j++)
        {
            tt::addToIntArray(&indices, i * off2 + off1 + ff - 1 + j, i * off2 + off1 + ff - 1 + j + 1,
                              (i + 1) * off2 + off1 + ff - 1 + j);
            tt::addToIntArray(&indices, i * off2 + off1 + ff - 1 + j + 1, (i + 1) * off2 + off1 + ff - 1 + j + 1,
                              (i + 1) * off2 + off1 + ff - 1 + j);
        }
        for (int j = 0; j < ff - 1; j++)
        {
            tt::addToIntArray(&indices, i * off2 + off1 + ff + faceSubdiv + j,
                              i * off2 + off1 + ff + faceSubdiv + j + 1, (i + 1) * off2 + off1 + ff + faceSubdiv + j);
            tt::addToIntArray(&indices, i * off2 + off1 + ff + faceSubdiv + j + 1,
                              (i + 1) * off2 + off1 + ff + faceSubdiv + j + 1,
                              (i + 1) * off2 + off1 + ff + faceSubdiv + j);
        }
        tt::addToIntArray(&indices, (i + 1) * off2 + off1 - 1, 1, (i + 2) * off2 + off1 - 1);
    }

    int off3 = off2 * sides;
    // We close the capsule:
    tt::addToIntArray(&indices, 0, off3 + off1 - off2, 2);
    for (int j = 0; j < ff - 1; j++)
    {
        tt::addToIntArray(&indices, off3 + off1 - off2 + j, off3 + off1 - off2 + j + 1, off1 + j);
        tt::addToIntArray(&indices, off3 + off1 - off2 + j + 1, off1 + j + 1, off1 + j);
    }
    for (int j = 0; j < faceSubdiv + 1; j++)
    {
        tt::addToIntArray(&indices, off3 + off1 - off2 + ff - 1 + j, off3 + off1 - off2 + ff - 1 + j + 1,
                          off1 + ff - 1 + j);
        tt::addToIntArray(&indices, off3 + off1 - off2 + ff - 1 + j + 1, off1 + ff - 1 + j + 1, off1 + ff - 1 + j);
    }
    for (int j = 0; j < ff - 1; j++)
    {
        tt::addToIntArray(&indices, off3 + off1 - off2 + ff + faceSubdiv + j,
                          off3 + off1 - off2 + ff + faceSubdiv + j + 1, off1 + ff + faceSubdiv + j);
        tt::addToIntArray(&indices, off3 + off1 - off2 + ff + faceSubdiv + j + 1, off1 + ff + faceSubdiv + j + 1,
                          off1 + ff + faceSubdiv + j);
    }
    tt::addToIntArray(&indices, off3 + off1 - 1, 1, off2 + off1 - 1);
}

void CMeshRoutines::createSphere(std::vector<double>& vertices, std::vector<int>& indices, const C3Vector& sizes,
                                 int sides, int faces)
{
    vertices.clear();
    indices.clear();
    double xhSize = sizes(0) / 2.0;
    double yhSize = sizes(1) / 2.0;
    double zhSize = sizes(2) / 2.0;

    double sa = 2.0 * piValue / ((double)sides);
    double fa = piValue / ((double)faces);
    // We set up the vertices:
    tt::addToFloatArray(&vertices, 0.0, 0.0, 1.0);
    tt::addToFloatArray(&vertices, 0.0, 0.0, -1.0);
    for (int i = 0; i < sides; i++)
    {
        for (int j = 1; j < faces; j++)
            tt::addToFloatArray(&vertices, (double)(sin(fa * j) * cos(sa * i)), (double)(sin(fa * j) * sin(sa * i)),
                                (double)cos(fa * j));
    }

    // We set up the indices:
    for (int i = 0; i < sides - 1; i++)
    {
        // First top and bottom part:
        tt::addToIntArray(&indices, 0, i * (faces - 1) + 2, (i + 1) * (faces - 1) + 2);
        tt::addToIntArray(&indices, (i + 1) * (faces - 1) + 1, 1, (i + 2) * (faces - 1) + 1);
        for (int j = 0; j < faces - 2; j++)
        { // Here the rest:
            tt::addToIntArray(&indices, i * (faces - 1) + 2 + j, i * (faces - 1) + 2 + j + 1,
                              (i + 1) * (faces - 1) + 2 + j);
            tt::addToIntArray(&indices, i * (faces - 1) + 2 + j + 1, (i + 1) * (faces - 1) + 2 + j + 1,
                              (i + 1) * (faces - 1) + 2 + j);
        }
    }
    // We have to close the sphere here:
    // First top and bottom part:
    tt::addToIntArray(&indices, 0, (sides - 1) * (faces - 1) + 2, 2);
    tt::addToIntArray(&indices, sides * (faces - 1) + 1, 1, (faces - 1) + 1);
    for (int j = 0; j < faces - 2; j++)
    { // Here the rest:
        tt::addToIntArray(&indices, (sides - 1) * (faces - 1) + 2 + j, (sides - 1) * (faces - 1) + 2 + j + 1, 2 + j);
        tt::addToIntArray(&indices, (sides - 1) * (faces - 1) + 2 + j + 1, 2 + j + 1, 2 + j);
    }
    // Now we scale the sphere:
    for (int i = 0; i < int(vertices.size()) / 3; i++)
    {
        C3Vector p(vertices[3 * i + 0], vertices[3 * i + 1], vertices[3 * i + 2]);
        p(0) = p(0) * xhSize;
        p(1) = p(1) * yhSize;
        p(2) = p(2) * zhSize;
        vertices[3 * i + 0] = p(0);
        vertices[3 * i + 1] = p(1);
        vertices[3 * i + 2] = p(2);
    }
}

void CMeshRoutines::createCylinder(std::vector<double>& vertices, std::vector<int>& indices, const C3Vector& sizes,
                                   int sides, int faces, int discDiv, bool openEnds, bool cone)
{
    vertices.clear();
    indices.clear();
    double xhSize = sizes(0) / 2.0;
    double yhSize = sizes(1) / 2.0;
    double zhSize = sizes(2) / 2.0;

    double zzz = 1.0 / faces;
    double dd = 1.0 / ((double)discDiv);
    double sa = 2.0 * piValue / ((double)sides);
    int sideStart = 0;
    // We set up the vertices:
    if (!openEnds)
    { // The two middle vertices:
        sideStart = 2;
        tt::addToFloatArray(&vertices, 0.0, 0.0, 0.5);
        tt::addToFloatArray(&vertices, 0.0, 0.0, -0.5);
    }

    if (cone)
    {
        for (int i = 0; i < sides; i++)
        { // The side vertices:
            for (int j = 0; j < faces + 1; j++)
                tt::addToFloatArray(&vertices, (double)cos(sa * i) * j / faces, (double)sin(sa * i) * j / faces,
                                    0.5 - j * zzz);
        }
    }
    else
    {
        for (int i = 0; i < sides; i++)
        { // The side vertices:
            for (int j = 0; j < faces + 1; j++)
                tt::addToFloatArray(&vertices, (double)cos(sa * i), (double)sin(sa * i), 0.5 - j * zzz);
        }
    }

    int dstStart = (int)vertices.size() / 3;
    int dsbStart = 0;
    if (!openEnds)
    { // The disc subdivision vertices:
        for (int i = 1; i < discDiv; i++)
            for (int j = 0; j < sides; j++)
                tt::addToFloatArray(&vertices, (1.0 - dd * i) * (double)cos(sa * j),
                                    (1.0 - dd * i) * (double)sin(sa * j), 0.5);
        dsbStart = (int)vertices.size() / 3;
        for (int i = 1; i < discDiv; i++)
            for (int j = 0; j < sides; j++)
                tt::addToFloatArray(&vertices, (1.0 - dd * i) * (double)cos(sa * j),
                                    (1.0 - dd * i) * (double)sin(sa * j), -0.5);
    }

    // We set up the indices:
    for (int i = 0; i < sides - 1; i++)
    {
        // First top and bottom part:
        if (!openEnds)
        {
            if (discDiv == 1)
            {
                tt::addToIntArray(&indices, 0, i * (faces + 1) + sideStart, (i + 1) * (faces + 1) + sideStart);
                tt::addToIntArray(&indices, (i + 1) * (faces + 1) + sideStart - 1, 1,
                                  (i + 2) * (faces + 1) + sideStart - 1);
            }
            else
            {
                tt::addToIntArray(&indices, 0, dstStart + i + sides * (discDiv - 2),
                                  dstStart + i + sides * (discDiv - 2) + 1);
                tt::addToIntArray(&indices, dsbStart + i + sides * (discDiv - 2), 1,
                                  dsbStart + i + sides * (discDiv - 2) + 1);
                for (int j = 0; j < discDiv - 2; j++)
                {
                    tt::addToIntArray(&indices, dstStart + j * sides + i, dstStart + j * sides + i + 1,
                                      dstStart + (j + 1) * sides + i);
                    tt::addToIntArray(&indices, dstStart + j * sides + i + 1, dstStart + (j + 1) * sides + i + 1,
                                      dstStart + (j + 1) * sides + i);
                    tt::addToIntArray(&indices, dsbStart + j * sides + i + 1, dsbStart + j * sides + i,
                                      dsbStart + (j + 1) * sides + i);
                    tt::addToIntArray(&indices, dsbStart + (j + 1) * sides + i + 1, dsbStart + j * sides + i + 1,
                                      dsbStart + (j + 1) * sides + i);
                }
                tt::addToIntArray(&indices, sideStart + i * (faces + 1), sideStart + (i + 1) * (faces + 1),
                                  dstStart + i);
                tt::addToIntArray(&indices, sideStart + (i + 1) * (faces + 1), dstStart + i + 1, dstStart + i);
                tt::addToIntArray(&indices, sideStart + (i + 2) * (faces + 1) - 1,
                                  sideStart + (i + 1) * (faces + 1) - 1, dsbStart + i);
                tt::addToIntArray(&indices, dsbStart + i + 1, sideStart + (i + 2) * (faces + 1) - 1, dsbStart + i);
            }
        }
        for (int j = 0; j < faces; j++)
        { // Here the rest:
            tt::addToIntArray(&indices, i * (faces + 1) + sideStart + j, i * (faces + 1) + sideStart + j + 1,
                              (i + 1) * (faces + 1) + sideStart + j);
            tt::addToIntArray(&indices, i * (faces + 1) + sideStart + j + 1, (i + 1) * (faces + 1) + sideStart + j + 1,
                              (i + 1) * (faces + 1) + sideStart + j);
        }
    }

    // We have to close the cylinder here:
    // First top and bottom part:
    if (!openEnds)
    {
        if (discDiv == 1)
        {
            tt::addToIntArray(&indices, 0, (sides - 1) * (faces + 1) + sideStart, sideStart);
            tt::addToIntArray(&indices, sides * (faces + 1) + sideStart - 1, 1, (faces + 1) + sideStart - 1);
        }
        else
        {
            tt::addToIntArray(&indices, 0, dstStart + sides * (discDiv - 1) - 1, dstStart + sides * (discDiv - 2));
            tt::addToIntArray(&indices, dsbStart + sides * (discDiv - 1) - 1, 1, dsbStart + sides * (discDiv - 2));

            for (int j = 0; j < discDiv - 2; j++)
            {
                tt::addToIntArray(&indices, dstStart + j * sides + sides - 1, dstStart + j * sides,
                                  dstStart + (j + 2) * sides - 1);
                tt::addToIntArray(&indices, dstStart + j * sides, dstStart + (j + 1) * sides,
                                  dstStart + (j + 2) * sides - 1);
                tt::addToIntArray(&indices, dsbStart + j * sides, dsbStart + (j + 1) * sides - 1,
                                  dsbStart + (j + 2) * sides - 1);
                tt::addToIntArray(&indices, dsbStart + (j + 1) * sides, dsbStart + j * sides,
                                  dsbStart + (j + 2) * sides - 1);
            }
            tt::addToIntArray(&indices, sideStart + (sides - 1) * (faces + 1), sideStart, dstStart + sides - 1);
            tt::addToIntArray(&indices, sideStart, dstStart, dstStart + sides - 1);
            tt::addToIntArray(&indices, sideStart + faces, sideStart + faces + (sides - 1) * (faces + 1),
                              dsbStart + sides - 1);
            tt::addToIntArray(&indices, dsbStart, sideStart + faces, dsbStart + sides - 1);
        }
    }
    for (int j = 0; j < faces; j++)
    { // Here the rest:
        tt::addToIntArray(&indices, (sides - 1) * (faces + 1) + sideStart + j,
                          (sides - 1) * (faces + 1) + sideStart + j + 1, sideStart + j);
        tt::addToIntArray(&indices, (sides - 1) * (faces + 1) + sideStart + j + 1, sideStart + j + 1, sideStart + j);
    }

    if (cone)
    { // We have a degenerate cylinder, we need to remove degenerate triangles and double vertices:
        removeDuplicateVerticesAndTriangles(vertices, &indices, nullptr, nullptr, 0.000001);
    }

    // Now we scale the cylinder:
    for (int i = 0; i < int(vertices.size()) / 3; i++)
    {
        C3Vector p(vertices[3 * i + 0], vertices[3 * i + 1], vertices[3 * i + 2]);
        p(0) = p(0) * xhSize;
        p(1) = p(1) * yhSize;
        p(2) = p(2) * 2.0 * zhSize;
        vertices[3 * i + 0] = p(0);
        vertices[3 * i + 1] = p(1);
        vertices[3 * i + 2] = p(2);
    }
}

void CMeshRoutines::createAnnulus(std::vector<double>& vertices, std::vector<int>& indices, double Dlarge,
                                  double Dsmall, double zShift, int sides, bool faceUp)
{
    vertices.clear();
    indices.clear();
    double R = Dlarge * 0.5;
    double r = Dsmall * 0.5;

    double sa = 2.0 * piValue / ((double)sides);

    // We set up the vertices:
    for (int i = 0; i < sides; i++)
        tt::addToFloatArray(&vertices, R * (double)cos(sa * i), R * (double)sin(sa * i), zShift);
    for (int i = 0; i < sides; i++)
        tt::addToFloatArray(&vertices, r * (double)cos(sa * i), r * (double)sin(sa * i), zShift);

    // We set up the indices:
    for (int i = 0; i < sides - 1; i++)
    {
        if (faceUp)
        {
            tt::addToIntArray(&indices, i, i + sides, i + 1);
            tt::addToIntArray(&indices, i + sides, i + sides + 1, i + 1);
        }
        else
        {
            tt::addToIntArray(&indices, i + sides, i, i + 1);
            tt::addToIntArray(&indices, i + sides + 1, i + sides, i + 1);
        }
    }
    // the closing part:
    if (faceUp)
    {
        tt::addToIntArray(&indices, sides - 1, 2 * sides - 1, 0);
        tt::addToIntArray(&indices, 2 * sides - 1, sides, 0);
    }
    else
    {
        tt::addToIntArray(&indices, 2 * sides - 1, sides - 1, 0);
        tt::addToIntArray(&indices, sides, 2 * sides - 1, 0);
    }
}

class CKdNode1
{
  public:
    CKdNode1(double newVal)
    {
        kdNodes[0] = nullptr;
        kdNodes[1] = nullptr;
        val = newVal;
        index = -1;
        cnt = 1;
    }
    virtual ~CKdNode1()
    {
        if (kdNodes[0] != nullptr)
            delete kdNodes[0];
        if (kdNodes[1] != nullptr)
            delete kdNodes[1];
    }

    CKdNode1* insert(double newVal, double tolerance)
    {
        if (fabs(val - newVal) < tolerance)
        {
            cnt++;
            return this;
        }
        else
        {
            if (newVal < val)
            {
                if (kdNodes[0] != nullptr)
                    return kdNodes[0]->insert(newVal, tolerance);
                kdNodes[0] = new CKdNode1(newVal);
                return kdNodes[0];
            }
            else
            {
                if (kdNodes[1] != nullptr)
                    return kdNodes[1]->insert(newVal, tolerance);
                kdNodes[1] = new CKdNode1(newVal);
                return kdNodes[1];
            }
        }
    }

    CKdNode1* kdNodes[2]; // neg and pos
    double val;
    int index;
    int cnt;
};

void CMeshRoutines::toDelaunayMesh(const std::vector<double>& vertices, std::vector<int>& indices,
                                   std::vector<double>* normals, std::vector<float>* texCoords)
{ // converts the mesh to a "Delaunay mesh", i.e. all touching edges have the same length
    if (getConvexType(vertices, indices, 0.015) == 2)
    {                                      // we indeed have a non-Delaunay mesh
        std::vector<int> directionIndices; // same size as indices
        CKdNode1* allDirectionsTree = nullptr;
        int nextDirectionsIndex = 0;
        std::vector<std::vector<int>> directionsOfVertices; // same size as number of nodes in allDirectionsTree
        for (size_t i = 0; i < indices.size() / 3; i++)
        {
            int ind[3] = {indices[3 * i + 0], indices[3 * i + 1], indices[3 * i + 2]};
            for (size_t j = 0; j < 3; j++)
            {
                size_t k = j + 1;
                if (k == 3)
                    k = 0;
                C3Vector p0(vertices.data() + 3 * ind[j]);
                C3Vector p1(vertices.data() + 3 * ind[k]);
                C3Vector dx(p1 - p0);
                dx.normalize();
                for (size_t l = 0; l < 3; l++)
                    dx(l) = fabs(dx(l));
                double directionHash = dx(0) + 2.0 * dx(1) * 3.0 * dx(2);
                CKdNode1* node = nullptr;
                if (allDirectionsTree == nullptr)
                {
                    allDirectionsTree = new CKdNode1(directionHash);
                    allDirectionsTree->index = nextDirectionsIndex++;
                    node = allDirectionsTree;
                }
                else
                {
                    node = allDirectionsTree->insert(directionHash,
                                                     0.05); // a ~1 meter segment may deviate by 5 cm. Better be too
                                                            // tolerant (pruning happens later on again)
                    if (node->index == -1)
                        node->index = nextDirectionsIndex++; // new direction/node
                }
                directionIndices.push_back(node->index);
                if (directionsOfVertices.size() < nextDirectionsIndex)
                    directionsOfVertices.push_back(std::vector<int>());
                directionsOfVertices[size_t(node->index)].push_back(ind[j]);
                directionsOfVertices[size_t(node->index)].push_back(ind[k]);
            }
        }
        delete allDirectionsTree;
        for (size_t i = 0; i < indices.size() / 3; i++)
        {
            int ind[3] = {indices[3 * i + 0], indices[3 * i + 1], indices[3 * i + 2]};
            int dirInd[3] = {directionIndices[3 * i + 0], directionIndices[3 * i + 1], directionIndices[3 * i + 2]};
            C3Vector ns[3];
            float nt[3][2];
            if (normals != nullptr)
            {
                ns[0] = C3Vector(normals->data() + 9 * i + 0);
                ns[1] = C3Vector(normals->data() + 9 * i + 3);
                ns[2] = C3Vector(normals->data() + 9 * i + 6);
            }
            if (texCoords != nullptr)
            {
                for (size_t j = 0; j < 3; j++)
                {
                    nt[j][0] = texCoords->at(6 * i + 2 * j + 0);
                    nt[j][1] = texCoords->at(6 * i + 2 * j + 1);
                }
            }
            for (size_t j = 0; j < 3; j++)
            {
                if (dirInd[j] >= 0)
                {
                    bool breakOut = false;
                    size_t k = j + 1;
                    if (k == 3)
                        k = 0;
                    size_t l = k + 1;
                    if (l == 3)
                        l = 0;
                    C3Vector p0(vertices.data() + 3 * ind[j]);
                    C3Vector p1(vertices.data() + 3 * ind[k]);
                    C3Vector dx(p1 - p0);
                    double le = dx.normalize();
                    size_t dirIndex = dirInd[j];
                    std::vector<int> vert;
                    for (size_t m = 0; m < directionsOfVertices[dirIndex].size(); m++)
                    {
                        int midInd = directionsOfVertices[dirIndex][m];
                        if ((midInd != ind[j]) && (midInd != ind[k]))
                        {
                            C3Vector mid(vertices.data() + 3 * midInd);
                            mid = mid - p0;
                            double d = dx * mid;
                            if ((d > 0.001 * le) && (d < 0.999 * le)) // distances from segment endpoints
                            {
                                if ((mid - dx * d).getLength() < 0.00001) // dist. perp. from line
                                {
                                    // Add 2 new triangles:
                                    indices.push_back(ind[j]);
                                    indices.push_back(midInd);
                                    indices.push_back(ind[l]);
                                    directionIndices.push_back(dirInd[j]);
                                    directionIndices.push_back(-1); // new direction/edge that is not relevant
                                    directionIndices.push_back(dirInd[l]);
                                    if (normals != nullptr)
                                    {
                                        normals->push_back(ns[j](0));
                                        normals->push_back(ns[j](1));
                                        normals->push_back(ns[j](2));
                                        normals->push_back((ns[j](0) + ns[k](0)) / 2.0);
                                        normals->push_back((ns[j](1) + ns[k](1)) / 2.0);
                                        normals->push_back((ns[j](2) + ns[k](2)) / 2.0);
                                        normals->push_back(ns[l](0));
                                        normals->push_back(ns[l](1));
                                        normals->push_back(ns[l](2));
                                    }
                                    if (texCoords != nullptr)
                                    {
                                        texCoords->push_back(nt[j][0]);
                                        texCoords->push_back(nt[j][1]);
                                        texCoords->push_back((nt[j][0] + nt[k][0]) /
                                                             2.0f); // we take the middle, which is not correct!
                                        texCoords->push_back((nt[j][1] + nt[k][1]) / 2.0f);
                                        texCoords->push_back(nt[l][0]);
                                        texCoords->push_back(nt[l][1]);
                                    }
                                    indices.push_back(midInd);
                                    indices.push_back(ind[k]);
                                    indices.push_back(ind[l]);
                                    directionIndices.push_back(dirInd[j]);
                                    directionIndices.push_back(dirInd[k]);
                                    directionIndices.push_back(-1); // new direction/edge that is not relevant
                                    if (normals != nullptr)
                                    {
                                        normals->push_back((ns[j](0) + ns[k](0)) / 2.0);
                                        normals->push_back((ns[j](1) + ns[k](1)) / 2.0);
                                        normals->push_back((ns[j](2) + ns[k](2)) / 2.0);
                                        normals->push_back(ns[k](0));
                                        normals->push_back(ns[k](1));
                                        normals->push_back(ns[k](2));
                                        normals->push_back(ns[l](0));
                                        normals->push_back(ns[l](1));
                                        normals->push_back(ns[l](2));
                                    }
                                    if (texCoords != nullptr)
                                    {
                                        texCoords->push_back((nt[j][0] + nt[k][0]) /
                                                             2.0f); // we take the middle, which is not correct!
                                        texCoords->push_back((nt[j][1] + nt[k][1]) / 2.0f);
                                        texCoords->push_back(nt[k][0]);
                                        texCoords->push_back(nt[k][1]);
                                        texCoords->push_back(nt[l][0]);
                                        texCoords->push_back(nt[l][1]);
                                    }

                                    indices[3 * i + 0] = -1; // disable the original triangle
                                    breakOut = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (breakOut)
                        break;
                }
            }
        }
        std::vector<int> ind(indices);
        indices.clear();
        std::vector<double> norm;
        if (normals != nullptr)
        {
            norm.assign(normals->begin(), normals->end());
            normals->clear();
        }
        std::vector<float> tex;
        if (texCoords != nullptr)
        {
            tex.assign(texCoords->begin(), texCoords->end());
            texCoords->clear();
        }
        for (size_t i = 0; i < ind.size() / 3; i++)
        {
            if (ind[3 * i + 0] >= 0)
            {
                for (size_t j = 0; j < 3; j++)
                    indices.push_back(ind[3 * i + j]);
                if (normals != nullptr)
                {
                    for (size_t j = 0; j < 9; j++)
                        normals->push_back(norm[9 * i + j]);
                }
                if (texCoords != nullptr)
                {
                    for (size_t j = 0; j < 6; j++)
                        texCoords->push_back(tex[6 * i + j]);
                }
            }
        }
    }
}

struct SGeodVertNode
{
    int index;
    double dist;
    SGeodVertNode* prevNode;
    bool visited;
    std::set<SGeodVertNode*> connectedNodes; // unordered_set is somehow slower
};

double CMeshRoutines::getGeodesicDistanceOnConvexMesh(const C3Vector& pt1, const C3Vector& pt2,
                                                      const std::vector<double>& vertices,
                                                      const std::vector<int>* auxIndices /*=nullptr*/,
                                                      std::vector<double>* path /*=nullptr*/,
                                                      double maxEdgeLength /*=0.01*/, int* debugShape /*=nullptr*/)
{
    double retVal = DBL_MAX;
    std::vector<double> vert;
    std::vector<int> ind;
    if ((auxIndices != nullptr) && (auxIndices->size() != 0))
    { // Try to work on the raw mesh, if it is a single mesh:
        std::vector<double> v1(vertices.begin(), vertices.end());
        std::vector<int> i1(auxIndices->begin(), auxIndices->end());
        std::vector<double> v2;
        std::vector<int> i2;
        if (!CMeshManip::extractOneShape(&v1, &i1, &v2, &i2))
        {
            vert.assign(vertices.begin(), vertices.end());
            ind.assign(auxIndices->begin(), auxIndices->end());
            removeDuplicateVerticesAndTriangles(vert, &ind, nullptr, nullptr, 0.0);
            removeNonReferencedVertices(vert, ind);
            toDelaunayMesh(vert, ind, nullptr, nullptr);
        }
    }
    if (vert.size() == 0)
    { // Extract a convex hull from the vertices:
        std::vector<double> inVert(vertices);
        inVert.push_back(pt2(0));
        inVert.push_back(pt2(1));
        inVert.push_back(pt2(2));
        inVert.push_back(pt1(0));
        inVert.push_back(pt1(1));
        inVert.push_back(pt1(2));
        if (!getConvexHull(inVert, vert, ind))
            vert.clear();
    }
    if ((vert.size() >= 9) && (ind.size() >= 3))
    {
        std::unordered_set<SGeodVertNode*> unvisitedNodes;
        std::vector<SGeodVertNode*> allNodes;
        CMeshManip::reduceTriangleSize(vert, ind, nullptr, nullptr, maxEdgeLength);
        if (debugShape != nullptr)
            debugShape[0] = CALL_C_API_CLEAR_ERRORS(simCreateShape, 0, 0.0, vert.data(), int(vert.size()), ind.data(), int(ind.size()), nullptr, nullptr, nullptr, nullptr);
        // Prepare data structure for dijkstra algo:
        for (size_t i = 0; i < vert.size() / 3; i++)
        {
            SGeodVertNode* n = new SGeodVertNode;
            n->index = int(i);
            n->dist = DBL_MAX;
            n->prevNode = nullptr;
            n->visited = false;
            allNodes.push_back(n);
            unvisitedNodes.insert(n);
        }
        for (size_t i = 0; i < ind.size() / 3; i++)
        {
            int tri[3] = {ind[3 * i + 0], ind[3 * i + 1], ind[3 * i + 2]};
            SGeodVertNode* nodes[3] = {allNodes[tri[0]], allNodes[tri[1]], allNodes[tri[2]]};
            nodes[0]->connectedNodes.insert(nodes[1]);
            nodes[0]->connectedNodes.insert(nodes[2]);
            nodes[1]->connectedNodes.insert(nodes[0]);
            nodes[1]->connectedNodes.insert(nodes[2]);
            nodes[2]->connectedNodes.insert(nodes[0]);
            nodes[2]->connectedNodes.insert(nodes[1]);
        }

        // Identify the start and goal nodes:
        SGeodVertNode* startNode;
        SGeodVertNode* goalNode;
        double startD = DBL_MAX;
        double goalD = DBL_MAX;
        for (size_t i = 0; i < vert.size() / 3; i++)
        { // we will actually search from p2=startNode to p1=goalNode
            C3Vector p(vert.data() + 3 * i);
            double d = (p - pt2).getLength();
            if (d < startD)
            {
                startD = d;
                startNode = allNodes[i];
            }
            d = (p - pt1).getLength();
            if (d < goalD)
            {
                goalD = d;
                goalNode = allNodes[i];
            }
        }

        if (startNode != goalNode)
        {
            startNode->dist = 0.0;

            // Main Dijkstra loop:
            while (true)
            {
                // Find closest in unvisited:
                double d = DBL_MAX;
                SGeodVertNode* mnode = nullptr;
                for (auto it = unvisitedNodes.begin(); it != unvisitedNodes.end(); it++)
                {
                    if ((*it)->dist < d)
                    {
                        d = (*it)->dist;
                        mnode = (*it);
                    }
                }
                // Loop through its neighbours, and check distance:
                for (auto it = mnode->connectedNodes.begin(); it != mnode->connectedNodes.end(); it++)
                {
                    if (unvisitedNodes.find(*it) != unvisitedNodes.end())
                    {
                        C3Vector origin(vert.data() + 3 * mnode->index);
                        C3Vector pt(vert.data() + 3 * (*it)->index);
                        double d = (origin - pt).getLength() + mnode->dist;
                        if (d < (*it)->dist)
                        {
                            (*it)->dist = d;
                            (*it)->prevNode = mnode;
                        }
                    }
                }
                // Remove current node from unvisited set:
                unvisitedNodes.erase(mnode);
                if (mnode == goalNode)
                    break; // we are done
            }

            // We have the minimum distance:
            retVal = goalNode->dist;
            // Get the path:
            if (path != nullptr)
            {
                path->clear();
                SGeodVertNode* node = goalNode;
                while (true)
                {
                    path->push_back(vert[3 * node->index + 0]);
                    path->push_back(vert[3 * node->index + 1]);
                    path->push_back(vert[3 * node->index + 2]);
                    if (node == startNode)
                        break;
                    node = node->prevNode;
                }
            }
        }

        for (size_t i = 0; i < allNodes.size(); i++)
            delete allNodes[i];
        allNodes.clear();
        unvisitedNodes.clear();
    }

    return (retVal);
}

class CKdNode3
{
  public:
    CKdNode3(const C3Vector& vert, size_t vertIndex)
    {
        kdNodes[0] = nullptr;
        kdNodes[1] = nullptr;
        vertex = vert;
        index = vertIndex;
    }
    virtual ~CKdNode3()
    {
        if (kdNodes[0] != nullptr)
            delete kdNodes[0];
        if (kdNodes[1] != nullptr)
            delete kdNodes[1];
    }

    CKdNode3* insert(const C3Vector& vert, size_t vertIndex, double tolerance, size_t axis = 0)
    {
        size_t naxis = axis + 1;
        if (naxis > 2)
            naxis = 0;
        double d = vert(axis) - vertex(axis);
        if (fabs(d) < tolerance)
        {
            CKdNode3* retNode = nullptr;
            if ((vert - vertex).getLength() < tolerance)
                retNode = this;
            else
            { // need to check both sides of the partition
                if (kdNodes[0] != nullptr)
                    retNode = kdNodes[0]->getSimilar(vert, vertIndex, tolerance, naxis);
                if ((retNode == nullptr) && (kdNodes[1] != nullptr))
                    retNode = kdNodes[1]->getSimilar(vert, vertIndex, tolerance, naxis);
            }
            if (retNode != nullptr)
                return retNode;
        }
        if (d < 0.0)
        {
            if (kdNodes[0] != nullptr)
                return kdNodes[0]->insert(vert, vertIndex, tolerance, naxis);
            kdNodes[0] = new CKdNode3(vert, vertIndex);
            return kdNodes[0];
        }
        else
        {
            if (kdNodes[1] != nullptr)
                return kdNodes[1]->insert(vert, vertIndex, tolerance, naxis);
            kdNodes[1] = new CKdNode3(vert, vertIndex);
            return kdNodes[1];
        }
    }

    CKdNode3* getSimilar(const C3Vector& vert, size_t vertIndex, double tolerance, size_t axis = 0)
    {
        size_t naxis = axis + 1;
        if (naxis > 2)
            naxis = 0;
        double d = vert(axis) - vertex(axis);
        if (fabs(d) < tolerance)
        {
            if ((vert - vertex).getLength() < tolerance)
                return this;
        }
        CKdNode3* retNode = nullptr;
        if (d < tolerance)
        {
            if (kdNodes[0] != nullptr)
                retNode = kdNodes[0]->getSimilar(vert, vertIndex, tolerance, naxis);
        }
        if ((retNode == nullptr) && (d > -tolerance))
        {
            if (kdNodes[1] != nullptr)
                retNode = kdNodes[1]->getSimilar(vert, vertIndex, tolerance, naxis);
        }
        return retNode;
    }

    CKdNode3* kdNodes[2]; // neg and pos
    C3Vector vertex;
    size_t index;
};

void CMeshRoutines::removeDuplicateVerticesAndTriangles(std::vector<double>& vertices, std::vector<int>* indices,
                                                        std::vector<double>* normals, std::vector<float>* texCoords,
                                                        double distTolerance)
{
    // slightly mix vertices:
    std::vector<size_t> map;
    map.resize(vertices.size() / 3, -1);
    std::vector<double> nvert;
    size_t h = vertices.size() / 6;
    for (size_t i = 0; i < h; i++)
    {
        map[i] = 2 * i + 0;
        nvert.push_back(vertices[3 * i + 0]);
        nvert.push_back(vertices[3 * i + 1]);
        nvert.push_back(vertices[3 * i + 2]);

        map[h + i] = 2 * i + 1;
        nvert.push_back(vertices[3 * h + 3 * i + 0]);
        nvert.push_back(vertices[3 * h + 3 * i + 1]);
        nvert.push_back(vertices[3 * h + 3 * i + 2]);
    }
    if (fmod(double(vertices.size()), 6.0) > 0.1)
    {
        map[map.size() - 1] = map.size() - 1;
        nvert.push_back(vertices[vertices.size() - 3]);
        nvert.push_back(vertices[vertices.size() - 2]);
        nvert.push_back(vertices[vertices.size() - 1]);
    }
    if (indices != nullptr)
    {
        for (size_t i = 0; i < indices->size(); i++)
            indices->at(i) = int(map[indices->at(i)]);
    }

    // Identify duplicate vertices:
    CKdNode3* startNode = nullptr;
    for (size_t i = 0; i < nvert.size() / 3; i++)
    {
        C3Vector v(nvert.data() + 3 * i);
        CKdNode3* node = nullptr;
        if (i == 0)
        {
            startNode = new CKdNode3(v, i);
            node = startNode;
        }
        else
            node = startNode->insert(v, i, distTolerance, 0);
        map[i] = node->index;
    }
    delete startNode;

    // Remove duplicate vertices:
    vertices.clear();
    std::vector<int> map2;
    map2.resize(map.size(), -1);
    for (size_t i = 0; i < map.size(); i++)
    {
        if (map[i] == i)
        {
            map2[i] = int(vertices.size()) / 3;
            vertices.push_back(nvert[3 * i + 0]);
            vertices.push_back(nvert[3 * i + 1]);
            vertices.push_back(nvert[3 * i + 2]);
        }
    }
    std::vector<int> nind;
    std::vector<double> nnorm;
    std::vector<float> ntex;
    if (indices != nullptr)
    { // fix triangles
        for (size_t i = 0; i < indices->size(); i++)
            indices->at(i) = map2[map[indices->at(i)]];
        // keep only non-degenerate triangles, keep smallest vertex triplet index first:
        for (size_t i = 0; i < indices->size() / 3; i++)
        {
            int ind[3] = {indices->at(3 * i + 0), indices->at(3 * i + 1), indices->at(3 * i + 2)};
            if ((ind[0] != ind[1]) && (ind[0] != ind[2]) && (ind[1] != ind[2]))
            {
                if ((ind[0] > ind[1]) && (ind[0] > ind[2]))
                {
                    nind.push_back(ind[0]);
                    nind.push_back(ind[1]);
                    nind.push_back(ind[2]);
                    if (normals != nullptr)
                    {
                        nnorm.push_back(normals->at(9 * i + 0));
                        nnorm.push_back(normals->at(9 * i + 1));
                        nnorm.push_back(normals->at(9 * i + 2));
                        nnorm.push_back(normals->at(9 * i + 3));
                        nnorm.push_back(normals->at(9 * i + 4));
                        nnorm.push_back(normals->at(9 * i + 5));
                        nnorm.push_back(normals->at(9 * i + 6));
                        nnorm.push_back(normals->at(9 * i + 7));
                        nnorm.push_back(normals->at(9 * i + 8));
                    }
                    if (texCoords != nullptr)
                    {
                        ntex.push_back(texCoords->at(6 * i + 0));
                        ntex.push_back(texCoords->at(6 * i + 1));
                        ntex.push_back(texCoords->at(6 * i + 2));
                        ntex.push_back(texCoords->at(6 * i + 3));
                        ntex.push_back(texCoords->at(6 * i + 4));
                        ntex.push_back(texCoords->at(6 * i + 5));
                    }
                }
                else if ((ind[1] > ind[0]) && (ind[1] > ind[2]))
                {
                    nind.push_back(ind[1]);
                    nind.push_back(ind[2]);
                    nind.push_back(ind[0]);
                    if (normals != nullptr)
                    {
                        nnorm.push_back(normals->at(9 * i + 3));
                        nnorm.push_back(normals->at(9 * i + 4));
                        nnorm.push_back(normals->at(9 * i + 5));
                        nnorm.push_back(normals->at(9 * i + 6));
                        nnorm.push_back(normals->at(9 * i + 7));
                        nnorm.push_back(normals->at(9 * i + 8));
                        nnorm.push_back(normals->at(9 * i + 0));
                        nnorm.push_back(normals->at(9 * i + 1));
                        nnorm.push_back(normals->at(9 * i + 2));
                    }
                    if (texCoords != nullptr)
                    {
                        ntex.push_back(texCoords->at(6 * i + 2));
                        ntex.push_back(texCoords->at(6 * i + 3));
                        ntex.push_back(texCoords->at(6 * i + 4));
                        ntex.push_back(texCoords->at(6 * i + 5));
                        ntex.push_back(texCoords->at(6 * i + 0));
                        ntex.push_back(texCoords->at(6 * i + 1));
                    }
                }
                else
                {
                    nind.push_back(ind[2]);
                    nind.push_back(ind[0]);
                    nind.push_back(ind[1]);
                    if (normals != nullptr)
                    {
                        nnorm.push_back(normals->at(9 * i + 6));
                        nnorm.push_back(normals->at(9 * i + 7));
                        nnorm.push_back(normals->at(9 * i + 8));
                        nnorm.push_back(normals->at(9 * i + 0));
                        nnorm.push_back(normals->at(9 * i + 1));
                        nnorm.push_back(normals->at(9 * i + 2));
                        nnorm.push_back(normals->at(9 * i + 3));
                        nnorm.push_back(normals->at(9 * i + 4));
                        nnorm.push_back(normals->at(9 * i + 5));
                    }
                    if (texCoords != nullptr)
                    {
                        ntex.push_back(texCoords->at(6 * i + 4));
                        ntex.push_back(texCoords->at(6 * i + 5));
                        ntex.push_back(texCoords->at(6 * i + 0));
                        ntex.push_back(texCoords->at(6 * i + 1));
                        ntex.push_back(texCoords->at(6 * i + 2));
                        ntex.push_back(texCoords->at(6 * i + 3));
                    }
                }
            }
        }
    }
    // remove similar triangles (a-b-c == a-c-b == etc.)
    if (indices != nullptr)
    {
        indices->clear();
        if (normals != nullptr)
            normals->clear();
        if (texCoords != nullptr)
            texCoords->clear();
        std::map<int, std::vector<int>> imap;
        for (size_t i = 0; i < nind.size() / 3; i++)
        {
            int ind[3] = {nind[3 * i + 0], nind[3 * i + 1], nind[3 * i + 2]};
            auto it = imap.find(ind[0]);
            bool add = true;
            if (it == imap.end())
                imap[ind[0]] = std::vector<int>();
            else
            {
                for (size_t j = 0; j < imap[ind[0]].size() / 2; j++)
                {
                    if ((imap[ind[0]][2 * j + 0] == ind[1]) && (imap[ind[0]][2 * j + 1] == ind[2]))
                    {
                        add = false;
                        break;
                    }
                    if ((imap[ind[0]][2 * j + 0] == ind[2]) && (imap[ind[0]][2 * j + 1] == ind[1]))
                    {
                        add = false;
                        break;
                    }
                }
            }
            if (add)
            {
                imap[ind[0]].push_back(ind[1]);
                imap[ind[0]].push_back(ind[2]);
                indices->push_back(ind[0]);
                indices->push_back(ind[1]);
                indices->push_back(ind[2]);
                if (normals != nullptr)
                {
                    for (size_t j = 0; j < 9; j++)
                        normals->push_back(nnorm[9 * i + j]);
                }
                if (texCoords != nullptr)
                {
                    for (size_t j = 0; j < 9; j++)
                        texCoords->push_back(ntex[6 * i + j]);
                }
            }
        }
    }
}

void CMeshRoutines::removeNonReferencedVertices(std::vector<double>& vertices, std::vector<int>& indices)
{
    std::vector<double> vertTmp(vertices);
    vertices.clear();
    std::vector<int> mapping(vertTmp.size() / 3, -1);
    int freeSlot = 0;
    for (size_t i = 0; i < indices.size(); i++)
    {
        if (mapping[indices[i]] == -1)
        {
            vertices.push_back(vertTmp[3 * indices[i] + 0]);
            vertices.push_back(vertTmp[3 * indices[i] + 1]);
            vertices.push_back(vertTmp[3 * indices[i] + 2]);
            mapping[indices[i]] = freeSlot;
            indices[i] = freeSlot;
            freeSlot++;
        }
        else
            indices[i] = mapping[indices[i]];
    }
}
