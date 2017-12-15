#ifndef  __UNIPAR_H__
#define  __UNIPAR_H__

#include <include/type.h>

namespace UniN {
struct Number;
};

#define  MAX_SIZE_PARAMETER 300

//
// Типы сигнализация на портовых блоках
//
const   TSignalling     SIGNALLING_NONE             =  0;
const   TSignalling     SIGNALLING_EXT              =  1;
const   TSignalling     SIGNALLING_DSS1             =  2;
const   TSignalling     SIGNALLING_CAS2_IN          =  3;
const   TSignalling     SIGNALLING_CAS2_OUT         =  4;
const   TSignalling     SIGNALLING_V5_ANSA          =  5;       // AN Subscriber adapter
const   TSignalling     SIGNALLING_V5_ANPSTN        =  6;       // AN PSTN protocol
const   TSignalling     SIGNALLING_V52AN_PCMPORT    =  7;       // v52 pcm
const   TSignalling     SIGNALLING_V52AN_SUBPORT    =  8;       // v52 sub(real to hardware)
const   TSignalling     SIGNALLING_V52LE_PCMPORT    =  9;       // v52 pcm
const   TSignalling     SIGNALLING_V52LE_SUBPORT    = 10;       // v52 sub (virtual to ext)
const   TSignalling     SIGNALLING_CITY             = 11;       // 2-ух проводка
const   TSignalling     SIGNALLING_CAS1             = 12;       // 1 ВСК на 4/6 проводке и ИКМ
const   TSignalling     SIGNALLING_SORM             = 13;       // SORM-KGB
const   TSignalling     SIGNALLING_DISA             = 14;
const   TSignalling     SIGNALLING_SERIAL           = 15;
const   TSignalling     SIGNALLING_ISUP             = 16;
const   TSignalling     SIGNALLING_SPEAKER          = 17;
const   TSignalling     SIGNALLING_PERMANENT        = 18;       // постоянная коммутация каналов
const   TSignalling     SIGNALLING_3SL_IN           = 19;       // 3-х проводка входяща
const   TSignalling     SIGNALLING_3SL_OUT          = 20;       // 3-х проводка исходяща
const   TSignalling     SIGNALLING_2600             = 21;       // сигнализация 2600
const   TSignalling     SIGNALLING_DSS1SUBSCRIBER   = 22;       // Порт абонентского выноса по DSS1
const   TSignalling     SIGNALLING_CAS_TEST         = 23;       // Tester
const   TSignalling     SIGNALLING_INT              = 24;       // Внутренние лини
const   TSignalling     SIGNALLING_CONTROL          = 25;       // Контрольный порт
const   TSignalling     SIGNALLING_MTP              = 26;       // Сигнализация для внутреннего обмена по MTP
const   TSignalling     SIGNALLING_ISDN             = 27;
const   TSignalling     SIGNALLING_CALLGEN          = 28;       // Генератор вызовов
const   TSignalling     SIGNALLING_ADASE            = 29;       // Сигнализация АДАСЭ 1200&1600
const   TSignalling     SIGNALLING_CAS1_NORKA_OUT   = 30;       // 1 ВСК "норка" на ИКМ, исх
const   TSignalling     SIGNALLING_CAS1_NORKA_IN    = 31;       // 1 ВСК "норка" на ИКМ, вх
const   TSignalling     SIGNALLING_CALLBACK         = 32;       // Обратный вызов
const   TSignalling     SIGNALLING_BUTTON           = 33;
const   TSignalling     SIGNALLING_ADASEPULT        = 34;
const   TSignalling     SIGNALLING_2100             = 35;       // Сигнализация 2100 для АДАСЭ
const   TSignalling     SIGNALLING_EM               = 36;
const   TSignalling     SIGNALLING_V52AN_ISDNPORT   = 37;
const   TSignalling     SIGNALLING_V52LE_ISDNPORT   = 38;       // v52 sub (virtual to isdn_ext)
const   TSignalling     SIGNALLING_MAX              = 39;
const char* getSignallingText(TSignalling  sig);
TSignalling getSignallingBin(const char* sig);      // обратно

