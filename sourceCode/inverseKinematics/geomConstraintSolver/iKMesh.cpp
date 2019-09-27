
#include "vrepMainHeader.h"
#include "iKMesh.h"
#include "gCSDefs.h"

CIKMesh::CIKMesh(C7Vector& localTransformation)
{
    objectType=IK_MESH_TYPE;
    transformation=localTransformation;
}

CIKMesh::~CIKMesh()
{
}
