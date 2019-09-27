
#include "broadcastDataVisualRendering.h"

#ifdef SIM_WITH_OPENGL

void displayEmitterOrReceiver(CBroadcastDataVisual* it)
{
    if (it->_emitter)
    {
        glPushMatrix();
        glTranslatef(it->_emitterConf.X(0),it->_emitterConf.X(1),it->_emitterConf.X(2));
        C4Vector axis=it->_emitterConf.Q.getAngleAndAxis();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
        float distances[11]={0.0f,0.001f*it->_actionRadius,0.00215f*it->_actionRadius,0.00462f*it->_actionRadius,0.01f*it->_actionRadius,0.0215f*it->_actionRadius,0.0462f*it->_actionRadius,0.1f*it->_actionRadius,0.215f*it->_actionRadius,0.462f*it->_actionRadius,it->_actionRadius};

        App::ct->environment->wirelessEmissionVolumeColor.makeCurrentColor(false);

        int vertSubdiv=int(it->_emissionAngle1*24.0f/piValue_f);
        if (vertSubdiv>12)
            vertSubdiv=12;
        int horizSubdiv=int(it->_emissionAngle2*48.0f/piValTimes2_f);
        if (horizSubdiv>24)
            horizSubdiv=24;
        for (int i=0;i<10;i++)
        {
            float d0=distances[i];
            float d1=distances[i+1];
            float b=-it->_emissionAngle2*0.5f;
            float dB=it->_emissionAngle2/float(horizSubdiv);
            for (int j=0;j<horizSubdiv;j++)
            {
                float a=-it->_emissionAngle1*0.5f;
                float dA=it->_emissionAngle1/float(vertSubdiv);
                ogl::buffer.clear();
                for (int k=0;k<vertSubdiv;k++)
                {
                    ogl::addBuffer3DPoints(d0*cos(a)*cos(b),d0*cos(a)*sin(b),d0*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a+dA)*cos(b),d1*cos(a+dA)*sin(b),d1*sin(a+dA));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                    if ( (j==horizSubdiv-1)&&(it->_emissionAngle2<piValTimes2_f*0.99f) )
                    { // Not 360 degrees. We have to close the edges:
                        ogl::addBuffer3DPoints(d0*cos(a)*cos(b+dB),d0*cos(a)*sin(b+dB),d0*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a+dA)*cos(b+dB),d1*cos(a+dA)*sin(b+dB),d1*sin(a+dA));
                    }
                    a+=dA;
                }
                if (it->_emissionAngle1<piValue_f*0.99f)
                { // Not 180 degrees. We have to close the edges:
                    ogl::addBuffer3DPoints(d0*cos(a)*cos(b),d0*cos(a)*sin(b),d0*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                    if ( (j==horizSubdiv-1)&&(it->_emissionAngle2<piValTimes2_f*0.99f) )
                    { // Not 360 degrees. We have to close one edge:
                        ogl::addBuffer3DPoints(d0*cos(a)*cos(b+dB),d0*cos(a)*sin(b+dB),d0*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                    }
                }
                ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
                ogl::buffer.clear();
                b+=dB;
            }
        }
        ogl::setBlending(false);
        glPopMatrix();
    }
    else
    {
        App::ct->environment->wirelessReceptionVolumeColor.makeCurrentColor(false);
        unsigned short stipple=255;
        glLineStipple(1,stipple);
        glLineWidth(4.0f);
        glEnable(GL_LINE_STIPPLE);
        ogl::drawSingle3dLine(it->_emitterConf.X.data,it->_receiverPos.data,nullptr);
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);
    }
}

#else

void displayEmitterOrReceiver(CBroadcastDataVisual* it)
{

}

#endif



