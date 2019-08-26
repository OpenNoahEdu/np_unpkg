#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "readcfg.h"
#include "crc32.h"
#include "checksum.h"

struct
{
    char *name;
    int32_t id;
} pkg_fstype_tbl[] = {
    {"none", 0},
    {"fat", 1},
    {"yaffs", 2},
    {"yaffs2", 3},
    {"ext2", 4},
    {"ram", 5},
    {"raw", 6},
    {"nor", 7},
    {"ubifs", 8},
    {NULL, 0},
};

struct
{                           // length 2048 0x800u
    int64_t tag;            // length 8
    int32_t ver;            // length 4
    char unset[64 - 8 - 4]; // length 52
    struct
    {                      // length 64
        uint32_t len;      // length 4
        uint32_t offset;   // length 4
        int32_t ver;       // length 4
        int32_t fstype;    // length 4
        uint32_t checksum; // length 4
        char dev[12];      // length 12
        char unset[32];    // length 32
    } item[31];            // length 64 * 31 = 1984
} pkg_file_header;

int ora_buf(char *buffer, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        buffer[i] = ((buffer[i] & 0x55) << 1) | ((buffer[i] & 0xAA) >> 1);
    }
    return i;
}

int main(int argc, const char **argv)
{
    int pkg_num = 0;
    int idx = 0;
    int32_t ver = 0;
    int64_t tag = 0;

    if (argc <= 1)
    {
        printf("\n Used : %s upgrade.bin \n", argv[0]);
        exit(1);
    }

    FILE *upgrade_stream = fopen(argv[1], "rb");
    if (!upgrade_stream)
        return -1;
    fread(&pkg_file_header, 0x800u, 1u, upgrade_stream);
    fclose(upgrade_stream);

    ora_buf((void *)&pkg_file_header, 2048);

    printf("\n tag = ");
    fwrite((char *)&pkg_file_header.tag, 8, 1, stdout);
    printf(" ");
    printf("\n ver = %d ", pkg_file_header.ver);
    printf("\n ");

    for (int i = 0; i < 31; ++i)
    {
        if (pkg_file_header.item[i].len)
        {
            printf("\n pkg_item_len = %d ", pkg_file_header.item[i].len);
            printf("\n offset = %d ", pkg_file_header.item[i].offset);
            printf("\n ver = %d ", pkg_file_header.item[i].ver);
            printf("\n dev = ");
            fwrite((char *)&pkg_file_header.item[i].dev, 12, 1, stdout);
            printf(" ");
            printf("\n checksum = 0x%08X ", pkg_file_header.item[i].checksum);
            printf("\n fstype = %d ", pkg_file_header.item[i].fstype);
            printf("\n ");
        }
    }

    return 0;
}