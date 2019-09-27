
#include "vrepMainHeader.h"
#include "oGL.h"
#include "glBufferObjects.h"
#include "vDateTime.h"
#include "app.h"

CGlBufferObjects::CGlBufferObjects()
{ // Can only be called by the GUI thread!
    _buffersAreSupported=false;
    _maxTimeInMsBeforeBufferRemoval=App::userSettings->vboPersistenceInMs;
    _nextId=0;
}

CGlBufferObjects::~CGlBufferObjects()
{ // Can only be called by the GUI thread!
    _deleteAllBuffers();
}

void CGlBufferObjects::_deleteAllBuffers()
{ // Can only be called by the GUI thread!
    for (std::map<int,SBuffwid>::iterator it=_vertexBuffers.begin();it!=_vertexBuffers.end();it++)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
    }
    _vertexBuffers.clear();

    for (std::map<int,SBuffwid>::iterator it=_normalBuffers.begin();it!=_normalBuffers.end();it++)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
    }
    _normalBuffers.clear();

    for (std::map<int,SBuffwid>::iterator it=_texCoordBuffers.begin();it!=_texCoordBuffers.end();it++)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
    }
    _texCoordBuffers.clear();

    for (std::map<int,SBuffwid>::iterator it=_edgeBuffers.begin();it!=_edgeBuffers.end();it++)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
    }
    _edgeBuffers.clear();

    _vertexBuffersToRemove.clear();
    _normalBuffersToRemove.clear();
    _texCoordBuffersToRemove.clear();
    _edgeBuffersToRemove.clear();
}

bool CGlBufferObjects::_checkIfBuffersAreSupported()
{ // Can only be called by the GUI thread!
    static int alreadyChecked=-1;
    if (alreadyChecked!=-1)
        return(alreadyChecked!=0);
    QGLBuffer vbuff(QGLBuffer::VertexBuffer);
    if (!vbuff.create())
    {
        alreadyChecked=0;
        return(false);
    }
    if (!vbuff.bind())
    {
        vbuff.destroy();
        alreadyChecked=0;
        return(false);
    }
    vbuff.destroy();
    alreadyChecked=1;
    return(true);
}

