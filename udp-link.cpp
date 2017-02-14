#include "udp-link.h"
#include "clientlog-api.h"
#include "udp-ms-epoll.h"

pthread_mutex_t MsUdpLink::m_link_lock;
std::map<HUDPLINK, CRefCountPtr<MsUdpLink> > MsUdpLink::m_maplink;
bool MsUdpLink::mb_send_flag;

//from MSLogServer
bool get_ip_by_name(const std::string& name, unsigned int& ipv4)
{
    if (name.empty())
    {
        return false;
    }

    unsigned long ip = ::inet_addr(name.c_str());
    if (ip != INADDR_NONE)
    {
        ipv4 = ntohl(ip);
        return true;
    }

    hostent* host = ::gethostbyname(name.c_str());
    if (host != NULL &&
        host->h_addrtype == AF_INET &&
        host->h_addr_list[0] != NULL)
    {
        ip = *(unsigned long*)host->h_addr_list[0];
        ipv4 = ntohl(ip);
        return true;
    }
    else
    {
        return false;
    }

}


MsUdpLink::MsUdpLink()
{
    pthread_mutex_init( &mo_send_buff_list_lock, NULL );
    mi_socket = -1;
    mh_link = 0;
    mi_create_flag = -1;
    mp_udpcallback = NULL;
    memset( &m_local_address, 0, sizeof( m_local_address ) );
    memset( &m_remote_address, 0, sizeof( m_remote_address ) );
}

MsUdpLink::~MsUdpLink()
{
    pthread_mutex_destroy( &mo_send_buff_list_lock );
}

bool MsUdpLink::find_link( HUDPLINK h_link, CRefCountPtr<MsUdpLink> &p_link )
{
    CGuardLock<pthread_mutex_t> guard( &m_link_lock );
    std::map<HUDPLINK, CRefCountPtr<MsUdpLink> >::iterator it = m_maplink.find( h_link );
    if( it == m_maplink.end() ){
        MS_LOGER_ERROR( "find link failed. link=0x%04x", h_link );
        return false;
    }
    p_link = it->second;

    return true;
}

bool MsUdpLink::del_link( HUDPLINK h_link )
{
    CGuardLock<pthread_mutex_t> guard( &m_link_lock );
    std::map<HUDPLINK, CRefCountPtr<MsUdpLink> >::iterator it = m_maplink.find( h_link );
    if( it == m_maplink.end() ){
        MS_LOGER_ERROR( "find link failed. link=0x%04x", h_link );
        return false;
    }
    CRefCountPtr<MsUdpLink> p_reflink = it->second;
    CSingletonMsUdpEpollHandler::Instance()->del_epoll_read( p_reflink->get_socket() );
    m_maplink.erase( it );

    return true;
}

void MsUdpLink::free_all_link_map()
{
    std::list<CRefCountPtr<MsUdpLink> > link_list;
    do{
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        if( m_maplink.empty() ){
            return;
        }
        std::map<HUDPLINK, CRefCountPtr<MsUdpLink> >::iterator it = m_maplink.begin();
        for( ; it != m_maplink.end(); ++it ){
            CRefCountPtr<MsUdpLink> p_link = it->second;
            link_list.push_back( p_link );
        }
    }while( 0 );

    std::list<CRefCountPtr<MsUdpLink> >::iterator it_link = link_list.begin();
    for( ; it_link != link_list.end(); ++it_link ){
        CRefCountPtr<MsUdpLink> p_link = *it_link;
        p_link->link_destroy();
    }
}


