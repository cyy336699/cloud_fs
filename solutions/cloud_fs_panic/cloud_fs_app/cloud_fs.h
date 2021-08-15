#define CONTENT "cloud demo\n"

int cloud_fs_read(char * filepath);
int cloud_fs_write(char * filepath, char * content);
int cloud_fs_touch(char * filepath);
int cloud_fs_rmfile(char * filepath);
int cloud_fs_lsfile(char * dirpath);
int cloud_fs_move(char  * oldpath, char * newpath);
int cloud_fs_cp(char  * oldpath, char * newpath);
int cloud_fs_mkdir(char *dirpath);
int cloud_fs_rmdir(char * dirpath);
void cloud_fs_syncall();
void cloud_fs_syncdir() ;