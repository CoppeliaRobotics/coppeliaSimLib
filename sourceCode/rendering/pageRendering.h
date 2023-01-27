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

#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_GUI
void _drawSeparationLines(int _pageType,const int* _pageSize);
#endif

void displayContainerPage(CSPage* page,const int* position,const int* size);
void displayContainerPageOverlay(const int* position,const int* size,int activePageIndex,int focusObject);
void displayContainerPageWatermark(const int* position,const int* size,int tagId);
void displayPage(CSPage* page,int auxViewResizingAction,int viewIndexOfResizingAction);

