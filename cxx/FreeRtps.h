//
// Created by root on 1/18/17.
//

#ifndef FREERTPS_FREERTPS_H
#define FREERTPS_FREERTPS_H

#include "ID.h"
#include "Pub.h"
#include "Sub.h"
#include "Config.h"
#include "Sedp.h"
#include "Spdp.h"

// maybe make this smarter someday
#define FREERTPS_INFO(...) \
  do { printf("freertps INFO : "); printf(__VA_ARGS__); } while (0)
#define FREERTPS_ERROR(...) \
  do { printf("freertps ERROR: "); printf(__VA_ARGS__); } while (0)
#define FREERTPS_FATAL(...) \
  do { printf("freertps FATAL: "); printf(__VA_ARGS__); } while (0)


union __attribute__((packed)) rtps_active_psms_t
{
    uint32_t val;
    struct rtps_active_psms_mask
    {
        uint32_t udp : 1;
        uint32_t ser : 1;
    } s;
};


class FreeRtps {

public:
    FreeRtps();

private:
    Sub subscriptions;
    Pub publishers;
    Sedp sedp;
    Spdp spdp;

    void freertps_create_sub(const char *topic_name, const char *type_name, freertps_msg_cb_t msg_cb);
    frudp_pub_t *freertps_create_pub(const char *topic_name, const char *type_name);
    bool freertps_publish(frudp_pub_t *pub, const uint8_t *msg, const uint32_t msg_len);
    void freertps_start(void);
    void freertps_init(void);
    void freertps_tick(void);  /// must be called periodically to broadcast SPDP

    bool g_freertps_init_complete;
    rtps_active_psms_t g_rtps_active_psms;


};


#endif //FREERTPS_FREERTPS_H
