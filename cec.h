#ifndef _CEC_H
#define _CEC_H

typedef void (*KeyCallback)(unsigned key, unsigned pressed);
int init_cec(KeyCallback cb);
void end_cec(void);
void cec_update(void);

#endif
