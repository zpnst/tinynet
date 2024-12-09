
#include <stdio.h>
#include <arpa/inet.h>

#include "src/tinynet.h"

int 
parse_mac_addr(mac_addr_t *mac, tinynet_char_t *mac_str)
{
    unsigned int bytes[MAC_ADDRSTRLEN];
    if (sscanf(mac_str, "%x:%x:%x:%x:%x:%x",
               &bytes[0], &bytes[1], &bytes[2],
               &bytes[3], &bytes[4], &bytes[5]) != 6) {
        fprintf(stderr, "Invalid MAC address: %s\n", mac_str);
        return -1;
    }
    for (int i = 0; i < 6; i++) {
        mac->addr[i] = (uint8_t)bytes[i];
    }
    return EXIT_SUCCESS;
}

void 
mac_addr_to_string(mac_addr_t *mac, tinynet_char_t *buffer, size_t len) 
{
    snprintf(buffer, len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac->addr[0], mac->addr[1], mac->addr[2],
             mac->addr[3], mac->addr[4], mac->addr[5]);
}