#ifndef NETPACKET_H
#define NETPACKET_H

#include <map>
#include <vector>
#include <memory>
#include "database-typedef.h"
#include "net-memory-io-base.h"

#define LOGIN_ID_LEN                   32
#define DIGEST_ID_LEN                  16
#define PIC_NAME_LEN                   128
#define DEVICE_NAME_LEN                PIC_NAME_LEN
#define URL_LEN                        1024
#define DEVICE_COUNT                   2                //设备个数：红外、可见光
#define  DEV_SN_LEN 128
#define  PTZ_SN_LEN 64
#define  CRUISE_LEN 256
#define  CRUISE_PRESET_LEN 64
#define  ALIAS_LEN  256
#define  PICPRESETS_LEN 256
#define  PICPRESET_LEN  64
#define  PSW_LEN 16
#define  SLIDE_IP_LEN 16
#define  SLIDE_PORT_COUNT 2
#define  FRAME_COUNT  2
#define control_protocol_modify

inline bool write_packet_size( std::vector<char> &vc_buffer )
{
    //INT32 i_offset = sizeof( ((CommResp_S*)0)->header.magic_type ) + sizeof( ((CommResp_S*)0)->header.msg_type );
    INT32 i_offset = sizeof( ((CommResp_S*)0)->magic_type ) + sizeof( ((CommResp_S*)0)->msg_type );
    uint32_t ui_packet_size = vc_buffer.size();
    return pack_resp( vc_buffer, i_offset, ms_endian(ui_packet_size) );
}

template<typename Packet>
bool write_packet( std::vector<char> &vc_buffer, const Packet &st_packet )
{
    bool b_ret = false;
    //if( (  (const CommResp_S& )st_packet ).header.msg_type == IR_MONITOR_LINK_HEART_RESP ) {
    if( (  (const CommResp_S& )st_packet ).msg_type == IR_MONITOR_LINK_HEART_RESP ) {
        b_ret = pack_resp( vc_buffer, (const MsgHeader_S&)st_packet );   //心跳只需要消息头
    } else {
        b_ret = pack_resp( vc_buffer, (const CommResp_S&)st_packet );
    }
    return (
        b_ret &&
        pack_resp( vc_buffer, st_packet.body ) &&
        write_packet_size( vc_buffer )
        );
}

template<typename Packet>
bool read_packet( const char *pc_buffer, INT32 size, Packet &st_packet )
{
    INT32 offset = 0;
    return (
        parse_req( pc_buffer, size, offset, (MsgHeader_S&)st_packet ) &&
        parse_req( pc_buffer, size, offset, st_packet.body )
        );
}

struct MSNULL_S       //空结构体
{

};

template <typename T>
struct MSPacketReq_S : public MsgHeader_S
{
    T body;
};

template <typename T>
struct MSPacketResp_S : public CommResp_S
{
    T body;
};

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSNULL_S &st_packet )
{
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSNULL_S &st_packet )
{
    return true;
}

typedef MSPacketReq_S<MSNULL_S> MSCommReq_S;
typedef MSPacketResp_S<MSNULL_S> MSCommResp_S;

template<>
inline bool parse_req( const char *pc_buffer, INT32 i_size, INT32 &i_offset, MsgHeader_S& s_head )
{
//    return (
//        parse_req( pc_buffer, i_size, i_offset, s_head.magic_type ) &&
//        parse_req( pc_buffer, i_size, i_offset, s_head.msg_type ) &&
//        parse_req( pc_buffer, i_size, i_offset, s_head.total_len ) &&
//        parse_req( pc_buffer, i_size, i_offset, s_head.seq_id )
//        );

    return (
        parse_req( pc_buffer, i_size, i_offset, s_head.magic_type ) &&
        parse_req( pc_buffer, i_size, i_offset, s_head.msg_type ) &&
        parse_req( pc_buffer, i_size, i_offset, s_head.total_len ) &&
        parse_req( pc_buffer, i_size, i_offset, s_head.seq_id ) &&
        parse_req( pc_buffer, i_size, i_offset, s_head.i_ret )
        );
}

//template<>
//inline bool pack_resp( std::vector<char> &vc_buffer, const CommResp_S &st_packet )
//{
////    return pack_resp( vc_buffer, st_packet.header.magic_type ) &&
////           pack_resp( vc_buffer, st_packet.header.msg_type ) &&
////           pack_resp( vc_buffer, st_packet.header.total_len ) &&
////           pack_resp( vc_buffer, st_packet.header.seq_id ) &&
////           pack_resp( vc_buffer, st_packet.i_ret );
//    return pack_resp( vc_buffer, st_packet.magic_type ) &&
//           pack_resp( vc_buffer, st_packet.msg_type ) &&
//           pack_resp( vc_buffer, st_packet.total_len ) &&
//           pack_resp( vc_buffer, st_packet.seq_id ) &&
//           pack_resp( vc_buffer, st_packet.i_ret );
//}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MsgHeader_S &st_packet )
{
//    return pack_resp( vc_buffer, st_packet.magic_type ) &&
//           pack_resp( vc_buffer, st_packet.msg_type ) &&
//           pack_resp( vc_buffer, st_packet.total_len ) &&
//           pack_resp( vc_buffer, st_packet.seq_id );
    return pack_resp( vc_buffer, st_packet.magic_type ) &&
           pack_resp( vc_buffer, st_packet.msg_type ) &&
           pack_resp( vc_buffer, st_packet.total_len ) &&
           pack_resp( vc_buffer, st_packet.seq_id ) &&
           pack_resp( vc_buffer, st_packet.i_ret );
}

//版本协商请求--MsgType = IR_MONITOR_VER_NEGO_REQ
typedef struct tagMSVerReqBody_S
{
    UINT32    ui_major_ver;
    UINT32    ui_sub_ver;
}MSVerReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSVerReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_major_ver ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_sub_ver );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSVerReqBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_major_ver ) &&
           pack_resp( vc_buffer, st_packet.ui_sub_ver ) ;
}


typedef MSPacketReq_S<MSVerReqBody_S> MSVerReq_S;

//鉴权请求--MsgType = IR_MONITOR_LINK_AUTH_REQ
typedef struct tagMSAuthReqBody_S
{
    INT8 ac_login_id[LOGIN_ID_LEN];
    UINT8 auc_digest[DIGEST_ID_LEN];
    UINT32 ui_time_stamp;
    UINT32 ui_rand;
    UINT32 ui_client_type;
}MSAuthReqBody_S;

typedef struct tagMSAuthRespBody_S
{
    UINT32 ui_client_id;
    INT32  i_client_type;     //用户权限类型
    UINT64 ul_out_dev_premission;
    std::map<UINT32, UINT64> ui_client_level_map;
    std::string str_extend;
}MSAuthRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSAuthReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ac_login_id,  sizeof(st_packet.ac_login_id) ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.auc_digest, sizeof(st_packet.auc_digest) ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_time_stamp ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_rand ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_type );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSAuthReqBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ac_login_id, sizeof( st_packet.ac_login_id ) ) &&
           pack_resp( vc_buffer, st_packet.auc_digest, sizeof( st_packet.auc_digest ) ) &&
           pack_resp( vc_buffer, st_packet.ui_time_stamp ) &&
           pack_resp( vc_buffer, st_packet.ui_rand ) &&
           pack_resp( vc_buffer, st_packet.ui_client_type ) ;
}

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSAuthRespBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_id ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.i_client_type ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ul_out_dev_premission ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_level_map ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.str_extend );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSAuthRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_client_id ) &&
           pack_resp( vc_buffer, st_packet.i_client_type ) &&
           pack_resp( vc_buffer, st_packet.ul_out_dev_premission ) &&
           pack_resp( vc_buffer, st_packet.ui_client_level_map ) &&
           pack_resp( vc_buffer, st_packet.str_extend ) ;
}

typedef MSPacketReq_S<MSAuthReqBody_S> MSAuthReq_S;
typedef MSPacketResp_S<MSAuthRespBody_S> MSAuthResp_S;

//心跳检测请求--MsgType = IR_MONITOR_LINK_HEART_REQ
typedef struct tagMSHeartReqBody_S
{
    UINT32 ui_client_id;
}MSHeartReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSHeartReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_id );
}

typedef MSPacketReq_S<MSHeartReqBody_S> MSHeartReq_S;

//XML文件请求--MsgType = IR_MONITOR_XML_DATA_REQ
typedef struct tagMSXMLReqBody_S
{
    UINT32 ui_client_id;
}MSXMLReqBody_S;

typedef struct tagMSXMLRespBody_S
{
   std::string s_xml;
}MSXMLRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSXMLReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_id );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSXMLRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.s_xml, st_packet.s_xml.size() );
}

typedef MSPacketReq_S<MSXMLReqBody_S> MSXMLReq_S;
typedef MSPacketResp_S<MSXMLRespBody_S> MSXMLResp_S;

//主码流播放请求--MsgType = IR_MONITOR_VID_DATA_PLAY_REQ
//关键视频帧数据播放请求--MsgType = IR_MONITOR_VID_KEY_DATA_PLAY_REQ
//子码流播放请求--MsgType = IR_MONITOR_VID_SUB_STREAM_PLAY_REQ
typedef struct tagMSPlayReqBody_S
{
    INT8 ac_device_sn[DEVICE_NAME_LEN];
}MSPlayReqBody_S;

typedef struct tagMSPlayRespBody_S
{
   INT8 ac_url_buf[URL_LEN];
}MSPlayRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSPlayReqBody_S &st_packet )
{
    UINT32 ui_len = strlen( pc_buffer + i_offset );
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ac_device_sn, ui_len );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSPlayRespBody_S &st_packet )
{
    UINT32 ui_len = strlen( st_packet.ac_url_buf );
    return pack_resp( vc_buffer, st_packet.ac_url_buf, ui_len );
}

typedef MSPacketReq_S<MSPlayReqBody_S> MSPlayReq_S;
typedef MSPacketResp_S<MSPlayRespBody_S> MSPlayResp_S;

//视频数据停止请求--MsgType = IR_MONITOR_VID_DATA_STOP_REQ,
//关键视频帧数据停止请求--MsgType = IR_MONITOR_VID_KEY_DATA_STOP_REQ
//子码流视频数据停止请求--MsgType = IR_MONITOR_VID_SUB_STREAM_STOP_REQ
typedef struct tagMSVIDStopReqBody_S
{
    INT8 ac_device_sn[DEVICE_NAME_LEN];
}MSVIDStopReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSVIDStopReqBody_S &st_packet )
{
    UINT32 ui_len = strlen( pc_buffer + i_offset );
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ac_device_sn, ui_len );
}

typedef MSPacketReq_S<MSVIDStopReqBody_S> MSVIDStopReq_S;

//回放个数设置请求--MsgType = IR_MONITOR_SET_REPLAY_NUM_REQ
typedef struct tagMSReplaySetNumReqBody_S
{
    UINT32 ui_num;
}MSReplaySetNumReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSReplaySetNumReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_num );
}

