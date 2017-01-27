//
// Created by root on 1/26/17.
//

#ifndef FREERTPS_PARTICIPATE_H
#define FREERTPS_PARTICIPATE_H

#include "FreeRtps.h"


#define FRUDP_LOCATOR_KIND_INVALID -1
#define FRUDP_LOCATOR_KIND_RESERVED 0
#define FRUDP_LOCATOR_KIND_UDPV4    1
#define FRUDP_LOCATOR_KIND_UDPV6    2

struct __attribute__((packed)) frudp_locator_t
{
    int32_t kind;
    uint32_t port;
    union
    {
        uint8_t raw[16];
        struct
        {
            uint8_t zeros[12];
            uint32_t addr;
        } udp4;
    } addr;
};


struct frudp_part_t
{
  frudp_pver_t pver;
  frudp_vid_t vid;
  frudp_guid_prefix_t guid_prefix;
  bool expects_inline_qos;
  frudp_locator_t default_unicast_locator;
  frudp_locator_t default_multicast_locator;
  frudp_locator_t metatraffic_unicast_locator;
  frudp_locator_t metatraffic_multicast_locator;
  frudp_duration_t lease_duration;
  frudp_builtin_endpoint_set_t builtin_endpoints;
};

class Participate {

    bool g_frudp_participant_init_complete;

    frudp_part_t *frudp_part_find(const frudp_guid_prefix_t *guid_prefix);
    bool frudp_part_create(void);
};


#endif //FREERTPS_PARTICIPATE_H
