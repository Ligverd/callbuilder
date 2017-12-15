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

void Loger(const char* str);

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
    nLogFileSize = 20;
    tfsFileType = 0;
    rotation = 0;
    memset(&Tm, 0, sizeof(tm));

    sSpiderIp = NULL;
    SpiderPort = 10002;
    ServerPort = 0;

    Sett.bEnable = true;
    Sett.iMaxDuration = 20000;

    SSettings.bWriteBinary2 = true;
    SSettings.bWriteUnansweredCalls = true;
    strcpy(SSettings.sNoNumberString, "-" );
    SSettings.btCDRStringFormat = FORMAT_CDR_EX;
    SSettings.bDecrementDuration = false;
    SSettings.bDeleteFirstDigitFromAON = false;
    SSettings.sSampleString[0] = 0;

    lstLocalNumbers.clear();
    SPrefix.bCutPrefix = false;
    SPrefix.sPrefix[0] = 0;

    fConvert = false;
    fRem_rm3 = false;
    fDaemon = false;
    fSS = false;
    fNoTarif = false;

    rh = NULL;
    sRadiusAccIp[0] = 0;
    sRadiusAccSecret[0] = 0;
    nRadiusAccPort = 1813;
    nRadiusAccBilling = 0;

    sRadiusAuthIp[0] = 0;
    sRadiusAuthSecret[0] = 0;
    nRadiusAuthPort = 1812;
    nRadiusAuthBilling = 0;
    sScommIp[0] = 0;
    nScommPort = 0;
    strcpy(sScommPassword, "100100");
    fRadAuthPrepay = false;


    sRadiusConfigFile[0] = 0;
    sRadiusDictionaryFile[0] = 0;
    sRadiusSeqFile[0] = 0;

    nRadiusSndRetry = 3;
    nRadiusSndTimeout = 5;
    fRadTrace = false;
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
    lstLocalNumbers.clear();
    if(rh)
        rc_destroy(rh);
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

