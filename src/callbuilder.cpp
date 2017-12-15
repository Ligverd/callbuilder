
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
#include "CDRRadius.h"
#include "client.h"
#include "netmes.h"
#include <stdarg.h>

using namespace std;

//global variables
struct sigaction sact;
volatile bool fMainRun = true;

CParser Parser;
CDRBuilding::CCDRBuilder builder;
CSMPClient *SMP = NULL;
pthread_mutex_t SMPfMutex = PTHREAD_MUTEX_INITIALIZER;

int Spider_fd = 0;
int fd_tfs = 0;
DWORD iReadBytes = 0;
int connect_point[MAX_CONNECT_POINT];
char sMessage[128];
//

void get_time_str(char* tm_str, tm T)
{
  sprintf(tm_str,"[%02d-%02d-%04d %02d:%02d:%02d]", T.tm_mday, T.tm_mon+1, (T.tm_year+1900), T.tm_hour, T.tm_min,T.tm_sec);
}

bool StrToLog(const char* str)
{
    int fd;
    bool fret = true;

    if(!Parser.sLogFile)
        return false;

    if ( (fd = open(Parser.sLogFile, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        printf("Can't write to log file:%s\n", Parser.sLogFile);
        return false;
    }

    if(write(fd, str, strlen(str)) < 0)
    {
        close(fd);
        return false;
    }

    off_t flen = lseek(fd, 0, SEEK_CUR);

    if(flen >= Parser.nLogFileSize * 1024 * 1024)
    {
        std::string name1(Parser.sLogFile);
        std::string name2(Parser.sLogFile);
        name1 += ".1";
        name2 += ".2";
        remove(name2.c_str());
        rename(name1.c_str(), name2.c_str());
        rename(Parser.sLogFile, name1.c_str());
    }

    close(fd);
    return true;
}

void Loger(const char* str)
{
    char time_str[22];
    char logstr[1024];
    time_t itime;
    tm T;
    time (&itime);
    localtime_r(&itime, &T);
    printf("%s", str);
    get_time_str(time_str, T);
    sprintf(logstr, "%s %s", time_str, str);
    StrToLog(logstr);
}

int Logerf(const char *format, ...)
{
    char buff[512];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buff, sizeof(buff), format, ap);
    va_end(ap);
    Loger(buff);
    return 0;
}


bool make_nonblock(int sock)
{
    int sock_opt;
    if((sock_opt = fcntl(sock, F_GETFL, O_NONBLOCK)) < 0)
        return false;

    if((sock_opt = fcntl(sock, F_SETFL, sock_opt | O_NONBLOCK)) < 0)
        return false;

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
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)) < 0)
    {
        Loger("Can't set callbuilder server socket option SO_REUSEADDR!\n");
    }

    sock_opt = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &sock_opt, sizeof(sock_opt)) < 0)
    {
        Loger("Can't set callbuilder server socket option SO_KEEPALIVE!\n");
    }

#ifdef TCP_OPT
    sock_opt = 60;
    if(setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &sock_opt, sizeof(sock_opt)) < 0)
    {
        Loger("Can't set callbuilder server socket option TCP_KEEPIDLE!\n");
    }

    sock_opt = 60;
    if(setsockopt(fd, IPPROTO_TCP,TCP_KEEPINTVL, &sock_opt, sizeof(sock_opt)) < 0)
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
        sprintf(log, "Error callbuilder server with port:%d\n", port);
        Loger(log);
        return -1;
    }

    if (listen(fd, 1) < 0)
    {
        close(fd);
        printf("Listen callbuilder server socket error!\n");
        return -1;
    }
    sprintf(log, "callbuilder %s server listening port:%d\n", GetConnectStr(i), ntohs(server_addr.sin_port));
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
    sprintf(path, "%s/%s", Parser.sOutDir,Parser.sLogFileName);
    remove(path);
    sprintf(path, "%s/%s", Parser.sOutDir,Parser.sErrFileName);
    remove(path);
    sprintf(path, "%s/%s", Parser.sOutDir,Parser.sJrnFileName);
    remove(path);
}


