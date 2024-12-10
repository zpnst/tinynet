#ifndef PARSER_H
#define PARSER_H

#include <yaml.h>

#include "src/tinynet.h"

int
parse_yaml(tinynet_conf_t **net_conf, const tinynet_char_t *yaml_filename);

#endif /** PARDER_H */