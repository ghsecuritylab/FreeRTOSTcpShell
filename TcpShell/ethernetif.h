#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "cmsis_os.h"

// VisualGDB doesn't #define this. Are we using a different board rev from them?
#define LAN8742A_PHY_ADDRESS            0x00U

/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);

#endif