bool WriteStrToFile(const char* path, const char *str)
{
    int fd;
    bool bRes = true;

    if ( (fd = open(path, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
        return false;

    if(write(fd, str, strlen(str)) < 0)
        bRes = false;

    close(fd);
    return bRes;
}

bool WriteStringsToFile(const char* sDir ,const char* sFileName, CDRBuilding::TPCharList& lstStr)
{
    if(lstStr.empty())
        return true;

    bool bRes = true;
    char path[200];
    sprintf(path,"%s/%s", sDir, sFileName);
    int fd;
    if ( (fd = open(path, O_RDWR | O_CREAT | O_APPEND , 0666 )) < 0)
    {
        printf("Error: can't write to file:%s!\n", sFileName);
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
            if( write(fd,sTmp, strlen(sTmp)) < 0)
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
        return;

    char log[300];

    char path[200];
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
                        int ERROR = -1;
                        socklen_t opt_size = sizeof(ERROR);
                        getsockopt(connect_point[i + MAX_CONNECT_POINT/2], SOL_SOCKET, SO_ERROR, &ERROR, &opt_size);
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
            printf("%s", sTmp);
        }
    }
    return;
}

int SaveResiduaryCalls(const char* sDir, const char* sFileName)
{
    int fd, n;
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
        return 0;

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
    return 0;
}

int Converter(const char* sDir ,const char* sFileName)
{
    int fd_in,fd_out;

    char path[204], c;
    sprintf(path, "%s/%s", sDir, sFileName);

    if ( (fd_in = open(path, O_RDONLY)) < 0)
    {
        printf("Converter: can't open file:%s\n",path);
        return -1;
    }

    strcat(path, ".cnv");
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
        printf("%s", sMessage);
        if (Parser.fConvert)
        {
            Converter(Parser.sOutDir, Parser.sErrFileName);
            Converter(Parser.sOutDir, Parser.sJrnFileName);
        }
        //сохраняем оставшиеся звонки
        Parser.RefreshResidFileName();
        SaveResiduaryCalls(Parser.sInDir, Parser.sOutRm3FileName);
        builder.CleanCalls();
}

void ComleteOnLineActions()
{
    Loger(sMessage);
    builder.CleanCalls();
}

int RestoreMessage(const DWORD dwLen, CDRBuilding::TPCharList& lstStr)
{
    iReadBytes -= 4;
    lseek(fd_tfs, -4, SEEK_CUR);
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
    Logerf("\n<-------------------------------Callbuilder v%s---------------------------->\n", VERSION);

    if (Parser.sSpiderIp)
    {
        Logerf("Spider ip address:%s\n", Parser.sSpiderIp);
        Logerf("Spider port:%d\n",Parser.SpiderPort);
        if(Parser.fDaemon) Loger("Daemon mode!\n");
    }
    else
        Parser.rotation = Parser.tfsFileType;

    if(Parser.sInDir) Logerf("Input directory:%s\n", Parser.sInDir);
    if(Parser.sOutDir) Logerf("Output directory:%s\n", Parser.sOutDir);
    Logerf("Tfs file name:%s\n", sCurrFileName);
    switch(Parser.rotation)
    {
        case ROTATION_REALTIME:
            Loger("Rotation:realtime\n");
        break;
        case ROTATION_DAY:
            Loger("Rotation:day\n");
        break;
        case ROTATION_DECADE:
            Loger("Rotation:decade\n");
        break;
        case ROTATION_MONTH:
            Loger("Rotation:month\n");
        break;
        case ROTATION_ONLINE:
            Loger("Rotation:online\n");
        break;
        case ROTATION_FROM_ATS:
            Loger("Rotation:from ats\n");
        break;
        case ROTATION_FROM_SS:
            Loger("Rotation:from ss\n");
        break;
        default:
        {
            WARNING;
            Loger("Warning:Rotation:unknown!\n");
        }
    }
    if(Parser.sInRm3FileName) Logerf("Input residuary file:%s\n", Parser.sInRm3FileName);
    if(Parser.sOutRm3FileName) Logerf("Output residuary file:%s\n", Parser.sOutRm3FileName);
    if(Parser.sTfsFileNameBase) Logerf("File name base:%s\n", Parser.sTfsFileNameBase);
    if(Parser.sLogFileName) Logerf("Log file name:%s\n", Parser.sLogFileName);
    if(Parser.sErrFileName) Logerf("Err file name:%s\n", Parser.sErrFileName);
    if(Parser.sJrnFileName) Logerf("Jrn file name:%s\n", Parser.sJrnFileName);
    if(!Parser.SSettings.bWriteUnansweredCalls) Loger("Unanswered calls are not writing!\n");
    if(!Parser.Sett.bEnable) Loger("Journal is off!\n");
    Logerf("Journal maximum busy duration %d\n", Parser.Sett.iMaxDuration);
    if(!Parser.SSettings.bWriteBinary2) Loger("No binary 2 at logfile strings!\n");
    if(!Parser.fConvert) Loger("No converted files!\n");
    if(Parser.fRem_rm3) Loger("Removing rm3 files after success!\n");
    Logerf("CDR string format:%d\n",Parser.SSettings.btCDRStringFormat);
    if(strcmp(Parser.SSettings.sNoNumberString,"-"))
        Logerf("No number string separator:\"%s\"\n",Parser.SSettings.sNoNumberString);
    if(Parser.SPrefix.bCutPrefix)
        Logerf("Cut prefix:%s\n",Parser.SPrefix.sPrefix);
    if(!Parser.lstLocalNumbers.empty())
    {
        Loger("Inner numbers:\n");
        CDRBuilding::TListInterval::const_iterator it;
        for (it = Parser.lstLocalNumbers.begin(); it != Parser.lstLocalNumbers.end(); it++)
        {
            Logerf("%s-%s\n", it->beg, it->end);
        }
    }
    if(Parser.sRadiusAccIp[0])
    {
        Logerf("Radius accounting server ip:%s\n", Parser.sRadiusAccIp);
        Logerf("Radius accounting server port:%d\n", Parser.nRadiusAccPort);
        switch(Parser.nRadiusAccBilling)
        {
            case BILLING_LANBILLING:
                Loger("Radius accounting system:LANBILLING\n");
            break;
            case BILLING_UTM5:
                Loger("Radius accounting system:UTM5\n");
            break;
            default:
                Loger("Radius accounting system:UNKNOWN\n");
        }
        if(Parser.sRadiusAccSecret[0]) Logerf("Radius accounting server secret:%s\n", Parser.sRadiusAccSecret);

        if(Parser.sRadiusConfigFile[0]) Logerf("Radius conig file:%s\n", Parser.sRadiusConfigFile);
        if(Parser.sRadiusDictionaryFile[0]) Logerf("Radius dictionary file:%s\n", Parser.sRadiusDictionaryFile);
        if(Parser.sRadiusSeqFile[0]) Logerf("Radius sequence file:%s\n", Parser.sRadiusSeqFile);
        Logerf("Radius send retryes:%u\n", Parser.nRadiusSndRetry);
        Logerf("Radius send timeout:%u\n", Parser.nRadiusSndTimeout);
    }
    if(Parser.sRadiusAuthIp[0])
    {
        Logerf("Radius authorization server ip:%s\n", Parser.sRadiusAuthIp);
        Logerf("Radius authorization server port:%d\n", Parser.nRadiusAuthPort);
        switch(Parser.nRadiusAuthBilling)
        {
            case BILLING_LANBILLING:
                Loger("Radius authorization system:LANBILLING\n");
            break;
            case BILLING_UTM5:
                Loger("Radius authorization system:UTM5\n");
            break;
            default:
                Loger("Radius authorization system:UNKNOWN\n");
        }

        if(Parser.sScommIp[0]) Logerf("scomm ip:%s\n", Parser.sScommIp);
        Logerf("scomm port:%u\n", Parser.nScommPort);
        if(Parser.sScommPassword[0]) Logerf("scomm password:%s\n", Parser.sScommPassword);
        if(Parser.fRadAuthPrepay) Loger("Authorization prepay\n");
        if(Parser.fRadTrace) Loger("Radius trace enabled\n");
    }
    Logerf("Log file size:%d MB\n", Parser.nLogFileSize);
    Loger("<--------------------------------------------------------------------------->\n\n");
}

int max(int x, int y)
{
    return x > y ? x : y;
}

void SMPWritePacket (BYTE* data, short len )
{
    pthread_mutex_lock(&SMPfMutex);
    if(SMP)
        SMP->SendPacket(data, len);
    pthread_mutex_unlock(&SMPfMutex);

    /*
    if(pthread_mutex_trylock(&SMPfMutex) == 0)
    {
        if(SMP)
            SMP->SendPacket(data, len);
        pthread_mutex_unlock(&SMPfMutex);
    }
    */
}

CSMPClient *SMPCreate (const char *ip, unsigned short port, int con )
{
    CSMPClient *psmp = new CSMPClient(con);
    if(!psmp)
        return NULL;
    psmp->SetThreadPriority(SCHED_OTHER, 0);
    psmp->SetMutex(&SMPfMutex);
    const char *str = psmp->MyConnect(ip, port);
    if (str)
    {
        psmp->OnClose(str);
        return NULL;
    }
    if(psmp->SwitchBinary(Parser.sScommPassword))
    {
        psmp->OnClose("BINARYMODE failed");
        return NULL;
    }
    return psmp;
}

void *Prepay_ptread(void* arg)
{
    pthread_detach(pthread_self());
    int delay = 30;

    while(fMainRun)
    {
        usleep(100*1000);
        if(delay++ < 50)
            continue;

        delay = 0;

        CNetMessageBody xmes;
        xmes.dst.nMod = 0xFF;
        xmes.nMessage = NET_MES_REGISTER_PREPAY;
        xmes.un.multi.set.Full();

        uc buf[sizeof(CNetMessageBody)];
        uc* p = xmes.encode(buf);
        short len = p - buf;
        SMPWritePacket(buf, len);
    }

    return NULL;
}

void *Server_ptread(void* arg)
{
    pthread_detach(pthread_self());

    bool fCheckSMP = false;
    int nNoSMPCnt = 5;

    if(Parser.sScommIp[0] && Parser.nScommPort)
    {
        fCheckSMP = true;
        if(Parser.fRadAuthPrepay)
        {
            pthread_t Prepay_tid;
            if (pthread_create(&Prepay_tid, NULL, &Prepay_ptread, NULL) != 0 )
                Loger("Can't create prepay tread!\n");
        }
    }

    char log[100];
    char Client_Ip[MAX_CONNECT_POINT/2][16];
    fd_set fds;
    struct timeval tv;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int retval;
    int i, maxfd = 0;

    while(fMainRun)
    {
        if(fCheckSMP)
        {
            if(!SMP)
            {
                if(nNoSMPCnt++ >= 5)
                {
                    SMP = SMPCreate (Parser.sScommIp, Parser.nScommPort, 0);
                    nNoSMPCnt = 0;
                }
            }
        }

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
                            if (make_nonblock(clnt_fd) < 0 )
                            {
                                close(clnt_fd);
                                sprintf(log,"Server %s: Can't make client socket nonblock! IP:%s\n", 
                                GetConnectStr(i), inet_ntoa(client_addr.sin_addr));
                                Loger(log);
                            }
                            else
                            {
                                connect_point[i + MAX_CONNECT_POINT/2] = clnt_fd;
                                strcpy(Client_Ip[i], inet_ntoa(client_addr.sin_addr));
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
                    else if ((size == 0) || ((size == -1) && (errno != EINTR)))
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
    return NULL;
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

void sig_SIGTERM_hndlr(int signo)
{
    Loger("Terminating process...\n");
    fMainRun = false;
}

void sig_SIGPIPE_hndlr(int signo)
{
    Loger("Signal SIGPIPE received!\n");
}

//============================== класс CSIPMessage ==============================
CSIPMessage::CSIPMessage(void)
{
    clearMessage();
}

CSIPMessage::CSIPMessage(const char* message)
{
    MYASSERT(setMessage(message));
}

CSIPMessage::~CSIPMessage(void)
{
}

void CSIPMessage::clearMessage()
{
    m_sMessage[0] = 0;
    m_MessageLen = 0;
}

bool CSIPMessage::setMessage(const char* message)
{
    size_t len = strlen(message);
    if(len >= sizeof(m_sMessage))
        return false;

    strcpy(m_sMessage, message);
    m_MessageLen = len;
    return true;
}

bool CSIPMessage::setMessage(const BYTE* data, size_t len)
{
    if(len >= sizeof(m_sMessage))
        return false;

    memcpy(m_sMessage, data, len);
    m_sMessage[len] = '\0';
//    MYASSERT(strlen(m_sMessage) == len);
    m_MessageLen = len;
    return true;
}

bool CSIPMessage::getMessage(char* buffer, size_t size) const
{
    if(!buffer)
        return false;

    if(size <= m_MessageLen)
        return false;

    strcpy(buffer, m_sMessage);
    return true;
}

const char* CSIPMessage::getMessageHeaderPtr( void ) const
{
    if(!m_MessageLen)
        return NULL;

    char *ptr = strstr(m_sMessage, CRLF_CHARS);
    if(!ptr)
        return NULL;

    while(*ptr == '\r' || *ptr == '\n')
        ptr++;

    if(*ptr == '\0')
        return NULL;

    if(ptr >= m_sMessage + m_MessageLen)
        return NULL;

    return ptr;
}

const char* CSIPMessage::getMessageBodyPtr( void ) const
{
    if(!m_MessageLen)
        return NULL;

    char *ptr = strstr(m_sMessage, "\r\n\r\n");
    if(!ptr)
        return NULL;

    while(*ptr == '\r' || *ptr == '\n')
        ptr++;

    if(*ptr == '\0')
        return NULL;

    if(ptr >= m_sMessage + m_MessageLen)
        return NULL;

    return ptr;
}

bool CSIPMessage::addString(const char* str)
{
    size_t len = strlen(str);
    MYASSERT_RET_FALSE((m_MessageLen + len + 2) < sizeof(m_sMessage));
    strcat(m_sMessage, str);
    m_MessageLen += len;
    strcat(m_sMessage, CRLF_CHARS);
    m_MessageLen += 2;
    return true;
}

void CSIPMessage::trimBuffer(char* buffer)
{
    char *p = buffer;

    while(*p == ' ')
        p++;

    if(p > buffer)
        strcpy(buffer, p);

    size_t len = strlen(buffer);
    if(!len)
        return;

    p = &buffer[len - 1];

    while(*p == ' ')
        p--;

    *(p + 1) = '\0';
}

const char* CSIPMessage::getShortHeaderName(const char* sFullHeaderName)
{
    if(sFullHeaderName == NULL)
        return NULL;

    const char* sRes = NULL;
    if(!strcmp(sFullHeaderName, SIP_HEADER__CONTENT_TYPE))
        sRes = SIP_HEADER_SHORT__CONTENT_TYPE;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__CONTENT_ENCODING))
        sRes = SIP_HEADER_SHORT__CONTENT_ENCODING;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__FROM))
        sRes = SIP_HEADER_SHORT__FROM;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__CALL_ID))
        sRes = SIP_HEADER_SHORT__CALL_ID;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__CONTACT))
        sRes = SIP_HEADER_SHORT__CONTACT;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__CONTENT_LENGTH))
        sRes = SIP_HEADER_SHORT__CONTENT_LENGTH;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__SUBJECT))
        sRes = SIP_HEADER_SHORT__SUBJECT;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__TO))
        sRes = SIP_HEADER_SHORT__TO;
    else
    if(!strcmp(sFullHeaderName, SIP_HEADER__VIA))
        sRes = SIP_HEADER_SHORT__VIA;
    return sRes;
}


