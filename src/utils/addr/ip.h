#ifndef IP_H
#define IP_H

#include <stddef.h>
#include "src/tinynet.h"

/** Парсинг IP-адреса из строки */
int  
parse_ip_addr(ip_addr_t *ip, tinynet_char_t *ip_str); 

/** Проверка, находится ли IP-адрес в подсети */
int 
is_ip_addr_in_subnet(ip_addr_t *ip, ip_addr_t *subnet);

/** Преобразование IP-адреса в строку */
void 
ip_addr_to_string(ip_addr_t *ip, tinynet_char_t *buffer, size_t len) ;

#endif /* IP_H */