#ifndef DATABASETYPEDEF_H
#define DATABASETYPEDEF_H

#include <string>
#include <list>
#include <mysql/mysql.h>
#include "comm_typedef.h"

#define SQL_QUERY_LEN 1024
#define DATETIME_STR_LEN 128
#define PRESET_INFO_LEN 1024
#define MAX_PIC_LEN 10*1024*1024
#define PIC_FILE_NAME_LEN 256
#define MAX_INIT_DATA_LEN 2*1024*1024

#define VISIBLE_TYPE "0"
#define INFRA_TYPE "1"

using namespace std;


typedef struct tagDeviceInfo{
    tagDeviceInfo(){
        i_class = 0;
        i_port = 0;
        i_cur_preset = 0;
    }
    INT32 i_class; //设备类型 0:可见光, 1:红外, 2:云台, 3,4:导轨
    string s_type;  //设备型号
    string s_sn;    //设备SN
    string s_ip;    //设备IP
    INT32 i_port;  //设备Port
    string s_mac;   //设备mac
    INT32  i_cur_preset; //设备当前预置点
}DeviceInfo_S;

struct DeviceInfoCmp_S
{
    bool operator()( const DeviceInfo_S &left, const DeviceInfo_S &right ) const
    {
        /*if( left.ui_group != right.ui_group ){
            return left.ui_group < right.ui_group;
        }
        if( left.us_slide != right.us_slide ){
            return left.us_slide < right.us_slide;
        }*/

        return left.s_sn < right.s_sn;
    }
};

typedef struct tagRecordFileInfo{
    tagRecordFileInfo(){
        type = 0;
        size = 0;
        group = 0;
        slide = 0;
        ptz = 0;
        b_fixed_point = false;
    }
    string name;
    INT32 type;
    INT32 size;
    timeval st_begin;
    timeval st_end;
    string sn;
    INT32 group;
    INT16 slide;
    INT16 ptz;
    bool b_fixed_point;
}RecordFileInfo;

typedef struct tagGroupCruisePreset{
    tagGroupCruisePreset(){
        slide_preset = 0;
        ptz_preset = 0;
        x_coordinate = 0;
        y_coordinate = 0;
    }
    INT32 slide_preset;
    INT32 ptz_preset;
    INT32 x_coordinate;
    INT32 y_coordinate;
}GroupCruisePreset;

typedef struct tagRecordGroupInfo{
    tagRecordGroupInfo(){
        group_id = 0;
    }
    INT32 group_id;
    string alias;
    string slide_sn;
    string ptz_sn;
    string visible_sn;
    string ifnra_sn;
}RecordGroupInfo;

typedef struct tagRecordPresetInfo{
    tagRecordPresetInfo(){
        presetnum = 0;
        slide = 0;
        ptz = 0;
        groupid = 0;
    }
    UINT32 presetnum;
    string alias;
    UINT16 slide;
    UINT16 ptz;
    UINT32 groupid;
    list<RecordAnalyseInfo> analyseinfo_list;
}RecordPresetInfo;

typedef struct tagRecordCruiseInfo{
    tagRecordCruiseInfo(){
        cruisenum = 0;
        groupid = 0;
        alive = 0;
    }
    UINT32 cruisenum;
    UINT32 groupid;
    string alias;
    UINT32 alive;
}RecordCruiseInfo;

typedef struct tagRecordCruisepresetInfo{
    tagRecordCruisepresetInfo(){
        cruisenum = 0;
        slide = 0;
        ptz = 0;
        second = 0;
        alive = 0;
    }
    UINT32 cruisenum;
    string alias;
    UINT16 slide;
    UINT16 ptz;
    UINT32 second;
    UINT32 alive;
}RecordCruisepresetInfo;

struct AlarmConKey_S{
    AlarmConKey_S(){
        ui_group = 0;
        us_slide = 0;
        us_ptz = 0;
    }
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
};

