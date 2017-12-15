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
#ifndef CALLBUILDER_H
#define CALLBUILDER_H

//#define TCP_OPT
//#define FREE_BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "parser.h"

#include <CDRBuilder.h>
#include <Errors.h>
#include <time.h>


#ifndef CYGWIN
    #include <sys/wait.h>
#endif

#ifdef FREE_BSD
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <netinet/tcp_var.h>
#define TCP_KEEPIDLE TCPCTL_KEEPIDLE
#define TCP_KEEPINTVL TCPCTL_KEEPINTVL
#endif

#define MAX_CONNECT_POINT 6

#ifndef __FILE__
#error Undefined macro __FILE__
#endif

#ifndef __LINE__
#error Undefined macro __LINE__
#endif
#define WARNING printf("WARNING: file:%s line:%d\n",__FILE__,__LINE__);

#define RETURNW {WARNING ; return -1;};
#define RETURNERR(x) {Loger((x)); return -1;};
#define RETNOMEM {Loger(("Error: Out of memory!")); return -1;};

#define ROTATION_REALTIME 0
#define ROTATION_DAY 1
#define ROTATION_DECADE 2
#define ROTATION_MONTH 3
#define ROTATION_ONLINE 4
//fictions
#define ROTATION_FROM_ATS 5
#define ROTATION_FROM_SS 6

#include <config.h>

#define CRLF_CHARS                  "\r\n"
#define SIP_CHARS                   "SIP/2.0"

#define MAX_SIPSTARTLINE_LENGTH     200
#define MAX_NAME_LENGTH             50
#define MAX_VALUE_LENGTH            100

#define MAX_BRANCH_LENGTH           20
#define MAX_TAG_LENGTH              10
#define MAX_CALLID_LENGTH           20

//ответы SIP
#define SIP_ANSWER_100_TRYING                               100
#define SIP_ANSWER_100_TRYING_STR                           "Trying"
#define SIP_ANSWER_180_RINGING                              180
#define SIP_ANSWER_180_RINGING_STR                          "Ringing"
#define SIP_ANSWER_183_SESSION_PROGRESS                     183
#define SIP_ANSWER_183_SESSION_PROGRESS_STR                 "Session progress"
#define SIP_ANSWER_200_OK                                   200
#define SIP_ANSWER_200_OK_STR                               "OK"
#define SIP_ANSWER_300_MULTIPLE_CHOICES                     300
#define SIP_ANSWER_300_MULTIPLE_CHOICES_STR                 "Multiple Choices"
#define SIP_ANSWER_400_BAD_REQUEST                          400
#define SIP_ANSWER_400_BAD_REQUEST_STR                      "Bad request"
#define SIP_ANSWER_401_UNAUTHORIZED                         401
#define SIP_ANSWER_401_UNAUTHORIZED_STR                     "Unauthorized"
#define SIP_ANSWER_403_FORBIDDEN                            403
#define SIP_ANSWER_403_FORBIDDEN_STR                        "Forbidden"
#define SIP_ANSWER_405_METHOD_NOT_ALLOWED                   405
#define SIP_ANSWER_405_METHOD_NOT_ALLOWED_STR               "Method Not Allowed"
#define SIP_ANSWER_407_PROXY_AUDENTIFICATION_REQUIRED       407
#define SIP_ANSWER_407_PROXY_AUDENTIFICATION_REQUIRED_STR   "Proxy audentification required"
#define SIP_ANSWER_480_TEMPORARILY_UNAVAILABLE              480
#define SIP_ANSWER_480_TEMPORARILY_UNAVAILABLE_STR          "Temporarily Unavailable"
#define SIP_ANSWER_481_CALL_TRANSACTION_DOES_NOT_EXIST      481
#define SIP_ANSWER_481_CALL_TRANSACTION_DOES_NOT_EXIST_STR  "Call/Transaction Does Not Exist"
#define SIP_ANSWER_486_BUSY_HERE                            486
#define SIP_ANSWER_486_BUSY_HERE_STR                        "Busy here"
#define SIP_ANSWER_487_REQUEST_TERMINATED                   487
#define SIP_ANSWER_487_REQUEST_TERMINATED_STR               "Request terminated"
#define SIP_ANSWER_488_NOT_ACCEPTABLE_HERE                  488
#define SIP_ANSWER_488_NOT_ACCEPTABLE_HERE_STR              "Not Acceptable here"
#define SIP_ANSWER_500_INTERNAL_SERVER_ERROR                500
#define SIP_ANSWER_500_INTERNAL_SERVER_ERROR_STR            "Server Internal Error"
#define SIP_ANSWER_600_BUSY_EVERYWHERE                      600
#define SIP_ANSWER_600_BUSY_EVERYWHERE_STR                  "Busy Everywhere"