//
// Типы ИКМ-потоков (для разных сигнализаций)
//
const   short   PCM_NONE                            =  0;
const   short   PCM_CAS                             =  1;
const   short   PCM_DSS1                            =  2;
const   short   PCM_V5AN                            =  3;
const   short   PCM_V5LE                            =  4;
const   short   PCM_V52AN                           =  5;
const   short   PCM_V52LE                           =  6;
const   short   PCM_SORM                            =  7;
const   short   PCM_MTP                             =  8;
const   short   PCM_PERMANENT                       =  9;
const   short   PCM_INT                             = 10;
const   short   PCM_MAX                             = 11;

const   short   SLOT_NONE                           =  0;
const   short   SLOT_A16                            =  1;
const   short   SLOT_C88                            =  2;
const   short   SLOT_C68                            =  3;
const   short   SLOT_A84                            =  4;
const   short   SLOT_I38                            =  5;  // 3-х проводки исходящие
const   short   SLOT_V38                            =  6;  // 3-х проводки входящие
const   short   SLOT_A08                            =  7;  // ISDN
const   short   SLOT_C48                            =  8;
const   short   SLOT_C412                           =  9;  // Это станция C412
const   short   SLOT_A8                             = 10; // Абонентская на 8 абонентов для офисной станции
const   short   SLOT_C44                            = 11; // Двух проводка для офисной 4 абонента 4 СЛки
const   short   SLOT_C16                            = 12; // Цифровой системник
const   short   SLOT_C08                            = 13; // Цифровой системник для офисной станции
const   short   SLOT_MAX                            = 14;

const   short   AIR_NONE                            =  0;
const   short   AIR_EXTV52                          =  1;
const   short   AIR_DISA                            =  2;
const   short   AIR_SERIAL                          =  3;
const   short   AIR_DSS1SUBSCRIBER                  =  4;
const   short   AIR_ISDN                            =  5;
const   short   AIR_UNIFIED                         =  6;
const   short   AIR_BUTTON                          =  7;
const   short   AIR_MODULE                          =  8;
const   short   AIR_RECORDPROP                      =  9;
const   short   AIR_CALLBACK                        = 10;

