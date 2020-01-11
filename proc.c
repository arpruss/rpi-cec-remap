#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

const char** monitored;
unsigned monitored_count;
DIR* proc;

int init_proc_monitor(const char** m, unsigned mc) {
    proc = opendir("/proc");
    if (proc == NULL)
	return -1;
    monitored = m;
    monitored_count = mc;
    return 0;
}

void end_proc_monitor(void) {
    closedir(proc);
}

void check(const char* name, char* data) {
    char fname[80] = "/proc/";
    char cmdline[1024];
    strcat(fname, name);
    strcat(fname, "/cmdline");
    FILE *f = fopen(fname,"r");
    if (f == NULL)
	return;
    cmdline[0] = 0;
    fread(cmdline, sizeof(char), 1024, f);
    cmdline[1023] = 0;
    fclose(f);
    if (cmdline[0] == 0)
	return;
    char *p = strrchr(cmdline, '/');
    if (p == NULL)
	p = cmdline;
    else
	p++;
    for (unsigned i=0;i<monitored_count;i++)
	if(0==strcmp(monitored[i],p))
	   data[i] = (char)1;
}

void proc_monitor(char* data) {
    for(unsigned i=0;i<monitored_count;i++)
	data[i] = (char)0;
    rewinddir(proc); 
    struct dirent* e;
    while(NULL != (e = readdir(proc)))
      if(isdigit(e->d_name[0]))
	  check(e->d_name, data);
}

#if 0
char* m[] = { "ls", "sleep" };
char data[2];
main() {
   init_proc_monitor(m,2);
   while(1) {
       proc_monitor(data);
       for (int i=0;i<2;i++)
	  putchar(data[i] ? '1':'0');
       putchar('\n');
   }
}
#endif
