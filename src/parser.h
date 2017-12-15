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
#include "CDRBuilder.h"

class CParser
{
	static int check_d(const char *str);
public:
	char* sMainDir;
	char* sOutDir;
	char* sMainFile;
	char* sResidFile;
	char* sFreshResidFile;
	char* sLogFileName;
	char* sErrFileName;
	char* sJrnFileName;
	char* sMainFileName;
	char* sSpiderIp;
	unsigned char tfsFileType;
	struct CDRBuilding::strJournalSettings pSett;
	bool fMaxDurChanged;
	bool fFormatChanged;
	struct CDRBuilding::strCDRBuilderSettings pSSettings;
	tm Tm;
	int RefreshResidFileName (void);
	int ParseCStringParams (int argc, char *argv[]);
	bool fConvert;
	bool fRem_rm3;
	CParser();
	~CParser();
};
