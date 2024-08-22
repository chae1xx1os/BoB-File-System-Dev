import sys
import struct

BPS = 512  
FAT_ENTRY_SIZE = 4
END_OF_CLUSTER_CHAIN = 0x0FFFFFFF

def read_fat_chain(fp, fat_start_offset, start_cluster):
    cluster = start_cluster
    chain = []
    while cluster < END_OF_CLUSTER_CHAIN:
        chain.append(cluster)

        fat_entry_offset = fat_start_offset + cluster * FAT_ENTRY_SIZE
        fp.seek(fat_entry_offset)

        cluster_data = fp.read(FAT_ENTRY_SIZE)
        if len(cluster_data) < FAT_ENTRY_SIZE:
            break

        next_cluster = struct.unpack('<I', cluster_data)[0] & 0x0FFFFFFF
        
        if next_cluster >= END_OF_CLUSTER_CHAIN or next_cluster == 0:
            break

        cluster = next_cluster
    
    return chain

def get_fat_start_offset(fp):
    fp.seek(14)
    reserved_sector_count = struct.unpack('<H', fp.read(2))[0]

    fp.seek(36)
    fat_size = struct.unpack('<I', fp.read(4))[0]

    fat_start_offset = reserved_sector_count * BPS
    return fat_start_offset

def main():
    if len(sys.argv) != 3:
        print(f"실행파일명: {sys.argv[0]} <증거이미지주소> <시작클러스터>")
        return

    image_path = sys.argv[1]
    start_cluster = int(sys.argv[2])

    try:
        with open(image_path, "rb") as fp:
            fat_start_offset = get_fat_start_offset(fp)
            cluster_chain = read_fat_chain(fp, fat_start_offset, start_cluster)
            print(" ".join(map(str, cluster_chain)))
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()