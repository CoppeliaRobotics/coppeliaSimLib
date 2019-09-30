
#pragma once

#include <QGLBuffer>

struct SBuffwid
{
    QGLBuffer* buffer;
    std::vector<float>* data;
    int refCnt;
    int verticesCnt;
    int lastTimeUsedInMs;
    bool qglBufferInitialized;
};


class CGlBufferObjects
{
public:

    CGlBufferObjects();
    virtual ~CGlBufferObjects();

    void drawTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,const float* textureCoords,int* vertexBufferId,int* normalBufferId,int* texCoordBufferId);
    void drawColorCodedTriangles(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const float* normals,int* vertexBufferId,int* normalBufferId);
    bool drawEdges(const float* vertices,int verticesCnt,const int* indices,int indicesCnt,const unsigned char* edges,int* edgeBufferId);

    void removeVertexBuffer(int vertexBufferId);
    void removeNormalBuffer(int normalBufferId);
    void removeTexCoordBuffer(int texCoordBufferId);
    void removeEdgeBuffer(int edgeBufferId);

    void increaseVertexBufferRefCnt(int vertexBufferId);
    void increaseNormalBufferRefCnt(int normalBufferId);
    void increaseTexCoordBufferRefCnt(int texCoordBufferId);
    void increaseEdgeBufferRefCnt(int edgeBufferId);

protected:
    bool _checkIfBuffersAreSupported();
    void _deleteAllBuffers();
    void _deleteBuffersThatNeedDestruction();
    void _deleteBuffersNotUsedSinceAWhile(int currentTimeInMs,int maxTimeInMs);
    void _updateAllBufferLastTimeUsed(int currentTimeInMs);


    int _buildVertexBuffer(const float* individualVertices,int individualVerticesCnt);
    int _buildNormalBuffer(const float* normals,int normalsCnt);
    int _buildTexCoordBuffer(const float* texCoords,int texCoordsCnt);
    int _buildEdgeBuffer(const float* individualVertices,int individualVerticesCnt);

    SBuffwid* _bindVertexBuffer(int vertexBufferId,int& verticesCnt,int currentTimeInMs);
    SBuffwid* _bindNormalBuffer(int normalBufferId,int currentTimeInMs);
    SBuffwid* _bindTexCoordBuffer(int texCoordBufferId,int currentTimeInMs);
    SBuffwid* _bindEdgeBuffer(int edgeBufferId,int& verticesCnt,int currentTimeInMs);

    void _unbindVertexBuffer(int vertexBufferId);
    void _unbindNormalBuffer(int normalBufferId);
    void _unbindTexCoordBuffer(int texCoordBufferId);
    void _unbindEdgeBuffer(int edgeBufferId);

    void _removeVertexBuffer(int vertexBufferId);
    void _removeNormalBuffer(int normalBufferId);
    void _removeTexCoordBuffer(int texCoordBufferId);
    void _removeEdgeBuffer(int edgeBufferId);

    void _fromSharedToIndividualVertices(const float* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,std::vector<float>& individualVertices);
    void _fromSharedToIndividualEdges(const float* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,const unsigned char* edges,std::vector<float>& individualVertices);


    bool _buffersAreSupported;
    int _maxTimeInMsBeforeBufferRemoval;
    int _nextId;

    std::map<int,SBuffwid> _vertexBuffers;
    std::map<int,SBuffwid> _normalBuffers;
    std::map<int,SBuffwid> _texCoordBuffers;
    std::map<int,SBuffwid> _edgeBuffers;

    std::vector<int> _vertexBuffersToRemove;
    std::vector<int> _normalBuffersToRemove;
    std::vector<int> _texCoordBuffersToRemove;
    std::vector<int> _edgeBuffersToRemove;
};
