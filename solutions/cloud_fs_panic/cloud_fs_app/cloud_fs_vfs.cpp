#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include <aos/vfs.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include "vfs_types.h"
#include "vfs_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <hash_map>
#include <math.h>

#include "../cloud_fs_oss/oss_def.h"
#include "littlefs.h"
#include "cloud_fs_vfs.h"

#include "delay_upload/timer.h"

#define LF_PATH "/data"
#define CLOUD_PATH "/cloud"

using namespace __gnu_cxx;

extern Cloud_Dir cloud_main_dir;
static vfs_filesystem_ops_t cloud_fs_ops;

//路径结构体，用于存储local路径和oss路径
struct oss_and_local {
    char* localfilepath;
    char* ossfilepath;
};

static hash_map <char*, oss_and_local*> fp_path_map; //用于存储fp为键值，路径结构体指针为value的hashmap
static hash_map <char*, Timer*> fp_timer_map; //用于存储fp为键值，Timer类指针为value的hashmap。用于标示哪些fp对应的文件正在上传（已绑定timer）

//该函数用于回调函数main_timer_call_func调用来将文件上传至OSS
//参数arg由于timer的要求，需要传入void*，函数内部再转换成vfs_file_t*
void oss_file_upload(void* arg) {
    vfs_file_t* temp_fp = (vfs_file_t*) arg;
    hash_map <char*, oss_and_local*>::iterator it = fp_path_map.find((char*) temp_fp);
    hash_map<char*, Timer*>::iterator iter = fp_timer_map.find((char*) temp_fp);
    if (it == fp_path_map.end()) { //没找到fp对应的路径信息
        printf("something goes wrong with fp_path_map, fp doesn't exist!\n");
        return;
    } else {
        //如果要进行上传，则先删除fp_timer_map中的fp-timer键值对，防止此时write发生错误
        if (iter == fp_timer_map.end()) {
            printf("fp-timer map doesn't exist!\n");
            return;
        } else {
            //删除已上传的fp对应的Timer类（计时器）
            fp_timer_map.erase(iter);
        }
        oss_and_local* temp = (oss_and_local*)aos_malloc(sizeof(oss_and_local));
        temp = it->second; //用temp存储fp对应的两个路径信息
        //调用oss上传函数，传入对应参数，将文件上传至OSS
        cloud_fs_oss_uploadFile(temp->localfilepath, NULL, temp->ossfilepath);
        //删除已上传的fp对应的路径结构体
        fp_path_map.erase(it);
        //释放临时申请的temp（路径结构体）
        aos_free(temp);
    }
}

//main_timer的延时回调函数
//在文件close之后，fd已经消失，因此利用fp的信息来定位文件并上传
void main_timer_call_func(void* timer, void* arg) {
    printf("It's time to upload file to OSS!\n");
    int ret = aos_task_new("upload_oss_task", oss_file_upload, arg, 8192);
    if (ret == 0) {
        printf("delay upload success!!!\n");
    }
    return;
}

//该函数会被ass_timer_call_func回调函数来调用，进行延迟时间的更新
//该函数在没有用户其他操作情况下，每15s触发一次（辅定时器到时），将延迟时间减少5s，加快上传进度
void flush_delay_time(void* arg) {
    vfs_file_t* fp = (vfs_file_t*) arg;
    //通过fp_timer_map找到fp对应的timer
    hash_map<char*, Timer*>::iterator iter = fp_timer_map.find((char*) fp);
    if (iter == fp_timer_map.end()) {
        printf("fp-timer map doesn't exist!\n");
        return;
    } else {
        Timer* timer = (Timer*)aos_malloc(sizeof(Timer));
        timer = iter->second;
        int delay_time = (*timer).get_delay_time();
        //将最近文件访问次数减少1次，避免多次不频繁的访问导致延迟时间过大
        (*timer).set_visit_times((*timer).get_visit_times()-1);
        //剩余延迟时间小于20s，则不再减少5s
        if(delay_time < 20) {
            ;
        } else { //剩余延迟时间大于20s，则将上一次记录的延迟时间减少20s（15s+5s),并更新主定时器的时间
            delay_time -= 20;
            (*timer).set_delay_time(delay_time);
            (*timer).reset_main_timer(delay_time*1000);
        }
        aos_free(timer);
    }
}

