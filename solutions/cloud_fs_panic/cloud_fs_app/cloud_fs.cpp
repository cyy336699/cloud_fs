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

int cloud_fs_read(char * filepath)
{
    std::string path = filepath;
    path = PATH + path;
    std::string downloadFilePath = filepath;
    downloadFilePath = "/" + downloadFilePath;

    int fd1, buff[1024] = {0};
    char content[1030];

    fd1 = aos_open(path.c_str(),  O_RDWR);
    if (fd1 <= 0) {
        // printf("local has no such file\r\n");
        // printf("should oss download file\r\n");
        
        int ret;
        ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
        if (ret != 1) {
            printf("cloud has no such file!\r\n");
            return -1;
        }
        ret = cloud_fs_oss_downloadFile(const_cast<char*>(downloadFilePath.c_str()), NULL, content);
        if (ret == 0) {
            // printf("download OK!\r\n");
            // printf("filepath:%s\r\n", const_cast<char*>(path.c_str()));
            fd1 = aos_open(path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
            if (fd1 <= 0) {
                printf("open file wrong!\r\n");
                return -2;
            }
            // printf("content:%s\r\n", content);
            ret = aos_write(fd1, content, 1024);
            if(ret <= 0)
            {
                printf("write local file wrong! ret:%d\r\r\n", ret);
                return -3;
            }
            aos_close(fd1);
            
            fd1 = aos_open(path.c_str(),  O_RDWR);
            if (fd1 <= 0) {
                printf("open local file wrong\r\n");
                return -2;
            }
            ret = aos_read(fd1, buff, 1024);
            if (ret > 0) {
                printf("cloud_read: %s\r\n", buff);
            }else{
                printf("cloud_read file wrong! ret:%d\r\r\n", ret);
                return -4;
            }
            aos_close(fd1);
        }
        else {
            printf("download error!\r\n");
            return -5;
        }
    } 
    else {
        int ret;
        ret = aos_read(fd1, buff, 1024);
        if (ret > 0) {
            printf("cloud_read: %s\r\n", buff);
        }else{
            printf("cloud_read file wrong! ret:%d\r\r\n", ret);
            return -4;
        }
        aos_close(fd1);
    }
    return 0;
}

int cloud_fs_write(char * filepath, char * content)
{
    std::string path = filepath;
    path = PATH + path;
    std::string downloadFilePath = filepath;
    downloadFilePath = "/" + downloadFilePath;

    int fd1, buff[1024] = {0}, ret;

    fd1 = aos_open(path.c_str(),  O_RDWR);

    if (fd1 > 0) {
        ret = aos_close(fd1);
        if (ret < 0) {
            printf("close file failed\r\n");
            return -1;
        }
        ret = aos_remove(path.c_str());
        if (ret < 0) {
            printf("remove file failed\r\n");
            return -2;
        }
    }

    fd1 = aos_open(path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
    if (fd1 <= 0) {
        printf("open file wrong!\r\n");
        return -3;
    }
    ret = aos_write(fd1, content, 1024);
    if (ret <= 0) {
        printf("write file wrong!\r\n");
        return -4;
    }
    aos_close(fd1);

    ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret == 1) {
        ret = cloud_fs_oss_deleteFile(const_cast<char*>(downloadFilePath.c_str()), NULL);
        if (ret != 0) {
            printf("delete file wrong!\r\n");
            return -5;
        }
    }

    ret = cloud_fs_oss_uploadFile(const_cast<char*>(path.c_str()), NULL, const_cast<char*>(downloadFilePath.c_str()));
    if (ret != 0) {
        printf("upload file failed!\r\n");
        return -6;
    }

    return 0;
}

int cloud_fs_touch(char * filepath)
{
    std::string path = filepath;
    path = PATH + path;
    std::string downloadFilePath = filepath;
    downloadFilePath = "/" + downloadFilePath;

    int fd1, buff[1024] = {0}, ret;

    ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret == 1) {
        printf("cloud already has the same file\r\n");
        return -1;
    }

    fd1 = aos_open(path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
    if (fd1 <= 0) {
        printf("open file wrong!\r\n");
        return -3;
    }
    ret = aos_write(fd1, "", 0);
    if (ret < 0) {
        printf("write file wrong!\r\n");
        return -4;
    }
    aos_close(fd1);

    ret = cloud_fs_oss_uploadFile(const_cast<char*>(path.c_str()), NULL, const_cast<char*>(downloadFilePath.c_str()));
    if (ret != 0) {
        printf("upload file failed!\r\n");
        return -6;
    }

    return 0;
}

int cloud_fs_rmfile(char * filepath)
{
    std::string path = filepath;
    path = PATH + path;
    std::string downloadFilePath = filepath;
    downloadFilePath = "/" + downloadFilePath;

    int fd1, buff[1024] = {0}, ret;

    fd1 = aos_open(path.c_str(),  O_RDWR);

    if (fd1 > 0) {
        ret = aos_close(fd1);
        if (ret < 0) {
            printf("close file failed\r\n");
            return -1;
        }
        ret = aos_remove(path.c_str());
        if (ret < 0) {
            printf("remove file failed\r\n");
            return -2;
        }
    }

    ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret != 1) {
        printf("cloud doesn't  have the same file, delete file wrong\r\n");
        return -3;
    }

    ret = cloud_fs_oss_deleteFile(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret != 0) {
        printf("delete cloud file wrong!\r\n");
        return -4;
    }

    return 0;
}

