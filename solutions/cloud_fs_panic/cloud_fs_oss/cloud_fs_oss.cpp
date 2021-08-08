#include <iostream>
#include <fstream>
#include "string.h"
#include "oss_def.h"
#include "alibabacloud/oss/OssClient.h"
#include <vector>

using namespace AlibabaCloud::OSS;

/**
 * create Buckets, default Standard and private
 * @return 0 for ok, negative number for error
 */
int cloud_fs_oss_mkbucket(char* bucketName)
{
    int flag = 0;
    if (bucketName == NULL) 
    {
        return -2;
    }

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    CreateBucketRequest request(bucketName, StorageClass::Standard, CannedAccessControlList::Private);

    auto outcome = client.CreateBucket(request);

    if (!outcome.isSuccess()) {
        std::cout << "CreateBucket fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        flag = -1;
    }

    ShutdownSdk();
    return flag;
}

/**
 * determine whether the storage space exists
 * @return 1 for exist, 0 for not exist, negative number for error.
 */
int cloud_fs_oss_isBucketExist(char* bucketName)
{
    int flag = -1;
    if (bucketName == NULL) 
    {
        return -2;
    }

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    auto outcome = client.DoesBucketExist(bucketName);

    if (outcome) {    
        // std::cout << " The Buckets exists" << std::endl;
        flag = 1;
    }
    else {
        // std::cout << "The Buckets does not exist" << std::endl;
        flag = 0;
    }

    ShutdownSdk();
    return flag;
}

/**
 * delete specified Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_deleteBucket(char* bucketName)
{
    int flag = -1;
    if (bucketName == NULL) 
    {
        return -2;
    }

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    DeleteBucketRequest request(bucketName);

    auto outcome = client.DeleteBucket(request);

    if (!outcome.isSuccess()) {
        std::cout << "DeleteBucket fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -1;
    }

    ShutdownSdk();
    return 0;
}

/**
 * upload specified file
 * @param localfilepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_uploadFile(char * localfilepath, char * bucketName, char * filename)
{
    std::string BucketName;
    std::string ObjectName ;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (localfilepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filename;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(localfilepath, std::ios::in | std::ios::binary);
    PutObjectRequest request(BucketName, ObjectName, content);

    auto outcome = client.PutObject(request);

    if (!outcome.isSuccess()) {
        std::cout << "PutObject fail" <<
                  ",code:" << outcome.error().Code() <<
                  ",message:" << outcome.error().Message() <<
                  ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -2;
    }

    ShutdownSdk();
    return 0;
}


/**
 * download file to specified file
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_downloadFile2File(char * filepath, char * bucketName, char * localfilepath)
{
    std::string BucketName;
    std::string ObjectName;
    std::string FileNametoSave;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    GetObjectRequest request(BucketName, ObjectName);
    request.setResponseStreamFactory([=]() {return std::make_shared<std::fstream>(localfilepath, std::ios_base::out | std::ios_base::in | std::ios_base::trunc| std::ios_base::binary); });

    auto outcome = client.GetObject(request);

    if (outcome.isSuccess()) {    
        std::cout << "GetObjectToFile success" << outcome.result().Metadata().ContentLength() << std::endl;
    }
    else {
        /*异常处理*/
        std::cout << "GetObjectToFile fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -1;
    }
}

/**
 * download file to specified file
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_downloadFile(char * filepath, char * bucketName, char * content)
{
    std::string BucketName;
    std::string ObjectName;
    std::string FileNametoSave;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    GetObjectRequest request(BucketName, ObjectName);
    auto outcome = client.GetObject(request);

    if (outcome.isSuccess()) {
        auto& stream = outcome.result().Content();
        // printf("download file well!\n");
        if (stream->good()) {
            stream->read(content, 1024);
            ShutdownSdk();
            // printf("store file well!\n");
            return 0;
	    }
        else 
        {
            printf("Somwthing wrong!\n");
            ShutdownSdk();
            return -3;
        }
    }
    else {
        std::cout << "getObjectToBuffer fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -1;
    }
}

/**
 * determine whether the file exists
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 1 for exist, 0 for not exist, negative number for error
 */
int cloud_fs_oss_isFileExist(char * filepath, char * bucketName)
{
    std::string BucketName;
    std::string ObjectName;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);
  
    auto outcome = client.DoesObjectExist(BucketName, ObjectName);

    if (outcome) {
        //   std::cout << "The Object exists!" << std::endl;         
        ShutdownSdk();
        return 1;              
    }                              
    else {                         
        //   std::cout << "The Object does not exist!" << std::endl;
        ShutdownSdk();
        return 0;
    }
}

/**
 * get file size
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return positive number for length, negative number for error.
 */
long int cloud_fs_oss_getFileSize(char * filepath, char * bucketName)
{
    std::string BucketName;
    std::string ObjectName;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    /* 获取文件的部分元信息 */
    auto outcome = client.GetObjectMeta(BucketName, ObjectName);

    if (!outcome.isSuccess()) {
        std::cout << "GetObjectMeta fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -2;
    }
    else { 
        auto metadata = outcome.result();
        ShutdownSdk();
        return metadata.ContentLength();
    }

    // /* 获取文件的全部元信息 */
    // outcome = client.HeadObject(BucketName, ObjectName);

    // if (!outcome.isSuccess()) {
    //     std::cout << "HeadObject fail" <<
    //     ",code:" << outcome.error().Code() <<
    //     ",message:" << outcome.error().Message() <<
    //     ",requestId:" << outcome.error().RequestId() << std::endl;
    //     ShutdownSdk();
    //     return -1;
    // }
    // else { 
    //     auto headMeta = outcome.result();
    //     std::cout <<"headMeta success, ContentType:" 
    //     << headMeta.ContentType() << "; ContentLength:" << headMeta.ContentLength() 
    //     << "; CacheControl:" << headMeta.CacheControl() << std::endl;
    // }
}

