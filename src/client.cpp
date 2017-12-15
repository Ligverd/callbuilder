#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include "client.h"
#include "lib.h"
#include "CDRRadius.h"

extern CSMPClient *SMP;

void Loger(const char* str);
void Logerf(const char *format, ...);
void SMPWritePacket (BYTE* data, short len );

void CSMPClient::OnClose( const char* reason )
{
    if(reason)
        Logerf("SMP Client %d %s\n", con, reason);
    else
        Logerf("SMP Client %d disconnected!\n", con);

    Destroy((CSYSTCPTransport**)&SMP, reason == NULL ); //com->clnt[con] = NULL; && //delete this;
}

void CSMPClient::OnConnect( void )
{
    Logerf("SMP Client %d connected!\n", con);
}

int CSMPClient::OnReceive(int fd2)
{
    int len = recv(fd2, recvBuf + recvLen, 10000 - recvLen, 0);

    if (len <= 0)
        return len;

    recvLen += len;
    if (!fBinaryRead)
    {
        int done = 0;
        for (int p = 0; p < recvLen && !fBinaryRead; )
        {
            if (recvBuf[p] == 0xA)
            {
                memmove(recvBuf + p, recvBuf + p + 1, recvLen - p - 1);
                recvLen --;
                continue;
            }
            if (recvBuf[p] == 0xD)
            {
                if (!strncmp((char *)(recvBuf + done), "BINARYMODE-OK", 10 + 1 + 2))
                {
                    fBinaryRead = true;
                    Logerf("SMP Client %d BINARYMODE-OK\n", con);
                }
                if (!strncmp((char *)(recvBuf + done), "BINARYMODE-ER", 10 + 1 + 2))
                {
                    fBinaryRead = true;
                    OnClose("BINARYMODE-ER");
                }
                else
                {
                    ReceivePacket(recvBuf + done, p - done);
                }
                p ++;
                if (recvBuf[p] == 0xA)
                    p ++;
                done = p;
            }
            else
                p ++;
        } //if binary mode ok
        if (done < recvLen)
        {
            memmove(recvBuf, recvBuf  + done, recvLen - done);
            recvLen -= done;
        }
        else
            recvLen = 0;
    }
    if (fBinaryRead)
    {
        while (recvLen >= 2)
        {
            WORD size = (WORD)(*recvBuf) + ((WORD)*(recvBuf+1) << 8);
            if (size > MAXSIZEONEPACKET)
            {
                Logerf("SMP Client %d BADPACKET\n", con);
                recvLen = 0;
                return len;
            }
            if (size + 2 > recvLen)
                return len;
            ReceivePacket(recvBuf + 2, size);
            if (recvLen > size + 2)
            {
                memmove(recvBuf, recvBuf + size + 2, recvLen - size - 2);
                recvLen -= size + 2;
            }
            else
                recvLen = 0;
        }
    }
    return len;
}
void CSMPClient::OnMessage(CNetMessageBody &net)
{
    switch (net.nMessage)
    {
        case NET_MES_PREPAY_NUMBER_COMPLETE:
        {
            if(!Parser.fRadAuthPrepay) //Схема prepay не включена
                return;

    #ifndef ARMLINUX

            int pos = 0;
            BYTE *pdata = net.data;

            DWORD id = *(pdata + pos++);
            id |= *(pdata + pos++) << 8;
            id |= *(pdata + pos++) << 16;
            id |= *(pdata + pos++) << 24;

            TClock cl;
            memcpy(&cl, pdata + pos, sizeof(TClock));
            pos += sizeof(TClock);

            bool fIsUnipar = *(bool *) (pdata + pos);
            pos += sizeof(bool);

            if(fIsUnipar)
            {
                DWORD dwLen = *(pdata + pos++);
                dwLen |= *(pdata + pos++) << 8;
                dwLen |= *(pdata + pos++) << 16;
                dwLen |= *(pdata + pos++) << 24;


                CUniPar *p_unipar = (CUniPar *)(pdata + pos);
                if(!p_unipar)
                {
                    Logerf("SMP Client %d: UNIPAR error\n", con);
                    return;
                }
                if (p_unipar->getLen() != dwLen)
                {
                    Logerf("SMP Client %d: UNIPAR lengh error\n", con);
                    return;
                }

                struct SUniparAttr UniparAttr;
                UniparAttr.len = dwLen;
                UniparAttr.id = id;
                UniparAttr.mod = net.src.nMod;

                AuthRadiusPrepay(&Parser, p_unipar, &UniparAttr);
            }
    #endif //ARMLINUX
        }
        break;
        default:;
    }


}

void CSMPClient::ReceivePacket (BYTE *data, short len )
{
    if (!fBinaryRead)
    {
        if (len < 0)
            return;
        /*
        CSMPMessage* mes = (CSMPMessage*) new BYTE[sizeof(CSMPMessage) + 1 + len];
        if(mes)
        {
            memcpy(mes->data, (char*)data, len);
            mes->data[len] = 0;
            mes->mes = SMPMESCLIENT_STRING;
            mes->len = len + 1;
            OnMessage(mes);
        }
        else
            Loger("Warning: Out of memory!\n");
        */
    }
    else
    {
        if (len <= 0)
            return;
        CNetMessageBody net;
        net.decode((unsigned char*)data, len);
        OnMessage(net);
    }
}

void CSMPClient::SendPacket ( BYTE* data, short len )
{
    const char* str;
    if (!fBinaryWrite)
    {
        str = Send(data, len);
        if(str)
            Logerf("SMP Client %d: %s\n", con, str);
    }
    else
    {
        BYTE tmp[1010];
        memcpy(tmp, &len, sizeof(short));
        memcpy(tmp + sizeof(short), data, len);
        str = Send(tmp, len + sizeof(short));
        if(str)
            Logerf("SMP Client %d: %s\n", con, str);
    }
}

int CSMPClient::SwitchBinary (const char *password)
{
    char str[10 + 1 + 6 + strlen("\r\n") + 1];
    sprintf(str, "BINARYMODE-%s\r\n", strlen(password) == 6 ? password : "100100");
    if (!fBinaryWrite)
    {
        int i;
        for(i = 0; i < 20; i++)
        {
            if(Connected() && fStarted())
                break;

            usleep(100*1000);
        }
        if(i == 20)
        {
            Logerf("SMP Client %d: SwitchBinary error\n", con);
            return -1;
        }
        pthread_mutex_lock(p_mutex);
        SendPacket((BYTE*)str, strlen(str));
        fBinaryWrite = true;
        pthread_mutex_unlock(p_mutex);
    }
    return 0;
}

const char* CSMPClient::MyConnect ( const char *ip, unsigned short port)
{
    struct hostent *host;
    host = gethostbyname(ip);
    if (!host)
        return "SMP Client: Error resolve server!";

    unsigned long adr;
    memcpy(&adr, host->h_addr_list[0], sizeof(unsigned long));
    return ConnectAddr(adr, port, true);
}