int cloud_fs_lsfile()
{
    std::vector<std::string> allFiles(1, "");
    int ret = cloud_fs_oss_listallfiles("/", NULL, allFiles);
    if (ret != 0) 
    {
        printf("get files list wrong!\r\n");
        return -1;
    }

    for (std::string file : allFiles) 
    {
        if (file == "") {
            continue;
        }
        printf("%s ", file.c_str());
    }
    printf("\r\n");

    return 0;
}

int cloud_fs_move(char  * oldpath, char * newpath)
{
    std::string old_path = oldpath;
    old_path = PATH + old_path;
    std::string downloadOldFilePath = oldpath;
    downloadOldFilePath = "/" + downloadOldFilePath;

    std::string new_path = newpath;
    new_path = PATH + new_path;
    std::string downloadNewFilePath = newpath;
    downloadNewFilePath = "/" + downloadNewFilePath;

    int fd1, buff[1024] = {0}, ret;
    char content[1030];

    ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadOldFilePath.c_str()), NULL);
    if (ret != 1) {
        printf("cloud doesn't  have the same file\r\n");
        return -1;
    }

    ret = cloud_fs_oss_downloadFile(const_cast<char*>(downloadOldFilePath.c_str()), NULL, content);
    if (ret == 0) {
        ret = cloud_fs_oss_deleteFile(const_cast<char*>(downloadOldFilePath.c_str()), NULL);
        if (ret != 0) {
            printf("delete cloud file wrong!\r\n");
            return -7;
        }

        fd1 = aos_open(old_path.c_str(),  O_RDWR);
        if (fd1 >= 0) {
            ret = aos_close(fd1);
            if (ret < 0) {
                printf("file close error\r\n");
                return -2;
            }
            ret = aos_remove(old_path.c_str());
            if (ret < 0) {
                printf("file remove error\r\n");
                return -3;
            }
        }

        fd1 = aos_open(new_path.c_str(),  O_RDWR);
        if (fd1 >= 0) {
            ret = aos_close(fd1);
            if (ret < 0) {
                printf("file close error\r\n");
                return -2;
            }
            ret = aos_remove(new_path.c_str());
            if (ret < 0) {
                printf("file remove error\r\n");
                return -3;
            }
        }
        
        fd1 = aos_open(new_path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
        if (fd1 <= 0) {
            printf("open file wrong!\r\n");
            return -4;
        }
        // printf("content:%s\r\n", content);
        ret = aos_write(fd1, content, 1024);
        if(ret <= 0)
        {
            printf("write local file wrong! ret:%d\r\r\n", ret);
            return -5;
        }
        aos_close(fd1);

        ret = cloud_fs_oss_uploadFile(const_cast<char*>(new_path.c_str()), NULL, const_cast<char*>(downloadNewFilePath.c_str()));
        if (ret != 0) {
            printf("upload file failed!\r\n");
            return -6;
        }
    }
    else {
        printf("download error!\r\n");
        return -8;
    }

    return 0;
}

int cloud_fs_cp(char  * oldpath, char * newpath)
{
    std::string old_path = oldpath;
    old_path = PATH + old_path;
    std::string downloadOldFilePath = oldpath;
    downloadOldFilePath = "/" + downloadOldFilePath;

    std::string new_path = newpath;
    new_path = PATH + new_path;
    std::string downloadNewFilePath = newpath;
    downloadNewFilePath = "/" + downloadNewFilePath;

    int fd1, buff[1024] = {0}, ret;
    char content[1030];

    ret = cloud_fs_oss_isFileExist(const_cast<char*>(downloadOldFilePath.c_str()), NULL);
    if (ret != 1) {
        printf("cloud doesn't  have the same file\r\n");
        return -1;
    }

    ret = cloud_fs_oss_downloadFile(const_cast<char*>(downloadOldFilePath.c_str()), NULL, content);
    if (ret == 0) {
        fd1 = aos_open(old_path.c_str(),  O_RDWR);
        if (fd1 >= 0) {
            ret = aos_close(fd1);
            if (ret < 0) {
                printf("file close error\r\n");
                return -2;
            }
            ret = aos_remove(old_path.c_str());
            if (ret < 0) {
                printf("file remove error\r\n");
                return -3;
            }
        }

        fd1 = aos_open(old_path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
        if (fd1 <= 0) {
            printf("open file wrong!\r\n");
            return -4;
        }
        // printf("content:%s\r\n", content);
        ret = aos_write(fd1, content, 1024);
        if(ret <= 0)
        {
            printf("write local file wrong! ret:%d\r\r\n", ret);
            return -5;
        }
        aos_close(fd1);

        fd1 = aos_open(new_path.c_str(),  O_RDWR);
        if (fd1 >= 0) {
            ret = aos_close(fd1);
            if (ret < 0) {
                printf("file close error\r\n");
                return -2;
            }
            ret = aos_remove(new_path.c_str());
            if (ret < 0) {
                printf("file remove error\r\n");
                return -3;
            }
        }
        
        fd1 = aos_open(new_path.c_str(),  O_CREAT | O_RDWR | O_APPEND);
        if (fd1 <= 0) {
            printf("open file wrong!\r\n");
            return -4;
        }
        // printf("content:%s\r\n", content);
        ret = aos_write(fd1, content, 1024);
        if(ret <= 0)
        {
            printf("write local file wrong! ret:%d\r\r\n", ret);
            return -5;
        }
        aos_close(fd1);

        ret = cloud_fs_oss_uploadFile(const_cast<char*>(new_path.c_str()), NULL, const_cast<char*>(downloadNewFilePath.c_str()));
        if (ret != 0) {
            printf("upload file failed!\r\n");
            return -6;
        }
    }
    else {
        printf("download error!\r\n");
        return -8;
    }

    return 0;
}