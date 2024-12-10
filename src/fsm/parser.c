#include <yaml.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/fsm/parser.h"
#include "src/fsm/construct.h"

#include "src/net/addr/ip.h"
#include "src/net/addr/mac.h"

static void 
error_message(machine_states_t expected, machine_states_t received) 
{
    fprintf(stderr, "[ERR] Yaml tinynet structure error\n[ERR] Expected state %d, received state %d.\
    \n[ERR] Here is template:\n[ERR]     dev_name: 'name'\n[ERR]     dev_ip: 'ip'\n[ERR]     dev_mac: 'mac'", expected, received);
}

static int
handle_event(parser_state_t *s, yaml_event_t *event, machine_states_t *expected_state)
{
    tinynet_char_t *value;

#ifdef DEBUG_STATE_MACHINE
    printf("[SMDEBUG] State=%d, Event=%d\n", s->state, event->type);
#endif

    switch (s->state) {

    /** LIBYAML STATES */

    case STATE_START:
        switch (event->type) {
        case YAML_STREAM_START_EVENT:
            s->state = STATE_STREAM;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_STREAM:
        switch (event->type) {
        case YAML_DOCUMENT_START_EVENT:
            s->state = STATE_DOCUMENT;
            break;
        case YAML_STREAM_END_EVENT:
            s->state = STATE_STOP;  /* All done. */
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_DOCUMENT:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            s->state = STATE_SECTION;
            break;
        case YAML_DOCUMENT_END_EVENT:
            s->state = STATE_STREAM;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SECTION:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (tinynet_char_t *)event->data.scalar.value;
            if (strcmp(value, "tinynet") == 0) {
               s->state = STATE_NETVALUES;
            } else {
               fprintf(stderr, "Unexpected yaml doc title: %s, must be: '%s'\n", value, YAML_DOC_TITLE);
               return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:
            break;
        case YAML_DOCUMENT_END_EVENT:
            s->state = STATE_STREAM;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** CONFIGURATION STATES */

    case STATE_NETVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            *expected_state = STATE_NETNAME;
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d, you must provide 'net_name' and 'net_description' to tinynet section\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "net_name") == 0) {
                s->state = STATE_NETNAME;
            } else if (strcmp(value, "net_description") == 0) {
                s->state = STATE_NETDESCRIPTION;
            } else if (strcmp(value, "wan_devs") == 0) { /** Change parsing level*/
                s->state = STATE_WANLIST;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:
            s->state = STATE_SECTION;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETNAME:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_name) {
                fprintf(stderr, "Warning: duplicate 'net_name' key.\n");
                free(s->net_conf_name);
            }
            s->net_conf_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            *expected_state = STATE_NETDESCRIPTION;
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETDESCRIPTION:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_description) {
                fprintf(stderr, "Warning: duplicate 'net_description' key.\n");
                free(s->net_conf_description);
            }
            s->net_conf_description = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            *expected_state = STATE_WANLIST;
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_WANLIST:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_WANVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_WANVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            *expected_state = STATE_ROUTERNAME;
            s->state = STATE_WANKEYS;
            break;
        case YAML_SEQUENCE_END_EVENT:
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** WAN(ROUTERS) STATES */

    case STATE_WANKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (tinynet_char_t *)event->data.scalar.value;
            if (strcmp(value, "router_name") == 0) {
                s->state = STATE_ROUTERNAME;
            } else if (strcmp(value, "router_ip") == 0) {
                s->state = STATE_ROUTERIP;
            } else if (strcmp(value, "router_mac") == 0) {
                s->state = STATE_ROUTERMAC;
            } else if (strcmp(value, "router_lan_devs") == 0) { /** Change parsing level*/
                s->state = STATE_LANLIST;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            if (!(s->router.ip_addr.addr) || !is_mac_exists(&s->router.mac_addr)) {
                fprintf(stderr, "[ERR] %s don't have an IP address or a MAC address\n", s->router.dev_name);
                return FAILURE;
            }

            /** Append new router because mapping end */
            add_router(&s->wans_list, ROUTER_T, s->router, s->lans_list);

            /** Free buffer */
            memset(&s->router, 0, sizeof(s->router));

            /** Reset the pointers */
            s->router.dev_name = NULL;
            s->lans_list = NULL;

            /** Parse other routers or go upper */
            s->state = STATE_WANVALUES;

            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERNAME:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.dev_name) {
                fprintf(stderr, "Warning: duplicate 'router_name' key.\n");
                free(s->router.dev_name);
            }
            s->router.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            *expected_state = STATE_ROUTERIP;
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERIP:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.ip_addr.addr) {
                fprintf(stderr, "Warning: duplicate 'router_ip' key.\n");
            }
            tinynet_char_t *ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_ip_addr(&s->router.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", ip_addr_str);
                return FAILURE;
            }
            free(ip_addr_str);
            *expected_state = STATE_ROUTERMAC;
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERMAC:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.mac_addr.addr[0]) {
                fprintf(stderr, "Warning: duplicate 'router_mac' key.\n");
            }
            tinynet_char_t *mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_mac_addr(&s->router.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect MAC address: %s.\n", mac_addr_str);
                return FAILURE;
            }
            free(mac_addr_str);
            *expected_state = STATE_LANLIST;
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** LAN(SWITCHES) STATES */

    case STATE_LANLIST:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_LANVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_LANVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            *expected_state = STATE_SWITCHNAME;
            s->state = STATE_LANKEYS;
            break;
        case YAML_SEQUENCE_END_EVENT:
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_LANKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (tinynet_char_t *)event->data.scalar.value;
            if (strcmp(value, "switch_name") == 0) {
                s->state = STATE_SWITCHNAME;
            } else if (strcmp(value, "switch_ip") == 0) {
                s->state = STATE_SWITCHIP;
            } else if (strcmp(value, "switch_mac") == 0) {
                s->state = STATE_SWITCHMAC;
            } else if (strcmp(value, "switch_hosts") == 0) { /** Change parsing level*/
                s->state = STATE_HOSTSLIST;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:


            if (!(s->switch_.ip_addr.addr) || !is_mac_exists(&s->switch_.mac_addr)) {
                fprintf(stderr, "[ERR] %s don't have an IP address or a MAC address\n", s->switch_.dev_name);
                return FAILURE;
            }

            /** Append new switch because mapping end */
            add_switch(&s->lans_list, SWITCH_T, s->switch_, s->host_list);

            /** Free buffers*/
            memset(&s->switch_, 0, sizeof(s->switch_));

            /** Reset the pointers */
            s->switch_.dev_name = NULL;
            s->host_list = NULL;

            /** Parse other switches or go upper */
            s->state = STATE_LANVALUES;

            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;



    case STATE_SWITCHNAME:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.dev_name) {
                fprintf(stderr, "Warning: duplicate 'switch_name' key.\n");
                free(s->switch_.dev_name);
            }
            s->switch_.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            *expected_state = STATE_SWITCHIP;
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SWITCHIP:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.ip_addr.addr) {
                fprintf(stderr, "Warning: duplicate 'switch_ip' key.\n");
            }
            tinynet_char_t *ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_ip_addr(&s->switch_.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", ip_addr_str);
                return FAILURE;
            }
            free(ip_addr_str);
            *expected_state = STATE_SWITCHMAC;
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SWITCHMAC:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.mac_addr.addr[0]) {
                fprintf(stderr, "Warning: duplicate 'switch_mac' key.\n");
            }
            tinynet_char_t *mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_mac_addr(&s->switch_.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect MAC address: %s.\n", mac_addr_str);
                return FAILURE;
            }
            free(mac_addr_str);
            *expected_state = STATE_HOSTSLIST;
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTSLIST:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_HOSTVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** HOSTS STATES */

    case STATE_HOSTVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            *expected_state = STATE_HOSTNAME;
            s->state = STATE_HOSTKEYS;
            break;
        case YAML_SEQUENCE_END_EVENT:
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "host_name") == 0) {
                s->state = STATE_HOSTNAME;
            } else if (strcmp(value, "host_ip") == 0) {
                s->state = STATE_HOSTIP;
            } else if (strcmp(value, "host_mac") == 0) {
                s->state = STATE_HOSTMAC;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            if (!(s->host.ip_addr.addr) || !is_mac_exists(&s->host.mac_addr)) {
                fprintf(stderr, "[ERR] %s don't have an IP address or a MAC address\n", s->host.dev_name);
                return FAILURE;
            }

            /** Append new host because mapping end */
            add_host(&s->host_list, HOST_T, s->host);

            /** Free buffer */
            memset(&s->host, 0, sizeof(s->host));

            /** Reset pointer */
            s->host.dev_name = NULL;
            
            /** Parse other hosts or go upper */
            s->state = STATE_HOSTVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTNAME:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->host.dev_name) {
                fprintf(stderr, "Warning: duplicate 'host_name' key.\n");
                free(s->host.dev_name);
            }
            s->host.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            *expected_state = STATE_HOSTIP;
            s->state = STATE_HOSTKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTIP:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->host.ip_addr.addr) {
                fprintf(stderr, "Warning: duplicate 'host_ip' key.\n");
            }
            tinynet_char_t *ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_ip_addr(&s->host.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", ip_addr_str);
                return FAILURE;
            }
            free(ip_addr_str);
            *expected_state = STATE_HOSTMAC;
            s->state = STATE_HOSTKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTMAC:
        if (s->state != *expected_state) {
            error_message(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->host.mac_addr.addr[0]) {
                fprintf(stderr, "Warning: duplicate 'host_mac' key.\n");
            }
            tinynet_char_t *mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            if (parse_mac_addr(&s->host.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect MAC address: %s.\n", mac_addr_str);
                return FAILURE;
            }
            free(mac_addr_str);
            s->state = STATE_HOSTKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_STOP:
        break;
    }
    return SUCCESS;
}

