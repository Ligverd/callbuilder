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
CParser::CParser()
{
	sOutDir = NULL;
	sSpiderIp = NULL;
	sMainFile = NULL;
	sResidFile = NULL;
	sFileName = NULL;
	sFreshResidFile = NULL;
	sLogFile = NULL;
	sErrFile = NULL;
	sJrnFile = NULL;
	tfsFileType = 0;
	memset(&Tm, 0, sizeof(tm));	

	pSett.bEnable = true;
	pSett.iMaxDuration = 20000;
	fMaxDurChanged = false;
	
	pSSettings.bWriteBinary2 = true;
	pSSettings.bWriteUnansweredCalls = true;
	strcpy(pSSettings.sNoNumberString,"-");
	pSSettings.btCDRStringFormat = FORMAT_CDR_EX;
	pSSettings.bDecrementDuration = false;
	pSSettings.sSampleString[0] = 0;
}
CParser::~CParser()
{
	free(sOutDir);
	free(sSpiderIp);
	free(sMainFile);
	free(sResidFile);
	free(sLogFile);
	free(sErrFile);
	free(sJrnFile);
	free(sFreshResidFile);
	free(sFileName);
}

int CParser::check_d(const char *str)
{
	unsigned char i;
	for (i = 0; i < strlen(str); i++)
		if (str[i] > '9' || str[i] < '0')
			return 0;
	return 1;
}

int CParser::RefreshResidFileName()
{
	char str[128];
	time_t itime;
	tm T;
	memmove(&T,&Tm,sizeof(tm));
	//выясняем что за ротация у открываемого файла
	//name_01_02_2004.tfs 1
	//name_mon02_2004.tfs 2
	//name_dec02_2004.tfs 3
	//либо может быть файл со флешки - например 12_10_6.TFS 4
	switch(tfsFileType)
	{
		case 1://day
			if(!sFileName) RETURNW;
			T.tm_year -= 1900;
			itime = mktime (&T);
			itime += 86400;
			localtime_r(&itime,&T);
			sprintf(str,"%s%s_%02d_%02d_%04d.rm3",sOutDir,sFileName,T.tm_mday,T.tm_mon+1,T.tm_year+1900);
		break;
		case 2://mon
			if(!sFileName) RETURNW;
			T.tm_mon++;
			if(T.tm_mon > 11)
			{
				T.tm_mon = 0;
				T.tm_year++;
			}
			sprintf(str,"%s%s_mon_%02d_%04d.rm3",sOutDir,sFileName,T.tm_mon+1,T.tm_year);
		break;
		case 3://dec
			if(!sFileName) RETURNW;
			T.tm_yday++;
			if(T.tm_yday > 37)
			{
				T.tm_yday = 1;
				T.tm_year++;
			}
			sprintf(str,"%s%s_dec_%02d_%04d.rm3",sOutDir,sFileName,T.tm_yday,T.tm_year);
		break;
		case 4://realtime
			T.tm_year += 100;
			itime = mktime (&T);
			itime += 86400;
			localtime_r(&itime,&T);
			sprintf(str,"%s%02d_%02d_%01d.rm3",sOutDir,T.tm_mday,T.tm_mon+1,T.tm_year-100);
		break;
		default:
			RETURNERR("Parser: Error tfs file type!\n");
	}
	if(sFreshResidFile) free(sFreshResidFile);
	sFreshResidFile = (char*)malloc(strlen(str)+1);
	if (!sFreshResidFile)
		RETURNERR("Parser: No memory to refresh residuary file name!\n");
	memmove(sFreshResidFile,str,strlen(str)+1);
	return tfsFileType;
}