bool CSIPMessage::__getNextHeader(char* buffer, size_t size, const char **last) const
{
    MYASSERT_RET_FALSE(buffer);
    const char *start = *last;
    char *end = strstr(start, CRLF_CHARS);

    if(end)
    {
        if(end == start)
            return false;

        size_t len = (size_t)(end - start);
        MYASSERT_RET_FALSE(len < size);

        strncpy(buffer, start, len);
        buffer[len] = '\0';
        *last = end + 2;
        return true;
    }

    return false;
}

bool CSIPMessage::getNextHeader(char* sHeaderName, size_t header_name_size, char* sHeader, size_t header_size, const char **last) const
{
    if(!m_MessageLen)
        return false;

    char str[500];
    if(!__getNextHeader(str, sizeof(str), last))
        return false;

    trimBuffer(str);

    if(strlen(str) >= header_size)
        return false;
    strcpy(sHeader, str);

    char *v = strchr(str, ':');
    if(v)
    {
        *v = '\0';

        trimBuffer(str);

        if(strlen(str) >= header_name_size)
            return false;
        strcpy(sHeaderName, str);

        return true;
    }
    return false;
}

bool CSIPMessage::getHeaderByName(const char* sHeaderName, char* buffer, size_t size) const
{
    const char *last = getMessageHeaderPtr();
    if(!last)
        return false;

    if(!sHeaderName)
        return false;

    size_t header_len = strlen(sHeaderName);
    if(!header_len)
        return false;

    const char *sh = getShortHeaderName(sHeaderName);

    while(__getNextHeader(buffer, size, &last))
    {
        char *v = strchr(buffer, ':');
        if(v)
        {
            trimBuffer(buffer);

            if(!strncmp(buffer, sHeaderName, header_len))
                return true;
            else if(sh)
            {
                if(*buffer == *sh)
                    return true;
            }
        }
    }

    return false;
}

