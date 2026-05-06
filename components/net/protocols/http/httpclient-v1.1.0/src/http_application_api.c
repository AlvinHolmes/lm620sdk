/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * \@file        http_application_api.c
 *
 * \@brief       http api implemnt
 *
 * \@details     provide interface for up application
 *
 * \@revision    v1.0
 * Date         Author          Notes
 * 2020-8-12   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "http_application_api.h"
#include "http_client.h"
#include "http_form_data.h"
#include "http_wrapper.h"

#ifdef CONFIG_HTTP_RECV_NON_BOLCK
#define http_set_recv_flags(client, optval)          ((client)->recv_flags = (optval))
#define http_set_response_stream(client, optval)     ((client)->is_response_stream = (optval))

#define http_get_recv_flags(client)                  ((client)->recv_flags)
#define http_get_response_stream(client)             ((client)->is_response_stream)
#endif

#define http_set_recv_timeout_ms(client, optval)     ((client)->recv_timeout_ms = (optval))
#define http_set_ssl_authmode(client, optval)        ((client)->ssl_authmode = (optval))

#define http_get_recv_timeout_ms(client)             ((client)->recv_timeout_ms)
#define http_get_ssl_authmode(client)                ((client)->ssl_authmode)

#define HTTP_OPT_CHECK_OPTLEN(optlen, opttype) do { if ((optlen) < sizeof(opttype)) { return HTTP_ENOBUFS; }}while(0)
#define HTTP_OPT_CHECK_OPTLEN_CONN(client, optlen, opttype) do { \
    HTTP_OPT_CHECK_OPTLEN( optlen, opttype); \
    if (client == NULL) { return HTTP_EARG; } }while(0)

HTTP_RESULT_CODE http_client_getopt(http_client_t *client, int optname, int *optlen, void *optval)
{
    HTTP_RESULT_CODE ret = HTTP_SUCCESS;

    switch (optname)
    {
#ifdef CONFIG_HTTP_RECV_NON_BOLCK
        case HO_RECV_NOBLOCK:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, *optlen, int);
            *(int *)optval = http_get_recv_flags(client);
            break;

        case HO_RECV_STREAM:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, *optlen, int);
            *(int *)optval = http_get_response_stream(client);
            break;
#endif
        case HO_RECV_TIMEOUT_MS:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, *optlen, int);
            *(int *)optval = http_get_recv_timeout_ms(client);
            break;

        case HO_SSL_AUTHMODE:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, *optlen, int);
            *(int *)optval = http_get_ssl_authmode(client);
            break;

        default:
            LOG_E(HTTP_TAG, "http_client_getopt, optname err[%d]", optname);
            ret = HTTP_ENOTSUPP;
            break;
    }

    return ret;
}

HTTP_RESULT_CODE http_client_setopt(http_client_t *client, int optname, int optlen, const void *optval)
{
    HTTP_RESULT_CODE ret = HTTP_SUCCESS;

    switch (optname)
    {
#ifdef CONFIG_HTTP_RECV_NON_BOLCK
        case HO_RECV_NOBLOCK:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, optlen, int);
            http_set_recv_flags(client, *(const int *)optval);
            break;

        case HO_RECV_STREAM:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, optlen, int);
            http_set_response_stream(client, *(const int *)optval);
            break;
#endif
        case HO_RECV_TIMEOUT_MS:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, optlen, int);
            http_set_recv_timeout_ms(client, *(const int *)optval);
            break;

        case HO_SSL_AUTHMODE:
            HTTP_OPT_CHECK_OPTLEN_CONN(client, optlen, int);
            http_set_ssl_authmode(client, *(const int *)optval);
            break;

        default:
            LOG_E(HTTP_TAG, "http_client_setopt, optname err[%d]", optname);
            ret = HTTP_ENOTSUPP;
            break;
    }

    return ret;
}

HTTP_RESULT_CODE http_request_common_extend(http_client_t *client, const char *url, int method, http_client_data_t *client_data)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    client_data->is_more = false;
    ret = http_client_send(client, url, method, client_data);

    return ret;
}

HTTP_RESULT_CODE http_client_create_extend(http_client_t *client, const char *url, uint8_t pdp_id)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    ret = http_client_conn_extend(client, url, pdp_id);

    return ret;
}

HTTP_RESULT_CODE http_client_recv_extend(http_client_t *client, http_client_data_t *client_data)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    ret = (HTTP_RESULT_CODE)http_client_recv(client, client_data);

    /* Don't reset form data when got a redirected response */
    if(client_data->is_redirected == 0)
    {
        httpclient_clear_form_data(client_data);
    }

    return ret;
}

