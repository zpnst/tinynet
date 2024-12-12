#include <stdio.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"

static void 
handle_device_type(device_e device_type, tinynet_char_t *buffer, size_t len) 
{
    if (device_type == ROUTER_T) {
        snprintf(buffer, len, "<rout>");
    } else if (device_type == SWITCH_T) {
        snprintf(buffer, len, "<swit>");
    } else if (device_type == HOST_T) {
        snprintf(buffer, len, "<host>");
    } else {
        panic("wrong device type");
    }
} 

static void 
handle_net_type(net_types_e net_type, tinynet_char_t *buffer, size_t len) 
{
    if (net_type == MESH_NET_T) {
        snprintf(buffer, len, "mesh");
    } else if (net_type == RING_NET_T) {
        snprintf(buffer, len, "ring");
    } else if (net_type == BUS_NET_T) {
        snprintf(buffer, len, "bus");
    } else {
        panic("wrong net type");
    }
} 


void 
log_msg_prefix(const char *msg_prefix, const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[%s%s%s] ", color, msg_prefix, ANSI_COLOR_RESET);
    vfprintf(stderr, format, args);
    va_end(args);
}

void 
dump_net_conf(tinynet_conf_t *net_conf) 
{   
    tinynet_char_t ip_buffer[IP_BUFFER_S];
    tinynet_char_t mac_buffer[MAC_BUFFER_S];

    tinynet_char_t net_type_buffer[NET_T_BUFFER_S];
    tinynet_char_t dev_type_buffer[DEVICE_T_BUFFER_S];

    handle_net_type(net_conf->net_type, net_type_buffer, sizeof(net_type_buffer));

    printf("[%s] type: '%s'\n[%s] name: '%s'\n[%s] description: '%s'\n\n", 
    COLORFY_GREEN("NET"), 
    net_type_buffer, 
    COLORFY_GREEN("NET"), 
    net_conf->net_name, 
    COLORFY_GREEN("NET"), 
    net_conf->net_description
    );

    for (abs_dev_t *w = net_conf->devs; w; w = w->next) {
        
        ip_addr_to_string(&w->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
        mac_addr_to_string(&w->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
        handle_device_type(w->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

        printf("[%s] name: %s, type: %s, ip: %s, mac: %s\n", COLORFY_PURPLE("ROUT"), w->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);

        for (abs_dev_t *l = w->lower_devs_list; l; l = l->next) {
            
            ip_addr_to_string(&l->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
            mac_addr_to_string(&l->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
            handle_device_type(l->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

            printf("[%s]     name: %s, type: %s, ip: %s, mac: %s\n", COLORFY_BLUE("SWIT"), l->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);

            for (abs_dev_t *h = l->lower_devs_list; h; h = h->next) {
                
                ip_addr_to_string(&h->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
                mac_addr_to_string(&h->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
                handle_device_type(h->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

                printf("[%s]         name: %s, type: %s, ip: %s, mac: %s\n", COLORFY_GRAY("HOST"), h->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);
            }
        }
    }
}