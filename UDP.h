//
// Created by root on 1/22/17.
//

#ifndef FREERTPS_UDP_H
#define FREERTPS_UDP_H

#include <cstdint>
#include "ID.h"

struct frudp_pver_t
{
    uint8_t major;
    uint8_t minor;
}; // protocol version

struct frudp_header_t
{
    uint32_t magic_word; // RTPS in ASCII
    frudp_pver_t pver; // protocol version
    frudp_vid_t  vid;  // vendor ID
    frudp_guid_prefix_t guid_prefix;
};

struct frudp_msg_t
{
    frudp_header_t header;
    uint8_t submsgs[];
};


enum {
    FRUDP_FLAGS_LITTLE_ENDIAN      = 0x01,
    FRUDP_FLAGS_INLINE_QOS         = 0x02,
    FRUDP_FLAGS_DATA_PRESENT       = 0x04,
    FRUDP_FLAGS_ACKNACK_FINAL      = 0x02,
    FRUDP_SUBMSG_ID_ACKNACK        = 0x06,
    FRUDP_SUBMSG_ID_HEARTBEAT      = 0x07,
    FRUDP_SUBMSG_ID_INFO_TS        = 0x09,
    FRUDP_SUBMSG_ID_INFO_DEST      = 0x0e,
    FRUPG_SUBMSG_ID_HEARTBEAT_FRAG = 0x13,
    FRUDP_SUBMSG_ID_DATA           = 0x15,
    FRUDP_SUBMSG_ID_DATA_FRAG      = 0x16,
    FRUDP_SCHEME_CDR_LE            = 0x0001,
    FRUDP_SCHEME_PL_CDR_LE         = 0x0003
};

struct frudp_submsg_header_t
{
    uint8_t id;
    uint8_t flags;
    uint16_t len;
};

struct frudp_submsg_t
{
    frudp_submsg_header_t header;
    uint8_t contents[];
};

struct frudp_receiver_state_t
{
    frudp_pver_t        src_pver;
    frudp_vid_t         src_vid;
    frudp_guid_prefix_t src_guid_prefix;
    frudp_guid_prefix_t dst_guid_prefix;
    bool                have_timestamp;
    fr_time_t           timestamp;
};

struct frudp_sn_t
{
    int32_t high;
    uint32_t low;

    frudp_sn_t():
    high(-1),
    low(0)
    {}
}; // sequence number

struct frudp_sn_set_t
{
    frudp_sn_t bitmap_base;
    uint32_t num_bits;
    uint32_t bitmap[];
};

struct frudp_sn_set_32bits_t
{
    frudp_sn_t bitmap_base;
    uint32_t num_bits;
    uint32_t bitmap;
};

struct __attribute__((packed)) frudp_submsg_data_t
{
    frudp_submsg_header_t header;
    uint16_t extraflags;
    uint16_t octets_to_inline_qos;
    frudp_eid_t reader_id;
    frudp_eid_t writer_id;
    frudp_sn_t writer_sn;
    uint8_t data[];
};

struct __attribute__((packed)) frudp_submsg_data_frag_t
{
    struct frudp_submsg_header header;
    uint16_t extraflags;
    uint16_t octets_to_inline_qos;
    frudp_eid_t reader_id;
    frudp_eid_t writer_id;
    frudp_sn_t writer_sn;
    uint32_t fragment_starting_number;
    uint16_t fragments_in_submessage;
    uint16_t fragment_size;
    uint32_t sample_size;
    uint8_t data[];
};

struct  __attribute__((packed)) frudp_submsg_heartbeat_t
{
    frudp_submsg_header_t header;
    frudp_eid_t reader_id;
    frudp_eid_t writer_id;
    frudp_sn_t first_sn;
    frudp_sn_t last_sn;
    uint32_t count;
};

struct __attribute__((packed)) frudp_submsg_gap_t
{
    frudp_submsg_header_t header;
    frudp_eid_t reader_id;
    frudp_eid_t writer_id;
    frudp_sn_t gap_start;
    frudp_sn_set_t gap_end;
};

struct __attribute__((packed)) frudp_submsg_acknack_t
{
    frudp_eid_t reader_id;
    frudp_eid_t writer_id;
    frudp_sn_set_t reader_sn_state;
    // the "count" field that goes here is impossible to declare in legal C
};

struct __attribute__((packed)) frudp_submsg_info_dest_t
{
    frudp_guid_prefix_t guid_prefix;
};

using frudp_parameterid_t = uint16_t;
struct __attribute__((packed)) frudp_parameter_list_item_t
{
    frudp_parameterid_t pid;
    uint16_t len;
    uint8_t value[];
};

struct __attribute__((packed)) frudp_encapsulation_scheme_t
{
    uint16_t scheme;
    uint16_t options;
};

struct frudp_duration_t
{
    int32_t sec;
    uint32_t nanosec;
};

using frudp_builtin_endpoint_set_t = uint32_t ;

struct  frudp_rtps_string_t
{
    uint32_t len;
    uint8_t data[];
};

using frudp_rx_data_cb_t = void (*)(frudp_receiver_state_t *rcvr,
                                   const frudp_submsg_t *submsg,
                                   const uint16_t scheme,
                                   const uint8_t *data);

class UDP {

public:
    static const frudp_sn_t frudp_sn_unknown_g;

    bool frudp_init(void);
    void frudp_fini(void);

    bool frudp_generic_init(void);
    bool frudp_init_participant_id(void);

    bool frudp_add_mcast_rx(const uint32_t group, const uint16_t port);

    bool frudp_add_ucast_rx(const uint16_t port);

    bool frudp_listen(const uint32_t max_usec);

    bool frudp_rx(const uint32_t src_addr,
                  const uint16_t src_port,
                  const uint32_t dst_addr,
                  const uint16_t dst_port,
                  const uint8_t *rx_data,
                  const uint16_t rx_len);

    bool frudp_tx(const uint32_t dst_addr,
                  const uint16_t dst_port,
                  const uint8_t *tx_data,
                  const uint16_t tx_len);

    uint16_t frudp_ucast_builtin_port(void);
    uint16_t frudp_mcast_builtin_port(void);
    uint16_t frudp_ucast_user_port(void);
    uint16_t frudp_mcast_user_port(void);
    uint16_t frudp_spdp_port(void);

    const char *frudp_ip4_ntoa(const uint32_t addr);

    bool frudp_parse_string(char *buf, uint32_t buf_len, frudp_rtps_string_t *s);

    frudp_msg_t *frudp_init_msg(frudp_msg_t *buf);


    const static struct rtps_psm rtps_psm_udp_g;

private:

    bool frudp_rx_submsg        (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg);
    bool frudp_rx_acknack       (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg);
    bool frudp_rx_heartbeat     (frudp_receiver_state_t &rcvr, const frudp_submsg_t &submsg);
    bool frudp_rx_gap           (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg);
    bool frudp_rx_info_ts       (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg);
    bool frudp_rx_info_src      (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_info_reply_ip4(frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_dst           (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_reply         (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_nack_frag     (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_heartbeat_frag(frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}
    bool frudp_rx_data          (frudp_receiver_state_t &rcvr, const frudp_submsg_t &submsg);
    bool frudp_rx_data_frag     (frudp_receiver_state_t &rcvr, const frudp_submsg_t submsg) {return true;}

    void frudp_tx_acknack(const frudp_guid_prefix_t *guid_prefix,
                          const frudp_eid_t *reader_eid,
                          const frudp_guid_t *writer_guid,
                          const frudp_sn_set_t *set);

};


#endif //FREERTPS_UDP_H
