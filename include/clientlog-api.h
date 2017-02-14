#ifndef CLIENTLOGDLL_H
#define CLIENTLOGDLL_H

#ifdef _WIN32
//#define WIN32
#endif

#include <vector>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <string.h>

#ifdef WIN32
#include <direct.h>
#include <share.h>
#endif

using namespace std;

#define LOG_CMD_FLAG "[CMD]"
#define LOG_VOD_FLAG "[VOD]"
#define LOG_DEVICE_FLAG "[DEV]"
#define LOG_DATABASE_FLAG "[DB]"
#define LOG_REPLAY_FLAG "[REP]"
#define LOG_ANALYSE_FLAG "[ANA]"

//常规日志
enum
{
    LOG_LEVEL_INFO       = 1,     //1
    LOG_LEVEL_WARNNING   = 1<<1,  //2
    LOG_LEVEL_ERROR      = 1<<2,  //4
    LOG_LEVEL_DEBUG      = 1<<3,  //8
    LOG_LEVEL_FATAL      = 1<<4,  //16


    LOG_LEVEL_MAX        = LOG_LEVEL_INFO | LOG_LEVEL_WARNNING | LOG_LEVEL_ERROR | LOG_LEVEL_DEBUG | LOG_LEVEL_FATAL,

};

//日志打印到本地或者远程
enum
{
    LOG_Local             = 1<<0,  //1
    LOG_Remote            = 1<<1,  //2
};

typedef struct tagSYSTEM_TIME_S
{
    tagSYSTEM_TIME_S()
    {
        year    = 0;
        month   = 0;
        day     = 0;
        hour    = 0;
        minute  = 0;
        second  = 0;
        millisecond = 0;
    }

    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
}SYSTEM_TIME_S;
typedef void (*FLogCallback)( const char* message, SYSTEM_TIME_S system_time, void* user_data );
void ms_log_set_callback( FLogCallback f_log_callback, void* user_data );
void ms_log_clean_callback( void* user_data );

/// @brief 初始化
/// @param[in] path 设置日志路径，必须是路经
/// @param[in] ui_remote_flag 远程或本地类型
bool ms_log_init( const char* path,  unsigned int ui_remote_flag = LOG_Local, const std::string& str_ip = "127.0.0.1", unsigned short ui_port = 8890 );

/// @brief 设置日志路径，必须是路经
/// @param[in] path
void ms_log_set_directory( const char* path );

/// @param[in] ui_remote_flag 远程或本地类型
void ms_log_set_remote_flag( unsigned int ui_remote_flag, const std::string& str_ip = "127.0.0.1", unsigned short ui_port = 8890);

/// @brief 设置日志级别
/// @param[in] level 日志级别
void ms_log_set_level( unsigned int level );

/// @brief 设置操作日志级别
/// @param[in] level 日志操作级别
void ms_log_set_opertor_level( unsigned int opertor_level );

//设置文件大小
void ms_log_set_log_file_size(unsigned int size);

/// @brief 设置日志存储类型
/// @param[in] storage_type
void ms_log_set_storage_type( unsigned int storage_type );
/// @brief
/// @param[in] field_type
void ms_log_set_field_type( unsigned int field_type );

/// @brief 设置日志保留天数
/// @param[in] days 日志保存天数
/// @note 日志保留天数，默认为-1天
void ms_log_set_reserve_days( int days );

void ms_log_uninit();

/// @brief
/// @param[in] level 日志级别
/// @param[in] filename 文件名
/// @param[in] file_name_suffex 文件名后缀
/// @param[in] line 行
/// @param[in] function 函数
/// @param[in] format 格式

void ms_log_print( unsigned int level, const char *file_name_suffex, const char*  filename,
            unsigned int line, const char *function, const char*  format, ... );


#define LOGER_PRINT(level, file_name_suffix, ... ) \
    ms_log_print( level, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define MS_LOGER_INFO( ... ) \
    ms_log_print( LOG_LEVEL_INFO, NULL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define MS_LOGER_INFO_EX( file_name_suffix, ... ) \
    ms_log_print( LOG_LEVEL_INFO, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define MS_LOGER_WARNING( ... ) \
    ms_log_print( LOG_LEVEL_WARNNING, NULL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define MS_LOGER_WARNING_EX( file_name_suffix, ...) \
    ms_log_print( LOG_LEVEL_WARNNING, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define MS_LOGER_ERROR( ... ) \
    ms_log_print( LOG_LEVEL_ERROR, NULL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define MS_LOGER_ERROR_EX( file_name_suffix, ... ) \
    ms_log_print( LOG_LEVEL_ERROR, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define MS_LOGER_DEBUG( ... ) \
    ms_log_print( LOG_LEVEL_DEBUG, NULL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define MS_LOGER_DEBUG_EX( file_name_suffix, ... ) \
    ms_log_print( LOG_LEVEL_DEBUG, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )

#define MS_LOGER_FATAL(...) \
    ms_log_print( LOG_LEVEL_FATAL, NULL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )
#define MS_LOGER_FATAL_EX( file_name_suffix, ... ) \
    ms_log_print( LOG_LEVEL_FATAL, file_name_suffix, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ )


#endif // CLIENTLOGDLL_H