/**
 * list all files
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @param allfiles: record all file names
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_listallfiles(char * filepath, char * bucketName, std::vector<std::string>& allFiles)
{
    std::string BucketName;
    std::string ObjectName;

    char *pfile_path,file_path[1024];

    allFiles.clear();

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::string nextMarker = "";
    bool isTruncated = false; 
    do {
        ListObjectsRequest request(BucketName);
        request.setMarker(nextMarker);
        auto outcome = client.ListObjects(request);

        if (!outcome.isSuccess()) {    
            std::cout << "ListObjects fail" <<
            ",code:" << outcome.error().Code() <<
            ",message:" << outcome.error().Message() <<
            ",requestId:" << outcome.error().RequestId() << std::endl;
            ShutdownSdk();
            return -2;  
        }
        else {
            for (const auto& object : outcome.result().ObjectSummarys()) {
                allFiles.push_back(object.Key());
            }      
        }
        nextMarker = outcome.result().NextMarker();
        isTruncated = outcome.result().IsTruncated();
    } while (isTruncated);

    ShutdownSdk();
    return 0;
}

/**
 * list files and subdirectories under the directory
 * @param dirpath: absolute path,e.g.: 'a/b/'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @param allFiles: record all file names
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_listfiles(char * filepath, char * bucketName, std::vector<std::string>& allFiles, std::vector<std::string>& allDirs)
{
    std::string BucketName;
    std::string ObjectName;

    char *pfile_path,file_path[1024];

    allFiles.clear();
    allDirs.clear();

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[0],strlen(pfile_path));
    ObjectName = file_path;

     InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::string nextMarker = "";
    bool isTruncated = false;
    do {
            ListObjectsRequest request(BucketName);
            /* 设置正斜线（/）为文件夹的分隔符 */
            request.setDelimiter("/");
            request.setPrefix(ObjectName);
            request.setMarker(nextMarker);
            auto outcome = client.ListObjects(request);

            if (!outcome.isSuccess()) {    
                std::cout << "ListObjects fail" <<
                ",code:" << outcome.error().Code() <<
                ",message:" << outcome.error().Message() <<
                ",requestId:" << outcome.error().RequestId() << std::endl;
                ShutdownSdk();
                return -1;
            }  
            for (const auto& object : outcome.result().ObjectSummarys()) {
                // std::cout << "object"<<
                // ",name:" << object.Key() <<
                // ",size:" << object.Size() <<
                // ",lastmodify time:" << object.LastModified() << std::endl;
                allFiles.push_back(object.Key());
            }
            for (const auto& commonPrefix : outcome.result().CommonPrefixes()) {
                // std::cout << "commonPrefix" << ",name:" << commonPrefix << std::endl;
                allDirs.push_back(commonPrefix);
            }
            nextMarker = outcome.result().NextMarker();
            isTruncated = outcome.result().IsTruncated();
    } while (isTruncated);

    ShutdownSdk();
    return 0;
}

/**
 * delete the specified file
 * @param filepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
long int cloud_fs_oss_deleteFile(char * filepath, char * bucketName)
{
    std::string BucketName;
    std::string ObjectName;

    char *pfile_path,file_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (filepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filepath;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    DeleteObjectRequest request(BucketName, ObjectName);

    auto outcome = client.DeleteObject(request);

    if (!outcome.isSuccess()) {
        std::cout << "DeleteObject fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -2;
    }

    ShutdownSdk();
    return 0;
}

/**
 * delete the dir
 * @param dirpath: absolute path,e.g.: '/a/b/c/'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
long int cloud_fs_oss_deleteDir(char * dirpath, char * bucketName)
{
    std::string BucketName;
    std::string ObjectName;

    char *pdir_path,dir_path[1024];

    if (bucketName == NULL) 
    {
        BucketName = Buckets;
    }
    else 
    {
        BucketName = bucketName;
    }

    if (dirpath == NULL) 
    {
        return -1;
    }

    memset(dir_path,0,1024);
    pdir_path = dirpath;
    strncpy(dir_path,&pdir_path[1],strlen(pdir_path)-1);
    ObjectName = dir_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::string nextMarker = "";
    bool isTruncated = false;

    do {
            ListObjectsRequest request(BucketName);
            request.setPrefix(dirpath);
            request.setMarker(nextMarker);
            auto outcome = client.ListObjects(request);

            if (!outcome.isSuccess()) {    
                std::cout << "ListObjects fail" <<
                ",code:" << outcome.error().Code() <<
                ",message:" << outcome.error().Message() <<
                ",requestId:" << outcome.error().RequestId() << std::endl;
                ShutdownSdk();
                return -1;
            }
            for (const auto& object : outcome.result().ObjectSummarys()) {
                DeleteObjectRequest request(BucketName, object.Key());
                auto delResult = client.DeleteObject(request);
            }      
            nextMarker = outcome.result().NextMarker();
            isTruncated = outcome.result().IsTruncated();
    } while (isTruncated);

    ShutdownSdk();
    return 0;
}