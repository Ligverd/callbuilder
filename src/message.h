
#ifndef  __MESSAGE_H__
#define  __MESSAGE_H__

#include  <type.h>



// системные мессаги
#define     M_SYS_GROUP                                                 0x1100
#define     M_SYS_LOGSTART                                              0x1101
#define     M_SYS_LOGEND                                                0x1102
#define     M_SYS_LOGACCEPT                                             0x1103
#define     M_SYS_LOGABORT                                              0x1104
#define     M_SYS_LOGFULL                                               0x1105



// мессаги от таймеров
#define     M_TMPBX_GROUP                                               0x1200
#define     M_TMPBX_TIMERA                                              0x1201
#define     M_TMPBX_TIMERB                                              0x1202
#define     M_TMPBX_TIMERC                                              0x1203
#define     M_TMPBX_TIMERD                                              0x1204
#define     M_TMPBX_TIMERE                                              0x1205
#define     M_TMPBX_TIMERF                                              0x1206
// это первый идентификатор других таймеров.
#define     M_TMPBX_                                                    0x1200



// драйвер FALC
#define     M_DFALC_GROUP                                               0x1300
#define     M_DFALC_SIGPRESENT                                          0x1301
#define     M_DFALC_SIGABSENT                                           0x1302
#define     M_DFALC_CAS_00                                              0x1303
#define     M_DFALC_CAS_01                                              0x1304
#define     M_DFALC_CAS_10                                              0x1305
#define     M_DFALC_CAS_11                                              0x1306
#define     M_DFALC_CAS_ACT                                             0x1307
#define     M_DFALC_CAS_PAS                                             0x1308
#define     M_DFALC_CAS_BUG                                             0x1309
#define     M_DFALC_FRAME                                               0x130A
#define     M_DFALC_CAS_XXXX                                            0x130B
#define     M_DFALC_CAS_TOOMANYWORK                                     0x130C
#define     M_DFALC_RRA_PRESENT                                         0x130D
#define     M_DFALC_RRA_ABSENT                                          0x130E
#define     M_DFALC_AIS_PRESENT                                         0x130F
#define     M_DFALC_AIS_ABSENT                                          0x1310
#define     M_DFALC_COMFRAME                                            0x1311


// драйвер EXT
#define     M_DEXT_GROUP                                                0x1400
#define     M_DEXT_UP                                                   0x1401
#define     M_DEXT_DOWN                                                 0x1402
#define     M_DEXT_CITYRING                                             0x1403
#define     M_DEXT_CITYSWITCH_ON                                        0x1404
#define     M_DEXT_CITYSWITCH_OFF                                       0x1405
#define     M_DEXT_1VSK_ACT                                             0x1406
#define     M_DEXT_1VSK_PAS                                             0x1407
#define     M_DEXT_LGSTRING                                             0x1408
//Драйвер 3-х проводки, 17 сообщений
#define     M_DEXT_3SL_00000                                            0x1409
#define     M_DEXT_3SL_00001                                            0x140A
#define     M_DEXT_3SL_00010                                            0x140B
#define     M_DEXT_3SL_00011                                            0x140C
#define     M_DEXT_3SL_00100                                            0x140D
#define     M_DEXT_3SL_00101                                            0x140E
#define     M_DEXT_3SL_00110                                            0x140F
#define     M_DEXT_3SL_00111                                            0x1410
#define     M_DEXT_3SL_01000                                            0x1411
#define     M_DEXT_3SL_01001                                            0x1412
#define     M_DEXT_3SL_01010                                            0x1413
#define     M_DEXT_3SL_01011                                            0x1414
#define     M_DEXT_3SL_01100                                            0x1415
#define     M_DEXT_3SL_01101                                            0x1416
#define     M_DEXT_3SL_01110                                            0x1417
#define     M_DEXT_3SL_01111                                            0x1418
#define     M_DEXT_3SL_01110                                            0x1417
#define     M_DEXT_3SL_01111                                            0x1418
#define     M_DEXT_3SL_10000                                            0x1419
#define     M_DEXT_3SL_10001                                            0x141A
#define     M_DEXT_3SL_10010                                            0x141B
#define     M_DEXT_3SL_10011                                            0x141C
#define     M_DEXT_3SL_10100                                            0x141D
#define     M_DEXT_3SL_10101                                            0x141E
#define     M_DEXT_3SL_10110                                            0x141F
#define     M_DEXT_3SL_10111                                            0x1420
#define     M_DEXT_3SL_11000                                            0x1421
#define     M_DEXT_3SL_11001                                            0x1422
#define     M_DEXT_3SL_11010                                            0x1423
#define     M_DEXT_3SL_11011                                            0x1424
#define     M_DEXT_3SL_11100                                            0x1425
#define     M_DEXT_3SL_11101                                            0x1426
#define     M_DEXT_3SL_11110                                            0x1427
#define     M_DEXT_3SL_11111                                            0x1428
#define     M_DEXT_3SL_11110                                            0x1427
#define     M_DEXT_3SL_11111                                            0x1428
// ISDN
#define     M_DEXT_ISDN_FRAME                                           0x1429
#define     M_DEXT_ISDN_COMMAND                                         0x142A
// плата А16
#define     M_DEXT_A16_ABSENT                                           0x142B
#define     M_DEXT_A16_PRESENT                                          0x142C