int CParser::CheckIp(const char *ipstr)
{

    if (!strcmp(ipstr, "localhost"))
        return 1;

    int len = strlen(ipstr);

    if (len > 6 && len < 16)
    {
        char i = 0, p = 0, dot = 0, s[100];
        memset(s, 0, len+1);
        while(i <= len)
        {
            if(i==len)
            {
                if(dot != 3) return 0;
                if((atoi(s) < 0) || (atoi(s) > 255)) return 0;
                break;
            }
            if (ipstr[i] == '.')
            {
                dot++;
                if((atoi(s) <0 ) || (atoi(s) > 255) || p == 0) return 0;
                memset(s, 0, p);
                i++;
                p = 0;
            }
            if (ipstr[i]> '9' || ipstr[i]< '0')return 0;
            s[p++] = ipstr[i++];
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
    memcpy(&T, &Tm, sizeof(tm));
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
            sprintf(str, "%s_%02d_%02d_%04d.rm3", sTfsFileNameBase, T.tm_mday, T.tm_mon + 1, T.tm_year + 1900);
        break;
        case ROTATION_MONTH://mon
            if(!sTfsFileNameBase) RETURNW;
            T.tm_mon++;
            if(T.tm_mon > 11)
            {
                T.tm_mon = 0;
                T.tm_year++;
            }
            sprintf(str, "%s_mon_%02d_%04d.rm3", sTfsFileNameBase, T.tm_mon + 1, T.tm_year);
        break;
        case ROTATION_DECADE://dec
            if(!sTfsFileNameBase) RETURNW;
            T.tm_yday++;
            if(T.tm_yday > 37)
            {
                T.tm_yday = 1;
                T.tm_year++;
            }
            sprintf(str, "%s_dec_%02d_%04d.rm3", sTfsFileNameBase, T.tm_yday, T.tm_year);
        break;
        case ROTATION_FROM_ATS://from ats 
            T.tm_year += 100;
            itime = mktime (&T);
            itime += 86400;
            localtime_r(&itime, &T);
            sprintf(str, "%02d_%02d_%01d.rm3", T.tm_mday, T.tm_mon + 1, T.tm_year - 100);
        break;
        case ROTATION_FROM_SS://from ss 
            T.tm_year += 100;
            itime = mktime (&T);
            itime += 86400;
            localtime_r(&itime, &T);
            sprintf(str, "%02d_%02d_%02d.rm3", T.tm_year - 100, T.tm_mon+1, T.tm_mday);
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

int CParser::ParseConigFile(const char *filename, CDRBuilding::TPCharList &CfgList)
{
    FILE *cfg;

    if(!filename)
        return -1;

    if ((cfg = fopen (filename, "r")) == NULL)
        RETURNERR("Parser: can't open config file");

    char buffer[512], *cp;
    char sep[] = " \t\n\r";
    char *phrase, *brkb;

    while (fgets (buffer, sizeof (buffer), cfg) != NULL)
    {
        if (*buffer == '#' || *buffer == '\0' || *buffer == '\n' || *buffer == '\r')
            continue;

        cp = strchr(buffer, '#');
        if (cp != NULL)
            *cp = '\0';

        for (phrase = strtok_r(buffer, sep, &brkb); phrase;
                phrase = strtok_r(NULL, sep, &brkb))
        {
            char *str = (char *)malloc(strlen(phrase) + 1);
            if(!phrase)
            {
                FreeListMem(CfgList);
                fclose(cfg);
                RETNOMEM;
            }
            strcpy(str, phrase);
            CfgList.push_back(str);
        }
    }
    fclose(cfg);
    return 0;
}

// ----CStringParams----
//1//-outdir
//2//-spiderip
//3//-spiderport
//4//-serverport
//5//-rotation
//6//-nojournal
//7//-maxduration
//8//-cdrformat
//9//-nonumstr
//10// /0
//11// /q
//12// /1
//13//-str2boff
//14//-convert
//15//-remrm3
//16//-d
//17//-logfile
//18//-a164
//19//-b164
//20//-prefix
//21//-in
//22//-tfsfile
//23//-tfsdir
//24//-filename
//25//-radaccip
//26//-radaccport
//27//-radaccsecret
//28//-radaccbill
//29//-radauthip
//30//-radauthport
//31//-radauthsecret
//32//-radauthbill
//33//-scommip
//34//-scommport
//35//-scommpassw
//36//-radconf
//37//-raddict
//38//-radseq
//39//-radretry
//40//-radtimeout
//41//-h
//42//-ss
//43//-f
//44//-logsize

int CParser::ParseCStringParams (int argc, char *argv[], bool fFromFile)
{
    int i;
    for( i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-outdir"))
        {
            i++;
            if (i < argc)
            {
                if(sOutDir)
                    continue;

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
                if(sSpiderIp)
                    continue;

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
            Sett.bEnable = false;
        }
        else if(!strcmp(argv[i],"-maxduration"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    Sett.iMaxDuration = atoi(argv[i]);
                }
                else
                    RETURNERR("Parser: Incorrect duration value!\n");
            }
        }
        else if(!strcmp(argv[i],"-logsize"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    unsigned int sz = atoi(argv[i]);
                    if(sz && sz <= 100)
                        nLogFileSize = sz;
                }
                else
                    RETURNERR("Parser: Incorrect log file size value!\n");
            }
        }
        else if(!strcmp(argv[i],"-cdrformat"))
        {
            i++;
            if (i < argc)
            {
                int fr = atoi(argv[i]);
                if ( fr > 0 && fr < 21)
                {
                    SSettings.btCDRStringFormat = fr;
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
                strcpy(SSettings.sNoNumberString,argv[i]);
            }
        }
        else if(!strcmp(argv[i],"/0"))
        {
            SSettings.bWriteUnansweredCalls = false;
        }
        else if(!strcmp(argv[i],"/q"))
        {
            SSettings.bDecrementDuration = true;
        }
        else if(!strcmp(argv[i],"/1"))
        {
            SSettings.bDeleteFirstDigitFromAON = true;
        }
        else if(!strcmp(argv[i],"-ss"))
        {
            fSS = true;
        }
        else if(!strcmp(argv[i],"-notarif"))
        {
            fNoTarif = true;
        }
        else if(!strcmp(argv[i],"-radtrace"))
        {
            fRadTrace = true;
        }
        else if(!strcmp(argv[i],"-str2boff"))
        {
            SSettings.bWriteBinary2 = false;
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
        else if(!strcmp(argv[i],"-logfile"))
        {
            i++;
            if (i < argc)
            {
                if(sLogFile)
                    continue;

                sLogFile = (char*)malloc(strlen(argv[i])+1);
                if (!sLogFile) RETNOMEM;
                strcpy(sLogFile, argv[i]);
            }
        }
        else if(!strcmp(argv[i],"-a164"))
        {
            i++;
            if (i < argc)
            {
                SSettings.sA164 = argv[i];
            }
        }
        else if(!strcmp(argv[i],"-b164"))
        {
            i++;
            if (i < argc)
            {
                SSettings.sB164 = argv[i];
            }
        }
        else if(!strcmp(argv[i],"-prefix"))
        {
            i++;
            if (i < argc)
            {
                if( strlen(argv[i]) > MAX_PREFIX_LENGTH || !check_d(argv[i]) )
                    continue;

                strcpy(SPrefix.sPrefix, argv[i]);
                SPrefix.bCutPrefix = true;
            }
        }
        else if(!strcmp(argv[i],"-in"))
        {
            i++;
            if (i < argc)
            {
                char str[2*MAX_INTERVAL_LENGTH + 1 + 1];

                if( strlen(argv[i]) > 2*MAX_INTERVAL_LENGTH + 1)
                    continue;
                strcpy(str,argv[i]);

                const char *sep = "-";
                char *pToken = NULL, *ptrptr = NULL;
                pToken = strtok_r(str, sep, &ptrptr);

                //checks
                if(!pToken || !ptrptr)
                    continue;

                if(strlen(pToken) != strlen(ptrptr))
                    continue;

                if( !check_d(pToken) || !check_d(ptrptr) )
                    continue;

                if( strcmp(pToken , ptrptr) > 0 )
                    continue;

                CDRBuilding::strInterval SI;
                strcpy(SI.beg, pToken);
                strcpy(SI.end, ptrptr);
                lstLocalNumbers.push_back(SI);
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
                #ifndef PTH_WIN
                    const char chSlash = '/';
                #else
                    const char chSlash = '\\';
                #endif
                    char *ext;
                    if(!(ext = strrchr( argv[i], (int)chSlash)))
                    {
                #ifndef CYGWIN
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
                    #else
                        RETURNERR("Parser: enter full path of tfs file!\n");
                    #endif
                    }
                    else
                    {
                        int len = strlen(argv[i]) - strlen(strrchr( argv[i], (int)chSlash));
                        sInDir = (char*)malloc(len + 1);
                        if (!sInDir) RETNOMEM;
                        sInDir[len] = 0;
                        memcpy(sInDir, argv[i], len);

                        char* CharList = (char *)malloc(strlen(strrchr( argv[i], (int)chSlash)+1)+1);	
                        if(!CharList) RETNOMEM; 
                        strcpy(CharList, strrchr( argv[i], (int)chSlash)+1); 
                        TfsFileList.push_back(CharList);
                    }
                }
            }
        }
        else if(!strcmp(argv[i],"-tfsdir"))
        {
#if !defined(CYGWIN)
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
                            if(execl("/bin/ls","/bin/ls", "-1", NULL) < 0 )
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
                    char str[1024], *ch;
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
#endif
        }
        else if(!strcmp(argv[i],"-filename"))
        {
            i++;
            if (i < argc)
            {
                if(sTfsFileNameBase)
                    continue;

                sTfsFileNameBase = (char*)malloc(strlen(argv[i])+1);
                if (!sTfsFileNameBase) RETNOMEM;
                strcpy(sTfsFileNameBase, argv[i]);
            }
        }

        //radius
        else if(!strcmp(argv[i],"-radaccip"))
        {
            i++;
            if (i < argc)
            {
                if(CheckIp(argv[i]))
                    strncpy(sRadiusAccIp, argv[i], sizeof(sRadiusAccIp));
                else
                    RETURNERR("Parser: error radius acc ip\n");
            }
        }
        else if(!strcmp(argv[i],"-radaccport"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                    nRadiusAccPort = atoi(argv[i]);
                else
                    RETURNERR("Parser: error radius acc port\n");
            }
        }
        else if(!strcmp(argv[i],"-radaccsecret"))
        {
            i++;
            if (i < argc)
                strncpy(sRadiusAccSecret, argv[i], sizeof(sRadiusAccSecret));
        }
        else if(!strcmp(argv[i],"-radaccbill"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    nRadiusAccBilling = atoi(argv[i]);
                    if(nRadiusAccBilling < 0 || nRadiusAccBilling > 1)
                        RETURNERR("Parser: unsupported radius acc billing type\n");
                }
                else
                    RETURNERR("Parser: error radius acc billing\n");
            }
        }

        else if(!strcmp(argv[i],"-radauthip"))
        {
            i++;
            if (i < argc)
            {
                if(CheckIp(argv[i]))
                    strncpy(sRadiusAuthIp, argv[i], sizeof(sRadiusAuthIp));
                else
                    RETURNERR("Parser: error radius auth ip\n");
            }
        }
        else if(!strcmp(argv[i],"-radauthport"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                    nRadiusAuthPort = atoi(argv[i]);
                else
                    RETURNERR("Parser: error radius auth port\n");
            }
        }
        else if(!strcmp(argv[i],"-radauthsecret"))
        {
            i++;
            if (i < argc)
                strncpy(sRadiusAuthSecret, argv[i], sizeof(sRadiusAuthSecret));
        }
        else if(!strcmp(argv[i],"-radauthbill"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                {
                    nRadiusAuthBilling = atoi(argv[i]);
                    if(nRadiusAuthBilling < 0 || nRadiusAuthBilling > 1)
                        RETURNERR("Parser: unsupported radius auth billing type\n");
                }
                else
                    RETURNERR("Parser: error radius auth billing\n");
            }
        }
        else if(!strcmp(argv[i],"-scommip"))
        {
            i++;
            if (i < argc)
            {
                if(CheckIp(argv[i]))
                    strncpy(sScommIp, argv[i], sizeof(sScommIp));
                else
                    RETURNERR("Parser: error scomm ip\n");
            }
        }
        else if(!strcmp(argv[i],"-scommport"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                    nScommPort = atoi(argv[i]);
                else
                    RETURNERR("Parser: error scomm port\n");
            }
        }
        else if(!strcmp(argv[i],"-scommpassw"))
        {
            i++;
            if (i < argc)
                strncpy(sScommPassword, argv[i], sizeof(sScommPassword));
        }

        else if(!strcmp(argv[i],"-radconf"))
        {
            i++;
            if (i < argc)
                strncpy(sRadiusConfigFile, argv[i], sizeof(sRadiusConfigFile));
        }
        else if(!strcmp(argv[i],"-raddict"))
        {
            i++;
            if (i < argc)
                strncpy(sRadiusDictionaryFile, argv[i], sizeof(sRadiusDictionaryFile));
        }
        else if(!strcmp(argv[i],"-radseq"))
        {
            i++;
            if (i < argc)
                strncpy(sRadiusSeqFile, argv[i], sizeof(sRadiusSeqFile));
        }
        else if(!strcmp(argv[i],"-radretry"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                    nRadiusSndRetry = atoi(argv[i]);
                else
                    RETURNERR("Parser: error radius retry\n");
            }
        }
        else if(!strcmp(argv[i],"-radtimeout"))
        {
            i++;
            if (i < argc)
            {
                if (check_d(argv[i]))
                    nRadiusSndTimeout = atoi(argv[i]);
                else
                    RETURNERR("Parser: error radius retry\n");
            }
        }
        else if(!strcmp(argv[i], "-f"))
        {
            i++;
            if (i < argc)
            {
                if(fFromFile)
                    continue;

                CDRBuilding::TPCharList CfgList;
                if(ParseConigFile(argv[i], CfgList) < 0)
                    continue;

                int size = CfgList.size() + 1;
                char **argv_file;
                argv_file = (char **)malloc(sizeof(char*)*size);
                if(!argv_file)
                {
                    FreeListMem(CfgList);
                    RETNOMEM;
                }

                CDRBuilding::TPCharList::const_iterator it;
                int j;
                for (it = CfgList.begin(), j = 1; it != CfgList.end(); it++, j++)
                    argv_file[j] = (char *)*it;

                if(ParseCStringParams (size, argv_file, true) < 0)
                {
                    free(argv_file);
                    FreeListMem(CfgList);
                    RETURNERR("Parser: error while parsing config file\n");
                }

                free(argv_file);
                FreeListMem(CfgList);
            }
        }
        else if(!strcmp(argv[i],"-radauthprepay"))
        {
#ifndef     ARMLINUX
            fRadAuthPrepay = true;
#endif
        }
        else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") ||
                !strcmp(argv[i], "-help"))
        {
            printf("callbuilder v%s (set UTF8 to view help)\n", VERSION);
            PrintHelp();
            return -1;
        }
    }
    return 0;
}

int CParser::Prepare( void )
{
    if(fNoTarif)
    {
        if(!sOutDir)
        {
            sLogFile = (char*)malloc(strlen("callbuilder.log") + 1);
            if (!sLogFile) RETNOMEM;
            strcpy(sLogFile,"callbuilder.log");
        }
        else
        {
            sLogFile = (char*)malloc(strlen(sOutDir) + strlen("/callbuilder.log") + 1);
            if (!sLogFile) RETNOMEM;
            strcpy(sLogFile, sOutDir);
            strcat(sLogFile,"/callbuilder.log");
        }
    }
    else
    {
        if(!sSpiderIp)
        {
            if(!sInDir)
                RETURNERR("Parser: No input file or spider ip addr\n");
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
            #ifndef PTH_WIN
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
            #endif
                if(!pwd)
                    RETURNERR("Parser: enter output directory!\n");
            }
            if(!sTfsFileNameBase)
            {
                sTfsFileNameBase = (char*)malloc(strlen("cdr_log") + 1);
                if (!sTfsFileNameBase) RETNOMEM;
                strcpy(sTfsFileNameBase, "cdr_log");
            }
            if (!ServerPort)
                ServerPort = SpiderPort + 1;
        }
        if(!sLogFile)
        {
            sLogFile = (char*)malloc(strlen(sOutDir) + strlen("/callbuilder.log") + 1);
            if (!sLogFile) RETNOMEM;
            strcpy(sLogFile, sOutDir);
            strcat(sLogFile,"/callbuilder.log");
        }
    }
    if(sRadiusAccIp[0] || sRadiusAuthIp[0] || sRadiusConfigFile[0])
    {
        if(sRadiusConfigFile[0])
        {
            if ((rh = rc_read_config(sRadiusConfigFile)) == NULL)
                RETURNERR("Parser: radius init error\n");
        }
        else
        {

            if(sRadiusAccIp[0])
                if(!sRadiusAccSecret[0])
                    RETURNERR("Parser: no radius acc secret\n");

            if(sRadiusAuthIp[0])
                if(!sRadiusAuthSecret[0])
                    RETURNERR("Parser: no radius auth secret\n");

            /* Initialize the 'rh' structure */
            if ((rh = rc_new()) == NULL)
                RETURNERR("Parser: Failed to allocate radius initial structure\n");

            /* Initialize the config structure */
            if ((rh = rc_config_init(rh)) == NULL)
                RETURNERR("Parser: Failed to initialze radius configuration\n");

            if (rc_add_config(rh, "auth_order", "radius", "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius auth_order\n");

            if (rc_add_config(rh, "login_tries", "4", "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius login_tries\n");

            if(!sRadiusDictionaryFile[0])
                strcpy(sRadiusDictionaryFile, "dictionary");

            // Тут собственно подключаем словарь, он все равно из внешнего файла
            if (rc_add_config(rh, "dictionary", sRadiusDictionaryFile, "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius dictionary\n");

            if(!sRadiusSeqFile[0])
                strcpy(sRadiusSeqFile, "/var/run/radius.seq");

            if (rc_add_config(rh, "seqfile", sRadiusSeqFile, "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius seq file\n");

            char retry[10];
            sprintf(retry, "%u", nRadiusSndRetry );
            if (rc_add_config(rh, "radius_retries", retry, "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius retries\n");

            char timeout[10];
            sprintf(timeout, "%u", nRadiusSndTimeout );
            if (rc_add_config(rh, "radius_timeout", timeout, "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius timeout\n");

            if(sRadiusAccIp[0])
            {
                char sps[200];
                sprintf(sps, "%s:%u:%s", sRadiusAccIp, nRadiusAccPort, sRadiusAccSecret);

                // Сервер аккаунтинга после : порт : потом секрет
                if (rc_add_config(rh, "acctserver", sps, "config", 0) != 0)
                    RETURNERR("Parser: Unable to set radius acctserver\n");
            }

            if(sRadiusAuthIp[0])
            {
                char sps[200];
                sprintf(sps, "%s:%u:%s", sRadiusAuthIp, nRadiusAuthPort, sRadiusAuthSecret);

                // Сервер аккаунтинга после : порт : потом секрет
                if (rc_add_config(rh, "authserver", sps, "config", 0) != 0)
                    RETURNERR("Parser: Unable to set radius authserver\n");
            }

            // Вот из-за этой переменной все трапалось, не было проверки на выделение памяти, исправил
            if (rc_add_config(rh, "radius_deadtime", "5", "config", 0) != 0)
                RETURNERR("Parser: Unable to set radius deadtime\n");
        }

        /* Read in the dictionary file(s) */
        if (rc_read_dictionary(rh, rc_conf_str(rh, (char *)"dictionary")) != 0)
            RETURNERR("Parser: Failed to initialize radius dictionary\n");
    }
    return 0;
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
    return 0;
}

int CParser::FillMainParams (const char *CurrTfsFileName)
{
    if(sInRm3FileName) free(sInRm3FileName);
    if(sLogFileName) free(sLogFileName);
    if(sErrFileName) free(sErrFileName);
    if(sJrnFileName) free(sJrnFileName);
    if(sTfsFileNameBase) free(sTfsFileNameBase);

    //Getting sResidInFileName, sLogFileName, sErrFileName, sJrnFileNameCAboutDlg
    char ext[256];
    strcpy(ext, CurrTfsFileName);

    char *str = strrchr( ext, (int)'.');
    if(str)
    {
        if( !strcmp(str,".tfs") || !strcmp(str,".TFS") )
        {
            memset(str, 0, strlen(str)); //name_01_02_2004 | name_mon02_2004 | name_dec02_2004 | 12_10_6

            sInRm3FileName = (char*)malloc(strlen(CurrTfsFileName) + 1);
            if (!sInRm3FileName)
                RETNOMEM;
            strcpy(sInRm3FileName, ext);
            strcat(sInRm3FileName, ".rm3");

            sLogFileName = (char*)malloc(strlen(CurrTfsFileName) + 1);
            if (!sLogFileName)
                RETNOMEM;
            strcpy(sLogFileName, ext);
            strcat(sLogFileName, ".log");

            sErrFileName = (char*)malloc(strlen(CurrTfsFileName) + 1);
            if (!sErrFileName)
                RETNOMEM;
            strcpy(sErrFileName, ext);
            strcat(sErrFileName, ".err");

            sJrnFileName = (char*)malloc(strlen(CurrTfsFileName) + 1);
            if (!sJrnFileName)
                RETNOMEM;
            strcpy(sJrnFileName, ext);
            strcat(sJrnFileName, ".jrn");
        }
        else RETURNERR("Parser: Main file not tfs!\n");
    }
    else RETURNERR("Parser: Main file have no extention!\n");

    //Getting sMainFileName and tfstype 
    //выясняем что за ротация у открываемого файла
    //name_01_02_2004.tfs 1
    //name_mon02_2004.tfs 2
    //name_dec02_2004.tfs 3
    //либо может быть файл с флешки - например 12_10_6.TFS 4
    //либо может быть файл с SS коммутатора - например 10_03_15.TFS 4 (год в переди)

    unsigned int tmptm;
    str = strrchr(ext, (int)'_');
    if (!str)
        RETURNW;

    if(fSS)
    {
        // 10_03_13
        if(!(tmptm = atoi(str + 1))) //_13
            RETURNW;

        Tm.tm_mday = tmptm;
        memset(str, 0, strlen(str));

        //10_03
        str = strrchr( ext, (int)'_');
        if (!str)
            RETURNW;

        if(!(tmptm =  atoi(str + 1))) //_03
            RETURNW;

        Tm.tm_mon = --tmptm;
        memset(str, 0, strlen(str));

        //10
        if(!(tmptm = atoi(ext)))
        {
             if(!( strlen(ext) == 2 && ext[0] == '0' && ext[1] == '0' )) //00
                 RETURNW;
        }

        Tm.tm_year = tmptm;
        tfsFileType = ROTATION_FROM_SS;
        rotation = tfsFileType;
        return 0;
    }

    if (!(tmptm = atoi(str + 1)) && !(strlen(str + 1) == 1 && str[1] == '0')) //? 2004 | 6
        RETURNW;

    Tm.tm_year = tmptm; 
    memset(str, 0, strlen(str)); //name_01_02 | name_mon02 | name_dec02 | 12_10

    str = strrchr( ext, (int)'_'); //_02 | _mon02 | _dec02 | _10
    if (!str)
        RETURNW;

    if(strstr(str + 1, "mon") &&  (tmptm = atoi(str + 4)))
    {
        Tm.tm_mon = --tmptm;
        tfsFileType = ROTATION_MONTH;
        memset(str, 0, strlen(str)); //name
        sTfsFileNameBase = (char*)malloc(strlen(ext) + 1);
        if (!sTfsFileNameBase) RETNOMEM;
        strcpy(sTfsFileNameBase, ext);
    }
    else if(strstr(str + 1, "dec") && (tmptm = atoi(str + 4)))
    {
        Tm.tm_yday = tmptm;
        tfsFileType = ROTATION_DECADE;
        memset(str, 0, strlen(str)); //name
        sTfsFileNameBase = (char*)malloc(strlen(ext) + 1);
        if (!sTfsFileNameBase) RETNOMEM;
        strcpy(sTfsFileNameBase, ext);
    }
    else if(tmptm =  atoi(str + 1))
    {
        Tm.tm_mon = --tmptm;
        memset(str, 0, strlen(str)); //name_01 | 12
        str = strrchr( ext, (int)'_');
        if (str && (tmptm = atoi(str + 1)))
        {
            Tm.tm_mday = tmptm;
            tfsFileType = ROTATION_DAY;
            memset(str, 0, strlen(str)); //name
            sTfsFileNameBase = (char*)malloc(strlen(ext) + 1);
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
    rotation = tfsFileType;
    return 0;
}

extern char help_str[];
void CParser::PrintHelp(void)
{
    puts(help_str);
}