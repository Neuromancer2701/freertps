//
// Created by root on 1/26/17.
//

#include <include/freertps/part.h>
#include "Sedp.h"

Sedp::Sedp():
g_sedp_sub_pub(nullptr),
g_sedp_pub_pub(nullptr),
g_sedp_pub_writer_id(0xc2030000),
g_sedp_pub_reader_id(0xc7030000),
g_sedp_sub_writer_id(0xc2040000),
g_sedp_sub_reader_id(0xc7040000)
{}

Sedp::~Sedp() {
    FREERTPS_INFO("sedp fini\r\n");
}


void Sedp::frudp_sedp_init(void)
{
    FREERTPS_INFO("sedp init\r\n");
    for (int i = 0; i < FRUDP_MAX_SUBS; i++)
    {
        frudp_submsg_data_t *d =
                (frudp_submsg_data_t *)&g_sedp_sub_writer_data_buf[i * SEDP_MSG_BUF_LEN];
        d->writer_sn = g_frudp_sn_unknown;
        g_sedp_sub_writer_data_submsgs[i] = d;
    }
    for (int i = 0; i < FRUDP_MAX_PUBS; i++)
    {
        frudp_submsg_data_t *d =
                (frudp_submsg_data_t *)&g_sedp_pub_writer_data_buf[i * SEDP_MSG_BUF_LEN];
        d->writer_sn = g_frudp_sn_unknown;
        g_sedp_pub_writer_data_submsgs[i] = d;
    }
    g_sedp_sub_pub = frudp_create_pub(
            NULL, // no topic name
            NULL, // no type name
            g_sedp_sub_writer_id,
            g_sedp_sub_writer_data_submsgs,
            FRUDP_MAX_SUBS);

    g_sedp_pub_pub = frudp_create_pub(
            NULL, // no topic name
            NULL, // no type name
            g_sedp_pub_writer_id,
            g_sedp_pub_writer_data_submsgs,
            FRUDP_MAX_PUBS);

    frudp_sub_t sedp_sub_sub;
    sedp_sub_sub.topic_name = NULL;
    sedp_sub_sub.type_name = NULL;
    sedp_sub_sub.reader_eid = g_sedp_sub_reader_id;
    sedp_sub_sub.data_cb = frudp_sedp_rx_sub_data;
    sedp_sub_sub.msg_cb = NULL;
    sedp_sub_sub.reliable = true;
    frudp_add_sub(&sedp_sub_sub);

    frudp_sub_t sedp_pub_sub; // subscribe to the publisher announcers
    sedp_pub_sub.topic_name = NULL;
    sedp_pub_sub.type_name = NULL;
    sedp_pub_sub.reader_eid = g_sedp_pub_reader_id;
    sedp_pub_sub.data_cb = frudp_sedp_rx_pub_data;
    sedp_pub_sub.msg_cb = NULL;
    sedp_pub_sub.reliable = true;
    frudp_add_sub(&sedp_pub_sub);
}

void Sedp::frudp_sedp_start(void)
{
    // go through and send SEDP messages for all of our subscriptions
    for (int i = 0; i < g_frudp_num_subs; i++)
    {
        if (g_frudp_subs[i].topic_name) // user subs have topic names
        {
            printf("sending SEDP message about subscription [%s]\r\n",
                   g_frudp_subs[i].topic_name);
            sedp_publish_sub(&g_frudp_subs[i]);
        }
    }
    // now go through and send SEDP messages for all our publications
    for (int i = 0; i < g_frudp_num_pubs; i++)
    {
        if (g_frudp_pubs[i].topic_name)
        {
            printf("sending SEDP message about publication [%s]\r\n",
                   g_frudp_pubs[i].topic_name);
            sedp_publish_pub(&g_frudp_pubs[i]);
        }
    }
}

void Sedp::frudp_sedp_tick(void)
{
    const fr_time_t t = fr_time_now();
    if (fr_time_diff(&t, &frudp_sedp_last_bcast).seconds >= 1)
    {
        frudp_sedp_bcast();
        frudp_sedp_last_bcast = t;
    }
}

void Sedp::sedp_publish_sub(frudp_sub_t *sub)
{
    if (!g_sedp_sub_pub)
    {
        printf("woah there partner.\r\n" "you need to call frudp_part_create()\r\n");
        return;
    }
    printf("sedp_publish_sub(%s)\r\n", sub->topic_name);
    sedp_publish(sub->topic_name, sub->type_name, g_sedp_sub_pub, sub->reader_eid, false); // false means "this is for a subscription"
}

