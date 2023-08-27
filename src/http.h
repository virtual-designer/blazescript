#pragma once

#include <curl/curl.h>

#include "http_response.h"

size_t write_callback(
    char* buf, size_t size,
    size_t nmemb, void* up
);

void make_request();