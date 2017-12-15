/***************************************************************************
 *   Copyright (C) 2007 by PAX   *
 *   pax@m-200.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/time.h>

#include "CDRRadius.h"
#include "radius_freeradius-client.h"
#include "client.h"
#include "netmes.h"
#include "message.h"

#ifndef ARMLINUX
#include "UniNumber.h"
#endif

//дефайны из SMP/include/route.h
#define DIR_NUMBER  ((DWORD)0x00000000) // по номеру телефона
#define DIR_HARD    ((DWORD)0x40000000) // жесткая привязка из конфигурации для V5
#define DIR_SPECIAL ((DWORD)0x80000000) // служебная нумерация
#define DIR_COMMON  ((DWORD)0xC0000000) // обычные

#define DIR_COMMON_ERROR  (DIR_COMMON | 0x0F000000 | 1)
#define DIR_COMMON_REPEAT (DIR_COMMON | 0x0F000000 | 2)
#define DIR_COMMON_WAIT   (DIR_COMMON | 0x0F000000 | 3)
#define DIR_COMMON_LOCAL  (DIR_COMMON | 0x0F000000 | 4)
#define DIR_COMMON_DIROUT (DIR_COMMON | 0x0F000000 | 5)
//
#define DEVIL_MODULE 63

const BYTE VENDOR_CODE = 9;

void Loger(const char* str);
void Logerf(const char *format, ...);
void SMPWritePacket (BYTE* data, short len );

DWORD MakeDirBySpecial ( BYTE module, BYTE slot, BYTE hole )
{
    DWORD res;
    if(module == DEVIL_MODULE)
    {
        TDir ret = 0;

        ret <<= 8;
        ret += module;

        ret <<= 16;
        ret += slot * 32 + hole;

        res =  (DIR_SPECIAL | ret);
    }
    else
    {
        DWORD ret = 0;

        ret <<= 8;
        ret += module;

        ret <<= 8;
        ret += slot;

        ret <<= 8;
        ret += hole;

        res = ret | DIR_SPECIAL;
    }
    return res;
}

void PortResetSend(BYTE in_Mod, DWORD in_ID)
{
    CNetMessageBody xmes;
    xmes.nMessage = NET_MES_TRANSIT_PORT;
    xmes.src.nMod = 0;
    xmes.dst.nMod = in_Mod;
    DWORD data[2];
    data[0] = in_ID;
    data[1] = (DWORD)M_AFR_ResetPort;
    xmes.input((const uc *)data, sizeof(data));

    uc buf[sizeof(CNetMessageBody)];
    uc* p = xmes.encode(buf);
    short len = p - buf;
    SMPWritePacket(buf, len);
}
/*
void PortReleaseSend(BYTE in_Mod, DWORD in_ID)
{
    CNetMessageBody xmes;
    xmes.nMessage = NET_MES_TRANSIT_PORT;
    xmes.src.nMod = 0;
    xmes.dst.nMod = in_Mod;
    DWORD data[2];
    data[0] = in_ID;
    data[1] = (DWORD)M_PBX_RELEASE | (((DWORD)100) << 16);

    xmes.input((const uc *)data, sizeof(data));

    uc buf[sizeof(CNetMessageBody)];
    uc* p = xmes.encode(buf);
    short len = p - buf;
    SMPWritePacket(buf, len);
}
*/
void AuthRadiusReject( CDRBuilding::strCallInfo &Call)
{
    DWORD id = MakeDirBySpecial ( Call.InUnit.btMod, Call.InUnit.btPcmSlot, Call.InUnit.btKIPort);
    PortResetSend(Call.InUnit.btMod, id);
}

#ifndef ARMLINUX