void CGlBufferObjects::drawTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,const float* textureCoords,int* vertexBufferId,int* normalBufferId,int* texCoordBufferId)
{   // textureCoords can be nullptr, in which case texCoordBufferId can also be nullptr
    // Can only be called by the GUI thread!
    _buffersAreSupported=_checkIfBuffersAreSupported();

    int currentTimeInMs=VDateTime::getTimeInMs();
    bool forceNotUsingBuffers=true;
#ifdef SIM_WITH_GUI
    forceNotUsingBuffers=(App::userSettings->vboOperation==0)||(App::mainWindow==nullptr); // in headless mode: we don't use VBO's for now (crash)
#endif

    static int lastTimeInMs=currentTimeInMs;
    static bool previousForceNotUsingBuffer=forceNotUsingBuffers;

    _deleteBuffersThatNeedDestruction();

    if (previousForceNotUsingBuffer!=forceNotUsingBuffers)
        _deleteAllBuffers();

    if (_maxTimeInMsBeforeBufferRemoval>0)
    {
        if (VDateTime::getTimeDiffInMs(lastTimeInMs,currentTimeInMs)>_maxTimeInMsBeforeBufferRemoval-1000) // we haven't rendered a mesh since a while. Modal dlg?
            _updateAllBufferLastTimeUsed(currentTimeInMs);
        else
            _deleteBuffersNotUsedSinceAWhile(currentTimeInMs,_maxTimeInMsBeforeBufferRemoval);
    }

    SBuffwid* theNormalBuff=_bindNormalBuffer(normalBufferId[0],currentTimeInMs);
    if (theNormalBuff==nullptr)
    {
        normalBufferId[0]=_buildNormalBuffer(normals,indicesCnt);
        theNormalBuff=_bindNormalBuffer(normalBufferId[0],currentTimeInMs);
    }

    SBuffwid* theTexCoordBuff=nullptr;
    if (textureCoords!=nullptr)
    {
        theTexCoordBuff=_bindTexCoordBuffer(texCoordBufferId[0],currentTimeInMs);
        if (theTexCoordBuff==nullptr)
        {
            texCoordBufferId[0]=_buildTexCoordBuffer(textureCoords,indicesCnt);
            theTexCoordBuff=_bindTexCoordBuffer(texCoordBufferId[0],currentTimeInMs);
        }
    }

    int individualVerticesCnt=0;
    SBuffwid* theVertexBuff=_bindVertexBuffer(vertexBufferId[0],individualVerticesCnt,currentTimeInMs);
    if (theVertexBuff==nullptr)
    {
        std::vector<float> individualVertices;
        _fromSharedToIndividualVertices(vertices,verticesCnt,indices,indicesCnt,individualVertices);

        vertexBufferId[0]=_buildVertexBuffer(&individualVertices[0],(int)individualVertices.size()/3);
        theVertexBuff=_bindVertexBuffer(vertexBufferId[0],individualVerticesCnt,currentTimeInMs);
    }

/*  if (_buffersAreSupported)
        printf("VBO supported\n");
    else
        printf("VBO NOT supported\n");
*/

    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        glDrawArrays(GL_TRIANGLES,0,individualVerticesCnt);
        //      glDrawElements(GL_TRIANGLES,individualVerticesCnt,GL_UNSIGNED_INT,0);
        _unbindVertexBuffer(vertexBufferId[0]);
        if (textureCoords!=nullptr)
            _unbindTexCoordBuffer(texCoordBufferId[0]);
        _unbindNormalBuffer(normalBufferId[0]);
    }
    else
    {
        if (textureCoords==nullptr)
        {
            glBegin(GL_TRIANGLES);
            for (int i=0;i<individualVerticesCnt;i++)
            {
                glNormal3fv(&theNormalBuff->data->at(3*i));
                glVertex3fv(&theVertexBuff->data->at(3*i));
            }
            glEnd();
        }
        else
        {
            glBegin(GL_TRIANGLES);
            for (int i=0;i<individualVerticesCnt;i++)
            {
                glNormal3fv(&theNormalBuff->data->at(3*i));
                glTexCoord2fv(&theTexCoordBuff->data->at(2*i));
                glVertex3fv(&theVertexBuff->data->at(3*i));
            }
            glEnd();
        }
    }
    lastTimeInMs=currentTimeInMs;
    previousForceNotUsingBuffer=forceNotUsingBuffers;

    // printf("%i %i %i %i\n",_vertexBuffers.size(),_normalBuffers.size(),_texCoordBuffers.size(),_edgeBuffers.size());
}

void CGlBufferObjects::drawColorCodedTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,int* vertexBufferId,int* normalBufferId)
{   // Can only be called by the GUI thread!
    _buffersAreSupported=_checkIfBuffersAreSupported();

    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack); // probably not needed
    glBegin(GL_TRIANGLES);
    for (int i=0;i<indicesCnt/3;i++)
    {
        glColor3ub(i&255,(i>>8)&255,(i>>16)&255);
        int ind[3]={indices[3*i+0],indices[3*i+1],indices[3*i+2]};
        const float* vert[3]={vertices+3*ind[0],vertices+3*ind[1],vertices+3*ind[2]};
        const float* norm[3]={normals+3*ind[0],normals+3*ind[1],normals+3*ind[2]};

        glNormal3fv(norm[0]);
        glVertex3fv(vert[0]);
        glNormal3fv(norm[1]);
        glVertex3fv(vert[1]);
        glNormal3fv(norm[2]);
        glVertex3fv(vert[2]);
    }
    glEnd();
}

