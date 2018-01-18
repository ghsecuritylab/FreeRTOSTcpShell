#pragma once

#define SERVER_PORT       23
#define MAX_CONNECTIONS   4
#define MAX_HOSTNAME_LEN  128
#define MAX_MACADDR_LEN   6
 
extern volatile unsigned int conns;
extern unsigned int max_conns;
extern const char hostname[MAX_HOSTNAME_LEN];
extern const char macaddress[MAX_MACADDR_LEN];

void tcpserver_init(int port, int maxConns);
