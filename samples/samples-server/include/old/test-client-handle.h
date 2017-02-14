#ifndef TEST_CLIENT_HANDLE_H_
#define TEST_CLIENT_HANDLE_H_

#include "comm_typedef.h"
#include "link-hdr.h"
#include "Guard.h"
#include "net-packet.h"

class TestClientHandle : public LinkHdr<MsgHeader_S>
{
public:
    TestClientHandle();
    ~TestClientHandle();

public:
    virtual bool recv_data( MsgHeader_S *p_pkt );

private:
    bool on_ver_nego_req( MSVerReq_S *p_ver_pkt );
    bool on_link_auth_req( MSAuthReq_S *p_auth_pkt  );
};

#endif
