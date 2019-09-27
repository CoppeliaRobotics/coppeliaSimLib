
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "sceneSelector.h"
#include "sPage.h"
#include "oGL.h"
#include "oglSurface.h"
#include "app.h"
#include "tt.h"
#include "vVarious.h"
#include "rendering.h"

CSceneSelector::CSceneSelector()
{
    setUpDefaultValues();
}

CSceneSelector::~CSceneSelector()
{
    setUpDefaultValues();
}

void CSceneSelector::newSceneProcedure()
{
    setUpDefaultValues();
}

void CSceneSelector::setUpDefaultValues()
{
    viewSelectionSize[0]=1;
    viewSelectionSize[1]=1;
    viewIndex=-1;
    subViewIndex=-1;
    _caughtElements=0;
}

void CSceneSelector::markAsFirstRender()
{
    _firstRender=true;
}

int CSceneSelector::getCaughtElements()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(_caughtElements);
}

void CSceneSelector::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _caughtElements&=keepMask;
}

void CSceneSelector::setViewSizeAndPosition(int sizeX,int sizeY,int posX,int posY)
{
    viewSize[0]=sizeX;
    viewSize[1]=sizeY;
    viewPosition[0]=posX;
    viewPosition[1]=posY;
}

void CSceneSelector::render()
{
    int cnnt=1;
    if (_firstRender)
        cnnt=2;
    _firstRender=false;
    for (int bla=0;bla<cnnt;bla++)
    { // this loop because of a bug I can't put the finger on..
    glClearColor(0.5f,0.5f,0.5f,1);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    viewSelectionSize[0]=1;
    viewSelectionSize[1]=1;
    // Compute grid size
    int sceneCnt=App::ct->getInstanceCount();
    if (sceneCnt<2)
    {
        App::mainWindow->oglSurface->setSceneSelectionActive(false);
        return; // nothing to see here!!
    }

    float smallWinRatio=1.33f;
    while (viewSelectionSize[0]*viewSelectionSize[1]<sceneCnt)
    {
        int cx=viewSize[0]/(viewSelectionSize[0]+1);
        int cy=viewSize[1]/(viewSelectionSize[1]+1);
        if (cx>smallWinRatio*cy)
            viewSelectionSize[0]++;
        else
            viewSelectionSize[1]++;
    }
    float sp=0.1f;
    int ax=viewSize[0]/viewSelectionSize[0];
    int ay=viewSize[1]/viewSelectionSize[1];
    int tnd[2]={0,0};
    if (ax>smallWinRatio*ay)
    {
        tns[1]=(int)(viewSize[1]/((float)viewSelectionSize[1]+sp*((float)(viewSelectionSize[1]+1))));
        tns[0]=(int)(smallWinRatio*(float)tns[1]);
        tnd[1]=(int)(tns[1]*sp);
        tnd[0]=(viewSize[0]-viewSelectionSize[0]*tns[0])/(viewSelectionSize[0]+1);
    }
    else
    {
        tns[0]=(int)(viewSize[0]/((float)viewSelectionSize[0]+sp*((float)(viewSelectionSize[0]+1))));
        tns[1]=(int)(((float)tns[0])/smallWinRatio);
        tnd[0]=(int)(tns[0]*sp);
        tnd[1]=(viewSize[1]-viewSelectionSize[1]*tns[1])/(viewSelectionSize[1]+1);
    }
    int mouseOn=-1;
    if (_caughtElements&sim_left_button)
    {
        int bufferInd1=getSceneIndexInViewSelection(mouseDownRelativePosition);
        int bufferInd2=getSceneIndexInViewSelection(mouseRelativePosition);
        if (bufferInd1==bufferInd2)
            mouseOn=bufferInd1;
    }
    int mouseOver=getSceneIndexInViewSelection(mouseRelativePosition);
    std::vector<std::string> sceneNames;
    App::ct->getAllSceneNames(sceneNames);
    for (int k=0;k<viewSelectionSize[1];k++)
    {
        for (int l=0;l<viewSelectionSize[0];l++)
        {
            if ((l+viewSelectionSize[0]*k)<sceneCnt)
            {
                glEnable(GL_SCISSOR_TEST);
                int pIndex=l+viewSelectionSize[0]*k;
                if (mouseOver==l+viewSelectionSize[0]*k)
                {
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-5+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-5,tns[0]+10,tns[1]+10);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-5+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-5,tns[0]+10,tns[1]+10);
                    glClearColor(0.7f,0.7f,0.4f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-4+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-4,tns[0]+8,tns[1]+8);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-4+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-4,tns[0]+8,tns[1]+8);
                    glClearColor(0.9f,0.75f,0.2f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-3+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-3,tns[0]+6,tns[1]+6);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-3+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-3,tns[0]+6,tns[1]+6);
                    glClearColor(1.0f,0.8f,0.0f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-2+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-2,tns[0]+4,tns[1]+4);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-2+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-2,tns[0]+4,tns[1]+4);
                    glClearColor(1.0f,0.9f,0.0f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-1+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-1,tns[0]+2,tns[1]+2);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-1+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-1,tns[0]+2,tns[1]+2);
                    glClearColor(1.0f,1.0f,0.0f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                }
                else
                {
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-5+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-5,tns[0]+10,tns[1]+10);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-5+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-5,tns[0]+10,tns[1]+10);
                    glClearColor(0.5f,0.5f,0.5f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                    glViewport(tnd[0]+l*(tns[0]+tnd[0])-1+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-1,tns[0]+2,tns[1]+2);
                    glScissor(tnd[0]+l*(tns[0]+tnd[0])-1+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1])-1,tns[0]+2,tns[1]+2);
                    glClearColor(0.6f,0.6f,0.6f,1);
                    glClear (GL_COLOR_BUFFER_BIT);
                }

                glViewport(tnd[0]+l*(tns[0]+tnd[0])+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1]),tns[0],tns[1]);
                glScissor(tnd[0]+l*(tns[0]+tnd[0])+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1]),tns[0],tns[1]);
                glClearColor(0.3f,0.3f,0.3f,1);
                glClear (GL_COLOR_BUFFER_BIT);



                if (mouseOn!=l+viewSelectionSize[0]*k)
                {
                    if (pIndex<sceneCnt)
                    {
                        int res[2];
                        unsigned char* data=App::mainWindow->getSceneThumbnail(pIndex,res);
                        std::vector<unsigned char> dummyData;
                        float tlth[4]={0.0,0.0,1.0,1.0};
                        if (data==nullptr)
                        {
                            dummyData.resize(64*64*3,60);
                            data=&dummyData[0];
                            res[0]=64;
                            res[1]=64;
                        }
                        // Compute the correct texture coordinates:
                        float ratio1=float(tns[0])/float(tns[1]-ogl::getFontHeight());
                        float ratio2=float(res[0])/float(res[1]);
                        if (ratio2<ratio1)
                        {
                            float r=0.5f*(ratio2/ratio1);
                            tlth[0]=0.0;
                            tlth[1]=0.5f-r;
                            tlth[2]=1.0;
                            tlth[3]=0.5f+r;
                        }
                        else
                        {
                            float r=0.5f*(ratio1/ratio2);
                            tlth[0]=0.5f-r;
                            tlth[1]=0.0;
                            tlth[2]=0.5f+r;
                            tlth[3]=1.0;
                        }

                        CTextureObject myTexture=CTextureObject(res[0],res[1]);
                        myTexture.setImage(false,false,true,data);
                        _startTextureDisplay(&myTexture,true,1,false,false);
                        ogl::disableLighting_useWithCare(); // only temporarily
                        glBegin(GL_QUADS);
                        glTexCoord2f(tlth[0],tlth[1]);
                        glVertex3i(0,0,0);
                        glTexCoord2f(tlth[2],tlth[1]);
                        glVertex3i(tns[0],0,0);
                        glTexCoord2f(tlth[2],tlth[3]);
                        glVertex3i(tns[0],tns[1]-ogl::getFontHeight(),0);
                        glTexCoord2f(tlth[0],tlth[3]);
                        glVertex3i(0,tns[1]-ogl::getFontHeight(),0);
                        glEnd();
                        ogl::enableLighting_useWithCare();
                        _endTextureDisplay();
                    }
                }
                // The floating views, but also vision sensors (?) change the viewport and scissors I think, so we redo it here:
                glViewport(tnd[0]+l*(tns[0]+tnd[0])+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1]),tns[0],tns[1]);
                glScissor(tnd[0]+l*(tns[0]+tnd[0])+viewPosition[0],viewPosition[1]+viewSize[1]-(k+1)*(tns[1]+tnd[1]),tns[0],tns[1]);
                glEnable(GL_SCISSOR_TEST);

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0,tns[0],0,tns[1],-1,1);
                glMatrixMode (GL_MODELVIEW);
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);

                glLoadIdentity ();
                glDisable(GL_DEPTH_TEST);

                std::string txxt(sceneNames[pIndex]);
                if (txxt=="")
                    txxt="new scene";
                txxt+=tt::decorateString(" (scene ",tt::FNb(pIndex+1),")");
                ogl::setTextColor(0.9f,0.9f,0.9f);
                ogl::drawText(1,tns[1]-11*App::sc,0,txxt.c_str());
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_SCISSOR_TEST);
            }
        }
    }
    }
}

