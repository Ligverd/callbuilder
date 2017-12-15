#ifndef _LIB_H_
#define _LIB_H_

#include <type.h>

int printdword( char *str, DWORD n, short basic );
int printformat( char *str, const char *format, BYTE *ptr );
int sprintformat( char *str, const char *format, ... );

DWORD StrToInt( const char *s, int *pos=0 );
DWORD StrToIntHex( const char *s, int *pos=0 );
int IntToStr( char *s, DWORD num );
int IntToStrHex( char *s, DWORD num );
void SkipSpaces( char *s, int *pos );
void nextDigit(char *s, int &pos);


#include  <string>
std::string intToStr(int num);
std::string getBufferAsString(void* buffer, int len);


// На время везде выравнивание 1 байт
#pragma pack (push, 1)
struct TClock
{
    BYTE
      Control,
      Seconds,
      Minutes,
      Hours,
      Day,
      Date,
      Month,
      Year;
};
#pragma pack (pop)

void GetTime ( TClock& clock );
void SetTime ( TClock& clock );
void WriteTime ( char *str, const TClock& clock );
bool ReadTime ( char *str, TClock& clock );
void GetTimeStr( char* s );

int   getSetSize ( char** set );
char* getSetElement ( char** set, int ind );
int   findSetElement ( char** set, const char* str );

char convDTMFToChar ( char dig );
char convCharToDTMF ( char dig );
char convMFRToChar ( char dig );
char convCharToMFR ( char dig );

void strins ( char* dst, char* src );

#endif
