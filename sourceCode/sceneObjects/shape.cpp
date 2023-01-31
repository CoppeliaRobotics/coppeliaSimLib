#include "simInternal.h"
#include "shape.h"
#include "tt.h"
#include "algos.h"
#include "app.h"
#include "meshWrapper.h"
#include "mesh.h"
#include "easyLock.h"
#include "pluginContainer.h"
#include "shapeRendering.h"
#include "meshManip.h"
#include "base64.h"
#include "imgLoaderSaver.h"
#include "ttUtil.h"

bool CShape::_visualizeObbStructures=false;

bool CShape::getDebugObbStructures()
{
    return(_visualizeObbStructures);
}

void CShape::setDebugObbStructures(bool d)
{
    _visualizeObbStructures=d;
}


CShape::CShape()
{
    commonInit();
}

CShape::CShape(const std::vector<double>& allHeights,int xSize,int ySize,double dx,double zSize)
{
    commonInit();

    C7Vector newLocalTr;

    std::vector<double> vert;
    std::vector<int> ind;
    double yPos=-double(ySize-1)*dx*0.5;
    for (int i=0;i<ySize;i++)
    {
        double xPos=-double(xSize-1)*dx*0.5;
        for (int j=0;j<xSize;j++)
        {
            vert.push_back(xPos);
            vert.push_back(yPos);
            vert.push_back(allHeights[i*xSize+j]);
            xPos+=dx;
        }
        yPos+=dx;
    }

    for (int i=0;i<ySize-1;i++)
    {
        for (int j=0;j<xSize-1;j++)
        {
            ind.push_back((i+0)*xSize+(j+0));
            ind.push_back((i+0)*xSize+(j+1));
            ind.push_back((i+1)*xSize+(j+0));

            ind.push_back((i+0)*xSize+(j+1));
            ind.push_back((i+1)*xSize+(j+1));
            ind.push_back((i+1)*xSize+(j+0));
        }
    }

    newLocalTr=_acceptNewGeometry(vert,ind,nullptr,nullptr);

    getMeshWrapper()->setPurePrimitiveType(sim_primitiveshape_heightfield,double(xSize-1)*dx,double(ySize-1)*dx,zSize);
    std::vector<double> heightsInCorrectOrder;
    for (int i=0;i<ySize;i++)
    {
        // Following doesn't work correctly somehow...
        //for (int j=xSize-1;j>=0;j--)
        //  heightsInCorrectOrder.push_back(allHeights[i*xSize+j]);
        for (int j=0;j<xSize;j++)
            heightsInCorrectOrder.push_back(allHeights[i*xSize+j]);
    }
    getSingleMesh()->setHeightfieldData(heightsInCorrectOrder,xSize,ySize);
    setLocalTransformation(newLocalTr);
}

CShape::CShape(const C7Vector* transformation,const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* normals,const std::vector<double>* textCoord)
{
    commonInit();
    setLocalTransformation(reinitMesh(transformation,vert,ind,normals,textCoord));
}

CShape::CShape(const C7Vector& transformation,CMeshWrapper* newGeomInfo)
{
    commonInit();
    setLocalTransformation(reinitMesh2(transformation,newGeomInfo));
}

CShape::~CShape()
{
    removeMeshCalculationStructure();
    delete _mesh;
    delete _dynMaterial;
}

C7Vector CShape::reinitMesh(const C7Vector* transformation,const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* normals,const std::vector<double>* textCoord)
{
    C7Vector retVal;

    std::vector<double>* norms=nullptr;
    std::vector<double> _norms;
    if (normals!=nullptr)
    {
        norms=&_norms;
        _norms.assign(normals->begin(),normals->end());
        // Make sure the normals are normalized:
        for (size_t i=0;i<_norms.size()/3;i++)
        {
            C3Vector n(&_norms[3*i]);
            n.normalize();
            _norms[3*i+0]=n(0);
            _norms[3*i+1]=n(1);
            _norms[3*i+2]=n(2);
        }
    }

    if (transformation==nullptr)
        retVal=_acceptNewGeometry(vert,ind,textCoord,norms);
    else
    {
        std::vector<double> wvert(vert);
        for (size_t i=0;i<vert.size()/3;i++)
        {
            C3Vector v(&vert[3*i+0]);
            v*=(*transformation);
            wvert[3*i+0]=v(0);
            wvert[3*i+1]=v(1);
            wvert[3*i+2]=v(2);
        }
        C7Vector tr(*transformation);
        for (size_t i=0;i<_norms.size()/3;i++)
        {
            C3Vector n(&_norms[3*i+0]);
            n*=tr;
            _norms[3*i+0]=n(0);
            _norms[3*i+1]=n(1);
            _norms[3*i+2]=n(2);
        }
        retVal=_acceptNewGeometry(wvert,ind,textCoord,norms);
    }
    actualizeContainsTransparentComponent();
    return(retVal);
}

void CShape::setNewMesh(CMeshWrapper* newGeomInfo)
{
    removeMeshCalculationStructure();
    delete _mesh;
    _mesh=newGeomInfo;
    _meshModificationCounter++;
    _computeMeshBoundingBox();
    computeBoundingBox();
    actualizeContainsTransparentComponent();
}

C7Vector CShape::reinitMesh2(const C7Vector& transformation,CMeshWrapper* newGeomInfo)
{
    C7Vector retVal;
    retVal.setIdentity();
    removeMeshCalculationStructure();
    delete _mesh;
    _meshModificationCounter++;

    newGeomInfo->preMultiplyAllVerticeLocalFrames(transformation);
    _mesh=newGeomInfo;
    std::vector<double> wvert;
    std::vector<int> wind;
    getMeshWrapper()->getCumulativeMeshes(wvert,&wind,nullptr);

    // We align the bounding box:
    if (wvert.size()!=0)
    {
        retVal=CAlgos::getMeshBoundingBoxPose(wvert,wind,true);
        getMeshWrapper()->preMultiplyAllVerticeLocalFrames(retVal.getInverse());
    }

    _computeMeshBoundingBox();
    computeBoundingBox();
    actualizeContainsTransparentComponent();
    return(retVal);
}

void CShape::invertFrontBack()
{
    getMeshWrapper()->flipFaces();
    removeMeshCalculationStructure();// proximity sensors might check for the side!
}

C3Vector CShape::getBoundingBoxHalfSizes() const
{
    return(_meshBoundingBoxHalfSizes);
}

void CShape::_computeMeshBoundingBox()
{
    std::vector<double> visibleVertices;
    _mesh->getCumulativeMeshes(visibleVertices,nullptr,nullptr);
    for (size_t i=0;i<visibleVertices.size()/3;i++)
    {
        if (i==0)
            _meshBoundingBoxHalfSizes.setData(&visibleVertices[3*i+0]);
        else
            _meshBoundingBoxHalfSizes.keepMax(C3Vector(&visibleVertices[3*i+0]));
    }
}

C7Vector CShape::_acceptNewGeometry(const std::vector<double>& vert,const std::vector<int>& ind,const std::vector<double>* textCoord,const std::vector<double>* norm)
{
    TRACE_INTERNAL;
    C7Vector retVal;
    retVal.setIdentity();

    std::vector<double> wwert(vert);
    std::vector<int> wwind(ind);
    CMeshManip::removeNonReferencedVertices(wwert,wwind);
    CMesh* newGeomInfo=new CMesh(wwert,wwind,norm);
    if (textCoord!=nullptr)
    {
        std::vector<float> f;
        f.resize(textCoord->size());
        for (size_t i=0;i<textCoord->size();i++)
            f[i]=(float)textCoord->at(i);
        newGeomInfo->setTextureCoords(&f);
    }

    newGeomInfo->color.setDefaultValues();
    newGeomInfo->color.setColor(0.9f,0.9f,0.9f,sim_colorcomponent_ambient_diffuse);

    if (_mesh!=nullptr)
    {
        if (getMeshWrapper()->isMesh())
            getSingleMesh()->copyVisualAttributesTo(newGeomInfo); // copy a few properties (not all)
    }

    delete _mesh;
    _mesh=newGeomInfo;
    removeMeshCalculationStructure();

    // We align the bounding box:
    if (wwert.size()!=0)
    {
        retVal=CAlgos::getMeshBoundingBoxPose(wwert,wwind,true);
        getMeshWrapper()->preMultiplyAllVerticeLocalFrames(retVal.getInverse());
    }

    _computeMeshBoundingBox();
    computeBoundingBox();
    _meshModificationCounter++;
    return(retVal);
}

C7Vector CShape::_recomputeOrientation(C7Vector& m,bool alignWithMainAxis)
{ // This routine will reorient the shape according to its main axis if
  // alignWithMainAxis is true, and according to the world if false.
  // Don't forget to recompute (not done here) the new local transformation
  // matrices of the objects linked to that geometric resource!!
  // Input m is the cumulative transf. if alignWithMainAxis==false
  // Returned m is the new local transformation of this geometry

    removeMeshCalculationStructure();

    // 2. We set-up the absolute vertices and normal position/orientation:
    getMeshWrapper()->preMultiplyAllVerticeLocalFrames(m);

    // 3. We calculate the new orientation:
    std::vector<double> visibleVertices;
    std::vector<int> visibleIndices;
    getMeshWrapper()->getCumulativeMeshes(visibleVertices,&visibleIndices,nullptr);
    C7Vector tr;
    if (visibleVertices.size()!=0)
        tr=CAlgos::getMeshBoundingBoxPose(visibleVertices,visibleIndices,alignWithMainAxis);
    else
        tr.setIdentity();

    // 4. We apply it:
    getMeshWrapper()->preMultiplyAllVerticeLocalFrames(tr.getInverse());

    // 5. We recompute usual things:
    _computeMeshBoundingBox();
    computeBoundingBox();
    _meshModificationCounter++;
    return(tr);
}