bool CSIPMessage::replaseHeaderByName(const char* sHeaderName, const char* sNewHeader)
{
    if(!sHeaderName)
        return false;

    size_t header_len = strlen(sHeaderName);
    if(!header_len)
        return false;

    const char *sShortHeaderName = getShortHeaderName(sHeaderName);

    if(!sNewHeader)
        return false;

    size_t new_len = strlen(sNewHeader);
    if(!new_len)
        return false;

    const char *last = getMessageHeaderPtr();
    if(!last)
        return false;

    char *first = const_cast<char *>(last);
    char *p1 = const_cast<char *>(last);

    char str[500];
    while(__getNextHeader(str, sizeof(str), &last))
    {
        char *v = strchr(str, ':');
        if(v)
        {
            bool found = false;
            if(!strncmp(str, sHeaderName, header_len))
            {
                found = true;
            }
            else if(sShortHeaderName)
            {
                if(*str == *sShortHeaderName)
                    found = true;
            }
            if(found)
            {
                char *p2 = const_cast<char *>(last);
                size_t slen = strlen(str) + 2; //CLRF
                if(new_len > slen)
                {
                    size_t del = new_len - slen;
                    if(m_MessageLen + del >= sizeof(m_sMessage))
                        return false;
                    memmove(p2 + del, p2, m_MessageLen - (size_t)(p2 - first));
                    memcpy(p1, sNewHeader, new_len);
                    m_MessageLen += del;
                    return true;
                }
                else
                {
                    size_t del = slen - new_len;
                    memcpy(p2 - del, p2, m_MessageLen - (size_t)(p2 - first));
                    memcpy(p1, sNewHeader, new_len);
                    m_MessageLen -= del;
                    return true;
                }
            }
        }
        p1 = const_cast<char *>(last);
    }
    return false;
}

char* CSIPMessage::splitHeaderValueFromParams(const char* field)
{
    size_t len = strlen(field);
    if(!len)
        return NULL;

    const char *p = field;
    int nInB = 0; // < >
    bool fInQ = false; // " "

    for(size_t i = 0; i < len; i ++)
    {
        switch (p[i])
        {
            case '<':
                nInB++;
            break;
            case '>':
                if(nInB > 0)
                    nInB--;
            break;
            case '\"':
                fInQ = !fInQ;
            break;
            default:;
        }

        if( nInB || fInQ )
            continue;

        if ( p[i] == ';' )
            return const_cast<char *>(&p[i]);
     }
    return NULL;
}

bool CSIPMessage::getHeaderValueByName(const char* sHeaderName, char* buffer, size_t size) const
{
    char str[500];
    if(!getHeaderByName(sHeaderName, str, sizeof(str)))
        return false;

    char *v = strchr(str, ':');
    if(v)
    {
        trimBuffer(++v);

        char *p = splitHeaderValueFromParams(v);
        if(p)
            *p = '\0';

        if(strlen(v) >= size)
            return false;

        strcpy(buffer, v);
        return true;
    }
    return false;
}

