
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "ikEl.h"
#include "ikRoutine.h"
#include "app.h"
#include "tt.h"

CikEl::CikEl()
{
    tooltip=-1;
    commonInit();
}

CikEl::CikEl(int theTooltip)
{
    tooltip=theTooltip;
    commonInit();
}

CikEl::~CikEl()
{
}

void CikEl::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    //_initialValuesInitialized=simulationIsRunning;
    //if (simulationIsRunning)
    //{
    //}
}

void CikEl::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    //if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    //{
    //}
    //_initialValuesInitialized=false;
}

void CikEl::performCollectionLoadingMapping(std::vector<int>* map)
{
}

bool CikEl::announceCollectionWillBeErased(int groupID)
{ // Return value true means that this avoidance object should be destroyed
    return(false);
}

void CikEl::setObjectID(int newID)
{
    objectID=newID;
}

void CikEl::setTooltip(int newTooltip)
{
    tooltip=newTooltip;
}

void CikEl::setAlternativeBaseForConstraints(int b)
{ 
    alternativeBaseForConstraints=b;
}

CikEl* CikEl::copyYourself()
{
    CikEl* newEl=new CikEl();
    newEl->alternativeBaseForConstraints=alternativeBaseForConstraints;
    newEl->objectID=objectID;
    newEl->tooltip=tooltip;
    newEl->base=base;
    newEl->minAngularPrecision=minAngularPrecision;
    newEl->minLinearPrecision=minLinearPrecision;
    newEl->active=active;
    newEl->constraints=constraints;
    newEl->positionWeight=positionWeight;
    newEl->orientationWeight=orientationWeight;
    return(newEl);
}

