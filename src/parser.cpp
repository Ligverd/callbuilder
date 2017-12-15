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

CParser::CParser()
{
	sInDir = NULL;
	sOutDir = NULL;
	sInRm3FileName = NULL;
	sOutRm3FileName = NULL;
	sLogFileName = NULL;
	sErrFileName = NULL;
	sJrnFileName = NULL;
	sTfsFileNameBase = NULL;
    sLogFile = NULL;
	tfsFileType = 0;
    rotation = 0;
	memset(&Tm, 0, sizeof(tm));

    sSpiderIp = NULL;
    SpiderPort = 0;
    ServerPort = 0;

	pSett.bEnable = true;
	pSett.iMaxDuration = 20000;
	
	pSSettings.bWriteBinary2 = true;
	pSSettings.bWriteUnansweredCalls = true;
	strcpy(pSSettings.sNoNumberString,"-");
	pSSettings.btCDRStringFormat = FORMAT_CDR_EX;
	pSSettings.bDecrementDuration = false;
	pSSettings.sSampleString[0] = 0;
	fConvert = false;
	fRem_rm3 = false;
    fDaemon = false;
}
CParser::~CParser()
{
	if(sInDir) free(sInDir);
	if(sOutDir) free(sOutDir);
	if(sInRm3FileName) free(sInRm3FileName);
	if(sOutRm3FileName) free(sOutRm3FileName);
	if(sLogFileName) free(sLogFileName);
	if(sErrFileName) free(sErrFileName);
	if(sJrnFileName) free(sJrnFileName);
	if(sTfsFileNameBase) free(sTfsFileNameBase);
    if(sSpiderIp) free(sSpiderIp);
    if(sLogFile) free(sLogFile);
	FreeListMem(TfsFileList);
}

int CParser::check_d(const char *str)
{
	unsigned char i;
	for (i = 0; i < strlen(str); i++)
		if (str[i] > '9' || str[i] < '0')
			return 0;
	return 1;
}

void CParser::FreeListMem(CDRBuilding::TPCharList &lst)
{
	CDRBuilding::TPCharList::iterator it;
	for(it = lst.begin(); it != lst.end(); it++)
		free (*it);
	lst.clear();
}

bool CParser::make_nonblock(int fd)
{
	int fd_opt;
	if((fd_opt = fcntl(fd, F_GETFL, O_NONBLOCK)) <0) 
		return false;
	if((fd_opt = fcntl(fd, F_SETFL, fd_opt | O_NONBLOCK)) <0)
		return false;
	return true;
}

bool CParser::Pipe(int *filedes)
{
	if(pipe(filedes)<0)
		return false;
	return true;
}

int CParser::CheckIp(const char * ipstr)
{

    if (!strcmp(ipstr,"localhost"))
    {
        return 1;
    }

    int len = strlen(ipstr);

    if (len>6 && len<16)
    {
        char i=0,p=0,dot=0,s[len+1];
        memset(s,0,len+1);  
        while(i<=len)
        {
            if(i==len)
            {
                if(dot!=3) return 0;
                if((atoi(s)<0) || (atoi(s)>255)) return 0;
                break;
            }
            if (ipstr[i]=='.')
            {
                dot++;
                if((atoi(s)<0) || (atoi(s)>255) || p == 0) return 0;
                memset(s,0,p);
                i++;
                p=0;
            }
            if (ipstr[i]> '9' || ipstr[i]< '0')return 0;
            s[p++]=ipstr[i++];
        }
    }
    else return 0;
    return 1;
}

