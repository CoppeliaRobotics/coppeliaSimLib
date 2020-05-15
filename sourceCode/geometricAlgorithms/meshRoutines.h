#pragma once

#include <vector>
#include "3Vector.h"

//FULLY STATIC CLASS
class CMeshRoutines  
{
public:
    static int convexDecompose(const float* vertices,int verticesLength,const int* indices,
                               int indicesLength,std::vector<std::vector<float>*>& verticesList,
                               std::vector<std::vector<int>*>& indicesList,size_t nClusters,
                               double concavity,bool addExtraDistPoints,bool addFacesPoints,
                               double ccConnectDist,size_t targetNTrianglesDecimatedMesh,
                               size_t maxHullVertices,double smallestClusterThreshold,
                               bool useHACD,int resolution_VHACD,int depth_VHACD_old,float concavity_VHACD,
                               int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                               float alpha_VHACD,float beta_VHACD,float gamma_VHACD_old,bool pca_VHACD,
                               bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,float minVolumePerCH_VHACD);

    static void getEdgeFeatures(float* vertices,int verticesLength,int* indices,int indicesLength,
            std::vector<int>* vertexIDs,std::vector<int>* edgeIDs,std::vector<int>* faceIDs,float angleTolerance,bool forDisplay,bool hideEdgeBorders);
    static bool getConvexHull(std::vector<float>* verticesInOut,std::vector<int>* indicesInOut);
    static bool getConvexHull(const std::vector<float>* verticesIn,std::vector<float>* verticesOut,std::vector<int>* indicesOut);
    static bool getConvexHull(const float* verticesIn,int verticesInLength,std::vector<float>* verticesOut,std::vector<int>* indicesOut);
    static bool getDecimatedMesh(const std::vector<float>& verticesIn,const std::vector<int>& indicesIn,float decimationPercentage,std::vector<float>& verticesOut,std::vector<int>& indicesOut);

    inline static bool getMinDistBetweenSegmentAndPoint_IfSmaller(const C3Vector& lp0,
                            const C3Vector& lv0,const C3Vector& dummyPos,float &dist,C3Vector& segA)
    {   // dist & segA are modified only if the distance is smaller than 'dist' which was input
        // Return value of true means: the dist is smaller
        // The segment can be degenerated (point)
        float d;
        if ((lv0(0)==0.0f)&&(lv0(1)==0.0f)&&(lv0(2)==0.0f))
        { // We have a degenerated segment here (point):
            C3Vector v(dummyPos-lp0);
            d=v.getLength();
            if (d<dist) 
            {
                dist=d;
                segA=lp0;
                return(true);
            }
            return(false);
        }
        // dist between lines described by segment and dummyPos
        float t=getMinDistPtBetweenPointAndLine(dummyPos,lp0,lv0);
        if ((t>=0.0f)&&(t<=1.0f))
        {
            C3Vector v(dummyPos-lp0-(lv0*t));
            d=v.getLength();
            if (d<dist)
            {
                dist=d;
                segA=lp0+lv0*t;
                return(true);
            }
            return(false);
        }
        // We have to compare point-point distances now
        C3Vector point(lp0-dummyPos);
        d=point.getLength();
        bool smaller=false;
        if (d<dist)
        {
            dist=d;
            segA=lp0;
            smaller=true;
        }
        point=lp0+lv0-dummyPos;
        d=point.getLength();
        if (d<dist)
        {
            dist=d;
            segA=lp0+lv0;
            smaller=true;
        }
        return(smaller);
    }

    inline static float getMinDistPtBetweenPointAndLine(const C3Vector& point,const C3Vector& lp,const C3Vector& lv)
    {
        C3Vector d(point-lp);
        return((d*lv)/(lv*lv));
    };

    static bool checkIfConvex(const std::vector<float>& vertices,const std::vector<int>& indices,float distanceToleranceInPercent);

    static void createCube(std::vector<float>& vertices,std::vector<int>& indices,const C3Vector& sizes,const int subdivisions[3]);
    static void createSphere(std::vector<float>& vertices,std::vector<int>& indices,const C3Vector& sizes,int sides,int faces);
    static void createCylinder(std::vector<float>& vertices,std::vector<int>& indices,const C3Vector& sizes,int sides,int faces,int discDiv,int openEnds,bool cone);
    static void createAnnulus(std::vector<float>& vertices,std::vector<int>& indices,float Dlarge,float Dsmall,float zShift,int sides,bool faceUp);

private:
    static void _insertEdge(std::vector<std::vector<int>* >& allEdges,int vertexIndex1,int vertexIndex2,int triangleIndex); // used for convex check
    static int _getTriangleIndexFromEdge(std::vector<std::vector<int>* >& allEdges,int vertexIndex1,int vertexIndex2,int triangleIndexToExclude); // used for convex check
    static bool _removeColinearTrianglePoints(std::vector<float>* vertices,std::vector<int>* indices,float toleranceDist);
};
