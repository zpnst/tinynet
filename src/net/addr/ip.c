#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "src/tinynet.h"

int 
parse_ip_addr(ip_addr_t *ip, tinynet_char_t *ip_str) 
{
    char addr_buf[INET_ADDRSTRLEN] = {0};
    char *slash = strchr(ip_str, '/');
    if (slash) {
        strncpy(addr_buf, ip_str, slash - ip_str);
        addr_buf[slash - ip_str] = '\0';
        ip->mask = atoi(slash + 1);
    } else {
        strncpy(addr_buf, ip_str, sizeof(addr_buf) - 1);
        ip->mask = 32;  
    }

    if (inet_pton(AF_INET, addr_buf, &ip->addr) != 1) {
        fprintf(stderr, "incorrect IP address: %s\n", ip_str);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int 
is_ip_addr_in_subnet(ip_addr_t *ip, ip_addr_t *subnet) 
{
    uint32_t mask = htonl(~((1 << (32 - subnet->mask)) - 1)); 
    return (ip->addr & mask) == (subnet->addr & mask);
}

void 
ip_addr_to_string(ip_addr_t *ip, tinynet_char_t *buffer, size_t len) 
{
    struct in_addr addr;
    addr.s_addr = ip->addr;
    snprintf(buffer, len, "%s/%d", inet_ntoa(addr), ip->mask);
}