typedef MSPacketReq_S<MSReplaySetNumReqBody_S> MSReplaySetNumReq_S;

//子码流容量请求--MsgType = IR_MONITOR_SET_RESERVE_HARD_CAPACITY_REQ
typedef struct tagMSReserveHardCapacityReqBody_S
{
    UINT32 ui_capacity;
}MSReserveHardCapacityReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSReserveHardCapacityReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_capacity );
}

typedef MSPacketReq_S<MSReserveHardCapacityReqBody_S> MSReserveHardCapacityReq_S;

//配置信息新增请求--MsgType = IR_MONITOR_GROUP_CFG_ADD_REQ
typedef struct tagMSGroupCfgAddReqBody_S
{
    UINT32 ui_num;
    std::string s_members;
    std::string s_alias;
}MSGroupCfgAddReqBody_S;

typedef struct tagMSGroupCfgAddRespBody_S
{
   UINT32 ui_id;
}MSGroupCfgAddRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSGroupCfgAddReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.ui_num ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_members ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_alias );
    if( b_ret ){
        st_packet.s_members = st_packet.s_members.c_str();
        st_packet.s_alias = st_packet.s_alias.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGroupCfgAddRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_id );
}

typedef MSPacketReq_S<MSGroupCfgAddReqBody_S> MSGroupCfgAddReq_S;
typedef MSPacketResp_S<MSGroupCfgAddRespBody_S> MSGroupCfgAddResp_S;

//配置信息删除请求--MsgType = IR_MONITOR_GROUP_CFG_DEL_REQ
typedef struct tagMSGroupCfgDelReqBody_S
{
    UINT32 ui_id;
}MSGroupCfgDelReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSGroupCfgDelReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_id );
}

typedef MSPacketReq_S<MSGroupCfgDelReqBody_S> MSGroupCfgDelReq_S;

//配置信息变更请求--MsgType = IR_MONITOR_GROUP_CFG_CHG_REQ
typedef struct tagMSGroupCfgChgReqBody_S
{
    UINT32 ui_id;
    UINT32 ui_num;
    std::string s_members;
    std::string s_alias;
}MSGroupCfgChgReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSGroupCfgChgReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.ui_id ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_num ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_members ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_alias );

    if( b_ret ){
        st_packet.s_members = st_packet.s_members.c_str();
        st_packet.s_alias = st_packet.s_alias.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSGroupCfgChgReqBody_S> MSGroupCfgChgReq_S;

//巡航重启标志设置请求--MSGTYPE = IR_MONITOR_SET_CRUISE_RESTART_FLAG_REQ
typedef struct tagMSSetCruiseRestartFlagReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_restart_flag;
    UINT32 ui_time;
}MSSetCruiseRestartFlagReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSSetCruiseRestartFlagReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_restart_flag ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_time );
}

typedef MSPacketReq_S<MSSetCruiseRestartFlagReqBody_S> MSSetCruiseRestartFlagReq_S;


//巡航重启标志获取请求--MSGTYPE = IR_MONITOR_GET_CRUISE_RESTART_FLAG_REQ
typedef struct tagMSGetCruiseRestartFlagReqBody_S
{
    UINT32 ui_group;
}MSGetCruiseRestartFlagReqBody_S;

typedef struct tagMSGetCruiseRestartFlagRespBody_S
{
    UINT32 ui_restart_flag;
    UINT32 ui_time;
}MSGetCruiseRestartFlagRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSGetCruiseRestartFlagReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetCruiseRestartFlagRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_restart_flag ) &&
           pack_resp( vc_buffer, st_packet.ui_time );
}

typedef MSPacketReq_S<MSGetCruiseRestartFlagReqBody_S> MSGetCruiseRestartFlagReq_S;
typedef MSPacketResp_S<MSGetCruiseRestartFlagRespBody_S> MSGetCruiseRestartFlagResp_S;

//红外数据分析新增请求--MsgType = IR_MONITOR_IR_ANALYSE_ADD_REQ
//红外数据分析重载请求--MsgType = IR_MONITOR_IR_ANALYSE_LOAD_REQ
typedef struct tagMSAddPrivateAnalyseReqBody_S
{
    std::string s_sn;
    std::map<UINT32, std::string> str_ana_map;
}MSAddPrivateAnalyseReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSAddPrivateAnalyseReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_sn ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.str_ana_map );

    if( b_ret ){
        st_packet.s_sn = st_packet.s_sn.c_str();
        std::map<UINT32, std::string>::iterator it = st_packet.str_ana_map.begin();
        for( ; it != st_packet.str_ana_map.end(); ++it ){
            std::string str_ana = it->second;
            str_ana = str_ana.c_str();
            st_packet.str_ana_map[it->first] = str_ana;
        }
    }

    return b_ret;
}

typedef MSPacketReq_S<MSAddPrivateAnalyseReqBody_S> MSAddPrivateAnalyseReq_S;

//红外数据分析删除请求--MsgType = IR_MONITOR_IR_ANALYSE_DEL_REQ
typedef struct tagMSDelPrivateAnalyseReqBody_S
{
    std::string s_sn;
    std::set<UINT32> ana_no_set;
}MSDelPrivateAnalyseReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSDelPrivateAnalyseReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.s_sn ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ana_no_set );
}

typedef MSPacketReq_S<MSDelPrivateAnalyseReqBody_S> MSDelPrivateAnalyseReq_S;

//红外焦距调远请求--MsgType = IR_MONITOR_IR_FOCUS_OUT_REQ
//红外焦距调近请求--MsgType = IR_MONITOR_IR_FOCUS_IN_REQ
//红外温度校准请求--MsgType = IR_MONITOR_IR_TEMP_CHK_REQ
//可见光正向调焦请求--MsgType = IR_MONITOR_VISI_FOCUS_INC_REQ
//可见光反向调焦请求--MsgType = IR_MONITOR_VISI_FOCUS_DEC_REQ
//可见光正向调焦停止请求--MsgType=IR_MONITOR_VISI_FOCUS_INC_STOP_REQ
//可见光反向调焦停止请求--MsgType = IR_MONITOR_VISI_FOCUS_DEC_STOP_REQ
typedef struct tagMSFocusChkReqBody_S
{
    UINT32 ui_client_id;
    std:: string s_sn;
}MSFocusChkReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSFocusChkReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.ui_client_id ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_sn );

    if( b_ret ){
        st_packet.s_sn = st_packet.s_sn.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSFocusChkReqBody_S> MSFocusChkReq_S;


//私有分析报警条件设置--MsgType = IR_MONITOR_PRIVATE_ANALYSE_ALARM_CONDITION_SET_REQ
//私有分析报警条件重载--MsgType = IR_MONITOR_PRIVATE_ANALYSE_ALARM_CONDITION_RELOAD_REQ
typedef struct tagMSAddPrivateAlarmConditionReqBody_S
{
    std::string s_sn;
    std::map<UINT32, std::string> str_condition_map;
}MSAddPrivateAlarmConditionReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSAddPrivateAlarmConditionReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_sn ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.str_condition_map );

    if( b_ret ){
        st_packet.s_sn = st_packet.s_sn.c_str();
        std::map<UINT32, std::string>::iterator it = st_packet.str_condition_map.begin();
        for( ; it != st_packet.str_condition_map.end(); ++it ){
            std::string str_con = it->second;
            str_con = str_con.c_str();
            st_packet.str_condition_map[it->first] = str_con;
        }
    }

    return b_ret;
}

typedef MSPacketReq_S<MSAddPrivateAlarmConditionReqBody_S> MSAddPrivateAlarmConditionReq_S;

//私有分析报警条件删除--MsgType = IR_MONITOR_PRIVATE_ANALYSE_ALARM_CONDITION_DEL_REQ
typedef struct tagMSDelPrivateAlarmConditionReqBody_S
{
    std::string s_sn;
    std::set<UINT32> ui_condition_set;
}MSDelPrivateAlarmConditionReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSDelPrivateAlarmConditionReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.s_sn ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_condition_set );
}

typedef MSPacketReq_S<MSDelPrivateAlarmConditionReqBody_S> MSDelPrivateAlarmConditionReq_S;

//导轨单步向左请求--MsgType = IR_MONITOR_RAIL_CTRL_STEP_LEFT_REQ
//导轨单步向右请求--MsgType = IR_MONITOR_RAIL_CTRL_STEP_RIGHT_REQ
//导轨持续向左请求--MsgType = IR_MONITOR_RAIL_CTRL_LEFT_REQ
//导轨持续向右请求--MsgType = IR_MONITOR_RAIL_CTRL_RIGHT_REQ
//导轨停止请求--MsgType = IR_MONITOR_RAIL_CTRL_STOP_REQ
typedef struct tagMSRailDirectReqBody_S
{
    std::string s_rail_sn;
}MSRailDirectReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSRailDirectReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_rail_sn );
    if( b_ret ){
        st_packet.s_rail_sn = st_packet.s_rail_sn.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSRailDirectReqBody_S> MSRailDirectReq_S;

//设置导轨速度请求--MsgType=IR_MONITOR_RAIL_CTRL_SET_SPEED_REQ
typedef struct tagMSSetRailSpeedReqBody_S
{
    std::string s_rail_sn;
    UINT32 ui_speed;
}MSSetRailSpeedReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSSetRailSpeedReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_rail_sn ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_speed );

    if( b_ret ){
        st_packet.s_rail_sn = st_packet.s_rail_sn.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSSetRailSpeedReqBody_S> MSSetRailSpeedReq_S;

//导轨校准请求--MsgType=IR_MONITOR_RAIL_CTRL_CHK_INFO_REQ
typedef struct tagMSRailChkReqBody_S
{
    std::string s_rail_sn;
}MSRailChkReqBody_S;

typedef struct tagMSRailChkRespBody_S
{
    UINT32 ui_len;
}MSRailChkRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSRailChkReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_rail_sn );
    if( b_ret ){
        st_packet.s_rail_sn = st_packet.s_rail_sn.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSRailChkRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_len );
}

typedef MSPacketReq_S<MSRailChkReqBody_S> MSRailChkReq_S;
typedef MSPacketResp_S<MSRailChkRespBody_S> MSRailChkResp_S;

//获取导轨当前位置及速度请求--MsgType=IR_MONITOR_RAIL_CTRL_CUR_POS_REQ
typedef struct tagMSRailPosReqBody_S
{
    std::string s_rail_sn;
}MSRailPosReqBody_S;

