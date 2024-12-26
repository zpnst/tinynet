#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>        
#include <signal.h>    
#include <errno.h>
  
#include <sys/wait.h>   
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>  
#include <sys/socket.h>

#include "src/tinynet.h"   
#include "src/alg/fw.h"
#include "src/viz/viz.h"
#include "src/fsm/construct.h"
#include "src/net/netman.h"

#define SOCKDIR "/tmp/tinynet"

static const char *DOT_FILE    = "conf/data/net.dot";
static const char *NODE_BINARY = "./src/net/dev/bin/xdev";


typedef struct device_info_s {
    char   name[64]; 
    pid_t  pid;   
} device_info_t;


static device_info_t *g_devices = NULL; 
static size_t         g_dev_count = 0;

static int 
send_command_to_host(const char *host_name, const char *cmd_line)
{
    char sock_path[FPATH_S];
    snprintf(sock_path, sizeof(sock_path),
             "%s/%s/%s.sock", SOCKDIR, host_name, host_name);

    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(client_fd);
        return -1;
    }

    ssize_t ret = write(client_fd, cmd_line, strlen(cmd_line));
    if (ret >= 0) {
        write(client_fd, "\n", 1);
    } else {
        perror("write");
    }

    close(client_fd);
    return 0;
}


static int 
net_up(tinynet_conf_t *network)
{
    if (g_devices) {
        printf("[manager] Network is already up.\n");
        return 0;
    }
    g_dev_count = network->net_graph->rc 
                + network->net_graph->sc 
                + network->net_graph->hc;

    g_devices = (device_info_t *)calloc(g_dev_count, sizeof(device_info_t));
    if (!g_devices) {
        perror("calloc g_devices");
        return -1;
    }

    for (size_t iter = 0; iter < g_dev_count; iter += 1) {
        const char *dev_name = get_device_name_by_index(network, iter);
        strncpy(g_devices[iter].name, dev_name, sizeof(g_devices[iter].name)-1);

        char routes_buf[TRACE_BUFFER_S];
        routes_buf[0] = '\0';

        char_node_t *table_node = network->ros_tables_list[iter];
        while (table_node) {
            strcat(routes_buf, table_node->entry);
            table_node = table_node->next;
            if (table_node) {
                strcat(routes_buf, ";");
            }
        }

        char *argv_exec[4];
        argv_exec[0] = (char *)NODE_BINARY;
        argv_exec[1] = (char *)dev_name;
        argv_exec[2] = routes_buf[0] ? routes_buf : (char *)"";
        argv_exec[3] = NULL;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(g_devices);
            g_devices = NULL;
            return -1;
        } 
        else if (pid == 0) {
            execv(NODE_BINARY, argv_exec); // execute xdev 
            perror("execv");
            exit(EXIT_FAILURE);
        } 
        else {
            g_devices[iter].pid = pid;
        }
    }
    sleep(1);
    printf("[manager] All devices started (%zu processes)\n", g_dev_count);
    return 0;
}


static void 
net_down(void)
{
    if (!g_devices) {
        printf("[manager] Network is not up.\n");
        return;
    }

    for (size_t iter = 0; iter < g_dev_count; iter += 1) {
        kill(g_devices[iter].pid, SIGTERM); 
    }

    for (size_t iter = 0; iter < g_dev_count; iter += 1) {
        int status;
        waitpid(g_devices[iter].pid, &status, 0);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("[manager] Device %s (pid=%d) exited with code %d\n",
                   g_devices[iter].name,
                   (int)g_devices[iter].pid,
                   code);
        } else {
            printf("[manager] Device %s (pid=%d) terminated abnormally\n",
                   g_devices[iter].name,
                   (int)g_devices[iter].pid);
        }
    }

    free(g_devices);
    g_devices = NULL;
    g_dev_count = 0;

    sleep(1);
    printf("[manager] All devices stopped and cleaned up.\n");
}

__int32_t 
init_tinynet(tinynet_conf_t **network) 
{
    int err = graph_by_config(network);
    if (err != EXIT_SUCCESS) {
        fprintf(stderr, "Failed to parse YAML file\n");
        return err;
    }

    build_viz_file(*network, DOT_FILE);
    floyd_warshall(*network);
    init_ros_tables(*network);

    return EXIT_SUCCESS;
}

static void 
dump_info(tinynet_conf_t *network) 
{   
    dump_to_png();
    dump_shortest_hops(network); printf("\n");
    dump_ros_tables(network); printf("\n");
    dump_net_links(network, ENANBLE_NET_INFO); printf("\n");
}

static void 
print_help_info() {
    printf("Network Manager of tinynet is ready <%d>.\n", getpid());
    printf("Commands:\n");
    printf("    1) net up   --> start all net processes\n");
    printf("    2) net down --> kill all net processes\n");
    printf("    3) send     --> send message from host to host, fromat: <src> <dest> <msg>\n");
    printf("    4) quit     --> quit manager\n");
    printf("    5) dump     --> dump info\n");
    printf("    6) help     --> print help info\n");
    printf("    7) clear    --> clear terminal output\n");
    printf("----------------------------------\n");
}

__int32_t 
netman_start(tinynet_conf_t **network)
{

    int rc = init_tinynet(network);
    if (rc != EXIT_SUCCESS) {
        fprintf(stderr, "init_tinynet failed\n");
        free(network);
        return EXIT_FAILURE;
    }

    int cmd_down = 0;

    print_help_info();

    while (1) {
        printf("tinynet> ");
        fflush(stdout);

        char line[1024];
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) {
            continue;
        }
        if (strcmp(line, "quit") == 0) {
            printf("[manager] Bye!\n");
            break;
        } else if (strcmp(line, "dump") == 0) {
            dump_info(*network);
        } else if (strcmp(line, "clear") == 0) {
            system("clear");
        } else if (strcmp(line, "help") == 0) {
            print_help_info();
        } else if (strcmp(line, "net up") == 0) {
            net_up(*network);
            cmd_down = 1;
        } else if (strcmp(line, "net down") == 0) {
            net_down();
            cmd_down = 2;
        } else if (strncmp(line, "send ", 5) == 0) {
        
            char cmd[BUFFERFI_S], host_src[BUFFERFI_S], host_dst[BUFFERFI_S], msg[MSG_BUFFER_S];
            memset(cmd, 0, sizeof(cmd));
            memset(host_src, 0, sizeof(host_src));
            memset(host_dst, 0, sizeof(host_dst));
            memset(msg, 0, sizeof(msg));

            int n = sscanf(line, "%127s %127s %127s %1023[^\n]",
                           cmd, host_src, host_dst, msg);
            if (n < 4) {
                printf("Usage: send <host_src> <host_dst> <message>\n");
                continue;
            }

            char command_str[MSG_BUFFER_S + (BUFFERFI_S * 3)];
            
            snprintf(command_str, sizeof(command_str),
                     "SEND TO %s MSG %s", host_dst, msg);

            if (send_command_to_host(host_src, command_str) == 0) {
                printf("[manager] Sent command to %s\n", host_src);
            } else {
                printf("[manager] Failed to send command to %s\n", host_src);
            }
        }
        else {
            printf("Unknown command: %s\n", line);
        }
    }
    
    if (cmd_down == 1) {
        net_down();
    }
    destroy_net_conf(*network);
    
    return 0;
}
