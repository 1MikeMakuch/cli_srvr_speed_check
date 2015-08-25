#ifndef __CLI_SRVR_SPEED_CHECK_H__
#define __CLI_SRVR_SPEED_CHECK_H__

#define MAXDATASIZE 1000000
       
                    
#define logit(_a_) \
 if (debugging) { \
     printf("%s\n", _a_) ; \
 }                           

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#include <afxsock.h>
#include "getopt.h"
#endif

#ifdef WIN32

#define MYTIMET time_t
#define MYTIME(_a_) time(_a_)
#define MYULONG u_long
#define socklen_t int

#else

#define MYTIMET struct timeval
#define closesocket close
#define MYTIME(_a_) gettimeofday(_a_, NULL)
#define MYULONG uint32_t

#endif

#endif
