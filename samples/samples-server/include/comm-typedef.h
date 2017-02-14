#ifndef COMM_TYPEDEF_H_
#define COMM_TYPEDEF_H_

#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/if_ether.h>
#include <unistd.h>
#include <pthread.h>
#include <string>

#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <map>


#include "ir-protocol-def.h"

#define YF_START "YFSTART"
#define YF_START_LEN 7

#define YF_END   "YFEND"
#define YF_END_LEN 5


#define __32_BIT__  1
#if __32_BIT__

typedef char                INT8;
typedef short               INT16;
typedef int                 INT32;
typedef signed long long    INT64;
typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;

#endif


#define MSG_HEAD_MAGIC        0xA2A22A2A
#define REPLAY_HEAD_MAGIC     0xA1A11A1A
#define VIDEO_HEAD_MAGIC      0xA0A05A5A

typedef enum {
    VOD_PKT_TYPE_USUAL      = 0x00,
    VOD_PKT_TYPE_I_FRAME    = 0x01,
    VOD_PKT_TYPE_H264       = 0x02,
    VOD_PKT_TYPE_PS_HEAD    = 0x03,
    VOD_PKT_TYPE_JPG_FRAME  = 0x05,
    VOD_PKT_TYPE_JPG_PLAY   = 0x06,
    VOD_PKT_TYPE_TEMP       = 0x10, /* 简单温度包type */
    VOD_PKT_TYPE_MAX_MIN_TEMP = 0x11,/* 最高温最低温*/
    VOD_PKT_TYPE_TEMP_END   = 0x1f, /* 温度结束包type */
    VOD_PKT_TYPE_COLOR      = 0x20,
    VOD_PKT_TYPE_TIME       = 0x21,
    VOD_PKT_TYPE_COLOR_IDX  = 0x22,
    VOD_PKT_TYPE_ANA_NOTICE = 0x31,
    VOD_PKT_TYPE_FRAME_END  = 0xffff,

    VOD_PKT_TYPE_CMD        = 0x1001, /* 命令包type */

    VOD_PKT_TYPE_BUTT
}VOD_PKT_TYPE_E;

typedef struct tagMONITOR_FRAME_TIME_HEAD{
    unsigned int ui_Magic;  // 该值固定为0XA0A05A5A
    unsigned int ui_type;   // 0x21
    unsigned int ui_Length; // sizeof( unsigned int ) * 6
    unsigned int ui_seq;    // 0
    unsigned int ui_sec;    //sec
    unsigned int ui_usec;   //usec
}MONITOR_FRAME_TIME_HEAD;

#pragma pack(push)
#pragma pack(1)
typedef struct tagVOD_RESP_DATA_HEAD_S{
    unsigned int ui_Magic; /* 该值固定为0xA1A11A1A */
    unsigned int ui_type;
    unsigned int ui_total_len;
    unsigned int ui_seq;
    unsigned char auc_data[0];
}VOD_RESP_DATA_HEAD_S;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct tagMONITOR_LIVE_HEAD{
    unsigned int ui_Magic; /* 该值固定为0XA0A05A5A */
    unsigned int ui_type;
    unsigned int ui_Length;
    unsigned int ui_seq;
    unsigned int auc_data[0];
}MONITOR_LIVE_HEAD_S;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct tagMONITOR_ADJUST_DATA_S{
    int    i_type;  //色标类型。
    float  f_max;  //最高温度
    float  f_min;  //最低温度
    float  f_range_max; //范围中的最高温度
    float  f_range_min; //范围中的最低温度
}MONITOR_ADJUST_DATA_S;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct tagIR_ANA_DATA_S {
    unsigned char *puc_data;
    int i_len;
    unsigned int ui_frame_no;
}IR_ANA_DATA_S;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct tagMONITOR_TEMPERATURE_PKT_S{
    unsigned int ul_tmp;
    u_int8_t t2;    //温度的小数位  （0~255)
}MONITOR_TEMPERATURE_PKT_S;
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)
typedef struct tagMsgHeader_S{
    std::string   str_yf_start;
    unsigned int  ui_total_len;  //ui_total_len = sizeof(MsgHeader_S) - 11 + ui_auc_data_len;
    unsigned char uc_package_type;
    unsigned char uc_main_cmd;			//< NetMainCmd
    unsigned char uc_vice_cmd;			//< NetViceCmd
    unsigned char uc_response_code;
    unsigned int  ui_seq;
    unsigned int  ui_auc_data_len;
    unsigned char auc_data[0];
    tagMsgHeader_S()
    {
        str_yf_start = YF_START;
        ui_total_len = 0;
        ui_auc_data_len = 0;
    }
}MsgHeader_S;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    unsigned char uch_len;
    unsigned char uch_meas_grade;
    unsigned char uch_emiss;
    unsigned char uch_rel_hum;
    unsigned short f_dist;
    unsigned short f_env_temp;
    unsigned short f_mdf;
}STRU_TEMP_PARA_S;
#pragma pack(pop)


#endif
