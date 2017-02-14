#include <string.h>
#include "ms-epoll.h"
#include "tcp-link.h"
#include "Guard.h"

extern HTCPLINK  g_ms_tcp_first_create_link;
extern ms_tcp_log_callback_func  g_tcp_log_callback ;

bool MsEpoll::init()
{
    if( -1 != mi_epoll_fd ){
        return false;
    }
    mi_epoll_fd = ::epoll_create( 1024 );
    if( mi_epoll_fd < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "epoll_create failed." );
        return false;
    }

//    mi_exit_event = ::eventfd( 0, EFD_NONBLOCK );
//    if( mi_exit_event < 0 ){
//        g_tcp_log_callback( LOG_LEVEL_ERROR, "eventfd failed." );
//        ::close( mi_epoll_fd );
//        mi_epoll_fd = -1;
//        return false;
//    }

//    epoll_event event;
//    memset( &event, 0, sizeof(event) );
//    event.events = EPOLLIN;
//    event.data.fd = mi_exit_event;
//    if( ::epoll_ctl( mi_epoll_fd, EPOLL_CTL_ADD, mi_exit_event, &event) < 0 ){
//        g_tcp_log_callback( LOG_LEVEL_ERROR, "epoll_ctl epoll_ctl_add exit_event failed." );
//        ::close( mi_epoll_fd );
//        mi_epoll_fd = -1;
//        ::close( mi_exit_event );
//        mi_exit_event = -1;
//        return false;
//    }

    MsTcpLink::init_link_lock();
    MsTcpLink::init_link_map();

    if( ms_pthread_creat( &m_pthread_id, &worker_thread, this ) < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "create worker_thread failed." );
        ::close( mi_epoll_fd );
        mi_epoll_fd = -1;
//        ::close( mi_exit_event );
//        mi_exit_event = -1;
        return false;
    }

    mb_timeout_thread_flag = true;
    if( ms_pthread_creat( &m_pthread_timeout, &timeoutThreadProc, this) < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "create timeout_thread_proc failed." );
        mb_timeout_thread_flag = false;
        ::close( mi_epoll_fd );
        mi_epoll_fd = -1;
//        ::close( mi_exit_event );
//        mi_exit_event = -1;
        return false;
    }

    return true;
}

void MsEpoll::end()
{
//    unsigned long long ull_exit = 1;
//    ::write( mi_exit_event, &ull_exit, sizeof(ull_exit) );

    mb_exit_thread = true;
    if( m_pthread_id != 0 ){
        void *result;
        pthread_join( m_pthread_id, &result );
        m_pthread_id = 0;
    }

    mb_timeout_thread_flag = false;
    if( m_pthread_timeout != 0 ){
        void *result;
        pthread_join( m_pthread_timeout, &result );
        m_pthread_timeout = 0;
    }

//    ::close( mi_exit_event );
    ::close( mi_epoll_fd );
    mi_epoll_fd = -1;
//    mi_exit_event = -1;

    MsTcpLink::free_all_link_map();
    MsTcpLink::destroy_link_lock();

    return;
}

void *MsEpoll::worker_thread( void *arg )
{
    MsEpoll *p_this = (MsEpoll *)arg;
    const int i_max_eventnum = 512;
    epoll_event events[i_max_eventnum];

    p_this->mb_exit_thread = false;
    while( !p_this->mb_exit_thread ){
        int i_event_num = epoll_wait( p_this->mi_epoll_fd, events, i_max_eventnum, 1000 );
        if( g_ms_tcp_first_create_link != INVALID_HTCPLINK ){
            static unsigned int ui_keep_alive_times = 0;
            if( ui_keep_alive_times % 10 == 0 ){
                CRefCountPtr<MsTcpLink> p_link_keepalive;
                if( !MsTcpLink::find_link( g_ms_tcp_first_create_link, p_link_keepalive ) ){
                    continue;
                }
                p_link_keepalive->send_link_event( EVT_TCP_KEEPALIVE );
            }
            ui_keep_alive_times++;
            if( ui_keep_alive_times >= 10000 ){
                ui_keep_alive_times = 0;
            }
        }

        if( i_event_num <= 0 ){
            continue;
        }
        for( int i = 0; i < i_event_num; i++ ){
//            if( events[i].data.fd == p_this->mi_exit_event ){
//                g_tcp_log_callback( LOG_LEVEL_INFO, "fd is exit_event, return." );
//                return NULL;
//            }

            unsigned int ui_event = events[i].events;
            HTCPLINK h_link = events[i].data.u64 >> 32;
            CRefCountPtr<MsTcpLink> p_link;
            if( !MsTcpLink::find_link( h_link, p_link ) ){
                g_tcp_log_callback( LOG_LEVEL_ERROR, "find link failed" );
                continue;
            }
            if( MsEpoll::is_disconnected( ui_event ) ){
                //notify disconnect
                printf( "p_link->send_link_event( EVT_TCP_DISCONNECTED );link=0x%04x\n", h_link );
                p_link->send_link_event( EVT_TCP_DISCONNECTED );
                p_link->link_destroy( false );
                continue;
            }
            if( p_link->get_link_type() == MsTcpLink::link_TypeListen ){
                if( MsEpoll::is_readable( ui_event ) ){
                    p_link->on_event_accept( );
                }
                continue;
            }
            if( p_link->get_link_type() == MsTcpLink::link_TypeConnect ){
                if( MsEpoll::is_readable( ui_event ) || MsEpoll::is_writeable( ui_event ) ){
                    p_link->on_event_connected();
                }
                continue;
            }
            if( p_link->get_link_type() == MsTcpLink::link_TypeConnected ||
                p_link->get_link_type() == MsTcpLink::link_TypeAccept ){
                if( MsEpoll::is_writeable( ui_event ) ){
                    p_link->set_link_write_flag( true );
                }
                if( MsEpoll::is_readable( ui_event ) ){
                    p_link->on_event_read();
                }
                continue;
            }
        }
    }
    return NULL;
}

