#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include "src/tinynet.h"

void
panic(tinynet_char_t *message);

void *
panic_alloc(size_t size);

tinynet_char_t *
panic_strdup(tinynet_char_t *s);

void
add_router(abs_dev_t **routers, device_e router_type, dev_basic_info_t router_binf, abs_dev_t *lans_list);

void
add_switch(abs_dev_t **switches, device_e switch_type, dev_basic_info_t switch_binf, abs_dev_t *hosts_list);

void
add_host(abs_dev_t **hosts, device_e host_type, dev_basic_info_t host_binf);

void 
destroy_state(parser_state_t *state);

void 
destroy_wans_list(abs_dev_t *wans_list);

#endif /** CONSTRUCT_H */