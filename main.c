#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "src/tinynet.h"

#include "src/fsm/construct.h"

#include "src/viz/viz.h"

const char *dot_file = "conf/data/net.dot";

int 
main(void) 
{
    tinynet_conf_t *network = NULL;
    int err = graph_by_config(&network);
    if (err != EXIT_SUCCESS) {
        LOG_ERROR_PREFIX("Failed to parse YAML file\n");
        return err;
    }

    dump_net_links(network);
    
    // write_dot_file(network, dot_file, SHOW_IP, SHOW_MAC);

    destroy_net_conf(network);

    return EXIT_SUCCESS;
}