C7Vector CShape::_recomputeTubeOrCuboidOrientation(C7Vector& m,bool tube,bool& error)
{ // This routine will reorient the tube shape according to its main axis
  // Don't forget to recompute (not done here) the new local transformation
  // matrices of the objects linked to that geometric resource!!
  // Input m is the cumulative transf.
  // Returned m is the new cumulative transformation of this geometry

    error=false; // no error yet
    C7Vector tr;

    // 0. We set-up the absolute vertices and retrieve them:
    getMeshWrapper()->preMultiplyAllVerticeLocalFrames(m);
    std::vector<double> visibleVertices;
    std::vector<int> visibleIndices;
    getMeshWrapper()->getCumulativeMeshes(visibleVertices,&visibleIndices,nullptr);

    // 1. We calculate the new orientation, based on the copy:
    bool success;
    if (tube)
        success=_getTubeReferenceFrame(visibleVertices,tr);
    else
        success=_getCuboidReferenceFrame(visibleVertices,visibleIndices,tr);
    if (!success)
    {
        error=true;
        getMeshWrapper()->preMultiplyAllVerticeLocalFrames(m.getInverse()); // don't forget to make operation backward before leaving!
        return(tr);
    }

    removeMeshCalculationStructure();

    // 4. We have the desired orientation (tr.Q), we now calculate the position (should be the same or very very close to what we have in tr.X)
    C7Vector trInv(tr.getInverse());
    C3Vector maxV,minV;
    for (size_t i=0;i<visibleVertices.size()/3;i++)
    {
        C3Vector v(&visibleVertices[3*i+0]);
        v=trInv*v;
        if (i==0)
        {
            maxV=v;
            minV=v;
        }
        else
        {
            maxV.keepMax(v);
            minV.keepMin(v);
        }
    }
    C3Vector newCenter((maxV(0)+minV(0))*0.5,(maxV(1)+minV(1))*0.5,(maxV(2)+minV(2))*0.5); // relative pos
    newCenter=tr*newCenter; // now abs pos
    tr.X=newCenter;

    // 5. We have the new center. We set all vertices relative to tr!!
    getMeshWrapper()->preMultiplyAllVerticeLocalFrames(tr.getInverse());

    // 6. We recompute usual things:
    _computeMeshBoundingBox();
    computeBoundingBox();
    _meshModificationCounter++;
    return(tr);
}

bool CShape::_getTubeReferenceFrame(const std::vector<double>& v,C7Vector& tr)
{
    tr.setIdentity();
    // 1) Do we have enough points?
    if (v.size()/3<6)
        return(false);
    // 2) Get the longest distance:
    int indexLeft=-1;
    int indexRight=-1;
    double longestDist=0.0;
    for (size_t i=0;i<v.size()/3;i++)
    {
        C3Vector pt1(&v[3*i+0]);
        for (size_t j=i+1;j<v.size()/3;j++)
        {
            C3Vector pt2(&v[3*j+0]);
            double l=(pt1-pt2).getLength();
            if (l>longestDist)
            {
                longestDist=l;
                indexLeft=int(i);
                indexRight=int(j);
            }
        }
    }
    if (indexLeft==-1)
        return(false); // all points are coincident!
    // 3) For each of the 2 found points, find 1 closest neighbour that is not coincident:
    C3Vector leftPt1(&v[3*indexLeft+0]);
    C3Vector rightPt1(&v[3*indexRight+0]);
    int indexLeft2=-1;
    int indexRight2=-1;
    double leftDist=FLOAT_MAX;
    double rightDist=FLOAT_MAX;
    for (size_t i=0;i<v.size()/3;i++)
    {
        C3Vector pt(&v[3*i+0]);
        if (int(i)!=indexLeft)
        {
            double l=(pt-leftPt1).getLength();
            if ( (l!=0.0)&&(l<leftDist) )
            {
                leftDist=l;
                indexLeft2=int(i);
            }
        }
        if (i!=indexRight)
        {
            double l=(pt-rightPt1).getLength();
            if ( (l!=0.0)&&(l<rightDist) )
            {
                rightDist=l;
                indexRight2=int(i);
            }
        }
    }
    if ((indexLeft2==-1)||(indexRight2==-1))
        return(false); // error

    // 4) For each of the 2 found segments, find 1 closest neighbour that forms a plane:
    C3Vector leftPt2(&v[3*indexLeft2+0]);
    C3Vector rightPt2(&v[3*indexRight2+0]);
    int indexLeft3=-1;
    int indexRight3=-1;
    leftDist=FLOAT_MAX;
    rightDist=FLOAT_MAX;
    for (size_t i=0;i<v.size()/3;i++)
    {
        C3Vector pt(&v[3*i+0]);
        if ( (int(i)!=indexLeft)&&(int(i)!=indexLeft2) )
        {
            double l1=(pt-leftPt1).getLength();
            double l2=(pt-leftPt2).getLength();
            if ( (l1!=0.0)&&(l2!=0.0)&&(l1<leftDist) )
            {

                double a=(leftPt1-pt).getAngle(leftPt2-pt);
                if ( (a>1.0*degToRad)&&(a<179.0*degToRad) )
                {
                    leftDist=l1;
                    indexLeft3=int(i);
                }
            }
        }
        if ((i!=indexRight)&&(i!=indexRight2))
        {
            double l1=(pt-rightPt1).getLength();
            double l2=(pt-rightPt2).getLength();
            if ( (l1!=0.0)&&(l2!=0.0)&&(l1<rightDist) )
            {

                double a=(rightPt1-pt).getAngle(rightPt2-pt);
                if ( (a>1.0*degToRad)&&(a<179.0*degToRad) )
                {
                    rightDist=l1;
                    indexRight3=int(i);
                }
            }
        }
    }
    if ( (indexLeft3==-1)||(indexRight3==-1) )
        return(false); // error

    // 5) Prepare the normal vectory of the 2 tube endings (direction doesn't matter):
    C3Vector leftPt3(&v[3*indexLeft3+0]);
    C3Vector rightPt3(&v[3*indexRight3+0]);
    C3Vector nLeft(((leftPt1-leftPt3)^(leftPt2-leftPt3)).getNormalized());
    C3Vector nRight(((rightPt1-rightPt3)^(rightPt2-rightPt3)).getNormalized());
    double a=nLeft.getAngle(nRight);
    if ( (a>1.0*degToRad)&&(a<179.0*degToRad) )
        return(false); // not precise enough

    // 6) Now get all points at each endings that are within 2% of distance to the end planes (relative to the longest distances) and calculate the average positions:
    C3Vector avgLeft,avgRight;
    avgLeft.clear();
    avgRight.clear();
    double cntLeft=0.0;
    double cntRight=0.0;
    for (size_t i=0;i<v.size()/3;i++)
    {
        C3Vector pt(&v[3*i+0]);
        C3Vector leftV(pt-leftPt1);
        double d=fabs(leftV*nLeft);
        if (d<longestDist*0.02)
        {
            cntLeft+=1.0;
            avgLeft+=pt;
        }

        C3Vector rightV(pt-rightPt1);
        d=fabs(rightV*nRight);
        if (d<longestDist*0.02)
        {
            cntRight+=1.0;
            avgRight+=pt;
        }
    }
    if ( (cntLeft<3.99)||(cntRight<3.99) ) // at least 4 points at each ending! (extruded triangle doesn't work anyway because it is not centered in the bounding box)
        return(false); // should not happen
    avgLeft/=cntLeft;
    avgRight/=cntRight;
    C3Vector avgPos((avgLeft+avgRight)*0.5);

    // 7) now compute a transformation matrix!
    C4X4Matrix m;
    m.setIdentity();
    m.X=avgPos;
    m.M.axis[2]=(avgLeft-avgRight).getNormalized();
    m.M.axis[0]=C3Vector(1.02,1.33,1.69).getNormalized(); // just a random vector;
    m.M.axis[1]=(m.M.axis[2]^m.M.axis[0]).getNormalized();
    m.M.axis[0]=(m.M.axis[1]^m.M.axis[2]).getNormalized();
    tr=m.getTransformation();

    // 8) Last: check if the bounding box is centered (e.g. a triangle-cylinder (with 3 faces)
    C3Vector maxV;
    C3Vector minV;
    C7Vector trInv(tr.getInverse());
    for (size_t i=0;i<v.size()/3;i++)
    {
        C3Vector pt(&v[3*i+0]);
        pt=trInv*pt;
        if (i==0)
        {
            maxV=pt;
            minV=pt;
        }
        else
        {
            maxV.keepMax(pt);
            minV.keepMin(pt);
        }
    }
    C3Vector dims(maxV(0)-minV(0),maxV(1)-minV(1),maxV(2)-minV(2));
    C3Vector vars(fabs(maxV(0)+minV(0)),fabs(maxV(1)+minV(1)),fabs(maxV(2)+minV(2)));
    for (size_t i=0;i<3;i++)
    {
        if (vars(i)/dims(1)>0.001) // 0.1% tolerance relative to the box dimension
            return(false); // the bounding box would not be centered!
    }
    return(true);
}

