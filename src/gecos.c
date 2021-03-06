#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pam_two_factor.h"

const char ETC_PASSWD[] = "/etc/passwd";


int get_user_phone_number(const char * username, char * phone_number)
{
    FILE * fp;
    char buffer[MAXLINE];
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
                            return 1;
                        }
                        strncpy(phone_number, buffer + i + 1, j - 1);
                        phone_number[j - 1] = '\0';
                        break;
                    }
                }
                break;
            }
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

