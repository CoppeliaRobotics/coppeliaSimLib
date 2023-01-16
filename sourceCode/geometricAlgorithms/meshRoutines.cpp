#include "simConst.h"
#include "meshRoutines.h"
#include "meshManip.h"
#include "tt.h"
#include "pluginContainer.h"
#include <stdio.h>
#include "simInternal.h"
#include <algorithm>
#include "app.h"

void CMeshRoutines::getEdgeFeatures(double* vertices,int verticesLength,int* indices,int indicesLength,
            std::vector<int>* theVertexIDs,std::vector<int>* theEdgeIDs,std::vector<int>* theFaceIDs,
            double angleTolerance,bool forDisplay,bool hideEdgeBorders)
{ // theVertexIDs, theEdgeIDs or theFaceIDs can be nullptr
    // For each vertex, edge and face, an identifier will be associated:
    // Same triangle ID --> triangles belong to the same face (with given tolerance between normals)
    // Same edge ID --> edges belong to the same edge (with given tolerance). -1 --> edge is embedded in a face 
    // Vertex ID is -1 --> vertex is embedded in an edge or face
    // If for Display is true, we calculate edges for nice display, otherwise, we calculate edges that can be used for the dynamic collision rersponse algorithms

    std::vector<int> vertexIDs(verticesLength/3,0);
    std::vector<int> edgeIDs(indicesLength,-2);
    std::vector<int> faceIDs(indicesLength/3,-1);


    // We use a CMeshManip-object for faster access:
    CMeshManip mesh(vertices,verticesLength,indices,indicesLength);
    // First we group similar triangles according to a max. tolerance angle:
    const double cosToleranceAngle=cos(angleTolerance);
    int faceIDCounter=0;
    for (int i=0;i<indicesLength/3;i++)
    {
        if (faceIDs[i]==-1)
        { // this triangle was not yet processed:
            C3Vector normalVect=mesh.faceNormals[i]; // We get its normal vector
            // We prepare recursion with the triangle itself (beginning):
            std::vector<int> neighbours;
            neighbours.push_back(i);
            // Now we recursively go through all its neighbours:
            while (neighbours.size()!=0)
            {
                int el=neighbours[neighbours.size()-1];
                neighbours.erase(neighbours.end()-1);
                faceIDs[el]=faceIDCounter;
                int indOr[3]={indices[3*el+0],indices[3*el+1],indices[3*el+2]};
                for (int ed=0;ed<3;ed++)
                { // We check the triangle's 3 edges:
                    int ind[3];
                    int ked=ed;
                    for (int led=0;led<3;led++)
                    {
                        ind[led]=indOr[ked];
                        ked++;
                        if (ked==3)
                            ked=0;
                    }
                    CEdgeElement* edgeIt=mesh.edges[ind[0]];
                    int neighbourCount=0;
                    int neighbourWithNoEdgeShowCount=0;
                    while (edgeIt!=nullptr)
                    { // We have to check all triangles with same edges:
                        if (edgeIt->vertex1==ind[1])
                        { // Same edge. We check if already processed:
                            if (i!=edgeIt->triangle)
                                neighbourCount++;
                            if ( (faceIDs[edgeIt->triangle]==-1) ||(forDisplay&&(i!=edgeIt->triangle)) )
                            { // Not yet processed! Is the angle between normals under tolerance?
                                C3Vector normalVect2=mesh.faceNormals[edgeIt->triangle];
                                if (normalVect.getAngle(normalVect2)<angleTolerance)
                                { // This triangle belongs to the same plane!
                                    neighbourWithNoEdgeShowCount++;
                                    if (!forDisplay)
                                        neighbours.push_back(edgeIt->triangle);
                                    // Now we have to mark the edge which is in common as non-existing (-1):
                                    edgeIDs[3*el+ed]=-1; // Edge from "From" triangle
                                    edgeIDs[3*edgeIt->triangle+edgeIt->pos]=-1; //Edge from "To" triangle
                                }
                            }
                        }
                        edgeIt=edgeIt->next;
                    }

                    if (hideEdgeBorders&&(neighbourCount==0))
                    {
                        edgeIDs[3*el+ed]=-1; // mark the edge as non-existing (-1)
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
    int edgeIDCounter=0;
    for (int i=0;i<int(mesh.edges.size());i++) // In fact we could go to the half only!
    {
        CEdgeElement* edgeIt=mesh.edges[i];
        while (edgeIt!=nullptr)
        {
            if (edgeIDs[3*edgeIt->triangle+edgeIt->pos]==-2)
            { // This edge exists and wasn't yet processed:
                C3Vector normalVect=edgeIt->n;
                // We prepare recursion with the edge itself (beginning):
                std::vector<int> neighbours;
                neighbours.push_back(3*edgeIt->triangle+edgeIt->pos);
                // Now we recursively go through all its neighbours:
                while (neighbours.size()!=0)
                {
                    int el=neighbours[neighbours.size()-1];
                    int thePos=el%3;
                    neighbours.erase(neighbours.end()-1);
                    edgeIDs[el]=edgeIDCounter;
                    // Now we check its neighbours on both sides:
                    int pointIDs[2];
                    pointIDs[0]=indices[el];
                    if (thePos==2)
                        pointIDs[1]=indices[el-2];
                    else
                        pointIDs[1]=indices[el+1];
                    for (int side=0;side<2;side++)
                    {
                        int vertexID=pointIDs[side];
                        CEdgeElement* edgeIt2=mesh.edges[vertexID];
                        while (edgeIt2!=nullptr)
                        {
                            if (edgeIDs[3*edgeIt2->triangle+edgeIt2->pos]==-2)
                            { // Wasn't processed yet
                                C3Vector normalVect2=edgeIt2->n;
                                double dd=normalVect*normalVect2; // Scalar product
                                if ( (dd>cosToleranceAngle)||(dd<-cosToleranceAngle) )
                                { // This segment belongs to the same edge!
                                    neighbours.push_back(3*edgeIt2->triangle+edgeIt2->pos);
                                    // Now we have to disable the vertex (-1), but only if the two
                                    // edges have different coordinates:
                                    if ( ((pointIDs[0]==edgeIt2->vertex0)&&(pointIDs[1]==edgeIt2->vertex1))||
                                        ((pointIDs[0]==edgeIt2->vertex1)&&(pointIDs[1]==edgeIt2->vertex0)) )
                                    {
                                    }
                                    else
                                        vertexIDs[vertexID]=-1;
                                }
                            }
                            edgeIt2=edgeIt2->next;
                        }
                    }
                }
                edgeIDCounter++;
            }
            edgeIt=edgeIt->next;
        }
    }
    // Now we have to do a last thing: disable all vertices which have only disabled edges:
    for (int i=0;i<verticesLength/3;i++)
    {
        CEdgeElement* edgeIt=mesh.edges[i];
        bool hasActiveEdge=false;
        while (edgeIt!=nullptr)
        {
            if (edgeIDs[3*edgeIt->triangle+edgeIt->pos]!=-1)
                hasActiveEdge=true;
            edgeIt=edgeIt->next;
        }
        if (!hasActiveEdge)
            vertexIDs[i]=-1; // We disable this point
    }

    if (theVertexIDs!=nullptr)
    {
        theVertexIDs->clear();
        theVertexIDs->assign(vertexIDs.begin(),vertexIDs.end());
    }
    if (theEdgeIDs!=nullptr)
    {
        theEdgeIDs->clear();
        theEdgeIDs->assign(edgeIDs.begin(),edgeIDs.end());
    }
    if (theFaceIDs!=nullptr)
    {
        theFaceIDs->clear();
        theFaceIDs->assign(faceIDs.begin(),faceIDs.end());
    }
}

bool CMeshRoutines::getConvexHull(std::vector<double>* verticesInOut,std::vector<int>* indicesInOut)
{
    std::vector<double> outV;
    std::vector<int> outI;
    bool result;
    if (indicesInOut!=nullptr)
        result=getConvexHull(verticesInOut,&outV,&outI);
    else
        result=getConvexHull(verticesInOut,&outV,nullptr);
    verticesInOut->clear();
    verticesInOut->assign(outV.begin(),outV.end());
    if (indicesInOut!=nullptr)
    {
        indicesInOut->clear();
        indicesInOut->assign(outI.begin(),outI.end());
    }
    return(result);
}

bool CMeshRoutines::getConvexHull(const std::vector<double>* verticesIn,std::vector<double>* verticesOut,std::vector<int>* indicesOut)
{
    return(getConvexHull(&verticesIn->at(0),(int)verticesIn->size(),verticesOut,indicesOut));
}

bool CMeshRoutines::getConvexHull(const double* verticesIn,int verticesInLength,std::vector<double>* verticesOut,std::vector<int>* indicesOut)
{
    void* data[10];
    data[0]=(double*)verticesIn;
    data[1]=&verticesInLength;
    bool generateIndices=true;
    if (indicesOut==nullptr)
        generateIndices=false;
    data[2]=&generateIndices;
    bool success=false;
    data[3]=&success;
    double* outVert=nullptr;
    data[4]=&outVert;
    int outVertLength;
    data[5]=&outVertLength;
    int* outInd=nullptr;
    data[6]=&outInd;
    int outIndLength;
    data[7]=&outIndLength;
    verticesOut->clear();
    if (indicesOut!=nullptr)
        indicesOut->clear();
    if (CPluginContainer::qhull(data))
    {
        if (success)
        {
            for (int i=0;i<outVertLength;i++)
                verticesOut->push_back(outVert[i]);
            delete[] outVert;
            if (indicesOut!=nullptr)
            {
                for (int i=0;i<outIndLength;i++)
                    indicesOut->push_back(outInd[i]);
                delete[] outInd;
            }

            C3Vector minV,maxV;
            for (size_t i=0;i<verticesOut->size()/3;i++)
            {
                C3Vector v(&verticesOut->at(3*i));
                if (i==0)
                {
                    minV=v;
                    maxV=v;
                }
                else
                {
                    minV.keepMin(v);
                    maxV.keepMax(v);
                }
            }

            C3Vector dim(maxV-minV);

            // We merge close vertices, in order to have less problems with tolerances (1% of the dimension of the hull):
            CMeshManip::checkVerticesIndicesNormalsTexCoords(*verticesOut,*indicesOut,nullptr,nullptr,true,(dim(0)+dim(1)+dim(2))*0.001/3.0,false);
            if ( (verticesOut->size()>=12)&&checkIfConvex(*verticesOut,*indicesOut,0.001) ) // 0.1%
                return(true);
        }
    }
    else
        App::logMsg(sim_verbosity_errors,"Qhull failed. Is the Qhull plugin loaded?");

    return(false);
}

bool CMeshRoutines::getDecimatedMesh(const std::vector<double>& verticesIn,const std::vector<int>& indicesIn,double decimationPercentage,std::vector<double>& verticesOut,std::vector<int>& indicesOut)
{
    bool retVal=false;
    if ( (verticesIn.size()>=9)&&(indicesIn.size()>=6) )
    {
        void* data[20];
        data[0]=(double*)&verticesIn[0];
        int vl=(int)verticesIn.size();
        data[1]=&vl;
        data[2]=(int*)&indicesIn[0];
        int il=(int)indicesIn.size();
        data[3]=&il;
        data[4]=&decimationPercentage;
        int version=0;
        data[5]=&version;
        bool success=false;
        data[6]=&success;
        double* outVert=nullptr;
        data[7]=&outVert;
        int outVertLength;
        data[8]=&outVertLength;
        int* outInd=nullptr;
        data[9]=&outInd;
        int outIndLength;
        data[10]=&outIndLength;
        verticesOut.clear();
        indicesOut.clear();
        if (CPluginContainer::meshDecimator(data))
        {
            if (success)
            {
                for (int i=0;i<outVertLength;i++)
                    verticesOut.push_back(outVert[i]);
                delete[] outVert;
                for (int i=0;i<outIndLength;i++)
                    indicesOut.push_back(outInd[i]);
                delete[] outInd;
                retVal=true;
            }
        }
        else
            App::logMsg(sim_verbosity_errors,"mesh decimation failed. Is the OpenMesh plugin loaded?");
    }
    return(retVal);
}



int CMeshRoutines::convexDecompose(const double* vertices,int verticesLength,const int* indices,int indicesLength,
                                   std::vector<std::vector<double>*>& verticesList,std::vector<std::vector<int>*>& indicesList,
                                   size_t nClusters,double concavity,bool addExtraDistPoints,bool addFacesPoints,
                                   double ccConnectDist,size_t targetNTrianglesDecimatedMesh,size_t maxHullVertices,
                                   double smallestClusterThreshold,bool useHACD,int resolution_VHACD,int depth_VHACD_old,double concavity_VHACD,
                                   int planeDownsampling_VHACD,int convexHullDownsampling_VHACD,
                                   double alpha_VHACD,double beta_VHACD,double gamma_VHACD_old,bool pca_VHACD,
                                   bool voxelBased_VHACD,int maxVerticesPerCH_VHACD,double minVolumePerCH_VHACD)
{ // 2 100 0 1 1 30 2000
    TRACE_INTERNAL;
    void* data[30];
    int el=0;
    double** vertList=nullptr;
    int** indList=nullptr;
    int* vertCountList=nullptr;
    int* indCountList=nullptr;

    data[0]=(double*)vertices;
    data[1]=&verticesLength;
    data[2]=(int*)indices;
    data[3]=&indicesLength;
    if (useHACD)
    {
        data[4]=&nClusters;
        data[5]=&concavity;
        data[6]=&addExtraDistPoints;
        data[7]=&addFacesPoints;
        data[8]=&ccConnectDist;
        data[9]=&targetNTrianglesDecimatedMesh;
        data[10]=&maxHullVertices;
        data[11]=&smallestClusterThreshold;
        data[12]=&el;
        data[13]=&vertList;
        data[14]=&indList;
        data[15]=&vertCountList;
        data[16]=&indCountList;
        CPluginContainer::hacd(data);
    }
    else
    {
        data[4]=&resolution_VHACD;
        data[5]=&depth_VHACD_old;
        data[6]=&concavity_VHACD;
        data[7]=&planeDownsampling_VHACD;
        data[8]=&convexHullDownsampling_VHACD;
        data[9]=&alpha_VHACD;
        data[10]=&beta_VHACD;
        data[11]=&gamma_VHACD_old;
        data[12]=&pca_VHACD;
        data[13]=&voxelBased_VHACD;
        data[14]=&maxVerticesPerCH_VHACD;
        data[15]=&minVolumePerCH_VHACD;
        data[16]=&el;
        data[17]=&vertList;
        data[18]=&indList;
        data[19]=&vertCountList;
        data[20]=&indCountList;
        CPluginContainer::vhacd(data);
    }

    for (int mesh=0;mesh<el;mesh++)
    {
        std::vector<double>* _vert=new std::vector<double>;
        std::vector<int>* _ind=new std::vector<int>;
        for (int i=0;i<vertCountList[mesh];i++)
            _vert->push_back(vertList[mesh][i]);
        for (int i=0;i<indCountList[mesh];i++)
            _ind->push_back(indList[mesh][i]);
        delete[] vertList[mesh];
        delete[] indList[mesh];

        getConvexHull(_vert,_ind); // better results with that! (convex decomp. routine has large tolerance regarding convexivity)

        // We do some checkings on our own here, just in case:
        C3Vector mmin,mmax;
        for (int i=0;i<int(_vert->size()/3);i++)
        {
            C3Vector v(&_vert->at(3*i+0));
            if (i==0)
            {
                mmin=v;
                mmax=v;
            }
            else
            {
                mmin.keepMin(v);
                mmax.keepMax(v);
            }
        }
        C3Vector extent(mmax-mmin);
        if ((_ind->size()>=12)&&(std::min<double>(std::min<double>(extent(0),extent(1)),extent(2))>0.0001))
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
    if (el>0)
    {
        delete[] indCountList;
        delete[] vertCountList;
        delete[] indList;
        delete[] vertList;
    }
    return((int)verticesList.size());
}

void CMeshRoutines::_insertEdge(std::vector<std::vector<int>* >& allEdges,int vertexIndex1,int vertexIndex2,int triangleIndex)
{
    int minI=std::min<int>(vertexIndex1,vertexIndex2);
    int maxI=std::max<int>(vertexIndex1,vertexIndex2);
    if (allEdges[minI]==nullptr)
        allEdges[minI]=new std::vector<int>;
    allEdges[minI]->push_back(maxI);
    allEdges[minI]->push_back(triangleIndex);
}

int CMeshRoutines::_getTriangleIndexFromEdge(std::vector<std::vector<int>* >& allEdges,int vertexIndex1,int vertexIndex2,int triangleIndexToExclude)
{ // returns -1 if there is no such triangle, or more than 1
    int minI=std::min<int>(vertexIndex1,vertexIndex2);
    int maxI=std::max<int>(vertexIndex1,vertexIndex2);
    if (allEdges[minI]==nullptr)
        return(-1);
    int cnt=0;
    int retVal=-1;
    for (int i=0;i<int(allEdges[minI]->size())/2;i++)
    {
        if ( (maxI==allEdges[minI]->at(2*i+0))&&(triangleIndexToExclude!=allEdges[minI]->at(2*i+1)) )
        {
            cnt++;
            retVal=allEdges[minI]->at(2*i+1);
        }
    }
    if (cnt==1)
        return(retVal);
    return(-1);
}

bool CMeshRoutines::checkIfConvex(const std::vector<double>& vertices,const std::vector<int>& indices,double distanceToleranceInPercent)
{
    // We need to check if neighbouring triangle are in a convex config, and if all triangles share an edge with exactly another triangle
    // Finally, we also need to check if the shape is in fact two (or more) convex shapes (i.e. 2 merged convex shapes) (only for test a) )
    bool convex=true;
    // 1. build a fast index:
    std::vector<std::vector<int>* > allEdges(vertices.size()/3,nullptr);
    for (int tri=0;tri<int(indices.size()/3);tri++)
    {
        int ind[3]={indices[3*tri+0],indices[3*tri+1],indices[3*tri+2]};
        _insertEdge(allEdges,ind[0],ind[1],tri);
        _insertEdge(allEdges,ind[1],ind[2],tri);
        _insertEdge(allEdges,ind[2],ind[0],tri);
    }


    // 2. Now check each triangle for connectivity (exactly 3 neighbours) and for convexity:
    // We check for a maximum distance with a vertex outside of the half-spaces defining the convex shape

    C3Vector minV,maxV;
    for (int i=0;i<int(vertices.size()/3);i++)
    {
        C3Vector v(&vertices[3*i]);
        if (i==0)
        {
            minV=v;
            maxV=v;
        }
        else
        {
            minV.keepMin(v);
            maxV.keepMax(v);
        }
    }
    C3Vector boxDim(maxV-minV);
    double toleratedDist=distanceToleranceInPercent*(boxDim(0)+boxDim(1)+boxDim(2))/3.0;
    std::vector<double> planeDefinitions;

    for (int tri=0;tri<int(indices.size()/3);tri++)
    {
        int ind[3]={indices[3*tri+0],indices[3*tri+1],indices[3*tri+2]};
        C3Vector p0(&vertices[3*ind[0]]);
        C3Vector p1(&vertices[3*ind[1]]);
        C3Vector p2(&vertices[3*ind[2]]);
        C3Vector v0(p1-p0);
        C3Vector v1(p2-p0);
        C3Vector v2(p2-p1);
        C3Vector n0(v0^v1);
        C3Vector n1(v2^(v1*-1.0));
        C3Vector n2(v1^v2);
        n0=n0.getNormalized()+n1.getNormalized()+n2.getNormalized();
        n0.normalize();
        double d=n0*((p0+p1+p2)*(0.33333333));
        planeDefinitions.push_back(n0(0));
        planeDefinitions.push_back(n0(1));
        planeDefinitions.push_back(n0(2));
        planeDefinitions.push_back(d);

        // Check all 3 neighbouring triangles:
        for (int i=0;i<3;i++)
        {
            int j=i+1;
            if (j>2)
                j=0;
            int tri2=_getTriangleIndexFromEdge(allEdges,ind[i],ind[j],tri);
            if (tri2==-1)
            { // the mesh is not closed!
                convex=false;
                break;
            }
        }
        if (!convex)
            break; // the test already failed
    }

    if (convex)
    {
        for (int vert=0;vert<int(vertices.size()/3);vert++)
        {
            C3Vector v(&vertices[3*vert+0]);
            for (int i=0;i<int(planeDefinitions.size()/4);i++)
            {
                double d=v(0)*planeDefinitions[4*i+0]+v(1)*planeDefinitions[4*i+1]+v(2)*planeDefinitions[4*i+2]-planeDefinitions[4*i+3];
                if (d>toleratedDist)
                {
                    convex=false;
                    break;
                }
            }
            if (!convex)
                break; // the test already failed
        }
    }

    // 3. Check if the shape contains 2 (or more) items:
    // Might be important for cases where two identical shapes are coincident
    if (convex)
    {
        std::vector<bool> allTriangles(int(indices.size()/3),false);
        std::vector<int> trianglesToExplore;
        trianglesToExplore.push_back(0);
        allTriangles[0]=true;
        int triCount=1;
        while (trianglesToExplore.size()!=0)
        { // iterative exploration
            int tri=trianglesToExplore[trianglesToExplore.size()-1];
            trianglesToExplore.pop_back();
            int ind[3]={indices[3*tri+0],indices[3*tri+1],indices[3*tri+2]};
            // Check all 3 neighbouring triangles:
            for (int i=0;i<3;i++)
            {
                int j=i+1;
                if (j>2)
                    j=0;
                int tri2=_getTriangleIndexFromEdge(allEdges,ind[i],ind[j],tri);
                if (!allTriangles[tri2])
                { // we haven't explored that one yet!
                    triCount++;
                    allTriangles[tri2]=true;
                    trianglesToExplore.push_back(tri2);
                }
            }   
        }
        convex=(triCount==int(indices.size()/3));
    }

    // 4. Clean-up!
    for (int i=0;i<int(allEdges.size());i++)
    {
        if (allEdges[i]!=nullptr)
            delete allEdges[i];
    }
    return(convex);
}

void CMeshRoutines::createCube(std::vector<double>& vertices,std::vector<int>& indices,const C3Vector& sizes,const int subdivisions[3])
{
    vertices.clear();
    indices.clear();
    int divX=subdivisions[0];
    int divY=subdivisions[1];
    int divZ=subdivisions[2];
    double xhSize=sizes(0)/2.0;
    double yhSize=sizes(1)/2.0;
    double zhSize=sizes(2)/2.0;
    double xs=sizes(0)/((double)divX);
    double ys=sizes(1)/((double)divY);
    double zs=sizes(2)/((double)divZ);

    // We first create the vertices:
    //******************************
    // Plane1:
    for (int i=0;i<(divY+1);i++)
    { // along y
        for (int j=0;j<(divX+1);j++)
        { // along x
            tt::addToFloatArray(&vertices,-xhSize+j*xs,-yhSize+i*ys,-zhSize);   
        }
    }
    // Plane2:
    for (int i=1;i<(divZ+1);i++)
    { // along z
        for (int j=0;j<(divX+1);j++)
        { // along x
            tt::addToFloatArray(&vertices,-xhSize+j*xs,-yhSize,-zhSize+i*zs);   
        }
    }
    // Plane3:
    for (int i=1;i<(divZ+1);i++)
    { // along z
        for (int j=0;j<(divX+1);j++)
        { // along x
            tt::addToFloatArray(&vertices,-xhSize+j*xs,+yhSize,-zhSize+i*zs);   
        }
    }
    // Plane4:
    for (int i=1;i<divY;i++)
    { // along y
        for (int j=0;j<(divX+1);j++)
        { // along x
            tt::addToFloatArray(&vertices,-xhSize+j*xs,-yhSize+i*ys,+zhSize);   
        }
    }
    // Plane5:
    for (int i=1;i<divY;i++)
    { // along y
        for (int j=1;j<divZ;j++)
        { // along z
            tt::addToFloatArray(&vertices,-xhSize,-yhSize+i*ys,-zhSize+j*zs);   
        }
    }
    // Plane6:
    for (int i=1;i<divY;i++)
    { // along y
        for (int j=1;j<divZ;j++)
        { // along z
            tt::addToFloatArray(&vertices,+xhSize,-yhSize+i*ys,-zhSize+j*zs);   
        }
    }
    
    // Now we create the indices:
    //***************************
    // Plane1:
    for (int i=0;i<divY;i++)
    { // along y
        for (int j=0;j<divX;j++)
        { // along x
            tt::addToIntArray(&indices,i*(divX+1)+j+1,i*(divX+1)+j,(i+1)*(divX+1)+j+1);
            tt::addToIntArray(&indices,(i+1)*(divX+1)+j+1,i*(divX+1)+j,(i+1)*(divX+1)+j);
        }
    }
    // Plane2:
    int off=divY*(divX+1)+divX+1;
    for (int i=0;i<divX;i++)
    { // along x
        tt::addToIntArray(&indices,i,i+1,off+i+1);
        tt::addToIntArray(&indices,i,off+i+1,off+i);
    }
    off=divY*(divX+1);
    for (int i=1;i<divZ;i++)
    { // along y
        for (int j=0;j<divX;j++)
        { // along x
            tt::addToIntArray(&indices,off+i*(divX+1)+j,off+i*(divX+1)+j+1,off+(i+1)*(divX+1)+j+1);
            tt::addToIntArray(&indices,off+i*(divX+1)+j,off+(i+1)*(divX+1)+j+1,off+(i+1)*(divX+1)+j);
        }
    }
    // Plane3:
    int oldOff=divY*(divX+1);
    off=(divY+1)*(divX+1)+divZ*(divX+1);
    for (int i=0;i<divX;i++)
    { // along x
        tt::addToIntArray(&indices,oldOff+i+1,oldOff+i,off+i+1);
        tt::addToIntArray(&indices,off+i+1,oldOff+i,off+i);
    }
    off=(divY+1)*(divX+1)+(divZ-1)*(divX+1);
    for (int i=1;i<divZ;i++)
    { // along y
        for (int j=0;j<divX;j++)
        { // along x
            tt::addToIntArray(&indices,off+i*(divX+1)+j+1,off+i*(divX+1)+j,off+(i+1)*(divX+1)+j+1);
            tt::addToIntArray(&indices,off+(i+1)*(divX+1)+j+1,off+i*(divX+1)+j,off+(i+1)*(divX+1)+j);
        }
    }
    // Plane4:
    oldOff=(divY+1)*(divX+1)+divZ*(divX+1)+(divZ-1)*(divX+1);
    if (divY>=2)
        off=(divY+1)*(divX+1)+2*divZ*(divX+1)+(divY-2)*(divX+1);
    else
        off=(divY+1)*(divX+1)+(divZ-1)*(divX+1);
    for (int i=0;i<divX;i++)
    { // along x
        tt::addToIntArray(&indices,oldOff+i+1,oldOff+i,off+i+1);
        tt::addToIntArray(&indices,off+i+1,oldOff+i,off+i);
    }
    if (divY>=2)
    {
        oldOff=(divY+1)*(divX+1)+2*divZ*(divX+1);
        off=(divY+1)*(divX+1)+(divZ-1)*(divX+1);
        for (int i=0;i<divX;i++)
        { // along x
            tt::addToIntArray(&indices,oldOff+i+1,oldOff+i,off+i+1);
            tt::addToIntArray(&indices,off+i+1,oldOff+i,off+i);
        }
    }
    off=(divY+1)*(divX+1)+2*divZ*(divX+1)-(divX+1);
    for (int i=1;i<divY-1;i++)
    { // along y
        for (int j=0;j<divX;j++)
        { // along x
            tt::addToIntArray(&indices,off+i*(divX+1)+j,off+i*(divX+1)+j+1,off+(i+1)*(divX+1)+j+1);
            tt::addToIntArray(&indices,off+i*(divX+1)+j,off+(i+1)*(divX+1)+j+1,off+(i+1)*(divX+1)+j);
        }
    }
    // Plane5:
    if (divZ==1)
    { // No vertical division
        if (divY==1)
        { // No division in Y -> here we have only one face
            tt::addToIntArray(&indices,(divX+1)*divY,0,(divY+1)*(divX+1)+2*divZ*(divX+1)-divX-1);   
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+2*divZ*(divX+1)-divX-1,0,(divY+1)*(divX+1)+(divZ-1)*(divX+1)); 
        }
        else
        {
            tt::addToIntArray(&indices,(divX+1)*divY,(divX+1)*(divY-1),(divY+1)*(divX+1)+2*divZ*(divX+1)-divX-1);   
            tt::addToIntArray(&indices,(divX+1)*(divY+1)+2*divZ*(divX+1)+(divY-2)*(divX+1),(divY+1)*(divX+1)+2*divZ*(divX+1)-divX-1,(divX+1)*(divY-1)); 
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+2*divZ*(divX+1),0,(divY+1)*(divX+1)+(divZ-1)*(divX+1));    
            tt::addToIntArray(&indices,divX+1,0,(divY+1)*(divX+1)+2*divZ*(divX+1)); 
        }
        int up=(divY+1)*(divX+1)+2*divZ*(divX+1);
        int down=divX+1;
        for (int i=0;i<divY-2;i++)
        {
            tt::addToIntArray(&indices,down+i*(divX+1),up+i*(divX+1),up+(i+1)*(divX+1));    
            tt::addToIntArray(&indices,down+(i+1)*(divX+1),down+i*(divX+1),up+(i+1)*(divX+1));  
        }
    }
    else
    { // Vertical division
        if (divY==1)
        { // No division in Y
            tt::addToIntArray(&indices,divY*(divX+1),0,(divY+1)*(divX+1));  
            tt::addToIntArray(&indices,divY*(divX+1),(divY+1)*(divX+1),(divY+1)*(divX+1)+divZ*(divX+1));    
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+2*divZ*(divX+1)-2*(divX+1),(divY+1)*(divX+1)+(divZ-1)*(divX+1),(divY+1)*(divX+1)+2*divZ*(divX+1)-(divX+1));    
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+(divZ-1)*(divX+1),(divY+1)*(divX+1)+2*divZ*(divX+1)-2*(divX+1),(divY+1)*(divX+1)+(divZ-2)*(divX+1));   
            int left=(divY+1)*(divX+1);
            int right=(divY+1)*(divX+1)+divZ*(divX+1);
            for (int i=0;i<divZ-2;i++)
            {
                tt::addToIntArray(&indices,right+i*(divX+1),left+i*(divX+1),left+(i+1)*(divX+1));   
                tt::addToIntArray(&indices,right+(i+1)*(divX+1),right+i*(divX+1),left+(i+1)*(divX+1));  
            }
        }
        else
        { // Here we have the very general case
            // The corners first:
            int pos=(divY+1)*(divX+1)+2*divZ*(divX+1)+(divY-1)*(divX+1);
            int posInt=(divY+1)*(divX+1)+2*divZ*(divX+1);
            tt::addToIntArray(&indices,divX+1,0,(divY+1)*(divX+1)); 
            tt::addToIntArray(&indices,divX+1,(divY+1)*(divX+1),pos);   
            tt::addToIntArray(&indices,divY*(divX+1),(divY-1)*(divX+1),pos+(divY-2)*(divZ-1));  
            tt::addToIntArray(&indices,divY*(divX+1),pos+(divY-2)*(divZ-1),(divY+1)*(divX+1)+divZ*(divX+1));    
            tt::addToIntArray(&indices,pos+divZ-2,(divY+1)*(divX+1)+(divZ-2)*(divX+1),(divY+1)*(divX+1)+(divZ-1)*(divX+1));
            tt::addToIntArray(&indices,pos+divZ-2,(divY+1)*(divX+1)+(divZ-1)*(divX+1),posInt);
            tt::addToIntArray(&indices,posInt-2*(divX+1),pos+(divZ-1)*(divY-1)-1,posInt+(divY-2)*(divX+1));
            tt::addToIntArray(&indices,posInt-2*(divX+1),posInt+(divY-2)*(divX+1),posInt-(divX+1));
            // The down and up part:
            int down1=divX+1;
            int down2=pos;
            int up1=pos+divZ-2;
            int up2=posInt;
            for (int i=0;i<divY-2;i++)
            {
                tt::addToIntArray(&indices,down1+(i+1)*(divX+1),down1+i*(divX+1),down2+i*(divZ-1)); 
                tt::addToIntArray(&indices,down1+(i+1)*(divX+1),down2+i*(divZ-1),down2+(i+1)*(divZ-1)); 
                tt::addToIntArray(&indices,up1+(i+1)*(divZ-1),up1+i*(divZ-1),up2+i*(divX+1));   
                tt::addToIntArray(&indices,up1+(i+1)*(divZ-1),up2+i*(divX+1),up2+(i+1)*(divX+1));   
            }
            // The left and right part:
            int left1=(divY+1)*(divX+1);
            int left2=pos;
            int right1=pos+(divZ-1)*(divY-2);
            int right2=(divY+1)*(divX+1)+divZ*(divX+1);
            for (int i=0;i<divZ-2;i++)
            {
                tt::addToIntArray(&indices,left1+i*(divX+1),left1+(i+1)*(divX+1),left2+i);  
                tt::addToIntArray(&indices,left2+i,left1+(i+1)*(divX+1),left2+i+1); 
                tt::addToIntArray(&indices,right1+i,right1+i+1,right2+i*(divX+1));  
                tt::addToIntArray(&indices,right2+i*(divX+1),right1+i+1,right2+(i+1)*(divX+1)); 
            }
        }
    }
    off=(divY+1)*(divX+1)+2*divZ*(divX+1)+(divY-1)*(divX+1);
    for (int i=0;i<divY-2;i++)
    { // along y
        for (int j=0;j<divZ-2;j++)
        { // along z
            tt::addToIntArray(&indices,off+i*(divZ-1)+j,off+i*(divZ-1)+j+1,off+(i+1)*(divZ-1)+j);
            tt::addToIntArray(&indices,off+i*(divZ-1)+j+1,off+(i+1)*(divZ-1)+j+1,off+(i+1)*(divZ-1)+j);
        }
    }

    // Plane6:
    if (divZ==1)
    { // No vertical division
        if (divY==1)
        { // No division in Y -> here we have only one face
            tt::addToIntArray(&indices,divX,(divX+1)*divY+divX,(divY+1)*(divX+1)+2*divZ*(divX+1)-1);    
            tt::addToIntArray(&indices,divX,(divY+1)*(divX+1)+2*divZ*(divX+1)-1,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX);  
        }
        else
        {
            tt::addToIntArray(&indices,(divX+1)*(divY-1)+divX,(divX+1)*divY+divX,(divY+1)*(divX+1)+2*divZ*(divX+1)-1);  
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+2*divZ*(divX+1)-1,(divX+1)*(divY+1)+2*divZ*(divX+1)+(divY-2)*(divX+1)+divX,(divX+1)*(divY-1)+divX);    
            tt::addToIntArray(&indices,divX,(divY+1)*(divX+1)+2*divZ*(divX+1)+divX,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX);   
            tt::addToIntArray(&indices,divX,2*divX+1,(divY+1)*(divX+1)+2*divZ*(divX+1)+divX);   
        }
        int up=(divY+1)*(divX+1)+2*divZ*(divX+1)+divX;
        int down=2*divX+1;
        for (int i=0;i<divY-2;i++)
        {
            tt::addToIntArray(&indices,up+i*(divX+1),down+i*(divX+1),up+(i+1)*(divX+1));    
            tt::addToIntArray(&indices,down+i*(divX+1),down+(i+1)*(divX+1),up+(i+1)*(divX+1));  
        }
    }
    else
    { // Vertical division
        if (divY==1)
        { // No division in Y
            tt::addToIntArray(&indices,divX,divY*(divX+1)+divX,(divY+1)*(divX+1)+divX); 
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+divX,divY*(divX+1)+divX,(divY+1)*(divX+1)+divZ*(divX+1)+divX); 
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX,(divY+1)*(divX+1)+2*divZ*(divX+1)-2*(divX+1)+divX,(divY+1)*(divX+1)+2*divZ*(divX+1)-1); 
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+2*divZ*(divX+1)-2*(divX+1)+divX,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX,(divY+1)*(divX+1)+(divZ-2)*(divX+1)+divX);    
            int left=(divY+1)*(divX+1)+divX;
            int right=(divY+1)*(divX+1)+divZ*(divX+1)+divX;
            for (int i=0;i<divZ-2;i++)
            {
                tt::addToIntArray(&indices,left+i*(divX+1),right+i*(divX+1),left+(i+1)*(divX+1));   
                tt::addToIntArray(&indices,right+i*(divX+1),right+(i+1)*(divX+1),left+(i+1)*(divX+1));  
            }
        }
        else
        { // Here we have the very general case
            // The corners first:
            int pos=(divY+1)*(divX+1)+2*divZ*(divX+1)+(divY-1)*(divX+1)+(divY-1)*(divZ-1);
            int posInt=(divY+1)*(divX+1)+2*divZ*(divX+1)+divX;
            tt::addToIntArray(&indices,divX,2*divX+1,(divY+1)*(divX+1)+divX);   
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+divX,2*divX+1,pos);    
            tt::addToIntArray(&indices,(divY-1)*(divX+1)+divX,divY*(divX+1)+divX,pos+(divY-2)*(divZ-1));    
            tt::addToIntArray(&indices,pos+(divY-2)*(divZ-1),divY*(divX+1)+divX,(divY+1)*(divX+1)+divZ*(divX+1)+divX);  
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+(divZ-2)*(divX+1)+divX,pos+divZ-2,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX);
            tt::addToIntArray(&indices,(divY+1)*(divX+1)+(divZ-1)*(divX+1)+divX,pos+divZ-2,posInt);
            tt::addToIntArray(&indices,pos+(divZ-1)*(divY-1)-1,posInt-2*(divX+1),posInt+(divY-2)*(divX+1));
            tt::addToIntArray(&indices,posInt+(divY-2)*(divX+1),posInt-2*(divX+1),posInt-(divX+1));
            // The down and up part:
            int down1=2*divX+1;
            int down2=pos;
            int up1=pos+divZ-2;
            int up2=posInt;
            for (int i=0;i<divY-2;i++)
            {
                tt::addToIntArray(&indices,down1+i*(divX+1),down1+(i+1)*(divX+1),down2+i*(divZ-1)); 
                tt::addToIntArray(&indices,down2+i*(divZ-1),down1+(i+1)*(divX+1),down2+(i+1)*(divZ-1)); 
                tt::addToIntArray(&indices,up1+i*(divZ-1),up1+(i+1)*(divZ-1),up2+i*(divX+1));   
                tt::addToIntArray(&indices,up2+i*(divX+1),up1+(i+1)*(divZ-1),up2+(i+1)*(divX+1));   
            }
            // The left and right part:
            int left1=(divY+1)*(divX+1)+divX;
            int left2=pos;
            int right1=pos+(divZ-1)*(divY-2);
            int right2=(divY+1)*(divX+1)+divZ*(divX+1)+divX;
            for (int i=0;i<divZ-2;i++)
            {
                tt::addToIntArray(&indices,left1+(i+1)*(divX+1),left1+i*(divX+1),left2+i);  
                tt::addToIntArray(&indices,left1+(i+1)*(divX+1),left2+i,left2+i+1); 
                tt::addToIntArray(&indices,right1+i+1,right1+i,right2+i*(divX+1));  
                tt::addToIntArray(&indices,right1+i+1,right2+i*(divX+1),right2+(i+1)*(divX+1)); 
            }
        }
    }
    off=(divY+1)*(divX+1)+2*divZ*(divX+1)+(divY-1)*(divX+1)+(divZ-1)*(divY-1);
    for (int i=0;i<divY-2;i++)
    { // along y
        for (int j=0;j<divZ-2;j++)
        { // along z
            tt::addToIntArray(&indices,off+i*(divZ-1)+j+1,off+i*(divZ-1)+j,off+(i+1)*(divZ-1)+j);
            tt::addToIntArray(&indices,off+(i+1)*(divZ-1)+j+1,off+i*(divZ-1)+j+1,off+(i+1)*(divZ-1)+j);
        }
    }
}