void Sedp::sedp_publish_pub(frudp_pub_t *pub)
{
    if (!g_sedp_pub_pub)
    {
        printf("woah there partner.\r\n" "you need to call frudp_part_create()\r\n");
        return;
    }
    printf("sedp_publish_pub(%s)\r\n", pub->topic_name);
    sedp_publish(pub->topic_name, pub->type_name, g_sedp_pub_pub, pub->writer_eid, true); // true means "this is for a publication"
}

void Sedp::sedp_add_builtin_endpoints(frudp_part_t *part)
{
    printf("adding endpoints for ");
    frudp_print_guid_prefix(&part->guid_prefix);
    printf("\r\n");

    frudp_reader_t pub_reader; // this reads the remote peer's publications
    pub_reader.writer_guid = g_frudp_guid_unknown;
    frudp_stuff_guid(&pub_reader.writer_guid, &part->guid_prefix, &g_sedp_pub_writer_id);
    pub_reader.reader_eid = g_sedp_pub_reader_id;
    pub_reader.max_rx_sn.low = 0;
    pub_reader.max_rx_sn.high = 0;
    pub_reader.data_cb = frudp_sedp_rx_pub_data;
    pub_reader.msg_cb = NULL;
    pub_reader.reliable = true;
    frudp_add_reader(&pub_reader);

    frudp_reader_t sub_reader; // this reads the remote peer's subscriptions
    sub_reader.writer_guid = g_frudp_guid_unknown;
    frudp_stuff_guid(&sub_reader.writer_guid, &part->guid_prefix, &g_sedp_sub_writer_id);
    sub_reader.reader_eid = g_sedp_sub_reader_id;
    sub_reader.max_rx_sn.low = 0;
    sub_reader.max_rx_sn.high = 0;
    sub_reader.data_cb = frudp_sedp_rx_sub_data;
    sub_reader.msg_cb = NULL;
    sub_reader.reliable = true;
    frudp_add_reader(&sub_reader);

    // blast our SEDP data at this participant
    frudp_send_sedp_msgs(part);
}


