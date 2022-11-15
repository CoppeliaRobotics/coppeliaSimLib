
#pragma once

#include <QGLBuffer>

struct SBuffwid
{
    QGLBuffer* buffer;
    std::vector<floatFloat>* data;
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

    void drawTriangles(const floatFloat* vertices,int verticesCnt,const int* indices,int indicesCnt,const floatFloat* normals,const floatFloat* textureCoords,int* vertexBufferId,int* normalBufferId,int* texCoordBufferId);
    void drawColorCodedTriangles(const floatFloat* vertices,int verticesCnt,const int* indices,int indicesCnt,const floatFloat* normals,int* vertexBufferId,int* normalBufferId);
    bool drawEdges(const floatFloat* vertices,int verticesCnt,const int* indices,int indicesCnt,const unsigned char* edges,int* edgeBufferId);

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


    int _buildVertexBuffer(const floatFloat* individualVertices,int individualVerticesCnt);
    int _buildNormalBuffer(const floatFloat* normals,int normalsCnt);
    int _buildTexCoordBuffer(const floatFloat* texCoords,int texCoordsCnt);
    int _buildEdgeBuffer(const floatFloat* individualVertices,int individualVerticesCnt);

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

    void _fromSharedToIndividualVertices(const floatFloat* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,std::vector<floatFloat>& individualVertices);
    void _fromSharedToIndividualEdges(const floatFloat* sharedVertices,int sharedVerticesCnt,const int* sharedIndices,int sharedIndicesCnt,const unsigned char* edges,std::vector<floatFloat>& individualVertices);


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