int CParser::RefreshResidFileName()
{
	char str[100];
	time_t itime;
	tm T;
	memcpy(&T,&Tm,sizeof(tm));
	//выясняем что за ротация у открываемого файла
	//name_01_02_2004.tfs 1
	//name_mon02_2004.tfs 2
	//name_dec02_2004.tfs 3
	//либо может быть файл со флешки - например 12_10_6.TFS 4
	switch (tfsFileType)
	{
		case ROTATION_DAY://day
			if(!sTfsFileNameBase) RETURNW;
			T.tm_year -= 1900;
			itime = mktime (&T);
			itime += 86400;
			localtime_r(&itime,&T);
			sprintf(str,"%s_%02d_%02d_%04d.rm3",sTfsFileNameBase,T.tm_mday,T.tm_mon+1,T.tm_year+1900);
		break;
		case ROTATION_MONTH://mon
			if(!sTfsFileNameBase) RETURNW;
			T.tm_mon++;
			if(T.tm_mon > 11)
			{
				T.tm_mon = 0;
				T.tm_year++;
			}
			sprintf(str,"%s_mon_%02d_%04d.rm3",sTfsFileNameBase,T.tm_mon+1,T.tm_year);
		break;
		case ROTATION_DECADE://dec
			if(!sTfsFileNameBase) RETURNW;
			T.tm_yday++;
			if(T.tm_yday > 37)
			{
				T.tm_yday = 1;
				T.tm_year++;
			}
			sprintf(str,"%s_dec_%02d_%04d.rm3",sTfsFileNameBase,T.tm_yday,T.tm_year);
		break;
		case ROTATION_FROM_ATS://from ats 
			T.tm_year += 100;
			itime = mktime (&T);
			itime += 86400;
			localtime_r(&itime,&T);
			sprintf(str,"%s%02d_%02d_%01d.rm3",T.tm_mday,T.tm_mon+1,T.tm_year-100);
		break;
		default:
			RETURNERR("Parser: Error tfs file type!\n");
	}
	if(sOutRm3FileName) free(sOutRm3FileName);
	sOutRm3FileName = (char*)malloc(strlen(str)+1);
	if (!sOutRm3FileName) RETNOMEM;
	strcpy(sOutRm3FileName, str);
	return tfsFileType;
}

