#include <pthread.h>
#include <list>
#include "tcp-link.h"
#include "ms-epoll.h"

pthread_mutex_t MsTcpLink::m_link_lock;
std::map<HTCPLINK, int> MsTcpLink::m_link_socket_map;
std::map<HTCPLINK, err_info_s> MsTcpLink::m_link_errinfo;
std::map<HTCPLINK, CRefCountPtr<MsTcpLink> > MsTcpLink::m_maplink;
std::map<HTCPLINK, std::list<send_buff_s> > MsTcpLink::m_send_buff_list_map;
bool MsTcpLink::mb_send_buff_thread_flag;
pthread_t MsTcpLink::mo_thread_id;

#define SOCKET_MAX_SEND_TIMES  20


void default_tcp_log_callback_func( int i_level, const char *pc_message )
{
    if( pc_message != NULL ){
        printf( "%s\n", pc_message );
    }
}

ms_tcp_log_callback_func  g_tcp_log_callback = default_tcp_log_callback_func;
HTCPLINK  g_ms_tcp_first_create_link = INVALID_HTCPLINK;

//from MSLogServer
bool get_ip_by_name(const std::string& name, unsigned int& ipv4)
{
    if (name.empty()) {
        return false;
    }

    unsigned long ip = ::inet_addr(name.c_str());
    if (ip != INADDR_NONE) {
        ipv4 = ntohl(ip);
        return true;
    }

    hostent* host = ::gethostbyname(name.c_str());
    if (host != NULL &&
        host->h_addrtype == AF_INET &&
        host->h_addr_list[0] != NULL) {
        ip = *(unsigned long*)host->h_addr_list[0];
        ipv4 = ntohl(ip);
        return true;
    } else {
        return false;
    }

}

MsTcpLink::MsTcpLink()
{
    pthread_mutex_init( &mo_last_alive_time_lock, NULL );
    mi_socket = -1;
    mh_link = -1;
    mp_tcpcallback = NULL;
    memset( &m_local_address, 0, sizeof(m_local_address) );
    memset( &m_remote_address, 0, sizeof(m_remote_address) );
    time( &mt_last_alive_time );
    m_link_type = link_TypeNull;
    mb_need_check_alive = false;

    mb_write_flag = true;
}

MsTcpLink::~MsTcpLink()
{
    pthread_mutex_destroy( &mo_last_alive_time_lock );
}

bool MsTcpLink::find_link( HTCPLINK h_link, CRefCountPtr<MsTcpLink> &p_link )
{
    CGuardLock<pthread_mutex_t> guard( &m_link_lock );
    std::map<HTCPLINK, CRefCountPtr<MsTcpLink> >::iterator it = m_maplink.find( h_link );
    if( it == m_maplink.end() ){
        std::string str_msg = "find link failed. link=" + T2string( h_link );
        g_tcp_log_callback( LOG_LEVEL_ERROR, str_msg.c_str() );
        return false;
    }
    p_link = it->second;
    return true;
}

bool MsTcpLink::del_link( HTCPLINK h_link )
{    
    CGuardLock<pthread_mutex_t> guard( &m_link_lock );
    std::map<HTCPLINK, CRefCountPtr<MsTcpLink> >::iterator it = m_maplink.find( h_link );
    if( it == m_maplink.end() ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "find link failed for del." );
        return false;
    }
    CRefCountPtr<MsTcpLink> p_reflink = it->second;
    CSingletonMsEpollHandler::Instance()->del_epoll_read( p_reflink->get_socket() );
    m_maplink.erase( it );

    std::map<HTCPLINK, int>::iterator iter = m_link_socket_map.find( h_link );
    if( iter != m_link_socket_map.end() ){
        m_link_socket_map.erase( iter );
    }

    std::map<HTCPLINK, err_info_s>::iterator it_err = m_link_errinfo.find( h_link );
    if( it_err != m_link_errinfo.end() ){
        m_link_errinfo.erase( it_err );
    }

    return true;
}

