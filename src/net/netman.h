#ifndef NETMAN_H
#define NETMAN_H

#include "src/tinynet.h"

#define FPATH_S 256 

#define BUFFERFI_S     128
#define MSG_BUFFER_S   1024
#define TRACE_BUFFER_S 4096

#define CMD_BUFFER_S (TRACE_BUFFER_S + MSG_BUFFER_S + (BUFFERFI_S * 3))

__int32_t 
netman_start(tinynet_conf_t **network);

#endif /* NETMAN_H */