#include "systcp.h"
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>

CSYSTCPTransport::CSYSTCPTransport()
{
    memset(this, 0, sizeof(*this));

    p_mutex = NULL;

    fLive           = true;
    fConnect        = false;
    fThrStart       = false;
    fCloseNotify    = true;

    fd = -1;

    pthread_create(&thr, NULL, &CSYSTCPTransport::_ThreadProc, this);
}

CSYSTCPTransport::~CSYSTCPTransport()
{
    assert(!fLive);

    // закрыть сокет, если остался открытым.
    // Это может произойти, если разрушить ClientBack, не запуская его вызовом Start().
    if( fd >= 0)
    {
        close(fd);
        fd = -1;
    }
    thr = 0;
}

void CSYSTCPTransport::SetThreadPriority( int nPolicy, int nPriority )
{
    struct sched_param p;
    p.sched_priority = nPriority;
    pthread_setschedparam(thr, nPolicy , &p);
}

void CSYSTCPTransport::Disconnect ( bool fWait )
{
    assert(!fWait);
    fConnect = false;
}

void CSYSTCPTransport::Destroy(CSYSTCPTransport** pBackEraser, bool closeNotify)
{
    fCloseNotify = closeNotify;
    Disconnect(false);

    this->pBackEraser = pBackEraser;
    fLive = false;
}

