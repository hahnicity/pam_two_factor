/*
   This module is a two factor module for pam. 

   Usage of this module inside a pam config file would be as follows

        account requisite pam_two_factor.so
        auth    requisite pam_two_factor.so
  
   The following things can be done better given more time and learning about C.

   1. It'd be better to hook up a legitimate configuration library
   so I don't have to go parsing through a config file line by line,
   char by char
   2. I'd like to find a way to store telephone carriers in some kind of
   file or DB so that I don't have to query the user each time when they
   want to login for what their carrier is
   3. There is probably a much better way to handle configuration for a module. Maybe
   using a struct? The problem is I use a lot of dynamically generated memory which
   leaves quite a bit of room for memory leak.
   4. Another nice thing would be to add API integration for something like twilio
   or another SMS gateway besides using email.
   5. Adding whitelisted users would be nice.
   6. unit tests!
*/
#include <stdio.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <time.h>

#include "pam_two_factor.h"

const int MAXLINE = 255;
const int TWO_FACTOR_MIN = 100000;
const int TWO_FACTOR_MAX = 999999;

PAM_EXTERN 
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{
    const char * username;
    int r = pam_get_user(pamh, &username, "Username: ");
    if (r != PAM_SUCCESS) {
        return r;
    }
    char * phone_number = get_user_phone_number(username);
    if ( phone_number[0] == '\0' )
    {
        printf("User %s does not have a phone number defined", username);
        return PAM_AUTH_ERR;
    }
    free(phone_number);
    return PAM_SUCCESS;
}

PAM_EXTERN 
int pam_sm_authenticate( pam_handle_t *pamh, int flags, int argc, const char **argv ) 
{   
    const char * username;
    int user_input;
    int r = pam_get_user(pamh, &username, "Username: ");
    if (r != PAM_SUCCESS) {
        return r;
    }
    srand(time(NULL));
    int code = TWO_FACTOR_MIN + rand() / (RAND_MAX / (TWO_FACTOR_MAX - TWO_FACTOR_MIN + 1) + 1);
    r = communicate_with_user(code, username);
    if ( r != 0 )
    {
        return PAM_AUTH_ERR;
    }
    printf("Input your two factor code: ");
    scanf("%d", &user_input);
    if (user_input != code) 
    {
        return PAM_AUTH_ERR;
    }
    return PAM_SUCCESS;
}
