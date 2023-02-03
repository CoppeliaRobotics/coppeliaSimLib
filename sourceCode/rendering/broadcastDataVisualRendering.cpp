/*
Source code based on the V-REP library source code from Coppelia
Robotics AG on September 2019

Copyright (C) 2006-2023 Coppelia Robotics AG
Copyright (C) 2019 Robot Nordic ApS

All rights reserved.

GNU GPL license:
================
The code in this file is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

See the GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*/

#include <broadcastDataVisualRendering.h>

#ifdef SIM_WITH_OPENGL

void displayEmitterOrReceiver(CBroadcastDataVisual* it)
{
    if (it->_emitter)
    {
        glPushMatrix();
        glTranslated(it->_emitterConf.X(0),it->_emitterConf.X(1),it->_emitterConf.X(2));
        C4Vector axis=it->_emitterConf.Q.getAngleAndAxis();
        glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
        double distances[11]={0.0,0.001*it->_actionRadius,0.00215*it->_actionRadius,0.00462*it->_actionRadius,0.01*it->_actionRadius,0.0215*it->_actionRadius,0.0462*it->_actionRadius,0.1*it->_actionRadius,0.215*it->_actionRadius,0.462*it->_actionRadius,it->_actionRadius};

        App::currentWorld->environment->wirelessEmissionVolumeColor.makeCurrentColor(false);

        int vertSubdiv=int(it->_emissionAngle1*24.0/piValue);
        if (vertSubdiv>12)
            vertSubdiv=12;
        int horizSubdiv=int(it->_emissionAngle2*48.0/piValT2);
        if (horizSubdiv>24)
            horizSubdiv=24;
        for (int i=0;i<10;i++)
        {
            double d0=distances[i];
            double d1=distances[i+1];
            double b=-it->_emissionAngle2*0.5;
            double dB=it->_emissionAngle2/double(horizSubdiv);
            for (int j=0;j<horizSubdiv;j++)
            {
                double a=-it->_emissionAngle1*0.5;
                double dA=it->_emissionAngle1/double(vertSubdiv);
                ogl::buffer.clear();
                for (int k=0;k<vertSubdiv;k++)
                {
                    ogl::addBuffer3DPoints(d0*cos(a)*cos(b),d0*cos(a)*sin(b),d0*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a+dA)*cos(b),d1*cos(a+dA)*sin(b),d1*sin(a+dA));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                    if ( (j==horizSubdiv-1)&&(it->_emissionAngle2<piValT2*0.99) )
                    { // Not 360 degrees. We have to close the edges:
                        ogl::addBuffer3DPoints(d0*cos(a)*cos(b+dB),d0*cos(a)*sin(b+dB),d0*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                        ogl::addBuffer3DPoints(d1*cos(a+dA)*cos(b+dB),d1*cos(a+dA)*sin(b+dB),d1*sin(a+dA));
                    }
                    a+=dA;
                }
                if (it->_emissionAngle1<piValue*0.99)
                { // Not 180 degrees. We have to close the edges:
                    ogl::addBuffer3DPoints(d0*cos(a)*cos(b),d0*cos(a)*sin(b),d0*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b),d1*cos(a)*sin(b),d1*sin(a));
                    ogl::addBuffer3DPoints(d1*cos(a)*cos(b+dB),d1*cos(a)*sin(b+dB),d1*sin(a));
                    if ( (j==horizSubdiv-1)&&(it->_emissionAngle2<piValT2*0.99) )
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
        App::currentWorld->environment->wirelessReceptionVolumeColor.makeCurrentColor(false);
        unsigned short stipple=255;
        glLineStipple(1,stipple);
        glLineWidth(4.0);
        glEnable(GL_LINE_STIPPLE);
        ogl::drawSingle3dLine(it->_emitterConf.X.data,it->_receiverPos.data,nullptr);
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0);
    }
}

#else

void displayEmitterOrReceiver(CBroadcastDataVisual* it)
{

}

#endif



