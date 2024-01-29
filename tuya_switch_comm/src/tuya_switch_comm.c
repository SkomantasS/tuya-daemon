#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include "become_daemon.h"
#include <argp.h>
#include "hsv2rgb.h"
#include "tuya_personal_funcions.h"

static char doc[] = "Program to communicate with tuya cloud";
static char args_doc[] = "Device-ID Pruduct-ID Device-secret";
static struct argp_option options[] = {
    {"device-ID", 'd', "Dev-ID", 0, "Enter Device ID", 0},
    {"product-ID", 'p', "Prod-ID", 0, "Enter Product ID ", 0},
    {"device-secret", 's', "Dev-S", 0, "Enter Device secret", 0},
    {"start-as-daemon", 'b', 0, 0, "Start as daemon", 0},
    {0}
};
static error_t parse_opt (int key, char *arg,struct argp_state *state)
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
            printf("Try `%s --help' or `%s --usage' for more information.\n",__FILE__,__FILE__);
            return(1);
        } else {
            printf("deviceId: %s\nproductId: %s\ndeviceSecret: %s\n",arguments->deviceId,arguments->productId,arguments->deviceSecret);
        }
        break;
    }
    return 0;
}
struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

tuya_mqtt_context_t client_instance;
tuya_mqtt_context_t* client = &client_instance;

static void sig_handler()
{
    tuya_mqtt_disconnect(client);
    tuya_mqtt_deinit(client);
    closelog();
    exit(0);
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
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

int main(int argc, char** argv)
{   
    int ret = OPRT_OK;

    //argp begin
    struct arguments arguments;
    arguments.start_as_daemon = 0;
    ret = argp_parse (&argp, argc, argv, 0, 0, &arguments);
    if (argc < 3){
        ret = -1;
        return ret;
    }
    // argp end

    signal(SIGINT,sig_handler); // Register signal handlers
    signal(SIGQUIT,sig_handler);
    signal(SIGTERM,sig_handler);

    const char *LOGNAME = "TUYA_COMM"; // Name and open logfile
    openlog(LOGNAME, LOG_PID, LOG_USER);
    
    // daemon begin
    if(arguments.start_as_daemon == 1){
        ret = become_daemon(0); 
        if(ret)
        {
            syslog(LOG_USER | LOG_ERR, "error starting daemon process");
            goto closelog;
        }
        syslog(LOG_USER | LOG_INFO, "starting daemon process");
    }
    // daemon end

    ret = tuya_mqtt_init_personal(client,&arguments,(&on_messages));
    if(ret != OPRT_OK){
        syslog(LOG_USER | LOG_INFO, "error initiating mqtt");
        goto closelog;
    }
    syslog(LOG_USER | LOG_INFO, "success initiating mqtt");

    ret = tuya_mqtt_connect(client);
    if(ret != OPRT_OK){
        syslog(LOG_USER | LOG_INFO, "error connecting to mqtt");
        goto deinit;
    }
    syslog(LOG_USER | LOG_INFO, "success connecting to mqtt");

    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        send_f_m_p_to_tuya(client);
        tuya_mqtt_loop(client);
    }
    tuya_mqtt_disconnect(client);
    deinit:
    tuya_mqtt_deinit(client);
    closelog:
    closelog();
    return ret;
}