//用于ass_timer的回调函数
void ass_timer_call_func(void* timer, void* arg) {
    printf("it's time to reduce 5s!\n");
    int ret = aos_task_new("reduce_5s_task", flush_delay_time, arg, 8192);
    if (ret == 0) {
        printf("after 15s reduce 5s success!\n");
    }
    return;
}


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

    //使用新建的路径结构体，存储oss路径和local路径，并与fp指针建立map关系
    oss_and_local* temp = (oss_and_local*)aos_malloc(sizeof(oss_and_local));
    temp->localfilepath = const_cast<char *>(lfPath.c_str());
    temp->ossfilepath = const_cast<char *>(downloadFilePath.c_str());
    fp_path_map.insert(make_pair((char*) fp,  temp));
    //fwt新增代码end

    int fd = -1, buff[1024] = {0};
    char content[1030];

    if(flags & O_CREAT){
        int32_t new_flag = flags & (~O_CREAT);
        fd= lfs_vfs_open(&fp_lfs, lfPath.c_str(),new_flag);
    }

    if (fd <= 0) {
        int ret = cloud_main_dir.isFileExists(downloadFilePath);
        if (ret != 1) {
            printf("cloud has no such file!\r\n");

            fd= lfs_vfs_open(&fp_lfs, lfPath.c_str(),flags);
            return fd;
        }

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

    hash_map<char*, Timer*>::iterator it = fp_timer_map.find((char*)fp);
    if(it == fp_timer_map.end()) {
        ;
    } else {
        Timer* timer = (Timer*)aos_malloc(sizeof(Timer));
        timer = it->second;
        //由于写操作，对该文件的访问次数加1
        (*timer).set_visit_times((*timer).get_visit_times()+1);
        //停止主定时器，避免在没有关闭/刷新文件之前，自动上传至OSS，导致最近的修改丢失
        aos_timer_t temp_timer = (*timer).get_main_timer();
        aos_timer_stop(&temp_timer);
        //因为对文件进行了操作，因此15s的间隔时间重新计时
        (*timer).reset_ass_timer(15000);
    }
    return ret;
}

static int32_t cloud_vfs_sync(vfs_file_t *fp)
{
    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;
    //先在本地littlefs系统里进行同步
    int32_t ret =lfs_vfs_sync(&fp_lfs);

    printf("local sync is done!\n");

    //通过fp路径，在fp_timer_map中查找是否有对应的Timer类，若有则说明是再次访问，需要更新时间；若没有则新建一个Timer类
    hash_map<char*, Timer*>::iterator it = fp_timer_map.find((char*)fp);
    if (it == fp_timer_map.end()) {
        //创建一个定时器类
        Timer timer(fp);
        //初始化主定时器，回调函数是main_timer_call_func，倒数第二个参数：0代表单次执行，最后一个参数：1代表自动开始执行
        timer.init_main_timer(main_timer_call_func, (void*)timer.get_file(), 20000, 0, 1);
        //初始化辅定时器，回调函数是ass_timer_call_func，倒数第二个参数：1代表周期执行（每隔15s执行一次），最后一个参数：1代表自动开始执行
        timer.init_ass_timer(ass_timer_call_func, (void*)timer.get_file(), 15000, 1, 1);
        //将新建的Timer类加入hashmap中，键值为fp
        fp_timer_map.insert(make_pair((char*) fp, &timer));
    } else {
        Timer* timer = (Timer*)aos_malloc(sizeof(Timer));
        timer = it->second;
        //由于刷新操作，因此对于该文件的访问次数加1
        (*timer).set_visit_times((*timer).get_visit_times()+1);
        //根据公式重新计算延迟时间
        int new_delay_time = 20 * pow(2, (*timer).get_visit_times());
        (*timer).set_delay_time(new_delay_time);
        //刷新主定时器的延迟时间
        (*timer).reset_main_timer(new_delay_time*1000);
        //也要让辅定时器重新从0开始计时
        (*timer).reset_ass_timer(15000);
        aos_free(timer);
    }
    return ret;
}

static int32_t cloud_vfs_close(vfs_file_t *fp)
{
    cloud_vfs_sync(fp);

    vfs_file_t fp_lfs =*fp;
    fp_lfs.node->i_name=(char*)LF_PATH;
   
    int32_t ret =lfs_vfs_close(&fp_lfs);
    return ret;
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

    int ret = cloud_main_dir.isFileExists(cloudNewFilePath);
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

        // TODO: set a timer to upload file
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

    cloud_main_dir.removefile(cloudOldFilePath);
    cloud_main_dir.mkfile(cloudNewFilePath);
    
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

    int ret = cloud_main_dir.isFileExists(downloadFilePath);
    if (ret != 1) {
        return 0;
    }

    ret = cloud_fs_oss_deleteFile(const_cast<char*>(downloadFilePath.c_str()), NULL);
    if (ret != 0) {
        printf("delete cloud file wrong!\r\n");
        return -1;
    }

    cloud_main_dir.removefile(downloadFilePath);

    return 0;
}

