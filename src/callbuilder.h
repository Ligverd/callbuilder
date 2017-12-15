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

#endif //CALLBUILDER_H
