
#include "pageRendering.h"

#ifdef SIM_WITH_GUI

#define FLOATING_VIEW_MIN_SIZE 80
const int auxViewsBtSize=12;

enum {AUX_VIEW_SHIFTING=0,AUX_VIEW_TOP_BORDER,AUX_VIEW_BOTTOM_BORDER,AUX_VIEW_RIGHT_BORDER,
    AUX_VIEW_LEFT_BORDER,AUX_VIEW_TOP_RIGHT_BORDER,AUX_VIEW_TOP_LEFT_BORDER,
    AUX_VIEW_BOTTOM_RIGHT_BORDER,AUX_VIEW_BOTTOM_LEFT_BORDER,AUX_VIEW_CLOSING_BUTTON};

void displayContainerPage(CSPage* page,const int* position,const int* size)
{
    if (page!=nullptr)
        page->render();
    else
    { // We draw a dark grey view:
        glEnable(GL_SCISSOR_TEST);
        glViewport(position[0],position[1],size[0],size[1]);
        glScissor(position[0],position[1],size[0],size[1]);
        glClearColor(0.3f,0.3f,0.3f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void displayContainerPageOverlay(const int* position,const int* size,int activePageIndex,int focusObject)
{
    // We prepare a new modelview to draw on top:
    glDisable(GL_SCISSOR_TEST);
    glViewport(position[0],position[1],size[0],size[1]);
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,size[0],0.0f,size[1],-1.0f,1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDisable(GL_DEPTH_TEST);

    if (App::getEditModeType()==BUTTON_EDIT_MODE)
    {
        glEnable(GL_SCISSOR_TEST);
        glViewport(position[0],position[1],size[0],size[1]);
        glScissor(position[0],position[1],size[0],size[1]);
        glClearColor(0.3f,0.3f,0.3f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        App::ct->globalGuiTextureCont->startTextureDisplay(CHECKED_BACKGROUND_PICTURE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

        glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
        glEnable(GL_TEXTURE_2D);
        ogl::disableLighting_useWithCare(); // only temporarily
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);
        glVertex3i(0,0,0);
        glTexCoord2f(size[0]/16.0f,0.0f);
        glVertex3i(size[0],0,0);
        glTexCoord2f(size[0]/16.0f,size[1]/16.0f);
        glVertex3i(size[0],size[1],0);
        glTexCoord2f(0.0f,size[1]/16.0f);
        glVertex3i(0,size[1],0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        ogl::enableLighting_useWithCare();
        App::ct->globalGuiTextureCont->endTextureDisplay();
    }

    // Now we draw soft dialogs:
    if (App::ct->buttonBlockContainer!=nullptr)
        App::ct->buttonBlockContainer->displayAllBlocks(activePageIndex,focusObject);

    App::ct->calcInfo->printInformation();

    glEnable(GL_DEPTH_TEST);
}

void displayContainerPageWatermark(const int* position,const int* size,int tagId)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,size[0],0,size[1],-1,1);
    glDisable(GL_SCISSOR_TEST);
    glViewport(position[0],position[1],size[0],size[1]);
    glDisable(GL_DEPTH_TEST);
    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
    ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    ogl::disableLighting_useWithCare(); // only temporarily
    App::ct->globalGuiTextureCont->startTextureDisplay(tagId);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);
    glVertex3i(0,0,0);
    glTexCoord2f(1.0f,0.0f);
    glVertex3i(256*App::sc,0,0);
    glTexCoord2f(1.0f,1.0f);
    glVertex3i(256*App::sc,128*App::sc,0);
    glTexCoord2f(0.0f,1.0f);
    glVertex3i(0,128*App::sc,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    ogl::setBlending(false);
    ogl::enableLighting_useWithCare();
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
}

void displayPage(CSPage* page,int auxViewResizingAction,int viewIndexOfResizingAction)
{
    // We first render all regular views:
    int mp[2];
    int ms[2];
    for (int i=0;i<page->getRegularViewCount();i++)
    {
        page->getViewSizeAndPosition(ms,mp,0);
        page->getView(i)->setViewIndex(i);
        page->getView(i)->render(mp[0],false,i==0,page->viewIsPassive(i));
    }
    // Then we handle mouse commands for all subviews:
    for (int i=0;i<page->getRegularViewCount();i++)
        page->getView(i)->handleCameraOrGraphMotion();
    // Now we render a grid on top of it (separation between windows):

    int _pageSize[2];
    int _pagePosition[2];
    page->getPageSizeAndPosition(_pageSize[0],_pageSize[1],_pagePosition[0],_pagePosition[1]);
    int _pageType=page->getPageType();
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glEnable(GL_SCISSOR_TEST);
    glViewport(_pagePosition[0],_pagePosition[1],_pageSize[0],_pageSize[1]);
    glScissor(_pagePosition[0],_pagePosition[1],_pageSize[0],_pageSize[1]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,_pageSize[0],0.0f,_pageSize[1],-1.0f,1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDisable(GL_DEPTH_TEST);
    glLineWidth(3.0f);
    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorLightGrey);
    _drawSeparationLines(_pageType,_pageSize);
    glLineWidth(1.0f);
    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
    _drawSeparationLines(_pageType,_pageSize);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    for (int i=page->getRegularViewCount();i<page->getViewCount();i++)
    {
        page->getView(i)->setViewIndex(i);
        page->getView(i)->render(_pagePosition[0],true,false,page->viewIsPassive(i));
        glEnable(GL_SCISSOR_TEST);
        glViewport(_pagePosition[0],_pagePosition[1],_pageSize[0],_pageSize[1]);
        glScissor(_pagePosition[0],_pagePosition[1],_pageSize[0],_pageSize[1]);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0f,_pageSize[0],0.0f,_pageSize[1],-1.0f,1.0f);
        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
        glDisable(GL_DEPTH_TEST);

        // Positions and sizes are relative now (2009/05/22)
        //int avs[2]={int(_allViewAuxSizesAndPos[4*i+0]*float(_pageSize[0])),int(_allViewAuxSizesAndPos[4*i+1]*float(_pageSize[1]))};
        //int avp[2]={int(_allViewAuxSizesAndPos[4*i+2]*float(_pageSize[0])),int(_allViewAuxSizesAndPos[4*i+3]*float(_pageSize[1]))};
        float ss_[2];
        float pp_[2];
        page->getFloatingViewRelativeSizeAndPosition(ss_,pp_,i);
        int avs[2]={int(ss_[0]*float(_pageSize[0])),int(ss_[1]*float(_pageSize[1]))};
        int avp[2]={int(pp_[0]*float(_pageSize[0])),int(pp_[1]*float(_pageSize[1]))};

        // Make sure we are not smaller than the minimum size:
        if (avs[0]<FLOATING_VIEW_MIN_SIZE)
            avs[0]=FLOATING_VIEW_MIN_SIZE;
        if (avs[1]<FLOATING_VIEW_MIN_SIZE)
            avs[1]=FLOATING_VIEW_MIN_SIZE;

        page->getBorderCorrectedFloatingViewPosition(avp[0],avp[1],avs[0],avs[1],avp[0],avp[1]);


        VPoint pos(_pageSize[0]-avp[0]-avs[0]/2-auxViewsBtSize*App::sc/2,_pageSize[1]-avp[1]-auxViewsBtSize*App::sc/2);
        VPoint size(avs[0]-auxViewsBtSize*App::sc,auxViewsBtSize*App::sc);

        if (!page->getView(i)->getCanBeClosed())
        { // New since 17/02/2011 (so that we don't have a gap where the button was)
            pos.x=_pageSize[0]-avp[0]-avs[0]/2;
            size.x=avs[0];
        }

        float txtCol[3]={0.2f,0.2f,0.2f};
        float* bkgrndCol=ogl::TITLE_BAR_COLOR;
        C3DObject* itObj=App::ct->objCont->getObjectFromHandle(page->getView(i)->getLinkedObjectID());
        std::string name("  Floating view (empty)");
        std::string altName(page->getView(i)->getAlternativeViewName());
        if (altName=="")
        {
            if (itObj!=nullptr)
                name="  "+itObj->getObjectName();
        }
        else
            name="  "+altName;
        int buttonAttrib=sim_buttonproperty_label|sim_buttonproperty_enabled|sim_buttonproperty_verticallycentered;
        ogl::drawButton(pos,size,txtCol,bkgrndCol,bkgrndCol,name,buttonAttrib,false,0,0.0f,false,0,nullptr,nullptr,nullptr,nullptr,nullptr);

        pos.x=_pageSize[0]-avp[0]-auxViewsBtSize*App::sc/2;
        size.x=auxViewsBtSize*App::sc;
        float* bkgrndCol2=ogl::TITLE_BAR_BUTTON_COLOR;
        buttonAttrib=sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered;
        if ( (auxViewResizingAction==AUX_VIEW_CLOSING_BUTTON)&&(viewIndexOfResizingAction==i) )
            buttonAttrib|=sim_buttonproperty_isdown;
        if (page->getView(i)->getCanBeClosed())
            ogl::drawButton(pos,size,txtCol,bkgrndCol2,bkgrndCol2,"&&Check",buttonAttrib,false,0,0.0f,false,0,nullptr,nullptr,nullptr,nullptr,nullptr);

        glLineWidth(3.0f);
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorLightGrey);
        for (int j=0;j<2;j++)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3i(_pageSize[0]-avp[0]-avs[0]-1,_pageSize[1]-avp[1]-avs[1]-1,0);
            glVertex3i(_pageSize[0]-avp[0]+1,_pageSize[1]-avp[1]-avs[1]-1,0);
            glVertex3i(_pageSize[0]-avp[0]+1,_pageSize[1]-avp[1]+2,0);
            glVertex3i(_pageSize[0]-avp[0]-avs[0]-1,_pageSize[1]-avp[1]+2,0);
            glVertex3i(_pageSize[0]-avp[0]-avs[0]-1,_pageSize[1]-avp[1]-avs[1]-1,0);
            glEnd();
            glLineWidth(1.0f);
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
        }
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
    }
}