bool CShape::_getCuboidReferenceFrame(const std::vector<double>& v,const std::vector<int>& ind,C7Vector& tr)
{
    tr.setIdentity();
    // 1) Do we have enough points?
    if (v.size()/3<8)
        return(false);
    // 2) Get the biggest triangle (in surface)
    int biggestTriIndex=-1;
    double biggestTriSurface=0.0;
    C3Vector triangleN1;
    for (int i=0;i<int(ind.size())/3;i++)
    {
        C3Vector pt1(&v[3*ind[3*i+0]+0]);
        C3Vector pt2(&v[3*ind[3*i+1]+0]);
        C3Vector pt3(&v[3*ind[3*i+2]+0]);
        C3Vector v1(pt1-pt2);
        C3Vector v2(pt1-pt3);
        double s=(v1^v2).getLength();
        if (s>biggestTriSurface)
        {
            biggestTriSurface=s;
            biggestTriIndex=i;
            triangleN1=(v1^v2).getNormalized();
        }
    }
    if (biggestTriIndex==-1)
        return(false);

    // 3) Get the biggest triangle where the surface normal is perpendicular to the first triangle
    int biggestTriIndex2=-1;
    double biggestTriSurface2=0.0;
    C3Vector triangleN2;
    for (int i=0;i<int(ind.size())/3;i++)
    {
        if (i!=biggestTriIndex)
        {
            C3Vector pt1(&v[3*ind[3*i+0]+0]);
            C3Vector pt2(&v[3*ind[3*i+1]+0]);
            C3Vector pt3(&v[3*ind[3*i+2]+0]);
            C3Vector v1(pt1-pt2);
            C3Vector v2(pt1-pt3);
            double s=(v1^v2).getLength();
            C3Vector n((v1^v2).getNormalized());
            if ((s>biggestTriSurface2)&&(fabs(triangleN1*n)<0.0001))
            {
                biggestTriSurface2=s;
                biggestTriIndex2=i;
                triangleN2=n;
            }
        }
    }
    if (biggestTriIndex2==-1)
        return(false);


    // 4) now compute a transformation matrix!
    C4X4Matrix m;
    m.setIdentity();
    m.X.clear();
    m.M.axis[0]=triangleN1;
    m.M.axis[1]=triangleN2;
    m.M.axis[2]=(m.M.axis[0]^m.M.axis[1]).getNormalized();

    // 4) Get the center!
    C3Vector maxV;
    C3Vector minV;
    C4X4Matrix mInv(m.getInverse());
    for (int i=0;i<int(v.size())/3;i++)
    {
        C3Vector pt(&v[3*i+0]);
        pt=mInv*pt;
        if (i==0)
        {
            maxV=pt;
            minV=pt;
        }
        else
        {
            maxV.keepMax(pt);
            minV.keepMin(pt);
        }
    }
    C3Vector avgPos((maxV(0)+minV(0))*0.5,(maxV(1)+minV(1))*0.5,(maxV(2)+minV(2))*0.5);
    avgPos=m*avgPos;
    m.X=avgPos;

    // 5) get the dimensions and reorient the frame to have z the longest dim, y the second longest
    C3Vector dim(maxV(0)-minV(0),maxV(1)-minV(1),maxV(2)-minV(2));
    C3X3Matrix rot;
    rot.setIdentity();
    double xDim=dim(0);
    double yDim=dim(1);
    if ((dim(0)>dim(1))&&(dim(0)>dim(2)))
    {
        rot.buildYRotation(piValD2);
        xDim=dim(2);
    }
    if ((dim(1)>dim(0))&&(dim(1)>dim(2)))
    {
        rot.buildXRotation(piValD2);
        yDim=dim(2);
    }
    m.M*=rot;
    // z has the biggest dimension now
    if (yDim<xDim)
    {
        rot.buildZRotation(piValD2);
        m.M*=rot;
    }
    // ok, now we have z,y,x ordered from largest to smallest
    tr=m.getTransformation();
    return(true);
}

void CShape::scaleMesh(double xVal,double yVal,double zVal)
{   // The geometric resource is scaled and the bounding box is recomputed.
    double xp,yp,zp;
    scaleMesh(xVal,yVal,zVal,xp,yp,zp);
}

void CShape::scaleMesh(double x,double y,double z,double& xp,double& yp,double& zp)
{   // The geometric resource is scaled and the bounding box is recomputed.
    // Normals are not recomputed if xVal==yVal==yVal
    _meshModificationCounter++;
    if (getMeshWrapper()->isPure())
    { // we have some constraints in case we have a pure mesh (or several pure meshes in a group)
        if (getMeshWrapper()->isMesh())
        { // we have a pure mesh (non-group)
            int purePrim=getSingleMesh()->getPurePrimitiveType();
            if (purePrim==sim_primitiveshape_plane)
                z=1.0;
            if (purePrim==sim_primitiveshape_disc)
            {
                z=1.0;
                y=x;
            }
            if ( (purePrim==sim_primitiveshape_spheroid)||(purePrim==sim_primitiveshape_capsule) )
            {
                y=x;
                z=x;
            }
            if ( (purePrim==sim_primitiveshape_cylinder)||(purePrim==sim_primitiveshape_cone)||(purePrim==sim_primitiveshape_heightfield) )
                y=x;
        }
    }
    if (!getMeshWrapper()->isMesh())
    { // we have a group. We do iso-scaling!
        y=x;
        z=x;
    }

    // Scale collision info if we have an isometric scaling:
    if ( (x==y)&&(x==z)&&(_meshCalculationStructure!=nullptr) )
        CPluginContainer::geomPlugin_scaleMesh(_meshCalculationStructure,x);
    else
        removeMeshCalculationStructure(); // we have to recompute it!

    // Scale meshes and adjust textures:
    getMeshWrapper()->scale(x,y,z);

    // recompute the bounding box:
    _computeMeshBoundingBox();
    computeBoundingBox();

    xp=x;
    yp=y;
    zp=z;
}

int CShape::getMeshModificationCounter()
{
    return(_meshModificationCounter);
}

CMeshWrapper* CShape::getMeshWrapper() const
{
    return(_mesh);
}

CMesh* CShape::getSingleMesh() const
{
    CMesh* retVal=nullptr;
    if (_mesh->isMesh())
        retVal=(CMesh*)_mesh;
    return(retVal);
}

void CShape::disconnectMesh()
{
    _mesh=nullptr;
}

CDynMaterialObject* CShape::getDynMaterial()
{
    return(_dynMaterial);
}

void CShape::setDynMaterial(CDynMaterialObject* mat)
{
    delete _dynMaterial;
    _dynMaterial=mat;
}

C3Vector CShape::getInitialDynamicLinearVelocity()
{
    return(_initialDynamicLinearVelocity);
}

void CShape::setInitialDynamicLinearVelocity(const C3Vector& vel)
{
    _initialDynamicLinearVelocity=vel;
}

C3Vector CShape::getInitialDynamicAngularVelocity()
{
    return(_initialDynamicAngularVelocity);
}

void CShape::setInitialDynamicAngularVelocity(const C3Vector& vel)
{
    _initialDynamicAngularVelocity=vel;
}

void CShape::setRigidBodyWasAlreadyPutToSleepOnce(bool s)
{
    _rigidBodyWasAlreadyPutToSleepOnce=s;
}

bool CShape::getRigidBodyWasAlreadyPutToSleepOnce()
{
    return(_rigidBodyWasAlreadyPutToSleepOnce);
}

void CShape::actualizeContainsTransparentComponent()
{
    _containsTransparentComponents=getMeshWrapper()->getContainsTransparentComponents();
}

bool CShape::getContainsTransparentComponent()
{
    return(_containsTransparentComponents);
}

void CShape::prepareVerticesIndicesNormalsAndEdgesForSerialization()
{
    getMeshWrapper()->prepareVerticesIndicesNormalsAndEdgesForSerialization();
}

void CShape::setDynamicCollisionMask(unsigned short m)
{
    _dynamicCollisionMask=m;
}

unsigned short CShape::getDynamicCollisionMask()
{
    return(_dynamicCollisionMask);
}

CSceneObject* CShape::getLastParentForLocalGlobalRespondable()
{
    CSceneObject* retVal;
    if (_lastParentForLocalGlobalRespondable==nullptr)
    {
        retVal=this;
        while (retVal->getParent()!=nullptr)
            retVal=retVal->getParent();
        _lastParentForLocalGlobalRespondable=retVal;
    }
    else
        retVal=_lastParentForLocalGlobalRespondable;
    return(retVal);
}

