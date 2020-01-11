#ifndef _INJECT_H
#define _INJECT_H

#include <linux/uinput.h>

int init_inject(void);
void end_inject(void);
int inject_key(unsigned key, unsigned pressed);

#endif
