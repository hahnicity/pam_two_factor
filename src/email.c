#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pam_two_factor.h"


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


int communicate_with_user(int code, const char * username)
{
    char * phone_number = get_user_phone_number(username);
    if ( phone_number[0] == '\0' )
    {
        printf("User %s does not have a phone number defined", username);
        return 1;
    }
    char * sms_gateway = get_sms_gateway();
    if ( sms_gateway[0] == '\0' )
    {
        printf("You did not enter a valid choice for carrier");
        return 1;
    }
    char send_to[strlen(phone_number) + 1 + strlen(sms_gateway)];
    sprintf(send_to, "%s@%s", phone_number, sms_gateway);
    free(phone_number);
    char * verify = get_config_var("VERIFY_HOST");
    if ( verify[0] == '\0' )
    {
        printf("You need to specify whether or not you want to verify the host you are sending an email from\n");
        return 1;
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
        return 1;
    }
    free(verify);
    char * from = get_config_var("FROM");
    if ( from[0] == '\0' )
    {
        printf("Unable to parse a value for the email address the two factor password is being sent from\n");
        return 1;
    }
    char * cacert = get_config_var("CA_CERT_FILE");
    if ( cacert[0] == '\0' )
    {
        free(from);
        printf("Unable to parse a file path that will be our certificate file\n");
        return 1;
    }
    char * smtp_url = get_config_var("SMTP_URL");
    if ( smtp_url[0] == '\0' )
    {
        free(from);
        free(smtp_url);
        printf("Unable to parse a url to send our email to\n");
        return 1;
    }
    int r = send_email(smtp_url, cacert, send_to, from, code, verify_host);
    free(smtp_url);
    free(cacert);
    free(from);
    if (r != 0)
    {
        printf("Unable to send email to %s", send_to);
        return 1;
    }
    return 0;
}