int CSceneSelector::getSceneIndexInViewSelection(int mousePos[2])
{ 
    int pos[2]={-1,-1};
    int space[2]={0,0};
    space[0]=(viewSize[0]-tns[0]*viewSelectionSize[0])/(viewSelectionSize[0]+1);
    space[1]=(viewSize[1]-tns[1]*viewSelectionSize[1])/(viewSelectionSize[1]+1);
    int mPos[2]={space[0],space[1]};
    int grid[2]={0,0};
    while (mousePos[0]>mPos[0])
    {
        if (mousePos[0]<mPos[0]+tns[0])
        {
            pos[0]=grid[0];
            break;
        }
        else
        {
            grid[0]++;
            mPos[0]=mPos[0]+tns[0]+space[0];
        }
    }
    while (viewSize[1]-mousePos[1]>mPos[1])
    {
        if (viewSize[1]-mousePos[1]<mPos[1]+tns[1])
        {
            pos[1]=grid[1];
            break;
        }
        else
        {
            grid[1]++;
            mPos[1]=mPos[1]+tns[1]+space[1];
        }
    }
    if ((pos[0]==-1)||(pos[1]==-1))
        return(-1);
    int pCnt=App::ct->getInstanceCount();
    if (pos[0]+viewSelectionSize[0]*pos[1]>=pCnt)
        return(-1);
    return(pos[0]+viewSelectionSize[0]*pos[1]);
}



