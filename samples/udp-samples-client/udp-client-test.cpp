#include "udp-client-test.h"
#include "md5.h"
#include <stdio.h>

bool UdpClient::init()
{
    mh_link = UdpNetHandler< UdpNetProtocolEvent<MsgHeader_S> >::udp_create( NULL, 8889, UDP_FLAG_NONE );
    if( mh_link != INVALID_HUDPLINK ){
        return true;
    }

    return false;
}

void UdpClient::uninit()
{
    if( mh_link != INVALID_HUDPLINK ){
        UdpNetHandler< UdpNetProtocolEvent<MsgHeader_S> >::udp_destroy( mh_link );
    }

    mh_link = INVALID_HUDPLINK;
}

void UdpClient::on_udp_created( HUDPLINK h_link )
{
    mh_link = h_link;
    MS_LOGER_INFO( "on_udp_created, link=0x%04x.", h_link );
}

void UdpClient::on_udp_closed( HUDPLINK h_link )
{
    mh_link = INVALID_HUDPLINK;
    MS_LOGER_INFO( "on_udp_closed, link=0x%04x.", h_link );
}

void UdpClient::on_udp_recv_data( HUDPLINK h_link, const CRefCountPtr<MsgHeader_S> &p_pkt, char *p_ip, unsigned short us_port )
{
    if( h_link != mh_link ){
        MS_LOGER_ERROR( "Invalid udp link=0x%04x", h_link );
        return;
    }

    unsigned int ui_type = p_pkt->msg_type;
    MS_LOGER_INFO( "on_udp_recv_data, type=%d", ui_type );
    switch( ui_type ){
    case IR_MONITOR_VER_NEGO_REQ:
    {
        on_recv_ver_nego_req( CRefCountPtr<MSVerReq_S>(p_pkt), p_ip, us_port );
    }
        return;
    case IR_MONITOR_LINK_AUTH_REQ:
    {
        on_recv_link_auth_req( CRefCountPtr<MSAuthReq_S>(p_pkt), p_ip, us_port );
    }
        return;
    case IR_MONITOR_VER_NEGO_RESP:
    {
        on_recv_ver_nego_resq( CRefCountPtr<MSCommResp_S>(p_pkt), p_ip, us_port );
    }
        return;
    case IR_MONITOR_LINK_AUTH_RESP:
    {
        on_recv_link_auth_resp( CRefCountPtr<MSAuthResp_S>(p_pkt), p_ip, us_port );
    }
        return;
    default:
        return;
    }
}

void UdpClient::do_ver_nego_req( std::string str_ip, unsigned short us_port )
{
    MSVerReq_S *p_req = new MSVerReq_S();
    p_req->magic_type = MSG_HEAD_MAGIC;
    p_req->msg_type = IR_MONITOR_VER_NEGO_REQ;
    p_req->seq_id = 0;
    p_req->total_len = sizeof( MSVerReq_S );
    p_req->body.ui_major_ver = 1;
    p_req->body.ui_sub_ver = 0;

    send_packet_to( mh_link, p_req, str_ip.c_str(), us_port, UDP_FLAG_NONE );

    delete p_req;
    p_req = NULL;
}

bool UdpClient::on_recv_ver_nego_req( const CRefCountPtr<MSVerReq_S> &p_pkt, char *p_ip, unsigned short us_port )
{
    MSCommResp_S st_resp;
    st_resp.magic_type = p_pkt->magic_type;
    st_resp.msg_type = IR_MONITOR_VER_NEGO_RESP;
    st_resp.seq_id = p_pkt->seq_id;
    st_resp.total_len = sizeof( st_resp );

    if( p_pkt->body.ui_major_ver == 1 ){
        if( p_pkt->body.ui_sub_ver == 0 ){
            st_resp.i_ret = 0;
        } else {
            st_resp.i_ret = 2;
        }
    } else {
        st_resp.i_ret = 2;
    }

    return send_packet_to( mh_link, &st_resp, p_ip, us_port, UDP_FLAG_NONE );
}

bool UdpClient::on_recv_ver_nego_resq( const CRefCountPtr<MSCommResp_S> &p_pkt, char *p_ip, unsigned short us_port )
{
    if( p_pkt->i_ret != 0 ){
        MS_LOGER_ERROR( "on_recv_ver_nego_resq error." );
        return false;
    }

    do_link_auth_req( p_ip, us_port );
    return true;
}

