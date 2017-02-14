#ifndef UDP_NETHANDLER_H_
#define UDP_NETHANDLER_H_

#include <vector>
#include "clientlog-api.h"
#include "ms-epoll-udp.h"
#include "Guard.h"
#include "comm-func.h"
#include "comm_typedef.h"
#include "net-packet.h"

template<typename PACKET>
class UdpNetProtocolEvent
{
public:
    virtual ~UdpNetProtocolEvent(){

    }

public:
    virtual void on_udp_created( HUDPLINK h_link ) = 0;
    virtual void on_udp_closed( HUDPLINK h_link ) = 0;
    virtual void on_udp_recv_data( HUDPLINK h_link, const CRefCountPtr<PACKET> &p_pkt, char *p_ip, unsigned short us_port ) = 0;
    virtual void on_udp_recv_unknown_packet( HUDPLINK h_link, const char *p_data, unsigned int ui_size, char *p_ip, unsigned short us_port )
    {
        MS_LOGER_WARNING( "UdpNetProtocolEvent link[0x%04x] OnUdpRecvUnknownPacket pktsize=%d, ip:port=%s:%d",
                          h_link, ui_size, p_ip, us_port );
    }

    virtual void on_udp_recv_data( HUDPLINK h_link, const char *p_data, unsigned int ui_size, char *p_ip, unsigned short us_port )
    {
        PACKET *p_msg = NULL;
        int i_parse_len = parse_data( p_data, ui_size, p_msg );
        if( i_parse_len > 0 ){
            CRefCountPtr<PACKET> pcref_packet = CRefCountPtr<PACKET>( p_msg );
            on_udp_recv_data( h_link, pcref_packet, p_ip, us_port );
        } else if( i_parse_len == 0 ){
            MS_LOGER_ERROR( "parse packet error." );
        } else {
            if( i_parse_len == CTRL_PRO_NOT_EXIST_ERROR ){
                on_udp_recv_unknown_packet( h_link, p_data, ui_size, p_ip, us_port );
            }
        }
    }

    static bool send_packet_to( HUDPLINK h_link, const CRefCountPtr<PACKET> &p_pkt, const char *p_ip, unsigned short us_port, int i_flag )
    {
        return true;
    }

    static bool send_packet_to( HUDPLINK h_link, const PACKET &p_pkt, const char *p_ip, unsigned short us_port, int i_flag )
    {
        return true;
    }

    static bool send_packet_to( HUDPLINK h_link, const PACKET *p_pkt, const char *p_ip, unsigned short us_port, int i_flag )
    {
        if( p_pkt == NULL ){
            return false;
        }

        std::vector<char> vc_buffer;
        switch( p_pkt->msg_type ){
#define CASE_PACKET( n, T_req )\
        case n: {\
            write_packet(vc_buffer, *(T_req*)p_pkt);\
            ::ms_udp_send_to( h_link, (INT8 *)&vc_buffer[0], vc_buffer.size(), i_flag, p_ip, us_port );\
            break;\
            }
        CASE_PACKET( IR_MONITOR_VER_NEGO_REQ, MSVerReq_S );
        CASE_PACKET( IR_MONITOR_VER_NEGO_RESP, MSCommResp_S );
        CASE_PACKET( IR_MONITOR_LINK_AUTH_REQ,  MSAuthReq_S );
        CASE_PACKET( IR_MONITOR_LINK_AUTH_RESP,  MSAuthResp_S );
#undef CASE_PACKET
        default:
            break;
        }

        vc_buffer.clear();
        return true;
    }

};

template<typename HANDLE>
class UdpNetHandler : public HANDLE
{
public:
    UdpNetHandler(){
        mp_this = this;
    }
    virtual ~UdpNetHandler(){
        mp_this = NULL;
    }

    static void udp_io_callback_func( void *p_param1, void *p_param2 )
    {
        if( mp_this == NULL ){
            return;
        }

        UdpEvent_S *p_event = (UdpEvent_S *)p_param1;
        UdpRecvDataParam_S *p_recv = (UdpRecvDataParam_S *)p_param2;

        try{
            switch( p_event->ui_event ){
            case EVT_UDP_CREATED:
                mp_this->on_udp_created( p_event->h_link );
                break;
            case EVT_UDP_CLOSED:
                mp_this->on_udp_closed( p_event->h_link );
                break;
            case EVT_UDP_RECEIVEDATA:
                mp_this->on_udp_recv_data( p_event->h_link, p_recv->p_RecvData, p_recv->ui_size, p_recv->p_dst_ip, p_recv->us_dst_port );
                break;
            }
        } catch(...){
            ;
        }

    }

    static HUDPLINK udp_create( const char *p_addr, unsigned short us_port, int i_flag )
    {
        return ::ms_udp_create( UdpNetHandler<HANDLE>::udp_io_callback_func, i_flag, p_addr, us_port  );
    }

    static bool udp_destroy( HUDPLINK h_link )
    {
        return ::ms_udp_destroy( h_link );
    }

    static bool udp_send_to( HUDPLINK h_link, const char *p_data, unsigned int ui_size, int i_flag,
                             const char *p_ip, unsigned short us_port )
    {
        return ::ms_udp_send_to( h_link, p_data, ui_size, i_flag, p_ip, us_port );
    }

private:
    static UdpNetHandler<HANDLE> *mp_this;

};

template<typename HANDLE>
UdpNetHandler<HANDLE> *UdpNetHandler<HANDLE>::mp_this = NULL;

#endif
