#ifndef PAM_TWO_FACTOR_H_
#define PAM_TWO_FACTOR_H_

extern const int MAXLINE;

int communicate_with_user(int code, const char * username);

char * get_config_var(char * get_config_var);

char * get_user_phone_number(const char * username);

int send_email(char * smtp_url, char * cacert, char * send_to, char * from, int code, long verify_host);

#endif
