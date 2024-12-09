#ifndef MAC_H
#define MAC_H

#include <sys/types.h>
#include "src/tinynet.h"

/** Тип MAC-адреса. */
typedef struct mac_addr_s {
    __uint8_t addr[MAC_ADDRSTRLEN]; 
} mac_addr_t;

/** Парсинг MAC-адреса из строки */
int 
parse_mac_addr(mac_addr_t *mac, const tinynet_char_t *mac_str);

/** Преобразование MAC-адреса в строку */
void 
mac_addr_to_string(mac_addr_t *mac, tinynet_char_t *buffer, size_t len);

#endif /* MAC_H */