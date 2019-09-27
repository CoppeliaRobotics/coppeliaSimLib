
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "ikRoutine.h"
#include "app.h"
#include "tt.h"

void CIkRoutine::multiply(C4X4FullMatrix& d0,C4X4FullMatrix& dp,int index,
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

void CIkRoutine::buildDeltaZRotation(C4X4FullMatrix& d0,C4X4FullMatrix& dp,float screwCoeff)
{
    d0.setIdentity();
    dp.clear();
    dp(0,1)=-1.0f;
    dp(1,0)=1.0f;
    dp(2,3)=screwCoeff;
}

void CIkRoutine::buildDeltaZTranslation(C4X4FullMatrix& d0,C4X4FullMatrix& dp)
{
    d0.setIdentity();
    dp.clear();
    dp(2,3)=1.0f;
}

CMatrix* CIkRoutine::getJacobian(CikEl* ikElement,C4X4Matrix& tooltipTransf,std::vector<int>* rowJointIDs,std::vector<int>* rowJointStages)
{   // rowJointIDs is nullptr by default. If not null, it will contain the ids of the joints
    // corresponding to the rows of the jacobian.
    // Return value nullptr means that is ikElement is either inactive, either invalid
    // tooltipTransf is the cumulative transformation matrix of the tooltip,
    // computed relative to the base!
    // The temporary joint parameters need to be initialized before calling this function!
    // We check if the ikElement's base is in the chain and that tooltip is valid!
    CDummy* tooltip=App::ct->objCont->getDummy(ikElement->getTooltip());
    if (tooltip==nullptr)
    { // Should normally never happen!
        // 28/4/2016 ikElement->setActive(false);
        return(nullptr);
    }
    C3DObject* base=App::ct->objCont->getObjectFromHandle(ikElement->getBase());
    if ( (base!=nullptr)&&(!tooltip->isObjectParentedWith(base)) )
    { // This case can happen (when the base's parenting was changed for instance)
        ikElement->setBase(-1);
        // 28/4/2016 ikElement->setActive(false);
        return(nullptr);
    }

    // We check the number of degrees of freedom and prepare the rowJointIDs vector:
    C3DObject* iterat=tooltip;
    int doF=0;
    while (iterat!=base)
    {
        iterat=iterat->getParentObject();
        if ( (iterat!=nullptr)&&(iterat!=base) )
        {
            if (iterat->getObjectType()==sim_object_joint_type)
            {
                if ( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) )
                {
                    int d=((CJoint*)iterat)->getDoFs();
                    for (int i=d-1;i>=0;i--)
                    {
                        if (rowJointIDs!=nullptr)
                        {
                            rowJointIDs->push_back(iterat->getObjectHandle());
                            rowJointStages->push_back(i);
                        }
                    }
                    doF+=d;
                }
            }
        }
    }
    CMatrix* J=new CMatrix(6,(unsigned char)doF);
    std::vector<C4X4FullMatrix*> jMatrices;
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
    CJoint* lastJoint=nullptr;
    int indexCnt=-1;
    int indexCntLast=-1;
    while (iterat!=base)
    {
        C3DObject* nextIterat=iterat->getParentObject();
        C7Vector local;
        if (iterat->getObjectType()==sim_object_joint_type)
        {
            if ( (((CJoint*)iterat)->getJointMode()!=sim_jointmode_ik)&&(((CJoint*)iterat)->getJointMode()!=sim_jointmode_reserved_previously_ikdependent)&&(((CJoint*)iterat)->getJointMode()!=sim_jointmode_dependent) )
                local=iterat->getLocalTransformation(true);
            else
            {
                CJoint* it=(CJoint*)iterat;
                if (it->getJointType()==sim_joint_spherical_subtype)
                {
                    if (indexCnt==-1)
                        indexCnt=it->getDoFs()-1;
                    it->getLocalTransformationExPart1(local,indexCnt--,true);
                    if (indexCnt!=-1)
                        nextIterat=iterat; // We keep the same object! (but indexCnt has decreased)
                }
                else
                    local=iterat->getLocalTransformationPart1(true);
            }
        }
        else
            local=iterat->getLocalTransformation(true); 

        buff=C4X4FullMatrix(local.getMatrix())*buff;
        iterat=nextIterat;
        bool activeJoint=false;
        if (iterat!=nullptr) // Following lines recently changed!
        {
            if (iterat->getObjectType()==sim_object_joint_type) 
                activeJoint=( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) );
        }
        if ( (iterat==base)||activeJoint )
        {   // If base is nullptr then the second part is not evaluated (iterat->getObjectType())
            if (positionCounter==0)
            {   // Here we have the first part (from tooltip to first joint)
                d0=buff;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            else
            {   // Here we have a joint:
                if (lastJoint->getJointType()==sim_joint_revolute_subtype)
                {
                    buildDeltaZRotation(d0,dp,lastJoint->getScrewPitch());
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildZRotation(lastJoint->getPosition(true));
                }
                else if (lastJoint->getJointType()==sim_joint_prismatic_subtype)
                {
                    buildDeltaZTranslation(d0,dp);
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildTranslation(0.0f,0.0f,lastJoint->getPosition(true));
                }
                else 
                { // Spherical joint part!
                    buildDeltaZRotation(d0,dp,0.0f);
                    multiply(d0,dp,positionCounter,jMatrices);
                    if (indexCntLast==-1)
                        indexCntLast=lastJoint->getDoFs()-1;
                    paramPart.buildZRotation(lastJoint->getTempParameterEx(indexCntLast--));
                }
                d0=buff*paramPart;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            buff.setIdentity();
            lastJoint=(CJoint*)iterat;
            positionCounter++;
        }
    }

    int alternativeBaseForConstraints=ikElement->getAlternativeBaseForConstraints();
    if (alternativeBaseForConstraints!=-1)
    {
        CDummy* alb=App::ct->objCont->getDummy(alternativeBaseForConstraints);
        if (alb!=nullptr)
        { // We want everything relative to the alternativeBaseForConstraints dummy orientation!
            C7Vector alternativeBase(alb->getCumulativeTransformationPart1(true));
            C7Vector currentBase;
            currentBase.setIdentity();
            if (base!=nullptr)
                currentBase=base->getCumulativeTransformation(true); // could be a joint, we want also the joint intrinsic transformation part!
            C4X4FullMatrix correction((alternativeBase.getInverse()*currentBase).getMatrix());
            dp.clear();
            multiply(correction,dp,0,jMatrices);
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
        (*jMatrices[1+i])/=IK_DIVISION_FACTOR;
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
        (*J)(3,i)=euler(0); // here we would have to multiply the euler angle with IK_DIVISION_FACTOR to get the "correct" Jacobian
        (*J)(4,i)=euler(1); // here we would have to multiply the euler angle with IK_DIVISION_FACTOR to get the "correct" Jacobian
        (*J)(5,i)=euler(2); // here we would have to multiply the euler angle with IK_DIVISION_FACTOR to get the "correct" Jacobian
    }

// output the jacobian!
//  for (int j=0;j<6;j++)
//  {
//      for (int i=0;i<doF;i++)
//          printf("%.2f ",(*J)(j,i));
//      printf("\n");
//  }
//  printf("****************** \n");

    /* // testing
    for (int j=0;j<2;j++)
    {
    for (int i=0;i<doF;i++)
    printf("%.2f ",(*J)(j,i));
    printf("\n");
    }
    printf("****************** \n");
    //*/


    // We free the memory allocated for each joint variable:
    for (int i=0;i<int(jMatrices.size());i++)
        delete jMatrices[i];
    return(J);
}

CMatrix* CIkRoutine::getAvoidanceJacobian(C3DObject* base,C3DObject* avoidingObject,const C4X4Matrix& relPos,std::vector<int>* rowJointIDs,std::vector<int>* rowJointStages)
{   // rowJointIDs is nullptr by default. If not null, it will contain the ids of the joints
    // corresponding to the rows of the jacobian.
    // This function computes the jacobian (position part only) of an avoidance object
    // The temporary joint parameters need to be initialized before calling this function!

    // We check the number of degrees of freedom and prepare the rowJointIDs vector:
    C3DObject* iterat=avoidingObject;
    int doF=0;
    while (iterat!=base)
    {
        iterat=iterat->getParentObject();
        if ( (iterat!=nullptr)&&(iterat!=base) )
        {
            if (iterat->getObjectType()==sim_object_joint_type)
            {
                if ( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) )
                {
                    int d=((CJoint*)iterat)->getDoFs();
                    for (int i=d-1;i>=0;i--)
                    {
                        if (rowJointIDs!=nullptr)
                        {
                            rowJointIDs->push_back(iterat->getObjectHandle());
                        }
                    }
                    doF+=d;
                }
            }
        }
    }
    CMatrix* J=new CMatrix(3,(unsigned char)doF); // We want only the position part!
    std::vector<C4X4FullMatrix*> jMatrices;
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
    iterat=avoidingObject;
    C4X4FullMatrix buff(relPos);// We have to take into account the point on the avoiding shape!!

    int positionCounter=0;
    C4X4FullMatrix d0;
    C4X4FullMatrix dp;
    C4X4FullMatrix paramPart;
    CJoint* lastJoint=nullptr;
    int indexCnt=-1;
    int indexCntLast=-1;
    while (iterat!=base)
    {
        C3DObject* nextIterat=iterat->getParentObject();
        C7Vector local;
        if (iterat->getObjectType()==sim_object_joint_type)
        {
            if ( (((CJoint*)iterat)->getJointMode()!=sim_jointmode_ik)&&(((CJoint*)iterat)->getJointMode()!=sim_jointmode_reserved_previously_ikdependent)&&(((CJoint*)iterat)->getJointMode()!=sim_jointmode_dependent) )
                local=iterat->getLocalTransformation(true);
            else
            {
                CJoint* it=(CJoint*)iterat;
                if (it->getJointType()==sim_joint_spherical_subtype)
                {
                    if (indexCnt==-1)
                        indexCnt=it->getDoFs()-1;
                    it->getLocalTransformationExPart1(local,indexCnt--,true);
                    if (indexCnt!=-1)
                        nextIterat=iterat; // We keep the same object! (but indexCnt has decreased)
                }
                else
                    local=iterat->getLocalTransformationPart1(true);
            }
        }
        else
            local=iterat->getLocalTransformation(true).getMatrix();
        buff=C4X4FullMatrix(local.getMatrix())*buff;
        iterat=nextIterat;
        bool activeJoint=false;
        if (iterat!=nullptr)
        {
            if (iterat->getObjectType()==sim_object_joint_type) 
                activeJoint=( (((CJoint*)iterat)->getJointMode()==sim_jointmode_ik)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_reserved_previously_ikdependent)||(((CJoint*)iterat)->getJointMode()==sim_jointmode_dependent) );
        }
        if ( (iterat==base)||activeJoint )
        {   // If base is nullptr then the second part is not evaluated (iterat->getObjectType())
            if (positionCounter==0)
            {   // Here we have the first part (from tooltip to first joint)
                d0=buff;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            else
            {   // Here we have a joint:
                if (lastJoint->getJointType()==sim_joint_revolute_subtype)
                {
                    buildDeltaZRotation(d0,dp,lastJoint->getScrewPitch());
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildZRotation(lastJoint->getPosition(true));
                }
                else if (lastJoint->getJointType()==sim_joint_prismatic_subtype)
                {
                    buildDeltaZTranslation(d0,dp);
                    multiply(d0,dp,positionCounter,jMatrices);
                    paramPart.buildTranslation(0.0f,0.0f,lastJoint->getPosition(true));
                }
                else 
                { // Spherical joint part!
                    buildDeltaZRotation(d0,dp,0.0f);
                    multiply(d0,dp,positionCounter,jMatrices);
                    if (indexCntLast==-1)
                        indexCntLast=lastJoint->getDoFs()-1;
                    paramPart.buildZRotation(lastJoint->getTempParameterEx(indexCntLast--));
                }
                d0=buff*paramPart;
                dp.clear();
                multiply(d0,dp,0,jMatrices);
            }
            buff.setIdentity();
            lastJoint=(CJoint*)iterat;
            positionCounter++;
        }
    }

// ------------------------ TEST (REMOVE LATER!!!!!!) ---------------------------------
//  for (int i=0;i<doF;i++)
//  {
//      CJoint* act=App::ct->objCont->getJoint(rowJointIDs->at(i));
//      tt::multiply(*jMatrices[1+i],act->getIKWeight());
//  }
// ------------------------------------------------------------------------------------
    
    // The x-, y- and z-component:
    for (int i=0;i<doF;i++)
    {
        (*J)(0,i)=(*jMatrices[1+i])(0,3);
        (*J)(1,i)=(*jMatrices[1+i])(1,3);
        (*J)(2,i)=(*jMatrices[1+i])(2,3);
    }

    // We free the memory allocated for each joint variable:
    for (int i=0;i<int(jMatrices.size());i++)
        delete jMatrices[i];
    return(J);
}


