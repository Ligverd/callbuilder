#ifndef _SYS_TCP_H_
#define _SYS_TCP_H_

#include <pthread.h>

class CSYSTCPTransport
{

public:
    CSYSTCPTransport();
    virtual ~CSYSTCPTransport();

    void SetThreadPriority( int nPolicy, int nPriority );

    void Disconnect ( bool fWait = true );

    void Destroy(CSYSTCPTransport** pBackEraser = NULL, bool closeNotify = true); // fWait == false

    const char* Send ( const void *data, int len ); // fWait == true

    pthread_t GetThread ( void ) { return thr; }

    void SetMutex ( pthread_mutex_t *pm ) {p_mutex = pm;}

    virtual void OnConnect (void);
    virtual int OnReceive ( int fd2 );
    virtual void OnClose ( const char* reason );

    bool Connected ( void ) { return fConnect; };

protected:

    static void *_ThreadProc(void* lpParameter);
    void ThreadProc ( void );

    volatile bool fLive;
    volatile bool fConnect;
    volatile bool fThrStart;
    volatile bool fCloseNotify;

    pthread_mutex_t *p_mutex;
    CSYSTCPTransport** pBackEraser;

    pthread_t thr;
    int fd; //socket
};

class CSYSTCPClientDirect : public CSYSTCPTransport
{

public:
    const char* Connect ( const char *host, unsigned short port, bool fWait = true);
    const char* ConnectAddr( unsigned long addr, unsigned short port, bool fWait = true);
};


class CSYSTCPClientBack : public CSYSTCPTransport
{
public:
    CSYSTCPClientBack ( int fd );
    void Start( void );
};


class CSYSTCPServer 
{

public:
    CSYSTCPServer();
    virtual ~CSYSTCPServer();

    bool Listen ( const char *host, unsigned short port ); // fWait == false
    void StopListen ( void ); // fWait == false

    void Destroy ( CSYSTCPServer** pBackEraser = NULL, bool closeNotify = true ); // fWait == false

    virtual bool OnAccept ( int newFd );

    void SetThreadPriority( int nPolicy, int nPriority );

protected:

    static void *_ThreadProc(void* lpParameter);

    void ThreadProc ( void );

    volatile bool fLive;
    volatile bool fConnect;
    volatile bool fCloseNotify;

    pthread_t thr;
    volatile int fd;

    CSYSTCPServer** pBackEraser;
};
#endif //_SYS_TCP_H_