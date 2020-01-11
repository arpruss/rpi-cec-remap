#ifndef _PROC_H
#define _PROC_H

int init_proc_monitor(const char** m, unsigned mc);
void end_proc_monitor(void);
void proc_monitor(char* data);

#endif