typedef struct tagMSRailPosRespBody_S
{
    UINT32 ui_cur_pos;
    UINT32 ui_cur_speed;
    UINT32 ui_total_len;
}MSRailPosRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSRailPosReqBody_S &st_packet )
{
    bool b_ret = parse_req( pc_buffer, i_size, i_offset, st_packet.s_rail_sn );
    if( b_ret ){
        st_packet.s_rail_sn = st_packet.s_rail_sn.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSRailPosRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_cur_pos ) &&
           pack_resp( vc_buffer, st_packet.ui_cur_speed ) &&
           pack_resp( vc_buffer, st_packet.ui_total_len );
}

typedef MSPacketReq_S<MSRailPosReqBody_S> MSRailPosReq_S;
typedef MSPacketResp_S<MSRailPosRespBody_S> MSRailPosResp_S;

//报警条件设置请求--MsgType = IR_MONITOR_SET_ALARM_CONDITION_REQ
typedef struct tagMSSetAlarmConditionReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_alarm_no;
    string str_cname;
    string str_alarm_condition;
}MSSetAlarmConditionReqBody_S;

typedef struct tagMSSetAlarmConditionRespBody_S
{
    UINT32 ui_alarm_no;
}MSSetAlarmConditionRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSSetAlarmConditionReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_alarm_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_cname) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_alarm_condition);

    if( b_ret ){
        st_packet.str_cname = st_packet.str_cname.c_str();
        st_packet.str_alarm_condition = st_packet.str_alarm_condition.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSSetAlarmConditionRespBody_S &st_packet )
{
   return pack_resp( vc_buffer, st_packet.ui_alarm_no );
}

typedef MSPacketReq_S<MSSetAlarmConditionReqBody_S> MSSetAlarmConditionReq_S;
typedef MSPacketResp_S<MSSetAlarmConditionRespBody_S> MSSetAlarmConditionResp_S;

//报警条件获取请求MSG-TYPE = IR_MONITOR_GET_ALARM_CONDITION_REQ
typedef struct tagMSGetAlarmConditionReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_alarm_no;// 0: all, other: one
}MSGetAlarmConditionReqBody_S;

typedef struct tagMSGetAlarmConditionRespBody_S
{
    std::map<UINT32, AlarmConditionInfo_S> alarm_condition_map;
}MSGetAlarmConditionRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, tagMSGetAlarmConditionReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_alarm_no);
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const AlarmConditionInfo_S &st_packet )
{
    std::string str_alias = st_packet.str_alias;
    std::string str_alarm_condition = st_packet.str_alarm_condition;
    str_alias.push_back( '\0' );
    str_alarm_condition.push_back( '\0' );
    return pack_resp( vc_buffer, st_packet.ui_alarm_no ) &&
          pack_resp( vc_buffer, str_alias ) &&
          pack_resp( vc_buffer, str_alarm_condition );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const std::map<UINT32, AlarmConditionInfo_S> &map_packet )
{
    bool b_ret = false;
    UINT32 ui_count = map_packet.size();   //个数，必须先赋值
    b_ret = pack_resp( vc_buffer, ui_count );
    std::map<UINT32, AlarmConditionInfo_S>::const_iterator it_premission = map_packet.begin();
    for( ; it_premission != map_packet.end(); ++it_premission ) {
        b_ret &= pack_resp( vc_buffer, it_premission->second );
    }
    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetAlarmConditionRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.alarm_condition_map );
}

typedef MSPacketReq_S<MSGetAlarmConditionReqBody_S> MSGetAlarmConditionReq_S;
typedef MSPacketResp_S<MSGetAlarmConditionRespBody_S> MSGetAlarmConditionResp_S;

//报警图片条件获取请求MSG-TYPE = IR_MONITOR_ALARM_PIC_LIST_REQ
typedef struct tagMSAlarmPicListReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    INT8   ac_begin[TIME_LEN];
    INT8   ac_end[TIME_LEN];
    std::set<UINT32> ui_alarm_no_list;
}MSAlarmPicListReqBody_S;

typedef struct tagMSAlarmPicListRespBody_S
{
    std::map<AlarmReasonKey_S, std::set<UINT32>, AlarmReasonCmp_S> alarm_reason;
}MSAlarmPicListRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSAlarmPicListReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ac_begin, sizeof( st_packet.ac_begin ) ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ac_end, sizeof( st_packet.ac_end ) ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_alarm_no_list);
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const AlarmReasonKey_S &st_packet )
{
   return pack_resp( vc_buffer, st_packet.ui_sec ) &&
          pack_resp( vc_buffer, st_packet.ui_msec );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const std::map<AlarmReasonKey_S, std::set<UINT32>, AlarmReasonCmp_S> &map_packet )
{
    bool b_ret = false;
    UINT32 ui_count = map_packet.size();   //个数，必须先赋值
    b_ret = pack_resp( vc_buffer, ui_count );
    std::map<AlarmReasonKey_S, std::set<UINT32>, AlarmReasonCmp_S>::const_iterator it_premission = map_packet.begin();
    for( ; it_premission != map_packet.end(); ++it_premission ) {
        b_ret &= pack_resp( vc_buffer, it_premission->first );
        b_ret &= pack_resp( vc_buffer, it_premission->second );
    }
    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSAlarmPicListRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.alarm_reason );
}

typedef MSPacketReq_S<MSAlarmPicListReqBody_S> MSAlarmPicListReq_S;
typedef MSPacketResp_S<MSAlarmPicListRespBody_S> MSAlarmPicListResp_S;

//报警图片获取请求MSG-TYPE = IR_MONITOR_ALARM_PIC_GET_REQ
typedef struct tagMSAlarmPicGetReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_alarm_no;
    UINT32 ui_utc_sec;
    UINT32 ui_utc_millsec;
}MSAlarmPicGetReqBody_S;

typedef struct tagMSAlarmPicGetRespBody_S
{
    UINT32 ui_num;
    MSAlarmPic_S ast_alarm_pic[DEVICE_COUNT];
    tagMSAlarmPicGetRespBody_S()
    {
        ui_num = 0;
    }
}MSAlarmPicGetRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSAlarmPicGetReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_alarm_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_utc_sec) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_utc_millsec);
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSAlarmPic_S &st_packet )
{
   return pack_resp( vc_buffer, st_packet.ui_pic_class ) &&
          pack_resp( vc_buffer, st_packet.pic_data );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSAlarmPicGetRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_num ) &&
           st_packet.ui_num <= ( sizeof(st_packet.ast_alarm_pic)/sizeof(MSAlarmPic_S) ) &&
           pack_resp( vc_buffer, st_packet.ast_alarm_pic, st_packet.ui_num );
}

typedef MSPacketReq_S<MSAlarmPicGetReqBody_S> MSAlarmPicGetReq_S;
typedef MSPacketResp_S<MSAlarmPicGetRespBody_S> MSAlarmPicGetResp_S;

//红外录制条件新增请求MSG-TYPE = IR_MONITOR_RECORD_CONDITION_REQ
typedef struct tagMSSetRecordConditionReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_record_time_interval;
    float  ft_max_temp_interval;//-9999 means invalid
    float  ft_max_temp;//-9999 means invalid
}MSSetRecordConditionReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSSetRecordConditionReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_record_time_interval) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ft_max_temp_interval) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ft_max_temp);
}

typedef MSPacketReq_S<MSSetRecordConditionReqBody_S> MSSetRecordConditionReq_S;

//红外录制条件获取请求MSG-TYPE = IR_MONITOR_RECORD_CONDITION_GET_REQ
typedef struct tagMSGetRecordConditionReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
}MSGetRecordConditionReqBody_S;

typedef struct tagMSGetRecordConditionRespBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_record_time_interval;
    float  ft_max_temp_interval;//-9999 means invalid
    float  ft_max_temp;//-9999 means invalid
}MSGetRecordConditionRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetRecordConditionReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetRecordConditionRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_group ) &&
           pack_resp( vc_buffer, st_packet.us_slide ) &&
           pack_resp( vc_buffer, st_packet.us_ptz ) &&
           pack_resp( vc_buffer, st_packet.ui_record_time_interval ) &&
           pack_resp( vc_buffer, st_packet.ft_max_temp_interval ) &&
           pack_resp( vc_buffer, st_packet.ft_max_temp );
}

typedef MSPacketReq_S<MSGetRecordConditionReqBody_S> MSGetRecordConditionReq_S;
typedef MSPacketResp_S<MSGetRecordConditionRespBody_S> MSGetRecordConditionResp_S;

//报警条件删除请求MSG-TYPE = IR_MONITOR_DEL_ALARM_CONDITION_REQ
typedef struct tagMSDelAlarmConditionReqBody_S
{
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_alarm_no;
}MSDelAlarmConditionReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSDelAlarmConditionReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_alarm_no);
}

typedef MSPacketReq_S<MSDelAlarmConditionReqBody_S> MSDelAlarmConditionReq_S;

//get pic by time MSG-TYPE = IR_MONITOR_TIMER_PIC_GET_REQ
typedef struct tagMSTimerPicGetReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_utc_sec;
    UINT32 ui_utc_millsec;
}MSTimerPicGetReqBody_S;

typedef struct tagMSTimerPicGetRespBody_S
{
    UINT32 ui_num;
    MSAlarmPic_S ast_alarm_pic[DEVICE_COUNT];
    tagMSTimerPicGetRespBody_S()
    {
        ui_num = 0;
    }
}MSTimerPicGetRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSTimerPicGetReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_utc_sec ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_utc_millsec );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSTimerPicGetRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_num ) &&
           st_packet.ui_num <= ( sizeof(st_packet.ast_alarm_pic)/sizeof(MSAlarmPic_S) ) &&
           pack_resp( vc_buffer, st_packet.ast_alarm_pic, st_packet.ui_num );
}

typedef MSPacketReq_S<MSTimerPicGetReqBody_S> MSTimerPicGetReq_S;
typedef MSPacketResp_S<MSTimerPicGetRespBody_S> MSTimerPicGetResp_S;

//获取当前巡航信息请求MSG-TYPE = IR_MONITOR_GET_CUR_CRUISE_INFO_REQ
typedef struct tagMSGetCurCruiseInfoReqBody_S
{
    UINT32 ui_group;
}MSGetCurCruiseInfoReqBody_S;

typedef struct tagMSGetCurCruiseInfoRespBody_S
{
    UINT32 ui_cruise_no;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_time;
}MSGetCurCruiseInfoRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetCurCruiseInfoReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group);
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetCurCruiseInfoRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_cruise_no ) &&
           pack_resp( vc_buffer, st_packet.us_slide ) &&
           pack_resp( vc_buffer, st_packet.us_ptz ) &&
           pack_resp( vc_buffer, st_packet.ui_time );
}

typedef MSPacketReq_S<MSGetCurCruiseInfoReqBody_S> MSGetCurCruiseInfoReq_S;
typedef MSPacketResp_S<MSGetCurCruiseInfoRespBody_S> MSGetCurCruiseInfoResp_S;

//设置图片请求MSG_TYPE= IR_MONITOR_SET_JPG_REQ
typedef struct tagMSSetPicInfoReqBody_S
{
    UINT32 ui_group;
    std::string s_pic_info;
}MSSetPicInfoReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSSetPicInfoReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_pic_info );
}

