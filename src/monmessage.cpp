//#include  <include/monitor.h>
#include  <string.h>
#include  <include/monmessage.h>
#include  <include/unipar.h>

#include  <stdlib.h>

#ifndef  _ATS_
#include  <stdio.h>
#endif


//int CMonMessageEx::readPos_;
//int CMonMessageEx_readPos_;

CMonMessageEx::CMonMessageEx(BYTE message)
{
    message_ = message;
    time_    = 0;
    id_      = 0;

    size_    = 0;
}
//CMonMessageEx::CMonMessageEx()
//{
//    message_ = 0;
//    time_    = 0;
//    id_      = 0;
//    size_    = 0;
//}

unsigned char* CMonMessageEx::encode(unsigned char* stream) const
{
    *stream++   = time_;                    // относительное время в секундах
    
    *stream++   = (unsigned char)(id_ & 0xFF);            // идентификатор вызова
    *stream++   = (unsigned char)((id_ >> 8) & 0xFF);
    *stream++   = (unsigned char)((id_ >> 16) & 0xFF);
    *stream++   = (unsigned char)((id_ >> 24) & 0xFF);

    *stream++   = message_;                 // мессага

    memcpy(stream, data_, size_);
    return stream+size_;
}

unsigned char* CMonMessageEx::decode(unsigned char* stream, int size)
{
    if (size < 6) 
        return NULL;

    time_ = *stream++;                      // относительное время в секундах

    id_ = *stream++;                        // идентификатор вызова
    id_ |= *stream++ << 8;
    id_ |= *stream++ << 16;
    id_ |= *stream++ << 24;

    message_ = *stream++;                    // мессага
    size -= 6;

    if (size > MAXSIZEDATAMON) 
        return NULL;
    memcpy(data_, stream, size);
    size_ = size;

    return stream+size;
}
        
//
// ЗАПИСЬ
void CMonMessageEx::addParameterString(const char* s)
{
    int l = strlen(s)+1;
    memcpy(data_+size_, s, l);  //strcpy((char*)data_+size_, s); 
    size_ += l;
}

void CMonMessageEx::addParameterTime(const TClock* cl)
{
    memcpy(data_+size_, cl, sizeof(TClock)); 
    size_ += sizeof(TClock);
}


//
// ЧТЕНИЕ
const char*   CMonMessageEx::getParameterStringPtr(int& readPos) const
{
    const char* s = (char*)(data_+readPos); 
    readPos += strlen(s)+1; 
    return s;
}

void CMonMessageEx::getParameterTime(TClock* cl, int& readPos) const
{
    memcpy(cl, data_+readPos, sizeof(TClock));
    readPos += sizeof(TClock); 
}
const TClock*  CMonMessageEx::getParameterTimePtr(int& readPos) const
{
    const TClock* cl = (const TClock*)(data_+readPos);
    readPos += sizeof(TClock); 
    return cl;
}

