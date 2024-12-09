#ifndef TINYNET_H
#define TINYNET_H

#include <stdlib.h>


#define YAML_DOC_TITLE "tinynet"

/** Длина MAC-адреса. */
#define MAC_ADDRSTRLEN 6

typedef enum device_t {
    ROUTER_T,
    SWITCH_T,
    HOST_T,
} device_e;

// types

/** Тип беззнакового символа (UTF-8 octet). */
typedef char tinynet_char_t;

/** Тип символа (UTF-8 octet). */
typedef unsigned char tinynet_uchar_t;


// utils

/** Тип IP-адреса. */
typedef struct ip_addr_s {
    __uint32_t addr; 
    __uint8_t mask;  
} ip_addr_t;

/** Тип MAC-адреса. */
typedef struct mac_addr_s {
    __uint8_t addr[MAC_ADDRSTRLEN]; 
} mac_addr_t;


// devices

/** Базовая инфоомация об устройсте. */
typedef struct dev_basic_info_s {
    tinynet_char_t *dev_name;
    tinynet_char_t *dev_type;

    ip_addr_t ip_addr;
    mac_addr_t mac_addr;
} dev_basic_info_t;

/** Абстрактное устройство. */
typedef struct abs_dev_s {
    dev_basic_info_t basic_info;

    device_e dev_type;

    struct abs_dev_s *next;
    struct abs_dev_s *lower_devs_list;      
} abs_dev_t;

/** Тип сетевой топологии. */
typedef struct tinynet_conf_s {
    tinynet_char_t *net_name;
    tinynet_char_t *net_description;

    abs_dev_t *devs;
} tinynet_conf_t;

#endif /** TINYNET_H */