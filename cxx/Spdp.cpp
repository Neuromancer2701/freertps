//
// Created by root on 1/26/17.
//

#include "Spdp.h"
#include "FreeRtps.h"

enum {
    WRITER_ID = 0xc2000100,
    READER_ID = 0xc7000100,
};



Spdp::Spdp() : g_spdp_writer_id(WRITER_ID ), g_spdp_reader_id(READER_ID) {}

void Spdp::frudp_spdp_init(void)
{
    frudp_spdp_last_bcast.seconds = 0;
    frudp_spdp_last_bcast.fraction = 0;
    frudp_reader_t spdp_reader;
    spdp_reader.writer_guid = g_frudp_guid_unknown;
    spdp_reader.reader_eid = g_spdp_reader_id;
    spdp_reader.max_rx_sn.low = 0;
    spdp_reader.max_rx_sn.high = 0;
    spdp_reader.data_cb = frudp_spdp_rx_data;
    spdp_reader.msg_cb = NULL;
    spdp_reader.reliable = false;
    frudp_add_reader(&spdp_reader);
}

void Spdp::frudp_spdp_start(void)
{
    frudp_spdp_tick();
}

void Spdp::frudp_spdp_fini(void)
{
    FREERTPS_INFO("sdp fini\n");
}
/*
 *
 * Replace this time tick with a HAL wrapper that
 * can use any timing mechanism.
 *
 */
void Spdp::frudp_spdp_tick(void)
{
    const fr_time_t t = fr_time_now();
    if (fr_time_diff(&t, &frudp_spdp_last_bcast).seconds >= 1) // every second
        //if (fr_time_diff(&t, &frudp_spdp_last_bcast).fraction >= 1000000000) // every second
    {
        //printf("spdp bcast\r\n");
        frudp_spdp_bcast();
        frudp_spdp_last_bcast = t;
        //printf("%d participants known\n", (int)g_frudp_discovery_num_participants);
    }
}

