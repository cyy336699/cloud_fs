#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include <aos/vfs.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include <string>
#include <vector>
#include "cloud_fs.h"
#include "../cloud_fs_oss/oss_def.h"
#include "cloud_fs_vfs.h"

extern Cloud_Dir cloud_main_dir;

int cloud_fs_read(char * filepath)
{
    int fd = aos_open(filepath, O_RDWR);

    if (fd < 0) {
        printf("file does not exist!\r\n");
        return -1;
    }

    char buf[1024] = {0};
    int ret = aos_read(fd, buf, 1023);
    buf[ret] = '\0';
    printf("%s\r\n", buf);

    aos_close(fd);
    return 0;
}

int cloud_fs_write(char * filepath, char * content)
{
    int fd = aos_open(filepath, O_RDWR);

    if (fd < 0) {
        printf("file does not exist!\r\n");
        return -1;
    }

    std::string _content = content;
    int ret = aos_write(fd, content, _content.size());
    if (ret == _content.size()) {
        printf("write cloud file well!!!\r\n");
        aos_close(fd);
        return 0;
    }
    else {
        printf("write cloud file failed!!!\r\n");
        aos_close(fd);
        return -1;
    }
}

int cloud_fs_touch(char * filepath)
{
    int fd = aos_open(filepath, O_RDWR | O_APPEND);

    if (fd >0) {
        printf("file already exists!\r\n");
        aos_close(fd);
        return 0;
    }

    fd = aos_open(filepath, O_CREAT | O_RDWR | O_APPEND);

    if (fd < 0) {
        printf("touch file failed!\r\n");
        return -1;
    }

    printf("touch file well!!!\r\n");
    aos_close(fd);
    return 0;
}

int cloud_fs_rmfile(char * filepath)
{
    int ret = aos_remove(filepath);
    if (ret < 0) {
        printf("Sorry, remove file failed!\r\n");
        return -1;
    }

    printf("remove file well!!!\r\n");
    return 0;
}

int cloud_fs_lsfile(char * dirpath)
{
    std::string dir_path = dirpath;

    int ret = cloud_main_dir.dirExists(dir_path);
    if (ret < 0) {
        printf("Sorry, the dir do not exists!\r\n");
        return -1;
    }

    cloud_main_dir.finddir(dir_path).listDir(0);
    return 0;
}

int cloud_fs_move(char  * oldpath, char * newpath)
{
    int fd1 = aos_open(oldpath, O_RDWR);

    if (fd1 < 0) {
        printf("file does not exist!\r\n");
        return -1;
    }

    int fd2 = aos_open(newpath, O_CREAT | O_RDWR | O_APPEND);

    char buf[1024] = {0};
    int ret = 0;
    do {
        ret = aos_read(fd1, buf, 1024);
        aos_write(fd2, buf, 1024);
    } while (ret > 0);

    aos_close(fd1);
    aos_close(fd2);

    aos_remove(oldpath);
    printf("move file well!!!\r\n");
    return 0;
}

int cloud_fs_cp(char  * oldpath, char * newpath)
{
    int fd1 = aos_open(oldpath, O_RDWR);

    if (fd1 < 0) {
        printf("file does not exist!\r\n");
        return -1;
    }

    int fd2 = aos_open(newpath, O_CREAT | O_RDWR | O_APPEND);

    char buf[1024] = {0};
    int ret = 0;
    do {
        ret = aos_read(fd1, buf, 1024);
        aos_write(fd2, buf, 1024);
    } while (ret > 0);

    aos_close(fd1);
    aos_close(fd2);

    printf("copy file well!!!\r\n");
    return 0;
}

int cloud_fs_mkdir(char *dirpath)
{
    std::string dir_path = dirpath;
    cloud_main_dir.mkdir(dir_path);

    printf("make dir well!!!\r\n");
    return 0;
}

int cloud_fs_rmdir(char * dirpath) 
{
    cloud_main_dir.rmdir(dirpath);

    printf("remove dir well!!!\r\n");
    return 0;
}