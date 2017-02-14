
#ifndef __MD5_INCLUDED__
#define __MD5_INCLUDED__

#include "comm_typedef.h"
//MD5摘要值结构体
typedef struct MD5VAL_STRUCT
{
    UINT32 n_a;
    UINT32 n_b;
    UINT32 n_c;
    UINT32 n_d;
} MD5VAL;

/**********************************************************************************
Func    Name: md5
Descriptions: 字符串按照md5格式加密
Input   para: pc_abstract_str:摘要字符串
              size: 摘要字符串的长度
In&Out  Para:
Output  para:
Return value: 成功返回MD5VAL
***********************************************************************************/
//计算字符串的MD5值(若不指定长度则由函数计算)
MD5VAL md5( INT8 * pc_abstract_str, UINT32 size=0 );

/**********************************************************************************
Func    Name: conv
Descriptions: 字节序转换
Input   para: n_a:需要转换的字节序
In&Out  Para:
Output  para:
Return value: 成功返回转换后的字节序
***********************************************************************************/
UINT32 conv( UINT32 n_a );

/**********************************************************************************
Func    Name: get_md5
Descriptions: 获取字符串加密后的ascii和二进制数据
Input   para: pc_abstract_str:摘要字符串
              size: 摘要字符串的长度
In&Out  Para:
Output  para: pc_md5_ascii:ascii码
              puc_md5:二进制
Return value: 成功返回0, 失败返回其他值
***********************************************************************************/
INT32 get_md5( const INT8 * pc_abstract_str, UINT32 n_size, INT8 *pc_md5_ascii, UINT8 *puc_md5 );

#endif
