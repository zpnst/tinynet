#include <stdio.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"

static void 
print_dev(adjacency_node_t *dev, const char *open, const char *close, const char *str)
{
    switch (dev->basic_info.dev_type) {
    case ROUTER_T:
        log_msg_prefix(dev->basic_info.dev_name, ANSI_COLOR_PURPLE, open, close, str);
        break;
    case SWITCH_T:
        log_msg_prefix(dev->basic_info.dev_name, ANSI_COLOR_BLUE, open, close, str);
        break;
    case HOST_T:
        log_msg_prefix(dev->basic_info.dev_name, ANSI_COLOR_GRAY, open, close, str);
        break;  
    default:
        panic("incorrect device type");
    }
}

void 
log_msg_prefix(const char *msg_prefix, const char *color, const char *open, const char *close, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s%s%s%s%s", open, color, msg_prefix, ANSI_COLOR_RESET, close);
    vfprintf(stderr, format, args);
    va_end(args);
}

void 
dump_net_links(tinynet_conf_t *net_conf, int enable_net_info) 
{   
    if (enable_net_info) {
        log_msg_prefix("NET NAME", ANSI_COLOR_GREEN, "[", "]", " --> %s", net_conf->net_name);
        printf("\n");

        switch (net_conf->net_type) {
        case MESH_NET_T:
            log_msg_prefix("NET TOPOLOGY", ANSI_COLOR_GREEN, "[", "]", " --> mesh");
            break;
        case BUS_NET_T:
            log_msg_prefix("NET TOPOLOGY", ANSI_COLOR_GREEN, "[", "]", " --> bus");
            break;
        case RING_NET_T:
            log_msg_prefix("NET TOPOLOGY", ANSI_COLOR_GREEN, "[", "]", " --> ring");
            break;
        }
        printf("\n");
        log_msg_prefix("NET DESCRIPTION", ANSI_COLOR_GREEN, "[", "]", " --> %s", net_conf->net_description);
        printf("\n\n");
    }

    adjacency_node_t **list = net_conf->net_graph->adjacency_list;
    size_t dev_c = net_conf->net_graph->rc + net_conf->net_graph->sc + net_conf->net_graph->hc;

    for (size_t iter = 0; iter < dev_c; iter += 1) {

        print_dev(list[iter], "[", "]", " --> ");
        for (adjacency_node_t *child = list[iter]->next; child != NULL; child = child->next) {
            if (child->next) {
                print_dev(child, "(", ")", ", ");
            } else {
                print_dev(child, "(", ")", "");
            }
        } 
        printf("\n");
    }
}