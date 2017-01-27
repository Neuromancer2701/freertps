//
// Created by root on 1/18/17.
//

#include "FreeRtps.h"

FreeRtps::FreeRtps() : g_freertps_init_complete(false) {}

void FreeRtps::freertps_create_sub(const char *topic_name, const char *type_name, freertps_msg_cb_t msg_cb)
{
    frudp_add_user_sub(topic_name, type_name, msg_cb);
}

frudp_pub_t *FreeRtps::freertps_create_pub(const char *topic_name, const char *type_name)
{
    return frudp_create_user_pub(topic_name, type_name);
}

bool FreeRtps::freertps_publish(frudp_pub_t *pub, const uint8_t *msg, const uint32_t msg_len)
{
    return frudp_publish_user_msg(pub, msg, msg_len);
}
void FreeRtps::freertps_init(void)
{
    FREERTPS_INFO("FreeRTPS Init\r\n");
    frudp_spdp_init();
    frudp_sedp_init();
}

void FreeRtps::freertps_start(void)
{
    FREERTPS_INFO("FreeRTPS Start\r\n");
    frudp_spdp_start();
    frudp_sedp_start();
}

void FreeRtps::freertps_tick(void)
{
    frudp_spdp_tick();
    frudp_sedp_tick();
}

