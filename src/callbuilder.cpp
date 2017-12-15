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

#include "callbuilder.h"
extern char **environ;
using namespace std;

#define day 86400
int main(int argc, char *argv[])
{
	CParser Parser;
	if(Parser.ParseCStringParams(argc, argv) <0)
		exit(1);
	Parser.RefreshResidFileName();
	CDialog::Visualisation(Parser);
	int fd_tfs;
	if ((fd_tfs = open(Parser.sMainFile, O_RDONLY)) <= 0)
	{
		printf("Error: can't open tfs file!\n");
		exit(1);
	}
	
	off_t MainFilelen = 0;
	MainFilelen = lseek(fd_tfs, 0, SEEK_END);
	lseek(fd_tfs, 0, SEEK_SET);
	
	CDialog::RemoveFiles(Parser);
	
	CDRBuilding::CCDRBuilder builder;
	builder.SetCommonSettings(&Parser.pSSettings);
	builder.SetJournalSettings(&Parser.pSett);
	CDialog::PutResiduaryCalls(Parser.sResidFile, builder);
		
	int dwTmp = 0;
	DWORD dwLen = 0;
	DWORD dwError = 0;
	DWORD iReadBytes = 0;
	unsigned char RdPersent = 1;
	CDRBuilding::TPCharList lstStr;
	
	char sTmp[128];
	strcpy(sTmp,"");
	while(true)
	{
		//chtenije dlinni mesagi
		dwTmp = read(fd_tfs,&dwLen,sizeof(DWORD));
		if(dwTmp != sizeof(DWORD))
		{
			if (dwTmp)
			{ 
				close(fd_tfs);
				CDialog::ErrorActions(Parser, builder, "Error: error while reading message size!\n");
				exit(1);
			}
			break;
		}
		iReadBytes += sizeof(DWORD);

		if( !dwLen || dwLen > sizeof(CMonMessageEx)) // «“’¬¡— –“œ◊≈“À¡
		{
			CDialog::RestoreMessage(fd_tfs, Parser, builder, iReadBytes, dwLen, lstStr);
		}
		//chtenije mesagi
		unsigned char buff[sizeof(CMonMessageEx)];
		dwTmp = read(fd_tfs,&buff,dwLen);

		if(!dwTmp || dwTmp != dwLen )
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: error while reading message!\n");
			exit(1);
		}
		iReadBytes += dwLen;

		//dekodirovanie mesagi
		CMonMessageEx mes(0);
		BYTE *p;
		if (!(p = mes.decode(buff, dwLen)))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: error while decoding message!\n");
			exit(1);
		}

		//obrabotka mesagi
		if(!builder.ProcessMessage(mes))
		{
			dwError = builder.GetLastError();
		}
		else
			dwError = ERROR_NONE;
	
		//zapis mesagi v faili
		builder.GetErrorList(lstStr);
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sErrFile,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to err-file\n");
			exit(1);
		}
		builder.FreeListMem(lstStr);

		builder.GetJournalList(lstStr);
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sJrnFile,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to jrn-file\n");
			exit(1);
		}
		builder.FreeListMem(lstStr);

		builder.GetCDRList(lstStr);
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sLogFile,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to jrn-file\n");
			exit(1);
		}
		builder.FreeListMem(lstStr);

		if(dwError)
		{//ÂÒÎË Ó¯Ë·Í‡ ÔË Ó·‡·ÓÚÍÂ ÒÓÓ·˘ÂÌËˇ
			strcpy(sTmp,"");
			switch(dwError)
			{
				case ERROR_UNKNOWN_MESSAGE:
					break;
				case ERROR_UNKNOWN_CDR_FORMAT:
					strcpy(sTmp, "Error: unknown CDR-string format!\n");
					break;
					default:
					strcpy (sTmp, "Error: unknown error while message processing!\n");
			}
			if(strcmp(sTmp,""))
			{
				close(fd_tfs);
				CDialog::ErrorActions(Parser, builder, sTmp);
				exit(1);
			}
		}
		if((iReadBytes >= RdPersent*(MainFilelen/4)))
		{
			printf("Comlete:%d%\n",RdPersent*25);
			RdPersent++;
		}
	}
	//”œ»“¡Œ—≈Õ œ”‘¡◊€…≈”— ⁄◊œŒÀ…
	Parser.RefreshResidFileName();
	CDialog::SaveResiduaryCalls(Parser.sFreshResidFile, builder);
	builder.CleanCalls();
	printf("Processing complete!\n");
	CDialog::Converter(Parser.sOutDir, Parser.sErrFile);
	CDialog::Converter(Parser.sOutDir, Parser.sJrnFile);
	return EXIT_SUCCESS;
}

