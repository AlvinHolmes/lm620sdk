

/***************************************************************************/
/*                                     */
/* ftplib.c - callable ftp access routines                 */
/* Copyright (C) 1996-2001, 2013, 2016 Thomas Pfau, tfpfau@gmail.com       */
/*  1407 Thomas Ave, North Brunswick, NJ, 08902            */
/*                                     */
/* This library is free software.  You can redistribute it and/or      */
/* modify it under the terms of the Artistic License 2.0.          */
/*                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/* Artistic License 2.0 for more details.                  */
/*                                     */
/* See the file LICENSE or                         */
/* http://www.perlfoundation.org/artistic_license_2_0              */
/*                                     */
/***************************************************************************/
#if defined(__unix__) || defined(__VMS)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if defined(__unix__)
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(VMS)
#include <types.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#elif defined(_WIN32)
#include <winsock.h>
#elif defined(FTP_RTOS)
#include <unistd.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <os_config.h>
#include <slog_print.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#endif


#if defined(__APPLE__)
#undef _REENTRANT
#endif

#define BUILDING_LIBRARY
#include "ftplib.h"

#if defined(__UINT64_MAX)       && !defined(PRIu64)

#if ULONG_MAX                   == __UINT32_MAX
#define PRIu64                  "llu"

#else

#define PRIu64                  "lu"
#endif

#endif

#if defined(_WIN32)
#define SETSOCKOPT_OPTVAL_TYPE  (const char *)

#else

#define SETSOCKOPT_OPTVAL_TYPE  (void *)
#endif


