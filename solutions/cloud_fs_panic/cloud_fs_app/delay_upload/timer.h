#include <aos/kernel.h>
#include "vfs_types.h"
using namespace std;

class Timer {
private:
    aos_timer_t main_timer;
    aos_timer_t ass_timer;
    int delay_time;
    int visit_times;
    vfs_file_t* fp;
public:
    Timer(vfs_file_t* fp);
    aos_timer_t get_main_timer();
    int init_main_timer(void(*fn)(void*, void*), void* arg, int ms, int repeat, unsigned char auto_run);
    aos_timer_t get_ass_timer();
    int init_ass_timer(void(*fn)(void*, void*), void* arg, int ms, int repeat, unsigned char auto_run);
    int get_delay_time();
    void set_delay_time(int time);
    int get_visit_times();
    void set_visit_times(int times);
    vfs_file_t* get_file();
    void set_file(vfs_file_t* fp);
    void reset_main_timer(int ms);
    void reset_ass_timer(int ms);
};