// драйвер COMMUT
#define     M_DCOMMUT_GROUP                                             0x1500
#define     M_DCOMMUT_DTMF                                              0x1501
#define     M_DCOMMUT_NODTMF                                            0x1502
#define     M_DCOMMUT_MFR                                               0x1503
#define     M_DCOMMUT_NOMFR                                             0x1504
#define     M_DCOMMUT_ADD                                               0x1505
#define     M_DCOMMUT_ADD_OFF                                           0x1506
#define     M_DCOMMUT_NOADD                                             0x1507
#define     M_DCOMMUT_425                                               0x1508
#define     M_DCOMMUT_425_OFF                                           0x1509
#define     M_DCOMMUT_NO425                                             0x150A
#define     M_DCOMMUT_CAPSULA                                           0x150B
#define     M_DCOMMUT_SIPPACKET                                         0x150C



// массаги от критического драйвера
#define     M_DCRITIC_GROUP                                             0x1600
#define     M_DCRITIC_BATR_DONE                                         0x1601
#define     M_DCRITIC_BATR_OVERLOAD                                     0x1602
#define     M_DCRITIC_TONE_DONE                                         0x1603
#define     M_DCRITIC_TONE_OVERLOAD                                     0x1604
#define     M_DCRITIC_CAPSULA                                           0x1605



// мессаги от драйвера ISDN
#define     M_ISDN_ACTIVATE                                             0x1700
#define     M_ISDN_ACTIVATE_COMPLITE                                    0x1701
#define     M_ISDN_DEACTIVATE_LAPD                                      0x1702
#define     M_ISDN_DEACTIVATE                                           0x1703

// ACT_req   - запрос на поднятия канала к телефону
// ACT_ind   - ответ, что все OK.
// DEACT_ind - не получилось поднять линк к телефону
#define     M_ISDN_ACTIVATE_request                                     0x1704
#define     M_ISDN_ACTIVATE_indication                                  0x1705
#define     M_ISDN_DEACTIVATE_indication                                0x1706
#define     M_ISDN_SET_TEI                                              0x1707
#define     M_ISDN_REMOVE_TEI                                           0x1708
#define     M_ISDN_SYNH_ABSENT                                          0x1709
#define     M_ISDN_TRANSMIT_REQUEST                                     0x170A
#define     M_ISDN_GET_COMMUT                                           0x170B


