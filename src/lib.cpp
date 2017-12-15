//#include  <include/sdl.h>
#include  <include/lib.h>

#ifdef  _ATS_
#include  <include/mem.h>
    #ifdef ARMLINUX
    #include <time.h>
    #include <stdio.h>
    #include <include/task.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <wait.h>

    #endif
#endif
//#include  <include/task.h>

#include  <string.h>

#ifdef _WIN_
#include  <windows.h>
#endif

int printdword( char *str, DWORD n, short basic )
{
    char tmp[20];
    int k=0, c, ret;

    if (basic <= 0)
        return 0;
    if (n == 0)
    {
        str[0] = '0';
        return 1;
    }
    while (n > 0)
    {
        tmp[k++] = (char)(n % basic + '0');
        n /= basic;
    }
    ret = 0;
    for (k--; k>=0; k--)
    {
        c = tmp[k];
        if (c > '9')
            c += 'A'-('0'+10);
        str[ret ++] = c;
    }
    if (basic == 16)
        str[ret ++] = 'H';
    return ret;
}

int printformat( char *str, const char *format, BYTE *ptr )
{
    int len = 0;

    while (*format)
    {
        if (*format == '%')
        {
            format ++;
            switch (*format)
            {
            case 'd':
                len += printdword(str + len, *(DWORD*)ptr, 10);
                ptr += sizeof(DWORD);
                break;
            case 'x':
                len += printdword(str + len, *(DWORD*)ptr, 16);
                ptr += sizeof(DWORD);
                break;
            case 's':
                {
                    char *src = *(char**)ptr;
                    while (*src)
                        *(str + (len ++)) = *(src ++);
                }
                ptr += sizeof(char*);
                break;
            case 'c':
                *(str + (len ++)) = *(char*)ptr;
                ptr += sizeof(int);//sizeof(char);
                break;
            case '%':
                *(str + (len ++)) = '%';
                break;
            case 't':
                WriteTime(str + len, **(TClock**)ptr);
                ptr += sizeof(DWORD);
                while (*(str + len))
                    len ++;
                break;
            case 'm':
                {
                    BYTE *arr = *(BYTE**)ptr;
                    ptr += sizeof(DWORD);

                    DWORD num = *(DWORD*)ptr;
                    ptr += sizeof(DWORD);

                    for (DWORD i = 0; i < num; i ++)
                    {
                        if (i)
                            *(str + (len ++)) = ' ';
                        len += printdword(str + len, (DWORD)arr[i], 16);
                    }
                }
                break;
            default:
                if (*format >= '0' && *format <= '9')
                    *(str + (len ++)) = *format - '0';
                else
                    *(str + (len ++)) = '?';
            }
        }
        else
            *(str + (len ++)) = *format;
        format ++;
    }
    *(str + len) = 0;
    return len;
}

int sprintformat( char *str, const char *format, ... )
{
    return printformat((char*)str, format, ((BYTE*)&format) + sizeof(char*));
}

void SkipSpaces( char *s, int *pos )
{
    while (s[*pos] == ' ')
        (*pos) ++;
}
void nextDigit(char *s, int &pos)
{
    while(s[pos]) {
        if (s[pos] >= '0' && s[pos] <= '9')
            return;
        pos++;
    }
}

int mmystrcpy( char *dst, char *src )
{
    int ret = 0;
    while (*src)
    {
        *(dst++) = *(src++);
        ret ++;
    }
    *dst = 0;
    return ret;
}

DWORD StrToInt(const char *s, int *pos )
{
    int tmp=0, flag=0;
    DWORD ret = 0;

    if (!pos)
        pos = &tmp;
    while (s[*pos] >= '0' && s[*pos] <= '9')
    {
        flag = 1;
        ret *= 10;
        ret += s[*pos] - '0';
        (*pos) ++;
    }
    if (!flag)
        *pos = 0;
    return ret;
}

