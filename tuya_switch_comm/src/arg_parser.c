#include "arg_parser.h"
#include <string.h>

error_t parse_opt (int key, char *arg,struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch (key) {
    case 'd':
        strcpy(arguments->deviceId, arg);
        break;
    case 'p':
        strcpy(arguments->productId, arg);
        break;
    case 's':
        strcpy(arguments->deviceSecret, arg);
        break;
    case 'b':
        arguments->start_as_daemon = 1;
        printf("START AS DAEMON ENABLED");
        break;
    case ARGP_KEY_FINI:
        if (!strcmp(arguments->deviceId,"") || !strcmp(arguments->productId,"") || !strcmp(arguments->deviceSecret,"")) {
            printf("please enter deviceId, productId and deviceSecret.\n");
            printf("Try `%s --help' or `%s --usage' for more information.\n",state->argv[0],state->argv[0]);
            return(1);
        } else {
            printf("deviceId: %s\nproductId: %s\ndeviceSecret: %s\n",arguments->deviceId,arguments->productId,arguments->deviceSecret);
        }
        break;
    }
    return 0;
}