#ifndef  _ATS_
bool  CMonMessageEx::monMessageToText(char* pTextBuffer, int iTextBufferSize) const
{
    pTextBuffer[0] = 0;
    int readPos = 0;    // clearReadPosition();
    char sInfo[1000];
    switch(message())
    {
    case MON_ALIVE:
        {
            BYTE btMod = getParameterByte(readPos);
            const TClock* pClock=getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  ALIVE        (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                id(),time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
    case MON_COMOVERLOAD:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock=getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  COMOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                id(),time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
    case MON_WIZOVERLOAD:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock=getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  WIZOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                id(),time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
    case MON_FATOVERLOAD:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock=getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  FATOVERLOAD  (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                id(),time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
    case MON_TIMECHANGE:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock=getParameterTimePtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  TIMECHANGE   (%02d)  [%02d-%02d-20%02d %02d:%02d:%02d]",
                id(),time(),btMod,pClock->Date,pClock->Month,pClock->Year,
                pClock->Hours,pClock->Minutes,pClock->Seconds);
        }
        break;
    case MON_INT_CALL_OUT:
        {
            BYTE btMod=getParameterByte(readPos);
            BYTE btPcm=getParameterByte(readPos);
            BYTE btKI=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_CALL_OUT  (%02d:%02d:%02d)",
                        id(),time(),btMod,btPcm,btKI);
        }
        break;
    case MON_INT_CALL_IN:
        {
            BYTE btMod=getParameterByte(readPos);
            BYTE btPcm=getParameterByte(readPos);
            BYTE btKI=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_CALL_IN   (%02d:%02d:%02d)",
                        id(),time(),btMod,btPcm,btKI);
        }
        break;
    case MON_INT_RLS_OUT:
        {
            BYTE btMod=getParameterByte(readPos);
            BYTE btPcm=getParameterByte(readPos);
            BYTE btKI=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_RLS_OUT   (%02d:%02d:%02d)",
                        id(),time(),btMod,btPcm,btKI);
        }
        break;
    case MON_INT_RLS_IN:
        {
            BYTE btMod=getParameterByte(readPos);
            BYTE btPcm=getParameterByte(readPos);
            BYTE btKI=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_INT_RLS_IN    (%02d:%02d:%02d)",
                        id(),time(),btMod,btPcm,btKI);
        }
        break;
    case MON_LINK_STATE:
        {
            BYTE btPcmBeq=getParameterByte(readPos);
            btPcmBeq ++;
            BYTE btPcmQnt=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_LINK_STATE    [%02d:%02d]  ",
                        id(),time(),btPcmBeq,btPcmBeq+btPcmQnt-1);
            BYTE param[4];

            for(int  i = 0; i < btPcmQnt; i++){
                for(int j = 0; j < 4; j++) {
                    param[j] =  getParameterByte(readPos);
                }
                char s[200];
                sprintf(s, "(%02d:%02d:%02d:%02d) ", param[0],param[1],param[2],param[3]);
                strcat(sInfo, s);
            }
        }
        break;
    case MON_PORT_FREE:
        {
            BYTE btMod=getParameterByte(readPos);
            BYTE btPcm=getParameterByte(readPos);
            BYTE btKI=getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_PORT_FREE     (%02d:%02d:%02d)",
                        id(),time(),btMod,btPcm,btKI);
        }
        break;
    case MON_DVO_REDIRECT:
        {
            BYTE btDvocode=getParameterByte(readPos);
            const char* pNumFrom = getParameterStringPtr(readPos);
            const char* pNumTo = getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  MON_DVO_REDIRECT  dvo:%03d num_from:%s num_to:%s",
                        id(),time(),btDvocode,pNumFrom[0]?pNumFrom:"-",pNumTo[0]?pNumTo:"-");
        }
        break;
    case MON_HOLDEND:
        {
            sprintf(sInfo, "%08X  (%03d)  MON_HOLDEND",id(),time());
        }
        break;
    case MON_SEIZURE:
        {
            BYTE btSig=getParameterByte(readPos);
            BYTE btMod=getParameterByte(readPos);
            BYTE btSlotPcm=getParameterByte(readPos);
            BYTE btPort=getParameterByte(readPos);
            const char* pNum=getParameterStringPtr(readPos);
            const char* pAon=getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  SEIZURE      %s  %s %s  (%02d:%02d:%02d)",
                        id(),time(),getSignallingText(btSig),pNum[0]?pNum:"-",pAon[0]?pAon:"-",btMod,btSlotPcm,btPort);
        }
        break;
    case MON_NUMBER:
        {
            const char* pNum=getParameterStringPtr(readPos);
            const char* pAon=getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  NUMBER       %s %s",
                id(),time(),pNum[0]?pNum:"-",pAon[0]?pAon:"-");
        }
        break;
    case MON_ACCEPT:
            sprintf(sInfo, "%08X  (%03d)  ACCEPT       <%08X>",
                id(),time(),getParameterDWord(readPos));
        break;
    case MON_CALL:
        {
            DWORD dwRemID=getParameterDWord(readPos);
            BYTE btSig=getParameterByte(readPos);
            BYTE btMod=getParameterByte(readPos);
            BYTE btSlotPcm=getParameterByte(readPos);
            BYTE btPort=getParameterByte(readPos);
            const char* pNum=getParameterStringPtr(readPos);
            const char* pAon=getParameterStringPtr(readPos);
            sprintf(sInfo, "%08X  (%03d)  CALL         %s  %s %s  (%02d:%02d:%02d)  <%08X>",
                        id(),time(),getSignallingText(btSig),pNum[0]?pNum:"-",pAon[0]?pAon:"-",btMod,btSlotPcm,btPort,dwRemID);
        }
        break;
    case MON_ANSWER:
            sprintf(sInfo, "%08X  (%03d)  ANSWER       ",id(),time());
        break;
    case MON_RELEASE_IN:
        {// reason:BYTE
            BYTE btReason = getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  RELEASE_IN   (%03d)",id(),time(),btReason);
        }
        break;
    case MON_RELEASE_OUT:
        {
            // reason:BYTE
            BYTE btReason = getParameterByte(readPos);
            sprintf(sInfo, "%08X  (%03d)  RELEASE_OUT  (%03d)",id(),time(),btReason);
        }
        break;
    case MON_COMBINE:
            sprintf(sInfo, "%08X  (%03d)  COMBINE      <%08X>",id(),time(),getParameterDWord(readPos));
        break;
    case MON_SPIDER_START:
        {
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_START        [%02d-%02d-20%02d %02d:%02d:%02d]",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_MODULE_UP:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_MODULE_UP    [%02d-%02d-20%02d %02d:%02d:%02d]  mod:%02d",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds, btMod);
        }
        break;
    case MON_SPIDER_MODULE_DOWN:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_MODULE_DOWN  [%02d-%02d-20%02d %02d:%02d:%02d]  mod:%02d",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds, btMod);
        }
        break;
    case MON_SPIDER_TCP_UP:
        {
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_TCP_UP       [%02d-%02d-20%02d %02d:%02d:%02d]",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_TCP_DOWN:
        {
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_TCP_DOWN     [%02d-%02d-20%02d %02d:%02d:%02d]",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_BAD_PACKET:
        {
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_BAD_PACKET   [%02d-%02d-20%02d %02d:%02d:%02d]",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_GATE_LOST:
        {
            BYTE btMod=getParameterByte(readPos);
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_GATE_LOST    [%02d-%02d-20%02d %02d:%02d:%02d]  mod:%02d",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds, btMod);
        }
        break;
    case MON_SPIDER_USER_STOP:
        {
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_USER_STOP    [%02d-%02d-20%02d %02d:%02d:%02d]",
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_INFO:
        {
            DWORD dwOSver = getParameterDWord(readPos);
            DWORD dwSPIDERver = getParameterDWord(readPos);
            char sSpiderVer[100];
            sprintf(sSpiderVer, "%d.%d.%d.%d", (dwSPIDERver >> 24) & 0xFF,
                                               (dwSPIDERver >> 16) & 0xFF,
                                               (dwSPIDERver >> 8) & 0xFF,
                                               dwSPIDERver & 0xFF);
            const TClock* pClock = getParameterTimePtr(readPos);
            sprintf(sInfo, "SPIDER_INFO         OS:%08X Spider:%s  [%02d-%02d-20%02d %02d:%02d:%02d]",dwOSver, sSpiderVer,
                pClock->Date, pClock->Month, pClock->Year, pClock->Hours, pClock->Minutes, pClock->Seconds);
        }
        break;
    case MON_SPIDER_INFO_EX:
        {
            const char* OSver = getParameterStringPtr(readPos);
            DWORD dwSPIDERver = getParameterDWord(readPos);
            TClock clock = *(getParameterTimePtr(readPos));

            char sSpiderVer[500];
            sprintf(sSpiderVer, "%d.%d.%d.%d", (dwSPIDERver >> 24) & 0xFF,
                                               (dwSPIDERver >> 16) & 0xFF,
                                               (dwSPIDERver >> 8) & 0xFF,
                                               dwSPIDERver & 0xFF);
            sprintf(sInfo, "SPIDER_INFO_EX      OS:%s Spider:%s  [%02d-%02d-20%02d %02d:%02d:%02d]", OSver, sSpiderVer,
                    clock.Date,clock.Month,clock.Year,clock.Hours,clock.Minutes,clock.Seconds);

        }
        break;
    default:
        sprintf(sInfo, "%08X  (%03d)  UNKNOWN_MES(%d)", id(), time(), message());
    }

    if(iTextBufferSize < (int)strlen(sInfo)+1)
        return false;

    strcpy(pTextBuffer, sInfo);
    return true;
}
#endif