DWORD StrToIntHex(const char *s, int *pos )
{
    int tmp=0, flag=0;
    DWORD ret = 0;

    if (!pos)
        pos = &tmp;
    while ((s[*pos] >= '0' && s[*pos] <= '9') ||
           (s[*pos] >= 'a' && s[*pos] <= 'f') ||
           (s[*pos] >= 'A' && s[*pos] <= 'F'))
    {
        char b = s[*pos];
        if (b >= '0' && b <= '9')
            b -= '0';
        else if (b >= 'a' && b <= 'f')
            b -= 'a'-10;
        else if (b >= 'A' && b <= 'F')
            b -= 'A'-10;
        flag = 1;
        ret *= 16;
        ret += b;
        (*pos) ++;
    }
    if (!flag)
        *pos = 0;
    return ret;
}

int IntToStr( char *s, DWORD num )
{
    char str[20];
    int k=0;

    if (num == 0)
    {
        *(s ++) = '0';
        *s = 0;
        return 1;
    }
    while (num > 0)
    {
        str[k++] = (char)(num % 10 + '0');
        num /= 10;
    }
    int ret = k;
    for (k--; k>=0; k--)
        *(s ++) = str[k];
    *s = 0;
    return ret;
}

int IntToStrHex( char *s, DWORD num )
{
    char str[20];
    int k=0;

    if (num == 0)
    {
        *(s ++) = '0';
        *s = 0;
        return 1;
    }
    while (num > 0)
    {
        int x = num % 16;
        if (x >= 10)
            str[k++] = (char)(x + ('A' - 10));
        else
            str[k++] = (char)(x + '0');
        num /= 16;
    }
    int ret = k;
    for (k--; k>=0; k--)
        *(s ++) = str[k];
    *s = 0;
    return ret;
}

#ifndef _ATS_
std::string intToStr(int num)
{
    char b[20];
    IntToStr(b, num);
    return b;
}

std::string getBufferAsString(void* buffer, int len)
{
    std::string result;
    for(int i = 0; i < len; i++) {

        char dt[] = "0123456789ABCDEF";
        unsigned char ch = ((unsigned char*)buffer)[i];;
        result += dt[ch >> 4];
        result += dt[ch & 0x0F];

        if (i != len-1) result += ' ';
    }

    return result;
}
#endif



BYTE code ( int num )
{
    return ((num / 10) << 4) + num % 10;
}

int decode ( BYTE text )
{
    return (text >> 4) * 10 + (text & 0xF);
}

void GetTime ( TClock& clk );

#ifdef _ATS_
void GetTime ( TClock& clk )
{
    #ifndef ARMLINUX
    clk.Control = (*(TClock *)MEM_FLASH_START_CLOCK).Control;
    clk.Year = decode((*(TClock *)MEM_FLASH_START_CLOCK).Year);
    clk.Month = decode((*(TClock *)MEM_FLASH_START_CLOCK).Month);
    clk.Date = decode((*(TClock *)MEM_FLASH_START_CLOCK).Date);
    clk.Hours = decode((*(TClock *)MEM_FLASH_START_CLOCK).Hours);
    clk.Minutes = decode((*(TClock *)MEM_FLASH_START_CLOCK).Minutes);
    clk.Seconds = decode((*(TClock *)MEM_FLASH_START_CLOCK).Seconds);
    #else
    time_t itime;
    tm T;
    time (&itime);
    localtime_r(&itime,&T);
    clk.Control = 0x01;
    clk.Year = T.tm_year - 100;
    clk.Month = T.tm_mon;
    clk.Date = T.tm_mday;
    clk.Hours = T.tm_hour;
    clk.Minutes = T.tm_min;
    clk.Seconds = T.tm_sec;
    #endif

}

void GetTimeStr ( char *s )
{
    TClock cl;

    GetTime(cl);
    WriteTime(s, cl);
}
#endif

#ifdef ARMLINUX
extern bool Pipe(int *filedes);
extern bool make_nonblock(int fd);
#endif