//
//
const   TUniPar UNIPAR_NONE                                 = 0x00;
const   TUniPar UNIPAR_NUMBER                               = 0x01;   // номер вызываемого аб
const   TUniPar UNIPAR_NUMBER_COMPLETE                      = 0x02;   // если номые полный
const   TUniPar UNIPAR_AON                                  = 0x03;   // номер вызывающего аонента
const   TUniPar UNIPAR_CATEGORY                             = 0x04;   // категори
const   TUniPar UNIPAR_CALLID                               = 0x05;   // идентификатор вызова
// V5
const   TUniPar UNIPAR_V5AN_LINE_SIGNAL                     = 0x06;
// для передачи сигнала по проводам
const   TUniPar UNIPAR_SIGNAL                               = 0x07;
const   TUniPar UNIPAR_DATA                                 = 0x08;
// Жопуслуги
const   TUniPar UNIPAR_CLIP                                 = 0x09;
const   TUniPar UNIPAR_CLIR                                 = 0x0A;
const   TUniPar UNIPAR_COLP                                 = 0x0B;
const   TUniPar UNIPAR_COLR                                 = 0x0C;
// DSS1
const   TUniPar UNIPAR_DSS1_BearerCapability                = 0x10;   // параметр от DSS1
const   TUniPar UNIPAR_DSS1_ProgressIndicator               = 0x11;   // параметр от DSS1
const   TUniPar UNIPAR_DSS1_ChannelIdentification           = 0x12;
const   TUniPar UNIPAR_DSS1_CallingPN                       = 0x13;
const   TUniPar UNIPAR_DSS1_CallingPartySubaddress          = 0x14;
const   TUniPar UNIPAR_DSS1_CalledPN                        = 0x15;
const   TUniPar UNIPAR_DSS1_Cause                           = 0x16;
const   TUniPar UNIPAR_DSS1_KeypadFacility                  = 0x17;
const   TUniPar UNIPAR_DSS1_LowLayerCompatibility           = 0x18;
const   TUniPar UNIPAR_DSS1_HighLayerCompatibility          = 0x19;
//const   TUniPar UNIPAR_DSS1_ConnectedNumber                 = 0x1A;
const   TUniPar UNIPAR_DSS1_ConnectedSubaddress             = 0x1B;
const   TUniPar UNIPAR_DSS1_Display                         = 0x1C;
const   TUniPar UNIPAR_DSS1_UserUser                        = 0x1D;
const   TUniPar UNIPAR_DSS1_Facility                        = 0x1E;
const   TUniPar UNIPAR_DSS1_Billing                         = 0x1F;
const   TUniPar UNIPAR_DSS1_DateTime                        = 0x20;
const   TUniPar UNIPAR_DSS1_NotificationIndicator           = 0x21;
//const   TUniPar UNIPAR_DSS1_RedirectionNumber               = 0x22;
const   TUniPar UNIPAR_DSS1_CalledPartySubaddress           = 0x23;
const   TUniPar UNIPAR_DSS1_Shift                           = 0x24;
const   TUniPar UNIPAR_DSS1_IEMap                           = 0x25;

const   TUniPar UNIPAR_CAS_TOLL                             = 0x30;   // междугородний
const   TUniPar UNIPAR_CAS_TYPE                             = 0x31;   // нету/декадка/челнок/пакет
const   TUniPar UNIPAR_CAS_SEMIAUTOMATIC                    = 0x32;   // полуавтоматика

