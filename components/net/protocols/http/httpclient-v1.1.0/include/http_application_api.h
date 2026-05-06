/**
 * @file http_application_api.h
 * http API header file.
 *
 * @version   V1.0
 * @date      2020-08-01
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 */


#ifndef HTTP_APPLICATION_API
#define HTTP_APPLICATION_API

#include "http.h"
#include "http_client.h"

#define CONFIG_HTTP_DOWNLOAD

#ifdef CONFIG_HTTP_RECV_NON_BOLCK
#define H_RECV_NONBLOCK   1 /* nonblocking */
#define H_RECV_STREAM   true /* stream */

#define HO_RECV_NOBLOCK   0x0001 /* recv response data with not block */
#define HO_RECV_STREAM   0x0002 /* recv response data by stream method */
#endif
#define HO_RECV_TIMEOUT_MS   0x0003 /* recv response data timeout in milliseconds */
#define HO_SSL_AUTHMODE   0x0004 /* ssl verify mode */

/**
 * This function executes get http option.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] optname            option type name.
 * @param[in]optlen              option value length
 * @param[out]optval             option value
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_getopt(http_client_t *client, int optname, int *optlen, void *optval);

/**
 * This function executes set http option.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] optname            option type name.
 * @param[in]optlen              option value length
 * @param[in]optval              option value
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_setopt(http_client_t *client, int optname, int optlen, const void *optval);


/**
 * This function executes URL parse and create tcp connection base on the given url.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in]pdp_id              active pdp id
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_create_extend(http_client_t *client, const char *url, uint8_t pdp_id);

/**
 * This function executes recv data without timeout.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_recv_extend(http_client_t *client, http_client_data_t *client_data);

/**
 * This function executes a GET request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_get_extend(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a POST request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be posted.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_post_extend(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a PUT request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be put.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_put_extend(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes URL parse and create tcp connection base on the given url.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_create(http_client_t *client, const char *url);

/**
 * This function executes a GET request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_get(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a POST request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be posted.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_post(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a PUT request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be put.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_put(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a HEAD request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_head(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a DELETE request on a given URL. It blocks until completion.
 * @param[in] client               client is a pointer to the #httpclient_t.
 * @param[in] url                  url is the URL to run the request.
 * @param[in, out] client_data client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_delete(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes tcp close .
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
void http_client_stop(http_client_t *client);

#ifdef CONFIG_HTTP_DOWNLOAD

HTTP_RESULT_CODE http_client_get_file(http_client_t *client, const char *url, http_client_data_t *client_data, const char *filename);

int http_tcp_recv_file_wrapper(http_client_t *client, char *buf, int buflen, int timeout_ms, int *p_read_len, const char *filename);

HTTP_RESULT_CODE http_client_post_file(http_client_t *client, const char *url, http_client_data_t *client_data, char *filePath, int req_method, char *content_type);
#endif

/**
 * This function executes a GET request on a given URL. It blocks until completion.
 * @param[in] client             client is a pointer to the #httpclient_t.
 * @param[in] url                url is the URL to run the request.
 * @param[in, out] client_data   client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_get_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a POST request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be posted.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_post_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a PUT request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be put.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_put_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data);

/**
 * This function executes a POST/PUT file request on a given URL. It blocks until completion.
 * @param[in] client              client is a pointer to the #httpclient_t.
 * @param[in] url                 url is the URL to run the request.
 * @param[in] filePath            filePath is the path of file.
 * @param[in] req_method          req_method is the request method of POST or PUT.
 * @param[in] content_type        content_type is the content type of file.
 * @param[in, out] client_data    client_data is a pointer to the #httpclient_data_t instance to collect the data returned by the request. It also contains the data to be posted.
 * @return           Please refer to #HTTPC_RESULT.
 */
HTTP_RESULT_CODE http_client_post_file_without_redirect(http_client_t *client, const char *url, http_client_data_t *client_data, char *filePath, int req_method, char *content_type);

#endif
