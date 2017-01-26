//
// Created by root on 1/18/17.
//

#ifndef FREERTPS_ID_H
#define FREERTPS_ID_H

#include <cstdint>
#include <array>
#include <string>

using std::array;
using std::string;

enum {
    FRUDP_ENTITY_KIND_USER_WRITER_WITH_KEY = 0x02,
    FRUDP_ENTITY_KIND_USER_WRITER_NO_KEY   = 0x03,
    FRUDP_ENTITY_KIND_USER_READER_NO_KEY   = 0x04,
    FRUDP_ENTITY_KIND_USER_READER_WITH_KEY = 0x07,
    FRUDP_GUID_PREFIX_LEN                  = 12,
    FREERTPS_VENDOR_ID                     = 0x2C2F
};

union __attribute__((packed)) frudp_eid_t  // entity ID
{
    struct s
    {
        uint8_t key[3];
        uint8_t kind;
    } s;
    uint32_t u;

    frudp_eid_t(): u(0){}
} ;

using frudp_guid_prefix_t = array<uint8_t, FRUDP_GUID_PREFIX_LEN>;
using frudp_vid_t =  uint16_t;

struct __attribute__((packed)) frudp_guid_t
{
    frudp_guid_prefix_t prefix;
    frudp_eid_t eid;
};

class ID {

    ID();

    static const frudp_eid_t frudp_eid_unknown_g;
    uint8_t frudp_next_user_eid;

    bool frudp_guid_prefix_identical(frudp_guid_prefix_t * const a, frudp_guid_prefix_t * const b);
    bool frudp_guid_identical(const frudp_guid_t * const a, const frudp_guid_t * const b);
    void frudp_stuff_guid(frudp_guid_t *guid,  const frudp_guid_prefix_t *prefix, const frudp_eid_t *id);
    void frudp_print_guid_prefix(const frudp_guid_prefix_t *guid_prefix);
    void frudp_print_guid(const frudp_guid_t *guid);
    string frudp_vendor(const frudp_vid_t vid);
    frudp_eid_t frudp_create_user_id(const uint8_t entity_kind);

};


#endif //FREERTPS_ID_H
