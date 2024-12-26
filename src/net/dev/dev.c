#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>  

#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "./dev.h" // since multiple definition of `main'

static int 
is_host_device(const char *node_name)
{
    if (strncmp(node_name, "host_", 5) == 0) {
        return 1;
    }
    return 0;
}

static void 
log_message_to_file(const char *node_name, const char *message, const char *header)
{
    char log_file_path[FPATH_S];
    snprintf(log_file_path, sizeof(log_file_path),
             "%s/%s/messages.log", SOCKDIR, node_name);

    FILE *logf = fopen(log_file_path, "a");
    if (!logf) {
        perror("[node] fopen messages.log");
        return;
    }

    fprintf(logf, "%s: ", header);
    fprintf(logf, "%s\n", message);
    fclose(logf);
}

static const char *
find_next_hop(route_entry_t *table, const char *dest)
{
    route_entry_t *cur = table;

    while (cur) {
        if (strcmp(cur->dest, dest) == 0) {
            return cur->next_hop;
        }
        if (strcmp(cur->dest, "__all__") == 0) {
            return cur->next_hop;
        }
        cur = cur->next;
    }
    return NULL;
}

static void 
pdu_serializator(void *dest, void *src, pdu_serializator_t mode)
{
    __uint32_t offset;
    for (__uint32_t iter = 0; iter < BASEFI_COUNT; iter += 1) {
        offset = (BUFFERFI_S * iter);
        memcpy((char*)dest + offset, (char*)(src) + offset, BUFFERFI_S); 
    }
    memcpy((char*)dest + offset, (char*)(src) + offset, MSG_BUFFER_S); 
    offset += MSG_BUFFER_S;
    memcpy((char*)dest + offset, (char*)(src) + offset, TRACE_BUFFER_S); 

    if (mode == SEREALIZE_T) {
        if (offset < CMD_BUFFER_S) {
            *((char*)dest + offset) = '\0';
        }
    }
}

static int 
forward_pdu(const tinynet_pdu_t *pdu)
{
    char sock_path[FPATH_S];
    snprintf(sock_path, sizeof(sock_path),
             "%s/%s/%s.sock", SOCKDIR, pdu->next_hop, pdu->next_hop);

    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("[node] socket");
        return -1;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path)-1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("[node] connect");
        close(client_fd);
        return -1;
    }

    char buffer[CMD_BUFFER_S];
    memset(buffer, 0, sizeof(buffer));
    pdu_serializator(buffer, (void*)pdu, SEREALIZE_T);

    if (write(client_fd, buffer, CMD_BUFFER_S) < 0) {
        perror("[node] write pdu");
    }

    close(client_fd);
    return EXIT_SUCCESS;
}

static route_entry_t* 
parse_routes(const char *routes_str)
{
    if (!routes_str || !*routes_str) {
        return NULL;
    }

    char *copy = strdup(routes_str);
    if (!copy) return NULL;

    route_entry_t *head = NULL;
    route_entry_t *tail = NULL;

    char *tmp;

    char *token = strtok_r(copy, ";", &tmp);
    while (token) {
        char *colon = strchr(token, ':');
        if (!colon) {
            fprintf(stderr, "[node] parse_routes: invalid '%s'\n", token);
            token = strtok_r(NULL, ";", &tmp);
            continue;
        }
        *colon = '\0';
        const char *dest_part = token;
        const char *nhop_part = colon + 1;

        route_entry_t *entry = calloc(1, sizeof(*entry));
        if (!entry) {
            perror("[node] calloc route_entry_t");
            free(copy);
            return head;
        }
        strncpy(entry->dest, dest_part, BUFFERFI_S - 1);
        strncpy(entry->next_hop, nhop_part, BUFFERFI_S - 1);

        if (!head) {
            head = tail = entry;
        } else {
            tail->next = entry;
            tail = entry;
        }
        token = strtok_r(NULL, ";", &tmp);
    }

    free(copy);
    return head;
}

