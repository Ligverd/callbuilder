
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

using namespace std;

//global variables
CParser Parser;
CDRBuilding::CCDRBuilder builder;

int Spider_fd = 0;
int fd_tfs = 0;
DWORD iReadBytes = 0;
int connect_point[MAX_CONNECT_POINT];
char sMessage[128];
//

void get_time_str(char* tm_str, tm T)
{
  sprintf(tm_str,"[%02d-%02d-%04d %02d:%02d:%02d]",T.tm_mday,T.tm_mon+1,(T.tm_year+1900),T.tm_hour,T.tm_min,T.tm_sec);
}

bool StrToLog(const char* str)
{
    int fd;
    bool fret = true;
    if ( (fd = open(Parser.sLogFile, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        printf("Can't write to log file:%s\n",Parser.sLogFile);
        return false;
    }
    if(write(fd, str, strlen(str))<0)
        fret = false;
    close(fd);
    return fret;
}

void Loger(const char* str)
{
    char time_str[22];
    char logstr[128];
    time_t itime;
    tm T;
    time (&itime);
    localtime_r(&itime,&T);
    printf("%s",str);
    get_time_str(time_str,T);
    sprintf(logstr,"%s %s",time_str, str);
    StrToLog(logstr);
}



bool make_nonblock(int sock)
{
    int sock_opt;
    if((sock_opt = fcntl(sock, F_GETFL, O_NONBLOCK)) <0) 
    {
        return false;
    }
    if((sock_opt = fcntl(sock, F_SETFL, sock_opt | O_NONBLOCK)) <0)
    {
        return false;
    }
    return true;
}

int Login_ethernet(const char *ip, in_addr_t port)
{
    struct sockaddr_in sock_addr;
    struct hostent *host;

    int fd, sock_opt;

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        Loger("Can't create spider socket!\n");
        return -1;
    }

    sock_opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
    {
        Loger("Can't set spider socket option SO_REUSEADDR!\n");
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);

    if (inet_aton(ip, &sock_addr.sin_addr) < 0)
    {
        close(fd);
        Loger("Spider ip error!\n");
        return -1;
    }

    host = gethostbyname(ip);
    if (!host)
    {
        Loger("Error resolve server!\n");
        return -1;
    }
    memcpy(&sock_addr.sin_addr, host->h_addr_list[0], sizeof(sock_addr.sin_addr));

    int state = 0;
    struct timeval tout;

    tout.tv_sec = 20;
    tout.tv_usec = 0;
    fd_set wset, rset;

    FD_ZERO(&wset);
    FD_SET(fd, &wset);
    rset = wset;
    int n;

    if (connect(fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            close(fd);
            printf("Connection to spider Error!\n");
            return -1;
        }
        state = 1;
        printf("Connection in progress...\n");
    }
    if (state)
    {
        n = select(fd + 1, &rset, &wset, NULL, &tout);
        if (n < 0 /*|| (FD_ISSET(fd, &wset) && FD_ISSET(fd, &rset))*/)
        {
            close(fd);
            printf("Connection to spider error!\n");
            return -1;
        }
        if (!n)
        {
            close(fd);
            printf("Time out of connection to spider\n");
            return -1;
        }
    }
    char log[50];

    sprintf(log, "Successfuly connected to:%s\n", ip);
    Loger(log);

    char hello_str[12];
    read(fd, hello_str, strlen("I_AM_SPIDER\0")+1);
    if(!strcmp(hello_str,"I_AM_SPIDER\0"))
        Loger("Autorization Ok!\n");
    else
    {
        printf("Autorization error!\n");
        return -1;
    }

    return fd;
}

const char* GetConnectStr(int i)
{
    switch(i)
    {
        case 0: return "log";
        case 1: return "err";
        case 2: return "jrn";
        default: return "xxx";
    }
}

int Create_server_point(in_addr_t port, int i)
{
    struct sockaddr_in server_addr;
    int fd,sock_opt;

    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        Loger("Can't create callbuilder server socket!\n");
        return -1;
    }

    if(!make_nonblock(fd)) 
    {
        close(fd); 
        Loger("Can't make callbuilder server socket nonblock!\n");
        return -1;
    }

    sock_opt = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set callbuilder server socket option SO_REUSEADDR!\n");
    }

    sock_opt = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set callbuilder server socket option SO_KEEPALIVE!\n");
    }

#ifdef TCP_OPT
    sock_opt = 60;
    if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set callbuilder server socket option TCP_KEEPIDLE!\n");
    }

    sock_opt = 60;
    if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&sock_opt,sizeof(sock_opt)) <0)
    {
        Loger("Can't set callbuilder server socket option TCP_KEEPINTVL!\n");
    }
