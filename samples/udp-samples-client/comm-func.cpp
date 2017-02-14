#include "comm-func.h"
#include <arpa/inet.h>
#include <time.h>

std::string ConvertIP( unsigned int ui_ip )
{
    std::string str_ip;
    unsigned int au_item[4] = { 0 };
    for( int i = 0; i < 4; ++i ){
        au_item[i] = ((ui_ip >> (i * 8)) & 0xff);
    }

    str_ip = T2string( au_item[3] ) + "." + T2string( au_item[2] ) + "." + T2string( au_item[1] ) + "." + T2string( au_item[0] );
    return str_ip;
}

//int parse_packet( char *p_data, unsigned int ui_len, MsgHeader_S *&pv_req )
//{
//    return 0;
//}

//int parse_data( char *p_data, unsigned int ui_len, MsgHeader_S *&pv_req )
//{
//    pv_req = NULL;
//    INT32 ui_total_len = ntohl( ((MsgHeader_S *)p_data)->total_len );
//    UINT32 msg_type = ntohl( ((MsgHeader_S *)p_data)->msg_type );
//    INT32 i_ret = -1;

//    if ( p_data == NULL ) {
//        MS_LOGER_FATAL( "data is null" );
//        return -1;
//    } else if( ui_len < (INT32)sizeof( MsgHeader_S ) ) {
//        MS_LOGER_FATAL( "data is null" );
//        return 0;
//    }else if( ui_total_len > ui_len ) {
//        MS_LOGER_FATAL( "data is null" );
//        return 0;
//    }
//    i_ret = parse_packet( p_data, ui_total_len, pv_req );
//    if ( !i_ret ) {         //属于实时协议，并解析成功
//        if( pv_req != NULL ) {
//            return ui_total_len;
//        } else {
//            MS_LOGER_FATAL("pv_req is NULL");
//            return -1;
//        }
//    } else if( i_ret != -2 ) {
//        return -1;
//    }

//    return 0;
//}


INT32 GetLocalTimeDay(UINT32 ui_sec, std::string &str_time)
{
    time_t tm = ui_sec;
    struct tm* tm_tm = localtime(&tm);
    std::string str_year = T2string(1900+tm_tm->tm_year);
    std::string str_mon;
    INT32 i_mon = tm_tm->tm_mon + 1;
    if(i_mon < 10){
        str_mon = "0" + T2string(i_mon);
    }else{
        str_mon = T2string(i_mon);
    }
    std::string str_day;
    INT32 i_day = tm_tm->tm_mday;
    if(i_day < 10){
        str_day = "0" + T2string(i_day);
    }else{
        str_day = T2string(i_day);
    }

    str_time = str_year + str_mon + str_day;

    return 0;
}
