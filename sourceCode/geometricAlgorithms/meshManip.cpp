#include <meshManip.h>
#include <algorithm>
#include <meshRoutines.h>

CMeshManip::CMeshManip(double* vertices,int verticesNb,int* indices,int indicesNb)
{
    // We first prepare the edges:
    edges.clear();
    for (int i=0;i<verticesNb/3;i++)
        edges.push_back(nullptr);
    for (int i=0;i<indicesNb/3;i++)
    {
        int ind[3]={indices[3*i+0],indices[3*i+1],indices[3*i+2]};
        int a=ind[0];
        int b=ind[1];
        edges[a]=new CEdgeElement(a,b,i,edges[a],0,vertices);
        edges[b]=new CEdgeElement(b,a,i,edges[b],0,vertices);
        a=ind[1];
        b=ind[2];
        edges[a]=new CEdgeElement(a,b,i,edges[a],1,vertices);
        edges[b]=new CEdgeElement(b,a,i,edges[b],1,vertices);
        a=ind[2];
        b=ind[0];
        edges[a]=new CEdgeElement(a,b,i,edges[a],2,vertices);
        edges[b]=new CEdgeElement(b,a,i,edges[b],2,vertices);
    }
    // Now we prepare the normals of all triangles:
    faceNormals.clear();
    for (int i=0;i<indicesNb/3;i++)
    {
        C3Vector pt0(&vertices[3*indices[3*i+0]+0]);
        C3Vector pt1(&vertices[3*indices[3*i+1]+0]);
        C3Vector pt2(&vertices[3*indices[3*i+2]+0]);
        // We make sure that almost degenerate triangles have a precise normal vector
        C3Vector v0((pt1-pt0).getNormalized());
        C3Vector v1((pt2-pt0).getNormalized());
        C3Vector w0((pt2-pt1).getNormalized());
        C3Vector w1(v0*-1.0);
        if (fabs(v0*v0)<fabs(w0*w0))
            faceNormals.push_back(v0^v1);
        else
            faceNormals.push_back(w0^w1);
    }
}

CMeshManip::~CMeshManip()
{
    for (int i=0;i<int(edges.size());i++)
        delete edges[i];
}


bool CMeshManip::extractOneShape(std::vector<double>* vertices,std::vector<int>* indices,std::vector<double>* sVertices,std::vector<int>* sIndices)
{   // This is the vector version of the routine!
    // Return value is true if the result is 2 shapes
    // Return value is false if the result is only one shape

    // We first build a fast access from edges to triangle indices:
    std::vector<std::vector<int> > allEdges;
    allEdges.resize(vertices->size()/3,std::vector<int>());
    for (int i=0;i<int(indices->size())/3;i++)
    {
        int ind[3]={indices->at(3*i+0),indices->at(3*i+1),indices->at(3*i+2)};
        for (int j=0;j<3;j++)
        {
            int k=j+1;
            if (k>2)
                k-=3;
            int a=std::min<int>(ind[j],ind[k]);
            int b=std::max<int>(ind[j],ind[k]);
            allEdges[a].push_back(b);
            allEdges[a].push_back(i);
        }
    }

    // We now iteratively explore the first triangle:
    std::vector<bool> alreadyExploredTriangles(indices->size()/3,false);
    std::vector<int> toExplore;
    toExplore.push_back(0);
    while (toExplore.size()!=0)
    {
        int triangle=toExplore[0];
        toExplore.erase(toExplore.begin());
        if (!alreadyExploredTriangles[triangle])
        {
            alreadyExploredTriangles[triangle]=true;
            int ind[3]={indices->at(3*triangle+0),indices->at(3*triangle+1),indices->at(3*triangle+2)};
            for (int j=0;j<3;j++)
            {
                int k=j+1;
                if (k>2)
                    k-=3;
                int a=std::min<int>(ind[j],ind[k]);
                int b=std::max<int>(ind[j],ind[k]);
                for (int i=0;i<int(allEdges[a].size())/2;i++)
                {
                    int b2=allEdges[a][2*i+0];
                    int triangle2=allEdges[a][2*i+1];
                    if ( (!alreadyExploredTriangles[triangle2])&&(b2==b) )
                        toExplore.push_back(triangle2); // same edge and not yet explored!
                }
            }
        }
    }

    // Now first check if we have at least two meshes:
    int totExplored=0;
    for (int i=0;i<int(alreadyExploredTriangles.size());i++)
    {
        if (alreadyExploredTriangles[i])
            totExplored++;
    }

    if (totExplored!=int(indices->size())/3)
    {
        // First the 'original', remaining part of the mesh:
        sIndices->clear();
        sVertices->clear();
        for (int i=0;i<int(alreadyExploredTriangles.size());i++)
        {
            if (alreadyExploredTriangles[i])
            {
                sIndices->push_back(indices->at(3*i+0));
                sIndices->push_back(indices->at(3*i+1));
                sIndices->push_back(indices->at(3*i+2));
            }
        }
        useOnlyReferencedVertices(vertices,sVertices,sIndices);

        // Now the 'extracted' part of the mesh:
        std::vector<int> ind(indices->begin(),indices->end());
        indices->clear();
        std::vector<double> vert(vertices->begin(),vertices->end());
        vertices->clear();
        for (int i=0;i<int(alreadyExploredTriangles.size());i++)
        {
            if (!alreadyExploredTriangles[i])
            {
                indices->push_back(ind[3*i+0]);
                indices->push_back(ind[3*i+1]);
                indices->push_back(ind[3*i+2]);
            }
        }
        useOnlyReferencedVertices(&vert,vertices,indices);
        return(true);
    }
    else
    {
        sIndices->assign(indices->begin(),indices->end());
        sVertices->assign(vertices->begin(),vertices->end());
        return(false);
    }
}

