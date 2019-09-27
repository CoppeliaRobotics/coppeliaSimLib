
#pragma once

#include "vrepMainHeader.h"
#include "ser.h"

class CTextureObject
{
public:
    CTextureObject(); // for serialization
    CTextureObject(int sizeX,int sizeY);
    virtual ~CTextureObject();

    void setObjectID(int newID);
    int getObjectID() const;
    bool isSame(const CTextureObject* obj) const;
    void setObjectName(const char* newName);
    std::string getObjectName() const;
    void getTextureSize(int& sizeX,int& sizeY) const;
    void setImage(bool rgba,bool horizFlip,bool vertFlip,const unsigned char* data);
    CTextureObject* copyYourself() const;
    void serialize(CSer& ar);
    void setTextureBuffer(const std::vector<unsigned char>& tb);
    void getTextureBuffer(std::vector<unsigned char>& tb) const;
    const unsigned char* getTextureBufferPointer() const;
    void lightenUp();
    void setRandomContent();

    bool announceGeneralObjectWillBeErased(int objectID,int subObjectID);
    void addDependentObject(int objectID,int subObjectID);
    void clearAllDependencies();
    void transferDependenciesToThere(CTextureObject* receivingObject);

    char* readPortionOfTexture(int posX,int posY,int sizeX,int sizeY) const;
    bool writePortionOfTexture(const unsigned char* rgbData,int posX,int posY,int sizeX,int sizeY,bool circular,float interpol);

    unsigned int getCurrentTextureContentUniqueId() const;

    void setOglTextureName(unsigned int n);
    unsigned int getOglTextureName() const;
    bool getChangedFlag() const;
    void setChangedFlag(bool c);

protected:
    std::vector<unsigned char> _textureBuffer;
    unsigned int _oglTextureName;
    int _objectID;
    std::string _objectName;
    int _textureSize[2];
    bool _providedImageWasRGBA;     // just needed to reduce serialization size!
    bool _changedFlag;
    unsigned int _currentTextureContentUniqueId;

    std::vector<int> _dependentObjects;
    std::vector<int> _dependentSubObjects;
    static unsigned int _textureContentUniqueId;
};
