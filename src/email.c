#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pam_two_factor.h"


static char * get_sms_gateway(void)
{
    int buff_len = strlen("Verizon") + 2;
    char carrier[buff_len]; // You need to allocate memory for fgets!
    printf("Input your phone carrier (ATT/Verizon/Sprint): ");
    fgets(carrier, buff_len, stdin);
    size_t len = strlen(carrier) - 1;
    if ( carrier[len] == '\n' )
    {
        carrier[len] = '\0';
    }
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
    char phone_number[14]; // 4 max digits for country code and 9 for number
    int r = get_user_phone_number(username, phone_number);
    if ( r != 0 )
    {
        printf("User %s does not have a phone number defined\n", username);
        return 1;
    }
    char * sms_gateway = get_sms_gateway();
    if ( sms_gateway[0] == '\0' )
    {
        printf("You did not enter a valid choice for carrier\n");
        return 1;
    }
    char send_to[strlen(phone_number) + 1 + strlen(sms_gateway)];
    sprintf(send_to, "%s@%s", phone_number, sms_gateway);
    char verify[4];
    r = get_config_var("VERIFY_HOST", verify);
    if ( r != 0 )
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
        return 1;
    }
    char from[72];
    r = get_config_var("FROM", from);
    if ( r != 0 )
    {
        printf("Unable to parse a value for the email address the two factor password is being sent from\n");
        return 1;
    }
    char cacert[144];
    r = get_config_var("CA_CERT_FILE", cacert);
    if ( r != 0 )
    {
        printf("Unable to parse a file path that will be our certificate file\n");
        return 1;
    }
    char smtp_url[72];
    r = get_config_var("SMTP_URL", smtp_url);
    if ( r != 0 )
    {
        printf("Unable to parse a url to send our email to\n");
        return 1;
    }
    r = send_email(smtp_url, cacert, send_to, from, code, verify_host);
    if (r != 0)
    {
        printf("Unable to send email to %s\n", send_to);
        return 1;
    }
    return 0;
}
