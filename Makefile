all : sudp-forwarder.c 
	gcc -Wall -o sudp-forwarder sudp-forwarder.c
