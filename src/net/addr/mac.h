#ifndef MAC_H
#define MAC_H

#include <sys/types.h>
#include "src/tinynet.h"

/** Парсинг MAC-адреса из строки */
int 
parse_mac_addr(mac_addr_t *mac, tinynet_char_t *mac_str);

/** Преобразование MAC-адреса в строку */
void 
mac_addr_to_string(mac_addr_t *mac, tinynet_char_t *buffer, size_t len);

int
is_mac_exists(mac_addr_t *mac);

#endif /* MAC_H */