void UdpClient::do_link_auth_req( char *p_ip, unsigned short us_port )
{
    MSAuthReq_S *p_auth_req = new MSAuthReq_S();
    p_auth_req->magic_type = MSG_HEAD_MAGIC;
    p_auth_req->msg_type = IR_MONITOR_LINK_AUTH_REQ;
    p_auth_req->seq_id = 0;
    p_auth_req->total_len = sizeof( MSAuthReq_S );

    UINT32 ui_sec = ::time(NULL);
    std::string str_time_of_day;
    GetLocalTimeDay( ui_sec, str_time_of_day );
    std::string str_user_name = MSADMIN_USER;
    std::string str_digest =  str_user_name + str_time_of_day;

    UINT8 auc_digest_digest[16] = {0};
    UINT8 auc_degist_tmp[16] = {0};

    const int i_str_len = 128;
    INT8 *pc_str_tmp = new INT8[i_str_len]();
    memset( pc_str_tmp, 0, i_str_len );
    snprintf( pc_str_tmp, i_str_len, "%s", str_digest.c_str() );
    get_md5( pc_str_tmp, strlen( pc_str_tmp), NULL, auc_digest_digest );

    INT8  ac_md5[33] = {0};
    INT32  i_rand_tmp=0;
    UINT32 ui_time_stamp_tmp=0;

    Md5Val_S st_md5;
    memcpy(&st_md5, auc_digest_digest, sizeof(st_md5));
    snprintf(ac_md5, sizeof(ac_md5), "%08x%08x%08x%08x", htonl(st_md5.n_a), htonl(st_md5.n_b), htonl(st_md5.n_c), htonl(st_md5.n_d));

    INT8  *pc_str = new INT8[i_str_len]();
    memset(pc_str, 0, i_str_len);
    snprintf(pc_str, i_str_len, "%s%s%urand=%d", str_user_name.c_str(), ac_md5, ui_time_stamp_tmp, i_rand_tmp);
    get_md5( pc_str, strlen(pc_str), NULL, auc_degist_tmp );

    p_auth_req->body.ui_client_type = CLIENT_MONITOR;
    p_auth_req->body.ui_time_stamp = ui_time_stamp_tmp;
    p_auth_req->body.ui_rand = i_rand_tmp;
    strcpy( p_auth_req->body.ac_login_id, str_user_name.c_str() );
    memcpy( p_auth_req->body.auc_digest, auc_degist_tmp, sizeof(auc_degist_tmp) );
    //send_packet( get_link(), p_auth_req, 1 );
    send_packet_to( mh_link, p_auth_req, p_ip, us_port, UDP_FLAG_NONE );

    delete p_auth_req;
    p_auth_req = NULL;
}

bool UdpClient::on_recv_link_auth_req( const CRefCountPtr<MSAuthReq_S> &p_pkt, char *p_ip, unsigned short us_port )
{
    MSAuthResp_S st_resp;
    st_resp.magic_type = p_pkt->magic_type;
    st_resp.msg_type = IR_MONITOR_LINK_AUTH_RESP;
    st_resp.seq_id = p_pkt->seq_id;
    //st_resp.total_len = sizeof( st_resp );

    st_resp.i_ret = 0;
    st_resp.body.ui_client_id = 1;
    st_resp.body.i_client_type = 1;
    st_resp.body.ul_out_dev_premission = 1;
    for( UINT32 ui_n = 0; ui_n < 2; ui_n++ ){
        st_resp.body.ui_client_level_map[ui_n] = (UINT64)ui_n;
    }
    st_resp.body.str_extend = "abcdefghijklmnopqrstuvwxyz.10086!";
    //send_packet( &st_resp );
    send_packet_to( mh_link, &st_resp, p_ip, us_port, UDP_FLAG_NONE );
}

bool UdpClient::on_recv_link_auth_resp( const CRefCountPtr<MSAuthResp_S> &p_pkt, char *p_ip, unsigned short us_port )
{
    unsigned int ui_client_id = p_pkt->body.ui_client_id;

    return true;
}
