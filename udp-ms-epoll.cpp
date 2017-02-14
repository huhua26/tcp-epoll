#include <string.h>
#include "udp-ms-epoll.h"
#include "clientlog-api.h"
#include "udp-link.h"
#include "Guard.h"

bool MsUdpEpoll::init()
{
    if( -1 != mi_epoll_fd ){
        MS_LOGER_ERROR( "mi_epoll_fd != -1, init failed." );
        return false;
    }
    mi_epoll_fd = ::epoll_create( 1024 );
    if( mi_epoll_fd < 0 ){
        int i_error = errno;
        MS_LOGER_ERROR( "epoll_create failed. error=%d, %s", i_error, strerror( i_error ) );
        return false;
    }

    mi_exit_event = ::eventfd( 0, EFD_NONBLOCK );
    if( mi_exit_event < 0 ){
        MS_LOGER_ERROR( "eventfd failed." );
        ::close( mi_epoll_fd );
        mi_epoll_fd = -1;
        return false;
    }

    epoll_event event;
    memset( &event, 0, sizeof(event) );
    event.events = EPOLLIN;
    event.data.fd = mi_exit_event;
    if( ::epoll_ctl( mi_epoll_fd, EPOLL_CTL_ADD, mi_exit_event, &event ) < 0 ){
        MS_LOGER_ERROR( "epoll_ctl add exit_event failed." );
        ::close( mi_epoll_fd );
        mi_epoll_fd = -1;
        ::close( mi_exit_event );
        mi_exit_event = -1;
        return false;
    }

    MsUdpLink::init_link_lock();
    MsUdpLink::init_link_map();
    MsUdpLink::set_send_flag( false );

    if( ms_pthread_creat( &m_pthread_id, &worker_thread, this ) < 0 ){
        MS_LOGER_ERROR( "create worker_thread failed." );
        ::close( mi_epoll_fd );
        mi_epoll_fd = -1;
        ::close( mi_exit_event );
        mi_exit_event = -1;
        return false;
    }

    return true;
}

void MsUdpEpoll::end()
{
    unsigned long long ull_exit = 1;
    ::write( mi_exit_event, &ull_exit, sizeof( ull_exit ) );

    if( m_pthread_id != 0 ){
        void *result;
        pthread_join( m_pthread_id, &result );
        m_pthread_id = 0;
    }

    ::close( mi_exit_event );
    ::close( mi_epoll_fd );
    mi_epoll_fd = -1;
    mi_exit_event = -1;

    MsUdpLink::free_all_link_map();
    MsUdpLink::destroy_link_lock();

    return;
}


void *MsUdpEpoll::worker_thread( void *arg )
{
    MsUdpEpoll *p_this = (MsUdpEpoll *)arg;
    const int i_max_eventnum = 512;
    epoll_event events[i_max_eventnum];

    while( true ){
        int i_event_num = ::epoll_wait( p_this->mi_epoll_fd, events, i_max_eventnum,  -1 );
        if( i_event_num < 0 ){
            MsUdpLink::do_send_buff();
            continue;
        }
        for( int i = 0; i < i_event_num; i++ ){
            if( events[i].data.fd == p_this->mi_exit_event ){
                MS_LOGER_INFO( "fd is exit_event, return." );
                return NULL;
            }

            unsigned int ui_event = events[i].events;
            HUDPLINK h_link = events[i].data.u64 >> 32;
            CRefCountPtr<MsUdpLink> p_reflink;
            if( MsUdpEpoll::is_readable( ui_event ) ){
                if( !MsUdpLink::find_link( h_link, p_reflink ) ){
                    MS_LOGER_ERROR( "recv data failed. can't find link=0x%04x", h_link );
                } else {
                    p_reflink->on_event_read();
                }
            }
            continue;
        }
        MsUdpLink::do_send_buff();
    }

    return arg;
}


bool MsUdpEpoll::is_disconnected( unsigned int ui_event )
{
    return ui_event & ( EPOLLERR | EPOLLHUP | EPOLLRDHUP );
}

bool MsUdpEpoll::is_readable( unsigned int ui_event )
{
    return ui_event & EPOLLIN;
}

bool MsUdpEpoll::is_writeable( unsigned int ui_event )
{
    return ui_event & EPOLLOUT;
}

bool MsUdpEpoll::attach_to_epoll( int i_fd, HUDPLINK h_link, bool b_write )
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
        MS_LOGER_ERROR( "epoll_ctl EPOLL_CTL_ADD failed." );
        return false;
    }
    return true;
}

bool MsUdpEpoll::mod_epoll_read( int i_fd, HUDPLINK h_link )
{
    unsigned int ui_events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;

    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events   = ui_events;
    event.data.u64 = h_link;
    event.data.u64 <<= 32;
    event.data.fd  = i_fd;

    if( ::epoll_ctl( mi_epoll_fd, EPOLL_CTL_MOD, i_fd, &event ) < 0 ){
        MS_LOGER_ERROR( "epoll_ctl EPOLL_CTL_MOD failed." );
        return false;
    }
    return true;
}

bool MsUdpEpoll::del_epoll_read( int i_fd )
{
    epoll_event event;
    event.data.fd = i_fd;
    event.events = EPOLLIN | EPOLLET;
    if( epoll_ctl( mi_epoll_fd, EPOLL_CTL_DEL, i_fd, &event ) < 0 ){
        MS_LOGER_ERROR( "epoll_ctl EPOLL_CTL_DEL failed." );
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
