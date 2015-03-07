#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pam_two_factor.h"

const char CONFIG_FILE[] = "/etc/security/pam_two_factor.conf";


static char * set_config_var(char * line, int var_len, int i)
{
    char * var = (char *)malloc(i + 1);
    if ( var == NULL )
    {
        return "";
    }
    strncpy(var, line + var_len, i);
    var[i] = '\0';
    return var;
}

char * get_config_var(char * config_var)
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
                    if ( result[i + s_var_len] == ' ' || result[i + s_var_len] == '\n' || result[i + s_var_len] == '\t' )
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

