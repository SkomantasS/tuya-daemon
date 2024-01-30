#include "tuyalink_core.h"
#include "arg_praser.h"

static void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg);
int tuya_mqtt_init_personal(tuya_mqtt_context_t *client, struct arguments *arguments);
void send_f_m_p_to_tuya(tuya_mqtt_context_t* client);