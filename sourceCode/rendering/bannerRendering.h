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

void displayBanner(int objId,int options,const float* bckColor,const C7Vector& tr,const char* label,const CColorObject& color,double height,const C4X4Matrix& cameraCTM,const int windowSize[2],double verticalViewSizeOrAngle,bool perspective);
