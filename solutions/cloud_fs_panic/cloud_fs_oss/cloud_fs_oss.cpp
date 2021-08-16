#include <iostream>
#include <fstream>
#include "string.h"
#include "oss_def.h"
#include <alibabacloud/oss/OssClient.h>
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

//用于进度条显示的回调函数
void ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData)
{
    std::cout << "ProgressCallback[" << userData << "] => " <<
    increment <<" ," << transfered << "," << total << std::endl;
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

    //设置进度条显示功能
    TransferProgress progressCallback = {ProgressCallback, nullptr};
    request.setTransferProgress(progressCallback);

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
 * upload content
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_uploadContent(char * bucketName, char * filename)
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

    memset(file_path,0,1024);
    pfile_path = filename;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::shared_ptr<std::iostream> content = std::make_shared<std::stringstream>();
    *content << "";

    PutObjectRequest request(BucketName, ObjectName, content);

    TransferProgress progressCallback = {ProgressCallback, nullptr};
    request.setTransferProgress(progressCallback);

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
 * upload specified file with breakpoint resume
 * @param localfilepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_uploadFile_breakpoint_resume(char * localfilepath, char * bucketName, char * filename)
{
    std::string BucketName;
    std::string ObjectName;
    std::string CheckpointFilePath = filename;
    CheckpointFilePath = "/data/checkpoint/" + CheckpointFilePath;
    std::string localfile_path = localfilepath;

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

    // ClientConfiguration conf;
    // OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    // UploadObjectRequest request(BucketName, ObjectName, localfile_path, CheckpointFilePath);
    // //将默认的分块大小8M调整为128K，以符合128K~128M使用断点续传的要求
    // request.setPartSize(128*1024);

    // //断点续传函数
    // auto outcome = client.ResumableUploadObject(request);

    // if (!outcome.isSuccess()) {
    //     /* 异常处理 */
    //     std::cout << "ResumableUploadObject fail" <<
    //     ",code:" << outcome.error().Code() <<
    //     ",message:" << outcome.error().Message() <<
    //     ",requestId:" << outcome.error().RequestId() << std::endl;
    //     ShutdownSdk();
    //     return -1;
    // }

    /* 释放网络等资源 */
    ShutdownSdk();
    return 0;
}

int64_t getFileSize(const std::string& file)
{
    std::fstream f(file, std::ios::in | std::ios::binary);
    f.seekg(0, f.end);
    int64_t size = f.tellg();
    f.close();
    return size;
}

/**
 * upload specified file with part upload
 * @param localfilepath: absolute path,e.g.: '/a/b/c/d.txt'
 * @param bucketName: If it is NULL, it defaults to Buckets
 * @return 0 for ok, negative number for error.
 */
int cloud_fs_oss_uploadFile_part_upload(char * localfilepath, char * bucketName, char * filename)
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

    if (localfilepath == NULL) 
    {
        return -1;
    }

    memset(file_path,0,1024);
    pfile_path = filename;
    strncpy(file_path,&pfile_path[1],strlen(pfile_path)-1);
    ObjectName = file_path;

    /* 初始化网络等资源 */
    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    InitiateMultipartUploadRequest initUploadRequest(BucketName, ObjectName);

    /* 初始化分片上传事件 */
    auto uploadIdResult = client.InitiateMultipartUpload(initUploadRequest);
    //获取唯一标识该分片上传任务的uploadId
    auto uploadId = uploadIdResult.result().UploadId();

    //A QUESTION：localfilepath是否就是localFileName?此处fileToUpload需要本地文件名
    std::string fileToUpload = localfilepath;

    //由于在文件大小大于128M时，才会使用分片上传，因此此处的分片大小选择为16MB
    int64_t partSize = 16 * 1024 * 1024;

    PartList partETagList;
    //获取文件大小
    auto fileSize = getFileSize(fileToUpload);

    int partCount = static_cast<int>(fileSize / partSize);
    /* 计算分片个数 */
    if (fileSize % partSize != 0) {
        partCount++;
    }

    /* 对每一个分片进行上传 */
    for (int i = 1; i <= partCount; i++) {
        auto skipBytes = partSize * (i - 1);
        auto size = (partSize < fileSize - skipBytes) ? partSize : (fileSize - skipBytes);
        std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(fileToUpload, std::ios::in|std::ios::binary);
        content->seekg(skipBytes, std::ios::beg);

        UploadPartRequest uploadPartRequest(BucketName, ObjectName, content);
        uploadPartRequest.setContentLength(size);
        uploadPartRequest.setUploadId(uploadId);
        uploadPartRequest.setPartNumber(i);
        auto uploadPartOutcome = client.UploadPart(uploadPartRequest);
        if (uploadPartOutcome.isSuccess()) {
            Part part(i, uploadPartOutcome.result().ETag());
            partETagList.push_back(part);
        }
        else {
            std::cout << "uploadPart fail" <<
            ",code:" << uploadPartOutcome.error().Code() <<
            ",message:" << uploadPartOutcome.error().Message() <<
            ",requestId:" << uploadPartOutcome.error().RequestId() << std::endl;
        }

    }

    /* 完成分片上传 */
    CompleteMultipartUploadRequest request(BucketName, ObjectName);
    request.setUploadId(uploadId);
    request.setPartList(partETagList);

    auto outcome = client.CompleteMultipartUpload(request);

    if (!outcome.isSuccess()) {
        /* 异常处理 */
        std::cout << "CompleteMultipartUpload fail" <<
        ",code:" << outcome.error().Code() <<
        ",message:" << outcome.error().Message() <<
        ",requestId:" << outcome.error().RequestId() << std::endl;
        ShutdownSdk();
        return -1;
    }

    /* 释放网络等资源 */
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
        // std::cout << "GetObjectToFile success" << outcome.result().Metadata().ContentLength() << std::endl;
        return 0;
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
    if (ObjectName.size() >= 1) {
        ObjectName = ObjectName.substr(1);
    }
    else {
        ObjectName = "";
    }

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
long int cloud_fs_oss_deleteDir(char * dirpath, char * bucketName, int flag)
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
    // printf("%s, %d\r\n", dirpath, flag);
    if (flag != 1) {
        if (dirpath == NULL) 
        {
            printf("dirpath == NULL");
            return -1;
        }

        memset(dir_path,0,1024);
        pdir_path = dirpath;
        strncpy(dir_path,&pdir_path[1],strlen(pdir_path)-1);
        ObjectName = dir_path;
        if (ObjectName[ObjectName.size() - 1] != '/') {
            ObjectName = ObjectName + "/";
        }
        // printf("ObjectName: %s", ObjectName.c_str());
    }
    else {
        ObjectName = "";
    }

    InitializeSdk();

    ClientConfiguration conf;
    OssClient client(Endpoint, AccessKeyId, AccessKeySecret, conf);

    std::string nextMarker = "";
    bool isTruncated = false;

    do {
            // printf("1\r\n");
            ListObjectsRequest request(BucketName);
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
                DeleteObjectRequest request(BucketName, object.Key());
                auto delResult = client.DeleteObject(request);
                // printf("2\r\n");
            }      
            nextMarker = outcome.result().NextMarker();
            isTruncated = outcome.result().IsTruncated();
    } while (isTruncated);

    ShutdownSdk();
    // printf("delete dir well!!\r\n");
    return 0;
}