void AuthRadiusPrepayAnswer( CUniPar &unipar, SUniparAttr &UniparAttr)
{
    CNetMessageBody xmes;
    xmes.nMessage = NET_MES_AUTHORIZATION_SYSTEM;
    xmes.src.nMod = 0;
    xmes.dst.nMod = UniparAttr.mod;

    int pos = 0;
    BYTE data[sizeof(DWORD) + sizeof(DWORD) + sizeof(DWORD) + sizeof(bool) + sizeof(DWORD)];

    *(data + pos++) = (BYTE)UniparAttr.id;
    *(data + pos++) = (BYTE)(UniparAttr.id >> 8);
    *(data + pos++) = (BYTE)(UniparAttr.id >> 16);
    *(data + pos++) = (BYTE)(UniparAttr.id >> 24);

    *(data + pos++) = (BYTE)((DWORD)M_AFR_PrePayNumberComplete);
    *(data + pos++) = (BYTE)(((DWORD)M_AFR_PrePayNumberComplete) >> 8);
    *(data + pos++) = (BYTE)(((DWORD)M_AFR_PrePayNumberComplete) >> 16);
    *(data + pos++) = (BYTE)(((DWORD)M_AFR_PrePayNumberComplete) >> 24);

    *(data + pos++) = (BYTE)UniparAttr.time;
    *(data + pos++) = (BYTE)(UniparAttr.time >> 8);
    *(data + pos++) = (BYTE)(UniparAttr.time >> 16);
    *(data + pos++) = (BYTE)(UniparAttr.time >> 24);

    DWORD dwLen = 0;
    if(UniparAttr.time)
    {
        *(bool *)(data + pos) = true;
        pos += sizeof(bool);
        dwLen = unipar.getLen();

        *(data + pos++) = (BYTE)dwLen;
        *(data + pos++) = (BYTE)(dwLen >> 8);
        *(data + pos++) = (BYTE)(dwLen >> 16);
        *(data + pos++) = (BYTE)(dwLen >> 24);
    }
    else
    {
        *(bool *)(data + pos) = false;
        pos += sizeof(bool);
    }

    xmes.input((const uc *)data, pos);
    if(dwLen)
        xmes.input((const uc *)&unipar, dwLen);

    uc buf[sizeof(CNetMessageBody)];
    uc* p = xmes.encode(buf);
    short len = p - buf;
    SMPWritePacket(buf, len);
}

#endif

void TClock2tm (struct tm *T, const struct TClock *cl)
{
    memset(T, 0x00, sizeof(*T));

    T->tm_sec = cl->Seconds;
    T->tm_min = cl->Minutes;
    T->tm_hour = cl->Hours;
    T->tm_mday = cl->Date;
    T->tm_mon = cl->Month - 1;
    T->tm_year = cl->Year + 100;
    T->tm_isdst = -1;
    time_t itime = mktime (T);
    if(itime == -1)
    {
        Loger("Radius: TClock2tm failure\n");
        return;
    }
    localtime_r(&itime, T);
}

void tm2TClock (struct TClock *cl, const struct tm *T)
{
    memset(cl, 0x00, sizeof(*cl));

    cl->Seconds = T->tm_sec;
    cl->Minutes = T->tm_min;
    cl->Hours = T->tm_hour;
    cl->Date = T->tm_mday;
    cl->Month = T->tm_mon + 1;
    cl->Year =  T->tm_year - 100; 

}

void AddSecToTClock(TClock *cl, int iDelta)
{
    tm T;
    TClock2tm (&T, cl);
    time_t itime = mktime (&T);
    if(itime == -1)
    {
        Loger("Radius: AddSecToTClock failure\n");
        return;
    }

    int ttt = (int)itime;
    ttt += iDelta;
    itime = (time_t)ttt;
    localtime_r(&itime, &T);
    tm2TClock (cl, &T);
}

void GetTimeStrLanBilling (char *str, const TClock *cl)
{
    const char *mon_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char *day_week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    tm T;
    TClock2tm (&T, cl);

    sprintf(str, "%0.2d:%0.2d:%0.2d.0000 MSK %s %s %0.2d %0.4d",
    T.tm_hour, T.tm_min, T.tm_sec, day_week[T.tm_wday], mon_str[T.tm_mon], T.tm_mday, T.tm_year + 1900);
}

void GetTimeStrUniversal(char *str, const TClock *cl)
{
    tm T;
    TClock2tm (&T, cl);
    time_t itime = mktime (&T);
    if(itime == -1)
    {
        Loger("Radius: GetTimeStrUniversal failure\n");
        return;
    }
    ctime_r(&itime, str);
}

static const unsigned int MAX_RADIUS_THR = 30;
static unsigned int nThrCount = 0;
static pthread_mutex_t nThrCount_mutex = PTHREAD_MUTEX_INITIALIZER;

//-------------------------------------Accounting----------------------------------


