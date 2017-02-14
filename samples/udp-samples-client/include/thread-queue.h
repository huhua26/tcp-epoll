#ifndef THREAD_QUEUE_H_
#define THREAD_QUEUE_H_

#include "timer-queue.h"
#include <pthread.h>

namespace MSThread {

typedef void(*callback_taskthread_func)( void* );
struct CallBackStruct{
    callback_taskthread_func callback_func;
    void *arg;
};

//TaskT = CallBackStruct
template<typename TaskT, template<typename> class TaskQueueT>
class ThreadQueue
{
public:
    ThreadQueue()
    {
        pthread_mutex_init( &mo_mutex, NULL );
        pthread_cond_init( &m_condition_var, NULL );
        m_exit_flag = false;
    }
    ~ThreadQueue()
    {

        m_exit_flag = false;
        //pthread_cond_broadcast( &m_condition_var );
        //pthread_cond_destroy( &m_condition_var );
        pthread_mutex_destroy( &mo_mutex );
    }

private:
    ThreadQueue( const ThreadQueue &other );
    ThreadQueue& operator=( const ThreadQueue &other );


public:
    void post_task( const TaskT &task )
    {
        pthread_mutex_lock( &mo_mutex );
        m_task_queue.push( task );
        pthread_mutex_unlock( &mo_mutex );

        pthread_cond_signal( &m_condition_var );
    }

    void post_front_task( const TaskT &task )
    {
        pthread_mutex_lock( &mo_mutex );
        m_task_queue.push_front( task );
        pthread_mutex_unlock( &mo_mutex );

        pthread_cond_signal( &m_condition_var );
    }

    int task_count()
    {
        pthread_mutex_lock( &mo_mutex );
        int i_size = m_task_queue.size();
        pthread_mutex_unlock( &mo_mutex );

        return i_size;
    }

    void set_timer( unsigned int id, unsigned int elapse, const MSThread::callback_on_timer &on_timer, void *arg )
    {
        pthread_mutex_lock( &mo_mutex );
        m_timer_queue.set_timer( id, elapse, on_timer, arg );
        pthread_mutex_unlock( &mo_mutex );

        pthread_cond_signal( &m_condition_var );
    }

    void kill_timer( unsigned int id )
    {
        pthread_mutex_lock( &mo_mutex );
        m_timer_queue.kill_timer( id );
        pthread_mutex_unlock( &mo_mutex );
    }

    void do_while()
    {
        pthread_mutex_lock( &mo_mutex );
        while( !m_exit_flag ){
            if( m_task_queue.empty() ){
                struct timespec st_tmout;
                struct timeval st_now;
                gettimeofday( &st_now, NULL );
                unsigned long long t_wait = m_timer_queue.get_wait_time();
                unsigned long long ul_nsec = ( st_now.tv_usec + (t_wait % 1000) * 1000 ) * 1000;
                st_tmout.tv_nsec = ul_nsec % 1000000000;
                //处理不足一秒加上超时时间超过一秒的情况（即tv_sec上需要加上nsec/1000000000）
                st_tmout.tv_sec = st_now.tv_sec + ul_nsec / 1000000000 + t_wait / 1000;
                pthread_cond_timedwait( &m_condition_var, &mo_mutex,  &st_tmout );
            }
            if( m_exit_flag ){
                break;
            }
            do_one_timer( );
            if( m_exit_flag ){
                break;
            }
            do_one_task( );
            if( m_exit_flag ){
                break;
            }
        }
        pthread_mutex_unlock( &mo_mutex );
    }

    void notify_exit()
    {
        pthread_mutex_lock( &mo_mutex );
        m_exit_flag = true;
        pthread_mutex_unlock( &mo_mutex );

        pthread_cond_broadcast( &m_condition_var );
    }

private:
    void do_one_timer( )
    {
        unsigned int id;
        MSThread::callback_on_timer on_timer;
        void *arg = NULL;
        if( m_timer_queue.peek( id, on_timer, arg ) ){
            pthread_mutex_unlock( &mo_mutex );
            on_timer( id, arg );
            pthread_mutex_lock( &mo_mutex );
        }
    }

    void do_one_task( )
    {
        TaskT task;
        if( m_task_queue.pop(task) ){
            pthread_mutex_unlock( &mo_mutex );
            task.callback_func( task.arg );
            pthread_mutex_lock( &mo_mutex );
        }
    }

private:
    pthread_mutex_t mo_mutex;
    TaskQueueT<TaskT> m_task_queue;
    TimerQueue m_timer_queue;

    bool m_exit_flag;

    pthread_cond_t m_condition_var;

};

}

#endif
