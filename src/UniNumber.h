/**************************************************************************
Codepage : Win-1251
File     : UniNumber.h
Project  : SMP
Desc     :
           Interface.

AUTHOR   : $Author: maksim $
           Alexander Zuykov, MTA
ID       : $Id: UniNumber.h,v 1.29.4.4 2009/11/06 14:40:22 maksim Exp $
MODIFIED : $Date: 2009/11/06 14:40:22 $
***************************************************************************


**************************************************************************/


#ifndef  __UNINUMBER_H__
#define  __UNINUMBER_H__

#include "unistd.h"

#pragma pack (push, 1)

namespace UniN {


const   size_t  MAX_NUMBER_LENGTH       = 25;

typedef  unsigned char      uc;

//
class CallFlags {

public:
    // transfer capability DSS1 default
    // DSS1     Info Transfer Capability
    // ISUP     Transmission Medium Requirement
    uc      trcap;      // trcapc_speech, trcapc_31khz, trcapc_64kunrestr, trcapc_video

    // ISUP only, ISUP default
    // ISUP ForwardCallIndicator - ISUP preference ind.
    uc      fcipref;    // fcipref_pref, fcipref_notreq, fcipref_req

public:
    CallFlags()
    {
        clear();
    }

    CallFlags(const CallFlags* src)
    {
        if (src)
        {
            trcap = src->trcap;
            fcipref = src->fcipref;
        }
        else
        {
            setnone();
        }
    }

    void  clear()  {
        trcap   = 0xFF;
        fcipref = 0xFF;
    }
    void  setnone()  {
        trcap   = 0xFE;
        fcipref = 0xFE;
    }
    bool operator==(const UniN::CallFlags& two) const {
        return (trcap == two.trcap || trcap == 0xFF || 0xFF == two.trcap) &&
               (fcipref == two.fcipref || fcipref == 0xFF || 0xFF == two.fcipref);
    }

    void applyFlags ( CallFlags& flags);
};


class NumberFlags {

public:
    // Numbering plan
    uc      nbplan;     // nbplan_unwn, nbplan_isdn, nbplan_data, nbplan_telex, nbplan_natnl, nbplan_pvt

    // DSS1 Type of number  [default]
    // ISUP Nature of address indicator
    uc      typenb;     // typenb_unwn, typenb_intnl, typenb_natnl, typenb_netspec, typenb_subscr

    // Screening indicator
    uc      scrind;     // scrind_notscr, scrind_vernpas, scrind_vernfail, scrind_netprov

    // Presentation indicator
    uc      presind;    // presind_allow, presind_restr, presind_notavail

    // ISUP only
    uc      innind;     // innind_allow, innind_notallow
    uc      niind;      // niind_compl, niind_incompl

    //
    uc      ReasonForRedirection;   //  для DSS1 redirecting number

public:
    NumberFlags() {
        clear();
    }
    void  clear() {
        nbplan  = 0xFF;
        typenb  = 0xFF;
        scrind  = 0xFF;
        presind = 0xFF;
        innind  = 0xFF;
        niind   = 0xFF;
        ReasonForRedirection = 0xFF;
    }
    void  setnone() {
        nbplan  = 0xFE;
        typenb  = 0xFE;
        scrind  = 0xFE;
        presind = 0xFE;
        innind  = 0xFE;
        niind   = 0xFE;
        ReasonForRedirection = 0xFE;
    }

    bool operator==(const UniN::NumberFlags& two) const;
    void operator=(NumberFlags& src);

};


//
struct Number {

public:
    uc              numberType;     // original number type

    NumberFlags     flags;

    bool            numberComplete;
    size_t          len;
    char            number[MAX_NUMBER_LENGTH+1];

public:
    Number();   //uc numberType);
    Number(const Number* src);   //uc numberType);

    //
    // Common function
    void  setNumber(const char* digits, bool numberComplete = false);
    void  setNumber(const char* digits, bool numberComplete, int len );
    void  clearNumber();    // только номер
    void  clear();          // вместе с флагами

    void  addDigit(char digit);
    void  addDigits(const char* digits);
    bool  insertDigit(size_t pos, char digit);
    bool  insertDigits(size_t  pos, const char* digits);
    bool  erase(size_t pos, size_t count);

    void  applyFlags(NumberFlags& flags);

    int   realSize() const { return sizeof(Number) - MAX_NUMBER_LENGTH + len; };  // возвращет реально необходимую длину

    const char* getNumberPtr() const { return number; };
    char operator[](int n) const { return number[n]; };
    char& operator[](int n) { return number[n]; };

    void operator=(const Number& src);
};

};  // namespace UniN

#pragma pack (pop)

#endif