// массаги системные упроавления портами
#define     M_CC_GROUP                                                  0x2100
#define     M_CC_CALL                                                   0x2101
#define     M_CC_ACCEPT                                                 0x2102
#define     M_CC_RELEASE                                                0x2103
#define     M_CC_CONNECTSIGNAL                                          0x2104
#define     M_CC_CONNECTSILENCE                                         0x2105
#define     M_CC_CHANGELINE                                             0x2106



// массаги сигнализации верхнего уровн
#define     M_PBX_GROUP                                                 0x2200
// системные
#define     M_PBX_CALL                                                  0x2201
#define     M_PBX_ACCEPT                                                0x2202
#define     M_PBX_RELEASE                                               0x2203
// прикладные
#define     M_PBX_ANSWER                                                0x2204
#define     M_PBX_DIGIT                                                 0x2205
#define     M_PBX_NUMBERCOMPLETE                                        0x2206
#define     M_PBX_SubscriberFree                                        0x2207
#define     M_PBX_V5AN_line_signal                                      0x2208
#define     M_PBX_StopRinging                                           0x2209
#define     M_PBX_TEST                                                  0x220A
// v52
const   TSignal M_PBX_V52_to_system_management                        = 0x220B;
const   TSignal M_PBX_V52_to_control_port                             = 0x220C;
const   TSignal M_PBX_V52_to_control_dl                               = 0x220D;
const   TSignal M_PBX_V52_to_pstn_prot                                = 0x220E;
const   TSignal M_PBX_V52_to_restart                                  = 0x220F;
const   TSignal M_PBX_V52_to_pstn_resource_manager                    = 0x2210;
const   TSignal M_PBX_V52_to_pstn_dl                                  = 0x2211;
const   TSignal M_PBX_V52_to_pstn_port_status_fsm                     = 0x2212;
const   TSignal M_PBX_V52_to_port_x731_status_fsm                     = 0x2213;
// DSS1
const   TSignal M_PBX_DSS1_ProgressIndication                         = 0x2214;
const   TSignal M_PBX_V52_to_lapv5ef                                  = 0x2215;
const   TSignal M_PBX_V52_to_isdn_adapter                             = 0x2216;
const   TSignal M_PBX_DSS1_ALERTING                                   = 0x2217;
const   TSignal M_PBX_DSS1_DisconnectRequest                          = 0x2218;
// MTP
const   TSignal M_PBX_MTP_MessageForRouting                           = 0x2219;
const   TSignal M_PBX_MTP_LinkSignalPresent                           = 0x221A;
const   TSignal M_PBX_MTP_LinkSignalAbsent                            = 0x221B;
const   TSignal M_PBX_MTP_LinkActive                                  = 0x221C;
const   TSignal M_PBX_MTP_LinkInactive                                = 0x221D;
const   TSignal M_PBX_MTP_LinkUpdateStatus                            = 0x221E;

const   TSignal M_PBX_DSS1_FACILITY                                   = 0x2220;
const   TSignal M_PBX_DSS1_NOTIFY                                     = 0x2221;
const   TSignal M_PBX_DSS1_INFORMATION                                = 0x2222;

