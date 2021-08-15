#pragma once
#include <string.h>
#include <vector>
#include <string>

static char AccessKeyId[] = "";
static char AccessKeySecret[] = "";
static char Endpoint[] = "";
static char Buckets[] = "";

int cloud_fs_oss_mkbucket(char* bucketName);
int cloud_fs_oss_isBucketExist(char* bucketName);
int cloud_fs_oss_deleteBucket(char* bucketName);
int cloud_fs_oss_uploadFile(char * localfilepath, char * bucketName, char * filename);
int cloud_fs_oss_uploadContent(char * bucketName, char * filename);
int cloud_fs_oss_uploadFile_breakpoint_resume(char * localfilepath, char * bucketName, char * filename);
int cloud_fs_oss_uploadFile_part_upload(char * localfilepath, char * bucketName, char * filename);
int cloud_fs_oss_downloadFile(char * filepath, char * bucketName, char * content);
int cloud_fs_oss_downloadFile2File(char * filepath, char * bucketName, char * localfilepath);
int cloud_fs_oss_isFileExist(char * filepath, char * bucketName);
long int cloud_fs_oss_getFileSize(char * filepath, char * bucketName);
int cloud_fs_oss_listallfiles(char * filepath, char * bucketName, std::vector<std::string> & allFiles);
int cloud_fs_oss_listfiles(char * filepath, char * bucketName, std::vector<std::string> & allFiles, std::vector<std::string> & allDirs);
long int cloud_fs_oss_deleteFile(char * filepath, char * bucketName);
long int cloud_fs_oss_deleteDir(char * dirpath, char * bucketName, int flag);