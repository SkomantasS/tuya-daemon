#include "tuyalink_core.h"
// #include <sys/sysinfo.h>

struct arguments
{
    char deviceId[30];
    char productId[30];
    char deviceSecret[30];
    int start_as_daemon;
};

int tuya_mqtt_init_personal(tuya_mqtt_context_t* client, struct arguments* arguments, void(*on_messages));
void send_f_m_p_to_tuya(tuya_mqtt_context_t* client);