bool CGlBufferObjects::drawEdges(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const unsigned char* edges,int* edgeBufferId)
{
    _buffersAreSupported=_checkIfBuffersAreSupported();
    // Can only be called by the GUI thread!
    int currentTimeInMs=VDateTime::getTimeInMs();
    bool forceNotUsingBuffers=true;
#ifdef SIM_WITH_GUI
    forceNotUsingBuffers=(App::userSettings->vboOperation==0)||(App::mainWindow==nullptr); // in headless mode: we don't use VBO's for now (crash)
#endif
    static int lastTimeInMs=currentTimeInMs;
    static bool previousForceNotUsingBuffer=forceNotUsingBuffers;

    _deleteBuffersThatNeedDestruction();

    if (previousForceNotUsingBuffer!=forceNotUsingBuffers)
        _deleteAllBuffers();

    if (_maxTimeInMsBeforeBufferRemoval>0)
    {
        if (VDateTime::getTimeDiffInMs(lastTimeInMs,currentTimeInMs)>_maxTimeInMsBeforeBufferRemoval-1000) // we haven't rendered a mesh since a while. Modal dlg?
            _updateAllBufferLastTimeUsed(currentTimeInMs);
        else
            _deleteBuffersNotUsedSinceAWhile(currentTimeInMs,_maxTimeInMsBeforeBufferRemoval);
    }

    int individualVerticesCnt=0;
    SBuffwid* theEdgeBuff=_bindEdgeBuffer(edgeBufferId[0],individualVerticesCnt,currentTimeInMs);
    if (theEdgeBuff==nullptr)
    {
        std::vector<float> individualVertices;
        _fromSharedToIndividualEdges(vertices,verticesCnt,indices,indicesCnt,edges,individualVertices);

        edgeBufferId[0]=_buildEdgeBuffer(&individualVertices[0],(int)individualVertices.size()/3);
        theEdgeBuff=_bindEdgeBuffer(edgeBufferId[0],individualVerticesCnt,currentTimeInMs);
    }

    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        glDrawArrays(GL_LINES,0,individualVerticesCnt);
        //      glDrawElements(GL_LINES,individualVerticesCnt,GL_UNSIGNED_INT,0);
        _unbindEdgeBuffer(edgeBufferId[0]);
    }
    else
    { // now without VBOs:
        glBegin(GL_LINES);
        for (int i=0;i<individualVerticesCnt;i++)
            glVertex3fv(&theEdgeBuff->data->at(3*i));
        glEnd();
    }

    lastTimeInMs=currentTimeInMs;
    previousForceNotUsingBuffer=forceNotUsingBuffers;
    return(individualVerticesCnt>0);
}

void CGlBufferObjects::_fromSharedToIndividualVertices(const float* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,std::vector<float>& individualVertices)
{
    individualVertices.clear();
    for (int i=0;i<sharedIndicesCnt;i++)
    {
        individualVertices.push_back(sharedVertices[3*sharedIndices[i]+0]);
        individualVertices.push_back(sharedVertices[3*sharedIndices[i]+1]);
        individualVertices.push_back(sharedVertices[3*sharedIndices[i]+2]);
    }
}

