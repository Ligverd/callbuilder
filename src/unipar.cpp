#include  <stdio.h>
#include  <string.h>

#include  "callbuilder.h"
#include  "unipar.h"
#include  "client.h"
#include  "UniNumber.h"



#if MAX_SIZE_PARAMETER > MAXSIZEONEPACKET
    #error
#endif

void CUniPar::clear ( void )
{
    Len = 0;
    Signalling = SIGNALLING_NONE;
}

void CUniPar::clear ( TSignalling sig)
{
    clear();
    setSignalling ( sig );
}

void CUniPar::setSignalling ( TSignalling sig )
{
    Signalling = sig;
}

void CUniPar::addByte ( TUniPar uniPar, BYTE data )
{
    if (Len + sizeof(BYTE) + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = sizeof(BYTE);

    *(BYTE*)&Buf[Len] = data;
    Len += sizeof(BYTE);
}

void CUniPar::addWord ( TUniPar uniPar, WORD data )
{
    if (Len + sizeof(WORD) + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = sizeof(WORD);

    *(WORD*)&Buf[Len] = data;
    Len += sizeof(WORD);
}

void CUniPar::addDWord ( TUniPar uniPar, DWORD data )
{
    if (Len + sizeof(DWORD) + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = sizeof(DWORD);

    *(DWORD*)&Buf[Len] = data;
    Len += sizeof(DWORD);
}

void* CUniPar::addBuffer(TUniPar uniPar, const void* buf, BYTE len )
{
    if (Len + len + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return NULL;
    }
    Buf[Len++] = (BYTE)uniPar;
    Buf[Len++] = len;

    memcpy(Buf+Len, buf, len);
    Len += len;

    return Buf + Len-len;    // except uniPar and len
}

void CUniPar::addString ( TUniPar uniPar, const char *str )
{
    if (uniPar == UNIPAR_NUMBER || uniPar == UNIPAR_AON)
    {
        UniN::Number num; // uniPar == UNIPAR_NUMBER ? UniN::NbType_CalledPartyNumber : UniN::NbType_CallingPartyNumber);
        num.setNumber(str, false);
        addNumber(uniPar == UNIPAR_NUMBER ? UNIPAR_CalledNumber : UNIPAR_CallingNumber, &num);
        //return; // пока пусть останется для совместимости со старыми версиями
    }

    BYTE len = strlen(str) + 1;
    if (Len + len + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = len;

    memcpy(Buf + Len, str, len);
    Len += len;
    Buf[Len - 1] = 0;
}

void CUniPar::addFlag ( TUniPar uniPar )
{
    if (Len + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = 0;

    if (uniPar == UNIPAR_NUMBER_COMPLETE)
    {
        int i;
        for (i = 0; i < Len && i < MAX_SIZE_PARAMETER; i += (Buf[i+1] + 2))
        {
            if (((TUniPar)Buf[i]) == UNIPAR_CalledNumber)
            {
                i += 2;
                ((UniN::Number*)&Buf[i])->numberComplete = true;
                break;
            }
        }
    }
}

void CUniPar::addNumber(TUniPar uniPar, const UniN::Number& num)
{
    BYTE len = num.realSize();
    if (Len + len + 2 >= MAX_SIZE_PARAMETER)
    {
        WARNING
        return;
    }
    Buf[Len ++] = (BYTE)uniPar;
    Buf[Len ++] = len;

    memcpy(Buf + Len, (void*)&num, len);
    Len += len;
    Buf[Len - 1] = 0;
}

bool CUniPar::addUni ( TUniPar uniPar, CUniPar* up, TUniPar uniParNew)
{

    BYTE rl;
    const BYTE* bb = up->getBufferPtr(uniPar, rl);
    if (bb)
    {
        addBuffer(uniParNew ? uniParNew : uniPar, bb, rl);
        return true;
    }
    else
        return false;
}

void CUniPar::remove ( TUniPar uniPar )
{
    int i;
    for (i = 0; i < Len && i < MAX_SIZE_PARAMETER; i += (Buf[i+1] + 2))
    {
        if (((TUniPar)Buf[i]) == uniPar)
        {
            int size = Buf[i+1] + 2;
            Len -= size;
            memcpy(Buf + i, Buf + i + size, Len - i);
            return;
        }
    }
    if (i != Len)
        WARNING
}

CUniPar* CUniPar::create ( void )
{
    CUniPar* ret = (CUniPar*)malloc(sizeof(short) + sizeof(TSignalling) + Len);
    ret->Len = Len;
    ret->Signalling = Signalling;
    memcpy(ret->Buf, Buf, Len);
    return ret;
}

CUniPar* CUniPar::create ( const BYTE* data )
{
    CUniPar* src = (CUniPar*)data;
    CUniPar* ret = (CUniPar*)malloc(src->Len + sizeof(TSignalling) + sizeof(short));
    ret->Len = src->Len;
    ret->Signalling = src->Signalling;
    memcpy(ret->Buf, src->Buf, src->Len);
    return ret;
}

// Чтение
TSignalling CUniPar::getSignalling ( void ) const
{
    return Signalling;
}

bool CUniPar::isPresent ( TUniPar uniPar ) const
{
    if (uniPar == UNIPAR_NUMBER && getPos(UNIPAR_CalledNumber) >= 0)
        return true;
    if (uniPar == UNIPAR_AON && getPos(UNIPAR_CallingNumber) >= 0)
        return true;
    return (getPos(uniPar) >= 0);
}

bool CUniPar::getByte ( TUniPar uniPar, BYTE& data ) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        data = *(BYTE*)&Buf[pos + 2];
        return true;
    }
    else
        return false;
}

bool CUniPar::getWord ( TUniPar uniPar, WORD& data ) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        data = *(WORD*)&Buf[pos + 2];
        return true;
    }
    else
        return false;
}

bool CUniPar::getDWord ( TUniPar uniPar, DWORD& data ) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        data = *(DWORD*)&Buf[pos + 2];
        return true;
    }
    else
        return false;
}

