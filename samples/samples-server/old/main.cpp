#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <sys/resource.h>
#include "ms-epoll-tcp.h"
#include "clientlog-api.h"
#include "test-server.h"

using namespace std;

static void WidebrightSegvHandler(int signum)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i = 0;

    signal(signum, SIG_DFL);

    size = backtrace (array, 10);
    strings = (char **)backtrace_symbols (array, size);

    MS_LOGER_FATAL( "signum = %d, stack info:", signum );

    //fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");
    for (i = 0; i < size; i++) {
        //fprintf(stderr, "%d %s \n",i,strings[i]);
        MS_LOGER_FATAL( "%s", strings[i] );
    }

    free (strings);
    exit(1);
}

void signal_WidebrightSegvHandler()
{
    signal( SIGSEGV, WidebrightSegvHandler );
    signal( SIGABRT, WidebrightSegvHandler );
    signal( SIGBUS, WidebrightSegvHandler );
    signal( SIGFPE, WidebrightSegvHandler );
    signal( SIGILL, WidebrightSegvHandler );
    signal( SIGXCPU, WidebrightSegvHandler );
    signal( SIGXFSZ, WidebrightSegvHandler );
    signal( SIGTERM, WidebrightSegvHandler );
}

void signal_sigign()
{
    sigset_t signal_mask;
    sigemptyset( &signal_mask );
    sigaddset( &signal_mask, SIGPIPE );
    pthread_sigmask( SIG_BLOCK, &signal_mask, NULL );

    signal( SIGHUP, SIG_IGN );
    signal( SIGINT, SIG_IGN );
    signal( SIGQUIT, SIG_IGN );
    signal( SIGPIPE, SIG_IGN );
}

int main()
{  
    signal_sigign();
    signal_WidebrightSegvHandler();

    ::ms_tcp_init();
    TestServer st_server( "0.0.0.0", 8899, 1 );
    st_server.Init();

    while( true ){
        sleep( 1 );
    }

    st_server.UnInit();
    ::ms_tcp_uninit();

    return 0;
}

