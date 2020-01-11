#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include "inject.h"

static int fd;

int init_inject() {
    struct uinput_setup usetup;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0) {
      return fd;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) 
      return -1;

    for (int i=1;i<255;i++)
      if (ioctl(fd, UI_SET_KEYBIT, i) < 0) 
        goto ERR;


    memset(&usetup, 0, sizeof(usetup));
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "uinput-injector");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor  = 0x1;
    usetup.id.product = 0x1;
    usetup.id.version = 1;

    if(ioctl(fd, UI_DEV_SETUP, &usetup) < 0 ||ioctl(fd, UI_DEV_CREATE)<0) {
	printf("error with create\n");
        goto ERR;
    }

    return 0;

ERR:
     close(fd);
     return -1;
}

static int emit_event(int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   if (sizeof(ie) != write(fd, &ie, sizeof(ie)))
      return -1;
   return 0;
}

int inject_key(unsigned code, unsigned press) {
    if(emit_event(EV_KEY, code, !!press)<0 || emit_event(EV_SYN, SYN_REPORT, 0)<0)
       return -1;
    return 0;
}

void end_inject() {
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

#if 0
int main() {
    if (init_inject()<0) {
	fputs("Cannot initialize injector: use sudo or check permissions?\n", stderr);
	return 1;
    }
    for (int i=KEY_A;i<=KEY_Z;i++) {
       if(inject_key(i,1) == 0) printf("inject %d\n", i);
       inject_key(i,0);
       sleep(1);
    }
    end_inject();
    return 0;
}
#endif
