ARCHIVE_FILE=src/pam_two_factor.o
CC=gcc
CFLAGS=-c -fPIC
CONFIG_TYPE=src/manual_config_parsing.c
EMAIL_TYPE=src/local_email.c
LINK_FILE=src/pam_two_factor.so
NUMBER_LOOKUP=src/gecos.c
USE_EMAIL=src/email.c

all: install

configure:
	cp pam_two_factor.conf /etc/security/

install: link 
	mv $(LINK_FILE) /lib/security

link: archive 
	$(CC) -shared $(CONFIG_TYPE) $(EMAIL_TYPE) $(USE_EMAIL) $(ARCHIVE_FILE) $(NUMBER_LOOKUP) -o $(LINK_FILE) -lcurl

archive: src/pam_two_factor.c
	$(CC) src/pam_two_factor.c -o $(ARCHIVE_FILE) $(CFLAGS)

clean:
	rm *.o
