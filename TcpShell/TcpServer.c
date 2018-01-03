
/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/app_ethernet.c 
  * @author  MCD Application Team
  * @brief   Ethernet specefic module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <lwip/api.h>
#include <lwip/opt.h>
#include <lwip/dhcp.h>
#include <lwip/tcpip.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <queue.h>
#include "tcpshell.h"
#include "User.h"
#include "ethernetif.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/*Static IP ADDRESS*/
#define IP_ADDR0   169
#define IP_ADDR1   254
#define IP_ADDR2   152
#define IP_ADDR3   100
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   0
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   169
#define GW_ADDR1   254
#define GW_ADDR2   0
#define GW_ADDR3   1 

/*DHCP states*/
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_DHCP
#define MAX_DHCP_TRIES  4
__IO uint8_t DHCP_state = DHCP_OFF;
#endif
struct netif gnetif; /* network interface structure */
static int Port = 0;
static unsigned int MaxConns = 0;
static volatile unsigned int conns = 0;
static const size_t recvsize = 128;

/* Private function prototypes -----------------------------------------------*/
static void TcpThread(void const * argument);
static void ServerThread(void const * argument);
static void ConnectionThread(void const * argument);
static void DHCP_thread(void const * argument);
static void User_notification(struct netif *netif);
static void Netif_Config(void);

