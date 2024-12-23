#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/fsm/parser.h"
#include "src/fsm/construct.h"

static __int32_t
tiny_rand()
{
    return (rand() % 99 + 1);
}

static void 
push_rib(net_graph_t *graph, size_t src, size_t dest, __uint8_t weight) 
{
    adjacency_node_t *new_node = (adjacency_node_t *)panic_alloc(sizeof(adjacency_node_t)); 
    
    new_node->basic_info.dev_name = (char *)panic_strdup(graph->adjacency_list[dest]->basic_info.dev_name);
    new_node->basic_info.dev_type = graph->adjacency_list[dest]->basic_info.dev_type;

    new_node->weight = weight;

    adjacency_node_t *current_node =  graph->adjacency_list[src];
    while (current_node->next) {
        current_node = current_node->next;
    }
    current_node->next = new_node;
}

/** Switches and hosts base topologies. 
 * Bus for switches and tree for hosts as a model */
static void 
lans_topology(net_graph_t *graph, parser_state_t *parser_state) 
{
    size_t to_switches_frame = graph->rc;
    size_t to_hosts_frame = graph->rc + graph->sc;

    size_t riter = 0, siter = 0, hiter = 0; 

    __int32_t ctx_rand = 0;

    for (abs_dev_t *router = parser_state->wans_list; router != NULL; router = router->next) {

        
        size_t internal_sc = 0;
        size_t prev_switch = to_switches_frame + siter;

        for (abs_dev_t *switch_ = router->lower_devs_list; switch_ != NULL; switch_ = switch_->next) {

            ctx_rand = tiny_rand();
            push_rib(graph, riter, to_switches_frame + siter, ctx_rand); 
            push_rib(graph, to_switches_frame + siter, riter, ctx_rand); 

            if (internal_sc != 0) {
                ctx_rand = tiny_rand();
                push_rib(graph, prev_switch, to_switches_frame + siter, ctx_rand); 
                push_rib(graph, to_switches_frame + siter, prev_switch, ctx_rand);
                prev_switch = to_switches_frame + siter;
            }
            internal_sc += 1;

            for (abs_dev_t *host = switch_->lower_devs_list; host != NULL; host = host->next) {
                ctx_rand = tiny_rand();
                push_rib(graph, to_switches_frame + siter, to_hosts_frame + hiter, ctx_rand);
                push_rib(graph, to_hosts_frame + hiter, to_switches_frame + siter, ctx_rand);  
                hiter += 1;
            }
            siter += 1;
        }
        riter += 1;
    } 
}

/** Routers topologies */

static void 
mesh_topology(net_graph_t *graph, parser_state_t *parser_state) 
{    
    __int32_t ctx_rand = 0;
    for (size_t iter = 0; iter < graph->rc; iter += 1) {
        for (size_t jter = iter + 1; jter < graph->rc; jter += 1) {
            ctx_rand = tiny_rand();
            push_rib(graph, iter, jter, ctx_rand); 
            push_rib(graph, jter, iter, ctx_rand);
        }
    }
    lans_topology(graph, parser_state);
}

static void 
ring_topology(net_graph_t *graph, parser_state_t *parser_state) 
{   
    __int32_t ctx_rand = 0;
    for (size_t iter = 0; iter < graph->rc; iter += 1) {
        ctx_rand = tiny_rand();
        push_rib(graph, iter, (iter + 1) % graph->rc, ctx_rand);
        push_rib(graph, (iter + 1) % graph->rc, iter, ctx_rand); 
    }
    lans_topology(graph, parser_state);
}

static void 
bus_topology(net_graph_t *graph, parser_state_t *parser_state) 
{   
    __int32_t ctx_rand = 0;
    for (size_t iter = 0; iter < graph->rc - 1; iter += 1) {
        ctx_rand = tiny_rand();
        push_rib(graph, iter, iter + 1, ctx_rand); 
        push_rib(graph, iter + 1, iter, ctx_rand); 
    }
    lans_topology(graph, parser_state);
}

static void 
basic_info_deep_copy(adjacency_node_t* dest, abs_dev_t *src)
{   
    dest->basic_info.dev_name = (char *)panic_strdup(src->basic_info.dev_name);
    dest->basic_info.dev_type = src->basic_info.dev_type;
}

