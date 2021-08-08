#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include "vfs_types.h"
#include "vfs_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../cloud_fs_oss/oss_def.h"
#include "littlefs.h"
#include "cloud_fs_vfs.h"

#define LF_PATH "/data"
#define CLOUD_PATH "/cloud"

static int32_t cloud_vfs_open(vfs_file_t *fp, const char *filepath, int32_t flags) 
{
    return 0;

    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    std::string path = filepath;
    std::string tmpPath = path.substr(0, 6);
    if (tmpPath != CLOUD_PATH) {
        return -1;
    }
    std::string downloadFilePath = path.substr(6);
    std::string lfPath = LF_PATH + downloadFilePath;

    int fd = -1, buff[1024] = {0};
    char content[1030];

    if(flags & O_CREAT){
        int32_t new_flag = flags & (~O_CREAT);
        fd= lfs_vfs_open(&fp_lfs, lfPath.c_str(),new_flag);
    }

    if (fd <= 0) {
        // TODO: modify file exist inquire
        int ret;
        ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
        if (ret != 1) {
            printf("cloud has no such file!\r\n");

            fd= lfs_vfs_open(&fp_lfs, lfPath.c_str(),flags);
            return fd;
        }

        // TODO: modify download
        ret = cloud_fs_oss_downloadFile2File(const_cast<char*>(downloadFilePath.c_str()), NULL, const_cast<char*>(lfPath.c_str()));
        if (ret == 0) {
            fd= lfs_vfs_open(&fp_lfs, lfPath.c_str(),flags);
            return fd;
        }
        else {
            printf("download error!\r\n");
            return -2;
        }
    } 
    else {
        return fd;
    }
    // TODO: timer xx
}

static int32_t cloud_vfs_close(vfs_file_t *fp)
{
    // TODO: sync

    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;
   
    int32_t ret =lfs_vfs_close(&fp_lfs);
    return ret;
}

static int32_t cloud_vfs_read(vfs_file_t *fp, char *buf, uint32_t len)
{   
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    int32_t ret =lfs_vfs_read(&fp_lfs,buf,len);
    return ret;
}

static int32_t cloud_vfs_write(vfs_file_t *fp, const char *buf, uint32_t len)
{
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    int32_t ret =lfs_vfs_write(&fp_lfs, buf, len);
    return ret;
}

static int32_t cloud_vfs_sync(vfs_file_t *fp)
{
    // TODO:sync
    return 0;
}

static int32_t cloud_vfs_rename(vfs_file_t *fp,const char *oldpath, const char *newpath)
{
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    std::string oldPath = oldpath;
    std::string oldTmpPath = oldPath.substr(0, 6);
    if (oldTmpPath != CLOUD_PATH) {
        return -1;
    }
    std::string cloudOldFilePath = oldPath.substr(6);
    std::string oldLfPath = LF_PATH + cloudOldFilePath;

    std::string newPath = newpath;
    std::string newTmpPath = newPath.substr(0, 6);
    if (newTmpPath != CLOUD_PATH) {
        return -1;
    }
    std::string cloudNewFilePath = newPath.substr(6);
    std::string newLfPath = LF_PATH + newPath.substr(6);

    char content[1030];

    // TODO: file exist ?
    int ret = cloud_fs_oss_isFileExist(const_cast<char*>(cloudOldFilePath.c_str()), NULL);
    if (ret != 1) {
        printf("cloud doesn't  have the same file\r\n");
        return -1;
    }

    ret = cloud_fs_oss_downloadFile2File(const_cast<char*>(cloudOldFilePath.c_str()), NULL, const_cast<char*>(oldLfPath.c_str()));
    if (ret == 0) {
        ret = cloud_fs_oss_deleteFile(const_cast<char*>(cloudOldFilePath.c_str()), NULL);
        if (ret != 0) {
            printf("delete cloud file wrong!\r\n");
            return -7;
        }

        int32_t ru =lfs_vfs_rename(&fp_lfs, oldLfPath.c_str(), newLfPath.c_str());

        // TODO: timer upload        
        ret = cloud_fs_oss_uploadFile(const_cast<char*>(newLfPath.c_str()), NULL, const_cast<char*>(cloudNewFilePath.c_str()));
        if (ret != 0) {
            printf("upload file failed!\r\n");
            return -6;
        }
    }
    else {
        printf("download file error!\n");
        return -1;
    }
    return 0;
}

static  uint32_t  cloud_vfs_lseek(vfs_file_t *fp, int64_t off, int32_t whence)
{
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    uint32_t  ret =lfs_vfs_lseek(&fp_lfs, off, whence);
    return ret;
}

static int32_t cloud_vfs_remove(vfs_file_t *fp, const char *filepath)
{
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;

    std::string path = filepath;
    std::string tmpPath = path.substr(0, 6);
    if (tmpPath != CLOUD_PATH) {
        return -1;
    }
    std::string downloadFilePath = path.substr(6);
    std::string lfPath = LF_PATH + downloadFilePath;

    // TODO: file exist ?
    int ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret != 1) {
        return 0;
    }

    ret = cloud_fs_oss_deleteFile(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret != 0) {
        printf("delete cloud file wrong!\r\n");
        return -1;
    }

    return 0;
}

static vfs_filesystem_ops_t cloud_fs_ops;

int32_t cloud_fs_register(const char* cloudMonut)
{   
    cloud_fs_ops.open = &cloud_vfs_open;
    cloud_fs_ops.close = &cloud_vfs_close;
    cloud_fs_ops.read = &cloud_vfs_read;
    cloud_fs_ops.write = &cloud_vfs_write;
    cloud_fs_ops.lseek = &cloud_vfs_lseek;
    cloud_fs_ops.sync = &cloud_vfs_sync;
    cloud_fs_ops.remove = &cloud_vfs_remove;
    cloud_fs_ops.rename = &cloud_vfs_rename;

    int ret;
    ret = vfs_register_fs(cloudMonut,&cloud_fs_ops,NULL);
    printf("cloud_fs_register well!\n"); 
    return ret;
}
int32_t cloud_fs_unregister(const char *cloudMonut)
{
    int ret;
    ret = vfs_unregister_fs(cloudMonut);
    printf("cloud_fs_unregister well!\n"); 
    return ret;
}