void CGlBufferObjects::_fromSharedToIndividualEdges(const float* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,const unsigned char* edges,std::vector<float>& individualVertices)
{
    individualVertices.clear();

    for (int i=0;i<sharedIndicesCnt/3;i++)
    { // for each triangle...
        int indOr[3]={3*sharedIndices[3*i+0],3*sharedIndices[3*i+1],3*sharedIndices[3*i+2]};
        const float* vertices0=&sharedVertices[indOr[0]];
        const float* vertices1=&sharedVertices[indOr[1]];
        const float* vertices2=&sharedVertices[indOr[2]];
        if ( ( (edges[(3*i+0)>>3]&(1<<((3*i+0)&7)))!=0) ) // -1 means the edge was disabled
        {
            individualVertices.push_back(vertices0[0]);
            individualVertices.push_back(vertices0[1]);
            individualVertices.push_back(vertices0[2]);
            individualVertices.push_back(vertices1[0]);
            individualVertices.push_back(vertices1[1]);
            individualVertices.push_back(vertices1[2]);
        }
        if ( ( (edges[(3*i+1)>>3]&(1<<((3*i+1)&7)))!=0) ) // -1 means the edge was disabled
        {
            individualVertices.push_back(vertices1[0]);
            individualVertices.push_back(vertices1[1]);
            individualVertices.push_back(vertices1[2]);
            individualVertices.push_back(vertices2[0]);
            individualVertices.push_back(vertices2[1]);
            individualVertices.push_back(vertices2[2]);
        }
        if ( ( (edges[(3*i+2)>>3]&(1<<((3*i+2)&7)))!=0) ) // -1 means the edge was disabled
        {
            individualVertices.push_back(vertices2[0]);
            individualVertices.push_back(vertices2[1]);
            individualVertices.push_back(vertices2[2]);
            individualVertices.push_back(vertices0[0]);
            individualVertices.push_back(vertices0[1]);
            individualVertices.push_back(vertices0[2]);
        }
    }
}

int CGlBufferObjects::_buildVertexBuffer(const float* individualVertices,int individualVerticesCnt)
{ // Can only be called by the GUI thread!
    // 1. Check if we don't yet have a similar object:
    for (std::map<int,SBuffwid>::iterator it=_vertexBuffers.begin();it!=_vertexBuffers.end();it++)
    {
        if (int(it->second.data->size())==3*individualVerticesCnt)
        {
            bool same=true;
            for (int i=3;i<3*individualVerticesCnt;i++)
                same=same&&(it->second.data->at(i)==individualVertices[i]);
            if (same)
            {
                it->second.refCnt++;
                return(it->first);
            }
        }
    }

    // 2. we didn't find a similar object. We build it:
    SBuffwid buff;
    buff.lastTimeUsedInMs=VDateTime::getTimeInMs();
    buff.refCnt=1;
    buff.data=new std::vector<float>;
    buff.data->assign(individualVertices,individualVertices+individualVerticesCnt*3);

    bool forceNotUsingBuffers=true;
#ifdef SIM_WITH_GUI
    forceNotUsingBuffers=(App::userSettings->vboOperation==0)||(App::mainWindow==nullptr); // in headless mode: we don't use VBO's for now (crash)
#endif
    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        buff.buffer=new QGLBuffer(QGLBuffer::VertexBuffer);
        buff.buffer->create();
        buff.buffer->bind();
        buff.buffer->setUsagePattern(QGLBuffer::StaticDraw);
        buff.buffer->allocate(individualVertices,3*individualVerticesCnt*sizeof(float));
        buff.qglBufferInitialized=true;
    }
    else
        buff.qglBufferInitialized=false;

    buff.verticesCnt=individualVerticesCnt;
    _vertexBuffers[_nextId]=buff;
    return(_nextId++);
}

int CGlBufferObjects::_buildNormalBuffer(const float* normals,int normalsCnt)
{ // Can only be called by the GUI thread!
    // 1. Check if we don't yet have a similar object:
    for (std::map<int,SBuffwid>::iterator it=_normalBuffers.begin();it!=_normalBuffers.end();it++)
    {
        if (int(it->second.data->size())==3*normalsCnt)
        {
            bool same=true;
            for (int i=3;i<3*normalsCnt;i++)
                same=same&&(it->second.data->at(i)==normals[i]);
            if (same)
            {
                it->second.refCnt++;
                return(it->first);
            }
        }
    }

    // 2. we didn't find a similar object. We build it:
    SBuffwid buff;
    buff.lastTimeUsedInMs=VDateTime::getTimeInMs();
    buff.refCnt=1;
    buff.data=new std::vector<float>;
    buff.data->assign(normals,normals+normalsCnt*3);

    bool forceNotUsingBuffers=true;
#ifdef SIM_WITH_GUI
    forceNotUsingBuffers=(App::userSettings->vboOperation==0)||(App::mainWindow==nullptr); // in headless mode: we don't use VBO's for now (crash)
#endif
    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        buff.buffer=new QGLBuffer(QGLBuffer::VertexBuffer);
        buff.buffer->create();
        buff.buffer->bind();
        buff.buffer->setUsagePattern(QGLBuffer::StaticDraw);
        buff.buffer->allocate(normals,3*normalsCnt*sizeof(float));
        buff.qglBufferInitialized=true;
    }
    else
        buff.qglBufferInitialized=false;

    _normalBuffers[_nextId]=buff;
    return(_nextId++);
}

