#ifndef _NETMES_H_
#define _NETMES_H_

#include <type.h>

#define MAX_MODULE 64

//
#define NET_MES_REGISTER_TERMINAL       0x01    // шлется каждые 5 сек, иначе unregister
#define NET_MES_REGISTER_MONITOR        0x02    // шлется каждые 5 сек, иначе unregister
#define NET_MES_REGISTER_ALARM          0x03    // шлется каждые 5 сек, иначе unregister
//#define NET_MES_REGISTER_PREPAY ??            // шлется каждые 5 сек, иначе unregister

#define NET_MES_HELLO                   0x04

#define NET_MES_TERMINALSTRING          0x05

#define NET_MES_CONFSTARTLOAD           0x06
#define NET_MES_CONFBLOCKLOAD           0x07
#define NET_MES_CONFFINISHLOAD          0x08
#define NET_MES_CONFACCEPT              0x09
#define NET_MES_CONFERROR               0x0A

#define NET_MES_BINSTARTLOAD            0x0B
#define NET_MES_BINBLOCKLOAD            0x0C
#define NET_MES_BINFINISHLOAD           0x0D
#define NET_MES_BINACCEPT               0x0E
#define NET_MES_BINERROR                0x0F

#define NET_MES_ROUTING                 0x10
#define NET_MES_REJECTMODUL             0x11

#define NET_MES_TEST                    0x12

#define NET_MES_WIDEUPDATE              0x13

#define NET_MES_WIDEHIMESSAGE           0x14

#define NET_MES_SETNUMBER               0x15

#define NET_MES_LOGSTART                0x16
#define NET_MES_LOGEND                  0x17
#define NET_MES_LOGACCEPT               0x18
#define NET_MES_LOGBLOCK                0x19
#define NET_MES_LOGFINISH               0x1A

#define NET_MES_CONFBACK_GET            0x1B
#define NET_MES_CONFBACK_TAKE           0x1C

#define NET_MES_TUNE_SET                0x1D
#define NET_MES_TUNE_SETOK              0x1E
#define NET_MES_TUNE_SETERROR           0x1F

#define NET_MES_TUNE_GET                0x20
#define NET_MES_TUNE_GETOK              0x21
#define NET_MES_TUNE_GETERROR           0x22

#define NET_MES_CONFEXPORT              0x23

#define NET_MES_MONITOR                 0x24    // ATS->PC(tarif) ATS->ATS(sorm)

#define NET_MES_TESTEXT_GET             0x25
#define NET_MES_TESTEXT_TAKE            0x26

#define NET_MES_ALARM                   0x27    // BYTE:code ...:param  // ATS->PC(alarm)

#define NET_MES_REJECTVIRCOM            0x28

#define NET_MES_INTELLECTROUTING_MODULE 0x29

#define NET_MES_LINK_STATE              0x2A    // Сообщить состояние потока MON<->ATS
#define NET_MES_PORT_STATE              0x2B    // Сообщить состояние портов MON<->ATS

#define NET_MES_EXTRIMTARIF_GETWIDE     0x2C    // Широковещательный запрос тарификации (раз в 15 сек)
#define NET_MES_EXTRIMTARIF_GETPOINT    0x2D    // Точечный запрос тарификации, по каждому ответу
#define NET_MES_EXTRIMTARIF_TAKE        0x2E    // Ответная тарификация (несколько строк)

#define NET_MES_SORM_OVERHEAR           0x2F    // Запрос на подслушивание (кого:DWORD back1:DWORD back2:DWORD)
#define NET_MES_SORM_OVERLOAD           0x30    // Запрос не выполнен      (кого:DWORD back1:DWORD back2:DWORD)

//#define NET_MES_FREQ_START         49 // Подача частоты абоненту
//#define NET_MES_FREQ_STOP          50 // Снятие частоты абоненту

#define NET_MES_TRANSIT_PORT            0x33    // транзитная доставка сообщения от писюка/атс к порту

#define NET_MES_SYNCHRO                 0x34

#define NET_MES_PCPHONE                 0x35

#define NET_MES_CALLBACKACTIVATE        0x36    // Старт процедуры callback

#define NET_MES_PCMSTATUS               0x37    // доставка pcmstatus до писюка

#define NET_MES_SETPASSWORD             0x38    // установка пароля, 6 симв пароля