void CMeshManip::useOnlyReferencedVertices(std::vector<double>* vertices,std::vector<double>* sVertices,std::vector<int>* sIndices)
{ // This is the vector version of the routine!
    // Now we have to remap the vertices and indices (some vertices have to be removed)
    sVertices->reserve(vertices->size());
    sVertices->clear();
    std::vector<int> mapping(vertices->size()/3,-1);
    int freeSlot=0;
    for (int i=0;i<int(sIndices->size());i++)
    {
        if (mapping[(*sIndices)[i]]==-1)
        {
            sVertices->push_back((*vertices)[3*(*sIndices)[i]+0]);
            sVertices->push_back((*vertices)[3*(*sIndices)[i]+1]);
            sVertices->push_back((*vertices)[3*(*sIndices)[i]+2]);
            mapping[(*sIndices)[i]]=freeSlot;
            (*sIndices)[i]=freeSlot;
            freeSlot++;
        }
        else
            (*sIndices)[i]=mapping[(*sIndices)[i]];
    }
}

double CMeshManip::getMaxEdgeLength(const std::vector<double>& vertices,const std::vector<int>& indices)
{
    double retVal=0.0;
    for (size_t i=0;i<indices.size()/3;i++)
    {
        int ind[3]={indices[3*i+0],indices[3*i+1],indices[3*i+2]};
        C3Vector v0(vertices[3*ind[0]+0],vertices[3*ind[0]+1],vertices[3*ind[0]+2]);
        C3Vector v1(vertices[3*ind[1]+0],vertices[3*ind[1]+1],vertices[3*ind[1]+2]);
        C3Vector v2(vertices[3*ind[2]+0],vertices[3*ind[2]+1],vertices[3*ind[2]+2]);
        double lt=(v0-v1).getLength();
        if (lt>retVal)
            retVal=lt;
        lt=(v0-v2).getLength();
        if (lt>retVal)
            retVal=lt;
        lt=(v2-v1).getLength();
        if (lt>retVal)
            retVal=lt;
    }
    return(retVal);
}

bool CMeshManip::reduceTriangleSize(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<float>* texCoords,double maxEdgeSize)
{ // returns false if nothing is left
    // if maxEdgeSize is 0.0, then half of the maximum triangle edge is used as maxEdgeSize
    // if verticeMergeTolerance is 0.0, vertices are not merged
    // normals or texCoords can be nullptr

    double el=getMaxEdgeLength(vertices,indices);
    if (maxEdgeSize<=0.0)
        maxEdgeSize=el/2.0;
    if (maxEdgeSize<el)
    {
        for (int i=0;i<12;i++)
        {
            if (_reduceTriangleSizePass(vertices,indices,normals,texCoords,maxEdgeSize)==0)
                break;
        }
        // Merge identical vertices:
        CMeshRoutines::removeDuplicateVerticesAndTriangles(vertices,&indices,normals,texCoords,0.000001);
    }
    return(indices.size()!=0);
}