// Ext
const   TUniPar UNIPAR_REALNUMBER                           = 0x35;   // реальный номер абонента который звонит
const   TUniPar UNIPAR_INTERVENTION                         = 0x36;   // вмешательство в разговор или конференцию
const   TUniPar UNIPAR_INTERVENTION_EXTLEVEL                = 0x37;   // уровень приоритета абонента
// MTP
const   TUniPar UNIPAR_MTP_LinkInfoUpdate                   = 0x40;   //
// ISUP
const   TUniPar UNIPAR_ISUP_CallingPN                       = 0x50;
const   TUniPar UNIPAR_ISUP_CalledPN                        = 0x51;
const   TUniPar UNIPAR_ISUP_CauseIndicators                 = 0x52;   //
const   TUniPar UNIPAR_ISUP_NatureOfConnectionIndicators    = 0x53;
const   TUniPar UNIPAR_ISUP_ForwardCallIndicators           = 0x54;
const   TUniPar UNIPAR_ISUP_CallingPartysCategory           = 0x55;
const   TUniPar UNIPAR_ISUP_TransmissionMediumReq           = 0x56;
const   TUniPar UNIPAR_ISUP_BackwardCallIndicators          = 0x57;
const   TUniPar UNIPAR_ISUP_EventInformation                = 0x58;
const   TUniPar UNIPAR_ISUP_SubsequentNumber                = 0x59;
const   TUniPar UNIPAR_ISUP_AccessTransport                 = 0x5A;
const   TUniPar UNIPAR_ISUP_MLPPPrecedence                  = 0x5B;
const   TUniPar UNIPAR_ISUP_UserServiceInformation          = 0x5C;
const   TUniPar UNIPAR_ISUP_CallHistoryInformation          = 0x5D;
const   TUniPar UNIPAR_ISUP_EchoControlInformation          = 0x5E;
const   TUniPar UNIPAR_ISUP_PropagationDelayCounter         = 0x5F;
const   TUniPar UNIPAR_ISUP_HopCounter                      = 0x60;
const   TUniPar UNIPAR_ISUP_SignallingPointCode             = 0x61;
const   TUniPar UNIPAR_ISUP_ParameterCompatibilityInformation = 0x62;
//const   TUniPar UNIPAR_ISUP_GenericNotification             = 0x63;
const   TUniPar UNIPAR_ISUP_LocationNumber                  = 0x64;
//const   TUniPar UNIPAR_ISUP_RedirectingNumber               = 0x65;
const   TUniPar UNIPAR_ISUP_RedirectionNumberRestriction    = 0x66;
//const   TUniPar UNIPAR_ISUP_OriginalCalledNumber            = 0x67;
//const   TUniPar UNIPAR_ISUP_RedirectionInformation          = 0x68;
const   TUniPar UNIPAR_ISUP_OptionalBackwardCallIndicators  = 0x69;
//const   TUniPar UNIPAR_ISUP_RedirectionNumber               = 0x6A;
const   TUniPar UNIPAR_ISUP_UserToUserIndicators            = 0x6B;
const   TUniPar UNIPAR_ISUP_CallDiversionInformation        = 0x6C;
const   TUniPar UNIPAR_ISUP_OptionalForwardCallIndicator    = 0x6D;
const   TUniPar UNIPAR_ISUP_AccessDeliveryInformation       = 0x6E;
const   TUniPar UNIPAR_ISUP_UserToUserInformation           = 0x6F;
const   TUniPar UNIPAR_ISUP_CCNRPossibleIndicator           = 0x70;
//const   TUniPar UNIPAR_ISUP_ConnectedNumber                 = 0x71;
const   TUniPar UNIPAR_ISUP_InformationRequestIndicators    = 0x72;
const   TUniPar UNIPAR_ISUP_InformationIndicators           = 0x73;
const   TUniPar UNIPAR_ISUP_GenericNumber                   = 0x74;
const   TUniPar UNIPAR_ISUP_OriginationISCPointCode         = 0x75;
const   TUniPar UNIPAR_ISUP_UserTeleserviceInformation      = 0x76;
const   TUniPar UNIPAR_ISUP_CalledINNumber                  = 0x77;
const   TUniPar UNIPAR_ISUP_ContinuityIndicators            = 0x78;
const   TUniPar UNIPAR_ISUP_KhazahTarifZone                 = 0x79;
const   TUniPar UNIPAR_ISUP_CallReference                   = 0x7A;
const   TUniPar UNIPAR_ISUP_ApplicationTransport            = 0x7B;
const   TUniPar UNIPAR_CUGInterlockCode                     = 0x7C;

// Интелект. маршрутизация
const   TUniPar UNIPAR_INTELROUTE_PATH                      = 0x80;
const   TUniPar UNIPAR_INTELROUTE_REDIRECT_COUNTER          = 0x81;

// АДАСЭ
const   TUniPar UNIPAR_ADASE_SeizureMode                    = 0x90;
const   TUniPar UNIPAR_ADASE_NotifyMeOnIdle                 = 0x91;
const   TUniPar UNIPAR_V52_ISDNSlotID                       = 0x96;

// Эти номера живут паралельно с  UNIPAR_NUMBER и UNIPAR_AON но более приоритетны
const   TUniPar UNIPAR_CalledNumber                         = 0xA0;
const   TUniPar UNIPAR_CallingNumber                        = 0xA1;
const   TUniPar UNIPAR_ConnectedNumber                      = 0xA2;
const   TUniPar UNIPAR_RedirectingNumber                    = 0xA3;
const   TUniPar UNIPAR_RedirectionNumber                    = 0xA4;
const   TUniPar UNIPAR_SubsequentNumber                     = 0xA5; // наврено не надо. Заменяется calledNumber
const   TUniPar UNIPAR_OriginalCalledNumber                 = 0xA6;
const   TUniPar UNIPAR_LocationNumber                       = 0xA7;
const   TUniPar UNIPAR_CalledINNumber                       = 0xA8;
const   TUniPar UNIPAR_GenericNumber                        = 0xA9;
const   TUniPar UNIPAR_CallFlags                            = 0xAA;
const   TUniPar UNIPAR_RedirectionInformation               = 0xAB;
const   TUniPar UNIPAR_GenericNotification                  = 0xAC;


