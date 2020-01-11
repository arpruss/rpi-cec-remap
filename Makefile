OBJS=cec.o inject.o remap.o proc.o
BIN=cec-to-keyboard.bin
#LDFLAGS+=-lilclient
CFLAGS += -O99

include /opt/vc/src/hello_pi/Makefile.include


