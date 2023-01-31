#pragma once

#include "colorObject.h"
#include "ser.h"
#include "3Vector.h"
#include "7Vector.h"
#include "dynMaterialObject.h"

class CViewableBase;
class CShape;
class CMesh;
class CSceneObject;

class CMeshWrapper
{
public:

    CMeshWrapper();
    virtual ~CMeshWrapper();

    virtual void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    virtual void performSceneObjectLoadingMapping(const std::map<int,int>* map);
    virtual void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    virtual void announceSceneObjectWillBeErased(const CSceneObject* object);
    virtual void setTextureDependencies(int shapeID);
    virtual bool getContainsTransparentComponents() const;
    virtual void displayGhost(CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors);
    virtual void display(CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
    virtual void display_extRenderer(CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex);
    virtual void display_colorCoded(CShape* geomData,int objectId,int displayAttrib);
    virtual CMeshWrapper* copyYourself();
    virtual void scale(double xVal,double yVal,double zVal);
    virtual void setPurePrimitiveType(int theType,double xOrDiameter,double y,double zOrHeight);
    virtual int getPurePrimitiveType() const;
    virtual bool isMesh() const;
    virtual bool isPure() const;
    virtual bool isConvex() const;
    virtual bool checkIfConvex();
    virtual void setConvex(bool convex);
    virtual bool containsOnlyPureConvexShapes();
    virtual void getCumulativeMeshes(std::vector<double>& vertices,std::vector<int>* indices,std::vector<double>* normals);
    virtual void setColor(const CShape* shape,int& elementIndex,const char* colorName,int colorComponent,const float* rgbData,int& rgbDataOffset);
    virtual bool getColor(const char* colorName,int colorComponent,float* rgbData,int& rgbDataOffset) const;
    virtual void getAllShapeComponentsCumulative(std::vector<CMesh*>& shapeComponentList);
    virtual CMesh* getShapeComponentAtIndex(int& index);
    virtual int getComponentCount() const;
    virtual void serialize(CSer& ar,const char* shapeName);
    virtual void preMultiplyAllVerticeLocalFrames(const C7Vector& preTr);
    virtual void flipFaces();
    virtual double getShadingAngle() const;
    virtual void setShadingAngle(double angle);
    virtual double getEdgeThresholdAngle() const;
    virtual void setEdgeThresholdAngle(double angle);
    virtual void setHideEdgeBorders_OLD(bool v);
    virtual bool getHideEdgeBorders_OLD() const;
    virtual int getTextureCount() const;
    virtual bool hasTextureThatUsesFixedTextureCoordinates() const;
    virtual void removeAllTextures();
    virtual void getColorStrings(std::string& colorStrings) const;

    void serializeWrapperInfos(CSer& ar,const char* shapeName);
    void scaleWrapperInfos(double xVal,double yVal,double zVal);
    void scaleMassAndInertia(double xVal,double yVal,double zVal);
    void copyWrapperInfos(CMeshWrapper* target);
    void setDefaultInertiaParams();
    void setMass(double m);
    double getMass() const;
    void setName(std::string newName);
    std::string getName() const;

    int getDynMaterialId_old() const;
    void setDynMaterialId_old(int id);
    // ---------------------


    C7Vector getLocalInertiaFrame() const;
    void setLocalInertiaFrame(const C7Vector& li);
    C3Vector getPrincipalMomentsOfInertia() const;
    void setPrincipalMomentsOfInertia(const C3Vector& inertia);

    C7Vector getTransformationsSinceGrouping() const;
    void setTransformationsSinceGrouping(const C7Vector& tr);

    static void findPrincipalMomentOfInertia(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments);
    static C3X3Matrix getNewTensor(const C3Vector& principalMoments,const C7Vector& newFrame);

    std::vector<CMeshWrapper*> childList;

protected:
    static double _getTensorNonDiagonalMeasure(const C3X3Matrix& tensor);

    std::string _name;
    bool _convex;
    C7Vector _transformationsSinceGrouping; // used to keep track of this geomWrap or geometric's configuration relative to the shape

    double _mass;
    C7Vector _localInertiaFrame; // frame relative to the shape.
    C3Vector _principalMomentsOfInertia; // remember that we always work with a massless tensor. The tensor is multiplied with the mass in the dynamics module!

    int _dynMaterialId_old;
};