// ВНИМАНИЕ: ЗАПРЕЩАЕТСЯ МЕНЯТЬ НОМЕРА СУЩЕСТВУЮЩИХ СООБЩЕНИЙ
// ТАК-ЖЕ МЕНЯТЬ ИХ ФОРМАТ
// НОВЫЕ СООБЩЕНИЯ ДОБАВЛЯТЬ ТОЛЬКО С НОВЫМИ НОМЕРАМИ



// Типы состояний портов
const   unsigned char   PortState_Free              = 0;    // свободен
const   unsigned char   PortState_PreAnswer         = 1;    // Предответное
const   unsigned char   PortState_Answer            = 2;    // Ответное
const   unsigned char   PortState_PreRelease        = 3;    // Отбойное
const   unsigned char   PortState_BlockRemote       = 4;    // Блокировка удаленна
const   unsigned char   PortState_BlockAdmin        = 5;    // Блокировка административна
const   unsigned char   PortState_ERROR             = 0xFF; // Ошибка запроса порта


// Коды DVO
const   unsigned char   DVOCode_TALK                = 0x01;     // Normal talk
const   unsigned char   DVOCode_CFU                 = 0x21;     // Call forwarding unconditional
const   unsigned char   DVOCode_CFB                 = 0x29;     // Call forwarding on subscriber busy
const   unsigned char   DVOCode_CFNRY               = 0x2A;     // Call forwarding on no reply
const   unsigned char   DVOCode_AllCF               = 0x20;     // Call call forwarding
const   unsigned char   DVOCode_CW                  = 0x41;     // Call waiting
const   unsigned char   DVOCode_HOLD                = 0x42;     // Call hold
const   unsigned char   DVOCode_CCBS                = 0x43;     // Completion of call to busy subscribers
const   unsigned char   DVOCode_HOLE                = 0x40;     // HOLD end
const   unsigned char   DVOCode_3PTY                = 0x52;     // 3-party conference
const   unsigned char   DVOCode_CT                  = 0x31;     // Call transfer
const   unsigned char   DVOCode_CP                  = 0x32;     // Call pickup
const   unsigned char   DVOCode_CC                  = 0x33;     // Consultation call
// жоп-коды испоьзуюься например в сорм для отложенных операций
//const   unsigned char   DVOCode_CWworked            = 0xF8;     //  Третий вызов в паре CW
const   unsigned char   DVOCode_SerialCall          = 0xF8;     // переадресованный вызов с серийника
const   unsigned char   DVOCode_3PTYcall1           = 0xF9;     //
const   unsigned char   DVOCode_3PTYcall2           = 0xFA;     //
const   unsigned char   DVOCode_3PTYpreRelease      = 0xFB;     //
const   unsigned char   DVOCode_HoldEndMessageless  = 0xFC;     // Возврат с удержания без нотификации holdend
const   unsigned char   DVOCode_ReleasedByDVO       = 0xFD;     // Значит что дальнейшие релизы идут по коду отбора DVO
const   unsigned char   DVOCode_Messageless         = 0xFE;     // Вызов без сормовских нотификацй
const char* getDVOCodeText(unsigned char DVOCode);



//
class CUniPar
{
    public:

        // Созидание
        static void clear ( void );
        static void clear ( TSignalling sig );

        static void setSignalling ( TSignalling sig );

