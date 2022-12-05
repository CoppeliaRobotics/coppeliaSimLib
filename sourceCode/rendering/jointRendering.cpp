#include "jointRendering.h"

#ifdef SIM_WITH_OPENGL

void displayJoint(CJoint* joint,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(joint,renderingObject,displayAttrib);

    bool guiIsRendering=((displayAttrib&sim_displayattribute_forvisionsensor)==0);

    // Bounding box display:
    double sizeParam=joint->getLength()/4.0;
    if (sizeParam<joint->getDiameter())
        sizeParam=joint->getDiameter();
    if (sizeParam>10.0*joint->getDiameter())
        sizeParam=10.0*joint->getDiameter();
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(joint,displayAttrib,true,sizeParam);

    // Object display:
    if (joint->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        _enableAuxClippingPlanes(joint->getObjectHandle());
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
        {
            if (joint->getDynamicFlag()==4)
            {
                if (joint->getDynCtrlMode()==sim_jointdynctrl_free)
                    ogl::setMaterialColor(0.0,0.3,1.0,0.5,0.5,0.5,0.0,0.0,0.0);
                else
                    ogl::setMaterialColor(1.0,0.0,0.0,0.5,0.5,0.5,0.0,0.0,0.0);
            }
        }
        else
            joint->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);

        _displayJoint(joint,displayAttrib,true,sizeParam);

        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            ogl::setMaterialColor(0.1,0.1,0.1,0.5,0.5,0.5,0.0,0.0,0.0);
        else
            joint->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        _displayJoint(joint,displayAttrib,false,sizeParam);
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(joint,renderingObject);
}

void _displayJoint(CJoint* joint,int displayAttrib,bool partOne,double sizeParam)
{
    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
    {
        if (joint->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
            glLoadName(joint->getModelSelectionHandle());
        else
            glLoadName(joint->getObjectHandle());
    }
    else
        glLoadName(-1);

    glPushAttrib(GL_POLYGON_BIT);
    if ((displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass))
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    glPushMatrix();
    if (joint->getJointType()==sim_joint_spherical_subtype)
    {
        if (partOne)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            ogl::drawSphere(joint->getDiameter(),16,8,true);
            glDisable(GL_CULL_FACE);
        }
        else
        {
            C7Vector tr(joint->getIntrinsicTransformation(true));
            glTranslated(tr.X(0),tr.X(1),tr.X(2));
            C4Vector axis=tr.Q.getAngleAndAxis();
            glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));

            ogl::drawSphere(joint->getDiameter()/1.5,16,8,true);
            if (displayAttrib&sim_displayattribute_selected)
                _drawReference(joint,sizeParam);
        }
    }
    if (joint->getJointType()==sim_joint_revolute_subtype)
    {
        if (partOne)
            ogl::drawCylinder(joint->getDiameter(),joint->getLength(),8,0,true);
        else
        {
            C7Vector tr(joint->getIntrinsicTransformation(true));
            glTranslated(tr.X(0),tr.X(1),tr.X(2));
            C4Vector axis=tr.Q.getAngleAndAxis();
            glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));

            ogl::drawCylinder(joint->getDiameter()/2.0,joint->getLength()*1.2,8,0,true);
            if (displayAttrib&sim_displayattribute_selected)
                _drawReference(joint,sizeParam);
        }
    }
    if (joint->getJointType()==sim_joint_prismatic_subtype)
    {
        if (partOne)
            ogl::drawBox(joint->getDiameter(),joint->getDiameter(),joint->getLength(),true,nullptr);
        else
        {
            C7Vector tr(joint->getIntrinsicTransformation(true));
            glTranslated(tr.X(0),tr.X(1),tr.X(2));
            C4Vector axis=tr.Q.getAngleAndAxis();
            glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));

            ogl::drawBox(joint->getDiameter()/2.0,joint->getDiameter()/2.0,joint->getLength()*1.2,true,nullptr);
            if (displayAttrib&sim_displayattribute_selected)
                _drawReference(joint,sizeParam);
        }
    }
    glPopAttrib();
    glPopMatrix();
}

#else

void displayJoint(CJoint* joint,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