int CMeshManip::_reduceTriangleSizePass(std::vector<double>& vertices,std::vector<int>& indices,std::vector<double>* normals,std::vector<float>* texCoords,double maxEdgeSize)
{ // normals or texCoords are optional. Return val is the nb of added triangles
    // We mark the triangles that need to be cut:

    int originalIndicesSize=int(indices.size());
    int trianglesAdded=0;
    for (int i=0;i<(originalIndicesSize/3);i++) // =editionIndices.size() will grow in this loop!
    {
        int ind[3]={indices[3*i+0],indices[3*i+1],indices[3*i+2]};
        C3Vector v0(vertices[3*ind[0]+0],vertices[3*ind[0]+1],vertices[3*ind[0]+2]);
        C3Vector v1(vertices[3*ind[1]+0],vertices[3*ind[1]+1],vertices[3*ind[1]+2]);
        C3Vector v2(vertices[3*ind[2]+0],vertices[3*ind[2]+1],vertices[3*ind[2]+2]);
        double s[3];
        s[0]=(v0-v1).getLength();
        s[1]=(v0-v2).getLength();
        s[2]=(v1-v2).getLength();
        int maxInd=0;
        double maxVal=0.0;
        for (int j=0;j<3;j++)
        {
            if (s[j]>maxVal)
            {
                maxVal=s[j];
                maxInd=j;
            }
        }
        if (maxVal>maxEdgeSize)
        { // We have to divide this edge
            C3Vector w[3];
            int vertInd[3];
            float texCoordsX[3];
            float texCoordsY[3];
            C3Vector n0,n1,n2;
            if (maxInd==0)
            {
                w[0]=v2;
                w[1]=v0;
                w[2]=v1;
                vertInd[0]=indices[3*i+2];
                vertInd[1]=indices[3*i+0];
                vertInd[2]=indices[3*i+1];
                if (texCoords!=nullptr)
                {
                    texCoordsX[0]=(*texCoords)[6*i+4];
                    texCoordsY[0]=(*texCoords)[6*i+5];
                    texCoordsX[1]=(*texCoords)[6*i+0];
                    texCoordsY[1]=(*texCoords)[6*i+1];
                    texCoordsX[2]=(*texCoords)[6*i+2];
                    texCoordsY[2]=(*texCoords)[6*i+3];
                }
                if (normals!=nullptr)
                {
                    n0=&((*normals)[9*i+6]);
                    n1=&((*normals)[9*i+0]);
                    n2=&((*normals)[9*i+3]);
                }
            }
            if (maxInd==1)
            {
                w[0]=v1;
                w[1]=v2;
                w[2]=v0;
                vertInd[0]=indices[3*i+1];
                vertInd[1]=indices[3*i+2];
                vertInd[2]=indices[3*i+0];
                if (texCoords!=nullptr)
                {
                    texCoordsX[0]=(*texCoords)[6*i+2];
                    texCoordsY[0]=(*texCoords)[6*i+3];
                    texCoordsX[1]=(*texCoords)[6*i+4];
                    texCoordsY[1]=(*texCoords)[6*i+5];
                    texCoordsX[2]=(*texCoords)[6*i+0];
                    texCoordsY[2]=(*texCoords)[6*i+1];
                }
                if (normals!=nullptr)
                {
                    n0=&((*normals)[9*i+3]);
                    n1=&((*normals)[9*i+6]);
                    n2=&((*normals)[9*i+0]);
                }
            }
            if (maxInd==2)
            {
                w[0]=v0;
                w[1]=v1;
                w[2]=v2;
                vertInd[0]=indices[3*i+0];
                vertInd[1]=indices[3*i+1];
                vertInd[2]=indices[3*i+2];
                if (texCoords!=nullptr)
                {
                    texCoordsX[0]=(*texCoords)[6*i+0];
                    texCoordsY[0]=(*texCoords)[6*i+1];
                    texCoordsX[1]=(*texCoords)[6*i+2];
                    texCoordsY[1]=(*texCoords)[6*i+3];
                    texCoordsX[2]=(*texCoords)[6*i+4];
                    texCoordsY[2]=(*texCoords)[6*i+5];
                }
                if (normals!=nullptr)
                {
                    n0=&((*normals)[9*i+0]);
                    n1=&((*normals)[9*i+3]);
                    n2=&((*normals)[9*i+6]);
                }
            }
            // Now we divide w[1]-w[2] and create a new vertex nw --> new triangle becomes w[0]-w[1]-nw
            C3Vector nw((w[1]+w[2])*0.5);
            // we insert the new vertex:
            for (int j=0;j<3;j++)
                vertices.push_back(nw(j));
            int newVertInd=int(vertices.size()/3)-1; // new index on new vertex
            // We correct the old triangle with the new triangle1:
            indices[3*i+0]=vertInd[0];
            indices[3*i+1]=vertInd[1];
            indices[3*i+2]=newVertInd;
            // We insert the new triangle2:
            indices.push_back(vertInd[0]);
            indices.push_back(newVertInd);
            indices.push_back(vertInd[2]);

            if (texCoords!=nullptr)
            { // we do the same with the texture coord:
                float ntc[2];
                ntc[0]=(texCoordsX[1]+texCoordsX[2])*0.5f;
                ntc[1]=(texCoordsY[1]+texCoordsY[2])*0.5f;
                // We correct the old triangle's tex coords:
                (*texCoords)[6*i+0]=texCoordsX[0];
                (*texCoords)[6*i+1]=texCoordsY[0];
                (*texCoords)[6*i+2]=texCoordsX[1];
                (*texCoords)[6*i+3]=texCoordsY[1];
                (*texCoords)[6*i+4]=ntc[0];
                (*texCoords)[6*i+5]=ntc[1];
                // We insert the new triangle2 tex coords:
                (*texCoords).push_back(texCoordsX[0]);
                (*texCoords).push_back(texCoordsY[0]);
                (*texCoords).push_back(ntc[0]);
                (*texCoords).push_back(ntc[1]);
                (*texCoords).push_back(texCoordsX[2]);
                (*texCoords).push_back(texCoordsY[2]);
            }

            if (normals!=nullptr)
            { // we do the same with normals:
                C3Vector nn((n1+n2)*0.5);
                // We correct the old triangle's normals:
                (*normals)[9*i+0]=n0(0);
                (*normals)[9*i+1]=n0(1);
                (*normals)[9*i+2]=n0(2);
                (*normals)[9*i+3]=n1(0);
                (*normals)[9*i+4]=n1(1);
                (*normals)[9*i+5]=n1(2);
                (*normals)[9*i+6]=nn(0);
                (*normals)[9*i+7]=nn(1);
                (*normals)[9*i+8]=nn(2);
                // We insert the new triangle2 normals:
                (*normals).push_back(n0(0));
                (*normals).push_back(n0(1));
                (*normals).push_back(n0(2));
                (*normals).push_back(nn(0));
                (*normals).push_back(nn(1));
                (*normals).push_back(nn(2));
                (*normals).push_back(n2(0));
                (*normals).push_back(n2(1));
                (*normals).push_back(n2(2));
            }
            trianglesAdded++;
        }
    }
    return(trianglesAdded);
}