        static void  addByte ( TUniPar uniPar, BYTE data );
        static void  addWord ( TUniPar uniPar, WORD data );
        static void  addDWord ( TUniPar uniPar, DWORD data );
        static void* addBuffer(TUniPar uniPar, const void* buf, BYTE len);
        static void  addString ( TUniPar uniPar, const char *str );
        static void  addFlag ( TUniPar uniPar );
        static void  addNumber(TUniPar uniPar, const UniN::Number& num);
        static bool  addUni ( TUniPar uniPar, CUniPar* up, TUniPar uniParNew = 0);

        static void remove ( TUniPar uniPar ); // удаление параметра

        static CUniPar* create ( void );

        static CUniPar* create ( const BYTE* data );

        // Чтение
        TSignalling getSignalling ( void ) const;

        bool  isPresent ( TUniPar uniPar ) const;
        bool  getByte ( TUniPar uniPar, BYTE& data ) const;
        bool  getWord ( TUniPar uniPar, WORD& data ) const;
        bool  getDWord ( TUniPar uniPar, DWORD& data ) const;
        bool  getBuffer ( TUniPar uniPar, BYTE *buf, BYTE &len, BYTE maxlen ) const;
        const BYTE* getBufferPtr(TUniPar uniPar, BYTE &len ) const;
        const void* getBufferPtr(TUniPar uniPar) const;
        bool  getFixBuffer(TUniPar uniPar, void* buf, BYTE maxlen) const;
        bool  getString ( TUniPar uniPar, char *str, BYTE maxlen ) const;
        const char* getStringPtr ( TUniPar uniPar ) const;
        bool  getFlag ( TUniPar uniPar ) const;
        bool  getNumber(TUniPar uniPar, UniN::Number& num ) const;
        const UniN::Number* getNumberPtr ( TUniPar uniPar ) const;

        short getLen() const;

        // Локализация (копирование в стат буфер)
        void localize() const;
        void localize(const BYTE* paramPtr) const;      // Мувим один параметр в стат буфер

        // Шаримся по параметрам
        const void* getFirstPtr() const;
        const void* getNextPtr(const void* paramPtr) const;

        // Доступ к полям параметра (eybabufebz)
        TUniPar typePar(const void* p) const { return *(BYTE*)p; }
        BYTE lenPar(const void* p) const { return *((BYTE*)p+1); }
        const BYTE* dataPar(const void* p) const { return (*((BYTE*)p+1)) ? ((BYTE*)p+2) : NULL; }


    private:
        short getPos ( TUniPar uniPar ) const;

    private:
        short Len;
        TSignalling Signalling;
        BYTE Buf[MAX_SIZE_PARAMETER];

        static short stLen;
        static TSignalling stSignalling;
        static char stBuf[MAX_SIZE_PARAMETER];
};