// ISUP
const   TSignal M_PBX_ISUP_MTP_RESUME                                 = 0x2225;
const   TSignal M_PBX_ISUP_MTP_PAUSE                                  = 0x2226;
const   TSignal M_PBX_ISUP_TransferIndication                         = 0x2227;
const   TSignal M_PBX_ISUP_ACM                                        = 0x2228;
const   TSignal M_PBX_ISUP_CPG                                        = 0x2229;
// DSS1 or ISUP interworking
const   TSignal M_PBX_DSS1ISUP_CallProceeding                         = 0x222A;
const   TSignal M_PBX_ISUP_ChargingInformation                        = 0x222B;
const   TSignal M_PBX_ISUP_NRM                                        = 0x222C;
const   TSignal M_PBX_CAS2_OneWayRelease                              = 0x222D;
// SORM
const   TSignal M_PBX_SORM_OVERHEAR_FIRST                             = 0x2230;
const   TSignal M_PBX_SORM_OVERHEAR_SECOND                            = 0x2231;
const   TSignal M_PBX_SORM_UPDATE_SPEECH                              = 0x2232;
// тест для отбивания двухпроводок замкнутых на себ
const   TSignal M_PBX_CITYSELFTEST                                    = 0x2233;
// CAS
#define     M_PBX_CALL_DIAL_START                                       0x2234
#define     M_PBX_CALL_DIAL_STOP                                        0x2235
#define     M_PBX_CANCELANSWER                                          0x2237
#define     M_PBX_BFREE                                                 0x2238
#define     M_PBX_BBUSY                                                 0x2239
#define     M_PBX_BRELEASE                                              0x223A
// DVO
#define     M_PBX_DVO_INTERCEPT                                         0x223B
//DISA
#define     M_PBX_DISA_Activate                                         0x223E
#define     M_PBX_DISA_Digit                                            0x223F
// Для обмена с системным телефоном
#define     M_PBX_LG_ME_FREE                                            0x2240
#define     M_PBX_LG_ME_BUSY                                            0x2241
#define     M_PBX_LG_ASK                                                0x2242
// Для тестирования абонентского комплекта
#define     M_PBX_EXT_PHISICALTEST                                      0x2243
// Мессага от прицепа к абоненту, вернуться назад
#define     M_PBX_EXT_RETRIVE                                           0x2244

// Для PC-PHONE
#define     M_PBX_EXT_PCPHONE                                           0x2245
#define     M_PBX_PCPHONE_ME_FREE                                       0x2246
#define     M_PBX_PCPHONE_ME_BUSY                                       0x2247
#define     M_PBX_PHONE_CHANGE_NUMBER                                   0x2248

const   TSignal M_PBX_InformationRequest                              = 0x224B;     // запрос информации (ex: aon, category)
const   TSignal M_PBX_InformationIndication                           = 0x224C;
const   TSignal M_PBX_ActivateCallBack                                = 0x224E;     // Активация обратного вызова
const   TSignal M_PBX_ISUP_COT                                        = 0x224F;

// Для permanent соединений
#define     M_PBX_PERM_A                                                0x2260
#define     M_PBX_PERM_B                                                0x2261
#define     M_PBX_PERM_C                                                0x2262
#define     M_PBX_PERM_D                                                0x2263

// Для Факин-АДАСЭ
#define     M_PBX_2100_DO_CALL                                          0x2270


// Дабацываем V52
//const   TSignal M_PBX_V52_PSTN_MDU_StartTraffic = 0;



// массаги для интернал линий
#define     M_INT_GROUP                                                 0x2300
#define     M_INT_FRAME                                                 0x2301
#define     M_INT_LINKUP                                                0x2303
#define     M_INT_LINKDOWN                                              0x2304
#define     M_INT_REMTOP                                                0x2305



// резервирую себе 40h груп - 64 штук. Шуриком.
#define     M_GROUP                                                     0x3000



// Стартовый номер для сообщений управления 0x7000
// В командах блокировок plus.cc.par1 имеет значение Block_Maintenance, Block_Hardware, Block_Incoming, Block_Outgoing
const   TSignal M_AFR_GROUP                                           = 0x7000;
const   TSignal M_AFR_BlockPort                                       = 0x7001;
const   TSignal M_AFR_UnblockPort                                     = 0x7002;
const   TSignal M_AFR_BlockLink                                       = 0x7003;
const   TSignal M_AFR_UnblockLink                                     = 0x7004;
const   TSignal M_AFR_ResetPort                                       = 0x7005;
const   TSignal M_AFR_ResetLink                                       = 0x7006;
const   TSignal M_AFR_TraceLinkOn                                     = 0x7007;
const   TSignal M_AFR_TraceLinkOff                                    = 0x7008;
const   TSignal M_AFR_TracePortOn                                     = 0x7009;
const   TSignal M_AFR_TracePortOff                                    = 0x700A;
// Для подачи снятия частоты на абоненте
const   TSignal M_AFR_FreqStart                                       = 0x7010;
const   TSignal M_AFR_FreqStop                                        = 0x7011;
// Для системы авансовых платежей
const   TSignal M_AFR_PrePayNumberComplete                            = 0x7012;
const   TSignal M_AFR_PrePayTalkBegin                                 = 0x7013;
const   TSignal M_AFR_PreStart                                        = 0x7014;
const   TSignal M_AFR_PreStop                                         = 0x7015;



