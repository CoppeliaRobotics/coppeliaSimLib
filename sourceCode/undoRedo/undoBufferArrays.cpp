
#include "vrepMainHeader.h"
#include "undoBufferArrays.h"

CUndoBufferArrays::CUndoBufferArrays()
{
    _nextId=0;
}

CUndoBufferArrays::~CUndoBufferArrays()
{
}

bool CUndoBufferArrays::_areFloatBuffersSame(const std::vector<float>& buff1,const std::vector<float>& buff2)
{
    if (buff1.size()==buff2.size())
    {
        int l=(int)buff1.size();
        int l2=l;
        if (l>20)
            l2=20;
        for (int i=0;i<l2;i++)
        {
            if (buff1[i]!=buff2[i])
                return(false);
        }
        // ok, the first 20 elements are same. Chances that the rest is same are high
        bool different=false;
        for (int i=0;i<l;i++)
            different|=(buff1[i]!=buff2[i]);
        return(!different);
    }
    return(false);
}

bool CUndoBufferArrays::_areIntBuffersSame(const std::vector<int>& buff1,const std::vector<int>& buff2)
{
    if (buff1.size()==buff2.size())
    {
        int l=(int)buff1.size();
        int l2=l;
        if (l>20)
            l2=20;
        for (int i=0;i<l2;i++)
        {
            if (buff1[i]!=buff2[i])
                return(false);
        }
        // ok, the first 20 elements are same. Chances that the rest is same are high
        bool different=false;
        for (int i=0;i<l;i++)
            different|=(buff1[i]!=buff2[i]);
        return(!different);
    }
    return(false);
}

bool CUndoBufferArrays::_areUCharBuffersSame(const std::vector<unsigned char>& buff1,const std::vector<unsigned char>& buff2)
{
    if (buff1.size()==buff2.size())
    {
        int l=(int)buff1.size();
        int l2=l;
        if (l>20)
            l2=20;
        for (int i=0;i<l2;i++)
        {
            if (buff1[i]!=buff2[i])
                return(false);
        }
        // ok, the first 20 elements are same. Chances that the rest is same are high
        bool different=false;
        for (int i=0;i<l;i++)
            different|=(buff1[i]!=buff2[i]);
        return(!different);
    }
    return(false);
}

