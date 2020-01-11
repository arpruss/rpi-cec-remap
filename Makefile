OBJS=cec.o inject.o remap.o proc.o
BIN=cec-to-keys.bin
#LDFLAGS+=-lilclient
CFLAGS += -O99

include ../Makefile.include