void MsTcpLink::free_all_link_map()
{
//    if( mo_thread_id != 0 ){
//        mb_send_buff_thread_flag = false;
//        void *result = NULL;
//        pthread_join( mo_thread_id, &result );
//        mo_thread_id = 0;
//    }

    std::list<CRefCountPtr<MsTcpLink> > link_list;
    do{
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        if( m_maplink.empty() ){
            return;
        }
        std::map<HTCPLINK, CRefCountPtr<MsTcpLink> >::iterator it = m_maplink.begin();
        for( ; it != m_maplink.end(); ++it ){
            CRefCountPtr<MsTcpLink> p_link = it->second;
            link_list.push_back( p_link );
        }
    }while( 0 );

    std::list<CRefCountPtr<MsTcpLink> >::iterator it_link = link_list.begin();
    for( ; it_link != link_list.end(); ++it_link ){
        CRefCountPtr<MsTcpLink> p_link = *it_link;
        p_link->link_destroy();
    }
}

void MsTcpLink::check_link_timeout()
{
    //if no any IO data after accept link, close it when timeout.
    //check handle and memory leak.
    std::list<CRefCountPtr<MsTcpLink> > link_list;
    do{
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        if( m_maplink.empty() ){
            return;
        }
        time_t st_cur_time = time( NULL );
        std::map<HTCPLINK, CRefCountPtr<MsTcpLink> >::iterator it = m_maplink.begin();
        for( ; it != m_maplink.end(); ++it ){
            CRefCountPtr<MsTcpLink> p_link = it->second;
            if( !p_link->is_alive( st_cur_time ) && (p_link->get_link_type() != link_TypeListen) ){
                link_list.push_back( p_link );
            }
        }

    }while( 0 );

    std::list<CRefCountPtr<MsTcpLink> >::iterator it_link = link_list.begin();
    for( ; it_link != link_list.end(); ++it_link ){
        CRefCountPtr<MsTcpLink> p_link = *it_link;
        g_tcp_log_callback( LOG_LEVEL_INFO, "tcp destroy because of timeout." );
        p_link->link_destroy();
    }
}

int MsTcpLink::send_data_socket( int i_socket, const char* p_data, unsigned int ui_size )
{
    if( NULL == p_data ){
        return 0;
    }

    int i_write = ::write( i_socket, p_data, ui_size );
    return i_write;
}

bool MsTcpLink::check_error_info( HTCPLINK h_link )
{
    CGuardLock<pthread_mutex_t> gaurd( &m_link_lock );
    std::map<HTCPLINK, err_info_s>::iterator it = m_link_errinfo.find( h_link );
    if( it == m_link_errinfo.end() ){
        err_info_s st_err;
        m_link_errinfo[h_link] = st_err;
    } else {
        err_info_s &st_err = it->second;
        if( ( (st_err.i_error_no == EAGAIN) || (st_err.i_error_no == EINTR) ) &&
            ( time(NULL) - st_err.ui_time > 10 ) ){
            return false;
        }
    }

    return true;
}

bool MsTcpLink::set_error_info( HTCPLINK h_link, int i_error_no )
{
    CGuardLock<pthread_mutex_t> gaurd( &m_link_lock );
    std::map<HTCPLINK, err_info_s>::iterator it = m_link_errinfo.find( h_link );
    if( it == m_link_errinfo.end() ){
        err_info_s st_err;
        st_err.i_error_no = i_error_no;
        st_err.ui_time = (unsigned int)time( NULL );
    } else {
        err_info_s &st_err = it->second;
        if( (st_err.i_error_no == i_error_no) && (i_error_no != 0) ){
            if( st_err.ui_time == 0 ){
                st_err.ui_time = (unsigned int)time( NULL );
            }
        } else {
            st_err.i_error_no = i_error_no;
            st_err.ui_time = (unsigned int)time( NULL );
        }
    }

    return true;
}

bool MsTcpLink::link_create( ms_tcp_io_callback backFunc, const char* p_localAddr, unsigned short us_localPort )
{
    if( backFunc == NULL ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "backfunc is null." );
    }

    mp_tcpcallback = backFunc;
    mi_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
    if( mi_socket < 0 ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "create socket err." );
        return false;
    }

    set_conn_nonblock( mi_socket );

    bzero( &m_local_address, sizeof(m_local_address) );
    m_local_address.sin_family = AF_INET;
    m_local_address.sin_port = htons( us_localPort );
    //m_local_address.sin_addr.s_addr = htonl( INADDR_ANY );

    int i_opt = 1;
    setsockopt( mi_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&i_opt, sizeof(int) );

    //from MSLogerServer