bool CMeshManip::correctTriangleWinding(std::vector<double>* vertices,std::vector<int>* indices)
{   // First create an edge list
    // The smallest index specifying the edge gives the position in edges array
    // eg. insertion of edge (4;6) belonging to triangle 3
    // edge[4]->Add(6); edge[4]->Add(3);
    std::vector<std::vector<int>*> edges(vertices->size()/3,nullptr);
    for (int i=0;i<int(indices->size())/3;i++)
    {
        int temp;
        int v[3][2];
        v[0][0]=indices->at(3*i+0);
        v[0][1]=indices->at(3*i+1);
        v[1][0]=indices->at(3*i+0);
        v[1][1]=indices->at(3*i+2);
        v[2][0]=indices->at(3*i+1);
        v[2][1]=indices->at(3*i+2);
        for (int j=0;j<3;j++)
        {
            if (v[j][0]>v[j][1])
            {
                temp=v[j][0];
                v[j][0]=v[j][1];
                v[j][1]=temp;
            }
            if (edges[v[j][0]]==nullptr)
            {
                std::vector<int>* newList=new std::vector<int>;
                edges[v[j][0]]=newList;
            }
            edges[v[j][0]]->push_back(v[j][1]);
            edges[v[j][0]]->push_back(i);
        }
    }
    // Check if there are some edges shared among more than 2
    // triangles. If yes, the side of their normal can't be
    // deduced from each others. We leave out these
    // triangle's edges, but we have to make many passes
    // until all triangles are processed

    for (int i=0;i<int(edges.size());i++)
    {
        if (edges.at(i)!=nullptr)
        {
            for (int j=0;j<int(edges.at(i)->size())/2;j++)
            {   
                int ccount=0;
                int p2=edges.at(i)->at(2*j);
                for (int k=0;k<int(edges.at(i)->size())/2;k++)
                {
                    if ((edges.at(i)->at(2*k)==p2)&&(p2!=-1))
                        ccount++;
                }
                if (ccount>2)
                {   // this edge is shared more than 2 times
                    for (int k=0;k<int(edges.at(i)->size())/2;k++)
                    {
                        if (edges.at(i)->at(2*k)==p2)
                            edges.at(i)->at(2*k)=-1;
                    }
                }
            }
            int j=0;
            while (j<int(edges.at(i)->size())/2)
            {
                if (edges.at(i)->at(2*j)==-1)
                    edges.at(i)->erase(edges.at(i)->begin()+2*j,edges.at(i)->begin()+2*j+2);
                else
                    j=j+2;
            }
        }
    }
    
    // Now creat an array giving the state of a triangle:
    // false : not yet touched; true : the triangle was checked and corrected
    std::vector<unsigned char> exploredState(indices->size()/3,0);
    std::vector<double> normals(indices->size());
    // Now we have to recursively explore the triangles.
    // To avoid using too much memory, the recursion is coded
    // using an array keeping track of the triangles:
    std::vector<int> recursion;
    int triangleCount=0;
    recursion.reserve(indices->size()/3);
    recursion.push_back(0); // We begin with triangle 0
    int actualEdge[2];
    C3Vector n,v1,v2,p1,p2,p3,lastDir;
    bool stillTrianglesLeftOut=true;
    int passes=0;
    while (stillTrianglesLeftOut)
    {   // We need this loop to catch triangles which were not processed
        // (because of problems due to some edges shared between
        // more than 2 triangles)
        passes++;
        // The following while-loop is the recursion loop in which all triangle
        // windings are corrected
        while (recursion.size()!=0)
        {
            int actualTriangle=recursion[recursion.size()-1];
            if (exploredState[actualTriangle]==0)
            {
                // This is a newly discovered triangle
                // Compute its normal
                p1(0)=vertices->at(3*indices->at(3*actualTriangle+0)+0);
                p1(1)=vertices->at(3*indices->at(3*actualTriangle+0)+1);
                p1(2)=vertices->at(3*indices->at(3*actualTriangle+0)+2);
                p2(0)=vertices->at(3*indices->at(3*actualTriangle+1)+0);
                p2(1)=vertices->at(3*indices->at(3*actualTriangle+1)+1);
                p2(2)=vertices->at(3*indices->at(3*actualTriangle+1)+2);
                p3(0)=vertices->at(3*indices->at(3*actualTriangle+2)+0);
                p3(1)=vertices->at(3*indices->at(3*actualTriangle+2)+1);
                p3(2)=vertices->at(3*indices->at(3*actualTriangle+2)+2);
                v1=p2-p1;
                v2=p3-p2;
                n=v1^v2;
                if (recursion.size()==1)
                {
                    // We are at the very beginning of the recursion
                    normals[3*actualTriangle+0]=n(0);
                    normals[3*actualTriangle+1]=n(1);
                    normals[3*actualTriangle+2]=n(2);
                    exploredState[actualTriangle]=1;
                    // Find the other triangle on this edge
                    int nextN=_getNeighbour(actualTriangle,indices,actualEdge,&edges,&exploredState);
                    if (nextN==-1)
                    {
                        // 1 unique triangle case
                        recursion.pop_back();
                    }
                    else
                    {
                        int k=0;
                        while ((indices->at(3*actualTriangle+k)==actualEdge[0])||(indices->at(3*actualTriangle+k)==actualEdge[1]))
                            k++;
                        double px=vertices->at(3*indices->at(3*actualTriangle+k)+0);
                        double py=vertices->at(3*indices->at(3*actualTriangle+k)+1);
                        double pz=vertices->at(3*indices->at(3*actualTriangle+k)+2);
                        double pxCopy=px;
                        double pyCopy=py;
                        double pzCopy=pz;
                        double lx=vertices->at(3*actualEdge[0]+0);
                        double ly=vertices->at(3*actualEdge[0]+1);
                        double lz=vertices->at(3*actualEdge[0]+2);
                        double lvx=vertices->at(3*actualEdge[1]+0)-vertices->at(3*actualEdge[0]+0);
                        double lvy=vertices->at(3*actualEdge[1]+1)-vertices->at(3*actualEdge[0]+1);
                        double lvz=vertices->at(3*actualEdge[1]+2)-vertices->at(3*actualEdge[0]+2);
                        getProjectionOfPointOnLine(lx,ly,lz,lvx,lvy,lvz,px,py,pz);
                        C3Vector r(px-pxCopy,py-pyCopy,pz-pzCopy);
                        lastDir=n^r;
                        // We have to add the next triangle to the recursion list
                        recursion.push_back(nextN);
                    }
                }
                else
                {
                    // compare to last Dir 
                    int k=0;
                    while ((indices->at(3*actualTriangle+k)==actualEdge[0])||(indices->at(3*actualTriangle+k)==actualEdge[1]))
                        k++;
                    double px=vertices->at(3*indices->at(3*actualTriangle+k)+0);
                    double py=vertices->at(3*indices->at(3*actualTriangle+k)+1);
                    double pz=vertices->at(3*indices->at(3*actualTriangle+k)+2);
                    double pxCopy=px;
                    double pyCopy=py;
                    double pzCopy=pz;
                    double lx=vertices->at(3*actualEdge[0]+0);
                    double ly=vertices->at(3*actualEdge[0]+1);
                    double lz=vertices->at(3*actualEdge[0]+2);
                    double lvx=vertices->at(3*actualEdge[1]+0)-vertices->at(3*actualEdge[0]+0);
                    double lvy=vertices->at(3*actualEdge[1]+1)-vertices->at(3*actualEdge[0]+1);
                    double lvz=vertices->at(3*actualEdge[1]+2)-vertices->at(3*actualEdge[0]+2);
                    getProjectionOfPointOnLine(lx,ly,lz,lvx,lvy,lvz,px,py,pz);
                    C3Vector r(px-pxCopy,py-pyCopy,pz-pzCopy);
                    C3Vector thisDir(n^r);
                    if ((thisDir*lastDir)>=0.0)
                    {
                        // We have to change this triangle's winding!
                        int temp=indices->at(3*actualTriangle+1);
                        indices->at(3*actualTriangle+1)=indices->at(3*actualTriangle+2);
                        indices->at(3*actualTriangle+2)=temp;
                        n*=-1.0;
                    }
                    // set normals
                    normals[3*actualTriangle+0]=n(0);
                    normals[3*actualTriangle+1]=n(1);
                    normals[3*actualTriangle+2]=n(2);
                    // that triangle was explored
                    exploredState[actualTriangle]=1;
                    triangleCount++;
                    // we have to select the next edge, set the explored flag for the direction we are going to
                    int nextN=_getNeighbour(actualTriangle,indices,actualEdge,&edges,&exploredState);
                    if (nextN!=-1)
                    {
                        // we have now to compute the next lastDir value
                        k=0;
                        while ((indices->at(3*actualTriangle+k)==actualEdge[0])||(indices->at(3*actualTriangle+k)==actualEdge[1]))
                            k++;
                        px=vertices->at(3*indices->at(3*actualTriangle+k)+0);
                        py=vertices->at(3*indices->at(3*actualTriangle+k)+1);
                        pz=vertices->at(3*indices->at(3*actualTriangle+k)+2);
                        pxCopy=px;
                        pyCopy=py;
                        pzCopy=pz;
                        lx=vertices->at(3*actualEdge[0]+0);
                        ly=vertices->at(3*actualEdge[0]+1);
                        lz=vertices->at(3*actualEdge[0]+2);
                        lvx=vertices->at(3*actualEdge[1]+0)-vertices->at(3*actualEdge[0]+0);
                        lvy=vertices->at(3*actualEdge[1]+1)-vertices->at(3*actualEdge[0]+1);
                        lvz=vertices->at(3*actualEdge[1]+2)-vertices->at(3*actualEdge[0]+2);
                        getProjectionOfPointOnLine(lx,ly,lz,lvx,lvy,lvz,px,py,pz);
                        r(0)=px-pxCopy;
                        r(1)=py-pyCopy;
                        r(2)=pz-pzCopy;
                        lastDir=n^r;
                        // we have to add the next neighbour to the recursion-list
                        recursion.push_back(nextN);
                    }
                    else
                    {   // We have to remove the actual triangle from the recursion-list
                        recursion.pop_back();
                    }
                }
            }
            else
            {   // We've come back to this triangle, and now we have to explore the other 1 or 2 edges
                int nextN=_getNeighbour(actualTriangle,indices,actualEdge,&edges,&exploredState);
                if (nextN!=-1)
                {   // We've found a new path
                    // we have now to compute the next lastDir value
                    int k=0;
                    while ((indices->at(3*actualTriangle+k)==actualEdge[0])||(indices->at(3*actualTriangle+k)==actualEdge[1]))
                        k++;
                    double px=vertices->at(3*indices->at(3*actualTriangle+k)+0);
                    double py=vertices->at(3*indices->at(3*actualTriangle+k)+1);
                    double pz=vertices->at(3*indices->at(3*actualTriangle+k)+2);
                    double pxCopy=px;
                    double pyCopy=py;
                    double pzCopy=pz;
                    double lx=vertices->at(3*actualEdge[0]+0);
                    double ly=vertices->at(3*actualEdge[0]+1);
                    double lz=vertices->at(3*actualEdge[0]+2);
                    double lvx=vertices->at(3*actualEdge[1]+0)-vertices->at(3*actualEdge[0]+0);
                    double lvy=vertices->at(3*actualEdge[1]+1)-vertices->at(3*actualEdge[0]+1);
                    double lvz=vertices->at(3*actualEdge[1]+2)-vertices->at(3*actualEdge[0]+2);
                    getProjectionOfPointOnLine(lx,ly,lz,lvx,lvy,lvz,px,py,pz);
                    C3Vector r(px-pxCopy,py-pyCopy,pz-pzCopy);
                    C3Vector nAux(normals[3*actualTriangle+0],normals[3*actualTriangle+1],normals[3*actualTriangle+2]); 
                    lastDir=nAux^r;
                    // we have to add the next neighbour to the recursion-list
                    recursion.push_back(nextN);
                }
                else
                {   // This triangle was completely explored and can now be removed
                    recursion.pop_back();
                }
            }
        }
        // check if some triangles are left out
        stillTrianglesLeftOut=false;
        int i;
        for (i=0;i<int(exploredState.size());i++)
        {
            if (exploredState[i]==0)
            {
                stillTrianglesLeftOut=true;
                break;
            }
        }
        if (stillTrianglesLeftOut) 
            recursion.push_back(i);
    }
    
    // Clean up the edge list
    for (int i=0;i<int(edges.size());i++)
    {
        if (edges[i]!=nullptr)
            delete edges[i];
    }
    return(passes==1);
}