void CMeshRoutines::createCapsule(std::vector<double>& vertices,std::vector<int>& indices,const C3Vector& sizes,int sides,int faceSubdiv)
{ // sizes[0]&sizes[1]: diameters, sizes[2]: tube length
    vertices.clear();
    indices.clear();
    double xhSize=sizes(0)/2.0;
    double yhSize=sizes(1)/2.0;
    double zhSize=sizes(2)/2.0;

    double sa=2.0*piValue/((double)sides);
    int ff=sides/2;
    double fa=piValD2/((double)ff);

    double rhSize=std::max<double>(xhSize,yhSize);

    // We set up the vertices:
    tt::addToFloatArray(&vertices,0.0,0.0,rhSize+zhSize);
    tt::addToFloatArray(&vertices,0.0,0.0,-rhSize-zhSize);
    for (int i=0;i<sides;i++)
    {
        for (int j=1;j<=ff;j++)
            tt::addToFloatArray(&vertices,xhSize*sin(fa*j)*cos(sa*i),yhSize*sin(fa*j)*sin(sa*i),zhSize+rhSize*cos(fa*j));

        for (int j=1;j<=faceSubdiv;j++)
            tt::addToFloatArray(&vertices,xhSize*cos(sa*i),yhSize*sin(sa*i),zhSize-j*sizes(2)/(faceSubdiv+1));

        for (int j=ff;j>0;j--)
            tt::addToFloatArray(&vertices,xhSize*sin(fa*j)*cos(sa*i),yhSize*sin(fa*j)*sin(sa*i),-zhSize-rhSize*cos(fa*j));
    }

    int off1=2;
    int off2=2*ff+faceSubdiv;
    
    // We set up the indices:
    for (int i=0;i<sides-1;i++)
    {
        tt::addToIntArray(&indices,0,i*off2+off1,(i+1)*off2+off1);
        for (int j=0;j<ff-1;j++)
        {
            tt::addToIntArray(&indices,i*off2+off1+j,i*off2+off1+j+1,(i+1)*off2+off1+j);
            tt::addToIntArray(&indices,i*off2+off1+j+1,(i+1)*off2+off1+j+1,(i+1)*off2+off1+j);
        }
        for (int j=0;j<faceSubdiv+1;j++)
        {
            tt::addToIntArray(&indices,i*off2+off1+ff-1+j,i*off2+off1+ff-1+j+1,(i+1)*off2+off1+ff-1+j);
            tt::addToIntArray(&indices,i*off2+off1+ff-1+j+1,(i+1)*off2+off1+ff-1+j+1,(i+1)*off2+off1+ff-1+j);
        }
        for (int j=0;j<ff-1;j++)
        {
            tt::addToIntArray(&indices,i*off2+off1+ff+faceSubdiv+j,i*off2+off1+ff+faceSubdiv+j+1,(i+1)*off2+off1+ff+faceSubdiv+j);
            tt::addToIntArray(&indices,i*off2+off1+ff+faceSubdiv+j+1,(i+1)*off2+off1+ff+faceSubdiv+j+1,(i+1)*off2+off1+ff+faceSubdiv+j);
        }
        tt::addToIntArray(&indices,(i+1)*off2+off1-1,1,(i+2)*off2+off1-1);
    }

    int off3=off2*sides;
    // We close the capsule:
    tt::addToIntArray(&indices,0,off3+off1-off2,2);
    for (int j=0;j<ff-1;j++)
    {
        tt::addToIntArray(&indices,off3+off1-off2+j,off3+off1-off2+j+1,off1+j);
        tt::addToIntArray(&indices,off3+off1-off2+j+1,off1+j+1,off1+j);
    }
    for (int j=0;j<faceSubdiv+1;j++)
    {
        tt::addToIntArray(&indices,off3+off1-off2+ff-1+j,off3+off1-off2+ff-1+j+1,off1+ff-1+j);
        tt::addToIntArray(&indices,off3+off1-off2+ff-1+j+1,off1+ff-1+j+1,off1+ff-1+j);
    }
    for (int j=0;j<ff-1;j++)
    {
        tt::addToIntArray(&indices,off3+off1-off2+ff+faceSubdiv+j,off3+off1-off2+ff+faceSubdiv+j+1,off1+ff+faceSubdiv+j);
        tt::addToIntArray(&indices,off3+off1-off2+ff+faceSubdiv+j+1,off1+ff+faceSubdiv+j+1,off1+ff+faceSubdiv+j);
    }
    tt::addToIntArray(&indices,off3+off1-1,1,off2+off1-1);
}