#endif
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    char log[64];
    if ( bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0)
    {
        close(fd);
        sprintf(log,"Error callbuilder server with port:%d\n",port);
        Loger(log);
        return -1;
    }

    if (listen(fd,1) < 0)
    {
        close(fd);
        printf("Listen callbuilder server socket error!\n");
        return -1;
    }
    sprintf(log,"callbuilder %s server listening port:%d\n",GetConnectStr(i),ntohs(server_addr.sin_port));
    Loger(log);
    return fd;
}

char ConvertTable(const char c)
{
    switch((unsigned char)c)
    {
        case 0xFE: return 'ю';//Small
        case 0xE0: return 'а';
        case 0xE1: return 'б';
        case 0xF6: return 'ц';
        case 0xE4: return 'д';
        case 0xE5: return 'е';
        case 0xF4: return 'ф';
        case 0xE3: return 'г';
        case 0xF5: return 'х';
        case 0xE8: return 'и';
        case 0xE9: return 'й';
        case 0xEA: return 'к';
        case 0xEB: return 'л';
        case 0xEC: return 'м';
        case 0xED: return 'н';
        case 0xEE: return 'о';
        case 0xEF: return 'п';
        case 0xFF: return 'я';
        case 0xF0: return 'р';
        case 0xF1: return 'с';
        case 0xF2: return 'т';
        case 0xF3: return 'у';
        case 0xE6: return 'ж';
        case 0xE2: return 'в';
        case 0xFC: return 'ь';
        case 0xFB: return 'ы';
        case 0xE7: return 'з';
        case 0xF8: return 'ш';
        case 0xFD: return 'э';
        case 0xF9: return 'щ';
        case 0xF7: return 'ч';
        case 0xFA: return 'ъ';
        case 0xDE: return 'Ю';//Big
        case 0xC0: return 'А';
        case 0xC1: return 'Б';
        case 0xD6: return 'Ц';
        case 0xC4: return 'Д';
        case 0xC5: return 'Е';
        case 0xD4: return 'Ф';
        case 0xC3: return 'Г';
        case 0xD5: return 'Х';
        case 0xC8: return 'И';
        case 0xC9: return 'Й';
        case 0xCA: return 'К';
        case 0xCB: return 'Л';
        case 0xCC: return 'М';
        case 0xCD: return 'Н';
        case 0xCE: return 'О';
        case 0xCF: return 'П';
        case 0xDF: return 'Я';
        case 0xD0: return 'Р';
        case 0xD1: return 'С';
        case 0xD2: return 'Т';
        case 0xD3: return 'У';
        case 0xC6: return 'Ж';
        case 0xC2: return 'В';
        case 0xDC: return 'Ь';
        case 0xDB: return 'Ы';
        case 0xC7: return 'З';
        case 0xD8: return 'Ш';
        case 0xDD: return 'Э';
        case 0xD9: return 'Щ';
        case 0xD7: return 'Ч';
        case 0xDA: return 'Ъ';
        default: return c;
    }
}

void RemoveFiles()
{
    char path[200];
    sprintf(path,"%s/%s",Parser.sOutDir,Parser.sLogFileName);
    remove(path);
    sprintf(path,"%s/%s",Parser.sOutDir,Parser.sErrFileName);
    remove(path);
    sprintf(path,"%s/%s",Parser.sOutDir,Parser.sJrnFileName);
    remove(path);
}