//сообщения SIP
#define SIP_MESSAGE_TYPE__INVITE                "INVITE"
#define SIP_MESSAGE_TYPE__REGISTER              "REGISTER"
#define SIP_MESSAGE_TYPE__ACK                   "ACK"
#define SIP_MESSAGE_TYPE__BYE                   "BYE"
#define SIP_MESSAGE_TYPE__CANCEL                "CANCEL"
#define SIP_MESSAGE_TYPE__OPTIONS               "OPTIONS"
#define SIP_MESSAGE_TYPE__INFO                  "INFO"

//заголовки SIP
#define SIP_HEADER__CONTENT_TYPE                "Content-Type"
#define SIP_HEADER__CONTENT_ENCODING            "Content-Encoding"
#define SIP_HEADER__FROM                        "From"
#define SIP_HEADER__CALL_ID                     "Call-ID"
#define SIP_HEADER__CONTACT                     "Contact"
#define SIP_HEADER__CONTENT_LENGTH              "Content-Length"
#define SIP_HEADER__SUBJECT                     "Subject"
#define SIP_HEADER__TO                          "To"
#define SIP_HEADER__VIA                         "Via"
#define SIP_HEADER__CSEQ                        "CSeq"
#define SIP_HEADER__EXPIRES                     "Expires"
#define SIP_HEADER__MAX_FORWARDS                "Max-Forwards"
#define SIP_HEADER__USER_AGENT                  "User-Agent"
#define SIP_HEADER__WWW_AUTHENTICATE            "WWW-Authenticate"
#define SIP_HEADER__AUTHORIZATION               "Authorization"
#define SIP_HEADER__PROXY_AUTHENTICATE          "Proxy-Authenticate"
#define SIP_HEADER__PROXY_AUTHORIZATION         "Proxy-Authorization"
#define SIP_HEADER__RECORD_ROUTE                "Record-Route"
#define SIP_HEADER__ROUTE                       "Route"
#define SIP_HEADER__SERVER                      "Server"
#define SIP_HEADER__ALLOW                       "Allow"
#define SIP_HEADER__ACCEPT                      "Accept"
#define SIP_HEADER__SUPPORTED                   "Supported"

#define SIP_HEADER_SHORT__CONTENT_TYPE          "c"
#define SIP_HEADER_SHORT__CONTENT_ENCODING      "e"
#define SIP_HEADER_SHORT__FROM                  "f"
#define SIP_HEADER_SHORT__CALL_ID               "i"
#define SIP_HEADER_SHORT__CONTACT               "m"
#define SIP_HEADER_SHORT__CONTENT_LENGTH        "l"
#define SIP_HEADER_SHORT__SUBJECT               "s"
#define SIP_HEADER_SHORT__TO                    "t"
#define SIP_HEADER_SHORT__VIA                   "v"

#define SIP_TR_TYPE_ICT                         1
#define SIP_TR_TYPE_NICT                        2
#define SIP_TR_TYPE_IST                         3
#define SIP_TR_TYPE_NIST                        4

#define SIP_DIALOG_TYPE_UAS                     1
#define SIP_DIALOG_TYPE_UAC                     2

