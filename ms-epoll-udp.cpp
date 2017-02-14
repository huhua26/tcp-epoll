#include <signal.h>
#include "ms-epoll-udp.h"
#include "udp-link.h"
#include "udp-ms-epoll.h"
#include "clientlog-api.h"

std::string GetExePath( std::string &str_exepath )
{
    char sysfile[15] = "/proc/self/exe";
    char szFile[512] = {0};
    readlink( sysfile, szFile, 512 );

    char* psz = strrchr(szFile,'\\');
    if( NULL == psz)
    {
        psz = strrchr(szFile,'/');
    }
    if( psz != NULL)
    {
        *(psz+1)='\0';
    }

    str_exepath = szFile;
    return str_exepath;
}

bool ms_udp_init()
{
    signal( SIGPIPE, SIG_IGN );
    std::string str_exe_path;
    GetExePath( str_exe_path );
    str_exe_path += "MS_EPOLLUDP/";
    ms_log_init( str_exe_path.c_str(), LOG_Local );

    return CSingletonMsUdpEpollHandler::Instance()->init();
}

void ms_udp_uninit()
{
    CSingletonMsUdpEpollHandler::Instance()->end();
    CSingletonMsUdpEpollHandler::Release();

    return;
}

HUDPLINK ms_udp_create( ms_udp_io_callback backFunc, int i_flag, const char *p_localAddr, unsigned short us_localPort )
{
    MsUdpLink *p_link = new MsUdpLink();
    if( p_link && p_link->link_create( backFunc, i_flag, p_localAddr, us_localPort ) ){
        CRefCountPtr<MsUdpLink> p_reflink = CRefCountPtr<MsUdpLink>( p_link );
        HUDPLINK h_link = INVALID_HUDPLINK;
        MsUdpLink::add_tcp_link( h_link, p_reflink );
        p_reflink->set_link( h_link );
        CSingletonMsUdpEpollHandler::Instance()->attach_to_epoll( p_reflink->get_socket(), h_link );
        p_reflink->send_link_event( EVT_UDP_CREATED );
        return h_link;
    } else {
        MS_LOGER_ERROR( "ms_udp_create failed." );
        delete p_link;
    }
    return INVALID_HUDPLINK;
}

bool ms_udp_destroy( HUDPLINK h_link )
{
    CRefCountPtr<MsUdpLink> p_reflink;
    if( MsUdpLink::find_link( h_link, p_reflink ) ){
        p_reflink->link_destroy();
        return true;
    } else {
        MS_LOGER_ERROR( "ms_udp_destroy failed. can not find link." );
    }

    return false;
}

bool ms_udp_send_to( HUDPLINK h_link, const char* p_data, unsigned int ui_size, int i_flag, const char *p_ip, unsigned short us_port )
{
    CRefCountPtr<MsUdpLink> p_reflink;
    if( MsUdpLink::find_link( h_link, p_reflink ) ){
        return p_reflink->link_send_to( p_data, ui_size, i_flag, p_ip, us_port );
    } else {
        MS_LOGER_ERROR( "ms_udp_send failed. can not find link." );
    }

    return false;
}
