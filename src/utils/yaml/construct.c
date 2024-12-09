#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/tinynet.h"

void
panic(const tinynet_char_t *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void *
panic_alloc(size_t size)
{
    void *new_prt = calloc(1, size);
    if (!new_prt) {
        panic("out of memory");
    }
    return new_prt;
}

tinynet_char_t *
panic_strdup(const tinynet_char_t *s)
{
    tinynet_char_t *new_char = strdup(s ? s : "");
    if (!new_char) {
        panic("out of memory");
    }
    return new_char;
}

void
add_router(abs_dev_t **routers, device_e router_type, dev_basic_info_t router_binf, abs_dev_t *lans_list)
{
    /* Create router object. */
    abs_dev_t *router = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    router->dev_type = router_type;
    router->basic_info = router_binf;
    router->lower_devs_list = lans_list;

    if (!*routers) {
        *routers = router;
    } else {
        abs_dev_t *devs_tail = *routers;
        while (devs_tail->next) {
            devs_tail = devs_tail->next;
        }
        devs_tail->next = router;
    }
}

void
add_switch(abs_dev_t **switches, device_e switch_type, dev_basic_info_t switch_binf, abs_dev_t *hosts_list)
{
    /* Create switch object. */
    abs_dev_t *switch_ = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    switch_->dev_type = switch_type;
    switch_->basic_info = switch_binf;
    switch_->lower_devs_list = hosts_list;

    /* Append to list. */
    if (!*switches) {
        *switches = switch_;
    } else {
        variety *devs_tail = *switches;
        while (devs_tail->next) {
            devs_tail = devs_tail->next;
        }
        devs_tail->next = switch_;
    }
}

void
add_host(abs_dev_t **hosts, device_e host_type, dev_basic_info_t host_binf)
{
    /* Create switch object. */
    abs_dev_t *host = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    host->dev_type = host_type;
    host->basic_info = host_binf;

    /* Append to list. */
    if (!*hosts) {
        *hosts = host;
    } else {
        variety *devs_tail = *hosts;
        while (devs_tail->next) {
            devs_tail = devs_tail->next;
        }
        devs_tail->next = host;
    }
}


void
destroy_routers(abs_dev_t **routers)
{
    for (abs_dev_t *r = *routers; r; r = *routers) {
        *routers = r->next;
        
        free(r->basic_info.dev_name);
        free(r->basic_info.dev_type);

        free(r->basic_info.ip_addr);
        free(r->basic_info.mac_addr);

        destroy_switches(&r->lower_devs_list);
        free(r);
    }
}

void
destroy_switches(abs_dev_t **switches)
{
    for (abs_dev_t *s = *switches; s; s = *switches) {
        *switches = s->next;

        free(s->basic_info.dev_name);
        free(s->basic_info.dev_type);

        free(s->basic_info.ip_addr);
        free(s->basic_info.mac_addr);

        destroy_hosts(&s->lower_devs_list);
        free(s);
    }
}

void
destroy_hosts(abs_dev_t **hosts)
{
    for (abs_dev_t *h = *hosts; h; h = *hosts) {
        *hosts = h->next;

        free(h->basic_info.dev_name);
        free(s->basic_info.dev_type);

        free(h->basic_info.ip_addr);
        free(h->basic_info.mac_addr);

        free(h);
    }
}