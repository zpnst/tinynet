#ifndef PARSER_H
#define PARSER_H

#include <yaml.h>

#include "src/tinynet.h"

int
parse_yaml(tinynet_conf_t **net_conf, const tinynet_char_t *yaml_filename);

int
handle_event(parser_state_t *s, yaml_event_t *event, machine_states_t *expected_state);

#endif /** PARDER_H */