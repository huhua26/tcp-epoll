#include <signal.h>
#include "ms-epoll-tcp.h"
#include "tcp-link.h"
#include "ms-epoll.h"

extern HTCPLINK  g_ms_tcp_first_create_link;
extern ms_tcp_log_callback_func  g_tcp_log_callback ;

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

bool ms_tcp_init( )
{
    signal( SIGPIPE, SIG_IGN );

    return CSingletonMsEpollHandler::Instance()->init();
}

void ms_tcp_uninit()
{
    CSingletonMsEpollHandler::Instance()->end();
    CSingletonMsEpollHandler::Release();
    return;
}

HTCPLINK ms_tcp_create( ms_tcp_io_callback backFunc, const char* p_localAddr, unsigned short us_localPort )
{
    MsTcpLink *p_link = new MsTcpLink();
    if( p_link && p_link->link_create( backFunc, p_localAddr, us_localPort ) ){
        CRefCountPtr<MsTcpLink> p_reflink = CRefCountPtr<MsTcpLink>( p_link );
        HTCPLINK h_link = INVALID_HTCPLINK;
        MsTcpLink::add_tcp_link( h_link, p_reflink );
        p_reflink->set_link( h_link );
        p_reflink->send_link_event( EVT_TCP_CREATED );

        if( g_ms_tcp_first_create_link == INVALID_HTCPLINK ){
            printf( "*********************ms_tcp_create link=0x%04x********************************\n" );
            g_ms_tcp_first_create_link = h_link;
        }

        return h_link;
    } else {
        g_tcp_log_callback( LOG_LEVEL_ERROR,  "ms_tcp_create failed." );
        delete p_link;
    }

    return INVALID_HTCPLINK;
}

bool ms_tcp_destroy( HTCPLINK h_link )
{
    CRefCountPtr<MsTcpLink> p_link;
    if( MsTcpLink::find_link( h_link, p_link ) ){
        p_link->link_destroy();
        return true;
    } else {
        g_tcp_log_callback( LOG_LEVEL_ERROR, "ms_tcp_destroy failed. can not find link." );
    }

    return false;
}

bool ms_tcp_connect( HTCPLINK h_link, const char* p_addr, unsigned short us_port )
{
    CRefCountPtr<MsTcpLink> p_link;
    if( MsTcpLink::find_link( h_link, p_link ) ){
        if( p_link->link_connect( p_addr, us_port ) ){
            CSingletonMsEpollHandler::Instance()->attach_to_epoll( p_link->get_socket(), h_link );

            return true;
        } else {
            g_tcp_log_callback( LOG_LEVEL_ERROR, "link_connect failed. " );
        }
    } else {
        g_tcp_log_callback( LOG_LEVEL_ERROR, "ms_tcp_connect failed. can not find link." );
    }

    return false;
}

bool ms_tcp_listen( HTCPLINK h_link, unsigned int ui_block )
{
    CRefCountPtr<MsTcpLink> p_link;
    if( MsTcpLink::find_link( h_link, p_link ) ){
        if( p_link->link_listen( ui_block ) ){
            int i_fd = p_link->get_socket();
            CSingletonMsEpollHandler::Instance()->attach_to_epoll( i_fd, h_link );

            return true;
        } else {
            g_tcp_log_callback( LOG_LEVEL_ERROR, "link_listen failed." );
        }
    } else {
        g_tcp_log_callback( LOG_LEVEL_ERROR, "ms_tcp_listen failed. can not find link." );
    }

    return false;
}

unsigned int ms_tcp_send( HTCPLINK h_link, const unsigned char* p_data, unsigned int ui_size, int *i_error )
{
    CRefCountPtr<MsTcpLink> p_link;
    if( MsTcpLink::find_link( h_link, p_link ) ){
        return p_link->link_send_data( p_data, ui_size, i_error );
    }

    *i_error = -1;
    g_tcp_log_callback( LOG_LEVEL_ERROR, "ms_tcp_send failed." );
    return 0;
}

bool ms_tcp_get_link_addr( HTCPLINK h_link, unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port )
{
    CRefCountPtr<MsTcpLink> p_link;
    if( MsTcpLink::find_link( h_link, p_link ) ){
        return p_link->get_link_addr( ui_type, pui_ip, pus_port );
    }

    g_tcp_log_callback( LOG_LEVEL_ERROR, "ms_tcp_get_link_addr failed." );
    return false;
}

bool ms_tcp_set_log_callback_func( ms_tcp_log_callback_func func )
{
    if( func == NULL ){
        return false;
    }

    g_tcp_log_callback = func;
    return true;
}
