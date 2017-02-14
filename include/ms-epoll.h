#ifndef EPOLL_H_
#define EPOLL_H_

#include <pthread.h>
#include <limits.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "ms-epoll-tcp.h"
#include "singletonHandler.h"

class MsEpoll
{
public:
//    MsEpoll() :
//        mi_epoll_fd(-1), mi_exit_event(-1), m_pthread_id(0),
//        m_pthread_timeout(0), mb_timeout_thread_flag(false) {

//    }

    MsEpoll() :
        mi_epoll_fd(-1), m_pthread_id(0),
        m_pthread_timeout(0), mb_timeout_thread_flag(false),
        mb_exit_thread( false ) {

    }

    virtual ~MsEpoll(){

    }

public:
    bool init();
    void end();

private:
    static void *worker_thread( void *arg );
    static void *timeoutThreadProc( void *arg );

public:
    static bool is_disconnected( unsigned int ui_event );
    static bool is_readable( unsigned int ui_event );
    static bool is_writeable( unsigned int ui_event );

    bool attach_to_epoll( int i_fd, HTCPLINK h_link, bool b_write = true );
    bool mod_epoll_read( int i_fd, HTCPLINK h_link );
    bool del_epoll_read( int i_fd );

private:
    int mi_epoll_fd;
    //int mi_exit_event;

    bool mb_exit_thread;

    pthread_t m_pthread_id;
    pthread_t m_pthread_timeout;

    bool mb_timeout_thread_flag;

};

typedef CSingletonHandler<MsEpoll>  CSingletonMsEpollHandler;

typedef void *(*pfRoutine) (void *);
int ms_pthread_creat(pthread_t *pst_tid, pfRoutine pf_func, void *pv_func_para = NULL, bool b_detached = false, unsigned int ui_stack_size = 0, int i_policy = -1, int i_priority = -1);

#endif