void CMeshRoutines::createSphere(std::vector<double>& vertices,std::vector<int>& indices,const C3Vector& sizes,int sides,int faces)
{
    vertices.clear();
    indices.clear();
    double xhSize=sizes(0)/2.0;
    double yhSize=sizes(1)/2.0;
    double zhSize=sizes(2)/2.0;

    double sa=2.0*piValue/((double)sides);
    double fa=piValue/((double)faces);
    // We set up the vertices:
    tt::addToFloatArray(&vertices,0.0,0.0,1.0);
    tt::addToFloatArray(&vertices,0.0,0.0,-1.0);
    for (int i=0;i<sides;i++)
    {
        for (int j=1;j<faces;j++)
            tt::addToFloatArray(&vertices,(double)(sin(fa*j)*cos(sa*i)),(double)(sin(fa*j)*sin(sa*i)),(double)cos(fa*j));
    }

    // We set up the indices:
    for (int i=0;i<sides-1;i++)
    {
        // First top and bottom part:
        tt::addToIntArray(&indices,0,i*(faces-1)+2,(i+1)*(faces-1)+2);
        tt::addToIntArray(&indices,(i+1)*(faces-1)+1,1,(i+2)*(faces-1)+1);
        for (int j=0;j<faces-2;j++)
        { // Here the rest:
            tt::addToIntArray(&indices,i*(faces-1)+2+j,i*(faces-1)+2+j+1,(i+1)*(faces-1)+2+j);
            tt::addToIntArray(&indices,i*(faces-1)+2+j+1,(i+1)*(faces-1)+2+j+1,(i+1)*(faces-1)+2+j);
        }
    }
    // We have to close the sphere here:
    // First top and bottom part:
    tt::addToIntArray(&indices,0,(sides-1)*(faces-1)+2,2);
    tt::addToIntArray(&indices,sides*(faces-1)+1,1,(faces-1)+1);
    for (int j=0;j<faces-2;j++)
    { // Here the rest:
        tt::addToIntArray(&indices,(sides-1)*(faces-1)+2+j,(sides-1)*(faces-1)+2+j+1,2+j);
        tt::addToIntArray(&indices,(sides-1)*(faces-1)+2+j+1,2+j+1,2+j);
    }
    // Now we scale the sphere:
    for (int i=0;i<int(vertices.size())/3;i++)
    {
        C3Vector p(vertices[3*i+0],vertices[3*i+1],vertices[3*i+2]);
        p(0)=p(0)*xhSize;
        p(1)=p(1)*yhSize;
        p(2)=p(2)*zhSize;
        vertices[3*i+0]=p(0);
        vertices[3*i+1]=p(1);
        vertices[3*i+2]=p(2);
    }
}