void CShape::clearLastParentForLocalGlobalRespondable()
{
    _lastParentForLocalGlobalRespondable=nullptr;
}

std::string CShape::getObjectTypeInfo() const
{
    return("Shape");
}

std::string CShape::getObjectTypeInfoExtended() const
{
    if (getMeshWrapper()->isMesh())
    {
        int pureType=getSingleMesh()->getPurePrimitiveType();
        if (pureType==sim_primitiveshape_none)
            return("Shape");
        if (pureType==sim_primitiveshape_plane)
            return("Shape (plane)");
        if (pureType==sim_primitiveshape_disc)
            return("Shape (disc)");
        if (pureType==sim_primitiveshape_cuboid)
            return("Shape (cuboid)");
        if (pureType==sim_primitiveshape_spheroid)
            return("Shape (spheroid)");
        if (pureType==sim_primitiveshape_cylinder)
            return("Shape (cylinder)");
        if (pureType==sim_primitiveshape_cone)
            return("Shape (cone)");
        if (pureType==sim_primitiveshape_capsule)
            return("Shape (capsule)");
        if (pureType==sim_primitiveshape_heightfield)
            return("Shape (heightfield)");
    }
    else
    {
        if (!getMeshWrapper()->isPure())
            return("Shape (compound)");
        else
            return("Shape (compound primitive)");
    }
    return("ERROR");
}
bool CShape::isPotentiallyCollidable() const
{
    return(true);
}
bool CShape::isPotentiallyMeasurable() const
{
    return(true);
}
bool CShape::isPotentiallyDetectable() const
{
    return(true);
}
bool CShape::isPotentiallyRenderable() const
{
    return(true);
}

void CShape::computeBoundingBox()
{
    _setBoundingBox(getBoundingBoxHalfSizes()*-1.0,getBoundingBoxHalfSizes());
}

void CShape::commonInit()
{
    _objectType=sim_object_shape_type;
    _containsTransparentComponents=false;
    _startInDynamicSleeping=false;
    _shapeIsDynamicallyStatic=true;
    _shapeIsDynamicallyKinematic=true;
    _setAutomaticallyToNonStaticIfGetsParent=false;
    _shapeIsDynamicallyRespondable=false; // keep false, otherwise too many "default" problems
    _respondableSuspendCount=0;
    _dynamicCollisionMask=0xffff;
    _lastParentForLocalGlobalRespondable=nullptr;
    _initialDynamicLinearVelocity.clear();
    _initialDynamicAngularVelocity.clear();

    _visibilityLayer=SHAPE_LAYER;
    _localObjectSpecialProperty=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable|sim_objectspecialproperty_detectable|sim_objectspecialproperty_renderable;
    _objectAlias="Shape";
    _objectName_old="Shape";
    _objectAltName_old=tt::getObjectAltNameFromObjectName(_objectName_old.c_str());

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();

    _meshCalculationStructure=nullptr;
    _mesh=nullptr;
    _meshModificationCounter=0;

    _dynMaterial=new CDynMaterialObject();
}

void CShape::setDynamicVelocity(const C3Vector& linearV,const C3Vector& angularV)
{
    _dynamicLinearVelocity=linearV;
    _dynamicAngularVelocity=angularV;
}

C3Vector CShape::getDynamicLinearVelocity()
{
    return(_dynamicLinearVelocity);
}

C3Vector CShape::getDynamicAngularVelocity()
{
    return(_dynamicAngularVelocity);
}

void CShape::addAdditionalForceAndTorque(const C3Vector& f,const C3Vector& t)
{
    _additionalForce+=f;
    _additionalTorque+=t;
}

void CShape::clearAdditionalForceAndTorque()
{
    clearAdditionalForce();
    clearAdditionalTorque();
}

void CShape::clearAdditionalForce()
{
    _additionalForce.clear();
}

void CShape::clearAdditionalTorque()
{
    _additionalTorque.clear();
}

C3Vector CShape::getAdditionalForce()
{
    return(_additionalForce);
}

C3Vector CShape::getAdditionalTorque()
{
    return(_additionalTorque);
}

void CShape::setRespondable(bool r)
{
    _shapeIsDynamicallyRespondable=r;
}

bool CShape::getRespondable()
{
    return( _shapeIsDynamicallyRespondable&&(_respondableSuspendCount==0) );
}

bool CShape::getSetAutomaticallyToNonStaticIfGetsParent()
{
    return(_setAutomaticallyToNonStaticIfGetsParent);
}

void CShape::setSetAutomaticallyToNonStaticIfGetsParent(bool autoNonStatic)
{
    _setAutomaticallyToNonStaticIfGetsParent=autoNonStatic;
}

bool CShape::getStartInDynamicSleeping() const
{
    return(_startInDynamicSleeping);
}

void CShape::setStartInDynamicSleeping(bool sleeping)
{
    _startInDynamicSleeping=sleeping;
}

bool CShape::getShapeIsDynamicallyStatic() const
{
    return(_shapeIsDynamicallyStatic);
}

void CShape::setShapeIsDynamicallyStatic(bool sta)
{
    _shapeIsDynamicallyStatic=sta;
    if (!sta)
        _setAutomaticallyToNonStaticIfGetsParent=false;
}

bool CShape::getShapeIsDynamicallyKinematic() const
{
    bool retVal=_shapeIsDynamicallyKinematic;
    if (_objectAlias=="Floor")
        retVal=false;
    return(retVal);
}

void CShape::setShapeIsDynamicallyKinematic(bool kin)
{
    _shapeIsDynamicallyKinematic=kin;
    if (_objectAlias=="Floor")
        _shapeIsDynamicallyKinematic=false;
}

void CShape::setInsideAndOutsideFacesSameColor_DEPRECATED(bool s)
{
    if (getMeshWrapper()->isMesh())
        getSingleMesh()->setInsideAndOutsideFacesSameColor_DEPRECATED(s);
}
bool CShape::getInsideAndOutsideFacesSameColor_DEPRECATED()
{
    if (getMeshWrapper()->isMesh())
        return(getSingleMesh()->getInsideAndOutsideFacesSameColor_DEPRECATED());
    return(true);
}

bool CShape::isCompound() const
{
    return(!getMeshWrapper()->isMesh());
}

int CShape::getEdgeWidth_DEPRECATED() const
{
    if (getMeshWrapper()->isMesh())
        return(getSingleMesh()->getEdgeWidth_DEPRECATED());
    return(0);
}

void CShape::setEdgeWidth_DEPRECATED(int w)
{
    w=tt::getLimitedInt(1,4,w);
    if (getMeshWrapper()->isMesh())
        getSingleMesh()->setEdgeWidth_DEPRECATED(w);
}

void CShape::display_extRenderer(CViewableBase* renderingObject,int displayAttrib)
{
    if (getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if (renderingObject->isObjectInsideView(getFullCumulativeTransformation(),getBoundingBoxHalfSizes()))
        { // the bounding box is inside of the view (at least some part of it!)
            C7Vector tr=getCumulativeTransformation();
            int componentIndex=0;
            getMeshWrapper()->display_extRenderer(this,displayAttrib,tr,_objectHandle,componentIndex);
        }
    }
}

void CShape::scaleObject(double scalingFactor)
{   
    scaleMesh(scalingFactor,scalingFactor,scalingFactor); // will set the geomObject dynamics full refresh flag!
    CSceneObject::scaleObject(scalingFactor);
    pushObjectRefreshEvent();
    _dynamicsResetFlag=true;
}

void CShape::scaleObjectNonIsometrically(double x,double y,double z)
{
    double xp,yp,zp;
    scaleMesh(x,y,z,xp,yp,zp); // will set the geomObject dynamics full refresh flag!
    CSceneObject::scaleObjectNonIsometrically(xp,yp,zp);
    pushObjectRefreshEvent();
    _dynamicsResetFlag=true;
}

void CShape::announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceObjectWillBeErased(object,copyBuffer);
    if (getMeshWrapper()!=nullptr)
        getMeshWrapper()->announceSceneObjectWillBeErased(object); // for textures based on vision sensors
}

void CShape::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollectionWillBeErased(groupID,copyBuffer);
}
void CShape::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID,copyBuffer);
}
void CShape::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID,copyBuffer);
}
void CShape::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID,copyBuffer);
}

void CShape::performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map,loadingAmodel);
    getMeshWrapper()->performSceneObjectLoadingMapping(map);
}
void CShape::performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map,loadingAmodel);
}
void CShape::performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map,loadingAmodel);
}
void CShape::performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map,loadingAmodel);
}
void CShape::performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map,loadingAmodel);
}

void CShape::performTextureObjectLoadingMapping(const std::map<int,int>* map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
    getMeshWrapper()->performTextureObjectLoadingMapping(map);
}

void CShape::performDynMaterialObjectLoadingMapping(const std::map<int,int>* map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
    getMeshWrapper()->performDynMaterialObjectLoadingMapping(map);
}

void CShape::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();
    _initialInitialDynamicLinearVelocity=_initialDynamicLinearVelocity;
    _initialInitialDynamicAngularVelocity=_initialDynamicAngularVelocity;
    _respondableSuspendCount=0;

    actualizeContainsTransparentComponent(); // added on 2010/11/22 to correct at least each time a simulation starts, when those values where not set correctly
}

