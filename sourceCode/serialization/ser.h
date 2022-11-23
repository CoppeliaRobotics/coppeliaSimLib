#pragma once

#include "vArchive.h"
#include "tinyxml2.h"
#include <string>
#include <vector>
#include "simTypes.h"

#define SER_SIM_HEADER "VREP" // The file header since 03/07/2009
#define SER_END_OF_OBJECT "EOO"
#define SER_NEXT_STEP "NXT"
#define SER_END_OF_FILE "EOF"
typedef sim::tinyxml2::XMLElement xmlNode;

class CSerTmp
{
public:
#ifdef DOUBLESERIALIZATIONOPERATION
    CSerTmp& operator<< (const floatFloat& v)
    {
        buffer.push_back(((unsigned char*)&v)[0]);
        buffer.push_back(((unsigned char*)&v)[1]);
        buffer.push_back(((unsigned char*)&v)[2]);
        buffer.push_back(((unsigned char*)&v)[3]);
        return(*this);
    }
    CSerTmp& operator<< (const double& v)
    {
        unsigned char* tmp=(unsigned char*)(&v);
        for (int i=0;i<int(sizeof(v));i++)
            buffer.push_back(tmp[i]);
        return(*this);
    }

    CSerTmp& operator>> (floatFloat& v)
    {
        unsigned char* tmp=(unsigned char*)(&v);
        for (int i=0;i<int(sizeof(v));i++)
            tmp[i]=_fileBuffer[_fileBufferReadPointer++];
        return(*this);
    }
    CSerTmp& operator>> (double& v)
    {
        unsigned char* tmp=(unsigned char*)(&v);
        for (int i=0;i<int(sizeof(v));i++)
            tmp[i]=_fileBuffer[_fileBufferReadPointer++];
        return(*this);
    }

    std::vector<unsigned char> buffer;
    std::vector<unsigned char> _fileBuffer;
    int _fileBufferReadPointer;
#endif
};

class CSer : public CSerTmp
{
public:

    enum {
        filetype_unspecified_file=0,        // ?
        filetype_csim_bin_scene_file,       // .ttt
        filetype_csim_bin_model_file,       // .ttm
        filetype_csim_bin_thumbnails_file,  // .ttmt
        reserved_was_brs,
        reserved_was_brm,
        filetype_csim_bin_scene_buff,       // ---
        filetype_csim_bin_model_buff,       // ---
        filetype_csim_bin_ui_file,          // .ttb
        filetype_bin_file,                  // .bin
        filetype_bin_buff,                  // ---
        filetype_csim_xml_xscene_file,       // .simscene.xml
        filetype_csim_xml_xmodel_file,       // .simmodel.xml
        filetype_csim_xml_simplescene_file,  // .simscene.xml
        filetype_csim_xml_simplemodel_file,  // .simmodel.xml
    };

    CSer(const char* filename,char filetype); // saving to/restoring from file
    CSer(std::vector<char>& bufferArchive,char filetype); // saving to/restoring from buffer

    virtual ~CSer();

    bool writeOpenBinary(bool compress);
    bool writeOpenXml(int maxInlineBufferSize,bool useImageAndMeshFileformats);
    bool writeOpenBinaryNoHeader(bool compress);
    void writeClose();

    int readOpenBinary(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,int& licenseTypeThatWroteThis,char& revNumber,bool ignoreTooOldSerializationVersion);
    int readOpenXml(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,int& licenseTypeThatWroteThis,char& revNumber,bool ignoreTooOldSerializationVersion);
    int readOpenBinaryNoHeader();
    void readClose();

#ifdef DOUBLESERIALIZATIONOPERATION
    CSerTmp& flt();
    CSerTmp& dbl();
#else
    CSer& flt();
    CSer& dbl();
#endif

    char getFileType() const;
    CSer& operator<< (const int& v);
#ifndef DOUBLESERIALIZATIONOPERATION
    CSer& operator<< (const floatFloat& v);
    CSer& operator<< (const double& v);
#endif
    CSer& operator<< (const unsigned short& v);
    CSer& operator<< (const unsigned int& v);
    CSer& operator<< (const quint64& v);
    CSer& operator<< (const long& v);
    CSer& operator<< (const unsigned char& v);
    CSer& operator<< (const char& v);
    CSer& operator<< (const std::string& v);

    CSer& operator>> (int& v);
#ifndef DOUBLESERIALIZATIONOPERATION
    CSer& operator>> (floatFloat& v);
    CSer& operator>> (double& v);
#endif
    CSer& operator>> (unsigned short& v);
    CSer& operator>> (unsigned int& v);
    CSer& operator>> (quint64& v);
    CSer& operator>> (long& v);
    CSer& operator>> (unsigned char& v);
    CSer& operator>> (char& v);
    CSer& operator>> (std::string& v);

    void flush(bool writeNbOfBytes=true);
    
