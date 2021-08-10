#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>
#include "vfs_types.h"
#include "vfs_api.h"
#include "../cloud_fs_oss/oss_def.h"

int32_t cloud_fs_register(const char* cloudMonut);
int32_t cloud_fs_unregister(const char *cloudMonut);
void cloud_fs_dir_sync();

class Cloud_File
{
private:
    std::string name;

public:
    Cloud_File(std::string name) {
        this->name = name;
    }

    std::string getname() {
        return this->name;
    }
};

class Cloud_Dir
{
private:
    std::string name;
    int mainflag = 0;

    std::vector<Cloud_Dir> subdirs;
    std::vector<Cloud_File> subfiles;
public:
    int pos = 0;

    Cloud_Dir()  {}

    Cloud_Dir(std::string name) {
        this->name = name;
    }

    void setDir(std::string name, int mainflag) {
        this->name = name;
        this->mainflag = 1;
    }

    std::string getname() {
        return this->name;
    }

    int getnums() {
        return subfiles.size() + subdirs.size();
    }

    /**
     * 1 for exists, 0 for not exists
     */
    int isFileExists(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            // file
            std::string filename = name.substr(1);
            if (fileExists(filename)) {
                return 1;
            }
            return 0;
        }
        else {
            std::string dirname = name.substr(1, index);
            std::string thenname = name.substr(index);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return 0;
            }
            return subdirs[flag].isFileExists(thenname);
        }
    }

    int fileExists(std::string name) {
        int i = 0;
        for (i = 0; i < subfiles.size(); i++) {
            if (subfiles[i].getname() == name) {
                return 1;
            }
        }
        return 0;
    }

    int dirsync(std::string name) {
        subdirs.clear();
        subfiles.clear();

        std::string dirname = "";
        if (mainflag) {
            dirname = "/";
        }
        else if (name == "/") {
            dirname = "/" + this->name;
        }
        else {
            dirname = name + "/" + this->name;
        }

        std::vector<std::string> allFiles(1, "");
        std::vector<std::string> allDirs(1, "");
        int ret = cloud_fs_oss_listfiles(const_cast<char*>(dirname.c_str()), NULL, allFiles, allDirs);
        if (ret != 0) 
        {
            printf("get root dir wrong! Please wait minutes to rerun cloud_fs_dir_sync!\r\n");
            return 0;
        }

        for (std::string file : allFiles) 
        {
            if (file == "") {
                continue;
            }
            Cloud_File tmpfile(file);
            subfiles.push_back(tmpfile);
        }

        for (std::string dir : allDirs) {
            if (dir == "") {
                continue;
            }
            Cloud_Dir tmpdir(dir);
            if (!tmpdir.dirsync(dirname)) {
                return 0;
            }
            subdirs.push_back(tmpdir);
        }
        return 1;
    }

    int dirExists(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            std::string dirname = name.substr(1);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return -1;
            }
            return flag;
        }
        else {
            std::string dirname = name.substr(1, index);
            std::string thenname = name.substr(index);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return -1;
            }
            return subdirs[flag].dirExists(thenname);
        }
    }

    Cloud_Dir getdir(int flag) {
        return subdirs[flag];
    }

    std::string getsubname() {
        if (pos >= subfiles.size() + subdirs.size()) {
            return "";
        }
        if (pos < subfiles.size()) {
            return subfiles[pos].getname();
        }
        return subdirs[pos - subfiles.size()].getname();
    }

    void rmdir(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            std::string dirname = name.substr(1);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return ;
            }
            subdirs.erase(subdirs.begin() + flag);
            return ;
        }
        else {
            std::string dirname = name.substr(1, index);
            std::string thenname = name.substr(index);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return ;
            }
            subdirs[flag].rmdir(thenname);
            return ;
        }
    }

    void mkdir(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            std::string dirname = name.substr(1);
             int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag != -1) {
                return;
            }
            Cloud_Dir tmpdir(name);
            subdirs.push_back(tmpdir);
            return ;
        }
        else {
            std::string dirname = name.substr(1, index);
            std::string thenname = name.substr(index);
            int i = 0, flag = -1;
            for (i = 0; i < subdirs.size(); i++) {
                if (subdirs[i].getname() == dirname) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1) {
                Cloud_Dir tmpdir(dirname);
                tmpdir.mkdir(thenname);
                subdirs.push_back(tmpdir);
                return ;
            }
            subdirs[flag].mkdir(thenname);
            return ;
        }
    }
};

typedef struct _Cloud_Vfs_Dir{
    vfs_dir_t vdir;
    Cloud_Dir cloud_dir;
    vfs_dirent_t vdirent;
} Cloud_Vfs_Dir;

Cloud_Dir cloud_main_dir;