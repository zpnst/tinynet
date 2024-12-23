#include <yaml.h>

#include "src/sys.h"
#include "src/tinynet.h"

#include "src/fsm/parser.h"
#include "src/fsm/construct.h"

static void 
yaml_err_msg(machine_states_t expected, machine_states_t received) 
{
    LOG_ERROR_PREFIX("Yaml tinynet structure error\n");
    LOG_ERROR_PREFIX("Expected state %d, received state %d.\n", expected, received);
    LOG_ERROR_PREFIX("Here is dev template:\n");
    LOG_ERROR_PREFIX("    dev_name: 'name'\n");
}

static int
handle_event(parser_state_t *s, yaml_event_t *event, machine_states_t *expected_state)
{
    char *value;

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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SECTION:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "tinynet") == 0) {
               s->state = STATE_NETVALUES;
            } else {
               LOG_ERROR_PREFIX("Unexpected yaml doc title: %s, must be: '%s'\n", value, YAML_DOC_TITLE);
               return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:
            break;
        case YAML_DOCUMENT_END_EVENT:
            s->state = STATE_STREAM;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** CONFIGURATION STATES */

    case STATE_NETVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
            *expected_state = STATE_NETTYPE;
            s->state = STATE_NETKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d, you must provide 'net_name' and 'net_description' to tinynet section\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "net_type") == 0) {
                s->state = STATE_NETTYPE;
            } else if (strcmp(value, "net_name") == 0) {
                s->state = STATE_NETNAME;
            } else if (strcmp(value, "net_description") == 0) {
                s->state = STATE_NETDESCRIPTION;
            } else if (strcmp(value, "wan_devs") == 0) { /** Change parsing level*/
                s->state = STATE_WANLIST;
            } else {
                LOG_ERROR_PREFIX("Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:
            s->state = STATE_SECTION;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETTYPE:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_type) {
                LOG_WARNING_PREFIX("Warning: duplicate 'net_type' key.\n");
            }

            char *string_net_type = panic_strdup((char *)event->data.scalar.value);
            if (strcmp(string_net_type, "mesh") == 0) {
                s->net_conf_type = MESH_NET_T;
            } else if (strcmp(string_net_type, "ring") == 0) {
                s->net_conf_type = RING_NET_T;
            } else if (strcmp(string_net_type, "bus") == 0) {
                s->net_conf_type = BUS_NET_T;
            } else {
                LOG_ERROR_PREFIX("Unexpected device type, must be 'mesh', 'ring' or 'bus'\n");
                return FAILURE;
            }

            safety_free(string_net_type);
            *expected_state = STATE_NETNAME;
            s->state = STATE_NETKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETNAME:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_name) {
                LOG_WARNING_PREFIX("duplicate 'net_name' key.\n");
                safety_free(s->net_conf_name);
            }
            s->net_conf_name = panic_strdup((char *)event->data.scalar.value);
            *expected_state = STATE_NETDESCRIPTION;
            s->state = STATE_NETKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETDESCRIPTION:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_description) {
                LOG_WARNING_PREFIX("duplicate 'net_description' key.\n");
                safety_free(s->net_conf_description);
            }
            s->net_conf_description = panic_strdup((char *)event->data.scalar.value);
            *expected_state = STATE_WANLIST;
            s->state = STATE_NETKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_WANLIST:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_WANVALUES;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** WAN(ROUTERS) STATES */

    case STATE_WANKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "router_name") == 0) {
                s->state = STATE_ROUTERNAME;
            } else if (strcmp(value, "router_lan_devs") == 0) { /** Change parsing level*/
                s->state = STATE_LANLIST;
            } else {
                LOG_ERROR_PREFIX("Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            /** Append new router because mapping end */
            add_router(&s->wans_list, ROUTER_T, s->router, s->lans_list);

            /** safety_free buffer */
            memset(&s->router, 0, sizeof(s->router));

            /** Reset the pointers */
            s->router.dev_name = NULL;
            s->lans_list = NULL;

            /** Parse other routers or go upper */
            s->state = STATE_WANVALUES;

            s->rout_c += 1;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERNAME:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.dev_name) {
                LOG_WARNING_PREFIX("duplicate 'router_name' key.\n");
                safety_free(s->router.dev_name);
            }
            s->router.dev_name = panic_strdup((char *)event->data.scalar.value);
            *expected_state = STATE_LANLIST;
            s->state = STATE_WANKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** LAN(SWITCHES) STATES */

    case STATE_LANLIST:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_LANVALUES;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_LANKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "switch_name") == 0) {
                s->state = STATE_SWITCHNAME;
            } else if (strcmp(value, "switch_hosts") == 0) { /** Change parsing level*/
                s->state = STATE_HOSTSLIST;
            } else {
                LOG_ERROR_PREFIX("Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            /** Append new switch because mapping end */
            add_switch(&s->lans_list, SWITCH_T, s->switch_, s->host_list);

            /** safety_free buffers*/
            memset(&s->switch_, 0, sizeof(s->switch_));

            /** Reset the pointers */
            s->switch_.dev_name = NULL;
            s->host_list = NULL;

            /** Parse other switches or go upper */
            s->state = STATE_LANVALUES;

            s->swit_c += 1;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;



    case STATE_SWITCHNAME:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.dev_name) {
                LOG_WARNING_PREFIX("duplicate 'switch_name' key.\n");
                safety_free(s->switch_.dev_name);
            }
            s->switch_.dev_name = panic_strdup((char *)event->data.scalar.value);
            *expected_state = STATE_HOSTSLIST;
            s->state = STATE_LANKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTSLIST:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_HOSTVALUES;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
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
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "host_name") == 0) {
                s->state = STATE_HOSTNAME;
            } else {
                LOG_ERROR_PREFIX("Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            /** Append new host because mapping end */
            add_host(&s->host_list, HOST_T, s->host);

            /** safety_free buffer */
            memset(&s->host, 0, sizeof(s->host));

            /** Reset pointer */
            s->host.dev_name = NULL;
            
            /** Parse other hosts or go upper */
            s->state = STATE_HOSTVALUES;

            s->host_c += 1;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTNAME:
        if (s->state != *expected_state) {
            yaml_err_msg(*expected_state, s->state);
            return FAILURE;
        }
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->host.dev_name) {
                LOG_WARNING_PREFIX("duplicate 'host_name' key.\n");
                safety_free(s->host.dev_name);
            }
            s->host.dev_name = panic_strdup((char *)event->data.scalar.value);
            s->state = STATE_HOSTKEYS;
            break;
        default:
            LOG_ERROR_PREFIX("Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_STOP:
        break;
    }
    return SUCCESS;
}

