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
#ifndef _CDRRadius_
#define _CDRRadius_

#define BILLING_LANBILLING      0
#define BILLING_UTM5            1

#include "CDRBuilder.h"
#include "parser.h"
#include <pthread.h>

void TClock2tm (struct tm *T, const struct TClock *cl);
void tm2TClock (struct TClock *cl, const struct tm *T);
void AddSecToTClock(TClock *cl, int iDelta);
void AccRadiusCDR(const CParser *_parser, const CDRBuilding::strCDR& _CDR);
void AuthRadius(const CParser *_parser, const CDRBuilding::strCallInfo* _pCall);

#ifndef ARMLINUX

#include "unipar.h"

struct SUniparAttr
{
    DWORD len;
    DWORD id;
    DWORD mod;
    DWORD time;
};

void AuthRadiusPrepay(const CParser *_parser, const CUniPar *_unipar, const struct SUniparAttr *_UniparAttr);

#endif

class CAccRadius
{

private:

    volatile bool fDelete;
    const CParser *pParser;
    pthread_t thr;
    CDRBuilding::strCDR CDR;

    static void *_ThreadProc(void* lpParameter);
    void ThreadProc ( void );

public:
    CAccRadius(const CParser *_parser, const CDRBuilding::strCDR& _CDR, pthread_t *_thr = NULL);
    ~CAccRadius();

    void  SetThreadPriority( int nPolicy, int nPriority );
    pthread_t GetThread ( void ) { return thr; }

};

class CAuthRadius
{

private:

    volatile bool fDelete;
    const CParser *pParser;
    pthread_t thr;

    CDRBuilding::strCallInfo Call;

    static void *_ThreadProc(void* lpParameter);
    void ThreadProc ( void );

public:
    CAuthRadius(const CParser *_parser, const CDRBuilding::strCallInfo* _pCall, pthread_t *_thr = NULL);
    ~CAuthRadius();


    void  SetThreadPriority( int nPolicy, int nPriority );
    pthread_t GetThread ( void ) { return thr; }

};

#ifndef ARMLINUX

class CAuthRadiusPrepay
{

private:

    volatile bool fDelete;
    const CParser *pParser;
    pthread_t thr;

    CUniPar unipar;
    struct SUniparAttr UniparAttr;

    static void *_ThreadProc(void* lpParameter);
    void ThreadProc ( void );

public:
    CAuthRadiusPrepay(const CParser *_parser, const CUniPar *_unipar, const struct SUniparAttr *_UniparAttr, pthread_t *_thr = NULL);
    ~CAuthRadiusPrepay();


    void  SetThreadPriority( int nPolicy, int nPriority );
    pthread_t GetThread ( void ) { return thr; }

};

#endif //ARMLINUX

#endif