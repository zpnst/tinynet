#ifndef DEV_H
#define DEV_H

#define FPATH_S 256 
#define SOCKDIR "/tmp/tinynet"

#define BASEFI_COUNT   3
#define BUFFERFI_S     128
#define MSG_BUFFER_S   1024
#define TRACE_BUFFER_S 4096

#define CMD_BUFFER_S (TRACE_BUFFER_S + MSG_BUFFER_S + (BUFFERFI_S * 3))

typedef enum pdu_serializator_e {
    SEREALIZE_T,
    DESEREALIZE_T
} pdu_serializator_t;

typedef struct route_entry_s {
    char dest[BUFFERFI_S];
    char next_hop[BUFFERFI_S];
    struct route_entry_s *next;
} route_entry_t;


typedef struct tinynet_pdu_s {
    char src[BUFFERFI_S];
    char dest[BUFFERFI_S];
    char next_hop[BUFFERFI_S];
    char compr_msg[MSG_BUFFER_S];
    char trace_buf[TRACE_BUFFER_S];
} tinynet_pdu_t;

#endif /* DEV_H */