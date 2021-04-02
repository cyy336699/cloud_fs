/*
 * Copyright (C) 2021-2023 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <stdio.h>
#include "ugraphics.h"

#ifdef AOS_COMP_CLI
#include "aos/cli.h"
#endif

#define WIDTH  SCREEN_W
#define HEIGHT SCREEN_H

static void ugraphics_comp_example(int argc, char **argv)
{
    int ret;

    if (argc < 2)
        printf("wrong parameter number\n");



    if (!strncmp(argv[1], "init", 4)) {
        /*Initialize ugraphics window*/
        ret = ugraphics_init(WIDTH, HEIGHT);
        if (ret < 0) {
            printf("ugraphics init fail, ret: %d\n", ret);
            return;
        }
    } else if (!strncmp(argv[1], "draw", 4)) {
        /*Clear buffer on screen*/
        ugraphics_clear();
        if (!strncmp(argv[2], "rect", 4)) {
            /*Draw empty rectangle*/
            ugraphics_draw_rect(0, 0, WIDTH / 2, HEIGHT / 2);
            printf("ugraphics draw rectangle ok!\n");
        } else if (!strncmp(argv[2], "line", 4)) {
            /*Draw line*/
            ugraphics_draw_line(0, HEIGHT / 4, WIDTH, HEIGHT / 4);
            printf("ugraphics draw line ok!\n");
        } else if (!strncmp(argv[2], "jpg", 3)) {
            /*Draw image*/
            ugraphics_draw_image("/data/ugraphics_image/object.jpg", 0, 0);
            printf("ugraphics draw jpg ok!\n");
        } else if (!strncmp(argv[2], "png", 3)) {
            /*Draw image*/
            ugraphics_draw_image("/data/ugraphics_image/anime.png", 0, 0);
            printf("ugraphics draw png ok!\n");
        } else if (!strncmp(argv[2], "string", 6)) {
            /*Load default ttf font*/
            ret = ugraphics_load_font("/data/ugraphics_image/font/Alibaba-PuHuiTi-Heavy.ttf", 18);
            if (ret < 0) {
                printf("ugraphics load font fail, ret: %d\n", ret);
                return;
            }

            /*Set color*/
            ugraphics_set_color(COLOR_BLACK);
            /*Set font style*/
            ugraphics_set_font_style(UGRAPHICS_FONT_STYLE);

            /*Draw string*/
            ugraphics_draw_string("Welcome to AliOS Things!", WIDTH / 8, HEIGHT / 2);
            printf("ugraphics draw string ok!\n");
        }
        /*Show graphics on screen*/
        ugraphics_flip();

    } else if (!strncmp(argv[1], "fill", 4)) {
        if (!strncmp(argv[2], "rect", 4)) {
            /*Clear buffer on screen*/
            ugraphics_clear();

            /*Fill full rectangle*/
            ugraphics_fill_rect(0, HEIGHT / 2, WIDTH / 2, HEIGHT / 2);
            printf("ugraphics fill rectangle ok!\n");
        } else {
            printf("unkown command\n");
        }

        /*Show graphics on screen*/
        ugraphics_flip();
    } else if (!strncmp(argv[2], "clear", 5)) {
        /*Clear buffer on screen*/
        ugraphics_clear();
        printf("ugraphics clear screen ok!\n");
    } else if (!strncmp(argv[2], "quit", 4)) {
        /*Quit ugraphics component*/
        ugraphics_quit();
    }

    return;
}

#ifdef AOS_COMP_CLI
/* reg args: fun, cmd, description*/
ALIOS_CLI_CMD_REGISTER(ugraphics_comp_example, ugraphics, ugraphics component base example)
#endif
