#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "tuya_cacert.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
#include <signal.h>
#include <sys/sysinfo.h>
//daemon begin
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include "become_daemon.h"
//daemon end

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

char deviceId[30];
char productId[30];
char deviceSecret[30];
int start_as_daemon = 0;
tuya_mqtt_context_t client_instance;
tuya_mqtt_context_t* client = &client_instance;

//argp begin
#include <argp.h>
static int parse_otp (int key, char *arg,struct argp_state *state)
{
    switch (key) {
    case 'd':
        strcpy(deviceId, arg);
        break;
    case 'p':
        strcpy(productId, arg);
        break;
    case 's':
        strcpy(deviceSecret, arg);
        break;
    case 'b':
        start_as_daemon = 1;
        printf("START AS DAEMON ENABLED");
        break;
    case ARGP_KEY_FINI:
        if (!strcmp(deviceId,"") || !strcmp(productId,"") || !strcmp(deviceSecret,"")) {
            printf("please enter deviceId, productId and deviceSecret.\n");
            printf("Try `%s --help' or `%s --usage' for more information.\n",__FILE__,__FILE__);
        } else {
            printf("deviceId: %s\nproductId: %s\ndeviceSecret: %s\n",deviceId,productId,deviceSecret);
        }
        break;
    }
    return 0;
}
//argp end

static rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

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
    signal(SIGINT,sig_handler); // Register signal handlers
    signal(SIGQUIT,sig_handler);

    const char *LOGNAME = "TUYA_COMM"; // Name and open logfile
    openlog(LOGNAME, LOG_PID, LOG_USER);

    int ret = OPRT_OK;

    //argp begin
    struct argp_option
        options[] = {
            {"device-ID", 'd', "NUM2", 0, "Device ID", 0},
            {"product-ID", 'p', "NUM1", 0, "Product ID ", 0},
            {"device-secret", 's', "NUM3", 0, "Device secret", 0},
            {"start-as-daemon", 'b', 0, 0, "Start as daemon?", 0},
            {0}
        };
    struct argp
        argp = {options, parse_otp, "", 0, 0, 0, 0};
    argp_parse (&argp, argc, argv, 0, 0, 0);
    //argp end

    // daemon begin
    if(start_as_daemon == 1){
        ret = become_daemon(0); 
        if(ret)
        {
            syslog(LOG_USER | LOG_ERR, "error starting daemon process");
            closelog();
            return EXIT_FAILURE;
        } else {
            syslog(LOG_USER | LOG_INFO, "starting daemon process");
        }
    }
    // daemon end

    // tuya_mqtt_context_t* client = &client_instance;
    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = deviceId,
        .device_secret = deviceSecret,
        .keepalive = 100,   
        .timeout_ms = 2000,
        .on_messages = on_messages
    });
    if(ret != OPRT_OK){
        syslog(LOG_USER | LOG_INFO, "error initiating mqtt");
        closelog();
        return(ret);
    }
    syslog(LOG_USER | LOG_INFO, "success initiating mqtt");

    ret = tuya_mqtt_connect(client);
    if(ret != OPRT_OK){
        tuya_mqtt_deinit(client);
        syslog(LOG_USER | LOG_INFO, "error connecting to mqtt");
        closelog();
        return(ret);
    }
    syslog(LOG_USER | LOG_INFO, "success connecting to mqtt");

    struct sysinfo info;
    char free_memory_percent[120];
    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        if (sysinfo(&info) != 0){
            syslog(LOG_USER | LOG_ERR, "Error getting system info");
        } else {
            double freePercentage = ((double)info.freeram/info.totalram)*100.0;
            sprintf(free_memory_percent,"{\"f_m_p\":%.2f}",freePercentage);
            tuyalink_thing_property_report_with_ack(client, NULL,free_memory_percent);
            syslog(LOG_USER | LOG_INFO, "sending: %s",free_memory_percent);
        }
        tuya_mqtt_loop(client);
    }
    return ret;
}