int CMeshManip::_getNeighbour(int actualTriangle,std::vector<int>* indices,
                      int actualEdge[2],std::vector<std::vector<int>*>* edges,
                      std::vector<unsigned char>* exploredState)
{   // This routine is needed by the routine "ExtractOneShapeEdge"
    // Find the other triangle(s) on edge specified by m
    int found=-1;
    int m=1;
    for (int k=0;k<3;k++)
    {

        int a=0;
        if (m==2) 
            a=1;
        int b=2;
        if (m==1) 
            b=1;
        if (indices->at(3*actualTriangle+a)>indices->at(3*actualTriangle+b))
        {
            actualEdge[0]=indices->at(3*actualTriangle+b);
            actualEdge[1]=indices->at(3*actualTriangle+a);
        }
        else
        {
            actualEdge[0]=indices->at(3*actualTriangle+a);
            actualEdge[1]=indices->at(3*actualTriangle+b);
        }
        int i=0;
        while (i<int(edges->at(actualEdge[0])->size())/2)
        {
            if (edges->at(actualEdge[0])->at(2*i+0)==actualEdge[1])
            {
                if (edges->at(actualEdge[0])->at(2*i+1)==actualTriangle)
                {
                    edges->at(actualEdge[0])->erase(edges->at(actualEdge[0])->begin()+2*i,edges->at(actualEdge[0])->begin()+2*i+2);
                }
                else
                {
                    found=edges->at(actualEdge[0])->at(2*i+1);
                    edges->at(actualEdge[0])->erase(edges->at(actualEdge[0])->begin()+2*i,edges->at(actualEdge[0])->begin()+2*i+2);
                }
            }
            else    
                i++;
            if (found!=-1) 
                break;
        }
        if (found!=-1) 
            break;
        if (m==2) 
            m=4;
        if (m==1) 
            m=2;
    }
    return(found);
}