//
// Причины отбоев
const   TCauseValue     ReleaseCause_Unallocated_number                           =   1;
const   TCauseValue     ReleaseCause_No_route_to_transit_network                  =   2;
const   TCauseValue     ReleaseCause_No_route_to_destination                      =   3;
const   TCauseValue     ReleaseCause_Channel_unacceptable                         =   6;
const   TCauseValue     ReleaseCause_Call_awarded                                 =   7;
const   TCauseValue     ReleaseCause_Preemption                                   =   8;  // SS-MLPP Q.955
const   TCauseValue     ReleaseCause_Normal_call_clearing                         =  16;
const   TCauseValue     ReleaseCause_User_busy                                    =  17;
const   TCauseValue     ReleaseCause_No_user_responding                           =  18;
const   TCauseValue     ReleaseCause_No_answer_from_user                          =  19;
const   TCauseValue     ReleaseCause_Subscriber_absent                            =  20;
const   TCauseValue     ReleaseCause_Call_rejected                                =  21;
const   TCauseValue     ReleaseCause_Number_changed                               =  22;
const   TCauseValue     ReleaseCause_Exchange_routing_error                       =  25;
const   TCauseValue     ReleaseCause_Non_selected_user_clearing                   =  26;
const   TCauseValue     ReleaseCause_Destination_out_of_order                     =  27;
const   TCauseValue     ReleaseCause_Invalid_number_format                        =  28;
const   TCauseValue     ReleaseCause_Facility_rejected                            =  29;
const   TCauseValue     ReleaseCause_Response_to_STATUS_ENQUIRY                   =  30;
const   TCauseValue     ReleaseCause_Normal_unspecified                           =  31;
const   TCauseValue     ReleaseCause_No_circuit_channel_available                 =  34;
const   TCauseValue     ReleaseCause_Network_out_of_order                         =  38;
const   TCauseValue     ReleaseCause_Temporary_failure                            =  41;
const   TCauseValue     ReleaseCause_Switching_equipment_congestion               =  42;
const   TCauseValue     ReleaseCause_Access_information_discarded                 =  43;
const   TCauseValue     ReleaseCause_Reqsted_channel_not_available                =  44;
const   TCauseValue     ReleaseCause_Precedence_call_blocked                      =  46;  // SS-MLPP Q.955
const   TCauseValue     ReleaseCause_Resource_unavailable_unspecif                =  47;
const   TCauseValue     ReleaseCause_Quality_of_service_unavailable               =  49;
const   TCauseValue     ReleaseCause_Reqsted_facility_not_subscribed              =  50;
const   TCauseValue     ReleaseCause_Bearer_capability_not_authrzed               =  57;
const   TCauseValue     ReleaseCause_Bearer_capability_not_available              =  58;
const   TCauseValue     ReleaseCause_Service_not_available_unspec                 =  63;
const   TCauseValue     ReleaseCause_Bearer_capability_not_implmted               =  65;
const   TCauseValue     ReleaseCause_Channel_type_not_implemented                 =  66;
const   TCauseValue     ReleaseCause_Reqsted_facility_not_implmted                =  69;
const   TCauseValue     ReleaseCause_Only_restricted_digital_bearer               =  70;
const   TCauseValue     ReleaseCause_Service_not_implemented                      =  79;
const   TCauseValue     ReleaseCause_Invalid_call_reference_value                 =  81;
const   TCauseValue     ReleaseCause_Identified_channel_not_exist                 =  82;
const   TCauseValue     ReleaseCause_No_call_ID_for_suspended_call                =  83;
const   TCauseValue     ReleaseCause_Call_identity_in_use                         =  84;
const   TCauseValue     ReleaseCause_No_call_suspended                            =  85;
const   TCauseValue     ReleaseCause_Call_has_been_cleared                        =  86;
const   TCauseValue     ReleaseCause_User_not_a_member_of_CUG                     =  87;
const   TCauseValue     ReleaseCause_Non_existent_closed_user_group               =  88;
const   TCauseValue     ReleaseCause_Called_user_not_member_of_GUG                =  89;
const   TCauseValue     ReleaseCause_Called_user_belongs_to_GUG                   =  90;
const   TCauseValue     ReleaseCause_Invalid_transit_network                      =  91;
const   TCauseValue     ReleaseCause_Invalid_message_unspecified                  =  95;
const   TCauseValue     ReleaseCause_Mandatory_IE_is_missing                      =  96;
const   TCauseValue     ReleaseCause_Messsage_type_non_existent                   =  97;
const   TCauseValue     ReleaseCause_Msg_uncompatible_non_existent                =  98;
const   TCauseValue     ReleaseCause_IE_non_existent                              =  99;
const   TCauseValue     ReleaseCause_Invalid_IE_contents                          = 100;
const   TCauseValue     ReleaseCause_Message_not_compatible                       = 101;
const   TCauseValue     ReleaseCause_Recovery_on_timer_expiry                     = 102;
const   TCauseValue     ReleaseCause_Protocol_error_unspecified                   = 111;
const   TCauseValue     ReleaseCause_Interworking_unspecified                     = 127;

#endif
