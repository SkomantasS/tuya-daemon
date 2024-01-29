#include "tuyalink_core.h"
#include "tuya_cacert.h"
#include "tuya_personal_funcions.h"
#include <syslog.h>
#include <sys/sysinfo.h>
#include <stdio.h>



int tuya_mqtt_init_personal(tuya_mqtt_context_t* client, struct arguments* arguments, void(*on_messages))
{
int ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
    .host = "m1.tuyacn.com",
    .port = 8883,
    .cacert = tuya_cacert_pem,
    .cacert_len = sizeof(tuya_cacert_pem),
    .device_id = arguments->deviceId,
    .device_secret = arguments->deviceSecret,
    .keepalive = 100,   
    .timeout_ms = 2000,
    .on_messages = on_messages
});
}

void send_f_m_p_to_tuya(tuya_mqtt_context_t* client)
{   
    struct sysinfo info;
    char free_memory_percent[120];
    if (sysinfo(&info) != 0){
        syslog(LOG_USER | LOG_ERR, "Error getting system info");
    } else {
        double freePercentage = ((double)info.freeram/info.totalram)*100.0;
        sprintf(free_memory_percent,"{\"f_m_p\":%.2f}",freePercentage);
        tuyalink_thing_property_report_with_ack(client, NULL,free_memory_percent);
        syslog(LOG_USER | LOG_INFO, "sending: %s",free_memory_percent);
    }
}