bool CMeshManip::isInsideOut(std::vector<double>* vertices,std::vector<int>* indices)
{
    C3Vector a,b,c,m,n,v,w,d;
    C3Vector center(0.0,0.0,0.0);
    double total=0.0;
    for (int i=0;i<int(vertices->size())/3;i++)
    {
        C3Vector adp(vertices->at(3*i+0),vertices->at(3*i+1),vertices->at(3*i+2));
        center+=adp;
    }
    center/=(double)(vertices->size()/3);
    for (int i=0;i<int(indices->size())/3;i++)
    {
        for (int j=0;j<3;j++)
        {
            a(j)=vertices->at(3*indices->at(3*i+0)+j);
            b(j)=vertices->at(3*indices->at(3*i+1)+j);
            c(j)=vertices->at(3*indices->at(3*i+2)+j);
            m(j)=(a(j)+b(j)+c(j))/3.0;
            v(j)=b(j)-a(j);
            w(j)=c(j)-b(j);
        }
        n=(v^w).getNormalized();
        d=(m-center).getNormalized();
        total=total+n*d;
    }
    return(total<0.0);
}

void CMeshManip::setInsideOut(std::vector<int>* indices)
{
    int temp;
    for (int i=0;i<int(indices->size())/3;i++)
    {
        temp=indices->at(3*i+1);
        indices->at(3*i+1)=indices->at(3*i+2);
        indices->at(3*i+2)=temp;
    }
}

