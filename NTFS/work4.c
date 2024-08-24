#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)

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

typedef struct{
    uint16_t fixupArr;
    uint16_t fixup_origin1;
    uint16_t fixup_origin2;
    uint16_t fixup_origin3;
} fixupArray;

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

typedef struct {
    uint32_t attributeTypeID;  
    uint32_t length;           
    uint8_t nonResidentFlag;   
    uint8_t nameLength;       
    uint16_t nameOffset;      
    uint16_t flags;           
    uint16_t attributeID;     
} ATTRIBUTE_HEADER;


typedef struct {
    uint32_t contentSize;           
    uint16_t contentOffset;         
    uint8_t indexedFlag;            
    uint8_t reserved;               
} RESIDENT_ATTRIBUTE_HEADER;


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


int parse_Attributes_Header(FILE *fp){
    ATTRIBUTE_HEADER header;
    fread(&header, sizeof(ATTRIBUTE_HEADER), 1, fp);
    if(header.attributeTypeID == 0xFFFFFFFF){
        fclose(fp);
        return 0;
    }
    
    if(header.nonResidentFlag == 0){
        RESIDENT_ATTRIBUTE_HEADER res_h;
        fread(&res_h, sizeof(RESIDENT_ATTRIBUTE_HEADER), 1, fp);
        fseek(fp, header.length - sizeof(ATTRIBUTE_HEADER) - sizeof(RESIDENT_ATTRIBUTE_HEADER), SEEK_CUR);

        parse_Attributes_Header(fp);

    } else if(header.nonResidentFlag == 1){
        NON_RESIDENT_ATTRIBUTE_HEADER non_res_h;
        fread(&non_res_h, sizeof(NON_RESIDENT_ATTRIBUTE_HEADER), 1, fp);
      
        fseek(fp, non_res_h.runListOffset - (sizeof(NON_RESIDENT_ATTRIBUTE_HEADER) + sizeof(ATTRIBUTE_HEADER)), SEEK_CUR);
        
        unsigned int runlist_len = header.length - non_res_h.runListOffset;
        int cnt = 0;
        int raw_flag = 0 ;
        uint8_t first_byte = 0;
        uint8_t length_size = 0;
        uint8_t offset_size = 0;

        int64_t offset; 
        uint64_t length;
        uint64_t raw_addr = 0;
        while(1) {
            fread(&first_byte, 1, 1, fp);
            cnt++;

            if(cnt == runlist_len) break;
            if(first_byte == 0) continue;
            

            length_size = first_byte & 0xF;
            offset_size = first_byte >> 4;

           
            length = 0;
            for (int i = 0; i < length_size; ++i) {
                uint8_t byte;
                fread(&byte, 1, 1, fp);
                cnt++;
                length |= ((uint64_t)byte << (i * 8));
            }

            
            offset = 0;
            for (int i = 0; i < offset_size; ++i) {
                uint8_t byte;
                fread(&byte, 1, 1, fp);
                cnt++;
                offset |= ((uint64_t)byte << (i * 8));
            }
            if(raw_flag == 0 && offset != 0){
                    raw_addr = offset;
                    raw_flag++;
                }
            else{
                    if (offset_size > 0) {
                        int64_t sign_bit = 1LL << (offset_size * 8 - 1);
                        if (offset & sign_bit) {
                            offset |= (-1LL << (offset_size * 8));
                        }
                    }
                    offset = raw_addr + offset;
                }
            if(offset_size == 0 ){
                printf("N/A %d", length);
            }else{
                printf("%d %d\n", offset, length);
            }
        }
        parse_Attributes_Header(fp);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    FILE *fp;
    NTFS_VBR vbr;
    MFT_ENTRY_HEADER mft_h;
    int mft_address;
    int cluster_size;
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Can not open file: %s\n", argv[1]);
        return 1;
    }
    
    fread(&vbr, sizeof(NTFS_VBR), 1, fp);
    cluster_size = vbr.bytesPerSector * vbr.sectorsPerCluster;
    mft_address = cluster_size * vbr.mftClusterNumber;

    fseek(fp, mft_address, SEEK_SET);
    fread(&mft_h, sizeof(MFT_ENTRY_HEADER), 1, fp);
    parse_Attributes_Header(fp);

    fclose(fp);
    return 0;
}
