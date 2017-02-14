#ifndef TASK_THREAD_H_
#define TASK_THREAD_H_

#include "thread-queue.h"
#include "task-queue.h"
#include "singletonHandler.h"
#include <vector>

namespace MSThread {

class TaskThread
{
public:
    enum{
        Timer_Cruise_Restart = 0x01,
        Timer_Clear_Outdated_File  = 0x02,
        Timer_Compress_Outdated_File = 0x03,
        Timer_Compress_Ana_File = 0x04,

        Timer_Error
    };

    TaskThread();
    ~TaskThread();

private:
    TaskThread( const TaskThread &other );
    TaskThread& operator=( const TaskThread &other );

public:
    void begin(int thread_number = 1);
    void end();

    void send_task( const CallBackStruct &task );
    void post_task( const CallBackStruct& task );
    void post_front_task( const CallBackStruct &task );
    int  task_count();
    void set_timer( unsigned int id, unsigned int elapse, const MSThread::callback_on_timer& on_timer, void *arg );
    void kill_timer( unsigned int id );

private:
    static void* thread_proc( void *arg );

private:
    std::vector<pthread_t> m_threads;
    ThreadQueue<CallBackStruct, TaskQueue> m_queues;

};

}


typedef CSingletonHandler<MSThread::TaskThread> CSingletonMSTaskThreadPool;


#endif
