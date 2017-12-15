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

int main(int argc, char *argv[])
{
	CParser Parser;
	if(Parser.ParseCStringParams(argc, argv) <0)
		exit(EXIT_FAILURE);
	Parser.RefreshResidFileName();
	CDialog::Visualisation(Parser);
	int fd_tfs;
	if ((fd_tfs = open(Parser.sMainFile, O_RDONLY)) <= 0)
	{
		printf("Error: can't open tfs file:%s!\n",Parser.sMainFile);
		exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
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
			exit(EXIT_FAILURE);
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
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sErrFileName,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to err-file\n");
			exit(EXIT_FAILURE);
		}
		builder.FreeListMem(lstStr);

		builder.GetJournalList(lstStr);
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sJrnFileName,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to jrn-file\n");
			exit(EXIT_FAILURE);
		}
		builder.FreeListMem(lstStr);

		builder.GetCDRList(lstStr);
		if(!CDialog::WriteStringsToFile(Parser.sOutDir,Parser.sLogFileName,lstStr))
		{
			close(fd_tfs);
			CDialog::ErrorActions(Parser, builder, "Error: can't write to jrn-file\n");
			exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
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
	if (Parser.fConvert)
	{
		CDialog::Converter(Parser.sOutDir, Parser.sErrFileName);
		CDialog::Converter(Parser.sOutDir, Parser.sJrnFileName);
	}
	if (Parser.fRem_rm3)
	{
		remove(Parser.sResidFile);
	}
	return EXIT_SUCCESS;
}

