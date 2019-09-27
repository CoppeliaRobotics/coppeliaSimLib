#pragma once

#include "serBase.h"
#include "vrepMainHeader.h"
#include "vArchive.h"

#define SER_VREP_HEADER "VREP" // The file header since 03/07/2009
#define SER_END_OF_OBJECT "EOO"
#define SER_NEXT_STEP "NXT"
#define SER_END_OF_FILE "EOF"

class CSer : public CSerBase
{
public:

    enum {
        filetype_unspecified_file=0,        // ?
        filetype_vrep_bin_scene_file,       // .ttt
        filetype_vrep_bin_model_file,       // .ttm
        filetype_vrep_bin_thumbnails_file,  // .ttmt
        filetype_br_bin_scene_file,         // .brs
        filetype_br_bin_model_file,         // .brm
        filetype_vrep_bin_scene_buff,       // ---
        filetype_vrep_bin_model_buff,       // ---
        filetype_vrep_bin_ui_file,          // .ttb
        filetype_bin_file,                  // .bin
        filetype_bin_buff,                  // ---
    };

    CSer(const char* filename,char filetype); // saving to/restoring from file
    CSer(std::vector<char>& bufferArchive,char filetype); // saving to/restoring from buffer

    virtual ~CSer();

    bool writeOpenBinary(bool compress);
    bool writeOpenBinaryNoHeader(bool compress);
    void writeClose();

    int readOpenBinary(int& serializationVersion,unsigned short& vrepVersionThatWroteThis,int& licenseTypeThatWroteThis,char& revNumber,bool ignoreTooOldSerializationVersion);
    int readOpenBinaryNoHeader();
    void readClose();

    char getFileType() const;
    CSer& operator<< (const int& v);
    CSer& operator<< (const float& v);
    CSer& operator<< (const double& v);
    CSer& operator<< (const unsigned short& v);
    CSer& operator<< (const unsigned int& v);
    CSer& operator<< (const quint64& v);
    CSer& operator<< (const long& v);
    CSer& operator<< (const unsigned char& v);
    CSer& operator<< (const char& v);
    CSer& operator<< (const std::string& v);

    CSer& operator>> (int& v);
    CSer& operator>> (float& v);
    CSer& operator>> (double& v);
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

    unsigned short getVrepVersionThatWroteThisFile();
    int getLicenseTypeThatWroteThisFile();
    int getSerializationVersionThatWroteThisFile();
    
    void storeDataName(const char* name);
    std::string readDataName();
    int readBytesButKeepPointerUnchanged(unsigned char* buffer,int desiredCount);
    VArchive& getArchive();

    static char getFileTypeFromName(const char* filename);

    static int SER_SERIALIZATION_VERSION;
    static int SER_MIN_SERIALIZATION_VERSION_THAT_CAN_READ_THIS;
    static int SER_MIN_SERIALIZATION_VERSION_THAT_THIS_CAN_READ;
    static int XML_SERIALIZATION_VERSION;

private:
    void _commonInit();
    void _writeBinaryHeader();
    VArchive* theArchive;
    VFile* theFile;
    std::vector<char>* _bufferArchive;
    bool _compress;
    bool _noHeader;
    char _filetype;
    std::string _filename;
    bool _storing;
    int counter;
    int countingMode;
    bool _coutingModeDisabledExceptForExceptions;
    std::vector<unsigned char> buffer;
    std::vector<unsigned char> _fileBuffer;
    int _fileBufferReadPointer;
    bool _foundUnknownCommands;

    unsigned short _vrepVersionThatWroteThis;
    int _licenseTypeThatWroteThis;
    int _serializationVersionThatWroteThisFile;
};
