#ifndef PARSER_H
#define PARSER_H

#include <yaml.h>

#include "src/tinynet.h"

int
parse_yaml(tinynet_conf_t *net_conf, const tinynet_char_t *yaml_filename);

int
handle_event(struct parser_state *s, yaml_event_t *event);

// void 
// dump_network_conf(tinynet_conf_t *net_conf, size_t sep_level);

#endif /** PARDER_H */