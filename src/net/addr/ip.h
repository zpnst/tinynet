#ifndef IP_H
#define IP_H

#include <stddef.h>
#include "src/tinynet.h"

/** Парсинг IP-адреса из строки */
__int32_t  
parse_ip_addr(ip_addr_t *ip, char *ip_str); 

/** Проверка, находится ли IP-адрес в подсети */
__int32_t 
is_ip_addr_in_subnet(ip_addr_t *ip, ip_addr_t *subnet);

/** Преобразование IP-адреса в строку */
void 
ip_addr_to_string(ip_addr_t *ip, char *buffer, size_t len) ;

#endif /* IP_H */