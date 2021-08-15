#include "timer.h"

//构造函数，将延迟时间初始化为20s，访问次数为1次（创建这次的访问），并存储文件描述符，与文件建立联系
Timer::Timer(vfs_file_t* fp) {
    delay_time = 20;
    visit_times = 1;
    this->fp = fp;
    printf("init_file fp:%d\r\n", fp);
    printf("init_file this->fp:%d\r\n", this->fp);
}

aos_timer_t Timer::get_main_timer() {
    return main_timer;
}

int Timer::init_main_timer(void(*fn)(void*, void*), void* arg, int ms, int repeat, unsigned char auto_run) {
    printf("init_main_timer arg: %d\r\n", arg);
    return aos_timer_new_ext(&main_timer, fn, arg, ms, repeat, auto_run);
}

aos_timer_t Timer::get_ass_timer() {
    return ass_timer;
}

int Timer::init_ass_timer(void(*fn)(void*, void*), void* arg, int ms, int repeat, unsigned char auto_run) {
    printf("init_ass_timer arg: %d\r\n", arg);
    return aos_timer_new_ext(&ass_timer, fn, arg, ms, repeat, auto_run);
}

int Timer::get_delay_time() {
    return delay_time;
}

void Timer::set_delay_time(int time) {
    delay_time = time;
}

int Timer::get_visit_times() {
    return visit_times;
}

void Timer::set_visit_times(int times) {
    visit_times = times;
}

vfs_file_t* Timer::get_file() {
    printf("getfp: %d\r\n", fp);
    return fp;
}

void Timer::set_file(vfs_file_t* fp) {
    this->fp = fp;
    printf("set_file fp:%d\r\n", fp);
    printf("set_file this->fp:%d\r\n", this->fp);
}

void Timer::reset_main_timer(int ms) {
    aos_timer_stop(&main_timer);
    aos_timer_change_once(&main_timer, ms);
    aos_timer_start(&main_timer);
}

void Timer::reset_ass_timer(int ms) {
    aos_timer_stop(&ass_timer);
    aos_timer_change_once(&ass_timer, ms);
    aos_timer_start(&ass_timer);
}


