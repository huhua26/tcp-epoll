#ifndef LINK_MGR_H_
#define LINK_MGR_H_

#include <list>
#include <map>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include "net-handler.h"

#include <stdio.h>


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
              mui_keep_alive( ui_keep_alive ),
              mb_thread_break( false ),
              mo_thread_id( 0 )
    {        
        pthread_mutex_init( &mo_link_map_lock, NULL );
        pthread_mutex_init( &mo_recv_buf_old_lock, NULL );
        sem_init( &mo_buff_sem, 0, 0 );
    }

    virtual ~LinkMgr()
    {
        pthread_mutex_destroy( &mo_link_map_lock );
        pthread_mutex_destroy( &mo_recv_buf_old_lock );
        sem_destroy( &mo_buff_sem );
    }

public:
    static void *handle_recv_buff( void *arg )
    {
        if( NULL == arg ){
            return NULL;
        }
        LinkMgr *p_this = (LinkMgr *)arg;
        while( !p_this->mb_thread_break ){
            struct timespec st_ts;
            st_ts.tv_sec = time( NULL ) + 1;
            st_ts.tv_nsec = 0;

            if( sem_timedwait( &p_this->mo_buff_sem, &st_ts ) != 0 ){
                continue;
            }

            std::map<HTCPLINK, std::vector<char> > recv_buf_map_old;
            recv_buf_map_old.clear();
            pthread_mutex_lock( &p_this->mo_recv_buf_old_lock );
            recv_buf_map_old.swap( p_this->m_recv_buf_old_map );
            pthread_mutex_unlock( &p_this->mo_recv_buf_old_lock );

            if( recv_buf_map_old.empty() ){
                continue;
            }

            std::map<HTCPLINK, std::vector<char> >::iterator it_link = recv_buf_map_old.begin();
            for( ; it_link != recv_buf_map_old.end(); ++it_link ){
                //memcpy to m_recv_buf_map
                HTCPLINK h_link = it_link->first;
                std::vector<char>::size_type vs_old_size = it_link->second.size();

                std::map<HTCPLINK, std::vector<char> >::iterator it_find = p_this->m_recv_buf_map.find( h_link );
                if( it_find != p_this->m_recv_buf_map.end() ){
                    std::vector<char>::size_type vs_recv_buf_size = it_find->second.size();
                    it_find->second.resize( vs_recv_buf_size + vs_old_size );
                    memcpy( it_find->second.data() + vs_recv_buf_size, it_link->second.data(), vs_old_size );
                } else {
                    std::vector<char> vec_temp;
                    vec_temp.clear();
                    vec_temp.resize( vs_old_size );
                    memcpy( vec_temp.data(), it_link->second.data(), vs_old_size );
                    p_this->m_recv_buf_map[h_link] = vec_temp;
                }
                //memcpy end

                unsigned int ui_msg_head = htonl( MSG_HEAD_MAGIC );
                std::map<HTCPLINK, std::vector<char> >::iterator it_find_sec = p_this->m_recv_buf_map.find( h_link );
                if( it_find_sec == p_this->m_recv_buf_map.end() ){
                    //error
                    MS_LOGER_ERROR( "no data, error." );
                    continue;
                }
                std::vector<char> &recv_buf = it_find_sec->second;
                std::vector<char>::size_type vs_recv_buf_size = recv_buf.size();

                unsigned int ui_data_offset = 0;
                while( ( vs_recv_buf_size - ui_data_offset ) > 0 ){
                    if( memcmp( recv_buf.data() + ui_data_offset, &ui_msg_head, sizeof(ui_msg_head) ) != 0 ){
                        ui_data_offset++;
                        continue;
                    }

                    unsigned int ui_total_len = parse_data_total_len<MsgHeader_S>( recv_buf.data() + ui_data_offset );
                    if( ui_total_len + ui_data_offset > recv_buf.size() ){
                        break;
                    }

                    MsgHeader_S *p_msg = NULL;
                    int i_parse_len = 0;
                    i_parse_len = parse_data( recv_buf.data() + ui_data_offset, vs_recv_buf_size - ui_data_offset, p_msg );
                    if( i_parse_len > 0 ){
                        ui_data_offset += i_parse_len;
                        //CRefCountPtr<MsgHeader_S> pcref_packet = CRefCountPtr<MsgHeader_S>( p_msg );
                        p_this->on_tcp_recv_data( h_link, p_msg );
                    } else if( i_parse_len == 0 ){
                        break;
                    } else {
                        ui_data_offset++;
                        if( i_parse_len == CTRL_PRO_NOT_EXIST_ERROR ){
                            p_this->on_tcp_recv_unknown_packet( h_link, recv_buf.data() + ui_data_offset, vs_recv_buf_size - ui_data_offset );
                        }
                    }
                }

                if( ui_data_offset > 0 ){
                    std::vector<char> vectemp;
                    unsigned int ui_temp_len = vs_recv_buf_size - ui_data_offset;
                    vectemp.resize( ui_temp_len );
                    memcpy( vectemp.data(), recv_buf.data() + ui_data_offset, ui_temp_len );
                    recv_buf.clear();
                    recv_buf = vectemp;

//                    it_find->second.clear();
//                    it_find->second.resize( ui_temp_len );
//                    memcpy( it_find->second.data(), recv_buf.data() + ui_data_offset, ui_temp_len );
                }
            }
        }

        return arg;
    }

    void do_post_recv_buff( HTCPLINK h_link, const char* p_data, unsigned int ui_size )
    {
        if( (p_data == NULL) || (ui_size == 0) ){
            return;
        }

        pthread_mutex_lock( &mo_recv_buf_old_lock );
        std::map<HTCPLINK, std::vector<char> >::iterator it = m_recv_buf_old_map.find( h_link );
        if( it != m_recv_buf_old_map.end() ){
            std::vector<char> &vec_buff_old = it->second;
            std::vector<char>::size_type ul_len_old = vec_buff_old.size();
            vec_buff_old.resize( ul_len_old + ui_size );
            memcpy( vec_buff_old.data() + ul_len_old, p_data, ui_size );
        } else {
            std::vector<char> vec_temp;
            vec_temp.resize( ui_size );
            memcpy( vec_temp.data(), p_data, ui_size );
            m_recv_buf_old_map[h_link] = vec_temp;
        }
        pthread_mutex_unlock( &mo_recv_buf_old_lock );

        sem_post( &mo_buff_sem );
        return;
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

        return true;
    }

    virtual bool Init()
    {
        if( pthread_create( &mo_thread_id, NULL, handle_recv_buff, this ) != 0 ){
            MS_LOGER_ERROR( "create handle_recv_buff thread failed." );
            return false;
        }

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

        return true;
    }

    virtual void UnInit()
    {
        MS_LOGER_INFO( "LinkMgr Shutdown, UnInit." );

        if( mo_thread_id != 0 ){
            mb_thread_break = true;
            void *result = NULL;
            pthread_join( mo_thread_id, &result );
            mo_thread_id = 0;
        }
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

    virtual void on_tcp_recv_data( HTCPLINK h_link,  PACKET *p_pkt )
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

    bool link_recv_data( HTCPLINK h_link, PACKET *p_pkt )
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


    bool       mb_thread_break;
    pthread_t  mo_thread_id;
    sem_t      mo_buff_sem;

    pthread_mutex_t mo_recv_buf_old_lock;
    std::map<HTCPLINK, std::vector<char> > m_recv_buf_old_map;

    std::map<HTCPLINK, std::vector<char> > m_recv_buf_map;
};

#endif
