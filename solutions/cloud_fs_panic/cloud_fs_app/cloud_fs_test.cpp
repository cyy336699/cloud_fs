#if AOS_COMP_CLI
#include "aos/cli.h"
#endif

#include <stdio.h>
#include "cloud_fs.h"
#include <assert.h>
#include <aos/vfs.h>
#include "cloud_fs_vfs.h"

extern Cloud_Dir cloud_main_dir;

static void cloud_fs_dir_init(int argc, char **argv)
{
    printf("***********start test cloud_fs_dir_init by panic***********\r\n");

    cloud_fs_register("/cloud");

    aos_rmdir("/cloud");
    aos_mkdir("/cloud/test");

    cloud_fs_dir_sync();
    cloud_main_dir.listDir(0);

    printf("***********test cloud_fs_dir_init by panic well!!!***********\r\n");
    return;    
}

static void cloud_fs_dir_test(int argc, char **argv)
{
    printf("***********start test cloud_fs_dirby panic***********\r\n");

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

    printf("***********test cloud_fs_dir by panic well!!!***********\r\n");
    return;    
}

static void cloud_fs_test(int argc, char **argv)
{
    printf("***********start test cloud_fs by panic***********\r\n");
    
    /*char buf[8] = {0};
    aos_open("/cloud/a.txt", 0);
    aos_close(1);
    aos_read(1, buf, 8);
    aos_write(1, "1234", 4);
    aos_sync(1);
    aos_rename("/cloud/a.txt", "/cloud/b.txt");
    aos_lseek(1, 0, 0);
    aos_remove("/cloud/a.txt");*/

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
ALIOS_CLI_CMD_REGISTER(cloud_fs_dir_init, cloud_fs_dir_init, cloud fs dir init)
ALIOS_CLI_CMD_REGISTER(cloud_fs_dir_test, cloud_fs_dir_test, cloud fs dir test)
#endif