bool CSIPMessage::getNextHeaderValue(char* header, size_t header_size, char* value, size_t value_size, const char **last) const
{
    if(!m_MessageLen)
        return false;

    char str[500];
    if(!__getNextHeader(str, sizeof(str), last))
        return false;

    trimBuffer(str);

    char *v = strchr(str, ':');
    if(v)
    {
        *v++ = '\0';

        trimBuffer(v);

        char *p = splitHeaderValueFromParams(v);
        if(p)
            *p = '\0';

        if(strlen(v) >= value_size)
            return false;
        strcpy(value, v);

        trimBuffer(str);

        if(strlen(str) >= header_size)
            return false;
        strcpy(header, str);

        return true;
    }
    return false;
}

bool CSIPMessage::findHeaderParamByName(const char* sHeaderName, const char* sParamName) const
{
    size_t param_len = strlen(sParamName);
    if(!param_len)
        return false;

    char str[500];
    if(!getHeaderByName(sHeaderName, str, sizeof(str)))
        return false;

    char *v = strchr(str, ':');
    if(!v)
        return false;

    char *start, *end = splitHeaderValueFromParams(++v);
    if(!end)
        return false;

    do
    {
        start = end + 1;
        end = strchr(start, ';');
        if(end)
            *end = '\0';

        while(*start == ' ')
            start++;

        if(!strncmp(start, sParamName, param_len))
            return true;

    } while (end);

    return false;
}

bool CSIPMessage::getHeaderParamValueByName(const char* sHeaderName, const char* sParamName, char* buffer, size_t size) const
{
    size_t param_len = strlen(sParamName);
    if(!param_len)
        return false;

    char str[500];
    if(!getHeaderByName(sHeaderName, str, sizeof(str)))
        return false;

    char *v = strchr(str, ':');
    if(!v)
        return false;

    char *start, *end = splitHeaderValueFromParams(++v);
    if(!end)
        return false;

    do
    {
        start = end + 1;
        end = strchr(start, ';');
        if(end)
            *end = '\0';

        while(*start == ' ')
            start++;

        if(!strncmp(start, sParamName, param_len))
        {
            char *ch = strchr(start, '=');
            if(ch)
            {
                if(strlen(++ch) >= size)
                    return false;

                strcpy(buffer, ch);
                return true;
            }
        }

    } while (end);

    return false;
}

bool CSIPMessage::retreiveParamFromHeader(const char* sParamName, const char* sHeader)
{
    if(!sHeader)
        return false;

    if(!sParamName)
        return false;

    size_t param_len = strlen(sParamName);

    char *v = strchr(sHeader, ':');
    if(!v)
        return false;

    char *start, *end = splitHeaderValueFromParams(++v);
    if(!end)
        return false;

    do
    {
        start = end;
        char *p = start + 1;
        while(*p == ' ')
            p++;
        end = strchr(p, ';');
        if(!strncmp(p, sParamName, param_len))
        {
            if(end)
                strcpy(start, end);
            else
                *start = '\0';
            return true;
        }

    } while (end);

    return false;
}

bool CSIPMessage::getBodyParamByName(const char ParamName, char* buffer, size_t size) const
{
    if(!m_MessageLen)
        return false;

    const char *last = getMessageBodyPtr();
    if(!last)
        return false;

    char str[500];

    while(__getNextHeader(str, sizeof(str), &last))
    {
        trimBuffer(str);

        size_t len = strlen(str);
        if(len > 2)
        {
            if(str[1] == '=')
            {
                if(str[0] == ParamName)
                {
                    char *v = &str[2];
                    trimBuffer(v);
                    if(strlen(v) >= size)
                        return false;

                    strcpy(buffer, v);
                    return true;
                }
            }
        }
    }
    return false;
}

bool CSIPMessage::getBodyParamByName(const char *sParamName, char* buffer, size_t size) const
{
    if(!m_MessageLen)
        return false;

    const char *last = getMessageBodyPtr();
    if(!last)
        return false;

    char str[500];

    while(__getNextHeader(str, sizeof(str), &last))
    {
        trimBuffer(str);

        char *v = strchr(str, '=');
        if(v)
        {
            *v++ = '\0';
            if(!strcmp(str, sParamName))
            {
                trimBuffer(v);
                if(strlen(v) >= size)
                    return false;

                strcpy(buffer, v);
                return true;
            }
        }
    }
    return false;
}

bool CSIPMessage::getNextBodyParam(char *param, char* value, size_t value_size, const char **last) const
{
    char str[500];
    if(!m_MessageLen)
        return false;

    if(!__getNextHeader(str, sizeof(str), last))
        return false;

    trimBuffer(str);

    if(strlen(str) <= 2)
        return false;

    if(str[1] == '=')
    {
        *param = str[0];
        char *sss = &str[2];
        trimBuffer(sss);
        if(strlen(sss) >= value_size)
            return false;

        strcpy(value, sss);
        return true;
    }
    return false;
}

bool CSIPMessage::isRequest() const
{
    MYASSERT_RET_FALSE(m_MessageLen);
    return !!strncmp(m_sMessage, SIP_CHARS, 7);
}

bool CSIPMessage::__getStartLine(char* buffer, size_t size) const
{
    const char *last = m_sMessage;
    if(!__getNextHeader(buffer, size, &last)) //Skip start line
        return false;
    trimBuffer(buffer);
    return true;
}

int CSIPMessage::getStatusCode( void ) const
{
    if(isRequest())
        return -1;

    char sTmpMessage[300];
    if(!__getStartLine(sTmpMessage, sizeof(sTmpMessage)))
        return -1;

    // SIPS/2.0
    char* p = strchr(sTmpMessage, ' ');
    if(!p)
        return -1;

    *p++ = '\0';

    //проверяем
    if(strcmp(sTmpMessage, SIP_CHARS))
        return -1;

    return atoi(p);
}

bool CSIPMessage::getMessageType(char* buffer, size_t size) const
{
    if(!isRequest())
        return false;

    char sTmpMessage[300];
    if(!__getStartLine(sTmpMessage, sizeof(sTmpMessage)))
        return false;

    char* p = strchr(sTmpMessage, ' ');
    if(!p)
        return false;

    *p = '\0';

    if(strlen(sTmpMessage) >= size)
        return false;

    strcpy(buffer, sTmpMessage);

    return true;
}

