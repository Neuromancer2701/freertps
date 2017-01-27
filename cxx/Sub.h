//
// Created by root on 1/18/17.
//

#ifndef FREERTPS_SUB_H
#define FREERTPS_SUB_H

#include "ID.h"
#include "UDP.h"
#include <vector>
using std::vector;

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


class Sub
{
    vector<frudp_sub_t> frudp_subs;
    vector<frudp_reader_t> frudp_readers;

    void frudp_add_reader(const frudp_reader_t reader);
    void frudp_print_readers(void);
    void frudp_add_user_sub(const char *topic_name, const char *type_name, freertps_msg_cb_t msg_cb);
    void frudp_add_sub(const frudp_sub_t *s);
};


#endif //FREERTPS_SUB_H
