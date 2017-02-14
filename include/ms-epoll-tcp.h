#ifndef MS_EPOLLTCP_H
#define MS_EPOLLTCP_H

typedef unsigned int       HTCPLINK;
#define INVALID_HTCPLINK   0xffffffff

#define LINK_ADDR_LOCAL   0x0001
#define LINK_ADDR_REMOTE  0x0002


/*
Tcp Event
*/
#define EVT_TCP_CREATED			0x0001
#define EVT_TCP_CLOSED			0x0002
#define EVT_TCP_ACCEPTED		0x0003
#define EVT_TCP_CONNECTED		0x0004
#define EVT_TCP_DISCONNECTED	0x0005
#define EVT_TCP_RECEIVEDATA		0x0006
#define EVT_TCP_KEEPALIVE		0x0007
#define EVT_TCP_ERRPROTOCOL		0x0010
#define EVT_TCP_EXCEPTION		0xffff

struct TcpEvent_S{
    unsigned int ui_event;
    HTCPLINK     h_link;
};

/*************************************************************
when event is EVT_TCP_RECEIVEDATA
*************************************************************/
#define  STATUS_RECV_BEGIN   0x0001
#define  STATUS_RECV_MIDDLE  0x0002
#define  STATUS_RECV_END     0x0003
struct TcpRecvDataParam_S{
    unsigned int ui_status;   //recv data status
    char *p_RecvData;         //recv data buf
    unsigned int ui_size;     //data buf size
};

/*************************************************************
when event is EVT_TCP_ACCEPTED
*************************************************************/
struct TcpAcceptedParam_S{
    HTCPLINK  h_acceptLink;
};

/*************************************************************
Func    name:
Descriptions: call back function when IO Completed
Input   para: pParam1:
                TcpEvent_S*
              pParam2:
                TcpAcceptedParam_S*  or  TcpRecvDataParam_S*  by different event
In&Out  para:
Output  para:
Return value:
*************************************************************/
typedef void( *ms_tcp_io_callback )( void* p_param1, void* p_param2 );


/*************************************************************
Func    name: ms_tcp_init
Descriptions: init func
Input   para:
In&Out  para:
Output  para:
Return value:
*************************************************************/
bool ms_tcp_init( );

/*************************************************************
Func    name: ms_tcp_uninit
Descriptions: uninit func
Input   para:
In&Out  para:
Output  para:
Return value:
*************************************************************/
void ms_tcp_uninit();

/*************************************************************
Func    name: ms_tcp_create
Descriptions: create tcp link, bind addr and port
Input   para: backFunc: when IO operator completed, call backFunc
              p_localAddr: local addr
              us_localPort: local port
In&Out  para:
Output  para:
Return value: HTCPLINK
*************************************************************/
HTCPLINK ms_tcp_create( ms_tcp_io_callback backFunc, const char* p_localAddr, unsigned short us_localPort );

/*************************************************************
Func    name: ms_tcp_destroy
Descriptions: destroy tcp link
Input   para: hLink
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_tcp_destroy( HTCPLINK h_link );

/*************************************************************
Func    name: ms_tcp_connect
Descriptions: call connect function by hLink, pAddr and nPort
Input   para:
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_tcp_connect( HTCPLINK h_link, const char* p_addr, unsigned short us_port );

/*************************************************************
Func    name: ms_tcp_listen
Descriptions: call listen function by hLink
Input   para: nBlock: listen param
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_tcp_listen( HTCPLINK h_link, unsigned int ui_block );


/*************************************************************
Func    name: ms_tcp_send
Descriptions: send data
Input   para:
In&Out  para:
Output  para: i_error:errno
Return value: send data len
*************************************************************/
unsigned int ms_tcp_send( HTCPLINK h_link, const unsigned char* p_data, unsigned int ui_size, int *i_error );


/*************************************************************
Func    name: ms_tcp_get_link_addr
Descriptions: get link addr
Input   para: h_link, ui_type
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
bool ms_tcp_get_link_addr( HTCPLINK h_link, unsigned int ui_type, unsigned int *pui_ip, unsigned short *pus_port );



/*************************************************************
Func    name: ms_tcp_set_log_callback_func
Descriptions: set ms_tcp_log_callback_func
Input   para: ms_tcp_log_callback_func
In&Out  para:
Output  para:
Return value: success true, failed false.
*************************************************************/
typedef void (*ms_tcp_log_callback_func)( int i_level, const char *pc_message );
bool  ms_tcp_set_log_callback_func( ms_tcp_log_callback_func func );

#endif // MS_EPOLLTCP_H