static net_graph_t *
build_net_graph(parser_state_t *parser_state) 
{
    net_graph_t *net_graph = (net_graph_t *)panic_alloc(sizeof(net_graph_t));

    net_graph->rc = parser_state->rout_c;
    net_graph->sc = parser_state->swit_c;
    net_graph->hc = parser_state->host_c;

    size_t dev_c = net_graph->rc + net_graph->sc + net_graph->hc;
    net_graph->adjacency_list = (adjacency_node_t **)panic_alloc(sizeof(adjacency_node_t *) * dev_c);

    size_t diter = 0;

    /** Add routers */
    for (abs_dev_t *router = parser_state->wans_list; router != NULL; router = router->next) {
        net_graph->adjacency_list[diter] = (adjacency_node_t *)panic_alloc(sizeof(adjacency_node_t));
        basic_info_deep_copy(net_graph->adjacency_list[diter], router);
        net_graph->adjacency_list[diter]->next = NULL;
        diter += 1;
    }

    /** Add switches */
    for (abs_dev_t *router = parser_state->wans_list; router != NULL; router = router->next) {
        for (abs_dev_t *switch_ = router->lower_devs_list; switch_ != NULL; switch_ = switch_->next) {
            net_graph->adjacency_list[diter] = (adjacency_node_t *)panic_alloc(sizeof(adjacency_node_t));
            basic_info_deep_copy(net_graph->adjacency_list[diter], switch_);
            net_graph->adjacency_list[diter]->next = NULL;
            diter += 1;
        }
    }

    /** Add hosts */
    for (abs_dev_t *router = parser_state->wans_list; router != NULL; router = router->next) {
        for (abs_dev_t *switch_ = router->lower_devs_list; switch_ != NULL; switch_ = switch_->next) {
            for (abs_dev_t *host = switch_->lower_devs_list; host != NULL; host = host->next) {
                net_graph->adjacency_list[diter] = (adjacency_node_t *)panic_alloc(sizeof(adjacency_node_t));
                basic_info_deep_copy(net_graph->adjacency_list[diter], host);
                net_graph->adjacency_list[diter]->next = NULL;
                diter += 1;
            }
        }
    }

    return net_graph;
}

__int32_t 
graph_by_config(tinynet_conf_t **net_conf) 
{   
    __int32_t err;
    parser_state_t *parser_state = NULL;

    err = parse_yaml(&parser_state);
    if (err != EXIT_SUCCESS) {
        return err;
    }

    net_graph_t *graph = build_net_graph(parser_state);
    net_types_e ctx_net_type = parser_state->net_conf_type;

    srand(time(NULL));

    switch (ctx_net_type) {
        case MESH_NET_T:
            mesh_topology(graph, parser_state);
            break;
        case RING_NET_T:
            ring_topology(graph, parser_state);
            break;
        case BUS_NET_T:
            bus_topology(graph, parser_state);
            break;
        default:   
            panic("incorrect net type");
            break;
    }


    (*net_conf) = (tinynet_conf_t *)panic_alloc(sizeof(tinynet_conf_t));

    (*net_conf)->net_type = parser_state->net_conf_type;
    (*net_conf)->net_name = (char *)panic_strdup(parser_state->net_conf_name);
    (*net_conf)->net_description = (char *)panic_strdup(parser_state->net_conf_description);

    (*net_conf)->net_graph = graph;
    (*net_conf)->hops_matrix = NULL;
    
    destroy_parser_state(parser_state);

    return EXIT_SUCCESS;
}

void
add_router(abs_dev_t **routers, device_e router_type, dev_basic_info_t router_binf, abs_dev_t *lans_list)
{
    /* Create router object. */
    abs_dev_t *router = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    router->basic_info = router_binf;
    router->basic_info.dev_type = router_type;

    router->lower_devs_list = lans_list;

    if (!*routers) {
        *routers = router;
    } else {
        abs_dev_t *routers_tail = *routers;
        while (routers_tail->next) {
            routers_tail = routers_tail->next;
        }
        routers_tail->next = router;
    }
}

void
add_switch(abs_dev_t **switches, device_e switch_type, dev_basic_info_t switch_binf, abs_dev_t *hosts_list)
{
    /* Create switch object. */
    abs_dev_t *switch_ = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    switch_->basic_info = switch_binf;
    switch_->basic_info.dev_type = switch_type;

    switch_->lower_devs_list = hosts_list;

    /* Append to list. */
    if (!*switches) {
        *switches = switch_;
    } else {
        abs_dev_t *switches_tail = *switches;
        while (switches_tail->next) {
            switches_tail = switches_tail->next;
        }
        switches_tail->next = switch_;
    }
}

void
add_host(abs_dev_t **hosts, device_e host_type, dev_basic_info_t host_binf)
{
    /* Create switch object. */
    abs_dev_t *host = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    host->basic_info = host_binf;
    host->basic_info.dev_type = host_type;

    /* Append to list. */
    if (!*hosts) {
        *hosts = host;
    } else {
        abs_dev_t *hosts_tail = *hosts;
        while (hosts_tail->next) {
            hosts_tail = hosts_tail->next;
        }
        hosts_tail->next = host;
    }
}

