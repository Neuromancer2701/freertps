//
// Created by root on 1/25/17.
//

#ifndef FREERTPS_PUB_H
#define FREERTPS_PUB_H

#include "ID.h"
#include "UDP.h"
#include "Config.h"

struct frudp_pub_t
{
    const char           *topic_name;
    const char           *type_name;
    frudp_eid_t           writer_eid;
    frudp_sn_t            max_tx_sn_avail;
    frudp_sn_t            min_tx_sn_avail;
    uint32_t              num_data_submsgs;
    frudp_submsg_data_t **data_submsgs;
    uint32_t              next_submsg_idx;
    frudp_sn_t            next_sn;
    bool                  reliable;
};
//////////////////////////////////////////////////////////////
struct frudp_writer_t;
{
    frudp_guid_t reader_guid;
    frudp_eid_t writer_eid;
} // currently only supports best-effort connections
class Pub {
    vector<frudp_pub_t> g_frudp_pubs;
    vector<frudp_writer_t> g_frudp_writers;




};


#endif //FREERTPS_PUB_H