void CikEl::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Oid");
            ar << objectID;
            ar.flush();

            ar.storeDataName("Ik2");
            ar << tooltip << base;
            ar.flush();

            ar.storeDataName("Abc");
            ar << alternativeBaseForConstraints;
            ar.flush();

            ar.storeDataName("Pr2");
            ar << minAngularPrecision << minLinearPrecision;
            ar.flush();

            ar.storeDataName("Ctr");
            ar << constraints;
            ar.flush();

            ar.storeDataName("Wgt");
            ar << positionWeight << orientationWeight;
            ar.flush();

            ar.storeDataName("Var");
            unsigned char nothing=0;
            nothing=nothing+1*active;
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
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> objectID;
                    }
                    if (theName.compare("Ik2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> tooltip >> base;
                    }
                    if (theName.compare("Abc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> alternativeBaseForConstraints;
                    }
                    if (theName.compare("Prc")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> minAngularPrecision >> minLinearPrecision;
                        minAngularPrecision*=degToRad_f;
                    }
                    if (theName.compare("Pr2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> minAngularPrecision >> minLinearPrecision;
                    }
                    if (theName.compare("Ctr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> constraints;
                    }
                    if (theName.compare("Wgt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> positionWeight >> orientationWeight;
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        active=((nothing&1)!=0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CikEl::serializeWExtIk(CExtIkSer& ar)
{
    ar.writeInt(objectID);

    ar.writeInt(tooltip);
    ar.writeInt(base);

    ar.writeInt(alternativeBaseForConstraints);

    ar.writeFloat(minAngularPrecision);
    ar.writeFloat(minLinearPrecision);

    ar.writeInt(constraints);

    ar.writeFloat(positionWeight);
    ar.writeFloat(orientationWeight);

    unsigned char nothing=0;
    nothing=nothing+1*active;
    ar.writeByte(nothing);
}

void CikEl::setMinAngularPrecision(float prec)
{
    tt::limitValue(0.001f*degToRad_f,180.0f*degToRad_f,prec);
    minAngularPrecision=prec;
}
void CikEl::setMinLinearPrecision(float prec)
{
    tt::limitValue(0.00001f,1.0f,prec);
    minLinearPrecision=prec;
}

void CikEl::performObjectLoadingMapping(std::vector<int>* map)
{
    tooltip=App::ct->objCont->getLoadingMapping(map,tooltip);
    base=App::ct->objCont->getLoadingMapping(map,base);
    alternativeBaseForConstraints=App::ct->objCont->getLoadingMapping(map,alternativeBaseForConstraints);
}

void CikEl::commonInit()
{
    base=-1;
    alternativeBaseForConstraints=-1; // -1 means use the regular base for constraints!

    minAngularPrecision=0.1f*degToRad_f;
    minLinearPrecision=0.0005f;
    active=true;
    constraints=(sim_ik_x_constraint|sim_ik_y_constraint|sim_ik_z_constraint);
    positionWeight=1.0f;
    orientationWeight=1.0f;
    matrix=nullptr;
    matrix_correctJacobian=nullptr; // same as matrix, but the orientation parts are multiplied by IK_DIVISION_FACTOR
    errorVector=nullptr;
    rowJointIDs=nullptr;
    rowJointStages=nullptr;
}

void CikEl::setAllInvolvedJointsToPassiveMode()
{
    C3DObject* iterat=App::ct->objCont->getDummy(tooltip);
    C3DObject* baseObj=App::ct->objCont->getObjectFromHandle(base);
    while ((iterat!=baseObj)&&(iterat!=nullptr))
    {
        iterat=iterat->getParentObject();
        if ( (iterat!=nullptr)&&(iterat!=baseObj) )
        {
            if (iterat->getObjectType()==sim_object_joint_type)
                ((CJoint*)iterat)->setJointMode(sim_jointmode_passive,false);
        }
    }
}

void CikEl::setBase(int newBase)
{
    base=newBase;
}

void CikEl::setActive(bool isActive)
{
    active=isActive;
}

bool CikEl::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this IK el object should be destroyed

    if ((base==objID)||(alternativeBaseForConstraints==objID)||(tooltip==objID))
        return(true); // This element has to be erased!
    /* Following commented out and replaced with above code on 9/3/2014
    if (base==objID)
    {
        base=-1;
    }
    if (alternativeBaseForConstraints==objID)
    {
        alternativeBaseForConstraints=-1;
    }
    if (tooltip==objID)
        return(true); // This element has to be erased!
    */
    return(false);

}

int CikEl::getObjectID()
{
    return(objectID);
}

int CikEl::getTooltip()
{
    return(tooltip);
}

int CikEl::getBase()
{
    return(base);
}

int CikEl::getAlternativeBaseForConstraints()
{ 
    return(alternativeBaseForConstraints);
}

int CikEl::getTarget()
{ 
    CDummy* tip=App::ct->objCont->getDummy(tooltip);
    if (tip==nullptr)
        return(-1);
    int linkedDummyID=tip->getLinkedDummyID();
    if (linkedDummyID==-1)
        return(-1);
    if (tip->getLinkType()!=sim_dummy_linktype_ik_tip_target)
        return(-1);
    return(linkedDummyID); // this should be the target!
}

float CikEl::getMinAngularPrecision()
{
    return(minAngularPrecision);
}

float CikEl::getMinLinearPrecision()
{
    return(minLinearPrecision);
}

bool CikEl::getActive()
{
    return(active);
}
void CikEl::setConstraints(int constr)
{
    constraints=constr;
}
int CikEl::getConstraints()
{
    return(constraints); 
}
void CikEl::setPositionWeight(float weight)
{
    tt::limitValue(0.001f,1.0f,weight);
    positionWeight=weight;
}
float CikEl::getPositionWeight()
{
    return(positionWeight);
}
void CikEl::setOrientationWeight(float weight)
{
    tt::limitValue(0.001f,1.0f,weight);
    orientationWeight=weight;
}
float CikEl::getOrientationWeight()
{
    return(orientationWeight);
}

void CikEl::checkIfWithinTolerance(bool& position,bool& orientation,bool useTempValues)
{
    position=true;
    orientation=true;
    CDummy* targetObj=App::ct->objCont->getDummy(getTarget());
    if (targetObj==nullptr)
        return; // The tooltip is not constrained!
    CDummy* tooltipObj=App::ct->objCont->getDummy(tooltip);
    C7Vector targetM(targetObj->getCumulativeTransformationPart1(useTempValues));
    C7Vector tooltipM(tooltipObj->getCumulativeTransformationPart1(useTempValues));

    // Since everything is relative to the base
    C7Vector baseM;
    baseM.setIdentity();
    CDummy* baseObj=App::ct->objCont->getDummy(base);
    if (baseObj!=nullptr)
        baseM=baseObj->getCumulativeTransformationPart1(useTempValues).getInverse();

    baseObj=App::ct->objCont->getDummy(alternativeBaseForConstraints);
    if (baseObj!=nullptr)
        baseM=baseObj->getCumulativeTransformationPart1(useTempValues).getInverse();

    targetM=baseM*targetM;
    tooltipM=baseM*tooltipM;

    float err[2];
    getError(targetM.getMatrix(),tooltipM.getMatrix(),err,(constraints&sim_ik_x_constraint)!=0,
        (constraints&sim_ik_y_constraint)!=0,(constraints&sim_ik_z_constraint)!=0,
        (constraints&sim_ik_alpha_beta_constraint)!=0,(constraints&sim_ik_gamma_constraint)!=0);
    if (constraints&(sim_ik_x_constraint|sim_ik_y_constraint|sim_ik_z_constraint))
    {
        if (minLinearPrecision<err[0])
            position=false;
    }
    if (constraints&(sim_ik_alpha_beta_constraint|sim_ik_gamma_constraint))
    {
        if (minAngularPrecision<err[1])
            orientation=false;
    }
}

void CikEl::prepareIkEquations(float interpolFact)
{   // Before calling this function, make sure that joint's temp. param. are initialized!
    // Make also sure the tooltip is built on a joint before 'base' and that base
    // is parent of 'tooltip'.
    // interpolFact is the interpolation factor we use to compute the target pose:
    // interpolPose=tooltipPose*(1-interpolFact)+targetPose*interpolFact

    // We first take care of dummies linked to path objects in a "sliding" manner (not fixed but assigned to the path):
    // Case 1. Target is the free sliding dummy:
    CDummy* dummyObj=App::ct->objCont->getDummy(getTarget());
    CDummy* tipObj=App::ct->objCont->getDummy(tooltip);
    if ( (dummyObj!=nullptr)&&dummyObj->getAssignedToParentPath()&&dummyObj->getFreeOnPathTrajectory()&&(tipObj!=nullptr) )
    {
        C3DObject* parent=dummyObj->getParentObject();
        if ( (parent!=nullptr)&&(parent->getObjectType()==sim_object_path_type) )
        { // We bring the target as close as possible onto the tip:
            CPathCont* pc=((CPath*)parent)->pathContainer;
            C7Vector pathCTMI(parent->getCumulativeTransformationPart1(true).getInverse());
            C3Vector tipPos(pathCTMI*tipObj->getCumulativeTransformationPart1(true).X);
            C7Vector newDummyLocal;
            if (pc->getConfigurationOnBezierCurveClosestTo(tipPos,newDummyLocal))
                dummyObj->setTempLocalTransformation(newDummyLocal);
        }
    }
    // Case 2. Tip is the free sliding dummy:
    if ( (tipObj!=nullptr)&&tipObj->getAssignedToParentPath()&&tipObj->getFreeOnPathTrajectory()&&(dummyObj!=nullptr) )
    {
        C3DObject* parent=tipObj->getParentObject();
        if ( (parent!=nullptr)&&(parent->getObjectType()==sim_object_path_type) )
        { // We bring the tip as close as possible onto the target:
            CPathCont* pc=((CPath*)parent)->pathContainer;
            C7Vector pathCTMI(parent->getCumulativeTransformationPart1(true).getInverse());
            C3Vector targetPos(pathCTMI*dummyObj->getCumulativeTransformationPart1(true).X);
            C7Vector newTipLocal;
            if (pc->getConfigurationOnBezierCurveClosestTo(targetPos,newTipLocal))
                tipObj->setTempLocalTransformation(newTipLocal);
        }
    }

    // We get the jacobian and the rowJointIDs:
    rowJointIDs=new std::vector<int>;
    rowJointStages=new std::vector<int>;
    C4X4Matrix oldMatr;
    CMatrix* Ja=CIkRoutine::getJacobian(this,oldMatr,rowJointIDs,rowJointStages);

    // oldMatr now contains the cumulative transf. matr. of tooltip relative to base
    C4X4Matrix oldMatrInv(oldMatr.getInverse());
    int doF=Ja->cols;
    int equationNumber=0;

    C4X4Matrix dummyCumul;
    C4X4Matrix m;
    if (dummyObj!=nullptr)
    {
        C3DObject* baseObj=App::ct->objCont->getObjectFromHandle(base);
        C4X4Matrix baseCumul;
        baseCumul.setIdentity();
        if (baseObj!=nullptr)
            baseCumul=baseObj->getCumulativeTransformation(true).getMatrix();

        baseObj=App::ct->objCont->getObjectFromHandle(alternativeBaseForConstraints);
        if (baseObj!=nullptr)
            baseCumul=baseObj->getCumulativeTransformation(true).getMatrix();

        baseCumul.inverse();

        dummyCumul=dummyObj->getCumulativeTransformationPart1(true).getMatrix();
        dummyCumul=baseCumul*dummyCumul; // target is relative to the base (or the alternative base)!
        C7Vector tr;
        tr.buildInterpolation(oldMatr.getTransformation(),dummyCumul.getTransformation(),interpolFact);
        m=tr;

        // We prepare matrix and errorVector and their respective sizes:
        if (constraints&sim_ik_x_constraint)
            equationNumber++;
        if (constraints&sim_ik_y_constraint)
            equationNumber++;
        if (constraints&sim_ik_z_constraint)
            equationNumber++;
        if (constraints&sim_ik_alpha_beta_constraint)
            equationNumber+=2;
        if (constraints&sim_ik_gamma_constraint)
            equationNumber++;
    }

    matrix=new CMatrix(equationNumber,doF);
    matrix_correctJacobian=new CMatrix(equationNumber,doF);
    errorVector=new CMatrix(equationNumber,1);
    if (dummyObj!=nullptr)
    {
        // We set up the position/orientation errorVector and the matrix:
        int pos=0;
        if (constraints&sim_ik_x_constraint)
        {
            for (int i=0;i<doF;i++)
            {
                (*matrix)(pos,i)=(*Ja)(0,i);
                (*matrix_correctJacobian)(pos,i)=(*Ja)(0,i);
            }
            (*errorVector)(pos,0)=(m.X(0)-oldMatr.X(0))*positionWeight;
            pos++;
        }
        if (constraints&sim_ik_y_constraint)
        {
            for (int i=0;i<doF;i++)
            {
                (*matrix)(pos,i)=(*Ja)(1,i);
                (*matrix_correctJacobian)(pos,i)=(*Ja)(1,i);
            }
            (*errorVector)(pos,0)=(m.X(1)-oldMatr.X(1))*positionWeight;
            pos++;
        }
        if (constraints&sim_ik_z_constraint)
        {
            for (int i=0;i<doF;i++)
            {
                (*matrix)(pos,i)=(*Ja)(2,i);
                (*matrix_correctJacobian)(pos,i)=(*Ja)(2,i);
            }
            (*errorVector)(pos,0)=(m.X(2)-oldMatr.X(2))*positionWeight;
            pos++;
        }
        if ( (constraints&sim_ik_alpha_beta_constraint)&&(constraints&sim_ik_gamma_constraint) )
        {
            for (int i=0;i<doF;i++)
            {
                (*matrix)(pos,i)=(*Ja)(3,i);
                (*matrix)(pos+1,i)=(*Ja)(4,i);
                (*matrix)(pos+2,i)=(*Ja)(5,i);
                (*matrix_correctJacobian)(pos,i)=(*Ja)(3,i)*IK_DIVISION_FACTOR;
                (*matrix_correctJacobian)(pos+1,i)=(*Ja)(4,i)*IK_DIVISION_FACTOR;
                (*matrix_correctJacobian)(pos+2,i)=(*Ja)(5,i)*IK_DIVISION_FACTOR;
            }
            C4X4Matrix diff(oldMatrInv*m);
            C3Vector euler(diff.M.getEulerAngles());
            (*errorVector)(pos,0)=euler(0)*orientationWeight/IK_DIVISION_FACTOR;
            (*errorVector)(pos+1,0)=euler(1)*orientationWeight/IK_DIVISION_FACTOR;
            (*errorVector)(pos+2,0)=euler(2)*orientationWeight/IK_DIVISION_FACTOR;
            pos=pos+3;
        }
        else
        {
            if (constraints&sim_ik_alpha_beta_constraint)
            {
                for (int i=0;i<doF;i++)
                {
                    (*matrix)(pos,i)=(*Ja)(3,i);
                    (*matrix)(pos+1,i)=(*Ja)(4,i);
                    (*matrix_correctJacobian)(pos,i)=(*Ja)(3,i)*IK_DIVISION_FACTOR;
                    (*matrix_correctJacobian)(pos+1,i)=(*Ja)(4,i)*IK_DIVISION_FACTOR;
                }
                C4X4Matrix diff(oldMatrInv*m);
                C3Vector euler(diff.M.getEulerAngles());
                (*errorVector)(pos,0)=euler(0)*orientationWeight/IK_DIVISION_FACTOR;
                (*errorVector)(pos+1,0)=euler(1)*orientationWeight/IK_DIVISION_FACTOR;
                pos=pos+2;
            }
            if (constraints&sim_ik_gamma_constraint)
            { // sim_gamma_constraint can't exist without sim_alpha_beta_constraint!
                for (int i=0;i<doF;i++)
                {
                    (*matrix)(pos,i)=(*Ja)(5,i);
                    (*matrix_correctJacobian)(pos,i)=(*Ja)(5,i)*IK_DIVISION_FACTOR;
                }
                C4X4Matrix diff(oldMatrInv*m);
                C3Vector euler(diff.M.getEulerAngles());
                (*errorVector)(pos,0)=euler(2)*orientationWeight/IK_DIVISION_FACTOR;
                pos++;
            }
        }
    }
    delete Ja; // We delete the jacobian!
}

void CikEl::removeIkEquations()
{
    if (matrix==nullptr)
        return;
    delete matrix;
    delete matrix_correctJacobian;
    matrix=nullptr;
    matrix_correctJacobian=nullptr;
    delete errorVector;
    errorVector=nullptr;
    rowJointIDs->clear();
    delete rowJointIDs;
    rowJointIDs=nullptr;

    rowJointStages->clear();
    delete rowJointStages;
    rowJointStages=nullptr;
}

void CikEl::getError(const C4X4Matrix& m1,const C4X4Matrix& m2,float err[2],bool xC,bool yC,bool zC,bool abC,bool gC)
{   // err[0] is position error, err[1] is orientation error
    // xC, yC and zC represents the position component we care about
    // Similar with abC and gB for the orientation: if abC and gC are false, orientation error is 0
    // All in rad or meter!!!
    C3Vector p(m1.X-m2.X);

    float xConstr=0.0f;
    if (xC) 
        xConstr=1.0f;
    float yConstr=0.0f;
    if (yC) 
        yConstr=1.0f;
    float zConstr=0.0f;
    if (zC) 
        zConstr=1.0f;
    err[0]=sqrtf(p(0)*p(0)*xConstr+p(1)*p(1)*yConstr+p(2)*p(2)*zConstr);

    float x=m1.M.axis[0]*m2.M.axis[0];
    float z=m1.M.axis[2]*m2.M.axis[2];
    x=tt::getLimitedFloat(-1.0f,1.0f,x);
    z=tt::getLimitedFloat(-1.0f,1.0f,z);
    if (gC)
    { // Full orientation constraints
        err[1]=fabs(CMath::robustAcos(x));
        float v=fabs(CMath::robustAcos(z));
        if (v>err[1])
            err[1]=v;
    }
    else if (abC)
    { // Orientation constraints, but free around z axis
        err[1]=fabs(CMath::robustAcos(z));
    }
    else
        err[1]=0.0f; // No orientation constraints!
}