int CGlBufferObjects::_buildTexCoordBuffer(const float* texCoords,int texCoordsCnt)
{ // Can only be called by the GUI thread!
    // 1. Check if we don't yet have a similar object:
    for (std::map<int,SBuffwid>::iterator it=_texCoordBuffers.begin();it!=_texCoordBuffers.end();it++)
    {
        if (int(it->second.data->size())==2*texCoordsCnt)
        {
            bool same=true;
            for (int i=2;i<2*texCoordsCnt;i++)
                same=same&&(it->second.data->at(i)==texCoords[i]);
            if (same)
            {
                it->second.refCnt++;
                return(it->first);
            }
        }
    }

    // 2. we didn't find a similar object. We build it:
    SBuffwid buff;
    buff.lastTimeUsedInMs=VDateTime::getTimeInMs();
    buff.refCnt=1;
    buff.data=new std::vector<float>;
    buff.data->assign(texCoords,texCoords+texCoordsCnt*2);

    bool forceNotUsingBuffers=(App::userSettings->vboOperation==0);
    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        buff.buffer=new QGLBuffer(QGLBuffer::VertexBuffer);
        buff.buffer->create();
        buff.buffer->bind();
        buff.buffer->setUsagePattern(QGLBuffer::StaticDraw);
        buff.buffer->allocate(texCoords,2*texCoordsCnt*sizeof(float));
        buff.qglBufferInitialized=true;
    }
    else
        buff.qglBufferInitialized=false;

    _texCoordBuffers[_nextId]=buff;
    return(_nextId++);
}

int CGlBufferObjects::_buildEdgeBuffer(const float* individualVertices,int individualVerticesCnt)
{ // Can only be called by the GUI thread!
    // 1. Check if we don't yet have a similar object:
    for (std::map<int,SBuffwid>::iterator it=_edgeBuffers.begin();it!=_edgeBuffers.end();it++)
    {
        if (int(it->second.data->size())==3*individualVerticesCnt)
        {
            bool same=true;
            for (int i=0;i<3*individualVerticesCnt;i++)
                same=same&&(it->second.data->at(i)==individualVertices[i]);
            if (same)
            {
                it->second.refCnt++;
                return(it->first);
            }
        }
    }

    // 2. we didn't find a similar object. We build it:
    SBuffwid buff;
    buff.lastTimeUsedInMs=VDateTime::getTimeInMs();
    buff.refCnt=1;
    buff.data=new std::vector<float>;
    buff.data->assign(individualVertices,individualVertices+individualVerticesCnt*3);

    bool forceNotUsingBuffers=(App::userSettings->vboOperation==0);
    if (_buffersAreSupported&&(!forceNotUsingBuffers))
    {
        buff.buffer=new QGLBuffer(QGLBuffer::VertexBuffer);
        buff.buffer->create();
        buff.buffer->bind();
        buff.buffer->setUsagePattern(QGLBuffer::StaticDraw);
        buff.buffer->allocate(individualVertices,3*individualVerticesCnt*sizeof(float));
        buff.qglBufferInitialized=true;
    }
    else
        buff.qglBufferInitialized=false;

    buff.verticesCnt=individualVerticesCnt;
    _edgeBuffers[_nextId]=buff;
    return(_nextId++);
}