struct AlarmConCmp_S
{
    bool operator()( const AlarmConKey_S &left, const AlarmConKey_S &right ) const
    {
        if( left.ui_group != right.ui_group ){
            return left.ui_group < right.ui_group;
        }
        if( left.us_slide != right.us_slide ){
            return left.us_slide < right.us_slide;
        }

        return left.us_ptz < right.us_ptz;

    }
};

struct AnalyseKey_S{
    AnalyseKey_S(){
        ui_group = 0;
        us_slide = 0;
        us_ptz = 0;
    }
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
};

struct AnalyseCmp_S
{
    bool operator()( const AnalyseKey_S &left, const AnalyseKey_S &right ) const
    {
        if( left.ui_group != right.ui_group ){
            return left.ui_group < right.ui_group;
        }
        if( left.us_slide != right.us_slide ){
            return left.us_slide < right.us_slide;
        }

        return left.us_ptz < right.us_ptz;

    }
};

struct AlarmReasonTimeKey_S{
    AlarmReasonTimeKey_S(){
        ui_group_no = 0;
        us_slide = 0;
        us_ptz = 0;
        ui_sec = 0;
        ui_msec = 0;
    }
    UINT32 ui_group_no;
    UINT16 us_slide;
    UINT16 us_ptz;
    UINT32 ui_sec;
    UINT32 ui_msec;    
};

struct AlarmReasonTimeCmp_S{
    bool operator()( const AlarmReasonTimeKey_S &left, const AlarmReasonTimeKey_S &right ) const
    {
        if( left.ui_group_no != right.ui_group_no ){
            return left.ui_group_no < right.ui_group_no;
        }

        if( left.us_slide != right.us_slide ){
            return left.us_slide < right.us_slide;
        }

        if( left.us_ptz != right.us_ptz ){
            return left.us_ptz < right.us_ptz;
        }

        if( left.ui_sec != right.ui_sec ){
            return left.ui_sec < right.ui_sec;
        }

        return left.ui_msec < right.ui_msec;
    }
};

struct AlarmReasonKey_S{
    AlarmReasonKey_S(){
        ui_sec = 0;
        ui_msec = 0;
    }
    UINT32      ui_sec;
    UINT32      ui_msec;
};

struct AlarmReasonCmp_S
{
    bool operator()( const AlarmReasonKey_S &left, const AlarmReasonKey_S &right ) const
    {
        if( left.ui_sec != right.ui_sec ){
            return left.ui_sec < right.ui_sec;
        }

        return left.ui_msec < right.ui_msec;
    }
};

struct RecordConKey_S{
    RecordConKey_S(){
        ui_group = 0;
        us_slide = 0;
        us_ptz = 0;
    }
    UINT32 ui_group;
    UINT16 us_slide;
    UINT16 us_ptz;
};

struct RecordConCmp_S
{
    bool operator()( const RecordConKey_S &left, const RecordConKey_S &right ) const
    {
        if( left.ui_group != right.ui_group ){
            return left.ui_group < right.ui_group;
        }
        if( left.us_slide != right.us_slide ){
            return left.us_slide < right.us_slide;
        }

        return left.us_ptz < right.us_ptz;

    }
};

typedef struct tagRecordCondition_S{
    tagRecordCondition_S(){
        ui_record_time_interval = 0;
        ft_max_temp_interval = 0.0f;
        ft_max_temp = 0.0f;
    }
    UINT32 ui_record_time_interval;
    float  ft_max_temp_interval;
    float  ft_max_temp;
}RecordCondition_S;

#define GROUP_INFRA_INTERFIX_FIELD_RADIATION 1 //辐射率
#define GROUP_INFRA_INTERFIX_FIELD_DISTANCE  2 //距离
#define GROUP_INFRA_INTERFIX_FIELD_ENVTEMP   3 //环境温度
#define GROUP_INFRA_INTERFIX_FIELD_HUMIDITY  4 //相对湿度
#endif // DATABASETYPEDEF_H
