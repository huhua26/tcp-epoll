#ifndef LINK_MGR_H_
#define LINK_MGR_H_

#include <list>
#include <map>
#include "net-handler.h"


template< typename HANDLE, typename PACKET >
class LinkMgr : public TcpNetHandler< TcpNetProtocolEvent<PACKET> >
{
public:
    typedef std::map<HTCPLINK, CRefCountPtr<HANDLE> > LINKMAP;
    LinkMgr( std::string str_listen_ip,
              unsigned short us_listen_port,
              unsigned int ui_keep_alive )
            : mstr_listen_ip( str_listen_ip ),
              ms_listen_port( us_listen_port ),
              mui_keep_alive( ui_keep_alive )
    {
        pthread_mutex_init( &mo_link_map_lock, NULL );
    }

    virtual ~LinkMgr()
    {
        pthread_mutex_destroy( &mo_link_map_lock );
    }

public:
    void broad_cast( PACKET *p_pkt )
    {
        std::vector<HTCPLINK> link_vec;
        do{
            CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
            if( m_link_map.empty() ){
                return;
            }
            typename LINKMAP::iterator it = m_link_map.begin();
            for( ; it != m_link_map.end(); ++it ){
                if( it->second->can_broadcast() ){
                    link_vec.push_back( it->first );
                }
            }
        } while( 0 );

        send_pkt_to_link( p_pkt, link_vec );
    }

    void broad_cast_except( PACKET *p_pkt, HTCPLINK h_link )
    {
        std::vector<HTCPLINK> link_vec;
        do{
            CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
            if( m_link_map.empty() ){
                return;
            }
            typename LINKMAP::iterator it = m_link_map.begin();
            for( ; it != m_link_map.end(); ++it ){
                if( it->second->can_broadcast() && (it->first != h_link) ){
                    link_vec.push_back( it->first );
                }
            }
        } while( 0 );

        send_pkt_to_link( p_pkt, link_vec );
    }

    void get_all_hdr( std::list<CRefCountPtr<HANDLE> > &hdr_list ) const
    {
        CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
        typename LINKMAP::iterator it = m_link_map.begin();
        for( ; it != m_link_map.end(); ++it ){
            hdr_list.push_back( it->second );
        }
    }

    unsigned int get_link_cnt()
    {
        CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
        return m_link_map.size();
    }

    bool find_hdr( const HTCPLINK h_link, CRefCountPtr<HANDLE> &hdr_ptr )
    {
        CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
        typename LINKMAP::iterator it = m_link_map.find( h_link );
        if( it == m_link_map.end() ){
            MS_LOGER_INFO( "can not find link=%d", h_link );
            return false;
        }

        hdr_ptr = it->second;
        return true;
    }

    bool find_hdr_by_name( const std::string &str_name, std::list<CRefCountPtr<HANDLE> > &hdr_list )
    {
        CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
        typename LINKMAP::iterator it = m_link_map.begin();
        for( ; it != m_link_map.end(); ++it ){
            if( it->second->get_link_name() == str_name ){
                hdr_list.push_back( it->second );
            }
        }
        return true;
    }

    bool send_pkt_to_link( PACKET *p_pkt, HTCPLINK h_link )
    {
        if( p_pkt == NULL ){
            return false;
        }

        std::vector<char> vc_buffer;
        switch( p_pkt->msg_type ){
#define CASE_PACKET( n, T_resp )\
        case n: {\
            write_packet(vc_buffer, *(T_resp*)p_pkt);\
            ::MS_TcpSend( h_link, (INT8 *)&vc_buffer[0], vc_buffer.size() );\
            return 0;\
            }
#undef CASE_PACKET
        default:
            break;
        }

        return true;
    }