void CGlBufferObjects::_unbindVertexBuffer(int vertexBufferId)
{ // Can only be called by the GUI thread!
    std::map<int,SBuffwid>::iterator it=_vertexBuffers.find(vertexBufferId);
    if ((it!=_vertexBuffers.end())&&it->second.qglBufferInitialized)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        it->second.buffer->release();
    }
}

void CGlBufferObjects::_unbindNormalBuffer(int normalBufferId)
{ // Can only be called by the GUI thread!
    std::map<int,SBuffwid>::iterator it=_normalBuffers.find(normalBufferId);
    if ((it!=_normalBuffers.end())&&it->second.qglBufferInitialized)
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        it->second.buffer->release();
    }
}

void CGlBufferObjects::_unbindTexCoordBuffer(int texCoordBufferId)
{ // Can only be called by the GUI thread!
    std::map<int,SBuffwid>::iterator it=_texCoordBuffers.find(texCoordBufferId);
    if ((it!=_texCoordBuffers.end())&&it->second.qglBufferInitialized)
    {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        it->second.buffer->release();
    }
}

void CGlBufferObjects::_unbindEdgeBuffer(int edgeBufferId)
{ // Can only be called by the GUI thread!
    std::map<int,SBuffwid>::iterator it=_edgeBuffers.find(edgeBufferId);
    if ((it!=_edgeBuffers.end())&&it->second.qglBufferInitialized)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        it->second.buffer->release();
    }
}

SBuffwid* CGlBufferObjects::_bindVertexBuffer(int vertexBufferId,int& verticesCnt,int currentTimeInMs)
{ // Can only be called by the GUI thread!
    if (vertexBufferId==-1)
        return(nullptr);
    std::map<int,SBuffwid>::iterator it=_vertexBuffers.find(vertexBufferId);
    if (it==_vertexBuffers.end())
        return(nullptr);

    if (it->second.qglBufferInitialized)
    {
        it->second.buffer->bind();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3,GL_FLOAT,0,0);
    }

    it->second.lastTimeUsedInMs=currentTimeInMs;
    verticesCnt=it->second.verticesCnt;
    return(&it->second);
}

SBuffwid* CGlBufferObjects::_bindNormalBuffer(int normalBufferId,int currentTimeInMs)
{ // Can only be called by the GUI thread!
    if (normalBufferId==-1)
        return(nullptr);
    std::map<int,SBuffwid>::iterator it=_normalBuffers.find(normalBufferId);
    if (it==_normalBuffers.end())
        return(nullptr);

    if (it->second.qglBufferInitialized)
    {
        it->second.buffer->bind();
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT,0,0);
    }

    it->second.lastTimeUsedInMs=currentTimeInMs;
    return(&it->second);
}

SBuffwid* CGlBufferObjects::_bindTexCoordBuffer(int texCoordBufferId,int currentTimeInMs)
{ // Can only be called by the GUI thread!
    if (texCoordBufferId==-1)
        return(nullptr);
    std::map<int,SBuffwid>::iterator it=_texCoordBuffers.find(texCoordBufferId);
    if (it==_texCoordBuffers.end())
        return(nullptr);

    if (it->second.qglBufferInitialized)
    {
        it->second.buffer->bind();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2,GL_FLOAT,0,0);
    }

    it->second.lastTimeUsedInMs=currentTimeInMs;
    return(&it->second);
}

SBuffwid* CGlBufferObjects::_bindEdgeBuffer(int edgeBufferId,int& verticesCnt,int currentTimeInMs)
{ // Can only be called by the GUI thread!
    if (edgeBufferId==-1)
        return(nullptr);
    std::map<int,SBuffwid>::iterator it=_edgeBuffers.find(edgeBufferId);
    if (it==_edgeBuffers.end())
        return(nullptr);

    if (it->second.qglBufferInitialized)
    {
        it->second.buffer->bind();
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3,GL_FLOAT,0,0);
    }

    it->second.lastTimeUsedInMs=currentTimeInMs;
    verticesCnt=it->second.verticesCnt;

    return(&it->second);
}