bool CUniPar::getBuffer ( TUniPar uniPar, BYTE *buf, BYTE &len, BYTE maxlen ) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        len = Buf[pos + 1];
        if (len > maxlen)
        {
            WARNING
            return false;
        }
        memcpy(buf, &Buf[pos + 2], len);
        return true;
    }
    else
        return false;
}

const BYTE* CUniPar::getBufferPtr ( TUniPar uniPar, BYTE &len ) const
{
    int pos = getPos(uniPar);
    len = 0;
    if (pos >= 0)
    {
        len = Buf[pos + 1];
        return &Buf[pos + 2];
    }
    else
        return NULL;
}
const void* CUniPar::getBufferPtr(TUniPar uniPar) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        return &Buf[pos + 2];
    }
    else
    {
        return NULL;
    }
}

bool CUniPar::getFixBuffer(TUniPar uniPar, void* buf, BYTE maxlen) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        BYTE len = *(Buf+pos+1);
        if (len != maxlen)
        {
            WARNING
            return false;
        }
        memcpy(buf, Buf+pos+2, len);
        return true;
    }
    else
        return false;
}

bool CUniPar::getString ( TUniPar uniPar, char *str, BYTE maxlen ) const
{
    const UniN::Number* num = NULL;
    if (uniPar == UNIPAR_NUMBER)
        num = getNumberPtr(UNIPAR_CalledNumber);
    if (uniPar == UNIPAR_AON)
        num = getNumberPtr(UNIPAR_CallingNumber);
    if (num)
    {
        BYTE len = strlen(num->number);
        if (len > maxlen)
        {
            WARNING
            return false;
        }
        strcpy(str, num->number);
        return true;
    }

    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        BYTE len = Buf[pos + 1];
        if (len > maxlen+1)
        {
            WARNING
            return false;
        }
        memcpy(str, &Buf[pos + 2], len);
        str[len - 1] = 0;
        return true;
    }
    else
    {
        str[0] = 0;
        return false;
    }
}