typedef MSPacketReq_S<MSSetPicInfoReqBody_S> MSSetPicInfoReq_S;

//获取电子地图图片列表请求--MsgType = IR_MONITOR_GET_E_PIC_LIST_REQ
typedef struct tagMSGetEPicListRespBody_S
{
    list<EPicInfo_S> e_pic_info_list;
}MSGetEPicListRespBody_S;

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const EPicInfo_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_pic_id) &&
           pack_resp( vc_buffer, st_packet.s_pic_cname);
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const list<EPicInfo_S> &lis_packet )
{
    list<EPicInfo_S>::const_iterator it_pic_list = lis_packet.begin();
    EPicInfo_S st_pic_info;
    if(!pack_resp( vc_buffer, (UINT32)lis_packet.size())) {
        return false;
    }
    while ( it_pic_list != lis_packet.end() )
    {
        st_pic_info = *it_pic_list;
        if(!pack_resp( vc_buffer, st_pic_info)) {
            return false;
        }
        it_pic_list++;
    }
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetEPicListRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.e_pic_info_list );
}

typedef MSPacketResp_S<MSGetEPicListRespBody_S> MSGetEPicListResp_S;

//设置或修改电子地图图片请求--MsgType = IR_MONITOR_SET_E_PIC_REQ
typedef struct tagMSSetEPicReqBody_S
{
    UINT32 ui_pic_id;
    UINT32 ui_pic_cname_len;
    INT8   ac_pic_cname[PIC_NAME_LEN];
    string s_pic_data;
}MSSetEPicReqBody_S;

typedef struct tagMSSetEPicRespBody_S
{
   UINT32 ui_pic_id;
}MSSetEPicRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSSetEPicReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_pic_id ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_pic_cname_len ) &&
           ( st_packet.ui_pic_cname_len + 1 <= sizeof(st_packet.ac_pic_cname) ) &&        //char*需判断长度,末尾将赋值'\0'
           parse_req( pc_buffer, i_size, i_offset, st_packet.ac_pic_cname, st_packet.ui_pic_cname_len ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_pic_data );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSSetEPicRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_pic_id );
}

typedef MSPacketReq_S<MSSetEPicReqBody_S> MSSetEPicReq_S;
typedef MSPacketResp_S<MSSetEPicRespBody_S> MSSetEPicResp_S;

//设置电子地图图片信息请求--MsgType = IR_MONITOR_SET_E_PIC_INFO_REQ
typedef struct tagMSSetEPicInfoReqBody_S
{
    UINT32 ui_pic_id;
    std::string s_pic_info;
}MSSetEPicInfoReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSSetEPicInfoReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_pic_id ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.s_pic_info );
}

typedef MSPacketReq_S<MSSetEPicInfoReqBody_S> MSSetEPicInfoReq_S;

//获取电子地图图片及信息请求--MsgType = IR_MONITOR_GET_E_PIC_INFO_REQ
typedef struct tagMSGetEPicInfoReqBody_S
{
    UINT32 ui_e_pic_id;
}MSGetEPicInfoReqBody_S;

typedef struct tagMSGetEPicInfoRespBody_S
{
    string s_pic_data;
    string s_pic_info;
}MSGetEPicInfoRespBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSGetEPicInfoReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_e_pic_id );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const EPicData_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_data_len );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetEPicInfoRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.s_pic_data ) &&
           pack_resp( vc_buffer, st_packet.s_pic_info );
}

typedef MSPacketReq_S<MSGetEPicInfoReqBody_S> MSGetEPicInfoReq_S;
typedef MSPacketResp_S<MSGetEPicInfoRespBody_S> MSGetEPicInfoResp_S;

//删除电子地图图片及信息请求--MsgType = IR_MONITOR_DEL_E_PIC_REQ
typedef struct tagMSDelEPicReqBody_S
{
    UINT32 ui_e_pic_id;
}MSDelEPicReqBody_S;

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, MSDelEPicReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_e_pic_id );
}

typedef MSPacketReq_S<MSDelEPicReqBody_S> MSDelEPicReq_S;

// 实时中获取当前调试板类型--MsgType = IR_MONITOR_GET_LIVE_COLOR_IDX_REQ
typedef struct  tagMSLiveGetClrIdxReqBody_S
{
    INT8 acSn[DEV_SN_LEN];
}MSLiveGetClrIdxReqBody_S;

typedef struct tagMSLiveGetClrIdxRespBody_S
{
    INT32 i_color_idx;
}MSLiveGetClrIdxRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset,  MSLiveGetClrIdxReqBody_S &st_packet )
{
    UINT32 i_total_len = ntohl(((MsgHeader_S *)pc_buffer)->total_len) -sizeof(MsgHeader_S);
    memset(st_packet.acSn ,0,sizeof(st_packet.acSn) );
    return (i_total_len <= sizeof(st_packet.acSn)) &&
           parse_req( pc_buffer,i_size,i_offset,st_packet.acSn,i_total_len);

}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSLiveGetClrIdxRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.i_color_idx );
}

typedef MSPacketReq_S<MSLiveGetClrIdxReqBody_S > MSLiveGetClrIdxReq_S ;
typedef MSPacketResp_S<MSLiveGetClrIdxRespBody_S> MSLiveGetClrIdxResp_S;

// 实时中设置调色板类型--MsgType = IR_MONITOR_SET_LIVE_COLOR_IDX_REQ
typedef struct tagMSLiveSetClrIdxReqBody_S
{
    INT32 i_color_idx;
    INT8 acSn[DEV_SN_LEN];
}MSLiveSetClrIdxReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32& i_offset,MSLiveSetClrIdxReqBody_S &st_packet )
{
    UINT32 i_total_len = ntohl(((MsgHeader_S *)pc_buffer)->total_len) -sizeof(MsgHeader_S)-sizeof(st_packet.i_color_idx);
    memset(st_packet.acSn ,0,sizeof(st_packet.acSn) );
    return parse_req( pc_buffer,i_size,i_offset,st_packet.i_color_idx ) &&
           (i_total_len <= sizeof(st_packet.acSn)) &&
           parse_req( pc_buffer,i_size,i_offset,st_packet.acSn,i_total_len);
}

typedef MSPacketReq_S<MSLiveSetClrIdxReqBody_S> MSLiveSetClrIdxReq_S;

// 实时中手动设置色标温度范围--MsgType =IR_MONITOR_SET_LIVE_ADJUST_RANGE_REQ
typedef struct tagMSLiveSetAdjustRangeReqBody_S
{
    float f_max;
    float f_min;
    INT8 acSn[DEV_SN_LEN];
}MSLiveSetAdjustRangeReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset,MSLiveSetAdjustRangeReqBody_S &st_packet )
{
    UINT32 i_total_len = ntohl(((MsgHeader_S *)pc_buffer)->total_len) -sizeof(MsgHeader_S)-sizeof(st_packet.f_max)-sizeof(st_packet.f_min);
    memset(st_packet.acSn ,0,sizeof( st_packet.acSn ) );
    return parse_req( pc_buffer, i_size, i_offset, st_packet.f_max ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.f_min ) &&
           ( i_total_len <= sizeof( st_packet.acSn ) ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.acSn, i_total_len );
}

typedef MSPacketReq_S<MSLiveSetAdjustRangeReqBody_S> MSLiveSetAdjustRangeReq_S;

// 实时中设置色标类型手动还是自动--MsgType = IR_MONITOR_SET_LIVE_ADJUST_TYPE_REQ
typedef struct  tagMSLiveSetAdjustTypeReqBody_S
{
    INT32 i_adjust_type;
    INT8 acSn[DEV_SN_LEN];
}MSLiveSetAdjustTypeReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32& i_offset,  MSLiveSetAdjustTypeReqBody_S &st_packet )
{
    UINT32 i_total_len = ntohl(((MsgHeader_S *)pc_buffer)->total_len) -sizeof(MsgHeader_S)-sizeof(st_packet.i_adjust_type);
    memset(st_packet.acSn ,0,sizeof( st_packet.acSn ) );
    return parse_req( pc_buffer, i_size, i_offset, st_packet.i_adjust_type ) &&
           ( i_total_len <= sizeof( st_packet.acSn ) ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.acSn,i_total_len );
}

typedef MSPacketReq_S<MSLiveSetAdjustTypeReqBody_S> MSLiveSetAdjustTypeReq_S;

// 云台向左请求-MsgType = IR_MONITOR_PLATFORM_LEFT_REQ
// 云台向右请求--MsgType = IR_MONITOR_PLATFORM_RIGHT_REQ
// 云台向上请求--MsgType = IR_MONITOR_PLATFORM_UP_REQ
// 云台向下请求--MsgType = IR_MONITOR_PLATFORM_DOWN_REQ
// 云台向左上请求--MsgType =  IR_MONITOR_PLATFORM_LEFT_UP_REQ
// 云台向右上请求--MsgType = IR_MONITOR_PLATFORM_RIGHT_UP_REQ
// 云台向左下请求--MsgType = IR_MONITOR_PLATFORM_LEFT_DOWN_REQ
// 云台向右下请求--MsgType = IR_MONITOR_PLATFORM_RIGHT_DOWN_REQ
// 云台停止请求--MsgType = IR_MONITOR_PLATFORM_STOP_REQ
typedef struct tagMSPtzDirectCommReqBody_S
{
    INT32 i_sn_len;
    INT8  ac_ptz_sn[PTZ_SN_LEN];
}MSPtzDirectCommReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSPtzDirectCommReqBody_S &st_packet )
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.i_sn_len ) &&
           (st_packet.i_sn_len <= (INT32)(sizeof( st_packet.ac_ptz_sn))) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ac_ptz_sn, st_packet.i_sn_len );
}

typedef MSPacketReq_S<MSPtzDirectCommReqBody_S> MSPtzDirectCommReq_S;

// 云台设置速度请求--MsgType = IR_MONITOR_PLATFORM_SET_SPEED_REQ
typedef struct tagMSPlatFormSpeedReqBody_S
{
    UINT32 ui_group_no;
    UINT32 ui_speed;
}MSPlatFormSpeedReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32& i_offset,MSPlatFormSpeedReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_speed) ;
}

typedef MSPacketReq_S<MSPlatFormSpeedReqBody_S> MSPlatFormSpeedtReq_S;

// 云台移动到预置点请求--MsgType = IR_MONITOR_PLATFORM_TO_PRESET_REQ
// 移动到预置点请求--MsgType = IR_MONITOR_TO_PRESET_REQ
typedef struct tagMSPlatFormToPresetReqBody_S
{
    UINT32 ui_group_no;
    UINT16 us_slide_no;
    UINT16 us_preset_no;
}MSPlatFormToPresetReqBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSPlatFormToPresetReqBody_S &st_packet)
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group_no ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.us_slide_no ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_preset_no );
}

