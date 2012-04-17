#include "LPC17xx.h"

#include "comm.h"
#include "monitor.h"
#include "lpc17xx_gpio.h"

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"

#include "netif/ethernetif.h"
#include "netif/etharp.h"

#include <string.h>

// leds for status
#define LED_OK		( 1UL << 25UL )
#define LED_NOK		( 1UL << 24UL )
#define LED_ON 	    1
#define LED_OFF 	0

static struct netif  netif_eth0_data;
static struct netif* netif_eth0 = &netif_eth0_data;

static struct ip_addr my_ipaddr_data;
static struct ip_addr my_netmask_data;
static struct ip_addr my_gw_data;

static u32_t         last_arp_time;
static u32_t         last_tcpslow_time;
static u32_t         last_tcpfast_time;
static u32_t         last_tcp_time;
static u32_t         light_on, light_off;

// NNeves
static void client_close(struct tcp_pcb *pcb)
{
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_close(pcb);
}

static err_t client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	// dummy - clear compiler warning
	u16_t dummy = len;
	dummy = 0;

	LWIP_UNUSED_ARG(arg);
	client_close(pcb);
	return ERR_OK;
}

static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{ 
// NOTE: haven't finished porting c++ code to c, recv is not working yet!  
/*	
   String str;
   int length = 0;

   int i;
   //conn=1;
   LWIP_UNUSED_ARG(arg);   
   
   //tcp_output(pcb)
   if (err == ERR_OK && p != NULL)
   {
       tcp_recved(pcb, p->tot_len);

       str = p->payload;
       length = p->len;
       tcp_write(pcb,str, length, 0);
	   printf("\nserver_recv(): Incoming string is %s\n", string);

       printf("\nserver_recv(): String length is %d byte\n", length);
       printf("server_recv(): pbuf->len is %d byte\n", p->len);
       printf("server_recv(): pbuf->tot_len is %d byte\n", p->tot_len);
       printf("server_recv(): pbuf->next is %d\n", p->next);
     
     for(i=0;i<length;i++)
     {
       stringdisp[i]=str[i];
     }
     i=0;
     for(i=length;i<11;i++)
     {
       stringdisp[i]=' ';
     }
     i=0;
     pbuf_free(p);
     if(stringdisp[0]=='f'&&stringdisp[1]=='i'&&stringdisp[2]=='n')
     {
	     for(i=0;i<11;i++)
	     {
	       stringdisp[i]=' ';
	     }
     
     	server_close(pcb);
      
	    pcb=tcp_new();
	    tcp_bind(pcb,IP_ADDR_ANY,23);
	    pcb = tcp_listen(pcb);
	    tcp_accept(pcb,server_accept);
     }
       
   }
   else
   {
       printf("\nserver_recv(): Errors-> ");
       if (err != ERR_OK)
         printf("1) Connection is not on ERR_OK state, but in %d state->\n", err);

       if (p == NULL)
         printf("2) Pbuf pointer p is a NULL pointer->\n ");

       printf("server_recv(): Closing server-side connection...");

       pbuf_free(p);
       server_close(pcb);
   }
*/   
   return ERR_OK;   
}

static err_t client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
	char *string = "Hello World\r\n";
	LWIP_UNUSED_ARG(arg);
	//l++;
	if (err != ERR_OK)
	{
	}
	else
	{
		tcp_sent(pcb, client_sent);
		tcp_write(pcb, string,/*sizeof(string)*/13,0);	
		tcp_recv(pcb, server_recv);
	}
	return err;
}

volatile u32_t systick_counter;				/* counts 1ms timeTicks */

/* SysTick Interrupt Handler (1ms) */
void SysTick_Handler (void)
{
	systick_counter++;
}

void Delay (uint32_t dlyTicks) {
	uint32_t curTicks;

	curTicks = systick_counter;
	while ((systick_counter - curTicks) < dlyTicks);
}

static void lwip_init(void)
{
	mem_init();

	memp_init();

	pbuf_init();

	etharp_init();

	ip_init();

	udp_init();

	tcp_init();

	IP4_ADDR(&my_ipaddr_data,  192, 168, 1, 100);
	IP4_ADDR(&my_netmask_data, 255, 255, 255, 0);
	IP4_ADDR(&my_gw_data, 192, 168, 1, 254);

	netif_add(netif_eth0, &my_ipaddr_data, &my_netmask_data, &my_gw_data, NULL, ethernetif_init, ethernet_input);

	netif_set_default(netif_eth0);

	netif_set_up(netif_eth0);
}

int main (int argc, char **argv)
{
	int i;
	uint32_t curTicks;

	// NNeves
	struct tcp_pcb *pcb;
	struct ip_addr ip_computer;
	unsigned long ulIPAddress,ulIPAddrComputer;

	// for output throw serial with xprintf
	comm_init();
	xfunc_out = xcomm_put;
	xfunc_in  = xcomm_get;

	// LEDs init
	LPC_SC->PCONP |= ( 1 << 15 ); // power up GPIO
	/* LEDs on port 1. */
	LPC_GPIO1->FIODIR  = (LED_OK | LED_NOK);
	GPIO_ClearValue(1,LED_OK);
	GPIO_SetValue(1,LED_NOK);

	SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 ms   */

	GPIO_ClearValue(1,LED_NOK);

	lwip_init();

	light_on = light_off = last_arp_time = last_tcpslow_time = last_tcpfast_time = last_tcp_time = systick_counter;

 	// NNeves
 	pcb=tcp_new();
	ulIPAddrComputer=0xC0A8011E;//address of your computer - hex code for 192.168.1.30
	tcp_bind(pcb,IP_ADDR_ANY,80);
	ip_computer.addr =htonl(ulIPAddrComputer);
	//htonl=change litle endian-big endian to format IP
	tcp_connect(pcb,&ip_computer,80,client_connected);
	

	while(1) {
		curTicks = systick_counter;

		// process packets that arrive in the interface
		ethernetif_input(netif_eth0);

		// functions that should call from times to times
		if (curTicks - last_arp_time > ETHARP_TMR_INTERVAL)
		{
			etharp_tmr();
			last_arp_time = curTicks;
		}

		if (curTicks - last_tcpfast_time > TCP_FAST_INTERVAL)
		{
			last_tcpfast_time = curTicks;
			tcp_fasttmr();			
		}

		if (curTicks - last_tcpslow_time > TCP_SLOW_INTERVAL)
		{
			last_tcpslow_time = curTicks;
			tcp_slowtmr();			
		}
		
		if (curTicks - light_off > 500) {
			GPIO_ClearValue(1,LED_OK);
		}

		if (curTicks - light_on > 1000) {
			GPIO_SetValue(1,LED_OK);
			light_on = curTicks;
			light_off = curTicks;
		}

		// NNeves
		if (curTicks - last_tcp_time > TCP_TMR_INTERVAL)
		{
			last_tcp_time = curTicks;
			tcp_tmr();			
		}		
	}

	return 0;
}

void startup_delay(void)
{
	for (volatile unsigned long i = 0; i < 500000; i++) { ; }
}
