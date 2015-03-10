#include <stdio.h>
#include <string.h>

#include "libconfig.h"

#include "pam_two_factor.h"

const char CONFIG_FILE[] = "/etc/security/pam_two_factor_test.conf";


int get_config_var(char * config_var_name, char * val)
{
    const char ** tmp_val;
    FILE * fp;
    config_t config;
    config_init(&config);
    fp = fopen(CONFIG_FILE, "r");
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
