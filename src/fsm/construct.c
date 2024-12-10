#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/sys.h"
#include "src/tinynet.h"
#include "src/fsm/construct.h"

void
add_router(abs_dev_t **routers, device_e router_type, dev_basic_info_t router_binf, abs_dev_t *lans_list)
{
    /* Create router object. */
    abs_dev_t *router = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    router->basic_info = router_binf;
    router->basic_info.dev_type = router_type;

    router->lower_devs_list = lans_list;

    if (!*routers) {
        *routers = router;
    } else {
        abs_dev_t *routers_tail = *routers;
        while (routers_tail->next) {
            routers_tail = routers_tail->next;
        }
        routers_tail->next = router;
    }
}

void
add_switch(abs_dev_t **switches, device_e switch_type, dev_basic_info_t switch_binf, abs_dev_t *hosts_list)
{
    /* Create switch object. */
    abs_dev_t *switch_ = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    switch_->basic_info = switch_binf;
    switch_->basic_info.dev_type = switch_type;

    switch_->lower_devs_list = hosts_list;

    /* Append to list. */
    if (!*switches) {
        *switches = switch_;
    } else {
        abs_dev_t *switches_tail = *switches;
        while (switches_tail->next) {
            switches_tail = switches_tail->next;
        }
        switches_tail->next = switch_;
    }
}

void
add_host(abs_dev_t **hosts, device_e host_type, dev_basic_info_t host_binf)
{
    /* Create switch object. */
    abs_dev_t *host = (abs_dev_t *)panic_alloc(sizeof(abs_dev_t));

    host->basic_info = host_binf;
    host->basic_info.dev_type = host_type;

    /* Append to list. */
    if (!*hosts) {
        *hosts = host;
    } else {
        abs_dev_t *hosts_tail = *hosts;
        while (hosts_tail->next) {
            hosts_tail = hosts_tail->next;
        }
        hosts_tail->next = host;
    }
}

void 
destroy_state(parser_state_t *state) 
{
    free(state->host.dev_name);
    free(state->switch_.dev_name);
    free(state->router.dev_name);
}

void
destroy_net_conf(tinynet_conf_t *net_conf)
{   
    free(net_conf->net_name);
    free(net_conf->net_description);
    destroy_wans_list(net_conf->devs);

    free(net_conf);
}

void 
destroy_wans_list(abs_dev_t *wans_list) 
{
    abs_dev_t *wan = wans_list;

    while (wan) {
        abs_dev_t *wan_next = wan->next;
        free(wan->basic_info.dev_name);
        abs_dev_t *lan = wan->lower_devs_list;

        while (lan) {
            abs_dev_t *lan_next = lan->next;
            free(lan->basic_info.dev_name);
            abs_dev_t *host = lan->lower_devs_list;

            while (host) {
                abs_dev_t *host_next = host->next;
                free(host->basic_info.dev_name);
                free(host);
                host = host_next;
            }
            free(lan);
            lan = lan_next;
        }
        free(wan);
        wan = wan_next;
    }
}