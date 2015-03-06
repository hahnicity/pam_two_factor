ARCHIVE_FILE=src/pam_two_factor.o
CC=gcc
CFLAGS=-c -fPIC
LINK_FILE=src/pam_two_factor.so

all: install

configure:
	cp pam_two_factor.conf /etc/security/

install: link 
	mv $(LINK_FILE) /lib/security

link: archive 
	$(CC) -shared $(ARCHIVE_FILE) -o $(LINK_FILE) -lcurl

archive: src/pam_two_factor.c
	$(CC) src/pam_two_factor.c -o $(ARCHIVE_FILE) $(CFLAGS)

clean:
	rm *.o
