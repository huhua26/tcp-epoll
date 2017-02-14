#ifndef TCP_LINK_H_
#define TCP_LINK_H_

#include <map>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/tcp.h>
#include <list>
#include <stdio.h>

#include "ms-epoll-tcp.h"
#include "Guard.h"
#include "ms-epoll.h"

enum
{
    LOG_LEVEL_INFO       = 1,     //1
    LOG_LEVEL_WARNNING   = 1<<1,  //2
    LOG_LEVEL_ERROR      = 1<<2,  //4
    LOG_LEVEL_DEBUG      = 1<<3,  //8
    LOG_LEVEL_FATAL      = 1<<4,  //16


    LOG_LEVEL_MAX        = LOG_LEVEL_INFO | LOG_LEVEL_WARNNING | LOG_LEVEL_ERROR | LOG_LEVEL_DEBUG | LOG_LEVEL_FATAL,

};

void default_tcp_log_callback_func( int i_level, const char *pc_message );



bool get_ip_by_name(const std::string& name, unsigned int& ipv4);



struct send_buff_s{
    send_buff_s(){
        p_data = NULL;
        ui_size = 0;
        ui_send_times = 0;
    }
    char *p_data;
    unsigned int ui_size;
    unsigned int ui_send_times;
};

struct err_info_s{
    err_info_s(){
        i_error_no = 0;
        ui_time = 0;
    }
    int i_error_no;
    unsigned int ui_time;
};

class MsTcpLink
{
public:
    enum{
        link_TypeNull = 0,
        link_TypeListen = 1,
        link_TypeConnect = 2,
        link_TypeAccept = 3,
        link_TypeConnected = 4,

        link_TypeError
    };

    MsTcpLink();
    ~MsTcpLink();
    static void init_link_lock(){
        pthread_mutex_init( &m_link_lock, NULL );
    }

    static void init_link_map(){
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        m_maplink.clear();
        m_link_socket_map.clear();
    }

    static void destroy_link_lock(){
        pthread_mutex_destroy( &m_link_lock );
    }

public:
    static bool find_link( HTCPLINK h_link, CRefCountPtr<MsTcpLink> &p_link );
    static bool del_link( HTCPLINK h_link );
    static void free_all_link_map();
    static void check_link_timeout();

    static void add_tcp_link( HTCPLINK &h_link, CRefCountPtr<MsTcpLink> p_link )
    {
        CGuardLock<pthread_mutex_t> guard( &m_link_lock );
        static int i_link = 0;
        h_link = (++i_link)%0x3ffffffff;
        m_maplink[h_link] = p_link;
        m_link_socket_map[h_link] = p_link->get_socket();
    }

    static int send_data_socket( int i_socket, const char* p_data, unsigned int ui_size );

    static bool check_error_info( HTCPLINK h_link );
    static bool set_error_info( HTCPLINK h_link, int i_error_no );
private:
    static pthread_mutex_t m_link_lock;
    static std::map<HTCPLINK, CRefCountPtr<MsTcpLink> > m_maplink;
    static std::map<HTCPLINK, int> m_link_socket_map;
    static std::map<HTCPLINK, err_info_s> m_link_errinfo;
    static const unsigned int ui_check_alive_timeout = 5 * 60;   //delete socket when no recv data after accept 5 min.
    static std::map<HTCPLINK, std::list< send_buff_s > >  m_send_buff_list_map;
    static bool mb_send_buff_thread_flag;
    static pthread_t mo_thread_id;

public:
    operator HTCPLINK() const { return mh_link; }
    bool link_create( ms_tcp_io_callback backFunc, const char* p_localAddr, unsigned short us_localPort );
    bool link_connect( const char* p_addr, unsigned short us_port );
    bool link_listen( unsigned int ui_block );
    void link_destroy( bool b_self = true );

    bool on_event_accept( );
    bool on_event_connected();
    bool on_event_read();
    bool on_event_write();

    int set_conn_nonblock( int fd );

    int link_send_data( const unsigned char* p_data, unsigned int ui_size, int *i_error );
    int send_data( const char* p_data, unsigned int ui_size );
    bool accept( unsigned int ui_listen_fd, ms_tcp_io_callback p_callback );

public:
    HTCPLINK get_link(){ return mh_link; }
    void set_link( HTCPLINK h_link ){ mh_link = h_link; }

    void send_link_event( unsigned int ui_event );
    void send_accept_link( HTCPLINK h_accept_link );
    void send_recv_data( char *buffer, int size, int i_status );

    void set_link_type( int i_type ) { m_link_type = i_type; }
    int get_link_type() { return m_link_type; }

    void update_link_time( time_t &st_time );
    time_t get_link_time();
    bool is_alive( time_t &st_time );

    int get_socket(){ return mi_socket; }

    bool get_link_addr( unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port );

    bool set_link_write_flag( bool b_flag ){ mb_write_flag = b_flag; return true; }

private:
    void set_check_alive_flag( bool b_flag );

public:
    int         mi_socket;
    HTCPLINK    mh_link;
    ms_tcp_io_callback  mp_tcpcallback;

    sockaddr_in  m_local_address;
    sockaddr_in  m_remote_address;

    int     m_link_type;

    pthread_mutex_t  mo_last_alive_time_lock;
    time_t  mt_last_alive_time;
    bool    mb_need_check_alive; //set true after accept

    bool mb_write_flag;

};


#endif