int CUndoBufferArrays::addVertexBuffer(const std::vector<float>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (int i=0;i<int(_vertexBuffers.size());i++)
    {
        if (_areFloatBuffersSame(buff,_vertexBuffers[i]._buffer))
        {
            _vertexBuffers[i]._undoBufferDependencyIds.push_back(undoBufferId);
            return(_vertexBuffers[i]._identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferFloatArray theBuff;
    theBuff._identifier=_nextId++;
    theBuff._undoBufferDependencyIds.push_back(undoBufferId);
    theBuff._buffer.assign(buff.begin(),buff.end());
    _vertexBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addIndexBuffer(const std::vector<int>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (int i=0;i<int(_indexBuffers.size());i++)
    {
        if (_areIntBuffersSame(buff,_indexBuffers[i]._buffer))
        {
            _indexBuffers[i]._undoBufferDependencyIds.push_back(undoBufferId);
            return(_indexBuffers[i]._identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferIntArray theBuff;
    theBuff._identifier=_nextId++;
    theBuff._undoBufferDependencyIds.push_back(undoBufferId);
    theBuff._buffer.assign(buff.begin(),buff.end());
    _indexBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addNormalsBuffer(const std::vector<float>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (int i=0;i<int(_normalsBuffers.size());i++)
    {
        if (_areFloatBuffersSame(buff,_normalsBuffers[i]._buffer))
        {
            _normalsBuffers[i]._undoBufferDependencyIds.push_back(undoBufferId);
            return(_normalsBuffers[i]._identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferFloatArray theBuff;
    theBuff._identifier=_nextId++;
    theBuff._undoBufferDependencyIds.push_back(undoBufferId);
    theBuff._buffer.assign(buff.begin(),buff.end());
    _normalsBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addTextureBuffer(const std::vector<unsigned char>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (int i=0;i<int(_textureBuffers.size());i++)
    {
        if (_areUCharBuffersSame(buff,_textureBuffers[i]._buffer))
        {
            _textureBuffers[i]._undoBufferDependencyIds.push_back(undoBufferId);
            return(_textureBuffers[i]._identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferUCharArray theBuff;
    theBuff._identifier=_nextId++;
    theBuff._undoBufferDependencyIds.push_back(undoBufferId);
    theBuff._buffer.assign(buff.begin(),buff.end());
    _textureBuffers.push_back(theBuff);
    return(_nextId-1);
}

void CUndoBufferArrays::getVertexBuffer(int id,std::vector<float>& buff)
{
    bool found=false;
    for (int i=0;i<int(_vertexBuffers.size());i++)
    {
        if (_vertexBuffers[i]._identifier==id)
        {
            buff.assign(_vertexBuffers[i]._buffer.begin(),_vertexBuffers[i]._buffer.end());
            found=true;
            break;
        }
    }
}

void CUndoBufferArrays::getIndexBuffer(int id,std::vector<int>& buff)
{
    for (int i=0;i<int(_indexBuffers.size());i++)
    {
        if (_indexBuffers[i]._identifier==id)
        {
            buff.assign(_indexBuffers[i]._buffer.begin(),_indexBuffers[i]._buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::getNormalsBuffer(int id,std::vector<float>& buff)
{
    for (int i=0;i<int(_normalsBuffers.size());i++)
    {
        if (_normalsBuffers[i]._identifier==id)
        {
            buff.assign(_normalsBuffers[i]._buffer.begin(),_normalsBuffers[i]._buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::getTextureBuffer(int id,std::vector<unsigned char>& buff)
{
    for (int i=0;i<int(_textureBuffers.size());i++)
    {
        if (_textureBuffers[i]._identifier==id)
        {
            buff.assign(_textureBuffers[i]._buffer.begin(),_textureBuffers[i]._buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::removeDependenciesFromUndoBufferId(int undoBufferId)
{
    // Vertices:
    for (int i=0;i<int(_vertexBuffers.size());i++)
    {
        for (int j=0;j<int(_vertexBuffers[i]._undoBufferDependencyIds.size());j++)
        {
            if (_vertexBuffers[i]._undoBufferDependencyIds[j]==undoBufferId)
            {
                _vertexBuffers[i]._undoBufferDependencyIds.erase(_vertexBuffers[i]._undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_vertexBuffers[i]._undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _vertexBuffers.erase(_vertexBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Indices:
    for (int i=0;i<int(_indexBuffers.size());i++)
    {
        for (int j=0;j<int(_indexBuffers[i]._undoBufferDependencyIds.size());j++)
        {
            if (_indexBuffers[i]._undoBufferDependencyIds[j]==undoBufferId)
            {
                _indexBuffers[i]._undoBufferDependencyIds.erase(_indexBuffers[i]._undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_indexBuffers[i]._undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _indexBuffers.erase(_indexBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Normals:
    for (int i=0;i<int(_normalsBuffers.size());i++)
    {
        for (int j=0;j<int(_normalsBuffers[i]._undoBufferDependencyIds.size());j++)
        {
            if (_normalsBuffers[i]._undoBufferDependencyIds[j]==undoBufferId)
            {
                _normalsBuffers[i]._undoBufferDependencyIds.erase(_normalsBuffers[i]._undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_normalsBuffers[i]._undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _normalsBuffers.erase(_normalsBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Textures:
    for (int i=0;i<int(_textureBuffers.size());i++)
    {
        for (int j=0;j<int(_textureBuffers[i]._undoBufferDependencyIds.size());j++)
        {
            if (_textureBuffers[i]._undoBufferDependencyIds[j]==undoBufferId)
            {
                _textureBuffers[i]._undoBufferDependencyIds.erase(_textureBuffers[i]._undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_textureBuffers[i]._undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _textureBuffers.erase(_textureBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
}

void CUndoBufferArrays::clearAll()
{
    for (int i=0;i<int(_vertexBuffers.size());i++)
    {
        _vertexBuffers[i]._buffer.clear();
        _vertexBuffers[i]._undoBufferDependencyIds.clear();
    }
    _vertexBuffers.clear(); 

    for (int i=0;i<int(_indexBuffers.size());i++)
    {
        _indexBuffers[i]._buffer.clear();
        _indexBuffers[i]._undoBufferDependencyIds.clear();
    }
    _indexBuffers.clear();  

    for (int i=0;i<int(_normalsBuffers.size());i++)
    {
        _normalsBuffers[i]._buffer.clear();
        _normalsBuffers[i]._undoBufferDependencyIds.clear();
    }
    _normalsBuffers.clear();    

    for (int i=0;i<int(_textureBuffers.size());i++)
    {
        _textureBuffers[i]._buffer.clear();
        _textureBuffers[i]._undoBufferDependencyIds.clear();
    }
    _textureBuffers.clear();    
}

int CUndoBufferArrays::getMemorySizeInBytes()
{
    int s=0;
    for (int i=0;i<int(_vertexBuffers.size());i++)
        s+=int(_vertexBuffers[i]._buffer.size()*4);

    for (int i=0;i<int(_indexBuffers.size());i++)
        s+=int(_indexBuffers[i]._buffer.size()*4);

    for (int i=0;i<int(_normalsBuffers.size());i++)
        s+=int(_normalsBuffers[i]._buffer.size()*4);

    for (int i=0;i<int(_textureBuffers.size());i++)
        s+=int(_textureBuffers[i]._buffer.size()*4);

    return(s);
}
