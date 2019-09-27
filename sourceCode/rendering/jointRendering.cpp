
#include "jointRendering.h"

#ifdef SIM_WITH_OPENGL

void displayJoint(CJoint* joint,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(joint,renderingObject,displayAttrib);

    bool guiIsRendering=((displayAttrib&sim_displayattribute_forvisionsensor)==0);

    // Bounding box display:
    float sizeParam=joint->getLength()/4.0f;
    if (sizeParam<joint->getDiameter())
        sizeParam=joint->getDiameter();
    if (sizeParam>10.0f*joint->getDiameter())
        sizeParam=10.0f*joint->getDiameter();
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(joint,displayAttrib,true,sizeParam);

    // Object display:
    if (joint->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        _enableAuxClippingPlanes(joint->getObjectHandle());
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
        {
            if (joint->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering)==4)
                ogl::setMaterialColor(0.0f,0.3f,1.0f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            if (joint->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering)==8)
                ogl::setMaterialColor(1.0f,0.0f,0.0f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
            if (joint->getDynamicObjectFlag_forVisualization_forDisplay(guiIsRendering)==16)
                ogl::setMaterialColor(0.9f,0.9f,0.0f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
        }
        else
            joint->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);

        _displayJoint(joint,displayAttrib,true,sizeParam);

        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            ogl::setMaterialColor(0.1f,0.1f,0.1f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
        else
            joint->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        _displayJoint(joint,displayAttrib,false,sizeParam);
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(joint,renderingObject);
}

void _displayJoint(CJoint* joint,int displayAttrib,bool partOne,float sizeParam)
{
    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
    {
        if (joint->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
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
            if (joint->getDynamicSecondPartIsValid()&&(!App::ct->simulation->isSimulationStopped()))
            { // for dynamic mode
                C7Vector tr(joint->getDynamicSecondPartLocalTransform());
                glTranslatef(tr.X(0),tr.X(1),tr.X(2));
                C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
                glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            }
            else
            {
                C4Vector tr(joint->getSphericalTransformation().getAngleAndAxis());
                glRotatef(tr(0)*radToDeg_f,tr(1),tr(2),tr(3));
            }

            ogl::drawSphere(joint->getDiameter()/1.5f,16,8,true);
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
            if (joint->getDynamicSecondPartIsValid()&&(!App::ct->simulation->isSimulationStopped()))
            { // for dynamic mode
                C7Vector tr(joint->getDynamicSecondPartLocalTransform());
                glTranslatef(tr.X(0),tr.X(1),tr.X(2));
                C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
                glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            }
            else
            {
                glRotatef(joint->getPosition()*radToDeg_f,0.0f,0.0f,1.0f);
                glTranslatef(0.0f,0.0f,joint->getPosition()*joint->getScrewPitch());
            }

            ogl::drawCylinder(joint->getDiameter()/2.0f,joint->getLength()*1.2f,8,0,true);
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
            if (joint->getDynamicSecondPartIsValid()&&(!App::ct->simulation->isSimulationStopped()))
            { // for dynamic mode
                C7Vector tr(joint->getDynamicSecondPartLocalTransform());
                glTranslatef(tr.X(0),tr.X(1),tr.X(2));
                C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
                glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            }
            else
                glTranslatef(0.0f,0.0f,joint->getPosition());

            ogl::drawBox(joint->getDiameter()/2.0f,joint->getDiameter()/2.0f,joint->getLength()*1.2f,true,nullptr);
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



