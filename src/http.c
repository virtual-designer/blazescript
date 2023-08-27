#include "http.h"

size_t write_callback(
    char* buf, size_t size,
    size_t nmemb, void* up)
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

char* make_request()
{
    CURL* curl = curl_easy_init();

    const char* url = "https://discord.com/api/v9/invites";
    char* response = NULL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION,
        write_callback);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "cURL error: %s\n", curl_easy_strerror(res));

    }
    else
    {
        

        // get info
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_easy_cleanup(curl);
    

}
