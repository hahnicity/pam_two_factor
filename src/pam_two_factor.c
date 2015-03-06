/*
   This module is a two factor module for pam. 

   Usage of this module inside a pam config file would be as follows

        account requisite pam_two_factor.so
        auth    requisite pam_two_factor.so
  
   The following things can be done better given more time and learning about C.

   1. It'd be better to hook up a legitimate configuration library
   so I don't have to go parsing through a config file line by line,
   char by char
   2. It'd be nice if I could hook up some varying ways to get user phone numbers.
   The GECOS entries are fine for users that are defined on a system but if LDAP is being
   used then we would be totally out of luck.
   3. I'd like to find a way to store telephone carriers in some kind of
   file or DB so that I don't have to query the user each time when they
   want to login for what their carrier is
   4. Put up better error handling for failure cases
   5. There is probably a much better way to handle configuration for a module. Maybe
   using a struct? The problem is I use a lot of dynamically generated memory which
   leaves quite a bit of room for memory leak.
   6. Right now this module only supports making email requests to unauthenticated 
   smtp servers (preferably locally of course) it would be nice if this could be 
   extended to google
   7. Another nice thing would be to add API integration for something like twilio
   or another SMS gateway besides using email.
   8. Adding whitelisted users would be nice.
   9. unit tests!
*/
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <time.h>

const int MAXLINE = 255;
const char CONFIG_FILE[] = "/etc/security/pam_two_factor.conf";
const char ETC_PASSWD[] = "/etc/passwd";


