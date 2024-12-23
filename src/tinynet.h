#ifndef TINYNET_H
#define TINYNET_H

#include <stdlib.h>

// #define DEBUG_STATE_MACHINE

#define YAML_DOC_TITLE "tinynet"

#define ENANBLE_NET_INFO 1
#define DISABLE_NET_INFO 0

#define ENANBLE_NET_INFO 1
#define DISABLE_NET_INFO 0

#define NET_T_BUFFER_S 5
#define DEVICE_T_BUFFER_S 7

#define ROS_ENTRY_BUFFER_S 256

void 
log_msg_prefix(const char *msg_prefix, const char *color, const char *open, const char *close, const char *format, ...);

/** ANSI COLORS */
#define ANSI_COLOR_RED       "\x1b[31m"
#define ANSI_COLOR_GRAY      "\x1b[37m"
#define ANSI_COLOR_GREEN     "\x1b[32m"
#define ANSI_COLOR_BLUE      "\x1b[34m"
#define ANSI_COLOR_PURPLE    "\x1b[35m"
#define ANSI_COLOR_RESET     "\x1b[0m"

#define COLORFY_BLUE(string) ANSI_COLOR_BLUE string ANSI_COLOR_RESET
#define COLORFY_GRAY(string) ANSI_COLOR_GRAY string ANSI_COLOR_RESET
#define COLORFY_GREEN(string) ANSI_COLOR_GREEN string ANSI_COLOR_RESET
#define COLORFY_PURPLE(string) ANSI_COLOR_PURPLE string ANSI_COLOR_RESET

#define LOG_PANIC_PREFIX(format, ...) log_msg_prefix("PANIC", ANSI_COLOR_RED, "[", "] ", format, ##__VA_ARGS__)
#define LOG_ERROR_PREFIX(format, ...) log_msg_prefix("ERR", ANSI_COLOR_RED, "[", "] ", format, ##__VA_ARGS__)
#define LOG_WARNING_PREFIX(format, ...) log_msg_prefix("WARN", ANSI_COLOR_PURPLE, "[", "] ", format, ##__VA_ARGS__)

typedef enum device_t {
    ROUTER_T,
    SWITCH_T,
    HOST_T,
} device_e;

typedef enum net_types_s {
    MESH_NET_T,
    RING_NET_T,
    BUS_NET_T,
} net_types_e;


/** Базовая инфоомация об устройсте. */
typedef struct dev_basic_info_s {
    device_e dev_type;
    char *dev_name;
} dev_basic_info_t;

/** Абстрактное устройство. */
typedef struct abs_dev_s {
    dev_basic_info_t basic_info;

    struct abs_dev_s *next;
    struct abs_dev_s *lower_devs_list;      
} abs_dev_t;

typedef enum machine_status_e {
    SUCCESS = 1,
    FAILURE = 0
} machine_status_t;

typedef struct adjacency_node_s {
    __uint8_t weight;    
    dev_basic_info_t basic_info;                
    struct adjacency_node_s *next;
} adjacency_node_t;

typedef struct net_graph_s {
    size_t rc;                            /** Number of routers. */
    size_t sc;                            /** Number of switches. */
    size_t hc;                            /** Number of hosts. */
     
    adjacency_node_t **adjacency_list;  
} net_graph_t;

typedef struct char_node_s {
    char *entry;
    struct char_node_s *next; 
} char_node_t;

typedef struct hops_matrix_cell_s {
    __int32_t data;
    __uint32_t weight;  
} hops_matrix_cell_t;

/** Тип сетевой топологии. */
typedef struct tinynet_conf_s {
    net_types_e net_type;

    char *net_name;
    char *net_description;

    /** Порядок устройств в списке тут такой же, как и в списке смежности */
    char_node_t **ros_tables_list;     

    net_graph_t *net_graph;
    hops_matrix_cell_t **hops_matrix;

} tinynet_conf_t;

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
    STATE_NETTYPE,
    STATE_NETNAME,   
    STATE_NETDESCRIPTION,  
    STATE_WANLIST, 

    /** Routers states */
    STATE_WANVALUES,  
    STATE_WANKEYS,  

    STATE_ROUTERNAME,  
    STATE_LANLIST,  
 
    /** Switches states */
    STATE_LANVALUES,  
    STATE_LANKEYS, 

    STATE_SWITCHNAME, 
    STATE_HOSTSLIST,

    /** Host states */
    STATE_HOSTVALUES, 
    STATE_HOSTKEYS, 

    STATE_HOSTNAME,

    /** End state */
    STATE_STOP     
} machine_states_t;

void 
dump_net_links(tinynet_conf_t *network, __int32_t enable_net_info);

void
destroy_net_conf(tinynet_conf_t *network);

__int32_t 
graph_by_config(tinynet_conf_t **network);

size_t 
get_device_count(tinynet_conf_t *network);

const char* 
get_device_name_by_index(tinynet_conf_t *network, __int32_t idx);

__int32_t 
get_device_index_by_name(tinynet_conf_t *network, const char *name);

void 
reconstruct_path(tinynet_conf_t *network, __int32_t iter, __int32_t jter);

void 
init_ros_tables(tinynet_conf_t *network);

void 
dump_shortest_hops(tinynet_conf_t *network);

void 
dump_ros_tables(tinynet_conf_t *network);

void 
add_to_ros_table(tinynet_conf_t *network, __int32_t iter, __int32_t jter, __int32_t kter);

#endif /** TINYNET_H */