#if !defined(FTP_RTOS)
#define FTPLOG_E(fmt, ...)                                              \
    do {                                                                \
        if (ftplib_debug) fprintf(stderr, fmt "\r\n", ##__VA_ARGS__);   \
    } while(0)

#define FTPLOG_W(fmt, ...)                                              \
    do {                                                                \
        if (ftplib_debug) fprintf(stderr, fmt "\r\n", ##__VA_ARGS__);   \
    } while(0)

#define FTPLOG_I(fmt, ...)                                              \
    do {                                                                \
        if (ftplib_debug) printf(fmt "\r\n", ##__VA_ARGS__);            \
    } while(0)

#define FTPLOG_D(fmt, ...)                                              \
    do {                                                                \
        if (ftplib_debug) printf(fmt "\r\n", ##__VA_ARGS__);            \
    } while(0)

#define FTPLOG_V(fmt, ...)                                              \
    do {                                                                \
        if (ftplib_debug > 1) printf(fmt "\r\n", ##__VA_ARGS__);        \
    } while(0)

#define FTPLIB_BUFSIZ           8192
#define RESPONSE_BUFSIZ         1024
#define TMP_BUFSIZ              1024
#define ACCEPT_TIMEOUT          30

#else

#ifdef USING_EMULATOR
#define FTPLOG_E(fmt, ...)  osPrintf(fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_W(fmt, ...)  osPrintf(fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_I(fmt, ...)  osPrintf(fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_D(fmt, ...)  osPrintf(fmt "\r\n", ##__VA_ARGS__)
#else
#define FTPLOG_E(fmt, ...)  slogPrintf(SLOG_LEVEL_ERROR, SLOG_PRINT_SUBMDL_MID_NET, fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_W(fmt, ...)  slogPrintf(SLOG_LEVEL_WARN,  SLOG_PRINT_SUBMDL_MID_NET, fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_I(fmt, ...)  slogPrintf(SLOG_LEVEL_INFO,  SLOG_PRINT_SUBMDL_MID_NET, fmt "\r\n", ##__VA_ARGS__)
#define FTPLOG_D(fmt, ...)  slogPrintf(SLOG_LEVEL_DEBUG, SLOG_PRINT_SUBMDL_MID_NET, fmt "\r\n", ##__VA_ARGS__)
#endif

#define FTP_BIND_DEVICE 1 // 绑定公网netif后，不可以跟PC对接
#if 0
#define FTPLOG_V(fmt, ...)  FTPLOG_D(fmt, ##__VA_ARGS__)
#else
#define FTPLOG_V(fmt, ...)
#endif

#define FTPLIB_BUFSIZ           (1024)
#define RESPONSE_BUFSIZ         (512)
#define TMP_BUFSIZ              (256 + 32)
#define ACCEPT_TIMEOUT          (30)

#endif  //!defined(FTP_RTOS)

#define FTPLIB_CONTROL          0
#define FTPLIB_READ             1
#define FTPLIB_WRITE            2

#if !defined(FTPLIB_DEFMODE)
#define FTPLIB_DEFMODE          FTPLIB_PASSIVE
#endif

#define ISDIGIT(x)  ((x)>='0'&&(x)<='9')

struct NetBuf {
    char  *cput,   *cget;
    int             handle;
    int cavail,     cleft;
    char *          buf;
    int             dir;
    netftp          *ctrl;
    netftp          *data;
    int             cmode;
    struct timeval  idletime;
    FtpCallback     idlecb;
    void *          idlearg;
    unsigned long   xfered;
    unsigned long   cbbytes;
    unsigned long   xfered1;
    char            response[RESPONSE_BUFSIZ];
    unsigned char ftptype;
    void * ioctx;
    FtpNetWrite netwrite;
    FtpNetRead netread;
};

#if !defined(FTP_RTOS)
static char * version = "ftplib Release 4.0 07-Jun-2013, copyright 1996-2003, 2013 Thomas Pfau";
GLOBALDEF int ftplib_debug = 0;
#endif

#if defined(__unix__) || defined(VMS)
int net_read(int fd, char * buf, size_t len)
{
    while (1) {
        int c = read(fd, buf, len);
        if (c == -1) {
            if (errno != EINTR && errno != EAGAIN)
                return - 1;
        }
        else {
            return c;
        }
    }
}

int net_write(int fd, const char * buf, size_t len)
{
    int done = 0;

    while (len > 0) {
        int c = write(fd, buf, len);
        if (c == -1) {
            if (errno != EINTR && errno != EAGAIN)
                return - 1;
        }
        else if (c == 0) {
            return done;
        }
        else {
            buf  += c;
            done += c;
            len  -= c;
        }
    }
    return done;
}

#define net_close               close
#elif defined(_WIN32)
#define net_read(x, y, z)       recv(x,y,z,0)
#define net_write(x, y, z)      send(x,y,z,0)
#define net_close               closesocket
#elif defined(FTP_RTOS)
static int net_read(int fd, char * buf, size_t len)
{
    while (1) {
        int c = recv(fd, buf, len, 0);
        if (c == -1) {
            int err = errno;
            if ((EINTR != err) && (EAGAIN != err) && (EWOULDBLOCK != err) &&
                (EPROTOTYPE != err) && (EALREADY != err) && (EINPROGRESS != err)) {
                return -1;
            }
        }
        else {
            return c;
        }
    }
}


static int net_write(int fd, const char * buf, size_t len)
{
    int done = 0;
    while (len > 0) {
        int c = send(fd, buf, len, 0);
        if (c == -1) {
            if (errno != EINTR && errno != EAGAIN)
                return - 1;
        }
        else if (c == 0) {
            return done;
        }
        else {
            buf  += c;
            done += c;
            len  -= c;
        }
    }
    return done;
}

static int Ftp_net_read(netftp *nHandler, char * buf, size_t len)
{
    if(nHandler->netread == NULL)
    {
        return net_read(nHandler->handle, buf, len);
    }
    else
    {
        return nHandler->netread(nHandler->ioctx, buf, len);
    }
}

static int Ftp_net_write(netftp *nHandler, const char * buf, size_t len)
{
    if(nHandler->netwrite == NULL)
    {
        return net_write(nHandler->handle, buf, len);
    }
    else
    {
        return nHandler->netwrite(nHandler->ioctx, buf, len);
    }
}


#define net_close               lwip_close
#endif

#if defined(NEED_MEMCCPY)

/*
 * VAX C does not supply a memccpy routine so I provide my own
 */
void * memccpy(void * dest, const void * src, int c, size_t n)
{
    int             i   = 0;
    const unsigned char * ip = src;
    unsigned char * op  = dest;

    while (i < n) {
        if ((*op++ = *ip++) == c)
            break;

        i++;
    }

    if (i == n)
        return NULL;

    return op;
}


#endif

#if defined(NEED_STRDUP)

/*
 * strdup - return a malloc'ed copy of a string
 */
char * strdup(const char * src)
{
    int    l   = strlen(src) + 1;
    char * dst = malloc(l);

    if (dst)
        strcpy(dst, src);

    return dst;
}


#endif


/*
 * socket_wait - wait for socket to receive or flush data
 *
 * return 1 if no user callback, otherwise, return value returned by
 * user callback
 */
static int socket_wait(netftp * ctl)
{
    fd_set fd, *rfd = NULL, *wfd = NULL;

    struct timeval tv;
    int           rv  = 0;

    if ((ctl->dir == FTPLIB_CONTROL) || (ctl->idlecb == NULL))
        return 1;

    if (ctl->dir == FTPLIB_WRITE)
        wfd = &fd;
    else
        rfd = &fd;

    FD_ZERO(&fd);

    do {
        FD_SET(ctl->handle, &fd);
        tv = ctl->idletime;
        rv = select(ctl->handle + 1, rfd, wfd, NULL, &tv);
        if (rv == -1) {
            rv = 0;
            strncpy(ctl->ctrl->response, strerror(errno), sizeof(ctl->ctrl->response));
            break;
        }
        else if (rv > 0) {
            rv = 1;
            break;
        }
    } while((rv = ctl->idlecb(ctl, ctl->xfered, ctl->idlearg)));

    return rv;
}



/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
static int readline(char *buf, int max, netftp *ctl)
{
    int  x, retval = 0;
    char *end, *bp = buf;
    int  eof = 0;

    if ((ctl->dir != FTPLIB_CONTROL) && (ctl->dir != FTPLIB_READ))
        return - 1;

    if (max == 0)
        return 0;

    do {
        if (ctl->cavail > 0) {
            x   = (max >= ctl->cavail) ? ctl->cavail: max - 1;
            end = memccpy(bp, ctl->cget, '\n', x);

            if (end != NULL)
                x = end - bp;

            retval      += x;
            bp          += x;
            *bp         = '\0';
            max         -= x;
            ctl->cget   += x;
            ctl->cavail -= x;

            if (end != NULL) {
                bp -= 2;
                if (strcmp(bp, "\r\n") == 0) {
                    *bp++ = '\n';
                    *bp++ = '\0';
                    --retval;
                }
                break;
            }
        }

        if (max == 1) {
            *buf = '\0';
            break;
        }

        if (ctl->cput == ctl->cget) {
            ctl->cput   = ctl->cget = ctl->buf;
            ctl->cavail = 0;
            ctl->cleft  = FTPLIB_BUFSIZ;
        }

        if (eof) {
            if (retval == 0)
                retval = -1;
            break;
        }

        if (!socket_wait(ctl))
            return retval;

        if ((x = Ftp_net_read(ctl, ctl->cput, ctl->cleft)) == -1) {
            FTPLOG_E("ftplib readline read");
            retval = -1;
            break;
        }

        if (x == 0)
            eof = 1;

        ctl->cleft  -= x;
        ctl->cavail += x;
        ctl->cput   += x;
    }while(1);

    return retval;
}



/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
static int writeline(const char *buf, int len, netftp *nData)
{
    int         x, nb = 0, w;
    char        *nbp;
    char        lc  = 0;
    const char  *ubp = buf;

    if (nData->dir != FTPLIB_WRITE)
        return - 1;

    nbp = nData->buf;

    for (x = 0; x < len; x++) {
        if ((*ubp == '\n') && (lc != '\r')) {
            if (nb == FTPLIB_BUFSIZ) {
                if (!socket_wait(nData))
                    return x;

                w = Ftp_net_write(nData, nbp, FTPLIB_BUFSIZ);
                if (w != FTPLIB_BUFSIZ) {
                    FTPLOG_D("ftplib writeline net_write(1) returned %d, errno = %d", w, errno);
                    return (-1);
                }
                nb = 0;
            }
            nbp[nb++] = '\r';
        }

        if (nb == FTPLIB_BUFSIZ) {
            if (!socket_wait(nData))
                return x;

            w = Ftp_net_write(nData, nbp, FTPLIB_BUFSIZ);
            if (w != FTPLIB_BUFSIZ) {
                FTPLOG_D("ftplib writeline net_write(2) returned %d, errno = %d", w, errno);
                return (-1);
            }
            nb = 0;
        }
        nbp[nb++] = lc = *ubp++;
    }

    if (nb) {
        if (!socket_wait(nData))
            return x;

        w = Ftp_net_write(nData, nbp, nb);
        if (w != nb) {
            FTPLOG_D("ftplib writeline net_write(3) returned %d, errno = %d", w, errno);
            return (-1);
        }
    }

    return len;
}



/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
static int readresp(char c, netftp *nControl)
{
    char match[5];

    if (readline(nControl->response, RESPONSE_BUFSIZ, nControl) == -1) {
        FTPLOG_E("ftplib readresp Control socket read failed");
        return 0;
    }

    FTPLOG_V("resp0: %s", nControl->response);

    if (nControl->response[3] == '-') {
        strncpy(match, nControl->response, 3);
        match[3] = ' ';
        match[4] = '\0';

        do {
            if (readline(nControl->response, RESPONSE_BUFSIZ, nControl) == -1) {
                FTPLOG_E("ftplib readresp Control socket read failed");
                return 0;
            }
            FTPLOG_V("resp1: %s", nControl->response);
        } while(strncmp(nControl->response, match, 4));
    }

    if (nControl->response[0] == c)
        return 1;

    return 0;
}



/*
 * FtpInit for stupid operating systems that require it (Windows NT)
 */
GLOBALDEF void FtpInit(void)
{
#if defined(_WIN32)
    WORD            wVersionRequested;
    WSADATA         wsadata;
    int             err;

    wVersionRequested   = MAKEWORD(1, 1);

    if ((err = WSAStartup(wVersionRequested, &wsadata)) != 0)
        fprintf(stderr, "Network failed to start: %d\n", err);

#endif
}

/*
 * FtpLastResponse - return a pointer to the last response received
 */
GLOBALDEF char* FtpLastResponse(netftp *nControl)
{
    if ((nControl) && (nControl->dir == FTPLIB_CONTROL))
        return nControl->response;

    return NULL;
}

#if defined(FTP_RTOS)

/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
GLOBALDEF int FtpConnect(const char *host, netftp **nControl)
{
    int                 sControl;
    struct sockaddr_in  sin;
    int                 on = 1;
    netftp              *ctrl;
    char                *lhost;
    char                *pnum;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    lhost = strdup(host);
    if (lhost == NULL)
        return 0;

    pnum = strchr(lhost, ':');
    if (pnum != NULL) {
        int port = atoi(pnum+1);
        if (port == 0)
            port = 21;
        sin.sin_port = htons(port);
        *pnum = '\0';
    } else {
        sin.sin_port = htons(21);
    }

    if ((sin.sin_addr.s_addr = inet_addr(lhost)) == INADDR_NONE) {
        int ret;
        struct addrinfo hints;
        struct addrinfo *result = NULL, *rp = NULL;

        memset(&hints, 0x00, sizeof(hints));
        //hints.ai_family = AF_UNSPEC;
        hints.ai_family = AF_INET; // 当前只支持IPv4
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        ret = getaddrinfo(lhost, 0, &hints, &result);
        if (ret == 0) {
            for(rp = result; rp != NULL; rp = rp->ai_next) {
                if (rp->ai_family == AF_INET) {
                    struct sockaddr_in *addr = (struct sockaddr_in *)rp->ai_addr;
                    sin.sin_addr.s_addr = addr->sin_addr.s_addr;
                    break;
                }
            }
            freeaddrinfo(result);
        }
        if (rp == NULL) {
            free(lhost);
            return 0;
        }

    }

    free(lhost);

    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1) {
        FTPLOG_E("ftplib FtpConnect socket fail");
        return 0;
    }

    if (setsockopt(sControl, SOL_SOCKET, SO_REUSEADDR,
        SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect setsockopt fail");
        return 0;
    }

    if (connect(sControl, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect connect fail");
        return 0;
    }

    ctrl = calloc(1, sizeof(netftp));
    if (ctrl == NULL) {
        net_close(sControl);
        return 0;
    }

    ctrl->buf = malloc(FTPLIB_BUFSIZ);
    if (ctrl->buf == NULL) {
        net_close(sControl);
        free(ctrl);
        return 0;
    }

    ctrl->handle        = sControl;
    ctrl->dir           = FTPLIB_CONTROL;
    ctrl->ctrl          = NULL;
    ctrl->data          = NULL;
    ctrl->cmode         = FTPLIB_DEFMODE;
    ctrl->idlecb        = NULL;
    ctrl->idletime.tv_sec = ctrl->idletime.tv_usec = 0;
    ctrl->idlearg       = NULL;
    ctrl->xfered        = 0;
    ctrl->xfered1       = 0;
    ctrl->cbbytes       = 0;

    if (readresp('2', ctrl) == 0) {
        net_close(sControl);
        free(ctrl->buf);
        free(ctrl);
        return 0;
    }

    *nControl = ctrl;
    return 1;
}

/*
 * FtpConnect_pro - connect to remote server
 *
 * 隐式FTPs时不等待服务器的220响应，且默认端口是990
 * 支持IPv4和IPv6
 * 支持FTPs
 *
 * return 1 if connected, 0 if not
 */
GLOBALDEF int FtpConnect_Pro(unsigned char ftptype, const char *host, int port, struct ifreq *ifname, netftp **nControl)
{
    int                 sControl;
    int                 connret;
    ip_addr_t hostIp    = {0};
    int                 on = 1;
    netftp              *ctrl;

    if (port == 0)
    {
        if(ftptype == FTPLIB_TYPE_FTPS_IMPLICIT)
            port = 990;
        else
            port = 21;
    }

    if (ipaddr_aton(host, &hostIp) == 0) {
        int ret;
        struct addrinfo hints;
        struct addrinfo *result = NULL;

        memset(&hints, 0x00, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        ret = getaddrinfo(host, 0, &hints, &result);
        if (ret == 0 && result != NULL) {
            if(result->ai_family == AF_INET)
            {
                struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
                IP_SET_TYPE_VAL(hostIp, IPADDR_TYPE_V4);
                inet_addr_to_ip4addr(ip_2_ip4(&hostIp), &(addr->sin_addr));
            }
            else
            {
                struct sockaddr_in6 *addr = (struct sockaddr_in6 *)result->ai_addr;
                IP_SET_TYPE_VAL(hostIp, IPADDR_TYPE_V6);
                inet6_addr_to_ip6addr(ip_2_ip6(&hostIp), &(addr->sin6_addr));
            }

            freeaddrinfo(result);
        }
        else {
            FTPLOG_E("ftplib FtpConnect Pro getaddrinfo fail");
            return 0;
        }

    }

    if (IP_IS_V4(&hostIp)) /* IPV4 */
        sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    else
        sControl = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (sControl == -1) {
        FTPLOG_E("ftplib FtpConnect Pro socket fail");
        return 0;
    }

#if FTP_BIND_DEVICE
    if(ifname != NULL)
    {
        if (setsockopt(sControl, SOL_SOCKET, SO_BINDTODEVICE,
            SETSOCKOPT_OPTVAL_TYPE ifname, sizeof(*ifname)) == -1) {
            net_close(sControl);
            FTPLOG_E("ftplib FtpConnect Pro setsockopt SO_BINDTODEVICE fail");
            return 0;
        }
    }
#endif

    if (setsockopt(sControl, SOL_SOCKET, SO_REUSEADDR,
        SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect Pro setsockopt SO_REUSEADDR fail");
        return 0;
    }

    if (IP_IS_V4(&hostIp)) /* IPV4 */
    {
        struct sockaddr_in remoteSockAddr;
        memset(&remoteSockAddr, 0, sizeof(struct sockaddr_in));

        remoteSockAddr.sin_len = sizeof(remoteSockAddr);
        remoteSockAddr.sin_family = AF_INET;
        inet_addr_from_ip4addr(&(remoteSockAddr.sin_addr), ip_2_ip4(&(hostIp)));
        remoteSockAddr.sin_port = htons(port);

        connret = lwip_connect(sControl, (struct sockaddr *)&remoteSockAddr, sizeof(struct sockaddr));
    }
    else /* IPV6 */
    {
        struct sockaddr_in6 remoteSockAddr6;
        memset(&remoteSockAddr6, 0, sizeof(struct sockaddr_in6));

        remoteSockAddr6.sin6_len = sizeof(remoteSockAddr6);
        remoteSockAddr6.sin6_family = AF_INET6;
        inet6_addr_from_ip6addr(&(remoteSockAddr6.sin6_addr), ip_2_ip6(&(hostIp)));
        remoteSockAddr6.sin6_port = htons(port);

        connret = lwip_connect(sControl, (struct sockaddr *)&remoteSockAddr6, sizeof(struct sockaddr));
    }

    if (connret == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect Pro connect fail");
        return 0;
    }

    ctrl = calloc(1, sizeof(netftp));
    if (ctrl == NULL) {
        net_close(sControl);
        return 0;
    }

    ctrl->buf = malloc(FTPLIB_BUFSIZ);
    if (ctrl->buf == NULL) {
        net_close(sControl);
        free(ctrl);
        return 0;
    }

    ctrl->handle        = sControl;
    ctrl->dir           = FTPLIB_CONTROL;
    ctrl->ctrl          = NULL;
    ctrl->data          = NULL;
    ctrl->cmode         = FTPLIB_DEFMODE;
    ctrl->idlecb        = NULL;
    ctrl->idletime.tv_sec = ctrl->idletime.tv_usec = 0;
    ctrl->idlearg       = NULL;
    ctrl->xfered        = 0;
    ctrl->xfered1       = 0;
    ctrl->cbbytes       = 0;

    if (ftptype != FTPLIB_TYPE_FTPS_IMPLICIT) //隐式FTPs要在SSL后才能读到220的响应
    {
        if (readresp('2', ctrl) == 0) {
            net_close(sControl);
            free(ctrl->buf);
            free(ctrl);
            return 0;
        }
    }

    *nControl = ctrl;
    return 1;
}


#else

/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
GLOBALDEF int FtpConnect(const char *host, netftp **nControl)
{
    int                 sControl;
    struct sockaddr_in  sin;
    int                 on  = 1;
    netftp              *ctrl;
    char               *lhost;
    char               *pnum;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    lhost = strdup(host);
    if (lhost == NULL)
        return 0;

    pnum = strchr(lhost, ':');
    if (pnum == NULL)
        pnum = "ftp";
    else
        *pnum++ = '\0';

    if (isdigit(*pnum))
        sin.sin_port = htons(atoi(pnum));
    else {
        struct servent * pse;

#if _REENTRANT
        struct servent  se;
        char            tmpbuf[TMP_BUFSIZ];
        int             i;

        if ((i = getservbyname_r(pnum, "tcp", &se, tmpbuf, TMP_BUFSIZ, &pse)) != 0) {
            errno = i;
            FTPLOG_E("ftplib FtpConnect getservbyname_r fail");
            free(lhost);
            return 0;
        }

#else

        if ((pse = getservbyname(pnum, "tcp")) == NULL) {
            FTPLOG_E("ftplib FtpConnect getservbyname fail");
            free(lhost);
            return 0;
        }

#endif
        sin.sin_port = pse->s_port;
    }

    if ((sin.sin_addr.s_addr = inet_addr(lhost)) == INADDR_NONE) {
        struct hostent * phe;

#ifdef _REENTRANT
        struct hostent he;
        char            tmpbuf[TMP_BUFSIZ];
        int             i, herr;

        if (((i = gethostbyname_r(lhost, &he, tmpbuf, TMP_BUFSIZ, &phe, &herr)) != 0) || (phe == NULL)) {
            FTPLOG_E("ftplib FtpConnect gethostbyname: %s\n", hstrerror(herr));
            free(lhost);
            return 0;
        }

#else

        if ((phe = gethostbyname(lhost)) == NULL) {
            FTPLOG_E("ftplib FtpConnect gethostbyname: %s\n", hstrerror(h_errno));
            free(lhost);
            return 0;
        }

#endif

        memcpy((char *) &sin.sin_addr, phe->h_addr, phe->h_length);
    }

    free(lhost);

    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1) {
        FTPLOG_E("ftplib FtpConnect socket fail");
        return 0;
    }

    if (setsockopt(sControl, SOL_SOCKET, SO_REUSEADDR,
        SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect setsockopt SO_REUSEADDR fail");
        return 0;
    }

    if (connect(sControl, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        net_close(sControl);
        FTPLOG_E("ftplib FtpConnect connect fail");
        return 0;
    }

    ctrl = calloc(1, sizeof(netftp));
    if (ctrl == NULL) {
        net_close(sControl);
        return 0;
    }

    ctrl->buf = malloc(FTPLIB_BUFSIZ);
    if (ctrl->buf == NULL) {
        net_close(sControl);
        free(ctrl);
        return 0;
    }

    ctrl->handle        = sControl;
    ctrl->dir           = FTPLIB_CONTROL;
    ctrl->ctrl          = NULL;
    ctrl->data          = NULL;
    ctrl->cmode         = FTPLIB_DEFMODE;
    ctrl->idlecb        = NULL;
    ctrl->idletime.tv_sec = ctrl->idletime.tv_usec = 0;
    ctrl->idlearg       = NULL;
    ctrl->xfered        = 0;
    ctrl->xfered1       = 0;
    ctrl->cbbytes       = 0;

    if (readresp('2', ctrl) == 0) {
        net_close(sControl);
        free(ctrl->buf);
        free(ctrl);
        return 0;
    }

    *nControl = ctrl;
    return 1;
}
#endif  // end of NET_USING_LWIP

GLOBALDEF int FtpSetCallback(const FtpCallbackOptions *opt, netftp *nControl)
{
    nControl->idlecb    = opt->cbFunc;
    nControl->idlearg   = opt->cbArg;
    nControl->idletime.tv_sec = opt->idleTime / 1000;
    nControl->idletime.tv_usec = (opt->idleTime % 1000) * 1000;
    nControl->cbbytes   = opt->bytesXferred;
    return 1;
}


GLOBALDEF int FtpClearCallback(netftp *nControl)
{
    nControl->idlecb    = NULL;
    nControl->idlearg   = NULL;
    nControl->idletime.tv_sec = 0;
    nControl->idletime.tv_usec = 0;
    nControl->cbbytes   = 0;
    return 1;
}


/*
 * FtpOptions - change connection options
 *
 * returns 1 if successful, 0 on error
 */
GLOBALDEF int FtpOptions(int opt, long val, netftp *nControl)
{
    int v, rv = 0;

    switch (opt)
    {
        case FTPLIB_CONNMODE:
            v = (int)
            val;

            if ((v == FTPLIB_PASSIVE) || (v == FTPLIB_PORT)) {
                nControl->cmode     = v;
                rv                  = 1;
            }

            break;

        case FTPLIB_CALLBACK:
            nControl->idlecb = (FtpCallback)
            val;
            rv = 1;
            break;

        case FTPLIB_IDLETIME:
            v = (int)
            val;
            rv = 1;
            nControl->idletime.tv_sec = v / 1000;
            nControl->idletime.tv_usec = (v % 1000) * 1000;
            break;

        case FTPLIB_CALLBACKARG:
            rv = 1;
            nControl->idlearg = (void *)
            val;
            break;

        case FTPLIB_CALLBACKBYTES:
            rv = 1;
            nControl->cbbytes = (int)
            val;
            break;

        default:
            break;
    }

    return rv;
}

/*
 * FtpGetOptions - get connection options value
 *
 * returns 1 if successful, 0 on error
 */
GLOBALDEF int FtpGetOptions(int opt, void *val, netftp *nethandler)
{
    int rv = 0;

    switch (opt)
    {
        case FTPLIB_GET_SOCKETHANDLER:
            if (val != NULL)
            {
                *((int *)val) = nethandler->handle;
                rv = 1;
            }
            break;

        case FTPLIB_GET_FTPTYPE:
            if (val != NULL)
            {
                *((unsigned char *)val) = nethandler->ftptype;
                rv = 1;
            }
            break;

        case FTPLIB_GET_IOCTX:
            if (val != NULL)
            {
                *(void **)val = nethandler->ioctx;
                rv = 1;
            }
            break;

        default:
            break;
    }

    return rv;
}

/*
 * FtpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
static int FtpSendCmd(const char *cmd, char expresp, netftp *nControl)
{
    if (nControl->dir != FTPLIB_CONTROL)
        return 0;

    FTPLOG_V("ftplib sendcmd: %s\n", cmd);

    if (Ftp_net_write(nControl, cmd, strlen(cmd)) <= 0) {
        FTPLOG_E("ftplib sendcmd write fail");
        return 0;
    }
    return readresp(expresp, nControl);
}



/*
 * FtpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
GLOBALDEF int FtpLogin(const char *user, const char *pass, netftp *nControl)
{
    char tempbuf[TMP_BUFSIZ];

    if (((strlen(user) + 8) > sizeof(tempbuf)) || ((strlen(pass) + 8) > sizeof(tempbuf)))
        return 0;

    sprintf(tempbuf, "USER %s\r\n", user);

    if (!FtpSendCmd(tempbuf, '3', nControl)) {
        if (nControl->response[0] == '2')
            return 1;
        return 0;
    }

    sprintf(tempbuf, "PASS %s\r\n", pass);
    return FtpSendCmd(tempbuf, '2', nControl);
}



/*
 * FtpOpenPort - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpOpenPort(netftp *nControl, netftp **nData, int mode, int dir, unsigned int dataIP, struct ifreq *ifname)
{
    int sData;
    int connret;

    union {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;

    union {
        struct sockaddr sa;
        struct sockaddr_in in;
    } ctrlperr;

    struct linger lng = {
        0, 0
    };

    unsigned int    l;
    int             on  = 1;
    netftp          *ctrl;
    char *          cp;
    unsigned int    v[6];
    char            buf[64];

    if (nControl->dir != FTPLIB_CONTROL)
        return - 1;

    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE)) {
        snprintf(nControl->response, sizeof(nControl->response), "Invalid direction %d\n", dir);
        return - 1;
    }

    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE)) {
        snprintf(nControl->response, sizeof(nControl->response), "Invalid mode %c\n", mode);
        return - 1;
    }

    l = sizeof(sin);

    if (nControl->cmode == FTPLIB_PASSIVE) {
        memset(&sin, 0, l);
        sin.in.sin_family   = AF_INET;

        if (!FtpSendCmd("PASV\r\n", '2', nControl))
            return - 1;

        cp = strchr(nControl->response, '(');
        if (cp == NULL)
            return - 1;

        cp++;
        sscanf(cp, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
        sin.sa.sa_data[2]   = v[2];
        sin.sa.sa_data[3]   = v[3];
        sin.sa.sa_data[4]   = v[4];
        sin.sa.sa_data[5]   = v[5];
        sin.sa.sa_data[0]   = v[0];
        sin.sa.sa_data[1]   = v[1];

        if(dataIP == FTPLIB_DATA_IP_CTRL || dataIP == FTPLIB_DATA_IP_CTRL_FIRST)
        {
            /*
                一些FTP Server在PASV中返回的地址是内网地址，不可以连接。
                这里使用控制连接的服务器地址代替数据连接，通常情况两者是一致的
            */
            if (getpeername(nControl->handle, &ctrlperr.sa, (socklen_t *)&l) < 0) {
                FTPLOG_E("ftplib FtpOpenPort getpeersockname fail");
                return - 1;
            }
            sin.in.sin_addr = ctrlperr.in.sin_addr;
        }
    }
    else {
        if (getsockname(nControl->handle, &sin.sa, (socklen_t *)&l) < 0) {
            FTPLOG_E("ftplib FtpOpenPort getsockname fail");
            return - 1;
        }
    }

    sData = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sData == -1) {
        FTPLOG_E("ftplib FtpOpenPort socket fail");
        return - 1;
    }

#if FTP_BIND_DEVICE
    if(ifname != NULL)
    {
        if (setsockopt(sData, SOL_SOCKET, SO_BINDTODEVICE,
            SETSOCKOPT_OPTVAL_TYPE ifname, sizeof(*ifname)) == -1) {
            net_close(sData);
            FTPLOG_E("ftplib FtpOpenPort setsockopt SO_BINDTODEVICE fail");
            return - 1;
        }
    }
#endif

    if (setsockopt(sData, SOL_SOCKET, SO_REUSEADDR,
        SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
        net_close(sData);
        FTPLOG_E("ftplib FtpOpenPort setsockopt: SO_REUSEADDR fail");
        return - 1;
    }

    if (setsockopt(sData, SOL_SOCKET, SO_LINGER,
        SETSOCKOPT_OPTVAL_TYPE & lng, sizeof(lng)) == -1) {
#if !defined(NET_USING_LWIP) || LWIP_SO_LINGER == 1
        net_close(sData);
        FTPLOG_E("ftplib FtpOpenPort setsockopt: SO_LINGER fail");
        return - 1;
#endif
    }

    if (nControl->cmode == FTPLIB_PASSIVE) {
        connret = connect(sData, &sin.sa, sizeof(sin.sa));
        if (connret == -1) {
            if(dataIP == FTPLIB_DATA_IP_CTRL_FIRST)//  控制连接的IP失败后再偿试服务器分配的IP
            {
                sin.sa.sa_data[2]   = v[2];
                sin.sa.sa_data[3]   = v[3];
                sin.sa.sa_data[4]   = v[4];
                sin.sa.sa_data[5]   = v[5];
                sin.sa.sa_data[0]   = v[0];
                sin.sa.sa_data[1]   = v[1];
                connret = connect(sData, &sin.sa, sizeof(sin.sa));
            }
            if(connret == -1)
            {
                net_close(sData);
                FTPLOG_E("ftplib FtpOpenPort connect fail");
                return - 1;
            }
        }
    }
    else {
        sin.in.sin_port = 0;
        if (bind(sData, &sin.sa, sizeof(sin)) == -1) {
            net_close(sData);
            FTPLOG_E("ftplib FtpOpenPort bind fail");
            return - 1;
        }

        if (listen(sData, 1) < 0) {
            net_close(sData);
            FTPLOG_E("ftplib FtpOpenPort listen fail");
            return - 1;
        }

        if (getsockname(sData, &sin.sa, (socklen_t *)&l) < 0) {
            net_close(sData);
            FTPLOG_E("ftplib FtpOpenPort getsockname fail");
            return - 1;
        }

        snprintf(buf, sizeof(buf), "PORT %d,%d,%d,%d,%d,%d\r\n",
            (unsigned char) sin.sa.sa_data[2],
            (unsigned char) sin.sa.sa_data[3],
            (unsigned char) sin.sa.sa_data[4],
            (unsigned char) sin.sa.sa_data[5],
            (unsigned char) sin.sa.sa_data[0],
            (unsigned char) sin.sa.sa_data[1]);

        if (!FtpSendCmd(buf, '2', nControl)) {
            net_close(sData);
            return - 1;
        }
    }

    ctrl = calloc(1, sizeof(netftp));
    if (ctrl == NULL) {
        net_close(sData);
        return - 1;
    }

    if ((mode == 'A') && ((ctrl->buf = malloc(FTPLIB_BUFSIZ)) == NULL)) {
        net_close(sData);
        free(ctrl);
        return - 1;
    }

    ctrl->handle        = sData;
    ctrl->dir           = dir;
    ctrl->idletime      = nControl->idletime;
    ctrl->idlearg       = nControl->idlearg;
    ctrl->xfered        = 0;
    ctrl->xfered1       = 0;
    ctrl->cbbytes       = nControl->cbbytes;
    ctrl->ctrl          = nControl;

    if (ctrl->idletime.tv_sec || ctrl->idletime.tv_usec || ctrl->cbbytes)
        ctrl->idlecb    = nControl->idlecb;
    else
        ctrl->idlecb    = NULL;

    nControl->data      = ctrl;
    *nData              = ctrl;
    return 1;
}

static int Ftp6OpenPort(netftp *nControl, netftp **nData, int mode, int dir, struct ifreq *ifname)
{
    int sData;
    int connret;

    union {
        struct sockaddr sa;
        struct sockaddr_in6 in;
    } sin;

    union {
        struct sockaddr sa;
        struct sockaddr_in6 in;
    } ctrlperr;

    struct linger lng = {
        0, 0
    };

    unsigned int    l;
    int             on  = 1;
    netftp          *ctrl;
    char *          cp;
    ip6_addr_t    ip6addr;
    char           ip6_str[50]= {0};
    char            buf[64];
    char            sep;

    if (nControl->dir != FTPLIB_CONTROL)
        return - 1;

    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE)) {
        snprintf(nControl->response, sizeof(nControl->response), "Invalid direction %d\n", dir);
        return - 1;
    }

    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE)) {
        snprintf(nControl->response, sizeof(nControl->response), "Invalid mode %c\n", mode);
        return - 1;
    }

    l = sizeof(sin);

    if (nControl->cmode == FTPLIB_PASSIVE) {
        memset(&sin, 0, l);
        int port = 0;

        if (!FtpSendCmd("EPSV\r\n", '2', nControl))
            return - 1;

        cp = strchr(nControl->response, '(');
        if (cp == NULL)
            return - 1;

        cp++;
        sep = cp[0];
        /*example |||12345|*/
        if((cp[1] == sep) && (cp[2] == sep) && ISDIGIT(cp[3]))
        {
            cp += 3;
        }

        memset(buf, 0, sizeof(buf));
        for(int i=0; i < sizeof(buf)-1; i++)
        {
            if(cp[i] != sep && ISDIGIT(cp[i]))
            {
                buf[i] = cp[i];
            }
            else
            {
                break;
            }
        }

        port = atoi(buf);

        if(port == 0 || port > 0xFFFF)
        {
            FTPLOG_E("ftplib Ftp6OpenPort epsv port error");
            return -1;
        }
        if (getpeername(nControl->handle, &ctrlperr.sa, (socklen_t *)&l) < 0) {
            FTPLOG_E("ftplib Ftp6OpenPort getpeersockname fail");
            return - 1;
        }
        sin.in.sin6_family = AF_INET6;
        sin.in.sin6_addr = ctrlperr.in.sin6_addr;
        sin.in.sin6_port = ntohs(port);
    }
    else {
        if (getsockname(nControl->handle, &sin.sa, (socklen_t *)&l) < 0) {
            FTPLOG_E("ftplib Ftp6OpenPort getsockname fail");
            return - 1;
        }
    }

    sData = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sData == -1) {
        FTPLOG_E("ftplib Ftp6OpenPort socket fail");
        return - 1;
    }

#if FTP_BIND_DEVICE
    if(ifname != NULL)
    {
        if (setsockopt(sData, SOL_SOCKET, SO_BINDTODEVICE,
            SETSOCKOPT_OPTVAL_TYPE ifname, sizeof(*ifname)) == -1) {
            net_close(sData);
            FTPLOG_E("ftplib Ftp6OpenPort setsockopt SO_BINDTODEVICE fail");
            return - 1;
        }
    }
#endif

    if (setsockopt(sData, SOL_SOCKET, SO_REUSEADDR,
        SETSOCKOPT_OPTVAL_TYPE & on, sizeof(on)) == -1) {
        net_close(sData);
        FTPLOG_E("ftplib Ftp6OpenPort setsockopt: SO_REUSEADDR fail");
        return - 1;
    }

    if (setsockopt(sData, SOL_SOCKET, SO_LINGER,
        SETSOCKOPT_OPTVAL_TYPE & lng, sizeof(lng)) == -1) {
#if !defined(NET_USING_LWIP) || LWIP_SO_LINGER == 1
        net_close(sData);
        FTPLOG_E("ftplib Ftp6OpenPort setsockopt: SO_LINGER fail");
        return - 1;
#endif
    }

    if (nControl->cmode == FTPLIB_PASSIVE) {
        connret = connect(sData, &sin.sa, sizeof(sin.sa));
        if(connret == -1)
        {
            net_close(sData);
            FTPLOG_E("ftplib Ftp6OpenPort connect fail");
            return - 1;
        }
    }
    else {
        sin.in.sin6_port = 0;
        if (bind(sData, &sin.sa, sizeof(sin)) == -1) {
            net_close(sData);
            FTPLOG_E("ftplib Ftp6OpenPort bind fail");
            return - 1;
        }

        if (listen(sData, 1) < 0) {
            net_close(sData);
            FTPLOG_E("ftplib Ftp6OpenPort listen fail");
            return - 1;
        }

        if (getsockname(sData, &sin.sa, (socklen_t *)&l) < 0) {
            net_close(sData);
            FTPLOG_E("ftplib Ftp6OpenPort getsockname fail");
            return - 1;
        }

        inet6_addr_to_ip6addr(&ip6addr, &sin.in.sin6_addr);
        ip6addr_ntoa_r(&ip6addr, ip6_str, sizeof(ip6_str));
        snprintf(buf, sizeof(buf), "EPRT |2|%s|%u|\r\n", ip6_str, ntohs(sin.in.sin6_port));

        if (!FtpSendCmd(buf, '2', nControl)) {
            net_close(sData);
            return - 1;
        }
    }

    ctrl = calloc(1, sizeof(netftp));
    if (ctrl == NULL) {
        net_close(sData);
        return - 1;
    }

    if ((mode == 'A') && ((ctrl->buf = malloc(FTPLIB_BUFSIZ)) == NULL)) {
        net_close(sData);
        free(ctrl);
        return - 1;
    }

    ctrl->handle        = sData;
    ctrl->dir           = dir;
    ctrl->idletime      = nControl->idletime;
    ctrl->idlearg       = nControl->idlearg;
    ctrl->xfered        = 0;
    ctrl->xfered1       = 0;
    ctrl->cbbytes       = nControl->cbbytes;
    ctrl->ctrl          = nControl;

    if (ctrl->idletime.tv_sec || ctrl->idletime.tv_usec || ctrl->cbbytes)
        ctrl->idlecb    = nControl->idlecb;
    else
        ctrl->idlecb    = NULL;

    nControl->data      = ctrl;
    *nData              = ctrl;
    return 1;
}


/*
 * FtpAcceptConnection - accept connection from server
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpAcceptConnection(netftp *nData, netftp *nControl)
{
    int             sData;

    struct sockaddr addr;
    unsigned int    l;
    int             i;

    struct timeval tv;
    fd_set          mask;
    int             rv = 0;

    FD_ZERO(&mask);
    FD_SET(nControl->handle, &mask);
    FD_SET(nData->handle, &mask);
    tv.tv_usec  = 0;
    tv.tv_sec   = ACCEPT_TIMEOUT;
    i           = nControl->handle;

    if (i < nData->handle)
        i = nData->handle;

    i = select(i + 1, &mask, NULL, NULL, &tv);
    if (i == -1) {
        strncpy(nControl->response, strerror(errno), sizeof(nControl->response));
        net_close(nData->handle);
        nData->handle       = 0;
        rv                  = 0;
    }
    else if (i == 0) {
        strncpy(nControl->response, "timed out waiting for connection", sizeof(nControl->response));
        net_close(nData->handle);
        nData->handle       = 0;
        rv                  = 0;
    }
    else {
        if (FD_ISSET(nData->handle, &mask)) {
            l       = sizeof(addr);
            sData   = accept(nData->handle, &addr, (socklen_t *)&l);
            i       = errno;
            net_close(nData->handle);

            if (sData > 0) {
                rv              = 1;
                nData->handle   = sData;
            }
            else {
                strncpy(nControl->response, strerror(i), sizeof(nControl->response));
                nData->handle   = 0;
                rv              = 0;
            }
        }
        else if (FD_ISSET(nControl->handle, &mask)) {
            net_close(nData->handle);
            nData->handle       = 0;
            readresp('2', nControl);
            rv                  = 0;
        }
    }

    return rv;
}



/*
 * FtpAccess - return a handle for a data stream
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpAccess(const char *path, int typ, int mode, netftp *nControl, netftp **nData)
{
    char    buf[TMP_BUFSIZ];
    int     dir;

    if ((path == NULL) && ((typ == FTPLIB_FILE_WRITE) || (typ == FTPLIB_FILE_READ))) {
        snprintf(nControl->response, sizeof(nControl->response),
            "Missing path argument for file transfer\n");
        return 0;
    }

    sprintf(buf, "TYPE %c\r\n", mode);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    switch (typ)
    {
        case FTPLIB_DIR:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                sprintf(buf, "NLST %s\r\n",path);
            }
            else
            {
                strcpy(buf, "NLST\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_DIR_VERBOSE:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                snprintf(buf, sizeof(buf), "LIST %s\r\n",path);
            }
            else
            {
                strcpy(buf, "LIST\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_DIR_MACHINE:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                sprintf(buf, "MLSD %s\r\n",path);
            }
            else
            {
                strcpy(buf, "MLSD\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_FILE_READ:
            if (strlen(path) + 8 > sizeof(buf))
                return 0;
            sprintf(buf, "RETR %s\r\n", path);
            dir = FTPLIB_READ;
            break;

        case FTPLIB_FILE_WRITE:
            if (strlen(path) + 8 > sizeof(buf))
                return 0;
            sprintf(buf, "STOR %s\r\n", path);
            dir = FTPLIB_WRITE;
            break;

        default:
            snprintf(nControl->response, sizeof(nControl->response), "Invalid open type %d\n", typ);
            return 0;
    }

    if (FtpOpenPort(nControl, nData, mode, dir, FTPLIB_DATA_IP_ASSIGN, NULL) == -1)
        return 0;

    if (!FtpSendCmd(buf, '1', nControl)) {
        FtpClose(*nData);
        *nData              = NULL;
        return 0;
    }

    if (nControl->cmode == FTPLIB_PORT) {
        if (!FtpAcceptConnection(*nData, nControl)) {
            FtpClose(*nData);
            *nData              = NULL;
            nControl->data      = NULL;
            return 0;
        }
    }

    return 1;
}

/*
 * FtpAccess_Pro - return a handle for a data stream
 *
 * path   服务器文件名或目录
 * getPos 下载文件时，文件的offset
 * typ    命令类型
 * mode   文本或二进制模式
 * dataIP 数据连接的IP地址策略
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpAccess_Pro(const char *path, unsigned int getPos, int typ, int mode, unsigned int dataIP, struct ifreq *ifname, netftp *nControl, netftp **nData)
{
    char    buf[TMP_BUFSIZ];
    int     dir;
    struct sockaddr sa;
    unsigned int    l = sizeof(sa);

    if ((path == NULL) && ((typ == FTPLIB_FILE_WRITE) || (typ == FTPLIB_FILE_READ))) {
        snprintf(nControl->response, sizeof(nControl->response),
            "Missing path argument for file transfer\n");
        return 0;
    }

    sprintf(buf, "TYPE %c\r\n", mode);
    if (!FtpSendCmd(buf, '2', nControl))
    {
        FTPLOG_E("ftplib access pro send TYPE fail\n");
        return 0;
    }

    switch (typ)
    {
        case FTPLIB_DIR:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                sprintf(buf, "NLST %s\r\n",path);
            }
            else
            {
                strcpy(buf, "NLST\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_DIR_VERBOSE:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                snprintf(buf, sizeof(buf), "LIST %s\r\n",path);
            }
            else
            {
                strcpy(buf, "LIST\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_DIR_MACHINE:
            if(path != NULL)
            {
                if (strlen(path) + 8 > sizeof(buf))
                    return 0;
                sprintf(buf, "MLSD %s\r\n",path);
            }
            else
            {
                strcpy(buf, "MLSD\r\n");
            }
            dir = FTPLIB_READ;
            break;

        case FTPLIB_FILE_READ:
            if (strlen(path) + 8 > sizeof(buf))
                return 0;
            sprintf(buf, "RETR %s\r\n", path);
            dir = FTPLIB_READ;
            break;

        case FTPLIB_FILE_WRITE:
            if (strlen(path) + 8 > sizeof(buf))
                return 0;
            sprintf(buf, "STOR %s\r\n", path);
            dir = FTPLIB_WRITE;
            break;

        default:
            snprintf(nControl->response, sizeof(nControl->response), "Invalid open type %d\n", typ);
            return 0;
    }

    memset(&sa, 0, l);
    if (getsockname(nControl->handle, &sa, (socklen_t *)&l) < 0) {
        FTPLOG_E("ftplib access pro getsockname fail");
        return 0;
    }

    FTPLOG_I("ftplib access pro ctrl family %u", sa.sa_family);

    if(sa.sa_family == AF_INET)
    {
        if (FtpOpenPort(nControl, nData, FTPLIB_IMAGE, dir, dataIP, ifname) == -1)
            return 0;
    }
    else
    {
        if (Ftp6OpenPort(nControl, nData, FTPLIB_IMAGE, dir, ifname) == -1)
            return 0;
    }

    if(typ == FTPLIB_FILE_READ)
    {
        char posbuf[20];
        sprintf(posbuf, "REST %u\r\n", getPos);
        if (!FtpSendCmd(posbuf, '3', nControl)) {
            FtpClose(*nData);
            *nData = NULL;
            return 0;
        }
    }

    if (!FtpSendCmd(buf, '1', nControl)) {
        FtpClose(*nData);
        *nData              = NULL;
        return 0;
    }

    if (nControl->cmode == FTPLIB_PORT) {
        if (!FtpAcceptConnection(*nData, nControl)) {
            FtpClose(*nData);
            *nData              = NULL;
            nControl->data      = NULL;
            return 0;
        }
    }

    return 1;
}



/*
 * FtpRead - read from a data connection
 */
GLOBALDEF int FtpRead(void *buf, int max, netftp *nData)
{
    int i;

    if (nData->dir != FTPLIB_READ)
        return 0;

    if (nData->buf) {
        i = readline(buf, max, nData);
    } else {
        i = socket_wait(nData);
        if (i != 1)
            return 0;
        i = Ftp_net_read(nData, buf, max);
    }

    if (i == -1)
        return 0;

    nData->xfered += i;

    if (nData->idlecb && nData->cbbytes) {
        nData->xfered1 += i;
        if (nData->xfered1 > nData->cbbytes) {
            if (nData->idlecb(nData, nData->xfered, nData->idlearg) == 0)
                return 0;
            nData->xfered1 = 0;
        }
    }

    return i;
}



/*
 * FtpWrite - write to a data connection
 */
GLOBALDEF int FtpWrite(const void *buf, int len, netftp *nData)
{
    int i;

    if (nData->dir != FTPLIB_WRITE)
        return 0;

    if (nData->buf) {
        i = writeline(buf, len, nData);
    } else {
        socket_wait(nData);
        i = Ftp_net_write(nData, buf, len);
    }

    if (i == -1)
        return 0;

    nData->xfered += i;

    if (nData->idlecb && nData->cbbytes) {
        nData->xfered1 += i;
        if (nData->xfered1 > nData->cbbytes) {
            nData->idlecb(nData, nData->xfered, nData->idlearg);
            nData->xfered1 = 0;
        }
    }

    return i;
}



/*
 * FtpClose - close a data connection
 */
GLOBALDEF int FtpClose(netftp *nData)
{
    netftp *ctrl;

    switch (nData->dir)
    {
        case FTPLIB_WRITE:

            /* potential problem - if buffer flush fails, how to notify user? */
            if (nData->buf != NULL)
                writeline(NULL, 0, nData);

        case FTPLIB_READ:
            if (nData->buf)
                free(nData->buf);

            shutdown(nData->handle, 2);
            net_close(nData->handle);
            ctrl = nData->ctrl;
            free(nData);
            if (ctrl)
                ctrl->data = NULL;

            if (ctrl && ctrl->response[0] != '4' && ctrl->response[0] != '5') {
                return (readresp('2', ctrl));
            }

            return 1;

        case FTPLIB_CONTROL:
            if (nData->data) {
                nData->ctrl = NULL;
                FtpClose(nData->data);
            }

            net_close(nData->handle);
            free(nData);
            return 0;
    }

    return 1;
}



/*
 * FtpSite - send a SITE command
 *
 * return 1 if command successful, 0 otherwise
 */
GLOBALDEF int FtpSite(const char *cmd, netftp *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(cmd) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "SITE %s\r\n", cmd);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpSysType - send a SYST command
 *
 * Fills in the user buffer with the remote system type.  If more
 * information from the response is required, the user can parse
 * it out of the response buffer returned by FtpLastResponse().
 *
 * return 1 if command successful, 0 otherwise
 */
GLOBALDEF int FtpSysType(char *buf, int max, netftp *nControl)
{
    int l   = max;
    char *b = buf;
    char *s;

    if (!FtpSendCmd("SYST\r\n", '2', nControl))
        return 0;

    s = &nControl->response[4];
    while ((--l) && (*s != ' '))
        *b++ = *s++;

    *b++ = '\0';
    return 1;
}



/*
 * FtpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpMkdir(const char *path, netftp *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "MKD %s\r\n", path);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpChdir(const char *path, netftp *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "CWD %s\r\n", path);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpCDUp - move to parent directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpCDUp(netftp *nControl)
{
    if (!FtpSendCmd("CDUP\r\n", '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpRmdir - remove directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpRmdir(const char *path, netftp *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "RMD %s\r\n", path);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}


/*
 * FtpPwd - get working directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpPwd(char *path, int max, netftp *nControl)
{
    int l   = max;
    char *b = path;
    char *s;

    if (!FtpSendCmd("PWD\r\n", '2', nControl))
        return 0;

    s = strchr(nControl->response, '"');
    if (s == NULL)
        return 0;

    s++;
    while ((--l) && (*s) && (*s != '"'))
        *b++ = *s++;

    *b++ = '\0';
    return 1;
}


/*
 * FtpXfer - issue a command and transfer data
 *
 * return 1 if successful, 0 otherwise
 */
#if defined(FTP_RTOS) && !defined(OS_USING_VFS)
static int FtpXfer(const char *localfile, const char *path, netftp *nControl, int typ, int mode)
{
    return 0;
}
#else
static int FtpXfer(const char *localfile, const char *path, netftp *nControl, int typ, int mode)
{
    int     l, c;
    char    *dbuf;
    FILE    *local = NULL;
    netftp  *nData;
    int     rv = 1;

    if (localfile != NULL) {
        char ac[4];
        memset(ac, 0, sizeof(ac));

        if (typ == FTPLIB_FILE_WRITE)
            ac[0] = 'r';
        else
            ac[0] = 'w';

        if (mode == FTPLIB_IMAGE)
            ac[1] = 'b';

        local = fopen(localfile, ac);
        if (local == NULL) {
            strncpy(nControl->response, strerror(errno), sizeof(nControl->response));
            return 0;
        }
    }

    if (local == NULL)
        local = (typ == FTPLIB_FILE_WRITE) ? stdin: stdout;

    if (!FtpAccess(path, typ, mode, nControl, &nData)) {
        if (localfile) {
            fclose(local);
            if (typ == FTPLIB_FILE_READ)
                unlink(localfile);
        }
        return 0;
    }

    dbuf = malloc(FTPLIB_BUFSIZ);
    if (dbuf == NULL) {
        if (localfile) {
            fclose(local);
            if (typ == FTPLIB_FILE_READ)
                unlink(localfile);
        }
        return 0;
    }

    if (typ == FTPLIB_FILE_WRITE) {
        while ((l = fread(dbuf, 1, FTPLIB_BUFSIZ, local)) > 0) {
            if ((c = FtpWrite(dbuf, l, nData)) < l) {
                FTPLOG_E("ftplib FtpXfer short write: passed %d, wrote %d\n", l, c);
                rv = 0;
                break;
            }
        }
    }
    else {
        while ((l = FtpRead(dbuf, FTPLIB_BUFSIZ, nData)) > 0) {
            if (fwrite(dbuf, 1, l, local) == 0) {
                FTPLOG_E("ftplib FtpXfer localfile write fail");
                rv = 0;
                break;
            }
        }
    }

    free(dbuf);
    fflush(local);

    if (localfile != NULL)
        fclose(local);

    FtpClose(nData);
    return rv;
}
#endif // defined(FTP_RTOS) && !defined(OS_USING_VFS)



/*
 * FtpNlst - issue an NLST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpNlst(const char *outputfile, const char *path, netftp *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR, FTPLIB_ASCII);
}

/*
 * FtpMlsd - issue an MLSD command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpMlsd(const char *outputfile, const char *path, netftp *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR_MACHINE, FTPLIB_ASCII);
}


/*
 * FtpDir - issue a LIST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpDir(const char *outputfile, const char *path, netftp *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII);
}



/*
 * FtpSize - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpSize(const char *path, unsigned int *size, char mode, netftp *nControl)
{
    char cmd[TMP_BUFSIZ];
    int resp, rv = 1;
    unsigned int sz;

    if ((strlen(path) + 8) > sizeof(cmd))
        return 0;

    sprintf(cmd, "TYPE %c\r\n", mode);
    if (!FtpSendCmd(cmd, '2', nControl))
        return 0;

    sprintf(cmd, "SIZE %s\r\n", path);
    if (!FtpSendCmd(cmd, '2', nControl)) {
        rv = 0;
    } else {
        if (sscanf(nControl->response, "%d %u", &resp, &sz) == 2)
            *size = sz;
        else
            rv = 0;
    }

    return rv;
}

#if defined(__UINT64_MAX)

/*
 * FtpSizeLong - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpSizeLong(const char *path, fsz_t *size, char mode, netftp *nControl)
{
    char cmd[TMP_BUFSIZ];
    int resp, rv = 1;
    fsz_t sz;

    if ((strlen(path) + 8) > sizeof(cmd))
        return 0;

    sprintf(cmd, "TYPE %c\r\n", mode);
    if (!FtpSendCmd(cmd, '2', nControl))
        return 0;

    sprintf(cmd, "SIZE %s\r\n", path);
    if (!FtpSendCmd(cmd, '2', nControl)) {
        rv = 0;
    } else {
        if (sscanf(nControl->response, "%d %" PRIu64 "", &resp, &sz) == 2)
            *size = sz;
        else
            rv = 0;
    }

    return rv;
}


#endif


/*
 * FtpModDate - determine the modification date of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpModDate(const char *path, char *dt, int max, netftp *nControl)
{
    char buf[TMP_BUFSIZ];
    int  rv = 1;

    if ((strlen(path) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "MDTM %s\r\n", path);
    if (!FtpSendCmd(buf, '2', nControl))
        rv = 0;
    else
        strncpy(dt, &nControl->response[4], max);

    return rv;
}



/*
 * FtpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpGet(const char *outputfile, const char *path, char mode, netftp *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_FILE_READ, mode);
}


/*
 * FtpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpPut(const char *inputfile, const char *path, char mode, netftp *nControl)
{
    return FtpXfer(inputfile, path, nControl, FTPLIB_FILE_WRITE, mode);
}


/*
 * FtpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpRename(const char *src, const char *dst, netftp *nControl)
{
    char cmd[TMP_BUFSIZ];

    if (((strlen(src) + 8) > sizeof(cmd)) || ((strlen(dst) + 8) > sizeof(cmd)))
        return 0;

    sprintf(cmd, "RNFR %s\r\n", src);
    if (!FtpSendCmd(cmd, '3', nControl))
        return 0;

    sprintf(cmd, "RNTO %s\r\n", dst);
    if (!FtpSendCmd(cmd, '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpDelete(const char *fnm, netftp *nControl)
{
    char cmd[TMP_BUFSIZ];

    if ((strlen(fnm) + 8) > sizeof(cmd))
        return 0;

    sprintf(cmd, "DELE %s\r\n", fnm);
    if (!FtpSendCmd(cmd, '2', nControl))
        return 0;

    return 1;
}



/*
 * FtpQuit - disconnect from remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF void FtpQuit(netftp *nControl)
{
    if (nControl->dir != FTPLIB_CONTROL)
        return;

    if (nControl->data)
        FtpClose(nControl->data);

    FtpSendCmd("QUIT\r\n", '2', nControl);

    net_close(nControl->handle);
    free(nControl->buf);
    free(nControl);
}

GLOBALREF int FtpSeek(unsigned int offset, netftp *nControl)
{
    char cmd[64];

    sprintf(cmd,"REST %u\r\n", offset);
    if (!FtpSendCmd(cmd, '3', nControl))
        return 0;

    return 1;
}

/*
 * FtpsAuth - start authentication for FTP over TLS
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpsAuth(const char *name, netftp *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(name) + 8) > sizeof(buf))
        return 0;

    sprintf(buf, "AUTH %s\r\n", name);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}

/*
 * FtpsPbsz - set protection buffer size
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpsPbsz(const unsigned int size, netftp *nControl)
{
    char buf[20];

    sprintf(buf, "PBSZ %u\r\n", size);
    if (!FtpSendCmd(buf, '2', nControl))
        return 0;

    return 1;
}

/*
 * FtpsProt - set data channel protection level
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpsProt(const char code, netftp *nControl)
{
    char buf[10];

    if (code == 'C' || code == 'S' || code == 'E' || code == 'P')
    {
        sprintf(buf, "PROT %c\r\n", code);
        if (!FtpSendCmd(buf, '2', nControl))
            return 0;
    }
    else
        return 0;

    return 1;
}

GLOBALDEF int FtpsReadRsp(char c, netftp *nControl)
{
    return readresp(c, nControl);
}

/*
 * FtpSetNetIOFunc - 设置用于从网络读写数据的API
 *
 * returns none
 */
GLOBALDEF void FtpSetNetIOFunc(unsigned char ftptype, void *ctx, FtpNetRead read, FtpNetWrite write, netftp *nhandle)
{
    nhandle->ftptype = ftptype;
    nhandle->ioctx = ctx;
    nhandle->netread = read;
    nhandle->netwrite = write;
}

/*
 * FtpClearNetIOFunc - 清除用于从网络读写数据的API
 *
 * returns none
 */
GLOBALDEF void FtpClearNetIOFunc(netftp *nhandle)
{
    nhandle->ioctx = NULL;
    nhandle->netread = NULL;
    nhandle->netwrite = NULL;
}


