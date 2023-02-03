#pragma once

#include <simTypes.h>
#include <vector>

struct SUndoBufferIntArray
{
    int identifier;
    std::vector<int> buffer;
    std::vector<int> undoBufferDependencyIds;
};

struct SUndoBufferFloatArray
{
    int identifier;
    std::vector<float> buffer;
    std::vector<int> undoBufferDependencyIds;
};

struct SUndoBufferUCharArray
{
    int identifier;
    std::vector<unsigned char> buffer;
    std::vector<int> undoBufferDependencyIds;
};

class CUndoBufferArrays
{
public:
    CUndoBufferArrays();
    virtual ~CUndoBufferArrays();

    int addVertexBuffer(const std::vector<float>& buff,int undoBufferId);
    int addIndexBuffer(const std::vector<int>& buff,int undoBufferId);
    int addNormalsBuffer(const std::vector<float>& buff,int undoBufferId);
    int addTextureBuffer(const std::vector<unsigned char>& buff,int undoBufferId);

    void getVertexBuffer(int id,std::vector<float>& buff);
    void getIndexBuffer(int id,std::vector<int>& buff);
    void getNormalsBuffer(int id,std::vector<float>& buff);
    void getTextureBuffer(int id,std::vector<unsigned char>& buff);

    void removeDependenciesFromUndoBufferId(int undoBufferId);
    void clearAll();
    int getMemorySizeInBytes();
private:

    bool _areFloatBuffersSame(const std::vector<float>& buff1,const std::vector<float>& buff2);
    bool _areIntBuffersSame(const std::vector<int>& buff1,const std::vector<int>& buff2);
    bool _areUCharBuffersSame(const std::vector<unsigned char>& buff1,const std::vector<unsigned char>& buff2);

    std::vector<SUndoBufferFloatArray> _vertexBuffers;
    std::vector<SUndoBufferIntArray> _indexBuffers;
    std::vector<SUndoBufferFloatArray> _normalsBuffers;
    std::vector<SUndoBufferUCharArray> _textureBuffers;
    int _nextId;
};