static const char *yaml_filename = "conf/net.yaml";

int
parse_yaml(parser_state_t **state)
{

    FILE *yamlf = fopen(yaml_filename, "r");
    if (!yamlf) {
        perror("yamlf fopen");
        return 1;
    }

    yaml_parser_t parser;
    machine_states_t status;

    machine_states_t expected_state = STATE_START;

    *state = (parser_state_t *)panic_alloc(sizeof(parser_state_t));
    (*state)->state = STATE_START;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, yamlf);

    do {
        yaml_event_t event;

        status = yaml_parser_parse(&parser, &event);

        if (status == FAILURE) {
            LOG_ERROR_PREFIX("yaml_parser_parse error\n");
            destroy_parser_state(*state);
            
            yaml_parser_delete(&parser);
            fclose(yamlf);
            return EXIT_FAILURE;
        }

        status = handle_event(*state, &event, &expected_state);
        yaml_event_delete(&event);

        if (status == FAILURE) {
            LOG_ERROR_PREFIX("handle_event error\n");
            destroy_parser_state(*state);

            yaml_parser_delete(&parser);
            fclose(yamlf);
            return EXIT_FAILURE;
            
        }
    } while ((*state)->state != STATE_STOP);

    yaml_parser_delete(&parser);
    fclose(yamlf);
    
    return EXIT_SUCCESS;
}