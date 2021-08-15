#if AOS_COMP_CLI
#include "aos/cli.h"
#endif

#include <stdio.h>
#include "cloud_fs.h"
#include <assert.h>
#include <aos/vfs.h>
#include "cloud_fs_vfs.h"

extern Cloud_Dir cloud_main_dir;

static void cloud_fs_init(int argc, char **argv)
{
    printf("***********start test cloud_fs init by panic***********\r\n");

    cloud_fs_register("/cloud");

    aos_rmdir("/cloud");
    aos_mkdir("/cloud/test");

    cloud_fs_dir_sync();
    cloud_main_dir.listDir(0);

    printf("***********test cloud_fs init by panic well!!!***********\r\n");
    return;    
}

static void cloud_fs_dir_test(int argc, char **argv)
{
    printf("***********start test cloud_fs dir by panic***********\r\n");

    int ret = 0;

    aos_mkdir("/cloud/test/testdir1");
    aos_mkdir("/cloud/test/testdir2");
    printf("mkdir well!!!\r\n");
    cloud_main_dir.listDir(0);

    aos_dir_t *dir = aos_opendir("/cloud/test");
    printf("opendir well!!!\r\n");

    aos_dirent_t *dp = NULL;
    do {
        dp = aos_readdir(dir);
        if (dp)
            printf("readdir: %s\r\n", dp->d_name);
    } while (dp != NULL);

    ret = aos_telldir(dir);
    assert(ret == 2);

    aos_rewinddir(dir);
    printf("rewinddir well!!!\r\n");

    dp = aos_readdir(dir);
    printf("readdir: %s\r\n", dp->d_name);

    aos_seekdir(dir, 0);
    printf("seekdir well!!!\r\n");

    dp = aos_readdir(dir);
    printf("readdir: %s\r\n", dp->d_name);

    aos_closedir(dir);
    printf("close dir well!!!\r\n");

    aos_rmdir("/cloud/test/testdir1");
    aos_rmdir("/cloud/test/testdir2");
    printf("rmdir well!!!\r\n");
    cloud_main_dir.listDir(0);

    printf("***********test cloud_fs dir by panic well!!!***********\r\n");
    return;    
}

static void cloud_fs_file_test(int argc, char **argv)
{
    printf("***********start test cloud_fs file by panic***********\r\n");

    char buf[64] = {0};
    int fd = 0, fd1 = 0, ret = 0;

    aos_remove("/data/a.txt");

    // should fd > 0
    fd = aos_open("/cloud/a.txt", O_CREAT | O_RDWR | O_APPEND);
    printf("aos_open well!!!\r\n");

    cloud_main_dir.listDir(0);

    // should write well
    ret = aos_write(fd, "Welcome to Our Cloud File System\n", 33);
    assert(ret == 33);
    printf("write file well!!!\r\n");


    ret = aos_lseek(fd, 0, 0);
    assert(ret == 0);
    printf("lseek file well!!!\r\n");

    // should read well
    ret = aos_read(fd, buf, 33);
    assert(ret == 33);
    buf[33] = '\0';
    printf("aos_read1: %s \r\n", buf);

    // should lseek well
    ret = aos_lseek(fd, 11, 0);
    assert(ret == 11);
    printf("lseek file well!!!\r\n");

    // should read well
    ret = aos_read(fd, buf, 22);
    assert(ret == 22);
    buf[22] = '\0';
    printf("aos_read2: %s \r\n", buf);

    // should close well
    ret = aos_close(fd);
    printf("aos_close ret1: %d\r\n", ret);

    cloud_main_dir.listDir(0);

    printf("***********test cloud_fs file by panic well!!!***********\r\n");
    return;
}

static void cloud_fs_test(int argc, char **argv)
{
    printf("***********start test cloud_fs by panic***********\r\n");

    // aos_remove("/data/cloud/demo.txt");
    // aos_remove("/data/cloud/demo1.txt");
    // aos_remove("/data/cloud/demo2.txt");
    // int ret;

    // ret = cloud_fs_lsfile();
    // assert(ret == 0);
    // printf("*******cloud_fs_lsfile test well\r\n");

    // ret = cloud_fs_write("cloud.txt", "cloud_fs_demo here is OK!\r\n");
    // assert(ret == 0);
    // printf("*******cloud_fs_write test well\r\n");

    // ret = cloud_fs_read("demo.txt");
    // assert(ret == -1);

    // ret = cloud_fs_read("cloud.txt");
    // assert(ret == 0);
    // printf("*******cloud_fs_read test well\r\n");

    // ret = cloud_fs_write("demo.txt", "cloud fs demo well\r\n");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo.txt");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo1.txt");
    // assert(ret == -1);

    // ret = cloud_fs_touch("demo1.txt");
    // assert(ret == 0);
    // printf("*******cloud_fs_touch test well\r\n");

    // ret = cloud_fs_lsfile();
    // assert(ret == 0);

    // ret = cloud_fs_read("demo2.txt");
    // assert(ret == -1);

    // ret = cloud_fs_move("demo.txt", "demo2.txt");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo2.txt");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo.txt");
    // assert(ret == -1);
    // printf("*******cloud_fs_move test well\r\n");

    // ret = cloud_fs_cp("demo2.txt", "demo.txt");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo2.txt");
    // assert(ret == 0);

    // ret = cloud_fs_read("demo.txt");
    // assert(ret == 0);
    // printf("*******cloud_fs_cp test well\r\n");

    // ret = cloud_fs_rmfile("demo.txt");
    // assert(ret == 0);

    // ret = cloud_fs_rmfile("demo1.txt");
    // assert(ret == 0);

    // ret = cloud_fs_rmfile("demo2.txt");
    // assert(ret == 0);
    // printf("*******cloud_fs_rmfile test well\r\n");

    // ret = cloud_fs_lsfile();
    // assert(ret == 0);

    printf("***********test cloud_fs by panic well!!!***********\r\n");
    return;
}

#if AOS_COMP_CLI
ALIOS_CLI_CMD_REGISTER(cloud_fs_test, cloud_fs_test, cloud fs test demo)
ALIOS_CLI_CMD_REGISTER(cloud_fs_init, cloud_fs_init, cloud fs init)
ALIOS_CLI_CMD_REGISTER(cloud_fs_dir_test, cloud_fs_dir_test, cloud fs dir test)
ALIOS_CLI_CMD_REGISTER(cloud_fs_file_test, cloud_fs_file_test, cloud fs file test)
#endif