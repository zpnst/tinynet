#ifndef PARSER_H
#define PARSER_H

#include <yaml.h>

#include "src/tinynet.h"

int
parse_yaml(parser_state_t **parsed_net_conf);

#endif /** PARDER_H */