typedef MSPacketReq_S<MSPlatFormToPresetReqBody_S> MSPlatFormToPresetReq_S;

// 删除预置点请求--MsgType = IR_MONITOR_DEL_PRESET_REQ
// 云台删除预置点请求--MsgType = IR_MONITOR_PLATFORM_DEL_PRESET_REQ
typedef struct tagMSDelPresetReqBody_S
{
    UINT32 ui_group_no;
    UINT16 us_slide_no;
    UINT16 us_preset_no;
}MSDelPresetReqBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSDelPresetReqBody_S &st_packet)
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group_no ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.us_slide_no ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_preset_no );
}

typedef MSPacketReq_S<MSDelPresetReqBody_S> MSDelPresetReq_S;

// 获取巡航信息请求--MsgType = IR_MONITOR_GET_CRUISE_INFO_REQ
typedef struct  tagMSGetCruiseInfoReqBody_S
{
    UINT32 ui_group;
}MSGetCruiseInfoReqBody_S;

typedef struct tagMSGetCruiseInfoRespBody_S
{
    UINT32 ui_num;
    tagMSGetCruiseInfoRespBody_S () {
        ui_num = 0;
    }
    CmdCruise cruises[CRUISE_LEN];
}MSGetCruiseInfoRespBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSGetCruiseInfoReqBody_S &st_packet)
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer,const CmdPreset &st_packet)
{
    CmdPreset st_cmd_preset = st_packet;
    st_cmd_preset.slide = htons( st_cmd_preset.slide );
    st_cmd_preset.ptz = htons( st_cmd_preset.ptz );
    st_cmd_preset.second = htonl( st_cmd_preset.second );//ci.alias.length()
    return  pack_resp(vc_buffer, st_cmd_preset.slide) &&
            pack_resp(vc_buffer, st_cmd_preset.ptz ) &&
            pack_resp(vc_buffer, st_cmd_preset.second );
//    return  pack_resp(vc_buffer, st_packet.slide) &&
//            pack_resp(vc_buffer, st_packet.ptz ) &&
//            pack_resp(vc_buffer, st_packet.second );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer,const CmdCruise &st_packet)
{
    CmdCruise st_cmd_cruise = st_packet;
    st_cmd_cruise.if_cur = htons( st_cmd_cruise.if_cur );
    st_cmd_cruise.no = htons( st_cmd_cruise.no );
    st_cmd_cruise.alias_len = htonl( st_cmd_cruise.alias_len );//ci.alias.length()
    st_cmd_cruise.num = htonl( st_cmd_cruise.num );
    return  pack_resp(vc_buffer, st_cmd_cruise.if_cur ) &&
            pack_resp(vc_buffer, st_cmd_cruise.no ) &&
            pack_resp(vc_buffer, st_cmd_cruise.alias_len ) &&
            pack_resp(vc_buffer, st_cmd_cruise.alias, st_cmd_cruise.alias_len ) &&
            pack_resp(vc_buffer, st_cmd_cruise.num ) &&
            ( st_cmd_cruise.num <= ( sizeof(st_cmd_cruise.presets)/sizeof(CmdPreset)) )&&
            pack_resp(vc_buffer, &st_cmd_cruise.presets[0],st_cmd_cruise.num );
//    return  pack_resp(vc_buffer, st_packet.if_cur ) &&
//            pack_resp(vc_buffer, st_packet.no ) &&
//            pack_resp(vc_buffer, st_packet.alias, st_packet.alias_len ) &&
//            pack_resp(vc_buffer, st_packet.num ) &&
//            ( st_packet.num <= ( sizeof(st_packet.presets)/sizeof(CmdPreset)) )&&
//            pack_resp(vc_buffer, &st_packet.presets[0],st_packet.num );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer,const MSGetCruiseInfoRespBody_S &st_packet)
{
    return  pack_resp(vc_buffer, st_packet.ui_num ) &&
            ( st_packet.ui_num <= (sizeof( st_packet.cruises )/sizeof(CmdCruise) ) ) &&
            pack_resp(vc_buffer, &st_packet.cruises[0] , st_packet.ui_num);
}

typedef  MSPacketReq_S<MSGetCruiseInfoReqBody_S> MSGetCruiseInfoReq_S;
typedef MSPacketResp_S<MSGetCruiseInfoRespBody_S> MSGetCruiseInfoResp_S;

// 获取组中预置点信息请求--MsgType = IR_MONITOR_GET_PRESET_INFO_REQ
typedef struct tagMSGetPresetInfoReqBody_S
{
    UINT32 ui_group;
}MSGetPresetInfoReqBody_S;

typedef struct tagMSGetPresetInfoRespBody_S
{
    list<RecordPresetInfo> preset_list;
}MSGetPresetInfoRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetPresetInfoReqBody_S &st_packet )
{
    return parse_req ( pc_buffer, i_size, i_offset, st_packet.ui_group);
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const RecordAnalyseInfo &st_packet )
{
    std::string s_ananame = st_packet.ananame;
    std::string s_analyse = st_packet.analyse;
    s_ananame.push_back( '\0' );
    s_analyse.push_back( '\0' );
    return pack_resp( vc_buffer,st_packet.ui_ana_no) &&
          pack_resp( vc_buffer,s_ananame ) &&
          pack_resp( vc_buffer,s_analyse );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const list< RecordAnalyseInfo > &list_packet )
{
    list<RecordAnalyseInfo>::const_iterator it = list_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)list_packet.size())) {
        return false;
    }
    while ( it!= list_packet.end() )
    {
        if(!pack_resp( vc_buffer, *it)) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const RecordPresetInfo &st_packet )
{
    std::string s_alias = st_packet.alias;
    s_alias.push_back( '\0' );
    return pack_resp( vc_buffer, st_packet.slide ) &&
           pack_resp( vc_buffer, st_packet.ptz ) &&
           pack_resp( vc_buffer, s_alias) &&
           pack_resp( vc_buffer,st_packet.analyseinfo_list);
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const list< RecordPresetInfo > &list_packet )
{

    list<RecordPresetInfo>::const_iterator it = list_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)list_packet.size())) {
        return false;
    }
    while ( it!= list_packet.end() )
    {
        if(!pack_resp( vc_buffer, *it)) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const MSGetPresetInfoRespBody_S &st_packet )
{

    return pack_resp( vc_buffer,st_packet.preset_list);
}

typedef  MSPacketReq_S<MSGetPresetInfoReqBody_S>  MSGetPresetInfoReq_S;
typedef MSPacketResp_S<MSGetPresetInfoRespBody_S>  MSGetPresetInfoResp_S;

// 新增或修改巡航方案请求--MsgType =IR_MONITOR_CHANGE_CRUISE_INFO_REQ
typedef struct tagMSBSetCruiseReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_no;
    UINT32 alias_len;
    INT8 alias[ALIAS_LEN];
    UINT32 ui_num;
    tagMSBSetCruiseReqBody_S() {
       ui_num = 0;
    }
    CmdPreset presets[CRUISE_PRESET_LEN];
}MSBSetCruiseReqBody_S;

typedef struct tagMSBSetCruiseRespBody_S
{
    UINT32 ui_no;
}MSBSetCruiseRespBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, CmdPreset &st_packet)
{
    return parse_req( pc_buffer, i_size, i_offset, st_packet.slide ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ptz ) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.second );
}

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset,  MSBSetCruiseReqBody_S &st_packet)
{
    UINT32 ui_max_num = (sizeof(st_packet.presets)/sizeof(CmdPreset));
    return parse_req( pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_no)&&
           parse_req( pc_buffer, i_size, i_offset, st_packet.alias_len)&&
            (st_packet.alias_len <= sizeof(st_packet.alias)) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.alias,st_packet.alias_len) &&
           parse_req( pc_buffer, i_size, i_offset, st_packet.ui_num ) &&
            (st_packet.ui_num <= (sizeof(st_packet.presets)/sizeof(CmdPreset)))&&
           parse_req( pc_buffer, i_size, i_offset, &st_packet.presets[0],ui_max_num,st_packet.ui_num  );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer,const MSBSetCruiseRespBody_S &st_packet)
{
    return pack_resp( vc_buffer,st_packet.ui_no);
}

typedef MSPacketReq_S<MSBSetCruiseReqBody_S> MSBSetCruiseReq_S ;
typedef MSPacketResp_S<MSBSetCruiseRespBody_S> MSBSetCruiseResp_S;

// 删除巡航方案请求--MsgType = IR_MONITOR_DEL_CRUISE_INFO_REQ
typedef struct tagMSDelCruiseReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_no;
}MSDelCruiseReqBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSDelCruiseReqBody_S &st_packet)
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_no);
}

typedef MSPacketReq_S<MSDelCruiseReqBody_S>MSDelCruiseReq_S ;

// 设置图片请求--MsgType = IR_MONITOR_SET_JPG_INFO_REQ
typedef struct tagMSSetPicReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_num;
    tagMSSetPicReqBody_S() {
        ui_num = 0;
    }
    PicPreset presets[PICPRESETS_LEN];
}MSSetPicReqBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, PicPreset &st_packet)
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.x ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.y );
}

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset, MSSetPicReqBody_S &st_packet)
{
    UINT32 ui_max_num =(sizeof(st_packet.presets)/sizeof( PicPreset ) );
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_num) &&
           (st_packet.ui_num <= (sizeof(st_packet.presets)/sizeof( PicPreset ))) &&
           parse_req(pc_buffer, i_size, i_offset, &st_packet.presets[0],ui_max_num,st_packet.ui_num );
 }
typedef MSPacketReq_S<MSSetPicReqBody_S> MSSetPicReq_S;

// 获取图片请求--MsgType = IR_MONITOR_GET_JPG_INFO_REQ
typedef struct tagMSGetPicReqBody_S
{
    UINT32 ui_group;
} MSGetPicReqBody_S;

typedef struct tagMSGetPicRespBody_S
{
    vector<GroupCruisePreset> preset;
    UINT32 pic_len;
    UINT8  pic[MAX_PIC_LEN];
} MSGetPicRespBody_S;

template<>
inline bool parse_req(const INT8* pc_buffer, INT32 i_size, INT32& i_offset,MSGetPicReqBody_S &st_packet)
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const GroupCruisePreset &st_packet )
{
    return pack_resp( vc_buffer, (UINT16)st_packet.slide_preset ) &&
           pack_resp( vc_buffer, (UINT16)st_packet.ptz_preset ) &&
           pack_resp( vc_buffer, (UINT16)st_packet.x_coordinate) &&
           pack_resp( vc_buffer, (UINT16)st_packet.y_coordinate );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer,const vector<GroupCruisePreset> &vec_packet )
{
    vector<GroupCruisePreset>::const_iterator it = vec_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)vec_packet.size())) {
        return false;
    }
    while ( it!= vec_packet.end() )
    {
        if(!pack_resp( vc_buffer, *it)) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSGetPicRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.preset ) &&
           (st_packet.pic_len <= sizeof(st_packet.pic)) &&
           pack_resp( vc_buffer, st_packet.pic,st_packet.pic_len );
}

