#include "test-server.h"

TestServer::TestServer( std::string str_listen_ip, unsigned short us_listen_port, unsigned int ui_keep_alive )
    : LinkMgr( str_listen_ip, us_listen_port, ui_keep_alive )
{

}

TestServer::~TestServer()
{

}

void TestServer::link_destroy( const CRefCountPtr<TestClientHandle> &p_hdr )
{

}

bool TestServer::Init()
{
    return LinkMgr<TestClientHandle, MsgHeader_S>::Init();
}

void TestServer::UnInit()
{
    LinkMgr<TestClientHandle, MsgHeader_S>::UnInit();
}
