#ifndef PARSER_H
#define PARSER_H

#include <yaml.h>

#include "src/tinynet.h"

/** parser state. */
typedef struct parser_state_s {
    char *net_conf_name;           /** Master network configuration name. */
    char *net_conf_description;    /** Master network configuration description. */
    net_types_e net_conf_type;               /** Master network configuration type. */

    machine_states_t state;                   /** The current parse state */

    dev_basic_info_t host;                    /** Host buffer. */
    dev_basic_info_t switch_;                 /** LAN(switch) buffer. */
    dev_basic_info_t router;                  /** WAN(router) buffer. */

    size_t rout_c;                            /** Number of routers. */
    size_t swit_c;                            /** Number of switches. */
    size_t host_c;                            /** Number of hosts. */

    abs_dev_t *host_list;                     /** Temporary buffer for each LAN. */
    abs_dev_t *lans_list;                     /** Temporary buffer for each WAN. */
    
    abs_dev_t *wans_list;                     /** Master list of WANs. */

} parser_state_t;

int
parse_yaml(parser_state_t **parsed_net_conf);

#endif /** PARDER_H */