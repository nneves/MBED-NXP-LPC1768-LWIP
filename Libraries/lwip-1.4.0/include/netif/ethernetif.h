#ifndef __NETIF_ETHERNETIF_H__
#define __NETIF_ETHERNETIF_H__

#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void  ethernetif_input(struct netif *netif);

#endif /* __NETIF_ETHERNETIF_H__ */
