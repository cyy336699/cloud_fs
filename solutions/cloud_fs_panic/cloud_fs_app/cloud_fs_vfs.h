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
            std::string dirname = name.substr(1, index - 1);
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
        else {
            dirname = name + this->name + "/";
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
            int tmp = file.find_last_of("/");
            if (tmp >= 0) {
                file = file.substr(tmp + 1);
            }
            if (file == "" || file == "_cloud_tmp_file.txt") {
                continue;
            }
            // printf("file: %s\n", file.c_str());
            Cloud_File tmpfile(file);
            subfiles.push_back(tmpfile);
        }

        for (std::string dir : allDirs) {
            if (dir[dir.size() - 1] == '/') {
                dir = dir.substr(0, dir.size() - 1);
            }
            int tmp = dir.find_last_of("/");
            if (tmp >= 0) {
                dir = dir.substr(tmp + 1);
            }
            if (dir == "") {
                continue;
            }
            // printf("dir: %s\n", dir.c_str());
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
            std::string dirname = name.substr(1, index - 1);
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
        if (pos >= subfiles.size() + subdirs.size() + 1) {
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
                    // printf("flag = %d\r\n", flag);
                    flag = i;
                    break;
                }
            }
            if (flag == -1) {
                return ;
            }
            // printf("start erase\r\n");
            subdirs.erase(subdirs.begin() + flag);
            // printf("erase end\r\n");
            return ;
        }
        else {
            std::string dirname = name.substr(1, index - 1);
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
            Cloud_Dir tmpdir(dirname);
            subdirs.push_back(tmpdir);
            return ;
        }
        else {
            std::string dirname = name.substr(1, index - 1);
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

    void removefile(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            // file
            std::string filename = name.substr(1);
            int i =0, flag = -1;
            for (i = 0; i < subfiles.size(); i++) {
                if (subfiles[i].getname() == filename) {
                    flag = i;
                    break;
                }
            }
            if (flag == -1 ) {
                return;
            }
            subfiles.erase(subfiles.begin() + flag);
            return;
        }
        else {
            std::string dirname = name.substr(1, index - 1);
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
            subdirs[flag].removefile(thenname);
            return;
        }
    }

    void mkfile(std::string name) {
        int index = name.find("/", 1);
        if (index <= 0) {
            std::string filename = name.substr(1);
             int i = 0, flag = -1;
            for (i = 0; i < subfiles.size(); i++) {
                if (subfiles[i].getname() == filename) {
                    flag = i;
                    break;
                }
            }
            if (flag != -1) {
                return;
            }
            Cloud_File tmpfile(filename);
            subfiles.push_back(tmpfile);
            return ;
        }
        else {
            std::string dirname = name.substr(1, index - 1);
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
                tmpdir.mkfile(thenname);
                subdirs.push_back(tmpdir);
                return ;
            }
            subdirs[flag].mkfile(thenname);
            return ;
        }
    }

    void listDir(int space) {
        int i = 0;
        for (i = 0; i < space; i++) {
            printf(" ");
        }
        printf("%s: ", this->name.c_str());
        for ( i =0 ; i < subfiles.size(); i++) {
            printf("%s", subfiles[i].getname().c_str());
            if (i < (subfiles.size() - 1)) {
                printf(", ");
            }
        }
        printf("\n");
        for (i = 0; i < subdirs.size(); i++) {
            subdirs[i].listDir(space + 4);
        }
    }
};

typedef struct _Cloud_Vfs_Dir{
    vfs_dir_t vdir;
    Cloud_Dir cloud_dir;
    vfs_dirent_t vdirent;
} Cloud_Vfs_Dir;
