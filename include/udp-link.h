#ifndef UDP_LINK_H_
#define UDP_LINK_H_

#include <map>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/tcp.h>
#include <list>

#include "ms-epoll-udp.h"
#include "Guard.h"

bool get_ip_by_name(const std::string& name, unsigned int& ipv4);

class MsUdpLink
{
public:
    struct send_buff_s{
        send_buff_s(){
            p_data = NULL;
            ui_size = 0;
            memset( &dst_addr, 0, sizeof(dst_addr) );
        }
        char *p_data;
        unsigned int ui_size;
        sockaddr_in dst_addr;
    };

    MsUdpLink();
    virtual~MsUdpLink();
    static void init_link_lock(){
        pthread_mutex_init( &m_link_lock, NULL );
    }

    static void init_link_map(){
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        m_maplink.clear();
    }

    static void destroy_link_lock(){
        pthread_mutex_destroy( &m_link_lock );
    }

    static void set_send_flag( bool b_flag ){ mb_send_flag = b_flag; }
    static bool get_send_flag() { return mb_send_flag; }

public:
    static bool find_link( HUDPLINK h_link, CRefCountPtr<MsUdpLink> &p_link );
    static bool del_link( HUDPLINK h_link );
    static void free_all_link_map();

    static void add_tcp_link( HUDPLINK &h_link, CRefCountPtr<MsUdpLink> p_link )
    {
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        static int i_link = 0;
        h_link = (++i_link)%0x3ffffffff;
        m_maplink[h_link] = p_link;
    }

    static void do_send_buff();

public:
    operator HUDPLINK() const { return mh_link; }
    bool link_create( ms_udp_io_callback backFunc, int i_flag, const char* p_localAddr, unsigned short us_localPort );
    bool link_send_to( const char* p_data, unsigned int ui_size, int i_flag, const char *p_ip, unsigned short us_port );
    void link_destroy();


    int get_socket(){ return mi_socket; }
    HUDPLINK get_link(){ return mh_link; }
    void set_link( HUDPLINK h_link ){ mh_link = h_link; }

    bool get_link_addr( unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port );

    void send_link_event( unsigned int ui_event );
    void send_recv_data( char *buffer, int size );

    void del_send_buff();

    bool on_event_read();

private:
    static pthread_mutex_t m_link_lock;
    static std::map<HUDPLINK, CRefCountPtr<MsUdpLink> > m_maplink;
    static bool   mb_send_flag;
public:
    int        mi_socket;
    HUDPLINK   mh_link;
    int        mi_create_flag;
    ms_udp_io_callback  mp_udpcallback;

    sockaddr_in  m_local_address;
    sockaddr_in  m_remote_address;

    pthread_mutex_t  mo_send_buff_list_lock;
    std::list<send_buff_s>  m_send_buff_list;
};

#endif
