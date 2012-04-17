#ifndef COMM_H_
#define COMM_H_

void comm_init(void);
int  comm_test(void);
void comm_put(char);
void comm_puts(const char*);
char comm_get(void);
int comm_txbusy(void);

// for monitor.h/.c:
void xcomm_put(unsigned char);
unsigned char xcomm_get(void);

#endif

