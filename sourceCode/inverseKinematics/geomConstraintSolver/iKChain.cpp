
#include "vrepMainHeader.h" 
#include "iKChain.h"
#include "iKJoint.h"
#include "gCSDefs.h"

CIKChain::CIKChain(CIKDummy* tip,float interpolFact,int jointNbToExclude)
{
    tooltip=tip;
    chainIsValid=true;
    // interpolFact is the interpolation factor we use to compute the target pose:
    // interpolPose=tooltipPose*(1-interpolFact)+targetPose*interpolFact
    // We get the jacobian and the rowJointIDs:
    C4X4Matrix oldMatr;
    int theConstraints=tip->constraints;
    tip->baseReorient.setIdentity();
    CMatrix* Ja=nullptr;

    if (tip->loopClosureDummy)
    {
        // The purpose of the following code is to have tip and target dummies reoriented
        // and the appropriate constraints calculated automatically (only for loop closure dummies!)
        tip->constraints=0;
        int posDimensions=0;
        int orDimensions=0;
        C3Vector firstJointZAxis(0.0f,0.0f,1.0f);
        C4X4Matrix firstJointCumul;
        // here we reorient tip and targetCumulativeMatrix
        // 1. We find the first revolute joint:
        CIKObject* iterat=tip->parent;
        while (iterat!=nullptr)
        {
            if ( (iterat->objectType==IK_JOINT_TYPE)&&((CIKJoint*)iterat)->revolute&&((CIKJoint*)iterat)->active )
                break;
            iterat=iterat->parent;
        }
        if (iterat!=nullptr)
        { // We have the first revolute joint from tip
            orDimensions=1;
            CIKObject* firstJoint=iterat;
            firstJointCumul=C4X4Matrix(firstJoint->getCumulativeTransformationPart1(true).getMatrix());
            firstJointZAxis=firstJointCumul.M.axis[2];
            C3Vector normalVect;
            // We search for a second revolute joint which is not aligned with the first one:
            iterat=iterat->parent;
            while (iterat!=nullptr)
            {
                if ( (iterat->objectType==IK_JOINT_TYPE)&&((CIKJoint*)iterat)->revolute&&((CIKJoint*)iterat)->active )
                {
                    C4X4Matrix secondJointCumul(iterat->getCumulativeTransformationPart1(true).getMatrix());
                    C3Vector secondJointZAxis(secondJointCumul.M.axis[2]);
                    if (fabs(firstJointZAxis*secondJointZAxis)<0.999999f) // Approx. 0.08 degrees
                    {
                        normalVect=(firstJointZAxis^secondJointZAxis).getNormalized();
                        if (firstJointZAxis*secondJointZAxis<0.0f)
                            secondJointZAxis=secondJointZAxis*-1.0f;
                        firstJointZAxis=((firstJointZAxis+secondJointZAxis)/2.0f).getNormalized();
                        break;
                    }
                }
                iterat=iterat->parent;
            }
            if (iterat!=nullptr)
            {
                orDimensions=2;
                // We search for a third revolute joint which is not orthogonal with normalVect:
                iterat=iterat->parent;
                while (iterat!=nullptr)
                {
                    if ( (iterat->objectType==IK_JOINT_TYPE)&&((CIKJoint*)iterat)->revolute&&((CIKJoint*)iterat)->active )
                    {
                        C4X4Matrix thirdJointCumul(iterat->getCumulativeTransformationPart1(true).getMatrix());
                        C3Vector thirdJointZAxis(thirdJointCumul.M.axis[2]);
                        if (fabs(normalVect*thirdJointZAxis)>0.0001f) // Approx. 0.005 degrees
                        {
                            orDimensions=3;
                            break;
                        }
                    }
                    iterat=iterat->parent;
                }
            }
        }           
        if ( (orDimensions==1)||(orDimensions==2) )
        {
            // We align the tip dummy's z axis with the joint axis 
            // (and do the same transformation to targetCumulativeMatrix)

            C4Vector rot(C4X4Matrix(firstJointZAxis).M.getQuaternion());
            C4Vector tipParentInverse(tip->getParentCumulativeTransformation(true).Q.getInverse());
            C4Vector tipNewLocal(tipParentInverse*rot);
            C4Vector postTr(tip->getLocalTransformation(true).Q.getInverse()*tipNewLocal);
            tip->transformation.Q=tipNewLocal;
            C7Vector postTr2;
            postTr2.setIdentity();
            postTr2.Q=postTr;
            tip->targetCumulativeTransformation=tip->targetCumulativeTransformation*postTr2;
        }
        Ja=getJacobian(tip,oldMatr,rowJoints,jointNbToExclude);
        C3Vector posV;
        for (int i=0;i<Ja->cols;i++)
        {
            // 1. Position space:
            C3Vector vc((*Ja)(0,i),(*Ja)(1,i),(*Ja)(2,i));
            float l=vc.getLength();
            if (l>0.0001f) // 0.1 mm
            {
                vc=vc/l;
                if (posDimensions==0)
                {
                    posV=vc;
                    posDimensions++;
                }
                else if (posDimensions==1)
                {
                    if (fabs(posV*vc)<0.999999f) // Approx. 0.08 degrees
                    {
                        posV=(posV^vc).getNormalized();
                        posDimensions++;
                    }
                }
                else if (posDimensions==2)
                {
                    if (fabs(posV*vc)>0.0001f) // Approx. 0.005 degrees
                        posDimensions++;
                }
            }
        }
        if (posDimensions!=3)
        {
            if (posDimensions!=0)
            {
                C4X4Matrix aligned(posV);
                tip->baseReorient=aligned.getInverse().getTransformation();
                for (int i=0;i<Ja->cols;i++)
                {
                    posV(0)=(*Ja)(0,i);
                    posV(1)=(*Ja)(1,i);
                    posV(2)=(*Ja)(2,i);
                    posV=tip->baseReorient.Q*posV; // baseReorient.X is 0 anyway...
                    (*Ja)(0,i)=posV(0);
                    (*Ja)(1,i)=posV(1);
                    (*Ja)(2,i)=posV(2);
                }
                oldMatr=tip->baseReorient.getMatrix()*oldMatr;
            
                if (posDimensions==1)
                    tip->constraints+=IK_Z_CONSTRAINT;
                else
                    tip->constraints+=(IK_X_CONSTRAINT+IK_Y_CONSTRAINT);
            }
        }
        else
            tip->constraints+=(IK_X_CONSTRAINT+IK_Y_CONSTRAINT+IK_Z_CONSTRAINT);

        int doF=Ja->cols;
        if (orDimensions==1)
        {
            if (doF-posDimensions>=1)
                tip->constraints+=IK_GAMMA_CONSTRAINT;
        }
        else if (orDimensions==2)
        {
            if (doF-posDimensions>=1) // Is this correct?
                tip->constraints+=IK_GAMMA_CONSTRAINT;
        }
        else if (orDimensions==3)
        {
            if (doF-posDimensions>=3)
                tip->constraints+=(IK_ALPHA_BETA_CONSTRAINT|IK_GAMMA_CONSTRAINT);
        }
        
        theConstraints=tip->constraints;
    }
    else
        Ja=getJacobian(tip,oldMatr,rowJoints,jointNbToExclude);

    if (Ja==nullptr)
    { // Error or not enough joints (effJoint=joints-jointNbToExclude) to get a Jacobian
        delete Ja;
        // We create dummy objects (so that we don't get an error when destroyed)
        matrix=new CMatrix(0,0);
        errorVector=new CMatrix(0,1);
        chainIsValid=false;
        return;
    }
    // oldMatr now contains the cumulative transf. matr. of tooltip relative to base
    C4X4Matrix oldMatrInv(oldMatr.getInverse());
    int doF=Ja->cols;
    int equationNumber=0;

    C4X4Matrix m;
    C4X4Matrix targetCumulativeMatrix((tip->baseReorient*tip->targetCumulativeTransformation).getMatrix());
    m.buildInterpolation(oldMatr,targetCumulativeMatrix,interpolFact);
    // We prepare matrix and errorVector and their respective sizes:
    if (theConstraints&IK_X_CONSTRAINT)
        equationNumber++;
    if (theConstraints&IK_Y_CONSTRAINT)
        equationNumber++;
    if (theConstraints&IK_Z_CONSTRAINT)
        equationNumber++;
    if (theConstraints&IK_ALPHA_BETA_CONSTRAINT)
        equationNumber=equationNumber+2;
    if (theConstraints&IK_GAMMA_CONSTRAINT)
        equationNumber++;
    matrix=new CMatrix(equationNumber,doF);
    errorVector=new CMatrix(equationNumber,1);
    // We set up the position/orientation errorVector and the matrix:
    
    float positionWeight=1.0f;
    float orientationWeight=1.0f;
    int pos=0;
    if (theConstraints&IK_X_CONSTRAINT)
    {
        for (int i=0;i<doF;i++)
            (*matrix)(pos,i)=(*Ja)(0,i);
        (*errorVector)(pos,0)=(m.X(0)-oldMatr.X(0))*positionWeight;
        pos++;
    }
    if (theConstraints&IK_Y_CONSTRAINT)
    {
        for (int i=0;i<doF;i++)
            (*matrix)(pos,i)=(*Ja)(1,i);
        (*errorVector)(pos,0)=(m.X(1)-oldMatr.X(1))*positionWeight;
        pos++;
    }
    if (theConstraints&IK_Z_CONSTRAINT)
    {
        for (int i=0;i<doF;i++)
            (*matrix)(pos,i)=(*Ja)(2,i);
        (*errorVector)(pos,0)=(m.X(2)-oldMatr.X(2))*positionWeight;
        pos++;
    }

    if (theConstraints&IK_ALPHA_BETA_CONSTRAINT)
    {
        for (int i=0;i<doF;i++)
        {
            (*matrix)(pos,i)=(*Ja)(3,i);
            (*matrix)(pos+1,i)=(*Ja)(4,i);
        }
        C4X4Matrix diff(oldMatrInv*m);
        C3Vector euler(diff.M.getEulerAngles());
        (*errorVector)(pos,0)=euler(0)*orientationWeight/GCSIK_DIVISION_FACTOR;
        (*errorVector)(pos+1,0)=euler(1)*orientationWeight/GCSIK_DIVISION_FACTOR;
        pos=pos+2;
    }
    if (theConstraints&IK_GAMMA_CONSTRAINT)
    {
        for (int i=0;i<doF;i++)
            (*matrix)(pos,i)=(*Ja)(5,i);
        C4X4Matrix diff(oldMatrInv*m);
        C3Vector euler(diff.M.getEulerAngles());
        (*errorVector)(pos,0)=euler(2)*orientationWeight/GCSIK_DIVISION_FACTOR;
        pos++;
    }

    delete Ja; // We delete the jacobian!
}


