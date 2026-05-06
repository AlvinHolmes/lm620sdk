//  The MIT License (MIT)
//  Copyright (c) 2018 liu2guang <liuguang@rt-thread.com>

//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:

//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.

//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
//  OR OTHER DEALINGS IN THE SOFTWARE.

#include "os.h"

#include "nr_micro_shell.h"
#include <librws.h>
#include "rws_port.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "drv_rtc.h"
#include "rws_string.h"


struct librws_app
{
    bool init;
    rws_socket *socket;
};
typedef struct librws_app *librws_app_t;

static librws_app_t app;

static void onopen(rws_socket socket)
{
    osPrintf("websocket connected. ");
}

static void onclose(rws_socket socket)
{
    rws_error error = rws_socket_get_error(socket);

    if (error)
    {
        osPrintf("websocket disconnect, error: %i, %s ", rws_error_get_code(error), rws_error_get_description(error));
    }
    else
    {
        osPrintf("websocket disconnect! ");
    }

    rws_socket_disconnect_and_release(app->socket);

    app->init = OS_FALSE;
}

static void onmessage_text(rws_socket socket, const char *text, const unsigned int len)
{
    char *buff = NULL;

    buff = (char *)osMalloc(2048);

    memset(buff, 0x00, 2048);
    memcpy(buff, text, len);

    osPrintf("message(txt), %d(byte): %s ", len, buff);

    if (buff != NULL)
    {
        osFree(buff);
    }
}

static void onmessage_bin(rws_socket socket, const void *data, const unsigned int len)
{
    char *buff = NULL;

    buff = (char *)osMalloc(2048);

    memset(buff, 0x00, 2048);
    memcpy(buff, data, len);

    osPrintf("message(bin), %d(byte): %s ", len, buff);

    if (buff != NULL)
    {
        osFree(buff);
    }
}

static char acess_key[64] = {0};
static char appid[32] = {0};
static int on_handshake(rws_socket socket, void *buf, const unsigned int bufLen)
{
    RTC_Time time_stamp = RTC_GetTime();

    int32_t len = rws_sprintf(buf, bufLen,
                            "X-Api-Resource-Id: volc.bigasr.sauc.duration\r\n"
                            "X-Api-Access-Key: %s\r\n"
                            "X-Api-App-Key: %s\r\n"
                            "X-Api-Request-Id: %d\r\n",
                            acess_key, appid,
                            time_stamp);

    return len;
}

//_rws_connect wss openspeech.bytedance.com 443 <access-key> <appid> 
static void _rws_connect(char argc, char *argv[])
{
    int port = -1;
    rws_bool ret = rws_false;

    if (argc != 6)
    {
        osPrintf("the msh cmd format: rws_conn ws/wss host [port]. ");
        return ;
    }

    strncpy(acess_key, argv[4], sizeof(acess_key));
    strncpy(appid, argv[5], sizeof(appid));

    osPrintf("acess_key: %s, app-id: %s \r\n", acess_key, appid);

    app = (librws_app_t)osMallocAlign(sizeof(struct librws_app), 32);
    if (app == NULL)
    {
        osPrintf("librws_conn cmd memory malloc failed. ");
        return ;
    }

    memset(app, 0x00, sizeof(struct librws_app));

    app->socket = rws_socket_create();
    if (app->socket == NULL)
    {
        osPrintf("librws socket create failed. ");
        return ;
    }

    if (strcmp(argv[1], "ws") == 0)
    {
        port = ((argv[3] == NULL) ? (80) : (atoi(argv[3])));
        rws_socket_set_url(app->socket, "ws", argv[2], port, "/api/v3/sauc/bigmodel");
    }
    else if (strcmp(argv[1], "wss") == 0)
    {
        port = ((argv[3] == NULL) ? (443) : (atoi(argv[3])));
        rws_socket_set_url(app->socket, "wss", argv[2], port, "/api/v3/sauc/bigmodel");
    }
    else
    {
        osPrintf("protocol types are not supported, only support ws/wss. ");
        return ;
    }

    rws_socket_set_on_connected(app->socket, &onopen);
    rws_socket_set_on_disconnected(app->socket, &onclose);
    rws_socket_set_on_received_text(app->socket, &onmessage_text);
    rws_socket_set_on_received_bin(app->socket, &onmessage_bin);
    rws_socket_set_on_handshake(app->socket, &on_handshake);

    ret = rws_socket_connect(app->socket);
    if (ret == rws_false)
    {
        if (strcmp(argv[1], "ws") == 0)
        {
            osPrintf("connect %s://%s:%d/ failed. ", argv[1], argv[2], port);
        }
        else if (strcmp(argv[1], "wss") == 0)
        {
            osPrintf("connect %s://%s:%d/ failed. ", argv[1], argv[2], port);
        }
    }
    else
    {
        if (strcmp(argv[1], "ws") == 0)
        {
            osPrintf("try connect %s://%s:%d/ ", argv[1], argv[2], port);
        }
        else if (strcmp(argv[1], "wss") == 0)
        {
            osPrintf("try connect %s://%s:%d/ ", argv[1], argv[2], port);
        }
    }

    app->init = OS_TRUE;

    return ;
}
NR_SHELL_CMD_EXPORT(_rws_connect, _rws_connect);

static void _rws_disconnect(char argc, char *argv[])
{
    if (app->init == OS_FALSE)
    {
        osPrintf("no websocket connection. ");
        return ;
    }

    rws_socket_disconnect_and_release(app->socket);
    osPrintf("try disconnect websocket connection. ");

    return ;
}
NR_SHELL_CMD_EXPORT(_rws_disconnect, _rws_disconnect);


// static int _rws_send(int argc, char *argv[])
// {
//     if (argc == 1)
//     {
//         osPrintf("The command format: rws_send \"content\". ");
//         return ;
//     }

//     if (app->init == OS_FALSE)
//     {
//         osPrintf("no websocket connection. ");
//         return ;
//     }

//     osPrintf("string = %s, len = %d", argv[1], strlen(argv[1]));

//     rws_socket_send_bin(app->socket, full_request, strlen(full_request), rws_opcode_binary_frame, true);

//     return ;
// }
// SH_CMD_EXPORT(_rws_send, _rws_send, "rws send");
