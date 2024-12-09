#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "src/tinynet.h"
#include "src/utils/ip.h"
#include "src/utils/mac.h"

const char *dot_file = "net.dot";
const char *yaml_file = "net.yaml";

typedef struct lan_devs_S {

} lan_devs_t;

typedef struct wan_devs_s {
    tinynet_char_t *dev_name;
    tinynet_char_t *dev_type;

    ip_addr_t ip_addr;
    mac_addr_t mac_addr;
    
    lan_devs_t *lan_devs_list;

} wan_devs_t;

typedef struct network_conf_s {
    char *dev_name;
    wan_devs_t *wan_devs_list;      
} network_conf_t;


int main(int argc, char **argv) {

    return 0;
}
