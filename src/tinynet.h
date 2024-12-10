#ifndef TINYNET_H
#define TINYNET_H

#include <stdlib.h>

// #define DEBUG_STATE_MACHINE

#define YAML_DOC_TITLE "tinynet"

#define SHOW_IP 1
#define DONT_SHOW_IP 0

#define SHOW_MAC 1
#define DONT_SHOW_MAC 0

/** Длина MAC-адреса. */
#define MAC_ADDRSTRLEN 6

#define IP_BUFFER_S 19
#define MAC_BUFFER_S 18
#define DEVICE_T_BUFFER_S 7

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
    device_e dev_type;
    tinynet_char_t *dev_name;

    ip_addr_t ip_addr;
    mac_addr_t mac_addr;
} dev_basic_info_t;

/** Абстрактное устройство. */
typedef struct abs_dev_s {
    dev_basic_info_t basic_info;

    struct abs_dev_s *next;
    struct abs_dev_s *lower_devs_list;      
} abs_dev_t;

/** Тип сетевой топологии. */
typedef struct tinynet_conf_s {
    tinynet_char_t *net_name;
    tinynet_char_t *net_description;

    abs_dev_t *devs;
} tinynet_conf_t;


typedef enum machine_status_e {
    SUCCESS = 1,
    FAILURE = 0
} machine_status_t;

/** Network configuratin
 *  parser state machine states. */
typedef enum machine_states_e {

    /** Libyaml states */
    STATE_START,   
    STATE_STREAM,  
    STATE_DOCUMENT,
    STATE_SECTION, 

    /** Network metadata states */ 
    STATE_NETVALUES,
    STATE_NETKEYS,
    STATE_NETNAME,   // 6
    STATE_NETDESCRIPTION,  // 7
    STATE_WANLIST,  // 8

    /** Routers states */
    STATE_WANVALUES,  // 9
    STATE_WANKEYS,  // 10

    STATE_ROUTERNAME,  // 11
    STATE_ROUTERIP,  // 12
    STATE_ROUTERMAC,  // 13
    STATE_LANLIST,  // 14
 
    /** Switches states */
    STATE_LANVALUES,  // 15
    STATE_LANKEYS,  // 16

    STATE_SWITCHNAME,  // 17
    STATE_SWITCHIP, // 18
    STATE_SWITCHMAC, // 19
    STATE_HOSTSLIST, // 20

    /** Host states */
    STATE_HOSTVALUES, // 21
    STATE_HOSTKEYS, // 22

    STATE_HOSTNAME, // 23
    STATE_HOSTIP, // 24
    STATE_HOSTMAC, // 25

    /** End state */
    STATE_STOP     
} machine_states_t;

/** parser state. */
typedef struct parser_state_s {
    machine_states_t state;                         /** The current parse state */

    dev_basic_info_t host;                    /** Host buffer. */
    dev_basic_info_t switch_;                     /** LAN(switch) buffer. */
    dev_basic_info_t router;                     /** WAN(router) buffer. */

    abs_dev_t *host_list;                     /** Temporary buffer for each LAN. */
    abs_dev_t *lans_list;                     /** Temporary buffer for each WAN. */

    abs_dev_t *wans_list;                     /** Master list of WANs. */

    tinynet_char_t *net_conf_name;           /** Master network configuration name. */
    tinynet_char_t *net_conf_description;    /** Master network configuration description. */
} parser_state_t;

void 
dump_net_conf(tinynet_conf_t *net_conf);

void
destroy_net_conf(tinynet_conf_t *net_conf);

#endif /** TINYNET_H */