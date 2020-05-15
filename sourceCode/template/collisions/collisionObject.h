#pragma once

#include "_collisionObject_.h"

class CCollisionObject : public _CCollisionObject_
{
public:
    CCollisionObject(int entity1Handle,int entity2Handle);
    virtual ~CCollisionObject();

private:
    static int cnt;
};
