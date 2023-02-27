#pragma once

#include <colorObject.h>
#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <dynMaterialObject.h>

class CViewableBase;
class CShape;
class CMesh;
class CSceneObject;

class CMeshWrapper
{
public:

    CMeshWrapper();
    virtual ~CMeshWrapper();

    void addItem(CMeshWrapper* m);
    void detachItems();

    virtual void prepareVerticesIndicesNormalsAndEdgesForSerialization();
    virtual void performSceneObjectLoadingMapping(const std::map<int,int>* map);
    virtual void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    virtual void announceSceneObjectWillBeErased(const CSceneObject* object);
    virtual void setTextureDependencies(int shapeID);
    virtual bool getContainsTransparentComponents() const;
    virtual void displayGhost(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,bool originalColors,bool backfaceCulling,double transparency,const float* newColors);
    virtual void display(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,CColorObject* collisionColor,int dynObjFlag_forVisualization,int transparencyHandling,bool multishapeEditSelected);
    virtual void display_extRenderer(const C7Vector& cumulIFrameTr,CShape* geomData,int displayAttrib,const C7Vector& tr,int shapeHandle,int& componentIndex);
    virtual void display_colorCoded(const C7Vector& cumulIFrameTr,CShape* geomData,int objectId,int displayAttrib);
    virtual CMeshWrapper* copyYourself();
    virtual void scale(double isoVal);
    virtual void setPurePrimitiveType(int theType,double xOrDiameter,double y,double zOrHeight);
    virtual int getPurePrimitiveType() const;
    virtual bool isMesh() const;
    virtual bool isPure() const;
    virtual bool isConvex() const;
    virtual bool checkIfConvex();
    virtual void setConvex(bool convex);
    virtual bool containsOnlyPureConvexShapes();
    virtual void takeVisualAttributesFrom(CMesh* origin);
    virtual void getCumulativeMeshes(const C7Vector& parentCumulTr,std::vector<double>& vertices,std::vector<int>* indices,std::vector<double>* normals);
    virtual void getCumulativeMeshes(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,std::vector<double>& vertices,std::vector<int>* indices,std::vector<double>* normals);
    virtual void setColor(const CShape* shape,int& elementIndex,const char* colorName,int colorComponent,const float* rgbData,int& rgbDataOffset);
    virtual bool getColor(const char* colorName,int colorComponent,float* rgbData,int& rgbDataOffset) const;
    virtual void getAllShapeComponentsCumulative(const C7Vector& parentCumulTr,std::vector<CMesh*>& shapeComponentList,std::vector<C7Vector>* OptParentCumulTrList=nullptr);
    virtual CMesh* getShapeComponentAtIndex(const C7Vector& parentCumulTr,int& index,C7Vector* optParentCumulTrOut=nullptr);
    virtual int getComponentCount() const;
    virtual bool serialize(CSer& ar,const char* shapeName,const C7Vector& parentCumulIFrame,bool rootLevel);
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

    void copyAttributesTo(CMeshWrapper* target);
    void copyWrapperData(CMeshWrapper* target);
    void setDefaultInertiaParams();
    void scaleMassAndInertia(double s);
    void setMass(double m);
    double getMass() const;
    void setName(std::string newName);
    std::string getName() const;

    int getDynMaterialId_old() const;
    void setDynMaterialId_old(int id);
    // ---------------------

    C7Vector getDiagonalInertiaInfo(C3Vector& diagMasslessI) const;
    C7Vector getBB(C3Vector* optBBSize) const;
    bool getShapeRelIFrame(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,C7Vector& shapeRelIFrame) const;
    bool getShapeRelBB(const C7Vector& parentCumulTr,const CMeshWrapper* wrapper,C7Vector& shapeRelBB,C3Vector* optBBSize) const;
    C3Vector getCOM() const;
    C7Vector getIFrame() const;
    void setIFrame(const C7Vector& iframe);
    void setCOM(const C3Vector& com);
    C3X3Matrix getMasslessInertiaMatrix() const;
    void setMasslessInertiaMatrix(const C3X3Matrix& im,int modifItemRow=-1,int modifItemCol=-1);
    C3Vector getPMI() const;
    void setPMI(const C3Vector& pmi);
    std::string getInertiaMatrixErrorString() const;
    void fixInertiaMatrixAndComputePMI();

    C7Vector getLocalInertiaFrame() const;
    void setLocalInertiaFrame(const C7Vector& li);
    C3Vector getPrincipalMomentsOfInertia() const;
    void setPrincipalMomentsOfInertia(const C3Vector& inertia);

    C7Vector getTransformationsSinceGrouping() const;
    void setTransformationsSinceGrouping(const C7Vector& tr);

    static bool getPMIFromMasslessTensor(const C3X3Matrix& tensor,C4Vector& rotation,C3Vector& principalMoments);
    static C3X3Matrix getMasslessTensorFromPMI(const C3Vector& principalMoments,const C7Vector& newFrame);
    static C3X3Matrix getMasslesInertiaMatrixInNewFrame(const C4Vector& oldFrame,const C3X3Matrix& oldMatrix,const C4Vector& newFrame);
    static std::string getInertiaMatrixErrorString(const C3X3Matrix& matrix);

    std::vector<CMeshWrapper*> childList;

protected:
    void _commonInit();
    void _computeInertiaFromChildren();
    std::string _name;
    bool _convex;

    double _mass;
    // --------------------
    C7Vector _transformationsSinceGrouping; // used to keep track of this geomWrap or geometric's configuration relative to the shape
    C7Vector _localInertiaFrame; // frame relative to the shape.
    C3Vector _principalMomentsOfInertia; // remember that we always work with a massless tensor. The tensor is multiplied with the mass in the dynamics module!
    // --------------------
    C7Vector _iFrame; // Inertia ref. frame, relative to parent _iFrame. Identity if root
    C3Vector _com; // Center of mass, relative to _iFrame
    C3X3Matrix _iMatrix; // Mass-less inertia matrix, expressed in the _iFrame
    C3Vector _pmi; // Principal moment of inertia (calculated from _iMatrix), expressed in the _iFrame
    C4Vector _pmiRotFrame; // Frame of the principal moment of inertia (calculated from _iMatrix), expressed in the _iFrame
    C7Vector _bbFrame; // Ref. frame of the bounding box and vertices, relative to _iFrame
    C3Vector _bbSize; // Size of the bounding box, relative to _iFrame
    // --------------------

    int _dynMaterialId_old;
};