bool CSceneSelector::leftMouseButtonDown(int x,int y,int selectionStatus)
{
    if ( (x<0)||(y<0)||(x>viewSize[0])||(y>viewSize[1]) )
        return(false);
    _caughtElements&=0xffff-sim_left_button;
    // The mouse went down in this window zone
    mouseDownRelativePosition[0]=x;
    mouseDownRelativePosition[1]=y;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    _caughtElements|=sim_left_button;
    return(true);
}
void CSceneSelector::leftMouseButtonUp(int x,int y)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    if (_caughtElements&sim_left_button)
    {
        int bufferInd1=getSceneIndexInViewSelection(mouseDownRelativePosition);
        int bufferInd2=getSceneIndexInViewSelection(mouseRelativePosition);
        if ( (bufferInd1==bufferInd2)&&(bufferInd1!=-1) )
        { // Mouse went down and up on the same item:
            App::mainWindow->oglSurface->setSceneSelectionActive(false);
            App::appendSimulationThreadCommand(SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD,bufferInd1);
        }
    }
}

int CSceneSelector::getSceneIndex(int x,int y)
{
    int pos[2]={x,y};
    int ind=getSceneIndexInViewSelection(pos);
    if (ind==-1)
        return(-1);
    return(ind);
}

int CSceneSelector::getCursor(int x,int y)
{
    if (getSceneIndex(x,y)!=-1)
        return(sim_cursor_finger);
    return(-1);
}

void CSceneSelector::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
}

bool CSceneSelector::rightMouseButtonDown(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if ( (x<0)||(y<0)||(x>viewSize[0])||(y>viewSize[1]) )
        return(false);
    _caughtElements&=0xffff-sim_right_button;
    return(false); // Not processed yet!
}
void CSceneSelector::rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow)
{
}
bool CSceneSelector::leftMouseButtonDoubleClick(int x,int y,int selectionStatus)
{
    return(false); // Not processed yet!
}

void CSceneSelector::keyPress(int key)
{
}
