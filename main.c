#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "src/tinynet.h"

#include "src/fsm/parser.h"

#include "src/viz/viz.h"

const tinynet_char_t *yaml_file = "conf/net.yaml";
const tinynet_char_t *dot_file = "conf/data/net.dot";

int 
main(void) 
{
    tinynet_conf_t *network = NULL;
    int exit_code = parse_yaml(&network, yaml_file);

    if (exit_code != EXIT_SUCCESS) {
        fprintf(stderr, "[ERR] Failed to parse YAML file\n");
        return EXIT_FAILURE;
    }

    dump_net_conf(network);

    write_dot_file(network, dot_file, SHOW_IP, SHOW_MAC);
    destroy_net_conf(network);

    return EXIT_SUCCESS;
}