void Sedp::sedp_publish(const char *topic_name, const char *type_name, frudp_pub_t *pub, const frudp_eid_t eid,
                        const bool is_pub)
{
    // first make sure we have an spdp packet out first
    printf("sedp publish [%s] via SEDP EID 0x%08x\r\n", topic_name, (unsigned)freertps_htonl(pub->writer_eid.u));
    frudp_submsg_data_t *d = (frudp_submsg_data_t *)g_sedp_msg_buf;
    d->header.id = FRUDP_SUBMSG_ID_DATA;
    d->header.flags = FRUDP_FLAGS_LITTLE_ENDIAN |
                      //FRUDP_FLAGS_INLINE_QOS    |
                      FRUDP_FLAGS_DATA_PRESENT  ;
    d->header.len = 0;
    d->extraflags = 0;
    d->octets_to_inline_qos = 16; // ?
    d->reader_id = is_pub ? g_sedp_pub_reader_id : g_sedp_sub_reader_id;
    d->writer_id = is_pub ? g_sedp_pub_writer_id : g_sedp_sub_writer_id;
    //d->writer_sn = g_frudp_sn_unknown;
    d->writer_sn.high = 0;
    d->writer_sn.low = 0; // todo: increment this
    //frudp_parameter_list_item_t *inline_qos_param =
    //  (frudp_parameter_list_item_t *)d->data;
    /*
    inline_qos_param->pid = FRUDP_PID_KEY_HASH;
    inline_qos_param->len = 16;
    memcpy(inline_qos_param->value, &reader_guid, 16);
    */
    /////////////////////////////////////////////////////////////
    frudp_encapsulation_scheme_t *scheme = (frudp_encapsulation_scheme_t *)((uint8_t *)d->data);
    scheme->scheme = freertps_htons(FRUDP_SCHEME_PL_CDR_LE);
    scheme->options = 0;
    /////////////////////////////////////////////////////////////
    frudp_parameter_list_item_t *param = (frudp_parameter_list_item_t *)(((uint8_t *)scheme) + sizeof(*scheme));
    /////////////////////////////////////////////////////////////
    param->pid = FRUDP_PID_PROTOCOL_VERSION;
    param->len = 4;
    param->value[0] = 2;
    param->value[1] = 1;
    param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
    /////////////////////////////////////////////////////////////
    FRUDP_PLIST_ADVANCE(param);
    param->pid = FRUDP_PID_VENDOR_ID;
    param->len = 4;
    param->value[0] = (FREERTPS_VENDOR_ID >> 8) & 0xff;
    param->value[1] = FREERTPS_VENDOR_ID & 0xff;
    param->value[2] = param->value[3] = 0; // pad to 4-byte boundary
    /////////////////////////////////////////////////////////////
    FRUDP_PLIST_ADVANCE(param);
    param->pid = FRUDP_PID_ENDPOINT_GUID;
    param->len = 16;
    frudp_guid_t guid;
    guid.prefix = g_frudp_config.guid_prefix;
    guid.eid = eid;
    memcpy(param->value, &guid, 16);
    //printf("reader_guid = 0x%08x\n", htonl(reader_guid.entity_id.u));
    /////////////////////////////////////////////////////////////
    if (topic_name)
    {
        FRUDP_PLIST_ADVANCE(param);
        param->pid = FRUDP_PID_TOPIC_NAME;
        int topic_len = topic_name ? strlen(topic_name) : 0;
        uint32_t *param_topic_len = (uint32_t *)param->value;
        *param_topic_len = topic_len + 1;
        //*((uint32_t *)param->value) = topic_len + 1;
        memcpy(param->value + 4, topic_name, topic_len + 1);
        //param->value[4 + topic_len + 1] = 0; // null-terminate plz
        param->len = (4 + topic_len + 1 + 3) & ~0x3; // params must be 32-bit aligned
    }
    /////////////////////////////////////////////////////////////
    if (type_name)
    {
        FRUDP_PLIST_ADVANCE(param);
        param->pid = FRUDP_PID_TYPE_NAME;
        int type_len = strlen(type_name);
        uint32_t *value = (uint32_t *)param->value;
        *value = type_len + 1;
        memcpy(param->value + 4, type_name, type_len + 1);
        param->len = (4 + type_len + 1 + 3) & ~0x3; // params must be 32-bit aligned
    }
    /////////////////////////////////////////////////////////////
    // todo: follow the "reliable" flag in the subscription structure
    FRUDP_PLIST_ADVANCE(param);
    param->pid = FRUDP_PID_RELIABILITY;
    param->len = 12;
    frudp_qos_reliability_t *reliability = (frudp_qos_reliability_t *)param->value;
    reliability->kind = FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT;
    reliability->max_blocking_time.sec = 0;
    reliability->max_blocking_time.nanosec = 0x19999999; // todo: figure this out
    /////////////////////////////////////////////////////////////
    /*
    FRUDP_PLIST_ADVANCE(param);
    param->pid = FRUDP_PID_PRESENTATION;
    param->len = 8;
    frudp_qos_presentation_t *presentation = (frudp_qos_presentation_t *)param->value;
    presentation->scope = FRUDP_QOS_PRESENTATION_SCOPE_TOPIC;
    presentation->coherent_access = 0;
    presentation->ordered_access = 0;
    */
    /////////////////////////////////////////////////////////////
    FRUDP_PLIST_ADVANCE(param);
    param->pid = FRUDP_PID_SENTINEL;
    param->len = 0;
    FRUDP_PLIST_ADVANCE(param);
    d->header.len = param->value - 4 - (uint8_t *)&d->extraflags;
    frudp_publish(pub, d); // this will be either on the sub or pub publisher
}

void Sedp::frudp_sedp_rx_pub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data)
{
    frudp_sedp_rx_pubsub_data(rcvr, submsg, scheme, data, true);
}

void Sedp::frudp_sedp_rx_sub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data)
{
    frudp_sedp_rx_pubsub_data(rcvr, submsg, scheme, data, false);
}

