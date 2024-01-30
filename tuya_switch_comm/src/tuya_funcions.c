#include "tuya_funcions.h"
#include "tuya_cacert.h"
#include "hsv2rgb.h"
#include <syslog.h>
#include <sys/sysinfo.h>
#include <stdio.h>

static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    // TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    syslog(LOG_USER | LOG_INFO,"on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
    case THING_TYPE_MODEL_RSP:
        // TY_LOGI("Model data:%s", msg->data_string);
        syslog(LOG_USER | LOG_INFO,"Model data:%s", msg->data_string);
        break;

    case THING_TYPE_PROPERTY_SET: ;
        char var_name[50];
        char var_var[50];
        char message_out[105];
        // TY_LOGI(msg->data_string);

        sscanf(msg->data_string,"{\"%49[^\"]\":%49[^}]\"}",var_name,var_var);
        sprintf(message_out,"{\"%s\":%s}",var_name,var_var);
        syslog(LOG_USER | LOG_INFO,"%s",message_out);
        tuyalink_thing_property_report_with_ack(context,NULL,message_out);
        syslog(LOG_USER | LOG_INFO,"property set:%s", msg->data_string);
        break;
    case THING_TYPE_ACTION_EXECUTE: ;
        hsv myHSV;
        sscanf(msg->data_string,"{\"inputParams\":{\"saturation_id\":%lf,\"hue_id\":%lf,\"value_id\":%lf}]},\"actionCode\":\"LED_controll\"}",&myHSV.s,&myHSV.h,&myHSV.v);
        syslog(LOG_USER | LOG_INFO,"H:%lf S:%lf V:%lf\n",myHSV.h,myHSV.s,myHSV.v);
        rgb myRGB = hsv2rgb(myHSV);
        syslog(LOG_USER | LOG_INFO,"R:%lf G:%lf B:%lf\n",myRGB.r,myRGB.g,myRGB.b);
        break;
    case THING_TYPE_PROPERTY_REPORT_RSP:
        break;
    default:
        break;
    }
    printf("\r\n");
}

int tuya_mqtt_init_personal(tuya_mqtt_context_t *client,struct arguments *arguments)
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
return ret;
}

void send_f_m_p_to_tuya(tuya_mqtt_context_t *client)
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