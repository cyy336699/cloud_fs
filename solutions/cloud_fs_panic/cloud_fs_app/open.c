#if AOS_COMP_CLI
#include "aos/cli.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <aos/errno.h>
#include <aos/kernel.h>
#include <aos/vfs.h>
#include "aos/init.h"
#include "board.h"
#include <k_api.h>


#define PATH "/data"
#define FILE PATH"/d1"
#define CONTENT "cloud demo\n"

void cloud_read() {
    int fd1, buff[128] = {0};
    fd1 = aos_open(FILE,  O_RDWR);
    if (fd1 <= 0) {
        printf("local has no file, fd = %d\n", fd1);
        printf("should oss download file\n");
        //simulate oss download file
        int fd2, ret;
        fd2 = aos_open(FILE,  O_CREAT | O_RDWR | O_APPEND);
        ret = aos_write(fd2, CONTENT, 11);
        if(ret > 0){
            printf("aos_write num=%d\n", ret);
        }else{
            printf("littlefs comp test fail! aos_write ret:%d\r\n", fd1);
        }
        fd1 = aos_open(FILE,  O_RDWR);
        ret = aos_read(fd1, buff, 128);
        if (ret > 0) {
            printf("aos_read: %s\n", buff);
        }else{
            printf("littlefs comp test fail! aos_read ret:%d\r\n", ret);
        }
    } 
    else {
        fd1 = aos_open(FILE,  O_RDWR);
        int ret;
        ret = aos_read(fd1, buff, 128);
        if (ret > 0) {
            printf("aos_read: %s\n", buff);
        }else{
            printf("littlefs comp test fail! aos_read ret:%d\r\n", ret);
        }
    }
}


#if AOS_COMP_CLI
/* reg args: fun, cmd, description*/
ALIOS_CLI_CMD_REGISTER(cloud_read, cloud_read, cloud_read_example)
#endif