bool CSIPMessage::getRequestURI(char* buffer, size_t size) const
{
    if(!isRequest())
        return false;

    char sTmpMessage[300];
    if(!__getStartLine(sTmpMessage, sizeof(sTmpMessage)))
        return false;

    char* end = strrchr(sTmpMessage, ' ');
    if(!end)
        return false;

    *end++ = '\0';

    if(strcmp(end, SIP_CHARS))
        return false;

    char* start = strchr(sTmpMessage, ' ');
    if(!start)
        return false;

    *start++ = '\0';

    trimBuffer(start);

    size_t len = strlen(start);
    if(len < 4)
        return false;

    //now only sip (not sips or tel)
    MYASSERT(!strncmp(start, "sip:", 4));

    if(len >= size)
        return false;

    strcpy(buffer, start);

    return true;
}

DWORD CSIPMessage::getCSeqNumber() const
{
    char str[300];
    if(!getHeaderValueByName(SIP_HEADER__CSEQ, str, sizeof(str)))
        return 0;

    return (DWORD)atoi(str);
}

bool CSIPMessage::getCSeqMetod(char* buffer, size_t size) const
{
    char str[300];
    if(!getHeaderValueByName(SIP_HEADER__CSEQ, str, sizeof(str)))
        return false;

    char* p = strrchr(str, ' ');

    if(strlen(++p) >= size)
        return false;

    strcpy(buffer, p);

    return true;
}

void CSIPMessage::trimBreakets( char* buffer )
{
    char *ch = strchr(buffer, '<');
    if(ch)
    {
        strcpy(buffer, ++ch);
        ch = strchr(buffer, '>');
        MYASSERT_RET(ch);
        *ch = '\0';
    }
}

bool CSIPMessage::getUriFromHeader(const char* sHeaderName, char* buffer, size_t size) const
{
    char str[300];
    if(!sHeaderName)
        return false;

    if(!getHeaderValueByName(sHeaderName, str, sizeof(str)))
        return false;

    trimBreakets(str);

    if(strlen(str) >= size)
        return false;
    strcpy(buffer, str);

    return true;
}

bool CSIPMessage::getUserHostPortFromHeader(const char* sHeaderName, char* user, size_t user_size, char* host, size_t host_size, WORD* port) const
{
    char str[300];
    if(sHeaderName)
    {
        if(!getHeaderValueByName(sHeaderName, str, sizeof(str)))
            return false;

        trimBreakets(str);
    }
    else
    {
        if(!getRequestURI(str, sizeof(str)))
            return false;
    }

    char* u = strstr(str, "sip:");
    if(!u)
        return false;
    u += 4;

    char* end;
    if((end = strchr(u, ' ')) != NULL)
        *end = '\0';
    else if((end = strchr(u, ';')) != NULL)
        *end = '\0';

    char* h = strchr(u, '@');
    if(!h)
        return false;

    *h++ = '\0';

    if(user)
    {
        if(strlen(u) >= user_size)
            return false;
        strcpy(user, u);
    }

    char* p = strchr(h, ':');
    if(!p)
    {
        if(host)
        {
            if(strlen(h) >= host_size)
                return false;
            strcpy(host, h);
        }

        return true;
    }

    *p++ = '\0';

    if(host)
    {
        if(strlen(h) >= host_size)
            return false;
        strcpy(host, h);
    }

    if(port)
        *port = (WORD)atoi(p);

    return true;
}

bool CSIPMessage::getDisplayName(const char* sHeaderName, char* name, size_t size) const
{
    bool b = true;
    if(!strcmp(sHeaderName, SIP_HEADER__FROM)) {}
    else if (!strcmp(sHeaderName, SIP_HEADER__TO)) {}
    else if (!strcmp(sHeaderName, SIP_HEADER__CONTACT)) {}
    else
        b = false;

    if(!b)
        return false;

    char str[300];
    if(!getHeaderValueByName(sHeaderName, str, sizeof(str)))
            return false;

    if(str[0] == '<')
        return false;

    char *ch = strchr(str, '<');
    if(!ch)
        return false;

    *ch-- = '\0';

    char *p = strchr(str, '\"');
    if(p)
    {
        strcpy(str, ++p);
        ch = strchr(str, '\"');
        if(!ch)
            return false;
        *ch = '\0';

        if(strlen(str) >= size)
            return false;

        strcpy(name, str);
        return true;
    }

    while(*ch == ' ')
        ch--;
    *(ch + 1) = '\0';

    if(strlen(str) >= size)
        return false;

    strcpy(name, str);
    return true;
}

bool CSIPMessage::getHostPortFromVia(const char* Via, char* host, size_t host_size, WORD* port)
{
    char *ch = strchr(Via, ':');
    if(!ch)
        return false;

    if(ch <= Via)
        return false;

    char *c = ch - 1;
    while(*c == ' ')
        c--;

    if(strncmp(Via, SIP_HEADER__VIA, (size_t)(c - Via)))
        return false;

    ch++;
    while(*ch == ' ')
        ch++;

    if(strncmp(ch, SIP_CHARS, 7))
        return false;

    ch += 7;
    MYASSERT(!strncmp(ch, "/UDP", 4)); //Now only UDP
    ch += 4;

    while(*ch == ' ')
        ch++;

    char *p = strchr(ch, ':');
    if(!p)
        return false;

    size_t len = (size_t)(p - ch);
    if(len >= host_size)
        return false;

    strncpy(host, ch, len);
    host[len] = '\0';

    *port = (WORD)atoi(++p);

    return true;
}

/* Specification.  */
#include <string.h>

#include <ctype.h>
#include <limits.h>

#define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))

int m_strcasecmp (const char *s1, const char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = TOLOWER (*p1);
      c2 = TOLOWER (*p2);

      if (c1 == '\0')
        break;

      ++p1;
      ++p2;
    }
  while (c1 == c2);

  if (UCHAR_MAX <= INT_MAX)
    return c1 - c2;
  else
    /* On machines where 'char' and 'int' are types of the same size, the
       difference of two 'unsigned char' values - including the sign bit -
       doesn't fit in an 'int'.  */
    return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}