void SetTime ( TClock& clk )
{
#ifdef _ATS_
    #ifndef ARMLINUX
    (*(TClock *)MEM_FLASH_START_CLOCK).Control = 0x80;
    (*(TClock *)MEM_FLASH_START_CLOCK).Year = code(clk.Year);
    (*(TClock *)MEM_FLASH_START_CLOCK).Month = code(clk.Month);
    (*(TClock *)MEM_FLASH_START_CLOCK).Date = code(clk.Date);
    (*(TClock *)MEM_FLASH_START_CLOCK).Hours = code(clk.Hours);
    (*(TClock *)MEM_FLASH_START_CLOCK).Minutes = code(clk.Minutes);
    (*(TClock *)MEM_FLASH_START_CLOCK).Seconds = code(clk.Seconds);
    (*(TClock *)MEM_FLASH_START_CLOCK).Control = 0x01;
    #else
    int pid, filedes[2];
    char buf[100];

    sprintf(buf,"%.2d%.2d%.2d%.2d%.4d.%.2d", clk.Month, clk.Date, clk.Hours, clk.Minutes, clk.Year + 2000, clk.Seconds);
    if(Pipe(filedes) && make_nonblock(filedes[0]) && make_nonblock(filedes[1]))
    {
        pid = fork();
        switch(pid)
        {
           case -1:
               WARNING;
           break;
           case 0:
           {
              if(execl("/bin/date","/bin/date",buf, NULL) < 0)
                  WARNING;
              exit(0);
           }
              break;
        }
    }
    else
    {
        WARNING;
        return;
    }
    bool fhw = true;
    pid = fork();
    switch(pid)
    {
        case -1:
             WARNING;
        break;
        case 0:
        {
            if(execl("/sbin/hwclock", "/sbin/hwclock", "-w", NULL) < 0)
                fhw = false;
            exit(0);
        }
        break;
        default:
        {
            int status;
            wait(&status);
            if (!fhw)
                WARNING;
        }
    }

    #endif
#endif
}

void WriteTime ( char *str, const TClock& clk )
{
    int p = 0;

    p += mmystrcpy(str + p, "[");
    if (clk.Year < 10)
        p += mmystrcpy(str + p, "200");
    else
        p += mmystrcpy(str + p, "20");
    p += IntToStr(str + p, clk.Year);
    p += mmystrcpy(str + p, "-");

    if (clk.Month < 10)
        p += mmystrcpy(str + p, "0");
    p += IntToStr(str + p, clk.Month);
    p += mmystrcpy(str + p, "-");

    if (clk.Date < 10)
        p += mmystrcpy(str + p, "0");
    p += IntToStr(str + p, clk.Date);
    p += mmystrcpy(str + p, " ");

    if (clk.Hours < 10)
        p += mmystrcpy(str + p, "0");
    p += IntToStr(str + p, clk.Hours);
    p += mmystrcpy(str + p, ":");

    if (clk.Minutes < 10)
        p += mmystrcpy(str + p, "0");
    p += IntToStr(str + p, clk.Minutes);
    p += mmystrcpy(str + p, ":");

    if (clk.Seconds < 10)
        p += mmystrcpy(str + p, "0");
    p += IntToStr(str + p, clk.Seconds);
    p += mmystrcpy(str + p, "]");
}

bool ReadTime ( char *str, TClock& clk )
{
    int p = 0;

    SkipSpaces(str, &p);
    clk.Date = (BYTE)StrToInt(str, &p);
    if (!p)
        return false;
    SkipSpaces(str, &p);
    clk.Month = (BYTE)StrToInt(str, &p);
    if (!p)
        return false;
    SkipSpaces(str, &p);
    clk.Year = (BYTE)(StrToInt(str, &p) - 2000);
    if (!p)
        return false;
    SkipSpaces(str, &p);
    clk.Hours = (BYTE)StrToInt(str, &p);
    if (!p)
        return false;
    SkipSpaces(str, &p);
    clk.Minutes = (BYTE)StrToInt(str, &p);
    if (!p)
        return false;
    SkipSpaces(str, &p);
    clk.Seconds = (BYTE)StrToInt(str, &p);
    if (!p)
        return false;
    return true;
}

int getSetSize ( char** set )
{
    if (!set)
        return 0;
    int ret = 0;
    while (set[ret]) {
        ret ++;

        if (ret > 30) {
            return 0;
        }
    }
    return ret;
}