CIKChain::~CIKChain()
{
    delete matrix;
    delete errorVector;
}

void CIKChain::multiply(C4X4FullMatrix& d0,C4X4FullMatrix& dp,int index,
                       std::vector<C4X4FullMatrix*>& allMatrices)
{
// Input transformation matrices:
// Right part:
// R=m0+m1*deltaQ1+m2*deltaQ2+m3*deltaQ3+...+m6*deltaQ6
// Left part:
// L=d0+dp*deltaQindex
//Output transformation matrices:
// R=L*R
// index should be between 1 and n (indication which deltaQ dp has to be multiplied with
// If index is different from that, d0+dp*deltQindex=d0=normal transformation matrix
// If index==1, it concerns the first joint in the chain (from the tooltip), etc.

    C4X4FullMatrix& m0=*allMatrices[0];
    C4X4FullMatrix m0Saved(m0);
    m0=d0*m0Saved;
    for (int i=1;i<int(allMatrices.size());i++)
        (*allMatrices[i])=d0*(*allMatrices[i]);
    if ((index>0)&&(index<int(allMatrices.size())))
    {
        C4X4FullMatrix w(dp*m0Saved);
        (*allMatrices[index])+=w;
    }
}

void CIKChain::buildDeltaZRotation(C4X4FullMatrix& d0,C4X4FullMatrix& dp,float screwCoeff)
{
    d0.setIdentity();
    dp.clear();
    dp(0,1)=-1.0f;
    dp(1,0)=1.0f;
    dp(2,3)=screwCoeff;
}
void CIKChain::buildDeltaZTranslation(C4X4FullMatrix& d0,C4X4FullMatrix& dp)
{
    d0.setIdentity();
    dp.clear();
    dp(2,3)=1.0f;
}