void CMeshRoutines::createCylinder(std::vector<double>& vertices,std::vector<int>& indices,const C3Vector& sizes,int sides,int faces,int discDiv,bool openEnds,bool cone)
{
    vertices.clear();
    indices.clear();
    double xhSize=sizes(0)/2.0;
    double yhSize=sizes(1)/2.0;
    double zhSize=sizes(2)/2.0;

    double zzz=1.0/faces;
    double dd=1.0/((double)discDiv);
    double sa=2.0*piValue/((double)sides);
    int sideStart=0;
    // We set up the vertices:
    if (!openEnds)
    { // The two middle vertices:
        sideStart=2;
        tt::addToFloatArray(&vertices,0.0,0.0,0.5);
        tt::addToFloatArray(&vertices,0.0,0.0,-0.5);
    }

    if (cone)
    {
        for (int i=0;i<sides;i++)
        { // The side vertices:
            for (int j=0;j<faces+1;j++)
                tt::addToFloatArray(&vertices,(double)cos(sa*i)*j/faces,(double)sin(sa*i)*j/faces,0.5-j*zzz);
        }
    }
    else
    {
        for (int i=0;i<sides;i++)
        { // The side vertices:
            for (int j=0;j<faces+1;j++)
                tt::addToFloatArray(&vertices,(double)cos(sa*i),(double)sin(sa*i),0.5-j*zzz);
        }
    }

    int dstStart=(int)vertices.size()/3;
    int dsbStart=0;
    if (!openEnds)
    { // The disc subdivision vertices:
        for (int i=1;i<discDiv;i++)
            for (int j=0;j<sides;j++)
                tt::addToFloatArray(&vertices,(1.0-dd*i)*(double)cos(sa*j),(1.0-dd*i)*(double)sin(sa*j),0.5);
        dsbStart=(int)vertices.size()/3;
        for (int i=1;i<discDiv;i++)
            for (int j=0;j<sides;j++)
                tt::addToFloatArray(&vertices,(1.0-dd*i)*(double)cos(sa*j),(1.0-dd*i)*(double)sin(sa*j),-0.5);
    }


    // We set up the indices:
    for (int i=0;i<sides-1;i++)
    {
        // First top and bottom part:
        if (!openEnds)
        {
            if (discDiv==1)
            {
                tt::addToIntArray(&indices,0,i*(faces+1)+sideStart,(i+1)*(faces+1)+sideStart);
                tt::addToIntArray(&indices,(i+1)*(faces+1)+sideStart-1,1,(i+2)*(faces+1)+sideStart-1);
            }
            else
            {
                tt::addToIntArray(&indices,0,dstStart+i+sides*(discDiv-2),dstStart+i+sides*(discDiv-2)+1);
                tt::addToIntArray(&indices,dsbStart+i+sides*(discDiv-2),1,dsbStart+i+sides*(discDiv-2)+1);
                for (int j=0;j<discDiv-2;j++)
                {
                    tt::addToIntArray(&indices,dstStart+j*sides+i,dstStart+j*sides+i+1,dstStart+(j+1)*sides+i); 
                    tt::addToIntArray(&indices,dstStart+j*sides+i+1,dstStart+(j+1)*sides+i+1,dstStart+(j+1)*sides+i);   
                    tt::addToIntArray(&indices,dsbStart+j*sides+i+1,dsbStart+j*sides+i,dsbStart+(j+1)*sides+i); 
                    tt::addToIntArray(&indices,dsbStart+(j+1)*sides+i+1,dsbStart+j*sides+i+1,dsbStart+(j+1)*sides+i);   
                }
                tt::addToIntArray(&indices,sideStart+i*(faces+1),sideStart+(i+1)*(faces+1),dstStart+i);
                tt::addToIntArray(&indices,sideStart+(i+1)*(faces+1),dstStart+i+1,dstStart+i);
                tt::addToIntArray(&indices,sideStart+(i+2)*(faces+1)-1,sideStart+(i+1)*(faces+1)-1,dsbStart+i);
                tt::addToIntArray(&indices,dsbStart+i+1,sideStart+(i+2)*(faces+1)-1,dsbStart+i);
            }
        }
        for (int j=0;j<faces;j++)
        { // Here the rest:
            tt::addToIntArray(&indices,i*(faces+1)+sideStart+j,i*(faces+1)+sideStart+j+1,(i+1)*(faces+1)+sideStart+j);  
            tt::addToIntArray(&indices,i*(faces+1)+sideStart+j+1,(i+1)*(faces+1)+sideStart+j+1,(i+1)*(faces+1)+sideStart+j);    
        }
    }

    // We have to close the cylinder here:
    // First top and bottom part:
    if (!openEnds)
    {
        if (discDiv==1)
        {
            tt::addToIntArray(&indices,0,(sides-1)*(faces+1)+sideStart,sideStart);
            tt::addToIntArray(&indices,sides*(faces+1)+sideStart-1,1,(faces+1)+sideStart-1);
        }
        else
        {
            tt::addToIntArray(&indices,0,dstStart+sides*(discDiv-1)-1,dstStart+sides*(discDiv-2));
            tt::addToIntArray(&indices,dsbStart+sides*(discDiv-1)-1,1,dsbStart+sides*(discDiv-2));

            for (int j=0;j<discDiv-2;j++)
            {
                tt::addToIntArray(&indices,dstStart+j*sides+sides-1,dstStart+j*sides,dstStart+(j+2)*sides-1);   
                tt::addToIntArray(&indices,dstStart+j*sides,dstStart+(j+1)*sides,dstStart+(j+2)*sides-1);   
                tt::addToIntArray(&indices,dsbStart+j*sides,dsbStart+(j+1)*sides-1,dsbStart+(j+2)*sides-1); 
                tt::addToIntArray(&indices,dsbStart+(j+1)*sides,dsbStart+j*sides,dsbStart+(j+2)*sides-1);   
            }
            tt::addToIntArray(&indices,sideStart+(sides-1)*(faces+1),sideStart,dstStart+sides-1);
            tt::addToIntArray(&indices,sideStart,dstStart,dstStart+sides-1);
            tt::addToIntArray(&indices,sideStart+faces,sideStart+faces+(sides-1)*(faces+1),dsbStart+sides-1);
            tt::addToIntArray(&indices,dsbStart,sideStart+faces,dsbStart+sides-1);
        }
    }
    for (int j=0;j<faces;j++)
    { // Here the rest:
        tt::addToIntArray(&indices,(sides-1)*(faces+1)+sideStart+j,(sides-1)*(faces+1)+sideStart+j+1,sideStart+j);  
        tt::addToIntArray(&indices,(sides-1)*(faces+1)+sideStart+j+1,sideStart+j+1,sideStart+j);    
    }

    if (cone)
    { // We have a degenerate cylinder, we need to remove degenerate triangles and double vertices:
        CMeshManip::checkVerticesIndicesNormalsTexCoords(vertices,indices,nullptr,nullptr,true,0.0000001,false);
    }

    // Now we scale the cylinder:
    for (int i=0;i<int(vertices.size())/3;i++)
    {
        C3Vector p(vertices[3*i+0],vertices[3*i+1],vertices[3*i+2]);
        p(0)=p(0)*xhSize;
        p(1)=p(1)*yhSize;
        p(2)=p(2)*2.0*zhSize;
        vertices[3*i+0]=p(0);
        vertices[3*i+1]=p(1);
        vertices[3*i+2]=p(2);
    }
}