void MsUdpLink::do_send_buff()
{
    if( false == MsUdpLink::get_send_flag() ){
        return;
    }

    MS_LOGER_INFO( "do_send_buff" );

    MsUdpLink::set_send_flag( false );
    CGuardLock<pthread_mutex_t> guard( &m_link_lock );
    std::map<HUDPLINK, CRefCountPtr<MsUdpLink> >::iterator it_link = m_maplink.begin();
    for( ; it_link != m_maplink.end(); ++it_link ){
        CRefCountPtr<MsUdpLink> pcrf_link = it_link->second;

        //send data from databuf.
        CGuardLock<pthread_mutex_t> guard_buff( &pcrf_link->mo_send_buff_list_lock );
        std::list<send_buff_s>::iterator it = pcrf_link->m_send_buff_list.begin();
        for( ; it != pcrf_link->m_send_buff_list.end(); ){
            send_buff_s st_send_buff = *it;
            char *p_data = st_send_buff.p_data;
            unsigned int ui_size = st_send_buff.ui_size;
            sockaddr_in dst_addr = st_send_buff.dst_addr;
            if( (p_data == NULL) || (ui_size == 0) ){
                MS_LOGER_ERROR( "data buff is null." );
                if( p_data != NULL ){
                    delete []p_data;
                    p_data = NULL;
                }
                it = pcrf_link->m_send_buff_list.erase( it );
                continue;
            }

            if( ::sendto( pcrf_link->get_socket(), p_data, ui_size, 0, (sockaddr *)&dst_addr, sizeof( dst_addr ) ) < 0 ){
                int i_error = errno;
                if( i_error != EAGAIN ){
                    //close??
                    MS_LOGER_ERROR( "send data failed. error=%d,%s", i_error, strerror( i_error ) );
                } else {
                    MS_LOGER_ERROR( "send data failed. EAGAIN." );
                }
                break;
            } else {
                delete []p_data;
                p_data = NULL;
                it = pcrf_link->m_send_buff_list.erase( it );
            }
        }

        if( !pcrf_link->m_send_buff_list.empty() ){
            MsUdpLink::set_send_flag( true );
        }
    }

    return;
}

bool MsUdpLink::link_create( ms_udp_io_callback backFunc, int i_flag, const char* p_localAddr, unsigned short us_localPort )
{
    if( backFunc == NULL ){
        MS_LOGER_ERROR( "link_create failed. backfunc is null." );
    }

    mp_udpcallback = backFunc;
    mi_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( mi_socket < 0 ){
        int i_error = errno;
        MS_LOGER_ERROR( "create socket err. err = %d, %s", i_error, strerror( i_error ) );
        return false;
    }
    m_local_address.sin_family = AF_INET;
    m_local_address.sin_port = htons( us_localPort );

    unsigned int ui_ipv4 = 0;
    std::string str_name;
    if( p_localAddr ){
        str_name = p_localAddr;
        if( get_ip_by_name( str_name, ui_ipv4 ) ){
            m_local_address.sin_addr.s_addr = htonl( ui_ipv4 );
        } else {
            MS_LOGER_ERROR( "get_ip_by_name failed. ip=%s, port=%s", str_name.c_str(), us_localPort );
            m_local_address.sin_addr.s_addr = htonl( INADDR_ANY );
        }
    } else {
        m_local_address.sin_addr.s_addr = htonl( INADDR_ANY );
    }

    mi_create_flag = i_flag;
    if( mi_create_flag == UDP_FLAG_BROADCAST ){
        int i_broadcast = 1;
        if( setsockopt( mi_socket, SOL_SOCKET, SO_BROADCAST, (const void *)&i_broadcast, sizeof( int ) ) < 0 ){
            ::close( mi_socket );
            MS_LOGER_ERROR( "setsocketopt SO_BROADCAST failed. error=%d,%s", errno, strerror( errno ) );
            return false;
        }
        //m_local_address.sin_addr.s_addr = htonl( INADDR_BROADCAST );
    }

    int i_opt = 1;
    setsockopt( mi_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&i_opt, sizeof(int) );

    //from MSLogerServer
//        set_socket_opt(fd, SOL_SOCKET, SO_KEEPALIVE, (int)1);
//        set_socket_opt(fd, SOL_TCP, TCP_KEEPIDLE, (int)1800);

    if (::bind( mi_socket, (sockaddr*)&m_local_address, sizeof(m_local_address) ) < 0)
    {
        ::close( mi_socket );
        MS_LOGER_ERROR( "bind err. err = %d, %s", errno, strerror( errno ) );
        return false;
    }

    //update local address
    socklen_t len = sizeof( m_local_address );
    ::getsockname( mi_socket, (sockaddr *)&m_local_address, &len );    

    return true;
}