int CParser::ParseCStringParams (int argc, char *argv[])
{
	int i;
	for( i=1; i<argc; i++)
	{
		if(!strcmp(argv[i],"-outdir"))
		{
			i++;
			if (i < argc)
			{
				sOutDir = (char*)malloc(strlen(argv[i])+1);
				if (!sOutDir) RETNOMEM;
				strcpy(sOutDir, argv[i]);
			}
		}
        else if(!strcmp(argv[i],"-spiderip"))
        {
            i++;
            if (i < argc)
            {
                if(CheckIp(argv[i]))
                {
                    sSpiderIp = (char*)malloc(strlen(argv[i])+1);
                    if (!sSpiderIp) RETNOMEM;
                    strcpy(sSpiderIp, argv[i]);
                }
                else
                    RETURNERR("Parser: Incorrect spider ip address!\n");
            }
        }
        else if(!strcmp(argv[i],"-spiderport"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    SpiderPort = atoi(argv[i]);
                }
                else
                    RETURNERR("Parser: Incorrect spider port value!\n");
            }
        }
        else if(!strcmp(argv[i],"-serverport"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    ServerPort = atoi(argv[i]);
                }
                else
                    RETURNERR("Parser: Incorrect server port value!\n");
            }
        }
        else if(!strcmp(argv[i],"-rotation"))
        {
            i++;
            if (i < argc)
            {
                unsigned int rt = (unsigned int)atoi(argv[i]);
                if (check_d(argv[i]) && rt <= ROTATION_ONLINE)
                {
                    rotation =  (unsigned char)rt;
                }
                else
                    RETURNERR("Parser: Incorrect rotation value!\n");
            }
        }
		else if(!strcmp(argv[i],"-nojournal"))
		{
			pSett.bEnable = false;
		}
		else if(!strcmp(argv[i],"-maxduration"))
		{
			i++;
			if (i < argc)
			{
				if (check_d(argv[i]))
				{
					pSett.iMaxDuration = atoi(argv[i]);
				}
				else
					RETURNERR("Parser: Incorrect duration value!\n");
			}
		}
		else if(!strcmp(argv[i],"-cdrformat"))
		{
			i++;
			if (i < argc)
			{
				int fr = atoi(argv[i]);
				if ( fr > 0 && fr < 19)
				{
					pSSettings.btCDRStringFormat = fr;
				}
				else
					RETURNERR("Parser: Incorrect CDR format!\n");
			}
		}
		else if(!strcmp(argv[i],"-nonumstr"))
		{
			i++;
			if (i < argc)
			{
				strcpy(pSSettings.sNoNumberString,argv[i]);
			}
		}
		else if(!strcmp(argv[i],"/0"))
		{
			pSSettings.bWriteUnansweredCalls = false;
		}
		else if(!strcmp(argv[i],"-str2boff"))
		{
			pSSettings.bWriteBinary2 = false;
		}
		else if(!strcmp(argv[i],"-convert"))
		{
			fConvert = true;
		}
		else if(!strcmp(argv[i],"-remrm3"))
		{
			fRem_rm3 = true;
		}
        else if(!strcmp(argv[i],"-d"))
        {
            fDaemon = true;
        }
        if(!strcmp(argv[i],"-logfile"))
        {
            i++;
            if (i < argc)
            {
                sLogFile = (char*)malloc(strlen(argv[i])+1);
                if (!sLogFile) RETNOMEM;
                strcpy(sLogFile, argv[i]);
            }
        }
		else if(!strcmp(argv[i],"-tfsfile"))
		{
			if(!sInDir)
			{
				i++;
				if (i< argc)
				{
					//Getting sInDir
					char *ext;
					if(!(ext = strrchr( argv[i], (int)'/')))
					{
						int j;
						char *pwd = NULL;
						for(j = 0; environ[j], !pwd; j++)
						{
							if( pwd = strstr(environ[j],"PWD="))
							{
								pwd += strlen("PWD="); 
								sInDir = (char*)malloc(strlen(pwd)+1);
								if (!sInDir) RETNOMEM;
								strcpy(sInDir,pwd);
	
								char* CharList = (char *)malloc(strlen(argv[i])+1);	
								if(!CharList) RETNOMEM; 
								strcpy(CharList, argv[i]);
								TfsFileList.push_back(CharList);
							}
						}
						if(!pwd)
							RETURNERR("Parser: enter full path of tfs file!\n");
					}
					else
					{
						int len = strlen(argv[i]) - strlen(strrchr( argv[i], (int)'/'));
						sInDir = (char*)malloc(len + 1);
						if (!sInDir) RETNOMEM;
						sInDir[len] = 0;
						memcpy(sInDir, argv[i], len);
						
						char* CharList = (char *)malloc(strlen(strrchr( argv[i], (int)'/')+1)+1);	
						if(!CharList) RETNOMEM; 
						strcpy(CharList, strrchr( argv[i], (int)'/')+1); 
						TfsFileList.push_back(CharList);
					}
				}
			}
		}
		else if(!strcmp(argv[i],"-tfsdir"))
		{
			if(!sInDir)
			{
				i++;
				if (i < argc)
				{
					sInDir = (char*)malloc(strlen(argv[i])+1);
					if (!sInDir) RETNOMEM;
					strcpy(sInDir, argv[i]);
					
					int pid, status, filedes[2];
					if(!(Pipe(filedes) && make_nonblock(filedes[0]) && make_nonblock(filedes[1])))
						RETURNERR("Parser: Error making pipe channel!\n");
					pid = fork();
					switch(pid)
					{
						case -1:
						RETURNERR("Parser: Fork <ls> error!\n");
						break;
						case 0:
						{
							dup2(filedes[1],1);
							chdir(sInDir);
							if(execl("/bin/ls","/bin/ls","-1",NULL)<0)
								printf("Parser:<ls> exec error!\n");
							exit(EXIT_FAILURE);
						}
						break;
						default: {status =-1; wait(&status);}
					}
					fd_set rset;
					FD_ZERO(&rset); // Set Zero
					FD_SET(filedes[0], &rset);
					struct timeval tv;
					tv.tv_sec = 1;
					tv.tv_usec = 0;
					char str[1024],*ch;
					if ((select( filedes[0] + 1, &rset, NULL, NULL, &tv)) > 0)
					{
						ch = str;
						while(read(filedes[0], ch++, 1) == 1)
						{
							if(*(ch-1) == '\n' || *(ch-1) == '\r')
							{
								*(ch-1) = 0;
								if(strstr(str,".tfs"))
								{
									char* CharList = (char *)malloc(strlen(str)+1);	
									if(!CharList) RETNOMEM; 
									strcpy(CharList, str); 
									TfsFileList.push_back(CharList);
								}
								ch = str;
							}
						}
					}
				}
			}
		}
	}
    if(!sSpiderIp)
    {
	   if(!sInDir)
		  RETURNERR("Parser: No imput file!\n");
	   if(!sOutDir)
	   {
		  sOutDir = (char*)malloc(strlen(sInDir)+1);
		  if (!sOutDir) RETNOMEM;
		  strcpy(sOutDir,sInDir);
	   }
    }
    else
    {
        if(!sOutDir)
        {
            int j;
            char *pwd = NULL;
            for(j = 0; environ[j], !pwd; j++)
            {
                if( pwd = strstr(environ[j],"PWD="))
                {
                    pwd += strlen("PWD="); 
                    sOutDir = (char*)malloc(strlen(pwd)+1);
                    if (!sOutDir) RETNOMEM;
                    strcpy(sOutDir,pwd);
                }
            }
            if(!pwd)
                RETURNERR("Parser: enter output directory!\n");
        }
    }
    if(!sLogFile)
    {
        sLogFile = (char*)malloc(strlen(sOutDir) + strlen("/callbuilder.log") + 1);
        if (!sLogFile) RETNOMEM;
        strcpy(sLogFile, sOutDir);
        strcat(sLogFile,"/callbuilder.log");
    }
	return 1;
}

