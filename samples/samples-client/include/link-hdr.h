#ifndef LINK_HDR_H_
#define LINK_HDR_H_

#include "ms-epoll-tcp.h"
#include "clientlog-api.h"
#include "Guard.h"
#include "comm_typedef.h"
#include "net-packet.h"

template<typename PACKET>
class LinkHdr{
public:
    LinkHdr( std::string str_hdr_name )
        : mh_remote_tcplink( INVALID_HTCPLINK ),
          mstr_link_name( str_hdr_name ),
          mus_remote_port( 0 ),
          mus_local_port( 0 ),
          mull_pkt_id( 1 ),
          mb_isalive( false ),
          mb_can_broadcast( false )
    {

    }

    virtual ~LinkHdr()
    {
        MS_LOGER_INFO( "LinkHdr Destruct." );
    }

public:
    void set_link( HTCPLINK h_link )
    {
        mh_remote_tcplink = h_link;
    }

    bool is_local_link()
    {
        return false;
        //if( mstr_remote_ip == mstr_local_ip ){
        //    return true;
        //}
        //return false;
    }

    bool send_packet( PACKET *p_pkt )
    {
        //MS_TcpSend()
        if( p_pkt == NULL ){
            return false;
        }

        std::vector<char> vc_buffer;
        switch( p_pkt->msg_type ){
#define CASE_PACKET( n, T_resp )\
        case n: {\
            write_packet(vc_buffer, *(T_resp*)p_pkt);\
            ::ms_tcp_send( mh_remote_tcplink, (INT8 *)&vc_buffer[0], vc_buffer.size() );\
            return 0;\
            }
        CASE_PACKET( IR_MONITOR_VER_NEGO_RESP, MSCommResp_S );
        CASE_PACKET( IR_MONITOR_LINK_AUTH_RESP, MSAuthResp_S  );
#undef CASE_PACKET
        default:
            break;
        }

        return true;
    }

    bool send_packet( const char *p_data, const unsigned int ui_len )
    {
        return ::ms_tcp_send( mh_remote_tcplink, p_data, ui_len );
    }

    HTCPLINK get_link(){ return mh_remote_tcplink; }
    void set_remote_ip( std::string &str_ip ){ mstr_remote_ip = str_ip; }

    void destroy()
    {
        ms_tcp_destroy( mh_remote_tcplink );
    }

    void set_can_broadcast( bool b_flag ){ mb_can_broadcast = b_flag; }
    bool can_broadcast(){ return mb_can_broadcast; }

    std::string &get_link_name(){ return mstr_link_name; }

    virtual bool recv_data( const CRefCountPtr<PACKET> &p_pkt ) = 0;

private:
    HTCPLINK         mh_remote_tcplink;
    std::string      mstr_remote_ip;
    unsigned short   mus_remote_port;
    std::string      mstr_local_ip;
    unsigned short   mus_local_port;

    unsigned int     mui_keepalive_tick;

    mutable unsigned long long mull_pkt_id;

    std::string      mstr_link_name;
    mutable bool     mb_isalive;

    bool             mb_can_broadcast;
};















#endif
