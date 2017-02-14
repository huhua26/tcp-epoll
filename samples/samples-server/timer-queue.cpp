#include "timer-queue.h"
#include <sys/time.h>
#include <stdio.h>

namespace MSThread {

unsigned long long get_tick_count()
{
    struct timeval time_now;
    gettimeofday( &time_now, NULL );

    return  (unsigned long long)time_now.tv_sec * (unsigned long long)1000 + (unsigned long long)time_now.tv_usec / (unsigned long long)1000 ;
}


TimerQueue::TimerQueue()
{
    ms_timers.clear();
}

TimerQueue::~TimerQueue()
{
    ms_timers.clear();
}

void TimerQueue::set_timer( unsigned int id, unsigned int elapse, const callback_on_timer &on_timer, void *arg )
{
    erase( id );
    Timer timer_tmp;
    timer_tmp.mui_id = id;
    timer_tmp.mull_interval = elapse;
    timer_tmp.mull_start= get_tick_count();
    timer_tmp.on_timer = on_timer;
    timer_tmp.func_arg = arg;
    ms_timers.insert( timer_tmp );
}

void TimerQueue::kill_timer( unsigned int id )
{
    erase( id );
}

bool TimerQueue::empty() const
{
    return ms_timers.empty();
}

bool TimerQueue::peek( unsigned int &id, callback_on_timer &on_timer, void *&arg )
{
    if ( ms_timers.empty() ){
        return false;
    }

    std::set<Timer>::iterator it_timer = ms_timers.begin();
    Timer first_timer = *it_timer;
    unsigned long long end_time = first_timer.mull_start + first_timer.mull_interval;
    unsigned long long current = get_tick_count();
    if ( current >= end_time ){
        id = first_timer.mui_id;
        on_timer = first_timer.on_timer;
        arg = first_timer.func_arg;
        ms_timers.erase( it_timer );
        first_timer.mull_start += first_timer.mull_interval;
        ms_timers.insert( first_timer );
        return true;
    }

    return false;
}

unsigned long long TimerQueue::get_wait_time()
{
    const unsigned long long kWaitInfinite = 60 * 60 * 1000;  // 1 hour
    if ( ms_timers.empty() ){
        return kWaitInfinite;
    }

    std::set<Timer>::iterator it_timer = ms_timers.begin();
    Timer first_timer = *it_timer;
    unsigned long long end_time = first_timer.mull_start + first_timer.mull_interval;
    unsigned long long current = get_tick_count();
    return (current <= end_time) ? (end_time - current) : 0;
}

void TimerQueue::erase( unsigned int id )
{
    std::set<Timer>::iterator it_timer = ms_timers.begin();
    for( ; it_timer != ms_timers.end(); ++it_timer ){
        if( it_timer->mui_id == id ){
            ms_timers.erase( it_timer );
            break;
        }
    }
}

bool TimerQueue::Timer::operator <( const Timer &right ) const
{
    unsigned long long ull_endtime = mull_start + mull_interval;
    unsigned long long ull_right_endtime = right.mull_start + right.mull_interval;

    if( ull_endtime < ull_right_endtime ){
        return true;
    } else if( (ull_endtime == ull_right_endtime) && (mui_id < right.mui_id) ){
        return true;
    }

    return false;
}


}