//        set_socket_opt(fd, SOL_SOCKET, SO_KEEPALIVE, (int)1);
//        set_socket_opt(fd, SOL_TCP, TCP_KEEPIDLE, (int)1800);

    if (::bind( mi_socket, (sockaddr*)&m_local_address, sizeof(m_local_address) ) < 0) {
        ::close( mi_socket );
        int i_error = errno;
        printf( "bind port failed. errno=%d, i_error=%d\n", errno, i_error );

        std::string str_msg = "bind socket err. err=" + T2string( i_error );
        g_tcp_log_callback( LOG_LEVEL_ERROR, str_msg.c_str() );
        return false;
    }

    //update local address
    socklen_t len = sizeof( m_local_address );
    ::getsockname( mi_socket, (sockaddr *)&m_local_address, &len );    

    return true;
}

bool MsTcpLink::link_connect( const char* p_addr, unsigned short us_port )
{
    if( (p_addr == NULL) || (us_port == 0) ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "link_connect failed. wrong para." );
        return false;
    }

    std::string str_name = p_addr;
    unsigned int ui_ipv4 = 0;
    if( get_ip_by_name( str_name, ui_ipv4 ) ){
        sockaddr_in st_addr_remote;
        st_addr_remote.sin_family = AF_INET;
        st_addr_remote.sin_port = htons( us_port );
        st_addr_remote.sin_addr.s_addr = htonl( ui_ipv4 );

        set_conn_nonblock( mi_socket );
        set_link_type( link_TypeConnect );

        int i_ret = ::connect( mi_socket, (const sockaddr *)&st_addr_remote, sizeof( m_remote_address ) );
        if( ((i_ret == -1) && errno == (EINPROGRESS)) || i_ret != -1 ){
            socklen_t len = sizeof( m_local_address );
            ::getsockname( mi_socket, (sockaddr *)&m_local_address, &len );
            memcpy( &m_remote_address, &st_addr_remote, sizeof(st_addr_remote) );
            set_conn_nonblock( mi_socket );
            //call back
            send_link_event( EVT_TCP_CONNECTED );
            time( &mt_last_alive_time );
            return true;
        } else {
            g_tcp_log_callback( LOG_LEVEL_ERROR, "connect failed." );
            ::close( mi_socket );
            mi_socket = -1;
            send_link_event( EVT_TCP_DISCONNECTED );
            return false;
        }
    } else {
        g_tcp_log_callback( LOG_LEVEL_ERROR, "get_ip_by_name failed." );
    }

    return false;
}

bool MsTcpLink::link_listen( unsigned int ui_block )
{
    if( ui_block == 0  ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "link_listen failed. ui_block = 0." );
        return false;
    }

    set_conn_nonblock( mi_socket );
    set_link_type( link_TypeListen );

    if( -1 != ::listen( mi_socket, ui_block ) ){
        time( &mt_last_alive_time );

        return true;
    }
    g_tcp_log_callback( LOG_LEVEL_ERROR, "listen failed" );
    return false;
}

void MsTcpLink::link_destroy( bool b_self )
{        
    MsTcpLink::del_link( mh_link );

    if( mi_socket != -1 ){
        ::close( mi_socket );
        if( b_self ){
            send_link_event( EVT_TCP_CLOSED );
        }
        mi_socket = -1;
    }    
}

