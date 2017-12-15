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
#ifndef PARSER_H
#define PARSER_H
#include "CDRBuilder.h"
#include <netinet/in.h>

class CParser
{
	int check_d(const char *str);
	void FreeListMem(CDRBuilding::TPCharList &lst);
	bool make_nonblock(int fd);
	bool Pipe(int *filedes);
    int CheckIp(const char * ipstr);
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
    unsigned char tfsFileType;
    unsigned char rotation;
	tm Tm;

    char* sSpiderIp;
    in_addr_t SpiderPort;
    in_addr_t ServerPort;

	bool fConvert;
	bool fRem_rm3;

    bool fDaemon;

	struct CDRBuilding::strJournalSettings pSett;
	struct CDRBuilding::strCDRBuilderSettings pSSettings;
	
	int RefreshResidFileName (void);
	int ParseCStringParams (int argc, char *argv[]);
	int FillMainParams (const char *CurrTfsFileName);
    int FillOnLineParams (const char *CurrTfsFileName);
	CParser();
	~CParser();
};
#endif