static vfs_dir_t *cloud_vfs_opendir(vfs_file_t *fp, const char *path)
{
    std::string dirname = path;
    int flag = cloud_main_dir.dirExists(dirname);
    if (flag == -1) {
        return NULL;
    }

    Cloud_Vfs_Dir * cloud_vfs_dir = new Cloud_Vfs_Dir;
    if(!cloud_vfs_dir){
        return NULL;
    }
    cloud_vfs_dir->cloud_dir = cloud_main_dir.getdir(flag);
    fp->f_arg=(void*)cloud_vfs_dir;
    return (vfs_dir_t *)cloud_vfs_dir;
}

static vfs_dirent_t *cloud_vfs_readdir(vfs_file_t *fp, vfs_dir_t *dir){
    Cloud_Vfs_Dir * cloud_fs_dir = (Cloud_Vfs_Dir*)dir;
    if (!cloud_fs_dir) {
        return NULL;
    }
    if(cloud_fs_dir->cloud_dir.pos>=cloud_fs_dir->cloud_dir.getnums()){
        return NULL;
    }
    cloud_fs_dir->vdirent.d_ino = 0;
    cloud_fs_dir->vdirent.d_type = 0;

    std::string name = cloud_fs_dir->cloud_dir.getsubname() + "\0";

    strncpy(cloud_fs_dir->vdirent.d_name, name.c_str(), 128);

    return &cloud_fs_dir->vdirent;
}

static int32_t cloud_vfs_closedir(vfs_file_t *fp, vfs_dir_t *dir){
    Cloud_Vfs_Dir * cloud_fs_dir = (Cloud_Vfs_Dir*)dir;
    if (!cloud_fs_dir) {
        return NULL;
    }
    delete cloud_fs_dir;
    return 0;
}

static void cloud_vfs_rewinddir(vfs_file_t *fp, vfs_dir_t *dir)
{
    Cloud_Vfs_Dir * cloud_fs_dir = (Cloud_Vfs_Dir*)dir;
    if (!cloud_fs_dir) {
        return;
    }
    cloud_fs_dir->cloud_dir.pos=0;
    return;
}

static int32_t cloud_vfs_telldir(vfs_file_t *fp, vfs_dir_t *dir)
{
    Cloud_Vfs_Dir * cloud_fs_dir = (Cloud_Vfs_Dir*)dir;
    if (!cloud_fs_dir) {
        return -1;
    }

    return cloud_fs_dir->cloud_dir.pos;
}

static void cloud_vfs_seekdir(vfs_file_t *fp, vfs_dir_t *dir, int32_t loc)
{
     Cloud_Vfs_Dir * cloud_fs_dir = (Cloud_Vfs_Dir*)dir;
    if (!cloud_fs_dir) {
        return ;
    }

    if(loc >= cloud_fs_dir->cloud_dir.getnums()){
            printf("cloud_vfs_seekdir: dir overflow!!!\r\n");
            return;
    }
    cloud_fs_dir->cloud_dir.pos=loc;
    return ;
}

static int32_t cloud_vfs_mkdir(vfs_file_t *fp, const char *path) {
    std::string name = path;
    cloud_main_dir.mkdir(path);

    std::string localpath = "/data/_cloud_tmp_file.txt";
    std::string filepath = name + "/_cloud_tmp_file.txt";
    int flag = cloud_fs_oss_uploadFile(const_cast<char *>(localpath.c_str()), NULL, const_cast<char *>(filepath.c_str()));
    if (flag < 0) {
        printf("mkdir has something wrong!!!\r\n");
        return 0;
    }
    printf("mkdir well!!!\r\n");
    return 0;
}

static int32_t cloud_vfs_rmdir(vfs_file_t *fp, const char *path) {
    std::string name = path;
    cloud_main_dir.rmdir(path);

    if (cloud_fs_oss_deleteDir(const_cast<char*>(path), NULL) < 0 ) {
        printf("rmdir has something wrong!!!\r\n");
        return 0;
    }
    printf("rmdir well!!!\r\n");
    return 0;
}

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
    cloud_fs_ops.opendir = &cloud_vfs_opendir;
    cloud_fs_ops.readdir = &cloud_vfs_readdir;
    cloud_fs_ops.closedir = &cloud_vfs_closedir;
    cloud_fs_ops.mkdir = &cloud_vfs_mkdir;
    cloud_fs_ops.rmdir = &cloud_vfs_rmdir;
    cloud_fs_ops.rewinddir = &cloud_vfs_rewinddir;
    cloud_fs_ops.telldir = &cloud_vfs_telldir;
    cloud_fs_ops.seekdir = &cloud_vfs_seekdir;

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

void cloud_fs_dir_sync() {
    cloud_main_dir.setDir("/cloud", 1);

    int fd = aos_open("/data/_cloud_tmp_file.txt", O_CREAT | O_RDWR);
    aos_close(fd);

    cloud_main_dir.dirsync("/");
}