const char* CSYSTCPTransport::Send ( const void* data, int len )
{
    if(!fThrStart)
        return NULL;

    if(!fConnect)
        return "error no connect1";

    if(fd < 0)
        return "error no connect2";

    for(int i = 0; i < 2;  i++ )
    {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(fd, &wset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200;

        int retval = select(fd + 1, NULL, &wset, NULL, &tv);

        if(!retval)
            continue;

        if(retval < 0)
        {
            if(errno == EINTR)
                continue;
            else { Disconnect(false); return "error send 1"; }
        }

        int err = -1;
        socklen_t opt_size = sizeof(int);
        getsockopt(fd , SOL_SOCKET, SO_ERROR, &err, &opt_size);
        if(err)
            { Disconnect(false); return "error send 2"; }

        retval = write(fd, (const char*)data, len);
        if(retval != len)
            { Disconnect(false); return "error send 3"; }

        break;
    }

    return NULL;
}

const char* CSYSTCPClientDirect::Connect ( const char *host, unsigned short port, bool fWait )
{
    unsigned long addr = inet_addr(host);

    if( addr == INADDR_NONE ){
        return "invalid host";
    }
    return ConnectAddr(addr, port, fWait);
}

const char* CSYSTCPClientDirect::ConnectAddr( unsigned long addr, unsigned short port, bool fWait)
{

    assert(fWait);

    if (fConnect)
        return "error bad logic";

    if( (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
        return "error create socket";

    int no = 1;

    if( setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&no, sizeof(int)) <0 )
    {
        close(fd);
        fd = -1;
        return "error setsockopt";
    }

    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr;
    sa.sin_port = htons(port);

    if (connect(fd, (sockaddr*)&sa, sizeof(sa)) != 0)
    {
        close(fd);
        fd = -1;
        return "error connect";
    }

    fConnect = true;

    return NULL;
}

void *CSYSTCPTransport::_ThreadProc(void* lpParameter)
{
    pthread_detach(pthread_self());
    CSYSTCPTransport* thiss = (CSYSTCPTransport*)lpParameter;
    thiss->ThreadProc();
    delete thiss;
    return NULL;
}

void CSYSTCPTransport::ThreadProc ( void )
{
    while (fLive)
    {
        while (fLive && !fConnect)
            usleep(100*1000);

        fThrStart = true;

        if (fLive && fConnect)
            OnConnect();

        while (fLive && fConnect)
        {
            if(fd < 0)
                break;

            int err = -1;
            socklen_t opt_size = sizeof(int);
            getsockopt(fd , SOL_SOCKET, SO_ERROR, &err, &opt_size);
            if(err)
                break;

            fd_set fdSet;
            FD_ZERO(&fdSet);
            FD_SET(fd, &fdSet);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 1000;

            int result = ::select(fd + 1, &fdSet, 0, 0, &tv);

            if( result == 0 )
                continue; // time out

            if( result < 0 )
            {
                if(errno == EINTR)
                    continue;
                else
                    break;
            }

            result = OnReceive(fd);

            if(result <= 0)
                break;

        }

        fConnect = false;

        close(fd);
        fd = -1;

        if (fCloseNotify)
            OnClose(NULL);
    }

    if(p_mutex)
        pthread_mutex_lock(p_mutex);

    if (pBackEraser)
        *pBackEraser = NULL;

    if(p_mutex)
        pthread_mutex_unlock(p_mutex);
}

// *********************

CSYSTCPClientBack::CSYSTCPClientBack ( int fd) : CSYSTCPTransport()
{
    this->fd = fd;
}

void CSYSTCPClientBack::Start ( void )
{
    fConnect = true;
}
// *********************

CSYSTCPServer::CSYSTCPServer()
{
    memset(this, 0, sizeof(*this));

    fLive           = true;
    fConnect        = false;
    fCloseNotify    = true;

    fd = -1;

    pthread_create(&thr, NULL, &CSYSTCPServer::_ThreadProc, this);
}

CSYSTCPServer::~CSYSTCPServer()
{
    assert(!fLive);
    thr = 0;
}



void *CSYSTCPServer::_ThreadProc(void* lpParameter)
{
    pthread_detach(pthread_self());
    CSYSTCPServer* thiss = (CSYSTCPServer*)lpParameter;
    thiss->ThreadProc();
    delete thiss;
    return NULL;
}

void CSYSTCPServer::Destroy ( CSYSTCPServer** pBackEraser, bool closeNotify)
{
    fCloseNotify = closeNotify;
    StopListen();

    this->pBackEraser = pBackEraser;
    fLive = false;
}

bool CSYSTCPServer::Listen ( const char *host, unsigned short port )
{
    if( (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
        return false;

    int no = 1;
    if( setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&no, sizeof(int)) < 0 )
    {
        close(fd);
        fd = -1;
        return false;
    }

    no = 1;
    if( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&no, sizeof(int)) < 0 )
    {
        close(fd);
        fd = -1;
        return false;
    }

    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;


    if(!strcmp(host," "))
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        sa.sin_addr.s_addr = strlen(host) ? inet_addr(host) : htonl(INADDR_ANY);

    sa.sin_port = htons(port);

    if( bind(fd, (sockaddr*)&sa, sizeof(sa)) < 0 )
    {
        close(fd);
        fd = -1;
        return false;
    }

    if( listen(fd, 20) < 0 )
    {
        close(fd);
        fd = -1;
        return false;
    }

    fConnect = true;
    return true;
}

void CSYSTCPServer::StopListen ( void )
{
    fConnect = false;
}

void CSYSTCPServer::ThreadProc ( void )
{
    while (fLive)
    {
        while (fLive && !fConnect)
            usleep(100*1000);

        while (fLive && fConnect)
        {
            int newFd;

            if(fd < 0)
                break;

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 1000;

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            int resultS = ::select(fd + 1, &fds, 0, 0, &tv);

            if( resultS == 0 )
                continue; // time out

            if( resultS < 0 )
            {
                if(errno == EINTR)
                    continue;
                else
                    break;
            }

            if( (newFd = accept(fd, NULL, 0)) < 0 )
            {
                usleep(100*1000);
                continue;
            }

            int no = 1;
            if( setsockopt(newFd, IPPROTO_TCP, TCP_NODELAY, (char*)&no, sizeof(int)) < 0 )
            {
                close(newFd);
                assert(0);
            }

            if(setsockopt(newFd, SOL_SOCKET, SO_KEEPALIVE, (char*)&no, sizeof(int)) < 0)
            {
                close(newFd);
                assert(0);
            }

/*
            no = 60;
            if (setsockopt(newFd, IPPROTO_TCP, TCP_KEEPIDLE, &no, sizeof(int)) < 0)
            {
                close(newFd);
                assert(0);
            }
            no = 60;
            if (setsockopt(newFd, IPPROTO_TCP, TCP_KEEPINTVL, &no, sizeof(int)) < 0)
            {
                close(newFd);
                assert(0);
            }
*/
            if (!OnAccept(newFd))
            {
                close(newFd);
            }
        }
    }

    close(fd);
    fd = -1;

    if (pBackEraser)
        *pBackEraser = NULL;
}

void CSYSTCPServer::SetThreadPriority( int nPolicy, int nPriority )
{
    struct sched_param p;
    p.sched_priority = nPriority;
    pthread_setschedparam(thr, nPolicy , &p);
}
// ************************

void CSYSTCPTransport::OnConnect ( void )
{
}

int CSYSTCPTransport::OnReceive ( int fd2 )
{
    return -1;
}

void CSYSTCPTransport::OnClose ( const char* reason )
{
}

bool CSYSTCPServer::OnAccept ( int fd2 )
{
    return false;
}