bool WriteStrToFile(const char* path, const char *str)
{
    int fd;
    bool bRes = true;

    if ( (fd = open(path, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        return false;
    }

    if(write(fd, str, strlen(str))<0)
    {
        bRes = false;
    }
    close(fd);
    return bRes;
}

bool WriteStringsToFile(const char* sDir ,const char* sFileName, CDRBuilding::TPCharList& lstStr)
{
    if(lstStr.empty())
    {
        return true;
    }
    bool bRes = true;
    char path[200];
    sprintf(path,"%s/%s",sDir,sFileName);
    int fd;
    if ( (fd = open(path, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        printf("Error: can't write to file:%s!\n",sFileName);
        return false;
    }

    CDRBuilding::TPCharList::iterator it;

    char sTmp[1502];
    for(it = lstStr.begin(); it != lstStr.end(); it++)
    {
        if(strlen(*it) >= 1500)
        {
            printf("Error: string is reater then 1500 characters!\n");
            bRes = false;
            break;
        }
        else
        {
            strcpy(sTmp, *it);
            strcat(sTmp,"\r\n"); //htob faili ne otlichalis ot windovih
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


void WriteStringsOnLine(const char* sDir ,const char* sFileName, CDRBuilding::TPCharList& lstStr, int i)
{
    if(lstStr.empty())
    {
        return;
    }

    char log[234];

    char path[200];

    //char path[strlen(sDir)+strlen("/")+strlen(sFileName)+1];

    sprintf(path,"%s/%s",sDir,sFileName);

    char cnv_path[204]; // dva puti iz za for
    sprintf(cnv_path,"%s.cnv",path);

    char sTmp[1502];
    CDRBuilding::TPCharList::iterator it;
    for(it = lstStr.begin(); it != lstStr.end(); it++)
    {
        if(strlen(*it) >= 1500)
        {
            Loger("Error: string is reater then 1500 characters!\n");
            continue;
        }
        else
        {
            strcpy(sTmp, *it);
            strcat(sTmp,"\r\n"); //htob faili ne otlichalis ot windovih
            if(connect_point[i + MAX_CONNECT_POINT/2])
            {
                struct timeval tv;
                fd_set wset;
                int retval;
                int count = 0;
                do 
                {
                    tv.tv_sec = 1;
                    tv.tv_usec = 0;
                    FD_ZERO(&wset);
                    FD_SET(connect_point[i + MAX_CONNECT_POINT/2], &wset);
                    if ((retval = select(connect_point[i + MAX_CONNECT_POINT/2] + 1, NULL, &wset, NULL, &tv)) > 0) 
                    {
                        int ERROR=-1;
                        socklen_t opt_size = sizeof(ERROR);
                        getsockopt(connect_point[i + MAX_CONNECT_POINT/2],SOL_SOCKET,SO_ERROR,&ERROR,&opt_size);
                        if(ERROR == 0)
                        {
                            //esli v peredi stroki ponadobitsia dlinna
                            //DWORD temp_len = strlen(sTmp);
                            //write(connect_point[i + MAX_CONNECT_POINT/2], &temp_len, sizeof(DWORD));
                            write(connect_point[i + MAX_CONNECT_POINT/2], sTmp, strlen(sTmp));
                        }
                        else
                        {
                            close(connect_point[i + MAX_CONNECT_POINT/2]);
                            connect_point[i + MAX_CONNECT_POINT/2] = 0;
                            Loger("Client socket error!\n");
                            break;
                        }
                    }
                    else if(retval < 0 && errno != EINTR)
                    {
                        close(connect_point[i + MAX_CONNECT_POINT/2]);
                        connect_point[i + MAX_CONNECT_POINT/2] = 0;
                        Loger("Client socket internal error!\n");
                        break;
                    }
                    count++;
                } while (retval < 0 && count < 2);
            }

            if(Parser.rotation != ROTATION_ONLINE)
            {

                if (!WriteStrToFile(path, sTmp))
                {
                    sprintf(log,"Error: can't write to file:%s!\n",path);
                    Loger(log);
                }
                if(i) //Esli ne log
                {
                    for(int j = 0; j < strlen(sTmp); j++)
                        sTmp[j] = ConvertTable(sTmp[j]);
                    if(Parser.fConvert)
                    {
                        if (!WriteStrToFile(cnv_path, sTmp))
                        {
                            sprintf(log,"Error: can't write to file:%s!\n",cnv_path);
                            Loger(log);
                        }
                    }
                }
            }
            printf("%s",sTmp);
        }
    }
    return;
}

int SaveResiduaryCalls(const char* sDir, const char* sFileName)
{
    int fd,n;
    char path[200];
    sprintf(path,"%s/%s", sDir, sFileName);
    if ( (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC , 0666)) < 0)
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
        delete [] *it;

        if(n != sizeof(CDRBuilding::strCallInfo))
            RETURNERR("Error: error while writing to residuary file!\n");
    }
    close(fd);
    return 1;
}

int PutResiduaryCalls(const char* sDir, const char* sFileName)
{
    //загружаем звонки
    int fd;
    char path[200];
    sprintf(path,"%s/%s", sDir, sFileName);
    if ( (fd = open(path, O_RDONLY)) < 0)
        RETURNERR("Warning: No input residuary file!\n");

    CDRBuilding::strModuleInfo ModInfo[MAX_MOD];
    CDRBuilding::TListPSCallInfo lstCalls;

    if(read(fd, ModInfo, sizeof(CDRBuilding::strModuleInfo)*MAX_MOD) == sizeof(CDRBuilding::strModuleInfo)*MAX_MOD)
    {
        CDRBuilding::strCallInfo* pSCI;
        pSCI = new CDRBuilding::strCallInfo;
        if(!pSCI) RETNOMEM;

        while(read(fd, pSCI, sizeof(CDRBuilding::strCallInfo)) == sizeof(CDRBuilding::strCallInfo))
        {
            lstCalls.push_back(pSCI);
            pSCI = new CDRBuilding::strCallInfo;
            if(!pSCI) RETNOMEM;
        }
        delete pSCI;
        close(fd);

        // perenosim zvonki v builder
        builder.PutResiduaryCalls(ModInfo, lstCalls);

        //удаляем память
        CDRBuilding::TListPSCallInfo::iterator it;
        for(it = lstCalls.begin(); it != lstCalls.end(); it++)
            delete [] *it;
    }
    else 
        RETURNERR("Error: error loading residuary file!\n");
    close(fd);
    return 1;
}

int Converter(const char* sDir ,const char* sFileName)
{
    int fd_in,fd_out;

    char path[204],c;
    sprintf(path,"%s/%s",sDir,sFileName);
    if ( (fd_in = open(path, O_RDONLY)) < 0)
    {
        printf("Converter: can't open file:%s\n",path);
        return -1;
    }

    strcat(path,".cnv");
    if ( (fd_out = open(path, O_WRONLY | O_CREAT | O_TRUNC , 0666)) < 0)
    {
        printf("Converter: can't create file:%s\n",path);
        return -1;
    }

    int Tmp;
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

void ComleteActions()
{
        printf("%s",sMessage);
        if (Parser.fConvert)
        {
            Converter(Parser.sOutDir, Parser.sErrFileName);
            Converter(Parser.sOutDir, Parser.sJrnFileName);
        }
        //сохраняем оставшиеся звонки
        Parser.RefreshResidFileName();
        SaveResiduaryCalls(Parser.sOutDir, Parser.sOutRm3FileName);
        builder.CleanCalls();
}

void ComleteOnLineActions()
{
    Loger(sMessage);
    builder.CleanCalls();
    Loger("Terminating process...");
}

int RestoreMessage(const DWORD dwLen, CDRBuilding::TPCharList& lstStr)
{
    iReadBytes -= 4;
    lseek(fd_tfs,-4, SEEK_CUR);
    builder.OnComoverload(ALL_MODULES);

    char *sTmpJrnStr = new char[128];
    if (!sTmpJrnStr) 
        WARNING;

    sprintf(sTmpJrnStr, "Error message lengh (%d) at address:0x%08X ", dwLen, iReadBytes);
    //запись в err
    lstStr.push_back(sTmpJrnStr);
    if(!WriteStringsToFile(Parser.sOutDir, Parser.sErrFileName, lstStr))
    {
        strcpy(sMessage, "Error: can't write to err-file\n");
        return -1;
    }
    builder.FreeListMem(lstStr);

    //может это tfs-файл со флжшки, тогда можно попытаться поискать сообщение ALIVE
    int iSpiderStartLen = sizeof(TClock) + sizeof(BYTE) + sizeof(BYTE) + sizeof(TCallID) + sizeof(BYTE); // 15
    int iCnt = 0;
    int dwTmp;
    BYTE btTmp;
    DWORD dwMesLen = 0xFFFFFFFF;
    while(true)
    {
        dwTmp = read(fd_tfs,&btTmp,sizeof(BYTE));

        if(dwTmp < sizeof(BYTE))//если конец файла или ошибка
        {
            strcpy(sMessage, "Error: can't find correct message lengh\n");
            return -1;
        }
        iCnt++;
        iReadBytes++;
        dwMesLen = dwMesLen >> 8;
        dwMesLen += ((DWORD)btTmp << 24);
        //(BYTE*)&dwMesLen+3 = btTmp;
        if(dwMesLen == iSpiderStartLen)
        {//как только нули кончатся - пойдут 4 байта длина мессаги
            sTmpJrnStr = new char[128];
            if (!sTmpJrnStr) 
                WARNING;
            sprintf(sTmpJrnStr, "Found correct message lengh (%d) after %d bytes", iSpiderStartLen,iCnt);
            //запись в err
            lstStr.push_back(sTmpJrnStr);
            if(!WriteStringsToFile(Parser.sOutDir, Parser.sErrFileName, lstStr))
            {
                strcpy(sMessage, "Error: can't find correct message lengh\n");
                return -1;
            }
            builder.FreeListMem(lstStr);
            return dwMesLen;
        }
    }
}

void Visualisation(const char* sCurrFileName)
{
    printf("\n<-------------------------------Callbuilder v0.5.4---------------------------->\n");

    if (Parser.sSpiderIp) 
    {
        printf("Spider ip address:%s\n", Parser.sSpiderIp);
        printf("Spider port:%d\n",Parser.SpiderPort);
        if(Parser.fDaemon) printf("Daemon mode!\n");
    }
    else
        Parser.rotation = Parser.tfsFileType;

    if(Parser.sInDir) printf("Input directory:%s\n",Parser.sInDir);
    if(Parser.sOutDir) printf("Output directory:%s\n",Parser.sOutDir);
    printf("Tfs file name:%s\n",sCurrFileName);
    switch(Parser.rotation)
    {
        case ROTATION_REALTIME:
            printf("Rotation:realtime\n");
        break;
        case ROTATION_DAY:
            printf("Rotation:day\n");
        break;
        case ROTATION_DECADE:
            printf("Rotation:decade\n");
        break;
        case ROTATION_MONTH:
            printf("Rotation:month\n");
        break;
        case ROTATION_ONLINE:
            printf("Rotation:online\n");
        break;
        case ROTATION_FROM_ATS:
            printf("Rotation:from ats\n");
        break;
        default:
        {
            WARNING;
            printf("Warning:Rotation:unknown!\n");
        }
    }
    if(Parser.sInRm3FileName) printf("Input residuary file:%s\n",Parser.sInRm3FileName);
    if(Parser.sOutRm3FileName) printf("Output residuary file:%s\n",Parser.sOutRm3FileName);
    if(Parser.sTfsFileNameBase) printf("File name base:%s\n",Parser.sTfsFileNameBase);
    if(Parser.sLogFileName) printf("Log file name:%s\n",Parser.sLogFileName);
    if(Parser.sErrFileName) printf("Err file name:%s\n",Parser.sErrFileName);
    if(Parser.sJrnFileName) printf("Jrn file name:%s\n",Parser.sJrnFileName);
    if(!Parser.SSettings.bWriteUnansweredCalls) printf("Unanswered calls are not writing!\n");
    if(!Parser.Sett.bEnable) printf("Journal is off!\n");
    printf("Journal maximum busy duration %d\n",Parser.Sett.iMaxDuration);
    if(!Parser.SSettings.bWriteBinary2) printf("No binary 2 at logfile strings!\n");
    if(!Parser.fConvert) printf("No converted files!\n");
    if(Parser.fRem_rm3) printf("Removing rm3 files after success!\n");
    printf("CDR string format:%d\n",Parser.SSettings.btCDRStringFormat);
    if(strcmp(Parser.SSettings.sNoNumberString,"-"))
        printf("No number string separator:\"%s\"\n",Parser.SSettings.sNoNumberString);
    if(Parser.SPrefix.bCutPrefix)
        printf("Cut prefix:%s\n",Parser.SPrefix.sPrefix);
    if(!Parser.lstLocalNumbers.empty())
    {
        printf("Inner numbers:\n");
        CDRBuilding::TListInterval::const_iterator it;
        for (it = Parser.lstLocalNumbers.begin(); it != Parser.lstLocalNumbers.end(); it++)
        {
            printf("%s-%s\n", it->beg, it->end);
        }
    }
    printf("<--------------------------------------------------------------------------->\n\n");
}

int max(int x, int y)
{
    return x > y ? x : y;
}

void *Server_ptread(void* arg)
{
    pthread_detach(pthread_self());
    char log[100];
    char Client_Ip[MAX_CONNECT_POINT/2][16];
    fd_set fds;
    struct timeval tv;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int retval;
    int i, maxfd = 0;
    while(1)
    {
        FD_ZERO(&fds);
        for (i = 0; i < MAX_CONNECT_POINT; i++) //3 servers and 3 clients
        {
            if (connect_point[i] > 0) 
            {
                FD_SET(connect_point[i], &fds);
                maxfd = max(maxfd, connect_point[i]);
            }
        }
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if ((retval =  select( maxfd + 1, &fds, NULL, NULL, &tv)) > 0)
        {
            for (i = 0; i < MAX_CONNECT_POINT/2; i++)
            {
                if (FD_ISSET(connect_point[i], &fds) > 0)
                {
                    int clnt_fd = accept(connect_point[i], (struct sockaddr *)&client_addr, &client_addr_size);
                    if (clnt_fd < 0)
                    {
                        sprintf(log,"Server %s: Client accept error!\n",GetConnectStr(i));
                        Loger(log);
                    }
                    else
                    {
                        if (connect_point[i + MAX_CONNECT_POINT/2])// Esli kanal s ATS ne rabochij
                        {
                            close(clnt_fd);
                            sprintf(log,"Server %s: Client accept denied! IP:%s\n", GetConnectStr(i),
                            inet_ntoa(client_addr.sin_addr));
                            Loger(log);
                        }
                        else//Esli kanal rabochij dobavliajem novogo klienta
                        {
                            if (make_nonblock(clnt_fd)<0)
                            {
                                close(clnt_fd);
                                sprintf(log,"Server %s: Can't make client socket nonblock! IP:%s\n", 
                                GetConnectStr(i), inet_ntoa(client_addr.sin_addr));
                                Loger(log);
                            }
                            else
                            {
                                connect_point[i + MAX_CONNECT_POINT/2] = clnt_fd;
                                strcpy(Client_Ip[i],inet_ntoa(client_addr.sin_addr));
                                sprintf(log,"Server %s: Client connected! IP:%s\n", GetConnectStr(i), Client_Ip[i]);
                                Loger(log);
                            }
                        }
                    }
                }
                if (FD_ISSET(connect_point[i + MAX_CONNECT_POINT/2], &fds) > 0)
                {
                    unsigned char temp_buff[64];
                    int size  = read(connect_point[i + MAX_CONNECT_POINT/2], temp_buff, 64);
                    if (size > 0) // if rcv data from client socket
                    {
                    }
                    else if ((size == 0) || ((size == -1)&&(errno != EINTR)))
                    {
                        sprintf(log, "Server %s: Client disconnected! IP:%s\n", GetConnectStr(i), Client_Ip[i]);
                        Loger(log);
                        close(connect_point[i + MAX_CONNECT_POINT/2]);
                        connect_point[i + MAX_CONNECT_POINT/2] = 0;
                    }
                }
            }
        }
        else if (retval < 0 /*&& errno != EINTR*/)//Esli oshibka ne sviazana s prepivanijem signalom
        {
            Loger("Server pthread internal error!\n");
        }
    }
    return(NULL);
}


void get_file_names(char *output, const char *base)
{
    tm current_time;
    time_t itime;

    time(&itime);
    localtime_r(&itime,&current_time);

    switch (Parser.rotation)
    {
      case ROTATION_DAY:
          sprintf(output,"%s_%02d_%02d_%d", base, current_time.tm_mday, current_time.tm_mon + 1, current_time.tm_year + 1900);
          break;
      case ROTATION_DECADE:
          sprintf(output,"%s_dec%02d_%d", base, (current_time.tm_yday/10)+1, current_time.tm_year + 1900);
          break;
      case ROTATION_MONTH:
          sprintf(output,"%s_mon%02d_%d", base, current_time.tm_mon + 1, current_time.tm_year + 1900);
          break;
    }
}


int main(int argc, char *argv[])
{
    /*
    printf("TClock:%d\n",sizeof(TClock));
    printf("strCDRTrunkUnit:%d\n",sizeof(CDRBuilding::strCDRTrunkUnit));
    printf("strCDR:%d\n",sizeof(CDRBuilding::strCDR));
    printf("strTime:%d\n",sizeof(CDRBuilding::strTime));
    printf("strCallInfoUnit:%d\n",sizeof(CDRBuilding::strCallInfoUnit));
    printf("strCallInfo:%d\n",sizeof(CDRBuilding::strCallInfo));
    printf("strModuleInfo:%d\n",sizeof(CDRBuilding::strModuleInfo));
    printf("strLocalNumbersSettings:%d\n",sizeof(CDRBuilding::strLocalNumbersSettings));
    printf("CCDRBuilder:%d\n",sizeof(CDRBuilding::CCDRBuilder));
    //exit(1);
    BYTE bt[8];
    bt[0] = 0;
    bt[1] = 1;
    bt[2] = 2;
    bt[3] = 3;
    bt[4] = 0xFF;
    bt[5] = 5;
    bt[6] = 6;
    bt[7] = 7;
    DWORD dw = 0;
    if( !((DWORD)&bt[0] & 3))
    {
        dw = *(DWORD*)&bt[1];
        printf("Odd adr0:%x adr:%x val:%x\n",(DWORD)&bt[0],(DWORD)&bt[1], dw);
    }
    else
    {
        dw = *(DWORD*)&bt[0];
        printf("Not odd adr0:%x adr:%x val:%x\n",(DWORD)&bt[0],(DWORD)&bt[0], dw);
    }
    */
    /*
    char test[80], blah[80];
     char *sep = "\\/:;=-";
     char *word, *phrase, *brkt, *brkb, *b1 = test, *b2 = blah;

     strcpy(test, "This;/is.a:test:of=the/string\\tokenizer-function.");

     for (word = strtok_r(test, sep, &brkt); word; word = strtok_r(NULL, sep, &brkt))
     {
         strcpy(blah, "blah:;;blat:blab:blag");

         for (phrase = strtok_r(blah, sep, &brkb); phrase; phrase = strtok_r(NULL, sep, &brkb))
         {
             printf("So far we're at{} %s:%s\n", word, phrase);
         }
     }
    */

    if (Parser.ParseCStringParams(argc, argv) < 0)
        exit(EXIT_FAILURE);

    builder.SetCommonSettings(&Parser.SSettings);
    builder.SetJournalSettings(&Parser.Sett);
    builder.SetLocalNumbersSettings(Parser.lstLocalNumbers, &Parser.SPrefix);
    CDRBuilding::TPCharList lstStr;

    int dwTmp;
    DWORD dwLen;
    DWORD dwError;

    if (Parser.sSpiderIp) //OnLine mode
    {
        if (!Parser.SpiderPort)
            Parser.SpiderPort = 10002;

        if (!Parser.ServerPort)
            Parser.ServerPort = Parser.SpiderPort + 1;

        if (Parser.FillOnLineParams(Parser.sTfsFileNameBase) < 0)
            exit(EXIT_FAILURE);

        StrToLog("Callbuilder started....\n");
        Visualisation("from spider online");

        for( int i = 0; i < MAX_CONNECT_POINT/2; i++ )
        {
            connect_point[i + MAX_CONNECT_POINT/2] = 0;
            if( (connect_point[i] = Create_server_point(Parser.ServerPort + 1 + i, i)) <0) 
            {
                printf("Error create server %s point!\n",GetConnectStr(i));
                connect_point[i] = 0;
            }
        }


        if (Parser.fDaemon)
            daemon(0,0);

        while ((Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0)
        {
            sleep(5);
        }


        pthread_t Server_tid;
        if(connect_point[0] || connect_point[1] || connect_point[2])
        {
            if (pthread_create(&Server_tid,NULL,&Server_ptread,NULL)!=0)
            {
                Loger("Can't create server tread!\n");
            }
        }

        iReadBytes = 0;
        while (true)
        {
            //chtenije dlinni mesagi
            dwTmp = read(Spider_fd, &dwLen, sizeof(DWORD));
            if (dwTmp != sizeof(DWORD))
            {
                Loger("Spider socket closed!\n");
                close(Spider_fd);
                while ((Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0)
                {
                    sleep(5);
                }
                continue;
            }
            iReadBytes += sizeof(DWORD);

            if (!dwLen || dwLen > sizeof(CMonMessageEx)+1)        // грубая проверка
            {
                    Loger( "Error: Message lengh is incorrect!\n");
                    close(Spider_fd);
                    while ((Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0)
                    {
                        sleep(5);
                    }
                    continue;
            }

            //chtenije mesagi
            unsigned char buff[sizeof(CMonMessageEx)+1];

            dwTmp = read(Spider_fd, &buff,dwLen);
            if (dwTmp != dwLen)
            {
                Loger("Spider socket error!\n");
                close(Spider_fd);
                while ((Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0)
                {
                    sleep(5);
                }
                continue;
            }
            iReadBytes += dwLen;

            //dekodirovanie mesagi
            CMonMessageEx mes(0);
            BYTE *p;
            if (!(p = mes.decode(buff, dwLen-1)))
            {
                Loger("Error: error while decoding message!\n");
                continue;
            }

            //obrabotka mesagi
            if (!builder.ProcessMessage(mes))
            {
                dwError = builder.GetLastError();
            }
            else
                dwError = ERROR_NONE;

            if (Parser.rotation && Parser.rotation != ROTATION_ONLINE) // esli faili drobiatsia po date
            {
                char file_names[200];
                get_file_names(file_names, Parser.sTfsFileNameBase);

                if(Parser.FillOnLineParams(file_names) < 0)
                {
                    strcpy(sMessage, "Error: Out of memory for dirs!\n");
                    break;
                }
            }
            //zapis mesagi v faili i klientam
            builder.GetErrorList(lstStr);
            WriteStringsOnLine(Parser.sOutDir, Parser.sErrFileName, lstStr, 1);
            builder.FreeListMem(lstStr);

            builder.GetJournalList(lstStr);
            WriteStringsOnLine(Parser.sOutDir, Parser.sJrnFileName, lstStr, 2);
            builder.FreeListMem(lstStr);

            builder.GetCDRList(lstStr);
            WriteStringsOnLine(Parser.sOutDir, Parser.sLogFileName, lstStr, 0);
            builder.FreeListMem(lstStr);

            if (dwError)
            {
                if (dwError == ERROR_UNKNOWN_MESSAGE)
                {
                }
                else if (dwError == ERROR_UNKNOWN_CDR_FORMAT)
                {
                    strcpy(sMessage, "Error: unknown CDR-string format!\n");
                    break;
                }
                else
                {
                    strcpy(sMessage, "Error: unknown error while message processing!\n");
                    break;
                }
            }
        }
        close(Spider_fd);
        ComleteOnLineActions();
        builder.FreeListMem(lstStr);
    }

    else if (!Parser.TfsFileList.empty()) //File mode
    {
        printf("Callbuilder: processing %d file(s)\n",Parser.TfsFileList.size());
        CDRBuilding::TPCharList::const_iterator it;
        for (it = Parser.TfsFileList.begin(); it != Parser.TfsFileList.end(); it++)
        {
            if (Parser.FillMainParams(*it) < 0)
            {
                if(Parser.TfsFileList.size() > 1)
                    continue;
                else
                    exit(EXIT_FAILURE);
            }

            Parser.RefreshResidFileName();
            Visualisation(*it);

            char path[200];
            sprintf(path, "%s/%s", Parser.sInDir, *it);
            if ((fd_tfs = open(path, O_RDONLY)) <= 0)
            {
                printf("Error: can't open file:%s!\n", path);
                if(Parser.TfsFileList.size() > 1)
                    continue;
                else
                    exit(EXIT_FAILURE);
            }

            off_t MainFilelen = 0;
            MainFilelen = lseek(fd_tfs, 0, SEEK_END);
            lseek(fd_tfs, 0, SEEK_SET);

            RemoveFiles();
            PutResiduaryCalls(Parser.sInDir, Parser.sInRm3FileName);

            BYTE RdPersent = 0;
            iReadBytes = 0;

            while (true)
            {
                //chtenije dlinni mesagi
                dwTmp = read(fd_tfs, &dwLen, sizeof(DWORD));
                if (dwTmp != sizeof(DWORD))
                {
                    if (!dwTmp) //EOF 
                    {
                        strcpy(sMessage, "Processing complete!\n");
                        if (Parser.fRem_rm3)
                        {
                            sprintf(path, "%s/%s", Parser.sInDir, Parser.sInRm3FileName);
                            remove(path);
                        }
                    }
                    else
                    {
                        strcpy(sMessage, "Error: error while reading message size!\n");
                    }
                    break;
                }
                iReadBytes += sizeof(DWORD);

                if (!dwLen || dwLen > sizeof(CMonMessageEx))    // грубая проверка
                {
                    if ((dwLen = RestoreMessage(dwLen, lstStr)) < 0)
                        break;
                }

                //chtenije mesagi
                unsigned char buff[sizeof(CMonMessageEx)];
                dwTmp = read(fd_tfs, &buff, dwLen);
                if (!dwTmp || dwTmp != dwLen)
                {
                    strcpy(sMessage, "Error: error while reading message!\n");
                    break;
                }
                iReadBytes += dwLen;

                //dekodirovanie mesagi
                CMonMessageEx mes(0);
                BYTE *p;
                if (!(p = mes.decode(buff, dwLen)))
                {
                    strcpy(sMessage,
                           "Error: error while decoding message!\n");
                    break;
                }

                //obrabotka mesagi
                if (builder.ProcessMessage(mes))
                {
                    dwError = builder.GetLastError();
                }
                else
                    dwError = ERROR_NONE;

                //zapis mesagi v faili
                builder.GetErrorList(lstStr);
                if (!WriteStringsToFile(Parser.sOutDir, Parser.sErrFileName, lstStr))
                {
                    strcpy(sMessage, "Error: can't write to err-file\n");
                    break;
                }
                builder.FreeListMem(lstStr);

                builder.GetJournalList(lstStr);
                if (!WriteStringsToFile(Parser.sOutDir, Parser.sJrnFileName, lstStr))
                {
                    strcpy(sMessage, "Error: can't write to jrn-file\n");
                    break;
                }
                builder.FreeListMem(lstStr);

                builder.GetCDRList(lstStr);
                if (!WriteStringsToFile(Parser.sOutDir, Parser.sLogFileName, lstStr))
                {
                    strcpy(sMessage, "Error: can't write to log-file\n");
                    break;
                }
                builder.FreeListMem(lstStr);

                if (dwError)
                {
                    if (dwError == ERROR_UNKNOWN_MESSAGE)
                    {
                    }
                    else if (dwError == ERROR_UNKNOWN_CDR_FORMAT)
                    {
                        strcpy(sMessage, "Error: unknown CDR-string format!\n");
                        break;
                    }
                    else
                    {
                        strcpy(sMessage, "Error: unknown error while message processing!\n");
                        break;
                    }
                }
                if ((iReadBytes >= (RdPersent+1) * (MainFilelen / 4)))
                {
                    RdPersent++;
                    printf("Comlete:%d%\n", RdPersent * 25);
                }
            }
            close(fd_tfs);
            ComleteActions();
            builder.FreeListMem(lstStr);
        }
        return EXIT_SUCCESS;
    }
    else
        printf("No tfs files in directory!\n");
    return EXIT_SUCCESS;
}
