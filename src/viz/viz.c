#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/sys.h"  
#include "src/tinynet.h"  

#define DEV_WEIGHT_BUFFER_S 3
#define DEV_NAME_BUFFER_S 128
#define DEV_DOT_LABEL_BUFFER_S 256

static const char*
get_color_by_device_type(device_e dev_type)
{
    switch (dev_type) {
    case ROUTER_T:
        return "purple";
    case SWITCH_T:
        return "blue";
    case HOST_T:
        return "black";
    default:
        panic("incorrect device type");
    }
}

static const char*
get_shape_by_device_type(device_e dev_type) 
{
    switch (dev_type) {
    case ROUTER_T:
        return "diamond";
    case SWITCH_T:
        return "ellipse";
    case HOST_T:
        return "box";
    default:
        panic("incorrect device type");
    }
}

static const char*
get_topology_type_string(net_types_e net_type)
{
    switch(net_type) {
    case MESH_NET_T:
        return "Mesh";
    case RING_NET_T:
        return "Ring";
    case BUS_NET_T:
        return "Bus";
    default:
        panic("incorrect device type");
    }
}

int 
build_viz_file(tinynet_conf_t *conf, const char *filename) 
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Cannot open file");
        return -1;
    }

    fprintf(fp, "graph tinynet {\n");
    fprintf(fp, "    rankdir=LR;\n");

    fprintf(fp, "    labelloc=top;\n");
    fprintf(fp, "    labeljust=left;\n");


    fprintf(fp, "    label=<\n");
    fprintf(fp, "    <TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\">\n");
    fprintf(fp, "        <TR><TD ALIGN=\"LEFT\"><B>Topology</B>: %s</TD></TR>\n", get_topology_type_string(conf->net_type));
    fprintf(fp, "        <TR><TD ALIGN=\"LEFT\"><B>Name</B>: %s</TD></TR>\n", conf->net_name);
    fprintf(fp, "        <TR><TD ALIGN=\"LEFT\"><B>Description</B>: %s</TD></TR>\n", conf->net_description);
    fprintf(fp, "    </TABLE>\n");
    fprintf(fp, "    >;\n");


    size_t dev_c = conf->net_graph->rc + conf->net_graph->sc + conf->net_graph->hc;

    /** Формирование узлоы */
    for (size_t iter = 0; iter < dev_c; iter += 1) {
        adjacency_node_t *ctx = conf->net_graph->adjacency_list[iter];
        if (!ctx) continue; 

        char dev_name[DEV_NAME_BUFFER_S];
        snprintf(dev_name, sizeof(dev_name), "%s", ctx->basic_info.dev_name);

        char dev_label[DEV_DOT_LABEL_BUFFER_S];
        snprintf(dev_label, sizeof(dev_label), "%s", dev_name);

        fprintf(fp, "    \"%s\" [shape=%s, color=%s, label=\"%s\"];\n", dev_name,
                get_shape_by_device_type(ctx->basic_info.dev_type),
                get_color_by_device_type(ctx->basic_info.dev_type),
                dev_label);
    }

    /** Формирование связей между узлами(рёбер) */
    for (size_t iter = 0; iter < dev_c; iter += 1) {
        adjacency_node_t *ctx = conf->net_graph->adjacency_list[iter];

        while (ctx) {
            
            char dev1[DEV_NAME_BUFFER_S], dev2[DEV_NAME_BUFFER_S];

            snprintf(dev1, sizeof(dev1), "%s", conf->net_graph->adjacency_list[iter]->basic_info.dev_name);
            snprintf(dev2, sizeof(dev2), "%s", ctx->basic_info.dev_name);

            if (strcmp(dev1, dev2) < 0) { 
                char weight_str[DEV_WEIGHT_BUFFER_S];
                snprintf(weight_str, sizeof(weight_str), "%d", ctx->weight);
                fprintf(fp, "    \"%s\" -- \"%s\" [label=\"%s\"];\n", dev1, dev2, weight_str);
            }
            
            ctx = ctx->next;
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);

    return 0;
}
