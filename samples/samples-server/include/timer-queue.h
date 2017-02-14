#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include <set>
#include <stdint.h>
#include <sys/time.h>

namespace MSThread {

typedef void(*callback_on_timer)( unsigned int, void * );

class TimerQueue{
public:
    TimerQueue();
    ~TimerQueue();

private:
    TimerQueue( const TimerQueue &other );
    TimerQueue& operator=( const TimerQueue &other );

public:
    void set_timer( unsigned int id, unsigned int elapse, const callback_on_timer& on_timer, void *arg );
    void kill_timer( unsigned int id );
    bool empty() const;

    bool peek( unsigned int& id, callback_on_timer& on_timer, void *&arg );
    unsigned long long get_wait_time();

private:
    void erase( unsigned int id );

private:
    struct Timer
    {
        unsigned int mui_id;//时钟ID
        unsigned long long mull_interval; //间隔,单位:毫秒
        unsigned long long mull_start;//开始时间
        callback_on_timer on_timer;
        void *func_arg;
        bool operator<(const Timer &right ) const;
    };

    std::set<Timer> ms_timers;
};

}


#endif