void Spdp::frudp_spdp_bcast(void)
{
    //FREERTPS_INFO("spdp bcast\n");
    frudp_msg_t *msg = frudp_init_msg((frudp_msg_t *)g_frudp_disco_tx_buf);
    fr_time_t t = fr_time_now();
    uint16_t submsg_wpos = 0;

    frudp_submsg_t *ts_submsg = (frudp_submsg_t *)&msg->submsgs[submsg_wpos];
    ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
    ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
    ts_submsg->header.len = 8;
    memcpy(ts_submsg->contents, &t, 8);
    submsg_wpos += 4 + 8;

/*
  frudp_submsg_t *data_submsg =
*/
    frudp_submsg_data_t *data_submsg = (frudp_submsg_data_t *)&msg->submsgs[submsg_wpos];
    //(frudp_submsg_data_t *)data_submsg->contents;
    data_submsg->header.id = FRUDP_SUBMSG_ID_DATA;
    data_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN | RUDP_FLAGS_INLINE_QOS | FRUDP_FLAGS_DATA_PRESENT;
    data_submsg->header.len = 336; // need to compute this dynamically?
    data_submsg->extraflags = 0;
    data_submsg->octets_to_inline_qos = 16; // ?
    data_submsg->reader_id = g_frudp_eid_unknown;
    data_submsg->writer_id = g_spdp_writer_id;
    data_submsg->writer_sn.high = 0;
    //static uint32_t bcast_count = 0;
    data_submsg->writer_sn.low = 1; //++bcast_count;
    /////////////////////////////////////////////////////////////
    frudp_parameter_list_item_t *inline_qos_param = (frudp_parameter_list_item_t *)(((uint8_t *)data_submsg) + sizeof(frudp_submsg_data_t));
    inline_qos_param->pid = FRUDP_PID_KEY_HASH;
    inline_qos_param->len = 16;
    memcpy(inline_qos_param->value, &g_frudp_config.guid_prefix, 12);
    // now i don't know what i'm doing
    inline_qos_param->value[12] = 0;
    inline_qos_param->value[13] = 0;
    inline_qos_param->value[14] = 1;
    inline_qos_param->value[15] = 0xc1;
    PLIST_ADVANCE(inline_qos_param);

    inline_qos_param->pid = FRUDP_PID_SENTINEL;
    inline_qos_param->len = 0;
    /////////////////////////////////////////////////////////////
    frudp_encapsulation_scheme_t *scheme = (frudp_encapsulation_scheme_t *)(((uint8_t *)inline_qos_param) + 4);
    scheme->scheme = freertps_htons(FRUDP_SCHEME_PL_CDR_LE);
    scheme->options = 0;
    /////////////////////////////////////////////////////////////
    frudp_parameter_list_item_t *param_list =  (frudp_parameter_list_item_t *)(((uint8_t *)scheme) + sizeof(*scheme));
    param_list->pid = FRUDP_PID_PROTOCOL_VERSION;
    param_list->len = 4;
    param_list->value[0] = 2;
    param_list->value[1] = 1;
    param_list->value[2] = param_list->value[3] = 0; // pad to 4-byte boundary
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_VENDOR_ID;
    param_list->len = 4;
    param_list->value[0] = (FREERTPS_VENDOR_ID >> 8) & 0xff;
    param_list->value[1] = FREERTPS_VENDOR_ID & 0xff;
    param_list->value[2] = param_list->value[3] = 0; // pad to 4-byte boundary
    /////////////////////////////////////////////////////////////
    frudp_locator_t *loc = NULL;
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_DEFAULT_UNICAST_LOCATOR;
    param_list->len = sizeof(frudp_locator_t); // aka 24, minus jack bauer
    loc = (frudp_locator_t *)param_list->value;
    loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
    loc->port = frudp_ucast_user_port();
    memset(loc->addr.udp4.zeros, 0, 12);
    loc->addr.udp4.addr = g_frudp_config.unicast_addr;
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_DEFAULT_MULTICAST_LOCATOR;
    param_list->len = sizeof(frudp_locator_t);
    loc = (frudp_locator_t *)param_list->value;
    loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
    loc->port = frudp_mcast_user_port();
    memset(loc->addr.udp4.zeros, 0, 12);
    loc->addr.udp4.addr = freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP);
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_METATRAFFIC_UNICAST_LOCATOR;
    param_list->len = sizeof(frudp_locator_t); // aka 24, minus jack bauer
    loc = (frudp_locator_t *)param_list->value;
    loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
    loc->port = frudp_ucast_builtin_port();
    memset(loc->addr.udp4.zeros, 0, 12);
    loc->addr.udp4.addr = g_frudp_config.unicast_addr;
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_METATRAFFIC_MULTICAST_LOCATOR;
    param_list->len = sizeof(frudp_locator_t);
    loc = (frudp_locator_t *)param_list->value;
    loc->kind = FRUDP_LOCATOR_KIND_UDPV4;
    loc->port = frudp_mcast_builtin_port();
    memset(loc->addr.udp4.zeros, 0, 12);
    loc->addr.udp4.addr = freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP);
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_PARTICIPANT_LEASE_DURATION;
    param_list->len = 8;
    frudp_duration_t *duration = (frudp_duration_t *)param_list->value;
    duration->sec = 100;
    duration->nanosec = 0;
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_PARTICIPANT_GUID;
    param_list->len = 16;
    frudp_guid_t *guid = (frudp_guid_t *)param_list->value;
    memcpy(&guid->prefix, &g_frudp_config.guid_prefix, sizeof(frudp_guid_prefix_t));
    guid->eid.s.key[0] = 0;
    guid->eid.s.key[1] = 0;
    guid->eid.s.key[2] = 1;
    guid->eid.s.kind = 0xc1; // wtf
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_BUILTIN_ENDPOINT_SET;
    param_list->len = 4;
    uint32_t endpoint_set = 0x3f; //b; // 0x3f;
    memcpy(param_list->value, &endpoint_set, 4);
    /////////////////////////////////////////////////////////////
    PLIST_ADVANCE(param_list);
    param_list->pid = FRUDP_PID_SENTINEL;
    param_list->len = 0;
    //data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
    PLIST_ADVANCE(param_list);
    data_submsg->header.len = param_list->value - 4 - (uint8_t *)&data_submsg->extraflags;
    frudp_submsg_t *next_submsg_ptr = (frudp_submsg_t *)param_list;
    /////////////////////////////////////////////////////////////
    /*
    ts_submsg = (frudp_submsg_t *)param_list;
    ts_submsg->header.id = FRUDP_SUBMSG_ID_INFO_TS;
    ts_submsg->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN;
    ts_submsg->header.len = 8;
    memcpy(ts_submsg->contents, &t, 8);
    uint8_t *next_submsg_ptr = ((uint8_t *)param_list) + 4 + 8;
    */

    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    //data_submsg->header.len = next_submsg_ptr - data_submsg->contents;
    //printf("len = %d\n", data_submsg->header.len);
    /////////////////////////////////////////////////////////////
    //int payload_len = ((uint8_t *)param_list) - ((uint8_t *)msg->submsgs);
    //int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg->submsgs);
    int payload_len = ((uint8_t *)next_submsg_ptr) - ((uint8_t *)msg);
    if (!frudp_tx(freertps_htonl(FRUDP_DEFAULT_MCAST_GROUP), frudp_mcast_builtin_port(), (const uint8_t *)msg, payload_len))
        FREERTPS_ERROR("couldn't transmit SPDP broadcast message\r\n");
}
