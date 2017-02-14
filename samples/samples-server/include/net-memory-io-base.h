#ifndef _MS_PACKET_MEMORY_IO_BASE_H_
#define _MS_PACKET_MEMORY_IO_BASE_H_

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string.h>
#include "ms_endian.h"


inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, UINT8 *puc_output_buffer, INT32 i_output_size )
{
    if ( ( i_output_size >= 0 ) && (( i_size - i_offset ) >= i_output_size) )
    {
        memcpy( puc_output_buffer, pc_buffer + i_offset, i_output_size );
        i_offset += i_output_size;
        return true;
    }
    else
    {
        return false;
    }
}

inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, INT8 *pc_output_buffer, INT32 i_output_size )
{
    if ( ( i_output_size >= 0 ) && (( i_size - i_offset ) >= i_output_size))
    {
        memcpy(pc_output_buffer, pc_buffer + i_offset, i_output_size);
        i_offset += i_output_size;
        return true;
    }
    else
    {
        return false;
    }
}

inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::string &s_output, INT32 i_output_size )
{
    if ( ( i_output_size >= 0 ) && ( ( i_size - i_offset ) >= i_output_size) )
    {
        s_output.assign(pc_buffer + i_offset, i_output_size);
        i_offset += i_output_size;
        return true;
    }
    else
    {
        return false;
    }
}

inline bool parse_req( const std::vector<char>& vc_buffer, INT32 &i_offset, INT8 *pc_output_buffer, INT32 i_output_size )
{
    if ( ( i_output_size >= 0 ) && ( ( (INT32)vc_buffer.size() - i_offset ) >= i_output_size ) )
    {
        memcpy(pc_output_buffer, &vc_buffer[i_offset], i_output_size);
        i_offset += i_output_size;
        return true;
    }
    else
    {
        return false;
    }
}

inline bool parse_req( const std::vector<char>& vc_buffer, INT32 &i_offset, std::string &s_output, INT32 i_output_size )
{
    if ( ( i_output_size >= 0 ) && ( ( (INT32)vc_buffer.size() - i_offset ) >= i_output_size ))
    {
        s_output.assign(&vc_buffer[i_offset], i_output_size);
        i_offset += i_output_size;
        return true;
    }
    else
    {
        return false;
    }
}

template<typename T>
bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, T& value )
{
    if (i_size - i_offset >= sizeof(T))
    {
        memcpy(&value, pc_buffer + i_offset, sizeof(T));
        value = ms_endian(value);
        i_offset += sizeof(T);
        return true;
    }
    else
    {
        return false;
    }
}

//ui_max_num:数组最大条数
//ui_num:数组的有效条数
template<typename T>
bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, T *p_value, const UINT32 ui_max_num, const UINT32 ui_num)
{
    if( ui_num > ui_max_num ) {      //保护
        return false;
    }
    for( UINT32 ui_t = 0; ui_t < ui_num; ui_t++ ) {
        if( !parse_req( pc_buffer, i_size, i_offset, *(p_value + ui_t) ) ) {
            return false;
        }
    }
    return true;
}

inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::string &s_output, INT32 &i_output_size )
{
    if( parse_req( pc_buffer, i_size, i_offset, i_output_size ) ) {
        if ( ( i_output_size >= 0 ) && ( ( i_size - i_offset ) >= i_output_size ))
        {
            s_output.assign(pc_buffer + i_offset, i_output_size);
            i_offset += i_output_size;
            return true;
        }
        else
        {
            return false;
        }
    }
}

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::string& s_value )
{
    uint32_t ui_value_size = 0;
    if (parse_req(pc_buffer, i_size, i_offset, ui_value_size))
    {
        return parse_req( pc_buffer, i_size, i_offset, s_value, (UINT32)ui_value_size );
    }
    else
    {
        return false;
    }
}

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::map<UINT32, std::string> &map_packet )
{
    UINT32 ui_ana_count = 0;
    UINT32 ui_ana_no = 0;
    std::string s_data = "";
    map_packet.clear();     //清空
    parse_req( pc_buffer, i_size, i_offset, ui_ana_count );
    for( UINT32 i = 0; i < ui_ana_count; i++ ){
        parse_req( pc_buffer, i_size, i_offset, ui_ana_no );
        parse_req( pc_buffer, i_size, i_offset, s_data );
        map_packet[ ui_ana_no ] = s_data;
    }
    return true;
}

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::map<UINT32, UINT64> &map_packet )
{
    UINT32 ui_ana_count = 0;
    UINT32 ui_ana_no = 0;
    UINT64 ul_data = 0;
    map_packet.clear();     //清空
    parse_req( pc_buffer, i_size, i_offset, ui_ana_count );
    for( UINT32 i = 0; i < ui_ana_count; i++ ){
        parse_req( pc_buffer, i_size, i_offset, ui_ana_no );
        parse_req( pc_buffer, i_size, i_offset, ul_data );
        map_packet[ ui_ana_no ] = ul_data;
    }
    return true;
}