static void 
handle_incoming_data(const char *node_name, route_entry_t *route_table,int is_host, const char *payload)
{
    if (strncmp(payload, "SEND TO ", 8) == 0) {
        char dest[BUFFERFI_S];
        char msg[MSG_BUFFER_S];
        memset(dest, 0, sizeof(dest));
        memset(msg, 0, sizeof(msg));

        const char *ptr = payload + 8; 

        int scanned = sscanf(ptr, "%127s MSG %1023[^\n]", dest, msg);
        if (scanned < 2) {
            printf("[%s] Invalid command: %s\n", node_name, payload);
            return;
        }

        tinynet_pdu_t pdu;
        memset(&pdu, 0, sizeof(pdu));
        strncpy(pdu.src, node_name, BUFFERFI_S);
        strncpy(pdu.dest, dest, BUFFERFI_S);
        strncpy(pdu.compr_msg, msg, MSG_BUFFER_S);

        snprintf(pdu.trace_buf, sizeof(pdu.trace_buf), "<%s --> ", node_name);

        const char *nh = find_next_hop(route_table, dest);
        if (!nh) {
            printf("[%s] No route to %s\n", node_name, dest);
            return;
        }
        strncpy(pdu.next_hop, nh, BUFFERFI_S);

        printf("[%s] Forwarding new message '%s' to %s via %s\n",
               node_name, msg, dest, pdu.next_hop);
        forward_pdu(&pdu);
    } else {
        tinynet_pdu_t inPDU;
        memset(&inPDU, 0, sizeof(inPDU));
        pdu_serializator(&inPDU, (void*)payload, DESEREALIZE_T);

        strncat(inPDU.trace_buf, node_name, TRACE_BUFFER_S - strlen(inPDU.trace_buf) - 1);

        if (is_host_device(node_name) == 1) {
            strncat(inPDU.trace_buf, ">", TRACE_BUFFER_S - strlen(inPDU.trace_buf) - 1);
        } else {
            strncat(inPDU.trace_buf, " --> ", TRACE_BUFFER_S - strlen(inPDU.trace_buf) - 1);
        }

        printf("[%s] Received PDU: src=%s, dest=%s, msg='%s'\n",
               node_name, inPDU.src, inPDU.dest, inPDU.compr_msg);

        if (strcmp(inPDU.dest, node_name) == 0 && is_host) {
            printf("[%s] I'm final dest. Logging message.\n", node_name);
            log_message_to_file(node_name, inPDU.compr_msg, "msg: ");
            log_message_to_file(node_name, inPDU.trace_buf, "trace: ");
        }
        else {
            const char *nh = find_next_hop(route_table, inPDU.dest);
            if (!nh) {
                printf("[%s] No route for %s\n", node_name, inPDU.dest);
            } else {
                strncpy(inPDU.next_hop, nh, BUFFERFI_S);
                printf("[%s] Forwarding PDU to next hop %s\n", node_name, nh);
                forward_pdu(&inPDU);
            }
        }
    }
}

int 
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <node_name> [route_table]\n", argv[0]);
        return 1;
    }

    char node_name[64];
    strncpy(node_name, argv[1], sizeof(node_name));
    node_name[sizeof(node_name)-1] = '\0';

    const char *routes_str = (argc >= 3) ? argv[2] : "";

    route_entry_t *route_table = parse_routes(routes_str);

    int is_host = is_host_device(node_name);

    {
        mkdir(SOCKDIR, 0777);
        char node_dir[FPATH_S];
        snprintf(node_dir, sizeof(node_dir), "%s/%s", SOCKDIR, node_name);
        mkdir(node_dir, 0777);
    }

    char sock_path[FPATH_S];
    snprintf(sock_path, sizeof(sock_path),
             "%s/%s/%s.sock", SOCKDIR, node_name, node_name);

    unlink(sock_path);

    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("[node] socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path));

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("[node] bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 10) == -1) {
        perror("[node] listen");
        close(listen_fd);
        return 1;
    }

    printf("[%s] Node is up, listening on %s\n", node_name, sock_path);

    while (1) {
        int conn_fd = accept(listen_fd, NULL, NULL);
        if (conn_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("[node] accept");
            break;
        }

        char buffer[CMD_BUFFER_S];
        memset(buffer, 0, sizeof(buffer));

        int ret = read(conn_fd, buffer, CMD_BUFFER_S);
        if (ret > 0) {
            buffer[ret] = '\0'; 
            handle_incoming_data(node_name, route_table, is_host, buffer);
        } else if (ret == 0) {
            // peer closed connection
        } else {
            perror("[node] read");
        }

        close(conn_fd);
    }

    close(listen_fd);

    route_entry_t *cur = route_table;
    while (cur) {
        route_entry_t *next = cur->next;
        free(cur);
        cur = next;
    }
    printf("[%s] Node shutting down.\n", node_name);
    return 0;
}