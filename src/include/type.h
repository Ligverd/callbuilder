#ifndef __SMP_TYPE_H__
#define __SMP_TYPE_H__

#ifdef  ARM
    #include  <memory.h>
#else
    #ifdef __GNUC__
         #include  <iostream.h>
        //#define _ATS_ //changed by pax
    #else
        #define _WIN_
    #endif
#endif

#ifdef  ARMLINUX
    #define LRW(X) _LRW(&(X))
    #define LRS(X) _LRS(&(X))
    #define LRD(X) _LRD(&(X))
    #define LRI(X) _LRI(&(X))

    #define LXW(X, Y) _LXW(&(X), (Y))
    #define LXS(X, Y) _LXS(&(X), (Y))
    #define LXD(X, Y) _LXD(&(X), (Y))
    #define LXI(X, Y) _LXI(&(X), (Y))

    #define LARMCHECK(X)  {if (_LARMCHECK(X)) WARNING;}
#else
    #define LRW(X) (X)
    #define LRS(X) (X)
    #define LRD(X) (X)
    #define LRI(X) (X)

    #define LXW(X, Y) ((X) = (Y))
    #define LXS(X, Y) ((X) = (Y))
    #define LXD(X, Y) ((X) = (Y))
    #define LXI(X, Y) ((X) = (Y))

    #define LARMCHECK(X)
#endif

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

#ifdef WIN32
    #include "windows.h"
#endif

//typedef char FLAG;

#ifdef _ATS_

//    typedef char BOOL;

#ifndef __ARM_LINUX_KERNEL__
  #ifndef NULL
    #define NULL 0
  #endif
#endif
    #define LOWORD(X) ((WORD)(X))
    #define HIWORD(X) ((WORD)((X)>>16))

#ifndef ARMLINUX
    #pragma pack(1)
#endif

#endif


#define NOP5 { __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop"); __asm("nop"); }

typedef unsigned long   DWORD ; // 32
typedef unsigned short  WORD  ; // 16
typedef unsigned char   BYTE  ; //  8
typedef unsigned char   uc;

inline  unsigned int armAlign(unsigned int n) {
    return (4-n) & 0x00000003;
};


//#define TRUE  1
//#define FALSE 0

#define VLOWORD(X) (*(WORD*)&(X))
#define VHIWORD(X) (*(((WORD*)&(X))+1))

#define GCSPLAPDVERSION 51

#ifdef _WIN_

    void stub_asm( char* s );
    BYTE stub_inportb( WORD x );
    WORD stub_inportw( WORD x );
    void stub_outportb( WORD x, BYTE y );
    void stub_outportw( WORD x, WORD y );

    #define __asm(X) stub_asm(X)
    #define inportb(X) (stub_inportb(X))
    #define inportw(X) (stub_inportw(X))
    #define outportb(X, Y) stub_outportb((X), (Y))
    #define outportw(X, Y) stub_outportw((X), (Y))
    #define __inline__

    void WinTrace ( int port, char *str );

#endif

#ifdef _ATS_
#define MAKEWORD(X, Y) ((WORD)(BYTE)(X)) + (((WORD)(BYTE)(Y)) << 8)
#define LOBYTE(X) ((BYTE)(WORD)(X))
#define HIBYTE(X) ((BYTE)(((WORD)(X)) >> 8))
#endif

#ifdef _WIN_
#endif

#define GETMIN(X,Y) ((X)<=(Y)?(X):(Y))

#define MAKEDWORD(X, Y) ((DWORD)(WORD)(X)) + (((DWORD)(WORD)(Y)) << 16)

#define PRINT HT.config.SendTerminalFormat
#define SECOND sysComPort.SecondString

#ifdef _ATS_
    #define CONTROLPOINT(X)      sysSave.ControlPoint(__LINE__, (X))
    #define CONTROLPOINTNAFIG(X) sysSave.ControlPoint(__LINE__, (X))
#endif
#ifdef _WIN_
    #define CONTROLPOINT(X)      sysLoad.ControlPoint(__LINE__, (X))
    #define CONTROLPOINTDEBUG(X) sysLoad.ControlPoint(((WORD)-1), (X))
    #define CONTROLPOINTNAFIG(X) sysLoad.ControlPoint(((WORD)-2), (X))
#endif

//
// ������ ������ ����
//

typedef     short           TState;         //
typedef     unsigned short  TBlock;         // ������������� ����������� �������� (� ��������� ���������� ������)
#ifndef __ARM_LINUX_KERNEL__
const       TState          InvalidState            = (TState)-1;
const       TState          InvalidStateID          = (TState)-1;
#endif

// ����
typedef     char            TModule;        // �� '99
typedef     unsigned short  TSignal;        // ������������� �������
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

typedef     unsigned long   TDir;           // ����������
typedef     short           TLine;          // �����-�� �����
typedef     unsigned long   TCallID;        // ������������� ������. �����������, ����������.
typedef     unsigned long   TCallPointID;   // ������������� ����� ������. ����.
typedef     unsigned char   TCauseValue;    // ������� �����

#ifndef __ARM_LINUX_KERNEL__
// ���� �������� ��� ���������� ���������
const   THiIdx  HiIdx_base_Slot                         = 1;
const   THiIdx  HiIdx_base_PCM                          = 50+1;
const   THiIdx  HiIdx_base_Air                          = 70+1;
const   THiIdx  HiIdx_base_AirShadowOffset              = 15;
const   THiIdx  HiIdx_base_AirShadow                    = 70+1 + HiIdx_base_AirShadowOffset;


// ����� ��������� � CPort

// ���� �����
const   unsigned char   BlockType_Locally       = 0x00;
const   unsigned char   BlockType_Remotely      = 0x01;

// ���� ���������� �������
const   unsigned char   Block_Maintenance       = 0x00;     // TypeInd_Maintenance_oriented;
const   unsigned char   Block_Hardware          = 0x01;     // TypeInd_Hardware_failure_oriented;
const   unsigned char   Block_Incoming          = 0x02;     // TypeInd_Hardware_failure_oriented;
const   unsigned char   Block_Outgoing          = 0x03;     // TypeInd_Hardware_failure_oriented;

// ������������ ������������ (�����):
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


WORD _LRW ( WORD* X );
short _LRS ( short* X );
DWORD _LRD ( DWORD* X );
int _LRI ( int* X );

void _LXW ( WORD* X, WORD Y );
void _LXS ( short* X, short Y );
void _LXD ( DWORD* X, DWORD Y );
void _LXI ( int* X, int Y );

bool _LARMCHECK ( void* X );
#endif


#endif