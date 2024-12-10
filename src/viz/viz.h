#ifndef VIZ_H
#define VIZ_H

#include "src/tinynet.h"

int
write_dot_file(tinynet_conf_t *conf, const char *filename, int show_ip, int show_mac);


#endif /** VIZ_H */