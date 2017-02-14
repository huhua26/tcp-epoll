#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include "link-mgr.h"
#include "test-client-handle.h"

class TestServer : public LinkMgr< TestClientHandle, MsgHeader_S >
{
public:
    TestServer( std::string str_listen_ip, unsigned short us_listen_port, unsigned int ui_keep_alive );
    virtual ~TestServer();

public:
    virtual void link_destroy( const CRefCountPtr<TestClientHandle> &p_hdr );
    bool Init();
    void UnInit();
};


#endif
