#ifndef TEST_CLIENT_H_
#define TEST_CLIENT_H_

#include <pthread.h>
#include <semaphore.h>
#include "net-handler.h"
#include "comm_typedef.h"



class Test_Client : public TcpNetHandler< TcpNetProtocolEvent<MsgHeader_S> >
{
public:
    Test_Client( std::string str_server_ip, unsigned short us_port );
    virtual ~Test_Client();

    bool Init();
    bool Uninit();

    static void* handle_recv_data( void * );

public:
    virtual void do_post_recv_buff( HTCPLINK h_link, const char* p_data, unsigned int ui_size );
    virtual void on_tcp_created( HTCPLINK h_link );
    virtual void on_tcp_closed( HTCPLINK h_link );
    virtual void on_tcp_accepted( HTCPLINK h_link, HTCPLINK h_accept_link );
    virtual void on_tcp_connected( HTCPLINK h_link );
    virtual void on_tcp_disconnected( HTCPLINK h_link );
    virtual void on_tcp_recv_data( HTCPLINK h_link, const CRefCountPtr<MsgHeader_S>& p_pkt );



private:
    void set_link( HTCPLINK h_link ){ CGuardLock<pthread_mutex_t> guard( &mo_link_lock ); mh_tcpLink = h_link; }
    HTCPLINK get_link(){ CGuardLock<pthread_mutex_t> guard( &mo_link_lock ); return mh_tcpLink; }

    bool process_connect_server();

    void do_ver_nego_req();
    void on_ver_nego_ack( const CRefCountPtr<MSCommResp_S> &p_pkt );

    void do_link_auth_req();
    void on_link_auth_ack( const CRefCountPtr<MSAuthResp_S> &p_pkt );


private:
    pthread_mutex_t  mo_link_lock;
    HTCPLINK  mh_tcpLink;

    pthread_mutex_t  mo_addr_lock;
    std::string mstr_serverAddr;
    unsigned short ms_serverPort;
    std::string mstr_localAddr;

    pthread_mutex_t  mo_client_lock;
    unsigned int mui_client_id;

    bool       mb_thread_break;
    pthread_t  mo_thread_id;
    sem_t      mo_buff_sem;

    pthread_mutex_t    mo_recv_buf_old_lock;
    std::vector<char>  mv_recv_buf_old;
    std::vector<char>  mv_recv_buf;
};

#endif
