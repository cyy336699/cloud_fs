/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/init.h"
#include "board.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <aos/vfs.h>

//添加网络
#include "netmgr.h"
#include <uservice/uservice.h>
#include <uservice/eventid.h>

int application_start(int argc, char *argv[])
{
    int count = 0;

    aos_set_log_level(AOS_LL_DEBUG);
    event_service_init(NULL);
    netmgr_service_init(NULL); //添加网络

    aos_rmdir("/data/cloud");
    aos_mkdir("/data/cloud");

    printf("Everything inits well!\r\n");
    printf("**************************************************************\r\n");
    printf("\r\n");
    printf("               Welcome to Our Cloud File System!\r\n");
    printf("                                      --panic\r\n");
    printf("\r\n");
    printf("**************************************************************\r\n");

    while(1) {
        aos_msleep(1000);
    };
}