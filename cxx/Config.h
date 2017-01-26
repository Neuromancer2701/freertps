//
// Created by root on 1/25/17.
//

#ifndef FREERTPS_CONFIG_H
#define FREERTPS_CONFIG_H

#include "ID.h"

enum {

        FRUDP_DEFAULT_MCAST_GROUP = 0xefff0001,
        FRUDP_MAX_PUBS = 5,
        FRUDP_MAX_SUBS = 5,
        FRUDP_MAX_READERS = 50,
        FRUDP_MAX_WRITERS = 50,
        FRUDP_DISCO_MAX_PARTS = 50,

        FRUDP_MAX_TOPIC_NAME_LEN = 128,
        FRUDP_MAX_TYPE_NAME_LEN = 128,

};

struct  frudp_config_t
{
    frudp_guid_prefix_t guid_prefix;
    int participant_id;
    uint32_t domain_id;
    uint32_t unicast_addr;
};


#define VERBOSE_MSG_RX
//#define VERBOSE_HEARTBEAT
//#define VERBOSE_DATA
//#define VERBOSE_ACKNACK
//#define VERBOSE_GAP

//#define VERBOSE_TX_ACKNACK
//#define SEDP_VERBOSE


class Config {
    frudp_config_t g_frudp_config
};


#endif //FREERTPS_CONFIG_H