void CShape::simulationAboutToStart()
{
    initializeInitialValues(false);
    _rigidBodyWasAlreadyPutToSleepOnce=false;
    CSceneObject::simulationAboutToStart();
}

void CShape::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
        {
            _initialDynamicLinearVelocity=_initialInitialDynamicLinearVelocity;
            _initialDynamicAngularVelocity=_initialInitialDynamicAngularVelocity;
        }
    }

    _dynamicLinearVelocity.clear();
    _dynamicAngularVelocity.clear();
    _additionalForce.clear();
    _additionalTorque.clear();
    _respondableSuspendCount=0;
    CSceneObject::simulationEnded();
}

void CShape::serialize(CSer& ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {   // Storing
            // Following tags are reserved (11/11/2012): Sco, Sc2, Eco, Ewt

            if (true)
            { // keep until 2022 for back compatibility
                ar.storeDataName("Ge2");
                ar.setCountingMode();
                _serializeBackCompatibility(ar);
                if (ar.setWritingMode())
                    _serializeBackCompatibility(ar);
            }
            else
            { // following was previously located in CGeomProxy:
                if (_mesh->isMesh())
                    ar.storeDataName("Gst");
                else
                    ar.storeDataName("Gsg");
                ar.setCountingMode();
                _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                if (ar.setWritingMode())
                    _mesh->serialize(ar,getObjectAliasAndHandle().c_str());

                if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp()&&isMeshCalculationStructureInitialized())
                {

                    std::vector<unsigned char> serializationData;
                    CPluginContainer::geomPlugin_getMeshSerializationData(_meshCalculationStructure,serializationData);
                    ar.storeDataName("_oi");
                    ar.setCountingMode(true);
                    for (int i=0;i<serializationData.size();i++)
                        ar << serializationData[i];
                    ar.flush(false);
                    if (ar.setWritingMode(true))
                    {
                        for (int i=0;i<serializationData.size();i++)
                            ar << serializationData[i];
                        ar.flush(false);
                    }
                }
            }

            ar.storeDataName("Mat");
            ar.setCountingMode();
            _dynMaterial->serialize(ar);
            if (ar.setWritingMode())
                _dynMaterial->serialize(ar);

            ar.storeDataName("Dc2");
            ar << _dynamicCollisionMask;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Idv");
            ar << (float)_initialDynamicLinearVelocity(0) << (float)_initialDynamicLinearVelocity(1) << (float)_initialDynamicLinearVelocity(2);
            ar << (float)_initialDynamicAngularVelocity(0) << (float)_initialDynamicAngularVelocity(1) << (float)_initialDynamicAngularVelocity(2);
            ar.flush();
#endif

            ar.storeDataName("_dv");
            ar << _initialDynamicLinearVelocity(0) << _initialDynamicLinearVelocity(1) << _initialDynamicLinearVelocity(2);
            ar << _initialDynamicAngularVelocity(0) << _initialDynamicAngularVelocity(1) << _initialDynamicAngularVelocity(2);
            ar.flush();


            ar.storeDataName("Sss");
            unsigned char nothing=0;
    //        SIM_SET_CLEAR_BIT(nothing,0,_shapeIsDynamicallyKinematic); reserved since 2022
            SIM_SET_CLEAR_BIT(nothing,1,!_shapeIsDynamicallyKinematic);
    //      SIM_SET_CLEAR_BIT(nothing,2,_culling); removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,3,tracing); removed on 13/09/2011
    //      SIM_SET_CLEAR_BIT(nothing,4,_shapeWireframe); removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,5,_displayInvertedFaces); removed on 2010/04/19
            SIM_SET_CLEAR_BIT(nothing,6,_startInDynamicSleeping);
            SIM_SET_CLEAR_BIT(nothing,7,!_shapeIsDynamicallyStatic);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Ss3");
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_shapeIsDynamicallyRespondable);
    //      SIM_SET_CLEAR_BIT(nothing,1,_visualizeInertia); // removed on 16/12/2012
            SIM_SET_CLEAR_BIT(nothing,2,false); // removed on 01/05/2020
    //      SIM_SET_CLEAR_BIT(nothing,3,!_insideAndOutsideFacesSameColor); // removed on 11/11/2012
    //      SIM_SET_CLEAR_BIT(nothing,4,_containsTransparentComponents); // removed on 11/11/2012
            SIM_SET_CLEAR_BIT(nothing,5,_setAutomaticallyToNonStaticIfGetsParent);
            ar << nothing;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Ge2")==0)
                    { // keep until 2024 for back compatibility
                        noHit=false;
                        ar >> byteQuantity; 
                        _serializeBackCompatibility(ar);
                        getMeshWrapper()->containsOnlyPureConvexShapes(); // needed since there was a bug where pure planes and pure discs were considered as convex
                    }
                    if (theName.compare("Gst")==0)
                    { // geometric (not yet used, but so that old versions will be able to read this)
                        noHit=false;
                        ar >> byteQuantity; 
                        delete _mesh;
                        _mesh=new CMesh();
                        ((CMesh*)_mesh)->serialize(ar,getObjectAliasAndHandle().c_str());
                        _computeMeshBoundingBox();
                        getMeshWrapper()->containsOnlyPureConvexShapes(); // needed since there was a bug where pure planes and pure discs were considered as convex
                    }
                    if (theName.compare("Gsg")==0)
                    { // geomWrap (not yet used, but so that old versions will be able to read this)
                        noHit=false;
                        ar >> byteQuantity; 
                        delete _mesh;
                        _mesh=new CMeshWrapper();
                        _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                        _computeMeshBoundingBox();
                        getMeshWrapper()->containsOnlyPureConvexShapes(); // needed since there was a bug where pure planes and pure discs were considered as convex
                    }

                    if (theName.compare("_oi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo never stores calc structures)

                        std::vector<unsigned char> data;
                        data.reserve(byteQuantity);
                        unsigned char dummy;
                        for (int i=0;i<byteQuantity;i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        _meshCalculationStructure=CPluginContainer::geomPlugin_getMeshFromSerializationData(&data[0]);
                    }

                    if (theName.compare("Mat")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; 
                        _dynMaterial->serialize(ar);
                    }

                    if (theName.compare("Dc2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _dynamicCollisionMask;
                    }

                    if (theName.compare("Idv")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        float bla,bli,blo;
                        ar >> bla >> bli >> blo;
                        _initialDynamicLinearVelocity(0)=(double)bla;
                        _initialDynamicLinearVelocity(1)=(double)bli;
                        _initialDynamicLinearVelocity(2)=(double)blo;
                        ar >> bla >> bli >> blo;
                        _initialDynamicAngularVelocity(0)=(double)bla;
                        _initialDynamicAngularVelocity(1)=(double)bli;
                        _initialDynamicAngularVelocity(2)=(double)blo;
                    }

                    if (theName.compare("_dv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _initialDynamicLinearVelocity(0) >> _initialDynamicLinearVelocity(1) >> _initialDynamicLinearVelocity(2);
                        ar >> _initialDynamicAngularVelocity(0) >> _initialDynamicAngularVelocity(1) >> _initialDynamicAngularVelocity(2);
                    }


                    if (theName=="Sss")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyKinematic=!SIM_IS_BIT_SET(nothing,1);
                        _startInDynamicSleeping=SIM_IS_BIT_SET(nothing,6);
                        _shapeIsDynamicallyStatic=!SIM_IS_BIT_SET(nothing,7);
                    }
                    if (theName=="Ss2")
                    { // keep for backward compatibility (2010/07/12)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable=SIM_IS_BIT_SET(nothing,0);
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                    }
                    if (theName=="Ss3")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _shapeIsDynamicallyRespondable=SIM_IS_BIT_SET(nothing,0);
                        // _parentFollowsDynamic=SIM_IS_BIT_SET(nothing,2); removed on 01/05/2020
                        _setAutomaticallyToNonStaticIfGetsParent=SIM_IS_BIT_SET(nothing,5);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            actualizeContainsTransparentComponent();
            computeBoundingBox();
            if (_objectAlias=="Floor")
                _shapeIsDynamicallyKinematic=false;
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (exhaustiveXml)
        { // for non-exhaustive, is done in CSceneObjectContainer
            if (ar.isStoring())
            {
                if (true)
                { // keep until 2022 for back compatibility
                    ar.xmlPushNewNode("meshProxy");
                    _serializeBackCompatibility(ar);
                    ar.xmlPopNode();
                }
                else
                { // following previously located in geomProxy:
                    ar.clearIncrementCounter();
                    if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp()&&isMeshCalculationStructureInitialized())
                    {
                        std::vector<unsigned char> collInfoData;
                        CPluginContainer::geomPlugin_getMeshSerializationData(_meshCalculationStructure,collInfoData);
                        ar.xmlPushNewNode("calculationStructure");
                        if (ar.xmlSaveDataInline(int(collInfoData.size())))
                        {
                            std::string str(base64_encode(&collInfoData[0],(unsigned int)collInfoData.size()));
                            ar.xmlAddNode_string("data_base64Coded",str.c_str());
                        }
                        else
                            ar.xmlAddNode_binFile("file",(std::string("calcStruct_")+_objectAlias+"-"+std::to_string(_objectHandle)).c_str(),&collInfoData[0],collInfoData.size());
                        ar.xmlPopNode();
                    }

                    if (_mesh->isMesh())
                        ar.xmlPushNewNode("mesh");
                    else
                        ar.xmlPushNewNode("compound");
                    _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }

                ar.xmlPushNewNode("dynamics");
                ar.xmlAddNode_int("respondableMask",_dynamicCollisionMask);
                C3Vector vel=_initialDynamicLinearVelocity;
                vel*=180.0/piValue;
                ar.xmlAddNode_floats("initialLinearVelocity",vel.data,3);
                vel=_initialDynamicAngularVelocity;
                vel*=180.0/piValue;
                ar.xmlAddNode_floats("initialAngularVelocity",vel.data,3);
                ar.xmlPushNewNode("switches");
                ar.xmlAddNode_bool("static",_shapeIsDynamicallyStatic);
                ar.xmlAddNode_bool("kinematic",_shapeIsDynamicallyKinematic);
                ar.xmlAddNode_bool("respondable",_shapeIsDynamicallyRespondable);
                // ar.xmlAddNode_bool("parentFollows",_parentFollowsDynamic); removed on 01/05/2020
                ar.xmlAddNode_bool("startSleeping",_startInDynamicSleeping);
                ar.xmlAddNode_bool("setToDynamicIfGetsParent",_setAutomaticallyToNonStaticIfGetsParent);
                ar.xmlPopNode();
                ar.xmlPushNewNode("material");
                _dynMaterial->serialize(ar);
                ar.xmlPopNode();
                ar.xmlPopNode();
            }
            else
            {
                bool meshProxyNotPresent=true;
                if (ar.xmlPushChildNode("meshProxy",false))
                { // keep until about 2024
                    _serializeBackCompatibility(ar);
                    ar.xmlPopNode();
                    meshProxyNotPresent=false;
                }
                if (ar.xmlPushChildNode("calculationStructure",false))
                { // (not yet used, but so that old versions will be able to read this)
                    std::string str;
                    if (ar.xmlGetNode_string("data_base64Coded",str,false))
                        str=base64_decode(str);
                    else
                        ar.xmlGetNode_binFile("file",str);

                    std::vector<double> wvert;
                    std::vector<int> wind;
                    _mesh->getCumulativeMeshes(wvert,&wind,nullptr);
                    _meshCalculationStructure=CPluginContainer::geomPlugin_getMeshFromSerializationData((unsigned char*)str.c_str());
                    ar.xmlPopNode();
                }
                if (ar.xmlPushChildNode("mesh",false))
                { // (not yet used, but so that old versions will be able to read this)
                    delete _mesh;
                    _mesh=new CMesh();
                    ((CMesh*)_mesh)->serialize(ar,getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                    _computeMeshBoundingBox();
                }
                else
                {
                    if (ar.xmlPushChildNode("compound",meshProxyNotPresent))
                    {  // (not yet used, but so that old versions will be able to read this)
                        delete _mesh;
                        _mesh=new CMeshWrapper();
                        _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                        ar.xmlPopNode();
                        _computeMeshBoundingBox();
                    }
                }

                if (ar.xmlPushChildNode("dynamics"))
                {
                    int m;
                    ar.xmlGetNode_int("respondableMask",m);
                    _dynamicCollisionMask=(unsigned short)m;
                    C3Vector vel;
                    ar.xmlGetNode_floats("initialLinearVelocity",vel.data,3);
                    _initialDynamicLinearVelocity=vel*piValue/180.0;
                    ar.xmlGetNode_floats("initialAngularVelocity",vel.data,3);
                    _initialDynamicAngularVelocity=vel*piValue/180.0;
                    if (ar.xmlPushChildNode("switches"))
                    {
                        ar.xmlGetNode_bool("static",_shapeIsDynamicallyStatic);
                        ar.xmlGetNode_bool("kinematic",_shapeIsDynamicallyKinematic);
                        ar.xmlGetNode_bool("respondable",_shapeIsDynamicallyRespondable);
                        // ar.xmlGetNode_bool("parentFollows",_parentFollowsDynamic);  removed on 01/05/2020
                        ar.xmlGetNode_bool("startSleeping",_startInDynamicSleeping);
                        ar.xmlGetNode_bool("setToDynamicIfGetsParent",_setAutomaticallyToNonStaticIfGetsParent);
                        ar.xmlPopNode();
                    }
                    if (ar.xmlPushChildNode("material"))
                    {
                        _dynMaterial->serialize(ar);
                        ar.xmlPopNode();
                    }
                    ar.xmlPopNode();
                }
                computeBoundingBox();
            }
        }
    }
}

void CShape::_serializeBackCompatibility(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            // geomInfo has to be stored before collInfo!!!
            if (_mesh->isMesh())
                ar.storeDataName("Gst");
            else
                ar.storeDataName("Gsg");
            ar.setCountingMode();
            _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
            if (ar.setWritingMode())
                _mesh->serialize(ar,getObjectAliasAndHandle().c_str());

            // (if undo/redo under way, getSaveExistingCalculationStructuresTemp is false)
            if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp()&&isMeshCalculationStructureInitialized())
            {

                std::vector<unsigned char> serializationData;
                CPluginContainer::geomPlugin_getMeshSerializationData(_meshCalculationStructure,serializationData);
                ar.storeDataName("Coi");
                ar.setCountingMode(true);
                for (int i=0;i<serializationData.size();i++)
                    ar << serializationData[i];
                ar.flush(false);
                if (ar.setWritingMode(true))
                {
                    for (int i=0;i<serializationData.size();i++)
                        ar << serializationData[i];
                    ar.flush(false);
                }
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;

                    if (theName.compare("Gst")==0)
                    { // geometric
                        noHit=false;
                        ar >> byteQuantity; 
                        delete _mesh;
                        _mesh=new CMesh();
                        ((CMesh*)_mesh)->serialize(ar,getObjectAliasAndHandle().c_str());
                    }
                    if (theName.compare("Gsg")==0)
                    { // geomWrap
                        noHit=false;
                        ar >> byteQuantity; 
                        delete _mesh;
                        _mesh=new CMeshWrapper();
                        _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                    }

                    if (theName.compare("Coi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo never stores calc structures)

                        std::vector<unsigned char> data;
                        data.reserve(byteQuantity);
                        unsigned char dummy;
                        for (int i=0;i<byteQuantity;i++)
                        {
                            ar >> dummy;
                            data.push_back(dummy);
                        }
                        std::vector<double> wvert;
                        std::vector<int> wind;
                        _mesh->getCumulativeMeshes(wvert,&wind,nullptr);
                        _meshCalculationStructure=CPluginContainer::geomPlugin_getMeshFromSerializationData(&data[0]);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _computeMeshBoundingBox();
            computeBoundingBox();
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.clearIncrementCounter();
            if (App::currentWorld->environment->getSaveExistingCalculationStructuresTemp()&&isMeshCalculationStructureInitialized())
            {
                std::vector<unsigned char> collInfoData;
                CPluginContainer::geomPlugin_getMeshSerializationData(_meshCalculationStructure,collInfoData);
                ar.xmlPushNewNode("calculationStructure");
                if (ar.xmlSaveDataInline(int(collInfoData.size())))
                {
                    std::string str(base64_encode(&collInfoData[0],(unsigned int)collInfoData.size()));
                    ar.xmlAddNode_string("data_base64Coded",str.c_str());
                }
                else
                    ar.xmlAddNode_binFile("file",(std::string("calcStruct_")+getObjectAliasAndHandle()).c_str(),&collInfoData[0],collInfoData.size());
                ar.xmlPopNode();
            }

            if (_mesh->isMesh())
                ar.xmlPushNewNode("mesh");
            else
                ar.xmlPushNewNode("compound");
            _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
            ar.xmlPopNode();
        }
        else
        {
            if (ar.xmlPushChildNode("calculationStructure",false))
            {
                std::string str;
                if (ar.xmlGetNode_string("data_base64Coded",str,false))
                    str=base64_decode(str);
                else
                    ar.xmlGetNode_binFile("file",str);

                std::vector<double> wvert;
                std::vector<int> wind;
                _mesh->getCumulativeMeshes(wvert,&wind,nullptr);
                _meshCalculationStructure=CPluginContainer::geomPlugin_getMeshFromSerializationData((unsigned char*)str.c_str());
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("mesh",false))
            {
                delete _mesh;
                _mesh=new CMesh();
                ((CMesh*)_mesh)->serialize(ar,getObjectAliasAndHandle().c_str());
                ar.xmlPopNode();
            }
            else
            {
                if (ar.xmlPushChildNode("compound"))
                {
                    delete _mesh;
                    _mesh=new CMeshWrapper();
                    _mesh->serialize(ar,getObjectAliasAndHandle().c_str());
                    ar.xmlPopNode();
                }
            }
            _computeMeshBoundingBox();
            computeBoundingBox();
        }
    }
}

void CShape::alignBoundingBoxWithMainAxis()
{
    _reorientGeometry(0);
}
void CShape::alignBoundingBoxWithWorld()
{
    _reorientGeometry(1);
}

bool CShape::alignTubeBoundingBoxWithMainAxis()
{
    return(_reorientGeometry(2));
}

bool CShape::alignCuboidBoundingBoxWithMainAxis()
{
    return(_reorientGeometry(3));
}

bool CShape::_reorientGeometry(int type)
{ // return value is the success state of the operation
    C7Vector m(getCumulativeTransformation());
    C7Vector mNew;
    bool error=false;
    if (type<2)
        mNew=_recomputeOrientation(m,type==0);
    if (type==2)
        mNew=_recomputeTubeOrCuboidOrientation(m,true,error);
    if (type==3)
        mNew=_recomputeTubeOrCuboidOrientation(m,false,error);

    if (error)
        return(false);

    C7Vector mCorr(m.getInverse()*mNew);
    C7Vector mCorrInv(mCorr.getInverse());

    // Now we have to compute the new local transformation:
    setLocalTransformation(getLocalTransformation()*mCorr);
    if (_isInScene)
        App::currentWorld->drawingCont->adjustForFrameChange(_objectHandle,mCorrInv);
    for (size_t i=0;i<getChildCount();i++)
    {
        CSceneObject* child=getChildFromIndex(i);
        child->setLocalTransformation(mCorrInv*child->getLocalTransformation());
    }
    App::setFullDialogRefreshFlag(); // so that textures and other things get updated!
    pushObjectRefreshEvent();
    return(true);
}

void CShape::removeMeshCalculationStructure()
{
    TRACE_INTERNAL;
    if (_meshCalculationStructure!=nullptr)
    {
        CPluginContainer::geomPlugin_destroyMesh(_meshCalculationStructure);
        _meshCalculationStructure=nullptr;
    }
}

bool CShape::isMeshCalculationStructureInitialized()
{
    return(_meshCalculationStructure!=nullptr);
}

void CShape::initializeMeshCalculationStructureIfNeeded()
{
    if ((_meshCalculationStructure==nullptr)&&(_mesh!=nullptr))
    {
        std::vector<double> wvert;
        std::vector<int> wind;
        _mesh->getCumulativeMeshes(wvert,&wind,nullptr);
        double maxTriSize=App::currentWorld->environment->getCalculationMaxTriangleSize();
        double minTriSize=(std::max<double>(std::max<double>(_meshBoundingBoxHalfSizes(0),_meshBoundingBoxHalfSizes(1)),_meshBoundingBoxHalfSizes(2)))*2.0*App::currentWorld->environment->getCalculationMinRelTriangleSize();
        if (maxTriSize<minTriSize)
            maxTriSize=minTriSize;
        _meshCalculationStructure=CPluginContainer::geomPlugin_createMesh(&wvert[0],(int)wvert.size(),&wind[0],(int)wind.size(),nullptr,maxTriSize,App::userSettings->triCountInOBB);
    }
}

bool CShape::getCulling() const
{
    if (getMeshWrapper()->isMesh())
        return(getSingleMesh()->getCulling());
    return(false);
}

void CShape::setCulling(bool culState)
{
    if (getMeshWrapper()->isMesh())
    {
        CMesh* m=getSingleMesh();
        if (m->getCulling()!=culState)
        {
            m->setCulling(culState);

            if ( _isInScene&&App::worldContainer->getEventsEnabled() )
            {
                const char* cmd="color";
                auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,true);
                CInterfaceStackTable* sdata=new CInterfaceStackTable();
                data->appendMapObject_stringObject(cmd,sdata);
                sdata->appendMapObject_stringBool("culling",culState);
                sdata->appendMapObject_stringInt32("index",0);
                App::worldContainer->pushEvent(event);
            }
        }
    }
}

