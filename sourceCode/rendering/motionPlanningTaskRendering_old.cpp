
#include "motionPlanningTaskRendering_old.h"

#ifdef SIM_WITH_OPENGL
#include "pathPlanningInterface.h"

void displayMotionPlanningTask(CMotionPlanningTask* task)
{
    C3DObject* basef=task->getBaseObject();
    if ((task->_data!=nullptr)&&(basef!=nullptr))
    {
        float* p;
        int ind=0;
        C7Vector tr(basef->getCumulativeTransformationPart1());
        glPushMatrix();
        glTranslatef(tr.X(0),tr.X(1),tr.X(2));
        C4Vector axis=tr.Q.getAngleAndAxis();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
        glLoadName(-1);

        ogl::buffer.clear();

        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        if (task->_showPhase1Nodes)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorGreen);
            int r;
            while (true)
            {
                r=CPathPlanningInterface::getMpPhase1NodesRenderData(task->_data,ind++,&p);
                if (r==-1)
                    break;
                if (r>0)
                {
                    ogl::buffer.push_back(p[0]);
                    ogl::buffer.push_back(p[1]);
                    ogl::buffer.push_back(p[2]);
                }
            }
            if (ogl::buffer.size()!=0)
                ogl::drawRandom3dPoints(&ogl::buffer[0],int(ogl::buffer.size())/3,nullptr);
            ogl::buffer.clear();
/* KEEP!!
            ind=0;
            float* p2;
            ogl::buffer.clear();
            while (false)
            { // edges visualization
                int r=task->_data->getPhase1NodesRenderData(ind,&p);
                if (r==-1)
                    break;
                if (r==1)
                {
                    for (int i=0;i<6;i++)
                    {
                        if (task->_data->getPhase1ConnectionData(ind,i,&p2)==1)
                        {
                            ogl::addBuffer3DPoints(p);
                            ogl::addBuffer3DPoints(p2);
                        }
                    }
                }
                ind++;
            }
            if (ogl::buffer.size()!=0)
                ogl::drawRandom3dLines(&ogl::buffer[0],ogl::buffer.size()/3,false,nullptr);
            ogl::buffer.clear();
*/
        }

        if (task->_showPhase2FromStartConnections)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,1.0f,0.5f,0.0f);
            _renderPhase2data(task,0);
        }

        if (task->_showPhase2FromGoalConnections)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,0.0f,0.5f,1.0f);
            _renderPhase2data(task,1);
        }

        if (task->_showPhase2Path)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
            glLineWidth(3.0f);
            _renderPhase2data(task,2);
            glLineWidth(1.0f);
        }

        glPopMatrix();
    }
}

void _renderPhase2data(CMotionPlanningTask* task,unsigned int what)
{
    int ind=0;
    float* p1;
    float* p2;
    ogl::buffer.clear();
    while (CPathPlanningInterface::getMpPhase2NodesRenderData(task->_data,what,ind++,&p1,&p2)!=-1)
    {
        ogl::buffer.push_back(p1[0]);
        ogl::buffer.push_back(p1[1]);
        ogl::buffer.push_back(p1[2]);
        ogl::buffer.push_back(p2[0]);
        ogl::buffer.push_back(p2[1]);
        ogl::buffer.push_back(p2[2]);
    }
    if (ogl::buffer.size()!=0)
        ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
    ogl::buffer.clear();
}

#else

void displayMotionPlanningTask(CMotionPlanningTask* task)
{

}

#endif



