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

void 
dump_net_conf(tinynet_conf_t *net_conf) 
{   
    tinynet_char_t ip_buffer[IP_BUFFER_S];
    tinynet_char_t mac_buffer[MAC_BUFFER_S];
    tinynet_char_t dev_type_buffer[DEVICE_T_BUFFER_S];

    printf("[NET] name: '%s'\n[NET] description: '%s'\n\n", net_conf->net_name, net_conf->net_description);
    for (abs_dev_t *w = net_conf->devs; w; w = w->next) {
        
        ip_addr_to_string(&w->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
        mac_addr_to_string(&w->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
        handle_device_type(w->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

        printf("[ROUT] name=%s, type=%s, ip=%s, mac=%s\n", w->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);

        for (abs_dev_t *l = w->lower_devs_list; l; l = l->next) {
            
            ip_addr_to_string(&l->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
            mac_addr_to_string(&l->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
            handle_device_type(l->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

            printf("[SWIT]     name=%s, type=%s, ip=%s, mac=%s\n", l->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);

            for (abs_dev_t *h = l->lower_devs_list; h; h = h->next) {
                
                ip_addr_to_string(&h->basic_info.ip_addr, ip_buffer, sizeof(ip_buffer));
                mac_addr_to_string(&h->basic_info.mac_addr, mac_buffer, sizeof(mac_buffer));
                handle_device_type(h->basic_info.dev_type, dev_type_buffer, sizeof(dev_type_buffer));

                printf("[HOST]         name=%s, type=%s, ip=%s, mac=%s\n",h->basic_info.dev_name, dev_type_buffer, ip_buffer, mac_buffer);
            }
        }
    }
}