bool CShape::getVisibleEdges() const
{
    if (getMeshWrapper()->isMesh())
        return(getSingleMesh()->getVisibleEdges());
    return(false);
}

void CShape::setVisibleEdges(bool v)
{
    if (getMeshWrapper()->isMesh())
    {
        CMesh* m=getSingleMesh();
        if (m->getVisibleEdges()!=v)
        {
            m->setVisibleEdges(v);

            if ( _isInScene&&App::worldContainer->getEventsEnabled() )
            {
                const char* cmd="color";
                auto [event,data]=App::worldContainer->prepareSceneObjectChangedEvent(this,false,cmd,false);
                CInterfaceStackTable* sdata=new CInterfaceStackTable();
                data->appendMapObject_stringObject(cmd,sdata);
                sdata->appendMapObject_stringBool("showEdges",v);
                sdata->appendMapObject_stringInt32("index",0);
                App::worldContainer->pushEvent(event);
            }
        }
    }
}

double CShape::getShadingAngle() const
{
    double retVal=0.0;
    if (getMeshWrapper()->isMesh())
        retVal=getSingleMesh()->getShadingAngle();
    return(retVal);
}

void CShape::setShadingAngle(double a)
{
    if (getMeshWrapper()->isMesh())
    {
        CMesh* m=getSingleMesh();
        if (fabs(m->getShadingAngle()-a)>0.001)
        {
            m->setShadingAngle(a);
            pushObjectRefreshEvent();
        }
    }
}

