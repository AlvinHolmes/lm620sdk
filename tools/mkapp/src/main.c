/********************************************************************************
 * Copyright (c) 2022, Nanjing Innochip Technology Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        main.c
 *
 * @brief
 *
 * @par History:
 *
 * Date           Author              Notes
 * 2022-04-06     ICT Team           First version
 *
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h> /* statfs */
#include <dirent.h>

#include "md5.h"
#include "dlimg.h"
#include "dlmodule.h"



#define LOG(fmt, ...)       printf("mkapp: " fmt "\r\n", ##__VA_ARGS__)


static const char usage_short_opts[] = "e:i:o:h";

static struct option const usage_long_opts[] = {
    {"entry",  required_argument,  NULL,   'e'},
    {"input",   required_argument,  NULL,   'i'},
    {"output",  required_argument,  NULL,   'o'},

    {"help",    no_argument,        NULL,   'h'},
    {0,         0,                  0,      0  }
};

static const char* const usage_opts_help[] = {
    "entry point",
    "input file",
    "output file",
    "Print this help and exit",
};

static void usage(void)
{
    uint32_t i;
    uint32_t cnt = sizeof(usage_long_opts)/sizeof(struct option);

    #define VER   0x01
    printf("\nmkapp[%02x] usage:\n", VER);
    for(i = 0; i < cnt - 1; i++) {
        printf("  -%c,--%-*s  %s\n", usage_long_opts[i].val, 9, usage_long_opts[i].name, usage_opts_help[i]);
    }
}


#ifdef _WIN32
static const char* _convertToWinPath(const char* path)
{
    //converts to windows path
    char *p = (char*)path;
    while (*p) {
        if (*p == '/')
            *p = '\\';
        if (*(p+1) == '/')
            *(p+1) = '\\';
        if (*p == '\\' && *(p+1) == '\\') {
            // removes continus '/'
            char *q = p + 2;
            while (*q == '\\') {q++;}
            memmove(p+1, q, strlen(q)+1);
        }
        p++;
    }
    return path;
}
#endif

const char* _convertToStandardPath(const char* path)
{
    //converts to linux path
    char *p = (char*)path;
    while (*p) {
        if (*p == '\\')
            *p = '/';
        if (*(p+1) == '\\')
            *(p+1) = '/';
        if (*p == '/' && *(p+1) == '/') {
            // removes continus '/'
            char *q = p + 2;
            while (*q == '/') {q++;}
            memmove(p+1, q, strlen(q)+1);
        }
        p++;
    }
    return path;
}


static int32_t _checkSubfileExsit(const char *dir, const char *file)
{
    char path[260] = { 0 };
    sprintf(path, "%s/%s", dir, file);
    return access(path, F_OK) ? 0x00 : 0x01;
}

static  int32_t _createDir(const char *path)
{
    size_t i, len = strlen(path);
    char dirPath[len+2];

    strncpy(dirPath, path, len);
    if (dirPath[len-1] != '/') {
        dirPath[len++] = '/';
    }
    dirPath[len] = '\0';

    for (i=1; i<len; i++) {
        if (dirPath[i] == '/') {
            dirPath[i] = '\0';
            if (access(dirPath, F_OK) < 0) {
#ifdef _WIN32
                if (mkdir(dirPath) < 0) {
                    return -1;
                }
#else
                mode_t  pre = umask(0002);
                if (mkdir(dirPath, 0755) < 0) {
                    umask(pre);
                    return -1;
                }
                umask(pre);
#endif
            }
            dirPath[i] = '/';
        }
    }
    return 0;
}

static int _createParentDir(const char *path)
{
    int ret = 0;
    char *pos = strrchr(path, '/');
    if (pos) {
        *pos = 0;
        ret = _createDir(path);
        *pos = '/';
    }
    return ret;
}

static int _copyDir(const char *dst, const char *src)
{
    char cmd[1024] = {0};
#ifdef _WIN32
    sprintf(cmd, "xcopy %s/* %s/", src, dst);
    _convertToWinPath(cmd);
    strcat(cmd, " /e/y/i > nul");
#else
    sprintf(cmd, "cp -arf %s/* %s/", src, dst);
#endif
    return system(cmd);
}

int main(int argc, char *argv[])
{
    char    *entry, *input, *output;
    int     opt;
/*
    if (argc < 2) {
        goto _help;
    }
*/
    entry = "0";
	input = output = NULL;
	
    while ((opt = getopt_long(argc, argv, usage_short_opts, usage_long_opts, NULL)) != EOF) {
        switch (opt) {
            case 'e':
                entry = optarg;
                break;
            case 'i':
                input = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            default:
				LOG("argv err: %d %s  %s  fail",argc,argv[0],argv[1]);
                goto _help;
        }
    }

//    if (symbol && input && output) {
	if (input && output) {
        uint32_t    szInput, szOutput;
        FILE        *fp;

        //_convertToStandardPath(symbol);
        _convertToStandardPath(input);
        _convertToStandardPath(output);
/*
        if (dlmodule_link_check(input, symbol)) {
            return -1;
        }
*/

        fp = fopen(input, "rb");
        if (fp == NULL) {
            LOG("Open input file: %s fail", input);
            return -1;
        }

        if (fseek(fp, 0, SEEK_END) == 0) {
            szInput = (uint32_t)ftell(fp);
            szOutput = sizeof(DLMO_ImageHead) + szInput + sizeof(DLMO_ImageTail);
            fseek(fp, 0, SEEK_SET);
        } else {
            LOG("Get file %s length fail", input);
            fclose(fp);
            return -1;
        }

        DLMO_ImageHead *imgHead = (DLMO_ImageHead*)malloc(szOutput);
        if (imgHead == NULL) {
            LOG("Alloc memory fail");
            fclose(fp);
            return -1;
        }
        uint8_t *body = (uint8_t*)imgHead + sizeof(DLMO_ImageHead);
        DLMO_ImageTail *imgTail = (DLMO_ImageTail*)((uint8_t*)imgHead + sizeof(DLMO_ImageHead) + szInput);

        memcpy(imgHead->symb, DLMO_IMAGE_HEAD, 4);
        imgHead->size = szOutput;
		//imgHead->addr_load = 0x803E7000;
		imgHead->addr_load = atoi(entry);
        imgHead->resv = -1;
		LOG("Entry[%x]\r\n",imgHead->addr_load);
		
        uint32_t size = fread(body, 1, szInput, fp);
        fclose(fp);
        if (size != szInput) {
            LOG("Read input file data fail");
            free(imgHead);
            goto _help;
        }

        MD5_CTX md5ctx;
        MD5Init(&md5ctx);
        MD5Update(&md5ctx, (unsigned char*)imgHead, szInput + sizeof(DLMO_ImageHead));
        MD5Final(imgTail->md5, &md5ctx);

        _createParentDir(output);
        fp = fopen(output, "wb");
        if (fp == NULL) {
            LOG("Create output file: %s fail", output);
            free(imgHead);
            return -1;
        }

        size = fwrite(imgHead, 1, szOutput, fp);
        fclose(fp);
        free(imgHead);
        if (size != szOutput) {
            LOG("Write output file fail");
            return -1;
        }
        return 0;
    }

_help:
    usage();
    return -1;
}