void TcpInit(int port, int maxConns)
{
	assert(port > 0 && maxConns > 0);
	Port = port;
	MaxConns = maxConns;
	
	/* Init thread */
#if defined(__GNUC__)
	osThreadDef(Tcp, TcpThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5);
#else
	osThreadDef(Tcp, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
#endif
  
	osThreadCreate(osThread(Tcp), NULL);
}

int TcpPutchar(char ch)
{
	PUserContext context = pvTaskGetThreadLocalStoragePointer(NULL, TLS_USER_CONTEXT);
	assert(context);
	if (context)
	{
		err_t err;
		// Send the char
		err = netconn_write(context->conn, &ch, sizeof(ch), NETCONN_NOCOPY);
		if (ERR_IS_FATAL(err))
		{
			dprintf("%d: netconn_write failed: %d (%s)\n", context->connid, err, lwip_strerr(err));
		}
	
		return err;
	}
	
	return -1;
}

int TcpGetchar()
{
	PUserContext context = pvTaskGetThreadLocalStoragePointer(NULL, TLS_USER_CONTEXT);
	assert(context);
	if (context)
	{
		if (context->buf && context->remaining > 0)
		{
			char ch;
			--context->remaining;
			ch = *context->ptr++;
			return ch;
		}
		else
		{
			err_t err;
	
			if (context->buf)
			{
				netbuf_delete(context->buf);
				context->buf = NULL;
			}
		
			// Read more...
			err = netconn_recv(context->conn, &context->buf);
			if (err == ERR_OK) 
			{
				netbuf_data(context->buf, (void**)&context->ptr, &context->remaining);
			}
			else
			{
				if (ERR_IS_FATAL(err))
				{
					dprintf("%d: netconn_recv failed: %d (%s)\n", context->connid, err, lwip_strerr(err));	
				}
			
				return err;
			}
		
			return TcpGetchar(context);
		}
	}
	
	return -1;
}

/* Private functions ---------------------------------------------------------*/
void TcpThread(void const* argument)
{
	/* Create tcp_ip stack thread */
	tcpip_init(NULL, NULL);
  
	/* Initialize the LwIP stack */
	Netif_Config();
  
	/* Initialize server thread */
	osThreadDef(TcpServer, ServerThread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate(osThread(TcpServer), &gnetif);
  
	/* Notify user about the network interface config */
	User_notification(&gnetif);
  
#ifdef USE_DHCP
	/* Start DHCPClient */
	osThreadDef(DHCP, DHCP_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE * 2);
	osThreadCreate(osThread(DHCP), &gnetif);
#endif

	for (;;)
	{
		/* Delete the Init Thread */ 
		osThreadTerminate(NULL);
	}
}

void ServerThread(void const * argument)
{
	struct netconn *conn, *newconn;
	err_t err, accept_err;
  
	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);
  
	if (conn != NULL)
	{
		/* Bind to port 80 (HTTP) with default IP address */
		err = netconn_bind(conn, NULL, Port);
    
		if (err == ERR_OK)
		{
			/* Put the connection into LISTEN state */
			netconn_set_recvtimeout(conn, 1000);
			netconn_set_recvbufsize(conn, 1 << 12);
			netconn_listen(conn);
  
			for (;;)
			{
				/* accept any icoming connection */
				accept_err = netconn_accept(conn, &newconn);
				if (accept_err == ERR_OK)
				{
					assert(conns >= 0 && conns <= MaxConns);
					if (conns == MaxConns)
					{
						/* delete connection... we're full */
						netconn_close(newconn);
						netconn_delete(newconn);
					}
					else
					{
						PUserContext newContext = pvPortMalloc(sizeof(UserContext));
						if (newContext)
						{
							memset(newContext, 0, sizeof(UserContext));
							newContext->conn = newconn;
							newContext->connid = conns;
							netconn_set_recvbufsize(newconn, recvsize);
							++conns;
							osThreadDef(Connection, ConnectionThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2);
							osThreadCreate(osThread(Connection), newContext);
						}
					}
				}
				else if (accept_err != ERR_TIMEOUT)
				{
					dprintf("netconn_accept failed: %d", accept_err);
					LedError(ErrorCodeNetconnAcceptFailure);
				}
			}
		}
	}
}

void ConnectionThread(void const * argument)
{
	PUserContext context = (PUserContext)argument;
	err_t err;
		
	u32_t remoteaddr = context->conn->pcb.ip->remote_ip.addr;
	dprintf("%d: Accepting connection from %lu.%lu.%lu.%lu\n", 
		context->connid,
		remoteaddr & 0xFF,
		(remoteaddr >> 8) & 0xFF,
		(remoteaddr >> 16) & 0xFF,
		(remoteaddr >> 24) & 0xFF);
	
	// Pretty simple state machine that just keeps running apps until no more apps are selected
	context->NextApp = &EchoApp;
	vTaskSetThreadLocalStoragePointer(NULL, TLS_USER_CONTEXT, context);
	
	// Go through telnet negotiation
	
	
	do
	{
		PUserApp nextApp = context->NextApp;
		context->NextApp = NULL;
		int err = nextApp->Run(context);
		dprintf("%d: %s exit code %d\n", context->connid, nextApp->AppName, err);
	} while (context->NextApp);
	
	dprintf("%d: Closing connection. netconn err=%d (%s)\n", context->connid, netconn_err(context->conn), lwip_strerr(netconn_err(context->conn)));
	--conns;
	assert(conns >= 0);
	
	if (context->conn)
	{
		if (netconn_err(context->conn) == ERR_OK)
		{
			// Only close if the conn is still open
			netconn_close(context->conn);	
		}
		
		netconn_delete(context->conn);
	}
	
	if (context->buf)
	{
		netbuf_delete(context->buf);
	}
	
	vPortFree(context);
	
	for (;;)
	{
		/* Delete the Init Thread */ 
		osThreadTerminate(NULL);
	}
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
static void Netif_Config(void)
{
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
 
#ifdef USE_DHCP
	ip_addr_set_zero_ip4(&ipaddr);
	ip_addr_set_zero_ip4(&netmask);
	ip_addr_set_zero_ip4(&gw);
#else
	IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
	IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif /* USE_DHCP */
  
	/* add the network interface */    
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  
	/*  Registers the default network interface. */
	netif_set_default(&gnetif);
  
	if (netif_is_link_up(&gnetif))
	{
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&gnetif);
	}
	else
	{
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}
}

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
void User_notification(struct netif *netif) 
{
	if (netif_is_up(netif))
	{
#ifdef USE_DHCP
		/* Update DHCP state machine */
		DHCP_state = DHCP_START;
#else
		/* Turn On LED 1 to indicate ETH and LwIP init success*/
		dprintf("ETH and LWIP init success\n");
#endif /* USE_DHCP */
	}
	else
	{  
#ifdef USE_DHCP
		/* Update DHCP state machine */
		DHCP_state = DHCP_LINK_DOWN;
#endif  /* USE_DHCP */
		/* Turn On LED 3 to indicate ETH and LwIP init error */
		dprintf("ETH and LWIP init error\n");
		LedError(ErrorCodeEthAndLwipInit);
	} 
}

#ifdef USE_DHCP
/**
* @brief  DHCP Process
* @param  argument: network interface
* @retval None
*/
void DHCP_thread(void const * argument)
{
	struct netif *netif = (struct netif *) argument;
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;
	struct dhcp *dhcp;
  
	for (;;)
	{
		switch (DHCP_state)
		{
		case DHCP_START:
			{
				ip_addr_set_zero_ip4(&netif->ip_addr);
				ip_addr_set_zero_ip4(&netif->netmask);
				ip_addr_set_zero_ip4(&netif->gw);       
				dhcp_start(netif);
				DHCP_state = DHCP_WAIT_ADDRESS;
			}
			break;
      
		case DHCP_WAIT_ADDRESS:
			{                
				if (dhcp_supplied_address(netif)) 
				{
					DHCP_state = DHCP_ADDRESS_ASSIGNED;	
          
					LedError(ErrorCodeNone);
					u32_t addr = netif->ip_addr.addr;
					dprintf("DHCP address changed to %lu.%lu.%lu.%lu\n", 
						addr & 0xFF,
						(addr >> 8) & 0xFF,
						(addr >> 16) & 0xFF,
						(addr >> 24) & 0xFF);
				}
				else
				{
					dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
					/* DHCP timeout */
					if (dhcp->tries > MAX_DHCP_TRIES)
					{
						DHCP_state = DHCP_TIMEOUT;
            
						/* Stop DHCP */
						dhcp_stop(netif);
            
						/* Static address used */
						IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
						IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
						IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
						netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));

						LedError(ErrorCodeDhcpTimeout);
						dprintf("DHCP timeout. Defaulting to %d.%d.%d.%d instead.\n", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
					}
					else
					{
						LedError(ErrorCodeNone);
					}
				}
			}
			break;
		case DHCP_LINK_DOWN:
			{
				/* Stop DHCP */
				dhcp_stop(netif);
				DHCP_state = DHCP_OFF; 
			}
			break;
		default: break;
		}
    
		/* wait 250 ms */
		osDelay(250);
	}
}
#endif  /* USE_DHCP */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