    bool isStoring() const;
    bool isBinary() const;
    void setCountingMode(bool force=false);
    bool setWritingMode(bool force=false);
    void disableCountingModeExceptForExceptions();
    std::vector<unsigned char>* getBufferPointer();
    std::vector<unsigned char>* getFileBuffer();
    int getFileBufferReadPointer() const;
    void addOffsetToFileBufferReadPointer(int off);
    int getCounter() const;

    void loadUnknownData();
    bool getFoundUnknownCommands();

    unsigned short getCoppeliaSimVersionThatWroteThisFile();
    int getLicenseTypeThatWroteThisFile();
    int getSerializationVersionThatWroteThisFile();
    static int getSerializationVersionThatWroteLastFile();

    void storeDataName(const char* name);
    std::string readDataName();
    int readBytesButKeepPointerUnchanged(unsigned char* buffer,int desiredCount);
    VArchive& getArchive();

    std::string getFilenamePath() const;
    std::string getFilenameBase() const;
    int getIncrementCounter();
    void clearIncrementCounter();

    std::string xmlGetStackString() const;
    bool xmlSaveDataInline(size_t bufferSize) const;

    xmlNode* _xmlCreateNode(const char* name);
    xmlNode* _xmlCreateNode(const char* name,const char* nameAttribute);
    xmlNode* _xmlCreateNode(const char* name,int idAttribute);
    xmlNode* xmlPushNewNode(const char* name);
    xmlNode* xmlPushNewNode(const char* name,const char* nameAttribute);
    xmlNode* xmlPushNewNode(const char* name,int idAttribute);
    void _xmlPushNode(xmlNode* node);
    void xmlPopNode();

    void warnMissingNode(const char* name) const;
    xmlNode* xmlGetCurrentNode();

    void xmlAddNode_comment(const char* comment,bool doNotInsert);
    void xmlAddNode_bool(const char* name,bool val);
    void xmlAddNode_bools(const char* name,const std::vector<bool>& vals);
    void xmlAddNode_binFile(const char* name,const char* localFilenameSuffix,const unsigned char* buff,size_t buffSize);
    CSer* xmlAddNode_binFile(const char* name,const char* localFilenameSuffix);
    void xmlAddNode_imageFile(const char* name,const char* localFilenameSuffix,const unsigned char* img,int resX,int resY,bool rgba);
    void xmlAddNode_meshFile(const char* name,const char* localFilenameSuffix,const floatFloat* vertices,int vl,const int* indices,int il,const floatFloat* normals,int nl,const unsigned char* edges,int el);
    void xmlAddNode_string(const char* name,const char* str);
    void xmlAddNode_strings(const char* name,const std::vector<std::string>& vals);
    void xmlAddNode_cdata(const char* name,const char* str);
    void xmlAddNode_enum(const char* name,int val,int v1,const char* str1,int v2,const char* str2,int v3=-1,const char* str3=nullptr,int v4=-1,const char* str4=nullptr,int v5=-1,const char* str5=nullptr,int v6=-1,const char* str6=nullptr,int v7=-1,const char* str7=nullptr,int v8=-1,const char* str8=nullptr,int v9=-1,const char* str9=nullptr);
    void xmlAddNode_enum(const char* name,int val,const std::vector<int>& vals,const std::vector<std::string>& strings);
    void xmlAddNode_int(const char* name,int val);
    void xmlAddNode_2int(const char* name,int val1,int val2);
    void xmlAddNode_3int(const char* name,int val1,int val2,int val3);
    void xmlAddNode_ints(const char* name,const int* vals,size_t cnt);
    void xmlAddNode_ints(const char* name,const std::vector<int>& vals);
    void xmlAddNode_uint(const char* name,unsigned int val);
    void xmlAddNode_ulonglong(const char* name,unsigned long long val);
    void xmlAddNode_uchars(const char* name,const std::vector<unsigned char>& vals);
    void xmlAddNode_float(const char* name,floatDouble val);
    void xmlAddNode_2float(const char* name,floatDouble val1,floatDouble val2);
    void xmlAddNode_3float(const char* name,floatDouble val1,floatDouble val2,floatDouble val3);
    void xmlAddNode_4float(const char* name,floatDouble val1,floatDouble val2,floatDouble val3,floatDouble val4);
    void xmlAddNode_floats(const char* name,const floatDouble* vals,size_t cnt);
    void xmlAddNode_floats(const char* name,const std::vector<floatDouble>& vals);
    void xmlAddNode_double(const char* name,double val);

    void xmlGetAllChildNodeNames(std::vector<std::string>& allNames);
    bool xmlPushChildNode(const char* name,bool required=true);
    bool xmlPushSiblingNode(const char* name,bool required=true);

    bool xmlGetNode_nameAttribute(std::string& val,bool required=true);
    bool xmlGetNode_idAttribute(int& val,bool required=true);