void CMeshRoutines::createAnnulus(std::vector<double>& vertices,std::vector<int>& indices,double Dlarge,double Dsmall,double zShift,int sides,bool faceUp)
{
    vertices.clear();
    indices.clear();
    double R=Dlarge*0.5;
    double r=Dsmall*0.5;

    double sa=2.0*piValue/((double)sides);

    // We set up the vertices:
    for (int i=0;i<sides;i++)
        tt::addToFloatArray(&vertices,R*(double)cos(sa*i),R*(double)sin(sa*i),zShift);    
    for (int i=0;i<sides;i++)
        tt::addToFloatArray(&vertices,r*(double)cos(sa*i),r*(double)sin(sa*i),zShift);    

    // We set up the indices:
    for (int i=0;i<sides-1;i++)
    {
        if (faceUp)
        {
            tt::addToIntArray(&indices,i,i+sides,i+1);
            tt::addToIntArray(&indices,i+sides,i+sides+1,i+1);
        }
        else
        {
            tt::addToIntArray(&indices,i+sides,i,i+1);
            tt::addToIntArray(&indices,i+sides+1,i+sides,i+1);
        }
    }
    // the closing part:
    if (faceUp)
    {
        tt::addToIntArray(&indices,sides-1,2*sides-1,0);
        tt::addToIntArray(&indices,2*sides-1,sides,0);
    }
    else
    {
        tt::addToIntArray(&indices,2*sides-1,sides-1,0);
        tt::addToIntArray(&indices,sides,2*sides-1,0);
    }
}
