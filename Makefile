ARCHIVE_FILE=src/pam_two_factor.o
CC=gcc
CFLAGS=-c -fPIC
LFLAGS=-lcurl -shared
LINK_FILE=src/pam_two_factor.so

all: config

config: cdistribute
	cp pam_two_factor.conf /etc/security/

cdistribute: link 
	mv $(LINK_FILE) /lib/security

link: archive 
	$(CC) $(LFLAGS) $(ARCHIVE_FILE) -o $(LINK_FILE)

archive: src/pam_two_factor.c
	$(CC) $(CFLAGS) src/pam_two_factor.c -o $(ARCHIVE_FILE)

clean:
	rm *.o
