#if AOS_COMP_CLI
#include "aos/cli.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cloud_fs.h"

static void cloud_read(int argc, char **argv)
{
    if (argc < 2) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string filepath = argv[1];
    // printf("filepath:%s\r\n", argv[1]);
    if (filepath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (filepath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_read(argv[1]);
    }
}

static void cloud_write(int argc, char **argv)
{
    if (argc < 3) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string filepath = argv[1];
    std::string content = argv[2];
    // printf("filepath:%s\r\n", argv[1]);
    if (filepath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (filepath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else if (content.length() >= 1024) {
        printf("content is too long!\r\n");
    }
    else {
        cloud_fs_write(argv[1], argv[2]);
    }
}

static void cloud_touch(int argc, char **argv)
{
    if (argc < 2) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string filepath = argv[1];
    // printf("filepath:%s\r\n", argv[1]);
    if (filepath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (filepath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_touch(argv[1]);
    }
}

static void cloud_rmfile(int argc, char **argv)
{
    if (argc < 2) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string filepath = argv[1];
    // printf("filepath:%s\r\n", argv[1]);
    if (filepath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (filepath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_rmfile(argv[1]);
    }
}

static void cloud_lsfile(int argc, char **argv)
{
    cloud_fs_lsfile();
}

static void cloud_move(int argc, char **argv)
{
    if (argc < 3) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string oldpath = argv[1];
    std::string newpath = argv[2];
    // printf("filepath:%s\r\n", argv[1]);
    if (oldpath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (oldpath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else if (newpath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (newpath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_move(argv[1], argv[2]);
    }
}

static void cloud_cp(int argc, char **argv)
{
    if (argc < 3) {
        printf("Command has a problem!\r\n");
        return;
    }
    std::string oldpath = argv[1];
    std::string newpath = argv[2];
    // printf("filepath:%s\r\n", argv[1]);
    if (oldpath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (oldpath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else if (newpath.length() >= 1024) {
        printf("filepath is too long!\r\n");
    }
    else if (newpath.find('/') != std::string::npos) {
        printf("Sorry, wo don't support directories now!\r\n");
    }
    else {
        cloud_fs_cp(argv[1], argv[2]);
    }
}

#if AOS_COMP_CLI
/* reg args: fun, cmd, description*/
ALIOS_CLI_CMD_REGISTER(cloud_read, cloud_read, cloud read file command)
ALIOS_CLI_CMD_REGISTER(cloud_write, cloud_write, cloud write file command)
ALIOS_CLI_CMD_REGISTER(cloud_touch, cloud_touch, cloud touch file command)
ALIOS_CLI_CMD_REGISTER(cloud_rmfile, cloud_rmfile, cloud remove file command)
ALIOS_CLI_CMD_REGISTER(cloud_lsfile, cloud_lsfile, cloud ls file command)
ALIOS_CLI_CMD_REGISTER(cloud_move, cloud_move, cloud move file command)
ALIOS_CLI_CMD_REGISTER(cloud_cp, cloud_cp, cloud copy file command)
#endif