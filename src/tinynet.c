#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"

size_t 
get_device_count(tinynet_conf_t *net_conf) 
{
    return net_conf->net_graph->rc + net_conf->net_graph->sc + net_conf->net_graph->hc;
}

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
dump_net_links(tinynet_conf_t *net_conf, __int32_t enable_net_info) 
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

const char* 
get_device_name_by_index(tinynet_conf_t *net_conf, __int32_t idx) 
{
    return net_conf->net_graph->adjacency_list[idx]->basic_info.dev_name;
}

__int32_t 
get_device_index_by_name(tinynet_conf_t *net_conf, const char *name) 
{
    size_t vertex = get_device_count(net_conf);
    for (size_t iter = 0; iter < vertex; iter += 1) {
        if (!net_conf->net_graph->adjacency_list[iter]) continue;
        if (net_conf->net_graph->adjacency_list[iter]->basic_info.dev_name &&
            strcmp(net_conf->net_graph->adjacency_list[iter]->basic_info.dev_name, name) == 0) {
            return (__int32_t)iter;
        }
    }
    return EXIT_FAILURE;
}

void 
reconstruct_path(tinynet_conf_t *net_conf, __int32_t iter, __int32_t jter, __int32_t **hops_matrix) 
{
    if (hops_matrix[iter][jter] == -1) {
        panic("incorrect path");
    }
    __int32_t path_size = 0;
    __int32_t path_capacity = 10;

    __int32_t *path = (__int32_t *)panic_alloc(sizeof(__int32_t) * path_capacity);

    path[path_size] = iter;
    path_size += 1;
    __int32_t current = iter;
    while (current != jter) {
        current = hops_matrix[current][jter];
        if (path_size == path_capacity) {
            path_capacity *= 2;
            path = (__int32_t *)realloc(path, sizeof(__int32_t) * path_capacity);
        }
        path[path_size] = current;
        path_size += 1;
    }

    for (__int32_t kter = 0; kter < path_size; kter += 1) {
        printf("%s", get_device_name_by_index(net_conf, path[kter]));
        if (kter < path_size - 1) printf(" --> ");
    }
    printf("\n");

    safety_free(path);
}

void 
dump_shortest_hops(tinynet_conf_t *net_conf)
{
    size_t start_hosts = net_conf->net_graph->rc + net_conf->net_graph->sc;
    size_t end_hosts = start_hosts + net_conf->net_graph->hc;

    for (size_t iter = start_hosts; iter < end_hosts; iter += 1) {
        for (size_t jter = start_hosts; jter < end_hosts; jter += 1) {
            if (iter == jter) continue;

            printf("[PATH] From %s to %s: ", get_device_name_by_index(net_conf, (__int32_t)iter), get_device_name_by_index(net_conf, (__int32_t)jter));
            reconstruct_path(net_conf, (__int32_t)iter, (__int32_t)jter, net_conf->hops_matrix);
            
        }
    }
}