int CParser::FillOnLineParams (const char *CurrTfsFileName)
{
    if(sLogFileName) free(sLogFileName);
    if(sErrFileName) free(sErrFileName);
    if(sJrnFileName) free(sJrnFileName);

    sLogFileName = (char*)malloc(strlen(CurrTfsFileName)+strlen(".log")+1);
    if (!sLogFileName) RETNOMEM;
    strcpy(sLogFileName, CurrTfsFileName);
    strcat(sLogFileName, ".log");
    sErrFileName = (char*)malloc(strlen(CurrTfsFileName)+strlen(".err")+1);
    if (!sErrFileName) RETNOMEM;
    strcpy(sErrFileName, CurrTfsFileName);
    strcat(sErrFileName, ".err");
    sJrnFileName = (char*)malloc(strlen(CurrTfsFileName)+strlen(".jrn")+1);
    if (!sJrnFileName) RETNOMEM;
    strcpy(sJrnFileName, CurrTfsFileName);
    strcat(sJrnFileName, ".jrn");
    return 1;
}

int CParser::FillMainParams (const char *CurrTfsFileName)
{
	if(sInRm3FileName) free(sInRm3FileName);
	if(sLogFileName) free(sLogFileName);
	if(sErrFileName) free(sErrFileName);
	if(sJrnFileName) free(sJrnFileName);
	if(sTfsFileNameBase) free(sTfsFileNameBase);
	//Getting sResidInFileName, sLogFileName, sErrFileName, sJrnFileNameCAboutDlg
	char *ext = (char*)malloc(strlen(CurrTfsFileName)+1);
	if (!ext) RETNOMEM;
	strcpy(ext,CurrTfsFileName);
	char *str = strrchr( ext, (int)'.');
	if(str)
	{
		if( !strcmp(str,".tfs") || !strcmp(str,".TFS") )
		{
			memset(str,0,strlen(str)); //name_01_02_2004 | name_mon02_2004 | name_dec02_2004 | 12_10_6

			sInRm3FileName = (char*)malloc(strlen(CurrTfsFileName)+1);
			if (!sInRm3FileName) RETNOMEM;
			strcpy(sInRm3FileName,ext);
			strcat(sInRm3FileName,".rm3");
			sLogFileName = (char*)malloc(strlen(CurrTfsFileName)+1);
			if (!sLogFileName) RETNOMEM;
			strcpy(sLogFileName,ext);
			strcat(sLogFileName,".log");
			sErrFileName = (char*)malloc(strlen(CurrTfsFileName)+1);
			if (!sErrFileName) RETNOMEM;
			strcpy(sErrFileName,ext);
			strcat(sErrFileName,".err");
			sJrnFileName = (char*)malloc(strlen(CurrTfsFileName)+1);
			if (!sJrnFileName) RETNOMEM;
			strcpy(sJrnFileName,ext);
			strcat(sJrnFileName,".jrn");
		}
		else RETURNERR("Parser: Main file not tfs!\n");
	}
	else RETURNERR("Parser: Main file have no extention!\n");
	
	//Getting sMainFileName and tfstype 
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
		tfsFileType = ROTATION_MONTH;
		memset(str,0,strlen(str)); //name
		sTfsFileNameBase = (char*)malloc(strlen(ext)+1);
		if (!sTfsFileNameBase) RETNOMEM;
		strcpy(sTfsFileNameBase, ext);
	}
	else if(strstr(str+1,"dec") && (tmptm = atoi(str+4)))
	{
		Tm.tm_yday = tmptm;
		tfsFileType = ROTATION_DECADE;
		memset(str,0,strlen(str)); //name
		sTfsFileNameBase = (char*)malloc(strlen(ext)+1);
		if (!sTfsFileNameBase) RETNOMEM;
		strcpy(sTfsFileNameBase, ext);
	}
	else if(tmptm =  atoi(str+1))
	{
		Tm.tm_mon = --tmptm;
		memset(str,0,strlen(str)); //name_01 | 12
		str = strrchr( ext, (int)'_');
		if (str && (tmptm = atoi(str+1)))
		{
			Tm.tm_mday = tmptm;
			tfsFileType = ROTATION_DAY;
			memset(str,0,strlen(str)); //name
			sTfsFileNameBase = (char*)malloc(strlen(ext)+1);
			if (!sTfsFileNameBase) RETNOMEM;
			strcpy(sTfsFileNameBase, ext);
		}
		else if (tmptm = atoi(ext))
		{ 
			Tm.tm_mday = tmptm;
			tfsFileType = ROTATION_FROM_ATS;
		}
		else RETURNW; 
	}
    else RETURNW;
	free(ext);
	return 1;
}