int m_strncasecmp (const char *s1, const char *s2, size_t n)
{
  register const unsigned char *p1 = (const unsigned char *) s1;
  register const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2 || n == 0)
    return 0;

  do
    {
      c1 = TOLOWER (*p1);
      c2 = TOLOWER (*p2);

      if (--n == 0 || c1 == '\0')
        break;

      ++p1;
      ++p2;
    }
  while (c1 == c2);

  if (UCHAR_MAX <= INT_MAX)
    return c1 - c2;
  else
    /* On machines where 'char' and 'int' are types of the same size, the
       difference of two 'unsigned char' values - including the sign bit -
       doesn't fit in an 'int'.  */
    return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}
    typedef int (*TMyPrint)(const char *format, ...);

    static bool Pipe(int *filedes)
    {
        if (pipe(filedes) < 0)
            return false;
        return true;
    }
    #include <list>
    typedef std::list<char*> TPCharList;
    static void FreeListMem(TPCharList &lst)
    {
        TPCharList::iterator it;
        for(it = lst.begin(); it != lst.end(); it++)
            delete [] *it;
        lst.clear();
    }
    #define Sleep(X) usleep((X)*1000)
    bool ExecCommand(TMyPrint MyPrint, const char *command, const char *param, bool fWait)
    {
        char **p;
        size_t size = 0;
        TPCharList List;

        if(param)
        {
            char *start = (char *)param, *end = start;

            while (*end)
            {
                while(*end == ' ')
                    end++;

                if(*end == '\0')
                    break;

                start = end;

                while(*end && *end != ' ')
                    end++;

                size_t len = (size_t)(end - start);
                if(len)
                {
                    char *v = new char[len + 1];
                    if(!v)
                    {
                        if(MyPrint)
                            MyPrint("ExecCmd: out of memory");
                        FreeListMem(List);
                        return false;
                    }
                    memcpy(v, start, len);
                    v[len] = '\0';
                    List.push_back(v);
                }
            }
            size = List.size();
        }

        p = new char*[size + 2];
        if(!p)
        {
            if(MyPrint)
                MyPrint("ExecCmd: out of memory");
            FreeListMem(List);
            return false;
        }

        p[0] = (char *)command;
        if(size)
        {
            int i = 1;
            TPCharList::const_iterator it;
            for (it = List.begin(); it != List.end(); it++, i++)
                p[i] = (char *)*it;
        }
        p[size + 1] = NULL;

        int pid, filedes[2];

        if (!Pipe(filedes))
        {
            if(MyPrint)
                MyPrint("ExecCmd: Pipe error");
            FreeListMem(List);
            delete[] p;
            return false;
        }
        if (!(make_nonblock(filedes[0])
            && make_nonblock(filedes[1])))
        {
            if(MyPrint)
                MyPrint("ExecCmd: nonblock error");
            close(filedes[0]);
            close(filedes[1]);
            FreeListMem(List);
            delete[] p;
            return false;
        }

        int ok = 0;
        pid = fork();
        switch (pid)
        {
            case -1:
                if(MyPrint)
                    MyPrint("ExecCmd: Fork error");
                close(filedes[0]);
                close(filedes[1]);
                FreeListMem(List);
                delete[] p;
                return false;
            case 0:
            {
                dup2(filedes[1], 1);
                if (execv(command, p) < 0)
                    printf("Exec error\n");
                exit(EXIT_FAILURE);
            }
            default:
            {
                FreeListMem(List);
                delete[] p;

                if(fWait)
                    wait(&ok);
                else
                    Sleep(100);

                fd_set rset;

                FD_ZERO(&rset);       // Set Zero
                FD_SET(filedes[0], &rset);
                struct timeval tv;

                tv.tv_sec = 1;
                tv.tv_usec = 0;
                char str[1024];

                if ((select(filedes[0] + 1, &rset, NULL, NULL, &tv)) > 0)
                {
                    char *ch = str;
                    while (read(filedes[0], ch++, 1) == 1)
                    {
                        if (*(ch - 1) == '\n' || *(ch - 1) == '\r')
                        {
                            *(ch - 1) = 0;
                            if(MyPrint)
                                MyPrint("ExecCmd: %s\n", str);
                            ch = str;
                        }
                    }
                }
            }
        }
        close(filedes[0]);
        close(filedes[1]);
        if(ok)
            return false;
        return true;
    }

//============================== класс CSIPMessage ==============================

