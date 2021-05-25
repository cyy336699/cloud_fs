#include <string.h>
#include <vector>
#include <string>

static char AccessKeyId[] = "";
static char AccessKeySecret[] = "";
static char Endpoint[] = "oss-cn-beijing.aliyuncs.com";
static char Buckets[] = "cloud-panic";

int cloud_fs_oss_mkbucket(char* bucketName);
int cloud_fs_oss_isBucketExist(char* bucketName);
int cloud_fs_oss_deleteBucket(char* bucketName);
int cloud_fs_oss_uploadFile(char * localfilepath, char * bucketName);
int cloud_fs_oss_downloadFile(char * filepath, char * bucketName, char * fileName);
int cloud_fs_oss_isFileExist(char * filepath, char * bucketName);
long int cloud_fs_oss_getFileSize(char * filepath, char * bucketName);
int cloud_fs_oss_listallfiles(char * filepath, char * bucketName, std::vector<std::string> & allFiles);
int cloud_fs_oss_listfiles(char * filepath, char * bucketName, std::vector<std::string> & allFiles, std::vector<std::string> & allDirs);
long int cloud_fs_oss_deleteFile(char * filepath, char * bucketName);
long int cloud_fs_oss_deleteDir(char * dirpath, char * bucketName);