#ifndef COMM_FUNC_H_
#define COMM_FUNC_H_

#include <sstream>
#include <iostream>
#include <string.h>

#include "clientlog-api.h"
#include "comm_typedef.h"
#include "net-packet.h"

template<typename T>
std::string T2string( T t )
{
    std::stringstream ss;
    ss << t;
    std::string str = ss.str();
    return str;
}

std::string ConvertIP( unsigned int ui_ip );

template<typename PACKET>
UINT32 parse_data_total_len( const char *p_data )
{
    UINT32 ui_total_len = ntohl( ((PACKET *)p_data)->total_len );
    return ui_total_len;
}

template<typename PACKET>
int parse_packet( const char *pc_buffer, int i_size, PACKET *&pv_req )
{
    UINT32 ui_msg_type = ntohl( ((PACKET *)pc_buffer)->msg_type );
    pv_req = NULL;
    switch( ui_msg_type ){
#define CASE_PACKET(n, T)\
    case n: {\
        T* packet = new T();\
        if(packet != NULL) {\
            pv_req = ( PACKET* )packet;\
        } else {\
            MS_LOGER_FATAL("malloc err");\
            return -1;\
        }\
        if ( read_packet( pc_buffer, i_size, *packet ) ) {\
            return 0;\
        } else {\
            MS_LOGER_FATAL("cmd not received" );\
            return -1;\
        }\
    }
    CASE_PACKET( IR_MONITOR_VER_NEGO_REQ, MSVerReq_S );
    CASE_PACKET( IR_MONITOR_VER_NEGO_RESP, MSCommResp_S );

    CASE_PACKET( IR_MONITOR_LINK_AUTH_REQ, MSAuthReq_S );
    CASE_PACKET( IR_MONITOR_LINK_AUTH_RESP, MSAuthResp_S );
#undef CASE_PACKET
    default:
        return CTRL_PRO_NOT_EXIST_ERROR;
    }
    return 0;
}

template<typename PACKET>
int parse_data( const char *p_data, unsigned int ui_len, PACKET *&pv_req )
{
    pv_req = NULL;
    INT32 ui_total_len = ntohl( ((PACKET *)p_data)->total_len );
    INT32 i_ret = -1;

    if ( p_data == NULL ) {
        MS_LOGER_FATAL( "data is null" );
        return -1;
    } else if( ui_len < (INT32)sizeof( PACKET ) ) {
        MS_LOGER_FATAL( "data is null" );
        return 0;
    }else if( ui_total_len > ui_len ) {
        MS_LOGER_FATAL( "data is null" );
        return 0;
    }
    i_ret = parse_packet( p_data, ui_total_len, pv_req );
    if ( !i_ret ) {         //????????????
        if( pv_req != NULL ) {
            return ui_total_len;
        } else {
            MS_LOGER_FATAL("pv_req is NULL");
            return -1;
        }
    } else if( i_ret != CTRL_PRO_NOT_EXIST_ERROR ) {
        return -1;
    } else {
        return CTRL_PRO_NOT_EXIST_ERROR;
    }
}


INT32 GetLocalTimeDay(UINT32 ui_sec, std::string &str_time);

#endif