char* getSetElement ( char** set, int ind )
{
    if (!set)
        return NULL;
    int pos = 0;
    while (set[pos])
    {
        if (pos == ind)
        {
            if (*set[ind])
                return set[ind];
            else
                return NULL;
        }
        pos ++;
    }
    return NULL;
}

int findSetElement ( char** set, const char* str )
{
    if (!set)
        return -1;
    int pos = 0;
    while (set[pos])
    {
        if (*set[pos] && !strcmp(set[pos], str))
            return pos;
        pos ++;
    }
    return -1;
}

char convDTMFToChar ( char dig )
{
    if (dig >= 1 && dig <= 9)
        return '0' + dig;
    else if (dig == 10)
        return '0';
    else if (dig == 11)
        return '*';
    else if (dig == 12)
        return '#';
    else if (dig == 13)
        return '%';
    else if (dig == 14)
        return 'E';
    else if (dig == 15)
        return 'F';
    else
        return 0;
}

char convCharToDTMF ( char dig )
{
    if (dig >= '1' && dig <= '9')
        return dig - '0';
    else if (dig == '0')
        return 10;
    else if (dig == '*')
        return 11;
    else if (dig == '#')
        return 12;
    else if (dig == '%')
        return 13;
    else
        return 0;
}

char convMFRToChar ( char dig )
{
    if (dig >= 1 && dig <= 9)
        return '0' + dig;
    else if (dig >= 10 && dig <= 15)
        return 'A' + dig - 10;
    else
        return 0;
}

char convCharToMFR ( char dig )
{
    if (dig >= '1' && dig <= '9')
        return dig - '0';
    if (dig >= 'A' && dig <= 'F')
        return dig - 'A' + 10;
    else
        return 0;
}

void strins ( char* dst, char* src )
{
    if (!src || !dst)
        return;
    if (!*src)
        return;

    size_t ns = strlen(src);
    size_t nd = strlen(dst);
    memmove(dst+ns, dst, nd+1);
    memcpy(dst, src, ns);
}

// ************************

WORD _LRW ( WORD* X )
{
    if (!(((DWORD)X) & 1))
        return *X;
    WORD ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    return ret;
}

short _LRS ( short* X )
{
    if (!(((DWORD)X) & 1))
        return *X;
    short ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    return ret;
}

DWORD _LRD ( DWORD* X )
{
    if (!(((DWORD)X) & 3))
        return *X;
    DWORD ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    ((BYTE*)&ret)[2] = ((BYTE*)X)[2];
    ((BYTE*)&ret)[3] = ((BYTE*)X)[3];
    return ret;
}

int _LRI ( int* X )
{
    if (!(((DWORD)X) & 3))
        return *X;
    int ret;
    ((BYTE*)&ret)[0] = ((BYTE*)X)[0];
    ((BYTE*)&ret)[1] = ((BYTE*)X)[1];
    ((BYTE*)&ret)[2] = ((BYTE*)X)[2];
    ((BYTE*)&ret)[3] = ((BYTE*)X)[3];
    return ret;
}


// ************************

void _LXW ( WORD* X, WORD Y )
{
    if (!(((DWORD)X) & 1))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
}

void _LXS ( short* X, short Y )
{
    if (!(((DWORD)X) & 1))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
}

void _LXD ( DWORD* X, DWORD Y )
{
    if (!(((DWORD)X) & 3))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
    ((BYTE*)X)[2] = ((BYTE*)&Y)[2];
    ((BYTE*)X)[3] = ((BYTE*)&Y)[3];
}

void _LXI ( int* X, int Y )
{
    if (!(((DWORD)X) & 3))
    {
        *X = Y;
        return;
    }
    ((BYTE*)X)[0] = ((BYTE*)&Y)[0];
    ((BYTE*)X)[1] = ((BYTE*)&Y)[1];
    ((BYTE*)X)[2] = ((BYTE*)&Y)[2];
    ((BYTE*)X)[3] = ((BYTE*)&Y)[3];
}
