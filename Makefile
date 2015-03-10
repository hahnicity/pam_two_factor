ARCHIVE_CFLAGS=-c -fPIC
ARCHIVE_FILE=src/pam_two_factor.o
CC=gcc
CONFIG_TYPE=src/configlib.c
EMAIL_TYPE=src/local_email.c
LINK_CFLAGS=$(shell pkg-config --libs libcurl,libconfig)
LINK_FILE=src/pam_two_factor.so
NUMBER_LOOKUP=src/gecos.c
USE_EMAIL=src/email.c

all: install

configure:
	cp pam_two_factor.conf /etc/security/

install: link 
	mv $(LINK_FILE) /lib/security

link: archive 
	$(CC) -shared $(CONFIG_TYPE) $(EMAIL_TYPE) $(USE_EMAIL) $(ARCHIVE_FILE) $(NUMBER_LOOKUP) -o $(LINK_FILE) $(LINK_CFLAGS)

archive: src/pam_two_factor.c
	$(CC) src/pam_two_factor.c -o $(ARCHIVE_FILE) $(ARCHIVE_CFLAGS)

clean:
	rm *.o
