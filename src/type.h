#ifndef __TYPE_H__
#define __TYPE_H__

#include  <string.h>

#define DEV_NONE 0
#define DEV_MAL  1
#define DEV_MPA  2
#define DEV_MPB  3
#define DEV_ISDN 4
#define DEV_C412 5

#define DEVICE_NONE (glDevice == DEV_NONE)
#define DEVICE_MAL  (glDevice == DEV_MAL)
#define DEVICE_MPA  (glDevice == DEV_MPA)
#define DEVICE_MPB  (glDevice == DEV_MPB)
#define DEVICE_ISDN (glDevice == DEV_ISDN)
#define DEVICE_C412 (glDevice == DEV_C412)


#define NOP5 { __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop"); }

typedef unsigned long   DWORD ; // 32
typedef unsigned short  WORD  ; // 16
typedef unsigned char   BYTE  ; //  8
typedef unsigned char   uc;


#define VLOWORD(X) (*(WORD*)&(X))
#define VHIWORD(X) (*(((WORD*)&(X))+1))


#define GETMIN(X,Y) ((X)<=(Y)?(X):(Y))

#define MAKEDWORD(X, Y) ((DWORD)(WORD)(X)) + (((DWORD)(WORD)(Y)) << 16)


//
// Разные нужные типы
//

typedef     short           TState;         //
typedef     unsigned short  TBlock;         // идентификатор сигнального процесса (в локальном физическом модуле)
const       TState          InvalidState            = (TState)-1;
const       TState          InvalidStateID          = (TState)-1;

// Типы
typedef     char            TModule;        // до '99
typedef     unsigned short  TSignal;        // Идентификатор сигнала
typedef     char            TPCM;
typedef     char            TTSL;
typedef     char            THiIdx;
typedef     char            TLoIdx;
typedef     char            TFalcType;
typedef     char            TLink;
typedef     char            TChannel;

typedef     unsigned char   TSignalling;
typedef     unsigned char   TUniPar;


typedef     char            TSlot;
typedef     char            THole;
typedef     char            TWide;

typedef     unsigned long   TDir;           // направение
typedef     short           TLine;          // какая-то линия
typedef     unsigned long   TCallID;        // идентификатор вызова. Тарификация, мониторинг.
typedef     unsigned long   TCallPointID;   // Идентификатор точки звонка. Сорм.
typedef     unsigned char   TCauseValue;    // причина отбоя


// Базы индексов для унификации пересчета
const   THiIdx  HiIdx_base_Slot                         = 1;
const   THiIdx  HiIdx_base_PCM                          = 50+1;
const   THiIdx  HiIdx_base_Air                          = 70+1;
const   THiIdx  HiIdx_base_AirShadowOffset              = 15;
const   THiIdx  HiIdx_base_AirShadow                    = 70+1 + HiIdx_base_AirShadowOffset;


// потом перенести в CPort

// типы типов
const   unsigned char   BlockType_Locally       = 0x00;
const   unsigned char   BlockType_Remotely      = 0x01;

// типы блокировок каналов
const   unsigned char   Block_Maintenance       = 0x00;     // TypeInd_Maintenance_oriented;
const   unsigned char   Block_Hardware          = 0x01;     // TypeInd_Hardware_failure_oriented;
const   unsigned char   Block_Incoming          = 0x02;     // TypeInd_Hardware_failure_oriented;
const   unsigned char   Block_Outgoing          = 0x03;     // TypeInd_Hardware_failure_oriented;

// Составленные перечисления (Маски):
const   unsigned char   BlockMask_LocalMaintenance      = 0x01;
const   unsigned char   BlockMask_RemoteMaintenance     = 0x02;
const   unsigned char   BlockMask_LocalHardware         = 0x04;
const   unsigned char   BlockMask_RemoteHardware        = 0x08;
const   unsigned char   BlockMask_Incoming              = 0x10;
const   unsigned char   BlockMask_Outgoing              = 0x40;

const   unsigned char   BusyBlockMask = BlockMask_LocalMaintenance  |
                                        BlockMask_LocalHardware     |
                                        BlockMask_RemoteMaintenance |
                                        BlockMask_RemoteHardware    |
                                        //BlockMask_Incoming          |
                                        BlockMask_Outgoing          ;

const   unsigned char   FullBlockMask = BlockMask_LocalMaintenance  |
                                        BlockMask_LocalHardware     |
                                        BlockMask_RemoteMaintenance |
                                        BlockMask_RemoteHardware    |
                                        BlockMask_Incoming          |
                                        BlockMask_Outgoing          ;


//
inline unsigned char makeBlockMask(unsigned char type, unsigned char block) {
    return (0x01 << ((2*block) + type));
}

#endif
