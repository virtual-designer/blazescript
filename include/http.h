#pragma once
#include "utils.h"

#include <curl/curl.h>
#include <stdbool.h>

struct http_response
{
    int status;
    char* body;
};

inline bool http_is_successful(struct http_response res) {
    return res.status >= 200 &&
           res.status <= 299;
}

size_t _http_write_callback(
    char* buf, size_t size,
    size_t nmemb, void* up
);

struct http_response http_make_request(const char* url);