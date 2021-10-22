#include "dummyRendering.h"

#ifdef SIM_WITH_OPENGL

void displayDummy(CDummy* dummy,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(dummy,renderingObject,displayAttrib);
    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(dummy,displayAttrib,true,dummy->getDummySize()*2.0f);

    C3Vector normalVectorForLinesAndPoints(dummy->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Object display:
    if (dummy->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (dummy->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(dummy->getModelSelectionHandle());
            else
                glLoadName(dummy->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
        if ( (displayAttrib&sim_displayattribute_forcewireframe)==0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(dummy->getObjectHandle());
        if ((displayAttrib&sim_displayattribute_selected)==0)
            ogl::drawReference(dummy->getDummySize()*2.0f,true,true,false,normalVectorForLinesAndPoints.data);
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            ogl::setMaterialColor(0.0f,0.6f,0.6f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
        else
        {
            bool setOtherColor=(App::currentWorld->collisions->getCollisionColor(dummy->getObjectHandle())!=0);
            for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
            {
                if (App::currentWorld->collections->getObjectFromIndex(i)->isObjectInCollection(dummy->getObjectHandle()))
                    setOtherColor|=(App::currentWorld->collisions->getCollisionColor(App::currentWorld->collections->getObjectFromIndex(i)->getCollectionHandle())!=0);
            }
            if (!setOtherColor)
                dummy->getDummyColor()->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            else
                App::currentWorld->mainSettings->collisionColor.makeCurrentColor(false);
        }
        CDummy* linkedDummy=nullptr;
        if (dummy->getLinkedDummyHandle()!=-1)
            linkedDummy=App::currentWorld->sceneObjects->getDummyFromHandle(dummy->getLinkedDummyHandle());
        ogl::drawSphere(dummy->getDummySize()/2.0f,12,6,false);
        glDisable(GL_CULL_FACE);
        if (linkedDummy!=nullptr)
        {
            C7Vector cumulBase(dummy->getFullCumulativeTransformation());
            C7Vector cumulMobile(linkedDummy->getFullCumulativeTransformation());
            cumulMobile=cumulBase.getInverse()*cumulMobile;
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            int _linkType=dummy->getLinkType();
            if ((_linkType==sim_dummy_linktype_gcs_loop_closure)||(_linkType==sim_dummy_linktype_gcs_tip)||(_linkType==sim_dummy_linktype_gcs_target))
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorGreen);
            if (_linkType==sim_dummy_linktype_ik_tip_target)
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorRed);
            if ((_linkType==sim_dummy_linktype_dynamics_loop_closure)||(_linkType==sim_dummy_linktype_dynamics_force_constraint))
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ogl::colorBlue);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(0.0f,0.0f,0.0f);
            ogl::addBuffer3DPoints(cumulMobile.X.data);
            ogl::drawRandom3dLines(&ogl::buffer[0],2,false,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
        }
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(dummy,renderingObject);
}

#else

void displayDummy(CDummy* dummy,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



