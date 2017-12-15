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
#ifndef _PARSER_H
#define _PARSER_H

#include "CDRBuilder.h"
#include "radius_freeradius-client.h"
#include <netinet/in.h>


class CParser
{
    int ParseConigFile( const char *filename, CDRBuilding::TPCharList &CfgList );
    int check_d( const char *str );
    void FreeListMem(CDRBuilding::TPCharList &lst);
    bool make_nonblock(int fd);
    bool Pipe(int *filedes);
    int CheckIp(const char * ipstr);
    void PrintHelp(void);

public:

    char* sInDir;
    char* sOutDir;
    char* sInRm3FileName;
    char* sOutRm3FileName;
    CDRBuilding::TPCharList TfsFileList;
    char* sLogFileName;
    char* sErrFileName;
    char* sJrnFileName;
    char* sTfsFileNameBase;
    char* sLogFile;
    unsigned int nLogFileSize;
    unsigned char tfsFileType;
    unsigned char rotation;
    tm Tm;

    char* sSpiderIp;
    in_addr_t SpiderPort;
    in_addr_t ServerPort;

    bool fConvert;
    bool fRem_rm3;

    bool fDaemon;

    struct CDRBuilding::strCDRBuilderSettings SSettings;
    struct CDRBuilding::strJournalSettings Sett;
    CDRBuilding::TListInterval lstLocalNumbers;
    CDRBuilding::strPrefix SPrefix;

    //radius params
    char sRadiusAccIp[100];
    unsigned short nRadiusAccPort;
    char sRadiusAccSecret[100];
    unsigned int nRadiusAccBilling;

    char sRadiusAuthIp[100];
    unsigned short nRadiusAuthPort;
    char sRadiusAuthSecret[100];
    unsigned int nRadiusAuthBilling;
    char sScommIp[100];
    unsigned short nScommPort;
    char sScommPassword[20];
    bool fRadAuthPrepay;

    char sRadiusConfigFile[100];
    char sRadiusDictionaryFile[100];
    char sRadiusSeqFile[100];


    unsigned int nRadiusSndRetry;
    unsigned int nRadiusSndTimeout;
    rc_handle *rh;

    bool fSS;
    bool fNoTarif;
    bool fRadTrace;

    int ParseCStringParams (int argc, char *argv[], bool fFromFile);
    int Prepare( void );
    int FillMainParams (const char *CurrTfsFileName);
    int FillOnLineParams (const char *CurrTfsFileName);
    int RefreshResidFileName (void);

    CParser();
    ~CParser();
};

extern CParser Parser;

#endif