void CShape::setRespondableSuspendCount(int cnt)
{
    _respondableSuspendCount=cnt;
}

void CShape::decrementRespondableSuspendCount()
{
    if (_respondableSuspendCount>0)
        _respondableSuspendCount--;
}

bool CShape::getHideEdgeBorders_OLD() const
{
    return(getMeshWrapper()->getHideEdgeBorders_OLD());
}

void CShape::setHideEdgeBorders_OLD(bool v)
{
    getMeshWrapper()->setHideEdgeBorders_OLD(v);
}

bool CShape::getShapeWireframe_OLD() const
{
    if (getMeshWrapper()->isMesh())
        return(getSingleMesh()->getWireframe_OLD());
    return(false);
}

void CShape::setShapeWireframe_OLD(bool w)
{
    if (getMeshWrapper()->isMesh())
        getSingleMesh()->setWireframe_OLD(w);
}

bool CShape::doesShapeCollideWithShape(CShape* collidee,std::vector<double>* intersections)
{   // If intersections is different from nullptr, we check for all intersections and
    // intersection segments are appended to the vector

    std::vector<double> _intersect;
    std::vector<double>* _intersectP=nullptr;
    if (intersections!=nullptr)
        _intersectP=&_intersect;
    if ( CPluginContainer::geomPlugin_getMeshMeshCollision(_meshCalculationStructure,getFullCumulativeTransformation(),collidee->_meshCalculationStructure,collidee->getFullCumulativeTransformation(),_intersectP,nullptr,nullptr))
    { // There was a collision
        if (intersections!=nullptr)
            intersections->insert(intersections->end(),_intersect.begin(),_intersect.end());
        return(true);
    }
    return(false);
}

bool CShape::getDistanceToDummy_IfSmaller(CDummy* dummy,double &dist,double ray[7],int& buffer)
{   // Distance is measured from this to dummy
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // Build the node only when needed. So do it right here!
    initializeMeshCalculationStructureIfNeeded();

    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector rayPart0;
    if (CPluginContainer::geomPlugin_getMeshPointDistanceIfSmaller(_meshCalculationStructure,getFullCumulativeTransformation(),dummyPos,dist,&rayPart0,&buffer))
    {
        rayPart0.getData(ray+0);
        dummyPos.getData(ray+3);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

bool CShape::getShapeShapeDistance_IfSmaller(CShape* it,double &dist,double ray[7],int buffer[2])
{   // this is shape number 1, 'it' is shape number 2 (for ordering (measured from 1 to 2))
    // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false

    CShape* shapeA=this;
    CShape* shapeB=it;
    C7Vector shapeATr=shapeA->getFullCumulativeTransformation();
    C7Vector shapeBTr=shapeB->getFullCumulativeTransformation();
    shapeA->initializeMeshCalculationStructureIfNeeded();
    shapeB->initializeMeshCalculationStructureIfNeeded();
    C3Vector ptOnShapeA;
    C3Vector ptOnShapeB;

    bool smaller=false;
    if (CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeA->_meshCalculationStructure)<CPluginContainer::geomPlugin_getMeshRootObbVolume(shapeB->_meshCalculationStructure))
        smaller=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeA->_meshCalculationStructure,shapeATr,shapeB->_meshCalculationStructure,shapeBTr,dist,&ptOnShapeA,&ptOnShapeB,&buffer[0],&buffer[1]);
    else
        smaller=CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shapeB->_meshCalculationStructure,shapeBTr,shapeA->_meshCalculationStructure,shapeATr,dist,&ptOnShapeB,&ptOnShapeA,&buffer[1],&buffer[0]);

    if (smaller)
    {
        ray[0]=ptOnShapeA(0);
        ray[1]=ptOnShapeA(1);
        ray[2]=ptOnShapeA(2);
        ray[3]=ptOnShapeB(0);
        ray[4]=ptOnShapeB(1);
        ray[5]=ptOnShapeB(2);
        ray[6]=dist;
        return(true);
    }
    return(false);
}

void CShape::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
}