void CGlBufferObjects::_deleteBuffersThatNeedDestruction()
{ // should only be called by the GUI thread!!
    for (size_t i=0;i<_vertexBuffersToRemove.size();i++)
        _removeVertexBuffer(_vertexBuffersToRemove[i]);
    _vertexBuffersToRemove.clear();

    for (size_t i=0;i<_normalBuffersToRemove.size();i++)
        _removeNormalBuffer(_normalBuffersToRemove[i]);
    _normalBuffersToRemove.clear();

    for (size_t i=0;i<_texCoordBuffersToRemove.size();i++)
        _removeTexCoordBuffer(_texCoordBuffersToRemove[i]);
    _texCoordBuffersToRemove.clear();

    for (size_t i=0;i<_edgeBuffersToRemove.size();i++)
        _removeEdgeBuffer(_edgeBuffersToRemove[i]);
    _edgeBuffersToRemove.clear();
}

void CGlBufferObjects::removeVertexBuffer(int vertexBufferId)
{ // can be called by any thread!
    if (vertexBufferId<0)
        return;
    _vertexBuffersToRemove.push_back(vertexBufferId);
}

void CGlBufferObjects::_removeVertexBuffer(int vertexBufferId)
{ // should only be called by the GUI thread!!
    std::map<int,SBuffwid>::iterator it=_vertexBuffers.find(vertexBufferId);
    if (it==_vertexBuffers.end())
        return;
    it->second.refCnt--;
    if (it->second.refCnt<=0)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
        _vertexBuffers.erase(it);
    }
}

void CGlBufferObjects::removeNormalBuffer(int normalBufferId)
{ // can be called by any thread!
    if (normalBufferId<0)
        return;
    _normalBuffersToRemove.push_back(normalBufferId);
}

void CGlBufferObjects::_removeNormalBuffer(int normalBufferId)
{ // should only be called by the GUI thread!!
    std::map<int,SBuffwid>::iterator it=_normalBuffers.find(normalBufferId);
    if (it==_normalBuffers.end())
        return;
    it->second.refCnt--;
    if (it->second.refCnt<=0)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
        _normalBuffers.erase(it);
    }
}


void CGlBufferObjects::removeTexCoordBuffer(int texCoordBufferId)
{ // can be called by any thread!
    if (texCoordBufferId<0)
        return;
    _texCoordBuffersToRemove.push_back(texCoordBufferId);
}

void CGlBufferObjects::_removeTexCoordBuffer(int texCoordBufferId)
{ // should only be called by the GUI thread!!
    std::map<int,SBuffwid>::iterator it=_texCoordBuffers.find(texCoordBufferId);
    if (it==_texCoordBuffers.end())
        return;
    it->second.refCnt--;
    if (it->second.refCnt<=0)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
        _texCoordBuffers.erase(it);
    }
}

void CGlBufferObjects::removeEdgeBuffer(int edgeBufferId)
{ // can be called by any thread!
    if (edgeBufferId<0)
        return;
    _edgeBuffersToRemove.push_back(edgeBufferId);
}

void CGlBufferObjects::_removeEdgeBuffer(int edgeBufferId)
{ // should only be called by the GUI thread!!
    std::map<int,SBuffwid>::iterator it=_edgeBuffers.find(edgeBufferId);
    if (it==_edgeBuffers.end())
        return;
    it->second.refCnt--;
    if (it->second.refCnt<=0)
    {
        if (it->second.qglBufferInitialized)
            delete it->second.buffer;
        delete it->second.data;
        _edgeBuffers.erase(it);
    }
}

