#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#define FIRST_CLUSTER_INDEX 2
#define FAT_OFFSET_LOWERBOUND 24
#define JUMPBOOT_INDEX_0 0xEB
#define JUMPBOOT_INDEX_1 0x76
#define JUMPBOOT_INDEX_2 0x90
#define MUST_BE_ZERO_BYTES 5
#define BOOT_SIGNATURE_VALUE 0xAA55

#pragma pack(1)
#pragma pack(push)
typedef struct MAIN_BOOT_SECTOR
{
    uint8_t jump_boot[3];
    char fs_name[8];
    uint8_t must_be_zero[53];
    uint64_t partition_offset;
    uint64_t volume_length;
    uint32_t fat_offset;
    uint32_t fat_length;
    uint32_t cluster_heap_offset;
    uint32_t cluster_count;
    uint32_t first_cluster_of_root_directory;
    uint32_t volume_serial_number;
    uint16_t fs_revision;
    uint16_t fs_flags;
    uint8_t bytes_per_sector_shift;
    uint8_t sectors_per_cluster_shift;
    uint8_t number_of_fats;
    uint8_t drive_select;
    uint8_t percent_in_use;
    uint8_t reserved[7];
    uint8_t bootcode[390];
    uint16_t boot_signature;
} main_boot_sector;
#pragma pack(pop)

void check_main_boot_sector(main_boot_sector main_boot_sector);

void check_allocation_bitmap(main_boot_sector main_boot_sector, int file_descriptor);

int main(int argc, char *argv[])
{
    char *filename;
    int exfat_file_descriptor;
    main_boot_sector main_boot_sector;

    // Required a filename to start the program 
    if (argc == 2) 
    {
        filename = argv[1];
        exfat_file_descriptor = open(filename, O_RDONLY);

        if (exfat_file_descriptor != -1)
        {
            // read all the bytes in the file main boot sector (minus the excess space)
            read(exfat_file_descriptor, &main_boot_sector, sizeof(main_boot_sector));

            check_main_boot_sector(main_boot_sector);
            check_allocation_bitmap(main_boot_sector, exfat_file_descriptor);

            return EXIT_SUCCESS;
        }
        else
        {
            printf("Failed to read the provided file, please provide a valid filename!\n");
            return EXIT_FAILURE;
        }
    }
    printf("A filename is required!\n");
    return EXIT_FAILURE;
}

