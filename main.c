#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/sys.h"
#include "src/net/netman.h"

int 
main(void) 
{   
    tinynet_conf_t *network = NULL;

    __int32_t ret = netman_start(&network);
    if (ret != EXIT_SUCCESS) {
        panic("init_tinynet\n");
    }

    return EXIT_SUCCESS;
}