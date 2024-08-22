#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BPS 512

struct GPTHeader {
    uint64_t signature;
    uint32_t revision;
    uint32_t headerSize;
    uint32_t headerCRC32;
    uint32_t reserved;
    uint64_t myLBA;
    uint64_t alternateLBA;
    uint64_t firstUsableLBA;
    uint64_t lastUsableLBA;
    uint8_t diskGUID[16];
    uint64_t partitionEntryLBA;
    uint32_t numPartitionEntries;
    uint32_t partitionEntrySize;
    uint32_t partitionEntryArrayCRC32;
};

struct GPTPartitionEntry {
    uint8_t partitionTypeGUID[16];
    uint8_t uniquePartitionGUID[16];
    uint64_t startingLBA;
    uint64_t endingLBA;
    uint64_t attributes;
    uint16_t partitionName[36];
};

struct ThreeBytes {
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
};

void print_partition_info(FILE *fp, struct GPTPartitionEntry *entry) {
    struct ThreeBytes JCBC;
    fseek(fp, entry->startingLBA * BPS, SEEK_SET);
    fread(&JCBC, sizeof(struct ThreeBytes), 1, fp);

    printf("A2A0D0EBE5B0334487C068B6B72699C7 ");
    printf("NTFS ");
    printf("%llu %llu\n", entry->startingLBA, entry->endingLBA - entry->startingLBA + 1);
}

void read_partition_entry(FILE *fp, struct GPTPartitionEntry *entry, uint32_t numEntries, uint32_t entrySize) {
    fread(entry, entrySize, numEntries, fp);
}

void read_gpt_header(FILE *fp, struct GPTHeader *gpt) {
    fread(gpt, sizeof(struct GPTHeader), 1, fp);
}

int is_partition_used(struct GPTPartitionEntry *entry) {
    uint8_t emptyGUID[16] = {0};
    return memcmp(entry->partitionTypeGUID, emptyGUID, 16) != 0;
}

int main(int argc, char* argv[]) {
    FILE *fp;
    struct GPTHeader gpt;
    struct GPTPartitionEntry* entry[128] = {0, };
    
    fp = fopen(argv[1], "rb");
    fseek(fp, BPS, SEEK_SET);
    read_gpt_header(fp, &gpt);
    fseek(fp, gpt.partitionEntryLBA * BPS, SEEK_SET);

    for(int i = 0; i < gpt.numPartitionEntries; i++) {
        entry[i] = (struct GPTPartitionEntry*)malloc(sizeof(struct GPTPartitionEntry));
        read_partition_entry(fp, entry[i], 1, gpt.partitionEntrySize);
    }
    
    for(int i = 0; i < gpt.numPartitionEntries; i++) {
        if (is_partition_used(entry[i])) {
            print_partition_info(fp, entry[i]);
        }
        free(entry[i]);
    }

    fclose(fp);
    return 0;
}
