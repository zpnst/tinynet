#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"


static __int32_t
get_path(tinynet_conf_t *network, __int32_t **ctx_path, __int32_t iter, __int32_t jter)
{
    __int32_t **hops_matrix = network->hops_matrix;

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
            if (!path) {
                panic("out of memory");
            }
        }
        path[path_size] = current;
        path_size += 1;
    }
    (*ctx_path) = path;
    return path_size;
}

static __int32_t 
if_dest_exists(tinynet_conf_t *network, __int32_t dest, __int32_t kter) 
{
    const char *device_name = get_device_name_by_index(network, dest);
    
    char_node_t *ctx = network->ros_tables_list[kter];
    while (ctx) {
        if (strstr(ctx->entry, device_name)) {
            return 1;
        }
        ctx = ctx->next;
    }

    return 0;
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

size_t 
get_device_count(tinynet_conf_t *network) 
{
    return network->net_graph->rc + network->net_graph->sc + network->net_graph->hc;
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
dump_net_links(tinynet_conf_t *network, __int32_t enable_net_info) 
{   
    if (enable_net_info) {
        log_msg_prefix("NET NAME", ANSI_COLOR_GREEN, "[", "]", " --> %s", network->net_name);
        printf("\n");

        switch (network->net_type) {
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
        log_msg_prefix("NET DESCRIPTION", ANSI_COLOR_GREEN, "[", "]", " --> %s", network->net_description);
        printf("\n\n");
    }

    adjacency_node_t **list = network->net_graph->adjacency_list;
    size_t dev_c = network->net_graph->rc + network->net_graph->sc + network->net_graph->hc;

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
get_device_name_by_index(tinynet_conf_t *network, __int32_t idx) 
{
    return network->net_graph->adjacency_list[idx]->basic_info.dev_name;
}

__int32_t 
get_device_index_by_name(tinynet_conf_t *network, const char *name) 
{
    size_t vertex = get_device_count(network);
    for (size_t iter = 0; iter < vertex; iter += 1) {
        if (!network->net_graph->adjacency_list[iter]) continue;
        if (network->net_graph->adjacency_list[iter]->basic_info.dev_name &&
            strcmp(network->net_graph->adjacency_list[iter]->basic_info.dev_name, name) == 0) {
            return (__int32_t)iter;
        }
    }
    return EXIT_FAILURE;
}

void 
reconstruct_path(tinynet_conf_t *network, __int32_t iter, __int32_t jter) 
{   
    __int32_t *ctx_path = NULL;
    __int32_t path_size = get_path(network, &ctx_path, iter, jter);

    for (__int32_t kter = 0; kter < path_size; kter += 1) {
        printf("%s", get_device_name_by_index(network, ctx_path[kter]));
        if (kter < path_size - 1) printf(" --> ");
    }
    printf("\n");

    safety_free(ctx_path);
}

void 
add_to_ros_table(tinynet_conf_t *network, __int32_t iter, __int32_t jter, __int32_t kter)
{

    if (if_dest_exists(network, jter, kter)) {
        return;
    }

    char_node_t **ros_tables_list = network->ros_tables_list;

    __int32_t *ctx_path = NULL;
    __int32_t path_size = get_path(network, &ctx_path, iter, jter);
    
    __int32_t ctx_position = -1;
    for (__int32_t pter = 0; pter < path_size; pter++) {
        if (ctx_path[pter] == kter) {
            ctx_position = pter;
            break;
        }
    }
    if (ctx_position == -1 || ctx_position == path_size - 1) {
        safety_free(ctx_path);
        return;
    }

    __int32_t next_hop_index = ctx_path[ctx_position + 1];

    char ros_entry_buffer[ROS_ENTRY_BUFFER_S];
    int err = snprintf(ros_entry_buffer, sizeof(ros_entry_buffer), "<%s>:<%s>",
                       get_device_name_by_index(network, jter),
                       get_device_name_by_index(network, next_hop_index));
    if (err < 0) {
        panic("snprintf ros_tables_list init err");
    }

    char_node_t *new_node = (char_node_t *)panic_alloc(sizeof(char_node_t));
    new_node->entry = panic_strdup(ros_entry_buffer);
    new_node->next = NULL;

    if (ros_tables_list[kter] == NULL) {
        ros_tables_list[kter] = new_node;
    } else {
        char_node_t *tail = ros_tables_list[kter];
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = new_node;
    }
    safety_free(ctx_path);
}

void 
dump_shortest_hops(tinynet_conf_t *network)
{
    size_t start_hosts = network->net_graph->rc + network->net_graph->sc;
    size_t end_hosts = start_hosts + network->net_graph->hc;

    for (size_t iter = start_hosts; iter < end_hosts; iter += 1) {
        for (size_t jter = start_hosts; jter < end_hosts; jter += 1) {
            if (iter == jter) continue;

            printf("[PATH] From %s to %s: ", get_device_name_by_index(network, (__int32_t)iter), get_device_name_by_index(network, (__int32_t)jter));
            reconstruct_path(network, (__int32_t)iter, (__int32_t)jter);
            
        }
    }
}

void 
init_ros_tables(tinynet_conf_t *network)
{
    size_t start_hosts = network->net_graph->rc + network->net_graph->sc;
    size_t end_hosts = start_hosts + network->net_graph->hc;

    size_t dev_c = get_device_count(network);
    network->ros_tables_list = (char_node_t **)panic_alloc(sizeof(char_node_t *) * dev_c);
    for (size_t i = 0; i < dev_c; i++) {
        network->ros_tables_list[i] = NULL;
    }

    for (size_t kter = 0; kter < start_hosts; kter += 1) {
        for (size_t iter = start_hosts; iter < end_hosts; iter += 1) {
            for (size_t jter = start_hosts; jter < end_hosts; jter += 1) {
                if (iter == jter) continue;
                add_to_ros_table(network, (__int32_t)iter, (__int32_t)jter, (__int32_t)kter);
            }
        }
    }
}

void 
dump_ros_tables(tinynet_conf_t *network)
{
    char_node_t **ros_tables_list = network->ros_tables_list;
    
    for (size_t iter = 0; iter < network->net_graph->rc + network->net_graph->sc; iter += 1) {
        
        char_node_t *ctx = ros_tables_list[iter];
        printf("%s ros table:\n", get_device_name_by_index(network, iter));

        size_t jter = 1;
        while (ctx) {
            printf("\t%zu) - %s\n", jter, ctx->entry);

            ctx = ctx->next;
            jter += 1;
        }

        printf("\n");
    }
    
}
