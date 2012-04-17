/*
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIP_LWIOPTS_H__
#define __LWIP_LWIOPTS_H__

#include <sys/time.h>

/* Align memory on 4 byte boundery (32-bit) */
#define MEM_ALIGNMENT                   4

/* No operating system present */
#define NO_SYS                          1

/* Time in milliseconds to perform ARP processing */
#define ETHARP_TMR_INTERVAL             5000
#define LWIP_TIMEVAL_PRIVATE            0
#define LWIP_SOCKET                     1
#define LWIP_NETCONN                    1
#define LWIP_ARP                        1
//#define MEMP_NUM_PBUF                   1
//#define MEMP_NUM_RAW_PCB                1
//#define MEMP_NUM_UDP_PCB                1
//#define MEMP_NUM_TCP_PCB                1
//#define MEMP_NUM_TCP_PCB_LISTEN         0
//#define MEMP_NUM_TCP_SEG                8
//#define MEMP_NUM_NETBUF                 0
//#define MEMP_NUM_NETCONN                2
//#define MEMP_NUM_API_MSG                0
//#define MEMP_NUM_TCPIP_MSG              0
#define PBUF_POOL_SIZE                  16 // reduz consideravelmente utilização de memória
#define PBUF_POOL_BUFSIZE               64 // reduz consideravelmente utilização de memória
//#define ARP_TABLE_SIZE                  2
#define IP_REASS_BUFSIZE                600
//#define TCP_WND                         1024
//#define TCP_QUEUE_OOSEQ                 0


#endif /* __LWIP_LWIOPTS_H__ */