void CMeshManip::getProjectionOfPointOnLine(double x,double y,double z,
                                    double vx,double vy,double vz,
                                    double &px,double &py,double &pz)
{
    double t=( vx*(px-x)+vy*(py-y)+vz*(pz-z) ) / ( vx*vx+vy*vy+vz*vz );
    px=x+t*vx;
    py=y+t*vy;
    pz=z+t*vz;
}

bool CMeshManip::mergeWith( std::vector<double>* tVertices,std::vector<int>* tIndices,std::vector<double>* tNormals,
                    std::vector<double>* sVertices,std::vector<int>* sIndices,std::vector<double>* sNormals)
{   // s will be merged with t. tNormals and sNormals can be nullptr (but have to be nullptr at the same time)
    // Rewrite later with correct stl function!!
    int pos=(int)tVertices->size()/3;
    for (int i=0;i<int(sVertices->size());i++)
        tVertices->push_back(sVertices->at(i));
    for (int i=0;i<int(sIndices->size());i++)
        tIndices->push_back(sIndices->at(i)+pos);
    if ( (tNormals!=nullptr)&&(sNormals!=nullptr) )
    {
        for (int i=0;i<int(sNormals->size());i++)
            tNormals->push_back(sNormals->at(i));
    }
    return(true);
}

void CMeshManip::reduceToUnit(double vector[3])
{
    double length=sqrt((vector[0]*vector[0])+(vector[1]*vector[1])+(vector[2]*vector[2]));
    if (length==0.0)
        length=1.0;
    vector[0]=vector[0]/length;
    vector[1]=vector[1]/length;
    vector[2]=vector[2]/length;
}

