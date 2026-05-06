/*
 *   Copyright (c) 2014 - 2017 Kulykov Oleh <info@resident.name>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "rws_common.h"
#include "rws_socket.h"

#ifdef RWS_THREAD_BLOCK
void rws_set_event_and_notify(rws_socket socket, int event_flag)
{
    _rws_socket *s = (_rws_socket *)socket;

    rws_mutex_lock(s->event_mutex);
    s->event_flag |= event_flag;
    rws_mutex_unlock(s->event_mutex);

    osSemaphoreRelease(s->event_sem);
    return;
}

int rws_get_event_and_clean(rws_socket socket)
{
    _rws_socket *s = (_rws_socket *)socket;
    int event_flag = 0;

    rws_mutex_lock(s->event_mutex);
    event_flag = s->event_flag;
    s->event_flag = 0;
    rws_mutex_unlock(s->event_mutex);

    return event_flag;
}

void rws_ping_timer_func(void *argument)
{
    _rws_socket *s = (_rws_socket *)argument;

    rws_set_event_and_notify(s, RWS_IDLE_EVENT_SEND_PING);
    return;
}

void rws_socket_event_callback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    _rws_socket *s = (_rws_socket *)cb_param;

    if(event == SOCKET_TCP_EVENT_RECV || event == SOCKET_TCP_EVENT_ERR)
        rws_set_event_and_notify(s, RWS_IDLE_EVENT_RECV_DATA);

    return;
}
#else
void rws_set_event_and_notify(rws_socket socket, int event_flag)
{
    return;
}

int rws_get_event_and_clean(rws_socket socket)
{
    return 0;
}

void rws_ping_timer_func(void *argument)
{
    return;
}

void rws_socket_event_callback(int fd, unsigned int event, void *p, int len, int8_t err, void *cb_param)
{
    return;
}
#endif

