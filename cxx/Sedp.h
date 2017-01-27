//
// Created by root on 1/26/17.
//

#ifndef FREERTPS_SEDP_H
#define FREERTPS_SEDP_H

#include "FreeRtps.h"

struct sedp_topic_info_t
{
    frudp_guid_t guid;
    char topic_name[FRUDP_MAX_TOPIC_NAME_LEN];
    char type_name[FRUDP_MAX_TYPE_NAME_LEN];
};

class Sedp {

public:
    Sedp();
    virtual ~Sedp();

    void frudp_sedp_init(void);
    void frudp_sedp_start(void);
    void frudp_sedp_tick(void);


    void sedp_publish_sub(frudp_sub_t *sub);
    void sedp_publish_pub(frudp_pub_t *pub);
    void sedp_add_builtin_endpoints(frudp_part_t *part);

    frudp_pub_t *g_sedp_sub_pub;
    frudp_pub_t *g_sedp_pub_pub;

    vector<uint8_t> g_sedp_sub_writer_data_buf;
    vector<uint8_t> g_sedp_pub_writer_data_buf;
    vector<frudp_submsg_data_t> g_sedp_sub_writer_data_submsgs;
    vector<frudp_submsg_data_t> g_sedp_pub_writer_data_submsgs;
    vector<uint8_t> g_sedp_msg_buf;

    sedp_topic_info_t g_topic_info;

private:

    const frudp_eid_t g_sedp_pub_writer_id;
    const frudp_eid_t g_sedp_pub_reader_id;
    const frudp_eid_t g_sedp_sub_writer_id;
    const frudp_eid_t g_sedp_sub_reader_id;

    void frudp_sedp_rx_pub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data);
    void frudp_sedp_rx_sub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data);
    void frudp_sedp_rx_pubsub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data, const bool is_pub);
    void frudp_sedp_bcast(void){} //commented out in the original program.

    void sedp_publish(const char *topic_name, const char *type_name, frudp_pub_t *pub, const frudp_eid_t eid, const bool is_pub); // is this for a pub or a sub

};


#endif //FREERTPS_SEDP_H