void AccRadiusCDR(const CParser *_parser, const CDRBuilding::strCDR& _CDR)
{

    if(!_parser->sRadiusAccIp[0])
        return;

    if(!_parser->rh)
        return;

    pthread_mutex_lock(&nThrCount_mutex);
    if(nThrCount >= MAX_RADIUS_THR)
    {
        Loger("Radius: WARNING TOO MANY THREADS!!!\n");
        pthread_mutex_unlock(&nThrCount_mutex);
        return;
    }
    nThrCount++;
    pthread_mutex_unlock(&nThrCount_mutex);

    pthread_t thr;
    CAccRadius* acc = new CAccRadius(_parser, _CDR, &thr);
    if(!acc)
    {
        Loger("Radius: new thr failure\n");
        return;
    }
    if(!_parser->sSpiderIp)
        pthread_join(thr, NULL);
}

CAccRadius::CAccRadius(const CParser *_parser, const CDRBuilding::strCDR& _CDR, pthread_t *_thr)
{

    memset(this, 0, sizeof(*this));

    fDelete = false;
    pParser = _parser;
    memcpy(&CDR, &_CDR, sizeof(CDRBuilding::strCDR));

    if(_thr)
    {
        pthread_create(_thr, NULL, &CAccRadius::_ThreadProc, this);
        memcpy(&thr, _thr, sizeof(pthread_t));
    }
    else
        pthread_create(&thr, NULL, &CAccRadius::_ThreadProc, this);

    fDelete = true;
}

CAccRadius::~CAccRadius()
{
}

void *CAccRadius::_ThreadProc(void* lpParameter)
{
    CAccRadius* thiss = (CAccRadius*)lpParameter;
    thiss->ThreadProc();
    delete thiss;

    pthread_mutex_lock(&nThrCount_mutex);
    nThrCount--;
    pthread_mutex_unlock(&nThrCount_mutex);

    return NULL;
}