void CShape::addSpecializedObjectEventData(CInterfaceStackTable* data) const
{
    CInterfaceStackTable* subC=new CInterfaceStackTable();
    data->appendMapObject_stringObject("shape",subC);
    data=subC;

    CInterfaceStackTable* meshData=new CInterfaceStackTable();
    data->appendMapObject_stringObject("meshes",meshData);

    std::vector<CMesh*> all;
    getMeshWrapper()->getAllShapeComponentsCumulative(all);
    for (size_t i=0;i<all.size();i++)
    {
        CMesh* geom=all[i];

        CInterfaceStackTable* mesh=new CInterfaceStackTable();
        meshData->appendArrayObject(mesh);

        C7Vector tr(geom->getVerticeLocalFrame());
        const std::vector<float>* wvert=geom->getVerticesForDisplayAndDisk();
        const std::vector<int>* wind=geom->getIndices();
        const std::vector<float>* wnorm=geom->getNormalsForDisplayAndDisk();
        std::vector<float> vertices;
        vertices.resize(wvert->size());
        for (size_t j=0;j<wvert->size()/3;j++)
        {
            C3Vector v;
            v.setData(wvert->data()+j*3);
            v=tr*v;
            vertices[3*j+0]=(float)v(0);
            vertices[3*j+1]=(float)v(1);
            vertices[3*j+2]=(float)v(2);
        }
        CCbor obj(nullptr,0);
        size_t l;
        obj.appendFloatArray(vertices.data(),vertices.size());
        const char* buff=(const char*)obj.getBuff(l);
        mesh->appendMapObject_stringString("vertices",buff,l,true);

        obj.clear();
        obj.appendIntArray(wind->data(),wind->size());
        buff=(const char*)obj.getBuff(l);
        mesh->appendMapObject_stringString("indices",buff,l,true);

        std::vector<float> normals;
        normals.resize(wind->size()*3);
        for (size_t j=0;j<wind->size();j++)
        {
            C3Vector n;
            n.setData(&(wnorm[0])[0]+j*3);
            n=tr.Q*n; // only orientation
            normals[3*j+0]=(float)n(0);
            normals[3*j+1]=(float)n(1);
            normals[3*j+2]=(float)n(2);
        }
        obj.clear();
        obj.appendFloatArray(normals.data(),normals.size());
        buff=(const char*)obj.getBuff(l);
        mesh->appendMapObject_stringString("normals",buff,l,true);

        float c[9];
        geom->color.getColor(c+0,sim_colorcomponent_ambient_diffuse);
        geom->color.getColor(c+3,sim_colorcomponent_specular);
        geom->color.getColor(c+6,sim_colorcomponent_emission);
        mesh->appendMapObject_stringFloatArray("color",c,9);

//        mesh->appendMapObject_stringFloat("edgeAngle",geom->);
        mesh->appendMapObject_stringFloat("shadingAngle",geom->getShadingAngle());
        mesh->appendMapObject_stringBool("showEdges",geom->getVisibleEdges());
        mesh->appendMapObject_stringBool("culling",geom->getCulling());
        double transp=0.0;
        if (geom->color.getTranslucid())
            transp=1.0-geom->color.getOpacity();
        mesh->appendMapObject_stringFloat("transparency",transp);


        int options=0;
        if (geom->getCulling())
            options|=1;
        if (geom->getWireframe_OLD())
            options|=2;
        mesh->appendMapObject_stringInt32("options",options);

        CTextureProperty* tp=geom->getTextureProperty();
        CTextureObject* to=nullptr;
        const std::vector<float>* tc=nullptr;
        if (tp!=nullptr)
        {
            to=tp->getTextureObject();
            tc=tp->getTextureCoordinates(-1,tr,wvert[0],wind[0]);
        }

        if ( (to!=nullptr)&&(tc!=nullptr) )
        {
            if (true)
            { // sending raw texture
                int tRes[2];
                to->getTextureSize(tRes[0],tRes[1]);
                CInterfaceStackTable* texture=new CInterfaceStackTable();
                mesh->appendMapObject_stringObject("texture",texture);

                texture->appendMapObject_stringString("rawTexture",(char*)to->getTextureBufferPointer(),tRes[1]*tRes[0]*4);

                texture->appendMapObject_stringInt32Array("resolution",tRes,2);

                obj.clear();
                obj.appendFloatArray(tc->data(),tc->size());
                buff=(const char*)obj.getBuff(l);
                texture->appendMapObject_stringString("coordinates",buff,l,true);

                texture->appendMapObject_stringInt32("applyMode",tp->getApplyMode());

                int options=0;
                if (tp->getRepeatU())
                    options|=1;
                if (tp->getRepeatV())
                    options|=2;
                if (tp->getInterpolateColors())
                    options|=4;
                texture->appendMapObject_stringInt32("options",options);

                texture->appendMapObject_stringInt32("id",tp->getTextureObjectID());
            }
            else
            { // sending PNG texture
                std::string buffer;
                int tRes[2];
                to->getTextureSize(tRes[0],tRes[1]);
                bool res=CImageLoaderSaver::save((unsigned char*)to->getTextureBufferPointer(),tRes,1,".png",-1,&buffer);
                if (res)
                {
                    CInterfaceStackTable* texture=new CInterfaceStackTable();
                    mesh->appendMapObject_stringObject("texture",texture);

                    buffer=CTTUtil::encode64(buffer);
                    texture->appendMapObject_stringString("texture",buffer.c_str(),buffer.size());

                    texture->appendMapObject_stringInt32Array("resolution",tRes,2);

                    obj.clear();
                    obj.appendFloatArray(tc->data(),tc->size());
                    buff=(const char*)obj.getBuff(l);
                    texture->appendMapObject_stringString("coordinates",buff,l,true);

                    texture->appendMapObject_stringInt32("applyMode",tp->getApplyMode());

                    int options=0;
                    if (tp->getRepeatU())
                        options|=1;
                    if (tp->getRepeatV())
                        options|=2;
                    if (tp->getInterpolateColors())
                        options|=4;
                    texture->appendMapObject_stringInt32("options",options);

                    texture->appendMapObject_stringInt32("id",tp->getTextureObjectID());
                }
            }
        }
    }
}

CSceneObject* CShape::copyYourself()
{   
    CShape* newShape=(CShape*)CSceneObject::copyYourself();

    if (_mesh!=nullptr)
        newShape->_mesh=_mesh->copyYourself();

    newShape->_meshBoundingBoxHalfSizes=_meshBoundingBoxHalfSizes;

    if (_meshCalculationStructure!=nullptr)
        newShape->_meshCalculationStructure=CPluginContainer::geomPlugin_copyMesh(_meshCalculationStructure);

    delete newShape->_dynMaterial;
    newShape->_dynMaterial=_dynMaterial->copyYourself();

    // Various:
    newShape->_shapeIsDynamicallyKinematic=_shapeIsDynamicallyKinematic;
    newShape->_startInDynamicSleeping=_startInDynamicSleeping;
    newShape->_shapeIsDynamicallyStatic=_shapeIsDynamicallyStatic;
    newShape->_shapeIsDynamicallyRespondable=_shapeIsDynamicallyRespondable;
    newShape->_dynamicCollisionMask=_dynamicCollisionMask;
    newShape->_containsTransparentComponents=_containsTransparentComponents;
    newShape->_rigidBodyWasAlreadyPutToSleepOnce=_rigidBodyWasAlreadyPutToSleepOnce;
    newShape->_setAutomaticallyToNonStaticIfGetsParent=_setAutomaticallyToNonStaticIfGetsParent;
    newShape->_initialDynamicLinearVelocity=_initialDynamicLinearVelocity;
    newShape->_initialDynamicAngularVelocity=_initialDynamicAngularVelocity;


    newShape->_initialValuesInitialized=_initialValuesInitialized;

    return(newShape);
}

void CShape::setColor(const char* colorName,int colorComponent,float r,float g,float b)
{
    float rgb[3]={r,g,b};
    setColor(colorName,colorComponent,rgb);
}

void CShape::setColor(const char* colorName,int colorComponent,const float* rgbData)
{
    int rgbDataOffset=0;
    int cnt=0;
    const CShape* s=nullptr;
    if (_isInScene)
        s=this;
    getMeshWrapper()->setColor(s,cnt,colorName,colorComponent,rgbData,rgbDataOffset);
    if (colorComponent==sim_colorcomponent_transparency)
        actualizeContainsTransparentComponent();
}

bool CShape::getColor(const char* colorName,int colorComponent,float* rgbData)
{
    int rgbDataOffset=0;
    return(getMeshWrapper()->getColor(colorName,colorComponent,rgbData,rgbDataOffset));
}

int CShape::getComponentCount() const
{
    return(getMeshWrapper()->getComponentCount());
}

void CShape::display(CViewableBase* renderingObject,int displayAttrib)
{
    EASYLOCK(_objectMutex);
    displayShape(this,renderingObject,displayAttrib);
}

bool CShape::setParent(CSceneObject* newParent)
{ // Overridden from CSceneObject
    bool retVal=CSceneObject::setParent(newParent);
    if (retVal&&getSetAutomaticallyToNonStaticIfGetsParent())
        setShapeIsDynamicallyStatic(false);
    return(retVal);
}
