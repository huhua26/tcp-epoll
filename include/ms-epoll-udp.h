#ifndef MS_EPOLL_UDP_H_
#define MS_EPOLL_UDP_H_

#include <netinet/in.h>

typedef unsigned int       HUDPLINK;
#define INVALID_HUDPLINK   0xffffffff

#define LINK_ADDR_LOCAL   0x0001
#define LINK_ADDR_REMOTE  0x0002


#define UDP_FLAG_NONE       0x0001
#define UDP_FLAG_BROADCAST  0x0002

/*
Udp Event
*/
#define EVT_UDP_CREATED			0x0001
#define EVT_UDP_CLOSED			0x0002
#define EVT_UDP_RECEIVEDATA		0x0003
#define EVT_UDP_EXCEPTION		0xffff

struct UdpEvent_S{
    unsigned int ui_event;
    HUDPLINK     h_link;
};

/*************************************************************
when event is EVT_UDP_RECEIVEDATA
*************************************************************/
struct UdpRecvDataParam_S{
    char *p_RecvData;           //recv data buf
    unsigned int ui_size;       //data buf size
    char *p_dst_ip;       //dst ip
    unsigned short us_dst_port; //dst port
};

/*************************************************************
Func    name:
Descriptions: call back function when IO Completed
Input   para: pParam1:
                UdpEvent_S*
              pParam2:
                UdpRecvDataParam_S*  or NULL
In&Out  para:
Output  para:
Return value:
*************************************************************/
typedef void( *ms_udp_io_callback )( void* p_param1, void* p_param2 );


/*************************************************************
Func    name: ms_udp_init
Descriptions: init func
Input   para:
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_udp_init();

/*************************************************************
Func    name: ms_udp_uninit
Descriptions: uninit func
Input   para:
In&Out  para:
Output  para:
Return value:
*************************************************************/
void ms_udp_uninit();

/*************************************************************
Func    name: ms_udp_create
Descriptions: create udp LINK, bind addr and port
Input   para: backFunc: when IO operator completed, call backFunc
              p_localAddr: local addr
              us_localPort: local port
In&Out  para:
Output  para:
Return value: HUDPLINK
*************************************************************/
HUDPLINK ms_udp_create( ms_udp_io_callback backFunc, int i_flag, const char* p_localAddr, unsigned short us_localPort );

/*************************************************************
Func    name: ms_udp_destroy
Descriptions: destroy udp link
Input   para: hLink
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_udp_destroy( HUDPLINK h_link );


/*************************************************************
Func    name: ms_udp_send
Descriptions: send data
Input   para:
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_udp_send_to( HUDPLINK h_link, const char* p_data, unsigned int ui_size, int i_flag, const char *p_ip, unsigned short us_port );




#endif // MS_EPOLL_UDP_H_
