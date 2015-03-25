/*
This file could use some error handling for segmentation faults. Right
now there is none.
*/
#include <stdio.h>
#include <string.h>

#include "libconfig.h"

#include "pam_two_factor.h"

const char CONFIG_FILE[] = "/etc/security/pam_two_factor.conf";


int get_config_var(char * config_var_name, char * val)
{
    const char ** tmp_val;
    FILE * fp;
    config_t config;
    config_init(&config);
    fp = fopen(CONFIG_FILE, "r");
    if ( fp == NULL )
    {
        return 1;
    }
    int r = config_read(&config, fp);
    if ( r != CONFIG_TRUE )
    { 
        return 1;
    }
    r = config_lookup_string(&config, config_var_name, tmp_val);
    if ( r != CONFIG_TRUE )
    {
        return 1;
    }
    strcpy(val, *tmp_val);
    return 0;
}
