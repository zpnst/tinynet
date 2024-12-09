#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "src/tinynet.h"
#include "src/utils/addr/ip.h"
#include "src/utils/addr/mac.h"
#include "src/utils/yaml/parser.h"


int 
main(void) 
{

    const tinynet_char_t *yaml_file = "conf/net.yaml";

    tinynet_conf_t *network;
    int exit_code = parse_yaml(network, yaml_file);
    
    if (exit_code != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to parse YAML file\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