    bool send_pkt_to_link( PACKET *p_pkt, const std::vector<HTCPLINK> &link_vec )
    {
        if( p_pkt == NULL ){
            return false;
        }

        std::vector<char> vc_buffer;
        switch( p_pkt->msg_type ){
#define CASE_PACKET( n, T_resp )\
        case n: {\
            write_packet(vc_buffer, *(T_resp*)p_pkt);\
            break;\
            }
#undef CASE_PACKET
        default:
            break;
        }

        std::vector<HTCPLINK>::const_iterator it = link_vec.begin();
        for( ; it != link_vec.end(); ++it ){
            ::ms_tcp_send( *it, (INT8 *)&vc_buffer[0], vc_buffer.size() );
        }

//        switch( p_pkt->msg_type ){
//#define CASE_PACKET_DEL( T_resp )\
//        case n: {\
//            delete (T_resp*)p_pkt;\
//            break;\
//            }
//#undef CASE_PACKET_DEL
//        default:
//            break;
//        }

        return true;
    }

    virtual bool Init()
    {
        MS_LOGER_INFO( "LinkMgr Setup, Init." );
        mh_tcp_link = TcpNetHandler< TcpNetProtocolEvent<PACKET> >::tcp_create( mstr_listen_ip.c_str(), ms_listen_port );
        if( INVALID_HTCPLINK == mh_tcp_link ){
            MS_LOGER_FATAL( "Tcp Create failed. ip:port=%s:%d", mstr_listen_ip.c_str(), ms_listen_port );
            return false;
        }

        bool b_listen = ::ms_tcp_listen( mh_tcp_link, 5 );
        if( !b_listen ){
            MS_LOGER_FATAL( "MS_TcpListen failed.ip:port=%s:%d", mstr_listen_ip.c_str(), ms_listen_port );
            return false;
        }

        MS_LOGER_INFO( "create tcplink=0x%04x, ip:port=%s:%d", mh_tcp_link, mstr_listen_ip.c_str(), ms_listen_port );

        if( mui_keep_alive ){
            //register keep alive timer.
        }

//        unsigned int ui_ip = 0;
//        unsigned short us_port = 0;
//        if( ::MS_tcp_get_link_addr(  mh_tcp_link, LINK_ADDR_LOCAL, &ui_ip, &us_port ) ){
//            ms_listen_port = us_port;
//        }

        return true;
    }

    virtual void UnInit()
    {
        MS_LOGER_INFO( "LinkMgr Shutdown, UnInit." );
        link_remove_all();
        TcpNetHandler< TcpNetProtocolEvent<PACKET> >::tcp_destroy( mh_tcp_link );
    }

    HTCPLINK get_link(){ return mh_tcp_link; }

    virtual void on_tcp_created( HTCPLINK h_link )
    {
        unsigned int ui_ip = 0;
        unsigned short us_port = 0;
        if( TcpNetHandler< TcpNetProtocolEvent<PACKET> >::tcp_get_link_addr( h_link, LINK_ADDR_LOCAL, &ui_ip, &us_port ) ){
            std::string str_ip = ConvertIP( ui_ip );
            MS_LOGER_INFO( "tcp_created link=0x%04x, local Ip:Port=%s:%d", h_link, str_ip.c_str(), us_port );
        }
    }

    virtual void on_tcp_closed( HTCPLINK h_link )
    {
        MS_LOGER_INFO( "TcpClosed link=0x%04x", h_link );
        if( h_link == mh_tcp_link ){
            MS_LOGER_INFO( "listen link closed." );
            //::exit( 1 );
        } else {
            link_remove( h_link );
        }
    }

    virtual void on_tcp_accepted( HTCPLINK h_link, HTCPLINK h_accept_link )
    {
        link_create( h_accept_link );
    }

    virtual void on_tcp_connected( HTCPLINK h_link )
    {
        MS_LOGER_INFO( "tcp connected, link=0x%04x", h_link );
    }