#define NET_MES_REGISTER_PREPAY         0x39    // PC -> ATS шлется каждые 5 сек, иначе unregister
#define NET_MES_PREPAY_NUMBER_COMPLETE  0x3A    // ATS  -> PC
#define NET_MES_PREPAY_TALK_BEGIN       0x3B    // ATS  -> PC
#define NET_MES_PREPAY_TALK_END         0x3C    // ATS  -> PC

// Мессага для упсов
#define NET_MES_UPS_DATA                0x3D    // ATS  -> PC (двухсторонняя)

#define NET_MES_SHUTDOWN                0x3E    // плавное выключение АТС
#define NET_MES_GETJURNAL               0x3F    // выкачка журнала

// Мессага о состоянии CF
// Через  возвращается процент свободного пространства на диске
// Мессага отправляется один раз в час
#define NET_MONITOR_DISK_INFO           0x40    // ATS  ->PC 1байт шлется каждые 10мин

#define NET_MES_REGISTER_MONITOR_INL    0x41    // регистрация монитора модулем

#define NET_MES_CONTROL_E08_COST        0x42    // контроль стоимости е08

#define NET_MES_FTP_OVER_SCOMM          0x43    // FTP через SComm

#define NET_MES_ELKOM                   0x44    // Протокол Элком

#define NET_MES_TSF_FOR_SORM            0x45    // tsf в сорм для эмуляции
#define NET_MES_REGISTER_SORM_PULT      0x46
#define NET_MES_MESS_TO_SORM            0x47
#define NET_MES_MESS_TO_PULT_CH1        0x48
#define NET_MES_MESS_TO_PULT_CH2        0x49

// Система авторизации
#define NET_MES_AUTHORIZATION_SYSTEM     0x4F

// ВНИМАНИЕ: ЗАПРЕЩАЕТСЯ МЕНЯТЬ НОМЕРА СУЩЕСТВУЮЩИХ СООБЩЕНИЙ
// ТАК-ЖЕ МЕНЯТЬ ИХ ФОРМАТ
// НОВЫЕ СООБЩЕНИЯ ДОБАВЛЯТЬ ТОЛЬКО С НАРАСТАЮЩИМИ НОМЕРАМИ


#pragma pack (push, 1)

struct TNetPoint
{
    BYTE nMod; // если == 0xFF то широковещание если == 0xFE то соседнему модулю
    char nVir; // если == -1 то АТС иначе PC
} ;

class CNetSetPoint
{
public:
    BYTE bitSet[MAX_MODULE / 8];

    void Clear();
    void Full();
    void SetIndex(short ind);
    void ResetIndex(short ind);
    bool CheckIndex(short ind);
};

class CNetMessage
{
public:
    TNetPoint dst;      // [1 + 1]
    TNetPoint src;      // [1 + 1]
    short nMessage;     // [2]
    char gate;          // [1] заполняется принимателем, для КОМ порта = -1
    union
    {
        struct
        {
            BYTE data[1];       // [1...] массив
        } single;
        struct
        {
            CNetSetPoint set;   //  [8] 8байтовый массив
            BYTE data[1];       //  [1..] массив
        } multi;
    } un;


    CNetMessage() {
        memset(this, 0x00, sizeof(*this));
    }
    const uc* decode(const uc* stream, int& size);
    uc* encode(uc* stream);


#ifndef ARMLINUX
    short nHead( void ) { return  7 + (dst.nMod == 0xFF ? sizeof(CNetSetPoint) : 0); }
    BYTE* pData( void ) { return  dst.nMod == 0xFF ? &un.multi.data[0] : &un.single.data[0]; }
#endif

    void fillReply(CNetMessage* mes, TModule srcModule);
    void fillWide(TModule srcModule);
    void fillNear(TModule srcModule);
};


const   int     Max_NetMessageDataSize =  2*100 + sizeof(BYTE) + sizeof(TCallID) + sizeof(BYTE);
class  CNetMessageBody : public CNetMessage 
{
public:
    int     dataSize;
    uc      data[Max_NetMessageDataSize];

public:
    CNetMessageBody() {
        dataSize = 0;
        memset(data, 0x00, sizeof(data));
    }

    const uc* decode(const uc* stream, int size);
    uc* encode(uc* stream);
    const uc* input(const uc* stream, int size);
};



#pragma pack (pop)

#endif