void CMeshManip::calcNormal(double v[3][3],double out[3])
{
    double v1[3],v2[3];
    v1[0]=v[0][0]-v[1][0];
    v1[1]=v[0][1]-v[1][1];
    v1[2]=v[0][2]-v[1][2];
    v2[0]=v[1][0]-v[2][0];
    v2[1]=v[1][1]-v[2][1];
    v2[2]=v[1][2]-v[2][2];
    out[0]=v1[1]*v2[2]-v1[2]*v2[1];
    out[1]=v1[2]*v2[0]-v1[0]*v2[2];
    out[2]=v1[0]*v2[1]-v1[1]*v2[0];
    reduceToUnit(out);
}

void CMeshManip::getSize(std::vector<double>* vertices,double x[2],double y[2],double z[2],bool &start)
{
    for (int i=0;i<int(vertices->size())/3;i++)
    {
        if (start)
        {
            x[0]=vertices->at(3*i+0);
            x[1]=vertices->at(3*i+0);
            y[0]=vertices->at(3*i+1);
            y[1]=vertices->at(3*i+1);
            z[0]=vertices->at(3*i+2);
            z[1]=vertices->at(3*i+2);
            start=false;
        }
        else
        {
            if (vertices->at(3*i+0)<x[0]) x[0]=vertices->at(3*i+0);
            if (vertices->at(3*i+0)>x[1]) x[1]=vertices->at(3*i+0);
            if (vertices->at(3*i+1)<y[0]) y[0]=vertices->at(3*i+1);
            if (vertices->at(3*i+1)>y[1]) y[1]=vertices->at(3*i+1);
            if (vertices->at(3*i+2)<z[0]) z[0]=vertices->at(3*i+2);
            if (vertices->at(3*i+2)>z[1]) z[1]=vertices->at(3*i+2);
        }
    }
}
void CMeshManip::centerAndScale(std::vector<double>* vertices,double x,double y,double z,double sf)
{
    for (int i=0;i<int(vertices->size())/3;i++)
    {
        vertices->at(3*i+0)=vertices->at(3*i+0)-x;
        vertices->at(3*i+0)=vertices->at(3*i+0)*sf;
        vertices->at(3*i+1)=vertices->at(3*i+1)-y;
        vertices->at(3*i+1)=vertices->at(3*i+1)*sf;
        vertices->at(3*i+2)=vertices->at(3*i+2)-z;
        vertices->at(3*i+2)=vertices->at(3*i+2)*sf;
    }
}

void CMeshManip::getNormals(const std::vector<double>* vertices,const std::vector<int>* indices,
                    std::vector<double>* normals)
{   // This is the vector version of the function!
    // The size of normals is set in this function
    double v[3][3],n[3];
    normals->clear();
    normals->insert(normals->begin(),3*indices->size(),0);
    for (size_t i=0;i<indices->size()/3;i++)
    {
        v[0][0]=vertices->at(3*indices->at(3*i)+0);
        v[0][1]=vertices->at(3*indices->at(3*i)+1);
        v[0][2]=vertices->at(3*indices->at(3*i)+2);
        v[1][0]=vertices->at(3*indices->at(3*i+1)+0);
        v[1][1]=vertices->at(3*indices->at(3*i+1)+1);
        v[1][2]=vertices->at(3*indices->at(3*i+1)+2);
        v[2][0]=vertices->at(3*indices->at(3*i+2)+0);
        v[2][1]=vertices->at(3*indices->at(3*i+2)+1);
        v[2][2]=vertices->at(3*indices->at(3*i+2)+2);
        calcNormal(v,n);
        normals->at(9*i+0)=n[0];
        normals->at(9*i+1)=n[1];
        normals->at(9*i+2)=n[2];
        normals->at(9*i+3)=n[0];
        normals->at(9*i+4)=n[1];
        normals->at(9*i+5)=n[2];
        normals->at(9*i+6)=n[0];
        normals->at(9*i+7)=n[1];
        normals->at(9*i+8)=n[2];
    }
}

void CMeshManip::getTrianglesFromPolygons(const std::vector<std::vector<int> >& polygons,std::vector<int>& indices)
{
    indices.clear();
    for (int i=0;i<int(polygons.size());i++)
    {
        if (polygons[i].size()>=3)
        {
            std::vector<int> pol(polygons[i]);
            int baseIndex=0;
            for (int j=0;j<int(polygons[i].size())-2;j++)
            {
                int secondIndex=baseIndex+1;
                int thirdIndex=baseIndex+2;
                if (secondIndex>=int(pol.size()))
                    secondIndex-=(int)pol.size();
                if (thirdIndex>=int(pol.size()))
                    thirdIndex-=(int)pol.size();
                indices.push_back(pol[baseIndex]);
                indices.push_back(pol[secondIndex]);
                indices.push_back(pol[thirdIndex]);
                pol.erase(pol.begin()+secondIndex);
                if (thirdIndex>secondIndex)
                    baseIndex=thirdIndex-1;
                else
                    baseIndex=thirdIndex;
            }
        }
    }
}