bool MsUdpLink::link_send_to( const char* p_data, unsigned int ui_size, int i_flag, const char *p_ip, unsigned short us_port )
{
    sockaddr_in dst_addr;
    bzero( &dst_addr, sizeof(sockaddr_in) );
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons( us_port );
    if( i_flag == UDP_FLAG_BROADCAST ){
        dst_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
    } else {
        if( p_ip ){
            dst_addr.sin_addr.s_addr = inet_addr( p_ip );
        } else {
            MS_LOGER_ERROR( "send flag=%d, ip is null.", i_flag );
            return false;
        }
    }

    if( ::sendto( mi_socket, p_data, ui_size, 0, (sockaddr *)&dst_addr, sizeof( dst_addr ) ) < 0 ){
        int i_error = errno;
        if( i_error != EAGAIN ){
            if( i_flag == UDP_FLAG_BROADCAST ){
                MS_LOGER_ERROR( "sendto failed. broadcast" );
            } else {
                MS_LOGER_ERROR( "sendto failed. ip:port=%s:%d", p_ip, us_port );
            }
            return false;
        }

        send_buff_s st_send_buff;
        char *p_buff = new char[ui_size];
        if( p_buff == NULL ){
            MS_LOGER_ERROR( "malloc failed." );
            return false;
        }
        memcpy( p_buff, p_data, ui_size );
        st_send_buff.p_data = p_buff;
        st_send_buff.ui_size = ui_size;
        st_send_buff.dst_addr = dst_addr;

        CGuardLock<pthread_mutex_t> guard( &mo_send_buff_list_lock );
        m_send_buff_list.push_back( st_send_buff );

        MsUdpLink::set_send_flag( true );
    }

    return true;
}

void MsUdpLink::link_destroy()
{
    del_send_buff();
    MsUdpLink::del_link( mh_link );
    if( mi_socket != -1 ){
        ::shutdown( mi_socket, SHUT_RDWR );
        mi_socket = -1;
        send_link_event( EVT_UDP_CLOSED );
    }
}

bool MsUdpLink::get_link_addr( unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port )
{
    sockaddr_in *p_addr = NULL;
    if( ui_type == LINK_ADDR_LOCAL ){
        p_addr = &m_local_address;
    } else if( ui_type == LINK_ADDR_REMOTE ){
        p_addr = &m_remote_address;
    } else {
        MS_LOGER_FATAL( "get_link_addr, undefine type=%d", ui_type );
        return false;
    }
    *pui_ip = ntohl( p_addr->sin_addr.s_addr );
    *pus_port = ntohs( p_addr->sin_port );
    return true;
}

void MsUdpLink::send_link_event( unsigned int ui_event )
{
    MS_LOGER_INFO( "link = 0x%04x, socket = %d, event=%d", mh_link, mi_socket, ui_event );
    UdpEvent_S st_event;
    st_event.h_link = mh_link;
    st_event.ui_event = ui_event;
    (*mp_udpcallback)( &st_event, 0 );
}

void MsUdpLink::send_recv_data( char *buffer, int size )
{
    MS_LOGER_INFO( "link = 0x%04x, recvdata", mh_link );
    UdpEvent_S st_event;
    st_event.h_link = mh_link;
    st_event.ui_event = EVT_UDP_RECEIVEDATA;

    UdpRecvDataParam_S param;
    param.p_RecvData = buffer;
    param.ui_size = size;
    param.p_dst_ip = inet_ntoa( m_remote_address.sin_addr );
    param.us_dst_port = ntohs( m_remote_address.sin_port );
    (*mp_udpcallback)( &st_event, &param );
}

void MsUdpLink::del_send_buff()
{
    CGuardLock<pthread_mutex_t> guard( &mo_send_buff_list_lock );
    std::list<send_buff_s>::iterator it = m_send_buff_list.begin();
    for( ; it != m_send_buff_list.end(); ){
        send_buff_s st_buff = *it;
        if( st_buff.p_data != NULL ){
            delete []st_buff.p_data;
            st_buff.p_data = NULL;
        }
        it = m_send_buff_list.erase( it );
    }
}

bool MsUdpLink::on_event_read()
{
    const int i_recv_buff_size = 10 * 1024 * 1024;
    char *p_buffer = new char[i_recv_buff_size]();
    if( p_buffer == NULL ){
        MS_LOGER_ERROR( "malloc failed. err=%s", strerror( errno ) );
        return false;
    }
    memset( &m_remote_address, 0, sizeof( m_remote_address ) );
    socklen_t i_len = sizeof( m_remote_address );
    int i_recv_size = ::recvfrom( mi_socket, p_buffer, i_recv_buff_size, 0, (sockaddr *)&m_remote_address, &i_len );
    if( i_recv_size > 0 ){
        ::getsockname( mi_socket, (sockaddr *)&m_local_address, &i_len );
        send_recv_data( p_buffer, i_recv_size );
    }
    delete []p_buffer;
    p_buffer = NULL;

    return true;
}
