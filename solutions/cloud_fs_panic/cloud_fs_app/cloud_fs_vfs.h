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
                if (subdirs[i].getname() == name) {
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

};

Cloud_Dir cloud_main_dir;