#ifndef _CLIENT_H
#define _CLIENT_H

#include <type.h>
#include "systcp.h"
#include "netmes.h"

#define WM_COMMAND 0x100

#define MAXSIZEONEPACKET 320
#define MAXSMPBUFFER 10000
#define MAX_MODULE 64


#define SMPMESCLIENT_NONE          0
#define SMPMESCLIENT_CONNECT_OK    1
#define SMPMESCLIENT_CONNECT_ERROR 2
#define SMPMESCLIENT_CLOSE         3
#define SMPMESCLIENT_BINARYMODE    4
#define SMPMESCLIENT_BADPACKET     5
#define SMPMESCLIENT_STRING        6
#define SMPMESCLIENT_PACKET        7
#define SMPMESCLIENT_BINARYMODE_OK 8
#define SMPMESCLIENT_BINARYMODE_ER 9

class CSMPMessage
{
public:
    CSMPMessage( int _mes ) { mes = _mes; len = 0; }

    int mes;
    int len;
    BYTE data[1];
};


class CSMPClient : public CSYSTCPClientDirect
{
public:
    CSMPClient( int _con ) { con = _con; fBinaryRead = false; fBinaryWrite = false; recvLen = 0; }


    virtual void OnConnect ( void );
    virtual int OnReceive ( int fd2 );
    virtual void OnClose ( const char* reason );
    const char* MyConnect ( const char *ip, unsigned short port);
    void ReceivePacket ( BYTE* data, short len );
    void SendPacket ( BYTE* data, short len );
    void SendString ( char* str );
    int SwitchBinary ( const char * password);
    bool fStarted ( void ) {return fThrStart;}
    int con;

private:

    void OnMessage(CNetMessageBody &net);
    bool fBinaryRead, fBinaryWrite;
    BYTE recvBuf[10000];
    int recvLen;
};

#endif
