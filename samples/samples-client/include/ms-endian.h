#ifndef _MS_ENDIAN_H_
#define _MS_ENDIAN_H_

#include "comm-typedef.h"

template<typename T>
T ms_endian( const T& value );

template<>
inline float ms_endian( const float & n )
{
    return n;
}

template<>
inline UINT8 ms_endian( const UINT8& n )
{
    return n;
}

template<>
inline UINT16 ms_endian( const UINT16& n )
{
    return (
                ((n & 0xFF) << 8) |
                ((n & 0xFF00) >> 8)
                );
}

template<>
inline UINT32 ms_endian( const UINT32& n )
{
    return (
                ((n & 0xFF) << 24) |
                ((n & 0xFF00) << 8) |
                ((n & 0xFF0000) >> 8) |
                ((n & 0xFF000000) >> 24)
                );
}

template<>
inline UINT64 ms_endian( const UINT64& n )
{
    return (
                ((n & 0xFF) << 56) |
                ((n & 0xFF00) << 40) |
                ((n & 0xFF0000) << 24) |
                ((n & 0xFF000000) << 8) |
                ((n & 0xFF00000000) >> 8) |
                ((n & 0xFF0000000000) >> 24) |
                ((n & 0xFF000000000000) >> 40) |
                ((n & 0xFF00000000000000) >> 56)
                );
}

template<>
inline INT8 ms_endian( const INT8& n )
{
    return n;
}

template<>
inline INT16 ms_endian( const INT16& n )
{
    return ms_endian((UINT16)n);
}

template<>
inline INT32 ms_endian( const INT32& n )
{
    return ms_endian((UINT32)n);
}

template<>
inline INT64 ms_endian( const INT64& n )
{
    return ms_endian((UINT64)n);
}

#endif
