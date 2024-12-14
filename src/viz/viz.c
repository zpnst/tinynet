// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

// #include "src/sys.h"  
// #include "src/tinynet.h"  

// #include "src/net/addr/ip.h"
// #include "src/net/addr/mac.h"

// static const char*
// get_color_by_device_type(device_e dev_type)
// {
//     switch (dev_type) {
//     case ROUTER_T:
//         return "purple";
//     case SWITCH_T:
//         return "blue";
//     case HOST_T:
//         return "black";
//     default:
//         panic("wrong device type");
//     }
// }

// static const char*
// get_shape_by_device_type(device_e dev_type) 
// {
//     switch (dev_type) {
//     case ROUTER_T:
//         return "diamond";
//     case SWITCH_T:
//         return "ellipse";
//     case HOST_T:
//         return "box";
//     default:
//         panic("wrong device type");
//     }
// }

// static void
// print_device(FILE *dotf, abs_dev_t *dev, int show_ip, int show_mac)
// {
//     if (dev == NULL) {
//         return;
//     }

//     const char *shape = get_shape_by_device_type(dev->basic_info.dev_type);
//     const char *color = get_color_by_device_type(dev->basic_info.dev_type);

//     char ip_str[IP_BUFFER_S];
//     char mac_str[MAC_BUFFER_S];

//     char label[256];
    
//     snprintf(label, sizeof(label), "%s", dev->basic_info.dev_name);

//     if (show_ip) {
//         ip_addr_to_string(&dev->basic_info.ip_addr, ip_str, sizeof(ip_str));

//         strncat(label, "\\n", sizeof(label) - strlen(label) - 1);
//         strncat(label, ip_str, sizeof(label) - strlen(label) - 1);
//     }

//     if (show_mac) {
//         mac_addr_to_string(&dev->basic_info.mac_addr, mac_str, sizeof(mac_str));

//         strncat(label, "\\n", sizeof(label) - strlen(label) - 1);
//         strncat(label, mac_str, sizeof(label) - strlen(label) - 1);
//     }

//     fprintf(dotf, "    \"%s\" [shape=%s, label=\"%s\", color=%s];\n", 
//             dev->basic_info.dev_name, 
//             shape,
//             label,
//             color);

//     abs_dev_t *child = dev->lower_devs_list;
//     while (child) {
//         print_device(dotf, child, show_ip, show_mac);
//         fprintf(dotf, "    \"%s\" -- \"%s\";\n", 
//                 dev->basic_info.dev_name,
//                 child->basic_info.dev_name);
//         child = child->next;
//     }
// }


// int
// write_dot_file(tinynet_conf_t *conf, const char *filename, int show_ip, int show_mac) 
// {
//     FILE *dotf = fopen(filename, "w");
//     if (!dotf) {
//         fprintf(stderr, "Failed to open file %s for writing.\n", filename);
//         return -1;
//     }

//     fprintf(dotf, "graph %s {\n", conf->net_name ? conf->net_name : "network");
//     fprintf(dotf, "    label=\"%s topology", conf->net_name ? conf->net_name : "");
//     if (conf->net_description && strlen(conf->net_description) > 0) {
//         fprintf(dotf, "\\n%s", conf->net_description);
//     }
//     fprintf(dotf, "\";\n");
//     fprintf(dotf, "    labelloc=top;\n");
//     fprintf(dotf, "    fontsize=12;\n");
//     fprintf(dotf, "    node [fontsize=10];\n");

//     //abs_dev_t *dev = conf->devs;
//     // abs_dev_t *prev_dev = NULL;

//     // while (dev) {
//     //     print_device(dotf, dev, show_ip, show_mac);
//     //     if (prev_dev) {
//     //         fprintf(dotf, "    \"%s\" -- \"%s\";\n", 
//     //                 prev_dev->basic_info.dev_name,
//     //                 dev->basic_info.dev_name);
//     //     }
//     //     prev_dev = dev;
//     //     dev = dev->next;
//     // }

//     // fprintf(dotf, "}\n");
//     // fclose(dotf);
//     return 0;
// }