void CAccRadius::ThreadProc ( void )
{

    uint32_t        client_port = 0;
    uint32_t        status_type = 0;
    VALUE_PAIR      *send = NULL;

    if(pParser->sSpiderIp)
       pthread_detach(pthread_self());

    switch (pParser->nRadiusAccBilling)
    {
        case BILLING_UTM5:
        case BILLING_LANBILLING:
        {
            //--------------------формируем старт-------------------------------

            status_type = PW_STATUS_START;

            if (rc_avpair_add(pParser->rh, &send, PW_ACCT_STATUS_TYPE, (void *)&status_type, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Acct-Status-Type: %d\n", status_type);
                rc_avpair_free(send);
                return;
            }

            struct timeval tv;
            gettimeofday(&tv, 0);
            char session_id[50];
            sprintf(session_id, "%u-%u", tv.tv_sec, tv.tv_usec);

            // Идентификатор вызова
            if (rc_avpair_add(pParser->rh, &send, PW_ACCT_SESSION_ID, (void *)session_id, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Acct-Session-ID: %s\n", session_id);
                rc_avpair_free(send);
                return;
            }

            char av_session_id[100];
            sprintf(av_session_id, "h323-conf-id=%s", session_id);

            // Идентификатор вызова в формате Cisco
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CONF_ID, (void *)av_session_id, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Conf_ID: %s\n", session_id);
                rc_avpair_free(send);
                return;
            }

            const char *username = CDR.STrunkInInfo.sCgPN[0] ? CDR.STrunkInInfo.sCgPN :
            (CDR.STrunkOutInfo.sCgPN[0] ? CDR.STrunkOutInfo.sCgPN : pParser->SSettings.sNoNumberString);

            // username = номер звонящего
            if (rc_avpair_add(pParser->rh, &send, PW_USER_NAME, (void *)username, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding User-Name: %s\n", username);
                rc_avpair_free(send);
                return;
            }

            const char *pCgPN = username;
            // номер звонящего
            if (rc_avpair_add(pParser->rh, &send, PW_CALLING_STATION_ID, (void *)pCgPN, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Calling-Station-ID: %s\n", pCgPN);
                rc_avpair_free(send);
                return;
            }

            const char *pCdPN = CDR.STrunkInInfo.sCdPN[0] ? CDR.STrunkInInfo.sCdPN :
            (CDR.STrunkOutInfo.sCdPN[0] ? CDR.STrunkOutInfo.sCdPN : pParser->SSettings.sNoNumberString);

            // Набранный номер
            if (rc_avpair_add(pParser->rh, &send, PW_CALLED_STATION_ID, (void *)pCdPN, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Called-Station-ID: %s\n", pCdPN);
                rc_avpair_free(send);
                return;
            }

            // Тип вызова
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_TYPE, (void *)"h323-call-type=voip", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Call_Type: voip\n");
                rc_avpair_free(send);
                return;
            }

            // Поле не очень понятно
             if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_ORIGIN, (void *)"h323-call-origin=originate", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Call_Origin: originate\n");
                rc_avpair_free(send);
                return;
            }

            char setup_time[50];
            GetTimeStrLanBilling(setup_time, &CDR.timeSeizIn);
            char av_setup_time[100];
            sprintf(av_setup_time, "h323-setup-time=%s", setup_time);

            // Добавляем поля время занятия канала
            if (rc_avpair_add(pParser->rh, &send, PW_H323_SETUP_TIME, (void *)av_setup_time, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Setup_Time: %s\n", setup_time);
                rc_avpair_free(send);
                return;
            }

            const char* pTrunk = CDR.STrunkInInfo.sTrunk;
            char trunk_in[50];
            strcpy(trunk_in, "0000000");
            do
            {
                if(!pTrunk) break;
                if(!pTrunk[0]) break;
                if(strlen(pTrunk) != 8) break;
                if(pTrunk[0] != 'C') break;
                strncpy(trunk_in, &pTrunk[1], 7);
            }while(0);

            if (rc_avpair_add(pParser->rh, &send, PW_CISCO_NAS_PORT, (void *)trunk_in, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding CISCO_NAS_PORT: %s\n", trunk_in);
                rc_avpair_free(send);
                return;
            }

            pTrunk = CDR.STrunkOutInfo.sTrunk;
            char trunk_out[50];
            strcpy(trunk_out, "0000000");
            do
            {
                if(!pTrunk) break;
                if(!pTrunk[0]) break;
                if(strlen(pTrunk) != 8) break;
                if(pTrunk[0] != 'C') break;
                strncpy(trunk_out, &pTrunk[1], 7);
            }while(0);

            char av_trunk_out[100];
            sprintf(av_trunk_out, "h323-gw-id=%s", trunk_out);

            if (rc_avpair_add(pParser->rh, &send, PW_H323_GW_ID, (void *)av_trunk_out, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_GW_ID: %s\n", trunk_out);
                rc_avpair_free(send);
                return;
            }

            if(rc_acct(pParser->rh, client_port, send) == OK_RC)
                printf("Acc radius: Acc START OK: sid: %s %s -> %s\n", session_id, username, pCdPN);
            else
                printf("Acc radius: Acc START Failed: sid: %s %s -> %s\n", session_id, username, pCdPN);

            //--------------------формируем стоп-------------------------------


            VALUE_PAIR *tmp_send = send->next;
            send->next = NULL;
            rc_avpair_free(send);
            send = NULL;

            status_type = PW_STATUS_STOP;

            if (rc_avpair_add(pParser->rh, &send, PW_ACCT_STATUS_TYPE, (void *)&status_type, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Acct-Status-Type: %d\n", status_type);
                rc_avpair_free(send);
                rc_avpair_free(tmp_send);
                return;
            }
            send->next = tmp_send;

            TClock cl;
            memcpy(&cl, &CDR.timeSeizIn, sizeof(TClock));
            if(CDR.dwTalkDuration)
                AddSecToTClock(&cl, (int)(((int)CDR.dwSeizDurationIn) - ((int)CDR.dwTalkDuration)));
            char connect_time[50];
            GetTimeStrLanBilling(connect_time, &cl);
            char av_connect_time[100];
            sprintf(av_connect_time, "h323-connect-time=%s", connect_time);

            // Время ответа
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CONNECT_TIME, (void *)av_connect_time, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Connect_Time: %s\n", connect_time);
                rc_avpair_free(send);
                return;
            }

            if(CDR.dwTalkDuration)
                AddSecToTClock(&cl, (int)CDR.dwTalkDuration);
            char disconnect_time[50];
            GetTimeStrLanBilling(disconnect_time, &cl);
            char av_disconnect_time[100];
            sprintf(av_disconnect_time, "h323-disconnect-time=%s", disconnect_time);

            // Время рассоединения
            if (rc_avpair_add(pParser->rh, &send, PW_H323_DISCONNECT_TIME, (void *)av_disconnect_time, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Radius: Failed adding H323_Disconnect_Time: %s\n", disconnect_time);
                rc_avpair_free(send);
                return;
            }

            // Время разговора
            if (rc_avpair_add(pParser->rh, &send, PW_ACCT_SESSION_TIME, (void *)&CDR.dwTalkDuration, -1, 0) == NULL)
            {
                Logerf("Acc radius: Failed adding Acc_Session_Time\n");
                rc_avpair_free(send);
                return;
            }

            BYTE bCause;
            switch(CDR.btReason)
            {
                case 17: bCause = 17; break;
                case 16: bCause = 0; break;
                case 19: bCause = 5; break;
                default: bCause = 22;
            }

            char cause[50];
            sprintf(cause, "%u", bCause);
            char av_cause[100];
            sprintf(av_cause, "h323-disconnect-cause=%s", cause);
            // Причина разъединения
            if (rc_avpair_add(pParser->rh, &send, PW_H323_DISCONNECT_CAUSE, (void *)av_cause, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Acc radius: Failed adding H323_Disconnect_Cause\n");
                rc_avpair_free(send);
                return;
            }

            if(rc_acct(pParser->rh, client_port, send) == OK_RC)
                printf("Acc radius: Acc STOP OK: sid: %s %s -> %s\n\n", session_id, username, pCdPN);
            else
                printf("Acc radius: Acc STOP Failed: sid: %s %s -> %s\n\n", session_id, username, pCdPN);

            rc_avpair_free(send);
        }
        break;
        default: Loger("Acc radius: Unknown billing system\n");
    }

    while(!fDelete)
        usleep(50*1000);
}

void CAccRadius::SetThreadPriority( int nPolicy, int nPriority )
{
    struct sched_param p;
    p.sched_priority = nPriority;
    pthread_setschedparam(thr, nPolicy , &p);
}

//-------------------------------------Authorization----------------------------------

void AuthRadius(const CParser *_parser, const CDRBuilding::strCallInfo* _pCall)
{
    if(!_parser->sSpiderIp)
        return;

    if(!_parser->sRadiusAuthIp[0])
        return;

    if(!_parser->rh)
        return;

    if(!_pCall)
        return;

    pthread_mutex_lock(&nThrCount_mutex);
    if(nThrCount >= MAX_RADIUS_THR)
    {
        Loger("Auth radius pr: WARNING TOO MANY THREADS!!!\n");
        pthread_mutex_unlock(&nThrCount_mutex);
        return;
    }
    nThrCount++;
    pthread_mutex_unlock(&nThrCount_mutex);

    pthread_t thr;
    CAuthRadius* auth = new CAuthRadius(_parser, _pCall, &thr);
    if(!auth)
    {
        Loger("Auth radius pr: new thr failure\n");
        return;
    }
}

CAuthRadius::CAuthRadius(const CParser *_parser, const CDRBuilding::strCallInfo* _pCall, pthread_t *_thr)
{

    memset(this, 0, sizeof(*this));

    fDelete = false;
    pParser = _parser;
    memcpy(&Call, _pCall, sizeof(CDRBuilding::strCallInfo));

    if(_thr)
    {
        pthread_create(_thr, NULL, &CAuthRadius::_ThreadProc, this);
        memcpy(&thr, _thr, sizeof(pthread_t));
    }
    else
        pthread_create(&thr, NULL, &CAuthRadius::_ThreadProc, this);

    fDelete = true;

}

CAuthRadius::~CAuthRadius()
{
}

void *CAuthRadius::_ThreadProc(void* lpParameter)
{
    CAuthRadius* thiss = (CAuthRadius*)lpParameter;
    thiss->ThreadProc();
    delete thiss;

    pthread_mutex_lock(&nThrCount_mutex);
    nThrCount--;
    pthread_mutex_unlock(&nThrCount_mutex);

    return NULL;
}

void CAuthRadius::ThreadProc ( void )
{
    uint32_t        service;
    char            msg[1024];
    VALUE_PAIR      *send = NULL, *received = NULL;

    pthread_detach(pthread_self());

    switch (pParser->nRadiusAuthBilling)
    {
        case BILLING_LANBILLING:
        case BILLING_UTM5:
        {
            service = PW_LOGIN;
            if (rc_avpair_add(pParser->rh, &send, PW_SERVICE_TYPE, &service, -1, 0) == NULL)
            {
                Loger("Auth radius: Failed adding Service type\n");
                rc_avpair_free(send);
                return;
            }

            struct timeval tv;
            gettimeofday(&tv, 0);
            char session_id[50];
            sprintf(session_id, "%u-%u", tv.tv_sec, tv.tv_usec);

            char av_session_id[100];
            sprintf(av_session_id, "h323-conf-id=%s", session_id);

            // Идентификатор вызова в формате Cisco
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CONF_ID, (void *)av_session_id, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Radius: Failed adding H323_Conf_ID: %s\n", session_id);
                rc_avpair_free(send);
                return;
            }

            // username = номер звонящего
            const char *username = Call.RadiusAuthUserName;

            if (rc_avpair_add(pParser->rh, &send, PW_USER_NAME, (void *)username, -1, 0) == NULL)
            {
                Logerf("Auth radius: Failed adding User-Name: %s\n", username);
                rc_avpair_free(send);
                return;
            }

            // номер звонящего
            const char *pCgPN = username;
            if (rc_avpair_add(pParser->rh, &send, PW_CALLING_STATION_ID, (void *)pCgPN, -1, 0) == NULL)
            {
                Logerf("Auth radius: Failed adding Calling-Station-ID: %s\n", pCgPN);
                rc_avpair_free(send);
                return;
            }

            // Набранный номер
            const char *pCdPN = Call.InUnit.acCdPN[0] ? Call.InUnit.acCdPN :
                (Call.OutUnit.acCdPN[0] ? Call.OutUnit.acCdPN :
                pParser->SSettings.sNoNumberString);

            if (rc_avpair_add(pParser->rh, &send, PW_CALLED_STATION_ID, (void *)pCdPN, -1, 0) == NULL)
            {
                Logerf("Auth radius: Failed adding Called-Station-ID: %s\n", pCdPN);
                rc_avpair_free(send);
                return;
            }

            /*
            // Тип вызова
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_TYPE, (void *)"h323-call-type=voip", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Auth radius: Failed adding H323_Call_Type: voip\n");
                rc_avpair_free(send);
                return;
            }

            // Поле не очень понятно
             if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_ORIGIN, (void *)"h323-call-origin=originate", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Auth radius: Failed adding H323_Call_Origin: originate\n");
                rc_avpair_free(send);
                return;
            }
            */

            char trunk_out[50];
            sprintf(trunk_out, "%02d%03d%02d", Call.OutUnit.btMod, Call.OutUnit.btPcmSlot, Call.OutUnit.btKIPort);

            char av_trunk_out[100];
            sprintf(av_trunk_out, "h323-gw-id=%s", trunk_out);

            if (rc_avpair_add(pParser->rh, &send, PW_H323_GW_ID, (void *)av_trunk_out, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Auth radius: Failed adding H323_GW_ID: %s\n", trunk_out);
                rc_avpair_free(send);
                return;
            }

            int result = rc_auth(pParser->rh, 0, send, &received, msg);

            //Отбиваем порт
            if(result != OK_RC)
                if(pParser->sScommIp[0] && pParser->nScommPort)
                    AuthRadiusReject(Call);

            if(result == OK_RC)
            {
                VALUE_PAIR * vp = received;
                while (vp)
                {
                    if(vp->strvalue)
                        if(strlen(vp->strvalue))
                            printf("Auth radius: Auth OK: sid: %s %s\n",session_id, vp->strvalue);
                    vp = vp->next;
                }
                printf("Auth radius: Auth OK: %s -> %s\n\n", username, pCdPN);
            }
            else if(result == REJECT_RC)
            {
                VALUE_PAIR *vp = received;
                while (vp)
                {
                    if(vp->strvalue)
                        if(strlen(vp->strvalue))
                            printf("Auth radius: Auth Rejected: sid: %s %s\n",session_id, vp->strvalue);
                    vp = vp->next;
                }
                printf("Auth radius: Auth Rejected: sid: %s %s -> %s\n\n", session_id, username, pCdPN);
            }
            else
                printf("Auth radius: Auth Failed: sid: %s %s -> %s\n\n", session_id, username, pCdPN);

            rc_avpair_free(send);
            rc_avpair_free(received);
        }
        break;
        default: Loger("Auth radius: Unknown billing system\n");
    }

    while(!fDelete)
        usleep(50*1000);
}

void CAuthRadius::SetThreadPriority( int nPolicy, int nPriority )
{
    struct sched_param p;
    p.sched_priority = nPriority;
    pthread_setschedparam(thr, nPolicy , &p);
}

//-------------------------------------Authorization Prepay----------------------------------

#ifndef ARMLINUX

void AuthRadiusPrepay(const CParser *_parser, const CUniPar *_unipar, const struct SUniparAttr *_UniparAttr)
{
    if(!_parser->sSpiderIp)
        return;

    if(!_parser->sRadiusAuthIp[0])
        return;

    if(!_parser->rh)
        return;

    if(!_unipar || !_UniparAttr)
        return;

    pthread_mutex_lock(&nThrCount_mutex);
    if(nThrCount >= MAX_RADIUS_THR)
    {
        Loger("Auth radius pr: WARNING TOO MANY THREADS!!!\n");
        pthread_mutex_unlock(&nThrCount_mutex);
        return;
    }
    nThrCount++;
    pthread_mutex_unlock(&nThrCount_mutex);

    pthread_t thr;
    CAuthRadiusPrepay* auth = new CAuthRadiusPrepay(_parser, _unipar, _UniparAttr, &thr);
    if(!auth)
    {
        Loger("Auth radius pr: new thr failure\n");
        return;
    }
}

CAuthRadiusPrepay::CAuthRadiusPrepay(const CParser *_parser, const CUniPar *_unipar, const struct SUniparAttr *_UniparAttr, pthread_t *_thr)
{
    memset(this, 0, sizeof(*this));

    fDelete = false;
    pParser = _parser;

    memcpy(&UniparAttr, _UniparAttr, sizeof(struct SUniparAttr));
    memcpy(&unipar, _unipar, _UniparAttr->len);

    if(_thr)
    {
        pthread_create(_thr, NULL, &CAuthRadiusPrepay::_ThreadProc, this);
        memcpy(&thr, _thr, sizeof(pthread_t));
    }
    else
        pthread_create(&thr, NULL, &CAuthRadiusPrepay::_ThreadProc, this);

    fDelete = true;

}

CAuthRadiusPrepay::~CAuthRadiusPrepay()
{
}

void *CAuthRadiusPrepay::_ThreadProc(void* lpParameter)
{
    CAuthRadiusPrepay* thiss = (CAuthRadiusPrepay*)lpParameter;
    thiss->ThreadProc();
    delete thiss;

    pthread_mutex_lock(&nThrCount_mutex);
    nThrCount--;
    pthread_mutex_unlock(&nThrCount_mutex);

    return NULL;
}

void CAuthRadiusPrepay::ThreadProc ( void )
{
    uint32_t        service;
    char            msg[1024];
    VALUE_PAIR      *send = NULL, *received = NULL;

    pthread_detach(pthread_self());

    switch (pParser->nRadiusAuthBilling)
    {
        case BILLING_LANBILLING:
        case BILLING_UTM5:
        {
            service = PW_LOGIN;
            if (rc_avpair_add(pParser->rh, &send, PW_SERVICE_TYPE, &service, -1, 0) == NULL)
            {
                Loger("Auth radius pr: Failed adding Service type\n");
                rc_avpair_free(send);
                return;
            }

            struct timeval tv;
            gettimeofday(&tv, 0);
            char session_id[50];
            sprintf(session_id, "%u-%u", tv.tv_sec, tv.tv_usec);

            char av_session_id[100];
            sprintf(av_session_id, "h323-conf-id=%s", session_id);

            // Идентификатор вызова в формате Cisco
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CONF_ID, (void *)av_session_id, -1, VENDOR_CODE) == NULL)
            {
                Logerf("Radius pr: Failed adding H323_Conf_ID: %s\n", session_id);
                rc_avpair_free(send);
                return;
            }

            // username = номер звонящего
            const char *username;
            char sCalling[100];

            if(!unipar.getString ( UNIPAR_AON, sCalling, sizeof(sCalling) ))
            {
                Loger("Auth radius pr: No AON in unipar\n");
                rc_avpair_free(send);
                return;
            }
            else
                username = sCalling;

            if (rc_avpair_add(pParser->rh, &send, PW_USER_NAME, (void *)username, -1, 0) == NULL)
            {
                Logerf("Auth radius pr: Failed adding User-Name: %s\n", username);
                rc_avpair_free(send);
                return;
            }

            // номер звонящего
            const char *pCgPN = username;
            if (rc_avpair_add(pParser->rh, &send, PW_CALLING_STATION_ID, (void *)pCgPN, -1, 0) == NULL)
            {
                Logerf("Auth radius pr: Failed adding Calling-Station-ID: %s\n", pCgPN);
                rc_avpair_free(send);
                return;
            }

            // Набранный номер
            const char *pCdPN;
            char sCalled[100];

            if(!unipar.getString ( UNIPAR_NUMBER, sCalled, sizeof(sCalled) ))
            {
                Loger("Auth radius pr: No NUMBER in unipar\n");
                rc_avpair_free(send);
                return;
            }
            else
                pCdPN =  sCalled;

            if (rc_avpair_add(pParser->rh, &send, PW_CALLED_STATION_ID, (void *)pCdPN, -1, 0) == NULL)
            {
                Logerf("Auth radius pr: Failed adding Called-Station-ID: %s\n", pCdPN);
                rc_avpair_free(send);
                return;
            }

            /*
            // Тип вызова
            if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_TYPE, (void *)"h323-call-type=voip", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Auth radius: Failed adding H323_Call_Type: voip\n");
                rc_avpair_free(send);
                return;
            }

            // Поле не очень понятно
             if (rc_avpair_add(pParser->rh, &send, PW_H323_CALL_ORIGIN, (void *)"h323-call-origin=originate", -1, VENDOR_CODE) == NULL)
            {
                Logerf("Auth radius: Failed adding H323_Call_Origin: originate\n");
                rc_avpair_free(send);
                return;
            }
            */

            int result = rc_auth(pParser->rh, 0, send, &received, msg);

            UniparAttr.time = 0;

            //Отбиваем порт
            if(result != OK_RC)
            {
                if(pParser->sScommIp[0] && pParser->nScommPort)
                    AuthRadiusPrepayAnswer(unipar, UniparAttr);
            }

            if(result == OK_RC)
            {
                VALUE_PAIR * vp = received;
                while (vp)
                {
                    if(vp->strvalue)
                    {
                        char *s;
                        if(strlen(vp->strvalue))
                        {
                            s = strstr(vp->strvalue, "h323-credit-time=");
                            if(s)
                            {
                                s += strlen("h323-credit-time=");
                                if(strlen(s))
                                {
                                    int t = atoi(s);
                                    if(t > 0)
                                        UniparAttr.time = (DWORD)t;
                                }
                            }
                            printf("Auth radius pr: Auth OK: sid: %s %s\n", session_id, vp->strvalue);
                        }
                    }
                    vp = vp->next;
                }

                //Подмена номера
                /*
                strcpy(sCalled, "6000");
                unipar.remove ( UNIPAR_NUMBER );
                unipar.remove ( UNIPAR_CalledNumber );
                UniN::Number num;
                num.setNumber(sCalled, true);
                unipar.addNumber(UNIPAR_CalledNumber, &num);
                unipar.addBuffer(UNIPAR_NUMBER, (const void*)sCalled, strlen(sCalled));
                */

                printf("Auth radius pr: Auth OK: %s -> %s\n\n", username, pCdPN);
                if(pParser->sScommIp[0] && pParser->nScommPort)
                    AuthRadiusPrepayAnswer(unipar, UniparAttr);
            }
            else if(result == REJECT_RC)
            {
                VALUE_PAIR *vp = received;
                while (vp)
                {
                    if(vp->strvalue)
                        if(strlen(vp->strvalue))
                            printf("Auth radius pr: Auth Rejected: sid: %s %s\n",session_id, vp->strvalue);
                    vp = vp->next;
                }
                printf("Auth radius pr: Auth Rejected: sid: %s %s -> %s\n\n", session_id, username, pCdPN);
            }
            else
                printf("Auth radius pr: Auth Failed: sid: %s %s -> %s\n\n", session_id, username, pCdPN);

            rc_avpair_free(send);
            rc_avpair_free(received);
        }
        break;
        default: Loger("Auth radius pr: Unknown billing system\n");
    }

    while(!fDelete)
        usleep(50*1000);
}

void CAuthRadiusPrepay::SetThreadPriority( int nPolicy, int nPriority )
{
    struct sched_param p;
    p.sched_priority = nPriority;
    pthread_setschedparam(thr, nPolicy , &p);
}

#endif //ARMLINUX