template<>
inline bool parse_req( const INT8 *pc_buffer, INT32 i_size, INT32 &i_offset, std::set<UINT32> &set_packet )
{
    UINT32 ui_ana_count = 0;
    UINT32 ui_ana_no = 0;
    set_packet.clear();     //清空
    parse_req( pc_buffer, i_size, i_offset, ui_ana_count );
    for( UINT32 i = 0; i < ui_ana_count; i++ ){
        parse_req( pc_buffer, i_size, i_offset, ui_ana_no );
        set_packet.insert( ui_ana_no );
    }
    return true;
}


template<typename T>
bool parse_req(const std::vector<char>& vc_buffer, INT32& i_offset, T& value)
{
    if (vc_buffer.size() - i_offset >= sizeof(T))
    {
        memcpy(&value, &vc_buffer[i_offset], sizeof(T));
        i_offset += sizeof(T);
        return true;
    }
    else
    {
        return false;
    }
}

template<>
inline bool parse_req(const std::vector<char>& vc_buffer, INT32& i_offset, std::string& s_value)
{
    if (vc_buffer.size() - i_offset > 0)
    {
        s_value.assign(&vc_buffer[i_offset], vc_buffer.size() - i_offset);
        return true;
    }
    else
    {
        return false;
    }
}

inline bool pack_resp( std::vector<char>& vc_buffer, INT32 &i_offset, const INT8 *pc_data, UINT32 ui_data_size )
{
    UINT32 ui_buffer_size = vc_buffer.size();
    if (ui_buffer_size >= i_offset + ui_data_size && ui_data_size >= 0)
    {
        memcpy(&vc_buffer[i_offset], pc_data, ui_data_size);
        return true;
    }
    else
    {
        return false;
    }
}

template<typename T>
inline bool pack_resp(std::vector<char>& vc_buffer, INT32 i_offset, const T& value )
{
    return pack_resp(vc_buffer, i_offset, (const char*)&value, sizeof(T));
}

inline bool ms_append( std::vector<char>& vc_buffer, const INT8 *pc_data, UINT32 ui_data_size )
{
    if (ui_data_size > 0)
    {
        INT32 i_old_size = vc_buffer.size();
        vc_buffer.resize(i_old_size + ui_data_size);
        memcpy(&vc_buffer[i_old_size], pc_data, ui_data_size);
    }

    return true;
}

template<typename T>
inline bool ms_append(std::vector<char>& vc_buffer, const T& value)
{
    return ms_append(vc_buffer, (const char*)&value, sizeof(T));
}

template<typename T>
bool pack_resp( std::vector<char>& vc_buffer, const T& value )
{
    ms_append( vc_buffer, ms_endian(value) );
    return true;
}

//使string类型数据可不包含长度
inline bool pack_resp( std::vector<char>& vc_buffer, const std::string& s_value, UINT32 ui_data_size )
{
    if( ui_data_size > s_value.size() ) {   //保护
        return false;
    }
    ms_append( vc_buffer, s_value.data(), ui_data_size );
    return true;
}

template<>
inline bool pack_resp( std::vector<char>& vc_buffer, const std::string& s_value )
{
    pack_resp( vc_buffer, (UINT32)s_value.size() );
    ms_append( vc_buffer, s_value.data(), s_value.size() );
    return true;
}

inline bool pack_resp( std::vector<char>& vc_buffer, const INT8* pc_data, UINT32 ui_data_size )
{
    ms_append( vc_buffer, pc_data, ui_data_size );
    return true;
}

inline bool pack_resp( std::vector<char>& vc_buffer, const UINT8* puc_data, UINT32 ui_data_size )
{
    pack_resp( vc_buffer, (INT8*)puc_data, ui_data_size );
    return true;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const std::map<UINT32, UINT64> &map_packet )
{
    bool b_ret = false;
    UINT32 ui_count = map_packet.size();   //个数，必须先赋值
    b_ret = pack_resp( vc_buffer, ui_count );
    std::map<UINT32, UINT64>::const_iterator it_premission = map_packet.begin();
    for( ; it_premission != map_packet.end(); ++it_premission ) {
        b_ret &= pack_resp( vc_buffer, it_premission->first );
        b_ret &= pack_resp( vc_buffer, it_premission->second );
    }
    return b_ret;
}

template<>
inline bool pack_resp( std::vector<char> &vc_buffer, const std::set<UINT32> &set_packet )
{
    bool b_ret = false;
    UINT32 ui_count = set_packet.size();   //个数，必须先赋值
    b_ret = pack_resp( vc_buffer, ui_count );
    std::set<UINT32>::const_iterator iter = set_packet.begin();
    for( ; iter != set_packet.end(); ++iter ) {
        b_ret &= pack_resp( vc_buffer, *iter );
    }
    return b_ret;
}

//结构体数组模版
template<typename T>
bool pack_resp( std::vector<char>& vc_buffer, const T *p_value, UINT32 ui_num )
{
    for( UINT32 ui_i = 0;ui_i < ui_num; ui_i++ ) {
           if( !pack_resp( vc_buffer, *p_value ) ) {
               return false;
           }
           ++p_value;
    }
    return true;
}

#endif
