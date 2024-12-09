#include <yaml.h>

#include "src/tinynet.h"
#include "src/utils/yaml/parser.h"
#include "src/utils/yaml/construct.h"

#include "src/utils/addr/ip.h"
#include "src/utils/addr/mac.h"

enum status {
    SUCCESS = 1,
    FAILURE = 0
};

/** Network configuratin parser states. */
enum state {

    /** Libyaml states */
    STATE_START,   
    STATE_STREAM,  
    STATE_DOCUMENT,
    STATE_SECTION, 

    /** Network metadata states */ 
    STATE_NETVALUES,
    STATE_NETKEYS,
    STATE_NETNAME,
    STATE_NETDESCRIPTION,
    STATE_WANLIST,

    /** Routers states */
    STATE_WANVALUES,
    STATE_WANKEYS,

    STATE_ROUTERNAME,
    STATE_ROUTERIP,
    STATE_ROUTERMAC,
    STATE_LANLIST,

    /** Switches states */
    STATE_LANVALUES,
    STATE_LANKEYS,

    STATE_SWITCHNAME,
    STATE_SWITCHIP,
    STATE_SWITCHMAC,
    STATE_HOSTSLIST,

    /** Host states */
    STATE_HOSTVALUES,
    STATE_HOSTKEYS,

    STATE_HOSTNAME,
    STATE_HOSTIP,
    STATE_HOSTMAC,

    /** End state */
    STATE_STOP     
};

/** parser state. */
struct parser_state {
    enum state state;                         /** The current parse state */

    dev_basic_info_t host;                    /** Host buffer. */
    dev_basic_info_t switch_;                     /** LAN(switch) buffer. */
    dev_basic_info_t router;                     /** WAN(router) buffer. */

    abs_dev_t *host_list;                     /** Temporary buffer for each LAN. */
    abs_dev_t *lans_list;                     /** Temporary buffer for each WAN. */

    abs_dev_t *wans_list;                     /** Master list of WANs. */

    tinynet_char_t *net_conf_name;           /** Master network configuration name. */
    tinynet_char_t *net_conf_description;    /** Master network configuration description. */
};


int
parse_yaml(tinynet_conf_t *net_conf, const tinynet_char_t *yaml_filename)
{

    FILE *yamlf = fopen(yaml_filename, "r");
    if (!yamlf) {
        perror("yamlf fopen");
        return 1;
    }


    int code;
    enum status status;
    struct parser_state state;

    yaml_parser_t parser;

    memset(&state, 0, sizeof(state));
    state.state = STATE_START;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, yamlf);

    do {
        yaml_event_t event;

        status = yaml_parser_parse(&parser, &event);
        if (status == FAILURE) {
            fprintf(stderr, "yaml_parser_parse error\n");
            code = EXIT_FAILURE;
            goto done;
        }
        status = consume_event(&state, &event);
        yaml_event_delete(&event);
        if (status == FAILURE) {
            fprintf(stderr, "consume_event error\n");
            code = EXIT_FAILURE;
            goto done;
        }
    } while (state.state != STATE_STOP);

    /* Output the parsed data. */
    for (struct fruit *f = state.flist; f; f = f->next) {
        printf("fruit: name=%s, color=%s, count=%d\n", f->name, f->color, f->count);
        for (struct variety *v = f->varieties; v; v = v->next) {
            printf("  variety: name=%s, color=%s, seedless=%s\n", v->name, v->color, v->seedless ? "true" : "false");
        }
    }
    code = EXIT_SUCCESS;

    net_conf = (tinynet_conf_t *)panic_alloc(sizeof(tinynet_conf_t));
    net_conf->net_name = panic_strdup((tinynet_char_t *)state.net_conf_name);
    net_conf->net_description = panic_strdup((tinynet_char_t *)state.net_conf_description);
    net_conf->devs = state.wans_list;

done:
    free(state.net_conf_name);
    free(state.net_conf_description);
    
    destroy_routers(&state.router);
    destroy_switches(&state.switch_);
    destroy_hosts($state.host)

    yaml_parser_delete(&parser);
    return code;
}

