#include <stdio.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"

static void 
handle_device_type(device_e device_type, char *buffer, size_t len) 
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
handle_net_type(net_types_e net_type, char *buffer, size_t len) 
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

// static void
// print_char() {
//     if (child->next) {
//             printf("(%s), ", child->basic_info.dev_name);
//         } else {
//             printf("(%s)", child->basic_info.dev_name);
//         }
// }

void 
dump_net_links(tinynet_conf_t *net_conf) 
{   
    adjacency_node_t **list = net_conf->net_graph->adjacency_list;
    size_t dev_c = net_conf->net_graph->rc + net_conf->net_graph->sc + net_conf->net_graph->hc;

    for (size_t iter = 0; iter < dev_c; iter += 1) {

        printf("[%s] --> ", list[iter]->basic_info.dev_name);
        for (adjacency_node_t *child = list[iter]->next; child != NULL; child = child->next) {

            switch (child->basic_info.dev_type) {
            case ROUTER_T:
                /* code */
                break;
            case SWITCH_T:
                /* code */
                break;
            case HOST_T:
                /* code */
                break;
            
            default:
                break;
            }
        }
        printf("\n");
    }
}