bool MsTcpLink::on_event_accept( )
{
    g_tcp_log_callback( LOG_LEVEL_ERROR, "on_event_accept." );
    time_t st_time;
    time( &st_time );

    while( true ){
        MsTcpLink *p_link = new MsTcpLink();
        if( !p_link->accept( mi_socket, mp_tcpcallback ) ){
            if( (errno != EAGAIN) && (errno != EINTR) ){
                g_tcp_log_callback( LOG_LEVEL_ERROR, "accept err" );
            }
            break;
        }

        //int i_opt = 1;
        //setsockopt( mi_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&i_opt, sizeof(int) );

        CRefCountPtr<MsTcpLink> p_reflink = CRefCountPtr<MsTcpLink>( p_link );
        HTCPLINK h_link = INVALID_HTCPLINK;
        MsTcpLink::add_tcp_link( h_link, p_reflink );
        p_reflink->set_link( h_link );
        p_reflink->update_link_time( st_time );        

        if( CSingletonMsEpollHandler::Instance()->attach_to_epoll(  p_reflink->get_socket(), p_reflink->get_link() ) ){
            send_accept_link( h_link );
        } else {
            g_tcp_log_callback( LOG_LEVEL_ERROR, "on_event_accept attach_to_epoll failed." );
        }
    }
    update_link_time( st_time );

    return true;
}

bool MsTcpLink::on_event_connected()
{
    g_tcp_log_callback( LOG_LEVEL_ERROR, "on_event_connected." );
    set_link_type( link_TypeConnected );
    time( &mt_last_alive_time );
    return true;
}

bool MsTcpLink::on_event_read()
{
    const int i_recv_buff_size = 10 * 1024 * 1024;
    static char *p_buffer = new char[i_recv_buff_size]();
    if( NULL == p_buffer ){
        g_tcp_log_callback( LOG_LEVEL_ERROR, "on_event_read, malloc failed" );
        return false;
    }

    memset( p_buffer, 0, i_recv_buff_size );

    //recv begin
    send_recv_data( NULL, 0, STATUS_RECV_BEGIN );
    //int i_recv_size = 0;
    while( true ){        
        memset( p_buffer, 0, i_recv_buff_size );
        //int i_recv_tmp = ::read( mi_socket, p_buffer + i_recv_size, i_recv_buff_size - i_recv_size );
        int i_recv_tmp = ::read( mi_socket, p_buffer, i_recv_buff_size );
        int err = errno;
        if( i_recv_tmp <= 0 ){
            if( (i_recv_tmp < 0) && (err != ECONNRESET) ){
                if( (err == EAGAIN) || (err == EWOULDBLOCK) ){
                    break;
                }
                if( err == EINTR ){
                    CSingletonMsEpollHandler::Instance()->mod_epoll_read( mi_socket, mh_link );
                }
                g_tcp_log_callback( LOG_LEVEL_ERROR, "read error, read less than 0" );
                break;
            } else {
                printf( "errno = %d\n", err );
                g_tcp_log_callback( LOG_LEVEL_ERROR, "read error, read len 0." );
                CSingletonMsEpollHandler::Instance()->del_epoll_read( mi_socket );
                //printf( "link_destroy 3\n" );
                link_destroy();
                break;
            }
        } else {
            //OK
            //send_recv_data( NULL, 0, STATUS_RECV_MIDDLE );
            send_recv_data( p_buffer, i_recv_tmp, STATUS_RECV_MIDDLE );
        }
        //i_recv_size += i_recv_tmp;
    }
    //recv end
    //send_recv_data( p_buffer, i_recv_size, STATUS_RECV_END );
    send_recv_data( NULL, 0, STATUS_RECV_END );

    time( &mt_last_alive_time );        

    return true;
}

bool MsTcpLink::on_event_write()
{    
    return true;
}

int MsTcpLink::set_conn_nonblock( int fd )
{
    int flag = fcntl( fd, F_GETFL, 0);
    if( -1 == flag ) {
        g_tcp_log_callback( LOG_LEVEL_FATAL, "fcntl get error");
        return -1;
    }
    flag |= O_NONBLOCK;
    if( fcntl( fd, F_SETFL, flag ) == -1 ) {
        g_tcp_log_callback( LOG_LEVEL_FATAL, "fcntl set error");
        return -1;
    }

    return 0;
}