int CParser::ParseCStringParams (int argc, char *argv[])
{
	unsigned char i;
	for( i=1; i<argc-1; i++)
	{
		if(!strcmp(argv[i],"-outdir"))
		{
			i++;
			sOutDir = (char*)malloc(strlen(argv[i])+1);
			if (!sOutDir)
				RETURNERR("Parser: No memory for parsing parameter: -outdir!\n");
			//memset(sOutDir, 0, strlen(argv[i])+1);
			strcpy(sOutDir, argv[i]);
		}
		else if(!strcmp(argv[i],"-spiderip"))
		{
			i++;
			sSpiderIp = (char*)malloc(strlen(argv[i])+1);
			if (!sSpiderIp)
				RETURNERR("Parser: No memory for parsing parameter: -spiderip!\n");
			//memset(sSpiderIp, 0, strlen(argv[i])+1);
			strcpy(sSpiderIp,argv[i]);
		}
		else if(!strcmp(argv[i],"-nojournal"))
		{
			pSett.bEnable = false;
		}
		else if(!strcmp(argv[i],"-maxduration"))
		{
			i++;
			if (check_d(argv[i]))
			{
				pSett.iMaxDuration = atoi(argv[i]);
				fMaxDurChanged = true;
			}
			else
				RETURNERR("Parser: Incorrect duration value!\n");
			//memset(sSpiderIp, 0, strlen(argv[i])+1);
			strcpy(sSpiderIp,argv[i]);
		}
		else if(!strcmp(argv[i],"/0"))
		{
			pSSettings.bWriteUnansweredCalls = false;
		}
		else if(!strcmp(argv[i],"-str2boff"))
		{
			pSSettings.bWriteBinary2 = false;
		}
		else if(!strcmp(argv[i],"-tfsfile"))
		{
			i++;
			char* ext;
			if(!(ext = strrchr( argv[i], (int)'/')))
			{
				int j;
				char *pwd = NULL;
				for(j = 0; environ[j], !pwd; j++)
				{
					if( pwd = strstr(environ[j],"PWD="))
					{
						pwd += strlen("PWD="); 
						sMainFile = (char*)malloc(strlen(pwd)+strlen("/")+strlen(argv[i])+1);
						if (!sMainFile)
							RETURNERR("Parser: No memory for parsing parameter: -tfsfile!\n");
						strcpy(sMainFile,pwd);
						strcat(sMainFile,"/");
						strcat(sMainFile,argv[i]);
					}
				}
				if(!pwd)
					RETURNERR("Parser: enter full path of tfs file!\n");
			}
			else
			{
				sMainFile = (char*)malloc(strlen(argv[i])+1);
				if (!sMainFile)
					RETURNERR("Parser: No memory for parsing parameter: -tfsfile!\n");
				strcpy(sMainFile,argv[i]);
			}
			sResidFile = (char*)malloc(strlen(sMainFile)+1);
			if (!sResidFile)
				RETURNERR("Parser: No memory to genrate residuary file name!\n");
			strcpy(sResidFile,argv[i]);

			ext = strrchr( sResidFile, (int)'.');
			if(ext)
			{
				ext++;
				if( !strcmp(ext,"tfs") || !strcmp(ext,"TFS") )
					strcpy(ext,"rm3");
				else RETURNERR("Parser: Main file not tfs!\n");
			}
			else RETURNERR("Parser: Main file have no extention!\n");
			
//получим имя файла name_01_02_2004.tfs | name_mon02_2004.tfs | name_dec02_2004.tfs | 12_10_6.TFS
			ext = new char[strlen(strrchr( sMainFile, (int)'/')+1)+1];
			if (!ext)
				RETURNERR("Parser: No memory to determine tfs file type!\n");
			memmove(ext, strrchr(sMainFile, (int)'/')+1, strlen(strrchr( sMainFile, (int)'/')+1)+1);//with 0
			
			sLogFile = (char*)malloc(strlen(ext)+1);
			if (!sLogFile)
				RETURNERR("Parser: No memory to create log file name!\n");

			sErrFile = (char*)malloc(strlen(ext)+1);
			if (!sErrFile)
				RETURNERR("Parser: No memory to create err file name!\n");

			sJrnFile = (char*)malloc(strlen(ext)+1);
			if (!sJrnFile)
				RETURNERR("Parser: No memory to create jrn file name!\n");

			char *str = strrchr( ext, (int)'.');
			memset(str,0,strlen(str)); //name_01_02_2004 | name_mon02_2004 | name_dec02_2004 | 12_10_6
			
			strcpy(sLogFile,ext);
			strcat(sLogFile,".log");
			strcpy(sErrFile,ext);
			strcat(sErrFile,".err");
			strcpy(sJrnFile,ext);
			strcat(sJrnFile,".jrn");
			//выясняем что за ротация у открываемого файла
			//name_01_02_2004.tfs 1
			//name_mon02_2004.tfs 2
			//name_dec02_2004.tfs 3
			//либо может быть файл со флешки - например 12_10_6.TFS 4
			unsigned int tmptm;
			str = strrchr(ext, (int)'_');
			if (!str)
				RETURNW;
			if (!(tmptm = atoi(str+1)) && !(strlen(str+1) == 1 && str[1] == '0')) //? 2004 | 6
				RETURNW;
			Tm.tm_year = tmptm; 
			memset(str,0,strlen(str)); //name_01_02 | name_mon02 | name_dec02 | 12_10
			
			str = strrchr( ext, (int)'_'); //_02 | _mon02 | _dec02 | _10
			if (!str)
				RETURNW;

			if(strstr(str+1,"mon") &&  (tmptm = atoi(str+4)))
			{
				Tm.tm_mon = --tmptm;
				tfsFileType = 2;
				memset(str,0,strlen(str)); //name
				sFileName = (char*)malloc(strlen(ext)+1);
				if (!sFileName)
					RETURNW;
				memmove(sFileName, ext, strlen(ext)+1);
			}
			else if(strstr(str+1,"dec") && (tmptm = atoi(str+4)))
			{
				Tm.tm_yday = tmptm;
				tfsFileType = 3;
				memset(str,0,strlen(str)); //name
				sFileName = (char*)malloc(strlen(ext)+1);
				if (!sFileName)
					RETURNW;
				memmove(sFileName, ext, strlen(ext)+1);
			}
			else if(tmptm =  atoi(str+1))
			{
				Tm.tm_mon = --tmptm;
				memset(str,0,strlen(str)); //name_01 | 12
				str = strrchr( ext, (int)'_');
				if (str && (tmptm = atoi(str+1)))
				{
					Tm.tm_mday = tmptm;
					tfsFileType = 1;
					memset(str,0,strlen(str)); //name
					sFileName = (char*)malloc(strlen(ext)+1);
					if (!sFileName)
						RETURNW;
					memmove(sFileName, ext, strlen(ext)+1);
				}
				else if (tmptm = atoi(ext))
				{ 
					Tm.tm_mday = tmptm;
					tfsFileType = 4;
				}
				else RETURNW; 
			}
			else RETURNW;
			delete [] ext;
		}
	}
	if(!sMainFile)
		RETURNERR("Parser: No imput file!\n");
	if(!sOutDir)
	{
		sOutDir = (char*)malloc(strlen(sMainFile)-strlen(strrchr( sMainFile, (int)'/')+1)+1);
		memset(sOutDir,0,strlen(sMainFile)-strlen(strrchr( sMainFile, (int)'/')+1)+1);
		if (!sOutDir)
			RETURNERR("Parser: No memory for parameter outdir!\n");
		memmove(sOutDir,sMainFile,strlen(sMainFile)-strlen(strrchr( sMainFile, (int)'/')+1));
	}
	return 1;
}
