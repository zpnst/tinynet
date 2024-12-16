#ifndef MAC_H
#define MAC_H

#include <sys/types.h>
#include "src/tinynet.h"

/** Парсинг MAC-адреса из строки */
__int32_t 
parse_mac_addr(mac_addr_t *mac, char *mac_str);

/** Преобразование MAC-адреса в строку */
void 
mac_addr_to_string(mac_addr_t *mac, char *buffer, size_t len);

__int32_t
is_mac_exists(mac_addr_t *mac);

#endif /* MAC_H */