CMatrix* CIKChain::getJacobian(CIKDummy* tooltip,C4X4Matrix& tooltipTransf,std::vector<CIKJoint*>& theRowJoints,int jointNbToExclude)
{   // theRowJoints will contain the IK-joint objects corresponding to the columns of the jacobian.
    // tooltipTransf is the cumulative transformation of the tooltip (aux. return value)
    // The temporary joint parameters need to be initialized before calling this function!

    // We check the number of degrees of freedom and prepare the theRowJoints vector:
    CIKObject* iterat=tooltip;
    int doF=0;
    while (iterat!=nullptr)
    {
        iterat=iterat->parent;
        if ( (iterat!=nullptr)&&(iterat->objectType==IK_JOINT_TYPE) )
        {
            if (((CIKJoint*)iterat)->active)
            {
                theRowJoints.push_back((CIKJoint*)iterat);
                doF++;
            }
        }
    }

    // Here we have to compensate for jointNbToExclude:
    if (jointNbToExclude>0)
    {   
        doF-=jointNbToExclude;
        if (doF<1)
        { // Impossible to get a Jacobian in that case!
            theRowJoints.clear();
            return(nullptr); 
        }
        theRowJoints.erase(theRowJoints.end()-jointNbToExclude,theRowJoints.end());
    }

    CMatrix* J=new CMatrix(6,(unsigned char)doF);
    std::vector<C4X4FullMatrix*> jMatrices;
    jMatrices.reserve(doF+1);
    jMatrices.clear();
    for (int i=0;i<(doF+1);i++)
    {
        C4X4FullMatrix* matr=new C4X4FullMatrix();
        if (i==0)
            (*matr).setIdentity();
        else
            (*matr).clear();
        jMatrices.push_back(matr);
    }
    // Now we go from tip to base:
    iterat=tooltip;
    C4X4FullMatrix buff;
    buff.setIdentity();
    int positionCounter=0;
    C4X4FullMatrix d0;
    C4X4FullMatrix dp;
    C4X4FullMatrix paramPart;
    CIKJoint* lastJoint=nullptr;
    int jointCounter=0;
    while (iterat!=nullptr)
    {
        C7Vector local;

        if ((jointCounter<doF)&&((CIKJoint*)iterat)->active )
            local=iterat->getLocalTransformationPart1(true);
        else
            local=iterat->getLocalTransformation(true);
        if ( (iterat!=nullptr)&&(iterat->objectType==IK_JOINT_TYPE)&&((CIKJoint*)iterat)->active )
            jointCounter++;


        buff=C4X4FullMatrix(local.getMatrix())*buff;
        iterat=iterat->parent;

        if ( (iterat==nullptr)||((iterat->objectType==IK_JOINT_TYPE)&&((CIKJoint*)iterat)->active&&(positionCounter<doF)) )
        {
            if (positionCounter==0)
            {   // Here we have the first part (from tooltip to first joint)
                d0=buff;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            else
            {   // Here we have a joint:
                if (lastJoint->revolute)
                {
                    buildDeltaZRotation(d0,dp,lastJoint->screwPitch);
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildZRotation(lastJoint->tempParameter);
                }
                else
                {
                    buildDeltaZTranslation(d0,dp);
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildTranslation(0.0f,0.0f,lastJoint->tempParameter);
                }
                d0=buff*paramPart;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            buff.setIdentity();
            lastJoint=(CIKJoint*)iterat;
            positionCounter++;
        }
    }

    // The x-, y- and z-component:
    for (int i=0;i<doF;i++)
    {
        (*J)(0,i)=(*jMatrices[1+i])(0,3);
        (*J)(1,i)=(*jMatrices[1+i])(1,3);
        (*J)(2,i)=(*jMatrices[1+i])(2,3);
    }
    // We divide all delta components (to avoid distorsions)...
    for (int i=0;i<doF;i++)
        (*jMatrices[1+i])/=GCSIK_DIVISION_FACTOR;
    // ...and add the cumulative transform to the delta-components:
    for (int i=0;i<doF;i++)
        (*jMatrices[1+i])+=(*jMatrices[0]);
    // We also copy the cumulative transform to 'tooltipTransf':
    tooltipTransf=(*jMatrices[0]);
    // Now we extract the delta Euler components:
    C4X4FullMatrix mainInverse(*jMatrices[0]);
    mainInverse.invert();
    C4X4FullMatrix tmp;
    // Alpha-, Beta- and Gamma-components:
    for (int i=0;i<doF;i++)
    {
        tmp=mainInverse*(*jMatrices[1+i]);
        C3Vector euler(tmp.getEulerAngles());
        (*J)(3,i)=euler(0);
        (*J)(4,i)=euler(1);
        (*J)(5,i)=euler(2);
    }
    // We free the memory allocated for each joint variable:
    for (int i=0;i<int(jMatrices.size());i++)
        delete jMatrices[i];

    // Now we have to combine columns which are linked to the same joints:
    for (int i=0;i<int(theRowJoints.size());i++)
    {
        CIKJoint* aJoint=theRowJoints[i];
        if (aJoint!=nullptr)
        {
            for (int j=i+1;j<int(theRowJoints.size());j++)
            {
                CIKJoint* bJoint=theRowJoints[j];
                if (bJoint!=nullptr)
                {
                    if (aJoint==bJoint->avatarParent)
                    {           
                        for (int k=0;k<J->rows;k++)
                            (*J)(k,i)+=(*J)(k,j);
                        theRowJoints[j]=nullptr; // We remove that joint
                        break;
                    }
                    if (bJoint==aJoint->avatarParent)
                    {           
                        for (int k=0;k<J->rows;k++)
                            (*J)(k,j)+=(*J)(k,i);
                        theRowJoints[i]=nullptr; // We remove that joint
                        break;
                    }
                }
            }
        }
    }
    // And now we create the new matrix and rowJoints:
    int colNb=0;
    std::vector<CIKJoint*> rowJointsCopy(theRowJoints);
    theRowJoints.clear();
    for (int i=0;i<int(rowJointsCopy.size());i++)
    {
        if (rowJointsCopy[i]!=nullptr)
            colNb++;
    }
    CMatrix* oldMatrix=J;
    J=new CMatrix(oldMatrix->rows,colNb);
    int horizPos=0;
    for (int i=0;i<oldMatrix->cols;i++)
    {
        if (rowJointsCopy[i]!=nullptr)
        {
            for (int j=0;j<oldMatrix->rows;j++)
                (*J)(j,horizPos)=(*oldMatrix)(j,i);
            theRowJoints.push_back(rowJointsCopy[i]);
            horizPos++;
        }
    }
    delete oldMatrix;
    return(J);
}

