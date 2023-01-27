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

#include "viewRendering.h"

#ifdef SIM_WITH_OPENGL

void displayView(CSView* view,CSceneObject* it,int mainWindowXPos,bool clipWithMainWindowXPos,bool drawText,bool passiveSubView)
{
    int _viewPosition[2];
    int _viewSize[2];
    view->getViewPosition(_viewPosition);
    view->getViewSize(_viewSize);
    if (it!=nullptr)
    {
        glEnable(GL_SCISSOR_TEST);
        glViewport(_viewPosition[0],_viewPosition[1],_viewSize[0],_viewSize[1]);
        int xMax=_viewPosition[0];
        int xSize=_viewSize[0];
        if ( (xMax<mainWindowXPos)&&clipWithMainWindowXPos )
        {
            xSize=xSize-(mainWindowXPos-xMax);
            xMax=mainWindowXPos;
        }
        glScissor(xMax,_viewPosition[1],xSize,_viewSize[1]);
        if (xSize>0) // to avoid strange effects!
        {
#ifdef SIM_WITH_GUI
            if (it->getObjectType()==sim_object_camera_type)
                ((CCamera*)it)->lookIn(nullptr,view,drawText,passiveSubView);
            if (it->getObjectType()==sim_object_graph_type)
                ((CGraph*)it)->lookAt(nullptr,view,view->getTimeGraph(),drawText,passiveSubView,true);
#endif
            if (it->getObjectType()==sim_object_visionsensor_type)
                ((CVisionSensor*)it)->lookAt(view);
        }
        glDisable(GL_SCISSOR_TEST);
    }
    else
    { // We draw a white sub-view:
        glEnable(GL_SCISSOR_TEST);
        glViewport(_viewPosition[0],_viewPosition[1],_viewSize[0],_viewSize[1]);
        int xMax=_viewPosition[0];
        if ( (xMax<mainWindowXPos)&&clipWithMainWindowXPos )
            xMax=mainWindowXPos;
        glScissor(xMax,_viewPosition[1],_viewSize[0],_viewSize[1]);
        glClearColor(0.75,0.75,0.75,1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }
}


#else

void displayView(CSView* view,CSceneObject* it,int mainWindowXPos,bool clipWithMainWindowXPos,bool drawText,bool passiveSubView)
{

}

#endif



