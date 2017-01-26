//
// Created by root on 1/18/17.
//

#ifndef FREERTPS_SUB_H
#define FREERTPS_SUB_H

#include "ID.h"

using freertps_msg_cb_t = void *;


struct frudp_sub_t
{
    string topic_name;
    const char *type_name;
    frudp_eid_t reader_eid;
    frudp_rx_data_cb_t data_cb;
    freertps_msg_cb_t msg_cb;
    bool reliable;
};

struct frudp_reader_t
{
    bool reliable;
    frudp_guid_t writer_guid;
    frudp_eid_t reader_eid;
    frudp_sn_t max_rx_sn;
    frudp_rx_data_cb_t data_cb;
    freertps_msg_cb_t msg_cb;
};


class Sub {
    vector<frudp_sub_t> g_frudp_subs;
    vector<frudp_reader_t> g_frudp_readers;




};


#endif //FREERTPS_SUB_H
