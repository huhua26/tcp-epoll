#include "task-thread.h"
#include "ms-epoll.h"

namespace MSThread {

TaskThread::TaskThread()
{

}

TaskThread::~TaskThread()
{

}

void TaskThread::begin( int thread_number )
{
    for( int i = 0; i < thread_number; ++i ){
        pthread_t p_tmp = 0;
        //pthread_create( &p_tmp, NULL, &thread_proc, this );
        ms_pthread_creat( &p_tmp, &thread_proc, this );
        if( p_tmp != 0 ){
            m_threads.push_back( p_tmp );
        }
    }
}

void TaskThread::end()
{
    m_queues.notify_exit();
    std::vector<pthread_t>::iterator it_thread = m_threads.begin();
    for( ; it_thread != m_threads.end(); ++it_thread ){
        void *result;
        pthread_join( *it_thread, &result );
    }
}

void TaskThread::send_task( const CallBackStruct &task )
{
    task.callback_func( task.arg );
}

void TaskThread::post_task( const CallBackStruct &task )
{
    m_queues.post_task( (CallBackStruct)task );
}

void TaskThread::post_front_task( const CallBackStruct &task )
{
    m_queues.post_front_task( (CallBackStruct)task );
}

int TaskThread::task_count()
{
    return m_queues.task_count();
}

void TaskThread::set_timer( unsigned int id, unsigned int elapse, const MSThread::callback_on_timer &on_timer, void *arg )
{
    m_queues.set_timer( id, elapse, on_timer, arg );
}

void TaskThread::kill_timer( unsigned int id )
{
    m_queues.kill_timer( id );
}

void* TaskThread::thread_proc( void *arg )
{
    TaskThread *p_this = (TaskThread *)arg;
    p_this->m_queues.do_while();

    return arg;
}


}