    virtual void on_tcp_disconnected( HTCPLINK h_link )
    {
        MS_LOGER_INFO( "tcp disconnected, link=0x%04x", h_link );
        if( h_link == mh_tcp_link ){
            MS_LOGER_INFO( "listen link closed." );
            //::exit( 1 );
        } else {
            link_remove( h_link );
        }
    }

    virtual void on_tcp_recv_data( HTCPLINK h_link, const CRefCountPtr<PACKET>& p_pkt )
    {
        link_recv_data( h_link, p_pkt );
    }

    virtual void link_destroy( const CRefCountPtr<HANDLE> &p_hdr ){}

private:
    bool link_create( HTCPLINK h_link )
    {
        CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
        if( m_link_map.find( h_link ) != m_link_map.end() ){
            MS_LOGER_ERROR( "link_create failed.(link=0x%04x duplicated).", h_link );
            return false;
        }

        HANDLE *p_hdr = new HANDLE();
        if( NULL == p_hdr ){
            MS_LOGER_ERROR( "link_create failed.(new handle failed)." );
            return false;
        }
        p_hdr->set_link( h_link );

        unsigned int ui_ip = 0;
        unsigned short us_port = 0;
        if( TcpNetHandler< TcpNetProtocolEvent<PACKET> >::tcp_get_link_addr( h_link, LINK_ADDR_REMOTE, &ui_ip, &us_port ) ){
            std::string str_ip = ConvertIP( ui_ip );
            MS_LOGER_INFO( "accepted acceptlink=0x%04x, Remote Ip:Port=%s:%d",
                           h_link, str_ip.c_str(), us_port );
            p_hdr->set_remote_ip( str_ip );
        }

        m_link_map[h_link] = CRefCountPtr<HANDLE>( p_hdr );

        return true;
    }

    bool link_remove( HTCPLINK h_link )
    {
        CRefCountPtr<HANDLE> pcref_hdr;
        do{
            CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
            typename LINKMAP::iterator it = m_link_map.find( h_link );
            if( it == m_link_map.end() ){
                MS_LOGER_ERROR( "link_remove failed.(link=0x%04x not exists).", h_link );
                return false;
            }
            pcref_hdr = it->second;
            m_link_map.erase( it );
        } while( 0 );

        if( pcref_hdr.Obj() ){
            link_destroy( pcref_hdr );
        }
        return true;
    }

    void link_remove_all()
    {
        MS_LOGER_INFO( "link_remove_all" );
        std::vector<HTCPLINK> hlinkvec;
        do{
            CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
            typename LINKMAP::iterator it = m_link_map.begin();
            for( ; it != m_link_map.end(); ++it ){
                hlinkvec.push_back( it->first );
            }
        } while( 0 );

        std::vector<HTCPLINK>::iterator iter = hlinkvec.begin();
        for( ; iter != hlinkvec.end(); ++iter ){
            TcpNetHandler< TcpNetProtocolEvent<PACKET> >::tcp_destroy( *iter );
        }
    }

    bool link_recv_data( HTCPLINK h_link, const CRefCountPtr<PACKET> &p_pkt )
    {
        CRefCountPtr<HANDLE> p_hdr;
        bool b_find = false;
        do{
            CGuardLock<pthread_mutex_t> guard( &mo_link_map_lock );
            typename LINKMAP::iterator it = m_link_map.find( h_link );
            if( it == m_link_map.end() ){
                MS_LOGER_ERROR( "link_recv_data failed.(link=0x%04x not exists).", h_link );
                return false;
            } else {
                p_hdr = it->second;
                b_find = true;
            }
        } while( 0 );

        if( b_find ){
            //p_hdr->setkeepalivetick();
            p_hdr->recv_data( p_pkt );
        }
        return true;
    }

private:
    pthread_mutex_t  mo_link_map_lock;
    LINKMAP          m_link_map;

    HTCPLINK         mh_tcp_link;
    std::string      mstr_listen_ip;
    unsigned short   ms_listen_port;
    unsigned int     mui_keep_alive;
};

#endif
