#pragma once
# include "stdint.h"

extern uint8_t  net_mac[6];
extern uint32_t net_ip;        // z.B. 0xC0A80164 für 192.168.1.100
extern uint32_t net_gateway;   // optional
extern uint32_t net_netmask;