int main(int argc, char *argv[])
{
    //ExecCommand(printf, "/bin/ls", "-1 -a -l", true);
    //ExecCommand(printf, "/bin/ls", "", true);
    //ExecCommand(printf, "/bin/ls", NULL, true);

#if 0
    CSIPMessage m_GlMesReceived;
    m_GlMesReceived.setMessage
(
"INVITE sip:412@192.168.5.123 SIP/2.0\r\n\
To: 412 <sip:412@192.168.5.123>\r\n\
v: SIP/2.0/UDP 192.168.5.174:5060;branch=z9hG4bK264816501892718566;rport;delete\r\n\
From: pax <sip:pax@192.168.5.123>;tag=16575257\r\n\
Call-ID: 285991115711115-5590190954521@192.168.5.174\r\n\
CSeq: 1 INVITE\r\n\
Contact: <sip:pax@192.168.5.174:5060>\r\n\
Max-Forwards: 70\r\n\
Supported: replaces\r\n\
User-Agent: Voip Phone 1.0\r\n\
Allow: INVITE, ACK, OPTIONS, BYE, CANCEL, REFER, NOTIFY, INFO, SUBSCRIBE, PRACK, UPDATE\r\n\
Content-Type: application/sdp\r\n\
Content-Length: 314\r\n\
\r\n\
v=0\r\n\
o=pax 22647238 20978217 IN IP4 192.168.5.174\r\n\
s=A conversation\r\n\
c=IN IP4 192.168.5.174\r\n\
t=0 0\r\n\
m=audio 10006 RTP/AVP 8 4 18 0 9 101\r\n\
a=rtpmap:8 PCMA/8000\r\n\
a=rtpmap:4 G723/8000\r\n\
a=rtpmap:18 G729/8000\r\n\
a=rtpmap:0 PCMU/8000\r\n\
a=rtpmap:9 G722/16000\r\n\
a=rtpmap:101 telephone-event/8000\r\n\
a=fmtp:101 0-15\r\n\
a=sendrecv\r\n"
);
    char str[MAX_VIA_HEADER_STRING_LEN];
    str[0] = 0;
    MYASSERT(m_GlMesReceived.getHeaderByName(SIP_HEADER__VIA, str, sizeof(str)));
    if(m_GlMesReceived.retreiveParamFromHeader("rport", str))
    {
        strcat(str, ";received=");
        strcat(str, "192.168.5.123");
        strcat(str, ";rport=");
        strcat(str, "5060");
    }
    MYASSERT(m_GlMesReceived.replaseHeaderByName(SIP_HEADER__VIA, str));
#endif
    sigfillset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sig_SIGTERM_hndlr;
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

    sigemptyset(&sact.sa_mask);
    sact.sa_handler = sig_SIGPIPE_hndlr;
    sigaction(SIGPIPE, &sact, NULL);

    if (Parser.ParseCStringParams(argc, argv, false) < 0)
        return EXIT_FAILURE;

    if (Parser.Prepare() < 0)
        return EXIT_FAILURE;
//-------------------------------Only Rarius------------------------------------------
    if(Parser.fNoTarif)
    {
        Visualisation("No tarif");

        if (Parser.fDaemon)
            daemon(0,0);

        bool fCheckSMP = false;
        int nNoSMPCnt = 5;

        if(Parser.sScommIp[0] && Parser.nScommPort)
        {
            fCheckSMP = true;
            if(Parser.fRadAuthPrepay)
            {
                pthread_t Prepay_tid;
                if (pthread_create(&Prepay_tid, NULL, &Prepay_ptread, NULL) != 0 )
                    Loger("Can't create prepay tread!\n");
            }
            else
            {
                Loger("Set authorization prepay!\n");
                return EXIT_FAILURE;
            }
        }

        while (fMainRun)
        {
            if(fCheckSMP)
            {
                if(!SMP)
                {
                    if(nNoSMPCnt++ >= 5)
                    {
                        SMP = SMPCreate (Parser.sScommIp, Parser.nScommPort, 0);
                        nNoSMPCnt = 0;
                    }
                }
            }
            sleep(1);
        }
        if(SMP)
            SMP->OnClose("Exit program");
        usleep(500*1000);
        return EXIT_SUCCESS;
    }

    builder.SetCommonSettings(&Parser.SSettings);
    builder.SetJournalSettings(&Parser.Sett);
    builder.SetLocalNumbersSettings(Parser.lstLocalNumbers, &Parser.SPrefix);
    CDRBuilding::TPCharList lstStr;

    int dwTmp;
    DWORD dwLen;
    DWORD dwError;

//-------------------------------OnLine mode------------------------------------------
    if (Parser.sSpiderIp)
    {
        if (Parser.FillOnLineParams(Parser.sTfsFileNameBase) < 0)
            return EXIT_FAILURE;

        Visualisation("from spider online");

        for( int i = 0; i < MAX_CONNECT_POINT/2; i++ )
        {
            connect_point[i + MAX_CONNECT_POINT/2] = 0;
            if( (connect_point[i] = Create_server_point(Parser.ServerPort + i, i)) < 0)
            {
                Logerf("Error create server %s point!\n", GetConnectStr(i));
                connect_point[i] = 0;
            }
        }

        if (Parser.fDaemon)
            daemon(0,0);

        while ((Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0 && fMainRun)
            sleep(5);

        pthread_t Server_tid;
        if(connect_point[0] || connect_point[1] || connect_point[2])
        {
            if (pthread_create(&Server_tid, NULL, &Server_ptread, NULL) != 0 )
            {
                Loger("Can't create server tread!\n");
                return EXIT_FAILURE;
            }
        }
        else
        {
            Loger("No any server point!\n");
            return EXIT_FAILURE;
        }

        iReadBytes = 0;

        while (fMainRun)
        {
            //chtenije dlinni mesagi
            dwTmp = read(Spider_fd, &dwLen, sizeof(DWORD));
            if (dwTmp != sizeof(DWORD))
            {
                Loger("Spider socket closed!\n");
                close(Spider_fd);
                while ( fMainRun && (Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0 )
                    sleep(5);
                continue;
            }
            iReadBytes += sizeof(DWORD);

            if (!dwLen || dwLen > sizeof(CMonMessageEx) + 1)        // грубая проверка
            {
                    Loger( "Error: Message lengh is incorrect!\n");
                    close(Spider_fd);
                    while (fMainRun && (Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0 )
                        sleep(5);
                    continue;
            }

            //chtenije mesagi
            unsigned char buff[sizeof(CMonMessageEx) + 1];

            dwTmp = read(Spider_fd, &buff,dwLen);
            if (dwTmp != dwLen)
            {
                Loger("Spider socket error!\n");
                close(Spider_fd);
                while ( fMainRun && (Spider_fd = Login_ethernet(Parser.sSpiderIp, Parser.SpiderPort)) < 0 )
                    sleep(5);
                continue;
            }
            iReadBytes += dwLen;

            //dekodirovanie mesagi
            CMonMessageEx mes(0);
            BYTE *p;
            if (!(p = mes.decode(buff, dwLen - 1)))
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
        if(SMP)
            SMP->OnClose("Exit program");
        usleep(500*1000);
    }

//-------------------------------File mode------------------------------------------
    else if (!Parser.TfsFileList.empty())
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
                    return EXIT_FAILURE;
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
                    return EXIT_FAILURE;
            }

            off_t MainFilelen = 0;
            MainFilelen = lseek(fd_tfs, 0, SEEK_END);
            lseek(fd_tfs, 0, SEEK_SET);

            RemoveFiles();
            if(PutResiduaryCalls(Parser.sInDir, Parser.sInRm3FileName) < 0)
            {
                if(Parser.TfsFileList.size() > 1)
                    continue;
                else
                    return EXIT_FAILURE;
            }
            BYTE RdPersent = 0;
            iReadBytes = 0;

            while (fMainRun)
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
                    strcpy(sMessage, "Error: error while decoding message!\n");
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
    }
    else
        printf("No tfs files in directory!\n");

    return EXIT_SUCCESS;
}