void Sedp::frudp_sedp_rx_pubsub_data(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg, const uint16_t scheme, const uint8_t *data, const bool is_pub)
{
#ifdef SEDP_VERBOSE
    printf("  sedp_writer data rx\r\n");
#endif
    if (scheme != FRUDP_SCHEME_PL_CDR_LE)
    {
        FREERTPS_ERROR("expected sedp data to be PL_CDR_LE. bailing...\r\n");
        return;
    }
    memset(&g_topic_info, 0, sizeof(sedp_topic_info_t));
    frudp_parameter_list_item_t *item = (frudp_parameter_list_item_t *)data;
    while ((uint8_t *)item < submsg->contents + submsg->header.len)
    {
        const frudp_parameterid_t pid = item->pid;
        if (pid == FRUDP_PID_SENTINEL)
            break;
        const uint8_t *pval = item->value;
        if (pid == FRUDP_PID_ENDPOINT_GUID)
        {
            frudp_guid_t *guid = (frudp_guid_t *)pval;
            memcpy(&g_topic_info.guid, guid, sizeof(frudp_guid_t));
#ifdef SEDP_VERBOSE
            //memcpy(&part->guid_prefix, &guid->guid_prefix, FRUDP_GUID_PREFIX_LEN);
      uint8_t *p = guid->prefix.prefix;
      printf("    endpoint guid 0x%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x"
                                 "%02x%02x%02x%02x\n",
             p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
             p[8], p[9], p[10], p[11],
             p[12], p[13], p[14], p[15]);
#endif
            //if (guid->entity_id.u == 0x03010000)
            //  printf("found entity 0x103\n");
        }
        else if (pid == FRUDP_PID_TOPIC_NAME)
        {
            if (frudp_parse_string(g_topic_info.topic_name, sizeof(g_topic_info.topic_name), (frudp_rtps_string_t *)pval))
            {
#ifdef SEDP_PRINT_TOPICS
                printf("    topic name: [%s]\r\n", g_topic_info.topic_name);
#endif
            }
            else
                FREERTPS_ERROR("couldn't parse topic name\n");
        }
        else if (pid == FRUDP_PID_TYPE_NAME)
        {
            if (frudp_parse_string(g_topic_info.type_name, sizeof(g_topic_info.type_name), (frudp_rtps_string_t *)pval))
            {
#ifdef SEDP_VERBOSE
                printf("    type name: [%s]\r\n", g_topic_info.type_name);
#endif
            }
            else
                FREERTPS_ERROR("couldn't parse type name\r\n");
        }
        else if (pid == FRUDP_PID_RELIABILITY)
        {
            frudp_qos_reliability_t *qos = (frudp_qos_reliability_t *)pval;
            if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_BEST_EFFORT)
            {
#ifdef SEDP_VERBOSE
                printf("    reliability QoS: [best-effort]\r\n");
#endif
            }
            else if (qos->kind == FRUDP_QOS_RELIABILITY_KIND_RELIABLE)
            {
#ifdef SEDP_VERBOSE
                printf("    reliability QoS: [reliable]\r\n");
#endif
            }
            else
                FREERTPS_ERROR("unhandled reliability kind: %d\r\n", (int)qos->kind);
        }
        else if (pid == FRUDP_PID_HISTORY)
        {
            frudp_qos_history_t *qos = (frudp_qos_history_t *)pval;
            if (qos->kind == FRUDP_QOS_HISTORY_KIND_KEEP_LAST)
            {
#ifdef SEDP_VERBOSE
                printf("    history QoS: [keep last %d]\r\n", (int)qos->depth);
#endif
            }
            else if (qos->kind == FRUDP_QOS_HISTORY_KIND_KEEP_ALL)
            {
#ifdef SEDP_VERBOSE
                printf("    history QoS: [keep all]\r\n");
#endif
            }
            else
                FREERTPS_ERROR("unhandled history kind: %d\r\n", (int)qos->kind);
        }
        else if (pid == FRUDP_PID_TRANSPORT_PRIORITY)
        {
#ifdef SEDP_VERBOSE
            uint32_t priority = *((uint32_t *)pval);
      printf("    transport priority: %d\r\n", (int)priority);
#endif
        }
        // now, advance to next item in list...
        item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
    }
    // make sure we have received all necessary parameters
    if (!strlen(g_topic_info.type_name) ||
        !strlen(g_topic_info.topic_name) ||
        frudp_guid_identical(&g_topic_info.guid, &g_frudp_guid_unknown))
    {
        FREERTPS_ERROR("insufficient SEDP information\r\n");
        return;
    }
    if (is_pub) // this is information about someone else's publication
        frudp_sedp_rx_pub_info(&g_topic_info);
    else // this is information about someone else's subscription
        frudp_sedp_rx_sub_info(&g_topic_info);
}

