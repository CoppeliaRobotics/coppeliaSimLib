
#pragma once

#include "iKObject.h"

class CIKMesh : public CIKObject  
{
public:
    CIKMesh(C7Vector& localTransformation);
    virtual ~CIKMesh();
};
