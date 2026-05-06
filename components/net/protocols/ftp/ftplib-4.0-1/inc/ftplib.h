/***************************************************************************/
/*                                     */
/* ftplib.h - header file for callable ftp access routines                 */
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

#if !defined(__FTPLIB_H)
#define __FTPLIB_H

#if defined(__unix__) || defined(VMS)
#define GLOBALDEF
#define GLOBALREF extern
#elif defined(_WIN32)
#if defined BUILDING_LIBRARY
#define GLOBALDEF __declspec(dllexport)
#define GLOBALREF __declspec(dllexport)
#else
#define GLOBALREF __declspec(dllimport)
#endif
#else
#define GLOBALDEF
#define GLOBALREF extern
#endif

#include <limits.h>
#include <inttypes.h>
#include <sys/time.h>
#include "lwip/sockets.h"

/* FtpAccess() type codes */
#define FTPLIB_DIR 1
#define FTPLIB_DIR_VERBOSE 2
#define FTPLIB_DIR_MACHINE 3
#define FTPLIB_FILE_READ 4
#define FTPLIB_FILE_WRITE 5

/* FtpAccess() mode codes */
#define FTPLIB_ASCII 'A'
#define FTPLIB_IMAGE 'I'
#define FTPLIB_TEXT FTPLIB_ASCII
#define FTPLIB_BINARY FTPLIB_IMAGE

/* connection modes */
#define FTPLIB_PASSIVE 1
#define FTPLIB_PORT 2

/* connection option names */
#define FTPLIB_CONNMODE 1
#define FTPLIB_CALLBACK 2
#define FTPLIB_IDLETIME 3
#define FTPLIB_CALLBACKARG 4
#define FTPLIB_CALLBACKBYTES 5

/* connection get option names */
#define FTPLIB_GET_SOCKETHANDLER 1
#define FTPLIB_GET_FTPTYPE       2
#define FTPLIB_GET_IOCTX         3

/* 被动模式下，数据连接使用的IP策略 */
#define FTPLIB_DATA_IP_ASSIGN          0  //  被动模式下，数据连接使用服务器分配的地址
#define FTPLIB_DATA_IP_CTRL            1  //  被动模式下，数据连接使用跟控制连接一样的IP地址
#define FTPLIB_DATA_IP_CTRL_FIRST      2  //  被动模式下，数据连接先使用跟控制连接一样的IP地址，失败后使用服务器分配的地址

/* FTP类型 */
#define FTPLIB_TYPE_FTP             0  //  普通FTP
#define FTPLIB_TYPE_FTPS_IMPLICIT   1  //  FTPs隐式模式
#define FTPLIB_TYPE_FTPS_EXPLICIT   2  //  FTPs显式模式
#define FTPLIB_TYPE_FTPS            3  // FTPS

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__UINT64_MAX)
typedef uint64_t fsz_t;
#else
typedef uint32_t fsz_t;
#endif

typedef struct NetBuf netftp;
typedef int (*FtpCallback)(netftp *nControl, fsz_t xfered, void *arg);

typedef struct FtpCallbackOptions {
    FtpCallback     cbFunc;         /* function to call */
    void            *cbArg;         /* argument to pass to function */
    unsigned int    bytesXferred;   /* callback if this number of bytes transferred */
    unsigned int    idleTime;       /* callback if this many milliseconds have elapsed */
} FtpCallbackOptions;

typedef int (*FtpNetWrite)(void *ctx, const char * buf, size_t len);
typedef int (*FtpNetRead)(void *ctx, char * buf, size_t len);

GLOBALREF int ftplib_debug;
GLOBALREF void FtpInit(void);
GLOBALREF char* FtpLastResponse(netftp *nControl);
GLOBALREF int FtpConnect(const char *host, netftp **nControl);
GLOBALREF int FtpOptions(int opt, long val, netftp *nControl);
GLOBALREF int FtpSetCallback(const FtpCallbackOptions *opt, netftp *nControl);
GLOBALREF int FtpClearCallback(netftp *nControl);
GLOBALREF int FtpLogin(const char *user, const char *pass, netftp *nControl);
GLOBALREF int FtpAccess(const char *path, int typ, int mode, netftp *nControl, netftp **nData);
GLOBALREF int FtpRead(void *buf, int max, netftp *nData);
GLOBALREF int FtpWrite(const void *buf, int len, netftp *nData);
GLOBALREF int FtpClose(netftp *nData);
GLOBALREF int FtpSite(const char *cmd, netftp *nControl);
GLOBALREF int FtpSysType(char *buf, int max, netftp *nControl);
GLOBALREF int FtpMkdir(const char *path, netftp *nControl);
GLOBALREF int FtpChdir(const char *path, netftp *nControl);
GLOBALREF int FtpCDUp(netftp *nControl);
GLOBALREF int FtpRmdir(const char *path, netftp *nControl);
GLOBALREF int FtpPwd(char *path, int max, netftp *nControl);
GLOBALREF int FtpNlst(const char *output, const char *path, netftp *nControl);
GLOBALREF int FtpDir(const char *output, const char *path, netftp *nControl);
GLOBALDEF int FtpMlsd(const char *outputfile, const char *path, netftp *nControl);// add by zz
GLOBALREF int FtpSize(const char *path, unsigned int *size, char mode, netftp *nControl);
#if defined(__UINT64_MAX)
GLOBALREF int FtpSizeLong(const char *path, fsz_t *size, char mode, netftp *nControl);
#endif
GLOBALREF int FtpModDate(const char *path, char *dt, int max, netftp *nControl);
GLOBALREF int FtpGet(const char *output, const char *path, char mode, netftp *nControl);
GLOBALREF int FtpPut(const char *input, const char *path, char mode, netftp *nControl);
GLOBALREF int FtpRename(const char *src, const char *dst, netftp *nControl);
GLOBALREF int FtpDelete(const char *fnm, netftp *nControl);
GLOBALREF void FtpQuit(netftp *nControl);

GLOBALREF int FtpSeek(unsigned int offsset, netftp *nControl);
GLOBALDEF int FtpAccess_Pro(const char *path, unsigned int getPos, int typ, int mode, unsigned int dataIP, struct ifreq *ifname, netftp *nControl, netftp **nData);
GLOBALDEF int FtpConnect_Pro(unsigned char ftptype, const char *host, int port, struct ifreq *ifname, netftp **nControl);
GLOBALDEF int FtpGetOptions(int opt, void *val, netftp *nethandler);

GLOBALDEF int FtpsAuth(const char *name, netftp *nControl);
GLOBALDEF int FtpsPbsz(const unsigned int size, netftp *nControl);
GLOBALDEF int FtpsProt(const char code, netftp *nControl);
GLOBALDEF int FtpsReadRsp(char c, netftp *nControl);
GLOBALDEF void FtpSetNetIOFunc(unsigned char ftptype, void *ctx, FtpNetRead read, FtpNetWrite write, netftp *nhandle);
GLOBALDEF void FtpClearNetIOFunc(netftp *nhandle);
#ifdef __cplusplus
};
#endif

#endif /* __FTPLIB_H */