typedef MSPacketReq_S<MSGetPicReqBody_S> MSGetPicReq_S;
typedef MSPacketResp_S<MSGetPicRespBody_S>MSGetPicResp_S ;

// 删除图片请求--MsgType = IR_MONITOR_DEL_JPG_INFO_REQ
typedef struct tagMSDelPicReqBody_S
{
    UINT32 ui_group;
}MSDelPicReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSDelPicReqBody_S & st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group);
}

typedef MSPacketReq_S<MSDelPicReqBody_S> MSDelPicReq_S;

// 开始巡航请求--MsgType =IR_MONITOR_START_CRUISE_REQ
 typedef struct tagMSStartCruiseReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_no;
}MSStartCruiseReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSStartCruiseReqBody_S & st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_no);
}

typedef MSPacketReq_S<MSStartCruiseReqBody_S> MSStartCruiseReq_S;

// 停止巡航请求--MsgType = IR_MONITOR_STOP_CRUISE_REQ
typedef struct tagMSStopCruiseReqBody_S
{
    UINT32 ui_group;
    UINT32 ui_no;
}MSStopCruiseReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSStopCruiseReqBody_S & st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_no);
}

typedef MSPacketReq_S<MSStopCruiseReqBody_S> MSStopCruiseReq_S;

// 设置预置点请求--MsgType = IR_MONITOR_SET_PRESET_REQ
typedef struct tagMSPresetReqBody_S
{
    UINT32 ui_group_no;
    UINT16 us_slide_no;
    UINT16 us_ptz_no;
    string s_cname;
    list<SingleAnaInfo_S> ana_list;
}MSPresetReqBody_S;

typedef struct tagMSPresetRespBody_S
{
    UINT16 us_slide_no;
    UINT16 us_ptz_no;
}MSPresetRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, SingleAnaInfo_S & st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.s_ana_cname) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.s_ana);

}
template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, list<SingleAnaInfo_S> & list_packet )
{
     UINT32 ui_ana_num;
     SingleAnaInfo_S st_ana_info;
     if( !parse_req(pc_buffer, i_size, i_offset, ui_ana_num ) ) {
         return false;
     }
     for( UINT32 ui_i = 0; ui_i < ui_ana_num; ui_i++ ) {
         if( !parse_req(pc_buffer, i_size, i_offset, st_ana_info ) ) {
             return false;
         }
         list_packet.push_back(st_ana_info);
     }
     return true;
}
template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPresetReqBody_S & st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_slide_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_ptz_no) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.s_cname) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ana_list );

    if( b_ret ){
        st_packet.s_cname = st_packet.s_cname.c_str();
        list<SingleAnaInfo_S>::iterator it = st_packet.ana_list.begin();
        for( ; it != st_packet.ana_list.end(); ++it ){
            SingleAnaInfo_S &st_tmp = *it;
            st_tmp.s_ana = st_tmp.s_ana.c_str();
            st_tmp.s_ana_cname = st_tmp.s_ana_cname.c_str();
        }
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPresetRespBody_S & st_packet )
{
    return pack_resp( vc_buffer, st_packet.us_slide_no ) &&
           pack_resp( vc_buffer, st_packet.us_ptz_no );
}

typedef MSPacketReq_S<MSPresetReqBody_S> MSPresetReq_S;
typedef MSPacketResp_S<MSPresetRespBody_S> MSPresetResp_S;

//增加预置点分析请求 --MsgType = IR_MONITOR_PRESET_ANA_ADD_REQ
typedef struct tagMSPresetAnaAddReqBody_S
{
       UINT32 ui_group_no;
       UINT16 ui_slide_no;
       UINT16 ui_ptz_no;
       std::string str_ana_cname;
       std::string str_ana;
} MSPresetAnaAddReqBody_S;

typedef struct tagMSPresetAnaAddRespBody_S
{
    UINT32 ui_no;
} MSPresetAnaAddRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset,  MSPresetAnaAddReqBody_S & st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_ana_cname )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_ana );

    if( b_ret ){
        st_packet.str_ana_cname = st_packet.str_ana_cname.c_str();
        st_packet.str_ana = st_packet.str_ana.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPresetAnaAddRespBody_S & st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_no );

}

typedef MSPacketReq_S<MSPresetAnaAddReqBody_S> MSPresetAnaAddReq_S ;
typedef MSPacketResp_S<MSPresetAnaAddRespBody_S> MSPresetAnaAddResp_S;

//change preset point analyse request--MsgType = IR_MONITOR_PRESET_ANA_CHG_REQ
typedef struct tagMSPresetAnaChgReqBody_S
{
    UINT32 ui_group_no;
    UINT16 ui_slide_no;
    UINT16 ui_ptz_no;
    UINT32 ui_preset_ana_no;
    std::string str_ana_cname;
    std::string str_ana;
}MSPresetAnaChgReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset, MSPresetAnaChgReqBody_S & st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_preset_ana_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_ana_cname )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_ana );

    if( b_ret ){
        st_packet.str_ana_cname = st_packet.str_ana_cname.c_str();
        st_packet.str_ana = st_packet.str_ana.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSPresetAnaChgReqBody_S> MSPresetAnaChgReq_S ;

//del preset point analyse request--MsgType = IR_MONITOR_PRESET_ANA_DEL_REQ
typedef struct tagMSPresetAnaDelReqBody_S
{
    UINT32 ui_group_no;
    UINT16 ui_slide_no;
    UINT16 ui_ptz_no;
    UINT32 ui_preset_ana_no;
}MSPresetAnaDelReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset, MSPresetAnaDelReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz_no )&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_preset_ana_no );

}

typedef MSPacketReq_S<MSPresetAnaDelReqBody_S>MSPresetAnaDelReq_S ;

//change preset point position request--MsgType = IR_MONITOR_MOD_PRESET_REQ
typedef struct tagMSModPresetReqBody_S
{
    UINT32 ui_group;
    UINT16 ui_slide;
    UINT16 ui_ptz;
}MSModPresetReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 & i_offset, MSModPresetReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz);
}

typedef MSPacketReq_S<MSModPresetReqBody_S> MSModPresetReq_S;

//添加账号--MsgType =  IR_MONITOR_ADD_ACCOUNT_REQ
typedef struct tagMSAddAccountReqBody_S
{
    UINT32 ui_passwd_len;
    UINT8  auc_digest[PSW_LEN];
    std::string str_user_name;
    UINT32 ui_user_premission;
    UINT64 ul_out_dev_premission;
    std::map<UINT32, UINT64> ui_group_premission;
    std::string str_extend;
}MSAddAccountReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSAddAccountReqBody_S &st_packet )
{
    bool b_ret =  parse_req(pc_buffer, i_size, i_offset, st_packet.ui_passwd_len ) &&
           (st_packet.ui_passwd_len <= sizeof(st_packet.auc_digest)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.auc_digest ,st_packet.ui_passwd_len ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_user_name ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_user_premission ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ul_out_dev_premission ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_premission)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_extend );

    if( b_ret ){
        st_packet.str_user_name = st_packet.str_user_name.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSAddAccountReqBody_S> MSAddAccountReq_S;

//修改账号权限--MsgType = IR_MONITOR_CHG_ACCOUNT_PREMISSION_REQ
typedef struct tagMSChgAccountPremissionReqBody_S
{
    UINT32 ui_passwd_len;
    UINT8  auc_digest[PSW_LEN];
    std::string str_user_name;
    UINT32 ui_user_premission;
    UINT64 ul_out_dev_premission;
    std::map<UINT32, UINT64> ui_group_premission;
    std::string str_extend;
}MSChgAccountPremissionReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSChgAccountPremissionReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_passwd_len ) &&
           (st_packet.ui_passwd_len <= sizeof(st_packet.auc_digest)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.auc_digest ,st_packet.ui_passwd_len ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_user_name ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_user_premission ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ul_out_dev_premission ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group_premission)&&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_extend );

    if( b_ret ){
        st_packet.str_user_name = st_packet.str_user_name.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSChgAccountPremissionReqBody_S> MSChgAccountPremissionReq_S;

//修改账号密码--MsgType = IR_MONITOR_CHG_ACCOUNT_PASSWORD_REQ
typedef struct tagMSChgAccountPasswdReqBody_S
{
    UINT32 ui_passwd_len;
    //std::string str_passwd;
    UINT8  auc_digest[PSW_LEN];
    std::string str_user_name;
    UINT32 ui_user_passwd_len;
    //std::string str_user_passwd;
    UINT8  auc_user_digest[PSW_LEN];
}MSChgAccountPasswdReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSChgAccountPasswdReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_passwd_len ) &&
           (st_packet.ui_passwd_len <= sizeof(st_packet.auc_digest)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.auc_digest ,st_packet.ui_passwd_len ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_user_name ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_user_passwd_len ) &&
            (st_packet.ui_user_passwd_len <= sizeof(st_packet.auc_user_digest)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.auc_user_digest, st_packet.ui_user_passwd_len );

    if( b_ret ){
        st_packet.str_user_name = st_packet.str_user_name.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S<MSChgAccountPasswdReqBody_S> MSChgAccountPasswdReq_S;

//删除账号--MsgType= IR_MONITOR_DEL_ACCOUNT_REQ
typedef struct tagMSDelAccountReqBody_S
{
    UINT32 ui_passwd_len;
    UINT8  auc_digest[PSW_LEN];
    UINT32 ui_user_len;
    std::string str_user_name;
}MSDelAccountReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSDelAccountReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.ui_passwd_len ) &&
           (st_packet.ui_passwd_len <= sizeof(st_packet.auc_digest)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.auc_digest ,st_packet.ui_passwd_len ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.str_user_name );

    if( b_ret ){
        st_packet.str_user_name = st_packet.str_user_name.c_str();
    }

    return b_ret;
}

typedef MSPacketReq_S< MSDelAccountReqBody_S>  MSDelAccountReq_S;

//请求账号列表--MsgType =  IR_MONITOR_QUERY_ACCOUNT_REQ
typedef struct tagMSQueryAccountPremissionReqBody_S
{
    std::string str_user_name;
    UINT32 ui_get_other_flag;//1 all
}MSQueryAccountPremissionReqBody_S;

