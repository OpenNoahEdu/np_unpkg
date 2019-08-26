#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

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
            char *fstype = "unknow";
            for (int ii = 0; pkg_fstype_tbl[ii].name; ++ii)
            {
                if (pkg_fstype_tbl[ii].id == pkg_file_header.item[i].fstype)
                {
                    fstype = pkg_fstype_tbl[ii].name;
                    break;
                }
            }
            printf("\n fstype = %s ", fstype);
            printf("\n checksum = 0x%08X ", pkg_file_header.item[i].checksum);
            printf("\n dev = ");
            fwrite(pkg_file_header.item[i].dev, 12, 1, stdout);
            printf(" ");

            char output_name[1024] = {0};
            if (pkg_file_header.item[i].dev[0] >= '0' && pkg_file_header.item[i].dev[0] <= '9')
            {
                char dev[13];
                memcpy(dev, pkg_file_header.item[i].dev, 12);
                dev[12] = '\0';
                long devi = strtol(dev, 0, 0);
                if (devi == 0)
                    strcpy(output_name, "u-boot-nand.bin");
                else if (devi == 0x400000)
                    strcpy(output_name, "uImage");
            }
            else if (!strcmp(pkg_file_header.item[i].dev, "/dev/null"))
            {
                strcpy(output_name, "uImage-initrd");
            }
            else
            {
                strcpy(output_name, strrchr(pkg_file_header.item[i].dev, '/') + 1);
            }
            if (output_name[0] == '\0')
                sprintf(output_name, "idx-%d-file.bin", i);
            printf("\n file = %s ", output_name);

            printf("\n ");
        }
    }

    fclose(upgrade_stream);
    return 0;
}