import struct
import sys

def read_mbr_partitions(mbr_data):
    partitions = []
    extended_start = None

    for i in range(4):
        start = 0x1BE + (i * 16)
        end = start + 16
        
        entry = mbr_data[start:end]
        
        parts = struct.unpack('<BBBBBBBBII', entry)
        
        part_type = parts[4]
        start_sector = parts[8]
        sector_count = parts[9]

        if part_type == 0x07:
            partitions.append(("NTFS", start_sector, sector_count))
        elif part_type == 0x05 or part_type == 0x0F:
            extended_start = start_sector

    return partitions, extended_start

def read_ebr_partitions(file, ebr_start):
    logical_partitions = []
    current_ebr = ebr_start

    while True:
        file.seek(current_ebr * 512)
        
        ebr_data = file.read(512)

        entry = ebr_data[0x1BE:0x1CE]
        parts = struct.unpack('<BBBBBBBBII', entry)
        
        part_type = parts[4]
        start_sector = parts[8]
        sector_count = parts[9]

        if part_type == 0x07:
            absolute_start = current_ebr + start_sector
            logical_partitions.append(("NTFS", absolute_start, sector_count))

        next_entry = ebr_data[0x1CE:0x1DE]
        next_parts = struct.unpack('<BBBBBBBBII', next_entry)
        
        next_part_type = next_parts[4]
        next_start = next_parts[8]

        if next_part_type == 0 or next_start == 0:
            break

        current_ebr = ebr_start + next_start

    return logical_partitions

def main(file_path):
    try:
        with open(file_path, 'rb') as file:
            mbr_data = file.read(512)

            partitions, extended_start = read_mbr_partitions(mbr_data)

            for partition in partitions:
                fs_type, start, count = partition
                print(f"{fs_type} {start} {count}")

            if extended_start is not None:
                logical_parts = read_ebr_partitions(file, extended_start)

                for partition in logical_parts:
                    fs_type, start, count = partition
                    print(f"{fs_type} {start} {count}")

    except FileNotFoundError:
        print(f"파일을 찾을 수 없습니다: {file_path}")
    except Exception as e:
        print(f"오류 발생: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("사용법: python3 mbr_parser.py <디스크_이미지_파일>")
        sys.exit(1)

    disk_image_path = sys.argv[1]
    
    main(disk_image_path)