int ms_msleep( int i_sleep_ms_time )
{
    int i_ret = 0;
    struct timeval st_tv;
    st_tv.tv_sec  = i_sleep_ms_time / 1000;
    st_tv.tv_usec = (i_sleep_ms_time % 1000) * 1000;

    for (;;) {
        i_ret = select(0, NULL, NULL, NULL, &st_tv);

        if (i_ret == 0) {
            break;
        } else {
            int i_err = errno;
            if ( i_err != EINTR ) {
                printf("ms_msleep err=%s\n", strerror(i_err));
                break;
            }
        }
    }

    return i_ret;
}

void *MsEpoll::timeoutThreadProc( void *arg )
{
    MsEpoll *p_this = (MsEpoll *)arg;
    while( p_this->mb_timeout_thread_flag ){
        //MsTcpLink::check_link_timeout();
        ms_msleep( 5000 );
    }

    g_tcp_log_callback( LOG_LEVEL_INFO, "thread timeoutThreadProc exit." );
    return NULL;
}

bool MsEpoll::is_disconnected( unsigned int ui_event )
{
    return ui_event & ( EPOLLERR | EPOLLHUP | EPOLLRDHUP );
}

bool MsEpoll::is_readable( unsigned int ui_event )
{
    return ui_event & EPOLLIN;
}

bool MsEpoll::is_writeable( unsigned int ui_event )
{
    return ui_event & EPOLLOUT;
}

bool MsEpoll::attach_to_epoll( int i_fd, HTCPLINK h_link, bool b_write )
{
    unsigned int ui_events = 0;
    if( b_write ){
        ui_events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;
    } else {
        ui_events = EPOLLIN | EPOLLET;
    }

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events   = ui_events;
    event.data.u64 = h_link;
    event.data.u64 <<= 32;
    event.data.fd  = i_fd;

    if( ::epoll_ctl( mi_epoll_fd, EPOLL_CTL_ADD, i_fd, &event ) < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "epoll_ctl EPOLL_CTL_ADD failed." );
        return false;
    }
    return true;
}

bool MsEpoll::mod_epoll_read( int i_fd, HTCPLINK h_link )
{
    unsigned int ui_events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events   = ui_events;
    event.data.u64 = h_link;
    event.data.u64 <<= 32;
    event.data.fd  = i_fd;

    if( ::epoll_ctl( mi_epoll_fd, EPOLL_CTL_MOD, i_fd, &event ) < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "epoll_ctl EPOLL_CTL_MOD failed." );
        return false;
    }
    return true;
}

bool MsEpoll::del_epoll_read( int i_fd )
{
    epoll_event event;
    event.data.fd = i_fd;
    event.events = EPOLLIN | EPOLLET;
    if( epoll_ctl( mi_epoll_fd, EPOLL_CTL_DEL, i_fd, &event ) < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "epoll_ctl EPOLL_CTL_DEL failed." );
        return false;
    }
    return true;
}

int ms_pthread_creat(pthread_t *pst_tid, pfRoutine pf_func, void *pv_func_para, bool b_detached, unsigned int ui_stack_size, int i_policy, int i_priority)
{
    pthread_attr_t st_attr;
    struct sched_param st_param;
    size_t ui_stack_tmp = 0;
    int  i_ret = 0;
    int  i_policy_tmp = 0;
    int  i_priority_tmp = 0;
    pthread_t st_tid_tmp = 0;

    if ( pst_tid == NULL ) {
        pst_tid = &st_tid_tmp;
    }
    if ( pf_func == NULL ) {
        return -1;
    }

    i_ret = pthread_attr_init(&st_attr);
    if (i_ret != 0) {
        return -1;
    }

    if ((ui_stack_size > 0) && (ui_stack_size < PTHREAD_STACK_MIN)) {
        ui_stack_tmp = PTHREAD_STACK_MIN;
    } else if (ui_stack_size == 0) {
        i_ret = pthread_attr_getstacksize(&st_attr, &ui_stack_tmp);
        if (i_ret != 0) {
            return -1;
        }
    } else {
        ui_stack_tmp = ui_stack_size;
    }

    if (-1 == i_policy) {
        pthread_attr_getschedpolicy(&st_attr, &i_policy_tmp);
    } else {
        i_policy_tmp = i_policy;
    }

    pthread_attr_setstacksize(&st_attr, ui_stack_tmp);
    pthread_attr_getschedparam(&st_attr, &st_param);
    if ( i_priority >= 0 ) {
        if (i_priority > sched_get_priority_max(i_policy_tmp)) {
            i_priority_tmp = sched_get_priority_max(i_policy_tmp);
        } else if (i_priority < sched_get_priority_min(i_policy_tmp)) {
            i_priority_tmp = sched_get_priority_min(i_policy_tmp);
        } else {
            i_priority_tmp = i_priority;
        }
        st_param.sched_priority = i_priority_tmp;
    }
    pthread_attr_setschedparam(&st_attr, &st_param);
    pthread_attr_setschedpolicy(&st_attr, i_policy_tmp);

    if ( b_detached == true ) {
        pthread_attr_setdetachstate(&st_attr, PTHREAD_CREATE_DETACHED);
    }

    i_ret = pthread_create(pst_tid, &st_attr, pf_func, pv_func_para);
    if (i_ret != 0) {
        return -1;
    }

    return 0;
}
