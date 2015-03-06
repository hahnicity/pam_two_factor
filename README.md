# pam_two_factor
Two factor authentication for PAM

## Purpose
This was a hobby project for me to learn C and PAM at the same time. If you are 
interested in a real two factor module then you can either grab one of the many that
are floating around the internet or help me improve this one! Since it was a hobby 
project it makes a few assumptions

 * You have a mailserver that you do not have to explicitly authenticate into
 * A users phone number would be stored in GECOS in /etc/passwd and not in LDAP.
 * Your phone carrier is either ATT/Verizon/Sprint.
 * You have an internet connection :)

## Usage

Modify configuration settings in pam_two_factor.conf according to your system.

    sudo make install
    sudo make configure

Now modify any of the pam configuration files you want to add two factor authentication to

    account <required OR requisite> pam_two_factor.so
    auth    requisite               pam_two_factor.so
