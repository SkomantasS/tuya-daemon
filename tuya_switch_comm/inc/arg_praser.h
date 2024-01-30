#include <argp.h>

struct arguments
{
    char deviceId[30];
    char productId[30];
    char deviceSecret[30];
    int start_as_daemon;
};

error_t parse_opt (int key, char *arg,struct argp_state *state);