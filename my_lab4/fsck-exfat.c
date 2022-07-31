#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

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

void check_mbs(main_boot_sector mbs);

void check_allocation_bitmap(main_boot_sector mbs, int file_descriptor);

void check_allocation_bitmap(main_boot_sector mbs, int file_descriptor)
{
    uint8_t *data;
    uint8_t entry_type;
    uint32_t first_cluster_of_root_directory;
    uint64_t allocated_bitmap_data_length;
    int found_root_directory_cluster = 0;
    int total_bits = 0;
    int set_bits = 0;
    int allocated_percentage;
    int bytes_per_sector_shift = mbs.bytes_per_sector_shift;
    int sectors_per_cluster_shift = mbs.sectors_per_cluster_shift;

    lseek(file_descriptor, (1 << bytes_per_sector_shift) * mbs.cluster_heap_offset, SEEK_SET);
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * (1 << sectors_per_cluster_shift) * (mbs.first_cluster_of_root_directory - 2), SEEK_CUR);

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
    
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * mbs.cluster_heap_offset, SEEK_SET);
    lseek(file_descriptor, (1 << bytes_per_sector_shift) * (1 << sectors_per_cluster_shift) * (first_cluster_of_root_directory - 2), SEEK_CUR);

    data = malloc(sizeof(uint8_t) * allocated_bitmap_data_length);

    read(file_descriptor, data, allocated_bitmap_data_length);

    for (int i = 0; i < (int) allocated_bitmap_data_length; i++)
    {
        total_bits += 8;
        set_bits += __builtin_popcount(data[i]);
    }

    allocated_percentage = set_bits * 100 / total_bits;

    if (mbs.percent_in_use != allocated_percentage)
    {
        printf("Inconsistent file system: PercentInUse is %i%%, allocation bitmap is %i/%i => %i%%.\n", mbs.percent_in_use, set_bits, total_bits, allocated_percentage);
        exit(EXIT_FAILURE);
    }

    // printf("In use and total: %i %i\n", set_bits, total_bits);
    // printf("%i\n", mbs.percent_in_use);
    printf("File system appears to be consistent.\n");

    free(data);
}

void check_mbs(main_boot_sector mbs)
{
    // if (strcmp(mbs.fs_name, "EXFAT   ") != 0)
    // {
    //     printf("Inconsistent file system: FileSystemName must be 'EXFAT   ', value is '%s'.\n", mbs.fs_name);
    // }
    // else
    // {
    //     printf("FileSystemName field value is: %s\n", mbs.fs_name);
    // }

    int bytes_per_sector_shift = mbs.bytes_per_sector_shift;
    int count = 0;
    int inconsistency_found = 0;
    unsigned long volume_length_lowerbound = (1 << 20) / (1 << bytes_per_sector_shift);
    unsigned long fat_offset_upperbound = mbs.cluster_heap_offset - (mbs.fat_length * mbs.number_of_fats);
    unsigned long fat_length_lowerbound = ceil((mbs.cluster_count + 2) * ((1 << 2) / (2 << bytes_per_sector_shift)));
    unsigned long fat_length_upperbound = floor((mbs.cluster_heap_offset - mbs.fat_offset) / mbs.number_of_fats);

    // printf("JumpBoot field value is: %X\n", mbs.jump_boot[0]);
    // printf("JumpBoot field value is: %X\n", mbs.jump_boot[1]);
    // printf("JumpBoot field value is: %X\n", mbs.jump_boot[2]);

    // printf("FileSystemName field value is: %s\n", mbs.fs_name);

    // printf("%lu\n", fat_length_lowerbound);
    // printf("%lu\n", fat_length_upperbound);

    if (mbs.jump_boot[0] != 0xEB || mbs.jump_boot[1] != 0x76 || mbs.jump_boot[2] != 0x90)
    {
        printf("Inconsistent file system: JumpBoot must be 0xEB 0x76 0x90, value is %#X %#X %#X.\n", mbs.jump_boot[0], mbs.jump_boot[1], mbs.jump_boot[2]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(mbs.fs_name, "EXFAT   ") != 0)
    {
        printf("Inconsistent file system: FileSystemName must be 'EXFAT   ', value is '%s'.\n", mbs.fs_name);
        exit(EXIT_FAILURE);
    }

    while (count < 53 || inconsistency_found)
    {
        if (mbs.must_be_zero[count])
        {
            inconsistency_found = 1;
            printf("Inconsistent file system: MustBeZero must be 0, value is %i.\n", mbs.must_be_zero[count]);
            exit(EXIT_FAILURE);
        }
        else
        {
            count += 1;
        }
    }

    if (mbs.volume_length < volume_length_lowerbound)
    {
        printf("Inconsistent file system: VolumeLength must be at least %lu, value is %lu.\n", volume_length_lowerbound, mbs.volume_length);
        exit(EXIT_FAILURE);
    }

    if (mbs.fat_offset < 24 || mbs.fat_offset > fat_offset_upperbound)
    {
        printf("Inconsistent file system: FatOffset must be between 24 and %lu, value is %u.\n", fat_offset_upperbound, mbs.fat_offset);
        exit(EXIT_FAILURE);
    }

    if (mbs.fat_length < fat_length_lowerbound || mbs.fat_length > fat_length_upperbound)
    {
        printf("Inconsistent file system: FatLength must be between %lu and %lu, value is %u.\n", fat_length_lowerbound, fat_length_upperbound, mbs.fat_length);
        exit(EXIT_FAILURE);
    }

    if (mbs.first_cluster_of_root_directory < 2 || mbs.first_cluster_of_root_directory > (mbs.cluster_count + 1))
    {
        printf("Inconsistent file system: FirstClusterOfRootDirectory must be between 2 and %i, value is %u.\n", (mbs.cluster_count + 1), mbs.first_cluster_of_root_directory);
        exit(EXIT_FAILURE);
    }

    if (mbs.boot_signature != 0xAA55)
    {
        printf("Inconsistent file system: BootSignature should be 0xAA55, value is %#X.\n", mbs.boot_signature);
        exit(EXIT_FAILURE);
    }

    printf("MBR appears to be consistent.\n");
}

int main(int argc, char *argv[])
{
    char *filename;
    int exfat_file_descriptor;

    // Initialize all properties to 0.
    main_boot_sector mbs;

    // Required a valid filepath to start the program 
    if (argc == 2) 
    {
        filename = argv[1];
        exfat_file_descriptor = open(filename, O_RDONLY);

        if (exfat_file_descriptor != -1)
        {
            read(exfat_file_descriptor, &mbs, sizeof(main_boot_sector));
            check_mbs(mbs);
            check_allocation_bitmap(mbs, exfat_file_descriptor);
            //void parse_allocation_bitmap(elf_header *elf_header, program_header *program_header, int file_descriptor, int index);
            //void check_allocation_bitmap(elf_header *elf_header, program_header *program_header, int index);

            return EXIT_SUCCESS;
        }
        else
        {
            printf("Failed to read the provided complied file, please provide a valid filepath!\n");
            return EXIT_FAILURE;
        }
    }
    printf("A filepath is required!\n");
    return EXIT_FAILURE;
}