int
parse_yaml(tinynet_conf_t **net_conf, const tinynet_char_t *yaml_filename)
{

    FILE *yamlf = fopen(yaml_filename, "r");
    if (!yamlf) {
        perror("yamlf fopen");
        return 1;
    }

    parser_state_t state;
    yaml_parser_t parser;
    machine_states_t status;

    machine_states_t expected_state = STATE_START;

    memset(&state, 0, sizeof(state));
    state.state = STATE_START;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, yamlf);

    do {
        yaml_event_t event;

        status = yaml_parser_parse(&parser, &event);

        if (status == FAILURE) {
            fprintf(stderr, "[ERR] yaml_parser_parse error\n");

            destroy_wans_list(state.wans_list);
            destroy_state(&state);
            
            yaml_parser_delete(&parser);
            fclose(yamlf);
            return EXIT_FAILURE;
        }

        status = handle_event(&state, &event, &expected_state);
        yaml_event_delete(&event);

        if (status == FAILURE) {
            fprintf(stderr, "[ERR] handle_event error\n");

            destroy_wans_list(state.wans_list);
            destroy_state(&state);

            yaml_parser_delete(&parser);
            fclose(yamlf);
            return EXIT_FAILURE;
            
        }
    } while (state.state != STATE_STOP);

    /* Output the parsed data. */


    *net_conf = (tinynet_conf_t *)panic_alloc(sizeof(tinynet_conf_t));
    (*net_conf)->net_name = state.net_conf_name;
    (*net_conf)->net_description = state.net_conf_description;
    (*net_conf)->devs = state.wans_list;

    destroy_state(&state);
    yaml_parser_delete(&parser);
    fclose(yamlf);
    
    return EXIT_SUCCESS;
}