// стартовый номер для локальных мессаг
#define     M_LOCAL_GROUP                                               0xE000



// все что >= M_CFG_ALL относится к очереди CFG
#define     M_CFG_ALL                                                   0xF000
#define     M_CFG_STRING                                                0xF001

#define     M_CFG_COM_PACKET                                            0xF002
#define     M_CFG_COM_UP                                                0xF003
#define     M_CFG_COM_DOWN                                              0xF004
#define     M_CFG_COM_LINKUP                                            0xF005
#define     M_CFG_COM_LINKDOWN                                          0xF006
#define     M_CFG_COM_OVERLOAD                                          0xF007
#define     M_CFG_WIZ_OVERLOAD                                          0xF008

#define     M_CFG_PCM_PACKET                                            0xF009
#define     M_CFG_PCM_LINKUP                                            0xF00A
#define     M_CFG_PCM_LINKDOWN                                          0xF00B

#define     M_CFG_MONITOR_PCM_CONNECT_UP                                0xF00C
#define     M_CFG_MONITOR_PCM_CONNECT_DOWN                              0xF00D
#define     M_CFG_MONITOR_PCM_BITSTATUS                                 0xF00E

#define     M_CFG_MONITOR_PCM_CAS_TMW_ON                                0xF00F
#define     M_CFG_MONITOR_PCM_CAS_TMW_OFF                               0xF010

#define     M_CFG_MONITOR_A16_ABSENT                                    0xF011
#define     M_CFG_MONITOR_A16_PRESENT                                   0xF012

// Мессага для упса
#define     M_CFG_MONITOR_UPS_DATA                                      0xF013

// Мессаги от нового измерителя
#define     M_CFG_MONITOR_IZM_ABSENT                                    0xF014
#define     M_CFG_MONITOR_IZM_RESET                                     0xF015
#define     M_CFG_MONITOR_IZM_DATA                                      0xF016

// От FAT при переполнении
#define     M_CFG_FAT_OVERLOAD                                          0xF017

#define     M_CFG_SPECIAL                                               0xF018

#define     M_CFG_SIP_PACKET                                            0xF020
#define     M_CFG_PLATA_UP                                              0xF021
#define     M_CFG_PLATA_DOWN                                            0xF022

// таймера CFG
#define     M_TMCFG_GROUP                                               0xF100
#define     M_TMCFG_ROUTE                                               0xF101
#define     M_TMCFG_WIDE                                                0xF102
#define     M_TMCFG_CFGLOADCOMPLETE                                     0xF103
#define     M_TMCFG_BINLOADCOMPLETE                                     0xF104
#define     M_TMCFG_INTELLECTROUTING                                    0xF105
#define     M_TMCFG_ALARMPOSITIVELINE                                   0xF106
#define     M_TMCFG_SHUTDOWN1                                           0xF107
#define     M_TMCFG_SHUTDOWN2                                           0xF108
#define     M_TMCFG_SHUTDOWN3                                           0xF109
#define     M_TMCFG_TESTMONITOR                                         0xF10A
#define     M_TMCFG_LOGPERIOD1                                          0xF10B
#define     M_TMCFG_LOGPERIOD2                                          0xF10C

#endif
