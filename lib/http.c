// FIXME: Do not use cURL

#include "http.h"
#include "alloca.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

size_t _http_write_callback(
    char* contents, size_t size,
    size_t nmemb, void* userp)
{
    size_t total_size = size * nmemb;
    char **response_ptr = (char **)userp;

    *response_ptr = realloc(*response_ptr, total_size + 1);
    if (*response_ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    memcpy(*response_ptr, contents, total_size);
    (*response_ptr)[total_size] = '\0';

    return total_size;
}

struct http_response http_make_request(const char* url)
{
    CURL* curl = curl_easy_init();
    char* response_str = NULL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_str);

    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION,
        _http_write_callback);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    int res = curl_easy_perform(curl);
    struct http_response http_res = {};

    if (res != CURLE_OK)
    {
        fatal_error(
            "cURL error: %s",
            curl_easy_strerror(res)
        );
    }
    else
    {
        int http_code;
        curl_easy_getinfo(
            curl, CURLINFO_RESPONSE_CODE,
            &http_code
        );

        strcpy(http_res.body, response_str);
        http_res.status = http_code;

        free(response_str);
        curl_easy_cleanup(curl);
    }

    curl_easy_cleanup(curl);
    return http_res;
}