void _drawSeparationLines(int _pageType,const int* _pageSize)
{
    if (_pageType==SINGLE_VIEW)
    {
    }
    if (_pageType==FOUR_VIEWS)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/2,_pageSize[0],_pageSize[1]/2);
        ogl::drawSingle2dLine_i(_pageSize[0]/2,0,_pageSize[0]/2,_pageSize[1]);
    }
    if (_pageType==SIX_VIEWS_OLD)
    {
        ogl::drawSingle2dLine_i(0,2*_pageSize[1]/3,_pageSize[0],2*_pageSize[1]/3);
        ogl::drawSingle2dLine_i(_pageSize[0]/3,_pageSize[1],_pageSize[0]/3,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,_pageSize[1],2*_pageSize[0]/3,2*_pageSize[1]/3);
        ogl::drawSingle2dLine_i(0,_pageSize[1]/3,_pageSize[0]/3,_pageSize[1]/3);
    }
    if (_pageType==HORIZONTALLY_DIVIDED)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/2,_pageSize[0],_pageSize[1]/2);
    }
    if (_pageType==VERTICALLY_DIVIDED)
    {
        ogl::drawSingle2dLine_i(_pageSize[0]/2,0,_pageSize[0]/2,_pageSize[1]);
    }
    if (_pageType==HORIZONTALLY_DIVIDED_3)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/2,_pageSize[0],_pageSize[1]/2);
        ogl::drawSingle2dLine_i(_pageSize[0]/2,0,_pageSize[0]/2,_pageSize[1]/2);
    }
    if (_pageType==VERTICALLY_DIVIDED_3)
    {
        ogl::drawSingle2dLine_i(_pageSize[0]/2,0,_pageSize[0]/2,_pageSize[1]);
        ogl::drawSingle2dLine_i(_pageSize[0]/2,_pageSize[1]/2,_pageSize[0],_pageSize[1]/2);
    }
    if (_pageType==HORIZONTAL_1_PLUS_3_VIEWS)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/3,_pageSize[0],_pageSize[1]/3);
        ogl::drawSingle2dLine_i(_pageSize[0]/3,_pageSize[1]/3,_pageSize[0]/3,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,_pageSize[1]/3,2*_pageSize[0]/3,0);
    }
    if (_pageType==VERTICAL_1_PLUS_3_VIEWS)
    {
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,_pageSize[1],2*_pageSize[0]/3,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,2*_pageSize[1]/3,_pageSize[0],2*_pageSize[1]/3);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,_pageSize[1]/3,_pageSize[0],_pageSize[1]/3);
    }
    if (_pageType==HORIZONTAL_1_PLUS_4_VIEWS)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/4,_pageSize[0],_pageSize[1]/4);
        ogl::drawSingle2dLine_i(_pageSize[0]/4,_pageSize[1]/4,_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/4,_pageSize[1]/4,2*_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,_pageSize[1]/4,3*_pageSize[0]/4,0);
    }
    if (_pageType==VERTICAL_1_PLUS_4_VIEWS)
    {
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,_pageSize[1],3*_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,3*_pageSize[1]/4,_pageSize[0],3*_pageSize[1]/4);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,2*_pageSize[1]/4,_pageSize[0],2*_pageSize[1]/4);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,_pageSize[1]/4,_pageSize[0],_pageSize[1]/4);
    }
    if (_pageType==SIX_VIEWS)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/3,_pageSize[0],_pageSize[1]/3);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,_pageSize[1],2*_pageSize[0]/3,0);
        ogl::drawSingle2dLine_i(_pageSize[0]/3,_pageSize[1]/3,_pageSize[0]/3,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/3,2*_pageSize[1]/3,_pageSize[0],2*_pageSize[1]/3);
    }
    if (_pageType==EIGHT_VIEWS)
    {
        ogl::drawSingle2dLine_i(0,_pageSize[1]/4,_pageSize[0],_pageSize[1]/4);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,_pageSize[1],3*_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(_pageSize[0]/4,_pageSize[1]/4,_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(2*_pageSize[0]/4,_pageSize[1]/4,2*_pageSize[0]/4,0);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,2*_pageSize[1]/4,_pageSize[0],2*_pageSize[1]/4);
        ogl::drawSingle2dLine_i(3*_pageSize[0]/4,3*_pageSize[1]/4,_pageSize[0],3*_pageSize[1]/4);
    }
}


#else

void displayContainerPage(CSPage* page,const int* position,const int* size)
{

}

void displayContainerPageOverlay(const int* position,const int* size,int activePageIndex,int focusObject)
{

}

void displayContainerPageWatermark(const int* position,const int* size,int tagId)
{

}

void displayPage(CSPage* page,int auxViewResizingAction,int viewIndexOfResizingAction)
{

}

#endif