void check_main_boot_sector(main_boot_sector main_boot_sector)
{
    int count = 0;
    int inconsistency_found = 0;
    int bytes_per_sector_shift = main_boot_sector.bytes_per_sector_shift;

    // 'Magic' numbers in the below variables are taken from the formulas in
    // https://docs.microsoft.com/en-us/windows/win32/fileio/exfat-specification#3-main-and-backup-boot-regions
    unsigned long volume_length_lowerbound = (1 << 20) / (1 << bytes_per_sector_shift);
    unsigned long fat_offset_upperbound = main_boot_sector.cluster_heap_offset - (main_boot_sector.fat_length * main_boot_sector.number_of_fats);
    unsigned long fat_length_lowerbound = ceil((main_boot_sector.cluster_count + 2) * ((1 << 2) / (2 << bytes_per_sector_shift)));
    unsigned long fat_length_upperbound = floor((main_boot_sector.cluster_heap_offset - main_boot_sector.fat_offset) / main_boot_sector.number_of_fats);

    if (main_boot_sector.jump_boot[0] != JUMPBOOT_INDEX_0 || main_boot_sector.jump_boot[1] != JUMPBOOT_INDEX_1 || main_boot_sector.jump_boot[2] != JUMPBOOT_INDEX_2)
    {
        printf("Inconsistent file system: JumpBoot must be 0xEB 0x76 0x90, value is %#X %#X %#X.\n", main_boot_sector.jump_boot[0], main_boot_sector.jump_boot[1], main_boot_sector.jump_boot[2]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(main_boot_sector.fs_name, "EXFAT   ") != 0)
    {
        printf("Inconsistent file system: FileSystemName must be 'EXFAT   ', value is '%s'.\n", main_boot_sector.fs_name);
        exit(EXIT_FAILURE);
    }

    while (count < MUST_BE_ZERO_BYTES || inconsistency_found)
    {
        if (main_boot_sector.must_be_zero[count])
        {
            inconsistency_found = 1;
            printf("Inconsistent file system: MustBeZero must be 0, value is %i at bit index %i.\n", main_boot_sector.must_be_zero[count], count);
            exit(EXIT_FAILURE);
        }
        else
        {
            count += 1;
        }
    }

    // No checking for max volume length (2^64-1) because it is the maximum value 8 bytes can describe

    if (main_boot_sector.volume_length < volume_length_lowerbound)
    {
        printf("Inconsistent file system: VolumeLength must be at least %lu, value is %lu.\n", volume_length_lowerbound, main_boot_sector.volume_length);
        exit(EXIT_FAILURE);
    }

    if (main_boot_sector.fat_offset < FAT_OFFSET_LOWERBOUND || main_boot_sector.fat_offset > fat_offset_upperbound)
    {
        printf("Inconsistent file system: FatOffset must be between 24 and %lu, value is %u.\n", fat_offset_upperbound, main_boot_sector.fat_offset);
        exit(EXIT_FAILURE);
    }

    if (main_boot_sector.fat_length < fat_length_lowerbound || main_boot_sector.fat_length > fat_length_upperbound)
    {
        printf("Inconsistent file system: FatLength must be between %lu and %lu, value is %u.\n", fat_length_lowerbound, fat_length_upperbound, main_boot_sector.fat_length);
        exit(EXIT_FAILURE);
    }

    if (main_boot_sector.first_cluster_of_root_directory < FIRST_CLUSTER_INDEX || main_boot_sector.first_cluster_of_root_directory > (main_boot_sector.cluster_count + 1))
    {
        printf("Inconsistent file system: FirstClusterOfRootDirectory must be between 2 and %i, value is %u.\n", (main_boot_sector.cluster_count + 1), main_boot_sector.first_cluster_of_root_directory);
        exit(EXIT_FAILURE);
    }

    if (main_boot_sector.boot_signature != BOOT_SIGNATURE_VALUE)
    {
        printf("Inconsistent file system: BootSignature should be 0xAA55, value is %#X.\n", main_boot_sector.boot_signature);
        exit(EXIT_FAILURE);
    }

    printf("MBR appears to be consistent.\n");
}

void check_allocation_bitmap(main_boot_sector main_boot_sector, int file_descriptor)
{
    uint8_t *data;
    uint8_t entry_type;
    uint32_t first_cluster_of_root_directory;
    uint64_t allocated_bitmap_data_length;
    int found_root_directory_cluster = 0;
    int set_bits = 0;
    int allocated_percentage;
    int bytes_per_sector_shift = main_boot_sector.bytes_per_sector_shift;
    int sectors_per_cluster_shift = main_boot_sector.sectors_per_cluster_shift;

    // Move to the cluster heap
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * main_boot_sector.cluster_heap_offset, SEEK_SET);
    // Move to the first cluster of root directory
    lseek(
            file_descriptor,
            // exchange units to bytes
            (1 << bytes_per_sector_shift) * (1 << sectors_per_cluster_shift) * (main_boot_sector.first_cluster_of_root_directory - FIRST_CLUSTER_INDEX), 
            SEEK_CUR
        );

    while (!found_root_directory_cluster)
    {   
        read(file_descriptor, &entry_type, 1);
        if (entry_type == 0x81)
        {
            lseek(file_descriptor, 19, SEEK_CUR);
            read(file_descriptor, &first_cluster_of_root_directory, 4);
            read(file_descriptor, &allocated_bitmap_data_length, 8);
            found_root_directory_cluster = 1;
        }
        else
        {
            lseek(file_descriptor, 31, SEEK_CUR);
        }
    }

    // Move back to the start of the cluster heap
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * main_boot_sector.cluster_heap_offset, SEEK_SET);
    // Move to the first cluster that contain the allocation bitmap, assuming all the required content is within this cluster (from Lab4 Description)
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * (1 << sectors_per_cluster_shift) * (first_cluster_of_root_directory - 2), SEEK_CUR);

    data = malloc(sizeof(uint8_t) * allocated_bitmap_data_length);

    read(file_descriptor, data, allocated_bitmap_data_length);

    for (int i = 0; i < (int) allocated_bitmap_data_length; i++)
    {
        set_bits += __builtin_popcount(data[i]);
    }

    allocated_percentage = set_bits * 100 / main_boot_sector.cluster_count;

    if (main_boot_sector.percent_in_use != allocated_percentage)
    {
        printf("Inconsistent file system: PercentInUse is %i%%, allocation bitmap is %i/%i => %i%%.\n", main_boot_sector.percent_in_use, set_bits, main_boot_sector.cluster_count, allocated_percentage);
        exit(EXIT_FAILURE);
    }

    printf("File system appears to be consistent.\n");

    free(data);
}



