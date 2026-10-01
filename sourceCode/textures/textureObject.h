#pragma once

#include <obj.h>
#include <ser.h>

class CTextureObject : public Obj
{
  public:
    CTextureObject(int sizeX, int sizeY);
    virtual ~CTextureObject();

    void setObjectHandle(int64_t h);
    bool isSame(const CTextureObject* obj) const;
    void setObjectName(const char* newName);
    std::string getObjectName() const;
    void getTextureSize(int& sizeX, int& sizeY) const;
    void setImage(bool rgba, bool horizFlip, bool vertFlip, const unsigned char* data);
    CTextureObject* copyYourself() const;
    void serialize(CSer& ar);
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType);
    void setTextureBuffer(const std::vector<unsigned char>& tb);
    void getTextureBuffer(std::vector<unsigned char>& tb) const;
    const unsigned char* getTextureBufferPointer() const;
    void lightenUp();
    void setRandomContent();

    bool announceGeneralObjectWillBeErased(int64_t objectID, int64_t subObjectID);
    void addDependentObject(int64_t objectID, int64_t subObjectID);
    void clearAllDependencies();
    void transferDependenciesToThere(CTextureObject* receivingObject);

    unsigned char* readPortionOfTexture(int posX, int posY, int sizeX, int sizeY) const;
    bool writePortionOfTexture(const unsigned char* rgbData, int posX, int posY, int sizeX, int sizeY, bool circular,
                               double interpol);

    unsigned int getCurrentTextureContentUniqueId() const;

    void setOglTextureName(unsigned int n);
    unsigned int getOglTextureName() const;
    bool getChangedFlag() const;
    void setChangedFlag(bool c);

    int getBufferProperty(const char* pName, std::string& pState) const;
    int getIntArray2Property(const char* pName, int* pState) const;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

  protected:
    std::vector<unsigned char> _textureBuffer;
    unsigned int _oglTextureName;
    std::string _objectName;
    int _textureSize[2];
    bool _providedImageWasRGBA; // just needed to reduce serialization size!
    bool _changedFlag;
    unsigned int _currentTextureContentUniqueId;

    std::vector<int64_t> _dependentObjects;
    std::vector<int64_t> _dependentSubObjects;
    static unsigned int _textureContentUniqueId;
};
