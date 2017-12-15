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
void CDialog::RemoveFiles(CParser &Parser)
{
	char path[256];
	sprintf(path,"%s%s",Parser.sOutDir,Parser.sLogFileName);
	remove(path);
	sprintf(path,"%s%s",Parser.sOutDir,Parser.sErrFileName);
	remove(path);
	sprintf(path,"%s%s",Parser.sOutDir,Parser.sJrnFileName);
	remove(path);
}

bool CDialog::WriteStringsToFile(const char* sDir ,const char* sFileName, CDRBuilding::TPCharList lst)
{
	if(lst.empty())
	{
		return true;
	}
    bool bRes = true;
	char path[256];
	//memset(path,0,sizeof(path));
	sprintf(path,"%s%s",sDir,sFileName);
    int fd;
	if ( (fd = open(path, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
	{
		printf("Error: can't write to file:%s!\n",sFileName);
		return false;
	}

	CDRBuilding::TPCharList::iterator it;

	char sTmp[1502];
	memset(sTmp,0,sizeof(sTmp));
	for(it = lst.begin(); it != lst.end(); it++)
	{
		if(strlen(*it) >= 1500)
		{
			printf("Error: string is reater then 1500 characters!\n");
			bRes = false;
			break;
		}
		else
		{
			sprintf(sTmp, "%s\n", *it);
		    if( write(fd,sTmp,strlen(sTmp))<0)
			{
				bRes = false;
				break;
			}
		}
	}

    close(fd);
    return bRes;
}

void CDialog::ErrorActions(CParser &Parser, const CDRBuilding::CCDRBuilder &builder, char* sMessage)
{
		printf("%s",sMessage);
		if (Parser.fConvert)
		{
			CDialog::Converter(Parser.sOutDir, Parser.sErrFileName);
			CDialog::Converter(Parser.sOutDir, Parser.sJrnFileName);
		}
		//��������� ���������� ������
		Parser.RefreshResidFileName();
		SaveResiduaryCalls(Parser.sFreshResidFile, builder);
}
int CDialog::SaveResiduaryCalls(const char* sFileName, const CDRBuilding::CCDRBuilder& builder)
{
	int fd,n,ret = 1;
	if ( (fd = open(sFileName, O_WRONLY | O_CREAT | O_TRUNC , 0666)) < 0)
	{
		printf("Error: can't create file to save residuary calls!\n");
		return -1;
	}
	CDRBuilding::strModuleInfo ModInfo[MAX_MOD];
	CDRBuilding::TListPSCallInfo lstCalls;
	builder.GetResiduaryCalls(ModInfo, lstCalls);
	n = write(fd, ModInfo, sizeof(CDRBuilding::strModuleInfo)*MAX_MOD);
	
	if(n != sizeof(CDRBuilding::strModuleInfo)*MAX_MOD)
		RETURNERR("Error: error writing to residuary file!\n");
	CDRBuilding::TListPSCallInfo::iterator it;
	for(it = lstCalls.begin(); it != lstCalls.end(); it++)
	{
		n = write(fd, *it, sizeof(CDRBuilding::strCallInfo));
		delete *it;
		if(n != sizeof(CDRBuilding::strCallInfo))
			RETURNERR("Error: error while writing to residuary file!\n");
	}
	close(fd);
	return 1;
	//builder.CleanCalls();
}

int CDialog::PutResiduaryCalls(const char* sFileName, CDRBuilding::CCDRBuilder& builder)
{
	//��������� ������
	int fd;
	if ( (fd = open(sFileName, O_RDONLY)) < 0)
		RETURNERR("Warning: No input residuary file!\n");
	CDRBuilding::strModuleInfo ModInfo[MAX_MOD];
	CDRBuilding::TListPSCallInfo lstCalls;
	if(read(fd, ModInfo, sizeof(CDRBuilding::strModuleInfo)*MAX_MOD) ==
	sizeof(CDRBuilding::strModuleInfo)*MAX_MOD)
	{
		CDRBuilding::strCallInfo* pSCI;
		pSCI = new CDRBuilding::strCallInfo;
		while(read(fd, pSCI, sizeof(CDRBuilding::strCallInfo)) == sizeof(CDRBuilding::strCallInfo))
		{
			lstCalls.push_back(pSCI);
			pSCI = new CDRBuilding::strCallInfo;
		}
		delete pSCI;
		close(fd);
		builder.PutResiduaryCalls(ModInfo, lstCalls);
		//������� ������
		CDRBuilding::TListPSCallInfo::iterator it;
		for(it = lstCalls.begin(); it != lstCalls.end(); it++)
			delete *it;
	}
	else 
		RETURNERR("Error: error loading residuary file!\n");
	close(fd);
	return 1;
}

void CDialog::RestoreMessage(int &fd_tfs, CParser &Parser, CDRBuilding::CCDRBuilder& builder, DWORD &iReadBytes, DWORD &dwLen, CDRBuilding::TPCharList &lstStr)
{
	iReadBytes -= 4;
	lseek(fd_tfs,-4, SEEK_CUR);
	builder.OnComoverload(ALL_MODULES);

	char *sTmpJrnStr = new char[128];
	if (!sTmpJrnStr) 
		WARNING;
	memset(sTmpJrnStr,0,sizeof(sTmpJrnStr));
	sprintf(sTmpJrnStr, "Error message lengh (%d) at address:0x%08X ", dwLen, iReadBytes);
	//������ � err
	lstStr.push_back(sTmpJrnStr);
	if(!WriteStringsToFile(Parser.sOutDir, Parser.sErrFileName, lstStr))
	{
		close(fd_tfs);
		ErrorActions(Parser, builder, "Error: can't write to err-file\n");
		exit(EXIT_FAILURE);
	}
	builder.FreeListMem(lstStr);
	//����� ��� tfs-���� �� ������, ����� ����� ���������� �������� ��������� ALIVE
	int iSpiderStartLen = sizeof(TClock) + sizeof(BYTE) + sizeof(BYTE) + sizeof(TCallID) + sizeof(BYTE);
	//iSpiderStartLen = 15;
	int iCnt = 0;
	int dwTmp;
	BYTE btTmp;
	BYTE btExit = 0;
	DWORD dwMesLen = 0xFFFFFFFF;
	while(true)
	{
		dwTmp = read(fd_tfs,&btTmp,sizeof(BYTE));
		
		if(dwTmp < sizeof(BYTE))//���� ����� ����� ��� ������
		{
			close(fd_tfs);
			ErrorActions(Parser, builder, "Error: can't find correct message lengh\n");
			exit(EXIT_FAILURE);
		}
		iCnt++;
		iReadBytes++;
		dwMesLen = dwMesLen >> 8;
		dwMesLen += ((DWORD)btTmp << 24);
		//(BYTE*)&dwMesLen+3 = btTmp;
		if(dwMesLen == iSpiderStartLen)
		{//��� ������ ���� �������� - ������ 4 ����� ����� �������
			sTmpJrnStr = new char[128];
			if (!sTmpJrnStr) 
				WARNING;
			//memset(sTmpJrnStr,0,sizeof(sTmpJrnStr));
			sprintf(sTmpJrnStr, "Found correct message lengh (%d) after %d bytes", iSpiderStartLen,iCnt);
			//������ � err
			lstStr.push_back(sTmpJrnStr);
			if(!WriteStringsToFile(Parser.sOutDir, Parser.sErrFileName, lstStr))
			{
				close(fd_tfs);
				ErrorActions(Parser, builder, "Error: can't write to err-file\n");
				exit(EXIT_FAILURE);
			}
			builder.FreeListMem(lstStr);
			dwLen = dwMesLen;
			return;
		}
	}
}

void CDialog::Visualisation(CParser &Parser)
{
	printf("\n<-------------------------------Callbuilder v0.4---------------------------->\n\n");
	if(Parser.sMainFile)
	{
		printf("Main tfs file:%s\n",Parser.sMainFile);
		switch(Parser.tfsFileType)
		{
			case 1:
				printf("Rotation:day\n");
			break;
			case 2:
				printf("Rotation:month\n");
			break;
			case 3:
				printf("Rotation:decade\n");
			break;
			case 4:
				printf("Rotation:realtime\n");
			break;
			default:
			{
				WARNING;
				printf("Warning:Rotation:unknown!\n");
			}
		}
	}
	//if(Parser.sMainDir) printf("Main directory:%s\n",Parser.sMainDir);
	if(Parser.sResidFile) printf("Residuary file:%s\n",Parser.sResidFile);
	if(Parser.sOutDir) printf("Output directory:%s\n",Parser.sOutDir);
	//if(Parser.sFileName) printf("Name of files:%s\n",Parser.sFileName);
	if(Parser.sLogFileName) printf("Log file name:%s\n",Parser.sLogFileName);
	if(Parser.sErrFileName) printf("Err file name:%s\n",Parser.sErrFileName);
	if(Parser.sJrnFileName) printf("Jrn file name:%s\n",Parser.sJrnFileName);
	//if(Parser.sSpiderIp) printf("Spider ip address:%s\n",Parser.sSpiderIp);
	if(!Parser.pSSettings.bWriteUnansweredCalls) printf("Unanswered calls are not writing!\n");
	if(!Parser.pSett.bEnable) printf("Journal is off!\n");
	if(Parser.fMaxDurChanged) printf("Journal maximum busy duration %u\n",Parser.pSett.iMaxDuration);
	if(!Parser.pSSettings.bWriteBinary2) printf("No binary 2 at logfile strings!\n");
	if(!Parser.fConvert) printf("No converted files!\n");
	if(Parser.fRem_rm3) printf("Removing rm3 files after success!\n");
	if(Parser.fFormatChanged) printf("CDR string format:%d\n",Parser.pSSettings.btCDRStringFormat);
	if(strcmp(Parser.pSSettings.sNoNumberString,"-"))
		printf("No number string separator:\"%s\"\n",Parser.pSSettings.sNoNumberString);
	printf("\n<--------------------------------------------------------------------------->\n\n");
}

int CDialog::Converter(const char* sDir ,const char* sFileName)
{
	int fd_in,fd_out;
	char path[512],c;
	int Tmp;
	sprintf(path,"%s%s",sDir,sFileName);
	if ( (fd_in = open(path, O_RDONLY)) < 0)
	{

		printf("Converter: can't open file:%s\n",path);
		return -1;
	}
	strcat(path,".converted");
	if ( (fd_out = open(path, O_WRONLY | O_CREAT | O_TRUNC , 0666)) < 0)
	{
		printf("Converter: can't create file:%s\n",path);
		return -1;
	}
	while (true)
	{
		Tmp = read(fd_in, &c, 1);
		if(Tmp != 1)
		{
			if(Tmp)
				RETURNERR("Converter: error reading input file!\n");
			break;
		}
		c = ConvertTable(c);
		Tmp = write(fd_out, &c, 1);
		if(Tmp != 1)
			RETURNERR("Converter: error writing output file!\n");
	}
	printf("File:%s converted!\n",sFileName);
}

char CDialog::ConvertTable(const char c)
{
	switch((unsigned char)c)
	{
		case 0xFE: return '�';//Small
		case 0xE0: return '�';
		case 0xE1: return '�';
		case 0xF6: return '�';
		case 0xE4: return '�';
		case 0xE5: return '�';
		case 0xF4: return '�';
		case 0xE3: return '�';
		case 0xF5: return '�';
		case 0xE8: return '�';
		case 0xE9: return '�';
		case 0xEA: return '�';
		case 0xEB: return '�';
		case 0xEC: return '�';
		case 0xED: return '�';
		case 0xEE: return '�';
		case 0xEF: return '�';
		case 0xFF: return '�';
		case 0xF0: return '�';
		case 0xF1: return '�';
		case 0xF2: return '�';
		case 0xF3: return '�';
		case 0xE6: return '�';
		case 0xE2: return '�';
		case 0xFC: return '�';
		case 0xFB: return '�';
		case 0xE7: return '�';
		case 0xF8: return '�';
		case 0xFD: return '�';
		case 0xF9: return '�';
		case 0xF7: return '�';
		case 0xFA: return '�';
		case 0xDE: return '�';//Big
		case 0xC0: return '�';
		case 0xC1: return '�';
		case 0xD6: return '�';
		case 0xC4: return '�';
		case 0xC5: return '�';
		case 0xD4: return '�';
		case 0xC3: return '�';
		case 0xD5: return '�';
		case 0xC8: return '�';
		case 0xC9: return '�';
		case 0xCA: return '�';
		case 0xCB: return '�';
		case 0xCC: return '�';
		case 0xCD: return '�';
		case 0xCE: return '�';
		case 0xCF: return '�';
		case 0xDF: return '�';
		case 0xD0: return '�';
		case 0xD1: return '�';
		case 0xD2: return '�';
		case 0xD3: return '�';
		case 0xC6: return '�';
		case 0xC2: return '�';
		case 0xDC: return '�';
		case 0xDB: return '�';
		case 0xC7: return '�';
		case 0xD8: return '�';
		case 0xDD: return '�';
		case 0xD9: return '�';
		case 0xD7: return '�';
		case 0xDA: return '�';
		default: return c;
	}
}


