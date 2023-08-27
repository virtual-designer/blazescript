#pragma once

using http_status_code = unsigned int;

struct http_response
{
    http_status_code status = 200;
    char* body;
};