static int send_email(char smtp_url[], 
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

static char * set_config_var(char * line, int var_len, int i)
{
    char * var = (char *)malloc(i + 1);
    if ( var == NULL )
    {
        // Keep this as a stub until I learn how to do better
        // error handling.
        return "";
    }
    strncpy(var, line + var_len, i);
    var[i] = '\0';
    return var;
}

static char * get_config_var(char * config_var)
{
    int i;
    int s_var_len = 1 + strlen(config_var);
    char * result;
    char buffer[MAXLINE];
    char search_var[s_var_len];
    FILE * fp;
    sprintf(search_var, "%s=", config_var);
    fp = fopen(CONFIG_FILE, "r");
    if ( fp != NULL )
    {
        while ( !feof(fp) )
        {
            fgets(buffer, MAXLINE, fp);
            result = strstr(buffer, search_var);
            if ( result != NULL )
            {
                for ( i = 0; i < strlen(result) - s_var_len; i++ )
                {
                    if ( result[i + s_var_len] == ' ')
                    {
                        break;
                    }
                }
                break;
            }
        }
    }
    else
    {
        return "";
    }
    if ( i >= strlen(result) - s_var_len )
    {
        return "";
    }
    else
    {
        return set_config_var(result, s_var_len, i);
    }
}

static char * get_user_phone_number(const char * username)
{
    FILE * fp;
    char buffer[MAXLINE];
    char * phone_number;
    fp = fopen(ETC_PASSWD, "r");
    if ( fp != NULL )
    {
        while ( !feof(fp) )
        {
            fgets(buffer, MAXLINE, fp);
            if ( strstr(buffer, username) != NULL )
            {
                int i;
                int comma_count = 0;
                for ( i = 0; i < strlen(buffer); i++ )
                {
                    // Go through and look for GECOS field entries.
                    if ( buffer[i] == ',')
                    {
                        comma_count++;
                    }
                    if ( comma_count == 3 )
                    {
                        int j;
                        for ( j = 1; j < strlen(buffer) + i;  j++ )
                        {
                            if ( buffer[i + j] == ':' )
                            {
                                break;
                            }
                        }
                        if ( j == 1 )
                        {
                            return "";
                        }
                        phone_number = (char *)malloc(j + 1);
                        strncpy(phone_number, buffer + i + 1, j - 1);
                        phone_number[j] = '\0';
                        break;
                    }
                }
                break;
            }
        }
    }
    return phone_number;
}

static char * get_sms_gateway(void)
{
    char * carrier;
    printf("Input your phone carrier (ATT/Verizon/Sprint): ");
    scanf("%s", carrier);
    if ( strcmp(carrier, "ATT") == 0 )
    {
        return "txt.att.net";
    }
    else if ( strcmp(carrier, "Verizon") == 0 )
    {
        return "vtext.com";
    }
    else if ( strcmp(carrier, "Sprint") == 0 )
    {
        return "messaging.sprintpcs.com";
    }
    else
    {
        return "";
    }
}

PAM_EXTERN 
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{
    const char * username;
    int retval = pam_get_user(pamh, &username, "Username: ");
    if (retval != PAM_SUCCESS) {
        return retval;
    }
    char * phone_number = get_user_phone_number(username);
    if ( phone_number[0] == '\0' )
    {
        printf("User %s is on the machine but does not have a GECOS "\
               "entry for a phone number. You must add one in order "\
               "to use two factor authentication", username);
        return PAM_AUTH_ERR;
    }
    free(phone_number);
    return PAM_SUCCESS;
}

PAM_EXTERN 
int pam_sm_authenticate( pam_handle_t *pamh, int flags, int argc, const char **argv ) 
{   
    const char* username;
    int retval = pam_get_user(pamh, &username, "Username: ");
    if (retval != PAM_SUCCESS) {
        return retval;
    }
    char * phone_number = get_user_phone_number(username);
    if ( phone_number[0] == '\0' )
    {
        printf("User %s is on the machine but does not have a GECOS "\
               "entry for a phone number. You must add one in order "\
               "to use two factor authentication", username);
        return PAM_AUTH_ERR;
    }
    char * sms_gateway = get_sms_gateway();
    if ( sms_gateway[0] == '\0' )
    {
        printf("You did not enter a valid choice for carrier");
        return PAM_AUTH_ERR;
    }
    char send_to[strlen(phone_number) + 1 + strlen(sms_gateway)];
    sprintf(send_to, "%s@%s", phone_number, sms_gateway);
    free(phone_number);
    // Add error handling to handle case of null string
    char * verify = get_config_var("VERIFY_HOST");
    if ( verify[0] == '\0' )
    {
        printf("You need to specify whether or not you want to verify the host you are sending an email from\n");
        return PAM_SYSTEM_ERR;
    }
    long verify_host;
    if ( strcmp(verify, "yes") == 0 )
    {
        verify_host = (long)1;
    } 
    else if ( strcmp(verify, "no") == 0 )
    {
        verify_host = (long)0;
    }
    else
    {
        free(verify);
        return PAM_SYSTEM_ERR;
    }
    free(verify);
    char * from = get_config_var("FROM");
    if ( from[0] == '\0' )
    {
        printf("Unable to parse a value for the email address the two factor password is being sent from\n");
        return PAM_SYSTEM_ERR;
    }
    char * cacert = get_config_var("CA_CERT_FILE");
    if ( cacert[0] == '\0' )
    {
        free(from);
        printf("Unable to parse a file path that will be our certificate file\n");
        return PAM_SYSTEM_ERR;
    }
    char * smtp_url = get_config_var("SMTP_URL");
    if ( smtp_url[0] == '\0' )
    {
        free(from);
        free(smtp_url);
        printf("Unable to parse a url to send our email to\n");
        return PAM_SYSTEM_ERR;
    }
    srand(time(NULL));
    int code = 100000 + rand() / (RAND_MAX / (999999 - 100000 + 1) + 1);
    int curl_res = send_email(smtp_url, cacert, send_to, from, code, verify_host);
    free(smtp_url);
    free(cacert);
    free(from);
    if (curl_res != 0)
    {
        printf("Unable to send email to %s", send_to);
        return PAM_AUTH_ERR;
    }
    int user_input;
    printf("Input your two factor code: ");
    scanf("%d", &user_input);
    if (user_input != code) 
    {
        return PAM_AUTH_ERR;
    }
    return PAM_SUCCESS;
}