void CGlBufferObjects::_deleteBuffersNotUsedSinceAWhile(int currentTimeInMs,int maxTimeInMs)
{ // call only from the GUI thread!
    std::vector<int> toRemove;
    for (std::map<int,SBuffwid>::iterator it=_vertexBuffers.begin();it!=_vertexBuffers.end();it++)
    {
        if (VDateTime::getTimeDiffInMs(it->second.lastTimeUsedInMs,currentTimeInMs)>maxTimeInMs)
            toRemove.push_back(it->first);
    }
    for (int i=0;i<int(toRemove.size());i++)
        _removeVertexBuffer(toRemove[i]);

    toRemove.clear();
    for (std::map<int,SBuffwid>::iterator it=_normalBuffers.begin();it!=_normalBuffers.end();it++)
    {
        if (VDateTime::getTimeDiffInMs(it->second.lastTimeUsedInMs,currentTimeInMs)>maxTimeInMs)
            toRemove.push_back(it->first);
    }
    for (int i=0;i<int(toRemove.size());i++)
        _removeNormalBuffer(toRemove[i]);

    toRemove.clear();
    for (std::map<int,SBuffwid>::iterator it=_texCoordBuffers.begin();it!=_texCoordBuffers.end();it++)
    {
        if (VDateTime::getTimeDiffInMs(it->second.lastTimeUsedInMs,currentTimeInMs)>maxTimeInMs)
            toRemove.push_back(it->first);
    }
    for (int i=0;i<int(toRemove.size());i++)
        _removeTexCoordBuffer(toRemove[i]);

    toRemove.clear();
    for (std::map<int,SBuffwid>::iterator it=_edgeBuffers.begin();it!=_edgeBuffers.end();it++)
    {
        if (VDateTime::getTimeDiffInMs(it->second.lastTimeUsedInMs,currentTimeInMs)>maxTimeInMs)
            toRemove.push_back(it->first);
    }
    for (int i=0;i<int(toRemove.size());i++)
        _removeEdgeBuffer(toRemove[i]);
}

void CGlBufferObjects::_updateAllBufferLastTimeUsed(int currentTimeInMs)
{
    for (std::map<int,SBuffwid>::iterator it=_vertexBuffers.begin();it!=_vertexBuffers.end();it++)
        it->second.lastTimeUsedInMs=currentTimeInMs;

    for (std::map<int,SBuffwid>::iterator it=_normalBuffers.begin();it!=_normalBuffers.end();it++)
        it->second.lastTimeUsedInMs=currentTimeInMs;

    for (std::map<int,SBuffwid>::iterator it=_texCoordBuffers.begin();it!=_texCoordBuffers.end();it++)
        it->second.lastTimeUsedInMs=currentTimeInMs;

    for (std::map<int,SBuffwid>::iterator it=_edgeBuffers.begin();it!=_edgeBuffers.end();it++)
        it->second.lastTimeUsedInMs=currentTimeInMs;
}

void CGlBufferObjects::increaseVertexBufferRefCnt(int vertexBufferId)
{
    std::map<int,SBuffwid>::iterator it=_vertexBuffers.find(vertexBufferId);
    if (it!=_vertexBuffers.end())
        it->second.refCnt++;
}

void CGlBufferObjects::increaseNormalBufferRefCnt(int normalBufferId)
{
    std::map<int,SBuffwid>::iterator it=_normalBuffers.find(normalBufferId);
    if (it!=_normalBuffers.end())
        it->second.refCnt++;
}

void CGlBufferObjects::increaseTexCoordBufferRefCnt(int texCoordBufferId)
{
    std::map<int,SBuffwid>::iterator it=_texCoordBuffers.find(texCoordBufferId);
    if (it!=_texCoordBuffers.end())
        it->second.refCnt++;
}

void CGlBufferObjects::increaseEdgeBufferRefCnt(int edgeBufferId)
{
    std::map<int,SBuffwid>::iterator it=_edgeBuffers.find(edgeBufferId);
    if (it!=_edgeBuffers.end())
        it->second.refCnt++;
}
