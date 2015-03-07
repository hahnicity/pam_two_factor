#include <stdio.h>
#include <string.h>

#include <curl/curl.h>


int send_email(char smtp_url[], 
               char cacert[], 
               char send_to[], 
               char from[], 
               int code,
               long verify_host)
{
    char buffer[580];
    struct curl_slist *recipients = NULL;
    char format[] = "To: %s\r\nFrom: %s(roboauth)\r\n\r\nYour two "\
                    "factor code is %d\r\n";
    FILE * stream;
    CURL *curl;
    CURLcode res = CURLE_OK;
    curl = curl_easy_init();
    if(curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_CAINFO, cacert);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_host);
        recipients = curl_slist_append(recipients, send_to);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        srand(time(NULL));
        sprintf(buffer, format, send_to, from, code);
        stream = fmemopen(buffer, strlen(buffer), "r");

        curl_easy_setopt(curl, CURLOPT_READDATA, stream);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        /* Free the list of recipients */
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    return (int)res;
}