//параметры заголовков
#define SIP_HEADER_PARAM__BRANCH                "branch"
#define SIP_HEADER_PARAM__RPORT                 "rport"
#define SIP_HEADER_PARAM__TAG                   "tag"
#define SIP_HEADER_PARAM__EXPIRES               "expires"

#define SIP_TR_SERV                             true
#define SIP_TR_CLNT                             false

#define MYASSERT(X) \
if(!(X)) \
{ \
    WARNING \
}

#define MYASSERT_RET_FALSE(X) \
if(!(X)) \
{ \
    WARNING \
    return false; \
}

#define MYASSERT_RET_NEG(X) \
if(!(X)) \
{ \
    WARNING \
    return -1; \
}

#define MYASSERT_RET_NULL(X) \
if(!(X)) \
{ \
    WARNING \
    return NULL; \
}

#define MYASSERT_RET(X) \
if(!(X)) \
{ \
    WARNING \
    return; \
}

#define MAX_VIA_HEADER_STRING_LEN               (3*128)

//============================== класс CSIPMessage ==============================
class CSIPMessage
{

public:

    CSIPMessage();
    CSIPMessage( const char* message );
    virtual ~CSIPMessage( void );

    //базовые
    void clearMessage( void );
    bool setMessage( const char* message );
    bool setMessage( const BYTE* data, size_t len );
    bool getMessage( char* buffer, size_t size ) const;
    size_t getMessageLen( void ) const { return m_MessageLen; }
    const char* getMessagePtr( void ) const { return m_sMessage; }
    const char* getMessageHeaderPtr( void ) const;
    const char* getMessageBodyPtr( void ) const;
    bool addString( const char* str );

    //стартовая строка
    bool isRequest( void ) const;
    bool getMessageType( char* buffer, size_t size ) const;
    bool getRequestURI( char* buffer, size_t size ) const;
    int  getStatusCode( void ) const;

    //заколовки и их параметры
    bool getNextHeader( char* sHeaderName, size_t header_name_size,  char* sHeader, size_t header_size, const char **last ) const;
    bool getHeaderByName( const char* sHeaderName, char* buffer, size_t size ) const;
    bool replaseHeaderByName( const char* sHeaderName, const char* sNewHeader );

    bool getHeaderValueByName( const char* sHeaderName, char* buffer, size_t size ) const;
    bool getNextHeaderValue( char* sHeaderName, size_t header_name_size, char* sValue, size_t value_size, const char **last ) const;

    bool findHeaderParamByName(const char* sHeaderName, const char* sParamName) const;
    bool getHeaderParamValueByName( const char* sHeaderName,  const char* sParamName, char* buffer, size_t size ) const;
    static bool retreiveParamFromHeader(const char* sParamName, const char* sHeader);

    bool getUriFromHeader(const char* sHeaderName, char* buffer, size_t size) const;
    bool getUserHostPortFromHeader( const char* sHeaderName, char* user, size_t user_size, char* host, size_t host_size, WORD* port ) const;
    bool getDisplayName( const char* sHeaderName, char* name, size_t size ) const;
    static bool getHostPortFromVia( const char* Via, char* host, size_t host_size, WORD* port );

    //тело сообщения
    bool getBodyParamByName( const char param, char* buffer, size_t size ) const;
    bool getBodyParamByName( const char *param, char* buffer, size_t size ) const;
    bool getNextBodyParam( char *param, char* value, size_t value_size, const char **last ) const;

    DWORD getCSeqNumber( void ) const;
    bool getCSeqMetod( char* buffer, size_t size ) const;

private:

    char m_sMessage[1500];
    size_t m_MessageLen;

    static void trimBuffer( char* buffer );
    static void trimBreakets( char* buffer );
    static const char* getShortHeaderName( const char* sFullHeaderName );
    bool __getNextHeader( char* buffer, size_t size, const char **last ) const;
    bool __getStartLine( char* buffer, size_t size ) const;
    static char* splitHeaderValueFromParams(const char* field );

};

#endif //CALLBUILDER_H
