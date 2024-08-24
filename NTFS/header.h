#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

#pragma pack(push, 1)

// NTFS 부트 섹터 구조체
typedef struct {
    uint8_t  jumpInstruction[3];    
    uint8_t  oemID[8];              
    uint16_t bytesPerSector;        
    uint8_t  sectorsPerCluster;     
    uint16_t reservedSectors;       
    uint8_t  zeroes1[3];            
    uint16_t unused1;               
    uint8_t  mediaDescriptor;       
    uint16_t zeroes2;               
    uint16_t sectorsPerTrack;       
    uint16_t numberOfHeads;         
    uint32_t hiddenSectors;         
    uint32_t unused2;               
    uint32_t unused3;               
    uint64_t totalSectors;          
    uint64_t mftClusterNumber;      
    uint64_t mftMirrClusterNumber;  
    int8_t   clustersPerFileRecordSegment; 
    uint8_t  reserved[3];           
    int8_t   clustersPerIndexBlock; 
    uint8_t  reserved2[3];          
    uint64_t volumeSerialNumber;    
    uint32_t checksum;              
    uint8_t  bootCode[426];         
    uint16_t endOfSectorMarker;     
} NTFS_VBR;

#pragma pack(pop)

#pragma pack(push, 1) 

// Fixup Array 구조체
typedef struct {
    uint16_t fixupArr;
    uint16_t fixup_origin1;
    uint16_t fixup_origin2;
    uint16_t fixup_origin3;
} fixupArray;

// MFT 엔트리 헤더 구조체
typedef struct {
    uint32_t signature;            
    uint16_t fixupOffset;          
    uint16_t fixupCount;           
    uint64_t logFileSequenceNumber;
    uint16_t sequenceNumber;       
    uint16_t hardLinkCount;        
    uint16_t firstAttributeOffset; 
    uint16_t flags;                
    uint32_t usedSize;             
    uint32_t allocatedSize;        
    uint64_t baseFileRecord;       
    uint16_t nextAttributeID;      
    uint16_t alignment;            
    uint32_t mftEntryNumber;       
    fixupArray fixupArray;         
} MFT_ENTRY_HEADER;

#pragma pack(pop) 

#pragma pack(push, 1) 

// 속성 헤더 구조체
typedef struct {
    uint32_t attributeTypeID;  
    uint32_t length;           
    uint8_t nonResidentFlag;   
    uint8_t nameLength;        
    uint16_t nameOffset;       
    uint16_t flags;            
    uint16_t attributeID;      
} ATTRIBUTE_HEADER;

// Resident 속성 헤더 구조체
typedef struct {
    uint32_t contentSize;           
    uint16_t contentOffset;         
    uint8_t indexedFlag;            
    uint8_t reserved;               
} RESIDENT_ATTRIBUTE_HEADER;

// Non-Resident 속성 헤더 구조체
typedef struct { 
    uint64_t startingVCN;          
    uint64_t endingVCN;            
    uint16_t runListOffset;        
    uint16_t compressionUnitSize;  
    uint32_t reserved;             
    uint64_t allocatedSize;        
    uint64_t realSize;             
    uint64_t initializedSize;      
} NON_RESIDENT_ATTRIBUTE_HEADER;

#pragma pack(pop) 

#endif // HEADER_H
