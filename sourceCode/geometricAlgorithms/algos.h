#pragma once

#include <vector>
#include "7Vector.h"
#include "4X4Matrix.h"

//FULLY STATIC CLASS
class CAlgos  
{
public:
    static C7Vector getMeshBoundingBoxPose(const std::vector<double>& vert,const std::vector<int>& ind,bool alignedWithMainAxis);
    static C4X4Matrix getMainAxis(const std::vector<double>& vertices,const std::vector<int>& indices);
    static bool isBoxOutsideVolumeApprox(const C4X4Matrix& tr,const C3Vector& s,const std::vector<double>* planes);
};
