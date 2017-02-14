#include "udp-client-test.h"
#include "ms-epoll-udp.h"

int main(int argc, char *argv[])
{
    ms_udp_init();
    UdpClient st_udp_client;
    if( !st_udp_client.init() ){
        return -1;
    }

    while( true ){
        sleep( 1 );
    }

    ms_udp_uninit();
    return 0;
	
//	ms_udp_init();
//    UdpClient st_udp_client;
//    if( !st_udp_client.init() ){
//        return -1;
//    }

//    std::string str_ip = "192.168.8.200";
//    st_udp_client.do_ver_nego_req( str_ip, 8889 );

//    while( true ){
//        sleep( 1 );
//    }

//    ms_udp_uninit();
//    return 0;
}