int
handle_event(struct parser_state *s, yaml_event_t *event)
{
    tinynet_char_t *value;

    if (debug) {
        printf("state=%d event=%d\n", s->state, event->type);
    }
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
        case YAML_MAPPING_START_EVENT: // mapping for tinynet section
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
            s->state = STATE_NETKEYS;
            break;
        }
        case YAML_MAPPING_END_EVENT:
            printf("YAML_MAPPING_END_EVENT in STATE_NETVALUES")
            s->state = STATE_SECTION;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d, you must provide 'name' and 'description' to tinynet section\n", event->type, s->state);
            return FAILURE;

    case STATE_NETKEYS:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            value = (char *)event->data.scalar.value;
            if (strcmp(value, "net_name") == 0) {
                s->state = STATE_NETNAME;
            } else if (strcmp(value, "net_description") == 0) {
                s->state = STATE_NETDESCRIPTION;
            } else if (strcmp(value, "wan_devs") == 0) {
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

    case STATE_NETNAME:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_name) {
                fprintf(stderr, "Warning: duplicate 'net_name' key.\n");
                free(s->net_conf_name);
            }
            s->net_conf_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_NETDESCRIPTION:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->net_conf_description) {
                fprintf(stderr, "Warning: duplicate 'net_description' key.\n");
                free(s->net_conf_name);
            }
            s->net_conf_description = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            s->state = STATE_NETKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_WANLIST:
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
            } else if (strcmp(value, "router_lan_devs") == 0) { /** Change parsin level*/
                s->state = STATE_LANLIST;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            add_router(&s->wans_list, ROUTER_T, s->router, s->lans_list)
            memset(s->router 0, sizeof(s->router));

            s->state = STATE_WANVALUES;

            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERNAME:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.dev_name) {
                fprintf(stderr, "Warning: duplicate 'router_name' key.\n");
                free(s->router.dev_name);
            }
            s->router.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERIP:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.ip_addr) {
                fprintf(stderr, "Warning: duplicate 'router_ip' key.\n");
                free(s->router.ip_addr);
            }
            s->router.ip_addr = (ip_addr_t *)panic_alloc(sizeof(ip_addr_t))
            tinynet_char_t ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_ip_addr(s->router.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }
            
            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_ROUTERMAC:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->router.mac_addr) {
                fprintf(stderr, "Warning: duplicate 'router_mac' key.\n");
                free(s->router.mac_addr);
            }
           
            s->router.mac_addr = (mac_addr_t *)panic_alloc(sizeof(mac_addr_t))
            tinynet_char_t mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_mac_addr(s->router.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }

            s->state = STATE_WANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    /** LAN(SWITCHES) STATES */

    case STATE_LANLIST:
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
            } else if (strcmp(value, "switch_hosts") == 0) { /** Change parsin level*/
                s->state = STATE_HOSTSLIST;
            } else {
                fprintf(stderr, "Unexpected key: %s\n", value);
                return FAILURE;
            }
            break;
        case YAML_MAPPING_END_EVENT:

            add_switch(&s->lans_list, SWITCH_T, s->switch_, s->host_list)
            memset(s->switch_ 0, sizeof(s->switch_));

            s->state = STATE_LANVALUES;

            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;



    case STATE_SWITCHNAME:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.dev_name) {
                fprintf(stderr, "Warning: duplicate 'switch_name' key.\n");
                free(s->switch_.dev_name);
            }
            s->switch_.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            s->switch_ = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SWITCHIP:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.ip_addr) {
                fprintf(stderr, "Warning: duplicate 'switch_ip' key.\n");
                free(s->switch_.ip_addr);
            }
            s->switch_.ip_addr = (ip_addr_t *)panic_alloc(sizeof(ip_addr_t))
            tinynet_char_t ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_ip_addr(s->switch_.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }
            
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_SWITCHMAC:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.mac_addr) {
                fprintf(stderr, "Warning: duplicate 'switch_mac' key.\n");
                free(s->switch_.mac_addr);
            }
           
            s->switch_.mac_addr = (mac_addr_t *)panic_alloc(sizeof(mac_addr_t))
            tinynet_char_t mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_mac_addr(s->switch_.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }

            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;


    /** HOSTS STATES */

    case STATE_HOSTSLIST:
        switch (event->type) {
        case YAML_SEQUENCE_START_EVENT:
            s->state = STATE_HOSTVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTVALUES:
        switch (event->type) {
        case YAML_MAPPING_START_EVENT:
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
            add_host(&s->host_list, HOST_T, s->host)
            memset(s->host 0, sizeof(s->host));
            s->state = STATE_HOSTVALUES;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTNAME:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->host.dev_name) {
                fprintf(stderr, "Warning: duplicate 'host_name' key.\n");
                free(s->host.dev_name);
            }
            s->host.dev_name = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            s->host = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTIP:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.ip_addr) {
                fprintf(stderr, "Warning: duplicate 'host_ip' key.\n");
                free(s->switch_.ip_addr);
            }
            s->switch_.ip_addr = (ip_addr_t *)panic_alloc(sizeof(ip_addr_t))
            tinynet_char_t ip_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_ip_addr(s->switch_.ip_addr, ip_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }
            
            s->state = STATE_LANKEYS;
            break;
        default:
            fprintf(stderr, "Unexpected event %d in state %d.\n", event->type, s->state);
            return FAILURE;
        }
        break;

    case STATE_HOSTMAC:
        switch (event->type) {
        case YAML_SCALAR_EVENT:
            if (s->switch_.mac_addr) {
                fprintf(stderr, "Warning: duplicate 'host_mac' key.\n");
                free(s->switch_.mac_addr);
            }
           
            s->switch_.mac_addr = (mac_addr_t *)panic_alloc(sizeof(mac_addr_t))
            tinynet_char_t mac_addr_str = panic_strdup((tinynet_char_t *)event->data.scalar.value);
            
            if (parse_mac_addr(s->switch_.mac_addr, mac_addr_str) != EXIT_SUCCESS) {
                fprintf(stderr, "Incorrect IP address: %s.\n", event->type, s->state);
                return FAILURE;
            }

            s->state = STATE_LANKEYS;
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


// void 
// dump_network_conf(tinynet_conf_t *net_conf,  *net_conf, size_t sep_level)
// {
// }