typedef struct tagMSQueryAccountPremissionRespBody_S
{
    std::map<std::string, UserSecret_S> account;
}MSQueryAccountPremissionRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSQueryAccountPremissionReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.str_user_name ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_get_other_flag );

    if( b_ret ){
        st_packet.str_user_name = st_packet.str_user_name.c_str();
    }

    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const UserSecret_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.str_user_name) &&
           pack_resp( vc_buffer, st_packet.i_user_premission ) &&
           pack_resp( vc_buffer, st_packet.ul_out_group_premission ) &&
           pack_resp( vc_buffer, st_packet.ui_group_premission ) &&
           pack_resp( vc_buffer, st_packet.str_extend );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const std::map<std::string, UserSecret_S> &map_packet )
{
    bool b_ret =false;
    UINT32  ui_account_count = map_packet.size();
    b_ret = pack_resp( vc_buffer,ui_account_count );
    std::map<std::string, UserSecret_S>::const_iterator it = map_packet.begin();
    for( ; it != map_packet.end(); ++it ){
        UserSecret_S user_secret_tmp = it->second;
        if( user_secret_tmp.str_user_name.empty() ){
            user_secret_tmp.str_user_name = it->first;
        }
        b_ret &= pack_resp( vc_buffer, it->second );
   }
   return b_ret;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSQueryAccountPremissionRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.account );
}

typedef MSPacketReq_S<MSQueryAccountPremissionReqBody_S> MSQueryAccountPremissionReq_S;
typedef MSPacketResp_S<MSQueryAccountPremissionRespBody_S> MSQueryAccountPremissionResp_S;

// start fixed point cruise --MsgType = IR_MONITOR_START_FIXED_POINT_CRUISE_REQ
typedef struct tagMSStartFixedPointCruiseReqBody_S
{
    UINT32 ui_group;
}MSStartFixedPointCruiseReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSStartFixedPointCruiseReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

typedef MSPacketReq_S<MSStartFixedPointCruiseReqBody_S> MSStartFixedPointCruiseReq_S;

// stop fixed point cruise--MsgType = IR_MONITOR_STOP_FIXED_POINT_CRUISE_REQ
typedef struct tagMSStopFixedPointCruiseReqBody_S
{
    UINT32 ui_group;
}MSStopFixedPointCruiseReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,  MSStopFixedPointCruiseReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

typedef MSPacketReq_S<MSStopFixedPointCruiseReqBody_S> MSStopFixedPointCruiseReq_S;

// 获取定点巡航状态 --MsgType = IR_MONITOR_GET_FIXED_POINT_CRUISE_STATE_REQ
typedef struct tagMSGetFixedPointCruiseStateReqBody_S
{
    UINT32 ui_group;
}MSGetFixedPointCruiseStateReqBody_S;

typedef struct tagMSGetFixedPointCruiseStateRespBody_S
{
    UINT32 ui_state;
    UINT32 ui_time;
}MSGetFixedPointCruiseStateRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetFixedPointCruiseStateReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSGetFixedPointCruiseStateRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_state ) &&
           pack_resp( vc_buffer, st_packet.ui_time );
}

typedef MSPacketReq_S<MSGetFixedPointCruiseStateReqBody_S> MSGetFixedPointCruiseStateReq_S;
typedef MSPacketResp_S<MSGetFixedPointCruiseStateRespBody_S> MSGetFixedPointCruiseStateResp_S;

//设置红外图像参数：辐射率--MsgType = IR_MONITOR_PICTURE_SET_RADIATION_REQ
typedef struct tagMSPicSetRadiationReqBody_S
{
    UINT32 ui_group;
    float  f_radiation;
}MSPicSetRadiationReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPicSetRadiationReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.f_radiation );
}

typedef MSPacketReq_S<MSPicSetRadiationReqBody_S> MSPicSetRadiationReq_S;

//设置红外图像参数：距离--MsgType =IR_MONITOR_PICTURE_SET_DISTANCE_REQ
typedef struct tagMSPicSetDistanceReqBody_S
{
    UINT32 ui_group;
    float  f_distance;
}MSPicSetDistanceReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPicSetDistanceReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.f_distance );
}

typedef MSPacketReq_S<MSPicSetDistanceReqBody_S> MSPicSetDistanceReq_S;

// 设置红外图像参数：环境温度 --MsgType = IR_MONITOR_PICTURE_SET_ENVIRONMENT_TEMP_REQ
typedef struct tagMSPicSetEnvTempReqBody_S
{
    UINT32 ui_group;
    float  f_envtemp;
}MSPicSetEnvTempReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPicSetEnvTempReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.f_envtemp);
}

typedef MSPacketReq_S<MSPicSetEnvTempReqBody_S> MSPicSetEnvTempReq_S;

//设置红外图像参数：相对湿度 --MsgType = IR_MONITOR_PICTURE_SET_OPPOHITE_HUMIDITY_REQ
typedef struct tagMSPicSetOppohiteHumidityReqBody_S
{
    UINT32 ui_group;
    float  f_humidity;
}MSPicSetOppohiteHumidityReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPicSetOppohiteHumidityReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.f_humidity );
}

typedef MSPacketReq_S<MSPicSetOppohiteHumidityReqBody_S> MSPicSetOppohiteHumidityReq_S;

//获取红外图像参数：辐射率 --MsgType = IR_MONITOR_PICTURE_GET_RADIATION_REQ
typedef struct tagMSPicGetRadiationReqBody_S
{
    UINT32 ui_group;
}MSPicGetRadiationReqBody_S;

typedef struct tagMSPicGetRadiationRespBody_S
{
    float  f_radiation;
}MSPicGetRadiationRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSPicGetRadiationReqBody_S&st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPicGetRadiationRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.f_radiation);
}

typedef MSPacketReq_S<MSPicGetRadiationReqBody_S>MSPicGetRadiationReq_S ;
typedef MSPacketResp_S<MSPicGetRadiationRespBody_S>MSPicGetRadiationResp_S ;

//获取红外图像参数：距离--MsgType =IR_MONITOR_PICTURE_GET_DISTANCE_REQ
typedef struct tagMSPicGetDistanceReqBody_S
{
    UINT32 ui_group;
}MSPicGetDistanceReqBody_S;

typedef struct tagMSPicGetDistanceRespBody_S
{
    float  f_distance;
}MSPicGetDistanceRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,MSPicGetDistanceReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPicGetDistanceRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.f_distance);
}

typedef MSPacketReq_S<MSPicGetDistanceReqBody_S> MSPicGetDistanceReq_S;
typedef MSPacketResp_S<MSPicGetDistanceRespBody_S> MSPicGetDistanceResp_S;

//获取红外图像参数：环境温度--MsgType =IR_MONITOR_PICTURE_GET_ENVIRONMENT_TEMP_REQ
typedef struct tagMSPicGetEnvTempReqBody_S
{
    UINT32 ui_group;
}MSPicGetEnvTempReqBody_S;

typedef struct tagMSPicGetEnvTempRespBody_S
{
    float  f_envtemp;
}MSPicGetEnvTempRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,MSPicGetEnvTempReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPicGetEnvTempRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.f_envtemp);
}

typedef MSPacketReq_S<MSPicGetEnvTempReqBody_S> MSPicGetEnvTempReq_S;
typedef MSPacketResp_S<MSPicGetEnvTempRespBody_S> MSPicGetEnvTempResp_S;

//获取红外图像参数：相对湿度--MsgType =IR_MONITOR_PICTURE_GET_OPPOHITE_HUMIDITY_REQ
typedef struct tagMSPicGetOppohiteHumidityReqBody_S
{
    UINT32 ui_group;
}MSPicGetOppohiteHumidityReqBody_S;

typedef struct tagMSPicGetOppohiteHumidityRespBody_S
{
    float  f_humidity;
}MSPicGetOppohiteHumidityRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,MSPicGetOppohiteHumidityReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group );
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSPicGetOppohiteHumidityRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.f_humidity );
}

typedef MSPacketReq_S<MSPicGetOppohiteHumidityReqBody_S> MSPicGetOppohiteHumidityReq_S;
typedef MSPacketResp_S<MSPicGetOppohiteHumidityRespBody_S>MSPicGetOppohiteHumidityResp_S ;

// 导轨设置请求--MsgType =IR_MONITOR_SLIDE_SET_REQ
typedef struct tagMSSlideSetReqBody_S
{
    INT8   ac_ip[SLIDE_IP_LEN];
    UINT16 us_port;
}MSSlideSetReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,MSSlideSetReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ac_ip,sizeof(st_packet.ac_ip)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_port);
}

typedef MSPacketReq_S<MSSlideSetReqBody_S> MSSlideSetReq_S;

// 导轨删除请求--MsgType =IR_MONITOR_SLIDE_DEL_REQ
typedef struct tagMSSlideDelReqBody_S
{
    INT8   ac_ip[SLIDE_IP_LEN];
    UINT16 us_port;
}MSSlideDelReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset,MSSlideDelReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ac_ip,sizeof(st_packet.ac_ip)) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.us_port);
}

typedef MSPacketReq_S<MSSlideDelReqBody_S> MSSlideDelReq_S;

// 温度趋势曲线获取请求--MsgType = IR_MONITOR_GET_ANALYS_TEMP_REQ
typedef struct tagMSGetAllAnaReqBody_S
{
    UINT32 ui_group;
    UINT16 ui_slide;
    UINT16 ui_ptz;
    UINT32 ui_start_time;
    UINT32 ui_end_time;
}MSGetAllAnaReqBody_S;

typedef struct tagMSGetAllAnaRespBody_S
{
    std::list<PresetAnaFileInfo_S> preset_ana_file_list;
}MSGetAllAnaRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetAllAnaReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_start_time ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_end_time );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const PresetAnaFileInfo_S &st_packet )
{

    return pack_resp( vc_buffer,st_packet.ui_start_time_res ) &&
           pack_resp( vc_buffer,st_packet.str_file_data_res );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const list< PresetAnaFileInfo_S > &list_packet )
{

    list<PresetAnaFileInfo_S>::const_iterator it = list_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)list_packet.size())) {
        return false;
    }
    while ( it!= list_packet.end() )
    {
        if(!pack_resp( vc_buffer, *it)) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetAllAnaRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.preset_ana_file_list );
}
typedef MSPacketReq_S<MSGetAllAnaReqBody_S> MSGetAllAnaReq_S;
typedef MSPacketResp_S<MSGetAllAnaRespBody_S> MSGetAllAnaResp_S;

// 温度趋势曲线上一帧图片请求--MsgType = IR_MONITOR_CURVE_UP_FRAME_REQ
typedef struct tagMSCurveUpFrameReqBody_S
{
    UINT32 ui_group;
    UINT16 ui_slide;
    UINT16 ui_ptz;
    UINT32 ui_cur_frame_utcs;
    UINT32 ui_cur_frame_utcms;
}MSCurveUpFrameReqBody_S;

