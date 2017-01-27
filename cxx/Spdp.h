//
// Created by root on 1/26/17.
//

#ifndef FREERTPS_SPDP_H
#define FREERTPS_SPDP_H

#include "ID.h"

class Spdp {
    frudp_eid_t g_spdp_writer_id;
    frudp_eid_t g_spdp_reader_id;
    fr_time_t frudp_spdp_last_bcast;

public:
    Spdp();

private:
    void frudp_spdp_init(void);
    void frudp_spdp_start(void);
    void frudp_spdp_tick(void);
    void frudp_spdp_fini(void);

    void frudp_spdp_bcast(void);
};


#endif //FREERTPS_SPDP_H
