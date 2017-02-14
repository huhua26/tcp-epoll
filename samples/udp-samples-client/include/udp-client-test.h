#ifndef UDP_CLIENT_TEST_H_
#define UDP_CLIENT_TEST_H_

#include "udp-nethandler.h"
#include "net-packet.h"

class UdpClient : public UdpNetHandler< UdpNetProtocolEvent<MsgHeader_S> >
{
public:
    UdpClient(){
        mh_link = INVALID_HUDPLINK;
    }
    virtual ~UdpClient(){

    }

    bool init();
    void uninit();
public:
    virtual void on_udp_created( HUDPLINK h_link );
    virtual void on_udp_closed( HUDPLINK h_link );
    virtual void on_udp_recv_data( HUDPLINK h_link, const CRefCountPtr<MsgHeader_S> &p_pkt, char *p_ip, unsigned short us_port );

public:
    void do_ver_nego_req( std::string str_ip, unsigned short us_port );
    bool on_recv_ver_nego_req( const CRefCountPtr<MSVerReq_S> &p_pkt, char *p_ip, unsigned short us_port );
    bool on_recv_ver_nego_resq( const CRefCountPtr<MSCommResp_S> &p_pkt, char *p_ip, unsigned short us_port );

    void do_link_auth_req( char *p_ip, unsigned short us_port );
    bool on_recv_link_auth_req( const CRefCountPtr<MSAuthReq_S> &p_pkt, char *p_ip, unsigned short us_port );
    bool on_recv_link_auth_resp( const CRefCountPtr<MSAuthResp_S> &p_pkt, char *p_ip, unsigned short us_port );

private:
    HUDPLINK  mh_link;
};


#endif