    bool xmlGetNode_bool(const char* name,bool& val,bool required=true);
    bool xmlGetNode_bools(const char* name,std::vector<bool>& vals,bool required=true);
    bool xmlGetNode_flags(const char* name,int& flags,int flagWhenTrue,bool required=true);
    bool xmlGetNode_binFile(const char* name,std::vector<unsigned char>& buffer,bool required=true);
    bool xmlGetNode_binFile(const char* name,std::string& buffer,bool required=true);
    CSer* xmlGetNode_binFile(const char* name,bool required=true);
    bool xmlGetNode_imageFile(const char* name,std::vector<unsigned char>& image,int* resX=nullptr,int* resY=nullptr,bool* rgba=nullptr,bool required=true);
    bool xmlGetNode_meshFile(const char* name,std::vector<floatFloat>& vertices,std::vector<int>& indices,std::vector<floatFloat>& normals,std::vector<unsigned char>& edges,bool required=true);
    bool xmlGetNode_string(const char* name,std::string& val,bool required=true);
    bool xmlGetNode_strings(const char* name,std::vector<std::string>& vals,bool required=true);
    bool xmlGetNode_cdata(const char* name,std::string& val,bool required=true);
    bool xmlGetNode_enum(const char* name,int& val,bool required,const char* str1,int v1,const char* str2,int v2,const char* str3=nullptr,int v3=-1,const char* str4=nullptr,int v4=-1,const char* str5=nullptr,int v5=-1,const char* str6=nullptr,int v6=-1,const char* str7=nullptr,int v7=-1,const char* str8=nullptr,int v8=-1,const char* str9=nullptr,int v9=-1);
    bool xmlGetNode_enum(const char* name,int& val,bool required,const std::vector<int>& vals,const std::vector<std::string>& strings);
    bool xmlGetNode_int(const char* name,int& val,bool required=true);
    bool xmlGetNode_2int(const char* name,int& val1,int& val2,bool required=true);
    bool xmlGetNode_3int(const char* name,int& val1,int& val2,int& val3,bool required=true);
    bool xmlGetNode_ints(const char* name,int* vals,size_t cnt,bool required=true);
    bool xmlGetNode_ints(const char* name,std::vector<int>& vals,bool required=true);
    bool xmlGetNode_uint(const char* name,unsigned int& val,bool required=true);
    bool xmlGetNode_ulonglong(const char* name,unsigned long long& val,bool required=true);
    bool xmlGetNode_uchars(const char* name,std::vector<unsigned char>& vals,bool required=true);
    bool xmlGetNode_float(const char* name,floatDouble& val,bool required=true);
    bool xmlGetNode_2float(const char* name,floatDouble& val1,floatDouble& val2,bool required=true);
    bool xmlGetNode_3float(const char* name,floatDouble& val1,floatDouble& val2,floatDouble& val3,bool required=true);
    bool xmlGetNode_4float(const char* name,floatDouble& val1,floatDouble& val2,floatDouble& val3,floatDouble& val4,bool required=true);
    bool xmlGetNode_floats(const char* name,floatDouble* vals,size_t cnt,bool required=true);
    bool xmlGetNode_floats(const char* name,std::vector<floatDouble>& vals,bool required=true);
    bool xmlGetNode_double(const char* name,double& val,bool required=true);
    static int XML_XSERIALIZATION_VERSION;
    static char getFileTypeFromName(const char* filename);

    static int SER_SERIALIZATION_VERSION;
    static int SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS;
    static int SER_MIN_SERIALIZATION_VERSION_THAT_THIS_CAN_READ;

private:
    void _commonInit();
    void _writeBinaryHeader();
    void _writeXmlHeader();
    void _writeXmlFooter();
    int _readXmlHeader(int& serializationVersion,unsigned short& coppeliaSimVersionThatWroteThis,char& revNumber);
    std::string _getNodeText(const xmlNode* node) const;
    std::string _getNodeCdataText(const xmlNode* node) const;
    VArchive* theArchive;
    VFile* theFile;
    std::vector<char>* _bufferArchive;
    sim::tinyxml2::XMLDocument _xmlDocument;
    xmlNode* _xmlCurrentNode;
    std::vector<xmlNode*> _xmlNodes;
    int _xmlMaxInlineBufferSize;
    int _multiPurposeCounter;
    bool _xmlUseImageAndMeshFileformats;
    bool _compress;
    bool _noHeader;
    char _filetype;
    std::string _filename;
    bool _storing;
    int counter;
    int countingMode;
    bool _coutingModeDisabledExceptForExceptions;
#ifndef DOUBLESERIALIZATIONOPERATION
    std::vector<unsigned char> buffer;
    std::vector<unsigned char> _fileBuffer;
    int _fileBufferReadPointer;
#endif
    bool _foundUnknownCommands;

    unsigned short _coppeliaSimVersionThatWroteThis;
    int _licenseTypeThatWroteThis;
    int _serializationVersionThatWroteThisFile;
    static int _serializationVersionThatWroteLastFile;
};
