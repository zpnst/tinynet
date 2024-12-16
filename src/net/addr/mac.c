
#include <stdio.h>
#include <arpa/inet.h>

#include "src/tinynet.h"

__int32_t 
parse_mac_addr(mac_addr_t *mac, char *mac_str)
{
    __int32_t bytes[MAC_ADDRSTRLEN];
    if (sscanf(mac_str, "%x:%x:%x:%x:%x:%x",
               &bytes[0], &bytes[1], &bytes[2],
               &bytes[3], &bytes[4], &bytes[5]) != 6) {
        fprintf(stderr, "Invalid MAC address: %s\n", mac_str);
        return -1;
    }
    for (__int32_t iter = 0; iter < MAC_ADDRSTRLEN; iter += 1) {
        mac->addr[iter] = (uint8_t)bytes[iter];
    }
    return EXIT_SUCCESS;
}

void 
mac_addr_to_string(mac_addr_t *mac, char *buffer, size_t len) 
{
    snprintf(buffer, len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac->addr[0], mac->addr[1], mac->addr[2],
             mac->addr[3], mac->addr[4], mac->addr[5]);
}

__int32_t
is_mac_exists(mac_addr_t *mac) 
{
    if (!mac->addr[0] && !mac->addr[1] && !mac->addr[2] && !mac->addr[3] && !mac->addr[4] && !mac->addr[5]) {
        return 0;
    }
    return 1;
}