#include "undoBufferArrays.h"

CUndoBufferArrays::CUndoBufferArrays()
{
    _nextId=0;
}

CUndoBufferArrays::~CUndoBufferArrays()
{
}

bool CUndoBufferArrays::_areFloatBuffersSame(const std::vector<floatFloat>& buff1,const std::vector<floatFloat>& buff2)
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
            different=different||(buff1[i]!=buff2[i]);
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
            different=different||(buff1[i]!=buff2[i]);
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
            different=different||(buff1[i]!=buff2[i]);
        return(!different);
    }
    return(false);
}

int CUndoBufferArrays::addVertexBuffer(const std::vector<floatFloat>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (size_t i=0;i<_vertexBuffers.size();i++)
    {
        if (_areFloatBuffersSame(buff,_vertexBuffers[i].buffer))
        {
            _vertexBuffers[i].undoBufferDependencyIds.push_back(undoBufferId);
            return(_vertexBuffers[i].identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferFloatArray theBuff;
    theBuff.identifier=_nextId++;
    theBuff.undoBufferDependencyIds.push_back(undoBufferId);
    theBuff.buffer.assign(buff.begin(),buff.end());
    _vertexBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addIndexBuffer(const std::vector<int>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (size_t i=0;i<_indexBuffers.size();i++)
    {
        if (_areIntBuffersSame(buff,_indexBuffers[i].buffer))
        {
            _indexBuffers[i].undoBufferDependencyIds.push_back(undoBufferId);
            return(_indexBuffers[i].identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferIntArray theBuff;
    theBuff.identifier=_nextId++;
    theBuff.undoBufferDependencyIds.push_back(undoBufferId);
    theBuff.buffer.assign(buff.begin(),buff.end());
    _indexBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addNormalsBuffer(const std::vector<floatFloat>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (size_t i=0;i<_normalsBuffers.size();i++)
    {
        if (_areFloatBuffersSame(buff,_normalsBuffers[i].buffer))
        {
            _normalsBuffers[i].undoBufferDependencyIds.push_back(undoBufferId);
            return(_normalsBuffers[i].identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferFloatArray theBuff;
    theBuff.identifier=_nextId++;
    theBuff.undoBufferDependencyIds.push_back(undoBufferId);
    theBuff.buffer.assign(buff.begin(),buff.end());
    _normalsBuffers.push_back(theBuff);
    return(_nextId-1);
}

int CUndoBufferArrays::addTextureBuffer(const std::vector<unsigned char>& buff,int undoBufferId)
{
    //1. search for a same buffer:
    for (size_t i=0;i<_textureBuffers.size();i++)
    {
        if (_areUCharBuffersSame(buff,_textureBuffers[i].buffer))
        {
            _textureBuffers[i].undoBufferDependencyIds.push_back(undoBufferId);
            return(_textureBuffers[i].identifier);
        }
    }
    //2. Create the buffer:
    SUndoBufferUCharArray theBuff;
    theBuff.identifier=_nextId++;
    theBuff.undoBufferDependencyIds.push_back(undoBufferId);
    theBuff.buffer.assign(buff.begin(),buff.end());
    _textureBuffers.push_back(theBuff);
    return(_nextId-1);
}

void CUndoBufferArrays::getVertexBuffer(int id,std::vector<floatFloat>& buff)
{
    bool found=false;
    for (size_t i=0;i<_vertexBuffers.size();i++)
    {
        if (_vertexBuffers[i].identifier==id)
        {
            buff.assign(_vertexBuffers[i].buffer.begin(),_vertexBuffers[i].buffer.end());
            found=true;
            break;
        }
    }
}

void CUndoBufferArrays::getIndexBuffer(int id,std::vector<int>& buff)
{
    for (size_t i=0;i<_indexBuffers.size();i++)
    {
        if (_indexBuffers[i].identifier==id)
        {
            buff.assign(_indexBuffers[i].buffer.begin(),_indexBuffers[i].buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::getNormalsBuffer(int id,std::vector<floatFloat>& buff)
{
    for (size_t i=0;i<_normalsBuffers.size();i++)
    {
        if (_normalsBuffers[i].identifier==id)
        {
            buff.assign(_normalsBuffers[i].buffer.begin(),_normalsBuffers[i].buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::getTextureBuffer(int id,std::vector<unsigned char>& buff)
{
    for (size_t i=0;i<_textureBuffers.size();i++)
    {
        if (_textureBuffers[i].identifier==id)
        {
            buff.assign(_textureBuffers[i].buffer.begin(),_textureBuffers[i].buffer.end());
            break;
        }
    }
}

void CUndoBufferArrays::removeDependenciesFromUndoBufferId(int undoBufferId)
{
    // Vertices:
    for (int i=0;i<int(_vertexBuffers.size());i++)
    {
        for (int j=0;j<int(_vertexBuffers[i].undoBufferDependencyIds.size());j++)
        {
            if (_vertexBuffers[i].undoBufferDependencyIds[j]==undoBufferId)
            {
                _vertexBuffers[i].undoBufferDependencyIds.erase(_vertexBuffers[i].undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_vertexBuffers[i].undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _vertexBuffers.erase(_vertexBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Indices:
    for (int i=0;i<int(_indexBuffers.size());i++)
    {
        for (int j=0;j<int(_indexBuffers[i].undoBufferDependencyIds.size());j++)
        {
            if (_indexBuffers[i].undoBufferDependencyIds[j]==undoBufferId)
            {
                _indexBuffers[i].undoBufferDependencyIds.erase(_indexBuffers[i].undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_indexBuffers[i].undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _indexBuffers.erase(_indexBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Normals:
    for (int i=0;i<int(_normalsBuffers.size());i++)
    {
        for (int j=0;j<int(_normalsBuffers[i].undoBufferDependencyIds.size());j++)
        {
            if (_normalsBuffers[i].undoBufferDependencyIds[j]==undoBufferId)
            {
                _normalsBuffers[i].undoBufferDependencyIds.erase(_normalsBuffers[i].undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_normalsBuffers[i].undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _normalsBuffers.erase(_normalsBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
    // Textures:
    for (int i=0;i<int(_textureBuffers.size());i++)
    {
        for (int j=0;j<int(_textureBuffers[i].undoBufferDependencyIds.size());j++)
        {
            if (_textureBuffers[i].undoBufferDependencyIds[j]==undoBufferId)
            {
                _textureBuffers[i].undoBufferDependencyIds.erase(_textureBuffers[i].undoBufferDependencyIds.begin()+j);
                j--; // we have to reprocess this position
            }
        }
        if (_textureBuffers[i].undoBufferDependencyIds.size()==0)
        { // we can remove this buffer!
            _textureBuffers.erase(_textureBuffers.begin()+i);
            i--; // we have to reprocess this position
        }
    }
}

void CUndoBufferArrays::clearAll()
{
    for (size_t i=0;i<_vertexBuffers.size();i++)
    {
        _vertexBuffers[i].buffer.clear();
        _vertexBuffers[i].undoBufferDependencyIds.clear();
    }
    _vertexBuffers.clear(); 

    for (size_t i=0;i<_indexBuffers.size();i++)
    {
        _indexBuffers[i].buffer.clear();
        _indexBuffers[i].undoBufferDependencyIds.clear();
    }
    _indexBuffers.clear();  

    for (size_t i=0;i<_normalsBuffers.size();i++)
    {
        _normalsBuffers[i].buffer.clear();
        _normalsBuffers[i].undoBufferDependencyIds.clear();
    }
    _normalsBuffers.clear();    

    for (size_t i=0;i<_textureBuffers.size();i++)
    {
        _textureBuffers[i].buffer.clear();
        _textureBuffers[i].undoBufferDependencyIds.clear();
    }
    _textureBuffers.clear();    
}

int CUndoBufferArrays::getMemorySizeInBytes()
{
    int s=0;
    for (size_t i=0;i<_vertexBuffers.size();i++)
        s+=int(_vertexBuffers[i].buffer.size()*4);

    for (size_t i=0;i<_indexBuffers.size();i++)
        s+=int(_indexBuffers[i].buffer.size()*4);

    for (size_t i=0;i<_normalsBuffers.size();i++)
        s+=int(_normalsBuffers[i].buffer.size()*4);

    for (size_t i=0;i<_textureBuffers.size();i++)
        s+=int(_textureBuffers[i].buffer.size()*4);
    return(s);
}