typedef struct tagMSCurveUpFrameRespBody_S
{
    UINT32 ui_frame_utcs;
    UINT32 ui_frame_utcms;
    UINT32 ui_frame_num;
    tagMSCurveUpFrameRespBody_S () {
        ui_frame_num = 0;
    }
    CurveUp_S st_frame_info[FRAME_COUNT];
}MSCurveUpFrameRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSCurveUpFrameReqBody_S &st_packet )
{
    return  parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
            parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide ) &&
            parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz ) &&
            parse_req(pc_buffer, i_size, i_offset, st_packet.ui_cur_frame_utcs ) &&
            parse_req(pc_buffer, i_size, i_offset, st_packet.ui_cur_frame_utcms );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const CurveUp_S &st_packet )
{
          return   pack_resp( vc_buffer, st_packet.ui_frame_type ) &&
                   pack_resp( vc_buffer, st_packet.str_frame );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSCurveUpFrameRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_frame_utcs) &&
           pack_resp( vc_buffer, st_packet.ui_frame_utcms ) &&
           pack_resp( vc_buffer, st_packet.ui_frame_num ) &&
           ( st_packet.ui_frame_num <= ( sizeof(st_packet.st_frame_info)/sizeof(CurveUp_S))) &&
           pack_resp( vc_buffer, &st_packet.st_frame_info[0],st_packet.ui_frame_num );
}
typedef MSPacketReq_S<MSCurveUpFrameReqBody_S> MSCurveUpFrameReq_S;
typedef MSPacketResp_S<MSCurveUpFrameRespBody_S> MSCurveUpFrameResp_S;

// 温度趋势曲线下一帧图片请求--MsgType = IR_MONITOR_CURVE_DOWN_FRAME_REQ
typedef struct tagMSCurveDownFrameReqBody_S
{
    UINT32 ui_group;
    UINT16 ui_slide;
    UINT16 ui_ptz;
    UINT32 ui_cur_frame_utcs;
    UINT32 ui_cur_frame_utcms;
}MSCurveDownFrameReqBody_S;

typedef struct tagMSCurveDownFrameRespBody_S
{
    UINT32 ui_frame_utcs;
    UINT32 ui_frame_utcms;
    UINT32 ui_frame_num;
    tagMSCurveDownFrameRespBody_S () {
         ui_frame_num = 0;
    }

    CurveDown_S st_frame_info[FRAME_COUNT];
}MSCurveDownFrameRespBody_S;
template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSCurveDownFrameReqBody_S &st_packet )
{
    return parse_req(pc_buffer, i_size, i_offset, st_packet.ui_group ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_slide ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_ptz ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_cur_frame_utcs ) &&
           parse_req(pc_buffer, i_size, i_offset, st_packet.ui_cur_frame_utcms );
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const CurveDown_S &st_packet )
{
    return  pack_resp( vc_buffer, st_packet.ui_frame_type ) &&
            pack_resp( vc_buffer, st_packet.str_frame);

}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSCurveDownFrameRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_frame_utcs) &&
           pack_resp( vc_buffer, st_packet.ui_frame_utcms ) &&
           pack_resp( vc_buffer, st_packet.ui_frame_num )&&
            (st_packet.ui_frame_num <= (sizeof(st_packet.st_frame_info)/sizeof(CurveDown_S))) &&
           pack_resp( vc_buffer, &st_packet.st_frame_info[0],st_packet.ui_frame_num );
}

typedef MSPacketReq_S<MSCurveDownFrameReqBody_S> MSCurveDownFrameReq_S;
typedef MSPacketResp_S<MSCurveDownFrameRespBody_S> MSCurveDownFrameResp_S;

// 获取服务端当前时间请求--MsgType = IR_MONITOR_GET_TIME_REQ
typedef struct tagMSGetServerTimeRespBody_S
{
    UINT32 ui_cur_time_utcs;
    UINT32 ui_cur_time_utcms;
}MSGetServerTimeRespBody_S;

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const MSGetServerTimeRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.ui_cur_time_utcs) &&
           pack_resp( vc_buffer, st_packet.ui_cur_time_utcms );
}

typedef MSPacketResp_S<MSGetServerTimeRespBody_S> MSGetServerTimeResp_S;

// 获取当前设备图片请求--MsgType =IR_MONITOR_DEV_GET_CUR_PIC_REQ
typedef struct tagMSGetDevCurisePicReqBody_S {
    list<std::string>dev_sn_list;
}MSGetDevCurisePicReqBody_S;

typedef struct tagMSGetDevCurisePicRespBody_S {
    list<DevCurPicData_S> pic_data_list;
}MSGetDevCurisePicRespBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, list<std::string> &list_packet )
{
     UINT32 ui_sn_num;
     std::string st_sn_info;
     if( !parse_req(pc_buffer, i_size, i_offset, ui_sn_num ) ) {
         return false;
     }
     for( UINT32 ui_i = 0; ui_i < ui_sn_num; ui_i++ ) {
         if( !parse_req(pc_buffer, i_size, i_offset, st_sn_info ) ) {
             return false;
         }
         list_packet.push_back(st_sn_info);
     }
     return true;
}

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSGetDevCurisePicReqBody_S &st_packet )
{
    bool b_ret = parse_req(pc_buffer, i_size, i_offset, st_packet.dev_sn_list );

    if( b_ret ){
        list<std::string>::iterator it = st_packet.dev_sn_list.begin();
        for( ; it != st_packet.dev_sn_list.end(); ++it ){
            std::string &st_tmp = *it;
            st_tmp = st_tmp.c_str();
        }
    }
    return b_ret;
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const DevCurPicData_S &st_packet )
{

    return pack_resp( vc_buffer,st_packet.s_sn )&&
           pack_resp( vc_buffer,st_packet.s_pic_data );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const list< DevCurPicData_S > &list_packet )
{

    list<DevCurPicData_S>::const_iterator it = list_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)list_packet.size())) {
        return false;
    }
    while ( it!= list_packet.end() )
    {
        if(!pack_resp( vc_buffer, *it)) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSGetDevCurisePicRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.pic_data_list );
}

typedef MSPacketReq_S<MSGetDevCurisePicReqBody_S> MSGetDevCurisePicReq_S;
typedef MSPacketResp_S<MSGetDevCurisePicRespBody_S> MSGetDevCurisePicResp_S;

//IPC设备信息请求协议--MsgType = IR_MONITOR_28181_QUERY_DEVICE_INFO
typedef struct tagMSQueryDevInfoRespBody_S
{
    list <DevDataInfo_S> dev_data_list;
}MSQueryDevInfoRespBody_S;

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const DevDataInfo_S &st_packet )
{

    return pack_resp( vc_buffer,st_packet.str_dev_name ) &&
           pack_resp( vc_buffer,st_packet.ui_dev_type ) &&
           pack_resp( vc_buffer,st_packet.str_dev_manufacturers ) &&
           pack_resp( vc_buffer,st_packet.str_dev_model ) &&
           pack_resp( vc_buffer,st_packet.str_dev_administrative_area ) &&
           pack_resp( vc_buffer,st_packet.str_precinct ) &&
           pack_resp( vc_buffer,st_packet.str_dev_install_address ) &&
           pack_resp( vc_buffer,st_packet.ui_sub_dev ) &&
           pack_resp( vc_buffer,st_packet.ui_singal_safe_mode ) &&
           pack_resp( vc_buffer,st_packet.ui_registry_mode ) &&
           pack_resp( vc_buffer,st_packet.str_certificate_serial_num ) &&
           pack_resp( vc_buffer,st_packet.ui_certificate_valid ) &&
           pack_resp( vc_buffer,st_packet.ui_invalid_reason_code ) &&
           pack_resp( vc_buffer,st_packet.str_certificate_termination_valid ) &&
           pack_resp( vc_buffer,st_packet.ui_confidentiality_properties ) &&
           pack_resp( vc_buffer,st_packet.str_ip_address ) &&
           pack_resp( vc_buffer,st_packet.ui_port ) &&
           pack_resp( vc_buffer,st_packet.str_dev_pwd ) &&
           pack_resp( vc_buffer,st_packet.ui_dev_stat ) &&
           pack_resp( vc_buffer,st_packet.f_longitude ) &&
           pack_resp( vc_buffer,st_packet.f_latitude ) &&
           pack_resp( vc_buffer,st_packet.str_dev_firmware_version ) &&
           pack_resp( vc_buffer,st_packet.ui_vid_input_num );
}

template<>
inline bool pack_resp(std::vector<char>& vc_buffer, const list< DevDataInfo_S > &list_packet )
{

    list<DevDataInfo_S>::const_iterator it = list_packet.begin();
    if(!pack_resp( vc_buffer, (UINT32)list_packet.size())) {
        return false;
    }
    while ( it!= list_packet.end() )
    {
        if( !pack_resp( vc_buffer, *it ) ) {
            return false;
        }
        it++;
    }
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const MSQueryDevInfoRespBody_S &st_packet )
{
    return pack_resp( vc_buffer, st_packet.dev_data_list );
}

typedef MSPacketResp_S<MSQueryDevInfoRespBody_S> MSQueryDevInfoResp_S;

//设备远程启动--MsgType = IR_MONITOR_28181_DEVICE_BOOT
typedef struct tagMSDevBootReqBody_S
{
    std::string str_sn;
}MSDevBootReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSDevBootReqBody_S &st_packet )
{
    return  parse_req( pc_buffer, i_size, i_offset, st_packet.str_sn ) ;
}

typedef MSPacketReq_S<MSDevBootReqBody_S> MSDevBootReq_S;

//开始发送视频数据协议--MsgType =IR_MONITOR_28181_SEND_FRAME
typedef struct tagMSSendFrameReqBody_S
{
    std::string str_sn;
    std::string str_ip_address;
    UINT32      ui_port;
    INT32       i_vid_type; /*STREAM_TYPE_E枚举类型*/
}MSSendFrameReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSSendFrameReqBody_S &st_packet )
{
    return  parse_req( pc_buffer, i_size, i_offset, st_packet.str_sn ) &&
            parse_req( pc_buffer, i_size, i_offset, st_packet.str_ip_address ) &&
            parse_req( pc_buffer, i_size, i_offset, st_packet.ui_port ) &&
            parse_req( pc_buffer, i_size, i_offset, st_packet.i_vid_type );
}
typedef MSPacketReq_S<MSSendFrameReqBody_S> MSSendFrameReq_S;

//停止发送视频数据协议 --MsgType = IR_MONITOR_28181_STOP_SEND_FRAME
typedef struct tagMSStopFrameReqBody_S
{
    std::string str_sn;
    std::string str_ip_address;
    UINT32      ui_port;
}MSStopFrameReqBody_S;

template<>
inline bool parse_req( const INT8* pc_buffer, INT32 i_size, INT32 &i_offset, MSStopFrameReqBody_S &st_packet )
{
    return  parse_req( pc_buffer, i_size, i_offset, st_packet.str_sn ) &&
            parse_req( pc_buffer, i_size, i_offset, st_packet.str_ip_address ) &&
            parse_req( pc_buffer, i_size, i_offset, st_packet.ui_port );
}

typedef MSPacketReq_S<MSStopFrameReqBody_S> MSStopFrameReq_S;

//* 客户端退出通知--MsgType = IR_MONITOR_CLI_LOGOUT_NOTICE

#endif