HTTP_RESULT_CODE http_client_get_extend(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common_extend(client, url, HTTP_GET, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common_extend(client, client_data->redirect_url, HTTP_GET, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_post_extend(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common_extend(client, url, HTTP_POST, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common_extend(client, client_data->redirect_url, HTTP_POST, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_put_extend(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common_extend(client, url, HTTP_PUT, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common_extend(client, client_data->redirect_url, HTTP_PUT, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_request_common(http_client_t *client, const char *url, int method, http_client_data_t *client_data)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    /* reset httpclient redirect flag */
    client_data->is_redirected = 0;

    ret = http_client_send(client, url, method, client_data);
    if(!ret)
    {
        ret = (HTTP_RESULT_CODE)http_client_recv(client, client_data);
    }

    /* Don't reset form data when got a redirected response */
    if(client_data->is_redirected == 0)
    {
        httpclient_clear_form_data(client_data);
    }

    //http_client_close(client);

    return ret;
}

HTTP_RESULT_CODE http_client_create(http_client_t *client, const char *url)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    ret = http_client_conn(client, url);

    return ret;
}

HTTP_RESULT_CODE http_client_get(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_GET, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_GET, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_post(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_POST, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_POST, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_put(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_PUT, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_PUT, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_head(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_HEAD, client_data);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_common(client, client_data->redirect_url, HTTP_HEAD, client_data);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_delete(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    return http_request_common(client, url, HTTP_DELETE, client_data);
}

void http_client_stop(http_client_t *client)
{
    http_client_close(client);

    return;
}

#ifdef CONFIG_HTTP_DOWNLOAD
HTTP_RESULT_CODE http_request_file_common(http_client_t *client, const char *url, int method, http_client_data_t *client_data, char *filePath)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;

    /* reset httpclient redirect flag */
    client_data->is_redirected = 0;

    ret = http_client_send_file(client, url, method, client_data, filePath);
    if(!ret)
    {
        ret = (HTTP_RESULT_CODE)http_client_recv(client, client_data);
    }

    /* Don't reset form data when got a redirected response */
    if(client_data->is_redirected == 0)
    {
        httpclient_clear_form_data(client_data);
    }

    return ret;
}

HTTP_RESULT_CODE http_client_get_file(http_client_t *client, const char *url, http_client_data_t *client_data, const char *filename)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;
    int reclen = 0;
    /* reset httpclient redirect flag */
    client_data->is_redirected = 0;

    char *buf = NULL;
    int timeout_ms = CONFIG_HTTP_RECV_TIMEOUT_MS;

    buf = malloc(HTTP_CLIENT_CHUNK_SIZE);
    if(NULL == buf)
    {
        return HTTP_ENOBUFS;
    }
    ret = http_client_send(client, url, HTTP_GET, client_data);
    if (!ret)
    {
        if (client->socket < 0)
        {
            LOG_E(HTTP_TAG, "Invalid socket fd %d!", client->socket);
            free(buf);
            return (HTTP_RESULT_CODE)ret;
        }

        if (client->is_http)
        {
            ret = (HTTP_RESULT_CODE)http_tcp_recv_file_wrapper(client, buf, HTTP_CLIENT_CHUNK_SIZE - 1, timeout_ms, &reclen, filename);
        }
        else
        {
            LOG_E(HTTP_TAG, "need http\n");
        }

        if (ret != HTTP_SUCCESS && ret != HTTP_ECLSD)
        {
            free(buf);
            return ret;
        }

        free(buf);
        LOG_D(HTTP_TAG, "httpclient_recv_data() result:%d, client:%p, reclen:%d\n", ret, client, reclen);
    }

    return ret;
}

#if 0
HTTP_RESULT_CODE http_client_post_file(http_client_t *client, const char *url, http_client_data_t *client_data, char *filePath, int req_method, char *content_type)
{
    HTTP_RESULT_CODE ret = HTTP_ECONN;
    client_data->is_redirected = 0; /* reset httpclient redirect flag */
    char *buf = NULL;

    buf = malloc(HTTP_CLIENT_CHUNK_SIZE);
    if(NULL == buf)
    {
        return HTTP_ENOBUFS;
    }

    ret = http_client_send_file(client, url, req_method, client_data, filename);
    ret = http_client_send(client, url, req_method, client_data, filename);
    if(!ret)
    {
        ret = (HTTP_RESULT_CODE)http_client_recv(client, client_data);
    }

    return ret;
}
#else
HTTP_RESULT_CODE http_client_post_file(http_client_t *client, const char *url, http_client_data_t *client_data, char *filePath, int req_method, char *content_type)
{
    char* content_disposition = "form-data";
    char* name = "file";
    int ret = -1;

    if((HTTP_POST != req_method) && (HTTP_PUT != req_method))
    {
        return (HTTP_RESULT_CODE)ret;
    }

    if(NULL == content_type)
    {
        content_type = "multipart/form-data";
    }

    ret = httpclient_formdata_addfile(client_data, content_disposition, name, content_type, filePath);
    if(0 != ret)
    {
        LOG_E(HTTP_TAG, "httpclient_formdata_addfile fail\n");
        return ret;
    }

    ret = http_request_file_common(client, url, req_method, client_data, filePath);

    while((0 == ret) && (1 == client_data->is_redirected))
    {
        ret = http_request_file_common(client, client_data->redirect_url, req_method, client_data, filePath);
    }

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}
#endif
#endif

HTTP_RESULT_CODE http_client_get_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_GET, client_data);

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_post_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_POST, client_data);

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_put_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data)
{
    int ret = http_request_common(client, url, HTTP_PUT, client_data);

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

HTTP_RESULT_CODE http_client_post_file_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data, char *filePath, int req_method, char *content_type)
{
    char* content_disposition = "form-data";
    char* name = "file";
    int ret = -1;

    if((HTTP_POST != req_method) && (HTTP_PUT != req_method))
    {
        return (HTTP_RESULT_CODE)ret;
    }

    if(NULL == content_type)
    {
        content_type = "multipart/form-data";
    }

    ret = httpclient_formdata_addfile(client_data, content_disposition, name, content_type, filePath);
    if(0 != ret)
    {
        LOG_E(HTTP_TAG, "httpclient_formdata_addfile fail\n");
        return ret;
    }

    ret = http_request_file_common(client, url, req_method, client_data, filePath);

    if(client_data->redirect_url != NULL)
    {
        free(client_data->redirect_url);
        client_data->redirect_url = NULL;
    }

    return (HTTP_RESULT_CODE)ret;
}