const char* CUniPar::getStringPtr ( TUniPar uniPar ) const
{
    const UniN::Number* num = NULL;
    if (uniPar == UNIPAR_NUMBER)
        num = getNumberPtr(UNIPAR_CalledNumber);
    if (uniPar == UNIPAR_AON)
        num = getNumberPtr(UNIPAR_CallingNumber);
    if (num)
        return num->number;

    short pos = getPos(uniPar);
    if (pos >= 0)
    {
        return (const char*)&Buf[pos + 2];
    }
    else
    {
        return "";
    }
}

bool CUniPar::getFlag ( TUniPar uniPar ) const
{
    const UniN::Number* num = NULL;
    if (uniPar == UNIPAR_NUMBER_COMPLETE)
        num = getNumberPtr(UNIPAR_CalledNumber);
    if (num)
        return num->numberComplete;

    return isPresent(uniPar);
}

bool CUniPar::getNumber ( TUniPar uniPar, UniN::Number& num ) const
{
    int pos = getPos(uniPar);
    if (pos >= 0)
    {
        BYTE len = Buf[pos + 1];
        memcpy(&num, &Buf[pos + 2], len);
        return true;
    }
    else
    {
        num.clearNumber();
        return false;
    }
}

const UniN::Number* CUniPar::getNumberPtr ( TUniPar uniPar )  const
{
    short pos = getPos(uniPar);
    if (pos >= 0)
    {
        return (const UniN::Number*)&Buf[pos + 2];
    }
    else
    {
        return NULL;
    }
}


short CUniPar::getLen() const
{
    return sizeof(TSignalling) + sizeof(short) + Len;
}

short CUniPar::getPos ( TUniPar uniPar ) const
{
    int i;
    for (i = 0; i < Len && i < MAX_SIZE_PARAMETER; i += (Buf[i+1] + 2))
    {
        if (((TUniPar)Buf[i]) == uniPar)
            return i;
    }
    if (i != Len)
        WARNING
    return -1;
}


// Шаримся по параметрам
const void* CUniPar::getFirstPtr() const
{
    if (Len)
        return Buf;
    else
        return NULL;
}
const void* CUniPar::getNextPtr(const void* paramPtr) const
{
    if (!paramPtr) return NULL;
    if ((paramPtr < Buf) || (paramPtr >= Buf+Len)) { WARNING; return NULL; }

    BYTE* p = (BYTE*)paramPtr;
    p += (*(p+1) +2);
    if (p == Buf+Len) return NULL;       // end

    if ((p < Buf) || (p > Buf+Len)) { WARNING; return NULL; }

    return p;
}


// Коды DVO
const char* getDVOCodeText(unsigned char DVOCode)
{
    switch(DVOCode) {
        case DVOCode_TALK               : return "TALK";                // Normal talk
        case DVOCode_CFU                : return "CFU";                 // Call forwarding unconditional
        case DVOCode_CFB                : return "CFB";                 // Call forwarding on subscriber busy
        case DVOCode_CFNRY              : return "CFNRY";               // Call forwarding on no reply
        case DVOCode_AllCF              : return "AllCF";               // Call call forwarding
        case DVOCode_CW                 : return "CW";                  // Call waiting
        case DVOCode_HOLD               : return "HOLD";                // Call hold
        case DVOCode_CCBS               : return "CCBS";                // Completion of call to busy subscribers
        case DVOCode_HOLE               : return "HOLE";                // HOLD end
        case DVOCode_3PTY               : return "3PTY";                // 3-party conference
        case DVOCode_CT                 : return "CT";                  // Call transfer
        case DVOCode_CP                 : return "CP";                  // Call pickup
        case DVOCode_CC                 : return "CC";                  // Consultation call
        case DVOCode_3PTYcall1          : return "3PTYcall1";           //
        case DVOCode_3PTYcall2          : return "3PTYcall2";           //
        case DVOCode_3PTYpreRelease     : return "3PTYpreRelease";      //
        case DVOCode_HoldEndMessageless : return "HoldEndMessageless";  // Возврат с удержания без нотификации holdend
        case DVOCode_ReleasedByDVO      : return "ReleasedByDVO";       // Значит что дальнейшие релизы идут по коду отбора DVO
        case DVOCode_Messageless        : return "Messageless";         // Вызов без сормовских нотификацй        case 
        default                         : return "UNKNOWN";
    }                                   
}