int MsTcpLink::link_send_data( const unsigned char* p_data, unsigned int ui_size, int *i_error )
{
    if( p_data == NULL ){
        *i_error = 0;
        return 0;
    }

//    if( mb_write_flag == false ){
//        *i_error = EAGAIN;
//        return 0;
//    }

    time( &mt_last_alive_time );
    int i_write = ::write( mi_socket, p_data, ui_size );
    //int i_write = ::send( mi_socket, p_data, ui_size, 0 );
    *i_error = errno;

//    if( (i_write <= 0) && ( *i_error == EAGAIN ) ){
//        set_link_write_flag( false );
//    }

    return i_write;
}

int MsTcpLink::send_data( const char* p_data, unsigned int ui_size )
{
    int i_write = ::write( mi_socket, p_data, ui_size );
    return i_write;
}

bool MsTcpLink::accept( unsigned int ui_listen_fd, ms_tcp_io_callback p_callback )
{
    struct sockaddr_in clientaddr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    int ui_socket = ::accept( ui_listen_fd, (sockaddr *)&clientaddr, &clilen );
    if( ui_socket < 0 ){
        int i_error = errno;
        std::string str_msg = "accept failed. errno=" + T2string( i_error );
        g_tcp_log_callback( LOG_LEVEL_ERROR, str_msg.c_str() );
        return false;
    }

    memcpy( &m_remote_address, &clientaddr, sizeof(clientaddr) );

    socklen_t local_size = sizeof(m_local_address);
    if (-1 == ::getsockname(ui_socket, (sockaddr*)&m_local_address, &local_size)){
        ::close(ui_socket);
        return false;
    }

    mp_tcpcallback = p_callback;
    set_link_type( link_TypeAccept );

    mi_socket = ui_socket;

    //set tcp_nodelay
    int flag = 1;
    setsockopt( ui_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof( int ) );

    set_check_alive_flag( true );
    set_conn_nonblock( ui_socket );

    return true;
}

void MsTcpLink::send_link_event( unsigned int ui_event )
{
    TcpEvent_S st_event;
    st_event.h_link = mh_link;
    st_event.ui_event = ui_event;
    (*mp_tcpcallback)( &st_event, 0 );
}

void MsTcpLink::send_accept_link( HTCPLINK h_accept_link )
{
    TcpEvent_S st_event;
    st_event.h_link = mh_link;
    st_event.ui_event = EVT_TCP_ACCEPTED;

    TcpAcceptedParam_S param;
    param.h_acceptLink = h_accept_link;
    (*mp_tcpcallback)( &st_event, &param );
}

void MsTcpLink::send_recv_data( char *buffer, int size, int i_status )
{
    TcpEvent_S st_event;
    st_event.h_link = mh_link;
    st_event.ui_event = EVT_TCP_RECEIVEDATA;

    TcpRecvDataParam_S param;
    param.p_RecvData = buffer;
    param.ui_size = size;
    param.ui_status = i_status;
    (*mp_tcpcallback)( &st_event, &param );
}

void MsTcpLink::update_link_time( time_t &st_time )
{
    CGuardLock<pthread_mutex_t> guard( &mo_last_alive_time_lock );
    mt_last_alive_time = st_time;
}

time_t MsTcpLink::get_link_time()
{
    CGuardLock<pthread_mutex_t> guard( &mo_last_alive_time_lock );
    return mt_last_alive_time;
}

bool MsTcpLink::is_alive( time_t &st_time )
{
    if( mb_need_check_alive ){
        if( st_time < get_link_time() ){
            update_link_time( st_time );
            return true;
        }
        return (st_time - get_link_time()) < MsTcpLink::ui_check_alive_timeout;
    }
    return true;
}

bool MsTcpLink::get_link_addr( unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port )
{
    sockaddr_in *p_addr = NULL;
    if( ui_type == LINK_ADDR_LOCAL ){
        p_addr = &m_local_address;
    } else if( ui_type == LINK_ADDR_REMOTE ){
        p_addr = &m_remote_address;
    } else {
        return false;
    }
    *pui_ip = ntohl( p_addr->sin_addr.s_addr );
    *pus_port = ntohs( p_addr->sin_port );
    return true;
}

void MsTcpLink::set_check_alive_flag( bool b_flag )
{
    mb_need_check_alive = b_flag;
}
